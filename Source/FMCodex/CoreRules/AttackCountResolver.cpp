#include "AttackCountResolver.h"

namespace AttackCountResolver
{
	constexpr int32 MinimumD6Roll = 1;
	constexpr int32 MaximumD6Roll = 6;

	bool IsValidD6Roll(const int32 Roll)
	{
		return Roll >= MinimumD6Roll && Roll <= MaximumD6Roll;
	}

	int32 GetRandomBonusAttackCount(const int32 D6Roll)
	{
		if (D6Roll <= 2)
		{
			return 1;
		}

		if (D6Roll <= 4)
		{
			return 2;
		}

		return 3;
	}

	void AddError(
		FAttackCountResolveResult& Result,
		const EAttackCountResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCodes.Add(ErrorCode);
		Result.ErrorMessages.Add(ErrorMessage);
	}
}

FAttackCountResolveResult FAttackCountResolver::ResolveAttackCounts(
	const int32 PlayerAInitialDeckRarityScore,
	const int32 PlayerBInitialDeckRarityScore,
	const int32 PlayerAD6Roll,
	const int32 PlayerBD6Roll)
{
	FAttackCountResolveResult Result;

	if (!AttackCountResolver::IsValidD6Roll(PlayerAD6Roll))
	{
		AttackCountResolver::AddError(
			Result,
			EAttackCountResolveErrorCode::PlayerAD6RollOutOfRange,
			FString::Printf(
				TEXT("PlayerA D6 roll must be between 1 and 6; received %d."),
				PlayerAD6Roll));
	}

	if (!AttackCountResolver::IsValidD6Roll(PlayerBD6Roll))
	{
		AttackCountResolver::AddError(
			Result,
			EAttackCountResolveErrorCode::PlayerBD6RollOutOfRange,
			FString::Printf(
				TEXT("PlayerB D6 roll must be between 1 and 6; received %d."),
				PlayerBD6Roll));
	}

	if (!Result.ErrorCodes.IsEmpty())
	{
		return Result;
	}

	if (PlayerAInitialDeckRarityScore > PlayerBInitialDeckRarityScore)
	{
		Result.PlayerARarityBonusAttackCount = 1;
	}
	else if (PlayerBInitialDeckRarityScore > PlayerAInitialDeckRarityScore)
	{
		Result.PlayerBRarityBonusAttackCount = 1;
	}

	Result.PlayerARandomBonusAttackCount =
		AttackCountResolver::GetRandomBonusAttackCount(PlayerAD6Roll);
	Result.PlayerBRandomBonusAttackCount =
		AttackCountResolver::GetRandomBonusAttackCount(PlayerBD6Roll);

	Result.PlayerAAttackCount =
		Result.PlayerABaseAttackCount
		+ Result.PlayerARarityBonusAttackCount
		+ Result.PlayerARandomBonusAttackCount;
	Result.PlayerBAttackCount =
		Result.PlayerBBaseAttackCount
		+ Result.PlayerBRarityBonusAttackCount
		+ Result.PlayerBRandomBonusAttackCount;

	Result.bSuccess = true;
	return Result;
}
