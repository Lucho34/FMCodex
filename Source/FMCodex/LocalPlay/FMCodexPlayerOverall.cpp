#include "FMCodexPlayerOverall.h"

int32 FFMCodexPlayerOverall::RarityValue(
	const EFMCodexOverallRarityTier Tier)
{
	switch (Tier)
	{
	case EFMCodexOverallRarityTier::Common: return 1;
	case EFMCodexOverallRarityTier::National: return 2;
	case EFMCodexOverallRarityTier::Continental: return 3;
	case EFMCodexOverallRarityTier::WorldClass: return 4;
	case EFMCodexOverallRarityTier::Legendary: return 5;
	default: return 0;
	}
}

bool FFMCodexPlayerOverall::TryMapRarity(
	const ECardRarity Rarity,
	EFMCodexOverallRarityTier& OutTier)
{
	switch (Rarity)
	{
	case ECardRarity::Common:
		OutTier = EFMCodexOverallRarityTier::Common;
		return true;
	case ECardRarity::National:
		OutTier = EFMCodexOverallRarityTier::National;
		return true;
	case ECardRarity::Continental:
		OutTier = EFMCodexOverallRarityTier::Continental;
		return true;
	case ECardRarity::WorldClass:
		OutTier = EFMCodexOverallRarityTier::WorldClass;
		return true;
	case ECardRarity::Regional:
	default:
		// Regional is not part of the user-approved Overall v1 mapping. Fail
		// closed rather than silently assigning a plausible numeric value.
		return false;
	}
}

FFMCodexPlayerOverallResult FFMCodexPlayerOverall::CalculateOutfield(
	const FPlayerAttributes& Attributes,
	const ECardRarity Rarity)
{
	const int32 Sum = Attributes.Shooting + Attributes.Passing + Attributes.Control
		+ Attributes.Speed + Attributes.Strength + Attributes.Defense;
	return { true, FMath::Clamp(59 + Sum, 65, 95) };
}

FFMCodexPlayerOverallResult FFMCodexPlayerOverall::CalculateGoalkeeper(
	const FGoalkeeperAttributes& Attributes,
	const ECardRarity Rarity)
{
	EFMCodexOverallRarityTier Tier;
	if (!TryMapRarity(Rarity, Tier))
	{
		return {};
	}
	const int32 AllSixSum = Attributes.Handling
		+ Attributes.Positioning
		+ Attributes.Reflex
		+ Attributes.Aerial
		+ Attributes.Anticipation
		+ Attributes.OneOnOne;
	return { true, AllSixSum * 3 + RarityValue(Tier) };
}

FFMCodexPlayerOverallResult FFMCodexPlayerOverall::Calculate(
	const FPlayerCardData& Card)
{
	return Card.bIsGoalkeeper
		? CalculateGoalkeeper(Card.GoalkeeperAttributes, Card.Rarity)
		: CalculateOutfield(Card.Attributes, Card.Rarity);
}
