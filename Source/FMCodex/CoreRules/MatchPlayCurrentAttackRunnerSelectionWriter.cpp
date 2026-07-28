#include "MatchPlayCurrentAttackRunnerSelectionWriter.h"

FMatchPlayCurrentAttackRunnerSelectionWriterResult
FMatchPlayCurrentAttackRunnerSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackRunnerSelectionRequest& Request)
{
	FMatchPlayCurrentAttackRunnerSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			Request);
	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage =
			Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	WorkingState.CurrentAttack.ActionPreparation.RunnerCardId =
		Request.RunnerCardId;
	WorkingState.CurrentAttack.SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;

	Result.SelectedRunnerCardId = Request.RunnerCardId;
	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionWriterErrorCode::None;
	return Result;
}
