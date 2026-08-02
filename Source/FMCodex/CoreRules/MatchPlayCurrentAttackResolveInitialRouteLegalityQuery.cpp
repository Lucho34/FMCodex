#include "MatchPlayCurrentAttackResolveInitialRouteLegalityQuery.h"

FMatchPlayCurrentAttackResolveInitialRouteLegalityResult
FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery::Query(
	const FMatchPlayState& State,
	const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request)
{
	FMatchPlayCurrentAttackResolveInitialRouteLegalityResult Result;
	Result.Request = Request;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
			State,
			Request);
	if (!Result.GlobalContextResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackResolveInitialRouteLegalityErrorCode
				::GlobalContextFailed;
		Result.ErrorMessage = Result.GlobalContextResult.ErrorMessage;
		return Result;
	}

	Result.bIsLegal = true;
	return Result;
}
