#include "MatchPlayAttackEntryRollProvider.h"

namespace MatchPlayAttackEntryRollProvider
{
	int32 GetMaximumRoll(
		const EMatchPlayAttackEntryRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayAttackEntryRollPurpose::InitialActionPoint:
			return 12;
		case EMatchPlayAttackEntryRollPurpose::SetPieceType:
			return 6;
		case EMatchPlayAttackEntryRollPurpose::None:
		default:
			return 0;
		}
	}

	void SetFailure(
		FMatchPlayAttackEntryRollProviderResultValidationResult& Result,
		const EMatchPlayAttackEntryRollProviderResultValidationErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayAttackEntryRollProviderResultValidationResult
FMatchPlayAttackEntryRollProviderResultValidator::Validate(
	const EMatchPlayAttackEntryRollPurpose RequestedPurpose,
	const FMatchPlayAttackEntryRollProviderResult& ProviderResult)
{
	using namespace MatchPlayAttackEntryRollProvider;

	FMatchPlayAttackEntryRollProviderResultValidationResult Result;
	const int32 MaximumRoll = GetMaximumRoll(RequestedPurpose);
	if (MaximumRoll == 0)
	{
		SetFailure(
			Result,
			EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::InvalidPurpose,
			TEXT("Attack-entry roll requires a valid semantic purpose."));
		return Result;
	}

	if (ProviderResult.bSuccess)
	{
		if (ProviderResult.ErrorCode
				!= EMatchPlayAttackEntryRollProviderErrorCode::None
			|| !ProviderResult.ErrorMessage.IsEmpty()
			|| ProviderResult.RawRoll < 1
			|| ProviderResult.RawRoll > MaximumRoll)
		{
			SetFailure(
				Result,
				EMatchPlayAttackEntryRollProviderResultValidationErrorCode
					::MalformedProviderResult,
				FString::Printf(
					TEXT("A successful attack-entry provider result must contain only a roll in range [1, %d]."),
					MaximumRoll));
			return Result;
		}

		Result.bIsCanonical = true;
		return Result;
	}

	if (ProviderResult.RawRoll == 0
		&& ProviderResult.ErrorCode
			== EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure
		&& !ProviderResult.ErrorMessage.IsEmpty())
	{
		SetFailure(
			Result,
			EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::ProviderFailure,
			ProviderResult.ErrorMessage);
		return Result;
	}

	SetFailure(
		Result,
		EMatchPlayAttackEntryRollProviderResultValidationErrorCode
			::MalformedProviderResult,
		TEXT("A failed attack-entry provider result is not canonical."));
	return Result;
}
