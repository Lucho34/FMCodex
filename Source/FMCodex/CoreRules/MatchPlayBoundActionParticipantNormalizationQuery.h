#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionParticipantNormalizationTypes.h"

class FMCODEX_API FMatchPlayBoundActionParticipantNormalizationQuery final
{
public:
	static FMatchPlayBoundActionParticipantNormalizationResult Query(
		const FMatchPlayState& State,
		const FMatchPlayBoundActionParticipantNormalizationRequest& Request);
};
