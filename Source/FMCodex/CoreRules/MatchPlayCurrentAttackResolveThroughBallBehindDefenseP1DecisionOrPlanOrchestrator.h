#pragma once
#include "CoreMinimal.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanTypes.h"
class FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest& Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
