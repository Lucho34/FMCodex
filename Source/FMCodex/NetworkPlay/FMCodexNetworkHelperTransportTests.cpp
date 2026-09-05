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
#include "Serialization/MemoryWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"

struct FFMCodexNetworkHelperTestAccess
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

namespace FMCodexNetworkHelperTests
{
	using Access = FFMCodexNetworkHelperTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	using Envelope = FFMCodexNetworkPlayerIntentEnvelope;
	using MarkerPayload = FFMCodexNetworkSubmitMarkerPayload;
	using Payload = FFMCodexNetworkSubmitHelperPayload;
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
		Envelope Request(AFMCodexNetworkMatchPlayerController* PC)
		{
			Envelope E; E.MatchInstanceId = Mode->GetMatchInstanceId(); E.RequestId = Next(PC)++;
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = Kind::SubmitHelper;
			if (!Defender()->GetOwnerView().HelperOptions.IsEmpty())
			{ E.Helper = Defender()->GetOwnerView().HelperOptions[0].Choice; }
			return E;
		}
		bool Send(AFMCodexNetworkMatchPlayerController* PC, Kind K,
			const FFMCodexNetworkDeployOrdinaryPayload& Ordinary = {}, const CarrierPayload& Carrier = {}, const MarkerPayload& Marker = {}, const FFMCodexNetworkSubmitRunnerPayload& Runner = {}, const Payload& Helper = {})
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
				: K == Kind::SubmitHelper && C.BeginHelper(V, Helper, E);
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
		bool ReachHelper(int32 Count = 2, bool WithSkill = true, bool ExtraDefenders = true, bool SubmitRunner = true)
		{
			auto* Attack = Attacker(); auto* Defense = Defender();
			const bool IsA = Attack == A;
			if (!Entropy || !Send(Attack, Kind::RequestInitialActionPointRoll)) { return false; }
			const FName CarrierId = WithSkill
				? FName(IsA ? TEXT("Prototype.Arsenal.MartinOdegaard") : TEXT("Prototype.ManchesterCity.Rodri"))
				: NAME_None;
			const auto Carrier = Choice(Attack, {}, CarrierId);
			const FString Half = Carrier.SlotId.ToString().Contains(TEXT("NearA")) ? TEXT("NearA") : TEXT("NearB");
			const FString Other = Half == TEXT("NearA") ? TEXT("NearB") : TEXT("NearA");
			const FName RunnerId(IsA ? TEXT("Prototype.Arsenal.MylesLewisSkelly") : TEXT("Prototype.ManchesterCity.RayanAitNouri"));
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

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexHelperAccepted,
	"FMCodex.NetworkPlay.HelperTransport.01.CanonicalAcceptanceAndFreeze",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexHelperAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AFirst.First"), TEXT("AFirst.Last"), TEXT("BFirst.First"), TEXT("BFirst.Last")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexHelperAccepted::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F(Parameter.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical deployment completion"), F.ReachHelper())) { return false; }
	auto* PC = F.Defender();
	const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
	TestEqual(TEXT("All canonical choices represented"), PC->GetOwnerView().HelperOptions.Num(), Safe.SelectionOptions.Num());
	const FUnchanged Before(F);
	auto E = F.Request(PC);
	if (Parameter.EndsWith(TEXT("Last"))) { E.Helper = PC->GetOwnerView().HelperOptions.Last().Choice; }
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	if (!TestEqual(TEXT("Helper accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	auto Expected = Before.State;
	Expected.CurrentAttack.ActionPreparation.HelperCardId = E.Helper.HelperCardId;
	Expected.CurrentAttack.ActionPreparation.bHasHelper = true;
	Expected.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;
	Expected.CurrentAttack.SelectionStage = EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
	TestTrue(TEXT("Only canonical Helper freeze and selection stage changed"), SameState(Expected, After));
	TestEqual(TEXT("One Coordinator invocation"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One stable publication"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("No entropy for Helper or stable Skill wait"), F.Entropy->Calls, Before.EntropyCalls);
	TestEqual(TEXT("Phase remains Resolution"), After.CurrentAttack.Phase, EMatchPlayCurrentAttackPhase::Resolution);
	TestTrue(TEXT("Skill not auto-selected"), After.CurrentAttack.ActionPreparation.SkillId.IsNone());
	TestFalse(TEXT("No selected tactic"), After.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No resolution session"), After.CurrentAttack.bHasResolutionSession);
	for (auto* Viewer : {F.A, F.B})
	{
		const auto& V = Viewer->GetOwnerView();
		TestEqual(TEXT("Same public selected Helper"), V.SelectedHelper.Choice.HelperCardId, E.Helper.HelperCardId);
		TestEqual(TEXT("Same stable revision"), V.ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("Actual next wait"), V.EntryWait, EFMCodexNetworkEntryWait::SkillSelection);
		TestEqual(TEXT("Next actor is attacker"), V.ExpectedActingSide, F.Attacker()->GetOwnerView().ViewerSide);
		TestTrue(TEXT("No Helper action after freeze"), V.HelperOptions.IsEmpty());
		TestFalse(TEXT("Deployment stays complete"), !V.bDeploymentComplete);
	}
	TestEqual(TEXT("Same public display name"), F.A->GetOwnerView().SelectedHelper.CardLabel.ToString(),
		F.B->GetOwnerView().SelectedHelper.CardLabel.ToString());
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexHelperRejection,
	"FMCodex.NetworkPlay.HelperTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexHelperRejection::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("WrongSide"), TEXT("OpponentCard"), TEXT("Undeployed"), TEXT("UnknownCard"),
		TEXT("Goalkeeper"), TEXT("CarrierAsHelper"), TEXT("MarkerAsHelper"), TEXT("MarkerMember"), TEXT("RunnerMember"), TEXT("RunnerAsHelper"), TEXT("WrongHalf"), TEXT("Empty"), TEXT("Oversize"), TEXT("WrongMatch"), TEXT("WrongSequence"),
		TEXT("OrdinaryMember"), TEXT("GoalkeeperMember"), TEXT("CarrierMember"), TEXT("WrongTag"), TEXT("HugeId"),
		TEXT("ZeroId"), TEXT("ZeroSequence"), TEXT("Nonparticipant"), TEXT("BeforeHelper"), TEXT("InternalTag")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexHelperRejection::RunTest(const FString& P)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("AwaitingHelper fixture"), F.ReachHelper())) { return false; }
	auto* Submitter = P == TEXT("WrongSide") ? F.Attacker() : F.Defender();
	auto E = F.Request(Submitter);
	Code Expected = Code::AuthorityRejected;
	const auto Safe = Access::Safe(*F.Mode, F.Defender()->GetOwnerView().ViewerSide);
	if (P == TEXT("OpponentCard")) { E.Helper.HelperCardId = Safe.PlayerACardRoster[1].CardId; }
	if (P == TEXT("Undeployed"))
	{
		for (const auto& Card : Safe.PlayerBCardRoster)
		{
			if (!Card.bGoalkeeper && !Safe.DeploymentPlacements.ContainsByPredicate([&](const auto& X) { return X.CardId == Card.CardId; }))
			{ E.Helper.HelperCardId = Card.CardId; break; }
		}
	}
	if (P == TEXT("UnknownCard")) { E.Helper.HelperCardId = TEXT("Unknown.Helper"); }
	if (P == TEXT("Goalkeeper")) { E.Helper.HelperCardId = FName(*Access::Session(*F.Mode).GetStateSnapshot().RuntimeState.PlayerBState.GoalkeeperCardId); }
	if (P == TEXT("CarrierAsHelper")) { E.Helper.HelperCardId = F.A->GetOwnerView().SelectedCarrier.Choice.CarrierCardId; }
	if (P == TEXT("MarkerAsHelper")) { E.Helper.HelperCardId = F.B->GetOwnerView().SelectedMarker.Choice.MarkerCardId; }
	if (P == TEXT("RunnerMember")) { E.Runner.RunnerCardId = TEXT("Runner"); Expected = Code::InvalidPayload; }
	if (P == TEXT("RunnerAsHelper")) { E.Helper.HelperCardId = F.A->GetOwnerView().SelectedRunner.Choice.RunnerCardId; }
	if (P == TEXT("WrongHalf"))
	{
		const auto State = Access::Session(*F.Mode).GetStateSnapshot();
		for (const auto& Placement : Safe.DeploymentPlacements)
		{
			if (Placement.PlayerSide == Side::PlayerB
				&& Placement.CardId != State.CurrentAttack.ActionPreparation.MarkerCardId
				&& Placement.SlotId.ToString().Contains(TEXT("NearA")))
			{ E.Helper.HelperCardId = Placement.CardId; break; }
		}
	}
	if (P == TEXT("MarkerMember")) { E.Marker.MarkerCardId = TEXT("Marker"); Expected = Code::InvalidPayload; }
	if (P == TEXT("CarrierMember")) { E.Carrier.CarrierCardId = TEXT("Carrier"); Expected = Code::InvalidPayload; }
	if (P == TEXT("Empty")) { E.Helper = {}; Expected = Code::InvalidPayload; }
	if (P == TEXT("Oversize")) { E.Helper.HelperCardId = FName(*FString::ChrN(129, TEXT('x'))); Expected = Code::InvalidPayload; }
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
	if (P == TEXT("BeforeHelper"))
	{
		FFixture Early;
		if (!TestTrue(TEXT("Initial bootstrap"), Early.Entropy != nullptr)) { return false; }
		auto EarlyE = Early.Request(Early.B); EarlyE.Helper.HelperCardId = TEXT("Prototype.ManchesterCity.RubenDias");
		const FUnchanged Before(Early);
		TestEqual(TEXT("Early phase rejected canonically"), Early.Mode->SubmitConnectionPlayerIntent(Early.B, EarlyE).Code, Code::AuthorityRejected);
		Before.Verify(*this, Early);
		return true;
	}
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact typed rejection"), F.Mode->SubmitConnectionPlayerIntent(Submitter, E).Code, Expected);
	Before.Verify(*this, F);
	auto Valid = F.Request(F.Defender());
	if (P == TEXT("HugeId")) { Valid.RequestId = E.RequestId = F.NextB - 2; }
	TestEqual(TEXT("Next normal Helper still works"), F.Mode->SubmitConnectionPlayerIntent(F.Defender(), Valid).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperReplay,
	"FMCodex.NetworkPlay.HelperTransport.03.DuplicateAndFreshRewrites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperReplay::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("Multiple Helper choices"), F.ReachHelper())) { return false; }
	const auto Choices = F.B->GetOwnerView().HelperOptions;
	auto E = F.Request(F.B);
	TestEqual(TEXT("Initial selection accepted"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Exact duplicate rejected"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::DuplicateOrAlreadyResolved);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.B)++;
	TestEqual(TEXT("Fresh ID same Helper rejected by canonical stage"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.B)++; E.Helper = Choices.Last().Choice;
	TestEqual(TEXT("Fresh ID different Helper cannot rewrite freeze"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperStale,
	"FMCodex.NetworkPlay.HelperTransport.04.RealAttackStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperStale::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("Attack N real Helper wait"), F.ReachHelper(2, false))) { return false; }
	auto Held = F.Request(F.B); --F.NextB;
	// Advance using the existing canonical Local PlayerIntent, not a new network command.
	FMatchPlayAuthoritativeDeclineHelperRequest Decline;
	Decline.ExpectedAttackSequence = Held.ExpectedAttackSequence; Decline.RequestingSide = Side::PlayerB;
	const auto Closure = Access::Session(*F.Mode).DeclineHelper(Decline);
	if (!TestTrue(TEXT("Canonical attack N closure"), Closure.RuntimeEnvelope.bDomainSuccess)) { return false; }
	if (!TestTrue(TEXT("Canonical next wait"), Access::Advance(*F.Mode).bSuccess)) { return false; }
	Access::Publish(*F.Mode);
	TestEqual(TEXT("A real later attack"), F.A->GetOwnerView().AttackSequence, Held.ExpectedAttackSequence + 1);
	if (!TestTrue(TEXT("Attack N+1 also reaches Helper"), F.ReachHelper())) { return false; }
	Held.RequestId = F.Next(F.B)++;
	const FUnchanged Before(F);
	TestEqual(TEXT("Stale N request cannot mutate N+1"), F.Mode->SubmitConnectionPlayerIntent(F.B, Held).Code, Code::StaleAttackSequence);
	Before.Verify(*this, F);
	auto Current = F.Request(F.A);
	TestEqual(TEXT("Current defender can select"), F.Mode->SubmitConnectionPlayerIntent(F.A, Current).Code, Code::Accepted);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexHelperAckOrder,
	"FMCodex.NetworkPlay.HelperTransport.05.PendingOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexHelperAckOrder::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AckFirst"), TEXT("ViewFirst"), TEXT("Rejected")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexHelperAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("Real eight-kind client namespace setup"), F.ReachHelper())) { return false; }
	auto& C = F.ClientB; const auto View = F.B->GetOwnerView();
	auto Choice = View.HelperOptions[0].Choice;
	if (P == TEXT("Rejected")) { Choice.HelperCardId = TEXT("Forged.Helper"); }
	Envelope E;
	TestTrue(TEXT("Common pending begins"), C.BeginHelper(View, Choice, E));
	const auto Original = E;
	TestFalse(TEXT("Second click cannot emit"), C.BeginHelper(View, Choice, E));
	TestEqual(TEXT("Failed begin preserves envelope"), E.RequestId, Original.RequestId);
	const FUnchanged Before(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.B, E);
	if (P == TEXT("Rejected"))
	{
		TestEqual(TEXT("Illegal candidate rejected"), Ack.Code, Code::AuthorityRejected);
		TestTrue(TEXT("Rejection correlated"), C.ObserveAck(Ack));
		TestFalse(TEXT("No publication needed to release pending"), C.IsPending());
		Before.Verify(*this, F);
		return true;
	}
	const auto After = F.B->GetOwnerView();
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperSafeProjection,
	"FMCodex.NetworkPlay.HelperTransport.06.CompleteCanonicalCandidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperSafeProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("Four real deployed Helper choices"), F.ReachHelper(4, true, false))) { return false; }
	const FUnchanged Before(F);
	const auto Safe = Access::Safe(*F.Mode, Side::PlayerB);
	const auto& View = F.B->GetOwnerView();
	TestEqual(TEXT("All four same-Runner-half choices"), View.HelperOptions.Num(), 4);
	TestEqual(TEXT("Full canonical set preserved"), View.HelperOptions.Num(), Safe.SelectionOptions.Num());
	for (int32 I = 0; I < Safe.SelectionOptions.Num(); ++I)
	{
		TestEqual(TEXT("Canonical identity and order"), View.HelperOptions[I].Choice.HelperCardId, Safe.SelectionOptions[I].RelatedCardId);
		TestEqual(TEXT("Existing safe preferred name"), View.HelperOptions[I].CardLabel.ToString(), Safe.SelectionOptions[I].Card.DisplayLabel);
	}
	TestTrue(TEXT("Nonacting viewer receives no actionable list"), F.A->GetOwnerView().HelperOptions.IsEmpty());
	for (Side Viewer : {Side::PlayerA, Side::PlayerB, Side::None})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Undisclosed route has no Helper action"), Wire.HelperOptions.IsEmpty());
		TestTrue(TEXT("Undisclosed route has no Helper fact"), Wire.SelectedHelper.Choice.IsEmpty());
	}
	TestTrue(TEXT("Choose fourth legitimate option"), F.Send(F.B, Kind::SubmitHelper, {}, {}, {}, {}, View.HelperOptions.Last().Choice));
	for (Side Viewer : {Side::PlayerA, Side::PlayerB})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Saved Helper remains withheld with undisclosed attack"), Wire.SelectedHelper.Choice.IsEmpty());
	}
	TestEqual(TEXT("Projection plus selection consumes no entropy"), F.Entropy->Calls, Before.EntropyCalls);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperBound,
	"FMCodex.NetworkPlay.HelperTransport.07.AtomicProjectionBound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperBound::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	if (!TestTrue(TEXT("Real safe source"), F.ReachHelper())) { return false; }
	auto Safe = Access::Safe(*F.Mode, Side::PlayerB);
	const auto Original = Safe.SelectionOptions[0];
	Safe.SelectionOptions.Reset();
	for (int32 I = 0; I < FFMCodexNetworkClientViewSnapshot::MaxHelperOptions; ++I)
	{
		auto O = Original; O.Id = O.RelatedCardId = FName(*FString::Printf(TEXT("Bounded.Candidate.%d"), I));
		O.Card.CardId = O.RelatedCardId; Safe.SelectionOptions.Add(O);
	}
	auto Project = [&]() { return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe, F.Mode->GetMatchInstanceId(), 99, Side::PlayerB, EFMCodexNetworkBootstrapState::MatchReady); };
	const auto Full = Project();
	TestEqual(TEXT("Complete canonical deck bound representable"), Full.HelperOptions.Num(), 18);
	TestFalse(TEXT("Valid set is available"), Full.bHelperOptionsUnavailable);
	const auto Last = Safe.SelectionOptions.Last();
	Safe.SelectionOptions.Add(Original);
	auto Bad = Project();
	TestTrue(TEXT("Overflow fails explicitly"), Bad.bHelperOptionsUnavailable);
	TestTrue(TEXT("No silent truncation"), Bad.HelperOptions.IsEmpty());
	Safe.SelectionOptions.Pop();
	Safe.SelectionOptions.Last().RelatedCardId = FName(*FString::ChrN(129, TEXT('x')));
	Bad = Project();
	TestTrue(TEXT("Unencodable candidate invalidates whole set"), Bad.bHelperOptionsUnavailable && Bad.HelperOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions.Last().RelatedCardId = Safe.SelectionOptions[0].RelatedCardId;
	Bad = Project();
	TestTrue(TEXT("Duplicate candidate fails rather than silently changing choices"), Bad.bHelperOptionsUnavailable && Bad.HelperOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions[0].RelatedCardId = NAME_None;
	Bad = Project();
	TestTrue(TEXT("Empty identity invalidates complete set"), Bad.bHelperOptionsUnavailable && Bad.HelperOptions.IsEmpty());
	Safe.SelectionOptions[0] = Original;
	Safe.SelectionOptions[0].Side = Side::PlayerA;
	Bad = Project();
	TestTrue(TEXT("Wrong source Side invalidates complete set"), Bad.bHelperOptionsUnavailable && Bad.HelperOptions.IsEmpty());
	Safe.SelectionOptions[0].Side = Side::PlayerB;
	Safe.SelectionOptions[0].bHasCard = false;
	TestEqual(TEXT("Missing display data uses generic name"), Project().HelperOptions[0].CardLabel.ToString(), FString(TEXT("球员")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperWire,
	"FMCodex.NetworkPlay.HelperTransport.08.BoundedWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	for (const FName Name : {FName(TEXT("Prototype.Helper")), FName(TEXT("持球球员")), FName(*FString::ChrN(128, TEXT('x')))})
	{
		Payload P; P.HelperCardId = Name;
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = false;
		P.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Bounded Helper write"), Success && Bytes.Num() <= 129);
		FMemoryReader Reader(Bytes); Payload Copy; Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Bounded Helper read"), Success && !Reader.IsError());
		TestEqual(TEXT("Canonical identity roundtrip"), Copy.HelperCardId, Name);
	}
	Payload Oversize; Oversize.HelperCardId = FName(*FString::ChrN(129, TEXT('x')));
	TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = true;
	Oversize.NetSerialize(Writer, nullptr, Success);
	TestFalse(TEXT("Oversize sender fails codec"), Success);
	for (TArray<uint8> Invalid : {TArray<uint8>{129}, TArray<uint8>{2,'x'}, TArray<uint8>{1,0}, TArray<uint8>{2,0xC0,0xAF}})
	{
		FMemoryReader Reader(Invalid); Payload P; Success = true; P.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Malformed wire fails safely"), Success);
		TestTrue(TEXT("Malformed identity not adopted"), P.IsEmpty());
	}
	for (const Kind K : {Kind::RequestInitialActionPointRoll, Kind::DeployOrdinary, Kind::DeployGoalkeeper, Kind::FinishDeployment, Kind::SubmitCarrier, Kind::SubmitMarker, Kind::SubmitRunner, Kind::SubmitHelper})
	{
		for (int32 Mask = 0; Mask < 64; ++Mask)
		{
			Envelope E; E.IntentKind = K;
			if (Mask & 1) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); }
			if (Mask & 2) { E.Goalkeeper.SlotId = TEXT("Slot"); }
			if (Mask & 4) { E.Carrier.CarrierCardId = TEXT("Carrier"); }
			if (Mask & 8) { E.Marker.MarkerCardId = TEXT("Marker"); }
			if (Mask & 16) { E.Runner.RunnerCardId = TEXT("Runner"); }
			if (Mask & 32) { E.Helper.HelperCardId = TEXT("Helper"); }
			const int32 ExpectedMask = K == Kind::DeployOrdinary ? 1 : K == Kind::DeployGoalkeeper ? 2 : K == Kind::SubmitCarrier ? 4 : K == Kind::SubmitMarker ? 8 : K == Kind::SubmitRunner ? 16 : K == Kind::SubmitHelper ? 32 : 0;
			TestEqual(TEXT("Eight kinds accept exactly their own shape"), E.ValidatePayloadShape(), Mask == ExpectedMask ? Code::None : Code::InvalidPayload);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperSharedPath,
	"FMCodex.NetworkPlay.HelperTransport.09.SharedSemanticsAndGeneratedRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperSharedPath::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture Network, Local;
	if (!TestTrue(TEXT("Equivalent canonical fixtures"), Network.ReachHelper() && Local.ReachHelper())) { return false; }
	TestTrue(TEXT("Identical before State"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const auto E = Network.Request(Network.B);
	FMatchPlayAuthoritativeSubmitHelperRequest Request;
	Request.ExpectedAttackSequence = E.ExpectedAttackSequence; Request.RequestingSide = Side::PlayerB; Request.HelperCardId = E.Helper.HelperCardId;
	const auto Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitHelper, Request);
	const int32 LocalBeforeCalls = Local.Calls();
	const auto LocalResult = Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
	TestTrue(TEXT("Transport-neutral shared HostPort accepts"), LocalResult.bSuccess);
	TestEqual(TEXT("Shared local-side dispatch coordinates once"), Local.Calls(), LocalBeforeCalls + 1);
	TestTrue(TEXT("With legal Skill no internal action needed"), LocalResult.CoordinatorResult.Steps.IsEmpty());
	TestEqual(TEXT("Coordinator stops at actual player intent"), LocalResult.CoordinatorResult.StopReason, EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Network adapter accepts same canonical choice"), Network.Mode->SubmitConnectionPlayerIntent(Network.B, E).Code, Code::Accepted);
	TestTrue(TEXT("Local/Network final State identical"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const FUnchanged Frozen(Local);
	FMatchPlayPlayerIntent Wrong; Wrong.CommandKind = EMatchPlayAuthoritativeCommandKind::SubmitHelper;
	Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	TestEqual(TEXT("Canonical wrong variant rejected before Session"), Access::Runtime(*Local.Mode).SubmitPlayerIntent(Wrong).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Frozen.Verify(*this, Local);
	FString Source;
	TestTrue(TEXT("Controller source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitHelperChoice"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidHelper"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const auto Dispatch = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Helper calls generated owning RPC"), Dispatch.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation shortcut"), Dispatch.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No Host authority bypass"), Dispatch.Contains(TEXT("HasAuthority")));
	TestFalse(TEXT("No client HostPort"), Dispatch.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local host source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	const int32 LocalStart = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitHelper:"));
	const int32 LocalEnd = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::DeclineHelper:"), ESearchCase::CaseSensitive, ESearchDir::FromStart, LocalStart);
	TestTrue(TEXT("Actual Local Helper branch uses same shared port"), Source.Mid(LocalStart, LocalEnd - LocalStart).Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	return true;
}




IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexHelperContinuation,
	"FMCodex.NetworkPlay.HelperTransport.10.CanonicalAbsenceAndNextWait",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexHelperContinuation::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("NoLegalHelper"), TEXT("DeclineHelper"), TEXT("AcceptedNoLegalSkill")})
	{ Names.Add(N); Commands.Add(N); }
}
bool FFMCodexHelperContinuation::RunTest(const FString& P)
{
	using namespace FMCodexNetworkHelperTests;
	FFixture F;
	const bool NoHelper = P == TEXT("NoLegalHelper");
	const bool NoSkill = P == TEXT("AcceptedNoLegalSkill");
	if (!TestTrue(TEXT("Canonical participant setup"), F.ReachHelper(NoHelper ? 0 : 2, !NoSkill, false, !NoHelper))) { return false; }
	const FUnchanged Before(F);
	FMatchPlayPlayerIntent Intent;
	if (NoHelper)
	{
		FMatchPlayAuthoritativeSubmitRunnerRequest R;
		R.ExpectedAttackSequence = F.A->GetOwnerView().AttackSequence; R.RequestingSide = Side::PlayerA;
		R.RunnerCardId = F.A->GetOwnerView().RunnerOptions[0].Choice.RunnerCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitRunner, R);
	}
	else if (!NoSkill)
	{
		FMatchPlayAuthoritativeDeclineHelperRequest R;
		R.ExpectedAttackSequence = F.A->GetOwnerView().AttackSequence; R.RequestingSide = Side::PlayerB;
		const auto Decline = Access::Session(*F.Mode).DeclineHelper(R);
		TestTrue(TEXT("Canonical voluntary decline accepted only with legal choices"), Decline.RuntimeEnvelope.bDomainSuccess);
		const auto Result = Access::Advance(*F.Mode);
		TestTrue(TEXT("Decline reaches actual Skill wait"), Result.bSuccess && Result.Steps.IsEmpty());
		TestEqual(TEXT("AwaitingSkill after decline"), Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.SelectionStage, EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);
		TestTrue(TEXT("Helper absent after decline"), Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.ActionPreparation.HelperCardId.IsNone());
		TestEqual(TEXT("Decline consumes no entropy"), F.Entropy->Calls, Before.EntropyCalls);
		return true;
	}
	else
	{
		FMatchPlayAuthoritativeSubmitHelperRequest R;
		R.ExpectedAttackSequence = F.A->GetOwnerView().AttackSequence; R.RequestingSide = Side::PlayerB;
		R.HelperCardId = F.B->GetOwnerView().HelperOptions[0].Choice.HelperCardId;
		Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitHelper, R);
	}
	const auto Result = Access::Runtime(*F.Mode).SubmitPlayerIntent(Intent);
	TestTrue(TEXT("Canonical command accepted"), Result.bSuccess);
	TestEqual(TEXT("One Coordinator invocation"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("Exactly one required internal action"), Result.CoordinatorResult.Steps.Num(), 1);
	if (Result.CoordinatorResult.Steps.Num() == 1)
	{ TestEqual(TEXT("Actual automatic continuation"), Result.CoordinatorResult.Steps[0].CommandKind,
		NoHelper ? EMatchPlayAuthoritativeCommandKind::ResolveNoLegalHelper : EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill); }
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Exact next stage"), State.CurrentAttack.SelectionStage,
		NoHelper ? EMatchPlayCurrentAttackSelectionStage::AwaitingSkill : EMatchPlayCurrentAttackSelectionStage::None);
	TestEqual(TEXT("No-Skill closure leaves no active attack"), State.bHasCurrentAttack, NoHelper);
	TestFalse(TEXT("No next player choice fabricated"), State.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No resolution session"), State.CurrentAttack.bHasResolutionSession);
	TestEqual(TEXT("No entropy"), F.Entropy->Calls, Before.EntropyCalls);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHelperNamespace,
	"FMCodex.NetworkPlay.HelperTransport.11.EightIntentNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexHelperNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkHelperTests;
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
	FFMCodexNetworkHelperOption Helper; Helper.Choice.HelperCardId = TEXT("Card"); V.HelperOptions.Add(Helper);
	FFMCodexNetworkIntentClientState C; FFMCodexNetworkIntentLedger Ledger; Envelope E;
	for (int32 I = 1; I <= 8; ++I)
	{
		if (I == 5) { V.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection; }
		if (I == 6) { V.EntryWait = EFMCodexNetworkEntryWait::MarkerSelection; }
		if (I == 7) { V.EntryWait = EFMCodexNetworkEntryWait::RunnerSelection; }
		if (I == 8) { V.EntryWait = EFMCodexNetworkEntryWait::HelperSelection; }
		const bool Began = I == 1 ? C.Begin(V, E) : I == 2 ? C.BeginDeployment(V, Ordinary.Choice, E)
			: I == 3 ? C.BeginGoalkeeper(V, GK, E) : I == 4 ? C.BeginFinishDeployment(V, E) : I == 5 ? C.BeginCarrier(V, Carrier.Choice, E) : I == 6 ? C.BeginMarker(V, Marker.Choice, E) : I == 7 ? C.BeginRunner(V, Runner.Choice, E) : C.BeginHelper(V, Helper.Choice, E);
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
	TestEqual(TEXT("Helper huge jump denied"), Ledger.Check(V.MatchInstanceId, Huge), Code::InvalidPayload);
	E.RequestId = 9;
	TestTrue(TEXT("Huge jump did not poison normal next ID"), Ledger.Consume(V.MatchInstanceId, E));
	return true;
}


#endif
