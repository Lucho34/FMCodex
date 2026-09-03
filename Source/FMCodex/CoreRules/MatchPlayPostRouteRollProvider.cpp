#include "MatchPlayPostRouteRollProvider.h"

namespace MatchPlayPostRouteRollProvider
{
	bool IsValidPurpose(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::BehindDefenseP2Defense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneChipShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneDirectShotDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickAngledA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::ShortFreeKickAngledB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickPowerA:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::LongFreeKickPowerB:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::PenaltyDirectAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::PenaltyDirectDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PenaltyPanenka:
		case EMatchPlayCurrentAttackPostRouteRollPurpose
			::CornerParticipantSelection:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerRoute:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerDefense:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::CornerAutomaticScorer:
			return true;
		case EMatchPlayCurrentAttackPostRouteRollPurpose::None:
		default:
			return false;
		}
	}

	void SetFailure(
		FMatchPlayPostRouteRollProviderResultValidationResult& Result,
		const EMatchPlayPostRouteRollProviderResultValidationErrorCode
			ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayPostRouteRollProviderResultValidationResult
FMatchPlayPostRouteRollProviderResultValidator::Validate(
	const EMatchPlayCurrentAttackPostRouteRollPurpose RequestedPurpose,
	const FMatchPlayPostRouteRollProviderResult& ProviderResult)
{
	using namespace MatchPlayPostRouteRollProvider;

	FMatchPlayPostRouteRollProviderResultValidationResult Result;
	if (!IsValidPurpose(RequestedPurpose))
	{
		SetFailure(
			Result,
			EMatchPlayPostRouteRollProviderResultValidationErrorCode
				::InvalidPurpose,
			TEXT("Post-route RollD6 requires a valid post-route purpose."));
		return Result;
	}

	if (ProviderResult.bSuccess)
	{
		if (ProviderResult.ErrorCode
				!= EMatchPlayPostRouteRollProviderErrorCode::None
			|| !ProviderResult.ErrorMessage.IsEmpty()
			|| ProviderResult.RawD6 < 1
			|| ProviderResult.RawD6 > 6)
		{
			SetFailure(
				Result,
				EMatchPlayPostRouteRollProviderResultValidationErrorCode
					::MalformedProviderResult,
				TEXT("A successful post-route provider result must contain only a D6 in range [1, 6]."));
			return Result;
		}

		Result.bIsCanonical = true;
		return Result;
	}

	if (ProviderResult.RawD6 == 0
		&& ProviderResult.ErrorCode
			== EMatchPlayPostRouteRollProviderErrorCode::ProviderFailure
		&& !ProviderResult.ErrorMessage.IsEmpty())
	{
		SetFailure(
			Result,
			EMatchPlayPostRouteRollProviderResultValidationErrorCode
				::ProviderFailure,
			ProviderResult.ErrorMessage);
		return Result;
	}

	SetFailure(
		Result,
		EMatchPlayPostRouteRollProviderResultValidationErrorCode
			::MalformedProviderResult,
		TEXT("A failed post-route provider result is not canonical."));
	return Result;
}
