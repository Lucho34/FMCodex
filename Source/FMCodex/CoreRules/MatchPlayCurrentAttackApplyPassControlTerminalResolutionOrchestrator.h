#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackApplyPassControlTerminalResolutionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator final
{
public:
	static FMatchPlayCurrentAttackApplyPassControlTerminalResolutionResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
