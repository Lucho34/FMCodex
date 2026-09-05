#pragma once

#include "CoreMinimal.h"
#include "../CoreRules/MatchPlayAttackEntryRollProvider.h"
#include "../CoreRules/MatchPlayInitialRouteRollProvider.h"
#include "../CoreRules/MatchPlayPostRouteRollProvider.h"
#include "../CoreRules/MatchPlayRecovery.h"

/** Private server entropy boundary. Never a UObject, wire payload, or match fact. */
class IFMCodexNetworkEntropySource
{
public:
	virtual ~IFMCodexNetworkEntropySource() = default;
	virtual bool Fill(TArrayView<uint8> Bytes) = 0;
};

/** All Network gameplay randomness, through the existing canonical provider interfaces. */
class FMCODEX_API FFMCodexNetworkRandomProvider final
	: public IMatchPlayAttackEntryRollProvider
	, public IMatchPlayInitialRouteRollProvider
	, public IMatchPlayPostRouteRollProvider
	, public IMatchPlayRecoveryProvider
{
public:
	FFMCodexNetworkRandomProvider();
	~FFMCodexNetworkRandomProvider();
#if WITH_DEV_AUTOMATION_TESTS
	explicit FFMCodexNetworkRandomProvider(TUniquePtr<IFMCodexNetworkEntropySource> TestEntropy);
#endif
	virtual FMatchPlayAttackEntryRollProviderResult RollD12(EMatchPlayAttackEntryRollPurpose Purpose) override;
	virtual FMatchPlayAttackEntryRollProviderResult RollD6(EMatchPlayAttackEntryRollPurpose Purpose) override;
	virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
		EMatchPlayAttackEntryRollPurpose Purpose, int32 CandidateCount) override;
	virtual FMatchPlayInitialRouteRollProviderResult RollD6(EMatchPlayCurrentAttackResolutionRollPurpose Purpose) override;
	virtual FMatchPlayPostRouteRollProviderResult RollD6(EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override;
	virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose Purpose, const TArray<FMatchPlayRecoveryCandidate>& OrderedCandidates,
		int32 ReturnCount) override;

private:
	// Uniform [0, Count). Rejection is bounded; failure never substitutes a weak RNG.
	bool SampleIndex(int32 Count, int32& OutIndex);
	TUniquePtr<IFMCodexNetworkEntropySource> Entropy;
};
