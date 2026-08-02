#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrationTypes.h"

class FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrator final
{
public:
	static FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
	Resolve(
		const FMatchPlayState& BeforeState,
		const
			FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest&
				Request,
		IMatchPlayInitialRouteRollProvider* RollProvider);
};
