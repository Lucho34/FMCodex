#include "MatchEndResolver.h"

namespace MatchEndResolver
{
	bool IsAttackCountStateValid(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount >= 0
			&& PlayerState.UsedAttackCount >= 0
			&& PlayerState.UsedAttackCount <= PlayerState.TotalAttackCount;
	}

	void SetError(
		FMatchEndResolveResult& Result,
		const EMatchEndResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchEndResolveResult FMatchEndResolver::ResolveMatchEnd(
	const FMatchRuntimeState& RuntimeState)
{
	FMatchEndResolveResult Result;
	Result.PlayerATotalAttackCount = RuntimeState.PlayerAState.TotalAttackCount;
	Result.PlayerAUsedAttackCount = RuntimeState.PlayerAState.UsedAttackCount;
	Result.PlayerBTotalAttackCount = RuntimeState.PlayerBState.TotalAttackCount;
	Result.PlayerBUsedAttackCount = RuntimeState.PlayerBState.UsedAttackCount;

	if (!RuntimeState.bIsInitialized)
	{
		MatchEndResolver::SetError(
			Result,
			EMatchEndResolveErrorCode::RuntimeStateNotInitialized,
			TEXT("Cannot determine whether the match has ended before the runtime state is initialized."));
		return Result;
	}

	if (!MatchEndResolver::IsAttackCountStateValid(RuntimeState.PlayerAState))
	{
		MatchEndResolver::SetError(
			Result,
			EMatchEndResolveErrorCode::InvalidPlayerAAttackCountState,
			FString::Printf(
				TEXT("PlayerA attack counts are invalid: total=%d, used=%d."),
				RuntimeState.PlayerAState.TotalAttackCount,
				RuntimeState.PlayerAState.UsedAttackCount));
		return Result;
	}

	if (!MatchEndResolver::IsAttackCountStateValid(RuntimeState.PlayerBState))
	{
		MatchEndResolver::SetError(
			Result,
			EMatchEndResolveErrorCode::InvalidPlayerBAttackCountState,
			FString::Printf(
				TEXT("PlayerB attack counts are invalid: total=%d, used=%d."),
				RuntimeState.PlayerBState.TotalAttackCount,
				RuntimeState.PlayerBState.UsedAttackCount));
		return Result;
	}

	Result.PlayerARemainingAttackCount =
		RuntimeState.PlayerAState.TotalAttackCount
		- RuntimeState.PlayerAState.UsedAttackCount;
	Result.PlayerBRemainingAttackCount =
		RuntimeState.PlayerBState.TotalAttackCount
		- RuntimeState.PlayerBState.UsedAttackCount;
	Result.bIsMatchEnded =
		Result.PlayerARemainingAttackCount == 0
		&& Result.PlayerBRemainingAttackCount == 0;
	Result.bSuccess = true;
	return Result;
}
