#include "PassControlRunAdvancePlanQuery.h"

FPassControlRunAdvancePlanQueryResult
FPassControlRunAdvancePlanQuery::BuildPlan(
	const FPlayerCardRuleSnapshotSet& PlayerCardSnapshots,
	const FSkillRuleSnapshotSet& SkillRules,
	const FPassControlRunAdvancePlanQueryInput& Input)
{
	// Stage 8.20: this tactic is unavailable; no substitute formula is defined.
	FPassControlRunAdvancePlanQueryResult Result;
	Result.Input = Input;
	Result.ErrorCode = EPassControlRunAdvancePlanQueryErrorCode::TacticUnavailable;
	Result.ErrorMessage = TEXT("PassControl is temporarily unavailable.");
	return Result;
}
