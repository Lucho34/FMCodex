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
#include "../LocalPlay/FMCodexPlayerUIPresentationText.h"
#include "../LocalPlay/FMCodexPrototypeTeamContent.h"
#include "../CoreRules/PlayerCardRuleSnapshotValidator.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionWriter.h"

struct FFMCodexNetworkSkillTestAccess
{
	static FFMCodexNetworkScriptedEntropy* Configure(AFMCodexNetworkMatchGameMode& Mode,
		AFMCodexNetworkMatchPlayerController* A, AFMCodexNetworkMatchPlayerController* B, bool BFirst, int32 RawD12)
	{
		Mode.MatchInstanceId = FGuid::NewGuid();
		Mode.BootstrapConfiguration = BFirst
			? FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch()
			: FFMCodexNetworkBootstrapConfigurationFactory::CreatePrototypeMatch();
		Mode.ParticipantRegistry.Admit(A, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		Mode.ParticipantRegistry.Admit(B, Mode.GetWorld()->SpawnActor<AFMCodexNetworkMatchPlayerState>());
		auto Entropy = MakeUnique<FFMCodexNetworkScriptedEntropy>(TArray<uint32>{uint32(RawD12 - 1), uint32(RawD12 - 1), uint32(RawD12 - 1), uint32(RawD12 - 1)});
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
	static FFMCodexLocalMatchInteractionView Safe(AFMCodexNetworkMatchGameMode& Mode, EInitialTurnOrderPlayer Side, bool Reveal = true)
	{
		FFMCodexLocalMatchViewerDisclosure Disclosure;
		Disclosure.bRevealInitialActionPointRoll = Reveal;
		return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
			Session(Mode).GetStateSnapshot(), Mode.MatchRuntime->SkillRuleSet, Side, Disclosure);
	}
};

namespace FMCodexNetworkSkillTests
{
	using Access = FFMCodexNetworkSkillTestAccess;
	using Code = EFMCodexNetworkIntentAckCode;
	using Kind = EFMCodexNetworkPlayerIntentKind;
	using Side = EInitialTurnOrderPlayer;
	using Envelope = FFMCodexNetworkPlayerIntentEnvelope;
	using MarkerPayload = FFMCodexNetworkSubmitMarkerPayload;
	using HelperPayload = FFMCodexNetworkSubmitHelperPayload;
	using Payload = FFMCodexNetworkSubmitSkillPayload;
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
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = Kind::SubmitSkill;
			if (!Attacker()->GetOwnerView().SkillOptions.IsEmpty())
			{ E.Skill = Attacker()->GetOwnerView().SkillOptions[0].Choice; }
			return E;
		}
		bool Send(AFMCodexNetworkMatchPlayerController* PC, Kind K,
			const FFMCodexNetworkDeployOrdinaryPayload& Ordinary = {}, const CarrierPayload& Carrier = {}, const MarkerPayload& Marker = {}, const FFMCodexNetworkSubmitRunnerPayload& Runner = {}, const HelperPayload& Helper = {}, const Payload& Skill = {})
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
				: K == Kind::SubmitSkill && C.BeginSkill(V, Skill, E);
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


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSkillAccepted,
	"FMCodex.NetworkPlay.SkillTransport.01.CanonicalAcceptance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexSkillAccepted::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("AFirst.PassControl"),TEXT("AFirst.ThroughBall"),TEXT("BFirst.PassControl"),TEXT("BFirst.ThroughBall")})
	{ Names.Add(N); Commands.Add(N); }
}
bool FFMCodexSkillAccepted::RunTest(const FString& P)
{
	using namespace FMCodexNetworkSkillTests;
	FFixture F(P.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical Skill fixture"), F.ReachSkill())) { return false; }
	auto* PC = F.Attacker(); const auto Safe = Access::Safe(*F.Mode, PC->GetOwnerView().ViewerSide);
	TestEqual(TEXT("Both legal options represented"), PC->GetOwnerView().SkillOptions.Num(), 2);
	const FUnchanged Before(F); auto E = F.Request(PC);
	if (P.EndsWith(TEXT("ThroughBall"))) { E.Skill = PC->GetOwnerView().SkillOptions.Last().Choice; }
	FMatchPlayCurrentAttackSkillSelectionRequest Domain;
	Domain.AttackSequence = E.ExpectedAttackSequence; Domain.RequestingSide = PC->GetOwnerView().ViewerSide; Domain.SkillId = E.Skill.SkillId;
	const auto Canonical = FMatchPlayCurrentAttackSkillSelectionWriter::Select(Before.State, Access::CallerRules(*F.Mode), Domain);
	TestTrue(TEXT("Pinned canonical lookup resolves selected Skill"), Canonical.LegalityResult.SkillRuleQueryResult.bSuccess);
	const auto Ack = F.Mode->SubmitConnectionPlayerIntent(PC, E);
	if (!TestEqual(TEXT("Accepted"), Ack.Code, Code::Accepted)) { return false; }
	const auto After = Access::Session(*F.Mode).GetStateSnapshot();
	TestTrue(TEXT("Exact canonical writer State; no later player action"), SameState(After, Canonical.AfterState));
	TestEqual(TEXT("One Coordinator"), F.Calls(), Before.CoordinatorCalls + 1);
	TestEqual(TEXT("One publication"), Ack.ViewRevision, Before.Revision + 1);
	TestEqual(TEXT("Zero all-provider entropy"), F.Entropy->Calls, Before.EntropyCalls);
	TestTrue(TEXT("Selected action frozen"), After.CurrentAttack.bHasSelectedAction);
	TestFalse(TEXT("Resolution not begun before player route roll"), After.CurrentAttack.bHasResolutionSession);
	TestFalse(TEXT("Skill no longer deferred"), After.CurrentAttack.ActionPreparation.bSkillSelectionDeferred);
	for (auto* V : {F.A,F.B})
	{
		TestEqual(TEXT("Public Skill matches"), V->GetOwnerView().SelectedSkill.Choice.SkillId, E.Skill.SkillId);
		TestEqual(TEXT("Synchronized revision"), V->GetOwnerView().ViewRevision, Ack.ViewRevision);
		TestEqual(TEXT("Next actor is attacker"), V->GetOwnerView().ExpectedActingSide, PC->GetOwnerView().ViewerSide);
		TestEqual(TEXT("Actual route wait"), V->GetOwnerView().EntryWait, P.EndsWith(TEXT("ThroughBall"))
			? EFMCodexNetworkEntryWait::ThroughBallRouteRoll : EFMCodexNetworkEntryWait::PassControlRouteRoll);
		TestTrue(TEXT("No Skill choices after freeze"), V->GetOwnerView().SkillOptions.IsEmpty());
	}
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSkillReject,
	"FMCodex.NetworkPlay.SkillTransport.02.Rejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexSkillReject::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("WrongSide"),TEXT("Unknown"),TEXT("KnownNotOwned"),TEXT("OwnedWrongTP"),TEXT("WrongPhase"),
		TEXT("Empty"),TEXT("Oversize"),TEXT("HelperMember"),TEXT("RunnerMember"),TEXT("MarkerMember"),TEXT("CarrierMember"),
		TEXT("OrdinaryMember"),TEXT("GoalkeeperMember"),TEXT("WrongTag"),TEXT("WrongMatch"),TEXT("WrongSequence"),
		TEXT("HugeId"),TEXT("ZeroId"),TEXT("ZeroSequence"),TEXT("InternalTag"),TEXT("Nonparticipant")})
	{ Names.Add(N); Commands.Add(N); }
}
bool FFMCodexSkillReject::RunTest(const FString& P)
{
	using namespace FMCodexNetworkSkillTests; FFixture F;
	if (!TestTrue(TEXT("Canonical fixture"), F.ReachSkill())) { return false; }
	auto* PC = P == TEXT("WrongSide") ? F.Defender() : F.Attacker();
	auto E = F.Request(PC); Code Expected = Code::AuthorityRejected;
	if (P == TEXT("Unknown")) { E.Skill.SkillId = TEXT("Unknown.Skill"); }
	if (P == TEXT("KnownNotOwned")) { E.Skill.SkillId = TEXT("Canonical.Skill.Cross.4.6"); }
	if (P == TEXT("OwnedWrongTP")) { E.Skill.SkillId = TEXT("Canonical.Skill.LongShot.3.5"); }
	if (P == TEXT("Empty")) { E.Skill = {}; Expected = Code::InvalidPayload; }
	if (P == TEXT("Oversize")) { E.Skill.SkillId = FName(*FString::ChrN(129,TEXT('x'))); Expected = Code::InvalidPayload; }
	if (P == TEXT("HelperMember")) { E.Helper.HelperCardId = TEXT("Helper"); Expected = Code::InvalidPayload; }
	if (P == TEXT("RunnerMember")) { E.Runner.RunnerCardId = TEXT("Runner"); Expected = Code::InvalidPayload; }
	if (P == TEXT("MarkerMember")) { E.Marker.MarkerCardId = TEXT("Marker"); Expected = Code::InvalidPayload; }
	if (P == TEXT("CarrierMember")) { E.Carrier.CarrierCardId = TEXT("Carrier"); Expected = Code::InvalidPayload; }
	if (P == TEXT("OrdinaryMember")) { E.Deployment.CardId = TEXT("Card"); Expected = Code::InvalidPayload; }
	if (P == TEXT("GoalkeeperMember")) { E.Goalkeeper.SlotId = TEXT("Slot"); Expected = Code::InvalidPayload; }
	if (P == TEXT("WrongTag")) { E.IntentKind = Kind::FinishDeployment; Expected = Code::InvalidPayload; }
	if (P == TEXT("WrongMatch")) { E.MatchInstanceId = FGuid::NewGuid(); Expected = Code::MatchMismatch; }
	if (P == TEXT("WrongSequence")) { ++E.ExpectedAttackSequence; Expected = Code::StaleAttackSequence; }
	if (P == TEXT("HugeId")) { E.RequestId = MAX_int64; Expected = Code::InvalidPayload; }
	if (P == TEXT("ZeroId")) { E.RequestId = 0; Expected = Code::InvalidPayload; }
	if (P == TEXT("ZeroSequence")) { E.ExpectedAttackSequence = 0; Expected = Code::InvalidPayload; }
	if (P == TEXT("InternalTag")) { E.IntentKind = static_cast<Kind>(255); Expected = Code::NotPlayerIntent; }
	if (P == TEXT("Nonparticipant")) { PC = F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(); Expected = Code::NotParticipant; }
	if (P == TEXT("WrongPhase"))
	{
		FFixture Early; auto EE = Early.Request(Early.A); EE.Skill = E.Skill;
		const FUnchanged Before(Early);
		TestEqual(TEXT("Early canonical phase rejection"), Early.Mode->SubmitConnectionPlayerIntent(Early.A,EE).Code, Code::AuthorityRejected);
		Before.Verify(*this,Early); return true;
	}
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact rejection"), F.Mode->SubmitConnectionPlayerIntent(PC,E).Code, Expected);
	Before.Verify(*this,F);
	auto Normal=F.Request(F.A);
	if (P == TEXT("HugeId")) { Normal.RequestId = F.NextA - 2; }
	TestEqual(TEXT("Normal request after rejection remains usable"), F.Mode->SubmitConnectionPlayerIntent(F.A,Normal).Code, Code::Accepted);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillReplay,
	"FMCodex.NetworkPlay.SkillTransport.03.FrozenRewriteAndReplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillReplay::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests; FFixture F;
	if (!TestTrue(TEXT("Two legal choices"),F.ReachSkill())) { return false; }
	auto E=F.Request(F.A); const auto Other=F.A->GetOwnerView().SkillOptions.Last().Choice;
	TestEqual(TEXT("First accepts"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Duplicate"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::DuplicateOrAlreadyResolved);Frozen.Verify(*this,F);
	E.RequestId=F.NextA++;
	TestEqual(TEXT("Fresh same choice cannot rewrite"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
	E.RequestId=F.NextA++;E.Skill=Other;
	TestEqual(TEXT("Fresh different choice cannot rewrite"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillStale,
	"FMCodex.NetworkPlay.SkillTransport.04.RealAttackStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillStale::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests; FFixture F;
	if (!TestTrue(TEXT("Attack N Skill wait"),F.ReachSkill())) { return false; }
	auto Held=F.Request(F.A);--F.NextA;
	FMatchPlayAuthoritativeDeclineSkillRequest Decline;Decline.ExpectedAttackSequence=Held.ExpectedAttackSequence;Decline.RequestingSide=Side::PlayerA;
	TestTrue(TEXT("Existing canonical decline closes N"),Access::Session(*F.Mode).DeclineSkill(Decline).RuntimeEnvelope.bDomainSuccess);
	TestTrue(TEXT("Coordinator reaches next wait"),Access::Advance(*F.Mode).bSuccess);Access::Publish(*F.Mode);
	TestEqual(TEXT("Real N+1"),F.B->GetOwnerView().AttackSequence,Held.ExpectedAttackSequence+1);
	if (!TestTrue(TEXT("N+1 also Skill wait"),F.ReachSkill())) { return false; }
	Held.RequestId=F.NextA++;const FUnchanged Before(F);
	TestEqual(TEXT("Old N rejected"),F.Mode->SubmitConnectionPlayerIntent(F.A,Held).Code,Code::StaleAttackSequence);Before.Verify(*this,F);
	TestEqual(TEXT("Current B Skill accepts"),F.Mode->SubmitConnectionPlayerIntent(F.B,F.Request(F.B)).Code,Code::Accepted);
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSkillAckOrder,
	"FMCodex.NetworkPlay.SkillTransport.05.PendingOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexSkillAckOrder::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AckFirst"), TEXT("ViewFirst"), TEXT("Rejected")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexSkillAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkSkillTests;
	FFixture F;
	if (!TestTrue(TEXT("Real nine-kind client namespace setup"), F.ReachSkill())) { return false; }
	auto& C = F.ClientA; const auto View = F.A->GetOwnerView();
	auto Choice = View.SkillOptions[0].Choice;
	if (P == TEXT("Rejected")) { Choice.SkillId = TEXT("Forged.Skill"); }
	Envelope E;
	TestTrue(TEXT("Common pending begins"), C.BeginSkill(View, Choice, E));
	const auto Original = E;
	TestFalse(TEXT("Second click cannot emit"), C.BeginSkill(View, Choice, E));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillWire,
	"FMCodex.NetworkPlay.SkillTransport.08.BoundedWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests;
	for (const FName Name : {FName(TEXT("Canonical.Skill")), FName(TEXT("战术标识")), FName(*FString::ChrN(128, TEXT('x')))})
	{
		Payload P; P.SkillId = Name;
		TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = false;
		P.NetSerialize(Writer, nullptr, Success);
		TestTrue(TEXT("Bounded Skill write"), Success && Bytes.Num() <= 129);
		FMemoryReader Reader(Bytes); Payload Copy; Copy.NetSerialize(Reader, nullptr, Success);
		TestTrue(TEXT("Bounded Skill read"), Success && !Reader.IsError());
		TestEqual(TEXT("Canonical identity roundtrip"), Copy.SkillId, Name);
	}
	Payload Oversize; Oversize.SkillId = FName(*FString::ChrN(129, TEXT('x')));
	TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); bool Success = true;
	Oversize.NetSerialize(Writer, nullptr, Success);
	TestFalse(TEXT("Oversize sender fails codec"), Success);
	for (TArray<uint8> Invalid : {TArray<uint8>{129}, TArray<uint8>{2,'x'}, TArray<uint8>{1,0}, TArray<uint8>{2,0xC0,0xAF}})
	{
		FMemoryReader Reader(Invalid); Payload P; Success = true; P.NetSerialize(Reader, nullptr, Success);
		TestFalse(TEXT("Malformed wire fails safely"), Success);
		TestTrue(TEXT("Malformed identity not adopted"), P.IsEmpty());
	}
	for (const Kind K : {Kind::RequestInitialActionPointRoll, Kind::DeployOrdinary, Kind::DeployGoalkeeper, Kind::FinishDeployment, Kind::SubmitCarrier, Kind::SubmitMarker, Kind::SubmitRunner, Kind::SubmitHelper, Kind::SubmitSkill})
	{
		for (int32 Mask = 0; Mask < 128; ++Mask)
		{
			Envelope E; E.IntentKind = K;
			if (Mask & 1) { E.Deployment.CardId = TEXT("Card"); E.Deployment.SlotId = TEXT("Slot"); }
			if (Mask & 2) { E.Goalkeeper.SlotId = TEXT("Slot"); }
			if (Mask & 4) { E.Carrier.CarrierCardId = TEXT("Carrier"); }
			if (Mask & 8) { E.Marker.MarkerCardId = TEXT("Marker"); }
			if (Mask & 16) { E.Runner.RunnerCardId = TEXT("Runner"); }
			if (Mask & 32) { E.Helper.HelperCardId = TEXT("Helper"); }
			if (Mask & 64) { E.Skill.SkillId = TEXT("Skill"); }
			const int32 ExpectedMask = K == Kind::DeployOrdinary ? 1 : K == Kind::DeployGoalkeeper ? 2 : K == Kind::SubmitCarrier ? 4 : K == Kind::SubmitMarker ? 8 : K == Kind::SubmitRunner ? 16 : K == Kind::SubmitHelper ? 32 : K == Kind::SubmitSkill ? 64 : 0;
			TestEqual(TEXT("Nine kinds accept exactly their own shape"), E.ValidatePayloadShape(), Mask == ExpectedMask ? Code::None : Code::InvalidPayload);
		}
	}
	return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillSharedPath,
	"FMCodex.NetworkPlay.SkillTransport.09.SharedSemanticsAndGeneratedRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillSharedPath::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests;
	FFixture Network, Local;
	if (!TestTrue(TEXT("Equivalent canonical fixtures"), Network.ReachSkill() && Local.ReachSkill())) { return false; }
	TestTrue(TEXT("Identical before State"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const auto E = Network.Request(Network.A);
	FMatchPlayAuthoritativeSubmitSkillRequest Request;
	Request.ExpectedAttackSequence = E.ExpectedAttackSequence; Request.RequestingSide = Side::PlayerA; Request.SkillId = E.Skill.SkillId;
	const auto Intent = FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitSkill, Request);
	const int32 LocalBeforeCalls = Local.Calls();
	const auto LocalResult = Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);
	TestTrue(TEXT("Transport-neutral shared HostPort accepts"), LocalResult.bSuccess);
	TestEqual(TEXT("Shared local-side dispatch coordinates once"), Local.Calls(), LocalBeforeCalls + 1);
	TestTrue(TEXT("With legal Skill no internal action needed"), LocalResult.CoordinatorResult.Steps.IsEmpty());
	TestEqual(TEXT("Coordinator stops at actual player intent"), LocalResult.CoordinatorResult.StopReason, EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Network adapter accepts same canonical choice"), Network.Mode->SubmitConnectionPlayerIntent(Network.A, E).Code, Code::Accepted);
	TestTrue(TEXT("Local/Network final State identical"), SameState(Access::Session(*Network.Mode).GetStateSnapshot(), Access::Session(*Local.Mode).GetStateSnapshot()));
	const FUnchanged Frozen(Local);
	FMatchPlayPlayerIntent Wrong; Wrong.CommandKind = EMatchPlayAuthoritativeCommandKind::SubmitSkill;
	Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	TestEqual(TEXT("Canonical wrong variant rejected before Session"), Access::Runtime(*Local.Mode).SubmitPlayerIntent(Wrong).ErrorCode, EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Frozen.Verify(*this, Local);
	FString Source;
	TestTrue(TEXT("Controller source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitSkillChoice"));
	const int32 End = Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidSkill"), ESearchCase::CaseSensitive, ESearchDir::FromStart, Start);
	const auto Dispatch = Source.Mid(Start, End - Start);
	TestTrue(TEXT("Skill calls generated owning RPC"), Dispatch.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation shortcut"), Dispatch.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No Host authority bypass"), Dispatch.Contains(TEXT("HasAuthority")));
	TestFalse(TEXT("No client HostPort"), Dispatch.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local host source readable"), FFileHelper::LoadFileToString(Source,
		*(FPaths::ProjectDir() / TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	const int32 LocalStart = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitSkill:"));
	const int32 LocalEnd = Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::DeclineSkill:"), ESearchCase::CaseSensitive, ESearchDir::FromStart, LocalStart);
	TestTrue(TEXT("Actual Local Skill branch uses same shared port"), Source.Mid(LocalStart, LocalEnd - LocalStart).Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	return true;
}




IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillNamespace,
	"FMCodex.NetworkPlay.SkillTransport.11.NineIntentNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests;
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
	FFMCodexNetworkSkillOption Skill; Skill.Choice.SkillId = TEXT("Skill"); V.SkillOptions.Add(Skill);
	FFMCodexNetworkIntentClientState C; FFMCodexNetworkIntentLedger Ledger; Envelope E;
	for (int32 I = 1; I <= 9; ++I)
	{
		if (I == 5) { V.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection; }
		if (I == 6) { V.EntryWait = EFMCodexNetworkEntryWait::MarkerSelection; }
		if (I == 7) { V.EntryWait = EFMCodexNetworkEntryWait::RunnerSelection; }
		if (I == 8) { V.EntryWait = EFMCodexNetworkEntryWait::HelperSelection; }
		if (I == 9) { V.EntryWait = EFMCodexNetworkEntryWait::SkillSelection; }
		const bool Began = I == 1 ? C.Begin(V, E) : I == 2 ? C.BeginDeployment(V, Ordinary.Choice, E)
			: I == 3 ? C.BeginGoalkeeper(V, GK, E) : I == 4 ? C.BeginFinishDeployment(V, E) : I == 5 ? C.BeginCarrier(V, Carrier.Choice, E) : I == 6 ? C.BeginMarker(V, Marker.Choice, E) : I == 7 ? C.BeginRunner(V, Runner.Choice, E) : I == 8 ? C.BeginHelper(V, Helper.Choice, E) : C.BeginSkill(V, Skill.Choice, E);
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
	TestEqual(TEXT("Skill huge jump denied"), Ledger.Check(V.MatchInstanceId, Huge), Code::InvalidPayload);
	E.RequestId = 10;
	TestTrue(TEXT("Huge jump did not poison normal next ID"), Ledger.Consume(V.MatchInstanceId, E));
	return true;
}



IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillProjection,
	"FMCodex.NetworkPlay.SkillTransport.06.CompleteCanonicalProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests; FFixture F;
	if (!TestTrue(TEXT("Real two-choice fixture"),F.ReachSkill())) { return false; }
	const FUnchanged Before(F); const auto Safe=Access::Safe(*F.Mode,Side::PlayerA);
	const auto View=F.A->GetOwnerView();
	TestEqual(TEXT("Entire legal set"),View.SkillOptions.Num(),Safe.SelectionOptions.Num());
	for (int32 I=0;I<Safe.SelectionOptions.Num();++I)
	{
		TestEqual(TEXT("Skill identity is Id, not Carrier"),View.SkillOptions[I].Choice.SkillId,Safe.SelectionOptions[I].Id);
		TestNotEqual(TEXT("Carrier is distinct"),View.SkillOptions[I].Choice.SkillId,Safe.SelectionOptions[I].RelatedCardId);
		TestEqual(TEXT("Canonical Chinese skill name"),View.SkillOptions[I].SkillLabel.ToString(),
			FFMCodexPlayerUIPresentationText::Skill(FFMCodexLocalMatchInteractionViewBuilder::ToString(Safe.SelectionOptions[I].SkillType)).ToString());
	}
	TestTrue(TEXT("Nonactor has no actionable Skills"),F.B->GetOwnerView().SkillOptions.IsEmpty());
	TestTrue(TEXT("Accept last offered choice"),F.Send(F.A,Kind::SubmitSkill,{},{},{},{},{},View.SkillOptions.Last().Choice));
	TestEqual(TEXT("Public selected skill"),F.A->GetOwnerView().SelectedSkill.Choice.SkillId,View.SkillOptions.Last().Choice.SkillId);
	TestEqual(TEXT("Both public names match"),F.A->GetOwnerView().SelectedSkill.SkillLabel.ToString(),F.B->GetOwnerView().SelectedSkill.SkillLabel.ToString());
	for (Side Viewer : {Side::PlayerA,Side::PlayerB,Side::None})
	{
		const auto Hidden=Access::Safe(*F.Mode,Viewer,false);
		const auto Wire=FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden,F.Mode->GetMatchInstanceId(),99,Viewer,EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Hidden safe view withholds frozen Skill"),Hidden.SelectedSkillId.IsNone());
		TestTrue(TEXT("Hidden wire has no Skill fact or choices"),Wire.SelectedSkill.Choice.IsEmpty() && Wire.SkillOptions.IsEmpty());
	}
	TestEqual(TEXT("Projection consumes no entropy"),F.Entropy->Calls,Before.EntropyCalls);
	int32 MaxAssigned=0,MaxConcurrent=0;
	for (const auto& Player : FFMCodexPrototypeTeamContent::GetDefinitions())
	{
		MaxAssigned=FMath::Max(MaxAssigned,Player.SkillAssignments.Num());
		for (int32 TP=2;TP<=8;++TP)
		{
			int32 Count=0; for (const auto& Skill : Player.SkillAssignments)
			{ if (TP>=Skill.MinTacticalPoint && TP<=Skill.MaxTacticalPoint) { ++Count; } }
			MaxConcurrent=FMath::Max(MaxConcurrent,Count);
		}
	}
	TestEqual(TEXT("Canonical inventory upper bound"),MaxAssigned,3);
	TestEqual(TEXT("Current TP-compatible maximum, attained in fixture"),MaxConcurrent,View.SkillOptions.Num());
	TestEqual(TEXT("Current content max is two"),MaxConcurrent,2);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillBound,
	"FMCodex.NetworkPlay.SkillTransport.07.AtomicProjectionBound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillBound::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests; FFixture F;
	if (!TestTrue(TEXT("Safe source"),F.ReachSkill())) { return false; }
	auto Safe=Access::Safe(*F.Mode,Side::PlayerA);const auto Original=Safe.SelectionOptions[0];
	Safe.SelectionOptions.Reset();
	TestEqual(TEXT("Network bound covers theoretical canonical maximum"),FFMCodexNetworkClientViewSnapshot::MaxSkillOptions,FPlayerCardRuleSnapshotValidator::MaxSkillIdCount);
	for (int32 I=0;I<3;++I) { auto O=Original;O.Id=FName(*FString::Printf(TEXT("Bounded.Skill.%d"),I));Safe.SelectionOptions.Add(O); }
	auto Project=[&]() { return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),99,Side::PlayerA,EFMCodexNetworkBootstrapState::MatchReady); };
	TestEqual(TEXT("Three different Skills on same Carrier all represented"),Project().SkillOptions.Num(),3);
	auto Bad=[&]() { const auto V=Project();TestTrue(TEXT("Invalid set fails atomically and explicitly"),V.bSkillOptionsUnavailable && V.SkillOptions.IsEmpty()); };
	Safe.SelectionOptions.Add(Original);Bad();Safe.SelectionOptions.Pop();
	const auto Last=Safe.SelectionOptions.Last();
	for (FName Id : {FName(NAME_None),FName(*FString::ChrN(129,TEXT('x'))),Safe.SelectionOptions[0].Id})
	{ Safe.SelectionOptions.Last().Id=Id;Bad(); }
	Safe.SelectionOptions.Last()=Last;Safe.SelectionOptions.Last().Side=Side::PlayerB;Bad();
	Safe.SelectionOptions.Last()=Last;Safe.SelectionOptions.Last().SkillType=ESkillRuleType::None;
	TestEqual(TEXT("Unknown presentation type uses generic Chinese fallback"),Project().SkillOptions.Last().SkillLabel.ToString(),FString(TEXT("战术")));
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSkillPinned,
	"FMCodex.NetworkPlay.SkillTransport.12.ServerPinnedRulesBehavior",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSkillPinned::RunTest(const FString&)
{
	using namespace FMCodexNetworkSkillTests; FFixture Reference,Tampered;
	if (!TestTrue(TEXT("Identical real fixtures"),Reference.ReachSkill() && Tampered.ReachSkill())) { return false; }
	auto R=Reference.Request(Reference.A),T=Tampered.Request(Tampered.A);
	auto* SourceRule=Access::CallerRules(*Tampered.Mode).SkillRules.FindByPredicate([&](const auto& Rule){return Rule.SkillId==T.Skill.SkillId;});
	if (!TestNotNull(TEXT("Source rule exists"),SourceRule)) { return false; }
	SourceRule->SkillType=ESkillRuleType::LongShot;
	SourceRule->MinTriggerActionPoint=SourceRule->MaxTriggerActionPoint=2;
	const FUnchanged Before(Tampered);
	TestEqual(TEXT("Reference accepts"),Reference.Mode->SubmitConnectionPlayerIntent(Reference.A,R).Code,Code::Accepted);
	TestEqual(TEXT("Caller rule alteration cannot replace private Session rules"),Tampered.Mode->SubmitConnectionPlayerIntent(Tampered.A,T).Code,Code::Accepted);
	TestTrue(TEXT("Exact State identical despite forged source ActionType and TP range"),
		SameState(Access::Session(*Reference.Mode).GetStateSnapshot(),Access::Session(*Tampered.Mode).GetStateSnapshot()));
	TestEqual(TEXT("Pinned PassControl wins"),Access::Session(*Tampered.Mode).GetStateSnapshot().CurrentAttack.SelectedAction.ActionType,ESkillRuleType::PassControl);
	TestEqual(TEXT("Accepted exactly one Coordinator"),Tampered.Calls(),Before.CoordinatorCalls+1);
	TestEqual(TEXT("No RNG used"),Tampered.Entropy->Calls,Before.EntropyCalls);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSkillTransitions,
	"FMCodex.NetworkPlay.SkillTransport.13.CanonicalTransitionMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexSkillTransitions::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("LongShot"),TEXT("CutInsideShot"),TEXT("Cross"),TEXT("PassControl"),TEXT("ThroughBall"),TEXT("CrossWrongRunner")})
	{ Names.Add(N);Commands.Add(N); }
}
bool FFMCodexSkillTransitions::RunTest(const FString& P)
{
	using namespace FMCodexNetworkSkillTests;
	const bool Route=P==TEXT("PassControl") || P==TEXT("ThroughBall");
	const bool Saka=P.Contains(TEXT("Cross")) || P==TEXT("CutInsideShot");
	FFixture F(false,Route ? 6 : 4);
	if (!TestTrue(TEXT("Canonical participant-first fixture"),F.ReachSkill(
		Saka ? FName(TEXT("Prototype.Arsenal.BukayoSaka")) : NAME_None,
		P==TEXT("Cross") ? FName(TEXT("Prototype.Arsenal.KaiHavertz")) : NAME_None))) { return false; }
	if (P==TEXT("CrossWrongRunner"))
	{
		auto E=F.Request(F.A);E.Skill.SkillId=TEXT("Canonical.Skill.Cross.4.6");const FUnchanged Before(F);
		TestEqual(TEXT("Owned TP-compatible Cross rejected with midfield Runner"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);
		Before.Verify(*this,F);
		TestEqual(TEXT("Legal CutInside remains available"),F.Mode->SubmitConnectionPlayerIntent(F.A,F.Request(F.A)).Code,Code::Accepted);
		return true;
	}
	ESkillRuleType Type=P==TEXT("LongShot") ? ESkillRuleType::LongShot : P==TEXT("CutInsideShot") ? ESkillRuleType::CutInsideShot :
		P==TEXT("Cross") ? ESkillRuleType::Cross : P==TEXT("PassControl") ? ESkillRuleType::PassControl : ESkillRuleType::ThroughBall;
	const auto Safe=Access::Safe(*F.Mode,Side::PlayerA);
	const auto* Choice=Safe.SelectionOptions.FindByPredicate([&](const auto& O){return O.SkillType==Type;});
	if (!TestNotNull(TEXT("Requested family canonically legal"),Choice)) { return false; }
	FMatchPlayAuthoritativeSubmitSkillRequest R;
	R.ExpectedAttackSequence=F.A->GetOwnerView().AttackSequence;R.RequestingSide=Side::PlayerA;R.SkillId=Choice->Id;
	const FUnchanged Before(F);
	const auto Result=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitSkill,R));
	TestTrue(TEXT("Shared port accepts exact request"),Result.bSuccess);
	TestEqual(TEXT("Coordinator exactly once"),F.Calls(),Before.CoordinatorCalls+1);
	TestTrue(TEXT("No internal branch or roll generated"),Result.CoordinatorResult.Steps.IsEmpty());
	TestEqual(TEXT("Stops for next player intent"),Result.CoordinatorResult.StopReason,EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	Access::Publish(*F.Mode);const auto S=Access::Session(*F.Mode).GetStateSnapshot();
	TestEqual(TEXT("Resolution phase preserved"),S.CurrentAttack.Phase,EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Actual selection state"),S.CurrentAttack.SelectionStage,Route ? EMatchPlayCurrentAttackSelectionStage::ReadyForResolution : EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent);
	TestEqual(TEXT("Selected action only route category"),S.CurrentAttack.bHasSelectedAction,Route);
	TestFalse(TEXT("No resolution session yet"),S.CurrentAttack.bHasResolutionSession);
	TestFalse(TEXT("Skill no longer deferred"),S.CurrentAttack.ActionPreparation.bSkillSelectionDeferred);
	TestFalse(TEXT("No route outcome"),S.CurrentAttack.ResolutionSession.bHasActualBranch);
	const bool Clears=P==TEXT("LongShot") || P==TEXT("CutInsideShot");
	if (!Route)
	{
		TestEqual(TEXT("Chosen Skill frozen"),S.CurrentAttack.ActionPreparation.SkillId,R.SkillId);
		TestEqual(TEXT("Runner cleared only for nonparticipant tactics"),S.CurrentAttack.ActionPreparation.RunnerCardId.IsNone(),Clears);
		TestEqual(TEXT("Helper cleared only for nonparticipant tactics"),S.CurrentAttack.ActionPreparation.HelperCardId.IsNone(),Clears);
	}
	const auto V=F.A->GetOwnerView();
	TestEqual(TEXT("Next actor attacker"),V.ExpectedActingSide,Side::PlayerA);
	TestEqual(TEXT("Bounded next wait from canonical safe category"),V.EntryWait,
		!Route ? EFMCodexNetworkEntryWait::BranchIntentSelection : Type==ESkillRuleType::PassControl ? EFMCodexNetworkEntryWait::PassControlRouteRoll : EFMCodexNetworkEntryWait::ThroughBallRouteRoll);
	TestEqual(TEXT("No entropy"),F.Entropy->Calls,Before.EntropyCalls);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSkillAbsence,
	"FMCodex.NetworkPlay.SkillTransport.14.CanonicalAbsenceAndDecline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexSkillAbsence::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("NoLegalSkill"),TEXT("DeclineSkill"),TEXT("NoHelper"),TEXT("NoRunner")}) { Names.Add(N);Commands.Add(N); }
}
bool FFMCodexSkillAbsence::RunTest(const FString& P)
{
	using namespace FMCodexNetworkSkillTests; FFixture F(false,P==TEXT("NoRunner") ? 4 : 6);
	if (P==TEXT("DeclineSkill"))
	{
		if (!TestTrue(TEXT("Legal Skill wait"),F.ReachSkill())) { return false; }
		const FUnchanged Before(F); FMatchPlayAuthoritativeDeclineSkillRequest R;
		R.ExpectedAttackSequence=F.A->GetOwnerView().AttackSequence;R.RequestingSide=Side::PlayerA;
		TestTrue(TEXT("Existing canonical voluntary decline"),Access::Session(*F.Mode).DeclineSkill(R).RuntimeEnvelope.bDomainSuccess);
		const auto C=Access::Advance(*F.Mode);
		TestTrue(TEXT("Decline reaches next player wait without internal steps"),C.bSuccess && C.Steps.IsEmpty());
		TestFalse(TEXT("Decline closes active attack"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);
		TestEqual(TEXT("Decline no RNG"),F.Entropy->Calls,Before.EntropyCalls);
		return true;
	}
	if (P==TEXT("NoRunner"))
	{
		if (!TestTrue(TEXT("Awaiting Runner"),F.ReachHelper(2,true,false,false))) { return false; }
		FMatchPlayAuthoritativeDeclineRunnerRequest R;R.ExpectedAttackSequence=F.A->GetOwnerView().AttackSequence;R.RequestingSide=Side::PlayerA;
		TestTrue(TEXT("Canonical decline Runner"),Access::Session(*F.Mode).DeclineRunner(R).RuntimeEnvelope.bDomainSuccess);
		TestTrue(TEXT("Canonical absence continuation"),Access::Advance(*F.Mode).bSuccess);Access::Publish(*F.Mode);
	}
	else if (!TestTrue(TEXT("Canonical setup"),F.ReachHelper(P==TEXT("NoHelper") ? 0 : 2,P!=TEXT("NoLegalSkill"),false))) { return false; }
	const FUnchanged Before(F);
	if (P==TEXT("NoLegalSkill"))
	{
		FMatchPlayAuthoritativeSubmitHelperRequest R;R.ExpectedAttackSequence=F.A->GetOwnerView().AttackSequence;R.RequestingSide=Side::PlayerB;
		R.HelperCardId=F.B->GetOwnerView().HelperOptions[0].Choice.HelperCardId;
		const auto C=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitHelper,R));
		TestTrue(TEXT("Helper accepted"),C.bSuccess);
		TestEqual(TEXT("One canonical internal NoSkill step"),C.CoordinatorResult.Steps.Num(),1);
		if (C.CoordinatorResult.Steps.Num()==1) { TestEqual(TEXT("NoSkill is server internal"),C.CoordinatorResult.Steps[0].CommandKind,EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill); }
		TestFalse(TEXT("NoSkill closes current attack"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);
	}
	else
	{
		TestEqual(TEXT("Absence allows actual legal Skill wait"),F.A->GetOwnerView().EntryWait,EFMCodexNetworkEntryWait::SkillSelection);
		TestEqual(TEXT("Skill valid without optional participants"),F.Mode->SubmitConnectionPlayerIntent(F.A,F.Request(F.A)).Code,Code::Accepted);
	}
	TestEqual(TEXT("No RNG"),F.Entropy->Calls,Before.EntropyCalls);
	return true;
}
#endif
