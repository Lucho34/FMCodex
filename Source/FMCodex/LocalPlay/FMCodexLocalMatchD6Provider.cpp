#include "FMCodexLocalMatchD6Provider.h"

namespace FMCodexLocalMatchD6Provider
{
	bool IsValidPostRoutePurpose(
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
			return true;
		case EMatchPlayCurrentAttackPostRouteRollPurpose::None:
		default:
			return false;
		}
	}
}

FFMCodexLocalMatchD6Provider::FFMCodexLocalMatchD6Provider(
	const int32 Seed)
	: RandomStream(Seed)
{
}

FMatchPlayInitialRouteRollProviderResult
FFMCodexLocalMatchD6Provider::RollD6(
	const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
{
	FMatchPlayInitialRouteRollProviderResult Result;
	if (Purpose
		!= EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
	{
		Result.ErrorCode =
			EMatchPlayInitialRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local D6 initial-route roll requires InitialRoute purpose.");
		return Result;
	}

	Result.bSuccess = true;
	Result.RawD6 = RollCanonicalD6();
	return Result;
}

FMatchPlayPostRouteRollProviderResult
FFMCodexLocalMatchD6Provider::RollD6(
	const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
{
	using namespace FMCodexLocalMatchD6Provider;

	FMatchPlayPostRouteRollProviderResult Result;
	if (!IsValidPostRoutePurpose(Purpose))
	{
		Result.ErrorCode =
			EMatchPlayPostRouteRollProviderErrorCode::InvalidPurpose;
		Result.ErrorMessage =
			TEXT("Local D6 post-route roll requires a valid purpose.");
		return Result;
	}

	Result.bSuccess = true;
	Result.RawD6 = RollCanonicalD6();
	return Result;
}

int32 FFMCodexLocalMatchD6Provider::RollOrdinaryTacticalPoint()
{
	return RandomStream.RandRange(2, 8);
}

int32 FFMCodexLocalMatchD6Provider::RollCanonicalD6()
{
	return RandomStream.RandRange(1, 6);
}
