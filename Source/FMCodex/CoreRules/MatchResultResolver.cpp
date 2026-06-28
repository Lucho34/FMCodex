#include "MatchResultResolver.h"

namespace MatchResultResolver
{
	void SetError(
		FMatchResultResolveResult& Result,
		const EMatchResultResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchResultResolveResult FMatchResultResolver::ResolveMatchResult(
	const FMatchRuntimeState& RuntimeState)
{
	FMatchResultResolveResult Result;

	// The minimal runtime state currently represents PlayerA as home and PlayerB as away.
	Result.HomeScore = RuntimeState.PlayerAState.Score;
	Result.AwayScore = RuntimeState.PlayerBState.Score;

	const FMatchEndResolveResult MatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(RuntimeState);
	Result.bIsMatchEnded = MatchEndResult.bIsMatchEnded;

	if (!MatchEndResult.bSuccess)
	{
		MatchResultResolver::SetError(
			Result,
			EMatchResultResolveErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match end validation failed: %s"),
				*MatchEndResult.ErrorMessage));
		return Result;
	}

	if (Result.HomeScore < 0 || Result.AwayScore < 0)
	{
		MatchResultResolver::SetError(
			Result,
			EMatchResultResolveErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match scores cannot be negative: home=%d, away=%d."),
				Result.HomeScore,
				Result.AwayScore));
		return Result;
	}

	if (!Result.bIsMatchEnded)
	{
		MatchResultResolver::SetError(
			Result,
			EMatchResultResolveErrorCode::MatchNotEnded,
			TEXT("Cannot resolve a match result while attack opportunities remain."));
		return Result;
	}

	if (Result.HomeScore > Result.AwayScore)
	{
		Result.ResultType = EMatchResultType::HomeWin;
	}
	else if (Result.AwayScore > Result.HomeScore)
	{
		Result.ResultType = EMatchResultType::AwayWin;
	}
	else
	{
		Result.ResultType = EMatchResultType::Draw;
	}

	Result.bSuccess = true;
	return Result;
}
