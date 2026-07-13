#pragma once

#include "CoreMinimal.h"

enum class ESkillRuleType : uint8
{
	None,
	LongShot,
	CutInsideShot,
	PassControl,
	Cross,
	ThroughBall
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
