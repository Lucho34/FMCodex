#include "AttackOpportunityResolver.h"

namespace AttackOpportunityResolver
{
	void AddError(
		FAttackOpportunityResolveResult& Result,
		const EAttackOpportunityResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCodes.Add(ErrorCode);
		Result.ErrorMessages.Add(ErrorMessage);
	}

	bool ValidatePlayerState(
		FAttackOpportunityResolveResult& Result,
		const FPlayerRuntimeState& PlayerState,
		const EAttackOpportunityResolveErrorCode ErrorCode,
		const TCHAR* PlayerName)
	{
		if (PlayerState.TotalAttackCount >= 0
			&& PlayerState.UsedAttackCount >= 0
			&& PlayerState.UsedAttackCount <= PlayerState.TotalAttackCount)
		{
			return true;
		}

		AddError(
			Result,
			ErrorCode,
			FString::Printf(
				TEXT("%s attack counts are invalid: total=%d, used=%d."),
				PlayerName,
				PlayerState.TotalAttackCount,
				PlayerState.UsedAttackCount));
		return false;
	}

	int32 GetRemainingAttackCount(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount - PlayerState.UsedAttackCount;
	}
}

FAttackOpportunityResolveResult
FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
	const FMatchRuntimeState& RuntimeState)
{
	FAttackOpportunityResolveResult Result;
	Result.UpdatedRuntimeState = RuntimeState;

	if (!RuntimeState.bIsInitialized)
	{
		AttackOpportunityResolver::AddError(
			Result,
			EAttackOpportunityResolveErrorCode::RuntimeStateNotInitialized,
			TEXT("Cannot consume an attack opportunity before the runtime state is initialized."));
		return Result;
	}

	const bool bPlayerAStateValid = AttackOpportunityResolver::ValidatePlayerState(
		Result,
		RuntimeState.PlayerAState,
		EAttackOpportunityResolveErrorCode::InvalidPlayerAAttackCountState,
		TEXT("PlayerA"));
	const bool bPlayerBStateValid = AttackOpportunityResolver::ValidatePlayerState(
		Result,
		RuntimeState.PlayerBState,
		EAttackOpportunityResolveErrorCode::InvalidPlayerBAttackCountState,
		TEXT("PlayerB"));
	if (!bPlayerAStateValid || !bPlayerBStateValid)
	{
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentPlayer = RuntimeState.CurrentAttackingPlayer;
	if (CurrentPlayer != EInitialTurnOrderPlayer::PlayerA
		&& CurrentPlayer != EInitialTurnOrderPlayer::PlayerB)
	{
		AttackOpportunityResolver::AddError(
			Result,
			EAttackOpportunityResolveErrorCode::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	const int32 PlayerARemainingBefore =
		AttackOpportunityResolver::GetRemainingAttackCount(RuntimeState.PlayerAState);
	const int32 PlayerBRemainingBefore =
		AttackOpportunityResolver::GetRemainingAttackCount(RuntimeState.PlayerBState);
	Result.bMatchHasRemainingAttack =
		PlayerARemainingBefore > 0 || PlayerBRemainingBefore > 0;

	if (!Result.bMatchHasRemainingAttack)
	{
		AttackOpportunityResolver::AddError(
			Result,
			EAttackOpportunityResolveErrorCode::NoRemainingAttackForBothPlayers,
			TEXT("Neither player has a remaining attack opportunity."));
		return Result;
	}

	const bool bCurrentPlayerHasRemainingAttack =
		CurrentPlayer == EInitialTurnOrderPlayer::PlayerA
			? PlayerARemainingBefore > 0
			: PlayerBRemainingBefore > 0;
	if (!bCurrentPlayerHasRemainingAttack)
	{
		AttackOpportunityResolver::AddError(
			Result,
			EAttackOpportunityResolveErrorCode::CurrentPlayerHasNoRemainingAttack,
			TEXT("The current attacking player has no remaining attack opportunity."));
		return Result;
	}

	Result.ActingPlayer = CurrentPlayer;
	FPlayerRuntimeState& CurrentPlayerState =
		CurrentPlayer == EInitialTurnOrderPlayer::PlayerA
			? Result.UpdatedRuntimeState.PlayerAState
			: Result.UpdatedRuntimeState.PlayerBState;
	++CurrentPlayerState.UsedAttackCount;

	const int32 PlayerARemainingAfter =
		AttackOpportunityResolver::GetRemainingAttackCount(
			Result.UpdatedRuntimeState.PlayerAState);
	const int32 PlayerBRemainingAfter =
		AttackOpportunityResolver::GetRemainingAttackCount(
			Result.UpdatedRuntimeState.PlayerBState);

	const EInitialTurnOrderPlayer Opponent =
		CurrentPlayer == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	const bool bOpponentHasRemainingAttack =
		Opponent == EInitialTurnOrderPlayer::PlayerA
			? PlayerARemainingAfter > 0
			: PlayerBRemainingAfter > 0;
	const bool bCurrentPlayerStillHasRemainingAttack =
		CurrentPlayer == EInitialTurnOrderPlayer::PlayerA
			? PlayerARemainingAfter > 0
			: PlayerBRemainingAfter > 0;

	if (bOpponentHasRemainingAttack)
	{
		Result.NextAttackingPlayer = Opponent;
	}
	else if (bCurrentPlayerStillHasRemainingAttack)
	{
		Result.NextAttackingPlayer = CurrentPlayer;
	}

	Result.bMatchHasRemainingAttack =
		PlayerARemainingAfter > 0 || PlayerBRemainingAfter > 0;
	Result.UpdatedRuntimeState.CurrentAttackingPlayer =
		Result.NextAttackingPlayer;
	Result.bSuccess = true;
	return Result;
}
