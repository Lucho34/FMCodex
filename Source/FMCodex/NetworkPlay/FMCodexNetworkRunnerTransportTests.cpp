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
#include "../CoreRules/MatchPlayOrdinaryDeploymentLegality.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

struct FFMCodexNetworkRunnerTestAccess
{
	static FFMCodexNetworkScriptedEntropy* Configure(AFMCodexNetworkMatchGameMode& Mode,
		AFMCodexNetworkMatchPlayerController* A, AFMCodexNetworkMatchPlayerController* B, bool BFirst)
	{
		Mode.MatchInstanceId = FGuid::NewGuid();
		Mode.BootstrapConfiguration = BFirst
			? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		Mode.ParticipantRegistry.Admit(A, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		Mode.ParticipantRegistry.Admit(B, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		auto Entropy = MakeUnique<FFMCodexNetworkScriptedEntropy>(TArray<uint32>{7, 7, 7, 7, 7, 7, 7, 7});
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
	static FFMCodexLocalMatchInteractionView Safe(AFMCodexNetworkMatchGameMode& Mode, EInitialTurnOrderPlayer Side, bool Reveal = true)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = Reveal;
		return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Session(Mode).GetStateSnapshot(), Mode.MatchRuntime->SkillRuleSet, Side, Disclosure);
	}
};

namespace FMCodexNetworkRunnerTests
{
	using Access = FFMCodexNetworkRunnerTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	using Envelope = FFMCodexNetworkPlayerIntentEnvelope;
	using MarkerPayload = FFMCodexNetworkSubmitMarkerPayload;
	using Payload = FFMCodexNetworkSubmitRunnerPayload;
	using CarrierPayload = FFMCodexNetworkSubmitCarrierPayload;
	struct FFixture
	{
		UWorld* World;
		AFMCodexNetworkMatchGameMode* Mode;
		AFMCodexNetworkMatchPlayerController* A;
		AFMCodexNetworkMatchPlayerController* B;
		FFMCodexNetworkScriptedEntropy* Entropy;
		FFMCodexNetworkIntentClientState ClientA, ClientB;
		int64 NextA = 1, NextB = 1;
		explicit FFixture(bool BFirst = false)
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Mode = World->SpawnActor<AFMCodexNetworkMatchGameMode>();
			A = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			B = World->SpawnActor<AFMCodexNetworkMatchPlayerController>();
			World->AddController(A); World->AddController(B);
			Entropy = Access::Configure(*Mode, A, B, BFirst);
		}
		~FFixture() { GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		AFMCodexNetworkMatchPlayerController* Attacker() const { return A->GetOwnerView().CurrentAttackingSide == Side::PlayerA ? A : B; }
		AFMCodexNetworkMatchPlayerController* Defender() const { return Attacker() == A ? B : A; }
		FFMCodexNetworkIntentClientState& Client(AFMCodexNetworkMatchPlayerController* PC) { return PC == A ? ClientA : ClientB; }
		int64& Next(AFMCodexNetworkMatchPlayerController* PC) { return PC == A ? NextA : NextB; }
		int32 Calls() const { return Access::Runtime(*Mode).GetCoordinatorInvocationCountForTests(); }
		Envelope Request(AFMCodexNetworkMatchPlayerController* PC, Kind K = Kind::SubmitRunner)
		{
			Envelope E; E.MatchInstanceId = Mode->GetMatchInstanceId(); E.RequestId = Next(PC)++;
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = K;
			if (K == Kind::SubmitMarker && !Defender()->GetOwnerView().MarkerOptions.IsEmpty())
			{
				E.Marker = Defender()->GetOwnerView().MarkerOptions[0].Choice;
			}
			if (K == Kind::SubmitRunner && !Attacker()->GetOwnerView().RunnerOptions.IsEmpty())
			{ E.Runner = Attacker()->GetOwnerView().RunnerOptions[0].Choice; }
			return E;
		}
		bool Send(AFMCodexNetworkMatchPlayerController* PC, Kind K,
			const FFMCodexNetworkDeployOrdinaryPayload& Ordinary = {}, const CarrierPayload& Carrier = {}, const MarkerPayload& Marker = {}, const FFMCodexNetworkSubmitRunnerPayload& Runner = {})
		{
			auto& C = Client(PC); Envelope E; const auto& V = PC->GetOwnerView();
			FFMCodexNetworkDeployGoalkeeperPayload GK; GK.SlotId = Ordinary.SlotId;
			bool Began = K == Kind::RequestInitialActionPointRoll ? C.Begin(V, E)
				: K == Kind::DeployOrdinary ? C.BeginDeployment(V, Ordinary, E)
				: K == Kind::DeployGoalkeeper ? C.BeginGoalkeeper(V, GK, E)
				: K == Kind::FinishDeployment ? C.BeginFinishDeployment(V, E)
				: K == Kind::SubmitCarrier ? C.BeginCarrier(V, Carrier, E)
				: K == Kind::SubmitMarker ? C.BeginMarker(V, Marker, E)
				: K == Kind::SubmitRunner && C.BeginRunner(V, Runner, E);
			if (!Began) { return false; }
			Next(PC) = E.RequestId + 1;
			const auto Ack = Mode->SubmitConnectionPlayerIntent(PC, E);
			C.ObserveView(PC->GetOwnerView()); C.ObserveAck(Ack);
			return Ack.Code == Code::Accepted && !C.IsPending();
		}
		FFMCodexNetworkDeployOrdinaryPayload Choice(AFMCodexNetworkMatchPlayerController* PC, const FString& Half = {})
		{
			const auto Safe = Access::Safe(*Mode, PC->GetOwnerView().ViewerSide);
			for (const auto& O : Safe.DeploymentOptions)
			{
				// Known test catalog geometry only; every selected pair comes from canonical safe availability.
				if (!O.bGoalkeeper && (Half.IsEmpty() || O.SlotId.ToString().Contains(Half)))
				{
					FFMCodexNetworkDeployOrdinaryPayload P; P.CardId = O.CardId; P.SlotId = O.SlotId; return P;
				}
			}
			return {};
		}
		// Canonical no-Runner setup: only the Carrier is deployed for the attacker.
		bool ReachMarkerWithoutRunner()
		{
			auto* Attack = Attacker(); auto* Defense = Defender();
			if (!Entropy || !Send(Attack, Kind::RequestInitialActionPointRoll)) { return false; }
			const auto Carrier = Choice(Attack);
			const FString Half = Carrier.SlotId.ToString().Contains(TEXT("NearA")) ? TEXT("NearA") : TEXT("NearB");
			if (!Send(Attack, Kind::DeployOrdinary, Carrier)
				|| !Send(Defense, Kind::DeployOrdinary, Choice(Defense, Half))
				|| !Send(Attack, Kind::FinishDeployment)
				|| !Send(Defense, Kind::FinishDeployment)) { return false; }
			CarrierPayload C; C.CarrierCardId = Carrier.CardId;
			return Send(Attack, Kind::SubmitCarrier, {}, C)
				&& Defense->GetOwnerView().MarkerOptions.Num() == 1;
		}
		bool ReachRunner(int32 Count = 2, bool WithHelper = true, bool SameHalf = false)
		{
			auto* Attack = Attacker(); auto* Defense = Defender();
			if (!Entropy || !Send(Attack, Kind::RequestInitialActionPointRoll)) { return false; }
			const auto Carrier = Choice(Attack);
			const FString Half = Carrier.SlotId.ToString().Contains(TEXT("NearA")) ? TEXT("NearA") : TEXT("NearB");
			const FString Other = Half == TEXT("NearA") ? TEXT("NearB") : TEXT("NearA");
			const FString RunnerHalf = SameHalf ? Half : Other;
			if (!Send(Attack, Kind::DeployOrdinary, Carrier)
				|| !Send(Defense, Kind::DeployOrdinary, Choice(Defense, Half))
				|| !Send(Attack, Kind::DeployOrdinary, Choice(Attack, RunnerHalf))) { return false; }
			if (WithHelper)
			{
				if (!Send(Defense, Kind::DeployOrdinary, Choice(Defense, RunnerHalf))) { return false; }
				if (Count > 1 && !Send(Attack, Kind::DeployOrdinary, Choice(Attack, RunnerHalf))) { return false; }
				if (Count == 1 && !Send(Attack, Kind::FinishDeployment)) { return false; }
			}
			if (!Send(Defense, Kind::FinishDeployment)) { return false; }
			for (int32 I = WithHelper ? 2 : 1; I < Count; ++I)
			{
				const auto NextChoice = Choice(Attack, Count <= 2 ? RunnerHalf : FString());
				if (!Send(Attack, Kind::DeployOrdinary, NextChoice)) { return false; }
			}
			if ((!WithHelper || Count > 1) && !Send(Attack, Kind::FinishDeployment)) { return false; }
			CarrierPayload C; C.CarrierCardId = Carrier.CardId;
			if (!Send(Attack, Kind::SubmitCarrier, {}, C)) { return false; }
			if (Defense->GetOwnerView().MarkerOptions.IsEmpty()) { return false; }
			const auto M = Defense->GetOwnerView().MarkerOptions[0].Choice;
			return Send(Defense, Kind::SubmitMarker, {}, {}, M)
				&& Attack->GetOwnerView().EntryWait == EFMCodexNetworkEntryWait::RunnerSelection
				&& Attack->GetOwnerView().RunnerOptions.Num() == Count;
		}

	};
	bool SameState(const FMatchPlayState& A, const FMatchPlayState& B)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(&A, &B, 0);
	}
	struct FUnchanged
	{
		FMatchPlayState State;
		int32 Revision, EntropyCalls, EntryCalls, D12Calls, CoordinatorCalls;
		explicit FUnchanged(FFixture& F)
			: State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			EntropyCalls(F.Entropy->Calls), EntryCalls(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()),
			D12Calls(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), CoordinatorCalls(F.Calls()) {}
		void Verify(FAutomationTestBase& Test, FFixture& F) const
		{
			Test.TestTrue(TEXT("Entire authoritative State unchanged"), SameState(State, Access::Session(*F.Mode).GetStateSnapshot()));
			Test.TestEqual(TEXT("No publication"), Access::Revision(*F.Mode), Revision);
			Test.TestEqual(TEXT("All-provider entropy unchanged"), F.Entropy->Calls, EntropyCalls);
			Test.TestEqual(TEXT("Entry calls unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), EntryCalls);
			Test.TestEqual(TEXT("D12 calls unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12Calls);
			Test.TestEqual(TEXT("Coordinator not invoked on rejection"), F.Calls(), CoordinatorCalls);
		}
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRunnerAccepted,
	"FMCodex.NetworkPlay.RunnerTransport.01.CanonicalAcceptanceAndFreeze",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRunnerAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AFirst.First"), TEXT("AFirst.Last"), TEXT("BFirst.First"), TEXT("BFirst.Last")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexRunnerAccepted::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F(Parameter.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical deployment completion"), F.ReachRunner())) { return false; }
	auto* PC = F.Attacker();
	const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
	TestEqual(TEXT("All canonical choices represented"), PC->GetOwnerView().RunnerOptions.Num(), Safe.SelectionOptions.Num());
	const FUnchanged Before(F);
	auto E = F.Request(PC);
	if (Parameter.EndsWith(TEXT("Last"))) { E.Runner = PC->GetOwnerView().RunnerOptions.Last().Choice; }
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	if (!TestEqual(TEXT("Runner accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	auto Expected = Before.State;
	Expected.CurrentAttack.ActionPreparation.RunnerCardId = E.Runner.RunnerCardId;
	Expected.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;
	Expected.CurrentAttack.SelectionStage = EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
	TestTrue(TEXT("Only canonical Runner freeze and selection stage changed"), SameState(Expected, After));
	TestEqual(TEXT("One Coordinator invocation"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One stable publication"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("No entropy for Runner or stable Helper wait"), F.Entropy->Calls, Before.EntropyCalls);
	TestEqual(TEXT("Phase remains Resolution"), After.CurrentAttack.Phase, EMatchPlayCurrentAttackPhase::Resolution);
	TestTrue(TEXT("Helper not auto-selected"), After.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
	TestFalse(TEXT("No selected tactic"), After.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No resolution session"), After.CurrentAttack.bHasResolutionSession);
	for (auto* Viewer : {F.B, F.A})
	{
		const auto& V = Viewer->GetOwnerView();
		TestEqual(TEXT("Same public selected Runner"), V.SelectedRunner.Choice.RunnerCardId, E.Runner.RunnerCardId);
		TestEqual(TEXT("Same stable revision"), V.ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("Actual next wait"), V.EntryWait, EFMCodexNetworkEntryWait::HelperSelection);
		TestEqual(TEXT("Next actor is defender"), V.ExpectedActingSide, F.Defender()->GetOwnerView().ViewerSide);
		TestTrue(TEXT("No Runner action after freeze"), V.RunnerOptions.IsEmpty());
		TestFalse(TEXT("Deployment stays complete"), !V.bDeploymentComplete);
	}
	TestEqual(TEXT("Same public display name"), F.B->GetOwnerView().SelectedRunner.CardLabel.ToString(),
		F.A->GetOwnerView().SelectedRunner.CardLabel.ToString());
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRunnerRejection,
	"FMCodex.NetworkPlay.RunnerTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRunnerRejection::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("WrongSide"), TEXT("OpponentCard"), TEXT("Undeployed"), TEXT("UnknownCard"),
		TEXT("Goalkeeper"), TEXT("CarrierAsRunner"), TEXT("MarkerAsRunner"), TEXT("MarkerMember"), TEXT("Empty"), TEXT("Oversize"), TEXT("WrongMatch"), TEXT("WrongSequence"),
		TEXT("OrdinaryMember"), TEXT("GoalkeeperMember"), TEXT("CarrierMember"), TEXT("WrongTag"), TEXT("HugeId"),
		TEXT("ZeroId"), TEXT("ZeroSequence"), TEXT("Nonparticipant"), TEXT("BeforeRunner"), TEXT("InternalTag")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexRunnerRejection::RunTest(const FString& P)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("AwaitingRunner fixture"), F.ReachRunner())) { return false; }
	auto* Submitter = P == TEXT("WrongSide") ? F.Defender() : F.Attacker();
	auto E = F.Request(Submitter);
	Code Expected = Code::AuthorityRejected;
	const auto Safe = Access::Safe(*F.Mode, F.Attacker()->GetOwnerView().ViewerSide);
	if (P == TEXT("OpponentCard")) { E.Runner.RunnerCardId = Safe.PlayerBCardRoster[1].CardId; }
	if (P == TEXT("Undeployed"))
	{
		for (const auto& Card : Safe.PlayerACardRoster)
		{
			if (!Card.bGoalkeeper && !Safe.DeploymentPlacements.ContainsByPredicate([&](const auto& X) { return X.CardId == Card.CardId; }))
			{ E.Runner.RunnerCardId = Card.CardId; break; }
		}
	}
	if (P == TEXT("UnknownCard")) { E.Runner.RunnerCardId = TEXT("Unknown.Runner"); }
	if (P == TEXT("Goalkeeper")) { E.Runner.RunnerCardId = FName(*Access::Session(*F.Mode).GetStateSnapshot().RuntimeState.PlayerAState.GoalkeeperCardId); }
	if (P == TEXT("CarrierAsRunner")) { E.Runner.RunnerCardId = F.B->GetOwnerView().SelectedCarrier.Choice.CarrierCardId; }
	if (P == TEXT("MarkerAsRunner")) { E.Runner.RunnerCardId = F.A->GetOwnerView().SelectedMarker.Choice.MarkerCardId; }
	if (P == TEXT("MarkerMember")) { E.Marker.MarkerCardId = TEXT("Marker"); Expected = Code::InvalidPayload; }
	if (P == TEXT("CarrierMember")) { E.Carrier.CarrierCardId = TEXT("Carrier"); Expected = Code::InvalidPayload; }
	if (P == TEXT("Empty")) { E.Runner = {}; Expected = Code::InvalidPayload; }
	if (P == TEXT("Oversize")) { E.Runner.RunnerCardId = FName(*FString::ChrN(129, TEXT('x'))); Expected = Code::InvalidPayload; }
	if (P == TEXT("WrongMatch")) { E.MatchInstanceId = FGuid::NewGuid(); Expected = Code::MatchMismatch; }
	if (P == TEXT("WrongSequence")) { ++E.ExpectedAttackSequence; Expected = Code::StaleAttackSequence; }
	if (P == TEXT("OrdinaryMember")) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); Expected = Code::InvalidPayload; }
	if (P == TEXT("GoalkeeperMember")) { E.Goalkeeper.SlotId = TEXT("Slot"); Expected = Code::InvalidPayload; }
	if (P == TEXT("WrongTag")) { E.IntentKind = Kind::FinishDeployment; Expected = Code::InvalidPayload; }
	if (P == TEXT("HugeId")) { E.RequestId = MAX_int64; Expected = Code::InvalidPayload; }
	if (P == TEXT("ZeroId")) { E.RequestId = 0; Expected = Code::InvalidPayload; }
	if (P == TEXT("ZeroSequence")) { E.ExpectedAttackSequence = 0; Expected = Code::InvalidPayload; }
	if (P == TEXT("Nonparticipant")) { Submitter = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(); Expected = Code::NotParticipant; }
	if (P == TEXT("InternalTag")) { E.IntentKind = static_cast<Kind>(255); Expected = Code::NotPlayerIntent; }
	if (P == TEXT("BeforeRunner"))
	{
		FFixture Early;
		if (!TestTrue(TEXT("Initial bootstrap"), Early.Entropy != nullptr)) { return false; }
		auto EarlyE = Early.Request(Early.B); EarlyE.Runner.RunnerCardId = TEXT("Prototype.ManchesterCity.RubenDias");
		const FUnchanged Before(Early);
		TestEqual(TEXT("Early phase rejected canonically"), Early.Mode->SubmitConnectionPlayerIntent(Early.B, EarlyE).Code, Code::AuthorityRejected);
		Before.Verify(*this, Early);
		return true;
	}
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact typed rejection"), F.Mode->SubmitConnectionPlayerIntent(Submitter, E).Code, Expected);
	Before.Verify(*this, F);
	auto Valid = F.Request(F.Attacker());
	if (P == TEXT("HugeId")) { Valid.RequestId = E.RequestId = F.NextA - 2; }
	TestEqual(TEXT("Next normal Runner still works"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Valid).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerReplay,
	"FMCodex.NetworkPlay.RunnerTransport.03.DuplicateAndFreshRewrites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerReplay::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Multiple Runner choices"), F.ReachRunner())) { return false; }
	const auto Choices = F.A->GetOwnerView().RunnerOptions;
	auto E = F.Request(F.A);
	TestEqual(TEXT("Initial selection accepted"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Exact duplicate rejected"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::DuplicateOrAlreadyResolved);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.A)++;
	TestEqual(TEXT("Fresh ID same Runner rejected by canonical stage"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.A)++; E.Runner = Choices.Last().Choice;
	TestEqual(TEXT("Fresh ID different Runner cannot rewrite freeze"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerStale,
	"FMCodex.NetworkPlay.RunnerTransport.04.RealAttackStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerStale::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Attack N real Runner wait"), F.ReachRunner())) { return false; }
	auto Held = F.Request(F.A); --F.NextA;
	// Advance using the existing canonical Local PlayerIntent, not a new network command.
	FMatchPlayAuthoritativeDeclineRunnerRequest Decline;
	Decline.ExpectedAttackSequence = Held.ExpectedAttackSequence; Decline.RequestingSide = Side::PlayerA;
	const auto Closure = Access::Session(*F.Mode).DeclineRunner(Decline);
	if (!TestTrue(TEXT("Canonical attack N closure"), Closure.RuntimeEnvelope.bDomainSuccess)) { return false; }
	if (!TestTrue(TEXT("Canonical next wait"), Access::Advance(*F.Mode).bSuccess)) { return false; }
	Access::Publish(*F.Mode);
	TestEqual(TEXT("A real later attack"), F.B->GetOwnerView().AttackSequence, Held.ExpectedAttackSequence + 1);
	if (!TestTrue(TEXT("Attack N+1 also reaches Runner"), F.ReachRunner())) { return false; }
	Held.RequestId = F.Next(F.A)++;
	const FUnchanged Before(F);
	TestEqual(TEXT("Stale N request cannot mutate N+1"), F.Mode->SubmitConnectionPlayerIntent(F.A, Held).Code, Code::StaleAttackSequence);
	Before.Verify(*this, F);
	auto Current = F.Request(F.B);
	TestEqual(TEXT("Current attacker can select"), F.Mode->SubmitConnectionPlayerIntent(F.B, Current).Code, Code::Accepted);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRunnerAckOrder,
	"FMCodex.NetworkPlay.RunnerTransport.05.PendingOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRunnerAckOrder::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AckFirst"), TEXT("ViewFirst"), TEXT("Rejected")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexRunnerAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Real seven-kind client namespace setup"), F.ReachRunner())) { return false; }
	auto& C = F.ClientA; const auto View = F.A->GetOwnerView();
	auto Choice = View.RunnerOptions[0].Choice;
	if (P == TEXT("Rejected")) { Choice.RunnerCardId = TEXT("Forged.Runner"); }
	Envelope E;
	TestTrue(TEXT("Common pending begins"), C.BeginRunner(View, Choice, E));
	const auto Original = E;
	TestFalse(TEXT("Second click cannot emit"), C.BeginRunner(View, Choice, E));
	TestEqual(TEXT("Failed begin preserves envelope"), E.RequestId, Original.RequestId);
	const FUnchanged Before(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
	if (P == TEXT("Rejected"))
	{
		TestEqual(TEXT("Illegal candidate rejected"), Ack.Code, Code::AuthorityRejected);
		TestTrue(TEXT("Rejection correlated"), C.ObserveAck(Ack));
		TestFalse(TEXT("No publication needed to release pending"), C.IsPending());
		Before.Verify(*this, F);
		return true;
	}
	const auto After = F.A->GetOwnerView();
	TestEqual(TEXT("Accepted"), Ack.Code, Code::Accepted);
	if (P == TEXT("AckFirst")) { C.ObserveAck(Ack); } else { C.ObserveView(After); }
	TestTrue(TEXT("Only one arrival cannot complete pending"), C.IsPending());
	C.ObserveView(View);
	TestTrue(TEXT("Old view cannot complete pending"), C.IsPending());
	if (P == TEXT("AckFirst")) { C.ObserveView(After); } else { C.ObserveAck(Ack); }
	TestFalse(TEXT("Matching ACK and stable view complete"), C.IsPending());
	TestFalse(TEXT("Repeated ACK ignored"), C.ObserveAck(Ack));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerSafeProjection,
	"FMCodex.NetworkPlay.RunnerTransport.06.CompleteCanonicalCandidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerSafeProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Eight real deployed Runner choices"), F.ReachRunner(8, false))) { return false; }
	const FUnchanged Before(F);
	const auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
	const auto& View = F.A->GetOwnerView();
	TestEqual(TEXT("All eight choices, including both physical halves"), View.RunnerOptions.Num(), 8);
	TestEqual(TEXT("Full canonical set preserved"), View.RunnerOptions.Num(), Safe.SelectionOptions.Num());
	for (int32 I = 0; I < Safe.SelectionOptions.Num(); ++I)
	{
		TestEqual(TEXT("Canonical identity and order"), View.RunnerOptions[I].Choice.RunnerCardId, Safe.SelectionOptions[I].RelatedCardId);
		TestEqual(TEXT("Existing safe preferred name"), View.RunnerOptions[I].CardLabel.ToString(), Safe.SelectionOptions[I].Card.DisplayLabel);
	}
	TestTrue(TEXT("Nonacting viewer receives no actionable list"), F.B->GetOwnerView().RunnerOptions.IsEmpty());
	for (Side Viewer : {Side::PlayerB, Side::PlayerA, Side::None})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Undisclosed route has no Runner action"), Wire.RunnerOptions.IsEmpty());
		TestTrue(TEXT("Undisclosed route has no Runner fact"), Wire.SelectedRunner.Choice.IsEmpty());
	}
	TestTrue(TEXT("Choose eighth legitimate option"), F.Send(F.A, Kind::SubmitRunner, {}, {}, {}, View.RunnerOptions.Last().Choice));
	for (Side Viewer : {Side::PlayerB, Side::PlayerA})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Saved Runner remains withheld with undisclosed attack"), Wire.SelectedRunner.Choice.IsEmpty());
	}
	TestEqual(TEXT("Projection plus selection consumes no entropy"), F.Entropy->Calls, Before.EntropyCalls);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerBound,
	"FMCodex.NetworkPlay.RunnerTransport.07.AtomicProjectionBound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerBound::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Real safe source"), F.ReachRunner())) { return false; }
	auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
	const auto Original = Safe.SelectionOptions[0];
	Safe.SelectionOptions.Reset();
	for (int32 I = 0; I < FFMCodexNetworkClientViewSnapshot::MaxRunnerOptions; ++I)
	{
		auto O = Original; O.Id = O.RelatedCardId = FName(*FString::Printf(TEXT("Bounded.Candidate.%d"), I));
		O.Card.CardId = O.RelatedCardId; Safe.SelectionOptions.Add(O);
	}
	auto Project = [&]() { return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe, F.Mode->GetMatchInstanceId(), 99, Side::PlayerA, EFMCodexNetworkBootstrapState::MatchReady); };
	const auto Full = Project();
	TestEqual(TEXT("Complete canonical deck bound representable"), Full.RunnerOptions.Num(), 18);
	TestFalse(TEXT("Valid set is available"), Full.bRunnerOptionsUnavailable);
	const auto Last = Safe.SelectionOptions.Last();
	Safe.SelectionOptions.Add(Original);
	auto Bad = Project();
	TestTrue(TEXT("Overflow fails explicitly"), Bad.bRunnerOptionsUnavailable);
	TestTrue(TEXT("No silent truncation"), Bad.RunnerOptions.IsEmpty());
	Safe.SelectionOptions.Pop();
	Safe.SelectionOptions.Last().RelatedCardId = FName(*FString::ChrN(129, TEXT('x')));
	Bad = Project();
	TestTrue(TEXT("Unencodable candidate invalidates whole set"), Bad.bRunnerOptionsUnavailable && Bad.RunnerOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions.Last().RelatedCardId = Safe.SelectionOptions[0].RelatedCardId;
	Bad = Project();
	TestTrue(TEXT("Duplicate candidate fails rather than silently changing choices"), Bad.bRunnerOptionsUnavailable && Bad.RunnerOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions[0].bHasCard = false;
	TestEqual(TEXT("Missing display data uses generic name"), Project().RunnerOptions[0].CardLabel.ToString(), FString(TEXT("球员")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerWire,
	"FMCodex.NetworkPlay.RunnerTransport.08.BoundedWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	for (const FName Name : {FName(TEXT("Prototype.Runner")), FName(TEXT("持球球员")), FName(*FString::ChrN(128, TEXT('x')))})
	{
		Payload P; P.RunnerCardId = Name;
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = false;
		P.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Bounded Runner write"), Success && Bytes.Num() <= 129);
		FMemoryReader Reader(Bytes); Payload Copy; Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Bounded Runner read"), Success && !Reader.IsError());
		TestEqual(TEXT("Canonical identity roundtrip"), Copy.RunnerCardId, Name);
	}
	Payload Oversize; Oversize.RunnerCardId = FName(*FString::ChrN(129, TEXT('x')));
	TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = true;
	Oversize.NetSerialize(Writer, nullptr, Success);
	TestFalse(TEXT("Oversize sender fails codec"), Success);
	for (TArray<uint8> Invalid : {TArray<uint8>{129}, TArray<uint8>{2,'x'}, TArray<uint8>{1,0}, TArray<uint8>{2,0xC0,0xAF}})
	{
		FMemoryReader Reader(Invalid); Payload P; Success = true; P.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Malformed wire fails safely"), Success);
		TestTrue(TEXT("Malformed identity not adopted"), P.IsEmpty());
	}
	for (const Kind K : {Kind::RequestInitialActionPointRoll, Kind::DeployOrdinary, Kind::DeployGoalkeeper, Kind::FinishDeployment, Kind::SubmitCarrier, Kind::SubmitMarker, Kind::SubmitRunner})
	{
		for (int32 Mask = 0; Mask < 32; ++Mask)
		{
			Envelope E; E.IntentKind = K;
			if (Mask & 1) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); }
			if (Mask & 2) { E.Goalkeeper.SlotId = TEXT("Slot"); }
			if (Mask & 4) { E.Carrier.CarrierCardId = TEXT("Carrier"); }
			if (Mask & 8) { E.Marker.MarkerCardId = TEXT("Marker"); }
			if (Mask & 16) { E.Runner.RunnerCardId = TEXT("Runner"); }
			const int32 ExpectedMask = K == Kind::DeployOrdinary ? 1 : K == Kind::DeployGoalkeeper ? 2 : K == Kind::SubmitCarrier ? 4 : K == Kind::SubmitMarker ? 8 : K == Kind::SubmitRunner ? 16 : 0;
			TestEqual(TEXT("Seven kinds accept exactly their own shape"), E.ValidatePayloadShape(), Mask == ExpectedMask ? Code::None : Code::InvalidPayload);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerSharedPath,
	"FMCodex.NetworkPlay.RunnerTransport.09.SharedSemanticsAndGeneratedRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerSharedPath::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture Network, Local;
	if (!TestTrue(TEXT("Equivalent canonical fixtures"), Network.ReachRunner() && Local.ReachRunner())) { return false; }
	TestTrue(TEXT("Identical before State"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const auto E = Network.Request(Network.A);
	FMatchPlayAuthoritativeSubmitRunnerRequest Request;
	Request.ExpectedAttackSequence = E.ExpectedAttackSequence; Request.RequestingSide = Side::PlayerA; Request.RunnerCardId = E.Runner.RunnerCardId;
	const auto Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitRunner, Request);
	const int32 LocalBeforeCalls = Local.Calls();
	const auto LocalResult = Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
	TestTrue(TEXT("Transport-neutral shared HostPort accepts"), LocalResult.bSuccess);
	TestEqual(TEXT("Shared local-side dispatch coordinates once"), Local.Calls(), LocalBeforeCalls + 1);
	TestTrue(TEXT("With legal Helper no internal action needed"), LocalResult.CoordinatorResult.Steps.IsEmpty());
	TestEqual(TEXT("Coordinator stops at actual player intent"), LocalResult.CoordinatorResult.StopReason, EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Network adapter accepts same canonical choice"), Network.Mode->SubmitConnectionPlayerIntent(Network.A, E).Code, Code::Accepted);
	TestTrue(TEXT("Local/Network final State identical"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const FUnchanged Frozen(Local);
	FMatchPlayPlayerIntent Wrong; Wrong.CommandKind = EMatchPlayAuthoritativeCommandKind::SubmitRunner;
	Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	TestEqual(TEXT("Canonical wrong variant rejected before Session"), Access::Runtime(*Local.Mode).SubmitPlayerIntent(Wrong).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Frozen.Verify(*this, Local);
	FString Source;
	TestTrue(TEXT("Controller source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitRunnerChoice"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidRunner"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const auto Dispatch = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Runner calls generated owning RPC"), Dispatch.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation shortcut"), Dispatch.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No Host authority bypass"), Dispatch.Contains(TEXT("HasAuthority")));
	TestFalse(TEXT("No client HostPort"), Dispatch.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local host source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	const int32 LocalStart = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitRunner:"));
	const int32 LocalEnd = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::DeclineRunner:"), ESearchCase::CaseSensitive, ESearchDir::FromStart, LocalStart);
	TestTrue(TEXT("Actual Local Runner branch uses same shared port"), Source.Mid(LocalStart, LocalEnd - LocalStart).Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerNoRunner,
	"FMCodex.NetworkPlay.RunnerTransport.10.CanonicalNoRunnerContinuation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerNoRunner::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	if (!TestTrue(TEXT("Only Carrier deployed for attacker"), F.ReachMarkerWithoutRunner())) { return false; }
	FMatchPlayAuthoritativeSubmitMarkerRequest R;
	R.ExpectedAttackSequence = F.A->GetOwnerView().AttackSequence; R.RequestingSide = Side::PlayerB;
	R.MarkerCardId = F.B->GetOwnerView().MarkerOptions[0].Choice.MarkerCardId;
	const FUnchanged Before(F);
	const auto Result = Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitMarker, R));
	TestTrue(TEXT("Legal Marker accepted"), Result.bSuccess);
	TestEqual(TEXT("One coordinator pass"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("Canonical Runner absence followed by no legal Skill"), Result.CoordinatorResult.Steps.Num(), 2);
	if (!Result.CoordinatorResult.Steps.IsEmpty())
	{ TestEqual(TEXT("Server-owned no-legal Runner"), Result.CoordinatorResult.Steps[0].CommandKind, EMatchPlayAuthoritativeCommandKind::ResolveNoLegalRunner); }
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	if (Result.CoordinatorResult.Steps.Num() == 2)
	{ TestEqual(TEXT("Canonical no-Skill follows Runner absence"), Result.CoordinatorResult.Steps[1].CommandKind, EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill); }
	TestFalse(TEXT("Closed attack is no longer active before next D12"), State.bHasCurrentAttack);
	TestEqual(TEXT("No legal Skill subsequently closes attack"), State.CurrentAttack.SelectionStage, EMatchPlayCurrentAttackSelectionStage::None);
	TestTrue(TEXT("Runner and Helper absent"), State.CurrentAttack.ActionPreparation.RunnerCardId.IsNone() && State.CurrentAttack.ActionPreparation.HelperCardId.IsNone());
	TestFalse(TEXT("Next skill intent remains unexecuted"), State.CurrentAttack.bHasSelectedAction);
	TestEqual(TEXT("No RNG consumed"), F.Entropy->Calls, Before.EntropyCalls);
	Access::Publish(*F.Mode);
	TestEqual(TEXT("Safe next wait after automatic no-skill completion"), F.A->GetOwnerView().EntryWait, EFMCodexNetworkEntryWait::InitialD12);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRunnerNamespace,
	"FMCodex.NetworkPlay.RunnerTransport.11.SevenIntentNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRunnerNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkRunnerTests;
	FFMCodexNetworkClientViewSnapshot V; V.MatchInstanceId = FGuid::NewGuid(); V.ViewRevision = 1;
	V.bMatchInitialized = true; V.BootstrapState = EFMCodexNetworkBootstrapState::MatchReady;
	V.ViewerSide = V.ExpectedActingSide = Side::PlayerB; V.AttackSequence = 1;
	V.InteractionState = EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint;
	V.EntryBranch = EFMCodexNetworkEntryBranch::Ordinary; V.EntryWait = EFMCodexNetworkEntryWait::Deployment;
	V.bCanDeployGoalkeeper = V.bCanFinishDeployment = true;
	FFMCodexNetworkDeploymentOption Ordinary; Ordinary.Choice.CardId = TEXT("Card"); Ordinary.Choice.SlotId = TEXT("Slot"); V.DeploymentOptions.Add(Ordinary);
	FFMCodexNetworkDeployGoalkeeperPayload GK; GK.SlotId = TEXT("Slot");
	FFMCodexNetworkCarrierOption Carrier; Carrier.Choice.CarrierCardId = TEXT("Card"); V.CarrierOptions.Add(Carrier);
	FFMCodexNetworkMarkerOption Marker; Marker.Choice.MarkerCardId = TEXT("Card"); V.MarkerOptions.Add(Marker);
	FFMCodexNetworkRunnerOption Runner; Runner.Choice.RunnerCardId = TEXT("Card"); V.RunnerOptions.Add(Runner);
	FFMCodexNetworkIntentClientState C; FFMCodexNetworkIntentLedger Ledger; Envelope E;
	for (int32 I = 1; I <= 7; ++I)
	{
		if (I == 5) { V.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection; }
		if (I == 6) { V.EntryWait = EFMCodexNetworkEntryWait::MarkerSelection; }
		if (I == 7) { V.EntryWait = EFMCodexNetworkEntryWait::RunnerSelection; }
		const bool Began = I == 1 ? C.Begin(V, E) : I == 2 ? C.BeginDeployment(V, Ordinary.Choice, E)
			: I == 3 ? C.BeginGoalkeeper(V, GK, E) : I == 4 ? C.BeginFinishDeployment(V, E) : I == 5 ? C.BeginCarrier(V, Carrier.Choice, E) : I == 6 ? C.BeginMarker(V, Marker.Choice, E) : C.BeginRunner(V, Runner.Choice, E);
		TestTrue(TEXT("Every intent uses common begin"), Began);
		TestEqual(TEXT("Single monotonically increasing namespace"), E.RequestId, static_cast<int64>(I));
		TestTrue(TEXT("One common ledger"), Ledger.Consume(V.MatchInstanceId, E));
		auto OtherKind = E; OtherKind.IntentKind = I == 5 ? Kind::FinishDeployment : Kind::SubmitRunner;
		TestEqual(TEXT("Kind cannot reuse prior ID"), Ledger.Check(V.MatchInstanceId, OtherKind), Code::DuplicateOrAlreadyResolved);
		FFMCodexNetworkPlayerIntentAck Ack; Ack.MatchInstanceId = E.MatchInstanceId; Ack.RequestId = E.RequestId; Ack.Code = Code::Accepted;
		Ack.ViewRevision = ++V.ViewRevision; C.ObserveAck(Ack); C.ObserveView(V);
		TestFalse(TEXT("Generic completion"), C.IsPending());
	}
	auto Huge = E; Huge.RequestId = MAX_int64;
	TestEqual(TEXT("Runner huge jump denied"), Ledger.Check(V.MatchInstanceId, Huge), Code::InvalidPayload);
	E.RequestId = 8;
	TestTrue(TEXT("Huge jump did not poison normal next ID"), Ledger.Consume(V.MatchInstanceId, E));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRunnerContinuation,
	"FMCodex.NetworkPlay.RunnerTransport.12.PhysicalHalvesAndNoHelper",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRunnerContinuation::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("SameCarrierHalf"), TEXT("OtherCarrierHalf"), TEXT("NoHelper")}) { Names.Add(N); Commands.Add(N); }
}
bool FFMCodexRunnerContinuation::RunTest(const FString& P)
{
	using namespace FMCodexNetworkRunnerTests;
	FFixture F;
	const bool WithHelper = P != TEXT("NoHelper");
	if (!TestTrue(TEXT("Canonical Runner fixture"), F.ReachRunner(1, WithHelper, P == TEXT("SameCarrierHalf")))) { return false; }
	const auto E = F.Request(F.A);
	FMatchPlayAuthoritativeSubmitRunnerRequest R;
	R.ExpectedAttackSequence = E.ExpectedAttackSequence; R.RequestingSide = Side::PlayerA; R.RunnerCardId = E.Runner.RunnerCardId;
	const FUnchanged Before(F);
	const auto Result = Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitRunner, R));
	TestTrue(TEXT("Canonical Runner accepted without network half restriction"), Result.bSuccess);
	TestEqual(TEXT("Exactly one Coordinator"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("Only required internal actions"), Result.CoordinatorResult.Steps.Num(), WithHelper ? 0 : 2);
	if (!WithHelper && !Result.CoordinatorResult.Steps.IsEmpty())
	{ TestEqual(TEXT("Canonical Helper absence only"), Result.CoordinatorResult.Steps[0].CommandKind, EMatchPlayAuthoritativeCommandKind::ResolveNoLegalHelper); }
	if (!WithHelper && Result.CoordinatorResult.Steps.Num() == 2)
	{ TestEqual(TEXT("Then canonical no-Skill continuation"), Result.CoordinatorResult.Steps[1].CommandKind, EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill); }
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Actual next wait"), State.CurrentAttack.SelectionStage, WithHelper ? EMatchPlayCurrentAttackSelectionStage::AwaitingHelper : EMatchPlayCurrentAttackSelectionStage::None);
	TestTrue(TEXT("No player decision fabricated"), State.CurrentAttack.ActionPreparation.HelperCardId.IsNone() && State.CurrentAttack.ActionPreparation.SkillId.IsNone());
	TestFalse(TEXT("No selected action"), State.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No resolution session"), State.CurrentAttack.bHasResolutionSession);
	TestEqual(TEXT("No RNG"), F.Entropy->Calls, Before.EntropyCalls);
	return true;
}

#endif
