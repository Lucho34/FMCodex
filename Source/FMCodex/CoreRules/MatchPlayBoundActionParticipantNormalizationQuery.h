#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionParticipantNormalizationTypes.h"

class FMatchPlayValidatedResolutionPreparationAccess;

class FMCODEX_API FMatchPlayBoundActionParticipantNormalizationQuery final
{
public:
	static FMatchPlayBoundActionParticipantNormalizationResult Query(
		const FMatchPlayState& State,
		const FMatchPlayBoundActionParticipantNormalizationRequest& Request);

private:
	friend class FMatchPlayValidatedResolutionPreparationAccess;

	static FMatchPlayBoundActionParticipantNormalizationResult
	QueryFromSuccessfulBinding(
		const FMatchPlayState& State,
		const FMatchPlayBoundActionParticipantNormalizationRequest& Request,
		const FMatchPlayCurrentAttackResolutionBindingResult& BindingResult);
};
