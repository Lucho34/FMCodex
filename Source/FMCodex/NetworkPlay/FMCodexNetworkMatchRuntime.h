#pragma once

#include "CoreMinimal.h"

#include "FMCodexNetworkMatchTypes.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"
#include "../LocalPlay/FMCodexLocalMatchDemoConfiguration.h"

class FFMCodexNetworkRandomProvider;
class IFMCodexNetworkEntropySource;
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
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	static FFMCodexNetworkBootstrapConfiguration CreateBFirstAutomationMatch();
#endif
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
	explicit FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId);
	~FFMCodexNetworkMatchRuntime();
	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FMatchPlayPlayerIntent& Intent) override;
	int32 GetEntryProviderInvocationCount() const;
	int32 GetD12ProviderInvocationCount() const;
#if WITH_DEV_AUTOMATION_TESTS
	FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId,
		TUniquePtr<IFMCodexNetworkEntropySource> TestEntropy,
		TUniquePtr<IMatchPlayAttackEntryRollProvider> TestEntryProvider = nullptr);
	friend struct FFMCodexNetworkIntentTestAccess;
	friend struct FFMCodexNetworkDeploymentTestAccess;
	friend struct FFMCodexNetworkDeploymentCompletionTestAccess;
	friend struct FFMCodexNetworkCarrierTestAccess;
	friend struct FFMCodexNetworkMarkerTestAccess;
	friend struct FFMCodexNetworkRunnerTestAccess;
	friend struct FFMCodexNetworkHelperTestAccess;
	friend struct FFMCodexNetworkSkillTestAccess;
	int32 GetCoordinatorInvocationCountForTests() const;
#if !UE_BUILD_SHIPPING
	void EnableDeploymentAutomationEntry(int32 InitialD12 = 4);
#endif
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
	FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId,
		TUniquePtr<FFMCodexNetworkRandomProvider> InRollProvider);
	FGuid MatchInstanceId;
	int32 InitializationAttemptCount = 0;
	int32 InitializationCount = 0;
	bool bInitialized = false;
	TUniquePtr<FFMCodexNetworkRandomProvider> RollProvider;
	TUniquePtr<FFMCodexNetworkEntryRollProvider> EntryProvider;
	int64 DisclosedInitialAttackSequence = 0;
	FSkillRuleSnapshotSet SkillRuleSet;
	TUniquePtr<FMatchPlayAuthoritativeSession> AuthoritativeSession;
	TUniquePtr<FMatchPlayServerCoordinator> ServerCoordinator;
};
