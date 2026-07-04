#pragma once

#include "CoreMinimal.h"

#include "SkillRuleSnapshotValidator.h"

struct FMCODEX_API FSkillRuleSnapshotQueryInput
{
	FName SkillId = NAME_None;
};

enum class ESkillRuleSnapshotQueryErrorCode : uint8
{
	None,
	InvalidSkillId,
	SnapshotSetValidationFailed,
	SkillRuleNotFound
};

struct FMCODEX_API FSkillRuleSnapshotQueryResult
{
	bool bSuccess = false;
	bool bFound = false;
	ESkillRuleSnapshotQueryErrorCode ErrorCode =
		ESkillRuleSnapshotQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidSkillId = NAME_None;
	FName InvalidField = NAME_None;
	FSkillRuleSnapshot Snapshot;
	FSkillRuleSnapshotValidationResult ValidationResult;
};

class FMCODEX_API FSkillRuleSnapshotQuery final
{
public:
	static FSkillRuleSnapshotQueryResult FindBySkillId(
		const FSkillRuleSnapshotSet& SnapshotSet,
		const FSkillRuleSnapshotQueryInput& Input);
};
