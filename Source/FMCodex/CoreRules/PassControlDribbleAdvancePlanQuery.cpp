#include "PassControlDribbleAdvancePlanQuery.h"

FPassControlDribbleAdvancePlanQueryResult
FPassControlDribbleAdvancePlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlDribbleAdvancePlanQueryInput& Input)
{
	// Stage 8.20: this tactic is unavailable; no substitute formula is defined.
	FPassControlDribbleAdvancePlanQueryResult Result;
	Result.Input = Input;
	Result.ErrorCode = EPassControlDribbleAdvancePlanQueryErrorCode::TacticUnavailable;
	Result.ErrorMessage = TEXT("PassControl is temporarily unavailable.");
	return Result;
}
