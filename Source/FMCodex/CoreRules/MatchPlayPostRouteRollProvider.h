#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollTypes.h"

enum class EMatchPlayPostRouteRollProviderErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure
};

struct FMCODEX_API FMatchPlayPostRouteRollProviderResult
{
	bool bSuccess = false;
	int32 RawD6 = 0;
	EMatchPlayPostRouteRollProviderErrorCode ErrorCode =
		EMatchPlayPostRouteRollProviderErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API IMatchPlayPostRouteRollProvider
{
public:
	virtual ~IMatchPlayPostRouteRollProvider() = default;

	virtual FMatchPlayPostRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) = 0;
};

enum class EMatchPlayPostRouteRollProviderResultValidationErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure,
	MalformedProviderResult
};

struct FMCODEX_API FMatchPlayPostRouteRollProviderResultValidationResult
{
	bool bIsCanonical = false;
	EMatchPlayPostRouteRollProviderResultValidationErrorCode ErrorCode =
		EMatchPlayPostRouteRollProviderResultValidationErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayPostRouteRollProviderResultValidator final
{
public:
	static FMatchPlayPostRouteRollProviderResultValidationResult Validate(
		EMatchPlayCurrentAttackPostRouteRollPurpose RequestedPurpose,
		const FMatchPlayPostRouteRollProviderResult& ProviderResult);
};
