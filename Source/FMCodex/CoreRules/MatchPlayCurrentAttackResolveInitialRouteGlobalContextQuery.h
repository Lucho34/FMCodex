#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveInitialRouteTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery final
{
public:
	static
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult
	Query(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request);
};
