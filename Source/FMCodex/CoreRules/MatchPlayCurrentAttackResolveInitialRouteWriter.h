#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveInitialRouteLegalityQuery.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteWriter final
{
public:
	static FMatchPlayCurrentAttackResolveInitialRouteWriterResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request,
		IMatchPlayInitialRouteRollProvider* RollProvider);
};
