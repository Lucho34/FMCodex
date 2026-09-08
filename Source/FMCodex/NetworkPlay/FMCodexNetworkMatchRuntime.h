#pragma once

#include "CoreMinimal.h"

#include "FMCodexNetworkMatchTypes.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"
#include "../LocalPlay/FMCodexLocalMatchDemoConfiguration.h"

class FFMCodexNetworkRandomProvider;
class IFMCodexNetworkEntropySource;
class FFMCodexNetworkEntryRollProvider;
class FFMCodexNetworkInitialRouteRollProvider;
class FFMCodexNetworkPostRouteRollProvider;
class FFMCodexNetworkRecoveryProvider;
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
	static FFMCodexNetworkBootstrapConfiguration CreateBFirstAutomationMatch(bool bBranchFixture = false);
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
	int32 GetInitialRouteProviderInvocationCount() const;
	int32 GetPostRouteProviderInvocationCount() const;
	int32 GetRecoveryProviderInvocationCount() const;
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
	friend struct FFMCodexNetworkBranchTestAccess;
	friend struct FFMCodexNetworkInitialRouteTestAccess;
	friend struct FFMCodexNetworkCrossContestTestAccess;
	friend struct FFMCodexNetworkCrossTerminalTestAccess;
	int32 GetCoordinatorInvocationCountForTests() const;
#if !UE_BUILD_SHIPPING
	void EnableSetPieceSelectionMilestone(int32 TypeD6);
	void EnableNearFreeKickMilestone(bool Goal);
	void EnableLongFreeKickMilestone(bool Goal, bool EarlyNoGoal);
	void EnablePenaltyMilestone(bool Goal);
	void EnableDeploymentAutomationEntry(int32 InitialD12 = 4);
	void EnableInitialRouteAutomation(int32 D6);
	void EnablePostRouteAutomation(int32 AttackD6, int32 DefenseD6);
	bool PrepareInitialRouteMilestone(ESkillRuleType Family, EFMCodexNetworkDeclineAction StopBefore = EFMCodexNetworkDeclineAction::None);
	void EnableSpecializedShotAutomation(bool Goal, bool ImmediateMiss, bool Final);
	void EnableThroughBallConditionalAutomation(const FString& Path, bool Goal, bool Final);
	void EnableOrdinaryTerminalAutomation(bool Goal, bool Final, ESkillRuleType Family = ESkillRuleType::Cross, int32 RouteD6 = 5);
	bool PrepareOrdinaryTerminalMilestone(bool Goal, bool Final, bool bAwaitSkill = false, ESkillRuleType Family = ESkillRuleType::Cross, EFMCodexNetworkDeclineAction StopBefore = EFMCodexNetworkDeclineAction::None);
#endif
#endif

	FFMCodexNetworkRuntimeInitializeResult InitializeOnce(
		const FFMCodexNetworkBootstrapConfiguration& Configuration);

	FFMCodexNetworkClientViewSnapshot BuildClientView(
		EInitialTurnOrderPlayer ViewerSide,
		int32 ViewRevision,
		EFMCodexNetworkBootstrapState BootstrapState) const;

	void EnablePlayerFacingPresentation() { bPlayerFacingPresentation = true; }
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
	bool bPlayerFacingPresentation = false;
	bool bSetPieceSelectionMilestone = false;
	TUniquePtr<FFMCodexNetworkRandomProvider> RollProvider;
	TUniquePtr<FFMCodexNetworkEntryRollProvider> EntryProvider;
	TUniquePtr<FFMCodexNetworkInitialRouteRollProvider> InitialRouteProvider;
	TUniquePtr<FFMCodexNetworkPostRouteRollProvider> PostRouteProvider;
	TUniquePtr<FFMCodexNetworkRecoveryProvider> RecoveryProvider;
	int64 DisclosedTerminalAttackSequence = 0;
	int64 DisclosedInitialAttackSequence = 0;
	int64 DisclosedSetPieceTypeSequence = 0;
	int64 DisclosedSetPieceResolutionSequence = 0;
	int32 DisclosedSetPieceResolutionRollCount = 0;
	int64 DisclosedRouteAttackSequence = 0;
	int64 DisclosedContestAttackSequence = 0;
	int32 DisclosedContestRollCount = 0;
	FSkillRuleSnapshotSet SkillRuleSet;
	TUniquePtr<FMatchPlayAuthoritativeSession> AuthoritativeSession;
	TUniquePtr<FMatchPlayServerCoordinator> ServerCoordinator;
};
