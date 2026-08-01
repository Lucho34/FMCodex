#pragma once

#include "CoreMinimal.h"

#include "SkillRuleSnapshot.h"

#include "MatchPlayCurrentAttackActualBranchTypes.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayLongShotActualBranch : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	DirectShot = 1 UMETA(DisplayName = "Direct Shot"),
	DeadCorner = 2 UMETA(DisplayName = "Dead Corner")
};

UENUM(BlueprintType)
enum class EMatchPlayCutInsideShotActualBranch : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	DirectShot = 1 UMETA(DisplayName = "Direct Shot"),
	DeadCorner = 2 UMETA(DisplayName = "Dead Corner")
};

UENUM(BlueprintType)
enum class EMatchPlayCrossActualBranch : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	High = 1 UMETA(DisplayName = "High"),
	Low = 2 UMETA(DisplayName = "Low")
};

UENUM(BlueprintType)
enum class EMatchPlayPassControlActualBranch : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	PassAdvance = 1 UMETA(DisplayName = "Pass Advance"),
	DribbleAdvance = 2 UMETA(DisplayName = "Dribble Advance"),
	RunAdvance = 3 UMETA(DisplayName = "Run Advance")
};

UENUM(BlueprintType)
enum class EMatchPlayThroughBallActualBranch : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Feet = 1 UMETA(DisplayName = "Feet"),
	BehindDefense = 2 UMETA(DisplayName = "Behind Defense"),
	AntiOffside = 3 UMETA(DisplayName = "Anti-Offside")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackActualBranch
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayLongShotActualBranch LongShot =
		EMatchPlayLongShotActualBranch::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCutInsideShotActualBranch CutInsideShot =
		EMatchPlayCutInsideShotActualBranch::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCrossActualBranch Cross =
		EMatchPlayCrossActualBranch::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayPassControlActualBranch PassControl =
		EMatchPlayPassControlActualBranch::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayThroughBallActualBranch ThroughBall =
		EMatchPlayThroughBallActualBranch::None;
};
