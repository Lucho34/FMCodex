#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveCrossPostRoutePlanTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest&
			Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
