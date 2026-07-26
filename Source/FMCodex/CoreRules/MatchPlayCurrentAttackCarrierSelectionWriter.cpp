#include "MatchPlayCurrentAttackCarrierSelectionWriter.h"

FMatchPlayCurrentAttackCarrierSelectionWriterResult
FMatchPlayCurrentAttackCarrierSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackCarrierSelectionRequest& Request)
{
	FMatchPlayCurrentAttackCarrierSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			Request);

	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackState& CurrentAttack =
		WorkingState.CurrentAttack;
	CurrentAttack.ActionPreparation.CarrierCardId =
		Request.CarrierCardId;
	CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;

	Result.SelectedCarrierCardId = Request.CarrierCardId;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackCarrierSelectionWriterErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
