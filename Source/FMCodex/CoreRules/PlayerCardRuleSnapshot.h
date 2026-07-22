#pragma once

#include "CoreMinimal.h"

#include "PlayerCardTypes.h"

#include "PlayerCardRuleSnapshot.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerCardRuleSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	TArray<EPlayerPositionType> PositionTypes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	FPlayerAttributes Attributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	bool bIsGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	bool bHasGoalkeeperAttributes = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	FGoalkeeperAttributes GoalkeeperAttributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	ECardRarity Rarity = ECardRarity::Common;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	TArray<FName> SkillIds;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerCardRuleSnapshotSet
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card Rule Snapshot")
	TArray<FPlayerCardRuleSnapshot> Cards;
};
