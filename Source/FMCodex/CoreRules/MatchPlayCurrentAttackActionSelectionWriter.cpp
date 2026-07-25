#include "MatchPlayCurrentAttackActionSelectionWriter.h"

FMatchPlayCurrentAttackActionSelectionWriterResult
FMatchPlayCurrentAttackActionSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackActionSelectionRequest& Request,
	const FSkillRuleSnapshotSet& SkillRules)
{
	FMatchPlayCurrentAttackActionSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			Request,
			SkillRules);

	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackActionSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackState& CurrentAttack =
		WorkingState.CurrentAttack;
	CurrentAttack.bHasSelectedAction = true;
	CurrentAttack.SelectedAction.CarrierCardId =
		Request.CarrierCardId;
	CurrentAttack.SelectedAction.SkillId = Request.SkillId;
	CurrentAttack.SelectedAction.ActionType =
		Result.LegalityResult.ResolvedActionType;

	Result.SelectedAction = CurrentAttack.SelectedAction;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackActionSelectionWriterErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
