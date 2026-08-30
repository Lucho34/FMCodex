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

FMatchPlayAttackEntrySelectionProviderResultValidationResult
FMatchPlayAttackEntrySelectionProviderResultValidator::Validate(
	const EMatchPlayAttackEntryRollPurpose RequestedPurpose,
	const int32 CandidateCount,
	const FMatchPlayAttackEntrySelectionProviderResult& ProviderResult)
{
	FMatchPlayAttackEntrySelectionProviderResultValidationResult Result;
	if (RequestedPurpose
			!= EMatchPlayAttackEntryRollPurpose::SendingOffSelection
		|| CandidateCount < 2)
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Sending-Off selection requires its semantic purpose and at least two candidates.");
		return Result;
	}

	if (ProviderResult.bSuccess)
	{
		if (ProviderResult.ErrorCode
				!= EMatchPlayAttackEntryRollProviderErrorCode::None
			|| !ProviderResult.ErrorMessage.IsEmpty()
			|| ProviderResult.SelectedIndex < 0
			|| ProviderResult.SelectedIndex >= CandidateCount)
		{
			Result.ErrorCode =
				EMatchPlayAttackEntryRollProviderResultValidationErrorCode
					::MalformedProviderResult;
			Result.ErrorMessage =
				TEXT("A successful Sending-Off selection must contain only an in-range candidate index.");
			return Result;
		}
		Result.bIsCanonical = true;
		return Result;
	}

	if (ProviderResult.SelectedIndex == INDEX_NONE
		&& ProviderResult.ErrorCode
			== EMatchPlayAttackEntryRollProviderErrorCode::ProviderFailure
		&& !ProviderResult.ErrorMessage.IsEmpty())
	{
		Result.ErrorCode =
			EMatchPlayAttackEntryRollProviderResultValidationErrorCode
				::ProviderFailure;
		Result.ErrorMessage = ProviderResult.ErrorMessage;
		return Result;
	}

	Result.ErrorCode =
		EMatchPlayAttackEntryRollProviderResultValidationErrorCode
			::MalformedProviderResult;
	Result.ErrorMessage =
		TEXT("A failed Sending-Off selection provider result is not canonical.");
	return Result;
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
