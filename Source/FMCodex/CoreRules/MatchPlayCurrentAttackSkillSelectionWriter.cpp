#include "MatchPlayCurrentAttackSkillSelectionWriter.h"

#include "MatchPlayElectiveBranchIntentRules.h"
#include "MatchPlayCurrentAttackHelperFinalization.h"

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
	const bool bParticipantsSelectedFirst =
		CurrentAttack.ActionPreparation.bSkillSelectionDeferred;

	CurrentAttack.ActionPreparation.SkillId = Request.SkillId;
	CurrentAttack.ActionPreparation.ActionType =
		Result.LegalityResult.ResolvedActionType;
	CurrentAttack.ActionPreparation.bSkillSelectionDeferred = false;

	if (bParticipantsSelectedFirst)
	{
		if (!Requirement.bRequiresRunner)
		{
			CurrentAttack.ActionPreparation.RunnerCardId = NAME_None;
			CurrentAttack.ActionPreparation.bHasHelper = false;
			CurrentAttack.ActionPreparation.HelperCardId = NAME_None;
		}
		if (MatchPlayElectiveBranchIntentRules::IsElectiveActionType(
			Result.LegalityResult.ResolvedActionType))
		{
			CurrentAttack.SelectionStage =
				EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent;
		}
		else
		{
			FMatchPlayCurrentAttackHelperFinalization::ApplyFinalSelectedAction(
				WorkingState,
				EMatchPlayElectiveBranchIntent::None);
		}
	}
	else if (Requirement.bCanBecomeReadyImmediately)
	{
		CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage
				::AwaitingBranchIntent;
	}
	else
	{
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
