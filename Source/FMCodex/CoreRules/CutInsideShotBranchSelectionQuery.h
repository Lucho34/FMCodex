#pragma once

#include "CoreMinimal.h"

#include "CutInsideShotDeadCornerDecisionQuery.h"
#include "CutInsideShotDirectShotPlanQuery.h"

enum class ECutInsideShotBranch : uint8
{
	None,
	DirectShot,
	DeadCorner
};

enum class ECutInsideShotBranchSelectionOutcome : uint8
{
	None,
	DirectShotImmediateMiss,
	DirectShotFormulaPlanRequired,
	DeadCornerGoal,
	DeadCornerMiss
};

enum class ECutInsideShotBranchSelectionQueryErrorCode : uint8
{
	None,
	InvalidBranch,
	DirectShotQueryFailed,
	DeadCornerQueryFailed
};

struct FMCODEX_API FCutInsideShotBranchSelectionQueryInput
{
	ECutInsideShotBranch Branch = ECutInsideShotBranch::None;
	FCutInsideShotDirectShotPlanQueryInput DirectShotInput;
	FCutInsideShotDeadCornerDecisionQueryInput DeadCornerInput;
};

struct FMCODEX_API FCutInsideShotBranchSelectionQueryResult
{
	bool bSuccess = false;
	ECutInsideShotBranch SelectedBranch = ECutInsideShotBranch::None;
	ECutInsideShotBranchSelectionOutcome Outcome =
		ECutInsideShotBranchSelectionOutcome::None;
	ECutInsideShotBranchSelectionQueryErrorCode ErrorCode =
		ECutInsideShotBranchSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FCutInsideShotBranchSelectionQueryInput Input;
	FCutInsideShotDirectShotPlanQueryResult DirectShotResult;
	FCutInsideShotDeadCornerDecisionQueryResult DeadCornerResult;
	bool bAttackEnded = false;
	bool bIsGoal = false;
};

class FMCODEX_API FCutInsideShotBranchSelectionQuery final
{
public:
	static FCutInsideShotBranchSelectionQueryResult Select(
		const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
		const FSkillRuleSnapshotSet& SkillRules,
		const FCutInsideShotBranchSelectionQueryInput& Input);
};
