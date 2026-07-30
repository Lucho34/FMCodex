#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

#include "MatchPlayCurrentAttackHelperFinalization.h"

FMatchPlayCurrentAttackBranchIntentSelectionWriterResult
FMatchPlayCurrentAttackBranchIntentSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackBranchIntentSelectionRequest& Request)
{
	FMatchPlayCurrentAttackBranchIntentSelectionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackBranchIntentSelectionLegalityEvaluator
			::Evaluate(BeforeState, Request);
	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackHelperFinalization
		::ApplyFinalSelectedAction(
			WorkingState,
			Result.LegalityResult.ResolvedIntent);
	Result.ReadyValidationResult =
		FMatchPlayCurrentAttackReadyForResolutionValidator::Validate(
			WorkingState);
	if (!Result.ReadyValidationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
				::ReadyValidationFailed;
		Result.ErrorMessage =
			Result.ReadyValidationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
			::None;
	return Result;
}
