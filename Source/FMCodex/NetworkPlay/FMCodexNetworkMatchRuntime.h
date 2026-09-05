#pragma once

#include "CoreMinimal.h"

#include "FMCodexNetworkMatchTypes.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"
#include "../LocalPlay/FMCodexLocalMatchDemoConfiguration.h"

class FFMCodexLocalMatchD6Provider;
class FFMCodexNetworkEntryRollProvider;
class FMatchPlayAuthoritativeSession;
class FMatchPlayServerCoordinator;

struct FMCODEX_API FFMCodexNetworkBootstrapConfiguration
{
	FFMCodexLocalMatchDemoConfiguration MatchConfiguration;
	FFMCodexNetworkTeamIdentity PlayerATeam;
	FFMCodexNetworkTeamIdentity PlayerBTeam;
	int32 AttackOpportunitiesPerSide = 3;
};

class FMCODEX_API FFMCodexNetworkBootstrapConfigurationFactory final
{
public:
	static FFMCodexNetworkBootstrapConfiguration CreatePrototypeMatch();
};

struct FMCODEX_API FFMCodexNetworkRuntimeInitializeResult
{
	bool bSuccess = false;
	bool bAlreadyInitialized = false;
	FString ErrorMessage;
};

/** One server-owned authoritative runtime for one immutable MatchInstanceId. */
class FMCODEX_API FFMCodexNetworkMatchRuntime final : public IMatchPlayPlayerIntentPort
{
public:
	FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId, int32 Seed);
	~FFMCodexNetworkMatchRuntime();
	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FMatchPlayPlayerIntent& Intent) override;
	int32 GetEntryProviderInvocationCount() const;
	int32 GetD12ProviderInvocationCount() const;
#if WITH_DEV_AUTOMATION_TESTS
	FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId, int32 Seed,
		TUniquePtr<IMatchPlayAttackEntryRollProvider> TestEntryProvider);
	friend struct FFMCodexNetworkIntentTestAccess;
#endif

	FFMCodexNetworkRuntimeInitializeResult InitializeOnce(
		const FFMCodexNetworkBootstrapConfiguration& Configuration);

	FFMCodexNetworkClientViewSnapshot BuildClientView(
		EInitialTurnOrderPlayer ViewerSide,
		int32 ViewRevision,
		EFMCodexNetworkBootstrapState BootstrapState) const;

	bool IsInitialized() const;
	int32 GetInitializationAttemptCount() const;
	int32 GetInitializationCount() const;
	const FGuid& GetMatchInstanceId() const;

private:
	FGuid MatchInstanceId;
	int32 InitializationAttemptCount = 0;
	int32 InitializationCount = 0;
	bool bInitialized = false;
	TUniquePtr<FFMCodexLocalMatchD6Provider> RollProvider;
	TUniquePtr<FFMCodexNetworkEntryRollProvider> EntryProvider;
	int64 DisclosedInitialAttackSequence = 0;
	FSkillRuleSnapshotSet SkillRuleSet;
	TUniquePtr<FMatchPlayAuthoritativeSession> AuthoritativeSession;
	TUniquePtr<FMatchPlayServerCoordinator> ServerCoordinator;
};
