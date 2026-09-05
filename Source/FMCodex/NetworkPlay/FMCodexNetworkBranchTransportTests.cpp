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
#include "../CoreRules/MatchPlayCurrentAttackBranchIntentSelectionWriter.h"

struct FFMCodexNetworkBranchTestAccess
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
		TArray<uint32> Values; Values.Init(uint32(RawD12 - 1), 32);
		auto Entropy = MakeUnique<FFMCodexNetworkScriptedEntropy>(Values);
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

namespace FMCodexNetworkBranchTests
{
	using Access = FFMCodexNetworkBranchTestAccess;
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
		FFMCodexNetworkScriptedEntropy* Entropy;
		FFMCodexNetworkIntentClientState ClientA, ClientB;
		int64 NextA = 1, NextB = 1;
		explicit FFixture(bool BFirst = false, int32 RawD12 = 4)
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
			E.ExpectedAttackSequence = A->GetOwnerView().AttackSequence; E.IntentKind = Kind::SubmitBranchIntent;
			if (!Attacker()->GetOwnerView().BranchOptions.IsEmpty())
			{ E.Branch = Attacker()->GetOwnerView().BranchOptions[0].Choice; }
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
				: K == Kind::SubmitBranchIntent && C.BeginBranch(V, BranchChoice, E);
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


IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexBranchAccepted,
	"FMCodex.NetworkPlay.BranchTransport.01.AllFamiliesBothActors",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexBranchAccepted::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* Side : {TEXT("A"),TEXT("B")})
	{
		for (const TCHAR* Type : {TEXT("LongShot"),TEXT("CutInside"),TEXT("Cross")})
		{
			for (int32 Choice=0;Choice<2;++Choice)
			{
				const auto N=FString::Printf(TEXT("%s.%s.%d"),Side,Type,Choice);Names.Add(N);Commands.Add(N);
			}
		}
	}
}
bool FFMCodexBranchAccepted::RunTest(const FString& P)
{
	using namespace FMCodexNetworkBranchTests;
	const bool Cross=P.Contains(TEXT("Cross")),Cut=P.Contains(TEXT("CutInside"));
	const auto Type=Cross ? ESkillRuleType::Cross : Cut ? ESkillRuleType::CutInsideShot : ESkillRuleType::LongShot;
	FFixture Network(P.StartsWith(TEXT("B"))),Local(P.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Both equivalent canonical fixtures"),Network.ReachBranch(Type) && Local.ReachBranch(Type))) { return false; }
	const FUnchanged Before(Network); auto E=Network.Request(Network.Attacker());
	const auto View=Network.Attacker()->GetOwnerView();const auto Safe=Access::Safe(*Network.Mode,View.ViewerSide);
	if (!TestEqual(TEXT("Full two-choice set"),View.BranchOptions.Num(),2)) { return false; }
	TestEqual(TEXT("Canonical set count"),View.BranchOptions.Num(),Safe.BranchIntentOptions.Num());
	for (int32 I=0;I<2;++I)
	{
		TestEqual(TEXT("Exact canonical identity/order"),View.BranchOptions[I].Choice.Intent,Safe.BranchIntentOptions[I]);
		const TCHAR* Label=Cross ? (I==0 ? TEXT("Cross High") : TEXT("Cross Low")) : (I==0 ? TEXT("Direct Shot") : TEXT("Dead Corner"));
		TestEqual(TEXT("Existing canonical Chinese label"),View.BranchOptions[I].BranchLabel.ToString(),FFMCodexPlayerUIPresentationText::MatchScreenLabel(Label).ToString());
	}
	TestTrue(TEXT("Nonacting viewer has no choices"),Network.Defender()->GetOwnerView().BranchOptions.IsEmpty());
	const int32 Index=P.EndsWith(TEXT("1")) ? 1 : 0; E.Branch=View.BranchOptions[Index].Choice;
	FMatchPlayAuthoritativeSubmitBranchIntentRequest Request;Request.AttackSequence=E.ExpectedAttackSequence;
	Request.RequestingSide=View.ViewerSide;Request.Intent=E.Branch.Intent;
	const auto LocalResult=Access::Runtime(*Local.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent,Request));
	TestTrue(TEXT("Local/shared canonical port accepts"),LocalResult.bSuccess);
	const auto Ack=Network.Mode->SubmitConnectionPlayerIntent(Network.Attacker(),E);
	if (!TestEqual(TEXT("Network accepts"),Ack.Code,Code::Accepted)) { return false; }
	const auto S=Access::Session(*Network.Mode).GetStateSnapshot();const auto& A=S.CurrentAttack;const auto& R=A.ResolutionSession;
	TestTrue(TEXT("Full Local/Network State identical"),SameState(S,Access::Session(*Local.Mode).GetStateSnapshot()));
	TestEqual(TEXT("One network Coordinator"),Network.Calls(),Before.CoordinatorCalls+1);
	TestEqual(TEXT("One publication"),Ack.ViewRevision,Before.Revision+1);
	TestEqual(TEXT("No entropy"),Network.Entropy->Calls,Before.EntropyCalls);
	TestEqual(TEXT("No entry provider"),Access::Runtime(*Network.Mode).GetEntryProviderInvocationCount(),Before.EntryCalls);
	TestEqual(TEXT("No D12 provider"),Access::Runtime(*Network.Mode).GetD12ProviderInvocationCount(),Before.D12Calls);
	TestEqual(TEXT("Phase unchanged"),A.Phase,EMatchPlayCurrentAttackPhase::Resolution);
	TestEqual(TEXT("Selection ready"),A.SelectionStage,EMatchPlayCurrentAttackSelectionStage::ReadyForResolution);
	TestTrue(TEXT("Selected action frozen"),A.bHasSelectedAction);
	TestEqual(TEXT("Frozen active Skill unchanged"),A.SelectedAction.SkillId,Before.State.CurrentAttack.ActionPreparation.SkillId);
	TestEqual(TEXT("Frozen ActionType unchanged"),A.SelectedAction.ActionType,Type);
	TestEqual(TEXT("Canonical branch frozen"),A.SelectedAction.ElectiveBranchIntent,E.Branch.Intent);
	TestEqual(TEXT("Session created only for deterministic shot route"),A.bHasResolutionSession,!Cross);
	TestEqual(TEXT("Actual route resolved only for shot"),R.bHasActualBranch,!Cross);
	TestTrue(TEXT("No route RNG record"),R.InitialRouteRollRecords.IsEmpty());
	TestEqual(TEXT("No player roll started"),R.PostRouteRollProgress.Phase,EMatchPlayCurrentAttackPostRouteRollPhase::None);
	TestEqual(TEXT("Stops before next player command"),LocalResult.CoordinatorResult.StopReason,EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
	TestEqual(TEXT("Canonical internal steps"),LocalResult.CoordinatorResult.Steps.Num(),Cross ? 0 : 2);
	if (!Cross && LocalResult.CoordinatorResult.Steps.Num()==2)
	{
		TestEqual(TEXT("Begin session internal"),LocalResult.CoordinatorResult.Steps[0].CommandKind,EMatchPlayAuthoritativeCommandKind::BeginResolutionSession);
		TestEqual(TEXT("Intent route internal"),LocalResult.CoordinatorResult.Steps[1].CommandKind,EMatchPlayAuthoritativeCommandKind::ResolveIntentDeterminedRoute);
		TestEqual(TEXT("Route stage"),R.Stage,EMatchPlayCurrentAttackResolutionStage::RouteResolved);
		TestEqual(TEXT("Exact deterministic route"),Cut ? static_cast<int32>(R.ActualBranch.CutInsideShot) : static_cast<int32>(R.ActualBranch.LongShot),Index+1);
	}
	const auto Wait=Cross ? EFMCodexNetworkEntryWait::CrossRouteRoll : Cut
		? (Index==0 ? EFMCodexNetworkEntryWait::CutInsideDirectAttackRoll : EFMCodexNetworkEntryWait::CutInsideDeadCornerRoll)
		: (Index==0 ? EFMCodexNetworkEntryWait::LongShotDirectAttackRoll : EFMCodexNetworkEntryWait::LongShotDeadCornerRoll);
	for (auto* PC : {Network.A,Network.B})
	{
		const auto V=PC->GetOwnerView();
		TestEqual(TEXT("Exact next wait"),V.EntryWait,Wait);
		TestEqual(TEXT("Next actor attacker"),V.ExpectedActingSide,View.ViewerSide);
		TestEqual(TEXT("Stable public branch"),V.SelectedBranch.Choice.Intent,E.Branch.Intent);
		TestEqual(TEXT("Stable public branch label"),V.SelectedBranch.BranchLabel.ToString(),View.BranchOptions[Index].BranchLabel.ToString());
		TestTrue(TEXT("No further branch controls"),V.BranchOptions.IsEmpty());
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchReplay,
	"FMCodex.NetworkPlay.BranchTransport.03.FrozenRewriteAndReplay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchReplay::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests; FFixture F;
	if (!TestTrue(TEXT("Two legal choices"),F.ReachBranch())) { return false; }
	auto E=F.Request(F.A); const auto Other=F.A->GetOwnerView().BranchOptions.Last().Choice;
	TestEqual(TEXT("First accepts"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::Accepted);
	const FUnchanged Frozen(F);
	TestEqual(TEXT("Duplicate"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::DuplicateOrAlreadyResolved);Frozen.Verify(*this,F);
	E.RequestId=F.NextA++;
	TestEqual(TEXT("Fresh same choice cannot rewrite"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
	E.RequestId=F.NextA++;E.Branch=Other;
	TestEqual(TEXT("Fresh different choice cannot rewrite"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);Frozen.Verify(*this,F);
	return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexBranchAckOrder,
	"FMCodex.NetworkPlay.BranchTransport.05.PendingOrders",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexBranchAckOrder::GetTests(TArray<FString>& Names, TArray<FString>& Commands) const
{
	for (const TCHAR* Name : {TEXT("AckFirst"), TEXT("ViewFirst"), TEXT("Rejected")}) { Names.Add(Name); Commands.Add(Name); }
}
bool FFMCodexBranchAckOrder::RunTest(const FString& P)
{
	using namespace FMCodexNetworkBranchTests;
	FFixture F;
	if (!TestTrue(TEXT("Real ten-kind client namespace setup"), F.ReachBranch())) { return false; }
	auto& C = F.ClientA; const auto View = F.A->GetOwnerView();
	auto Choice = View.BranchOptions[0].Choice;
	if (P == TEXT("Rejected")) { Choice.Intent = Branch::CrossHigh; }
	Envelope E;
	TestTrue(TEXT("Common pending begins"), C.BeginBranch(View, Choice, E));
	FFMCodexNetworkPlayerIntentAck Stale;
	Stale.MatchInstanceId=E.MatchInstanceId; Stale.RequestId=E.RequestId+1; Stale.Code=Code::Accepted; Stale.ViewRevision=View.ViewRevision+1;
	TestFalse(TEXT("Uncorrelated ACK cannot establish a branch"),C.ObserveAck(Stale));
	TestTrue(TEXT("Stale ACK leaves pending"),C.IsPending());
	const auto Original = E;
	TestFalse(TEXT("Second click cannot emit"), C.BeginBranch(View, Choice, E));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchNamespace,
	"FMCodex.NetworkPlay.BranchTransport.11.TenIntentNamespace",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchNamespace::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests;
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
	FFMCodexNetworkBranchOption B; B.Choice.Intent = Branch::DirectShot; V.BranchOptions.Add(B);
	FFMCodexNetworkIntentClientState C; FFMCodexNetworkIntentLedger Ledger; Envelope E;
	for (int32 I = 1; I <= 10; ++I)
	{
		if (I == 5) { V.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection; }
		if (I == 6) { V.EntryWait = EFMCodexNetworkEntryWait::MarkerSelection; }
		if (I == 7) { V.EntryWait = EFMCodexNetworkEntryWait::RunnerSelection; }
		if (I == 8) { V.EntryWait = EFMCodexNetworkEntryWait::HelperSelection; }
		if (I == 9) { V.EntryWait = EFMCodexNetworkEntryWait::SkillSelection; }
		if (I == 10) { V.EntryWait = EFMCodexNetworkEntryWait::BranchIntentSelection; }
		const bool Began = I == 1 ? C.Begin(V, E) : I == 2 ? C.BeginDeployment(V, Ordinary.Choice, E)
			: I == 3 ? C.BeginGoalkeeper(V, GK, E) : I == 4 ? C.BeginFinishDeployment(V, E) : I == 5 ? C.BeginCarrier(V, Carrier.Choice, E) : I == 6 ? C.BeginMarker(V, Marker.Choice, E) : I == 7 ? C.BeginRunner(V, Runner.Choice, E) : I == 8 ? C.BeginHelper(V, Helper.Choice, E) : I == 9 ? C.BeginSkill(V, Skill.Choice, E) : C.BeginBranch(V, B.Choice, E);
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
	TestEqual(TEXT("Branch huge jump denied"), Ledger.Check(V.MatchInstanceId, Huge), Code::InvalidPayload);
	E.RequestId = 11;
	TestTrue(TEXT("Huge jump did not poison normal next ID"), Ledger.Consume(V.MatchInstanceId, E));
	return true;
}




IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexBranchRejected,
	"FMCodex.NetworkPlay.BranchTransport.02.SecurityRejections",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexBranchRejected::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	for (const TCHAR* N : {TEXT("WrongSide"),TEXT("CrossUnderLongShot"),TEXT("CrossUnderCutInside"),TEXT("ShotUnderCross"),
		TEXT("BeforeSkill"),TEXT("PassControl"),TEXT("ThroughBall"),TEXT("Empty"),TEXT("OutOfRange"),
		TEXT("SkillMember"),TEXT("HelperMember"),TEXT("RunnerMember"),TEXT("MarkerMember"),TEXT("CarrierMember"),
		TEXT("OrdinaryMember"),TEXT("GoalkeeperMember"),TEXT("WrongTag"),TEXT("WrongMatch"),TEXT("WrongSequence"),
		TEXT("HugeId"),TEXT("ZeroId"),TEXT("ZeroSequence"),TEXT("InternalTag"),TEXT("Nonparticipant")})
	{ Names.Add(N);Commands.Add(N); }
}
bool FFMCodexBranchRejected::RunTest(const FString& P)
{
	using namespace FMCodexNetworkBranchTests;
	if (P==TEXT("BeforeSkill"))
	{
		FFixture F;const FUnchanged Before(F);auto E=F.Request(F.A);E.Branch.Intent=Branch::DirectShot;
		TestEqual(TEXT("No Skill frozen"),F.Mode->SubmitConnectionPlayerIntent(F.A,E).Code,Code::AuthorityRejected);
		Before.Verify(*this,F);return true;
	}
	const bool Route=P==TEXT("PassControl") || P==TEXT("ThroughBall");
	FFixture F(false,Route ? 6 : 4);
	const auto Type=P==TEXT("ShotUnderCross") ? ESkillRuleType::Cross : P==TEXT("CrossUnderCutInside") ? ESkillRuleType::CutInsideShot
		: P==TEXT("PassControl") ? ESkillRuleType::PassControl : P==TEXT("ThroughBall") ? ESkillRuleType::ThroughBall : ESkillRuleType::LongShot;
	if (!TestTrue(TEXT("Frozen server Skill fixture"),F.ReachBranch(Type))) { return false; }
	auto* PC=P==TEXT("WrongSide") ? F.B : F.A;
	auto E=F.Request(PC);const int64 NormalId=E.RequestId;Code Expected=Code::AuthorityRejected;
	if (Route || P==TEXT("ShotUnderCross")) { E.Branch.Intent=Branch::DirectShot; }
	if (P.StartsWith(TEXT("CrossUnder"))) { E.Branch.Intent=Branch::CrossHigh; }
	if (P==TEXT("Empty")) { E.Branch={};Expected=Code::InvalidPayload; }
	if (P==TEXT("OutOfRange")) { E.Branch.Intent=static_cast<Branch>(255);Expected=Code::InvalidPayload; }
	if (P==TEXT("SkillMember")) { E.Skill.SkillId=TEXT("Canonical.Skill.Cross.4.6");Expected=Code::InvalidPayload; }
	if (P==TEXT("HelperMember")) { E.Helper.HelperCardId=TEXT("Helper");Expected=Code::InvalidPayload; }
	if (P==TEXT("RunnerMember")) { E.Runner.RunnerCardId=TEXT("Runner");Expected=Code::InvalidPayload; }
	if (P==TEXT("MarkerMember")) { E.Marker.MarkerCardId=TEXT("Marker");Expected=Code::InvalidPayload; }
	if (P==TEXT("CarrierMember")) { E.Carrier.CarrierCardId=TEXT("Carrier");Expected=Code::InvalidPayload; }
	if (P==TEXT("OrdinaryMember")) { E.Deployment.CardId=TEXT("Card");Expected=Code::InvalidPayload; }
	if (P==TEXT("GoalkeeperMember")) { E.Goalkeeper.SlotId=TEXT("Slot");Expected=Code::InvalidPayload; }
	if (P==TEXT("WrongTag")) { E.IntentKind=Kind::SubmitSkill;Expected=Code::InvalidPayload; }
	if (P==TEXT("WrongMatch")) { E.MatchInstanceId=FGuid::NewGuid();Expected=Code::MatchMismatch; }
	if (P==TEXT("WrongSequence")) { ++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence; }
	if (P==TEXT("HugeId")) { E.RequestId=MAX_int64;Expected=Code::InvalidPayload; }
	if (P==TEXT("ZeroId")) { E.RequestId=0;Expected=Code::InvalidPayload; }
	if (P==TEXT("ZeroSequence")) { E.ExpectedAttackSequence=0;Expected=Code::InvalidPayload; }
	if (P==TEXT("InternalTag")) { E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent; }
	if (P==TEXT("Nonparticipant")) { PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant; }
	const FUnchanged Before(F);
	TestEqual(TEXT("Exact rejection"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Expected);
	Before.Verify(*this,F);
	if (!Route)
	{
		auto Normal=F.Request(F.A);if (P==TEXT("HugeId")) { Normal.RequestId=NormalId; }
		TestEqual(TEXT("Next normal current-family choice remains usable"),F.Mode->SubmitConnectionPlayerIntent(F.A,Normal).Code,Code::Accepted);
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchStale,
	"FMCodex.NetworkPlay.BranchTransport.04.RealAttackStaleness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchStale::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests;FFixture F;
	if (!TestTrue(TEXT("Attack N branch wait"),F.ReachBranch())) { return false; }
	auto Held=F.Request(F.A);--F.NextA;Payload Actual;Actual.Intent=Branch::DeadCorner;
	TestTrue(TEXT("N branch through common pending/transport"),F.Send(F.A,Kind::SubmitBranchIntent,{},{},{},{},{},{},Actual));
	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest Roll;Roll.AttackSequence=Held.ExpectedAttackSequence;Roll.RequestingSide=Side::PlayerA;
	TestTrue(TEXT("Canonical player roll closes N in fixture only"),Access::Session(*F.Mode).ResolveLongShotDeadCornerRoll(Roll).RuntimeEnvelope.bDomainSuccess);
	TestEqual(TEXT("Terminal requires explicit canonical advance"),Access::Advance(*F.Mode).StopReason,EMatchPlayServerCoordinatorStopReason::TerminalPendingAdvance);
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest Advance;Advance.AttackSequence=Held.ExpectedAttackSequence;Advance.RequestingSide=Side::PlayerA;
	TestTrue(TEXT("Explicit fixture-only terminal advance"),Access::Session(*F.Mode).AdvanceAfterTerminal(Advance).CompletionResult.bSuccess);
	TestTrue(TEXT("Canonical next player wait"),Access::Advance(*F.Mode).bSuccess);Access::Publish(*F.Mode);
	TestEqual(TEXT("Next real sequence"),F.B->GetOwnerView().AttackSequence,Held.ExpectedAttackSequence+1);
	if (!TestTrue(TEXT("N+1 also actual Branch wait"),F.ReachBranch())) { return false; }
	Held.RequestId=F.NextA++;const FUnchanged Before(F);
	TestEqual(TEXT("Old sequence with fresh ID rejected"),F.Mode->SubmitConnectionPlayerIntent(F.A,Held).Code,Code::StaleAttackSequence);
	Before.Verify(*this,F);
	TestEqual(TEXT("Current B branch still accepts"),F.Mode->SubmitConnectionPlayerIntent(F.B,F.Request(F.B)).Code,Code::Accepted);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchProjection,
	"FMCodex.NetworkPlay.BranchTransport.06.AtomicBoundAndDisclosure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchProjection::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests;FFixture F;
	if (!TestTrue(TEXT("Real safe branch source"),F.ReachBranch())) { return false; }
	auto Safe=Access::Safe(*F.Mode,Side::PlayerA);const FUnchanged Before(F);
	auto Project=[&](){return FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),99,Side::PlayerA,EFMCodexNetworkBootstrapState::MatchReady);};
	TestEqual(TEXT("All three families theoretical max two"),FFMCodexNetworkClientViewSnapshot::MaxBranchOptions,2);
	TestEqual(TEXT("Entire two-choice set"),Project().BranchOptions.Num(),2);
	auto Bad=[&](){const auto V=Project();TestTrue(TEXT("No partial choices on invalid representation"),V.bBranchOptionsUnavailable && V.BranchOptions.IsEmpty());};
	Safe.BranchIntentOptions.Add(Branch::CrossHigh);Bad();Safe.BranchIntentOptions.Pop();
	for (auto I : {Branch::DirectShot,Branch::None,static_cast<Branch>(255)}) { Safe.BranchIntentOptions[1]=I;Bad(); }
	Safe.BranchIntentOptions[1]=Branch::DeadCorner;Safe.ExpectedActingPlayer=Side::PlayerB;
	TestTrue(TEXT("No nonactor options even if source contains them"),Project().BranchOptions.IsEmpty());
	TestEqual(TEXT("Accept branch"),F.Mode->SubmitConnectionPlayerIntent(F.A,F.Request(F.A)).Code,Code::Accepted);
	for (auto SideValue : {Side::PlayerA,Side::PlayerB,Side::None})
	{
		const auto Hidden=Access::Safe(*F.Mode,SideValue,false);
		const auto V=FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden,F.Mode->GetMatchInstanceId(),99,SideValue,EFMCodexNetworkBootstrapState::MatchReady);
		TestTrue(TEXT("Undisclosed branch fact and controls withheld"),V.SelectedBranch.Choice.IsEmpty() && V.BranchOptions.IsEmpty());
	}
	TestEqual(TEXT("Projection never uses entropy"),F.Entropy->Calls,Before.EntropyCalls);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchWire,
	"FMCodex.NetworkPlay.BranchTransport.07.FixedWireAndClosedTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchWire::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests;
	for (int32 Raw=0;Raw<=255;++Raw)
	{
		TArray<uint8> Bytes{static_cast<uint8>(Raw)};FMemoryReader Reader(Bytes);Payload P;bool OK=true;
		P.NetSerialize(Reader,nullptr,OK);
		TestEqual(TEXT("Only known enum values decode, including inactive None"),OK,Raw<=4);
		TestEqual(TEXT("Only genuine choices have valid active shape"),P.IsValidShape(),Raw>=1 && Raw<=4);
		if (Raw<=4)
		{
			TArray<uint8> Encoded;FMemoryWriter Writer(Encoded);P.NetSerialize(Writer,nullptr,OK);
			TestTrue(TEXT("Fixed one-byte exact roundtrip"),OK && Encoded==Bytes);
		}
		else { TestTrue(TEXT("Unknown wire value not adopted"),P.IsEmpty()); }
	}
	TArray<uint8> Empty;FMemoryReader Truncated(Empty);Payload P;bool OK=true;P.NetSerialize(Truncated,nullptr,OK);
	TestFalse(TEXT("Truncated wire rejects"),OK);
	TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);P.Intent=static_cast<Branch>(255);P.NetSerialize(Writer,nullptr,OK);
	TestFalse(TEXT("Invalid sender cannot serialize valid choice"),OK);
	for (int32 Tag=1;Tag<=10;++Tag)
	{
		for (int32 Mask=0;Mask<256;++Mask)
		{
			Envelope E;E.IntentKind=static_cast<Kind>(Tag);
			if (Mask&1) { E.Deployment.CardId=TEXT("Card");E.Deployment.SlotId=TEXT("Slot"); }
			if (Mask&2) { E.Goalkeeper.SlotId=TEXT("Slot"); }
			if (Mask&4) { E.Carrier.CarrierCardId=TEXT("Carrier"); }
			if (Mask&8) { E.Marker.MarkerCardId=TEXT("Marker"); }
			if (Mask&16) { E.Runner.RunnerCardId=TEXT("Runner"); }
			if (Mask&32) { E.Helper.HelperCardId=TEXT("Helper"); }
			if (Mask&64) { E.Skill.SkillId=TEXT("Skill"); }
			if (Mask&128) { E.Branch.Intent=Branch::DirectShot; }
			const int32 Expected=Tag==1 || Tag==4 ? 0 : Tag==2 ? 1 : Tag==3 ? 2 : 1<<(Tag-3);
			TestEqual(TEXT("Ten tags accept exactly their own closed member"),E.ValidatePayloadShape(),Mask==Expected ? Code::None : Code::InvalidPayload);
		}
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchShared,
	"FMCodex.NetworkPlay.BranchTransport.08.SharedPathAndGeneratedRpc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchShared::RunTest(const FString&)
{
	using namespace FMCodexNetworkBranchTests;FFixture F;
	if (!TestTrue(TEXT("Branch fixture"),F.ReachBranch())) { return false; }
	const FUnchanged Before(F);FMatchPlayPlayerIntent Wrong;Wrong.CommandKind=EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent;
	Wrong.Payload.Set<FMatchPlayFullD12EntryRequest>({});
	TestEqual(TEXT("Wrong canonical variant rejected before Session"),Access::Runtime(*F.Mode).SubmitPlayerIntent(Wrong).ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);
	Before.Verify(*this,F);
	FString Source;
	TestTrue(TEXT("Controller source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"))));
	const int32 Start=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::SubmitBranchChoice"));
	const int32 End=Source.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevProbeInvalidBranch"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	const auto Block=Source.Mid(Start,End-Start);
	TestTrue(TEXT("Same generated owner RPC"),Block.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No direct implementation or authority bypass"),Block.Contains(TEXT("_Implementation")) || Block.Contains(TEXT("HasAuthority")) || Block.Contains(TEXT("HostPort")));
	TestTrue(TEXT("Local source"),FFileHelper::LoadFileToString(Source,*(FPaths::ProjectDir()/TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	const int32 Local=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent:"));
	const int32 Next=Source.Find(TEXT("case EMatchPlayAuthoritativeCommandKind::SubmitThroughBallOneOnOneShotChoice:"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Local);
	TestTrue(TEXT("Actual Local typed branch uses same port"),Source.Mid(Local,Next-Local).Contains(TEXT("FMatchPlayEntryDeploymentPlayerIntentPort")));
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexBranchFixture,
	"FMCodex.NetworkPlay.BranchTransport.09.CanonicalServerFixture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexBranchFixture::RunTest(const FString&)
{
	const auto Normal=FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch();
	const auto Fixture=FFMCodexNetworkBootstrapConfigurationFactory::CreateBFirstAutomationMatch(true);
	const auto& A=Normal.MatchConfiguration.OpeningInput.OpeningInput.PlayerBDeck;
	const auto& B=Fixture.MatchConfiguration.OpeningInput.OpeningInput.PlayerBDeck;
	TestEqual(TEXT("Canonical deck count preserved"),A.Num(),B.Num());
	for (const auto& Card : A)
	{
		const auto* Other=B.FindByPredicate([&](const auto& C){return C.CardId==Card.CardId;});
		if (!TestNotNull(TEXT("Every canonical identity retained"),Other)) { return false; }
		TestTrue(TEXT("Entire canonical card unchanged; only order differs"),FPlayerCardData::StaticStruct()->CompareScriptStruct(&Card,Other,0));
	}
	TestEqual(TEXT("Doku occupies prior Rodri offered position"),B[A.IndexOfByPredicate([](const auto& C){return C.CardId==FName(TEXT("Prototype.ManchesterCity.Rodri"));})].CardId,FName(TEXT("Prototype.ManchesterCity.JeremyDoku")));
	return true;
}
#endif
