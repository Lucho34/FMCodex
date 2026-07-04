#pragma once

#include "CoreMinimal.h"

#include "SkillRuleSnapshot.h"

enum class ESkillRuleSnapshotValidationErrorCode : uint8
{
	None,
	InvalidSkillId,
	DuplicateSkillId,
	InvalidSkillType,
	UnsupportedSkillType,
	TriggerActionPointBelowMinimum,
	TriggerActionPointAboveMaximum,
	InvalidTriggerActionPointRange
};

struct FMCODEX_API FSkillRuleSnapshotValidationResult
{
	bool bSuccess = false;
	bool bIsValid = false;
	ESkillRuleSnapshotValidationErrorCode ErrorCode =
		ESkillRuleSnapshotValidationErrorCode::None;
	FString ErrorMessage;
	FName InvalidSkillId = NAME_None;
	FName InvalidField = NAME_None;
};

class FMCODEX_API FSkillRuleSnapshotValidator final
{
public:
	static constexpr int32 MinTriggerActionPoint = 2;
	static constexpr int32 MaxTriggerActionPoint = 8;

	static FSkillRuleSnapshotValidationResult Validate(
		const FSkillRuleSnapshotSet& SnapshotSet);
};
