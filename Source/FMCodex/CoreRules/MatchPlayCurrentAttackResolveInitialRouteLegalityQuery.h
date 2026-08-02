#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteLegalityQuery final
{
public:
	static FMatchPlayCurrentAttackResolveInitialRouteLegalityResult Query(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request);
};
