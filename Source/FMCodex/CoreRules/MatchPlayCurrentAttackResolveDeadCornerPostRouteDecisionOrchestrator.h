#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest&
			Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
