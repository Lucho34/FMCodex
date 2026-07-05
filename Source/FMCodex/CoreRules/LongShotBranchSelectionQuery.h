#pragma once

#include "CoreMinimal.h"

#include "LongShotDeadCornerDecisionQuery.h"
#include "LongShotDirectShotPlanQuery.h"

enum class ELongShotShotBranch : uint8
{
	None,
	DirectShot,
	DeadCorner
};

enum class ELongShotBranchSelectionOutcome : uint8
{
	None,
	DirectShotImmediateMiss,
	DirectShotFormulaPlanRequired,
	DeadCornerGoal,
	DeadCornerMiss
};

enum class ELongShotBranchSelectionQueryErrorCode : uint8
{
	None,
	InvalidBranch,
	DirectShotQueryFailed,
	DeadCornerQueryFailed
};

struct FMCODEX_API FLongShotBranchSelectionQueryInput
{
	ELongShotShotBranch Branch = ELongShotShotBranch::None;
	FLongShotDirectShotPlanQueryInput DirectShotInput;
	FLongShotDeadCornerDecisionQueryInput DeadCornerInput;
};

struct FMCODEX_API FLongShotBranchSelectionQueryResult
{
	bool bSuccess = false;
	ELongShotShotBranch SelectedBranch = ELongShotShotBranch::None;
	ELongShotBranchSelectionOutcome Outcome =
		ELongShotBranchSelectionOutcome::None;
	ELongShotBranchSelectionQueryErrorCode ErrorCode =
		ELongShotBranchSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FLongShotBranchSelectionQueryInput Input;
	FLongShotDirectShotPlanQueryResult DirectShotResult;
	FLongShotDeadCornerDecisionQueryResult DeadCornerResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
};

class FMCODEX_API FLongShotBranchSelectionQuery final
{
public:
	static FLongShotBranchSelectionQueryResult Select(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FLongShotBranchSelectionQueryInput& Input);
};
