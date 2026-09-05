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

struct FFMCodexNetworkDeploymentTestAccess
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
		auto Entropy = MakeUnique<FFMCodexNetworkScriptedEntropy>(TArray<uint32>{3, 3, 3, 3});
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
	static FFMCodexLocalMatchInteractionView Safe(AFMCodexNetworkMatchGameMode& Mode, EInitialTurnOrderPlayer Side)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = true;
		return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Session(Mode).GetStateSnapshot(), Mode.MatchRuntime->SkillRuleSet, Side, Disclosure);
	}
};

namespace FMCodexNetworkDeploymentTests
{
	using Access = FFMCodexNetworkDeploymentTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	struct FFixture
	{
		UWorld* World;
		AFMCodexNetworkMatchGameMode* Mode;
		AFMCodexNetworkMatchPlayerController* A;
		AFMCodexNetworkMatchPlayerController* B;
		FFMCodexNetworkScriptedEntropy* Entropy;
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
		AFMCodexNetworkMatchPlayerController* Acting() const
		{
			return A->GetOwnerView().ExpectedActingSide == Side::PlayerA ? A : B;
		}
		FFMCodexNetworkPlayerIntentEnvelope Request(int64 Id, Kind IntentKind = Kind::DeployOrdinary) const
		{
			FFMCodexNetworkPlayerIntentEnvelope E;
			E.MatchInstanceId = Mode->GetMatchInstanceId();
			E.RequestId = Id;
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence;
			E.IntentKind = IntentKind;
			return E;
		}
		bool Enter() { return Entropy && Mode->SubmitConnectionPlayerIntent(Acting(), Request(1, Kind::RequestInitialActionPointRoll)).Code == Code::Accepted; }
		FFMCodexNetworkPlayerIntentEnvelope Deployment(int64 Id, bool Alternate = false)
		{
			auto E = Request(Id);
			const auto View = Access::Safe(*Mode, Acting()->GetOwnerView().ViewerSide);
			for (const auto& Option : View.DeploymentOptions)
			{
				if (Option.bGoalkeeper) { continue; }
				E.Deployment.CardId = Option.CardId;
				E.Deployment.SlotId = Option.SlotId;
				if (!Alternate) { break; }
			}
			return E;
		}
	};
	bool SameState(const FMatchPlayState& A, const FMatchPlayState& B)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(&A, &B, 0);
	}
	struct FUnchanged
	{
		FMatchPlayState State;
		int32 Revision, EntropyCalls, EntryCalls, D12Calls;
		explicit FUnchanged(FFixture& F)
			: State(Access::Session(*F.Mode).GetStateSnapshot()), Revision(Access::Revision(*F.Mode)),
			EntropyCalls(F.Entropy->Calls), EntryCalls(Access::Runtime(*F.Mode).GetEntryProviderInvocationCount()),
			D12Calls(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()) {}
		void Verify(FAutomationTestBase& Test, FFixture& F) const
		{
			Test.TestTrue(TEXT("Entire authoritative State unchanged"), SameState(State, Access::Session(*F.Mode).GetStateSnapshot()));
			Test.TestEqual(TEXT("No publication"), Access::Revision(*F.Mode), Revision);
			Test.TestEqual(TEXT("All-provider entropy unchanged"), F.Entropy->Calls, EntropyCalls);
			Test.TestEqual(TEXT("Entry calls unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), EntryCalls);
			Test.TestEqual(TEXT("D12 calls unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12Calls);
		}
	};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexDeploymentAcceptTest,
	"FMCodex.NetworkPlay.DeploymentTransport.01.AcceptedSidesAndAlternateChoices",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexDeploymentAcceptTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("A"), TEXT("B"), TEXT("AAlternate"), TEXT("BAlternate")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexDeploymentAcceptTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFixture F(Parameters.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical D12=4 enters deployment"), F.Enter())) { return false; }
	const bool Alternate = Parameters.Contains(TEXT("Alternate"));
	auto* PC = F.Acting();
	auto E = F.Deployment(2, Alternate);
	const FUnchanged Before(F);
	const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
	TestTrue(TEXT("Choices offered"), !PC->GetOwnerView().DeploymentOptions.IsEmpty());
	TestTrue(TEXT("Projection cap"), PC->GetOwnerView().DeploymentOptions.Num() <= FFMCodexNetworkClientViewSnapshot::MaxDeploymentOptions);
	if (Alternate)
	{
		TestTrue(TEXT("Alternate choice is outside the advertised subset"),
			!PC->GetOwnerView().DeploymentOptions.ContainsByPredicate([&](const auto& O)
			{ return O.Choice.CardId == E.Deployment.CardId && O.Choice.SlotId == E.Deployment.SlotId; }));
	}
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	if (!TestEqual(TEXT("Registry-owned deployment accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Exactly one placement"), State.CurrentAttack.DeploymentPlacements.Num(), 1);
	TestEqual(TEXT("State card equals actual submitted identity"), State.CurrentAttack.DeploymentPlacements[0].CardId, E.Deployment.CardId);
	TestEqual(TEXT("State slot equals actual submitted identity"), State.CurrentAttack.DeploymentPlacements[0].SlotId, E.Deployment.SlotId);
	TestEqual(TEXT("One stable publication"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("No gameplay entropy for accepted deployment"), F.Entropy->Calls, Before.EntropyCalls);
	TestEqual(TEXT("No D12 or entry draw"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), Before.EntryCalls);
	// The canonical writer only adds placement and rotates legal side; no cards/formula/score/lifecycle change.
	auto Expected = Before.State;
	Expected.CurrentAttack.DeploymentPlacements = State.CurrentAttack.DeploymentPlacements;
	Expected.CurrentAttack.CurrentLegalDeploymentSide = State.CurrentAttack.CurrentLegalDeploymentSide;
	TestTrue(TEXT("Only canonical placement and turn rotation changed"), SameState(Expected, State));
	for (auto* Viewer : {F.A, F.B})
	{
		const auto& V = Viewer->GetOwnerView();
		TestEqual(TEXT("Both owner-safe views same revision"), V.ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("Both public counts"), V.DeploymentCount, 1);
		TestEqual(TEXT("Public placement card"), V.LastDeployment.Placement.Choice.CardId, E.Deployment.CardId);
		TestEqual(TEXT("Public placement slot"), V.LastDeployment.Placement.Choice.SlotId, E.Deployment.SlotId);
		TestEqual(TEXT("Public side"), V.LastDeployment.Side, PC->GetOwnerView().ViewerSide);
		TestEqual(TEXT("Expected side is canonical deployment side"), V.ExpectedActingSide, State.CurrentAttack.CurrentLegalDeploymentSide);
		TestEqual(TEXT("Disclosed D12 retained"), V.DisclosedInitialD12, 4);
		if (V.ViewerSide != V.ExpectedActingSide)
		{ TestTrue(TEXT("Nonacting owner gets no legal choices"), V.DeploymentOptions.IsEmpty()); }
	}
	TestEqual(TEXT("Same public preferred name"), F.A->GetOwnerView().LastDeployment.Placement.CardLabel.ToString(),
		F.B->GetOwnerView().LastDeployment.Placement.CardLabel.ToString());
	TestEqual(TEXT("Same public slot label on both viewers"), F.A->GetOwnerView().LastDeployment.Placement.SlotLabel.ToString(),
		F.B->GetOwnerView().LastDeployment.Placement.SlotLabel.ToString());
	const FUnchanged After(F);
	TestEqual(TEXT("Exact request replay refused"), F.Mode->SubmitConnectionPlayerIntent(PC, E).Code, Code::DuplicateOrAlreadyResolved);
	After.Verify(*this, F);
	// Defender must be able to deploy next while the attacker remains unchanged.
	const auto Next = F.Deployment(1, true);
	TestEqual(TEXT("Defender-side deployment is not blocked by Full D12 attacker gate"),
		F.Mode->SubmitConnectionPlayerIntent(F.Acting(), Next).Code, Code::Accepted);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexDeploymentRejectTest,
	"FMCodex.NetworkPlay.DeploymentTransport.02.RejectionMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexDeploymentRejectTest::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : { TEXT("WrongSide"), TEXT("OpponentCard"), TEXT("NonexistentCard"),
		TEXT("UnknownSlot"), TEXT("IllegalSlot"), TEXT("EmptyCard"), TEXT("EmptySlot"), TEXT("OversizeIdentity"),
		TEXT("WrongMatch"), TEXT("StaleSequence"), TEXT("KindPayloadMismatch"), TEXT("UnknownKind"),
		TEXT("HugeRequest"), TEXT("Nonparticipant"), TEXT("ZeroRequest"), TEXT("ZeroSequence") })
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexDeploymentRejectTest::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFixture F;
	if (!TestTrue(TEXT("Deployment ready"), F.Enter())) { return false; }
	auto E = F.Deployment(2);
	auto* PC = F.A;
	Code Expected = Code::AuthorityRejected;
	if (Parameter == TEXT("WrongSide")) { PC = F.B; Expected = Code::WrongSide; }
	else if (Parameter == TEXT("OpponentCard"))
	{
		const auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
		for (const auto& Card : Safe.PlayerBCardRoster)
		{ if (!Card.bGoalkeeper && Card.bAvailable) { E.Deployment.CardId = Card.CardId; break; } }
		TestNotEqual(TEXT("Opponent identity selected"), E.Deployment.CardId, F.Deployment(2).Deployment.CardId);
	}
	else if (Parameter == TEXT("NonexistentCard")) { E.Deployment.CardId = TEXT("NoSuchPlayer"); }
	else if (Parameter == TEXT("UnknownSlot")) { E.Deployment.SlotId = TEXT("NoSuchSlot"); }
	else if (Parameter == TEXT("IllegalSlot"))
	{
		bool Found = false;
		const auto Safe = Access::Safe(*F.Mode, Side::PlayerA);
		for (const auto& Group : Safe.DeploymentGroups)
		{
			if (Group.bGoalkeeper) { continue; }
			for (const auto& Region : Safe.PitchRegions)
			{
				for (const auto& Slot : Region.Slots)
				{
					if (!Group.LegalSlotIds.Contains(Slot.SlotId))
					{ E.Deployment.CardId = Group.CardId; E.Deployment.SlotId = Slot.SlotId; Found = true; break; }
				}
				if (Found) { break; }
			}
			if (Found) { break; }
		}
		if (!TestTrue(TEXT("Found a real canonical slot illegal for this card"), Found)) { return false; }
	}
	else if (Parameter == TEXT("EmptyCard")) { E.Deployment.CardId = NAME_None; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("EmptySlot")) { E.Deployment.SlotId = NAME_None; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("OversizeIdentity"))
	{ E.Deployment.CardId = FName(*FString::ChrN(129, TEXT('x'))); Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("WrongMatch")) { E.MatchInstanceId = FGuid::NewGuid(); Expected = Code::MatchMismatch; }
	else if (Parameter == TEXT("StaleSequence")) { ++E.ExpectedAttackSequence; Expected = Code::StaleAttackSequence; }
	else if (Parameter == TEXT("KindPayloadMismatch")) { E.IntentKind = Kind::RequestInitialActionPointRoll; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("UnknownKind")) { E.IntentKind = static_cast<Kind>(255); Expected = Code::NotPlayerIntent; }
	else if (Parameter == TEXT("HugeRequest")) { E.RequestId = MAX_int64; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("Nonparticipant")) { PC = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(); Expected = Code::NotParticipant; }
	else if (Parameter == TEXT("ZeroRequest")) { E.RequestId = 0; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("ZeroSequence")) { E.ExpectedAttackSequence = 0; Expected = Code::InvalidPayload; }
	const FUnchanged Before(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	TestEqual(Parameter, Ack.Code, Expected);
	TestEqual(TEXT("ACK request correlation"), Ack.RequestId, E.RequestId);
	TestEqual(TEXT("ACK submitted match correlation"), Ack.MatchInstanceId, E.MatchInstanceId);
	TestEqual(TEXT("Reject receipt has unchanged revision"), Ack.ViewRevision, Before.Revision);
	Before.Verify(*this, F);
	const auto Recovery = F.Deployment(Parameter == TEXT("HugeRequest") ? 2 : 3, true);
	TestEqual(TEXT("Next ordinary request succeeds, rejection cannot poison connection"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, Recovery).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentReplayTest,
	"FMCodex.NetworkPlay.DeploymentTransport.03.GameplayReplayAndSharedIdNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentReplayTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFixture F;
	if (!TestTrue(TEXT("Entry accepted with ID 1"), F.Enter())) { return false; }
	auto Reused = F.Deployment(1);
	const FUnchanged Before(F);
	TestEqual(TEXT("Full D12 ID cannot be reused for deployment"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, Reused).Code, Code::DuplicateOrAlreadyResolved);
	Before.Verify(*this, F);
	auto First = F.Deployment(2);
	TestEqual(TEXT("A deploys"), F.Mode->SubmitConnectionPlayerIntent(F.A, First).Code, Code::Accepted);
	auto Other = F.Deployment(1, true);
	TestEqual(TEXT("B deploys on its separate connection namespace"), F.Mode->SubmitConnectionPlayerIntent(F.B, Other).Code, Code::Accepted);
	const FUnchanged After(F);
	First.RequestId = 3;
	TestEqual(TEXT("New ID repeated card/slot reaches canonical rejection"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, First).Code, Code::AuthorityRejected);
	After.Verify(*this, F);
	auto Occupied = F.Deployment(4, true); Occupied.Deployment.SlotId = First.Deployment.SlotId;
	TestNotEqual(TEXT("Another card into already occupied slot"), Occupied.Deployment.CardId, First.Deployment.CardId);
	TestEqual(TEXT("Occupied slot canonical rejection"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, Occupied).Code, Code::AuthorityRejected);
	After.Verify(*this, F);
	TestEqual(TEXT("Normal alternate remains legal"), F.Mode->SubmitConnectionPlayerIntent(F.A, F.Deployment(5, true)).Code, Code::Accepted);
	auto FullD12 = F.Request(5, Kind::RequestInitialActionPointRoll);
	TestEqual(TEXT("Deployment ID cannot be reused for Full D12"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, FullD12).Code, Code::DuplicateOrAlreadyResolved);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentStaleTest,
	"FMCodex.NetworkPlay.DeploymentTransport.04.StaleDeploymentIntoNextDeployment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentStaleTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFixture F;
	if (!TestTrue(TEXT("Attack N deployment"), F.Enter())) { return false; }
	auto Delayed = F.Deployment(2); // A real legal N choice held, never sent.
	auto& Session = Access::Session(*F.Mode);
	const int64 N = Delayed.ExpectedAttackSequence;
	TestTrue(TEXT("Server fixture finishes A without placements"), Session.FinishDeployment(N, Side::PlayerA).RuntimeEnvelope.bDomainSuccess);
	TestTrue(TEXT("Server fixture finishes B without placements"), Session.FinishDeployment(N, Side::PlayerB).RuntimeEnvelope.bDomainSuccess);
	TestTrue(TEXT("Canonical no-carrier completion and coordinator"), Access::Advance(*F.Mode).bSuccess);
	Access::Publish(*F.Mode);
	TestEqual(TEXT("Next Full D12 remains manual"), F.B->GetOwnerView().InteractionState,
		EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint);
	TestEqual(TEXT("B enters N+1 through existing Full D12 boundary"),
		F.Mode->SubmitConnectionPlayerIntent(F.B, F.Request(1, Kind::RequestInitialActionPointRoll)).Code, Code::Accepted);
	const auto NextState = Session.GetStateSnapshot();
	TestTrue(TEXT("N+1 has a real live deployment phase"), NextState.bHasCurrentAttack
		&& NextState.CurrentAttack.AttackSequence == N + 1
		&& NextState.CurrentAttack.Phase == EMatchPlayCurrentAttackPhase::Deployment);
	const FUnchanged Before(F);
	TestEqual(TEXT("Fresh-ID delayed N deployment is stale, before wrong-side preflight"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, Delayed).Code, Code::StaleAttackSequence);
	Before.Verify(*this, F);
	TestEqual(TEXT("N+1 correct player remains able to deploy"), F.Mode->SubmitConnectionPlayerIntent(F.B, F.Deployment(2, true)).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentAckTest,
	"FMCodex.NetworkPlay.DeploymentTransport.05.AckViewOrdersAndPending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentAckTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentTests;
	for (const bool AckFirst : {true, false})
	{
		FFixture F;
		FFMCodexNetworkIntentClientState Client;
		FFMCodexNetworkPlayerIntentEnvelope Entry;
		TestTrue(TEXT("Initial request uses common client state"), Client.Begin(F.A->GetOwnerView(), Entry));
		const auto EntryAck = F.Mode->SubmitConnectionPlayerIntent(F.A, Entry);
		Client.ObserveAck(EntryAck); Client.ObserveView(F.A->GetOwnerView());
		const auto Before = F.A->GetOwnerView();
		if (!TestFalse(TEXT("Entry pending released"), Client.IsPending())
			|| !TestTrue(TEXT("Deployment options available"), !Before.DeploymentOptions.IsEmpty())) { return false; }
		FFMCodexNetworkPlayerIntentEnvelope E;
		TestTrue(TEXT("Deploy begins"), Client.BeginDeployment(Before, Before.DeploymentOptions[0].Choice, E));
		TestEqual(TEXT("Same monotonic ID sequence after Full D12"), E.RequestId, Entry.RequestId + 1);
		const auto Original = E;
		TestFalse(TEXT("Double click blocked"), Client.BeginDeployment(Before, Before.DeploymentOptions[0].Choice, E));
		TestEqual(TEXT("Failed begin preserves existing envelope"), E.RequestId, Original.RequestId);
		TestFalse(TEXT("Pending deployment also blocks Full D12"), Client.Begin(Before, E));
		const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
		auto Unrelated = Ack; ++Unrelated.RequestId;
		TestFalse(TEXT("Unrelated ACK ignored"), Client.ObserveAck(Unrelated));
		if (AckFirst)
		{
			TestTrue(TEXT("Accepted ACK correlated"), Client.ObserveAck(Ack));
			TestTrue(TEXT("ACK alone does not finish pending"), Client.IsPending());
			Client.ObserveView(Before);
			TestTrue(TEXT("Old view cannot satisfy accepted revision"), Client.IsPending());
			Client.ObserveView(F.A->GetOwnerView());
		}
		else
		{
			Client.ObserveView(F.A->GetOwnerView());
			TestTrue(TEXT("New view alone does not finish pending"), Client.IsPending());
			TestTrue(TEXT("Delayed ACK correlated"), Client.ObserveAck(Ack));
		}
		TestFalse(TEXT("Both signals release pending"), Client.IsPending());
		TestFalse(TEXT("Duplicate ACK ignored"), Client.ObserveAck(Ack));
		TestFalse(TEXT("Old actionable snapshot rejected after new view"),
			Client.BeginDeployment(Before, Before.DeploymentOptions[0].Choice, E));
	}
	FFixture F;
	if (!TestTrue(TEXT("Rejection fixture"), F.Enter())) { return false; }
	FFMCodexNetworkIntentClientState Client;
	FFMCodexNetworkPlayerIntentEnvelope E;
	auto Choice = F.A->GetOwnerView().DeploymentOptions[0].Choice;
	Choice.CardId = TEXT("NoSuchCard");
	TestTrue(TEXT("Client correlation does not decide card legality"), Client.BeginDeployment(F.A->GetOwnerView(), Choice, E));
	// This test client starts after entry; use server-valid next ID in a fresh correlation state.
	// Burn client ID 1 by its duplicate receipt, then retry the actual forged card as ID 2.
	auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
	TestEqual(TEXT("Common namespace duplicate"), Ack.Code, Code::DuplicateOrAlreadyResolved);
	Client.ObserveAck(Ack);
	TestTrue(TEXT("Next retry"), Client.BeginDeployment(F.A->GetOwnerView(), Choice, E));
	Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
	TestEqual(TEXT("Canonical reject"), Ack.Code, Code::AuthorityRejected);
	TestTrue(TEXT("Typed rejection correlates"), Client.ObserveAck(Ack));
	TestFalse(TEXT("Rejection releases pending without any view update"), Client.IsPending());
	TestEqual(TEXT("Rejected view remains unchanged"), F.A->GetOwnerView().DeploymentCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentWireTest,
	"FMCodex.NetworkPlay.DeploymentTransport.06.BoundedCanonicalWire",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentWireTest::RunTest(const FString&)
{
	using Payload = FFMCodexNetworkDeployOrdinaryPayload;
	TestTrue(TEXT("Custom bounded network serializer registered"), Payload::StaticStruct()->GetCppStructOps()->HasNetSerializer());
	for (const FString& Name : {FString(TEXT("Prototype.Arsenal.BukayoSaka")), FString(TEXT("球员卡_9")), FString::ChrN(128, TEXT('x')), FString()})
	{
		Payload Original;
		Original.CardId = Name.IsEmpty() ? NAME_None : FName(*Name);
		Original.SlotId = Name.IsEmpty() ? NAME_None : FName(TEXT("Slot_NearA_2"));
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes);
		bool Success = false; Original.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Serialize canonical bounded identity"), Success && !Writer.IsError());
		TestTrue(TEXT("Hard payload wire cap is 258 bytes"), Bytes.Num() <= 2 * (Payload::MaxIdentityUtf8Bytes + 1));
		FMemoryReader Reader(Bytes); Payload Copy;
		Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Valid wire decoded"), Success && !Reader.IsError());
		TestEqual(TEXT("Canonical CardId roundtrip"), Copy.CardId, Original.CardId);
		TestEqual(TEXT("Canonical SlotId roundtrip"), Copy.SlotId, Original.SlotId);
	}
	Payload TooLong; TooLong.CardId = FName(*FString::ChrN(129, TEXT('x'))); TooLong.SlotId = TEXT("Slot");
	TestFalse(TEXT("Shape rejects >128 bytes"), TooLong.IsValidShape());
	TArray<uint8> Out; FMemoryWriter Writer(Out); bool Success = true;
	TooLong.NetSerialize(Writer, nullptr, Success);
	TestFalse(TEXT("Oversize cannot leave sender serializer"), Success);
	for (TArray<uint8> Bad : {TArray<uint8>{129}, TArray<uint8>{3, 'a'}, TArray<uint8>{1, 0, 0},
		TArray<uint8>{2, 0xC0, 0xAF, 0}})
	{
		FMemoryReader Reader(Bad); Payload Copy; Success = true;
		Copy.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Oversize/truncated/NUL/invalid UTF8 rejected before FName construction"), Success);
		TestTrue(TEXT("Malformed wire marks archive failure"), Reader.IsError());
		TestTrue(TEXT("No invalid received card identity"), Copy.CardId.IsNone());
	}
	int32 Fields = 0;
	for (TFieldIterator<FProperty> It(Payload::StaticStruct()); It; ++It)
	{
		++Fields;
		TestTrue(TEXT("Only canonical FName fields on payload"), CastField<FNameProperty>(*It) != nullptr);
		TestTrue(TEXT("No Side/provider/roll/actor field"), It->GetFName() == TEXT("CardId") || It->GetFName() == TEXT("SlotId"));
	}
	TestEqual(TEXT("Exactly two payload fields"), Fields, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentIdWindowTest,
	"FMCodex.NetworkPlay.DeploymentTransport.07.IdWindowBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentIdWindowTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFMCodexNetworkIntentLedger L;
	auto Match = FGuid::NewGuid();
	FFMCodexNetworkPlayerIntentEnvelope E; E.MatchInstanceId = Match;
	E.RequestId = MAX_int64;
	TestEqual(TEXT("Huge first ID invalid"), L.Check(Match, E), Code::InvalidPayload);
	TestFalse(TEXT("Huge cannot consume"), L.Consume(Match, E));
	E.RequestId = 1;
	TestTrue(TEXT("Normal first ID after huge accepted"), L.Consume(Match, E));
	E.RequestId = 1 + FFMCodexNetworkIntentLedger::MaxForwardDelta + 1;
	TestEqual(TEXT("Just beyond window denied"), L.Check(Match, E), Code::InvalidPayload);
	TestFalse(TEXT("Outside window no mutation"), L.Consume(Match, E));
	--E.RequestId;
	TestTrue(TEXT("Inclusive window boundary accepted"), L.Consume(Match, E));
	TestEqual(TEXT("Same ID duplicate"), L.Check(Match, E), Code::DuplicateOrAlreadyResolved);
	E.RequestId = MAX_int64;
	TestFalse(TEXT("Overflow-sized jump safely denied"), L.Consume(Match, E));
	E.RequestId = 2 + FFMCodexNetworkIntentLedger::MaxForwardDelta;
	TestTrue(TEXT("Next normal request after high-water jump denial"), L.Consume(Match, E));
	TestTrue(TEXT("Constant memory, no ID set growth"), sizeof(L) <= 32);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeploymentPortTest,
	"FMCodex.NetworkPlay.DeploymentTransport.08.CommonSpecificSplitAndSharedPort",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDeploymentPortTest::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentTests;
	FFixture F;
	// Shape is independent of phase, side and current attacker.
	auto E = F.Request(1);
	E.Deployment.CardId = TEXT("AnyBoundedCard"); E.Deployment.SlotId = TEXT("AnyBoundedSlot");
	TestEqual(TEXT("Deployment-shaped request passes common shape before Full D12"), E.ValidatePayloadShape(), Code::None);
	const FUnchanged Before(F);
	TestEqual(TEXT("Deployment-specific phase denies before initial D12"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::InvalidPhase);
	Before.Verify(*this, F);
	TestEqual(TEXT("Full D12 still succeeds through its own preflight"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(2, Kind::RequestInitialActionPointRoll)).Code, Code::Accepted);
	FMatchPlayPlayerIntent Mismatch;
	Mismatch.CommandKind = EMatchPlayAuthoritativeCommandKind::DeployOrdinary;
	Mismatch.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	const FUnchanged Active(F);
	TestEqual(TEXT("HostPort closed variant rejects mismatch"), Access::Runtime(*F.Mode).SubmitPlayerIntent(Mismatch).ErrorCode,
		EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Active.Verify(*this, F);
	// Both real consumers must route BOTH commands through this bounded shared port.
	for (const TCHAR* File : {TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchRuntime.cpp")})
	{
		FString Source;
		TestTrue(TEXT("Consumer source read"), FFileHelper::LoadFileToString(Source, *(FPaths::ProjectDir() / File)));
		TestTrue(TEXT("Shared entry/deployment port consumed"), Source.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	}
	FString Source;
	FFileHelper::LoadFileToString(Source, *(FPaths::ProjectDir() / TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayEntryDeploymentPlayerIntentPort.cpp")));
	TestTrue(TEXT("Existing Full D12 implementation retained"), Source.Contains(TEXT("FMatchPlayFullD12PlayerIntentPort(Session, Coordinator)")));
	TestTrue(TEXT("Deployment goes directly to Session"), Source.Contains(TEXT("Session.DeployOrdinary(")));
	TestEqual(TEXT("One coordinator invocation site for deployment"), Source.Replace(TEXT("Coordinator.AdvanceToStableState()"), TEXT("")).Len(),
		Source.Len() - FString(TEXT("Coordinator.AdvanceToStableState()")).Len());
	FFileHelper::LoadFileToString(Source, *(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp")));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitDeploymentChoice"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidDeploymentCard"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const FString Submit = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Host and remote both call generated RPC"), Submit.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No implementation shortcut"), Submit.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No authority-side UI shortcut"), Submit.Contains(TEXT("HasAuthority")));
	return true;
}

#endif
