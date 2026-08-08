#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
	final
{
public:
	static
		FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest&
				Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
