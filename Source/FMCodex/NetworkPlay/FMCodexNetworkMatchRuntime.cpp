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
/** Private accounting only; weighted sampling remains in the existing secure provider. */
class FFMCodexNetworkRecoveryProvider final : public IMatchPlayRecoveryProvider
{
public:
	explicit FFMCodexNetworkRecoveryProvider(IMatchPlayRecoveryProvider& InInner) : Inner(InInner) {}
	virtual FMatchPlayRecoveryProviderResult DrawWeightedWithoutReplacement(
		EMatchPlayRecoveryPurpose Purpose, const TArray<FMatchPlayRecoveryCandidate>& Candidates, int32 ReturnCount) override
	{
		++InvocationCount;
		return Inner.DrawWeightedWithoutReplacement(Purpose, Candidates, ReturnCount);
	}
	int32 InvocationCount = 0;
private:
	IMatchPlayRecoveryProvider& Inner;
};
int32 FFMCodexNetworkMatchRuntime::GetRecoveryProviderInvocationCount() const
{
	return RecoveryProvider.IsValid() ? RecoveryProvider->InvocationCount : 0;
}
/** Counts the canonical post-route boundary; production uses the existing secure provider. */
class FFMCodexNetworkPostRouteRollProvider final : public IMatchPlayPostRouteRollProvider
{
public:
	explicit FFMCodexNetworkPostRouteRollProvider(IMatchPlayPostRouteRollProvider& InInner) : Inner(InInner) {}
	virtual FMatchPlayPostRouteRollProviderResult RollD6(EMatchPlayCurrentAttackPostRouteRollPurpose Purpose) override
	{
		++InvocationCount;
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
		const int32 D6 = Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA ? AutomationPairedAD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB ? AutomationPairedBD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack ? AutomationAttackD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense ? AutomationDefenseD6
            : Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack || Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneChipShotAttack ? AutomationOneOnOneAttackD6
            : Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense ? AutomationOneOnOneDefenseD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::ShortFreeKickDirectAttack ? AutomationNearAttackD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::ShortFreeKickDirectDefense ? AutomationNearDefenseD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::ShortFreeKickAngledA ? AutomationNearPairedAD6
			: Purpose == EMatchPlayCurrentAttackPostRouteRollPurpose::ShortFreeKickAngledB ? AutomationNearPairedBD6 : 0;
		if (D6 != 0)
		{
			FMatchPlayPostRouteRollProviderResult Result; Result.bSuccess = true; Result.RawD6 = D6; return Result;
		}
#endif
		return Inner.RollD6(Purpose);
	}
	int32 InvocationCount = 0;
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	int32 AutomationAttackD6 = 0, AutomationDefenseD6 = 0;
	int32 AutomationPairedAD6 = 0, AutomationPairedBD6 = 0;
	int32 AutomationOneOnOneAttackD6 = 0, AutomationOneOnOneDefenseD6 = 0;
	int32 AutomationNearAttackD6 = 0, AutomationNearDefenseD6 = 0, AutomationNearPairedAD6 = 0, AutomationNearPairedBD6 = 0;
#endif
private:
	IMatchPlayPostRouteRollProvider& Inner;
};
int32 FFMCodexNetworkMatchRuntime::GetPostRouteProviderInvocationCount() const
{
	return PostRouteProvider.IsValid() ? PostRouteProvider->InvocationCount : 0;
}
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
void FFMCodexNetworkMatchRuntime::EnablePostRouteAutomation(int32 AttackD6, int32 DefenseD6)
{
	check(!bInitialized && AttackD6 >= 1 && AttackD6 <= 6 && DefenseD6 >= 1 && DefenseD6 <= 6);
	PostRouteProvider->AutomationAttackD6 = AttackD6; PostRouteProvider->AutomationDefenseD6 = DefenseD6;
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation post-route provider enabled; default secure provider unchanged."));
}
#endif
/** Counts the canonical provider boundary; production draws delegate to the existing secure provider. */
class FFMCodexNetworkInitialRouteRollProvider final : public IMatchPlayInitialRouteRollProvider
{
public:
	explicit FFMCodexNetworkInitialRouteRollProvider(IMatchPlayInitialRouteRollProvider& InInner) : Inner(InInner) {}
	virtual FMatchPlayInitialRouteRollProviderResult RollD6(EMatchPlayCurrentAttackResolutionRollPurpose Purpose) override
	{
		++InvocationCount;
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
		if (AutomationD6 != 0 && Purpose == EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute)
		{
			FMatchPlayInitialRouteRollProviderResult Result;
			Result.bSuccess = true; Result.RawD6 = AutomationD6;
			return Result;
		}
#endif
		return Inner.RollD6(Purpose);
	}
	int32 InvocationCount = 0;
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
	int32 AutomationD6 = 0;
#endif
private:
	IMatchPlayInitialRouteRollProvider& Inner;
};
int32 FFMCodexNetworkMatchRuntime::GetInitialRouteProviderInvocationCount() const
{
	return InitialRouteProvider.IsValid() ? InitialRouteProvider->InvocationCount : 0;
}
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
void FFMCodexNetworkMatchRuntime::EnableInitialRouteAutomation(int32 D6)
{
	check(!bInitialized && D6 >= 1 && D6 <= 6);
	InitialRouteProvider->AutomationD6 = D6;
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation initial-route provider enabled; default secure provider unchanged."));
}
#endif
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
/** Fixture changes only the initial entry draw; all other entry draws still use secure RNG. */
class FFMCodexDeploymentAutomationEntry final : public IMatchPlayAttackEntryRollProvider
{
public:
	explicit FFMCodexDeploymentAutomationEntry(IMatchPlayAttackEntryRollProvider& InSecure, int32 InInitialD12, bool InPrelude = false, int32 InTypeD6 = 0)
		: Secure(InSecure), InitialD12(InInitialD12), bSendingOffPrelude(InPrelude), TypeD6(InTypeD6) {}
	virtual FMatchPlayAttackEntryRollProviderResult RollD12(EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		if (Purpose != EMatchPlayAttackEntryRollPurpose::InitialActionPoint) { return Secure.RollD12(Purpose); }
		FMatchPlayAttackEntryRollProviderResult Result;
		Result.bSuccess = true;
		Result.RawRoll = bSendingOffPrelude && InitialCalls++ == 0 ? 1 : InitialD12;
		return Result;
	}
	virtual FMatchPlayAttackEntryRollProviderResult RollD6(EMatchPlayAttackEntryRollPurpose Purpose) override
	{
		if (Purpose == EMatchPlayAttackEntryRollPurpose::SetPieceType && TypeD6 > 0)
		{ FMatchPlayAttackEntryRollProviderResult R; R.bSuccess = true; R.RawRoll = TypeD6; return R; }
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
	bool bSendingOffPrelude = false;
	int32 InitialCalls = 0;
	int32 TypeD6 = 0;
};
void FFMCodexNetworkMatchRuntime::EnableSetPieceSelectionMilestone(int32 TypeD6)
{
	check(!bInitialized); check(TypeD6 >= 1 && TypeD6 <= 6);
	bSetPieceSelectionMilestone = true;
	EnablePlayerFacingPresentation();
	EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, 9, false, TypeD6));
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server DEV set-piece selection fixture: D12=9 TypeD6=%d; canonical entry and selection only."), TypeD6);
}
void FFMCodexNetworkMatchRuntime::EnableNearFreeKickMilestone(bool Goal)
{
	check(!bInitialized);
	EnableSetPieceSelectionMilestone(5);
	// Purpose-specific provider overrides only. Method, taker, Formula and terminal
	// still come from genuine shared player intents and the canonical lifecycle.
	PostRouteProvider->AutomationNearAttackD6 = Goal ? 6 : 1;
	PostRouteProvider->AutomationNearDefenseD6 = Goal ? 1 : 6;
	PostRouteProvider->AutomationNearPairedAD6 = Goal ? 6 : 2;
	PostRouteProvider->AutomationNearPairedBD6 = Goal ? 3 : 3;
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server DEV Near provider enabled: Direct=%d/%d Pair=%d/%d. All methods remain player-selected."),
		PostRouteProvider->AutomationNearAttackD6, PostRouteProvider->AutomationNearDefenseD6,
		PostRouteProvider->AutomationNearPairedAD6, PostRouteProvider->AutomationNearPairedBD6);
}
void FFMCodexNetworkMatchRuntime::EnableDeploymentAutomationEntry(int32 InitialD12)
{
	check(!bInitialized);
	check(InitialD12 == 4 || InitialD12 == 6);
	EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, InitialD12));
	UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation deployment fixture: initial D12=%d; other providers remain secure."), InitialD12);
}
#endif
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
void FFMCodexNetworkMatchRuntime::EnableSpecializedShotAutomation(bool Goal, bool ImmediateMiss, bool Final)
{
	check(!bInitialized);
	EnablePostRouteAutomation(ImmediateMiss ? 1 : Goal ? 6 : 3, Goal ? 1 : 6);
	PostRouteProvider->AutomationPairedAD6 = Goal ? 6 : 1;
	PostRouteProvider->AutomationPairedBD6 = Goal ? 5 : 1;
	EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, 4, Final));
}
void FFMCodexNetworkMatchRuntime::EnableThroughBallConditionalAutomation(const FString& Path, bool Goal, bool Final)
{
    check(!bInitialized);
    EnableInitialRouteAutomation(Path.StartsWith(TEXT("Behind")) ? 3 : 5);
    EnablePostRouteAutomation(Path == TEXT("BehindOutOfPlay") || Path == TEXT("AntiOffside") ? 1 : 6, 1);
    PostRouteProvider->AutomationOneOnOneAttackD6 = Goal ? 6 : 1;
    PostRouteProvider->AutomationOneOnOneDefenseD6 = Goal ? 1 : 6;
    EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, 6, Final));
}
void FFMCodexNetworkMatchRuntime::EnableOrdinaryTerminalAutomation(bool Goal, bool Final, ESkillRuleType Family, int32 RouteD6)
{
	check(!bInitialized);
	EnableInitialRouteAutomation(Family == ESkillRuleType::ThroughBall ? 1 : RouteD6);
	EnablePostRouteAutomation(Goal ? 6 : 1, Goal ? 1 : 6);
	EntryProvider->Inject(MakeUnique<FFMCodexDeploymentAutomationEntry>(*RollProvider, 6, Final));
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
FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch(bool bBranchFixture)
{
	auto Result = CreatePrototypeMatch();
	auto& Opening = Result.MatchConfiguration.OpeningInput.OpeningInput;
	// Equal rarity totals let the canonical tie-break rule decide the opening side.
	// Only this explicit server automation fixture changes these card inputs.
	for (auto& Card : Opening.PlayerADeck) { Card.Rarity = ECardRarity::Common; }
	for (auto& Card : Opening.PlayerBDeck) { Card.Rarity = ECardRarity::Common; }
	Opening.PlayerATieBreakerRoll = 6;
	Opening.PlayerBTieBreakerRoll = 2;
	if (bBranchFixture)
	{
		// Reorder two intact canonical cards before initialization so normal DEV deployment
		// reaches a Remote CutInside Carrier. No Skill, attributes or availability are changed.
		const int32 Rodri = Opening.PlayerBDeck.IndexOfByPredicate([](const auto& C) { return C.CardId == FName(TEXT("Prototype.ManchesterCity.Rodri")); });
		const int32 Doku = Opening.PlayerBDeck.IndexOfByPredicate([](const auto& C) { return C.CardId == FName(TEXT("Prototype.ManchesterCity.JeremyDoku")); });
		check(Rodri != INDEX_NONE && Doku != INDEX_NONE);
		Opening.PlayerBDeck.Swap(Rodri, Doku);
		UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("Server automation branch fixture: canonical Rodri/Doku deck order swapped; card rules unchanged."));
	}
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
	, InitialRouteProvider(MakeUnique<FFMCodexNetworkInitialRouteRollProvider>(*RollProvider))
	, PostRouteProvider(MakeUnique<FFMCodexNetworkPostRouteRollProvider>(*RollProvider))
	, RecoveryProvider(MakeUnique<FFMCodexNetworkRecoveryProvider>(*RollProvider))
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
		*InitialRouteProvider,
		*PostRouteProvider,
		*RecoveryProvider,
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
	Disclosure.bRevealSetPieceTypeRoll = Disclosure.bRevealInitialActionPointRoll
		&& DisclosedSetPieceTypeSequence == Snapshot.CurrentAttack.AttackSequence;
	Disclosure.bRevealRouteRoll = Disclosure.bRevealInitialActionPointRoll
		&& DisclosedRouteAttackSequence == Snapshot.CurrentAttack.AttackSequence;
	Disclosure.RevealedContestD6Count = Disclosure.bRevealRouteRoll
		&& DisclosedContestAttackSequence == Snapshot.CurrentAttack.AttackSequence
		? DisclosedContestRollCount : 0;
	if (Disclosure.bRevealSetPieceTypeRoll && Snapshot.CurrentAttack.SetPieceRoute.SelectedType == ESetPieceSelectedType::ShortFreeKick
		&& DisclosedNearAttackSequence == Snapshot.CurrentAttack.AttackSequence)
		Disclosure.RevealedContestD6Count = DisclosedNearRollCount;
	// Network DEV reveals a completed ordinary contest at the stable terminal publication, without Local Reel timing.
	// The independent exact-attack permission still distinguishes persistence from disclosure.
	Disclosure.bRevealTerminalOutcome = Snapshot.bHasCurrentAttack
		&& Snapshot.CurrentAttack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
		&& DisclosedTerminalAttackSequence == Snapshot.CurrentAttack.AttackSequence
		&& ((Disclosure.RevealedContestD6Count > 0
			&& Disclosure.RevealedContestD6Count == Snapshot.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords.Num())
			|| (Disclosure.bRevealSetPieceTypeRoll && DisclosedNearAttackSequence == Snapshot.CurrentAttack.AttackSequence
				&& DisclosedNearRollCount == 2 && Snapshot.CurrentAttack.SetPieceRoute.SelectedType == ESetPieceSelectedType::ShortFreeKick)
			|| (Disclosure.bRevealSetPieceTypeRoll && Snapshot.CurrentAttack.RouteKind == EMatchPlayCurrentAttackRouteKind::SetPiece
				&& (Snapshot.CurrentAttack.SetPieceRoute.ShortFreeKick.bNoLegalCarrier
					|| Snapshot.CurrentAttack.SetPieceRoute.LongFreeKick.bNoLegalCarrier || Snapshot.CurrentAttack.SetPieceRoute.Penalty.bNoLegalCarrier)));
	const FFMCodexLocalMatchInteractionView SafeViewerView =
		FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Snapshot,
			SkillRuleSet,
			ViewerSide,
			Disclosure);
	auto Result = FFMCodexNetworkClientViewSnapshotFactory::Build(
		SafeViewerView,
		MatchInstanceId,
		ViewRevision,
		ViewerSide,
		BootstrapState);
	if (bPlayerFacingPresentation)
	{
		// The narrow transport snapshot intentionally contains only the accepted prefix.
		// The display variant additionally retains safe unresolved ordinary operands.
		// Both are built from this exact immutable State snapshot and disclosure permission.
		Disclosure.bPreservePendingOrdinaryFormula = true;
		const auto DisplayView = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Snapshot, SkillRuleSet, ViewerSide, Disclosure);
		Result.Presentation = FFMCodexNetworkMatchPresentationAdapter::Project(DisplayView, ViewerSide);
		Result.Presentation.SetPiece.bSelectionSupported = bSetPieceSelectionMilestone
			|| Result.Presentation.SetPiece.bTypeWait || Result.Presentation.SetPiece.Type == ESetPieceSelectedType::ShortFreeKick;
	}
	return Result;
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
#if WITH_DEV_AUTOMATION_TESTS
	const int32 BeforeDeclineRng = GetEntryProviderInvocationCount() + GetD12ProviderInvocationCount()
		+ GetInitialRouteProviderInvocationCount() + GetPostRouteProviderInvocationCount() + GetRecoveryProviderInvocationCount();
#endif
	FMatchPlayEntryDeploymentPlayerIntentPort Port(*AuthoritativeSession, *ServerCoordinator);
	auto Result = Port.SubmitPlayerIntent(Intent);
#if WITH_DEV_AUTOMATION_TESTS
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineRunner
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineHelper
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineSkill
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineMarker)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto Safe = BuildClientView(State.RuntimeState.CurrentAttackingPlayer, 0, EFMCodexNetworkBootstrapState::MatchReady);
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Decline authority: Kind=%s Success=%d HasAttack=%d Sequence=%lld Attacker=%d ExpectedSide=%d Wait=%d SelectionStage=%d Runner=%s Helper=%s Skill=%s Ended=%d UsedA=%d UsedB=%d ScoreA=%d ScoreB=%d History=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d RngDelta=%d"),
			Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineRunner ? TEXT("DeclineRunner")
				: Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineHelper ? TEXT("DeclineHelper")
				: Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::DeclineMarker ? TEXT("DeclineMarker") : TEXT("DeclineSkill"),
			Result.bSuccess, State.bHasCurrentAttack, Safe.AttackSequence, static_cast<int32>(State.RuntimeState.CurrentAttackingPlayer),
			static_cast<int32>(Safe.ExpectedActingSide), static_cast<int32>(Safe.EntryWait), static_cast<int32>(State.CurrentAttack.SelectionStage),
			*State.CurrentAttack.ActionPreparation.RunnerCardId.ToString(), *State.CurrentAttack.ActionPreparation.HelperCardId.ToString(),
			*State.CurrentAttack.ActionPreparation.SkillId.ToString(), Safe.bMatchEnded,
			State.RuntimeState.PlayerAState.UsedAttackCount, State.RuntimeState.PlayerBState.UsedAttackCount,
			State.RuntimeState.PlayerAState.Score, State.RuntimeState.PlayerBState.Score, State.GoalHistory.Num(),
			GetCoordinatorInvocationCountForTests(), Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason),
			GetEntryProviderInvocationCount() + GetD12ProviderInvocationCount() + GetInitialRouteProviderInvocationCount()
			+ GetPostRouteProviderInvocationCount() + GetRecoveryProviderInvocationCount() - BeforeDeclineRng);
	}
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

	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Attack = State.CurrentAttack; const auto& Session = Attack.ResolutionSession;
		const auto& A = Attack.SelectedAction;
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = State.bHasCurrentAttack
			&& DisclosedInitialAttackSequence == Attack.AttackSequence;
		const auto Safe = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			State, SkillRuleSet, State.RuntimeState.CurrentAttackingPlayer, Disclosure);
		FString Steps;
		for (const auto& Step : Result.CoordinatorResult.Steps)
		{
			if (!Steps.IsEmpty()) { Steps += TEXT(","); }
			Steps += Step.CommandKind == EMatchPlayAuthoritativeCommandKind::BeginResolutionSession
				? TEXT("BeginResolutionSession")
				: Step.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute
					? TEXT("ResolveIntentDeterminedRoute") : FString::FromInt(static_cast<int32>(Step.CommandKind));
		}
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Branch authority: Success=%d Phase=%s SelectionStage=%s ExpectedSide=%d Skill=%s ActionType=%s Branch=%d SelectedAction=%d ResolutionSession=%d RouteStage=%s RouteResolved=%d ActualLongShot=%d ActualCutInside=%d ActualCross=%d RollProgress=%d Terminal=%d Interaction=%d CoordinatorCalls=%d InternalSteps=%d Steps=%s Stop=%d"),
			Result.bSuccess, *StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(Attack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(Attack.SelectionStage)),
			static_cast<int32>(Safe.ExpectedActingPlayer), *(Attack.bHasSelectedAction ? A.SkillId : Attack.ActionPreparation.SkillId).ToString(),
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(Attack.bHasSelectedAction ? A.ActionType : Attack.ActionPreparation.ActionType)),
			static_cast<int32>(Safe.ElectiveBranchIntent), Attack.bHasSelectedAction, Attack.bHasResolutionSession,
			*StaticEnum<EMatchPlayCurrentAttackResolutionStage>()->GetNameStringByValue(static_cast<int64>(Session.Stage)),
			Session.bHasActualBranch, static_cast<int32>(Session.ActualBranch.LongShot),
			static_cast<int32>(Session.ActualBranch.CutInsideShot), static_cast<int32>(Session.ActualBranch.Cross),
			static_cast<int32>(Session.PostRouteRollProgress.Phase), Safe.bTerminalPendingAdvance,
			static_cast<int32>(Safe.InteractionCategory), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), Steps.IsEmpty() ? TEXT("None") : *Steps,
			static_cast<int32>(Result.CoordinatorResult.StopReason));
	}

#endif
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent && Result.bSuccess)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Attack = State.CurrentAttack;
		if (State.bHasCurrentAttack && Attack.bHasResolutionSession && Attack.ResolutionSession.bHasActualBranch
			&& (Attack.ResolutionSession.ActualBranch.ActionType == ESkillRuleType::LongShot
				|| Attack.ResolutionSession.ActualBranch.ActionType == ESkillRuleType::CutInsideShot))
			DisclosedRouteAttackSequence = Attack.AttackSequence; // Public intent-determined route, without a route die.
	}
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal && Result.bSuccess)
	{
		DisclosedInitialAttackSequence = 0;
		DisclosedSetPieceTypeSequence = 0;
		DisclosedNearAttackSequence = 0;
		DisclosedNearRollCount = 0;
		DisclosedRouteAttackSequence = 0;
		DisclosedContestAttackSequence = 0;
		DisclosedContestRollCount = 0;
		DisclosedTerminalAttackSequence = 0;
	}
#if WITH_DEV_AUTOMATION_TESTS
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto View = BuildClientView(EInitialTurnOrderPlayer::PlayerA, 0, EFMCodexNetworkBootstrapState::MatchReady);
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Advance authority: Success=%d HasCurrentAttack=%d NextSequence=%lld NextAttacker=%d ExpectedSide=%d Ended=%d MatchResult=%d UsedA=%d UsedB=%d ScoreA=%d ScoreB=%d PublicScoreA=%d PublicScoreB=%d GoalHistory=%d PublicHistory=%d RecoverySource=%lld RecoveryCards=%d EntryCalls=%d D12Calls=%d RouteCalls=%d PostCalls=%d RecoveryCalls=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d"),
			Result.bSuccess, State.bHasCurrentAttack, View.AttackSequence, static_cast<int32>(View.CurrentAttackingSide),
			static_cast<int32>(View.ExpectedActingSide), View.bMatchEnded, static_cast<int32>(View.MatchResult),
			State.RuntimeState.PlayerAState.UsedAttackCount, State.RuntimeState.PlayerBState.UsedAttackCount,
			State.RuntimeState.PlayerAState.Score, State.RuntimeState.PlayerBState.Score, View.PlayerAScore, View.PlayerBScore,
			State.GoalHistory.Num(), View.PublicGoalHistory.Num(), View.Recovery.SourceAttackSequence, View.Recovery.Cards.Num(),
			GetEntryProviderInvocationCount(), GetD12ProviderInvocationCount(), GetInitialRouteProviderInvocationCount(),
			GetPostRouteProviderInvocationCount(), GetRecoveryProviderInvocationCount(), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason));
	}
#endif
	const bool bNearRoll = Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickAngledRoll;
	if (bNearRoll && Result.bSuccess)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Near = State.CurrentAttack.SetPieceRoute.ShortFreeKick;
		DisclosedNearAttackSequence = State.CurrentAttack.AttackSequence;
		DisclosedNearRollCount = Near.bHasAngledD6Pair ? 2 : int32(Near.bHasAttackD6) + int32(Near.bHasDefenseD6);
		if (State.CurrentAttack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
			DisclosedTerminalAttackSequence = DisclosedNearAttackSequence;
	}
	const bool bContestAttack = Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCrossLowAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolvePassControlAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1AttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallAntiOffsideAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneChipShotAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveLongShotDeadCornerRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectAttackRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDeadCornerRoll;
	const bool bContestDefense = Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCrossHighDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCrossLowDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolvePassControlDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1DefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectDefenseRoll;
	if ((bContestAttack || bContestDefense) && Result.bSuccess)
	{
		DisclosedContestAttackSequence = AuthoritativeSession->GetStateSnapshot().CurrentAttack.AttackSequence;
		DisclosedContestRollCount = AuthoritativeSession->GetStateSnapshot().CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords.Num();
		if (AuthoritativeSession->GetStateSnapshot().CurrentAttack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
		{
			DisclosedTerminalAttackSequence = DisclosedContestAttackSequence;
		}
	}
#if WITH_DEV_AUTOMATION_TESTS
	if (bContestAttack || bContestDefense)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Attack = State.CurrentAttack;
		const auto Safe = BuildClientView(State.RuntimeState.CurrentAttackingPlayer, 0, EFMCodexNetworkBootstrapState::MatchReady);
		for (const auto& Roll : Safe.AcceptedContestRolls)
			UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("DEV Accepted contest: Sequence=%lld ThroughBall=%d Index=%d Purpose=%d D6=%d Side=%d Terminal=%d"),
				Attack.AttackSequence, int32(Safe.InitialRoute.ThroughBall), Roll.SequenceIndex, int32(Roll.Purpose), Roll.D6, int32(Roll.OwnerSide), int32(Safe.Terminal.Outcome));
		if (bContestDefense && Result.bSuccess && bPlayerFacingPresentation)
		{
			// Diagnostics may read only the prefix and terminal facts already permitted in this public snapshot.
			FFMCodexLocalMatchViewerDisclosure Published;
			Published.bRevealInitialActionPointRoll = Safe.DisclosedInitialD12 > 0;
			Published.bRevealRouteRoll = DisclosedRouteAttackSequence == Attack.AttackSequence;
			Published.RevealedContestD6Count = Safe.AcceptedContestRolls.Num();
			Published.bRevealTerminalOutcome = Safe.Terminal.Outcome != EFMCodexNetworkTerminalOutcome::None;
			const auto Facts = FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State, SkillRuleSet,
				State.RuntimeState.CurrentAttackingPlayer, Published).ResolutionFacts;
			for (const auto& Contest : Facts.FormulaContests) if (Contest.bHasResolvedFormula)
				UE_LOG(LogFMCodexNetworkPlay, Log, TEXT("DEV Ordinary formula authority: Sequence=%lld Contest=%s ActualPassControl=%d ActualThroughBall=%d AttackTotal=%.2f DefenseTotal=%.2f Winner=%d Goal=%d Runner=%s"),
					Attack.AttackSequence, *Contest.ContestId.ToString(), static_cast<int32>(Safe.InitialRoute.PassControl), static_cast<int32>(Safe.InitialRoute.ThroughBall),
					Contest.ResolvedResult.AttackerFinalValue, Contest.ResolvedResult.DefenderFinalValue, static_cast<int32>(Contest.ResolvedResult.Winner),
					Contest.ResolvedResult.bIsGoal, *Attack.SelectedAction.RunnerCardId.ToString());
		}
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV Contest authority: Success=%d Command=%d Skill=%s Branch=%d ActualCross=%d AttackD6=%d DefenseD6=%d FormulaComplete=%d ProviderCalls=%d Phase=%s SelectionStage=%s RouteStage=%s RollProgress=%d RollRecords=%d Terminal=%d AuthorityScoreA=%d AuthorityScoreB=%d PublicScoreA=%d PublicScoreB=%d GoalHistory=%d ExpectedSide=%d Wait=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d"),
			Result.bSuccess, static_cast<int32>(Intent.CommandKind), *Attack.SelectedAction.SkillId.ToString(),
			static_cast<int32>(Attack.SelectedAction.ElectiveBranchIntent), static_cast<int32>(Safe.InitialRoute.Cross),
			Safe.Contest.AttackD6, Safe.Contest.DefenseD6, Safe.Contest.bFormulaResolved, GetPostRouteProviderInvocationCount(),
			*StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(Attack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(Attack.SelectionStage)),
			*StaticEnum<EMatchPlayCurrentAttackResolutionStage>()->GetNameStringByValue(static_cast<int64>(Attack.ResolutionSession.Stage)),
			static_cast<int32>(Attack.ResolutionSession.PostRouteRollProgress.Phase), Attack.ResolutionSession.PostRouteRollProgress.RollRecords.Num(),
			Attack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance,
			State.RuntimeState.PlayerAState.Score, State.RuntimeState.PlayerBState.Score, Safe.PlayerAScore, Safe.PlayerBScore, State.GoalHistory.Num(),
			static_cast<int32>(Safe.ExpectedActingSide), static_cast<int32>(Safe.EntryWait), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason));
	}
#endif
	const bool bInitialRoute = Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll
		|| Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll;
	if (bInitialRoute && Result.bSuccess)
	{
		DisclosedRouteAttackSequence = AuthoritativeSession->GetStateSnapshot().CurrentAttack.AttackSequence;
	}
#if WITH_DEV_AUTOMATION_TESTS
	if (bInitialRoute)
	{
		const auto State = AuthoritativeSession->GetStateSnapshot();
		const auto& Attack = State.CurrentAttack;
		const auto Safe = BuildClientView(State.RuntimeState.CurrentAttackingPlayer, 0, EFMCodexNetworkBootstrapState::MatchReady);
		UE_LOG(LogFMCodexNetworkPlay, Log,
			TEXT("DEV InitialRoute authority: Success=%d Skill=%s Branch=%d D6=%d Family=%d Cross=%d PassControl=%d ThroughBall=%d ProviderCalls=%d Phase=%s SelectionStage=%s ResolutionSession=%d RouteStage=%s RollProgress=%d Terminal=%d ExpectedSide=%d Wait=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d"),
			Result.bSuccess, *Attack.SelectedAction.SkillId.ToString(), static_cast<int32>(Attack.SelectedAction.ElectiveBranchIntent),
			Safe.InitialRoute.D6, static_cast<int32>(Safe.InitialRoute.ActionType),
			static_cast<int32>(Safe.InitialRoute.Cross), static_cast<int32>(Safe.InitialRoute.PassControl), static_cast<int32>(Safe.InitialRoute.ThroughBall),
			GetInitialRouteProviderInvocationCount(),
			*StaticEnum<EMatchPlayCurrentAttackPhase>()->GetNameStringByValue(static_cast<int64>(Attack.Phase)),
			*StaticEnum<EMatchPlayCurrentAttackSelectionStage>()->GetNameStringByValue(static_cast<int64>(Attack.SelectionStage)),
			Attack.bHasResolutionSession,
			*StaticEnum<EMatchPlayCurrentAttackResolutionStage>()->GetNameStringByValue(static_cast<int64>(Attack.ResolutionSession.Stage)),
			static_cast<int32>(Attack.ResolutionSession.PostRouteRollProgress.Phase),
			Attack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance,
			static_cast<int32>(Safe.ExpectedActingSide), static_cast<int32>(Safe.EntryWait), GetCoordinatorInvocationCountForTests(),
			Result.CoordinatorResult.Steps.Num(), static_cast<int32>(Result.CoordinatorResult.StopReason));
	}
#endif
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll
		&& Result.AuthoritativeResult.RuntimeEnvelope.bDomainSuccess)
	{
		DisclosedSetPieceTypeSequence = Result.AuthoritativeResult.RuntimeEnvelope.AttackSequence;
		const auto State = AuthoritativeSession->GetStateSnapshot();
		if (Result.bSuccess && State.bHasCurrentAttack && State.CurrentAttack.LifecycleState == EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance
			&& (State.CurrentAttack.SetPieceRoute.ShortFreeKick.bNoLegalCarrier || State.CurrentAttack.SetPieceRoute.LongFreeKick.bNoLegalCarrier
				|| State.CurrentAttack.SetPieceRoute.Penalty.bNoLegalCarrier)) DisclosedTerminalAttackSequence = DisclosedSetPieceTypeSequence;
	}
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
 if (bNearRoll || Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll
  || Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier
  || Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod
  || Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod
  || Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod)
 {
  const auto State = AuthoritativeSession->GetStateSnapshot();
  const auto V = BuildClientView(State.RuntimeState.CurrentAttackingPlayer,0,EFMCodexNetworkBootstrapState::MatchReady);
  if (bNearRoll)
  {
   for (const auto& R : V.AcceptedContestRolls)
    UE_LOG(LogFMCodexNetworkPlay,Log,TEXT("DEV Near accepted: Sequence=%lld Purpose=%d Index=%d D6=%d Owner=%d"),V.AttackSequence,int32(R.Purpose),R.SequenceIndex,R.D6,int32(R.OwnerSide));
   UE_LOG(LogFMCodexNetworkPlay,Log,TEXT("DEV Near disclosure: Command=%d Success=%d Rolls=%d Formula=%d Terminal=%d Scorer=%s ScoreA=%d ScoreB=%d Goals=%d"),
    int32(Intent.CommandKind),Result.bSuccess,V.AcceptedContestRolls.Num(),V.Contest.bFormulaResolved,int32(V.Terminal.Outcome),*V.Terminal.Goal.ScorerCardId.ToString(),V.PlayerAScore,V.PlayerBScore,V.PublicGoalHistory.Num());
  }
  UE_LOG(LogFMCodexNetworkPlay,Log,TEXT("DEV SetPiece authority: Success=%d Sequence=%lld TypeD6=%d Type=%d CarrierStage=%d CornerStage=%d Actor=%d Wait=%d EntryCalls=%d D12Calls=%d RouteCalls=%d PostCalls=%d RecoveryCalls=%d CoordinatorCalls=%d InternalSteps=%d Stop=%d ScoreA=%d ScoreB=%d Goals=%d"),
   Result.bSuccess,V.AttackSequence,V.SetPiece.TypeD6,int32(V.SetPiece.Type),int32(V.SetPiece.CarrierStage),int32(V.SetPiece.CornerStage),int32(V.ExpectedActingSide),int32(V.EntryWait),
   GetEntryProviderInvocationCount(),GetD12ProviderInvocationCount(),GetInitialRouteProviderInvocationCount(),GetPostRouteProviderInvocationCount(),GetRecoveryProviderInvocationCount(),
   GetCoordinatorInvocationCountForTests(),Result.CoordinatorResult.Steps.Num(),int32(Result.CoordinatorResult.StopReason),V.PlayerAScore,V.PlayerBScore,V.PublicGoalHistory.Num());
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
