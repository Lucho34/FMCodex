#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/PlayerCardTypes.h"

/**
 * Overall-v1 rarity contract. This is deliberately separate from the
 * gameplay rarity enum: the repository still contains Regional and does not
 * yet contain a gameplay Legendary tier.
 */
enum class EFMCodexOverallRarityTier : uint8
{
	Common,
	National,
	Continental,
	WorldClass,
	Legendary
};

struct FMCODEX_API FFMCodexPlayerOverallResult
{
	bool bSuccess = false;
	int32 Value = 0;
};

/** Presentation-only six-base outfield OVR; goalkeeper keeps the existing rarity formula. */
class FMCODEX_API FFMCodexPlayerOverall final
{
public:
	static int32 RarityValue(EFMCodexOverallRarityTier Tier);
	static bool TryMapRarity(
		ECardRarity Rarity,
		EFMCodexOverallRarityTier& OutTier);

	static FFMCodexPlayerOverallResult CalculateOutfield(
		const FPlayerAttributes& Attributes,
		ECardRarity Rarity);
	static FFMCodexPlayerOverallResult CalculateGoalkeeper(
		const FGoalkeeperAttributes& Attributes,
		ECardRarity Rarity);
	static FFMCodexPlayerOverallResult Calculate(
		const FPlayerCardData& Card);
};
