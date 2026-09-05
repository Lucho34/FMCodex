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

struct FFMCodexNetworkDeploymentCompletionTestAccess
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
	static FFMCodexLocalMatchInteractionView Safe(AFMCodexNetworkMatchGameMode& Mode, EInitialTurnOrderPlayer Side, bool Reveal = true)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = Reveal;
		return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Session(Mode).GetStateSnapshot(), Mode.MatchRuntime->SkillRuleSet, Side, Disclosure);
	}
};

namespace FMCodexNetworkDeploymentCompletionTests
{
	using Access = FFMCodexNetworkDeploymentCompletionTestAccess;
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
		AFMCodexNetworkMatchPlayerController* Attacker() const { return A->GetOwnerView().CurrentAttackingSide == Side::PlayerA ? A : B; }
		AFMCodexNetworkMatchPlayerController* Defender() const { return Attacker() == A ? B : A; }
		int32 CoordinatorCalls() const { return Access::Runtime(*Mode).GetCoordinatorInvocationCountForTests(); }
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
		FFMCodexNetworkPlayerIntentEnvelope Goalkeeper(int64 Id, bool Alternate = false)
		{
			auto E = Request(Id, Kind::DeployGoalkeeper);
			const auto View = Access::Safe(*Mode, Acting()->GetOwnerView().ViewerSide);
			for (const auto& Group : View.DeploymentGroups)
			{
				if (Group.bGoalkeeper && !Group.LegalSlotIds.IsEmpty())
				{
					E.Goalkeeper.SlotId = Alternate ? Group.LegalSlotIds.Last() : Group.LegalSlotIds[0];
					break;
				}
			}
			return E;
		}
		bool DefenderTurn()
		{
			return Enter() && Mode->SubmitConnectionPlayerIntent(Attacker(), Deployment(2)).Code == Code::Accepted;
		}
		bool BothOrdinary()
		{
			return DefenderTurn() && Mode->SubmitConnectionPlayerIntent(Defender(), Deployment(1)).Code == Code::Accepted;
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
			D12Calls(Access::Runtime(*F.Mode).GetD12ProviderInvocationCount()), CoordinatorCalls(F.CoordinatorCalls()) {}
		void Verify(FAutomationTestBase& Test, FFixture& F) const
		{
			Test.TestTrue(TEXT("Entire authoritative State unchanged"), SameState(State, Access::Session(*F.Mode).GetStateSnapshot()));
			Test.TestEqual(TEXT("No publication"), Access::Revision(*F.Mode), Revision);
			Test.TestEqual(TEXT("All-provider entropy unchanged"), F.Entropy->Calls, EntropyCalls);
			Test.TestEqual(TEXT("Entry calls unchanged"), Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(), EntryCalls);
			Test.TestEqual(TEXT("D12 calls unchanged"), Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(), D12Calls);
			Test.TestEqual(TEXT("Coordinator not invoked on rejection"), F.CoordinatorCalls(), CoordinatorCalls);
		}
	};
}



IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexGoalkeeperTransportAccepted,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.01.CanonicalSideAndSlot",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexGoalkeeperTransportAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("A"), TEXT("B"), TEXT("AAlternate"), TEXT("BAlternate")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexGoalkeeperTransportAccepted::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F(Parameter.StartsWith(TEXT("A"))); // B-first makes A the legal defender.
	if (!TestTrue(TEXT("Canonical defender turn"), F.DefenderTurn())) { return false; }
	auto* Defender = F.Defender();
	const Side DefenderSide = Defender->GetOwnerView().ViewerSide;
	const auto Before = FUnchanged(F);
	auto E = F.Goalkeeper(1, Parameter.Contains(TEXT("Alternate")));
	TestTrue(TEXT("Only the legal defender receives GK action"), Defender->GetOwnerView().bCanDeployGoalkeeper);
	TestFalse(TEXT("Attacker gets no GK action"), F.Attacker()->GetOwnerView().bCanDeployGoalkeeper);
	TestTrue(TEXT("No unnecessary ordinary payload"), E.Deployment.IsEmpty());
	if (Parameter.Contains(TEXT("Alternate")))
	{
		TestNotEqual(TEXT("Non-advertised legal slot is also supported"), E.Goalkeeper.SlotId, Defender->GetOwnerView().GoalkeeperOption.Choice.SlotId);
	}
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(Defender, E);
	if (!TestEqual(TEXT("GK accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	const FName UniqueGK(DefenderSide == Side::PlayerA ? *Before.State.RuntimeState.PlayerAState.GoalkeeperCardId
		: *Before.State.RuntimeState.PlayerBState.GoalkeeperCardId);
	TestEqual(TEXT("Session derives the side's unique goalkeeper"), After.CurrentAttack.DeploymentPlacements.Last().CardId, UniqueGK);
	TestEqual(TEXT("Actual player-selected slot used"), After.CurrentAttack.DeploymentPlacements.Last().SlotId, E.Goalkeeper.SlotId);
	TestTrue(TEXT("Current defense activation recorded"), After.CurrentAttack.bCurrentDefenseGoalkeeperActivated);
	TestEqual(TEXT("One Coordinator pass"), F.CoordinatorCalls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One stable revision"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("GK consumes no gameplay entropy"), F.Entropy->Calls, Before.EntropyCalls);
	auto Expected = Before.State;
	Expected.CurrentAttack.DeploymentPlacements = After.CurrentAttack.DeploymentPlacements;
	Expected.CurrentAttack.CurrentLegalDeploymentSide = After.CurrentAttack.CurrentLegalDeploymentSide;
	Expected.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	if (DefenderSide == Side::PlayerA) { Expected.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed = true; }
	else { Expected.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true; }
	TestTrue(TEXT("Only canonical GK placement/activation/usage/turn change; card zones stay unchanged"), SameState(Expected, After));
	for (auto* Viewer : {F.A, F.B})
	{
		const auto& V = Viewer->GetOwnerView();
		TestEqual(TEXT("Both public GK Side"), V.GoalkeeperDeployment.Side, DefenderSide);
		TestEqual(TEXT("Both public GK CardId"), V.GoalkeeperDeployment.Placement.Choice.CardId, UniqueGK);
		TestEqual(TEXT("Both public GK slot"), V.GoalkeeperDeployment.Placement.Choice.SlotId, E.Goalkeeper.SlotId);
		TestEqual(TEXT("Atomic shared revision"), V.ViewRevision, Ack.ViewRevision);
		TestFalse(TEXT("Activation is no longer actionable"), V.bCanDeployGoalkeeper);
	}
	TestEqual(TEXT("Both public GK name labels"), F.A->GetOwnerView().GoalkeeperDeployment.Placement.CardLabel.ToString(),
		F.B->GetOwnerView().GoalkeeperDeployment.Placement.CardLabel.ToString());
	const FUnchanged Stable(F);
	TestEqual(TEXT("Exact duplicate cannot activate or coordinate twice"), F.Mode->SubmitConnectionPlayerIntent(Defender, E).Code, Code::DuplicateOrAlreadyResolved);
	Stable.Verify(*this, F);
	TestEqual(TEXT("Attacker finishes canonically"), F.Mode->SubmitConnectionPlayerIntent(F.Attacker(), F.Request(3, Kind::FinishDeployment)).Code, Code::Accepted);
	E.RequestId = 2;
	const FUnchanged Repeated(F);
	TestEqual(TEXT("New ID repeated GK reaches canonical usage rejection"), F.Mode->SubmitConnectionPlayerIntent(Defender, E).Code, Code::AuthorityRejected);
	Repeated.Verify(*this, F);
	TestEqual(TEXT("Defender can still finish"), F.Mode->SubmitConnectionPlayerIntent(Defender, F.Request(3, Kind::FinishDeployment)).Code, Code::Accepted);
	TestTrue(TEXT("Both finished; stop at next real Carrier wait"), F.A->GetOwnerView().bDeploymentComplete
		&& F.B->GetOwnerView().EntryWait == EFMCodexNetworkEntryWait::CarrierSelection);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexGoalkeeperTransportRejected,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexGoalkeeperTransportRejected::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("WrongSide"), TEXT("UnknownSlot"), TEXT("OccupiedSlot"), TEXT("IllegalSlot"),
		TEXT("EmptySlot"), TEXT("WrongMatch"), TEXT("StaleSequence"), TEXT("HugeId"), TEXT("OrdinaryPayload"), TEXT("Nonparticipant")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexGoalkeeperTransportRejected::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	if (!TestTrue(TEXT("Defender fixture"), F.DefenderTurn())) { return false; }
	auto E = F.Goalkeeper(1);
	auto* PC = F.Defender();
	Code Expected = Code::AuthorityRejected;
	if (Parameter == TEXT("WrongSide")) { PC = F.Attacker(); E.RequestId = 3; }
	else if (Parameter == TEXT("UnknownSlot")) { E.Goalkeeper.SlotId = TEXT("NotACanonicalSlot"); }
	else if (Parameter == TEXT("OccupiedSlot")) { E.Goalkeeper.SlotId = Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.DeploymentPlacements[0].SlotId; }
	else if (Parameter == TEXT("IllegalSlot"))
	{
		const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
		const auto* Group = Safe.DeploymentGroups.FindByPredicate([](const auto& G) { return G.bGoalkeeper; });
		bool Found = false;
		if (Group)
		{
			for (const auto& Region : Safe.PitchRegions)
			{
				for (const auto& Slot : Region.Slots)
				{
					if (!Slot.bOccupied && !Group->LegalSlotIds.Contains(Slot.SlotId))
					{ E.Goalkeeper.SlotId = Slot.SlotId; Found = true; break; }
				}
				if (Found) { break; }
			}
		}
		if (!TestTrue(TEXT("Real unoccupied slot outside defender backfield"), Found)) { return false; }
	}
	else if (Parameter == TEXT("EmptySlot")) { E.Goalkeeper = {}; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("WrongMatch")) { E.MatchInstanceId = FGuid::NewGuid(); Expected = Code::MatchMismatch; }
	else if (Parameter == TEXT("StaleSequence")) { ++E.ExpectedAttackSequence; Expected = Code::StaleAttackSequence; }
	else if (Parameter == TEXT("HugeId")) { E.RequestId = MAX_int64; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("OrdinaryPayload")) { E.Deployment = F.Deployment(1).Deployment; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("Nonparticipant")) { PC = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(); Expected = Code::NotParticipant; }
	const FUnchanged Before(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	TestEqual(Parameter, Ack.Code, Expected);
	TestEqual(TEXT("Request correlation"), Ack.RequestId, E.RequestId);
	TestEqual(TEXT("Submitted match correlation"), Ack.MatchInstanceId, E.MatchInstanceId);
	Before.Verify(*this, F);
	const auto Retry = F.Goalkeeper(Parameter == TEXT("HugeId") ? 1 : 2, true);
	TestEqual(TEXT("Rejection/huge ID cannot poison valid next GK action"), F.Mode->SubmitConnectionPlayerIntent(F.Defender(), Retry).Code, Code::Accepted);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexGoalkeeperTransportOwnership,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.03.AttackerAndEarlyPhase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexGoalkeeperTransportOwnership::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	auto E = F.Request(1, Kind::DeployGoalkeeper);
	E.Goalkeeper.SlotId = Access::Session(*F.Mode).GetStateSnapshot().DeploymentSlotCatalog.Slots[0].SlotId;
	const FUnchanged Before(F);
	TestEqual(TEXT("Before initial D12 canonical GK phase rejection"), F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Before.Verify(*this, F);
	TestEqual(TEXT("Full D12 retry still works"), F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(2, Kind::RequestInitialActionPointRoll)).Code, Code::Accepted);
	E.RequestId = 3;
	const FUnchanged Active(F);
	TestEqual(TEXT("Attacker on its legal ordinary deployment turn cannot activate GK"),
		F.Mode->SubmitConnectionPlayerIntent(F.A, E).Code, Code::AuthorityRejected);
	Active.Verify(*this, F);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexFinishTransportAccepted,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.01.StablePhaseClosure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexFinishTransportAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AFirst"), TEXT("BFirst")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexFinishTransportAccepted::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F(Parameter == TEXT("BFirst"));
	if (!TestTrue(TEXT("Both ordinary deployments"), F.BothOrdinary())) { return false; }
	auto* Attacker = F.Attacker(); auto* Defender = F.Defender();
	const FUnchanged Before(F);
	TestTrue(TEXT("Acting viewer can finish"), Attacker->GetOwnerView().bCanFinishDeployment);
	TestFalse(TEXT("Other viewer cannot finish now"), Defender->GetOwnerView().bCanFinishDeployment);
	const auto First = F.Request(3, Kind::FinishDeployment);
	TestTrue(TEXT("Finish has no gameplay payload"), First.Deployment.IsEmpty() && First.Goalkeeper.IsEmpty());
	const auto FirstAck = F.Mode->SubmitConnectionPlayerIntent(Attacker, First);
	TestEqual(TEXT("First finish accepted"), FirstAck.Code, Code::Accepted);
	TestEqual(TEXT("One Coordinator invocation"), F.CoordinatorCalls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One revision"), FirstAck.ViewRevision, Before.Revision + 1);
	TestFalse(TEXT("One side finishing is not phase completion"), Attacker->GetOwnerView().bDeploymentComplete);
	const FUnchanged OneDone(F);
	auto Repeat = First; Repeat.RequestId = 4;
	TestEqual(TEXT("Fresh-ID already finished side is canonically refused"), F.Mode->SubmitConnectionPlayerIntent(Attacker, Repeat).Code, Code::AuthorityRejected);
	OneDone.Verify(*this, F);
	const auto Final = F.Request(2, Kind::FinishDeployment);
	const auto FinalAck = F.Mode->SubmitConnectionPlayerIntent(Defender, Final);
	TestEqual(TEXT("Other side finishes"), FinalAck.Code, Code::Accepted);
	TestEqual(TEXT("Exactly one further Coordinator pass"), F.CoordinatorCalls(), OneDone.CoordinatorCalls + 1);
	TestEqual(TEXT("Atomic final revision"), FinalAck.ViewRevision, OneDone.Revision + 1);
	const auto State = Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Next exact canonical selection state"), State.CurrentAttack.SelectionStage, EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier);
	TestTrue(TEXT("No next PlayerIntent auto-executed"), State.CurrentAttack.ActionPreparation.CarrierCardId.IsNone()
		&& !State.CurrentAttack.bHasSelectedAction && !State.CurrentAttack.bHasResolutionSession);
	TestEqual(TEXT("This closure needs no internal RNG"), F.Entropy->Calls, Before.EntropyCalls);
	for (auto* PC : {F.A, F.B})
	{
		const auto& V = PC->GetOwnerView();
		TestTrue(TEXT("Public completion is stable on both sides"), V.bPlayerADeploymentFinished && V.bPlayerBDeploymentFinished && V.bDeploymentComplete);
		TestEqual(TEXT("Public exact next wait"), V.EntryWait, EFMCodexNetworkEntryWait::CarrierSelection);
		TestEqual(TEXT("Expected player remains canonical attacker"), V.ExpectedActingSide, State.RuntimeState.CurrentAttackingPlayer);
		TestEqual(TEXT("Both published at final stable revision"), V.ViewRevision, FinalAck.ViewRevision);
		TestFalse(TEXT("No finish action after deployment"), V.bCanFinishDeployment);
		TestFalse(TEXT("No GK action after deployment"), V.bCanDeployGoalkeeper);
		TestTrue(TEXT("No ordinary actions after deployment"), V.DeploymentOptions.IsEmpty());
	}
	const FUnchanged Complete(F);
	TestEqual(TEXT("Duplicate final finish cannot coordinate again"), F.Mode->SubmitConnectionPlayerIntent(Defender, Final).Code, Code::DuplicateOrAlreadyResolved);
	Repeat = Final; Repeat.RequestId = 3;
	TestEqual(TEXT("Fresh ID final repeat rejected by canonical phase"), F.Mode->SubmitConnectionPlayerIntent(Defender, Repeat).Code, Code::AuthorityRejected);
	Complete.Verify(*this, F);
	Access::Publish(*F.Mode);
	TestEqual(TEXT("Publication itself never invokes Coordinator"), F.CoordinatorCalls(), Complete.CoordinatorCalls);
	return true;
}


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexFinishTransportRejected,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexFinishTransportRejected::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("BeforeFullD12"), TEXT("WrongSide"), TEXT("WrongMatch"), TEXT("StaleSequence"),
		TEXT("OrdinaryPayload"), TEXT("GoalkeeperPayload"), TEXT("HugeId"), TEXT("ZeroId"), TEXT("ZeroSequence"), TEXT("Nonparticipant")})
	{ Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexFinishTransportRejected::RunTest(const FString& Parameter)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	if (Parameter != TEXT("BeforeFullD12") && !TestTrue(TEXT("Deployment fixture"), F.Enter())) { return false; }
	auto E = F.Request(2, Kind::FinishDeployment);
	auto* PC = F.A;
	Code Expected = Code::AuthorityRejected;
	if (Parameter == TEXT("WrongSide")) { PC = F.B; }
	else if (Parameter == TEXT("WrongMatch")) { E.MatchInstanceId = FGuid::NewGuid(); Expected = Code::MatchMismatch; }
	else if (Parameter == TEXT("StaleSequence")) { ++E.ExpectedAttackSequence; Expected = Code::StaleAttackSequence; }
	else if (Parameter == TEXT("OrdinaryPayload"))
	{ E.Deployment = F.Deployment(2).Deployment; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("GoalkeeperPayload"))
	{ E.Goalkeeper.SlotId = TEXT("AnyNonemptySlot"); Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("HugeId")) { E.RequestId = MAX_int64; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("ZeroId")) { E.RequestId = 0; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("ZeroSequence")) { E.ExpectedAttackSequence = 0; Expected = Code::InvalidPayload; }
	else if (Parameter == TEXT("Nonparticipant")) { PC = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(); Expected = Code::NotParticipant; }
	const FUnchanged Before(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	TestEqual(Parameter, Ack.Code, Expected);
	TestEqual(TEXT("ACK echoes match"), Ack.MatchInstanceId, E.MatchInstanceId);
	TestEqual(TEXT("ACK echoes request"), Ack.RequestId, E.RequestId);
	Before.Verify(*this, F);
	if (Parameter != TEXT("BeforeFullD12"))
	{
		// Zero placed cards and no active GK are legal: transport must not invent prerequisites.
		TestEqual(TEXT("Normal finish remains legal without minimum card/GK rule"),
			F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(Parameter == TEXT("HugeId") ? 2 : 3, Kind::FinishDeployment)).Code,
			Code::Accepted);
	}
	return true;
}

namespace FMCodexNetworkDeploymentCompletionTests
{
	bool StaleIntoNextAttack(FAutomationTestBase& Test, Kind IntentKind)
	{
		FFixture F;
		if (!Test.TestTrue(TEXT("Attack N entered"), F.Enter())) { return false; }
		auto& Session = Access::Session(*F.Mode);
		const int64 N = F.A->GetOwnerView().AttackSequence;
		// Canonical fixture: no attacker placements, finish A, then B has genuine legal GK/Finish choices.
		Test.TestTrue(TEXT("Server fixture finishes attacker"), Session.FinishDeployment(N, Side::PlayerA).RuntimeEnvelope.bDomainSuccess);
		Access::Publish(*F.Mode);
		auto Delayed = IntentKind == Kind::DeployGoalkeeper ? F.Goalkeeper(2) : F.Request(2, Kind::FinishDeployment);
		Test.TestTrue(TEXT("Delayed request was a valid legal N action"), F.B->GetOwnerView().bCanFinishDeployment
			&& (IntentKind != Kind::DeployGoalkeeper || F.B->GetOwnerView().bCanDeployGoalkeeper));
		Test.TestTrue(TEXT("Server fixture finishes defender"), Session.FinishDeployment(N, Side::PlayerB).RuntimeEnvelope.bDomainSuccess);
		const auto Advance = Access::Advance(*F.Mode);
		Test.TestTrue(TEXT("Canonical no-carrier internal completion"), Advance.bSuccess);
		Test.TestTrue(TEXT("Internal step is recorded"), !Advance.Steps.IsEmpty());
		Access::Publish(*F.Mode);
		Test.TestEqual(TEXT("B now awaits manual initial roll"), F.B->GetOwnerView().InteractionState,
			EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint);
		Test.TestEqual(TEXT("Enter N+1 through normal Full D12 boundary"),
			F.Mode->SubmitConnectionPlayerIntent(F.B, F.Request(1, Kind::RequestInitialActionPointRoll)).Code, Code::Accepted);
		Test.TestEqual(TEXT("N+1 deployment is active"), F.B->GetOwnerView().EntryWait, EFMCodexNetworkEntryWait::Deployment);
		Test.TestEqual(TEXT("Sequence N+1"), F.B->GetOwnerView().AttackSequence, N + 1);
		const FUnchanged Before(F);
		Test.TestEqual(TEXT("Fresh-ID old GK/Finish is stale before any side/phase mutation"),
			F.Mode->SubmitConnectionPlayerIntent(F.B, Delayed).Code, Code::StaleAttackSequence);
		Before.Verify(Test, F);
		Test.TestEqual(TEXT("Current normal request remains usable"), F.Mode->SubmitConnectionPlayerIntent(F.B, F.Deployment(3)).Code, Code::Accepted);
		return true;
	}
	bool AckOrders(FAutomationTestBase& Test, Kind IntentKind)
	{
		for (bool AckFirst : {true, false})
		{
			FFixture F;
			if (!Test.TestTrue(TEXT("Fresh legal defender"), F.DefenderTurn())) { return false; }
			auto* PC = F.Defender();
			const auto OldView = PC->GetOwnerView();
			FFMCodexNetworkIntentClientState Client;
			FFMCodexNetworkPlayerIntentEnvelope E;
			auto Begin = [&](FFMCodexNetworkPlayerIntentEnvelope& Out)
			{
				return IntentKind == Kind::DeployGoalkeeper ? Client.BeginGoalkeeper(OldView, OldView.GoalkeeperOption.Choice, Out)
					: Client.BeginFinishDeployment(OldView, Out);
			};
			Test.TestTrue(TEXT("New action begins with shared client state"), Begin(E));
			const auto Original = E;
			Test.TestFalse(TEXT("Double click cannot create a second envelope"), Begin(E));
			Test.TestEqual(TEXT("Failed begin preserves prepared envelope"), E.RequestId, Original.RequestId);
			FFMCodexNetworkPlayerIntentEnvelope Ignored;
			Test.TestFalse(TEXT("Pending also blocks ordinary action"), Client.BeginDeployment(OldView, F.Deployment(1).Deployment, Ignored));
			Test.TestFalse(TEXT("Pending also blocks Full D12"), Client.Begin(OldView, Ignored));
			const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
			Test.TestEqual(TEXT("ACK accepted"), Ack.Code, Code::Accepted);
			const auto Calls = F.CoordinatorCalls();
			if (AckFirst)
			{
				Test.TestTrue(TEXT("ACK correlated"), Client.ObserveAck(Ack));
				Test.TestTrue(TEXT("ACK alone leaves pending"), Client.IsPending());
				Client.ObserveView(OldView);
				Test.TestTrue(TEXT("Old view insufficient"), Client.IsPending());
				Client.ObserveView(PC->GetOwnerView());
			}
			else
			{
				Client.ObserveView(PC->GetOwnerView());
				Test.TestTrue(TEXT("View alone leaves pending"), Client.IsPending());
				Test.TestTrue(TEXT("Delayed ACK correlated"), Client.ObserveAck(Ack));
			}
			Test.TestFalse(TEXT("Both facts release pending"), Client.IsPending());
			Test.TestFalse(TEXT("Repeated ACK ignored"), Client.ObserveAck(Ack));
			Test.TestFalse(TEXT("Old actionable snapshot cannot be reused"), Begin(E));
			Test.TestEqual(TEXT("Client ACK/view handling never calls Coordinator"), F.CoordinatorCalls(), Calls);
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexGoalkeeperTransportStale,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.04.StaleAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexGoalkeeperTransportStale::RunTest(const FString&)
{
	return FMCodexNetworkDeploymentCompletionTests::StaleIntoNextAttack(*this, EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper);
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFinishTransportStale,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.03.StaleAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFinishTransportStale::RunTest(const FString&)
{
	return FMCodexNetworkDeploymentCompletionTests::StaleIntoNextAttack(*this, EFMCodexNetworkPlayerIntentKind::FinishDeployment);
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexGoalkeeperTransportAck,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.05.AckViewOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexGoalkeeperTransportAck::RunTest(const FString&)
{
	return FMCodexNetworkDeploymentCompletionTests::AckOrders(*this, EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper);
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFinishTransportAck,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.04.AckViewOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFinishTransportAck::RunTest(const FString&)
{
	return FMCodexNetworkDeploymentCompletionTests::AckOrders(*this, EFMCodexNetworkPlayerIntentKind::FinishDeployment);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFinishTransportNamespace,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.05.FourIntentNamespaceAndRejectedPending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFinishTransportNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	if (!TestTrue(TEXT("A Full D12=1 and ordinary=2 IDs used"), F.DefenderTurn())) { return false; }
	auto GK = F.Goalkeeper(2);
	const FUnchanged Before(F);
	TestEqual(TEXT("GK cannot reuse an ordinary request ID"), F.Mode->SubmitConnectionPlayerIntent(F.A, GK).Code, Code::DuplicateOrAlreadyResolved);
	Before.Verify(*this, F);
	GK.RequestId = 1;
	FFMCodexNetworkIntentClientState BClient;
	FFMCodexNetworkPlayerIntentEnvelope E;
	TestTrue(TEXT("B begins GK using ID1"), BClient.BeginGoalkeeper(F.B->GetOwnerView(), GK.Goalkeeper, E));
	const auto GKAck = F.Mode->SubmitConnectionPlayerIntent(F.B, E);
	TestEqual(TEXT("B GK accepted"), GKAck.Code, Code::Accepted);
	BClient.ObserveView(F.B->GetOwnerView()); BClient.ObserveAck(GKAck);
	TestEqual(TEXT("A finishes using same namespace ID3"), F.Mode->SubmitConnectionPlayerIntent(F.A, F.Request(3, Kind::FinishDeployment)).Code, Code::Accepted);
	auto Finish = F.Request(1, Kind::FinishDeployment);
	const FUnchanged Active(F);
	TestEqual(TEXT("Finish cannot reuse GK ID"), F.Mode->SubmitConnectionPlayerIntent(F.B, Finish).Code, Code::DuplicateOrAlreadyResolved);
	Finish.RequestId = MAX_int64;
	TestEqual(TEXT("Finish cannot bypass bounded forward window"), F.Mode->SubmitConnectionPlayerIntent(F.B, Finish).Code, Code::InvalidPayload);
	Active.Verify(*this, F);
	TestTrue(TEXT("Same client next finish ID after GK"), BClient.BeginFinishDeployment(F.B->GetOwnerView(), E));
	TestEqual(TEXT("No separate counter"), E.RequestId, int64(2));
	const auto FinalAck = F.Mode->SubmitConnectionPlayerIntent(F.B, E);
	TestEqual(TEXT("Normal ID after huge succeeds"), FinalAck.Code, Code::Accepted);
	BClient.ObserveAck(FinalAck); BClient.ObserveView(F.B->GetOwnerView());
	TestFalse(TEXT("Final finish pending released"), BClient.IsPending());
	TestEqual(TEXT("Full D12 cannot reuse a finish ID"), F.Mode->SubmitConnectionPlayerIntent(F.B, F.Request(2, Kind::RequestInitialActionPointRoll)).Code, Code::DuplicateOrAlreadyResolved);
	// A rejected request must release pending even though no view was published.
	FFixture Rejected;
	if (!TestTrue(TEXT("Rejected pending fixture"), Rejected.DefenderTurn())) { return false; }
	FFMCodexNetworkIntentClientState C;
	auto Bad = Rejected.B->GetOwnerView().GoalkeeperOption.Choice;
	Bad.SlotId = TEXT("InvalidGoalkeeperSlot");
	TestTrue(TEXT("Wire-shaped invalid slot can be submitted"), C.BeginGoalkeeper(Rejected.B->GetOwnerView(), Bad, E));
	const FUnchanged Unchanged(Rejected);
	const auto RejectAck = Rejected.Mode->SubmitConnectionPlayerIntent(Rejected.B, E);
	TestEqual(TEXT("Canonical invalid slot rejection"), RejectAck.Code, Code::AuthorityRejected);
	TestTrue(TEXT("Rejected ACK correlates"), C.ObserveAck(RejectAck));
	TestFalse(TEXT("No new view needed to release rejected pending"), C.IsPending());
	Unchanged.Verify(*this, Rejected);
	const auto Duplicate = Rejected.Mode->SubmitConnectionPlayerIntent(Rejected.B, E);
	TestEqual(TEXT("Gameplay rejection consumes admitted ID"), Duplicate.Code, Code::DuplicateOrAlreadyResolved);
	Unchanged.Verify(*this, Rejected);
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexGoalkeeperTransportWire,
	"FMCodex.NetworkPlay.GoalkeeperDeploymentTransport.06.ExactSlotOnlyWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexGoalkeeperTransportWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	using Payload = FFMCodexNetworkDeployGoalkeeperPayload;
	int32 FieldCount = 0;
	for (TFieldIterator<FProperty> It(Payload::StaticStruct()); It; ++It)
	{
		++FieldCount;
		TestEqual(TEXT("Only SlotId is a player choice"), It->GetFName(), FName(TEXT("SlotId")));
		TestTrue(TEXT("Canonical FName identity"), CastField<FNameProperty>(*It) != nullptr);
	}
	TestEqual(TEXT("No client CardId/Side field"), FieldCount, 1);
	TestTrue(TEXT("Custom bounded serializer registered"), Payload::StaticStruct()->GetCppStructOps()->HasNetSerializer());
	for (const FString& Name : {FString(TEXT("Demo.Slot.NearB.01")), FString(TEXT("槽位_2")), FString::ChrN(128, TEXT('x'))})
	{
		Payload Original; Original.SlotId = FName(*Name);
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = false;
		Original.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Bounded valid write"), Success && Bytes.Num() <= 129);
		FMemoryReader Reader(Bytes); Payload Copy;
		Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Valid read"), Success && !Reader.IsError());
		TestEqual(TEXT("Stable slot roundtrip"), Copy.SlotId, Original.SlotId);
	}
	Payload TooLong; TooLong.SlotId = FName(*FString::ChrN(129, TEXT('x')));
	TestFalse(TEXT("Oversized slot shape refused"), TooLong.IsValidShape());
	for (TArray<uint8> Bytes : {TArray<uint8>{129}, TArray<uint8>{2, 'x'}, TArray<uint8>{1, 0}, TArray<uint8>{2, 0xC0, 0xAF}})
	{
		FMemoryReader Reader(Bytes); Payload Copy; bool Success = true;
		Copy.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Oversize/truncated/NUL/invalid UTF8 fail closed"), Success);
		TestTrue(TEXT("Malformed wire does not create a slot identity"), Copy.SlotId.IsNone());
	}
	for (const Kind K : {Kind::RequestInitialActionPointRoll, Kind::DeployOrdinary, Kind::DeployGoalkeeper, Kind::FinishDeployment})
	{
		for (const bool Ordinary : {false, true})
		{
			for (const bool Goalkeeper : {false, true})
			{
				FFMCodexNetworkPlayerIntentEnvelope E; E.IntentKind = K;
				if (Ordinary) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); }
				if (Goalkeeper) { E.Goalkeeper.SlotId = TEXT("Slot"); }
				const bool Valid = K == Kind::DeployOrdinary ? Ordinary && !Goalkeeper
					: K == Kind::DeployGoalkeeper ? Goalkeeper && !Ordinary : !Ordinary && !Goalkeeper;
				TestEqual(TEXT("Each kind accepts exactly its own closed shape"), E.ValidatePayloadShape(), Valid ? Code::None : Code::InvalidPayload);
			}
		}
	}
	FFixture F;
	if (!TestTrue(TEXT("Dispatch fixture"), F.DefenderTurn())) { return false; }
	const FUnchanged Before(F);
	auto E = F.Goalkeeper(1);
	E.IntentKind = Kind::FinishDeployment;
	TestEqual(TEXT("Finish with GK payload never reaches Session"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::InvalidPayload);
	E = F.Goalkeeper(1); E.Deployment.CardId = TEXT("ForgedOpponentGK");
	TestEqual(TEXT("A claimed GK CardId has no supported field and ordinary-member smuggling fails"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::InvalidPayload);
	E = F.Request(1, static_cast<Kind>(255));
	TestEqual(TEXT("Unknown/internal command has no allowed wire tag"), F.Mode->SubmitConnectionPlayerIntent(F.B, E).Code, Code::NotPlayerIntent);
	Before.Verify(*this, F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFinishTransportProjection,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.06.SafeActionAndCompletionProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFinishTransportProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	if (!TestTrue(TEXT("Defender fixture"), F.DefenderTurn())) { return false; }
	const FUnchanged Before(F);
	const auto A = Access::Safe(*F.Mode, Side::PlayerA);
	const auto B = Access::Safe(*F.Mode, Side::PlayerB);
	const auto Invalid = Access::Safe(*F.Mode, Side::None);
	TestFalse(TEXT("Nonacting viewer cannot finish"), A.bCanFinishDeployment);
	TestTrue(TEXT("Acting viewer has canonical Finish availability"), B.bCanFinishDeployment);
	TestFalse(TEXT("Invalid viewer cannot finish"), Invalid.bCanFinishDeployment);
	TestTrue(TEXT("Invalid viewer has no GK or ordinary choices"), Invalid.DeploymentGroups.IsEmpty());
	Before.Verify(*this, F); // Pure canonical Finish projection never commits a candidate.
	TestTrue(TEXT("GK option is populated from safe GK group"),
		B.DeploymentGroups.ContainsByPredicate([&](const auto& G)
		{ return G.bGoalkeeper && G.LegalSlotIds.Contains(F.B->GetOwnerView().GoalkeeperOption.Choice.SlotId); }));
	TestTrue(TEXT("Ordinary option cap retained"), F.B->GetOwnerView().DeploymentOptions.Num() <= 3);
	TestEqual(TEXT("Defender Finish accepted"), F.Mode->SubmitConnectionPlayerIntent(F.B, F.Request(1, Kind::FinishDeployment)).Code, Code::Accepted);
	for (auto* PC : {F.A, F.B})
	{
		TestTrue(TEXT("Public finished side is B on both views"), PC->GetOwnerView().bPlayerBDeploymentFinished);
		TestFalse(TEXT("A has not finished"), PC->GetOwnerView().bPlayerADeploymentFinished);
		TestFalse(TEXT("Phase not prematurely complete"), PC->GetOwnerView().bDeploymentComplete);
	}
	TestTrue(TEXT("A now has finish action"), F.A->GetOwnerView().bCanFinishDeployment);
	TestFalse(TEXT("Finished B has no finish action"), F.B->GetOwnerView().bCanFinishDeployment);
	for (Side Viewer : {Side::PlayerA, Side::PlayerB})
	{
		const auto Hidden = Access::Safe(*F.Mode, Viewer, false);
		TestFalse(TEXT("Undisclosed route cannot leak finish availability"), Hidden.bCanFinishDeployment);
		TestFalse(TEXT("Undisclosed route cannot leak finished A"), Hidden.bPlayerADeploymentFinished);
		TestFalse(TEXT("Undisclosed route cannot leak finished B"), Hidden.bPlayerBDeploymentFinished);
		TestFalse(TEXT("Undisclosed route cannot leak phase completion"), Hidden.bDeploymentComplete);
		const auto Wire = FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden, F.Mode->GetMatchInstanceId(),
			99, Viewer, EFMCodexNetworkBootstrapState::MatchReady);
		TestFalse(TEXT("No GK action in withheld route"), Wire.bCanDeployGoalkeeper);
		TestFalse(TEXT("No finish action in withheld route"), Wire.bCanFinishDeployment);
		TestTrue(TEXT("No ordinary option in withheld route"), Wire.DeploymentOptions.IsEmpty());
	}
	FFMCodexNetworkIntentClientState Client;
	FFMCodexNetworkPlayerIntentEnvelope E;
	// A already used IDs 1 and 2. Correlation-only rejection is safe even when the test client is fresh.
	TestTrue(TEXT("Finish pending begins"), Client.BeginFinishDeployment(F.A->GetOwnerView(), E));
	const FUnchanged Reject(F);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(F.A, E);
	TestEqual(TEXT("Admitted connection namespace rejects old client ID"), Ack.Code, Code::DuplicateOrAlreadyResolved);
	TestTrue(TEXT("Finish rejection correlated"), Client.ObserveAck(Ack));
	TestFalse(TEXT("Finish rejection releases pending without new view"), Client.IsPending());
	Reject.Verify(*this, F);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFinishTransportSharedBoundary,
	"FMCodex.NetworkPlay.FinishDeploymentTransport.07.SharedPortAndGeneratedRpcParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFinishTransportSharedBoundary::RunTest(const FString&)
{
	using namespace FMCodexNetworkDeploymentCompletionTests;
	FFixture F;
	if (!TestTrue(TEXT("Initialized"), F.Entropy != nullptr)) { return false; }
	const FUnchanged Before(F);
	for (const auto K : {EMatchPlayAuthoritativeCommandKind::DeployGoalkeeper, EMatchPlayAuthoritativeCommandKind::FinishDeployment})
	{
		FMatchPlayPlayerIntent Wrong;
		Wrong.CommandKind = K;
		Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
		TestEqual(TEXT("Shared Port rejects wrong canonical variant"),
			Access::Runtime(*F.Mode).SubmitPlayerIntent(Wrong).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	}
	FMatchPlayPlayerIntent Internal;
	Internal.CommandKind = EMatchPlayAuthoritativeCommandKind::ResolveSendingOff;
	TestEqual(TEXT("Shared port excludes server internal commands"),
		Access::Runtime(*F.Mode).SubmitPlayerIntent(Internal).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent);
	Before.Verify(*this, F);
	FString Source;
	TestTrue(TEXT("Controller source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitDeploymentCompletion"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidGoalkeeperSlot"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const auto Dispatch = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Both GK and Finish use shared generated owner RPC"), Dispatch.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation shortcut"), Dispatch.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No listen-host bypass"), Dispatch.Contains(TEXT("HasAuthority")));
	TestFalse(TEXT("No Controller HostPort direct dispatch"), Dispatch.Contains(TEXT("HostPort")));
	for (const TCHAR* File : {TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchRuntime.cpp")})
	{
		TestTrue(TEXT("Host source readable"), FFileHelper::LoadFileToString(Source, *(FPaths::ProjectDir() / File)));
		TestTrue(TEXT("Local and Network consume same deployment port"), Source.Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	}
	return true;
}

#endif
