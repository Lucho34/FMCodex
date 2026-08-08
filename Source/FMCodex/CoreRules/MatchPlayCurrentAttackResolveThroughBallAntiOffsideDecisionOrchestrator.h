#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest&
			Request,
		const FSkillRuleSnapshotSet* SkillRuleSet,
		IMatchPlayPostRouteRollProvider* RollProvider);
};
