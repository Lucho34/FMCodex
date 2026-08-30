#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayInitialRouteRollProvider.h"
#include "../CoreRules/MatchPlayPostRouteRollProvider.h"
#include "../CoreRules/MatchPlayRecovery.h"

class FMCODEX_API FFMCodexLocalMatchD6Provider final
	: public IMatchPlayInitialRouteRollProvider
	, public IMatchPlayPostRouteRollProvider
	, public IMatchPlayRecoveryProvider
{
public:
	FFMCodexLocalMatchD6Provider() = delete;
	explicit FFMCodexLocalMatchD6Provider(int32 Seed);

	virtual FMatchPlayInitialRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackResolutionRollPurpose Purpose) override;

	virtual FMatchPlayPostRouteRollProviderResult RollD6(
		EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override;

	virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose Purpose,
		const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
		int32 ReturnCount) override;

	/**
	 * Host-owned roll for the currently implemented ordinary-attack Tactical
	 * Point slice. Values outside 2..8 belong to deferred AP1/set-piece flows.
	 */
	int32 RollOrdinaryTacticalPoint();

private:
	FFMCodexLocalMatchD6Provider(
		const FFMCodexLocalMatchD6Provider&) = delete;
	FFMCodexLocalMatchD6Provider& operator=(
		const FFMCodexLocalMatchD6Provider&) = delete;
	FFMCodexLocalMatchD6Provider(
		FFMCodexLocalMatchD6Provider&&) = delete;
	FFMCodexLocalMatchD6Provider& operator=(
		FFMCodexLocalMatchD6Provider&&) = delete;

	int32 RollCanonicalD6();

	FRandomStream RandomStream;
};
