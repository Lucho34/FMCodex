#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionTypes.h"

// Regenerates exactly one supported source before Candidate State owns ChipShot.
class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator
	final
{
public:
	static
		FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
