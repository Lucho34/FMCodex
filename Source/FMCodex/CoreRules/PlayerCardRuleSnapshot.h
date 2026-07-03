#pragma once

#include "CoreMinimal.h"

#include "PlayerCardTypes.h"

struct FMCODEX_API FPlayerCardRuleSnapshot
{
	FName CardId = NAME_None;
	TArray<EPlayerPositionType> PositionTypes;
	FPlayerAttributes Attributes;
	bool bIsGoalkeeper = false;
	bool bHasGoalkeeperAttributes = false;
	FGoalkeeperAttributes GoalkeeperAttributes;
	ECardRarity Rarity = ECardRarity::Common;
	TArray<FName> SkillIds;
};

struct FMCODEX_API FPlayerCardRuleSnapshotSet
{
	TArray<FPlayerCardRuleSnapshot> Cards;
};
