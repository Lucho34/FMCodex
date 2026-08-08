#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator
	final
{
public:
	static
		FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
		Resolve(
			const FMatchPlayState& BeforeState,
			const FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest&
				Request,
			const FSkillRuleSnapshotSet* SkillRuleSet,
			IMatchPlayPostRouteRollProvider* RollProvider);
};
