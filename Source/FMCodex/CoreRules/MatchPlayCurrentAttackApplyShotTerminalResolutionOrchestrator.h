#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackApplyShotTerminalResolutionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackApplyShotTerminalResolutionResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
