#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionLegality.h"

class FMCODEX_API
	FMatchPlayCurrentAttackBeginResolutionSessionWriter final
{
public:
	static FMatchPlayCurrentAttackBeginResolutionSessionWriterResult
	Begin(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackBeginResolutionSessionRequest&
			Request);
};
