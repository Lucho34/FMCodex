#pragma once

#include "CoreMinimal.h"

#include "SkillRuleSnapshot.generated.h"

UENUM(BlueprintType)
enum class ESkillRuleType : uint8
{
	None UMETA(DisplayName = "None"),
	LongShot UMETA(DisplayName = "Long Shot"),
	CutInsideShot UMETA(DisplayName = "Cut Inside Shot"),
	PassControl UMETA(DisplayName = "Pass Control"),
	Cross UMETA(DisplayName = "Cross"),
	ThroughBall UMETA(DisplayName = "Through Ball")
};

struct FMCODEX_API FSkillRuleSnapshot
{
	FName SkillId = NAME_None;
	ESkillRuleType SkillType = ESkillRuleType::None;
	int32 MinTriggerActionPoint = 0;
	int32 MaxTriggerActionPoint = 0;
};

struct FMCODEX_API FSkillRuleSnapshotSet
{
	TArray<FSkillRuleSnapshot> SkillRules;
};
