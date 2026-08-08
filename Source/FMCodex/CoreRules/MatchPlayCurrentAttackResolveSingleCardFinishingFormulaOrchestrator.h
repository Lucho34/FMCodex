#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
