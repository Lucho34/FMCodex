#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
	final
{
public:
	static
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
