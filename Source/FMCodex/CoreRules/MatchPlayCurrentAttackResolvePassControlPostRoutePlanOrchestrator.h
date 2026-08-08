#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolvePassControlPostRoutePlanTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest&
			Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
