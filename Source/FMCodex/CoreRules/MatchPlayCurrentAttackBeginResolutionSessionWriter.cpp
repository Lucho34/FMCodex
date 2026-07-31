#include "MatchPlayCurrentAttackBeginResolutionSessionWriter.h"

FMatchPlayCurrentAttackBeginResolutionSessionWriterResult
FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackBeginResolutionSessionRequest& Request)
{
	FMatchPlayCurrentAttackBeginResolutionSessionWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator
			::Evaluate(BeforeState, Request);
	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode = Result.LegalityResult.ErrorCode;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	Result.Session = Result.LegalityResult.Session;
	if (Result.LegalityResult.bSessionAlreadyExists)
	{
		Result.bSuccess = true;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	WorkingState.CurrentAttack.bHasResolutionSession = true;
	WorkingState.CurrentAttack.ResolutionSession = Result.Session;
	const FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		ValidationResult =
			FMatchPlayCurrentAttackResolutionSessionStateValidator
				::Validate(WorkingState.CurrentAttack);
	if (!ValidationResult.bIsCanonical)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackBeginResolutionSessionErrorCode
				::InvalidCanonicalBundle;
		Result.ErrorMessage = ValidationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.bCreatedNewSession = true;
	return Result;
}
