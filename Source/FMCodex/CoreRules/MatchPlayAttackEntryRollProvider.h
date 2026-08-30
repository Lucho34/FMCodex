#pragma once

#include "CoreMinimal.h"

enum class EMatchPlayAttackEntryRollPurpose : uint8
{
	None,
	InitialActionPoint,
	SetPieceType
};

enum class EMatchPlayAttackEntryRollProviderErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure
};

struct FMCODEX_API FMatchPlayAttackEntryRollProviderResult
{
	bool bSuccess = false;
	int32 RawRoll = 0;
	EMatchPlayAttackEntryRollProviderErrorCode ErrorCode =
		EMatchPlayAttackEntryRollProviderErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API IMatchPlayAttackEntryRollProvider
{
public:
	virtual ~IMatchPlayAttackEntryRollProvider() = default;

	virtual FMatchPlayAttackEntryRollProviderResult RollD12(
		EMatchPlayAttackEntryRollPurpose Purpose) = 0;

	virtual FMatchPlayAttackEntryRollProviderResult RollD6(
		EMatchPlayAttackEntryRollPurpose Purpose) = 0;
};

enum class EMatchPlayAttackEntryRollProviderResultValidationErrorCode : uint8
{
	None,
	InvalidPurpose,
	ProviderFailure,
	MalformedProviderResult
};

struct FMCODEX_API FMatchPlayAttackEntryRollProviderResultValidationResult
{
	bool bIsCanonical = false;
	EMatchPlayAttackEntryRollProviderResultValidationErrorCode ErrorCode =
		EMatchPlayAttackEntryRollProviderResultValidationErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAttackEntryRollProviderResultValidator final
{
public:
	static FMatchPlayAttackEntryRollProviderResultValidationResult Validate(
		EMatchPlayAttackEntryRollPurpose RequestedPurpose,
		const FMatchPlayAttackEntryRollProviderResult& ProviderResult);
};
