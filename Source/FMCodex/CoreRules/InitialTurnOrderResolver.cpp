#include "InitialTurnOrderResolver.h"

namespace InitialTurnOrderResolver
{
	constexpr int32 MinimumD6Roll = 1;
	constexpr int32 MaximumD6Roll = 6;

	bool IsValidD6Roll(const int32 Roll)
	{
		return Roll >= MinimumD6Roll && Roll <= MaximumD6Roll;
	}

	void AddError(
		FInitialTurnOrderResult& Result,
		const EInitialTurnOrderErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCodes.Add(ErrorCode);
		Result.ErrorMessages.Add(ErrorMessage);
	}

	void SetSuccess(
		FInitialTurnOrderResult& Result,
		const EInitialTurnOrderPlayer FirstPlayer,
		const EInitialTurnOrderReason Reason)
	{
		Result.bSuccess = true;
		Result.FirstPlayer = FirstPlayer;
		Result.Reason = Reason;
	}
}

FInitialTurnOrderResult FInitialTurnOrderResolver::ResolveInitialTurnOrder(
	const int32 PlayerAAttackCount,
	const int32 PlayerBAttackCount,
	const int32 PlayerAInitialDeckRarityScore,
	const int32 PlayerBInitialDeckRarityScore,
	const int32 PlayerATieBreakerRoll,
	const int32 PlayerBTieBreakerRoll)
{
	FInitialTurnOrderResult Result;

	if (PlayerAAttackCount < 0)
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerAAttackCount,
			FString::Printf(
				TEXT("PlayerA attack count cannot be negative; received %d."),
				PlayerAAttackCount));
	}

	if (PlayerBAttackCount < 0)
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerBAttackCount,
			FString::Printf(
				TEXT("PlayerB attack count cannot be negative; received %d."),
				PlayerBAttackCount));
	}

	if (PlayerAInitialDeckRarityScore < 0)
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerAInitialDeckRarityScore,
			FString::Printf(
				TEXT("PlayerA initial deck rarity score cannot be negative; received %d."),
				PlayerAInitialDeckRarityScore));
	}

	if (PlayerBInitialDeckRarityScore < 0)
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerBInitialDeckRarityScore,
			FString::Printf(
				TEXT("PlayerB initial deck rarity score cannot be negative; received %d."),
				PlayerBInitialDeckRarityScore));
	}

	if (!Result.ErrorCodes.IsEmpty())
	{
		return Result;
	}

	if (PlayerAAttackCount != PlayerBAttackCount)
	{
		InitialTurnOrderResolver::SetSuccess(
			Result,
			PlayerAAttackCount > PlayerBAttackCount
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderReason::HigherAttackCount);
		return Result;
	}

	if (PlayerAInitialDeckRarityScore != PlayerBInitialDeckRarityScore)
	{
		InitialTurnOrderResolver::SetSuccess(
			Result,
			PlayerAInitialDeckRarityScore < PlayerBInitialDeckRarityScore
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderReason::LowerInitialDeckRarityScore);
		return Result;
	}

	if (!InitialTurnOrderResolver::IsValidD6Roll(PlayerATieBreakerRoll))
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerATieBreakerRoll,
			FString::Printf(
				TEXT("PlayerA tie breaker roll must be between 1 and 6; received %d."),
				PlayerATieBreakerRoll));
	}

	if (!InitialTurnOrderResolver::IsValidD6Roll(PlayerBTieBreakerRoll))
	{
		InitialTurnOrderResolver::AddError(
			Result,
			EInitialTurnOrderErrorCode::InvalidPlayerBTieBreakerRoll,
			FString::Printf(
				TEXT("PlayerB tie breaker roll must be between 1 and 6; received %d."),
				PlayerBTieBreakerRoll));
	}

	if (!Result.ErrorCodes.IsEmpty())
	{
		return Result;
	}

	if (PlayerATieBreakerRoll != PlayerBTieBreakerRoll)
	{
		InitialTurnOrderResolver::SetSuccess(
			Result,
			PlayerATieBreakerRoll < PlayerBTieBreakerRoll
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderReason::LowerTieBreakerRoll);
		return Result;
	}

	Result.bRequiresTieBreakerReroll = true;
	Result.Reason = EInitialTurnOrderReason::TieBreakerStillTied;
	InitialTurnOrderResolver::AddError(
		Result,
		EInitialTurnOrderErrorCode::TieBreakerStillTied,
		FString::Printf(
			TEXT("Tie breaker rolls are still tied at %d; external reroll is required."),
			PlayerATieBreakerRoll));
	return Result;
}
