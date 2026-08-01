#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolutionRollTypes.h"

enum class EMatchPlayInitialRouteRollProviderErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure
};

struct FMCODEX_API FMatchPlayInitialRouteRollProviderResult
{
	bool bSuccess = false;
	int32 RawD6 = 0;
	EMatchPlayInitialRouteRollProviderErrorCode ErrorCode =
		EMatchPlayInitialRouteRollProviderErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API IMatchPlayInitialRouteRollProvider
{
public:
	virtual ~IMatchPlayInitialRouteRollProvider() = default;

	virtual FMatchPlayInitialRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose Purpose) = 0;
};
