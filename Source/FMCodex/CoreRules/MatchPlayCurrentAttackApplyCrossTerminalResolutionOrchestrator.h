#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackApplyCrossTerminalResolutionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackApplyCrossTerminalResolutionResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
