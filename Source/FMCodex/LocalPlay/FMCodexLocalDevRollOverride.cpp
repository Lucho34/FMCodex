#include "FMCodexLocalDevRollOverride.h"

#if !UE_BUILD_SHIPPING

namespace FMCodexLocalDevRollOverride
{
	bool IsTacticalPointTarget(const EFMCodexLocalDevRollTarget Target)
	{
		return Target == EFMCodexLocalDevRollTarget::TacticalPoint;
	}

	bool IsValidTarget(const EFMCodexLocalDevRollTarget Target)
	{
		return Target > EFMCodexLocalDevRollTarget::None
			&& Target <= EFMCodexLocalDevRollTarget
				::OneOnOneDirectShotDefense;
	}
}

FFMCodexLocalDevRollOverride::FFMCodexLocalDevRollOverride(
	FFMCodexLocalMatchD6Provider& InProductionProvider)
	: ProductionProvider(InProductionProvider)
{
}

FFMCodexLocalDevRollOverrideCommandResult
FFMCodexLocalDevRollOverride::SetOverride(
	const FFMCodexLocalDevRollOverrideRequest& Request)
{
	using namespace FMCodexLocalDevRollOverride;
	FFMCodexLocalDevRollOverrideCommandResult Result;
	if (!IsValidTarget(Request.Target))
	{
		Result.ErrorMessage = TEXT("A valid DEV roll target is required.");
		return Result;
	}
	const int32 Minimum = IsTacticalPointTarget(Request.Target) ? 2 : 1;
	const int32 Maximum = IsTacticalPointTarget(Request.Target) ? 8 : 6;
	if (Request.Value < Minimum || Request.Value > Maximum)
	{
		Result.ErrorMessage = FString::Printf(
			TEXT("DEV roll value must be in range [%d, %d]."),
			Minimum,
			Maximum);
		return Result;
	}

	PendingOverrides.Add(Request.Target, Request.Value);
	Result.bSuccess = true;
	return Result;
}

bool FFMCodexLocalDevRollOverride::ClearOverride(
	const EFMCodexLocalDevRollTarget Target)
{
	return PendingOverrides.Remove(Target) > 0;
}

void FFMCodexLocalDevRollOverride::ClearAllOverrides()
{
	PendingOverrides.Reset();
}

bool FFMCodexLocalDevRollOverride::HasPendingOverride(
	const EFMCodexLocalDevRollTarget Target) const
{
	return PendingOverrides.Contains(Target);
}

TArray<FFMCodexLocalDevPendingRollOverride>
FFMCodexLocalDevRollOverride::GetPendingOverrides() const
{
	TArray<FFMCodexLocalDevPendingRollOverride> Result;
	Result.Reserve(PendingOverrides.Num());
	for (const TPair<EFMCodexLocalDevRollTarget, int32>& Pair
		: PendingOverrides)
	{
		FFMCodexLocalDevPendingRollOverride Item;
		Item.Target = Pair.Key;
		Item.Value = Pair.Value;
		Result.Add(Item);
	}
	Result.Sort([](
		const FFMCodexLocalDevPendingRollOverride& Left,
		const FFMCodexLocalDevPendingRollOverride& Right)
	{
		return static_cast<uint8>(Left.Target)
			< static_cast<uint8>(Right.Target);
	});
	return Result;
}

FMatchPlayInitialRouteRollProviderResult
FFMCodexLocalDevRollOverride::RollD6(
	const EMatchPlayCurrentAttackResolutionRollPurpose Purpose)
{
	if (const TOptional<int32> Override = Consume(
		ResolveInitialRouteTarget(Purpose)); Override.IsSet())
	{
		FMatchPlayInitialRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = Override.GetValue();
		return Result;
	}
	return ProductionProvider.RollD6(Purpose);
}

FMatchPlayPostRouteRollProviderResult
FFMCodexLocalDevRollOverride::RollD6(
	const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
{
	if (const TOptional<int32> Override = Consume(
		ResolvePostRouteTarget(Purpose)); Override.IsSet())
	{
		FMatchPlayPostRouteRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawD6 = Override.GetValue();
		return Result;
	}
	return ProductionProvider.RollD6(Purpose);
}

int32 FFMCodexLocalDevRollOverride::RollOrdinaryTacticalPoint()
{
	if (const TOptional<int32> Override = Consume(
		EFMCodexLocalDevRollTarget::TacticalPoint); Override.IsSet())
	{
		return Override.GetValue();
	}
	return ProductionProvider.RollOrdinaryTacticalPoint();
}

TOptional<int32> FFMCodexLocalDevRollOverride::Consume(
	const EFMCodexLocalDevRollTarget Target)
{
	if (Target == EFMCodexLocalDevRollTarget::None)
	{
		return {};
	}
	const int32* PendingValue = PendingOverrides.Find(Target);
	if (PendingValue == nullptr)
	{
		return {};
	}
	const int32 Value = *PendingValue;
	PendingOverrides.Remove(Target);
	return Value;
}

EFMCodexLocalDevRollTarget
FFMCodexLocalDevRollOverride::ResolveInitialRouteTarget(
	const EMatchPlayCurrentAttackResolutionRollPurpose Purpose) const
{
	if (Purpose != EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
	{
		return EFMCodexLocalDevRollTarget::None;
	}
	switch (ActiveInvocation)
	{
	case EFMCodexLocalDevRollInvocation::ThroughBallInitialRoute:
		return EFMCodexLocalDevRollTarget::ThroughBallRoute;
	case EFMCodexLocalDevRollInvocation::CrossInitialRoute:
		return EFMCodexLocalDevRollTarget::CrossRoute;
	default:
		return EFMCodexLocalDevRollTarget::None;
	}
}

EFMCodexLocalDevRollTarget
FFMCodexLocalDevRollOverride::ResolvePostRouteTarget(
	const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) const
{
	using EInvocation = EFMCodexLocalDevRollInvocation;
	using EPostPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	switch (ActiveInvocation)
	{
	case EInvocation::ThroughBallBehindDefenseP1:
		return Purpose == EPostPurpose::PrimaryAttack
			? EFMCodexLocalDevRollTarget::ThroughBallBehindDefenseP1
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::ThroughBallAntiOffside:
		return Purpose == EPostPurpose::PrimaryAttack
			? EFMCodexLocalDevRollTarget::ThroughBallAntiOffside
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::ThroughBallFeetAttack:
		return Purpose == EPostPurpose::PrimaryAttack
			? EFMCodexLocalDevRollTarget::ThroughBallFeetAttack
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::ThroughBallFeetDefense:
		return Purpose == EPostPurpose::PrimaryDefense
			? EFMCodexLocalDevRollTarget::ThroughBallFeetDefense
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::CrossHighAttack:
		return Purpose == EPostPurpose::PrimaryAttack
			? EFMCodexLocalDevRollTarget::CrossHighAttack
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::CrossHighDefense:
		return Purpose == EPostPurpose::PrimaryDefense
			? EFMCodexLocalDevRollTarget::CrossHighDefense
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::CrossLowAttack:
		return Purpose == EPostPurpose::PrimaryAttack
			? EFMCodexLocalDevRollTarget::CrossLowAttack
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::CrossLowDefense:
		return Purpose == EPostPurpose::PrimaryDefense
			? EFMCodexLocalDevRollTarget::CrossLowDefense
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::OneOnOneChipShot:
		return Purpose == EPostPurpose::OneOnOneChipShotAttack
			? EFMCodexLocalDevRollTarget::OneOnOneChipShotAttack
			: EFMCodexLocalDevRollTarget::None;
	case EInvocation::OneOnOneDirectShot:
		if (Purpose == EPostPurpose::OneOnOneDirectShotAttack)
		{
			return EFMCodexLocalDevRollTarget::OneOnOneDirectShotAttack;
		}
		return Purpose == EPostPurpose::OneOnOneDirectShotDefense
			? EFMCodexLocalDevRollTarget::OneOnOneDirectShotDefense
			: EFMCodexLocalDevRollTarget::None;
	default:
		return EFMCodexLocalDevRollTarget::None;
	}
}

#endif
