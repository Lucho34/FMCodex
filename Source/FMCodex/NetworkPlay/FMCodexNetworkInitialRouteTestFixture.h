#pragma once
// Shared canonical setup for the initial-route and Cross-contest transport test consumers.
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
			// These controller/Session tests do not simulate physics. Avoid one Chaos TLS slot per temporary world.
			const auto Initialization = UWorld::InitializationValues().CreatePhysicsScene(false);
			World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Initialization);
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
				: K == Kind::SubmitBranchIntent ? C.BeginBranch(V, BranchChoice, E)
				: K == Kind::CrossInitialRouteRoll || K == Kind::PassControlInitialRouteRoll || K == Kind::ThroughBallInitialRouteRoll
					? C.BeginInitialRoute(V, K, E) : C.BeginCrossContest(V, K, E);
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
	inline bool SameState(const FMatchPlayState& A, const FMatchPlayState& B)
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



#endif
