#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionTypes.h"

// Derives the P1 continuation from State before acquiring or replaying P2.
class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
	final
{
public:
	static
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
