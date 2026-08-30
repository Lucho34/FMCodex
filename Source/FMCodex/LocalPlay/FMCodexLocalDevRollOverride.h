#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "FMCodexLocalMatchD6Provider.h"

/** Developer-facing semantic targets. These are infrastructure labels, not gameplay state. */
enum class EFMCodexLocalDevRollTarget : uint8
{
	None = 0,
	TacticalPoint,
	ThroughBallRoute,
	ThroughBallBehindDefenseP1,
	ThroughBallAntiOffside,
	ThroughBallFeetAttack,
	ThroughBallFeetDefense,
	CrossRoute,
	CrossHighAttack,
	CrossHighDefense,
	CrossLowAttack,
	CrossLowDefense,
	OneOnOneChipShotAttack,
	OneOnOneDirectShotAttack,
	OneOnOneDirectShotDefense,
	LongShotDirectAttack,
	LongShotDirectDefense,
	LongShotDeadCornerA,
	LongShotDeadCornerB,
	CutInsideShotDirectAttack,
	CutInsideShotDirectDefense,
	CutInsideShotDeadCornerA,
	CutInsideShotDeadCornerB,
	PassControlRoute,
	PassControlAttack,
	PassControlDefense
};

/** Host-only call-site identity used to disambiguate shared CoreRules purposes. */
enum class EFMCodexLocalDevRollInvocation : uint8
{
	None = 0,
	ThroughBallInitialRoute,
	CrossInitialRoute,
	ThroughBallBehindDefenseP1,
	ThroughBallAntiOffside,
	ThroughBallFeetAttack,
	ThroughBallFeetDefense,
	CrossHighAttack,
	CrossHighDefense,
	CrossLowAttack,
	CrossLowDefense,
	OneOnOneChipShot,
	OneOnOneDirectShot,
	LongShotDirectShot,
	LongShotDeadCorner,
	CutInsideShotDirectShot,
	CutInsideShotDeadCorner,
	PassControlInitialRoute,
	PassControlAttack,
	PassControlDefense
};

struct FMCODEX_API FFMCodexLocalDevRollOverrideRequest final
{
	EFMCodexLocalDevRollTarget Target = EFMCodexLocalDevRollTarget::None;
	int32 Value = 0;
};

struct FMCODEX_API FFMCodexLocalDevRollOverrideCommandResult final
{
	bool bSuccess = false;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalDevPendingRollOverride final
{
	EFMCodexLocalDevRollTarget Target = EFMCodexLocalDevRollTarget::None;
	int32 Value = 0;
};

/**
 * Non-Shipping decorator around the unchanged production RNG provider.
 * A matching override returns before calling the wrapped provider, so the
 * seeded stream cursor is not advanced. No canonical state owns this object.
 */
class FMCODEX_API FFMCodexLocalDevRollOverride final
	: public IMatchPlayInitialRouteRollProvider
	, public IMatchPlayPostRouteRollProvider
	, public IMatchPlayRecoveryProvider
{
public:
	explicit FFMCodexLocalDevRollOverride(
		FFMCodexLocalMatchD6Provider& InProductionProvider);

	FFMCodexLocalDevRollOverrideCommandResult SetOverride(
		const FFMCodexLocalDevRollOverrideRequest& Request);
	bool ClearOverride(EFMCodexLocalDevRollTarget Target);
	void ClearAllOverrides();
	bool HasPendingOverride(EFMCodexLocalDevRollTarget Target) const;
	TArray<FFMCodexLocalDevPendingRollOverride> GetPendingOverrides() const;
	FFMCodexLocalDevRollOverrideCommandResult SetRecoveryOverride(
		const TArray<int32>& OrderedCandidateIndices);
	bool ClearRecoveryOverride();
	bool HasPendingRecoveryOverride() const;

	template <typename TCallable>
	decltype(auto) InvokeAs(
		const EFMCodexLocalDevRollInvocation Invocation,
		TCallable&& Callable)
	{
		TGuardValue<EFMCodexLocalDevRollInvocation> Guard(
			ActiveInvocation,
			Invocation);
		return Forward<TCallable>(Callable)();
	}

	virtual FMatchPlayInitialRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose Purpose) override;
	virtual FMatchPlayPostRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override;
	virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose Purpose,
		const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
		int32 ReturnCount) override;

	int32 RollOrdinaryTacticalPoint();

private:
	TOptional<int32> Consume(EFMCodexLocalDevRollTarget Target);
	EFMCodexLocalDevRollTarget ResolveInitialRouteTarget(
		EMatchPlayCurrentAttackResolutionRollPurpose Purpose) const;
	EFMCodexLocalDevRollTarget ResolvePostRouteTarget(
		EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) const;

	FFMCodexLocalMatchD6Provider& ProductionProvider;
	TMap<EFMCodexLocalDevRollTarget, int32> PendingOverrides;
	TOptional<TArray<int32>> PendingRecoveryOverride;
	EFMCodexLocalDevRollInvocation ActiveInvocation =
		EFMCodexLocalDevRollInvocation::None;
};

#endif
