#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallFeetFormulaTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
