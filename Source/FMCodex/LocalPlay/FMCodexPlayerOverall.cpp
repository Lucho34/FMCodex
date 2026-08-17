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
	EFMCodexOverallRarityTier Tier;
	if (!TryMapRarity(Rarity, Tier))
	{
		return {};
	}

	TArray<int32, TInlineAllocator<10>> Values = {
		Attributes.Shooting,
		Attributes.Dribbling,
		Attributes.Passing,
		Attributes.OffBall,
		Attributes.Marking,
		Attributes.Tackling,
		Attributes.Speed,
		Attributes.Strength,
		Attributes.Stamina,
		Attributes.LongShot
	};
	Values.Sort(TGreater<int32>());
	int32 TopSixSum = 0;
	for (int32 Index = 0; Index < 6; ++Index)
	{
		TopSixSum += Values[Index];
	}
	return { true, TopSixSum * 3 + RarityValue(Tier) };
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
