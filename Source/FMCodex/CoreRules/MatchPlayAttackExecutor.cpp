#include "MatchPlayAttackExecutor.h"

FMatchPlayAttackExecutionResult
FMatchPlayAttackExecutor::ExecuteAttackRequest(
	const FMatchPlayState& CurrentMatchPlayState,
	const FMatchPlayAttackRequest& AttackRequest)
{
	FMatchPlayAttackExecutionResult Result;
	Result.AttackRequest = AttackRequest;

	Result.ValidationResult =
		FMatchPlayAttackRequestValidator::ValidateRequest(
			CurrentMatchPlayState,
			AttackRequest);
	if (!Result.ValidationResult.bValid)
	{
		Result.ErrorCode =
			EMatchPlayAttackExecutionErrorCode::RequestValidationFailed;
		Result.UnderlyingRequestValidationErrorCode =
			Result.ValidationResult.ErrorCode;
		Result.ErrorMessage = Result.ValidationResult.ErrorMessage;
		return Result;
	}

	Result.AttackFlowResult =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			CurrentMatchPlayState,
			AttackRequest.CardId,
			AttackRequest.FormulaInput);
	if (!Result.AttackFlowResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayAttackExecutionErrorCode::AttackFlowFailed;
		Result.UnderlyingMatchPlayAttackFlowErrorCode =
			Result.AttackFlowResult.ErrorCode;
		Result.ErrorMessage = Result.AttackFlowResult.ErrorMessage;
		return Result;
	}

	Result.UpdatedMatchPlayState =
		Result.AttackFlowResult.UpdatedMatchPlayState;
	Result.bGoalScored = Result.AttackFlowResult.bGoalScored;
	Result.bAttackResolved = Result.AttackFlowResult.bAttackResolved;
	Result.bMatchEnded = Result.AttackFlowResult.bMatchEnded;
	Result.MatchResultType = Result.AttackFlowResult.MatchResultType;
	Result.bSuccess = true;
	return Result;
}
