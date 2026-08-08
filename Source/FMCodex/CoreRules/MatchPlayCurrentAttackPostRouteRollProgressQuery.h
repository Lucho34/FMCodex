#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

enum class EMatchPlayCurrentAttackPostRouteRollProgressErrorCode : uint8
{
	None,
	RouteNotResolved,
	MissingActualBranch,
	RecordsWithoutPhase,
	UnsupportedPhaseForBranch,
	MissingRequiredPhasePrefix,
	TooManyRolls,
	InvalidPurpose,
	InvalidD6,
	InvalidRollOrder,
	DuplicatePurpose,
	ConditionalDefenseNotAllowed,
	InvalidLaterPhasePrerequisite
};

struct FMCODEX_API FMatchPlayCurrentAttackPostRouteRollProgressResult
{
	bool bIsCanonical = false;
	bool bContractComplete = false;
	bool bHasNextPurpose = false;
	EMatchPlayCurrentAttackPostRouteRollPurpose NextPurpose =
		EMatchPlayCurrentAttackPostRouteRollPurpose::None;
	EMatchPlayCurrentAttackPostRouteRollProgressErrorCode ErrorCode =
		EMatchPlayCurrentAttackPostRouteRollProgressErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackPostRouteRollProgressQuery final
{
public:
	static FMatchPlayCurrentAttackPostRouteRollProgressResult Evaluate(
		const FMatchPlayCurrentAttackResolutionSession& Session);
};
