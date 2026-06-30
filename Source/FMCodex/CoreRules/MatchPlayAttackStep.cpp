#include "MatchPlayAttackStep.h"

FMatchPlayAttackStepResult FMatchPlayAttackStep::ExecuteAttackStep(
	const FMatchPlayState& BeforeMatchPlayState,
	const FMatchPlayAttackRequest& AttackRequest)
{
	FMatchPlayAttackStepResult Result;
	Result.ExecutionResult =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			BeforeMatchPlayState,
			AttackRequest);
	Result.ExecutionSummary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			BeforeMatchPlayState,
			Result.ExecutionResult);

	if (!Result.ExecutionResult.bSuccess)
	{
		Result.ErrorCode = EMatchPlayAttackStepErrorCode::ExecutionFailed;
		Result.ErrorMessage = Result.ExecutionResult.ErrorMessage;
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}

