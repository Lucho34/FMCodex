#include "MatchPlayCurrentAttackMarkerSelectionWriter.h"

FMatchPlayCurrentAttackMarkerSelectionWriterResult
FMatchPlayCurrentAttackMarkerSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackMarkerSelectionRequest& Request)
{
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
	CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;

	Result.SelectedMarkerCardId = Request.MarkerCardId;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackMarkerSelectionWriterErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
