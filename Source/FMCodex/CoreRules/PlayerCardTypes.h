#pragma once

#include "CoreMinimal.h"
#include "CoreRuleEnums.h"

#include "PlayerCardTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerAttributes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Shooting = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Dribbling = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Passing = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 OffBall = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Marking = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Tackling = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Speed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Strength = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 Stamina = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Attributes")
	int32 LongShot = 1;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FGoalkeeperAttributes
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 Handling = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 Positioning = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 Reflex = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 Aerial = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 Anticipation = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Goalkeeper Attributes")
	int32 OneOnOne = 1;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayerCardData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FName CardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	int32 HeightCm = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	int32 WeightKg = 0;

	// Stored as YYYY-MM-DD. It is display-only in the current rules.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FString BirthDate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FString Notes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	ECardRarity Rarity = ECardRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	TArray<EPlayerPositionType> PositionTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FPlayerAttributes Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	FGoalkeeperAttributes GoalkeeperAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	TArray<FName> AttackSkillIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Player Card")
	bool bIsGoalkeeper = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FSkillDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	FName SkillId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	FText SkillDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	ESkillType SkillType = ESkillType::None;

	// Inclusive action point range. Zero means that the value is not configured.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	int32 MinTriggerActionPoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	int32 MaxTriggerActionPoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	TArray<FName> RequiredRoles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	FName RequiredRunnerZone = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	TArray<FName> ResolutionSteps;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	FName ScorerRole = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	FName ConsumedPlayersRule = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Skill")
	TArray<FName> FormulaReferences;
};

