#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextQuery final
{
public:
	static
		FMatchPlayCurrentAttackBeginResolutionSessionGlobalContextResult
	Query(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest&
			Request);
};
