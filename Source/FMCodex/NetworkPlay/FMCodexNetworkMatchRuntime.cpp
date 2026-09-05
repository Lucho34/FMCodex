#include "FMCodexNetworkMatchRuntime.h"

#include "FMCodexNetworkRandomProvider.h"
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../LocalPlay/FMCodexPrototypeTeamContent.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../MatchPlayRuntime/MatchPlayServerCoordinator.h"
#include "../MatchPlayRuntime/MatchPlayEntryDeploymentPlayerIntentPort.h"

/** Server-only decorator; delegates randomness unchanged. */
class FFMCodexNetworkEntryRollProvider final : public IMatchPlayAttackEntryRollProvider
{
public:
	explicit FFMCodexNetworkEntryRollProvider(IMatchPlayAttackEntryRollProvider& InInner)
		: Inner(&InInner) {}
	virtual FMatchPlayAttackEntryRollProviderResult RollD12(
		EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		++InvocationCount;
		++D12Count;
		return Inner->RollD12(Purpose);
	}
	virtual FMatchPlayAttackEntryRollProviderResult RollD6(
		EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		++InvocationCount;
		return Inner->RollD6(Purpose);
	}
	virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
		EMatchPlayAttackEntryRollPurpose Purpose, int32 CandidateCount) override
	{
		++InvocationCount;
		return Inner->SelectUniformIndex(Purpose, CandidateCount);
	}
#if WITH_DEV_AUTOMATION_TESTS
	void Inject(TUniquePtr<IMatchPlayAttackEntryRollProvider> Provider)
	{
		TestProvider = MoveTemp(Provider);
		Inner = TestProvider.Get();
	}
	TUniquePtr<IMatchPlayAttackEntryRollProvider> TestProvider;
#endif
	IMatchPlayAttackEntryRollProvider* Inner;
	int32 InvocationCount = 0;
	int32 D12Count = 0;
};
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
/** Fixture changes only the initial entry draw; all other entry draws still use secure RNG. */
class FFMCodexDeploymentAutomationEntry final : public IMatchPlayAttackEntryRollProvider
{
public:
	explicit FFMCodexDeploymentAutomationEntry(IMatchPlayAttackEntryRollProvider& InSecure, int32 InInitialD12)
		: Secure(InSecure), InitialD12(InInitialD12) {}
	virtual FMatchPlayAttackEntryRollProviderResult RollD12(EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		if (Purpose != EMatchPlayAttackEntryRollPurpose::InitialActionPoint) { return Secure.RollD12(Purpose); }
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawRoll = InitialD12;
		return Result;
	}
	virtual FMatchPlayAttackEntryRollProviderResult RollD6(EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		return Secure.RollD6(Purpose);
	}
	virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(
		EMatchPlayAttackEntryRollPurpose Purpose, int32 Count) override
	{
		return Secure.SelectUniformIndex(Purpose, Count);
	}
private:
	IMatchPlayAttackEntryRollProvider& Secure;
	int32 InitialD12;
};
void FFMCodexNetworkMatchRuntime::EnableDeploymentAutomationEntry(int32 InitialD12)
{
	check(!bInitialized);
	check(InitialD12 == 4 || InitialD12 == 6);
	EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, InitialD12));
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation deployment fixture: initial D12=%d; other providers remain secure."), InitialD12);
}
#endif
namespace FMCodexNetworkMatchRuntime
{
	FFMCodexNetworkTeamIdentity MakeTeamIdentity(const FName TeamId)
	{
		FFMCodexNetworkTeamIdentity Result;
		Result.TeamId = TeamId;
		for (const FFMCodexPrototypePlayerDefinition& Definition
			: FFMCodexPrototypeTeamContent::GetDefinitions())
		{
			if (Definition.TeamId == TeamId)
			{
				Result.TeamDisplayName =
					Definition.TeamDisplayName.ToString();
				break;
			}
		}
		return Result;
	}
}

FFMCodexNetworkBootstrapConfiguration
FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch()
{
	FFMCodexNetworkBootstrapConfiguration Result;
	Result.MatchConfiguration =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Result.PlayerATeam = FMCodexNetworkMatchRuntime::MakeTeamIdentity(
		FFMCodexPrototypeTeamContent::ArsenalTeamId());
	Result.PlayerBTeam = FMCodexNetworkMatchRuntime::MakeTeamIdentity(
		FFMCodexPrototypeTeamContent::ManchesterCityTeamId());
	Result.AttackOpportunitiesPerSide = 3;
	return Result;
}

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
FFMCodexNetworkBootstrapConfiguration
FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
{
	auto Result = CreatePrototypeMatch();
	auto& Opening = Result.MatchConfiguration.OpeningInput.OpeningInput;
	// Equal rarity totals let the canonical tie-break rule decide the opening side.
	// Only this explicit server automation fixture changes these card inputs.
	for (auto& Card : Opening.PlayerADeck) { Card.Rarity = ECardRarity::Common; }
	for (auto& Card : Opening.PlayerBDeck) { Card.Rarity = ECardRarity::Common; }
	Opening.PlayerATieBreakerRoll = 6;
	Opening.PlayerBTieBreakerRoll = 2;
	return Result;
}
#endif

FFMCodexNetworkMatchRuntime::FFMCodexNetworkMatchRuntime(const FGuid& InMatchInstanceId)
	: FFMCodexNetworkMatchRuntime(InMatchInstanceId, MakeUnique<FFMCodexNetworkRandomProvider>())
{
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Network production RNG: PlatformCrypto secure bytes per draw; no public seed."));
}

FFMCodexNetworkMatchRuntime::FFMCodexNetworkMatchRuntime(
	const FGuid& InMatchInstanceId, TUniquePtr<FFMCodexNetworkRandomProvider> InRollProvider)
	: MatchInstanceId(InMatchInstanceId)
	, RollProvider(MoveTemp(InRollProvider))
	, EntryProvider(MakeUnique<FFMCodexNetworkEntryRollProvider>(*RollProvider))
{
}

FFMCodexNetworkMatchRuntime::~FFMCodexNetworkMatchRuntime() = default;

FFMCodexNetworkRuntimeInitializeResult
FFMCodexNetworkMatchRuntime::InitializeOnce(
	const FFMCodexNetworkBootstrapConfiguration& Configuration)
{
	FFMCodexNetworkRuntimeInitializeResult Result;
	++InitializationAttemptCount;
	if (bInitialized)
	{
		Result.bSuccess = true;
		Result.bAlreadyInitialized = true;
		return Result;
	}
	if (!MatchInstanceId.IsValid())
	{
		Result.ErrorMessage = TEXT("MatchInstanceId is invalid.");
		return Result;
	}

	SkillRuleSet = Configuration.MatchConfiguration.SkillRuleSet;
	AuthoritativeSession = MakeUnique<FMatchPlayAuthoritativeSession>(
		*EntryProvider,
		*RollProvider,
		*RollProvider,
		*RollProvider,
		SkillRuleSet);
	ServerCoordinator = MakeUnique<FMatchPlayServerCoordinator>(
		*AuthoritativeSession,
		SkillRuleSet);
	const FMatchPlayAuthoritativeInitializeMatchResult InitializeResult =
		AuthoritativeSession->InitializeMatch(
			Configuration.MatchConfiguration.OpeningInput);
	if (!InitializeResult.RuntimeEnvelope.bAccepted
		|| !InitializeResult.RuntimeEnvelope.bDomainSuccess
		|| !InitializeResult.OpeningResult.bSuccess)
	{
		Result.ErrorMessage = !InitializeResult.RuntimeEnvelope.ErrorMessage.IsEmpty()
			? InitializeResult.RuntimeEnvelope.ErrorMessage
			: InitializeResult.OpeningResult.ErrorMessage;
		AuthoritativeSession.Reset();
		ServerCoordinator.Reset();
		return Result;
	}
	const FMatchPlayServerCoordinatorResult CoordinatorResult =
		ServerCoordinator->AdvanceToStableState();
	if (!CoordinatorResult.bSuccess)
	{
		Result.ErrorMessage = CoordinatorResult.ErrorMessage;
		AuthoritativeSession.Reset();
		ServerCoordinator.Reset();
		return Result;
	}

	bInitialized = true;
	++InitializationCount;
	Result.bSuccess = true;
	return Result;
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkMatchRuntime::BuildClientView(
	const EInitialTurnOrderPlayer ViewerSide,
	const int32 ViewRevision,
	const EFMCodexNetworkBootstrapState BootstrapState) const
{
	if (!bInitialized || !AuthoritativeSession.IsValid())
	{
		return FFMCodexNetworkClientViewSnapshotFactory::BuildWaiting(
			MatchInstanceId,
			ViewRevision,
			ViewerSide,
			BootstrapState);
	}
	// Only the accepted entry of this exact attack is disclosed.
	const FMatchPlayState Snapshot = AuthoritativeSession->GetStateSnapshot();
	FFMCodexLocalMatchViewerDisclosure Disclosure;
	Disclosure.bRevealInitialActionPointRoll = Snapshot.bHasCurrentAttack
		&& DisclosedInitialAttackSequence == Snapshot.CurrentAttack.AttackSequence;
	const FFMCodexLocalMatchInteractionView SafeViewerView =
		FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Snapshot,
			SkillRuleSet,
			ViewerSide,
			Disclosure);
	return FFMCodexNetworkClientViewSnapshotFactory::Build(
		SafeViewerView,
		MatchInstanceId,
		ViewRevision,
		ViewerSide,
		BootstrapState);
}

bool FFMCodexNetworkMatchRuntime::IsInitialized() const
{
	return bInitialized;
}

int32 FFMCodexNetworkMatchRuntime::GetInitializationAttemptCount() const
{
	return InitializationAttemptCount;
}

int32 FFMCodexNetworkMatchRuntime::GetInitializationCount() const
{
	return InitializationCount;
}

const FGuid& FFMCodexNetworkMatchRuntime::GetMatchInstanceId() const
{
	return MatchInstanceId;
}

#if WITH_DEV_AUTOMATION_TESTS
int32 FFMCodexNetworkMatchRuntime::GetCoordinatorInvocationCountForTests() const
{
	return ServerCoordinator ? ServerCoordinator->GetInvocationCountForTests() : 0;
}
FFMCodexNetworkMatchRuntime::FFMCodexNetworkMatchRuntime(
	const FGuid& InMatchInstanceId, TUniquePtr<IFMCodexNetworkEntropySource> TestEntropy,
	TUniquePtr<IMatchPlayAttackEntryRollProvider> TestEntryProvider)
	: FFMCodexNetworkMatchRuntime(InMatchInstanceId,
		MakeUnique<FFMCodexNetworkRandomProvider>(MoveTemp(TestEntropy)))
{
	if (TestEntryProvider)
	{
		EntryProvider->Inject(MoveTemp(TestEntryProvider));
	}
}
#endif

FMatchPlayPlayerIntentSubmissionResult FFMCodexNetworkMatchRuntime::SubmitPlayerIntent(
	const FMatchPlayPlayerIntent& Intent)
{
	if (!bInitialized || !AuthoritativeSession || !ServerCoordinator)
	{
		FMatchPlayPlayerIntentSubmissionResult Result;
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NoActiveMatch;
		return Result;
	}
	FMatchPlayEntryDeploymentPlayerIntentPort Port(*AuthoritativeSession, *ServerCoordinator);
	auto Result = Port.SubmitPlayerIntent(Intent);
#if WITH_DEV_AUTOMATION_TESTS
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitCarrier)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Carrier authority: Success=%d Phase=%s SelectionStage=%s Carrier=%s Marker=%s Attacker=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.SelectionStage)),
			*State.CurrentAttack.ActionPreparation.CarrierCardId.ToString(), *State.CurrentAttack.ActionPreparation.MarkerCardId.ToString(),
			static_cast<int32>(State.RuntimeState.CurrentAttackingPlayer), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason));
	}
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitMarker)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Marker authority: Success=%d Phase=%s SelectionStage=%s Carrier=%s Marker=%s Attacker=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d Runner=%s Helper=%s Skill=%s ActionType=%s SkillDeferred=%d SelectedAction=%d ResolutionSession=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.SelectionStage)),
			*State.CurrentAttack.ActionPreparation.CarrierCardId.ToString(), *State.CurrentAttack.ActionPreparation.MarkerCardId.ToString(),
			static_cast<int32>(State.RuntimeState.CurrentAttackingPlayer), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason),
			*State.CurrentAttack.ActionPreparation.RunnerCardId.ToString(), *State.CurrentAttack.ActionPreparation.HelperCardId.ToString(),
			*State.CurrentAttack.ActionPreparation.SkillId.ToString(),
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.ActionPreparation.ActionType)),
			State.CurrentAttack.ActionPreparation.bSkillSelectionDeferred,
			State.CurrentAttack.bHasSelectedAction, State.CurrentAttack.bHasResolutionSession);
	}
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitRunner)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Runner authority: Success=%d Phase=%s SelectionStage=%s Carrier=%s Marker=%s Attacker=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d Runner=%s Helper=%s Skill=%s ActionType=%s SkillDeferred=%d SelectedAction=%d ResolutionSession=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.SelectionStage)),
			*State.CurrentAttack.ActionPreparation.CarrierCardId.ToString(), *State.CurrentAttack.ActionPreparation.MarkerCardId.ToString(),
			static_cast<int32>(State.RuntimeState.CurrentAttackingPlayer), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason),
			*State.CurrentAttack.ActionPreparation.RunnerCardId.ToString(), *State.CurrentAttack.ActionPreparation.HelperCardId.ToString(),
			*State.CurrentAttack.ActionPreparation.SkillId.ToString(),
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.ActionPreparation.ActionType)),
			State.CurrentAttack.ActionPreparation.bSkillSelectionDeferred,
			State.CurrentAttack.bHasSelectedAction, State.CurrentAttack.bHasResolutionSession);
	}	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitHelper)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Helper authority: Success=%d Phase=%s SelectionStage=%s Carrier=%s Marker=%s Attacker=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d Runner=%s Helper=%s Skill=%s ActionType=%s SkillDeferred=%d SelectedAction=%d ResolutionSession=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.SelectionStage)),
			*State.CurrentAttack.ActionPreparation.CarrierCardId.ToString(), *State.CurrentAttack.ActionPreparation.MarkerCardId.ToString(),
			static_cast<int32>(State.RuntimeState.CurrentAttackingPlayer), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason),
			*State.CurrentAttack.ActionPreparation.RunnerCardId.ToString(), *State.CurrentAttack.ActionPreparation.HelperCardId.ToString(),
			*State.CurrentAttack.ActionPreparation.SkillId.ToString(),
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(State.CurrentAttack.ActionPreparation.ActionType)),
			State.CurrentAttack.ActionPreparation.bSkillSelectionDeferred,
			State.CurrentAttack.bHasSelectedAction, State.CurrentAttack.bHasResolutionSession);
	}

	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitSkill)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Attack = State.CurrentAttack;
		const auto& P = Attack.ActionPreparation;
		const auto& A = Attack.SelectedAction;
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = State.bHasCurrentAttack
			&& DisclosedInitialAttackSequence == Attack.AttackSequence;
		const auto Safe = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			State, SkillRuleSet, State.RuntimeState.CurrentAttackingPlayer, Disclosure);
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Skill authority: Success=%d Phase=%s SelectionStage=%s ExpectedSide=%d Carrier=%s Marker=%s Runner=%s Helper=%s Skill=%s ActionType=%s BranchIntent=%s RouteResolved=%d SkillDeferred=%d SelectedAction=%d ResolutionSession=%d Terminal=%d Interaction=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(Attack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(Attack.SelectionStage)),
			static_cast<int32>(Safe.ExpectedActingPlayer),
			*(Attack.bHasSelectedAction ? A.CarrierCardId : P.CarrierCardId).ToString(),
			*(Attack.bHasSelectedAction ? A.MarkerCardId : P.MarkerCardId).ToString(),
			*(Attack.bHasSelectedAction ? A.RunnerCardId : P.RunnerCardId).ToString(),
			*(Attack.bHasSelectedAction ? A.HelperCardId : P.HelperCardId).ToString(),
			*(Attack.bHasSelectedAction ? A.SkillId : P.SkillId).ToString(),
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(Attack.bHasSelectedAction ? A.ActionType : P.ActionType)),
			*StaticEnum<EMatchPlayElectiveBranchIntent>()->GetNameStringByValue(static_cast<int64>(A.ElectiveBranchIntent)),
			Attack.ResolutionSession.bHasActualBranch, P.bSkillSelectionDeferred,
			Attack.bHasSelectedAction, Attack.bHasResolutionSession, Safe.bTerminalPendingAdvance,
			static_cast<int32>(Safe.InteractionCategory), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason));
	}

#endif
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess)
	{
		DisclosedInitialAttackSequence = Result.AuthoritativeResult.RuntimeEnvelope.AttackSequence;
	}
	return Result;
}

int32 FFMCodexNetworkMatchRuntime::GetEntryProviderInvocationCount() const
{
	return EntryProvider->InvocationCount;
}

int32 FFMCodexNetworkMatchRuntime::GetD12ProviderInvocationCount() const
{
	return EntryProvider->D12Count;
}
