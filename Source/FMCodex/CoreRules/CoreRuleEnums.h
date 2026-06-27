#pragma once

#include "CoreMinimal.h"

#include "CoreRuleEnums.generated.h"

UENUM(BlueprintType)
enum class EPlayerPositionType : uint8
{
	Attack UMETA(DisplayName = "A"),
	Midfield UMETA(DisplayName = "M"),
	Defense UMETA(DisplayName = "D"),
	Goalkeeper UMETA(DisplayName = "GK")
};

UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
	NotStarted UMETA(DisplayName = "Not Started"),
	CalculatingAttackCounts UMETA(DisplayName = "Calculating Attack Counts"),
	DeterminingAttackOrder UMETA(DisplayName = "Determining Attack Order"),
	AwaitingActionPoint UMETA(DisplayName = "Awaiting Action Point"),
	Deployment UMETA(DisplayName = "Deployment"),
	SkillSelection UMETA(DisplayName = "Skill Selection"),
	Resolution UMETA(DisplayName = "Resolution"),
	Finished UMETA(DisplayName = "Finished")
};

UENUM(BlueprintType)
enum class EFormulaType : uint8
{
	None UMETA(DisplayName = "None"),
	Transition UMETA(DisplayName = "Transition"),
	Determination UMETA(DisplayName = "Determination"),
	Finishing UMETA(DisplayName = "Finishing")
};

UENUM(BlueprintType)
enum class ESkillType : uint8
{
	None UMETA(DisplayName = "None"),
	LongShot UMETA(DisplayName = "Long Shot"),
	CutInsideShot UMETA(DisplayName = "Cut Inside Shot"),
	Cross UMETA(DisplayName = "Cross"),
	ThroughBall UMETA(DisplayName = "Through Ball"),
	PossessionPlay UMETA(DisplayName = "Possession Play")
};

UENUM(BlueprintType)
enum class ECardRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Regional UMETA(DisplayName = "Regional"),
	National UMETA(DisplayName = "National"),
	Continental UMETA(DisplayName = "Continental"),
	WorldClass UMETA(DisplayName = "World Class")
};
