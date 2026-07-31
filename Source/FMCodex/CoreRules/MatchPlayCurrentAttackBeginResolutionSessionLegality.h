#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery.h"

class FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackBeginResolutionSessionLegalityResult
	Evaluate(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest&
			Request);
};
