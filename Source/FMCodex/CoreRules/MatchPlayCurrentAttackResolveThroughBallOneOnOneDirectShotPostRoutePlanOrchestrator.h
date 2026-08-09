#pragma once

#include "CoreMinimal.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanTypes.h"

struct FSkillRuleSnapshotSet;
class IMatchPlayPostRouteRollProvider;

class FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
