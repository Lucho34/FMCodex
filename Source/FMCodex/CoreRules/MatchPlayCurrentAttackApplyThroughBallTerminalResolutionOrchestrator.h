#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackApplyThroughBallTerminalResolutionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
