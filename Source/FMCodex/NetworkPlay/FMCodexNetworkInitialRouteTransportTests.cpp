#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "FMCodexNetworkMatchGameMode.h"
#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"
#include "FMCodexNetworkRNGTestEntropy.h"
#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../MatchPlayRuntime/MatchPlayServerCoordinator.h"
#include "../MatchPlayRuntime/MatchPlayEntryDeploymentPlayerIntentPort.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"
#include "../LocalPlay/FMCodexPlayerUIPresentationText.h"
#include "../LocalPlay/FMCodexPrototypeTeamContent.h"
#include "../CoreRules/PlayerCardRuleSnapshotValidator.h"
#include "../CoreRules/MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

class FInitialRouteEntropy final : public IFMCodexNetworkEntropySource
{
public:
	uint32 Word = 5;
	int32 Calls = 0;
	bool bFail = false;
	virtual bool Fill(TArrayView<uint8> Bytes) override
	{
		++Calls;
		if (bFail || Bytes.Num() != sizeof(Word)) { return false; }
		FMemory::Memcpy(Bytes.GetData(), &Word, sizeof(Word)); return true;
	}
};

struct FFMCodexNetworkInitialRouteTestAccess
{
	static FInitialRouteEntropy* Configure(AFMCodexNetworkMatchGameMode& Mode,
		AFMCodexNetworkMatchPlayerController* A, AFMCodexNetworkMatchPlayerController* B, bool BFirst, int32 RawD12)
	{
		Mode.MatchInstanceId = FGuid::NewGuid();
		Mode.BootstrapConfiguration = BFirst
			? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		Mode.ParticipantRegistry.Admit(A, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		Mode.ParticipantRegistry.Admit(B, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		auto Entropy = MakeUnique<FInitialRouteEntropy>();
		auto* Source = Entropy.Get();
		Mode.MatchRuntime = MakeUnique<FFMCodexNetworkMatchRuntime>(Mode.MatchInstanceId, MoveTemp(Entropy));
		if (!Mode.MatchRuntime->InitializeOnce(Mode.BootstrapConfiguration).bSuccess) { return nullptr; }
		Publish(Mode);
		return Source;
	}
	static FMatchPlayAuthoritativeSession& Session(AFMCodexNetworkMatchGameMode& Mode) { return *Mode.MatchRuntime->AuthoritativeSession; }
	static FFMCodexNetworkMatchRuntime& Runtime(AFMCodexNetworkMatchGameMode& Mode) { return *Mode.MatchRuntime; }
	static void Publish(AFMCodexNetworkMatchGameMode& Mode) { Mode.PublishOwnerViews(EFMCodexNetworkBootstrapState::MatchReady); }
	static int32 Revision(const AFMCodexNetworkMatchGameMode& Mode) { return Mode.ViewRevision; }
	static FMatchPlayServerCoordinatorResult Advance(AFMCodexNetworkMatchGameMode& Mode)
	{
		return Mode.MatchRuntime->ServerCoordinator->AdvanceToStableState();
	}

	static FSkillRuleSnapshotSet& CallerRules(AFMCodexNetworkMatchGameMode& Mode) { return Mode.MatchRuntime->SkillRuleSet; }
	static FFMCodexLocalMatchInteractionView Safe(AFMCodexNetworkMatchGameMode& Mode, EInitialTurnOrderPlayer Side, bool Reveal = true, bool RouteReveal = false)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = Reveal;
		Disclosure.bRevealRouteRoll = RouteReveal;
		return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Session(Mode).GetStateSnapshot(), Mode.MatchRuntime->SkillRuleSet, Side, Disclosure);
	}
};

namespace FMCodexNetworkInitialRouteTests
{
	using Access = FFMCodexNetworkInitialRouteTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	using Envelope = FFMCodexNetworkPlayerIntentEnvelope;
	using MarkerPayload = FFMCodexNetworkSubmitMarkerPayload;
	using HelperPayload = FFMCodexNetworkSubmitHelperPayload;
	using SkillPayload = FFMCodexNetworkSubmitSkillPayload;
	using Payload = FFMCodexNetworkSubmitBranchIntentPayload;
	using Branch = EMatchPlayElectiveBranchIntent;
	using CarrierPayload = FFMCodexNetworkSubmitCarrierPayload;
	struct FFixture
	{
		UWorld* World;
		AFMCodexNetworkMatchGameMode* Mode;
		AFMCodexNetworkMatchPlayerController* A;
		AFMCodexNetworkMatchPlayerController* B;
		FInitialRouteEntropy* Entropy;
		FFMCodexNetworkIntentClientState ClientA, ClientB;
		int64 NextA = 1, NextB = 1;
		Kind RequestedKind = Kind::CrossInitialRouteRoll;
		explicit FFixture(bool BFirst = false, int32 RawD12 = 6)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Mode = World->SpawnActor<AFMCodexNetworkMatchGameMode>();
			A = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			B = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			World->AddController(A); World->AddController(B);
			Entropy = Access::Configure(*Mode, A, B, BFirst, RawD12);
		}
		~FFixture() { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		AFMCodexNetworkMatchPlayerController* Attacker() const { return A->GetOwnerView().CurrentAttackingSide == Side::PlayerA ? A : B; }
		AFMCodexNetworkMatchPlayerController* Defender() const { return Attacker() == A ? B : A; }
		FFMCodexNetworkIntentClientState& Client(AFMCodexNetworkMatchPlayerController* PC) { return PC == A ? ClientA : ClientB; }
		int64& Next(AFMCodexNetworkMatchPlayerController* PC) { return PC == A ? NextA : NextB; }
		int32 Calls() const { return Access::Runtime(*Mode).GetCoordinatorInvocationCountForTests(); }

		Envelope Request(AFMCodexNetworkMatchPlayerController* PC)
		{
			Envelope E; E.MatchInstanceId = Mode->GetMatchInstanceId(); E.RequestId = Next(PC)++;
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = RequestedKind;
			return E;
		}
		bool Send(AFMCodexNetworkMatchPlayerController* PC, Kind K,
			const FFMCodexNetworkDeployOrdinaryPayload& Ordinary = {}, const CarrierPayload& Carrier = {}, const MarkerPayload& Marker = {}, const FFMCodexNetworkSubmitRunnerPayload& Runner = {}, const HelperPayload& Helper = {}, const SkillPayload& Skill = {}, const Payload& BranchChoice = {})
		{
			auto& C = Client(PC); Envelope E; const auto& V = PC->GetOwnerView();
			FFMCodexNetworkDeployGoalkeeperPayload GK; GK.SlotId = Ordinary.SlotId;
			bool Began = K == Kind::RequestInitialActionPointRoll ? C.Begin(V, E)
				: K == Kind::DeployOrdinary ? C.BeginDeployment(V, Ordinary, E)
				: K == Kind::DeployGoalkeeper ? C.BeginGoalkeeper(V, GK, E)
				: K == Kind::FinishDeployment ? C.BeginFinishDeployment(V, E)
				: K == Kind::SubmitCarrier ? C.BeginCarrier(V, Carrier, E)
				: K == Kind::SubmitMarker ? C.BeginMarker(V, Marker, E)
				: K == Kind::SubmitRunner ? C.BeginRunner(V, Runner, E)
				: K == Kind::SubmitHelper ? C.BeginHelper(V, Helper, E)
				: K == Kind::SubmitSkill ? C.BeginSkill(V, Skill, E)
				: K == Kind::SubmitBranchIntent ? C.BeginBranch(V, BranchChoice, E) : C.BeginInitialRoute(V, K, E);
			if (!Began) { return false; }
			Next(PC) = E.RequestId + 1;
			const auto Ack = Mode->SubmitConnectionPlayerIntent(PC, E);
			C.ObserveView(PC->GetOwnerView()); C.ObserveAck(Ack);
			return Ack.Code == Code::Accepted && !C.IsPending();
		}
		FFMCodexNetworkDeployOrdinaryPayload Choice(AFMCodexNetworkMatchPlayerController* PC, const FString& Half = {}, FName CardId = NAME_None)
		{
			const auto Safe = Access::Safe(*Mode, PC->GetOwnerView().ViewerSide);
			for (const auto& O : Safe.DeploymentOptions)
			{
				// Known test catalog geometry only; every selected pair comes from canonical safe availability.
				if (!O.bGoalkeeper && (CardId.IsNone() || O.CardId == CardId) && (Half.IsEmpty() || O.SlotId.ToString().Contains(Half)))
				{
					FFMCodexNetworkDeployOrdinaryPayload P; P.CardId = O.CardId; P.SlotId = O.SlotId; return P;
				}
			}
			return {};
		}

		// Canonical safe deployment choices only; alternate skill-less Carrier proves automatic continuation.
		bool ReachHelper(int32 Count = 2, bool WithSkill = true, bool ExtraDefenders = true, bool SubmitRunner = true, FName CarrierOverride = NAME_None, FName RunnerOverride = NAME_None)
		{
			auto* Attack = Attacker(); auto* Defense = Defender();
			const bool IsA = Attack == A;
			if (!Entropy || !Send(Attack, Kind::RequestInitialActionPointRoll)) { return false; }
			const FName CarrierId = !CarrierOverride.IsNone() ? CarrierOverride : WithSkill
				? FName(IsA ? TEXT("Prototype.Arsenal.MartinOdegaard") : TEXT("Prototype.ManchesterCity.Rodri"))
				: NAME_None;
			const auto Carrier = Choice(Attack, IsA ? TEXT("NearA") : TEXT("NearB"), CarrierId);
			const FString Half = Carrier.SlotId.ToString().Contains(TEXT("NearA")) ? TEXT("NearA") : TEXT("NearB");
			const FString Other = Half == TEXT("NearA") ? TEXT("NearB") : TEXT("NearA");
			const FName RunnerId = !RunnerOverride.IsNone() ? RunnerOverride : FName(IsA ? TEXT("Prototype.Arsenal.MylesLewisSkelly") : TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			const auto Runner = [&]()
			{
				if (!Send(Attack, Kind::DeployOrdinary, Carrier)
					|| !Send(Defense, Kind::DeployOrdinary, Choice(Defense, Half))) { return FFMCodexNetworkDeployOrdinaryPayload{}; }
				return Choice(Attack, Other, RunnerId);
			}();
			if (Runner.IsEmpty() || !Send(Attack, Kind::DeployOrdinary, Runner)) { return false; }
			if (Count > 0 && !Send(Defense, Kind::DeployOrdinary, Choice(Defense, Other))) { return false; }
			if (Count == 0 && !Send(Defense, Kind::FinishDeployment)) { return false; }
			if (!Send(Attack, Kind::FinishDeployment)) { return false; }
			for (int32 I = 1; I < Count; ++I)
			{ if (!Send(Defense, Kind::DeployOrdinary, Choice(Defense, Other))) { return false; } }
			if (Count > 0 && ExtraDefenders)
			{
				if (!Send(Defense, Kind::DeployOrdinary, Choice(Defense, Half))) { return false; }
				FFMCodexNetworkDeployOrdinaryPayload GK;
				GK.SlotId = Defense->GetOwnerView().GoalkeeperOption.Choice.SlotId;
				if (!Send(Defense, Kind::DeployGoalkeeper, GK)) { return false; }
			}
			if (Count > 0 && !Send(Defense, Kind::FinishDeployment)) { return false; }
			CarrierPayload C; C.CarrierCardId = Carrier.CardId;
			if (!Send(Attack, Kind::SubmitCarrier, {}, C)) { return false; }
			if (Defense->GetOwnerView().MarkerOptions.IsEmpty()) { return false; }
			const auto M = Defense->GetOwnerView().MarkerOptions[0].Choice;
			if (!Send(Defense, Kind::SubmitMarker, {}, {}, M)) { return false; }
			if (!SubmitRunner) { return true; }
			FFMCodexNetworkSubmitRunnerPayload R; R.RunnerCardId = Runner.CardId;
			if (!Send(Attack, Kind::SubmitRunner, {}, {}, {}, R)) { return false; }
			return Count == 0 || (Defense->GetOwnerView().EntryWait == EFMCodexNetworkEntryWait::HelperSelection
				&& Defense->GetOwnerView().HelperOptions.Num() == Count);
		}

		bool ReachSkill(FName Carrier = NAME_None, FName Runner = NAME_None)
		{
			if (!ReachHelper(2, true, false, true, Carrier, Runner)) { return false; }
			const auto H = Defender()->GetOwnerView().HelperOptions[0].Choice;
			return Send(Defender(), Kind::SubmitHelper, {}, {}, {}, {}, H)
				&& Attacker()->GetOwnerView().EntryWait == EFMCodexNetworkEntryWait::SkillSelection;
		}

		bool ReachBranch(ESkillRuleType Type = ESkillRuleType::LongShot)
		{
			const bool IsA = Attacker() == A;
			const bool Shot = Type == ESkillRuleType::LongShot;
			const bool Route = Type == ESkillRuleType::PassControl || Type == ESkillRuleType::ThroughBall;
			const FName Carrier = FName(IsA
				? (Shot || Route ? TEXT("Prototype.Arsenal.MartinOdegaard") : TEXT("Prototype.Arsenal.BukayoSaka"))
				: (Shot ? TEXT("Prototype.ManchesterCity.PhilFoden") : Route ? TEXT("Prototype.ManchesterCity.Rodri") : TEXT("Prototype.ManchesterCity.JeremyDoku")));
			const FName Runner = Type == ESkillRuleType::Cross ? FName(IsA
				? TEXT("Prototype.Arsenal.KaiHavertz") : TEXT("Prototype.ManchesterCity.ErlingHaaland")) : NAME_None;
			if (!ReachSkill(Carrier, Runner)) { return false; }
			const auto Safe = Access::Safe(*Mode, Attacker()->GetOwnerView().ViewerSide);
			const auto* Option = Safe.SelectionOptions.FindByPredicate([&](const auto& O) { return O.SkillType == Type; });
			if (!Option) { return false; }
			SkillPayload P; P.SkillId = Option->Id;
			return Send(Attacker(), Kind::SubmitSkill, {}, {}, {}, {}, {}, P);
		}

		bool ReachRoute(ESkillRuleType Type, int32 D6 = 1, bool High = true, bool BeforeBranch = false)
		{
			RequestedKind = Type == ESkillRuleType::Cross ? Kind::CrossInitialRouteRoll
				: Type == ESkillRuleType::PassControl ? Kind::PassControlInitialRouteRoll : Kind::ThroughBallInitialRouteRoll;
			if (!ReachBranch(Type)) { return false; }
			if (Type == ESkillRuleType::Cross && !BeforeBranch)
			{
				Payload BranchChoice; BranchChoice.Intent = High ? Branch::CrossHigh : Branch::CrossLow;
				if (!Send(Attacker(), Kind::SubmitBranchIntent, {}, {}, {}, {}, {}, {}, BranchChoice)) { return false; }
			}
			Entropy->Word = D6 - 1; return true;
		}


	};
	bool SameState(const FMatchPlayState& A, const FMatchPlayState& B)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(&A, &B, 0);
	}
	struct FUnchanged
	{
		FMatchPlayState State;
		int32 Revision, EntropyCalls, EntryCalls, D12Calls, CoordinatorCalls, RouteCalls;
		explicit FUnchanged(FFixture& F)
			: State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			EntropyCalls(F.Entropy->Calls), EntryCalls(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()),
			D12Calls(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), CoordinatorCalls(F.Calls()), RouteCalls(Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount()) {}
		void Verify(FAutomationTestBase& Test, FFixture& F, bool ProviderFailure = false) const
		{
			Test.TestTrue(TEXT("Entire authoritative State unchanged"), SameState(State, Access::Session(*F.Mode).GetStateSnapshot()));
			Test.TestEqual(TEXT("No publication"), Access::Revision(*F.Mode), Revision);
			Test.TestEqual(TEXT("All-provider entropy unchanged"), F.Entropy->Calls, EntropyCalls + int32(ProviderFailure));
			Test.TestEqual(TEXT("Route provider boundary"), Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(), RouteCalls + int32(ProviderFailure));
			Test.TestEqual(TEXT("Entry calls unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), EntryCalls);
			Test.TestEqual(TEXT("D12 calls unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12Calls);
			Test.TestEqual(TEXT("Coordinator not invoked on rejection"), F.Calls(), CoordinatorCalls);
		}
	};
}



namespace FMCodexNetworkInitialRouteTests
{
	ESkillRuleType Family(const FString& P)
	{
		return P.Contains(TEXT("Cross")) ? ESkillRuleType::Cross
			: P.Contains(TEXT("PassControl")) ? ESkillRuleType::PassControl : ESkillRuleType::ThroughBall;
	}
	FMatchPlayPlayerIntent Canonical(ESkillRuleType Type, int64 Seq, Side Player)
	{
		if (Type == ESkillRuleType::Cross)
		{
			FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest R; R.AttackSequence=Seq; R.RequestingSide=Player;
			return FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll,R);
		}
		if (Type == ESkillRuleType::PassControl)
		{
			FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest R; R.AttackSequence=Seq; R.RequestingSide=Player;
			return FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll,R);
		}
		FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest R; R.AttackSequence=Seq; R.RequestingSide=Player;
		return FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll,R);
	}
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteOutcomes,
	"FMCodex.NetworkPlay.InitialRouteTransport.01.AllD6BothActors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteOutcomes::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* A : {TEXT("A"),TEXT("B")})
		for (const TCHAR* F : {TEXT("CrossHigh"),TEXT("CrossLow"),TEXT("PassControl"),TEXT("ThroughBall")})
			for (int32 D=1;D<=6;++D) { const auto N=FString::Printf(TEXT("%s.%s.%d"),A,F,D);Names.Add(N);Commands.Add(N); }
}
bool FFMCodexRouteOutcomes::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;
	const auto Type=Family(P);const int32 D=FCString::Atoi(*P.Right(1));const bool High=!P.Contains(TEXT("CrossLow"));
	FFixture F(P.StartsWith(TEXT("B"))),Local(P.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Both canonical route waits"),F.ReachRoute(Type,D,High) && Local.ReachRoute(Type,D,High))) { return false; }
	const auto BeforeView=F.Attacker()->GetOwnerView();const FUnchanged Before(F);
	TestEqual(TEXT("No result before roll"),BeforeView.InitialRoute.D6,0);
	TestTrue(TEXT("Only actor offered typed action"),BeforeView.InitialRouteAction!=EFMCodexNetworkInitialRouteAction::None
		&& F.Defender()->GetOwnerView().InitialRouteAction==EFMCodexNetworkInitialRouteAction::None);
	const auto E=F.Request(F.Attacker());const auto Request=Canonical(Type,E.ExpectedAttackSequence,BeforeView.ViewerSide);
	TestEqual(TEXT("Canonical structural origin is PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(Request.CommandKind),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
	const auto L=Access::Runtime(*Local.Mode).SubmitPlayerIntent(Request);
	TestTrue(TEXT("Shared local port succeeds"),L.bSuccess);
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E);
	if (!TestEqual(TEXT("Generated adapter accepts empty roll request"),Ack.Code,Code::Accepted)) { return false; }
	const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& A=State.CurrentAttack;const auto& R=A.ResolutionSession;
	TestTrue(TEXT("Complete Local/Network authority identical"),SameState(State,Access::Session(*Local.Mode).GetStateSnapshot()));
	TestEqual(TEXT("Exactly one entropy request"),F.Entropy->Calls,Before.EntropyCalls+1);
	TestEqual(TEXT("Exactly one route-provider request"),Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(),Before.RouteCalls+1);
	TestEqual(TEXT("No entry RNG"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),Before.EntryCalls);
	TestEqual(TEXT("No extra D12"),Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(),Before.D12Calls);
	TestEqual(TEXT("Coordinator exactly once"),F.Calls(),Before.CoordinatorCalls+1);
	TestEqual(TEXT("One stable publication"),Ack.ViewRevision,Before.Revision+1);
	TestEqual(TEXT("No Coordinator internal continuation"),L.CoordinatorResult.Steps.Num(),0);
	TestEqual(TEXT("Stop before next PlayerIntent"),L.CoordinatorResult.StopReason,EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Phase"),A.Phase,EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Selection stage"),A.SelectionStage,EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestTrue(TEXT("Resolution session exists"),A.bHasResolutionSession);
	TestEqual(TEXT("Route resolved"),R.Stage,EMatchPlayCurrentAttackResolutionStage::RouteResolved);
	if (!TestEqual(TEXT("Exactly one committed route record"),R.InitialRouteRollRecords.Num(),1)) { return false; }
	TestEqual(TEXT("Exact server D6"),R.InitialRouteRollRecords[0].RawD6,D);
	TestTrue(TEXT("No later player roll"),R.PostRouteRollProgress.RollRecords.IsEmpty());
	TestEqual(TEXT("No player-roll phase entered"),R.PostRouteRollProgress.Phase,EMatchPlayCurrentAttackPostRouteRollPhase::None);
	TestEqual(TEXT("Frozen Skill"),A.SelectedAction.SkillId,Before.State.CurrentAttack.SelectedAction.SkillId);
	TestEqual(TEXT("Frozen branch"),A.SelectedAction.ElectiveBranchIntent,Before.State.CurrentAttack.SelectedAction.ElectiveBranchIntent);
	const int32 Outcome=Type==ESkillRuleType::Cross ? ((D<=4)==High ? 1:2) : (D+1)/2;
	const auto Wait=Type==ESkillRuleType::Cross ? EFMCodexNetworkEntryWait::CrossAttackRoll
		: Type==ESkillRuleType::PassControl ? EFMCodexNetworkEntryWait::PassControlAttackRoll
		: D<=2 ? EFMCodexNetworkEntryWait::ThroughBallFeetAttackRoll
		: D<=4 ? EFMCodexNetworkEntryWait::ThroughBallBehindDefenseAttackRoll : EFMCodexNetworkEntryWait::ThroughBallAntiOffsideAttackRoll;
	const int32 Actual=Type==ESkillRuleType::Cross ? int32(R.ActualBranch.Cross)
		: Type==ESkillRuleType::PassControl ? int32(R.ActualBranch.PassControl) : int32(R.ActualBranch.ThroughBall);
	TestEqual(TEXT("Exact canonical route outcome matrix"),Actual,Outcome);
	for (auto* PC : {F.A,F.B})
	{
		const auto V=PC->GetOwnerView();const auto Safe=Access::Safe(*F.Mode,V.ViewerSide,true,true);
		TestEqual(TEXT("Public disclosed D6"),V.InitialRoute.D6,D);
		TestEqual(TEXT("Copied safe family"),V.InitialRoute.ActionType,Safe.ResolutionFacts.ActualBranch.ActionType);
		TestEqual(TEXT("Copied safe Cross"),V.InitialRoute.Cross,Safe.ResolutionFacts.ActualBranch.Cross);
		TestEqual(TEXT("Copied safe PassControl"),V.InitialRoute.PassControl,Safe.ResolutionFacts.ActualBranch.PassControl);
		TestEqual(TEXT("Copied safe ThroughBall"),V.InitialRoute.ThroughBall,Safe.ResolutionFacts.ActualBranch.ThroughBall);
		TestTrue(TEXT("Chinese route label"),!V.InitialRoute.RouteLabel.IsEmpty() && !V.InitialRoute.RouteLabel.ToString().Contains(TEXT("Canonical.")));
		TestEqual(TEXT("Precise next wait"),V.EntryWait,Wait);
		TestEqual(TEXT("Next actor attacker"),V.ExpectedActingSide,BeforeView.ViewerSide);
		TestEqual(TEXT("No reroll control"),V.InitialRouteAction,EFMCodexNetworkInitialRouteAction::None);
	}
	return true;
}


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteReject,
	"FMCodex.NetworkPlay.InitialRouteTransport.02.Security",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteReject::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")})
		for (const TCHAR* C : {TEXT("WrongSide"),TEXT("BeforeSkill"),TEXT("WrongMatch"),TEXT("WrongSequence"),TEXT("HugeId"),
			TEXT("ZeroId"),TEXT("ZeroSequence"),TEXT("Internal"),TEXT("Nonparticipant"),TEXT("Branch"),TEXT("Skill"),
			TEXT("Carrier"),TEXT("Marker"),TEXT("Runner"),TEXT("Helper"),TEXT("Ordinary"),TEXT("Goalkeeper")})
		{const auto N=FString::Printf(TEXT("%s.%s"),F,C);Names.Add(N);Commands.Add(N);}
	Names.Add(TEXT("Cross.BeforeBranch"));Commands.Add(Names.Last());
}
bool FFMCodexRouteReject::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;
	const auto Type=Family(P);FString Prefix,Case;P.Split(TEXT("."),&Prefix,&Case);
	FFixture F;
	if (Case==TEXT("BeforeSkill"))
	{
		if (!TestTrue(TEXT("Actual Skill wait before freezing"),F.ReachSkill())) { return false; }
		F.RequestedKind=Type==ESkillRuleType::Cross ? Kind::CrossInitialRouteRoll : Type==ESkillRuleType::PassControl ? Kind::PassControlInitialRouteRoll : Kind::ThroughBallInitialRouteRoll;
	}
	else if (!TestTrue(TEXT("Current canonical family"),F.ReachRoute(Type,1,true,Case==TEXT("BeforeBranch")))) { return false; }
	auto* PC=Case==TEXT("WrongSide") ? F.Defender() : F.Attacker();
	auto E=F.Request(PC);const auto NormalId=E.RequestId;Code Expected=Code::AuthorityRejected;
	if (Case==TEXT("WrongMatch")) {E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
	if (Case==TEXT("WrongSequence")) {++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
	if (Case==TEXT("HugeId")) {E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
	if (Case==TEXT("ZeroId")) {E.RequestId=0;Expected=Code::InvalidPayload;}
	if (Case==TEXT("ZeroSequence")) {E.ExpectedAttackSequence=0;Expected=Code::InvalidPayload;}
	if (Case==TEXT("Internal")) {E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
	if (Case==TEXT("Nonparticipant")) {PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
	if (Case==TEXT("Branch")) {E.Branch.Intent=Branch::CrossHigh;Expected=Code::InvalidPayload;}
	if (Case==TEXT("Skill")) {E.Skill.SkillId=TEXT("Canonical.Skill.Cross.4.6");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Carrier")) {E.Carrier.CarrierCardId=TEXT("Carrier");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Marker")) {E.Marker.MarkerCardId=TEXT("Marker");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Runner")) {E.Runner.RunnerCardId=TEXT("Runner");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Helper")) {E.Helper.HelperCardId=TEXT("Helper");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Ordinary")) {E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	if (Case==TEXT("Goalkeeper")) {E.Goalkeeper.SlotId=TEXT("Slot");Expected=Code::InvalidPayload;}
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact rejection"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Expected);Before.Verify(*this,F);
	if (Case!=TEXT("BeforeSkill") && Case!=TEXT("BeforeBranch"))
	{
		auto Normal=F.Request(F.Attacker());
		if (Case==TEXT("HugeId") || Case==TEXT("WrongMatch")) {Normal.RequestId=NormalId;}
		TestEqual(TEXT("No poisoning; next normal succeeds"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),Normal).Code,Code::Accepted);
	}
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteWrongFamily,
	"FMCodex.NetworkPlay.InitialRouteTransport.03.WrongFamily",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteWrongFamily::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")})
		for (int32 K=11;K<=13;++K)
		{
			const int32 Own=FCString::Strcmp(F,TEXT("Cross"))==0 ? 11 : FCString::Strcmp(F,TEXT("PassControl"))==0 ? 12 : 13;
			if(K==Own){continue;}const auto N=FString::Printf(TEXT("%s.%d"),F,K);Names.Add(N);Commands.Add(N);
		}
}
bool FFMCodexRouteWrongFamily::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;
	if (!TestTrue(TEXT("Frozen family"),F.ReachRoute(Family(P)))) {return false;}
	auto E=F.Request(F.Attacker());E.IntentKind=static_cast<Kind>(FCString::Atoi(*P.Right(2)));const FUnchanged Before(F);
	TestEqual(TEXT("Other typed family rejected before provider"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E).Code,Code::AuthorityRejected);Before.Verify(*this,F);
	TestEqual(TEXT("Current family still usable"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),F.Request(F.Attacker())).Code,Code::Accepted);
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteReplay,
	"FMCodex.NetworkPlay.InitialRouteTransport.04.ReplayAndReroll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteReplay::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")}) {N.Add(F);C.Add(F);}}
bool FFMCodexRouteReplay::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;
	if (!TestTrue(TEXT("Route wait"),F.ReachRoute(Family(P)))) {return false;}
	auto E=F.Request(F.Attacker());TestEqual(TEXT("First draw"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E).Code,Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Duplicate cannot draw"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E).Code,Code::DuplicateOrAlreadyResolved);Frozen.Verify(*this,F);
	for (int32 K=11;K<=13;++K)
	{
		E.RequestId=F.Next(F.Attacker())++;E.IntentKind=static_cast<Kind>(K);
		TestEqual(TEXT("Fresh same/cross-family reroll rejected"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
	}
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteFailure,
	"FMCodex.NetworkPlay.InitialRouteTransport.05.ProviderFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteFailure::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")}) {N.Add(F);C.Add(F);}}
bool FFMCodexRouteFailure::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;
	if (!TestTrue(TEXT("Route wait"),F.ReachRoute(Family(P)))) {return false;}
	auto& Client=F.Client(F.Attacker());Envelope E;
	TestTrue(TEXT("Normal pending"),Client.BeginInitialRoute(F.Attacker()->GetOwnerView(),F.RequestedKind,E));
	F.Entropy->bFail=true;const FUnchanged Before(F);
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E);
	TestEqual(TEXT("Typed failure, no fallback"),Ack.Code,Code::AuthorityRejected);Before.Verify(*this,F,true);
	TestTrue(TEXT("Failure receipt correlated"),Client.ObserveAck(Ack));TestFalse(TEXT("Failure releases pending"),Client.IsPending());
	TestEqual(TEXT("No published D6"),F.Attacker()->GetOwnerView().InitialRoute.D6,0);
	TestTrue(TEXT("No partial session adoption"),!Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.bHasResolutionSession);
	F.Entropy->bFail=false;Envelope Retry;
	TestTrue(TEXT("Only explicit new player request retries"),Client.BeginInitialRoute(F.Attacker()->GetOwnerView(),F.RequestedKind,Retry));
	TestEqual(TEXT("Explicit retry accepts after provider repaired"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),Retry).Code,Code::Accepted);
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRoutePending,
	"FMCodex.NetworkPlay.InitialRouteTransport.06.PendingOrder",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRoutePending::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")})
		for (const TCHAR* O : {TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("Rejected")})
		{const auto S=FString::Printf(TEXT("%s.%s"),F,O);N.Add(S);C.Add(S);}
}
bool FFMCodexRoutePending::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;
	if (!TestTrue(TEXT("Route wait"),F.ReachRoute(Family(P)))) {return false;}
	auto& Client=F.Client(F.Attacker());const auto Before=F.Attacker()->GetOwnerView();Envelope E;
	TestTrue(TEXT("Begin typed route in shared pending"),Client.BeginInitialRoute(Before,F.RequestedKind,E));const auto Original=E;
	TestFalse(TEXT("Duplicate click suppressed"),Client.BeginInitialRoute(Before,F.RequestedKind,E));TestEqual(TEXT("Envelope preserved"),E.RequestId,Original.RequestId);
	FFMCodexNetworkPlayerIntentAck Stale;Stale.MatchInstanceId=E.MatchInstanceId;Stale.RequestId=E.RequestId+1;Stale.Code=Code::Accepted;Stale.ViewRevision=Before.ViewRevision+1;
	TestFalse(TEXT("Uncorrelated ACK ignored"),Client.ObserveAck(Stale));TestTrue(TEXT("Still pending"),Client.IsPending());
	if(P.EndsWith(TEXT("Rejected"))){E.IntentKind=F.RequestedKind==Kind::CrossInitialRouteRoll ? Kind::PassControlInitialRouteRoll : Kind::CrossInitialRouteRoll;}
	const auto Ack=F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E);
	if(P.EndsWith(TEXT("Rejected")))
	{
		TestEqual(TEXT("Canonical rejection"),Ack.Code,Code::AuthorityRejected);Client.ObserveAck(Ack);
		TestFalse(TEXT("Rejection releases without publication"),Client.IsPending());return true;
	}
	const auto After=F.Attacker()->GetOwnerView();
	TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);
	if(P.EndsWith(TEXT("AckFirst"))){Client.ObserveAck(Ack);}else{Client.ObserveView(After);}
	TestTrue(TEXT("One arrival insufficient"),Client.IsPending());Client.ObserveView(Before);TestTrue(TEXT("Old view insufficient"),Client.IsPending());
	if(P.EndsWith(TEXT("AckFirst"))){Client.ObserveView(After);}else{Client.ObserveAck(Ack);}
	TestFalse(TEXT("Correlated stable pair completes"),Client.IsPending());TestFalse(TEXT("Duplicate ACK ignored"),Client.ObserveAck(Ack));return true;
}


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteStale,
	"FMCodex.NetworkPlay.InitialRouteTransport.07.RealAttackStale",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteStale::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")}) {N.Add(F);C.Add(F);}}
bool FFMCodexRouteStale::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;const auto Type=Family(P);
	if (!TestTrue(TEXT("Attack N real route wait"),F.ReachRoute(Type))) {return false;}
	auto Held=F.Request(F.Attacker());--F.Next(F.Attacker());
	TestTrue(TEXT("N route through generic client and transport"),F.Send(F.Attacker(),F.RequestedKind));
	auto& Session=Access::Session(*F.Mode);const int64 Seq=Held.ExpectedAttackSequence;
	// Fixture-only later canonical inputs prepare the next attack; no added network command.
	auto Roll=[&](auto Request, auto Method, Side Player)
	{
		Request.AttackSequence=Seq;Request.RequestingSide=Player;
		return (Session.*Method)(Request).RuntimeEnvelope.bDomainSuccess;
	};
	bool AttackOK=false,DefenseOK=false;
	if(Type==ESkillRuleType::Cross)
	{
		AttackOK=Roll(FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest{},&FMatchPlayAuthoritativeSession::ResolveCrossHighAttackRoll,Side::PlayerA);
		DefenseOK=Roll(FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest{},&FMatchPlayAuthoritativeSession::ResolveCrossHighDefenseRoll,Side::PlayerB);
	}
	else if(Type==ESkillRuleType::PassControl)
	{
		AttackOK=Roll(FMatchPlayAuthoritativeResolvePassControlAttackRollRequest{},&FMatchPlayAuthoritativeSession::ResolvePassControlAttackRoll,Side::PlayerA);
		DefenseOK=Roll(FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest{},&FMatchPlayAuthoritativeSession::ResolvePassControlDefenseRoll,Side::PlayerB);
	}
	else
	{
		AttackOK=Roll(FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest{},&FMatchPlayAuthoritativeSession::ResolveThroughBallFeetAttackRoll,Side::PlayerA);
		DefenseOK=Roll(FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest{},&FMatchPlayAuthoritativeSession::ResolveThroughBallFeetDefenseRoll,Side::PlayerB);
	}
	if (!TestTrue(TEXT("Canonical N contest inputs"),AttackOK && DefenseOK)) {return false;}
	TestEqual(TEXT("Canonical terminal needs explicit advance"),Access::Advance(*F.Mode).StopReason,EMatchPlayServerCoordinatorStopReason::TerminalPendingAdvance);
	F.Entropy->Word=5;
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;Advance.AttackSequence=Seq;Advance.RequestingSide=Side::PlayerA;
	if (!TestTrue(TEXT("Explicit fixture-only terminal advance"),Session.AdvanceAfterTerminal(Advance).CompletionResult.bSuccess)) {return false;}
	TestTrue(TEXT("Canonical next entry wait"),Access::Advance(*F.Mode).bSuccess);Access::Publish(*F.Mode);
	TestEqual(TEXT("Old route disclosure cannot cross attack boundary"),F.A->GetOwnerView().InitialRoute.D6,0);
	TestEqual(TEXT("Real N+1"),F.B->GetOwnerView().AttackSequence,Seq+1);
	if (!TestTrue(TEXT("N+1 same family route wait"),F.ReachRoute(Type))) {return false;}
	Held.RequestId=F.Next(F.A)++;const FUnchanged Before(F);
	TestEqual(TEXT("Fresh ID old sequence rejected before RNG"),F.Mode->SubmitConnectionPlayerIntent(F.A,Held).Code,Code::StaleAttackSequence);
	Before.Verify(*this,F);
	TestEqual(TEXT("New attacker route remains usable"),F.Mode->SubmitConnectionPlayerIntent(F.B,F.Request(F.B)).Code,Code::Accepted);
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteDisclosure,
	"FMCodex.NetworkPlay.InitialRouteTransport.08.SafeProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteDisclosure::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for (const TCHAR* F : {TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")}) {N.Add(F);C.Add(F);}}
bool FFMCodexRouteDisclosure::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F;
	if (!TestTrue(TEXT("Route wait"),F.ReachRoute(Family(P),5))) {return false;}
	auto Project=[&](const auto& Safe,Side Player){return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),99,Player,EFMCodexNetworkBootstrapState::MatchReady);};
	const auto Offered=Access::Safe(*F.Mode,Side::PlayerA);
	TestEqual(TEXT("Nonactor cannot gain action even with actor source"),Project(Offered,Side::PlayerB).InitialRouteAction,EFMCodexNetworkInitialRouteAction::None);
	TestEqual(TEXT("Invalid viewer gets no action"),Project(Offered,Side::None).InitialRouteAction,EFMCodexNetworkInitialRouteAction::None);
	TestEqual(TEXT("Successful route"),F.Mode->SubmitConnectionPlayerIntent(F.A,F.Request(F.A)).Code,Code::Accepted);
	const FUnchanged Frozen(F);
	for (const auto Player : {Side::PlayerA,Side::PlayerB})
	{
		const auto Hidden=Project(Access::Safe(*F.Mode,Player,true,false),Player);
		TestEqual(TEXT("Unrevealed route D6 withheld"),Hidden.InitialRoute.D6,0);
		TestEqual(TEXT("Unrevealed actual route withheld"),Hidden.InitialRoute.ActionType,ESkillRuleType::None);
		TestTrue(TEXT("Unrevealed text withheld"),Hidden.InitialRoute.RouteLabel.IsEmpty());
		const auto PriorAttackHidden=Project(Access::Safe(*F.Mode,Player,false,true),Player);
		TestEqual(TEXT("Initial attack disclosure still gates route"),PriorAttackHidden.InitialRoute.D6,0);
		auto Safe=Access::Safe(*F.Mode,Player,true,true);
		if (!TestEqual(TEXT("Single safe initial roll"),Safe.AcceptedRolls.Num(),1)) {return false;}
		const auto Duplicate = Safe.AcceptedRolls[0]; Safe.AcceptedRolls.Add(Duplicate);
		TestEqual(TEXT("Duplicate route record projection fails as whole"),Project(Safe,Player).InitialRoute.D6,0);
		Safe.AcceptedRolls.Pop();Safe.AcceptedRolls[0].RawD6=7;
		TestEqual(TEXT("Invalid D6 projection fails"),Project(Safe,Player).InitialRoute.D6,0);
		Safe.AcceptedRolls.Reset();TestEqual(TEXT("Route without disclosed roll fails closed"),Project(Safe,Player).InitialRoute.D6,0);
	}
	Frozen.Verify(*this,F);return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRouteMilestone,
	"FMCodex.NetworkPlay.InitialRouteTransport.09.CanonicalMilestone",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRouteMilestone::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	for (const TCHAR* A:{TEXT("A"),TEXT("B")})for(const TCHAR* F:{TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall")})
	{const auto P=FString::Printf(TEXT("%s.%s"),A,F);N.Add(P);C.Add(P);}
}
bool FFMCodexRouteMilestone::RunTest(const FString& P)
{
	using namespace FMCodexNetworkInitialRouteTests;FFixture F(P.StartsWith(TEXT("B")));const auto Type=Family(P);
	if(!TestTrue(TEXT("Server canonical milestone setup"),Access::Runtime(*F.Mode).PrepareInitialRouteMilestone(Type))){return false;}
	Access::Publish(*F.Mode);const auto S=Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Stops before Skill"),S.CurrentAttack.SelectionStage,EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);
	TestTrue(TEXT("No selected action, session or branch authored"),!S.CurrentAttack.bHasSelectedAction && !S.CurrentAttack.bHasResolutionSession && S.CurrentAttack.ActionPreparation.SkillId.IsNone());
	TestEqual(TEXT("No route provider in setup"),Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(),0);
	const FUnchanged Before(F);
	TestFalse(TEXT("Cannot replay milestone over active match"),Access::Runtime(*F.Mode).PrepareInitialRouteMilestone(Type));Before.Verify(*this,F);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRouteClosedWire,
	"FMCodex.NetworkPlay.InitialRouteTransport.10.ClosedWireNamespaceAndSharedPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRouteClosedWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkInitialRouteTests;
	for (int32 Tag=1;Tag<=13;++Tag)
		for (int32 Mask=0;Mask<256;++Mask)
		{
			Envelope E;E.IntentKind=static_cast<Kind>(Tag);
			if(Mask&1){E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot");}
			if(Mask&2){E.Goalkeeper.SlotId=TEXT("Slot");}
			if(Mask&4){E.Carrier.CarrierCardId=TEXT("Carrier");}
			if(Mask&8){E.Marker.MarkerCardId=TEXT("Marker");}
			if(Mask&16){E.Runner.RunnerCardId=TEXT("Runner");}
			if(Mask&32){E.Helper.HelperCardId=TEXT("Helper");}
			if(Mask&64){E.Skill.SkillId=TEXT("Skill");}
			if(Mask&128){E.Branch.Intent=Branch::CrossHigh;}
			const int32 Expected=Tag==1 || Tag==4 || Tag>=11 ? 0 : Tag==2 ? 1 : Tag==3 ? 2 : 1<<(Tag-3);
			TestEqual(TEXT("13 tags x 256 masks: exactly closed shape"),E.ValidatePayloadShape(),Mask==Expected ? Code::None : Code::InvalidPayload);
		}

	for (int32 Tag=11;Tag<=13;++Tag)
	{
		Envelope Original;Original.MatchInstanceId=FGuid::NewGuid();Original.RequestId=17;Original.ExpectedAttackSequence=3;Original.IntentKind=static_cast<Kind>(Tag);
		TArray<uint8> Bytes;FMemoryWriter Storage(Bytes);FObjectAndNameAsStringProxyArchive Writer(Storage,false);
		Envelope::StaticStruct()->SerializeItem(Writer,&Original,nullptr);
		FMemoryReader Input(Bytes);FObjectAndNameAsStringProxyArchive Reader(Input,false);Envelope Decoded;
		Envelope::StaticStruct()->SerializeItem(Reader,&Decoded,nullptr);
		TestTrue(TEXT("Reflected empty route envelope roundtrip"),!Writer.IsError() && !Reader.IsError()
			&& Envelope::StaticStruct()->CompareScriptStruct(&Original,&Decoded,0));
		TestEqual(TEXT("Decoded route remains empty gameplay shape"),Decoded.ValidatePayloadShape(),Code::None);
	}

	FFMCodexNetworkIntentLedger Ledger;Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.ExpectedAttackSequence=1;
	for(int32 I=1;I<=13;++I)
	{
		E.RequestId=I;E.IntentKind=static_cast<Kind>(I);TestTrue(TEXT("Shared 13-kind ID namespace"),Ledger.Consume(E.MatchInstanceId,E));
		auto Other=E;Other.IntentKind=Kind::CrossInitialRouteRoll;
		TestEqual(TEXT("Kind cannot reuse an ID"),Ledger.Check(E.MatchInstanceId,Other),Code::DuplicateOrAlreadyResolved);
	}
	E.RequestId=13+1025;TestEqual(TEXT("1025 forward jump denied"),Ledger.Check(E.MatchInstanceId,E),Code::InvalidPayload);
	E.RequestId=13+1024;TestEqual(TEXT("1024 bound allowed"),Ledger.Check(E.MatchInstanceId,E),Code::None);
	E.RequestId=14;TestTrue(TEXT("Rejected jump did not poison next ID"),Ledger.Consume(E.MatchInstanceId,E));
	FFixture F;if(!TestTrue(TEXT("Typed variant check fixture"),F.ReachRoute(ESkillRuleType::Cross))){return false;}
	const FUnchanged Before(F);
	for(const auto Type:{ESkillRuleType::Cross,ESkillRuleType::PassControl,ESkillRuleType::ThroughBall})
	{
		auto Wrong=Canonical(Type,1,Side::PlayerA);Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
		TestEqual(TEXT("Canonical variant mismatch before Session"),Access::Runtime(*F.Mode).SubmitPlayerIntent(Wrong).ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
		Before.Verify(*this,F);
	}
	FString Source;TestTrue(TEXT("Controller source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	int32 Start=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitInitialRoute("));
	int32 End=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeWrongRouteFamily"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	auto Block=Source.Mid(Start,End-Start);
	TestTrue(TEXT("Generated RPC common for both owning roles"),Block.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation or Host bypass"),Block.Contains(TEXT("_Implementation")) || Block.Contains(TEXT("HasAuthority")) || Block.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	Start=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll:"));
	End=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll:"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	Block=Source.Mid(Start,End-Start);
	TestTrue(TEXT("Shared Local dispatch retains DEV decorators"),Block.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")) && Block.Contains(TEXT("ExecuteProviderCall")));
	return true;
}

#endif
