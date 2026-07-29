#include "MatchPlayCurrentAttackHelperSelectionWriter.h"

#include "MatchPlayCurrentAttackHelperFinalization.h"

FMatchPlayCurrentAttackHelperSelectionWriterResult
FMatchPlayCurrentAttackHelperSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackHelperSelectionRequest& Request)
{
	FMatchPlayCurrentAttackHelperSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			Request);
	if (!Result.LegalityResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	const FMatchPlayValidatedHelperPresence Presence(
		FMatchPlayValidatedHelperPresence::FSelectedHelperTag(),
		Request.HelperCardId);
	FMatchPlayCurrentAttackHelperFinalization::ApplyFinalSelectedAction(
		WorkingState,
		Presence);

	Result.ReadyValidationResult =
		FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(
			WorkingState);
	if (!Result.ReadyValidationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackHelperSelectionWriterErrorCode
				::ReadyValidationFailed;
		Result.ErrorMessage =
			Result.ReadyValidationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionWriterErrorCode::None;
	return Result;
}
