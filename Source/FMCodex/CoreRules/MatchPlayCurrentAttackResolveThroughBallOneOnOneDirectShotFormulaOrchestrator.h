#pragma once

#include "CoreMinimal.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaTypes.h"

struct FSkillRuleSnapshotSet;

class FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaResult Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet);
};
