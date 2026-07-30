#include "MatchPlayCurrentAttackSkillSelectionWriter.h"

FMatchPlayCurrentAttackSkillSelectionWriterResult
FMatchPlayCurrentAttackSkillSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayCurrentAttackSkillSelectionRequest& Request)
{
	FMatchPlayCurrentAttackSkillSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			SkillRuleSet,
			Request);

	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackSkillSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackState& CurrentAttack =
		WorkingState.CurrentAttack;
	const FMatchPlaySkillParticipantRequirementResult& Requirement =
		Result.LegalityResult.ParticipantRequirementResult;

	if (Requirement.bCanBecomeReadyImmediately)
	{
		CurrentAttack.ActionPreparation.SkillId =
			Request.SkillId;
		CurrentAttack.ActionPreparation.ActionType =
			Result.LegalityResult.ResolvedActionType;
		CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent;
	}
	else
	{
		CurrentAttack.ActionPreparation.SkillId =
			Request.SkillId;
		CurrentAttack.ActionPreparation.ActionType =
			Result.LegalityResult.ResolvedActionType;
		CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
	}

	Result.SelectedSkillId = Request.SkillId;
	Result.SelectedActionType =
		Result.LegalityResult.ResolvedActionType;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionWriterErrorCode::None;
	return Result;
}
