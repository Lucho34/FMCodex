#include "MatchPlayExecutionSummary.h"

FMatchPlayExecutionSummary
FMatchPlayExecutionSummaryBuilder::BuildSummary(
	const FMatchPlayState& BeforeMatchPlayState,
	const FMatchPlayAttackExecutionResult& ExecutionResult)
{
	FMatchPlayExecutionSummary Summary;
	Summary.bExecutionSuccess = ExecutionResult.bSuccess;
	Summary.bAttackResolved = ExecutionResult.bAttackResolved;
	Summary.bGoalScored = ExecutionResult.bGoalScored;
	Summary.bMatchEnded = ExecutionResult.bMatchEnded;
	Summary.MatchResultType = ExecutionResult.MatchResultType;
	Summary.RequestPlayer = ExecutionResult.AttackRequest.PlayerSide;
	Summary.CardId = ExecutionResult.AttackRequest.CardId;
	Summary.BeforeStatus =
		FMatchPlayStatusQuery::QueryStatus(BeforeMatchPlayState);
	Summary.bValidationFailed =
		ExecutionResult.ErrorCode
			== EMatchPlayAttackExecutionErrorCode
				::RequestValidationFailed;
	Summary.bAttackFlowFailed =
		ExecutionResult.ErrorCode
			== EMatchPlayAttackExecutionErrorCode::AttackFlowFailed;
	Summary.ErrorCode = ExecutionResult.ErrorCode;
	Summary.ErrorMessage = ExecutionResult.ErrorMessage;

	if (ExecutionResult.bSuccess)
	{
		Summary.AfterStatus =
			FMatchPlayStatusQuery::QueryStatus(
				ExecutionResult.UpdatedMatchPlayState);
		Summary.bHasAfterStatus = true;
	}

	return Summary;
}
