#include "MatchPlayCurrentAttackBeginResolutionSessionLegality.h"

FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult
FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator::Evaluate(
	const FMatchPlayState& State,
	const FMatchPlayCurrentAttackBeginResolutionSessionRequest& Request)
{
	FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery
			::Query(State, Request);
	if (!Result.GlobalContextResult.bSuccess)
	{
		Result.ErrorCode = Result.GlobalContextResult.ErrorCode;
		Result.ErrorMessage =
			Result.GlobalContextResult.ErrorMessage;
		return Result;
	}

	Result.bIsLegal = true;
	Result.bSessionAlreadyExists =
		Result.GlobalContextResult.bSessionAlreadyExists;
	Result.Session = Result.GlobalContextResult.Session;
	return Result;
}
