#include "MatchPlayCurrentAttackMarkerSelectionWriter.h"

namespace MatchPlayCurrentAttackMarkerSelectionWriterImplementation
{
	FMatchPlayCurrentAttackMarkerSelectionWriterResult SelectInternal(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request)
	{
		(void)SkillRuleSet;
		FMatchPlayCurrentAttackMarkerSelectionWriterResult Result;
		Result.Request = Request;
		Result.BeforeState = BeforeState;
		Result.AfterState = BeforeState;
		Result.LegalityResult =
			FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator::Evaluate(
				BeforeState,
				Request);

		if (!Result.LegalityResult.bIsLegal)
		{
			Result.ErrorCode =
				EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode
					::LegalityFailed;
			Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
			return Result;
		}

		FMatchPlayState WorkingState = BeforeState;
		FMatchPlayCurrentAttackState& CurrentAttack =
			WorkingState.CurrentAttack;
		CurrentAttack.ActionPreparation.MarkerCardId =
			Request.MarkerCardId;
		CurrentAttack.ActionPreparation.SkillId = NAME_None;
		CurrentAttack.ActionPreparation.ActionType = ESkillRuleType::None;
		CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;
		CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		Result.bDeferredSkillSelection = true;
		Result.DeferredActionType = ESkillRuleType::None;

		Result.SelectedMarkerCardId = Request.MarkerCardId;
		Result.AfterState = MoveTemp(WorkingState);
		Result.bSuccess = true;
		Result.ErrorCode =
			EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode::None;
		Result.ErrorMessage.Empty();
		return Result;
	}
}

FMatchPlayCurrentAttackMarkerSelectionWriterResult
FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackMarkerSelectionRequest& Request)
{
	return MatchPlayCurrentAttackMarkerSelectionWriterImplementation
		::SelectInternal(BeforeState, nullptr, Request);
}

FMatchPlayCurrentAttackMarkerSelectionWriterResult
FMatchPlayCurrentAttackMarkerSelectionWriter::SelectWithSkillRules(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayCurrentAttackMarkerSelectionRequest& Request)
{
	(void)SkillRuleSet;
	return MatchPlayCurrentAttackMarkerSelectionWriterImplementation
		::SelectInternal(BeforeState, nullptr, Request);
}
