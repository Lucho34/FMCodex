#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"

namespace FMCodexLocalMatchControlSurfaceTests
{
	void AcknowledgeIfPending(
		AFMCodexLocalMatchPlayerController& Controller)
	{
		if (Controller.IsAwaitingHotSeatHandoff())
		{
			Controller.AcknowledgeHotSeatHandoff();
		}
	}

	class FScopedPlayableWorld final
	{
	public:
		FScopedPlayableWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr || GEngine == nullptr)
			{
				return;
			}

			FWorldContext& Context =
				GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			Controller = World->SpawnActor<AFMCodexLocalMatchPlayerController>();
			if (Controller != nullptr)
			{
				Controller->RefreshPresentation();
			}
		}

		~FScopedPlayableWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		AFMCodexLocalMatchHostGameMode* GetHost() const { return Host; }
		AFMCodexLocalMatchPlayerController* GetController() const
		{
			return Controller;
		}

	private:
		UWorld* World = nullptr;
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;
	};

	bool LoadProductionSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	TArray<uint8> SerializeState(const FMatchPlayState& State)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes);
		FMatchPlayState Copy = State;
		FMatchPlayState::StaticStruct()->SerializeItem(Writer, &Copy, nullptr);
		return Bytes;
	}

	bool DeployNextOrdinary(
		AFMCodexLocalMatchPlayerController& Controller,
		const FString& SlotFragment)
	{
		AcknowledgeIfPending(Controller);
		const auto& View = Controller.GetInteractionView();
		for (const FFMCodexLocalMatchDeploymentOption& Option
			: View.DeploymentOptions)
		{
			if (!Option.bGoalkeeper
				&& Option.SlotId.ToString().Contains(SlotFragment))
			{
				Controller.DeployOrdinary(Option.CardId, Option.SlotId);
				return Controller.GetLastDiagnostic().bHostSuccess;
			}
		}
		return false;
	}

	bool SubmitFirstSelection(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		AcknowledgeIfPending(Controller);
		const auto& View = Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
			Controller.SubmitCarrier(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			Controller.SubmitMarker(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
			Controller.SubmitSkill(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			Controller.SubmitRunner(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			Controller.SubmitHelper(Id); break;
		default:
			return false;
		}
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	int32 FindSeedForRolls(const TArray<int32>& Rolls)
	{
		for (int32 Seed = 0; Seed < 4000000; ++Seed)
		{
			FRandomStream Stream(Seed);
			bool bMatches = true;
			for (const int32 Expected : Rolls)
			{
				if (Stream.RandRange(1, 6) != Expected)
				{
					bMatches = false;
					break;
				}
			}
			if (bMatches)
			{
				return Seed;
			}
		}
		return INDEX_NONE;
	}

	FFMCodexLocalMatchInteractionView ViewFor(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules)
	{
		return FFMCodexLocalMatchInteractionViewBuilder::Build(
			Host.GetMatchSnapshot().Snapshot, Rules);
	}

	bool DeployNextOrdinary(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules,
		const FString& SlotFragment)
	{
		const FFMCodexLocalMatchInteractionView View = ViewFor(Host, Rules);
		for (const FFMCodexLocalMatchDeploymentOption& Option
			: View.DeploymentOptions)
		{
			if (Option.bGoalkeeper
				|| !Option.SlotId.ToString().Contains(SlotFragment))
			{
				continue;
			}
			FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
			Request.RequestingSide = Option.Side;
			Request.CardId = Option.CardId;
			Request.SlotId = Option.SlotId;
			return Host.DeployOrdinary(Request).bSuccess;
		}
		return false;
	}

	bool SubmitFirstHostSelection(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		const FFMCodexLocalMatchInteractionView View = ViewFor(Host, Rules);
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
		{
			FMatchPlayAuthoritativeSubmitCarrierRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.CarrierCardId = Id;
			return Host.SubmitCarrier(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
		{
			FMatchPlayAuthoritativeSubmitMarkerRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.MarkerCardId = Id;
			return Host.SubmitMarker(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
		{
			FMatchPlayAuthoritativeSubmitSkillRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.SkillId = Id;
			return Host.SubmitSkill(Rules, Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
		{
			FMatchPlayAuthoritativeSubmitRunnerRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.RunnerCardId = Id;
			return Host.SubmitRunner(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		{
			FMatchPlayAuthoritativeSubmitHelperRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.HelperCardId = Id;
			return Host.SubmitHelper(Request).bSuccess;
		}
		default:
			return false;
		}
	}

	bool CompleteCrossAttack(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const TCHAR* Label)
	{
		AcknowledgeIfPending(Controller);
		Controller.BeginDemoOrdinaryAttack();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: BeginOrdinaryAttack failed: %s"),
				Label,
				*Controller.GetLastDiagnostic().Message));
			return false;
		}
		const EInitialTurnOrderPlayer Attacker =
			Controller.GetInteractionView().CurrentAttackingPlayer;
		const FString PhysicalForward =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("NearB")
				: TEXT("NearA");
		for (int32 Index = 0; Index < 4; ++Index)
		{
			if (!DeployNextOrdinary(Controller, PhysicalForward))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: ordinary deployment %d failed."),
					Label, Index + 1));
				return false;
			}
		}

		AcknowledgeIfPending(Controller);
		Controller.FinishDeployment();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: first FinishDeployment failed."), Label));
			return false;
		}
		AcknowledgeIfPending(Controller);
		Controller.FinishDeployment();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: second FinishDeployment failed."), Label));
			return false;
		}

		for (const EFMCodexLocalMatchInteractionCategory Category : {
			EFMCodexLocalMatchInteractionCategory::SelectCarrier,
			EFMCodexLocalMatchInteractionCategory::SelectMarker,
			EFMCodexLocalMatchInteractionCategory::SelectSkill,
			EFMCodexLocalMatchInteractionCategory::SelectRunner,
			EFMCodexLocalMatchInteractionCategory::SelectHelper })
		{
			if (!SubmitFirstSelection(Controller, Category))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: selection category %s failed (actual %s, diagnostic %s)."),
					Label,
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(Category),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Controller.GetInteractionView().InteractionCategory),
					*Controller.GetLastDiagnostic().Message));
				return false;
			}
		}

		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: Cross did not request BranchIntent."), Label));
			return false;
		}
		AcknowledgeIfPending(Controller);
		Controller.SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: BranchIntent failed."), Label));
			return false;
		}

		for (int32 Step = 0; Step < 3; ++Step)
		{
			if (Controller.IsAwaitingHotSeatHandoff())
			{
				Test.AddError(FString::Printf(
					TEXT("%s: system step %d created a false handoff."),
					Label, Step + 1));
				return false;
			}
			if (Controller.GetInteractionView().InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
			{
				Test.AddError(FString::Printf(
					TEXT("%s: system step %d was not Continue."),
					Label, Step + 1));
				return false;
			}
			Controller.ContinueResolution();
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				Test.AddError(FString::Printf(
					TEXT("%s: system step %d failed: %s"),
					Label,
					Step + 1,
					*Controller.GetLastDiagnostic().Message));
				return false;
			}
			if (Controller.IsAwaitingHotSeatHandoff())
			{
				Test.AddError(FString::Printf(
					TEXT("%s: completed system step %d created a false handoff."),
					Label, Step + 1));
				return false;
			}
		}
		if (Controller.GetInteractionView().AcceptedRolls.Num() != 3)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: Cross did not expose its route and two post-route D6 records."),
				Label));
			return false;
		}

		Controller.ContinueResolution();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: terminal application failed: %s"),
				Label,
				*Controller.GetLastDiagnostic().Message));
			return false;
		}
		return !Controller.GetInteractionView().bCurrentAttackActive
			&& Controller.IsAwaitingHotSeatHandoff();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPresentationBoundaryTest,
	"FMCodex.LocalPlay.ControlSurface.11.PresentationBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPresentationBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	const auto Empty =
		FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
	TestFalse(TEXT("NoActiveMatch is not active"), Empty.bMatchActive);
	TestEqual(TEXT("NoActiveMatch asks StartMatch"),
		Empty.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::StartMatch);

	AFMCodexLocalMatchHostGameMode* HostClassDefaults =
		GetMutableDefault<AFMCodexLocalMatchHostGameMode>();
	TestNotNull(TEXT("Host class default exists"), HostClassDefaults);
	if (HostClassDefaults != nullptr)
	{
		TestTrue(TEXT("Host owns the normal presentation class"),
			HostClassDefaults->PlayerControllerClass.Get()
				== AFMCodexLocalMatchPlayerController::StaticClass());
	}

	FString ControllerSource;
	FString ViewHeader;
	TestTrue(TEXT("Controller production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Interaction view header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.h"),
		ViewHeader));
	TestFalse(TEXT("UI has no Session owner/ref"),
		ControllerSource.Contains(TEXT("FMatchPlayAuthoritativeSession")));
	TestFalse(TEXT("UI has no provider owner/ref"),
		ControllerSource.Contains(TEXT("D6Provider")));
	TestFalse(TEXT("UI has no generic reflection dispatch"),
		ControllerSource.Contains(TEXT("ProcessEvent"))
			|| ControllerSource.Contains(TEXT("FindFunction")));
	TestTrue(TEXT("Interaction view is presentation-only value data"),
		ViewHeader.Contains(TEXT("FFMCodexLocalMatchInteractionView"))
			&& !ViewHeader.Contains(TEXT("FMatchPlayState Snapshot")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchControlSurfaceFlowTest,
	"FMCodex.LocalPlay.ControlSurface.12.CrossEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchControlSurfaceFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Real World has authoritative GameMode Host"), Host);
	TestNotNull(TEXT("Real World has local hot-seat Controller"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Initial controller view asks StartMatch"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::StartMatch);
	Controller->StartNewDemoMatch();
	TestTrue(TEXT("StartNewMatch succeeds through Controller"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Snapshot refresh asks BeginAttack"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::BeginAttack);
	TestTrue(TEXT("Initial human interaction requires hot-seat handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	Controller->AcknowledgeHotSeatHandoff();
	TestFalse(TEXT("Ready reveals initial human interaction"),
		Controller->IsAwaitingHotSeatHandoff());

	Controller->FinishDeployment();
	TestFalse(TEXT("Rejected out-of-stage command is displayed"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Failure refresh preserves authoritative category"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::BeginAttack);

	const EInitialTurnOrderPlayer FirstAttacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	TestTrue(TEXT("Representative Cross attack completes"),
		CompleteCrossAttack(*this, *Controller, TEXT("AttackOne")));
	TestTrue(TEXT("Completion exposes next-player hot-seat handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	TestTrue(TEXT("Terminal completion switches the current attacker"),
		Controller->GetInteractionView().CurrentAttackingPlayer
			!= FirstAttacker);
	TestEqual(TEXT("Rendered score A matches snapshot"),
		Controller->GetInteractionView().PlayerAScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Rendered score B matches snapshot"),
		Controller->GetInteractionView().PlayerBScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerBState.Score);
	TestTrue(TEXT("Terminal command exposes safe readable result summary"),
		Controller->GetLastDiagnostic().PresentationSummary == TEXT("GOAL")
			|| Controller->GetLastDiagnostic().PresentationSummary
				== TEXT("Attack complete - no goal"));

	FMatchPlayState EndedSnapshot = Host->GetMatchSnapshot().Snapshot;
	EndedSnapshot.RuntimeState.PlayerAState.UsedAttackCount =
		EndedSnapshot.RuntimeState.PlayerAState.TotalAttackCount;
	EndedSnapshot.RuntimeState.PlayerBState.UsedAttackCount =
		EndedSnapshot.RuntimeState.PlayerBState.TotalAttackCount;
	EndedSnapshot.bHasCurrentAttack = false;
	const auto Rules = Host->GetSkillRuleSnapshot();
	const auto EndedView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		EndedSnapshot, Rules.Snapshot);
	TestTrue(TEXT("Canonical ended snapshot derives match ended"),
		EndedView.bMatchEnded);
	TestEqual(TEXT("Canonical ended snapshot derives MatchEnded category"),
		EndedView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::MatchEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchOneOnOnePresentationTest,
	"FMCodex.LocalPlay.ControlSurface.13.OneOnOneCategory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchOneOnOnePresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	TestNotNull(TEXT("OneOnOne Host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}

	FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FName ThroughBallSkillId(TEXT("Demo.Skill.ThroughBall"));
	for (TArray<FPlayerCardData>* Deck : {
		&Demo.OpeningInput.OpeningInput.PlayerADeck,
		&Demo.OpeningInput.OpeningInput.PlayerBDeck })
	{
		for (FPlayerCardData& Card : *Deck)
		{
			if (!Card.bIsGoalkeeper)
			{
				Card.AttackSkillIds = { ThroughBallSkillId };
			}
		}
	}
	FSkillRuleSnapshot Rule;
	Rule.SkillId = ThroughBallSkillId;
	Rule.SkillType = ESkillRuleType::ThroughBall;
	Rule.MinTriggerActionPoint = 2;
	Rule.MaxTriggerActionPoint = 8;
	Demo.SkillRuleSet.SkillRules = { Rule };

	const int32 Seed = FindSeedForRolls({ 3, 6, 1, 2 });
	TestTrue(TEXT("Deterministic OneOnOne seed exists"), Seed != INDEX_NONE);
	if (Seed == INDEX_NONE
		|| !Host->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, Seed).bSuccess
		|| !Host->BeginOrdinaryAttack(6).bSuccess)
	{
		return false;
	}

	const EInitialTurnOrderPlayer Attacker =
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB")
			: TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("OneOnOne fixture deploys a legal ordinary card"),
			DeployNextOrdinary(*Host, Demo.SkillRuleSet, PhysicalForward));
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const auto View = ViewFor(*Host, Demo.SkillRuleSet);
		TestTrue(TEXT("OneOnOne fixture finishes deployment"),
			Host->FinishDeployment(
				View.AttackSequence, View.ExpectedActingPlayer).bSuccess);
	}
	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectCarrier,
		EFMCodexLocalMatchInteractionCategory::SelectMarker,
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		TestTrue(TEXT("OneOnOne fixture submits canonical selection"),
			SubmitFirstHostSelection(*Host, Demo.SkillRuleSet, Category));
	}

	TestTrue(TEXT("OneOnOne fixture begins resolution"),
		Host->BeginResolutionSession().bSuccess);
	TestTrue(TEXT("OneOnOne fixture resolves BehindDefense route"),
		Host->ResolveInitialRoute().bSuccess);
	TestEqual(TEXT("Deterministic route is BehindDefense"),
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
			.ActualBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::BehindDefense);
	TestTrue(TEXT("OneOnOne fixture resolves P1 plan"),
		Host->ResolveThroughBallBehindDefenseP1DecisionOrPlan().bSuccess);
	TestTrue(TEXT("OneOnOne fixture resolves P1 Formula"),
		Host->ResolveThroughBallBehindDefenseP1Formula().bSuccess);
	const auto P2 = Host->ResolveThroughBallBehindDefenseP2Decision();
	TestTrue(TEXT("OneOnOne fixture resolves P2"), P2.bSuccess);
	TestEqual(TEXT("P2 authority requires OneOnOne"),
		P2.AuthoritativeResult.OrchestrationResult.QueryResult.Decision,
		EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired);

	const auto View = ViewFor(*Host, Demo.SkillRuleSet);
	TestEqual(TEXT("Snapshot-derived view asks OneOnOne choice"),
		View.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("OneOnOne is visibly a human interaction"),
		View.bHumanInteraction);
	TestEqual(TEXT("OneOnOne expected side is attacker"),
		View.ExpectedActingPlayer, Attacker);
	TestEqual(TEXT("OneOnOne exposes exactly ChipShot and DirectShot"),
		View.OneOnOneOptions.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHotSeatPolicyTest,
	"FMCodex.LocalPlay.ControlSurface.14.HotSeatPolicy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHotSeatPolicyTest::RunTest(const FString& Parameters)
{
	FFMCodexLocalMatchHotSeatHandoffState State;
	FFMCodexLocalMatchInteractionView View;
	View.bMatchActive = true;
	View.bHumanInteraction = true;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::Deploy;

	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestTrue(TEXT("Initial human actor requires handoff"),
		State.bAwaitingAcknowledgement);
	TestEqual(TEXT("Initial handoff targets Player A"),
		State.PendingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Ready acknowledges the current authoritative actor"),
		FFMCodexLocalMatchHotSeatHandoffPolicy::Acknowledge(View, State));
	TestFalse(TEXT("Ready clears presentation mask"),
		State.bAwaitingAcknowledgement);

	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectCarrier;
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestFalse(TEXT("Same human actor after refresh has no handoff"),
		State.bAwaitingAcknowledgement);

	View.bHumanInteraction = false;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::ContinueResolution;
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestFalse(TEXT("System continuation has no handoff"),
		State.bAwaitingAcknowledgement);
	TestEqual(TEXT("System continuation does not replace revealed human actor"),
		State.LastRevealedHumanPlayer, EInitialTurnOrderPlayer::PlayerA);

	View.bHumanInteraction = true;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectMarker;
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestTrue(TEXT("Player A to Player B triggers handoff"),
		State.bAwaitingAcknowledgement);
	TestEqual(TEXT("Pending interaction is rebuilt from newest view"),
		State.PendingInteraction,
		EFMCodexLocalMatchInteractionCategory::SelectMarker);

	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectHelper;
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestEqual(TEXT("Visible mask tracks authoritative interaction changes"),
		State.PendingInteraction,
		EFMCodexLocalMatchInteractionCategory::SelectHelper);

	View.bMatchEnded = true;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::MatchEnded;
	FFMCodexLocalMatchHotSeatHandoffPolicy::Reconcile(View, State);
	TestFalse(TEXT("MatchEnded suppresses pending handoff"),
		State.bAwaitingAcknowledgement);
	TestEqual(TEXT("MatchEnded clears pending player"),
		State.PendingPlayer, EInitialTurnOrderPlayer::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHotSeatAuthorityAndReadabilityTest,
	"FMCodex.LocalPlay.ControlSurface.15.HotSeatAuthorityAndReadability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHotSeatAuthorityAndReadabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Hot-seat Host exists"), Host);
	TestNotNull(TEXT("Hot-seat Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	TestTrue(TEXT("Start succeeds"), Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("First human interaction is masked"),
		Controller->IsAwaitingHotSeatHandoff());
	const auto BeforeBlocked = SerializeState(Host->GetMatchSnapshot().Snapshot);
	Controller->BeginDemoOrdinaryAttack();
	TestFalse(TEXT("Gameplay command is rejected while masked"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Blocked command leaves authority byte-identical"),
		BeforeBlocked == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestTrue(TEXT("Blocked command leaves handoff pending"),
		Controller->IsAwaitingHotSeatHandoff());
	const FFMCodexLocalMatchCommandDiagnostic BlockedDiagnostic =
		Controller->GetLastDiagnostic();

	const auto BeforeReady = SerializeState(Host->GetMatchSnapshot().Snapshot);
	Controller->AcknowledgeHotSeatHandoff();
	TestFalse(TEXT("Ready reveals gameplay surface"),
		Controller->IsAwaitingHotSeatHandoff());
	TestTrue(TEXT("Ready leaves authority byte-identical"),
		BeforeReady == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Ready preserves gameplay diagnostic command"),
		Controller->GetLastDiagnostic().CommandName,
		BlockedDiagnostic.CommandName);
	TestEqual(TEXT("Ready preserves gameplay diagnostic message"),
		Controller->GetLastDiagnostic().Message,
		BlockedDiagnostic.Message);
	Controller->RefreshPresentation();
	TestFalse(TEXT("Same actor refresh does not re-mask"),
		Controller->IsAwaitingHotSeatHandoff());

	Controller->FinishDeployment();
	TestFalse(TEXT("Out-of-stage command fails authoritatively"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestFalse(TEXT("Failed command does not trigger handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	Controller->BeginDemoOrdinaryAttack();
	TestTrue(TEXT("Begin remains reachable after Ready"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestFalse(TEXT("Begin to same-player deployment has no handoff"),
		Controller->IsAwaitingHotSeatHandoff());

	const auto& DeploymentView = Controller->GetInteractionView();
	int32 GroupedSlotCount = 0;
	bool bHasOrdinaryGroup = false;
	bool bHasGoalkeeperGroup = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: DeploymentView.DeploymentGroups)
	{
		GroupedSlotCount += Group.LegalSlotIds.Num();
		bHasGoalkeeperGroup |= Group.bGoalkeeper;
		bHasOrdinaryGroup |= !Group.bGoalkeeper;
	}
	TestTrue(TEXT("Deployment has readable ordinary groups"), bHasOrdinaryGroup);
	TestEqual(TEXT("Grouping preserves every legal deployment option"),
		GroupedSlotCount, DeploymentView.DeploymentOptions.Num());
	TestTrue(TEXT("Large legal option set remains complete"),
		DeploymentView.DeploymentOptions.Num() > 10);

	const FFMCodexLocalMatchDeploymentOption FirstOption =
		DeploymentView.DeploymentOptions[0];
	Controller->DeployOrdinary(FirstOption.CardId, FirstOption.SlotId);
	TestTrue(TEXT("First deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Deployment actor change triggers handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	bHasGoalkeeperGroup = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: Controller->GetInteractionView().DeploymentGroups)
	{
		bHasGoalkeeperGroup |= Group.bGoalkeeper;
	}
	TestTrue(TEXT("Defender deployment has readable goalkeeper group"),
		bHasGoalkeeperGroup);
	const auto BeforeSecondBlocked =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FFMCodexLocalMatchDeploymentOption SecondOption =
		Controller->GetInteractionView().DeploymentOptions[0];
	Controller->DeployOrdinary(SecondOption.CardId, SecondOption.SlotId);
	TestFalse(TEXT("Second side cannot deploy before Ready"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Masked deployment leaves authority unchanged"),
		BeforeSecondBlocked == SerializeState(Host->GetMatchSnapshot().Snapshot));
	Controller->AcknowledgeHotSeatHandoff();
	Controller->DeployOrdinary(SecondOption.CardId, SecondOption.SlotId);
	TestTrue(TEXT("Second-side deployment works after Ready"),
		Controller->GetLastDiagnostic().bHostSuccess);

	FString ControllerSource;
	TestTrue(TEXT("Controller source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Full control surface remains scrollable"),
		ControllerSource.Contains(TEXT("SNew(SScrollBox)"))
			&& ControllerSource.Contains(TEXT("ScrollBarAlwaysVisible(true)")));
	TestTrue(TEXT("Deployment uses wrapped grouped slot controls"),
		ControllerSource.Contains(TEXT("DeploymentGroups"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)")));
	TestTrue(TEXT("Handoff screen replaces normal controls"),
		ControllerSource.Contains(TEXT("PASS CONTROL"))
			&& ControllerSource.Contains(TEXT("AllowGameplayCommand")));
	TestTrue(TEXT("Choice controls carry explicit presentation headings"),
		ControllerSource.Contains(TEXT("Branch / Shot Type"))
			&& ControllerSource.Contains(TEXT("One-on-One Shot Type")));
	const int32 ReadyStart = ControllerSource.Find(
		TEXT("void AFMCodexLocalMatchPlayerController::AcknowledgeHotSeatHandoff"));
	const int32 ReadyEnd = ControllerSource.Find(
		TEXT("bool AFMCodexLocalMatchPlayerController::AllowGameplayCommand"));
	const FString ReadyBody = ReadyStart != INDEX_NONE && ReadyEnd > ReadyStart
		? ControllerSource.Mid(ReadyStart, ReadyEnd - ReadyStart)
		: FString();
	TestTrue(TEXT("Ready production body is statically isolated"),
		!ReadyBody.IsEmpty()
			&& !ReadyBody.Contains(TEXT("Host->"))
			&& !ReadyBody.Contains(TEXT("AuthoritativeSession"))
			&& !ReadyBody.Contains(TEXT("D6Provider"))
			&& !ReadyBody.Contains(TEXT("RecordCommandResult")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHotSeatTwoSideFlowTest,
	"FMCodex.LocalPlay.ControlSurface.16.HotSeatTwoSideFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHotSeatTwoSideFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Controller = PlayableWorld.GetController();
	if (Controller == nullptr)
	{
		return false;
	}
	Controller->StartNewDemoMatch();
	Controller->AcknowledgeHotSeatHandoff();
	Controller->BeginDemoOrdinaryAttack();
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("Two-side flow deploys through handoffs"),
			DeployNextOrdinary(*Controller, PhysicalForward));
	}
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	TestEqual(TEXT("Attacker is prompted for Carrier"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectCarrier);
	const FName CarrierId =
		Controller->GetInteractionView().SelectionOptions[0].Id;
	TestEqual(TEXT("Carrier option shows attacker side"),
		Controller->GetInteractionView().SelectionOptions[0].Side,
		Attacker);
	Controller->SubmitCarrier(CarrierId);
	TestTrue(TEXT("Carrier selection succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Carrier to Marker actor transition is masked"),
		Controller->IsAwaitingHotSeatHandoff());
	TestEqual(TEXT("Marker is assigned to the other player"),
		Controller->GetHotSeatHandoffState().PendingPlayer,
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA);
	Controller->AcknowledgeHotSeatHandoff();
	const FName MarkerId =
		Controller->GetInteractionView().SelectionOptions[0].Id;
	TestEqual(TEXT("Marker option shows defender side"),
		Controller->GetInteractionView().SelectionOptions[0].Side,
		Controller->GetInteractionView().ExpectedActingPlayer);
	Controller->SubmitMarker(MarkerId);
	TestTrue(TEXT("Marker selection succeeds after defender Ready"),
		Controller->GetLastDiagnostic().bHostSuccess);

	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		AcknowledgeIfPending(*Controller);
		if (Category
			== EFMCodexLocalMatchInteractionCategory::SelectSkill)
		{
			TestTrue(TEXT("Skill choice includes readable family text"),
				Controller->GetInteractionView().SelectionOptions[0]
					.Label.Contains(TEXT("Cross")));
		}
		TestTrue(TEXT("Remaining selection stays reachable"),
			SubmitFirstSelection(*Controller, Category));
	}
	AcknowledgeIfPending(*Controller);
	Controller->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
	TestTrue(TEXT("BranchIntent remains reachable"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestFalse(TEXT("System resolution begins without handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	TestEqual(TEXT("Continue label identifies Begin Resolution"),
		Controller->GetInteractionView().ContinueActionLabel,
		FString(TEXT("Continue - Begin Resolution")));
	Controller->ContinueResolution();
	TestFalse(TEXT("BeginResolutionSession has no handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	TestEqual(TEXT("Continue label identifies route resolution"),
		Controller->GetInteractionView().ContinueActionLabel,
		FString(TEXT("Continue - Resolve Route")));
	Controller->ContinueResolution();
	TestFalse(TEXT("RNG route resolution has no handoff"),
		Controller->IsAwaitingHotSeatHandoff());
	TestTrue(TEXT("Authoritative route D6 remains visible"),
		Controller->GetInteractionView().AcceptedRolls.Num() > 0);
	TestEqual(TEXT("Initial roll is grouped separately"),
		Controller->GetInteractionView().AcceptedRolls[0].Group,
		EFMCodexLocalMatchRollGroup::InitialRoute);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCardPitchPresentationTest,
	"FMCodex.LocalPlay.ControlSurface.17.CardPitchPresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCardPitchPresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	Controller->AcknowledgeHotSeatHandoff();
	Controller->BeginDemoOrdinaryAttack();
	TestTrue(TEXT("Presentation fixture begins attack"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const auto& InitialView = Controller->GetInteractionView();
	TestEqual(TEXT("Pitch has both canonical physical halves"),
		InitialView.PitchRegions.Num(), 2);
	int32 PresentedSlotCount = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: InitialView.PitchRegions)
	{
		PresentedSlotCount += Region.CanonicalSlotIds.Num();
		TestFalse(TEXT("Pitch region has a player-readable label"),
			Region.Label.IsEmpty());
	}
	TestEqual(TEXT("Pitch regions preserve every canonical slot"),
		PresentedSlotCount,
		Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots.Num());

	int32 PresentedLegalSlots = 0;
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup = nullptr;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: InitialView.DeploymentGroups)
	{
		PresentedLegalSlots += Group.LegalSlots.Num();
		TestEqual(TEXT("Readable locations preserve raw legal slot count"),
			Group.LegalSlots.Num(), Group.LegalSlotIds.Num());
		if (!Group.bGoalkeeper && OrdinaryGroup == nullptr)
		{
			OrdinaryGroup = &Group;
		}
	}
	TestEqual(TEXT("Card grouping preserves all legal options"),
		PresentedLegalSlots, InitialView.DeploymentOptions.Num());
	TestNotNull(TEXT("An ordinary readable card is available"), OrdinaryGroup);
	if (OrdinaryGroup == nullptr || OrdinaryGroup->LegalSlots.IsEmpty())
	{
		return false;
	}
	TestEqual(TEXT("Card view preserves canonical CardId"),
		OrdinaryGroup->Card.CardId, OrdinaryGroup->CardId);
	TestTrue(TEXT("Missing display name uses explicit Card fallback"),
		OrdinaryGroup->Card.DisplayLabel.StartsWith(TEXT("Card ")));
	TestFalse(TEXT("Card shows canonical role"),
		OrdinaryGroup->Card.PositionLabel.IsEmpty());
	TestTrue(TEXT("Card shows compact authoritative attributes"),
		OrdinaryGroup->Card.AttributeSummary.Contains(TEXT("SHO"))
			&& OrdinaryGroup->Card.AttributeSummary.Contains(TEXT("PAS")));
	TestTrue(TEXT("Demo ordinary card shows readable Cross skill"),
		OrdinaryGroup->Card.SkillLabels.Contains(TEXT("Cross")));

	const FName DeployedCardId = OrdinaryGroup->CardId;
	const FName DeployedSlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const EMatchPlayNeutralSlotSide ExpectedHalf =
		OrdinaryGroup->LegalSlots[0].NeutralSide;
	const EMatchPlayRelativeDeploymentZone ExpectedZone =
		OrdinaryGroup->LegalSlots[0].RelativeZone;
	Controller->DeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Readable deployment command succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);

	int32 PresentedPlacements = 0;
	bool bFoundCanonicalCard = false;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		PresentedPlacements += Region.DeployedCards.Num();
		for (const FFMCodexLocalMatchCardView& Card : Region.DeployedCards)
		{
			if (Card.CardId == DeployedCardId)
			{
				bFoundCanonicalCard = Card.bDeployed
					&& Card.SlotId == DeployedSlotId
					&& Card.NeutralSide == ExpectedHalf
					&& Card.RelativeZone == ExpectedZone;
			}
		}
	}
	TestEqual(TEXT("Exactly one deployed card appears on pitch"),
		PresentedPlacements, 1);
	TestTrue(TEXT("Deployed card maps to canonical half, zone and slot"),
		bFoundCanonicalCard);

	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const int32 ScoreABefore = Controller->GetInteractionView().PlayerAScore;
	const int32 ScoreBBefore = Controller->GetInteractionView().PlayerBScore;
	const auto CategoryBefore =
		Controller->GetInteractionView().InteractionCategory;
	TestFalse(TEXT("Next side still has canonical deployment options"),
		Controller->GetInteractionView().DeploymentOptions.IsEmpty());
	if (Controller->GetInteractionView().DeploymentOptions.IsEmpty())
	{
		return false;
	}
	const FFMCodexLocalMatchDeploymentOption MaskedOption =
		Controller->GetInteractionView().DeploymentOptions[0];
	Controller->DeployOrdinary(MaskedOption.CardId, MaskedOption.SlotId);
	TestFalse(TEXT("Masked next-side deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Rejected command has readable result summary"),
		Controller->GetLastDiagnostic().PresentationSummary,
		FString(TEXT("Command rejected")));
	TestTrue(TEXT("Rejected command leaves authority byte-identical"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Rejected command leaves score A unchanged"),
		Controller->GetInteractionView().PlayerAScore, ScoreABefore);
	TestEqual(TEXT("Rejected command leaves score B unchanged"),
		Controller->GetInteractionView().PlayerBScore, ScoreBBefore);
	TestEqual(TEXT("Rejected command leaves interaction unchanged"),
		Controller->GetInteractionView().InteractionCategory, CategoryBefore);

	int32 PlacementsAfterFailure = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		PlacementsAfterFailure += Region.DeployedCards.Num();
	}
	TestEqual(TEXT("Rejected command causes no optimistic card movement"),
		PlacementsAfterFailure, PresentedPlacements);

	Controller->AcknowledgeHotSeatHandoff();
	bool bHasReadableGoalkeeper = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: Controller->GetInteractionView().DeploymentGroups)
	{
		if (Group.bGoalkeeper)
		{
			bHasReadableGoalkeeper = Group.Card.bGoalkeeper
				&& Group.Card.PositionLabel.Contains(TEXT("Goalkeeper"))
				&& Group.Card.GoalkeeperAttributeSummary.Contains(TEXT("HAN"))
				&& !Group.Card.bGoalkeeperUsedThisMatch;
			break;
		}
	}
	TestTrue(TEXT("Defender goalkeeper is visibly distinct and authoritative"),
		bHasReadableGoalkeeper);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCandidatePresentationTest,
	"FMCodex.LocalPlay.ControlSurface.18.CandidatePresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCandidatePresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	Controller->AcknowledgeHotSeatHandoff();
	Controller->BeginDemoOrdinaryAttack();
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("Candidate fixture deploys through readable options"),
			DeployNextOrdinary(*Controller, PhysicalForward));
	}
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);

	const auto& CarrierView = Controller->GetInteractionView();
	TestEqual(TEXT("Fixture reaches Carrier"), CarrierView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectCarrier);
	TestFalse(TEXT("Carrier candidates exist"),
		CarrierView.SelectionOptions.IsEmpty());
	if (CarrierView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const FFMCodexLocalMatchSelectionOption Carrier =
		CarrierView.SelectionOptions[0];
	TestTrue(TEXT("Carrier candidate includes its card view"),
		Carrier.bHasCard);
	TestEqual(TEXT("Carrier button and card share canonical identity"),
		Carrier.RelatedCardId, Carrier.Id);
	TestEqual(TEXT("Carrier card keeps expected acting side"),
		Carrier.Card.Side, CarrierView.ExpectedActingPlayer);
	TestTrue(TEXT("Carrier card is visibly deployed"),
		Carrier.Card.bDeployed);
	TestEqual(TEXT("Header attacker derives from authority"),
		CarrierView.CurrentAttackingPlayer,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer);
	TestEqual(TEXT("Header score A derives from authority"),
		CarrierView.PlayerAScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Header score B derives from authority"),
		CarrierView.PlayerBScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerBState.Score);

	Controller->SubmitCarrier(Carrier.Id);
	AcknowledgeIfPending(*Controller);
	const auto& MarkerView = Controller->GetInteractionView();
	TestFalse(TEXT("Marker candidates exist"),
		MarkerView.SelectionOptions.IsEmpty());
	if (MarkerView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const auto Marker = MarkerView.SelectionOptions[0];
	TestTrue(TEXT("Marker candidate includes matching deployed card"),
		Marker.bHasCard && Marker.RelatedCardId == Marker.Id
			&& Marker.Card.CardId == Marker.Id && Marker.Card.bDeployed);
	Controller->SubmitMarker(Marker.Id);
	AcknowledgeIfPending(*Controller);

	const auto& SkillView = Controller->GetInteractionView();
	TestEqual(TEXT("Fixture reaches Skill"), SkillView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectSkill);
	TestFalse(TEXT("Skill choices exist"), SkillView.SelectionOptions.IsEmpty());
	if (SkillView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const auto Skill = SkillView.SelectionOptions[0];
	TestTrue(TEXT("Skill label maps canonical enum to readable text"),
		Skill.Label.Contains(TEXT("Cross")));
	TestTrue(TEXT("Skill choice shows the carrier using it"),
		Skill.bHasCard && Skill.RelatedCardId == Carrier.Id
			&& Skill.Card.CardId == Carrier.Id);
	TestEqual(TEXT("Expected actor remains snapshot-derived"),
		SkillView.ExpectedActingPlayer, Attacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPresentationShellContractTest,
	"FMCodex.LocalPlay.ControlSurface.19.PresentationShellContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPresentationShellContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	if (Host == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TestTrue(TEXT("Ended-view fixture starts"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet).bSuccess);
	FMatchPlayState EndedState = Host->GetMatchSnapshot().Snapshot;
	EndedState.RuntimeState.PlayerAState.Score = 2;
	EndedState.RuntimeState.PlayerBState.Score = 1;
	EndedState.RuntimeState.PlayerAState.UsedAttackCount =
		EndedState.RuntimeState.PlayerAState.TotalAttackCount;
	EndedState.RuntimeState.PlayerBState.UsedAttackCount =
		EndedState.RuntimeState.PlayerBState.TotalAttackCount;
	EndedState.bHasCurrentAttack = false;
	const auto EndedView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		EndedState, Demo.SkillRuleSet);
	TestTrue(TEXT("Ended shell is snapshot-derived"), EndedView.bMatchEnded);
	TestEqual(TEXT("Ended shell retains final score A"),
		EndedView.PlayerAScore, 2);
	TestEqual(TEXT("Ended shell retains final score B"),
		EndedView.PlayerBScore, 1);
	TestEqual(TEXT("Ended shell exposes canonical winner"),
		EndedView.MatchResult, EMatchResultType::HomeWin);
	TestEqual(TEXT("Ended shell exposes no human actor"),
		EndedView.ExpectedActingPlayer, EInitialTurnOrderPlayer::None);

	TestEqual(TEXT("LongShot readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::LongShot), FString(TEXT("Long Shot")));
	TestEqual(TEXT("CutInside readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::CutInsideShot), FString(TEXT("Cut Inside")));
	TestEqual(TEXT("PassControl readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::PassControl), FString(TEXT("Pass Control")));
	TestEqual(TEXT("ThroughBall readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::ThroughBall), FString(TEXT("Through Ball")));

	FString ControllerSource;
	TestTrue(TEXT("Presentation source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Player-readable header and field shell exist"),
		ControllerSource.Contains(TEXT("FMCODEX LOCAL MATCH"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION")));
	TestTrue(TEXT("Cards and legal locations use derived view types"),
		ControllerSource.Contains(TEXT("MakeCardPanel"))
			&& ControllerSource.Contains(TEXT("Group.LegalSlots")));
	TestTrue(TEXT("Dice groups include canonical OneOnOne presentation"),
		ControllerSource.Contains(TEXT("Accepted Dice - One-on-One")));
	TestTrue(TEXT("Match-ended shell offers explicit final result/restart"),
		ControllerSource.Contains(TEXT("FINAL RESULT"))
			&& ControllerSource.Contains(TEXT("Start New Local Match")));
	TestTrue(TEXT("Interaction remains scrollable for large option sets"),
		ControllerSource.Contains(TEXT("SNew(SScrollBox)"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)")));
	TestFalse(TEXT("Presentation source has no direct authoritative State write"),
		ControllerSource.Contains(TEXT("CurrentAttack.DeploymentPlacements.Add"))
			|| ControllerSource.Contains(TEXT("RuntimeState.PlayerAState.Score ="))
			|| ControllerSource.Contains(TEXT("RuntimeState.PlayerBState.Score =")));
	return true;
}

#endif
