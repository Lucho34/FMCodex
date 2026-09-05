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

struct FFMCodexNetworkCarrierTestAccess
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
		auto Entropy = MakeUnique<FFMCodexNetworkScriptedEntropy>(TArray<uint32>{3, 3, 3, 3, 3, 3, 3, 3});
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

namespace FMCodexNetworkCarrierTests
{
	using Access = FFMCodexNetworkCarrierTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	using Envelope = FFMCodexNetworkPlayerIntentEnvelope;
	using Payload = FFMCodexNetworkSubmitCarrierPayload;
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
		Envelope Request(AFMCodexNetworkMatchPlayerController* PC, Kind K = Kind::SubmitCarrier)
		{
			Envelope E; E.MatchInstanceId = Mode->GetMatchInstanceId(); E.RequestId = Next(PC)++;
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = K;
			if (K == Kind::SubmitCarrier && !Attacker()->GetOwnerView().CarrierOptions.IsEmpty())
			{
				E.Carrier = Attacker()->GetOwnerView().CarrierOptions[0].Choice;
			}
			return E;
		}
		bool Send(AFMCodexNetworkMatchPlayerController* PC, Kind K,
			const FFMCodexNetworkDeployOrdinaryPayload& Ordinary = {}, const Payload& Carrier = {})
		{
			auto& C = Client(PC); Envelope E; const auto& V = PC->GetOwnerView();
			bool Began = K == Kind::RequestInitialActionPointRoll ? C.Begin(V, E)
				: K == Kind::DeployOrdinary ? C.BeginDeployment(V, Ordinary, E)
				: K == Kind::FinishDeployment ? C.BeginFinishDeployment(V, E)
				: K == Kind::SubmitCarrier && C.BeginCarrier(V, Carrier, E);
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
		bool ReachCarrier(int32 Count = 2, bool WithMarker = true)
		{
			auto* Attack = Attacker(); auto* Defense = Defender();
			if (!Entropy || !Send(Attack, Kind::RequestInitialActionPointRoll)) { return false; }
			const auto First = Choice(Attack);
			const FString Half = First.SlotId.ToString().Contains(TEXT("NearA")) ? TEXT("NearA") : TEXT("NearB");
			const FString DefenseHalf = WithMarker ? Half : Half == TEXT("NearA") ? TEXT("NearB") : TEXT("NearA");
			if (!Send(Attack, Kind::DeployOrdinary, First)
				|| !Send(Defense, Kind::DeployOrdinary, Choice(Defense, DefenseHalf))) { return false; }
			if (Count > 1 && !Send(Attack, Kind::DeployOrdinary, Choice(Attack, Half))) { return false; }
			if (Count == 1 && !Send(Attack, Kind::FinishDeployment)) { return false; }
			if (!Send(Defense, Kind::FinishDeployment)) { return false; }
			for (int32 I = 2; I < Count; ++I)
			{
				if (!Send(Attack, Kind::DeployOrdinary, Choice(Attack, Half))) { return false; }
			}
			if (Count > 1 && !Send(Attack, Kind::FinishDeployment)) { return false; }
			return Attack->GetOwnerView().EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
				&& Attack->GetOwnerView().CarrierOptions.Num() == Count;
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

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCarrierAccepted,
	"FMCodex.NetworkPlay.CarrierTransport.01.CanonicalAcceptanceAndFreeze",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCarrierAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AFirst.First"), TEXT("AFirst.Last"), TEXT("BFirst.First"), TEXT("BFirst.Last")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexCarrierAccepted::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F(Parameter.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical deployment completion"), F.ReachCarrier())) { return false; }
	auto* PC = F.Attacker();
	const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
	TestEqual(TEXT("All canonical choices represented"), PC->GetOwnerView().CarrierOptions.Num(), Safe.SelectionOptions.Num());
	const FUnchanged Before(F);
	auto E = F.Request(PC);
	if (Parameter.EndsWith(TEXT("Last"))) { E.Carrier = PC->GetOwnerView().CarrierOptions.Last().Choice; }
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	if (!TestEqual(TEXT("Carrier accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	auto Expected = Before.State;
	Expected.CurrentAttack.ActionPreparation.CarrierCardId = E.Carrier.CarrierCardId;
	Expected.CurrentAttack.SelectionStage = EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
	TestTrue(TEXT("Only canonical Carrier freeze and selection stage changed"), SameState(Expected, After));
	TestEqual(TEXT("One Coordinator invocation"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One stable publication"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("No entropy for Carrier or stable Marker wait"), F.Entropy->Calls, Before.EntropyCalls);
	TestEqual(TEXT("Phase remains Resolution"), After.CurrentAttack.Phase, EMatchPlayCurrentAttackPhase::Resolution);
	TestTrue(TEXT("Marker not auto-selected"), After.CurrentAttack.ActionPreparation.MarkerCardId.IsNone());
	TestFalse(TEXT("No selected tactic"), After.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("No resolution session"), After.CurrentAttack.bHasResolutionSession);
	for (auto* Viewer : {F.A, F.B})
	{
		const auto& V = Viewer->GetOwnerView();
		TestEqual(TEXT("Same public selected Carrier"), V.SelectedCarrier.Choice.CarrierCardId, E.Carrier.CarrierCardId);
		TestEqual(TEXT("Same stable revision"), V.ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("Actual next wait"), V.EntryWait, EFMCodexNetworkEntryWait::MarkerSelection);
		TestEqual(TEXT("Next actor is defender"), V.ExpectedActingSide, F.Defender()->GetOwnerView().ViewerSide);
		TestTrue(TEXT("No Carrier action after freeze"), V.CarrierOptions.IsEmpty());
		TestFalse(TEXT("Deployment stays complete"), !V.bDeploymentComplete);
	}
	TestEqual(TEXT("Same public display name"), F.A->GetOwnerView().SelectedCarrier.CardLabel.ToString(),
		F.B->GetOwnerView().SelectedCarrier.CardLabel.ToString());
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCarrierRejection,
	"FMCodex.NetworkPlay.CarrierTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCarrierRejection::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("WrongSide"), TEXT("OpponentCard"), TEXT("Undeployed"), TEXT("UnknownCard"),
		TEXT("Goalkeeper"), TEXT("Empty"), TEXT("Oversize"), TEXT("WrongMatch"), TEXT("WrongSequence"),
		TEXT("OrdinaryMember"), TEXT("GoalkeeperMember"), TEXT("WrongTag"), TEXT("HugeId"),
		TEXT("ZeroId"), TEXT("ZeroSequence"), TEXT("Nonparticipant"), TEXT("BeforeCarrier"), TEXT("InternalTag")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexCarrierRejection::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("AwaitingCarrier fixture"), F.ReachCarrier())) { return false; }
	auto* Submitter = P == TEXT("WrongSide") ? F.Defender() : F.Attacker();
	auto E = F.Request(Submitter);
	Code Expected = Code::AuthorityRejected;
	const auto Safe = Access::Safe(*F.Mode, F.Attacker()->GetOwnerView().ViewerSide);
	if (P == TEXT("OpponentCard")) { E.Carrier.CarrierCardId = Safe.PlayerBCardRoster[1].CardId; }
	if (P == TEXT("Undeployed"))
	{
		for (const auto& Card : Safe.PlayerACardRoster)
		{
			if (!Card.bGoalkeeper && !Safe.DeploymentPlacements.ContainsByPredicate([&](const auto& X) { return X.CardId == Card.CardId; }))
			{ E.Carrier.CarrierCardId = Card.CardId; break; }
		}
	}
	if (P == TEXT("UnknownCard")) { E.Carrier.CarrierCardId = TEXT("Unknown.Carrier"); }
	if (P == TEXT("Goalkeeper")) { E.Carrier.CarrierCardId = FName(*Access::Session(*F.Mode).GetStateSnapshot().RuntimeState.PlayerAState.GoalkeeperCardId); }
	if (P == TEXT("Empty")) { E.Carrier = {}; Expected = Code::InvalidPayload; }
	if (P == TEXT("Oversize")) { E.Carrier.CarrierCardId = FName(*FString::ChrN(129, TEXT('x'))); Expected = Code::InvalidPayload; }
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
	if (P == TEXT("BeforeCarrier"))
	{
		FFixture Early;
		if (!TestTrue(TEXT("Initial bootstrap"), Early.Entropy != nullptr)) { return false; }
		auto EarlyE = Early.Request(Early.A); EarlyE.Carrier.CarrierCardId = TEXT("Prototype.Arsenal.GabrielMagalhaes");
		const FUnchanged Before(Early);
		TestEqual(TEXT("Early phase rejected canonically"), Early.Mode->SubmitConnectionPlayerIntent(Early.A, EarlyE).Code, Code::AuthorityRejected);
		Before.Verify(*this, Early);
		return true;
	}
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact typed rejection"), F.Mode->SubmitConnectionPlayerIntent(Submitter, E).Code, Expected);
	Before.Verify(*this, F);
	auto Valid = F.Request(F.Attacker());
	if (P == TEXT("HugeId")) { Valid.RequestId = E.RequestId = F.NextA - 2; }
	TestEqual(TEXT("Next normal Carrier still works"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), Valid).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierReplay,
	"FMCodex.NetworkPlay.CarrierTransport.03.DuplicateAndFreshRewrites",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierReplay::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Multiple Carrier choices"), F.ReachCarrier())) { return false; }
	const auto Choices = F.A->GetOwnerView().CarrierOptions;
	auto E = F.Request(F.A);
	TestEqual(TEXT("Initial selection accepted"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Exact duplicate rejected"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::DuplicateOrAlreadyResolved);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.A)++;
	TestEqual(TEXT("Fresh ID same Carrier rejected by canonical stage"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	E.RequestId = F.Next(F.A)++; E.Carrier = Choices.Last().Choice;
	TestEqual(TEXT("Fresh ID different Carrier cannot rewrite freeze"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Frozen.Verify(*this, F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierStale,
	"FMCodex.NetworkPlay.CarrierTransport.04.RealAttackStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierStale::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Attack N with no legal Marker"), F.ReachCarrier(2, false))) { return false; }
	auto Held = F.Request(F.A); --F.NextA; // Capture without sending or reserving a transport ID.
	const auto Chosen = F.A->GetOwnerView().CarrierOptions.Last().Choice;
	if (!TestTrue(TEXT("Canonical no-Marker completion"), F.Send(F.A, Kind::SubmitCarrier, {}, Chosen))) { return false; }
	TestEqual(TEXT("Actual next attack wait"), F.A->GetOwnerView().AttackSequence, Held.ExpectedAttackSequence + 1);
	if (!TestTrue(TEXT("Attack N+1 also reaches Carrier"), F.ReachCarrier())) { return false; }
	Held.RequestId = F.Next(F.A)++;
	const FUnchanged Before(F);
	TestEqual(TEXT("Old attack rejected before candidate/side gameplay"), F.Mode->SubmitConnectionPlayerIntent(F.A, Held).Code, Code::StaleAttackSequence);
	Before.Verify(*this, F);
	auto Current = F.Request(F.B);
	TestEqual(TEXT("Current attack valid Carrier still works"), F.Mode->SubmitConnectionPlayerIntent(F.B, Current).Code, Code::Accepted);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexCarrierAckOrder,
	"FMCodex.NetworkPlay.CarrierTransport.05.PendingOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexCarrierAckOrder::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AckFirst"), TEXT("ViewFirst"), TEXT("Rejected")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexCarrierAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Real five-kind client namespace setup"), F.ReachCarrier())) { return false; }
	auto& C = F.ClientA; const auto View = F.A->GetOwnerView();
	auto Choice = View.CarrierOptions[0].Choice;
	if (P == TEXT("Rejected")) { Choice.CarrierCardId = TEXT("Forged.Carrier"); }
	Envelope E;
	TestTrue(TEXT("Common pending begins"), C.BeginCarrier(View, Choice, E));
	const auto Original = E;
	TestFalse(TEXT("Second click cannot emit"), C.BeginCarrier(View, Choice, E));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierSafeProjection,
	"FMCodex.NetworkPlay.CarrierTransport.06.CompleteCanonicalCandidates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierSafeProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Four real deployed Carrier choices"), F.ReachCarrier(4))) { return false; }
	const FUnchanged Before(F);
	const auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
	const auto& View = F.A->GetOwnerView();
	TestEqual(TEXT("All four choices, not the ordinary three-option sample"), View.CarrierOptions.Num(), 4);
	TestEqual(TEXT("Full canonical set preserved"), View.CarrierOptions.Num(), Safe.SelectionOptions.Num());
	for (int32 I = 0; I < Safe.SelectionOptions.Num(); ++I)
	{
		TestEqual(TEXT("Canonical identity and order"), View.CarrierOptions[I].Choice.CarrierCardId, Safe.SelectionOptions[I].RelatedCardId);
		TestEqual(TEXT("Existing safe preferred name"), View.CarrierOptions[I].CardLabel.ToString(), Safe.SelectionOptions[I].Card.DisplayLabel);
	}
	TestTrue(TEXT("Nonacting viewer receives no actionable list"), F.B->GetOwnerView().CarrierOptions.IsEmpty());
	for (Side Viewer : {Side::PlayerA, Side::PlayerB, Side::None})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Undisclosed route has no Carrier action"), Wire.CarrierOptions.IsEmpty());
		TestTrue(TEXT("Undisclosed route has no Carrier fact"), Wire.SelectedCarrier.Choice.IsEmpty());
	}
	TestTrue(TEXT("Choose fourth legitimate option"), F.Send(F.A, Kind::SubmitCarrier, {}, View.CarrierOptions.Last().Choice));
	for (Side Viewer : {Side::PlayerA, Side::PlayerB})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(), 99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Saved Carrier remains withheld with undisclosed attack"), Wire.SelectedCarrier.Choice.IsEmpty());
	}
	TestEqual(TEXT("Projection plus selection consumes no entropy"), F.Entropy->Calls, Before.EntropyCalls);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierBound,
	"FMCodex.NetworkPlay.CarrierTransport.07.AtomicProjectionBound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierBound::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Real safe source"), F.ReachCarrier())) { return false; }
	auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
	const auto Original = Safe.SelectionOptions[0];
	Safe.SelectionOptions.Reset();
	for (int32 I = 0; I < FFMCodexNetworkClientViewSnapshot::MaxCarrierOptions; ++I)
	{
		auto O = Original; O.Id = O.RelatedCardId = FName(*FString::Printf(TEXT("Bounded.Candidate.%d"), I));
		O.Card.CardId = O.RelatedCardId; Safe.SelectionOptions.Add(O);
	}
	auto Project = [&]() { return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe, F.Mode->GetMatchInstanceId(), 99, Side::PlayerA, EFMCodexNetworkBootstrapState::MatchReady); };
	const auto Full = Project();
	TestEqual(TEXT("Complete canonical deck bound representable"), Full.CarrierOptions.Num(), 19);
	TestFalse(TEXT("Valid set is available"), Full.bCarrierOptionsUnavailable);
	const auto Last = Safe.SelectionOptions.Last();
	Safe.SelectionOptions.Add(Original);
	auto Bad = Project();
	TestTrue(TEXT("Overflow fails explicitly"), Bad.bCarrierOptionsUnavailable);
	TestTrue(TEXT("No silent truncation"), Bad.CarrierOptions.IsEmpty());
	Safe.SelectionOptions.Pop();
	Safe.SelectionOptions.Last().RelatedCardId = FName(*FString::ChrN(129, TEXT('x')));
	Bad = Project();
	TestTrue(TEXT("Unencodable candidate invalidates whole set"), Bad.bCarrierOptionsUnavailable && Bad.CarrierOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions.Last().RelatedCardId = Safe.SelectionOptions[0].RelatedCardId;
	Bad = Project();
	TestTrue(TEXT("Duplicate candidate fails rather than silently changing choices"), Bad.bCarrierOptionsUnavailable && Bad.CarrierOptions.IsEmpty());
	Safe.SelectionOptions.Last() = Last;
	Safe.SelectionOptions[0].bHasCard = false;
	TestEqual(TEXT("Missing display data uses generic name"), Project().CarrierOptions[0].CardLabel.ToString(), FString(TEXT("球员")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierWire,
	"FMCodex.NetworkPlay.CarrierTransport.08.BoundedWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	for (const FName Name : {FName(TEXT("Prototype.Carrier")), FName(TEXT("持球球员")), FName(*FString::ChrN(128, TEXT('x')))})
	{
		Payload P; P.CarrierCardId = Name;
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = false;
		P.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Bounded Carrier write"), Success && Bytes.Num() <= 129);
		FMemoryReader Reader(Bytes); Payload Copy; Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Bounded Carrier read"), Success && !Reader.IsError());
		TestEqual(TEXT("Canonical identity roundtrip"), Copy.CarrierCardId, Name);
	}
	Payload Oversize; Oversize.CarrierCardId = FName(*FString::ChrN(129, TEXT('x')));
	TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = true;
	Oversize.NetSerialize(Writer, nullptr, Success);
	TestFalse(TEXT("Oversize sender fails codec"), Success);
	for (TArray<uint8> Invalid : {TArray<uint8>{129}, TArray<uint8>{2,'x'}, TArray<uint8>{1,0}, TArray<uint8>{2,0xC0,0xAF}})
	{
		FMemoryReader Reader(Invalid); Payload P; Success = true; P.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Malformed wire fails safely"), Success);
		TestTrue(TEXT("Malformed identity not adopted"), P.IsEmpty());
	}
	for (const Kind K : {Kind::RequestInitialActionPointRoll, Kind::DeployOrdinary, Kind::DeployGoalkeeper, Kind::FinishDeployment, Kind::SubmitCarrier})
	{
		for (int32 Mask = 0; Mask < 8; ++Mask)
		{
			Envelope E; E.IntentKind = K;
			if (Mask & 1) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); }
			if (Mask & 2) { E.Goalkeeper.SlotId = TEXT("Slot"); }
			if (Mask & 4) { E.Carrier.CarrierCardId = TEXT("Carrier"); }
			const int32 ExpectedMask = K == Kind::DeployOrdinary ? 1 : K == Kind::DeployGoalkeeper ? 2 : K == Kind::SubmitCarrier ? 4 : 0;
			TestEqual(TEXT("Five kinds accept exactly their own shape"), E.ValidatePayloadShape(), Mask == ExpectedMask ? Code::None : Code::InvalidPayload);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierSharedPath,
	"FMCodex.NetworkPlay.CarrierTransport.09.SharedSemanticsAndGeneratedRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierSharedPath::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture Network, Local;
	if (!TestTrue(TEXT("Equivalent canonical fixtures"), Network.ReachCarrier() && Local.ReachCarrier())) { return false; }
	TestTrue(TEXT("Identical before State"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const auto E = Network.Request(Network.A);
	FMatchPlayAuthoritativeSubmitCarrierRequest Request;
	Request.ExpectedAttackSequence = E.ExpectedAttackSequence; Request.RequestingSide = Side::PlayerA; Request.CarrierCardId = E.Carrier.CarrierCardId;
	const auto Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitCarrier, Request);
	const int32 LocalBeforeCalls = Local.Calls();
	const auto LocalResult = Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
	TestTrue(TEXT("Transport-neutral shared HostPort accepts"), LocalResult.bSuccess);
	TestEqual(TEXT("Shared local-side dispatch coordinates once"), Local.Calls(), LocalBeforeCalls + 1);
	TestTrue(TEXT("With legal Marker no internal action needed"), LocalResult.CoordinatorResult.Steps.IsEmpty());
	TestEqual(TEXT("Coordinator stops at actual player intent"), LocalResult.CoordinatorResult.StopReason, EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Network adapter accepts same canonical choice"), Network.Mode->SubmitConnectionPlayerIntent(Network.A, E).Code, Code::Accepted);
	TestTrue(TEXT("Local/Network final State identical"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const FUnchanged Frozen(Local);
	FMatchPlayPlayerIntent Wrong; Wrong.CommandKind = EMatchPlayAuthoritativeCommandKind::SubmitCarrier;
	Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	TestEqual(TEXT("Canonical wrong variant rejected before Session"), Access::Runtime(*Local.Mode).SubmitPlayerIntent(Wrong).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Frozen.Verify(*this, Local);
	FString Source;
	TestTrue(TEXT("Controller source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitCarrierChoice"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidCarrier"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const auto Dispatch = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Carrier calls generated owning RPC"), Dispatch.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation shortcut"), Dispatch.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No Host authority bypass"), Dispatch.Contains(TEXT("HasAuthority")));
	TestFalse(TEXT("No client HostPort"), Dispatch.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local host source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	const int32 LocalStart = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitCarrier:"));
	const int32 LocalEnd = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitMarker:"), ESearchCase::CaseSensitive, ESearchDir::FromStart, LocalStart);
	TestTrue(TEXT("Actual Local Carrier branch uses same shared port"), Source.Mid(LocalStart, LocalEnd - LocalStart).Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierNoMarker,
	"FMCodex.NetworkPlay.CarrierTransport.10.CanonicalNoMarkerContinuation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierNoMarker::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFixture F;
	if (!TestTrue(TEXT("Legal Carrier with no legal Marker"), F.ReachCarrier(2, false))) { return false; }
	const FUnchanged Before(F);
	FMatchPlayAuthoritativeSubmitCarrierRequest R;
	R.ExpectedAttackSequence = F.A->GetOwnerView().AttackSequence; R.RequestingSide = Side::PlayerA;
	R.CarrierCardId = F.A->GetOwnerView().CarrierOptions[0].Choice.CarrierCardId;
	const auto Result = Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitCarrier, R));
	TestTrue(TEXT("Tactically different outcome does not make Carrier illegal"), Result.bSuccess);
	TestEqual(TEXT("One Coordinator pass"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("Exactly canonical no-legal-Marker internal action"), Result.CoordinatorResult.Steps.Num(), 1);
	if (!Result.CoordinatorResult.Steps.IsEmpty())
	{
		TestEqual(TEXT("No simulated Marker choice"), Result.CoordinatorResult.Steps[0].CommandKind, EMatchPlayAuthoritativeCommandKind::ResolveNoLegalMarker);
	}
	TestEqual(TEXT("Existing automatic completion consumes no entropy"), F.Entropy->Calls, Before.EntropyCalls);
	Access::Publish(*F.Mode);
	TestEqual(TEXT("Canonical next attack Full D12 wait preserved"), F.B->GetOwnerView().EntryWait, EFMCodexNetworkEntryWait::InitialD12);
	TestTrue(TEXT("Previous attack Carrier not copied into new attack"), F.B->GetOwnerView().SelectedCarrier.Choice.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCarrierNamespace,
	"FMCodex.NetworkPlay.CarrierTransport.11.FiveIntentNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCarrierNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkCarrierTests;
	FFMCodexNetworkClientViewSnapshot V; V.MatchInstanceId = FGuid::NewGuid(); V.ViewRevision = 1;
	V.bMatchInitialized = true; V.BootstrapState = EFMCodexNetworkBootstrapState::MatchReady;
	V.ViewerSide = V.ExpectedActingSide = Side::PlayerA; V.AttackSequence = 1;
	V.InteractionState = EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint;
	V.EntryBranch = EFMCodexNetworkEntryBranch::Ordinary; V.EntryWait = EFMCodexNetworkEntryWait::Deployment;
	V.bCanDeployGoalkeeper = V.bCanFinishDeployment = true;
	FFMCodexNetworkDeploymentOption Ordinary; Ordinary.Choice.CardId = TEXT("Card"); Ordinary.Choice.SlotId = TEXT("Slot"); V.DeploymentOptions.Add(Ordinary);
	FFMCodexNetworkDeployGoalkeeperPayload GK; GK.SlotId = TEXT("Slot");
	FFMCodexNetworkCarrierOption Carrier; Carrier.Choice.CarrierCardId = TEXT("Card"); V.CarrierOptions.Add(Carrier);
	FFMCodexNetworkIntentClientState C; FFMCodexNetworkIntentLedger Ledger; Envelope E;
	for (int32 I = 1; I <= 5; ++I)
	{
		if (I == 5) { V.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection; }
		const bool Began = I == 1 ? C.Begin(V, E) : I == 2 ? C.BeginDeployment(V, Ordinary.Choice, E)
			: I == 3 ? C.BeginGoalkeeper(V, GK, E) : I == 4 ? C.BeginFinishDeployment(V, E) : C.BeginCarrier(V, Carrier.Choice, E);
		TestTrue(TEXT("Every intent uses common begin"), Began);
		TestEqual(TEXT("Single monotonically increasing namespace"), E.RequestId, static_cast<int64>(I));
		TestTrue(TEXT("One common ledger"), Ledger.Consume(V.MatchInstanceId, E));
		auto OtherKind = E; OtherKind.IntentKind = I == 5 ? Kind::FinishDeployment : Kind::SubmitCarrier;
		TestEqual(TEXT("Kind cannot reuse prior ID"), Ledger.Check(V.MatchInstanceId, OtherKind), Code::DuplicateOrAlreadyResolved);
		FFMCodexNetworkPlayerIntentAck Ack; Ack.MatchInstanceId = E.MatchInstanceId; Ack.RequestId = E.RequestId; Ack.Code = Code::Accepted;
		Ack.ViewRevision = ++V.ViewRevision; C.ObserveAck(Ack); C.ObserveView(V);
		TestFalse(TEXT("Generic completion"), C.IsPending());
	}
	auto Huge = E; Huge.RequestId = MAX_int64;
	TestEqual(TEXT("Carrier huge jump denied"), Ledger.Check(V.MatchInstanceId, Huge), Code::InvalidPayload);
	E.RequestId = 6;
	TestTrue(TEXT("Huge jump did not poison normal next ID"), Ledger.Consume(V.MatchInstanceId, E));
	return true;
}

#endif
