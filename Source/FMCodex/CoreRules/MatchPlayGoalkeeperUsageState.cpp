#include "MatchPlayGoalkeeperUsageState.h"

namespace MatchPlayGoalkeeperUsageStateImplementation
{
	bool IsPlayer(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}
}

FMatchPlayGoalkeeperUsageQueryResult
FMatchPlayGoalkeeperUsageStateResolver::Query(
	const FMatchPlayGoalkeeperUsageState& BeforeState,
	const EInitialTurnOrderPlayer PlayerSide)
{
	FMatchPlayGoalkeeperUsageQueryResult Result;
	Result.PlayerSide = PlayerSide;

	if (!MatchPlayGoalkeeperUsageStateImplementation::IsPlayer(PlayerSide))
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperUsageErrorCode::InvalidPlayerSide;
		Result.ErrorMessage =
			TEXT("PlayerSide must be PlayerA or PlayerB.");
		return Result;
	}

	Result.bGoalkeeperCardUsed =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? BeforeState.bPlayerAGoalkeeperCardUsed
			: BeforeState.bPlayerBGoalkeeperCardUsed;
	Result.bSucceeded = true;
	return Result;
}

FMatchPlayGoalkeeperUsageUpdateResult
FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
	const FMatchPlayGoalkeeperUsageState& BeforeState,
	const EInitialTurnOrderPlayer PlayerSide)
{
	FMatchPlayGoalkeeperUsageUpdateResult Result;
	Result.PlayerSide = PlayerSide;
	Result.UpdatedUsageState = BeforeState;

	if (!MatchPlayGoalkeeperUsageStateImplementation::IsPlayer(PlayerSide))
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperUsageErrorCode::InvalidPlayerSide;
		Result.ErrorMessage =
			TEXT("PlayerSide must be PlayerA or PlayerB.");
		return Result;
	}

	const bool bAlreadyUsed =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? BeforeState.bPlayerAGoalkeeperCardUsed
			: BeforeState.bPlayerBGoalkeeperCardUsed;
	if (bAlreadyUsed)
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperUsageErrorCode::GoalkeeperAlreadyUsed;
		Result.ErrorMessage =
			TEXT("The selected player side has already used its goalkeeper card.");
		return Result;
	}

	if (PlayerSide == EInitialTurnOrderPlayer::PlayerA)
	{
		Result.UpdatedUsageState.bPlayerAGoalkeeperCardUsed = true;
	}
	else
	{
		Result.UpdatedUsageState.bPlayerBGoalkeeperCardUsed = true;
	}
	Result.bSucceeded = true;
	return Result;
}
