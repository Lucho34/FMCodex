#include "PassControlPassAdvancePlanQuery.h"

FPassControlPassAdvancePlanQueryResult
FPassControlPassAdvancePlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlPassAdvancePlanQueryInput& Input)
{
	// Stage 8.20: this tactic is unavailable; no substitute formula is defined.
	FPassControlPassAdvancePlanQueryResult Result;
	Result.Input = Input;
	Result.ErrorCode = EPassControlPassAdvancePlanQueryErrorCode::TacticUnavailable;
	Result.ErrorMessage = TEXT("PassControl is temporarily unavailable.");
	return Result;
}
