#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexDiceResultWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexResolutionPanelWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/UniformGridSlot.h"

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
			const auto& Feedback = Controller.GetResolutionFeedback();
			if (Step == 1
				&& (!Feedback.StepTitle.Contains(TEXT("Route"))
					|| !Feedback.RouteSummary.Contains(TEXT("Cross"))
					|| Feedback.DiceEntries.IsEmpty()))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: route feedback did not expose authoritative Cross branch/D6 evidence."),
					Label));
				return false;
			}
			if (Step == 2
				&& (!Feedback.DecisionSummary.Contains(TEXT("Formula"))
					|| Feedback.DiceEntries.Num() != 3))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: Cross plan feedback did not retain its three authoritative D6 records."),
					Label));
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
		const auto& TerminalFeedback = Controller.GetResolutionFeedback();
		if (!TerminalFeedback.bTerminal
			|| !TerminalFeedback.TerminalSummary.StartsWith(TEXT("RESULT: "))
			|| TerminalFeedback.ComparisonEntries.Num() < 2
			|| !TerminalFeedback.DecisionSummary.Contains(TEXT("Winner:"))
			|| !TerminalFeedback.ContinuationSummary.Contains(TEXT("Next attacker:")))
		{
			Test.AddError(FString::Printf(
				TEXT("%s: terminal feedback did not expose authoritative Formula/Completion evidence."),
				Label));
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
	const auto BeforeP2View = ViewFor(*Host, Demo.SkillRuleSet);
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
	const auto P2Feedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallBehindDefenseP2Decision"),
			P2,
			BeforeP2View,
			View);
	TestTrue(TEXT("P2 feedback exposes authoritative continuation"),
		P2Feedback.StepTitle.Contains(TEXT("P2"))
			&& P2Feedback.DecisionSummary.Contains(TEXT("One-on-One")));
	TestTrue(TEXT("P2 feedback exposes regenerated P1 Formula evidence"),
		P2Feedback.ComparisonEntries.Num() >= 2);

	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Choice;
	Choice.RequestingSide = Attacker;
	Choice.Choice = EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
	TestTrue(TEXT("DirectShot choice succeeds"),
		Host->SubmitThroughBallOneOnOneShotChoice(Choice).bSuccess);
	const auto BeforePlanView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto DirectPlan =
		Host->ResolveThroughBallOneOnOneDirectShotPostRoutePlan();
	TestTrue(TEXT("DirectShot authoritative plan succeeds"),
		DirectPlan.bSuccess);
	const auto AfterPlanView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto PlanFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"),
			DirectPlan,
			BeforePlanView,
			AfterPlanView);
	TestTrue(TEXT("DirectShot plan shows shooter and goalkeeper evidence"),
		PlanFeedback.ComparisonEntries.Num() == 2
			&& PlanFeedback.ComparisonEntries[0].Contains(TEXT("Shooting"))
			&& PlanFeedback.ComparisonEntries[1].Contains(TEXT("OneOnOne"))
			&& PlanFeedback.ComparisonEntries[1].Contains(TEXT("activation")));
	TestTrue(TEXT("DirectShot plan exposes both OneOnOne D6 records"),
		PlanFeedback.DiceEntries.Num() >= 2);

	const auto DirectFormula =
		Host->ResolveThroughBallOneOnOneDirectShotFormula();
	TestTrue(TEXT("DirectShot authoritative Formula succeeds"),
		DirectFormula.bSuccess);
	const auto FormulaFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneDirectShotFormula"),
			DirectFormula,
			AfterPlanView,
			AfterPlanView);
	TestTrue(TEXT("DirectShot Formula feedback exposes final values"),
		FormulaFeedback.ComparisonEntries.Num() == 2
			&& FormulaFeedback.ComparisonEntries[0].Contains(TEXT("final"))
			&& FormulaFeedback.ComparisonEntries[1].Contains(
				TEXT("authoritative modifier")));
	TestTrue(TEXT("DirectShot Formula feedback exposes winner/reason"),
		FormulaFeedback.DecisionSummary.Contains(TEXT("Winner:"))
			&& FormulaFeedback.DecisionSummary.Contains(TEXT("Reason:")));

	const auto BeforeTerminalView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto Terminal = Host->ApplyThroughBallTerminalResolution();
	TestTrue(TEXT("DirectShot terminal application succeeds"),
		Terminal.bSuccess);
	const auto AfterTerminalView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto TerminalFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ApplyThroughBallTerminalResolution"),
			Terminal,
			BeforeTerminalView,
			AfterTerminalView);
	TestTrue(TEXT("DirectShot terminal shows Goal or Miss"),
		TerminalFeedback.TerminalSummary == TEXT("RESULT: GOAL")
			|| TerminalFeedback.TerminalSummary == TEXT("RESULT: MISS"));
	TestTrue(TEXT("DirectShot terminal shows score and next attacker"),
		TerminalFeedback.ContinuationSummary.Contains(TEXT("Score:"))
			&& TerminalFeedback.ContinuationSummary.Contains(
				TEXT("Next attacker:")));
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
		PresentedSlotCount += Region.Slots.Num();
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
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			if (!Slot.bOccupied)
			{
				continue;
			}
			++PresentedPlacements;
			const FFMCodexLocalMatchCardView& Card = Slot.Card;
			if (Card.CardId == DeployedCardId)
			{
				bFoundCanonicalCard = Card.bDeployed
					&& Slot.SlotId == DeployedSlotId
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
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			PlacementsAfterFailure += Slot.bOccupied ? 1 : 0;
		}
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchFeedbackSemanticShapesTest,
	"FMCodex.LocalPlay.ControlSurface.20.FeedbackSemanticShapes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchFeedbackSemanticShapesTest::RunTest(
	const FString& Parameters)
{
	FFMCodexLocalMatchInteractionView View;
	View.ActionLabel = TEXT("Through Ball");
	View.ActualBranchLabel = TEXT("Anti-Offside");

	FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult AntiOffside;
	AntiOffside.AuthoritativeResult.OrchestrationResult.OutcomeResult.Decision =
		EThroughBallAntiOffsideOutcomeDecision::Offside;
	const auto AntiOffsideFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallAntiOffsideDecision"),
			AntiOffside, View, View);
	TestTrue(TEXT("AntiOffside uses authoritative decision label"),
		AntiOffsideFeedback.DecisionSummary.Contains(TEXT("Offside")));
	TestTrue(TEXT("AntiOffside retains authoritative route context"),
		AntiOffsideFeedback.RouteSummary.Contains(TEXT("Anti-Offside")));

	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult Chip;
	Chip.AuthoritativeResult.OrchestrationResult.QueryResult.Decision =
		EThroughBallOneOnOneChipShotOutcomeDecision::Goal;
	const auto ChipFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneChipShotDecision"),
			Chip, View, View);
	TestEqual(TEXT("ChipShot exposes authoritative Goal"),
		ChipFeedback.DecisionSummary, FString(TEXT("Goal")));
	TestTrue(TEXT("ChipShot never fabricates goalkeeper Formula evidence"),
		ChipFeedback.ComparisonEntries.IsEmpty());
	TestTrue(TEXT("ChipShot explicitly identifies non-Formula shape"),
		ChipFeedback.StepSummary.Contains(TEXT("without Formula")));

	FFMCodexLocalMatchApplyShotTerminalResolutionResult ImmediateMiss;
	auto& ShotTerminal =
		ImmediateMiss.AuthoritativeResult.OrchestrationResult;
	ShotTerminal.TerminalSource =
		EMatchPlayShotTerminalSource::LongShotDirectShotImmediateMiss;
	ShotTerminal.CompletionResult.bSuccess = true;
	ShotTerminal.CompletionResult.AfterState.RuntimeState.PlayerAState.Score = 0;
	ShotTerminal.CompletionResult.AfterState.RuntimeState.PlayerBState.Score = 0;
	ShotTerminal.CompletionResult.NextAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;
	ShotTerminal.CompletionResult.OpportunityResolveResult.bSuccess = true;
	const auto ImmediateMissFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ApplyShotTerminalResolution"),
			ImmediateMiss, View, FFMCodexLocalMatchInteractionView());
	TestEqual(TEXT("ImmediateMiss terminal semantic is direct"),
		ImmediateMissFeedback.TerminalSummary,
		FString(TEXT("RESULT: IMMEDIATE MISS")));
	TestTrue(TEXT("Non-Formula terminal has no fake comparison"),
		ImmediateMissFeedback.ComparisonEntries.IsEmpty());
	TestTrue(TEXT("Completion feedback exposes score/opportunity/next actor"),
		ImmediateMissFeedback.ContinuationSummary.Contains(TEXT("Score:"))
			&& ImmediateMissFeedback.ContinuationSummary.Contains(
				TEXT("Opportunity consumed: yes"))
			&& ImmediateMissFeedback.ContinuationSummary.Contains(
				TEXT("Next attacker: Player B")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchRejectedFeedbackAuthorityTest,
	"FMCodex.LocalPlay.ControlSurface.21.RejectedFeedbackAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchRejectedFeedbackAuthorityTest::RunTest(
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
	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const auto CategoryBefore =
		Controller->GetInteractionView().InteractionCategory;
	Controller->BeginDemoOrdinaryAttack();
	const auto& Rejected = Controller->GetResolutionFeedback();
	TestTrue(TEXT("Rejected feedback is explicit"),
		Rejected.bVisible && Rejected.bRejected
			&& Rejected.StepTitle == TEXT("Command Rejected"));
	TestEqual(TEXT("Rejected feedback preserves typed command"),
		Rejected.CommandName, FString(TEXT("BeginOrdinaryAttack")));
	TestTrue(TEXT("Rejected feedback preserves diagnostic reason"),
		!Rejected.ErrorMessage.IsEmpty()
			&& Rejected.StepSummary.Contains(TEXT("blocked")));
	TestTrue(TEXT("Rejected feedback has no false accepted evidence"),
		Rejected.DiceEntries.IsEmpty()
			&& Rejected.ComparisonEntries.IsEmpty()
			&& Rejected.TerminalSummary.IsEmpty());
	TestTrue(TEXT("Rejected feedback leaves State byte-identical"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Feedback does not drive InteractionCategory"),
		Controller->GetInteractionView().InteractionCategory, CategoryBefore);

	const FString FeedbackBeforeReady = Rejected.ErrorMessage;
	Controller->AcknowledgeHotSeatHandoff();
	TestTrue(TEXT("Ready remains authority-neutral with feedback present"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Ready retains the public previous feedback"),
		Controller->GetResolutionFeedback().ErrorMessage,
		FeedbackBeforeReady);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchFeedbackAuthorityBoundaryTest,
	"FMCodex.LocalPlay.ControlSurface.22.FeedbackAuthorityBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchFeedbackAuthorityBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FString FeedbackSource;
	FString ControllerSource;
	TestTrue(TEXT("Feedback production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchResolutionFeedback.cpp"),
		FeedbackSource));
	TestTrue(TEXT("Controller production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	const FString PresentationSource = FeedbackSource + ControllerSource;
	TestFalse(TEXT("Presentation never calls FormulaResolver"),
		PresentationSource.Contains(TEXT("ResolveFormula(")));
	TestFalse(TEXT("Presentation never calculates goalkeeper half"),
		PresentationSource.Contains(TEXT("CalculateGoalkeeperHalf(")));
	TestFalse(TEXT("Presentation contains no route threshold table"),
		PresentationSource.Contains(TEXT("Roll <="))
			|| PresentationSource.Contains(TEXT("RawD6 <="))
			|| PresentationSource.Contains(TEXT("Roll >=")));
	TestFalse(TEXT("Feedback contains no RNG or authoritative history cache"),
		FeedbackSource.Contains(TEXT("RandRange"))
			|| FeedbackSource.Contains(TEXT("LastGameplayD6"))
			|| FeedbackSource.Contains(TEXT("CurrentAttackRolls"))
			|| FeedbackSource.Contains(TEXT("FormulaHistoryAuthority")));
	TestTrue(TEXT("Feedback panel is visibly separate"),
		ControllerSource.Contains(TEXT("MakeFeedbackPanel"))
			&& ControllerSource.Contains(TEXT("RESOLUTION"))
			&& ControllerSource.Contains(TEXT("COMMAND REJECTED")));

	const int32 ContinueStart = ControllerSource.Find(
		TEXT("void AFMCodexLocalMatchPlayerController::ContinueResolution"));
	const int32 ContinueEnd = ControllerSource.Find(
		TEXT("void AFMCodexLocalMatchPlayerController::RebuildControlSurface"));
	const FString ContinueBody = ContinueStart != INDEX_NONE
		&& ContinueEnd > ContinueStart
			? ControllerSource.Mid(
				ContinueStart, ContinueEnd - ContinueStart)
			: FString();
	TestTrue(TEXT("Feedback never selects the next gameplay command"),
		!ContinueBody.IsEmpty()
			&& !ContinueBody.Contains(TEXT("ResolutionFeedback")));
	TestTrue(TEXT("Stage 5.7 card/pitch shell remains present"),
		ControllerSource.Contains(TEXT("FMCODEX LOCAL MATCH"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("MakeCardPanel")));
	TestTrue(TEXT("Handoff remains a blocking early return"),
		ControllerSource.Contains(TEXT("PASS CONTROL"))
			&& ControllerSource.Contains(TEXT("AllowGameplayCommand")));
	return true;
}

namespace FMCodexLocalMatchFullFamilyTests
{
	struct FFamilyExpectation
	{
		ESkillRuleType SkillType = ESkillRuleType::None;
		const TCHAR* SkillId = TEXT("");
		const TCHAR* ReadableLabel = TEXT("");
		int32 FirstCardIndex = 0;
		int32 CardsPerSide = 0;
	};

	TArray<FFamilyExpectation> FamilyExpectations()
	{
		return {
			{ ESkillRuleType::Cross, TEXT("Demo.Skill.Cross"),
				TEXT("Cross"), 1, 4 },
			{ ESkillRuleType::LongShot, TEXT("Demo.Skill.LongShot"),
				TEXT("Long Shot"), 2, 4 },
			{ ESkillRuleType::CutInsideShot,
				TEXT("Demo.Skill.CutInsideShot"), TEXT("Cut Inside"), 3, 4 },
			{ ESkillRuleType::PassControl, TEXT("Demo.Skill.PassControl"),
				TEXT("Pass Control"), 4, 4 },
			{ ESkillRuleType::ThroughBall, TEXT("Demo.Skill.ThroughBall"),
				TEXT("Through Ball"), 5, 3 }
		};
	}

	FName OutfieldCardId(
		const EInitialTurnOrderPlayer Side,
		const int32 Index)
	{
		return FName(*FString::Printf(
			TEXT("Demo.%s.Outfield.%02d"),
			Side == EInitialTurnOrderPlayer::PlayerA ? TEXT("A") : TEXT("B"),
			Index));
	}

	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	bool Fail(
		FAutomationTestBase& Test,
		const FString& Family,
		const FString& Message)
	{
		Test.AddError(Family + TEXT(": ") + Message);
		return false;
	}

	bool DeployParticipants(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFamilyExpectation& Family,
		const EInitialTurnOrderPlayer Attacker)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		const FName CarrierCardId = OutfieldCardId(
			Attacker, Family.FirstCardIndex);
		const FString PhysicalForward =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("NearB") : TEXT("NearA");
		bool bCarrierDeployed = false;
		bool bDefenderGoalkeeperDeployed = false;

		for (int32 Step = 0; Step < 5; ++Step)
		{
			AcknowledgeIfPending(Controller);
			const auto& View = Controller.GetInteractionView();
			if (View.InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::Deploy)
			{
				return Fail(Test, FamilyLabel,
					TEXT("deployment interaction was not available"));
			}

			FFMCodexLocalMatchDeploymentOption SelectedOption;
			bool bFoundOption = false;
			if (View.CurrentLegalDeploymentSide == Defender
				&& !bDefenderGoalkeeperDeployed)
			{
				for (const auto& Option : View.DeploymentOptions)
				{
					if (Option.bGoalkeeper)
					{
						SelectedOption = Option;
						bFoundOption = true;
						break;
					}
				}
			}

			if (!bFoundOption)
			{
				for (const auto& Option : View.DeploymentOptions)
				{
					if (Option.bGoalkeeper
						|| !Option.SlotId.ToString().Contains(PhysicalForward))
					{
						continue;
					}
					if (View.CurrentLegalDeploymentSide == Attacker
						&& !bCarrierDeployed
						&& Option.CardId != CarrierCardId)
					{
						continue;
					}
					SelectedOption = Option;
					bFoundOption = true;
					break;
				}
			}

			if (!bFoundOption)
			{
				return Fail(Test, FamilyLabel,
					TEXT("no required normal-demo deployment option was available"));
			}

			if (SelectedOption.bGoalkeeper)
			{
				Controller.DeployGoalkeeper(SelectedOption.SlotId);
				bDefenderGoalkeeperDeployed = true;
			}
			else
			{
				Controller.DeployOrdinary(
					SelectedOption.CardId, SelectedOption.SlotId);
				bCarrierDeployed = bCarrierDeployed
					|| SelectedOption.CardId == CarrierCardId;
			}

			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("normal-demo deployment command was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}

		if (!bCarrierDeployed || !bDefenderGoalkeeperDeployed)
		{
			return Fail(Test, FamilyLabel,
				TEXT("carrier or defending goalkeeper was not deployed"));
		}

		for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
		{
			AcknowledgeIfPending(Controller);
			Controller.FinishDeployment();
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("Finish Deployment was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}
		return true;
	}

	bool SubmitRequiredSelections(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFamilyExpectation& Family,
		const EInitialTurnOrderPlayer Attacker)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		const FName CarrierCardId = OutfieldCardId(
			Attacker, Family.FirstCardIndex);
		const FName SkillId(Family.SkillId);

		AcknowledgeIfPending(Controller);
		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectCarrier)
		{
			return Fail(Test, FamilyLabel, TEXT("Carrier was not requested"));
		}
		bool bCarrierVisible = false;
		for (const auto& Option
			: Controller.GetInteractionView().SelectionOptions)
		{
			bCarrierVisible = bCarrierVisible || Option.Id == CarrierCardId;
		}
		if (!bCarrierVisible)
		{
			return Fail(Test, FamilyLabel,
				TEXT("the family carrier was not an authoritative legal option"));
		}
		Controller.SubmitCarrier(CarrierCardId);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Carrier submission failed"));
		}

		AcknowledgeIfPending(Controller);
		if (!SubmitFirstSelection(
			Controller, EFMCodexLocalMatchInteractionCategory::SelectMarker))
		{
			return Fail(Test, FamilyLabel, TEXT("Marker submission failed"));
		}

		AcknowledgeIfPending(Controller);
		const auto& SkillView = Controller.GetInteractionView();
		if (SkillView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectSkill)
		{
			return Fail(Test, FamilyLabel, TEXT("Skill was not requested"));
		}
		bool bSkillVisible = false;
		bool bReadableLabelVisible = false;
		for (const auto& Option : SkillView.SelectionOptions)
		{
			if (Option.Id == SkillId)
			{
				bSkillVisible = true;
				bReadableLabelVisible = Option.Label.Contains(Family.ReadableLabel);
			}
		}
		if (!bSkillVisible || !bReadableLabelVisible)
		{
			return Fail(Test, FamilyLabel,
				TEXT("SkillId or readable family label was not presented"));
		}
		Controller.SubmitSkill(SkillId);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Skill submission failed"));
		}

		for (int32 Guard = 0; Guard < 4; ++Guard)
		{
			AcknowledgeIfPending(Controller);
			const auto& View = Controller.GetInteractionView();
			if (View.InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::SelectRunner
				&& View.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectHelper)
			{
				break;
			}
			if (!View.SelectionOptions.IsEmpty())
			{
				if (!SubmitFirstSelection(Controller, View.InteractionCategory))
				{
					return Fail(Test, FamilyLabel,
						TEXT("Runner/Helper submission failed"));
				}
			}
			else if (View.bCanResolveNoLegalChoice)
			{
				Controller.ResolveNoLegalCurrentSelection();
			}
			else if (View.bCanDecline)
			{
				Controller.DeclineCurrentSelection();
			}
			else
			{
				return Fail(Test, FamilyLabel,
					TEXT("Runner/Helper stage had no actionable operation"));
			}
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("Runner/Helper operation was rejected"));
			}
		}

		AcknowledgeIfPending(Controller);
		const auto& ChoiceView = Controller.GetInteractionView();
		if (Family.SkillType == ESkillRuleType::LongShot
			|| Family.SkillType == ESkillRuleType::CutInsideShot)
		{
			if (ChoiceView.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::DirectShot)
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::DeadCorner))
			{
				return Fail(Test, FamilyLabel,
					TEXT("DirectShot/DeadCorner choices were incomplete"));
			}
			Controller.SubmitBranchIntent(
				EMatchPlayElectiveBranchIntent::DirectShot);
		}
		else if (Family.SkillType == ESkillRuleType::Cross)
		{
			if (ChoiceView.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::CrossHigh)
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::CrossLow))
			{
				return Fail(Test, FamilyLabel,
					TEXT("Cross High/Low choices were incomplete"));
			}
			Controller.SubmitBranchIntent(
				EMatchPlayElectiveBranchIntent::CrossHigh);
		}

		if ((Family.SkillType == ESkillRuleType::LongShot
				|| Family.SkillType == ESkillRuleType::CutInsideShot
				|| Family.SkillType == ESkillRuleType::Cross)
			&& !Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Branch choice was rejected"));
		}

		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			return Fail(Test, FamilyLabel,
				TEXT("normal selections did not reach Begin Resolution"));
		}
		return true;
	}

	bool CompleteNormalDemoFamilyAttack(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchHostGameMode& Host,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFMCodexLocalMatchDemoConfiguration& Demo,
		const FFamilyExpectation& Family,
		const int32 Seed)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		if (!Host.StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, Seed).bSuccess)
		{
			return Fail(Test, FamilyLabel,
				TEXT("normal production demo configuration did not start"));
		}
		Controller.RefreshPresentation();
		AcknowledgeIfPending(Controller);
		Controller.BeginDemoOrdinaryAttack();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("attack did not begin"));
		}

		const EInitialTurnOrderPlayer Attacker =
			Controller.GetInteractionView().CurrentAttackingPlayer;
		if (!DeployParticipants(Test, Controller, Family, Attacker)
			|| !SubmitRequiredSelections(Test, Controller, Family, Attacker))
		{
			return false;
		}

		bool bSawResolvedRoute = false;
		bool bSawOneOnOne = false;
		for (int32 Guard = 0;
			Guard < 12 && Controller.GetInteractionView().bCurrentAttackActive;
			++Guard)
		{
			const auto& View = Controller.GetInteractionView();
			bSawResolvedRoute = bSawResolvedRoute
				|| !View.ActualBranchLabel.IsEmpty();
			if (View.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
			{
				if (Family.SkillType != ESkillRuleType::ThroughBall
					|| !View.OneOnOneOptions.Contains(
						EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
					|| !View.OneOnOneOptions.Contains(
						EMatchPlayThroughBallOneOnOneShotChoice::DirectShot))
				{
					return Fail(Test, FamilyLabel,
						TEXT("OneOnOne choices were missing or unexpected"));
				}
				bSawOneOnOne = true;
				AcknowledgeIfPending(Controller);
				Controller.SubmitOneOnOneShotChoice(
					EMatchPlayThroughBallOneOnOneShotChoice::DirectShot);
			}
			else if (View.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
			{
				Controller.ContinueResolution();
			}
			else
			{
				return Fail(Test, FamilyLabel,
					TEXT("resolution reached an unexpected interaction category"));
			}

			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("typed Host resolution command was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}

		if (Controller.GetInteractionView().bCurrentAttackActive)
		{
			return Fail(Test, FamilyLabel,
				TEXT("attack did not reach terminal Completion within the guard"));
		}
		const auto& Feedback = Controller.GetResolutionFeedback();
		if (!bSawResolvedRoute
			|| !Feedback.bTerminal
			|| !Feedback.TerminalSummary.StartsWith(TEXT("RESULT: "))
			|| !Feedback.RouteSummary.Contains(Family.ReadableLabel))
		{
			return Fail(Test, FamilyLabel,
				TEXT("route/feedback/terminal evidence was incomplete"));
		}
		if (Family.SkillType == ESkillRuleType::ThroughBall)
		{
			bool bHasActiveGoalkeeperEvidence = false;
			for (const FString& Entry : Feedback.ComparisonEntries)
			{
				bHasActiveGoalkeeperEvidence = bHasActiveGoalkeeperEvidence
					|| (Entry.Contains(TEXT("Goalkeeper"))
						&& Entry.Contains(TEXT("activation active")));
			}
			if (!bSawOneOnOne || !bHasActiveGoalkeeperEvidence)
			{
				return Fail(Test, FamilyLabel,
					TEXT("normal-demo OneOnOne or active GK Formula evidence was absent"));
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchNormalDemoFamilyInventoryTest,
	"FMCodex.LocalPlay.ControlSurface.23.NormalDemoFamilyInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchNormalDemoFamilyInventoryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchFullFamilyTests;
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const TArray<FFamilyExpectation> Expected = FamilyExpectations();
	TestEqual(TEXT("Normal demo contains exactly five Skill rules"),
		Demo.SkillRuleSet.SkillRules.Num(), 5);

	TSet<FName> RuleIds;
	TSet<uint8> RuleTypes;
	for (const FSkillRuleSnapshot& Rule : Demo.SkillRuleSet.SkillRules)
	{
		RuleIds.Add(Rule.SkillId);
		RuleTypes.Add(static_cast<uint8>(Rule.SkillType));
		TestEqual(TEXT("Demo family minimum AP remains two"),
			Rule.MinTriggerActionPoint, 2);
		TestEqual(TEXT("Demo family maximum AP remains eight"),
			Rule.MaxTriggerActionPoint, 8);
	}
	TestEqual(TEXT("Five demo SkillIds are unique"), RuleIds.Num(), 5);
	TestEqual(TEXT("Five demo Skill types are unique"), RuleTypes.Num(), 5);

	for (const FFamilyExpectation& Family : Expected)
	{
		const FName ExpectedSkillId(Family.SkillId);
		const FSkillRuleSnapshot* MatchingRule =
			Demo.SkillRuleSet.SkillRules.FindByPredicate(
				[&](const FSkillRuleSnapshot& Rule)
				{
					return Rule.SkillId == ExpectedSkillId;
				});
		TestNotNull(FString::Printf(TEXT("%s rule exists"), Family.ReadableLabel),
			MatchingRule);
		if (MatchingRule != nullptr)
		{
			TestEqual(FString::Printf(TEXT("%s rule maps unambiguously"),
				Family.ReadableLabel), MatchingRule->SkillType, Family.SkillType);
		}
		TestEqual(FString::Printf(TEXT("%s readable UI mapping"),
			Family.ReadableLabel),
			FFMCodexLocalMatchInteractionViewBuilder::ToString(Family.SkillType),
			FString(Family.ReadableLabel));
	}

	TSet<FName> AllCardIds;
	for (const TArray<FPlayerCardData>* Deck : {
		&Demo.OpeningInput.OpeningInput.PlayerADeck,
		&Demo.OpeningInput.OpeningInput.PlayerBDeck })
	{
		TestEqual(TEXT("Each normal demo deck still has twenty cards"),
			Deck->Num(), 20);
		int32 GoalkeeperCount = 0;
		TMap<FName, int32> FamilyCounts;
		for (const FPlayerCardData& Card : *Deck)
		{
			AllCardIds.Add(Card.CardId);
			if (Card.bIsGoalkeeper)
			{
				++GoalkeeperCount;
				continue;
			}
			TestEqual(TEXT("Each ordinary demo card has one readable family"),
				Card.AttackSkillIds.Num(), 1);
			if (Card.AttackSkillIds.Num() == 1)
			{
				FamilyCounts.FindOrAdd(Card.AttackSkillIds[0])++;
			}
		}
		TestEqual(TEXT("Existing goalkeeper composition is preserved"),
			GoalkeeperCount, 1);
		for (const FFamilyExpectation& Family : Expected)
		{
			TestEqual(FString::Printf(TEXT("%s mirrored card distribution"),
				Family.ReadableLabel),
				FamilyCounts.FindRef(FName(Family.SkillId)), Family.CardsPerSide);
		}
	}
	TestEqual(TEXT("All forty demo CardIds remain unique"),
		AllCardIds.Num(), 40);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchNormalDemoFullFamilyReachabilityTest,
	"FMCodex.LocalPlay.ControlSurface.24.NormalDemoFullFamilyReachability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchNormalDemoFullFamilyReachabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Normal-demo full-family Host exists"), Host);
	TestNotNull(TEXT("Normal-demo full-family Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const int32 ThroughBallSeed = FindSeedForRolls({ 3, 6, 1, 2 });
	TestTrue(TEXT("Deterministic normal-demo OneOnOne seed exists"),
		ThroughBallSeed != INDEX_NONE);
	if (ThroughBallSeed == INDEX_NONE)
	{
		return false;
	}

	for (const FFamilyExpectation& Family : FamilyExpectations())
	{
		const int32 Seed = Family.SkillType == ESkillRuleType::ThroughBall
			? ThroughBallSeed
			: 1000 + static_cast<int32>(Family.SkillType);
		TestTrue(FString::Printf(
			TEXT("Normal production demo reaches %s selection/resolution/terminal"),
			Family.ReadableLabel),
			CompleteNormalDemoFamilyAttack(
				*this, *Host, *Controller, Demo, Family, Seed));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPlayerFacingScreenFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.25.PlayerFacingScreenFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPlayerFacingScreenFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FFMCodexLocalMatchInteractionView View;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	auto Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("No-match status is player-readable"),
		Screen.MatchStatusLabel, FString(TEXT("READY TO PLAY")));
	TestEqual(TEXT("No-match interaction invites a local match"),
		Screen.InteractionTitle, FString(TEXT("Start a Local Match")));
	TestFalse(TEXT("No-match waiting state is not system resolution"),
		Screen.bSystemResolution);

	View.bMatchActive = true;
	View.bHumanInteraction = true;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectMarker;
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Human action is explicit"), Screen.InteractionKicker,
		FString(TEXT("PLAYER ACTION")));
	TestEqual(TEXT("Acting player is prominent"), Screen.ActingStatusLabel,
		FString(TEXT("PLAYER B TO ACT")));
	TestEqual(TEXT("Human task is plain language"), Screen.InteractionTitle,
		FString(TEXT("Select Marker")));

	View.bHumanInteraction = false;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::ContinueResolution;
	View.ContinueActionLabel = TEXT("Roll Route Dice");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestTrue(TEXT("System-owned step is explicit"), Screen.bSystemResolution);
	TestEqual(TEXT("System action uses authoritative continuation label"),
		Screen.InteractionTitle, FString(TEXT("Roll Route Dice")));

	View.bHumanInteraction = true;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectBranchIntent;
	View.ActionLabel = TEXT("Cross");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Cross branch prompt is family-specific"),
		Screen.InteractionTitle, FString(TEXT("Choose Cross Type")));
	View.ActionLabel = TEXT("Long Shot");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Shot branch prompt is family-specific"),
		Screen.InteractionTitle, FString(TEXT("Choose Shot Type")));

	View.bMatchEnded = true;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::MatchEnded;
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Ended match has final-result status"),
		Screen.ActingStatusLabel, FString(TEXT("FINAL RESULT")));
	TestEqual(TEXT("Ended match has completion title"),
		Screen.InteractionTitle, FString(TEXT("Match Complete")));

	FString ControllerSource;
	TestTrue(TEXT("Player-facing production source loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
			ControllerSource));
	TestTrue(TEXT("Four primary screen regions are explicit"),
		ControllerSource.Contains(TEXT("MATCH HEADER"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION"))
			&& ControllerSource.Contains(TEXT("RESOLUTION RESULT")));
	TestTrue(TEXT("Developer diagnostics are secondary and collapsible"),
		ControllerSource.Contains(TEXT("SNew(SExpandableArea)"))
			&& ControllerSource.Contains(TEXT("Developer Details"))
			&& ControllerSource.Contains(TEXT("InitiallyCollapsed(true)")));
	TestTrue(TEXT("Field exposes canonical side-relative football zones"),
		ControllerSource.Contains(TEXT("RELATIVE ZONES"))
			&& ControllerSource.Contains(TEXT("Slot.PlayerARelativeZone"))
			&& ControllerSource.Contains(TEXT("Slot.PlayerBRelativeZone")));
	TestTrue(TEXT("Card reference is secondary to readable card data"),
		ControllerSource.Contains(TEXT("DeveloperReferenceLabel"))
			&& ControllerSource.Contains(TEXT("SkillSummaryLabel")));
	TestTrue(TEXT("Resolution evidence has structured dice and comparison labels"),
		ControllerSource.Contains(TEXT("DICE"))
			&& ControllerSource.Contains(TEXT("D6  %d"))
			&& ControllerSource.Contains(TEXT("FORMULA COMPARISON"))
			&& ControllerSource.Contains(TEXT("ATTACK"))
			&& ControllerSource.Contains(TEXT("DEFENSE")));
	TestTrue(TEXT("All established player controls remain reachable"),
		ControllerSource.Contains(TEXT("Finish Deployment"))
			&& ControllerSource.Contains(TEXT("Decline"))
			&& ControllerSource.Contains(TEXT("Resolve No Legal Choice"))
			&& ControllerSource.Contains(TEXT("Branch / Shot Type"))
			&& ControllerSource.Contains(TEXT("One-on-One Shot Type"))
			&& ControllerSource.Contains(TEXT("ContinueResolution()"))
			&& ControllerSource.Contains(TEXT("PASS CONTROL")));

	const int32 SurfaceStart = ControllerSource.Find(
		TEXT("AFMCodexLocalMatchPlayerController::BuildControlSurface"));
	const FString SurfaceBody = SurfaceStart == INDEX_NONE
		? FString() : ControllerSource.Mid(SurfaceStart);
	TestTrue(TEXT("Screen builder remains a presentation-only consumer"),
		!SurfaceBody.IsEmpty()
			&& !SurfaceBody.Contains(TEXT("Host->"))
			&& !SurfaceBody.Contains(TEXT("RandRange"))
			&& !SurfaceBody.Contains(TEXT("ResolveFormula("))
			&& !SurfaceBody.Contains(TEXT("RuntimeState.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPitchAndZoneRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.26.PitchAndZoneRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPitchAndZoneRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Pitch refinement Host exists"), Host);
	TestNotNull(TEXT("Pitch refinement Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Controller->StartNewDemoMatch();
	AcknowledgeIfPending(*Controller);
	Controller->BeginDemoOrdinaryAttack();
	AcknowledgeIfPending(*Controller);
	TestTrue(TEXT("Pitch fixture begins authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess);

	const FFMCodexLocalMatchInteractionView InitialView =
		Controller->GetInteractionView();
	TestEqual(TEXT("Pitch has two physical halves"),
		InitialView.PitchRegions.Num(), 2);
	if (InitialView.PitchRegions.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("Player B half is stably first"),
		InitialView.PitchRegions[0].NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerB);
	TestEqual(TEXT("Player A half is stably second"),
		InitialView.PitchRegions[1].NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerA);

	TArray<FName> PresentedSlotOrder;
	int32 EmptySlotCount = 0;
	int32 AttackingSideEmphasisCount = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: InitialView.PitchRegions)
	{
		AttackingSideEmphasisCount += Region.bCurrentAttackingSide ? 1 : 0;
		TestFalse(TEXT("Physical half has side-relative zone context"),
			Region.ZoneContextLabel.IsEmpty());
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			PresentedSlotOrder.Add(Slot.SlotId);
			EmptySlotCount += Slot.bOccupied ? 0 : 1;
			const auto PlayerAZone =
				FMatchPlayRelativeDeploymentZoneResolver::Resolve(
					Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog,
					Slot.SlotId,
					InitialView.CurrentAttackingPlayer,
					EInitialTurnOrderPlayer::PlayerA);
			const auto PlayerBZone =
				FMatchPlayRelativeDeploymentZoneResolver::Resolve(
					Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog,
					Slot.SlotId,
					InitialView.CurrentAttackingPlayer,
					EInitialTurnOrderPlayer::PlayerB);
			TestTrue(TEXT("Canonical resolver accepts displayed slot for A"),
				PlayerAZone.bSuccess);
			TestTrue(TEXT("Canonical resolver accepts displayed slot for B"),
				PlayerBZone.bSuccess);
			if (PlayerAZone.bSuccess && PlayerBZone.bSuccess)
			{
				TestEqual(TEXT("Player A displayed zone equals canonical resolver"),
					Slot.PlayerARelativeZone, PlayerAZone.RelativeZone);
				TestEqual(TEXT("Player B displayed zone equals canonical resolver"),
					Slot.PlayerBRelativeZone, PlayerBZone.RelativeZone);
			}
		}
	}
	TestEqual(TEXT("All canonical slots are represented"),
		PresentedSlotOrder.Num(),
		Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots.Num());
	TestEqual(TEXT("Initial field visibly represents every slot as empty"),
		EmptySlotCount, PresentedSlotOrder.Num());
	TestEqual(TEXT("Exactly one physical side is emphasized as attacking"),
		AttackingSideEmphasisCount, 1);
	TestTrue(TEXT("Direction is derived and player-readable"),
		InitialView.AttackDirectionLabel.Contains(
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InitialView.CurrentAttackingPlayer))
			&& InitialView.AttackDirectionLabel.Contains(TEXT("Half")));

	TArray<FName> ExpectedStableOrder;
	for (const EMatchPlayNeutralSlotSide Side : {
		EMatchPlayNeutralSlotSide::NearPlayerB,
		EMatchPlayNeutralSlotSide::NearPlayerA })
	{
		for (const FMatchPlayDeploymentSlotDefinition& Slot
			: Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots)
		{
			if (Slot.NeutralSide == Side)
			{
				ExpectedStableOrder.Add(Slot.SlotId);
			}
		}
	}
	TestTrue(TEXT("Slot ordering is physical-half then canonical catalog order"),
		PresentedSlotOrder == ExpectedStableOrder);

	auto FindPitchSlot = [](
		const FFMCodexLocalMatchInteractionView& View,
		const FName SlotId) -> const FFMCodexLocalMatchPitchSlotView*
	{
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			if (const FFMCodexLocalMatchPitchSlotView* Slot =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexLocalMatchPitchSlotView& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Slot;
			}
		}
		return nullptr;
	};

	FName RejectedCardId = NAME_None;
	FName RejectedSlotId = NAME_None;
	bool bDeployedGoalkeeper = false;
	TSet<EInitialTurnOrderPlayer> PopulatedSides;
	for (int32 Step = 0; Step < 4; ++Step)
	{
		AcknowledgeIfPending(*Controller);
		const FFMCodexLocalMatchInteractionView BeforeView =
			Controller->GetInteractionView();
		const EInitialTurnOrderPlayer Side =
			BeforeView.CurrentLegalDeploymentSide;
		const bool bPreferGoalkeeper = !bDeployedGoalkeeper
			&& Side != BeforeView.CurrentAttackingPlayer;
		const FFMCodexLocalMatchDeploymentGroup* Group =
			BeforeView.DeploymentGroups.FindByPredicate(
				[Side, bPreferGoalkeeper](
					const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return Candidate.Side == Side
						&& Candidate.bGoalkeeper == bPreferGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				});
		TestNotNull(TEXT("Dense fixture has a legal authoritative group"), Group);
		if (Group == nullptr)
		{
			return false;
		}
		const FFMCodexLocalMatchSlotView Destination = Group->LegalSlots[0];
		TestTrue(TEXT("Destination label relates action to zone and canonical slot"),
			Destination.Label.Contains(TEXT("Slot "))
				&& Destination.Label.Contains(
					FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Destination.RelativeZone))
				&& Destination.Label.Contains(
					FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Destination.NeutralSide)));
		const FFMCodexLocalMatchCardView ExpectedCard = Group->Card;
		if (Group->bGoalkeeper)
		{
			Controller->DeployGoalkeeper(Destination.SlotId);
			bDeployedGoalkeeper = true;
		}
		else
		{
			Controller->DeployOrdinary(Group->CardId, Destination.SlotId);
			if (RejectedCardId.IsNone())
			{
				RejectedCardId = Group->CardId;
				RejectedSlotId = Destination.SlotId;
			}
		}
		TestTrue(TEXT("Authoritative deployment succeeds"),
			Controller->GetLastDiagnostic().bHostSuccess);
		const FFMCodexLocalMatchInteractionView AfterView =
			Controller->GetInteractionView();
		const FFMCodexLocalMatchPitchSlotView* PresentedSlot =
			FindPitchSlot(AfterView, Destination.SlotId);
		TestNotNull(TEXT("Snapshot rebuild retains exact destination slot"),
			PresentedSlot);
		if (PresentedSlot != nullptr)
		{
			TestTrue(TEXT("Exact slot becomes occupied by authoritative card"),
				PresentedSlot->bOccupied
					&& PresentedSlot->Card.CardId == ExpectedCard.CardId
					&& PresentedSlot->Card.SlotId == Destination.SlotId);
			TestTrue(TEXT("Pitch and candidate identity presentation agree"),
				PresentedSlot->Card.DisplayLabel == ExpectedCard.DisplayLabel
					&& PresentedSlot->Card.Side == ExpectedCard.Side
					&& PresentedSlot->Card.SkillLabels == ExpectedCard.SkillLabels);
			TestEqual(TEXT("Pitch card zone equals destination resolver zone"),
				PresentedSlot->Card.RelativeZone, Destination.RelativeZone);
		}
		PopulatedSides.Add(Side);
	}
	TestTrue(TEXT("Dense fixture includes a goalkeeper"), bDeployedGoalkeeper);
	TestEqual(TEXT("Dense fixture populates both player sides"),
		PopulatedSides.Num(), 2);

	int32 OccupiedSlotCount = 0;
	int32 GoalkeeperCount = 0;
	TArray<FName> OrderAfterDeployments;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			OrderAfterDeployments.Add(Slot.SlotId);
			if (Slot.bOccupied)
			{
				++OccupiedSlotCount;
				GoalkeeperCount += Slot.Card.bGoalkeeper ? 1 : 0;
			}
		}
	}
	TestEqual(TEXT("Dense pitch drops no deployed cards"), OccupiedSlotCount, 4);
	TestEqual(TEXT("Goalkeeper is represented exactly once"), GoalkeeperCount, 1);
	TestTrue(TEXT("Deployment refresh never changes stable slot order"),
		OrderAfterDeployments == ExpectedStableOrder);

	AcknowledgeIfPending(*Controller);
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	auto PitchSignature = [](const FFMCodexLocalMatchInteractionView& View)
	{
		TArray<FString> Result;
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
			{
				Result.Add(FString::Printf(
					TEXT("%s|%s|%s"),
					*Slot.SlotId.ToString(),
					Slot.bOccupied ? TEXT("occupied") : TEXT("empty"),
					*Slot.Card.CardId.ToString()));
			}
		}
		return Result;
	};
	const TArray<FString> PitchBeforeRejected =
		PitchSignature(Controller->GetInteractionView());
	Controller->DeployOrdinary(RejectedCardId, RejectedSlotId);
	TestFalse(TEXT("Repeated deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Rejected deployment leaves State byte-identical"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestTrue(TEXT("Rejected deployment causes no optimistic pitch movement"),
		PitchBeforeRejected == PitchSignature(Controller->GetInteractionView()));
	TestTrue(TEXT("Rejected deployment remains visibly diagnosed"),
		Controller->GetResolutionFeedback().bRejected);

	FString ControllerSource;
	FString ViewSource;
	TestTrue(TEXT("Pitch controller source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Pitch view source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
		ViewSource));
	TestTrue(TEXT("Dense slots use wrapping and bounded compact cards"),
		ControllerSource.Contains(TEXT("MakeCompactPitchCard"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)"))
			&& ControllerSource.Contains(TEXT("WidthOverride(218.0f)")));
	TestTrue(TEXT("Empty slot copy explicitly avoids false legality"),
		ControllerSource.Contains(TEXT("EMPTY SLOT"))
			&& ControllerSource.Contains(
				TEXT("Legality is shown in player actions")));
	TestTrue(TEXT("Stage 6.1 architecture and handoff remain present"),
		ControllerSource.Contains(TEXT("MATCH HEADER"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION"))
			&& ControllerSource.Contains(TEXT("RESOLUTION RESULT"))
			&& ControllerSource.Contains(TEXT("Developer Details"))
			&& ControllerSource.Contains(TEXT("PASS CONTROL")));

	const int32 PitchSlotStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakePitchSlot"));
	const int32 FeedbackStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeFeedbackPanel"));
	const FString PitchWidgetBody = PitchSlotStart != INDEX_NONE
		&& FeedbackStart > PitchSlotStart
			? ControllerSource.Mid(
				PitchSlotStart, FeedbackStart - PitchSlotStart)
			: FString();
	TestTrue(TEXT("Pitch slots remain non-interactive state presentation"),
		!PitchWidgetBody.IsEmpty()
			&& !PitchWidgetBody.Contains(TEXT("MakeButton"))
			&& !PitchWidgetBody.Contains(TEXT("Host->"))
			&& !PitchWidgetBody.Contains(TEXT("LegalityEvaluator"))
			&& !PitchWidgetBody.Contains(TEXT("AvailabilityQuery")));

	const int32 PitchModelStart = ViewSource.Find(
		TEXT("void BuildPitchRegions"));
	const int32 SelectionStart = ViewSource.Find(
		TEXT("void AddSelectionOption"));
	const FString PitchModelBody = PitchModelStart != INDEX_NONE
		&& SelectionStart > PitchModelStart
			? ViewSource.Mid(PitchModelStart, SelectionStart - PitchModelStart)
			: FString();
	TestTrue(TEXT("Pitch model reuses canonical resolver without legality logic"),
		!PitchModelBody.IsEmpty()
			&& PitchModelBody.Contains(
				TEXT("FMatchPlayRelativeDeploymentZoneResolver::Resolve"))
			&& !PitchModelBody.Contains(TEXT("Legality"))
			&& !PitchModelBody.Contains(TEXT("Availability"))
			&& !PitchModelBody.Contains(TEXT("ResolveFormula("))
			&& !PitchModelBody.Contains(TEXT("RandRange")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCardVisualHierarchyRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.27.CardVisualHierarchyRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCardVisualHierarchyRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Card hierarchy Host exists"), Host);
	TestNotNull(TEXT("Card hierarchy Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Controller->StartNewDemoMatch();
	AcknowledgeIfPending(*Controller);
	Controller->BeginDemoOrdinaryAttack();
	AcknowledgeIfPending(*Controller);
	const FFMCodexLocalMatchInteractionView InitialView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		InitialView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("Ordinary interaction card exists"), OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView OrdinaryCard = OrdinaryGroup->Card;
	TestEqual(TEXT("DisplayName fallback remains deterministic"),
		OrdinaryCard.DisplayLabel,
		FString::Printf(TEXT("Card %s"), *OrdinaryCard.CardId.ToString()));
	TestFalse(TEXT("Compact role is derived and readable"),
		OrdinaryCard.CompactRoleLabel.IsEmpty());
	TestTrue(TEXT("Production card exposes a readable Skill"),
		!OrdinaryCard.SkillLabels.IsEmpty()
			&& OrdinaryCard.SkillSummaryLabel.Contains(
				OrdinaryCard.SkillLabels[0]));
	TestTrue(TEXT("Available status is presentation-derived"),
		OrdinaryCard.bAvailable
			&& OrdinaryCard.StatusLabels.Contains(TEXT("AVAILABLE"))
			&& OrdinaryCard.StatusSummaryLabel.Contains(TEXT("AVAILABLE")));
	TestTrue(TEXT("Developer reference retains CardId and rarity"),
		OrdinaryCard.DeveloperReferenceLabel.Contains(
			OrdinaryCard.CardId.ToString())
			&& OrdinaryCard.DeveloperReferenceLabel.Contains(TEXT("Rarity:")));
	for (const TCHAR* FullAttribute : {
		TEXT("SHO"), TEXT("DRI"), TEXT("PAS"), TEXT("OFF"), TEXT("MRK"),
		TEXT("TKL"), TEXT("SPD"), TEXT("STR"), TEXT("STA"), TEXT("LS") })
	{
		TestTrue(FString::Printf(TEXT("Interaction attributes retain %s"),
			FullAttribute), OrdinaryCard.AttributeSummary.Contains(FullAttribute));
	}
	TestTrue(TEXT("Compact outfield subset is stable and bounded"),
		OrdinaryCard.CompactAttributeSummary.Contains(TEXT("SHO"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("PAS"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("DRI"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("SPD"))
			&& !OrdinaryCard.CompactAttributeSummary.Contains(TEXT("OFF"))
			&& !OrdinaryCard.CompactAttributeSummary.Contains(TEXT("STR")));

	auto FindGroup = [](const FFMCodexLocalMatchInteractionView& View,
		const FName CardId) -> const FFMCodexLocalMatchDeploymentGroup*
	{
		return View.DeploymentGroups.FindByPredicate(
			[CardId](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	};
	auto FindAuthorityCard = [](FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId) -> FPlayerCardRuleSnapshot*
	{
		FPlayerCardRuleSnapshotSet& Set =
			Side == EInitialTurnOrderPlayer::PlayerA
				? State.CardSnapshotAuthority.PlayerACardSnapshots
				: State.CardSnapshotAuthority.PlayerBCardSnapshots;
		return Set.Cards.FindByPredicate(
			[CardId](const FPlayerCardRuleSnapshot& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	};
	FMatchPlayState MultiSkillState = Host->GetMatchSnapshot().Snapshot;
	FPlayerCardRuleSnapshot* MultiSkillSnapshot = FindAuthorityCard(
		MultiSkillState, OrdinaryCard.Side, OrdinaryCard.CardId);
	TestNotNull(TEXT("Multi-Skill fixture finds authoritative snapshot"),
		MultiSkillSnapshot);
	if (MultiSkillSnapshot != nullptr && Demo.SkillRuleSet.SkillRules.Num() >= 2)
	{
		MultiSkillSnapshot->SkillIds = {
			Demo.SkillRuleSet.SkillRules[0].SkillId,
			Demo.SkillRuleSet.SkillRules[1].SkillId
		};
		const FFMCodexLocalMatchInteractionView MultiSkillView =
			FFMCodexLocalMatchInteractionViewBuilder::Build(
				MultiSkillState, Demo.SkillRuleSet);
		const FFMCodexLocalMatchDeploymentGroup* MultiSkillGroup =
			FindGroup(MultiSkillView, OrdinaryCard.CardId);
		TestNotNull(TEXT("Multi-Skill card remains present"), MultiSkillGroup);
		if (MultiSkillGroup != nullptr)
		{
			TestEqual(TEXT("All authoritative Skills remain in CardView"),
				MultiSkillGroup->Card.SkillLabels.Num(), 2);
			for (const FString& Skill : MultiSkillGroup->Card.SkillLabels)
			{
				TestTrue(TEXT("Compact/detail summary never drops a Skill"),
					MultiSkillGroup->Card.SkillSummaryLabel.Contains(Skill));
			}
		}

		MultiSkillSnapshot->SkillIds.Reset();
		const FFMCodexLocalMatchInteractionView NoSkillView =
			FFMCodexLocalMatchInteractionViewBuilder::Build(
				MultiSkillState, Demo.SkillRuleSet);
		const FFMCodexLocalMatchDeploymentGroup* NoSkillGroup =
			FindGroup(NoSkillView, OrdinaryCard.CardId);
		TestNotNull(TEXT("No-Skill fallback card remains present"), NoSkillGroup);
		if (NoSkillGroup != nullptr)
		{
			TestTrue(TEXT("No-Skill fallback is bounded and explicit"),
				NoSkillGroup->Card.SkillLabels.IsEmpty()
					&& NoSkillGroup->Card.SkillSummaryLabel == TEXT("NO SKILL"));
		}
	}

	auto FindPitchSlot = [](const FFMCodexLocalMatchInteractionView& View,
		const FName SlotId) -> const FFMCodexLocalMatchPitchSlotView*
	{
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			if (const FFMCodexLocalMatchPitchSlotView* Slot =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexLocalMatchPitchSlotView& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Slot;
			}
		}
		return nullptr;
	};
	const FFMCodexLocalMatchSlotView OrdinaryDestination =
		OrdinaryGroup->LegalSlots[0];
	Controller->DeployOrdinary(
		OrdinaryCard.CardId, OrdinaryDestination.SlotId);
	TestTrue(TEXT("Card hierarchy deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const FFMCodexLocalMatchInteractionView AfterOrdinary =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchPitchSlotView* OrdinaryPitchSlot =
		FindPitchSlot(AfterOrdinary, OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Deployed ordinary card appears on pitch"),
		OrdinaryPitchSlot);
	if (OrdinaryPitchSlot != nullptr)
	{
		const FFMCodexLocalMatchCardView& PitchCard = OrdinaryPitchSlot->Card;
		TestTrue(TEXT("Pitch and interaction use identical shared identity"),
			PitchCard.CardId == OrdinaryCard.CardId
				&& PitchCard.DisplayLabel == OrdinaryCard.DisplayLabel
				&& PitchCard.Side == OrdinaryCard.Side
				&& PitchCard.CompactRoleLabel == OrdinaryCard.CompactRoleLabel
				&& PitchCard.SkillLabels == OrdinaryCard.SkillLabels
				&& PitchCard.SkillSummaryLabel == OrdinaryCard.SkillSummaryLabel
				&& PitchCard.DeveloperReferenceLabel
					== OrdinaryCard.DeveloperReferenceLabel);
		TestTrue(TEXT("Deployed status is freshly snapshot-derived"),
			PitchCard.bDeployed
				&& PitchCard.StatusLabels.Contains(TEXT("DEPLOYED")));
	}

	FFMCodexLocalMatchDeploymentGroup GoalkeeperGroup;
	bool bFoundGoalkeeper = false;
	for (int32 Attempt = 0; Attempt < 2 && !bFoundGoalkeeper; ++Attempt)
	{
		AcknowledgeIfPending(*Controller);
		const FFMCodexLocalMatchInteractionView View =
			Controller->GetInteractionView();
		if (const FFMCodexLocalMatchDeploymentGroup* Found =
			View.DeploymentGroups.FindByPredicate(
				[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return Candidate.bGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				}))
		{
			GoalkeeperGroup = *Found;
			bFoundGoalkeeper = true;
			break;
		}
		const FFMCodexLocalMatchDeploymentGroup* NextOrdinary =
			View.DeploymentGroups.FindByPredicate(
				[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				});
		if (NextOrdinary == nullptr)
		{
			break;
		}
		Controller->DeployOrdinary(
			NextOrdinary->CardId, NextOrdinary->LegalSlots[0].SlotId);
	}
	TestTrue(TEXT("Authoritative deployment exposes goalkeeper card"),
		bFoundGoalkeeper);
	if (bFoundGoalkeeper)
	{
		TestTrue(TEXT("GK hierarchy uses only faithful goalkeeper data"),
			GoalkeeperGroup.Card.bGoalkeeper
				&& GoalkeeperGroup.Card.CompactRoleLabel == TEXT("GK")
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("HAN"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("POS"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("REF"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("AER"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("ANT"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("1v1"))
				&& GoalkeeperGroup.Card.CompactAttributeSummary.Contains(TEXT("AER"))
				&& !GoalkeeperGroup.Card.CompactAttributeSummary.Contains(TEXT("SHO")));
		const FName GoalkeeperSlotId = GoalkeeperGroup.LegalSlots[0].SlotId;
		Controller->DeployGoalkeeper(GoalkeeperSlotId);
		TestTrue(TEXT("Goalkeeper deployment succeeds"),
			Controller->GetLastDiagnostic().bHostSuccess);
		const FFMCodexLocalMatchPitchSlotView* GoalkeeperPitchSlot =
			FindPitchSlot(Controller->GetInteractionView(), GoalkeeperSlotId);
		TestNotNull(TEXT("Goalkeeper appears in exact pitch slot"),
			GoalkeeperPitchSlot);
		if (GoalkeeperPitchSlot != nullptr)
		{
			TestTrue(TEXT("GK pitch/candidate/developer identity agrees"),
				GoalkeeperPitchSlot->Card.CardId == GoalkeeperGroup.Card.CardId
					&& GoalkeeperPitchSlot->Card.DisplayLabel
						== GoalkeeperGroup.Card.DisplayLabel
					&& GoalkeeperPitchSlot->Card.CompactRoleLabel == TEXT("GK")
					&& GoalkeeperPitchSlot->Card.DeveloperReferenceLabel
						== GoalkeeperGroup.Card.DeveloperReferenceLabel
					&& GoalkeeperPitchSlot->Card.StatusLabels.Contains(
						TEXT("DEPLOYED")));
		}
	}

	AcknowledgeIfPending(*Controller);
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FFMCodexLocalMatchPitchSlotView* BeforeRejectedSlot =
		FindPitchSlot(
			Controller->GetInteractionView(), OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Rejected fixture retains ordinary pitch card"),
		BeforeRejectedSlot);
	if (BeforeRejectedSlot == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView CardBeforeRejected =
		BeforeRejectedSlot->Card;
	Controller->DeployOrdinary(
		OrdinaryCard.CardId, OrdinaryDestination.SlotId);
	TestFalse(TEXT("Repeated card deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Rejected card action leaves State byte-identical"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	const FFMCodexLocalMatchPitchSlotView* CardAfterRejected =
		FindPitchSlot(Controller->GetInteractionView(), OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Rejected refresh retains the pitch card"),
		CardAfterRejected);
	if (CardAfterRejected != nullptr)
	{
		TestTrue(TEXT("Rejected action changes no card presentation evidence"),
			CardAfterRejected->Card.CardId == CardBeforeRejected.CardId
				&& CardAfterRejected->Card.DisplayLabel
					== CardBeforeRejected.DisplayLabel
				&& CardAfterRejected->Card.SkillLabels
					== CardBeforeRejected.SkillLabels
				&& CardAfterRejected->Card.AttributeSummary
					== CardBeforeRejected.AttributeSummary
				&& CardAfterRejected->Card.CompactAttributeSummary
					== CardBeforeRejected.CompactAttributeSummary
				&& CardAfterRejected->Card.StatusLabels
					== CardBeforeRejected.StatusLabels);
	}

	FString ControllerSource;
	FString ViewSource;
	TestTrue(TEXT("Card renderer source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Card presentation source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
		ViewSource));
	TestTrue(TEXT("Typography and bounded interaction hierarchy are explicit"),
		ControllerSource.Contains(TEXT("GetDefaultFontStyle(\"Bold\", 16)"))
			&& ControllerSource.Contains(TEXT("MaxDesiredWidth(520.0f)"))
			&& ControllerSource.Contains(TEXT("OUTFIELD ATTRIBUTES"))
			&& ControllerSource.Contains(TEXT("GOALKEEPER ATTRIBUTES"))
			&& ControllerSource.Contains(TEXT("STATUS  |  ")));
	const int32 InteractionCardStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeCardPanel"));
	const int32 CompactCardStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeCompactPitchCard"));
	const int32 PitchSlotStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakePitchSlot"));
	const FString InteractionCardBody = InteractionCardStart != INDEX_NONE
		&& CompactCardStart > InteractionCardStart
			? ControllerSource.Mid(
				InteractionCardStart, CompactCardStart - InteractionCardStart)
			: FString();
	const FString CompactCardBody = CompactCardStart != INDEX_NONE
		&& PitchSlotStart > CompactCardStart
			? ControllerSource.Mid(
				CompactCardStart, PitchSlotStart - CompactCardStart)
			: FString();
	TestTrue(TEXT("Both variants consume one shared CardView presentation"),
		!InteractionCardBody.IsEmpty() && !CompactCardBody.IsEmpty()
			&& InteractionCardBody.Contains(TEXT("Card.SkillSummaryLabel"))
			&& CompactCardBody.Contains(TEXT("Card.SkillSummaryLabel"))
			&& InteractionCardBody.Contains(TEXT("Card.StatusSummaryLabel"))
			&& CompactCardBody.Contains(TEXT("Card.StatusSummaryLabel")));
	TestTrue(TEXT("Developer reference stays out of compact pitch hierarchy"),
		InteractionCardBody.Contains(TEXT("Card.DeveloperReferenceLabel"))
			&& !CompactCardBody.Contains(TEXT("Card.DeveloperReferenceLabel")));
	TestTrue(TEXT("Card renderers contain no gameplay legality or command logic"),
		!InteractionCardBody.Contains(TEXT("LegalityEvaluator"))
			&& !CompactCardBody.Contains(TEXT("LegalityEvaluator"))
			&& !InteractionCardBody.Contains(TEXT("AvailabilityQuery"))
			&& !CompactCardBody.Contains(TEXT("AvailabilityQuery"))
			&& !InteractionCardBody.Contains(TEXT("Host->"))
			&& !CompactCardBody.Contains(TEXT("Host->")));
	TestTrue(TEXT("Stage 6.2 pitch and hot-seat shell remain present"),
		ControllerSource.Contains(TEXT("CENTER / PHYSICAL HALF BOUNDARY"))
			&& ControllerSource.Contains(TEXT("RELATIVE ZONES"))
			&& ControllerSource.Contains(TEXT("EMPTY SLOT"))
			&& ControllerSource.Contains(TEXT("CURRENT ATTACKING SIDE"))
			&& ControllerSource.Contains(TEXT("PASS CONTROL")));
	TestTrue(TEXT("Card status derivation contains no legality calculation"),
		ViewSource.Contains(TEXT("FinalizeCardPresentation"))
			&& !ViewSource.Contains(TEXT("StatusSummaryLabel ==")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchUMGPlayerFacingFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.28.UMGPlayerFacingFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchUMGPlayerFacingFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("UMG foundation Host exists"), Host);
	TestNotNull(TEXT("UMG foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Controller creates and owns root UMG screen"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	TestTrue(TEXT("Widget input boundary points only to its Controller"),
		Screen->GetMatchController() == Controller
			&& (Screen->GetOwningPlayer() == Controller
				|| Screen->GetOwningPlayer() == nullptr));
	for (const FName RegionName : {
		FName(TEXT("MatchHeaderRegion")),
		FName(TEXT("FootballCardFieldRegion")),
		FName(TEXT("CurrentInteractionRegion")),
		FName(TEXT("ResolutionResultRegion")) })
	{
		TestNotNull(FString::Printf(TEXT("Root screen exposes %s"),
			*RegionName.ToString()), Screen->GetWidgetFromName(RegionName));
	}
	TestTrue(TEXT("No-match UMG exposes explicit Start intent"),
		Screen->GetPresentation().Interaction.bCanStartNewMatch
			&& Screen->GetPresentation().Interaction.CategoryLabel
				== TEXT("Start Match"));

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("UMG StartNewMatch reaches typed authoritative Host path"),
		Host->HasActiveLocalMatch()
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("StartNewLocalMatch"));
	const FFMCodexLocalMatchInteractionView& StartedView =
		Controller->GetInteractionView();
	const FFMCodexUMGMatchScreenViewModel& StartedUMG =
		Screen->GetPresentation();
	TestTrue(TEXT("UMG header equals authoritative presentation contract"),
		StartedUMG.Header.ScoreLabel == FString::Printf(
			TEXT("%d - %d"), StartedView.PlayerAScore,
			StartedView.PlayerBScore)
			&& StartedUMG.Header.CurrentAttackerLabel.Contains(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					StartedView.CurrentAttackingPlayer))
			&& StartedUMG.Header.ExpectedActorLabel.Contains(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					StartedView.ExpectedActingPlayer))
			&& StartedUMG.Header.bMatchEnded == StartedView.bMatchEnded);
	TestTrue(TEXT("Authoritative human transition displays UMG handoff"),
		Controller->IsAwaitingHotSeatHandoff()
			&& StartedUMG.Handoff.bVisible
			&& StartedUMG.Handoff.TitleLabel == TEXT("PASS CONTROL")
			&& StartedUMG.Handoff.NextPlayerLabel.Contains(TEXT("Player")));
	if (!Controller->IsAwaitingHotSeatHandoff())
	{
		return false;
	}
	const TArray<uint8> StateBeforeReady =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestReady();
	TestTrue(TEXT("UMG Ready is presentation-only and reveals controls"),
		!Controller->IsAwaitingHotSeatHandoff()
			&& !Screen->GetPresentation().Handoff.bVisible
			&& StateBeforeReady
				== SerializeState(Host->GetMatchSnapshot().Snapshot));

	Screen->RequestBeginOrdinaryAttack();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	TestTrue(TEXT("UMG BeginAttack creates authoritative CurrentAttack"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bCurrentAttackActive
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy
			&& Screen->GetPresentation().Interaction.bCanFinishDeployment);

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexUMGMatchScreenViewModel& DeploymentUMG =
		Screen->GetPresentation();
	int32 PitchSlotCount = 0;
	for (const FFMCodexUMGPitchRegionViewModel& Region
		: DeploymentUMG.PitchRegions)
	{
		PitchSlotCount += Region.Slots.Num();
	}
	TestTrue(TEXT("UMG pitch preserves two physical halves and 10 slots"),
		DeploymentUMG.PitchRegions.Num() == 2 && PitchSlotCount == 10);
	TestTrue(TEXT("UMG interaction preserves bounded candidate cards"),
		!DeploymentUMG.Interaction.CandidateCards.IsEmpty()
			&& DeploymentUMG.Interaction.CandidateCards.Num()
				== DeploymentView.DeploymentGroups.Num());

	TSet<FString> SkillFamilies;
	for (const FFMCodexUMGCardViewModel& Card
		: DeploymentUMG.Interaction.CandidateCards)
	{
		for (const FString& Skill : Card.SkillLabels)
		{
			SkillFamilies.Add(Skill);
		}
	}
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("UMG retains %s family"), Family),
			SkillFamilies.Contains(Family));
	}

	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("UMG deployment proof finds ordinary candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView OrdinaryCardView = OrdinaryGroup->Card;
	const FFMCodexUMGCardViewModel* CandidateCard =
		DeploymentUMG.Interaction.CandidateCards.FindByPredicate(
			[OrdinaryGroup](const FFMCodexUMGCardViewModel& Candidate)
			{
				return Candidate.CardId == OrdinaryGroup->CardId;
			});
	TestNotNull(TEXT("UMG candidate has shared Card identity"), CandidateCard);
	if (CandidateCard != nullptr)
	{
		TestTrue(TEXT("UMG Card DTO preserves Stage 6.3 semantics"),
			CandidateCard->IdentityLabel == OrdinaryGroup->Card.DisplayLabel
				&& CandidateCard->RoleLabel
					== OrdinaryGroup->Card.CompactRoleLabel
				&& CandidateCard->SkillLabels == OrdinaryGroup->Card.SkillLabels
				&& CandidateCard->CompactAttributeSummary
					== OrdinaryGroup->Card.CompactAttributeSummary
				&& CandidateCard->StatusLabels
					== OrdinaryGroup->Card.StatusLabels);
	}

	const FName DeployedCardId = OrdinaryGroup->CardId;
	const FName DeployedSlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const TArray<uint8> BeforeDeployment =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Representative typed UMG deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeDeployment
				!= SerializeState(Host->GetMatchSnapshot().Snapshot));
	auto FindUMGSlot = [](const FFMCodexUMGMatchScreenViewModel& View,
		const FName SlotId) -> const FFMCodexUMGPitchSlotViewModel*
	{
		for (const FFMCodexUMGPitchRegionViewModel& Region : View.PitchRegions)
		{
			if (const FFMCodexUMGPitchSlotViewModel* Found =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexUMGPitchSlotViewModel& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Found;
			}
		}
		return nullptr;
	};
	const FFMCodexUMGPitchSlotViewModel* DeployedSlot =
		FindUMGSlot(Screen->GetPresentation(), DeployedSlotId);
	TestNotNull(TEXT("Successful UMG command refreshes exact pitch slot"),
		DeployedSlot);
	if (DeployedSlot != nullptr)
	{
		TestTrue(TEXT("Candidate-to-pitch UMG identity remains equivalent"),
			DeployedSlot->bOccupied
				&& DeployedSlot->Card.CardId == DeployedCardId
				&& DeployedSlot->Card.IdentityLabel
					== OrdinaryCardView.DisplayLabel
				&& DeployedSlot->Card.SkillLabels
					== OrdinaryCardView.SkillLabels);
	}

	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Rejected UMG intent leaves State unchanged and refreshes failure"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Screen->GetPresentation().Resolution.bVisible
			&& Screen->GetPresentation().Resolution.bRejected
			&& !Screen->GetPresentation().Resolution.ErrorLabel.IsEmpty());
	const FFMCodexUMGPitchSlotViewModel* AfterRejectedSlot =
		FindUMGSlot(Screen->GetPresentation(), DeployedSlotId);
	TestTrue(TEXT("Rejected UMG intent performs no optimistic card mutation"),
		AfterRejectedSlot != nullptr && AfterRejectedSlot->bOccupied
			&& AfterRejectedSlot->Card.CardId == DeployedCardId
			&& AfterRejectedSlot->Card.SkillLabels
				== OrdinaryCardView.SkillLabels);

	FFMCodexLocalMatchCardView GoalkeeperSource;
	GoalkeeperSource.CardId = TEXT("TestGK");
	GoalkeeperSource.DisplayLabel = TEXT("Card TestGK");
	GoalkeeperSource.CompactRoleLabel = TEXT("GK");
	GoalkeeperSource.SkillLabels = { TEXT("Cross"), TEXT("Through Ball") };
	GoalkeeperSource.SkillSummaryLabel = TEXT("Cross  |  Through Ball");
	GoalkeeperSource.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperSource.GoalkeeperAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 2 | 1v1 2");
	GoalkeeperSource.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	GoalkeeperSource.StatusSummaryLabel = TEXT("GK USED  |  GK ACTIVE");
	GoalkeeperSource.bGoalkeeper = true;
	const FFMCodexUMGCardViewModel GoalkeeperDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(GoalkeeperSource);
	TestTrue(TEXT("UMG DTO retains complete multi-Skill GK presentation"),
		GoalkeeperDTO.bGoalkeeper && GoalkeeperDTO.RoleLabel == TEXT("GK")
			&& GoalkeeperDTO.SkillLabels.Num() == 2
			&& GoalkeeperDTO.FullAttributeSummary
				== GoalkeeperSource.GoalkeeperAttributeSummary
			&& GoalkeeperDTO.StatusLabels == GoalkeeperSource.StatusLabels);
	const FFMCodexUMGCardViewModel MissingCardDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard({});
	TestTrue(TEXT("UMG Card DTO missing-data fallback is bounded"),
		MissingCardDTO.IdentityLabel == TEXT("UNKNOWN CARD")
			&& MissingCardDTO.RoleLabel == TEXT("ROLE N/A")
			&& MissingCardDTO.SkillSummaryLabel == TEXT("NO SKILL")
			&& MissingCardDTO.StatusSummaryLabel == TEXT("UNAVAILABLE"));

	FFMCodexLocalMatchResolutionFeedback FeedbackFixture;
	FeedbackFixture.bVisible = true;
	FeedbackFixture.bTerminal = true;
	FeedbackFixture.StepTitle = TEXT("STEP FIXTURE");
	FeedbackFixture.DiceEntries.Add({
		EFMCodexLocalMatchRollGroup::PostRoute, TEXT("Fixture"), 6 });
	FeedbackFixture.DecisionSummary = TEXT("DECISION FIXTURE");
	FeedbackFixture.TerminalSummary = TEXT("RESULT: GOAL");
	const FFMCodexUMGMatchScreenViewModel FeedbackDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(), FeedbackFixture,
			Controller->GetLastDiagnostic().Message, false, FString());
	TestTrue(TEXT("UMG resolution DTO preserves Step/Dice/Decision/Terminal"),
		FeedbackDTO.Resolution.bVisible
			&& FeedbackDTO.Resolution.StepLabel == TEXT("STEP FIXTURE")
			&& FeedbackDTO.Resolution.DiceLabels.Num() == 1
			&& FeedbackDTO.Resolution.DiceLabels[0].Contains(TEXT("D6 6"))
			&& FeedbackDTO.Resolution.DecisionLabel
				== TEXT("DECISION FIXTURE")
			&& FeedbackDTO.Resolution.TerminalLabel == TEXT("RESULT: GOAL"));

	FString RootWidgetHeader;
	FString RootWidgetSource;
	FString CardWidgetSource;
	FString PresentationHeader;
	FString PresentationSource;
	FString ControllerSource;
	FString BuildRules;
	TestTrue(TEXT("UMG source boundary files load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
			RootWidgetHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h"),
				PresentationHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"),
				PresentationSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/FMCodex.Build.cs"),
				BuildRules));
	const FString WidgetSources = RootWidgetHeader + RootWidgetSource
		+ CardWidgetSource;
	TestTrue(TEXT("Widget has no Session/provider/State authority escape"),
		!WidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !WidgetSources.Contains(TEXT("D6Provider"))
			&& !WidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !WidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Widget contains no rule, Formula, route, or legality logic"),
		!WidgetSources.Contains(TEXT("FormulaResolver"))
			&& !WidgetSources.Contains(TEXT("Legality"))
			&& !WidgetSources.Contains(TEXT("RandRange"))
			&& !WidgetSources.Contains(TEXT("RouteThreshold")));
	TestTrue(TEXT("Widget exposes explicit typed intents, not generic dispatch"),
		RootWidgetHeader.Contains(TEXT("RequestStartNewMatch"))
			&& RootWidgetHeader.Contains(TEXT("RequestBeginOrdinaryAttack"))
			&& RootWidgetHeader.Contains(TEXT("RequestDeployOrdinary"))
			&& !WidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !WidgetSources.Contains(TEXT("ProcessEvent"))
			&& !WidgetSources.Contains(TEXT("FindFunction")));
	TestTrue(TEXT("Presentation DTO is bounded and never mirrors MatchPlay State"),
		PresentationHeader.Contains(TEXT("FFMCodexUMGMatchHeaderViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGCardViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGPitchSlotViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGResolutionViewModel"))
			&& !PresentationHeader.Contains(TEXT("FMatchPlayState")));
	TestTrue(TEXT("Controller owns UMG lifecycle and preserves Slate fallback"),
		ControllerSource.Contains(TEXT("InitializePlayerFacingUI"))
			&& ControllerSource.Contains(TEXT("RefreshPlayerMatchScreen"))
			&& ControllerSource.Contains(TEXT("PlayerMatchScreen->RemoveFromParent"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface"))
			&& ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("bEnableDeveloperSlateSurface")));
	TestTrue(TEXT("UMG is the only added module dependency"),
		BuildRules.Contains(TEXT("\"Slate\", \"SlateCore\", \"UMG\""))
			&& !BuildRules.Contains(
				TEXT("PrivateDependencyModuleNames.Add(\"OnlineSubsystemSteam\")")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGPitchWidgetVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.29.UMGPitchWidgetVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGPitchWidgetVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Pitch foundation Host exists"), Host);
	TestNotNull(TEXT("Pitch foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Root UMG screen exists for Pitch composition"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Root composes dedicated Pitch Widget"), Pitch);
	if (Pitch == nullptr)
	{
		return false;
	}
	Pitch->TakeWidget();
	TestNotNull(TEXT("Root keeps dedicated Pitch region"),
		Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")));

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("Pitch stage preserves blocking handoff overlay"),
		Controller->IsAwaitingHotSeatHandoff()
			&& Screen->GetPresentation().Handoff.bVisible);
	if (!Controller->IsAwaitingHotSeatHandoff())
	{
		return false;
	}
	const TArray<uint8> StateBeforeReady =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestReady();
	TestTrue(TEXT("Pitch-stage Ready remains authority-neutral"),
		StateBeforeReady == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& !Screen->GetPresentation().Handoff.bVisible);
	Screen->RequestBeginOrdinaryAttack();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	TestTrue(TEXT("Pitch fixture reaches authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy);

	const TArray<FFMCodexUMGPitchRegionViewModel>& PitchPresentation =
		Pitch->GetPresentation();
	TestTrue(TEXT("Stable pitch orientation is Player B top / Player A bottom"),
		PitchPresentation.Num() == 2
			&& PitchPresentation[0].RegionLabel == TEXT("Half Near Player B")
			&& PitchPresentation[1].RegionLabel == TEXT("Half Near Player A")
			&& Pitch->GetWidgetFromName(TEXT("PlayerBPhysicalHalf")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerAPhysicalHalf")) != nullptr);
	TestNotNull(TEXT("Pitch contains visual-only center field separator"),
		Pitch->GetWidgetFromName(TEXT("CenterFieldVisualSeparator")));

	const TArray<TObjectPtr<UFMCodexPitchSlotWidget>>& RenderedSlots =
		Pitch->GetRenderedSlotWidgets();
	int32 DTOCount = 0;
	TArray<FName> ExpectedSlotOrder;
	for (const FFMCodexUMGPitchRegionViewModel& Region : PitchPresentation)
	{
		DTOCount += Region.Slots.Num();
		for (const FFMCodexUMGPitchSlotViewModel& PitchSlot : Region.Slots)
		{
			ExpectedSlotOrder.Add(PitchSlot.SlotId);
		}
	}
	TestTrue(TEXT("Dedicated Pitch Widget renders 10/10 canonical slots"),
		DTOCount == 10 && RenderedSlots.Num() == 10);
	bool bDeterministicOrder = RenderedSlots.Num() == ExpectedSlotOrder.Num();
	for (int32 Index = 0;
		bDeterministicOrder && Index < RenderedSlots.Num(); ++Index)
	{
		bDeterministicOrder = RenderedSlots[Index] != nullptr
			&& RenderedSlots[Index]->GetPresentation().SlotId
				== ExpectedSlotOrder[Index];
	}
	TestTrue(TEXT("Pitch slots preserve deterministic catalog ordering"),
		bDeterministicOrder);
	bool bSingleRowPerHalf = RenderedSlots.Num() == 10;
	for (int32 Index = 0;
		bSingleRowPerHalf && Index < RenderedSlots.Num(); ++Index)
	{
		const UUniformGridSlot* GridSlot = RenderedSlots[Index] == nullptr
			? nullptr : Cast<UUniformGridSlot>(RenderedSlots[Index]->Slot);
		bSingleRowPerHalf = GridSlot != nullptr
			&& GridSlot->GetRow() == 0
			&& GridSlot->GetColumn() == Index % 5;
	}
	TestTrue(TEXT("Each physical half renders one canonical row of five slots"),
		bSingleRowPerHalf
			&& PitchPresentation[0].Slots.Num() == 5
			&& PitchPresentation[1].Slots.Num() == 5);

	int32 EmptySlots = 0;
	for (UFMCodexPitchSlotWidget* SlotWidget : RenderedSlots)
	{
		if (SlotWidget == nullptr)
		{
			continue;
		}
		SlotWidget->TakeWidget();
		if (!SlotWidget->GetPresentation().bOccupied)
		{
			++EmptySlots;
			TestFalse(TEXT("Empty slot never fabricates a Card binding"),
				SlotWidget->IsShowingOccupiedCard());
			TestNotNull(TEXT("Empty slot exposes non-action spatial state"),
				SlotWidget->GetWidgetFromName(TEXT("EmptySpatialLocation")));
		}
	}
	TestEqual(TEXT("Initial deployment pitch has 10 empty locations"),
		EmptySlots, 10);

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("Pitch correspondence finds legal deployment candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView DeployedCardSource = OrdinaryGroup->Card;
	const FFMCodexLocalMatchSlotView Destination = OrdinaryGroup->LegalSlots[0];
	auto FindRenderedSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName SlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == SlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};
	UFMCodexPitchSlotWidget* DestinationBefore =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestNotNull(TEXT("Every legal destination corresponds to a visible slot"),
		DestinationBefore);
	if (DestinationBefore != nullptr)
	{
		const FFMCodexUMGPitchSlotViewModel& DestinationPresentation =
			DestinationBefore->GetPresentation();
		const FString ExpectedZone =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Destination.RelativeZone);
		const FString& ActingSideZone =
			DeploymentView.ExpectedActingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? DestinationPresentation.PlayerARelativeZoneLabel
					: DestinationPresentation.PlayerBRelativeZoneLabel;
		TestTrue(TEXT("Destination and Pitch share physical half and relative zone"),
			DestinationPresentation.PhysicalHalfLabel
				== FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Destination.NeutralSide)
				&& ActingSideZone == ExpectedZone);
	}

	int32 AttackingRegionCount = 0;
	for (const FFMCodexUMGPitchRegionViewModel& Region : PitchPresentation)
	{
		AttackingRegionCount += Region.bCurrentAttackingSide ? 1 : 0;
		TestTrue(TEXT("Each Pitch half retains readable relative-zone context"),
			!Region.ZoneContextLabel.IsEmpty()
				&& Region.ZoneContextLabel.Contains(TEXT("Player A:"))
				&& Region.ZoneContextLabel.Contains(TEXT("Player B:")));
	}
	TestEqual(TEXT("Exactly one physical half receives attacker emphasis"),
		AttackingRegionCount, 1);
	const int32 ExpectedAttackingRegion =
		DeploymentView.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerB ? 0 : 1;
	TestTrue(TEXT("Attacker emphasis comes from DTO, never Widget calculation"),
		PitchPresentation.IsValidIndex(ExpectedAttackingRegion)
			&& PitchPresentation[ExpectedAttackingRegion].bCurrentAttackingSide);

	TSet<FString> SkillFamilies;
	for (const FFMCodexUMGCardViewModel& Candidate
		: Screen->GetPresentation().Interaction.CandidateCards)
	{
		for (const FString& Skill : Candidate.SkillLabels)
		{
			SkillFamilies.Add(Skill);
		}
	}
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("Pitch stage preserves %s family"), Family),
			SkillFamilies.Contains(Family));
	}

	const TArray<uint8> BeforeDeployment =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardSource.CardId, Destination.SlotId);
	TestTrue(TEXT("Successful deployment refreshes Pitch from authority"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeDeployment
				!= SerializeState(Host->GetMatchSnapshot().Snapshot));
	UFMCodexPitchSlotWidget* DestinationAfter =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestNotNull(TEXT("Refreshed occupied slot remains visible"), DestinationAfter);
	if (DestinationAfter != nullptr)
	{
		DestinationAfter->TakeWidget();
		UFMCodexPlayerCardWidget* PitchCard =
			DestinationAfter->GetCardWidget();
		TestNotNull(TEXT("Occupied Slot binds shared Player Card Widget"), PitchCard);
		TestTrue(TEXT("Pitch Card binding preserves Stage 6.3 Card contract"),
			DestinationAfter->IsShowingOccupiedCard()
				&& PitchCard != nullptr
				&& PitchCard->GetPresentation().CardId
					== DeployedCardSource.CardId
				&& PitchCard->GetPresentation().IdentityLabel
					== DeployedCardSource.DisplayLabel
				&& PitchCard->GetPresentation().RoleLabel
					== DeployedCardSource.CompactRoleLabel
				&& PitchCard->GetPresentation().SkillLabels
					== DeployedCardSource.SkillLabels
				&& PitchCard->GetPresentation().CompactAttributeSummary
					== DeployedCardSource.CompactAttributeSummary
				&& PitchCard->GetPresentation().StatusLabels.Contains(
					TEXT("DEPLOYED")));
	}

	TArray<FName> OccupancyOrderBeforeRejected;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		OccupancyOrderBeforeRejected.Add(
			SlotWidget != nullptr
				? SlotWidget->GetPresentation().SlotId : NAME_None);
	}
	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardSource.CardId, Destination.SlotId);
	TArray<FName> OccupancyOrderAfterRejected;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		OccupancyOrderAfterRejected.Add(
			SlotWidget != nullptr
				? SlotWidget->GetPresentation().SlotId : NAME_None);
	}
	UFMCodexPitchSlotWidget* DestinationRejected =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestTrue(TEXT("Rejected deployment causes no optimistic Pitch movement"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& OccupancyOrderBeforeRejected == OccupancyOrderAfterRejected
			&& DestinationRejected != nullptr
			&& DestinationRejected->GetPresentation().bOccupied
			&& DestinationRejected->GetPresentation().Card.CardId
				== DeployedCardSource.CardId);

	FFMCodexUMGPitchSlotViewModel GoalkeeperSlotFixture =
		Pitch->GetPresentation()[0].Slots[0];
	GoalkeeperSlotFixture.bOccupied = true;
	GoalkeeperSlotFixture.Card.CardId = TEXT("PitchGK");
	GoalkeeperSlotFixture.Card.IdentityLabel = TEXT("Card PitchGK");
	GoalkeeperSlotFixture.Card.OwnerLabel = TEXT("Player B");
	GoalkeeperSlotFixture.Card.RoleLabel = TEXT("GK");
	GoalkeeperSlotFixture.Card.SkillLabels = { TEXT("Cross"), TEXT("Through Ball") };
	GoalkeeperSlotFixture.Card.SkillSummaryLabel = TEXT("Cross | Through Ball");
	GoalkeeperSlotFixture.Card.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperSlotFixture.Card.StatusLabels = { TEXT("GK ACTIVE") };
	GoalkeeperSlotFixture.Card.StatusSummaryLabel = TEXT("GK ACTIVE");
	GoalkeeperSlotFixture.Card.bGoalkeeper = true;
	UFMCodexPitchSlotWidget* GoalkeeperSlotWidget =
		CreateWidget<UFMCodexPitchSlotWidget>(
			Screen->GetWorld(), UFMCodexPitchSlotWidget::StaticClass());
	TestNotNull(TEXT("Standalone GK Pitch Slot constructs"), GoalkeeperSlotWidget);
	if (GoalkeeperSlotWidget != nullptr)
	{
		GoalkeeperSlotWidget->RefreshFromPitchSlotPresentation(
			GoalkeeperSlotFixture);
		GoalkeeperSlotWidget->TakeWidget();
		const UFMCodexPlayerCardWidget* GKCard =
			GoalkeeperSlotWidget->GetCardWidget();
		TestTrue(TEXT("GK Slot retains GK role, Skills, attributes and status"),
			GoalkeeperSlotWidget->IsShowingOccupiedCard()
				&& GKCard != nullptr
				&& GKCard->GetPresentation().bGoalkeeper
				&& GKCard->GetPresentation().RoleLabel == TEXT("GK")
				&& GKCard->GetPresentation().SkillLabels.Num() == 2
				&& GKCard->GetPresentation().CompactAttributeSummary
					== GoalkeeperSlotFixture.Card.CompactAttributeSummary
				&& GKCard->GetPresentation().StatusSummaryLabel
					== TEXT("GK ACTIVE"));
	}

	FString PitchHeader;
	FString PitchSource;
	FString SlotHeader;
	FString SlotSource;
	FString RootHeader;
	FString RootSource;
	FString ControllerSource;
	TestTrue(TEXT("Pitch boundary production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.h"), PitchHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"), PitchSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.h"), SlotHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"), SlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString PitchWidgetSources = PitchHeader + PitchSource
		+ SlotHeader + SlotSource;
	TestTrue(TEXT("Pitch/Slot Widgets receive presentation DTOs only"),
		PitchHeader.Contains(TEXT("FFMCodexUMGPitchRegionViewModel"))
			&& SlotHeader.Contains(TEXT("FFMCodexUMGPitchSlotViewModel"))
			&& !PitchWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !PitchWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !PitchWidgetSources.Contains(TEXT("D6Provider"))
			&& !PitchWidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Pitch/Slot Widgets contain zero gameplay authority or input"),
		!PitchWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !PitchWidgetSources.Contains(TEXT("Legality"))
			&& !PitchWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !PitchWidgetSources.Contains(TEXT("RandRange"))
			&& !PitchWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !PitchWidgetSources.Contains(TEXT("UButton"))
			&& !PitchWidgetSources.Contains(TEXT("OnClicked"))
			&& !PitchWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Pitch shell is recognizable without gameplay geography"),
		PitchSource.Contains(TEXT("FootballFieldBackground"))
			&& PitchSource.Contains(TEXT("PlayerBPhysicalHalf"))
			&& PitchSource.Contains(TEXT("CenterFieldVisualSeparator"))
			&& PitchSource.Contains(TEXT("PlayerAPhysicalHalf"))
			&& PitchSource.Contains(TEXT("ATTACKING SIDE"))
			&& !PitchSource.Contains(TEXT("Backfield row"))
			&& !PitchSource.Contains(TEXT("Forward row")));
	TestTrue(TEXT("Root delegates Pitch layout to dedicated configurable class"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexPitchWidget>"))
			&& RootSource.Contains(TEXT("DedicatedFootballPitchWidget"))
			&& RootSource.Contains(TEXT("RefreshFromPitchPresentation"))
			&& !RootSource.Contains(TEXT("UWrapBox* SlotRow")));
	TestTrue(TEXT("Controller remains unaware of Pitch geometry"),
		!ControllerSource.Contains(TEXT("UFMCodexPitchWidget"))
			&& !ControllerSource.Contains(TEXT("UniformGrid"))
			&& !ControllerSource.Contains(TEXT("SlotIndex / 5")));
	TestTrue(TEXT("Slate developer surface remains unchanged and available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGPlayerCardVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.30.UMGPlayerCardVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGPlayerCardVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Card foundation Host exists"), Host);
	TestNotNull(TEXT("Card foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Card foundation root UMG screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel MultiSkillFixture;
	MultiSkillFixture.CardId = TEXT("VisualCard");
	MultiSkillFixture.IdentityLabel = TEXT("Card VisualCard");
	MultiSkillFixture.OwnerLabel = TEXT("Player A");
	MultiSkillFixture.RoleLabel = TEXT("FW / MF");
	MultiSkillFixture.RarityLabel = TEXT("National");
	MultiSkillFixture.SkillLabels = {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball")
	};
	MultiSkillFixture.SkillSummaryLabel = FString::Join(
		MultiSkillFixture.SkillLabels, TEXT(" | "));
	MultiSkillFixture.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 2");
	MultiSkillFixture.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 2 | MRK 1 | TKL 1 | SPD 2 | STR 3 | STA 4 | LS 5");
	MultiSkillFixture.StatusLabels = { TEXT("AVAILABLE") };
	MultiSkillFixture.StatusSummaryLabel = TEXT("AVAILABLE");
	MultiSkillFixture.DeveloperReferenceLabel =
		TEXT("Card reference: VisualCard | Rarity: National");

	UFMCodexPlayerCardWidget* CardWidget =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	TestNotNull(TEXT("Reusable Player Card Widget constructs"), CardWidget);
	if (CardWidget == nullptr)
	{
		return false;
	}
	CardWidget->RefreshFromPresentation(
		MultiSkillFixture,
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	CardWidget->TakeWidget();
	for (const TCHAR* RegionName : {
		TEXT("PlayerCardFrame"), TEXT("RoleRarityHeaderRegion"),
		TEXT("PortraitPresentationRegion"), TEXT("CardIdentityRegion"),
		TEXT("SkillPresentationRegion"),
		TEXT("AttributePresentationRegion"),
		TEXT("StatusBadgePresentationRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Card hierarchy contains %s"),
			RegionName), CardWidget->GetWidgetFromName(RegionName));
	}
	TestTrue(TEXT("PitchCompact has bounded card and portrait shells"),
		CardWidget->GetWidgetFromName(TEXT("PlayerCardBounds")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("PortraitAssetBounds"))
				!= nullptr);
	TestTrue(TEXT("Role and rarity have independent discoverable positions"),
		CardWidget->GetWidgetFromName(TEXT("CardRole")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("CardRarity")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("RoleIconHook")) != nullptr);
	TestTrue(TEXT("Portrait and Skill future asset hooks are explicit"),
		CardWidget->GetWidgetFromName(TEXT("PortraitPlaceholderText")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("SkillIconHook0")) != nullptr);
	TestTrue(TEXT("PitchCompact preserves every multi-Skill identity"),
		CardWidget->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::PitchCompact
			&& CardWidget->GetRenderedSkillCount()
				== MultiSkillFixture.SkillLabels.Num());
	TestTrue(TEXT("PitchCompact renders stable SHO PAS DRI SPD stat block"),
		CardWidget->GetRenderedAttributeCount() == 4
			&& CardWidget->GetRenderedAttributeSummary()
				== MultiSkillFixture.CompactAttributeSummary
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SHO"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("PAS"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("DRI"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SPD")));
	TestEqual(TEXT("Status labels become structured badge entries"),
		CardWidget->GetRenderedStatusBadgeCount(), 1);
	const UTextBlock* DeveloperReference = Cast<UTextBlock>(
		CardWidget->GetWidgetFromName(TEXT("CardDeveloperReference")));
	TestTrue(TEXT("Developer reference does not dominate PitchCompact"),
		DeveloperReference != nullptr
			&& DeveloperReference->GetVisibility()
				== ESlateVisibility::Collapsed);

	CardWidget->SetPresentationMode(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("Interaction mode preserves all ten outfield attributes"),
		CardWidget->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& CardWidget->GetRenderedAttributeCount() == 10
			&& CardWidget->GetRenderedAttributeSummary()
				== MultiSkillFixture.FullAttributeSummary);
	for (const TCHAR* Attribute : {
		TEXT("SHO"), TEXT("DRI"), TEXT("PAS"), TEXT("OFF"), TEXT("MRK"),
		TEXT("TKL"), TEXT("SPD"), TEXT("STR"), TEXT("STA"), TEXT("LS") })
	{
		TestTrue(FString::Printf(TEXT("Interaction mode retains %s"),
			Attribute),
			CardWidget->GetRenderedAttributeSummary().Contains(Attribute));
	}
	TestTrue(TEXT("Developer reference remains secondary interaction detail"),
		DeveloperReference != nullptr
			&& DeveloperReference->GetVisibility()
				== ESlateVisibility::Visible);

	FFMCodexUMGCardViewModel GoalkeeperFixture = MultiSkillFixture;
	GoalkeeperFixture.CardId = TEXT("VisualGK");
	GoalkeeperFixture.IdentityLabel = TEXT("Card VisualGK");
	GoalkeeperFixture.RoleLabel = TEXT("GK");
	GoalkeeperFixture.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperFixture.FullAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 3 | 1v1 2");
	GoalkeeperFixture.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	GoalkeeperFixture.StatusSummaryLabel = TEXT("GK USED | GK ACTIVE");
	GoalkeeperFixture.bGoalkeeper = true;
	CardWidget->RefreshFromPresentation(
		GoalkeeperFixture,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("GK is a distinct presentation-only card variant"),
		CardWidget->IsGoalkeeperVisualVariant()
			&& CardWidget->GetPresentation().RoleLabel == TEXT("GK")
			&& CardWidget->GetRenderedAttributeCount() == 6
			&& CardWidget->GetRenderedStatusBadgeCount() == 2
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("HAN"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("POS"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("REF"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("AER"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("ANT"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("1v1"))
			&& !CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SHO")));

	FFMCodexLocalMatchCardView MissingCardView;
	const FFMCodexUMGCardViewModel MissingFixture =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(MissingCardView);
	CardWidget->RefreshFromPresentation(
		MissingFixture,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("Missing presentation data has safe bounded fallbacks"),
		MissingFixture.IdentityLabel == TEXT("UNKNOWN CARD")
			&& MissingFixture.RoleLabel == TEXT("ROLE N/A")
			&& MissingFixture.SkillSummaryLabel == TEXT("NO SKILL")
			&& MissingFixture.CompactAttributeSummary
				== TEXT("Attributes unavailable")
			&& MissingFixture.FullAttributeSummary
				== TEXT("Attributes unavailable")
			&& MissingFixture.StatusSummaryLabel == TEXT("UNAVAILABLE")
			&& MissingFixture.RarityLabel == TEXT("RARITY N/A")
			&& CardWidget->GetRenderedSkillCount() == 1
			&& CardWidget->GetRenderedAttributeCount() == 1
			&& CardWidget->GetRenderedStatusBadgeCount() == 1);

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("Card stage preserves full-screen hot-seat blocking"),
		Controller->IsAwaitingHotSeatHandoff()
			&& Screen->GetPresentation().Handoff.bVisible);
	if (!Controller->IsAwaitingHotSeatHandoff())
	{
		return false;
	}
	const TArray<uint8> StateBeforeReady =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestReady();
	TestTrue(TEXT("Card-stage Ready remains authority-neutral"),
		StateBeforeReady == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& !Screen->GetPresentation().Handoff.bVisible);
	Screen->RequestBeginOrdinaryAttack();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	TestTrue(TEXT("Card fixture reaches authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy);

	const TArray<FFMCodexUMGCardViewModel>& CandidateDTOs =
		Screen->GetPresentation().Interaction.CandidateCards;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>& CandidateWidgets =
		Screen->GetRenderedCandidateCardWidgets();
	TestEqual(TEXT("Every candidate DTO receives one Card Widget"),
		CandidateWidgets.Num(), CandidateDTOs.Num());
	bool bCandidateBindingsExact = CandidateWidgets.Num() == CandidateDTOs.Num();
	TSet<FString> CandidateSkillFamilies;
	for (int32 Index = 0; Index < CandidateWidgets.Num(); ++Index)
	{
		const UFMCodexPlayerCardWidget* CandidateWidget = CandidateWidgets[Index];
		bCandidateBindingsExact = bCandidateBindingsExact
			&& CandidateWidget != nullptr
			&& CandidateWidget->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& CandidateWidget->GetPresentation().CardId
				== CandidateDTOs[Index].CardId
			&& CandidateWidget->GetPresentation().IdentityLabel
				== CandidateDTOs[Index].IdentityLabel;
		for (const FString& Skill : CandidateDTOs[Index].SkillLabels)
		{
			CandidateSkillFamilies.Add(Skill);
		}
	}
	TestTrue(TEXT("Interaction candidates use shared DTO in Interaction mode"),
		bCandidateBindingsExact);
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("Card Widget supports %s family"), Family),
			CandidateSkillFamilies.Contains(Family));
	}

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("Card refresh fixture has ordinary candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FName CardId = OrdinaryGroup->CardId;
	const FName SlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const FFMCodexUMGCardViewModel* CandidateBeforeDeployment =
		CandidateDTOs.FindByPredicate(
			[CardId](const FFMCodexUMGCardViewModel& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	TestNotNull(TEXT("Selected candidate has shared UMG Card DTO"),
		CandidateBeforeDeployment);
	if (CandidateBeforeDeployment == nullptr)
	{
		return false;
	}
	const FFMCodexUMGCardViewModel CandidateSnapshot =
		*CandidateBeforeDeployment;
	Screen->RequestDeployOrdinary(CardId, SlotId);
	TestTrue(TEXT("Successful command refreshes card from authority"),
		Controller->GetLastDiagnostic().bHostSuccess);

	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Card stage retains dedicated Pitch Widget"), Pitch);
	if (Pitch == nullptr)
	{
		return false;
	}
	auto FindRenderedSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName ExpectedSlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == ExpectedSlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};
	UFMCodexPitchSlotWidget* OccupiedSlot = FindRenderedSlot(*Pitch, SlotId);
	TestNotNull(TEXT("Successful refresh keeps deployed card in exact slot"),
		OccupiedSlot);
	if (OccupiedSlot == nullptr)
	{
		return false;
	}
	OccupiedSlot->TakeWidget();
	UFMCodexPlayerCardWidget* PitchCard = OccupiedSlot->GetCardWidget();
	TestTrue(TEXT("PitchSlot reuses Card Widget in PitchCompact mode"),
		Pitch->GetRenderedSlotWidgets().Num() == 10
			&& PitchCard != nullptr
			&& PitchCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchCompact);
	if (PitchCard == nullptr)
	{
		return false;
	}
	const FFMCodexUMGCardViewModel DeployedSnapshot =
		PitchCard->GetPresentation();
	TestTrue(TEXT("Pitch and candidate retain one presentation identity"),
		DeployedSnapshot.CardId == CandidateSnapshot.CardId
			&& DeployedSnapshot.IdentityLabel == CandidateSnapshot.IdentityLabel
			&& DeployedSnapshot.OwnerLabel == CandidateSnapshot.OwnerLabel
			&& DeployedSnapshot.RoleLabel == CandidateSnapshot.RoleLabel
			&& DeployedSnapshot.RarityLabel == CandidateSnapshot.RarityLabel
			&& DeployedSnapshot.SkillLabels == CandidateSnapshot.SkillLabels
			&& DeployedSnapshot.CompactAttributeSummary
				== CandidateSnapshot.CompactAttributeSummary
			&& DeployedSnapshot.FullAttributeSummary
				== CandidateSnapshot.FullAttributeSummary
			&& DeployedSnapshot.StatusLabels.Contains(TEXT("DEPLOYED")));

	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(CardId, SlotId);
	TestFalse(TEXT("Repeated deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	UFMCodexPitchSlotWidget* RejectedSlot = FindRenderedSlot(*Pitch, SlotId);
	if (RejectedSlot != nullptr)
	{
		RejectedSlot->TakeWidget();
	}
	const UFMCodexPlayerCardWidget* RejectedCard = RejectedSlot != nullptr
		? RejectedSlot->GetCardWidget() : nullptr;
	TestTrue(TEXT("Rejected command has no optimistic card/status mutation"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& RejectedSlot != nullptr
			&& RejectedCard != nullptr
			&& RejectedSlot->GetPresentation().SlotId == SlotId
			&& RejectedCard->GetPresentation().CardId
				== DeployedSnapshot.CardId
			&& RejectedCard->GetPresentation().IdentityLabel
				== DeployedSnapshot.IdentityLabel
			&& RejectedCard->GetPresentation().RoleLabel
				== DeployedSnapshot.RoleLabel
			&& RejectedCard->GetPresentation().SkillLabels
				== DeployedSnapshot.SkillLabels
			&& RejectedCard->GetPresentation().CompactAttributeSummary
				== DeployedSnapshot.CompactAttributeSummary
			&& RejectedCard->GetPresentation().FullAttributeSummary
				== DeployedSnapshot.FullAttributeSummary
			&& RejectedCard->GetPresentation().StatusLabels
				== DeployedSnapshot.StatusLabels);

	FString CardHeader;
	FString CardSource;
	FString DTOHeader;
	FString RootSource;
	FString InteractionPanelSource;
	FString SlotSource;
	FString ControllerSource;
	TestTrue(TEXT("Card visual boundary production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.h"),
			CardHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h"),
				DTOHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionPanelSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				SlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString CardWidgetSources = CardHeader + CardSource;
	TestTrue(TEXT("Card Widget consumes reflected presentation DTO only"),
		CardHeader.Contains(TEXT("FFMCodexUMGCardViewModel"))
			&& DTOHeader.Contains(TEXT("BlueprintType"))
			&& !CardWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !CardWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !CardWidgetSources.Contains(TEXT("D6Provider"))
			&& !CardWidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Card Widget contains zero gameplay calculation or input"),
		!CardWidgetSources.Contains(TEXT("Legality"))
			&& !CardWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !CardWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !CardWidgetSources.Contains(TEXT("RandRange"))
			&& !CardWidgetSources.Contains(TEXT("UButton"))
			&& !CardWidgetSources.Contains(TEXT("OnClicked"))
			&& !CardWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Card modes affect presentation density only"),
		CardHeader.Contains(TEXT("PitchCompact"))
			&& CardHeader.Contains(TEXT("InteractionChoice"))
			&& CardSource.Contains(TEXT("SetWidthOverride"))
			&& CardSource.Contains(TEXT("FullAttributeSummary"))
			&& CardSource.Contains(TEXT("CompactAttributeSummary")));
	TestTrue(TEXT("Pitch and interaction select one shared configurable card"),
		SlotSource.Contains(TEXT("PlayerCardWidgetClass"))
			&& SlotSource.Contains(TEXT("PitchCompact"))
			&& RootSource.Contains(TEXT("InteractionPanelWidgetClass"))
			&& InteractionPanelSource.Contains(TEXT("PlayerCardWidgetClass"))
			&& InteractionPanelSource.Contains(TEXT("InteractionChoice")));
	TestTrue(TEXT("Controller remains unaware of card visual geometry"),
		!ControllerSource.Contains(TEXT("PlayerCardBounds"))
			&& !ControllerSource.Contains(TEXT("PortraitPresentationRegion"))
			&& !ControllerSource.Contains(TEXT("StructuredAttributeGrid")));
	TestTrue(TEXT("Root and Pitch semantics remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerBPhysicalHalf")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("CenterFieldVisualSeparator")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerAPhysicalHalf")) != nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGInteractionPanelVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.31.UMGInteractionPanelVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGInteractionPanelVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Interaction foundation Host exists"), Host);
	TestNotNull(TEXT("Interaction foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Interaction foundation root screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	TestNotNull(TEXT("Root composes dedicated Interaction Panel"), Panel);
	if (Panel == nullptr)
	{
		return false;
	}
	Panel->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("InteractionPanelBounds"), TEXT("InteractionActionHeader"),
		TEXT("InteractionCandidateScroll"), TEXT("InteractionChoiceRegion"),
		TEXT("InteractionSecondaryActions"), TEXT("InteractionPrimaryActions"),
		TEXT("InteractionBoundedFallback") })
	{
		TestNotNull(FString::Printf(TEXT("Panel hierarchy contains %s"), Region),
			Panel->GetWidgetFromName(Region));
	}

	FFMCodexUMGCardViewModel Card;
	Card.CardId = TEXT("Panel.Card.Cross");
	Card.IdentityLabel = TEXT("Panel Candidate");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW");
	Card.SkillLabels = { TEXT("Cross") };
	Card.SkillSummaryLabel = TEXT("Cross");
	Card.CompactAttributeSummary = TEXT("SHO 4 | PAS 5 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 4 | DRI 3 | PAS 5 | OFF 4 | MRK 1 | TKL 1 | SPD 4 | STR 2 | STA 3 | LS 2");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");
	Card.RarityLabel = TEXT("Club");

	auto BasePresentation = [](
		const EFMCodexUMGInteractionCategory Category,
		const TCHAR* Title)
	{
		FFMCodexUMGInteractionViewModel Result;
		Result.Category = Category;
		Result.KickerLabel = TEXT("PLAYER ACTION");
		Result.ExpectedActorLabel = TEXT("Player A");
		Result.TitleLabel = Title;
		Result.CategoryLabel = Title;
		return Result;
	};
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};

	FFMCodexUMGInteractionViewModel Start = BasePresentation(
		EFMCodexUMGInteractionCategory::StartMatch, TEXT("START LOCAL MATCH"));
	Start.bCanStartNewMatch = true;
	Start.PrimaryActionLabel = TEXT("START LOCAL MATCH");
	Panel->RefreshFromPresentation(Start);
	TestTrue(TEXT("StartMatch exposes one readable primary action"),
		IsVisible(Panel->GetWidgetFromName(TEXT("InteractionStartMatchButton")))
			&& Panel->GetPresentation().TitleLabel == TEXT("START LOCAL MATCH"));

	FFMCodexUMGInteractionViewModel Begin = BasePresentation(
		EFMCodexUMGInteractionCategory::BeginAttack, TEXT("BEGIN ATTACK"));
	Begin.bCanBeginOrdinaryAttack = true;
	Begin.PrimaryActionLabel = TEXT("BEGIN ATTACK");
	Begin.ActionPointLabel = TEXT("Action Point: 6");
	Panel->RefreshFromPresentation(Begin);
	TestTrue(TEXT("BeginAttack exposes AP presentation without gameplay rules"),
		IsVisible(Panel->GetWidgetFromName(TEXT("InteractionBeginAttackButton")))
			&& Panel->GetPresentation().ActionPointLabel.Contains(TEXT("6")));

	FFMCodexUMGInteractionViewModel Deploy = BasePresentation(
		EFMCodexUMGInteractionCategory::Deploy, TEXT("DEPLOYMENT"));
	Deploy.bCanFinishDeployment = true;
	Deploy.PrimaryActionLabel = TEXT("FINISH DEPLOYMENT");
	FFMCodexUMGDeploymentChoiceViewModel Ordinary;
	Ordinary.CardId = Card.CardId;
	Ordinary.Card = Card;
	Ordinary.Destinations.Add({ TEXT("Slot.A"), TEXT("Slot A") });
	Ordinary.Destinations.Add({ TEXT("Slot.B"), TEXT("Slot B") });
	FFMCodexUMGDeploymentChoiceViewModel Goalkeeper = Ordinary;
	Goalkeeper.CardId = TEXT("Panel.Card.GK");
	Goalkeeper.Card.CardId = Goalkeeper.CardId;
	Goalkeeper.Card.IdentityLabel = TEXT("Panel Goalkeeper");
	Goalkeeper.Card.RoleLabel = TEXT("GK");
	Goalkeeper.Card.bGoalkeeper = true;
	Goalkeeper.bGoalkeeper = true;
	Goalkeeper.Destinations = { { TEXT("Slot.GK"), TEXT("Goalkeeper Slot") } };
	Deploy.DeploymentChoices = { Ordinary, Goalkeeper };
	Panel->RefreshFromPresentation(Deploy);
	TestTrue(TEXT("Deployment presents two draggable hand cards without buttons"),
		Panel->GetRenderedCandidateCardWidgets().Num() == 2
			&& Panel->GetRenderedOptionWidgets().IsEmpty()
			&& Panel->GetRenderedCandidateCardWidgets()[0]->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& Panel->GetRenderedCandidateCardWidgets()[0]
				->IsDeploymentDragEnabled()
			&& Panel->GetRenderedCandidateCardWidgets()[0]
				->GetDeploymentDragCardId() == Ordinary.CardId
			&& Panel->GetRenderedCandidateCardWidgets()[1]
				->IsGoalkeeperVisualVariant()
			&& Panel->GetRenderedCandidateCardWidgets()[1]
				->IsDeploymentDragGoalkeeper()
			&& Panel->GetRenderedCandidateCardWidgets()[1]
				->GetDeploymentDragCardId() == Goalkeeper.CardId
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("DeploymentHandInstruction")))
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionFinishDeploymentButton"))));

	const TArray<EFMCodexUMGInteractionCategory> SelectionCategories = {
		EFMCodexUMGInteractionCategory::SelectCarrier,
		EFMCodexUMGInteractionCategory::SelectMarker,
		EFMCodexUMGInteractionCategory::SelectSkill,
		EFMCodexUMGInteractionCategory::SelectRunner,
		EFMCodexUMGInteractionCategory::SelectHelper
	};
	for (const EFMCodexUMGInteractionCategory Category : SelectionCategories)
	{
		FFMCodexUMGInteractionViewModel Selection = BasePresentation(
			Category, TEXT("SELECT LEGAL OPTION"));
		FFMCodexUMGSelectionChoiceViewModel Choice;
		Choice.OptionId = Category == EFMCodexUMGInteractionCategory::SelectSkill
			? FName(TEXT("Demo.Skill.Cross")) : Card.CardId;
		Choice.Label = Category == EFMCodexUMGInteractionCategory::SelectSkill
			? TEXT("Cross") : TEXT("Select Panel Candidate");
		Choice.bHasCard = Category != EFMCodexUMGInteractionCategory::SelectSkill;
		Choice.Card = Card;
		Selection.SelectionChoices = { Choice, Choice };
		Selection.SelectionChoices[1].OptionId = TEXT("Panel.SecondOption");
		Selection.bCanDecline = Category
			!= EFMCodexUMGInteractionCategory::SelectCarrier;
		Selection.bCanResolveNoLegal = true;
		Selection.DeclineActionLabel = TEXT("DECLINE");
		Selection.NoLegalActionLabel = TEXT("RESOLVE NO LEGAL");
		Panel->RefreshFromPresentation(Selection);
		TestTrue(FString::Printf(TEXT("Category %d preserves all supplied options"),
			static_cast<int32>(Category)),
			Panel->GetRenderedOptionWidgets().Num() == 2
				&& Panel->GetRenderedCandidateCardWidgets().Num()
					== (Choice.bHasCard ? 2 : 0)
				&& IsVisible(Panel->GetWidgetFromName(
					TEXT("InteractionNoLegalButton"))));
	}
	FFMCodexUMGInteractionViewModel DistinctFallbacks = BasePresentation(
		EFMCodexUMGInteractionCategory::SelectMarker, TEXT("SELECT MARKER"));
	DistinctFallbacks.bCanDecline = true;
	DistinctFallbacks.bCanResolveNoLegal = true;
	DistinctFallbacks.DeclineActionLabel = TEXT("DECLINE MARKER");
	DistinctFallbacks.NoLegalActionLabel = TEXT("NO LEGAL MARKER - RESOLVE");
	Panel->RefreshFromPresentation(DistinctFallbacks);
	const UButton* DeclineButton = Cast<UButton>(
		Panel->GetWidgetFromName(TEXT("InteractionDeclineButton")));
	const UButton* NoLegalButton = Cast<UButton>(
		Panel->GetWidgetFromName(TEXT("InteractionNoLegalButton")));
	TestTrue(TEXT("Decline and NoLegal remain distinct secondary actions"),
		IsVisible(DeclineButton) && IsVisible(NoLegalButton)
			&& Cast<UTextBlock>(DeclineButton->GetChildAt(0))->GetText().ToString()
				!= Cast<UTextBlock>(NoLegalButton->GetChildAt(0))->GetText().ToString());

	auto VerifyBranch = [this, Panel, &BasePresentation](
		const FString& Section,
		const EFMCodexUMGBranchIntent First,
		const EFMCodexUMGBranchIntent Second)
	{
		FFMCodexUMGInteractionViewModel Branch = BasePresentation(
			EFMCodexUMGInteractionCategory::SelectBranchIntent,
			TEXT("CHOOSE ROUTE"));
		Branch.BranchSectionLabel = Section;
		Branch.BranchChoices = {
			{ First, First == EFMCodexUMGBranchIntent::CrossHigh
				? TEXT("HIGH") : TEXT("DIRECT SHOT") },
			{ Second, Second == EFMCodexUMGBranchIntent::CrossLow
				? TEXT("LOW") : TEXT("DEAD CORNER") }
		};
		Panel->RefreshFromPresentation(Branch);
		TestTrue(FString::Printf(TEXT("%s renders two distinct branch intents"),
			*Section), Panel->GetRenderedOptionWidgets().Num() == 2
			&& Panel->GetPresentation().BranchSectionLabel == Section);
	};
	VerifyBranch(TEXT("SHOT TYPE"), EFMCodexUMGBranchIntent::DirectShot,
		EFMCodexUMGBranchIntent::DeadCorner);
	VerifyBranch(TEXT("CROSS TYPE"), EFMCodexUMGBranchIntent::CrossHigh,
		EFMCodexUMGBranchIntent::CrossLow);

	FFMCodexUMGInteractionViewModel OneOnOne = BasePresentation(
		EFMCodexUMGInteractionCategory::SelectOneOnOneShot,
		TEXT("ONE-ON-ONE"));
	OneOnOne.BranchSectionLabel = TEXT("CHOOSE SHOT");
	OneOnOne.OneOnOneChoices = {
		{ EFMCodexUMGOneOnOneChoice::ChipShot, TEXT("CHIP SHOT") },
		{ EFMCodexUMGOneOnOneChoice::DirectShot, TEXT("DIRECT SHOT") }
	};
	Panel->RefreshFromPresentation(OneOnOne);
	TestEqual(TEXT("OneOnOne renders ChipShot and DirectShot only"),
		Panel->GetRenderedOptionWidgets().Num(), 2);

	FFMCodexUMGInteractionViewModel Continue = BasePresentation(
		EFMCodexUMGInteractionCategory::ContinueResolution,
		TEXT("Resolve Route"));
	Continue.KickerLabel = TEXT("SYSTEM RESOLUTION");
	Continue.bSystemResolution = true;
	Continue.bCanContinue = true;
	Continue.PrimaryActionLabel = TEXT("CONTINUE");
	Panel->RefreshFromPresentation(Continue);
	TestTrue(TEXT("System resolution presents advancing rather than choosing"),
		Panel->GetPresentation().bSystemResolution
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionContinueButton"))));

	FFMCodexUMGInteractionViewModel Ended = BasePresentation(
		EFMCodexUMGInteractionCategory::MatchEnded, TEXT("MATCH COMPLETE"));
	Ended.bMatchEnded = true;
	Ended.EmptyStateLabel = TEXT("Match complete. No action required.");
	Panel->RefreshFromPresentation(Ended);
	TestTrue(TEXT("MatchEnded contains no gameplay progression control"),
		Panel->GetRenderedOptionWidgets().IsEmpty()
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionStartMatchButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionBeginAttackButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionFinishDeploymentButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionContinueButton"))));

	// Real player-facing flow: Panel -> root -> Controller -> authoritative Host.
	Controller->RefreshPresentation();
	Panel = Screen->GetInteractionPanel();
	TestEqual(TEXT("Real panel starts in StartMatch"),
		Panel->GetPresentation().Category,
		EFMCodexUMGInteractionCategory::StartMatch);
	Panel->RequestStartMatch();
	TestTrue(TEXT("Panel StartMatch reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->IsAwaitingHotSeatHandoff()
			&& Panel->IsInteractionBlocked());
	const TArray<uint8> StateWhileBlocked =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Panel->RequestBeginAttack();
	TestTrue(TEXT("Handoff blocks every Interaction Panel intent"),
		StateWhileBlocked == SerializeState(Host->GetMatchSnapshot().Snapshot));
	Screen->RequestReady();
	TestTrue(TEXT("Ready is presentation-only and unblocks panel"),
		StateWhileBlocked == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& !Panel->IsInteractionBlocked());
	Panel->RequestBeginAttack();
	TestTrue(TEXT("Panel BeginAttack reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const EInitialTurnOrderPlayer FlowAttacker =
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer FlowDefender =
		FMCodexLocalMatchFullFamilyTests::OtherSide(FlowAttacker);
	const FName CrossCarrierId =
		FMCodexLocalMatchFullFamilyTests::OutfieldCardId(FlowAttacker, 1);
	const FString FlowPhysicalForward =
		FlowAttacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");

	auto ReadyIfPending = [Controller, Screen]()
	{
		if (Controller->IsAwaitingHotSeatHandoff())
		{
			Screen->RequestReady();
		}
	};
	ReadyIfPending();
	int32 SuccessfulDeployments = 0;
	bool bRejectedIntentVerified = false;
	bool bCrossCarrierDeployed = false;
	bool bDefenderGoalkeeperDeployed = false;
	while (SuccessfulDeployments < 5)
	{
		const FFMCodexUMGInteractionViewModel& Deployment =
			Panel->GetPresentation();
		if (Deployment.Category != EFMCodexUMGInteractionCategory::Deploy
			|| Deployment.DeploymentChoices.IsEmpty())
		{
			AddError(TEXT("Real Panel flow lost deployment choices"));
			return false;
		}
		const EInitialTurnOrderPlayer CurrentDeploymentSide =
			Controller->GetInteractionView().CurrentLegalDeploymentSide;
		const bool bNeedDefenderGoalkeeper =
			CurrentDeploymentSide == FlowDefender
			&& !bDefenderGoalkeeperDeployed;
		const FFMCodexUMGDeploymentChoiceViewModel* Choice = nullptr;
		if (bNeedDefenderGoalkeeper)
		{
			Choice = Deployment.DeploymentChoices.FindByPredicate(
				[](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
				{
					return Candidate.bGoalkeeper
						&& !Candidate.Destinations.IsEmpty();
				});
		}
		else
		{
			Choice = Deployment.DeploymentChoices.FindByPredicate(
				[CurrentDeploymentSide, FlowAttacker, CrossCarrierId,
					bCrossCarrierDeployed, &FlowPhysicalForward](
					const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
				{
					if (Candidate.bGoalkeeper
						|| (CurrentDeploymentSide == FlowAttacker
							&& !bCrossCarrierDeployed
							&& Candidate.CardId != CrossCarrierId))
					{
						return false;
					}
					return Candidate.Destinations.ContainsByPredicate(
						[&FlowPhysicalForward](
							const FFMCodexUMGDeploymentDestinationViewModel& Destination)
						{
							return Destination.SlotId.ToString().Contains(
								FlowPhysicalForward);
						});
				});
		}
		if (Choice == nullptr)
		{
			AddError(TEXT("Real Panel flow found no required deployment"));
			return false;
		}
		const FName DeployedCardId = Choice->CardId;
		const bool bDeployedGoalkeeper = Choice->bGoalkeeper;
		const FFMCodexUMGDeploymentDestinationViewModel* Destination =
			bDeployedGoalkeeper ? &Choice->Destinations[0]
			: Choice->Destinations.FindByPredicate(
				[&FlowPhysicalForward](
					const FFMCodexUMGDeploymentDestinationViewModel& Candidate)
				{
					return Candidate.SlotId.ToString().Contains(
						FlowPhysicalForward);
				});
		if (Destination == nullptr)
		{
			return false;
		}
		const FName DeployedSlotId = Destination->SlotId;
		Panel->RequestDeployment(
			DeployedCardId, DeployedSlotId, bDeployedGoalkeeper);
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			AddError(TEXT("Real Panel ordinary deployment was rejected"));
			return false;
		}
		bCrossCarrierDeployed = bCrossCarrierDeployed
			|| DeployedCardId == CrossCarrierId;
		bDefenderGoalkeeperDeployed = bDefenderGoalkeeperDeployed
			|| bDeployedGoalkeeper;
		++SuccessfulDeployments;
		ReadyIfPending();
		if (!bRejectedIntentVerified)
		{
			const TArray<uint8> BeforeRejected =
				SerializeState(Host->GetMatchSnapshot().Snapshot);
			const EFMCodexUMGInteractionCategory BeforeCategory =
				Panel->GetPresentation().Category;
			const int32 BeforeChoiceCount =
				Panel->GetPresentation().DeploymentChoices.Num();
			Panel->RequestDeployment(
				DeployedCardId, DeployedSlotId, bDeployedGoalkeeper);
			bRejectedIntentVerified =
				!Controller->GetLastDiagnostic().bHostSuccess
				&& BeforeRejected
					== SerializeState(Host->GetMatchSnapshot().Snapshot)
				&& Panel->GetPresentation().Category == BeforeCategory
				&& Panel->GetPresentation().DeploymentChoices.Num()
					== BeforeChoiceCount;
		}
	}
	TestTrue(TEXT("Rejected Panel intent has no optimistic selection mutation"),
		bRejectedIntentVerified);
	for (int32 Side = 0; Side < 2; ++Side)
	{
		ReadyIfPending();
		Panel->RequestFinishDeployment();
		TestTrue(TEXT("Panel FinishDeployment reaches authoritative Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
	}
	ReadyIfPending();

	TestTrue(TEXT("Real Panel deployment includes Cross carrier and defender GK"),
		bCrossCarrierDeployed && bDefenderGoalkeeperDeployed);
	const FFMCodexUMGSelectionChoiceViewModel* Carrier =
		Panel->GetPresentation().SelectionChoices.FindByPredicate(
			[CrossCarrierId](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.OptionId == CrossCarrierId;
			});
	TestNotNull(TEXT("Real Panel exposes deployed Cross carrier"), Carrier);
	if (Carrier == nullptr)
	{
		return false;
	}
	Panel->RequestCarrier(Carrier->OptionId);
	TestTrue(TEXT("Panel Carrier reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	ReadyIfPending();
	if (!Panel->GetPresentation().SelectionChoices.IsEmpty())
	{
		Panel->RequestMarker(
			Panel->GetPresentation().SelectionChoices[0].OptionId);
	}
	else if (Panel->GetPresentation().bCanResolveNoLegal)
	{
		Panel->RequestNoLegal();
	}
	else
	{
		AddError(TEXT("Real Panel exposes neither Marker nor NoLegal Marker"));
		return false;
	}
	TestTrue(TEXT("Panel Marker/NoLegal Marker reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	ReadyIfPending();
	const FFMCodexUMGSelectionChoiceViewModel* CrossSkill =
		Panel->GetPresentation().SelectionChoices.FindByPredicate(
			[](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.Label.Contains(TEXT("Cross"));
			});
	TestNotNull(TEXT("Real Panel exposes only authoritative legal Skills"),
		CrossSkill);
	if (CrossSkill == nullptr)
	{
		return false;
	}
	Panel->RequestSkill(CrossSkill->OptionId);
	TestTrue(TEXT("Panel Skill reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	ReadyIfPending();
	for (int32 Guard = 0; Guard < 4
		&& (Panel->GetPresentation().Category
			== EFMCodexUMGInteractionCategory::SelectRunner
			|| Panel->GetPresentation().Category
				== EFMCodexUMGInteractionCategory::SelectHelper); ++Guard)
	{
		const EFMCodexUMGInteractionCategory Category =
			Panel->GetPresentation().Category;
		if (!Panel->GetPresentation().SelectionChoices.IsEmpty())
		{
			const FName Id =
				Panel->GetPresentation().SelectionChoices[0].OptionId;
			if (Category == EFMCodexUMGInteractionCategory::SelectRunner)
			{
				Panel->RequestRunner(Id);
			}
			else
			{
				Panel->RequestHelper(Id);
			}
		}
		else if (Panel->GetPresentation().bCanResolveNoLegal)
		{
			Panel->RequestNoLegal();
		}
		else
		{
			Panel->RequestDecline();
		}
		TestTrue(TEXT("Panel Runner/Helper route reaches Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
		ReadyIfPending();
	}
	TestTrue(TEXT("Representative Panel flow reaches Cross BranchIntent"),
		Panel->GetPresentation().Category
			== EFMCodexUMGInteractionCategory::SelectBranchIntent
			&& Panel->GetPresentation().BranchChoices.Num() == 2);
	if (Panel->GetPresentation().Category
		== EFMCodexUMGInteractionCategory::SelectBranchIntent)
	{
		Panel->RequestBranch(EFMCodexUMGBranchIntent::CrossHigh);
		TestTrue(TEXT("Panel Cross BranchIntent reaches authoritative Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
	}

	// Deterministic normal-demo OneOnOne proof, with the actual choice routed
	// through the dedicated Widget and existing typed Controller method.
	FScopedPlayableWorld OneOnOneWorld;
	AFMCodexLocalMatchHostGameMode* OneOnOneHost = OneOnOneWorld.GetHost();
	AFMCodexLocalMatchPlayerController* OneOnOneController =
		OneOnOneWorld.GetController();
	TestNotNull(TEXT("OneOnOne UMG Host exists"), OneOnOneHost);
	TestNotNull(TEXT("OneOnOne UMG Controller exists"), OneOnOneController);
	if (OneOnOneHost == nullptr || OneOnOneController == nullptr)
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
		for (FPlayerCardData& DemoCard : *Deck)
		{
			if (!DemoCard.bIsGoalkeeper)
			{
				DemoCard.AttackSkillIds = { ThroughBallSkillId };
			}
		}
	}
	FSkillRuleSnapshot ThroughBallRule;
	ThroughBallRule.SkillId = ThroughBallSkillId;
	ThroughBallRule.SkillType = ESkillRuleType::ThroughBall;
	ThroughBallRule.MinTriggerActionPoint = 2;
	ThroughBallRule.MaxTriggerActionPoint = 8;
	Demo.SkillRuleSet.SkillRules = { ThroughBallRule };
	const int32 OneOnOneSeed = FindSeedForRolls({ 3, 6, 1, 2 });
	if (OneOnOneSeed == INDEX_NONE
		|| !OneOnOneHost->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, OneOnOneSeed).bSuccess
		|| !OneOnOneHost->BeginOrdinaryAttack(6).bSuccess)
	{
		AddError(TEXT("Could not initialize deterministic OneOnOne UMG fixture"));
		return false;
	}
	const EInitialTurnOrderPlayer OneOnOneAttacker = OneOnOneHost
		->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const FString PhysicalForward =
		OneOnOneAttacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (!DeployNextOrdinary(
			*OneOnOneHost, Demo.SkillRuleSet, PhysicalForward))
		{
			AddError(TEXT("Could not deploy deterministic OneOnOne fixture"));
			return false;
		}
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FFMCodexLocalMatchInteractionView View =
			ViewFor(*OneOnOneHost, Demo.SkillRuleSet);
		if (!OneOnOneHost->FinishDeployment(
			View.AttackSequence, View.ExpectedActingPlayer).bSuccess)
		{
			return false;
		}
	}
	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectCarrier,
		EFMCodexLocalMatchInteractionCategory::SelectMarker,
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		if (!SubmitFirstHostSelection(
			*OneOnOneHost, Demo.SkillRuleSet, Category))
		{
			AddError(TEXT("Could not select deterministic OneOnOne fixture"));
			return false;
		}
	}
	if (!OneOnOneHost->BeginResolutionSession().bSuccess
		|| !OneOnOneHost->ResolveInitialRoute().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP1DecisionOrPlan().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP1Formula().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP2Decision().bSuccess)
	{
		AddError(TEXT("Could not reach deterministic OneOnOne decision"));
		return false;
	}
	OneOnOneController->RefreshPresentation();
	OneOnOneController->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* OneOnOneScreen =
		OneOnOneController->GetPlayerMatchScreen();
	if (OneOnOneScreen == nullptr)
	{
		return false;
	}
	OneOnOneScreen->TakeWidget();
	UFMCodexInteractionPanelWidget* OneOnOnePanel =
		OneOnOneScreen->GetInteractionPanel();
	TestTrue(TEXT("Authoritative OneOnOne renders both player choices"),
		OneOnOnePanel != nullptr
			&& OneOnOnePanel->GetPresentation().Category
				== EFMCodexUMGInteractionCategory::SelectOneOnOneShot
			&& OneOnOnePanel->GetPresentation().OneOnOneChoices.Num() == 2
			&& OneOnOnePanel->GetRenderedOptionWidgets().Num() == 2);
	if (OneOnOnePanel == nullptr)
	{
		return false;
	}
	if (OneOnOneController->IsAwaitingHotSeatHandoff())
	{
		OneOnOneScreen->RequestReady();
	}
	TestFalse(TEXT("OneOnOne player acknowledges handoff before choosing"),
		OneOnOnePanel->IsInteractionBlocked());
	OneOnOnePanel->RequestOneOnOne(EFMCodexUMGOneOnOneChoice::DirectShot);
	TestTrue(TEXT("OneOnOne Widget choice is accepted by Host"),
		OneOnOneController->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Accepted OneOnOne refresh changes interaction category"),
		OneOnOnePanel->GetPresentation().Category
			!= EFMCodexUMGInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("Accepted OneOnOne refresh removes choice controls"),
		OneOnOnePanel->GetPresentation().OneOnOneChoices.IsEmpty());

	FString PanelHeader;
	FString PanelSource;
	FString OptionHeader;
	FString OptionSource;
	FString RootHeader;
	FString RootSource;
	FString ControllerSource;
	TestTrue(TEXT("Interaction authority audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.h"),
			PanelHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				PanelSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.h"),
				OptionHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.cpp"),
				OptionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString InteractionWidgetSources =
		PanelHeader + PanelSource + OptionHeader + OptionSource;
	TestTrue(TEXT("Interaction widgets consume reflected DTOs only"),
		PanelHeader.Contains(TEXT("FFMCodexUMGInteractionViewModel"))
			&& !InteractionWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !InteractionWidgetSources.Contains(TEXT("HostGameMode"))
			&& !InteractionWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !InteractionWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Interaction widgets contain no gameplay authority"),
		!InteractionWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !InteractionWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !InteractionWidgetSources.Contains(TEXT("RandRange"))
			&& !InteractionWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !InteractionWidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !InteractionWidgetSources.Contains(TEXT("ProcessEvent")));
	TestTrue(TEXT("Root delegates to panel and Controller owns typed commands"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexInteractionPanelWidget>"))
			&& RootSource.Contains(TEXT("DedicatedInteractionPanelWidget"))
			&& RootSource.Contains(TEXT("RequestSubmitCarrier"))
			&& RootSource.Contains(TEXT("RequestContinueResolution"))
			&& ControllerSource.Contains(TEXT("SubmitCarrier"))
			&& ControllerSource.Contains(TEXT("ContinueResolution")));
	TestTrue(TEXT("Stage 6.4 root regions remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) != nullptr);
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGResolutionVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.32.UMGResolutionVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGResolutionVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Resolution foundation Host exists"), Host);
	TestNotNull(TEXT("Resolution foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Resolution foundation root UMG screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexResolutionPanelWidget* RootResolution =
		Screen->GetResolutionPanel();
	TestNotNull(TEXT("Root composes dedicated Resolution Panel"), RootResolution);
	if (RootResolution == nullptr)
	{
		return false;
	}
	RootResolution->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("ResolutionPanelBounds"), TEXT("ResolutionStepRegion"),
		TEXT("ResolutionDiceRegion"), TEXT("ResolutionComparisonRegion"),
		TEXT("ResolutionDecisionRegion"),
		TEXT("ResolutionContinuationRegion"),
		TEXT("ResolutionTerminalRegion"), TEXT("ResolutionRejectedRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Resolution hierarchy contains %s"),
			Region), RootResolution->GetWidgetFromName(Region));
	}
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};

	FFMCodexUMGResolutionViewModel Empty;
	RootResolution->RefreshFromPresentation(Empty);
	TestTrue(TEXT("No-feedback state is bounded and non-diagnostic"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionEmptyState")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy"))));

	FFMCodexUMGResolutionViewModel Rich;
	Rich.bVisible = true;
	Rich.StepLabel = TEXT("One-on-One - Direct Shot");
	Rich.StepSummaryLabel = TEXT("Direct Shot Formula resolved");
	Rich.RouteLabel = TEXT("Through Ball -> Behind Defense");
	Rich.DiceResults = {
		{ TEXT("ROUTE"), TEXT("Initial Route"), 6 },
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Direct Shot Attack"), 4 },
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Direct Shot Defense"), 2 }
	};
	Rich.ComparisonEvidence = {
		{ TEXT("ATTACK"),
			TEXT("Shooter: Shooting 5 | modifier +1.0 | D6 4 | final 10.0") },
		{ TEXT("DEFENSE"),
			TEXT("Goalkeeper: OneOnOne 5 | activation active | authoritative modifier +2.5 | D6 2 | final 9.5") },
		{ TEXT("EVIDENCE"),
			TEXT("Goalkeeper participated in the authoritative Formula") }
	};
	Rich.DecisionLabel =
		TEXT("Winner: Attacker | Reason: Higher final value");
	Rich.ContinuationLabel = TEXT("Continue: Apply terminal result");
	Rich.TerminalLabel = TEXT("RESULT: GOAL");
	Rich.bTerminal = true;
	RootResolution->RefreshFromPresentation(Rich);
	TestEqual(TEXT("Every authoritative die receives one Dice Widget"),
		RootResolution->GetRenderedDiceWidgets().Num(), 3);
	bool bExactDiceBinding = true;
	for (int32 Index = 0;
		Index < RootResolution->GetRenderedDiceWidgets().Num(); ++Index)
	{
		const UFMCodexDiceResultWidget* Die =
			RootResolution->GetRenderedDiceWidgets()[Index];
		bExactDiceBinding = bExactDiceBinding && Die != nullptr
			&& Die->GetDisplayedRawD6() == Rich.DiceResults[Index].RawD6
			&& Die->GetPresentation().PurposeLabel
				== Rich.DiceResults[Index].PurposeLabel;
	}
	TestTrue(TEXT("Dice Widgets bind exact accepted RawD6 and Purpose"),
		bExactDiceBinding);
	TestTrue(TEXT("Formula evidence renders ATTACK VS DEFENSE without math"),
		RootResolution->GetRenderedComparisonCount() == 3
			&& RootResolution->GetWidgetFromName(
				TEXT("ResolutionVersusLabel")) != nullptr
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ComparisonEvidenceValue0")))->GetText().ToString()
					== Rich.ComparisonEvidence[0].EvidenceLabel
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ComparisonEvidenceValue1")))->GetText().ToString()
					== Rich.ComparisonEvidence[1].EvidenceLabel);
	TestTrue(TEXT("Winner/reason and GK evidence remain authoritative text"),
		Cast<UTextBlock>(RootResolution->GetWidgetFromName(
			TEXT("ResolutionDecisionSectionValue")))->GetText().ToString()
				== Rich.DecisionLabel
			&& Rich.ComparisonEvidence[1].EvidenceLabel.Contains(
				TEXT("Goalkeeper"))
			&& Rich.ComparisonEvidence[1].EvidenceLabel.Contains(TEXT("final 9.5")));
	TestTrue(TEXT("Terminal result is a distinct prominent section"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionTerminalRegion")))
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalSectionValue")))->GetText().ToString()
					== TEXT("RESULT: GOAL"));

	FFMCodexUMGResolutionViewModel Chip;
	Chip.bVisible = true;
	Chip.StepLabel = TEXT("One-on-One - Chip Shot");
	Chip.StepSummaryLabel = TEXT("Chip Shot decision resolved without Formula");
	Chip.DiceResults = {
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Chip Shot Attack"), 5 }
	};
	Chip.DecisionLabel = TEXT("Goal");
	RootResolution->RefreshFromPresentation(Chip);
	TestTrue(TEXT("ChipShot is a distinct non-Formula presentation"),
		RootResolution->GetRenderedDiceWidgets().Num() == 1
			&& RootResolution->GetRenderedComparisonCount() == 0
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionComparisonRegion")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion"))));

	const TArray<TPair<FString, FString>> ReadableShapes = {
		{ TEXT("Cross - High Cross"), TEXT("Cross -> High Cross") },
		{ TEXT("Pass Control"), TEXT("Pass Control -> Pass Advance") },
		{ TEXT("Long Shot - Direct Shot"), TEXT("Long Shot -> Direct Shot") },
		{ TEXT("Cut Inside - Dead Corner"), TEXT("Cut Inside -> Dead Corner") },
		{ TEXT("Through Ball - Behind Defense P1"),
			TEXT("Through Ball -> Behind Defense") },
		{ TEXT("Through Ball - Behind Defense P2"),
			TEXT("Through Ball -> Behind Defense") },
		{ TEXT("Through Ball - Anti-Offside"),
			TEXT("Through Ball -> Anti-Offside") }
	};
	for (const TPair<FString, FString>& Shape : ReadableShapes)
	{
		FFMCodexUMGResolutionViewModel ShapeDTO;
		ShapeDTO.bVisible = true;
		ShapeDTO.StepLabel = Shape.Key;
		ShapeDTO.RouteLabel = Shape.Value;
		ShapeDTO.DecisionLabel = TEXT("Authoritative decision");
		RootResolution->RefreshFromPresentation(ShapeDTO);
		TestTrue(FString::Printf(TEXT("%s shape renders without bespoke rules"),
			*Shape.Key),
			Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionStepTitle")))->GetText().ToString() == Shape.Key
				&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
					TEXT("ResolutionRouteSummary")))->GetText().ToString()
						== Shape.Value);
	}

	for (const TCHAR* TerminalSemantic : {
		TEXT("RESULT: GOAL"), TEXT("RESULT: MISS"), TEXT("RESULT: NO GOAL"),
		TEXT("RESULT: IMMEDIATE MISS"), TEXT("RESULT: OFFSIDE"),
		TEXT("RESULT: OUT OF PLAY"),
		TEXT("RESULT: DEFENDER STOPPED ATTACK") })
	{
		FFMCodexUMGResolutionViewModel TerminalDTO;
		TerminalDTO.bVisible = true;
		TerminalDTO.bTerminal = true;
		TerminalDTO.StepLabel = TEXT("Attack Completed");
		TerminalDTO.TerminalLabel = TerminalSemantic;
		TerminalDTO.ContinuationLabel =
			TEXT("Attack complete | Score: Player A 1 - 0 Player B | Next attacker: Player B | Opportunity consumed: yes");
		RootResolution->RefreshFromPresentation(TerminalDTO);
		TestTrue(FString::Printf(TEXT("%s remains readable"), TerminalSemantic),
			Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalSectionValue")))->GetText().ToString()
					== TerminalSemantic
				&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
					TEXT("ResolutionContinuationSectionValue")))
					->GetText().ToString().Contains(TEXT("Next attacker: Player B")));
	}

	FFMCodexUMGResolutionViewModel Rejected = Rich;
	Rejected.bRejected = true;
	Rejected.StepLabel = TEXT("Command Rejected");
	Rejected.DecisionLabel = TEXT("No resolution result was accepted");
	Rejected.ErrorLabel = TEXT("Readable rejection reason");
	RootResolution->RefreshFromPresentation(Rejected);
	TestTrue(TEXT("Rejected command cannot visually mix old accepted evidence"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionRejectedRegion")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy")))
			&& RootResolution->GetRenderedDiceWidgets().IsEmpty()
			&& RootResolution->GetRenderedComparisonCount() == 0
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion"))));

	// Normal-demo Cross: use the existing public demo configuration and the
	// Interaction Panel for resolution advancement while observing the new panel.
	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	TestNotNull(TEXT("Cross flow retains dedicated Interaction Panel"), Interaction);
	if (Interaction == nullptr)
	{
		return false;
	}
	Interaction->RequestStartMatch();
	TestTrue(TEXT("Cross UMG flow starts through Interaction Panel"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Screen->GetPresentation().Handoff.bVisible
			&& Screen->GetResolutionPanel() == RootResolution);
	const TArray<uint8> CrossStateBeforeReady =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestReady();
	TestTrue(TEXT("Handoff remains above Resolution and Ready is neutral"),
		CrossStateBeforeReady == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& !Screen->GetPresentation().Handoff.bVisible);
	Interaction->RequestBeginAttack();
	TestTrue(TEXT("Cross UMG flow begins through Interaction Panel"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const EInitialTurnOrderPlayer CrossAttacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FFamilyExpectation CrossFamily = FamilyExpectations()[0];
	if (!DeployParticipants(*this, *Controller, CrossFamily, CrossAttacker)
		|| !SubmitRequiredSelections(
			*this, *Controller, CrossFamily, CrossAttacker))
	{
		return false;
	}
	bool bSawCrossRouteDice = false;
	bool bSawCrossComparison = false;
	for (int32 Guard = 0;
		Guard < 12 && Controller->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		if (Controller->GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			AddError(TEXT("Cross resolution left ContinueResolution unexpectedly"));
			return false;
		}
		Interaction->RequestContinue();
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			AddError(TEXT("Cross UMG Continue was rejected"));
			return false;
		}
		const FFMCodexUMGResolutionViewModel& Resolution =
			RootResolution->GetPresentation();
		bSawCrossRouteDice = bSawCrossRouteDice
			|| (Resolution.RouteLabel.Contains(TEXT("Cross"))
				&& !Resolution.DiceResults.IsEmpty());
		bSawCrossComparison = bSawCrossComparison
			|| (Resolution.ComparisonEvidence.Num() >= 2
				&& Resolution.DecisionLabel.Contains(TEXT("Winner:")));
	}
	TestTrue(TEXT("Cross ResolutionPanel progresses route/dice/comparison"),
		bSawCrossRouteDice && bSawCrossComparison);
	TestTrue(TEXT("Cross terminal renders result and completion summary"),
		RootResolution->GetPresentation().bTerminal
			&& RootResolution->GetPresentation().TerminalLabel.StartsWith(
				TEXT("RESULT: "))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Attack complete"))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Score:"))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Next attacker:")));

	// Normal-demo deterministic ThroughBall: no test-only rules or deck mutation.
	FScopedPlayableWorld OneOnOneWorld;
	AFMCodexLocalMatchHostGameMode* OneOnOneHost = OneOnOneWorld.GetHost();
	AFMCodexLocalMatchPlayerController* OneOnOneController =
		OneOnOneWorld.GetController();
	if (OneOnOneHost == nullptr || OneOnOneController == nullptr)
	{
		return false;
	}
	OneOnOneController->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* OneOnOneScreen =
		OneOnOneController->GetPlayerMatchScreen();
	if (OneOnOneScreen == nullptr)
	{
		return false;
	}
	OneOnOneScreen->TakeWidget();
	UFMCodexInteractionPanelWidget* OneOnOneInteraction =
		OneOnOneScreen->GetInteractionPanel();
	UFMCodexResolutionPanelWidget* OneOnOneResolution =
		OneOnOneScreen->GetResolutionPanel();
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FFamilyExpectation ThroughBallFamily = FamilyExpectations()[4];
	const int32 ThroughBallSeed = FindSeedForRolls({ 3, 6, 1, 2 });
	if (ThroughBallSeed == INDEX_NONE
		|| !OneOnOneHost->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, ThroughBallSeed).bSuccess)
	{
		return false;
	}
	OneOnOneController->RefreshPresentation();
	AcknowledgeIfPending(*OneOnOneController);
	OneOnOneController->BeginDemoOrdinaryAttack();
	const EInitialTurnOrderPlayer ThroughBallAttacker =
		OneOnOneController->GetInteractionView().CurrentAttackingPlayer;
	if (!OneOnOneController->GetLastDiagnostic().bHostSuccess
		|| !DeployParticipants(
			*this, *OneOnOneController, ThroughBallFamily,
			ThroughBallAttacker)
		|| !SubmitRequiredSelections(
			*this, *OneOnOneController, ThroughBallFamily,
			ThroughBallAttacker))
	{
		return false;
	}
	bool bSawOneOnOneHandoff = false;
	bool bSawShooterGoalkeeperPlan = false;
	bool bSawAuthoritativeDirectFormula = false;
	for (int32 Guard = 0;
		Guard < 14 && OneOnOneController->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		const EFMCodexLocalMatchInteractionCategory Category =
			OneOnOneController->GetInteractionView().InteractionCategory;
		if (Category
			== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
		{
			bSawOneOnOneHandoff = true;
			TestTrue(TEXT("P2 feedback announces authoritative OneOnOne handoff"),
				OneOnOneResolution->GetPresentation().DecisionLabel.Contains(
					TEXT("One-on-One")));
			if (OneOnOneController->IsAwaitingHotSeatHandoff())
			{
				OneOnOneScreen->RequestReady();
			}
			OneOnOneInteraction->RequestOneOnOne(
				EFMCodexUMGOneOnOneChoice::DirectShot);
		}
		else if (Category
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			OneOnOneInteraction->RequestContinue();
		}
		else
		{
			AddError(TEXT("Normal-demo OneOnOne reached unexpected category"));
			return false;
		}
		if (!OneOnOneController->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
		const FFMCodexUMGResolutionViewModel& Resolution =
			OneOnOneResolution->GetPresentation();
		if (Resolution.ComparisonEvidence.Num() >= 2
			&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
				TEXT("Shooter"))
			&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
				TEXT("Goalkeeper")))
		{
			bSawShooterGoalkeeperPlan = bSawShooterGoalkeeperPlan
				|| (Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
					TEXT("Shooter"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("Goalkeeper")));
			bSawAuthoritativeDirectFormula = bSawAuthoritativeDirectFormula
				|| (Resolution.DecisionLabel.Contains(TEXT("Winner:"))
					&& Resolution.DecisionLabel.Contains(TEXT("Reason:"))
					&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
						TEXT("D6"))
					&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
						TEXT("final"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("D6"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("authoritative modifier"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("final")));
		}
	}
	TestTrue(TEXT("Normal demo reaches OneOnOne and shooter/GK plan"),
		bSawOneOnOneHandoff && bSawShooterGoalkeeperPlan);
	TestTrue(TEXT("DirectShot shows authoritative final values/winner/reason"),
		bSawAuthoritativeDirectFormula);
	TestTrue(TEXT("OneOnOne terminal remains Goal/Miss with completion"),
		OneOnOneResolution->GetPresentation().bTerminal
			&& (OneOnOneResolution->GetPresentation().TerminalLabel
					== TEXT("RESULT: GOAL")
				|| OneOnOneResolution->GetPresentation().TerminalLabel
					== TEXT("RESULT: MISS"))
			&& OneOnOneResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Attack complete")));

	FString ResolutionHeader;
	FString ResolutionSource;
	FString DiceHeader;
	FString DiceSource;
	FString RootHeader;
	FString RootSource;
	FString InteractionSource;
	FString ControllerSource;
	TestTrue(TEXT("Resolution authority audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.h"),
			ResolutionHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"),
				ResolutionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.h"),
				DiceHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.cpp"),
				DiceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString ResolutionWidgetSources =
		ResolutionHeader + ResolutionSource + DiceHeader + DiceSource;
	TestTrue(TEXT("Resolution Widgets receive reflected presentation only"),
		ResolutionHeader.Contains(TEXT("FFMCodexUMGResolutionViewModel"))
			&& DiceHeader.Contains(TEXT("FFMCodexUMGDiceResultViewModel"))
			&& !ResolutionWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !ResolutionWidgetSources.Contains(TEXT("HostGameMode"))
			&& !ResolutionWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !ResolutionWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Resolution Widgets contain no Formula/route/RNG authority"),
		!ResolutionWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !ResolutionWidgetSources.Contains(TEXT("CalculateGoalkeeperHalf"))
			&& !ResolutionWidgetSources.Contains(TEXT("EFormulaWinner"))
			&& !ResolutionWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !ResolutionWidgetSources.Contains(TEXT("RandomStream"))
			&& !ResolutionWidgetSources.Contains(TEXT("RandRange"))
			&& !ResolutionWidgetSources.Contains(TEXT("RollD6")));
	TestTrue(TEXT("Resolution Widgets own no commands or generic dispatcher"),
		!ResolutionWidgetSources.Contains(TEXT("UButton"))
			&& !ResolutionWidgetSources.Contains(TEXT("OnClicked"))
			&& !ResolutionWidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !ResolutionWidgetSources.Contains(TEXT("ProcessEvent"))
			&& InteractionSource.Contains(TEXT("RequestContinue")));
	TestTrue(TEXT("Root delegates result rendering to configurable panel"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexResolutionPanelWidget>"))
			&& RootSource.Contains(TEXT("DedicatedResolutionPanelWidget"))
			&& RootSource.Contains(TEXT("RefreshFromPresentation"))
			&& !RootSource.Contains(TEXT("ResolutionResultText")));
	TestTrue(TEXT("Stage 6.4-6.7 root regions remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) != nullptr);
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGMatchHeaderVisualRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.33.UMGMatchHeaderVisualRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGMatchHeaderVisualRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Header refinement Host exists"), Host);
	TestNotNull(TEXT("Header refinement Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexMatchHeaderWidget* Header = Screen->GetMatchHeader();
	TestNotNull(TEXT("Root composes dedicated Match Header"), Header);
	if (Header == nullptr)
	{
		return false;
	}
	Header->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("MatchHeaderBounds"), TEXT("MatchHeaderScoreboardRegion"),
		TEXT("PlayerACrestAssetHook"), TEXT("PlayerBCrestAssetHook"),
		TEXT("MatchHeaderCentralScoreRegion"),
		TEXT("MatchHeaderAttackerStatusRegion"),
		TEXT("MatchHeaderActorStatusRegion"),
		TEXT("MatchHeaderFinalResultRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Header hierarchy contains %s"), Region),
			Header->GetWidgetFromName(Region));
	}
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};
	auto BuildHeader = [](const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString(),
			false, FString()).Header;
	};

	const FFMCodexLocalMatchInteractionView PreMatch =
		FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
	const FFMCodexUMGMatchHeaderViewModel PreMatchDTO = BuildHeader(PreMatch);
	Header->RefreshFromPresentation(PreMatchDTO);
	TestTrue(TEXT("Pre-match Header is neutral and bounded"),
		!PreMatchDTO.bMatchActive && !PreMatchDTO.bMatchEnded
			&& PreMatchDTO.MatchStatusLabel == TEXT("READY TO PLAY")
			&& PreMatchDTO.ActorStatusLabel == TEXT("WAITING TO START")
			&& Header->GetDisplayedScoreLabel() == TEXT("0 - 0")
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("MatchHeaderAttackerStatusRegion")))
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("MatchHeaderFinalResultRegion"))));

	auto ActiveView = [](const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Actor,
		const bool bSystem)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.PlayerAScore = 2;
		View.PlayerBScore = 1;
		View.CurrentAttackingPlayer = Attacker;
		View.ExpectedActingPlayer = bSystem
			? EInitialTurnOrderPlayer::None : Actor;
		View.bHumanInteraction = !bSystem;
		View.InteractionCategory = bSystem
			? EFMCodexLocalMatchInteractionCategory::ContinueResolution
			: EFMCodexLocalMatchInteractionCategory::SelectMarker;
		return View;
	};
	const TArray<FFMCodexLocalMatchInteractionView> ActiveCases = {
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerA, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::None, true),
		ActiveView(EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerA, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerB, false)
	};
	const TArray<FString> ExpectedAttackers = {
		TEXT("PLAYER A ATTACKING"), TEXT("PLAYER A ATTACKING"),
		TEXT("PLAYER A ATTACKING"), TEXT("PLAYER B ATTACKING"),
		TEXT("PLAYER B ATTACKING")
	};
	const TArray<FString> ExpectedActors = {
		TEXT("PLAYER A TO ACT"), TEXT("PLAYER B TO ACT"),
		TEXT("SYSTEM RESOLUTION"), TEXT("PLAYER A TO ACT"),
		TEXT("PLAYER B TO ACT")
	};
	for (int32 Index = 0; Index < ActiveCases.Num(); ++Index)
	{
		const FFMCodexUMGMatchHeaderViewModel DTO = BuildHeader(ActiveCases[Index]);
		Header->RefreshFromPresentation(DTO);
		TestTrue(FString::Printf(TEXT("Attacker/actor case %d stays distinct"),
			Index), Header->GetDisplayedAttackerLabel() == ExpectedAttackers[Index]
			&& Header->GetDisplayedActorLabel() == ExpectedActors[Index]
			&& Header->GetDisplayedScoreLabel() == TEXT("2 - 1")
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("MatchHeaderAttackerStatusRegion")))
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("MatchHeaderActorStatusRegion"))));
	}

	FFMCodexUMGMatchHeaderViewModel DirectScore;
	DirectScore.PlayerALabel = TEXT("Player A");
	DirectScore.PlayerBLabel = TEXT("Player B");
	DirectScore.ScoreLabel = TEXT("7 - 3");
	DirectScore.PlayerAScoreLabel = TEXT("7");
	DirectScore.PlayerBScoreLabel = TEXT("3");
	DirectScore.MatchStatusLabel = TEXT("MATCH IN PROGRESS");
	Header->RefreshFromPresentation(DirectScore);
	TestTrue(TEXT("Scoreboard displays DTO values without score arithmetic"),
		Header->GetDisplayedScoreLabel() == TEXT("7 - 3")
			&& Cast<UTextBlock>(Header->GetWidgetFromName(
				TEXT("PlayerAScoreValue")))->GetText().ToString() == TEXT("7")
			&& Cast<UTextBlock>(Header->GetWidgetFromName(
				TEXT("PlayerBScoreValue")))->GetText().ToString() == TEXT("3"));

	const TArray<TPair<EMatchResultType, FString>> EndedCases = {
		{ EMatchResultType::HomeWin, TEXT("Player A Win") },
		{ EMatchResultType::AwayWin, TEXT("Player B Win") },
		{ EMatchResultType::Draw, TEXT("Draw") }
	};
	for (const TPair<EMatchResultType, FString>& EndedCase : EndedCases)
	{
		FFMCodexLocalMatchInteractionView EndedView = ActiveCases[0];
		EndedView.bMatchEnded = true;
		EndedView.bCurrentAttackActive = false;
		EndedView.MatchResult = EndedCase.Key;
		EndedView.PlayerAScore = EndedCase.Key == EMatchResultType::AwayWin ? 1 : 3;
		EndedView.PlayerBScore = EndedCase.Key == EMatchResultType::HomeWin ? 2 : 3;
		EndedView.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
		const FFMCodexUMGMatchHeaderViewModel EndedDTO = BuildHeader(EndedView);
		Header->RefreshFromPresentation(EndedDTO);
		TestTrue(FString::Printf(TEXT("Ended result %s is canonical"),
			*EndedCase.Value),
			EndedDTO.MatchStatusLabel == TEXT("MATCH ENDED")
				&& EndedDTO.MatchResultLabel == EndedCase.Value
				&& IsVisible(Header->GetWidgetFromName(
					TEXT("MatchHeaderFinalResultRegion")))
				&& !IsVisible(Header->GetWidgetFromName(
					TEXT("MatchHeaderAttackerStatusRegion")))
				&& !IsVisible(Header->GetWidgetFromName(
					TEXT("MatchHeaderActorStatusRegion"))));
	}

	// Real normal-demo Cross flow proves refresh, attacker/defender distinction,
	// system resolution, rejection atomicity and completion/next-attack state.
	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	if (Interaction == nullptr)
	{
		return false;
	}
	Controller->RefreshPresentation();
	Interaction->RequestStartMatch();
	TestTrue(TEXT("Real Header refreshes active match beneath handoff"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Screen->GetPresentation().Handoff.bVisible
			&& Header->GetPresentation().bMatchActive
			&& Header->GetPresentation().ActorStatusLabel.Contains(TEXT("TO ACT")));
	const FString HandoffPlayer =
		Screen->GetPresentation().Handoff.NextPlayerLabel;
	TestTrue(TEXT("Handoff and underlying Header actor remain consistent"),
		(HandoffPlayer.Contains(TEXT("Player A"))
				&& Header->GetDisplayedActorLabel().Contains(TEXT("PLAYER A")))
			|| (HandoffPlayer.Contains(TEXT("Player B"))
				&& Header->GetDisplayedActorLabel().Contains(TEXT("PLAYER B"))));
	const TArray<uint8> BeforeReady =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FFMCodexUMGMatchHeaderViewModel HeaderBeforeReady =
		Header->GetPresentation();
	Screen->RequestReady();
	TestTrue(TEXT("Ready changes neither authority nor Header snapshot values"),
		BeforeReady == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Header->GetPresentation().ScoreLabel
				== HeaderBeforeReady.ScoreLabel
			&& Header->GetPresentation().AttackerStatusLabel
				== HeaderBeforeReady.AttackerStatusLabel
			&& Header->GetPresentation().ActorStatusLabel
				== HeaderBeforeReady.ActorStatusLabel);
	Interaction->RequestBeginAttack();
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
	const FFamilyExpectation CrossFamily = FamilyExpectations()[0];
	if (!DeployParticipants(*this, *Controller, CrossFamily, Attacker))
	{
		return false;
	}
	const FName CarrierId = OutfieldCardId(Attacker, CrossFamily.FirstCardIndex);
	AcknowledgeIfPending(*Controller);
	TestTrue(TEXT("Carrier Header shows attacker acting"),
		Header->GetDisplayedAttackerLabel().Contains(
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("PLAYER A") : TEXT("PLAYER B"))
			&& Header->GetDisplayedActorLabel().Contains(
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));
	Interaction->RequestCarrier(CarrierId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	TestTrue(TEXT("Marker Header keeps attacker but shows defender acting"),
		Header->GetDisplayedAttackerLabel().Contains(
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("PLAYER A") : TEXT("PLAYER B"))
			&& Header->GetDisplayedActorLabel().Contains(
				Defender == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));
	if (Interaction->GetPresentation().SelectionChoices.IsEmpty())
	{
		return false;
	}
	Interaction->RequestMarker(
		Interaction->GetPresentation().SelectionChoices[0].OptionId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	const FFMCodexUMGSelectionChoiceViewModel* CrossSkill =
		Interaction->GetPresentation().SelectionChoices.FindByPredicate(
			[](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.Label.Contains(TEXT("Cross"));
			});
	if (CrossSkill == nullptr)
	{
		return false;
	}
	Interaction->RequestSkill(CrossSkill->OptionId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	for (int32 Guard = 0; Guard < 4; ++Guard)
	{
		if (Controller->IsAwaitingHotSeatHandoff())
		{
			Screen->RequestReady();
		}
		const EFMCodexUMGInteractionCategory Category =
			Interaction->GetPresentation().Category;
		if (Category != EFMCodexUMGInteractionCategory::SelectRunner
			&& Category != EFMCodexUMGInteractionCategory::SelectHelper)
		{
			break;
		}
		if (!Interaction->GetPresentation().SelectionChoices.IsEmpty())
		{
			const FName Id =
				Interaction->GetPresentation().SelectionChoices[0].OptionId;
			if (Category == EFMCodexUMGInteractionCategory::SelectRunner)
			{
				Interaction->RequestRunner(Id);
			}
			else
			{
				Interaction->RequestHelper(Id);
			}
		}
		else if (Interaction->GetPresentation().bCanResolveNoLegal)
		{
			Interaction->RequestNoLegal();
		}
		else
		{
			Interaction->RequestDecline();
		}
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
	}
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	Interaction->RequestBranch(EFMCodexUMGBranchIntent::CrossHigh);
	TestTrue(TEXT("System resolution is explicit and attacker is retained"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Header->GetPresentation().bSystemResolution
			&& Header->GetDisplayedActorLabel() == TEXT("SYSTEM RESOLUTION")
			&& Header->GetDisplayedAttackerLabel().Contains(
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));

	const FFMCodexUMGMatchHeaderViewModel BeforeRejected =
		Header->GetPresentation();
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestBeginOrdinaryAttack();
	TestTrue(TEXT("Rejected command cannot optimistically mutate Header"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& StateBeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Header->GetPresentation().ScoreLabel == BeforeRejected.ScoreLabel
			&& Header->GetPresentation().AttackerStatusLabel
				== BeforeRejected.AttackerStatusLabel
			&& Header->GetPresentation().ActorStatusLabel
				== BeforeRejected.ActorStatusLabel
			&& Header->GetPresentation().MatchStatusLabel
				== BeforeRejected.MatchStatusLabel);

	for (int32 Guard = 0;
		Guard < 12 && Controller->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		Interaction->RequestContinue();
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
	}
	const FMatchPlayState& CompletedState = Host->GetMatchSnapshot().Snapshot;
	const FMatchRuntimeState& CompletedRuntime = CompletedState.RuntimeState;
	TestTrue(TEXT("Completion Header score is refreshed from authority"),
		Header->GetPresentation().PlayerAScoreLabel
			== FString::FromInt(CompletedRuntime.PlayerAState.Score)
			&& Header->GetPresentation().PlayerBScoreLabel
				== FString::FromInt(CompletedRuntime.PlayerBState.Score));
	TestTrue(TEXT("Between attacks identifies next attacker without active claim"),
		!Header->GetPresentation().bAttackActive
			&& Header->GetDisplayedAttackerLabel().StartsWith(
				TEXT("NEXT ATTACKER:"))
			&& !Header->GetDisplayedAttackerLabel().EndsWith(TEXT(" ATTACKING")));

	FString HeaderHeader;
	FString HeaderSource;
	FString RootHeader;
	FString RootSource;
	FString PitchSource;
	FString CardSource;
	FString InteractionSource;
	FString ResolutionSource;
	FString ControllerSource;
	TestTrue(TEXT("Header boundary audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.h"),
			HeaderHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"),
				HeaderSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"),
				PitchSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"),
				ResolutionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString HeaderWidgetSources = HeaderHeader + HeaderSource;
	TestTrue(TEXT("Match Header receives reflected presentation DTO only"),
		HeaderHeader.Contains(TEXT("FFMCodexUMGMatchHeaderViewModel"))
			&& !HeaderWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !HeaderWidgetSources.Contains(TEXT("HostGameMode"))
			&& !HeaderWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !HeaderWidgetSources.Contains(TEXT("FMatchPlayCurrentAttack"))
			&& !HeaderWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Match Header contains no score/winner/actor calculation"),
		!HeaderWidgetSources.Contains(TEXT("CurrentAttackingPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("ExpectedActingPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("EInitialTurnOrderPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("EMatchResultType"))
			&& !HeaderWidgetSources.Contains(TEXT("FMath"))
			&& !HeaderWidgetSources.Contains(TEXT("Score =")));
	TestTrue(TEXT("Match Header contains no gameplay rules, RNG or input"),
		!HeaderWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !HeaderWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !HeaderWidgetSources.Contains(TEXT("Legality"))
			&& !HeaderWidgetSources.Contains(TEXT("RandRange"))
			&& !HeaderWidgetSources.Contains(TEXT("RandomStream"))
			&& !HeaderWidgetSources.Contains(TEXT("UButton"))
			&& !HeaderWidgetSources.Contains(TEXT("OnClicked"))
			&& !HeaderWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Header is family-independent and contains no resolution detail"),
		!HeaderWidgetSources.Contains(TEXT("Cross"))
			&& !HeaderWidgetSources.Contains(TEXT("ThroughBall"))
			&& !HeaderWidgetSources.Contains(TEXT("Dice"))
			&& !HeaderWidgetSources.Contains(TEXT("Formula")));
	TestTrue(TEXT("Root delegates Header to a configurable dedicated Widget"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexMatchHeaderWidget>"))
			&& RootSource.Contains(TEXT("DedicatedMatchHeaderWidget"))
			&& !RootSource.Contains(TEXT("MatchHeaderText")));
	TestNotNull(TEXT("Stage 6.5 Pitch Widget remains present"),
		Screen->GetPitchWidget());
	TestNotNull(TEXT("Stage 6.7 Interaction Panel remains present"),
		Screen->GetInteractionPanel());
	TestNotNull(TEXT("Stage 6.8 Resolution Panel remains present"),
		Screen->GetResolutionPanel());
	TestTrue(TEXT("Stage 6.5-6.8 source structures remain present"),
		PitchSource.Contains(TEXT("CanonicalPitchSlot"))
			&& PitchSource.Contains(TEXT("RenderedSlotWidgets.Add"))
			&& CardSource.Contains(TEXT("PlayerCardFrame"))
			&& InteractionSource.Contains(TEXT("InteractionPanelBounds"))
			&& ResolutionSource.Contains(TEXT("ResolutionPanelBounds")));
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGVisualStyleFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.34.UMGVisualStyleFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGVisualStyleFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	TestTrue(TEXT("Shared UI style defaults are valid"),
		Style.HasValidDefaults());
	TestTrue(TEXT("Score and terminal typography dominate body text"),
		Style.GetFontSize(EFMCodexPlayerUITextRole::Score)
			> Style.GetFontSize(EFMCodexPlayerUITextRole::Identity)
			&& Style.GetFontSize(EFMCodexPlayerUITextRole::TerminalResult)
				> Style.GetFontSize(EFMCodexPlayerUITextRole::Body));
	TestTrue(TEXT("Temporary player accents are replaceable and distinct"),
		!Style.GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent)));
	TestTrue(TEXT("Action roles have visibly distinct presentation colors"),
		!Style.GetColor(EFMCodexPlayerUIColorRole::ActionPrimary).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::ActionSecondary))
			&& !Style.GetColor(EFMCodexPlayerUIColorRole::ActionDecline).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::ActionPrimary)));
	TestTrue(TEXT("Presentation-only status and terminal emphasis is defined"),
		Style.GetStatusBadgeColor(TEXT("AVAILABLE")).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::StatusAvailable))
			&& Style.GetStatusBadgeColor(TEXT("USED")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::StatusUsed))
			&& Style.GetTerminalColor(TEXT("GOAL")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::Success))
			&& Style.GetTerminalColor(TEXT("NO GOAL")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::Danger)));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Style foundation Host exists"), Host);
	TestNotNull(TEXT("Style foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	for (const TCHAR* RootRegion : {
		TEXT("MatchScreenStyleBackground"), TEXT("MatchHeaderRegion"),
		TEXT("FootballCardFieldRegion"), TEXT("CurrentInteractionRegion"),
		TEXT("ResolutionResultRegion"), TEXT("HotSeatHandoffOverlay"),
		TEXT("HotSeatHandoffCard"), TEXT("HotSeatReadyButton") })
	{
		TestNotNull(FString::Printf(TEXT("Styled root contains %s"), RootRegion),
			Screen->GetWidgetFromName(RootRegion));
	}

	UFMCodexMatchHeaderWidget* Header = Screen->GetMatchHeader();
	if (Header == nullptr)
	{
		return false;
	}
	Header->TakeWidget();
	FFMCodexUMGMatchHeaderViewModel HeaderDTO;
	HeaderDTO.bMatchActive = true;
	HeaderDTO.bAttackActive = true;
	HeaderDTO.MatchStatusLabel = TEXT("MATCH IN PROGRESS");
	HeaderDTO.PlayerAScoreLabel = TEXT("2");
	HeaderDTO.PlayerBScoreLabel = TEXT("1");
	HeaderDTO.ScoreLabel = TEXT("2 - 1");
	HeaderDTO.AttackerStatusLabel = TEXT("PLAYER A ATTACKING");
	HeaderDTO.ActorStatusLabel = TEXT("PLAYER B TO ACT");
	HeaderDTO.bHumanAction = true;
	Header->RefreshFromPresentation(HeaderDTO);
	const UBorder* AttackerRegion = Cast<UBorder>(Header->GetWidgetFromName(
		TEXT("MatchHeaderAttackerStatusRegion")));
	const UBorder* ActorRegion = Cast<UBorder>(Header->GetWidgetFromName(
		TEXT("MatchHeaderActorStatusRegion")));
	TestTrue(TEXT("Header styling preserves ATTACKING versus TO ACT"),
		AttackerRegion != nullptr && ActorRegion != nullptr
			&& AttackerRegion->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent))
			&& ActorRegion->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent))
			&& Header->GetDisplayedAttackerLabel()
				== TEXT("PLAYER A ATTACKING")
			&& Header->GetDisplayedActorLabel() == TEXT("PLAYER B TO ACT"));
	HeaderDTO.bSystemResolution = true;
	HeaderDTO.bHumanAction = false;
	HeaderDTO.ActorStatusLabel = TEXT("SYSTEM RESOLUTION");
	Header->RefreshFromPresentation(HeaderDTO);
	TestTrue(TEXT("Header system resolution uses dedicated shared style"),
		ActorRegion != nullptr && ActorRegion->GetBrushColor().Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)));
	HeaderDTO.bMatchEnded = true;
	HeaderDTO.MatchStatusLabel = TEXT("MATCH ENDED");
	HeaderDTO.MatchResultLabel = TEXT("Player A Win");
	Header->RefreshFromPresentation(HeaderDTO);
	TestTrue(TEXT("Match-ended style preserves canonical result priority"),
		Header->GetPresentation().MatchResultLabel == TEXT("Player A Win")
			&& Header->GetWidgetFromName(TEXT("MatchHeaderFinalResultRegion"))
				->GetVisibility() == ESlateVisibility::Visible
			&& Header->GetWidgetFromName(
				TEXT("MatchHeaderActorStatusRegion"))->GetVisibility()
					== ESlateVisibility::Collapsed);

	FFMCodexUMGCardViewModel Card;
	Card.CardId = TEXT("Style.Card.AllFamilies");
	Card.IdentityLabel = TEXT("Style Foundation Card");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW");
	Card.RarityLabel = TEXT("Club");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary = TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		Region.RegionLabel = RegionIndex == 0
			? TEXT("PLAYER B HALF") : TEXT("PLAYER A HALF");
		Region.ZoneContextLabel = TEXT("Canonical relative zones");
		Region.bCurrentAttackingSide = RegionIndex == 0;
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Style.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.SlotLabel = FString::Printf(TEXT("POSITION %d"), SlotIndex + 1);
			Slot.PhysicalHalfLabel = Region.RegionLabel;
			Slot.PlayerARelativeZoneLabel = TEXT("Relative A");
			Slot.PlayerBRelativeZoneLabel = TEXT("Relative B");
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied)
			{
				Slot.Card = Card;
			}
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	if (Pitch == nullptr)
	{
		return false;
	}
	Pitch->TakeWidget();
	Pitch->RefreshFromPitchPresentation(PitchPresentation);
	TestTrue(TEXT("Styled Pitch preserves two halves and 10/10 slots"),
		Pitch->GetPresentation().Num() == 2
			&& Pitch->GetRenderedSlotWidgets().Num() == 10
			&& Pitch->GetWidgetFromName(TEXT("PitchBackgroundAssetHook"))
				!= nullptr
			&& Pitch->GetWidgetFromName(TEXT("CenterFieldVisualSeparator"))
				!= nullptr);
	UFMCodexPitchSlotWidget* OccupiedSlot =
		Pitch->GetRenderedSlotWidgets()[0];
	UFMCodexPitchSlotWidget* EmptySlot =
		Pitch->GetRenderedSlotWidgets()[1];
	OccupiedSlot->TakeWidget();
	EmptySlot->TakeWidget();
	TestTrue(TEXT("Pitch slots distinguish occupied from passive empty state"),
		OccupiedSlot->IsShowingOccupiedCard()
			&& !EmptySlot->IsShowingOccupiedCard()
			&& EmptySlot->GetWidgetFromName(TEXT("EmptySpatialLocation"))
				!= nullptr
			&& Cast<UBorder>(OccupiedSlot->GetWidgetFromName(
				TEXT("CanonicalPitchSlotBorder")))->GetBrushColor().Equals(
					Style.GetColor(
						EFMCodexPlayerUIColorRole::OccupiedPitchSlot))
			&& Cast<UBorder>(EmptySlot->GetWidgetFromName(
				TEXT("CanonicalPitchSlotBorder")))->GetBrushColor().Equals(
					Style.GetColor(
						EFMCodexPlayerUIColorRole::EmptyPitchSlot)));
	UFMCodexPlayerCardWidget* PitchCard = OccupiedSlot->GetCardWidget();
	if (PitchCard == nullptr)
	{
		return false;
	}
	PitchCard->TakeWidget();
	TestTrue(TEXT("PitchCompact Card preserves identity and asset-ready hooks"),
		PitchCard->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::PitchCompact
			&& PitchCard->GetRenderedSkillCount() == 5
			&& PitchCard->GetWidgetFromName(TEXT("CardFrameAssetHook")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PortraitAssetHook")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("RoleIconHook")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("SkillIconHook0")) != nullptr);
	PitchCard->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("InteractionChoice retains all five Skill labels"),
		PitchCard->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& PitchCard->GetRenderedSkillCount() == Card.SkillLabels.Num()
			&& PitchCard->GetRenderedAttributeCount() == 10);
	Card.bGoalkeeper = true;
	Card.RoleLabel = TEXT("GK");
	Card.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	Card.CompactAttributeSummary = TEXT("HAN 5 | REF 4 | AER 3 | 1v1 4");
	Card.FullAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 3 | 1v1 4");
	PitchCard->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("GK Card uses style-only visual variant and status badges"),
		PitchCard->IsGoalkeeperVisualVariant()
			&& PitchCard->GetRenderedStatusBadgeCount() == 2
			&& Cast<UBorder>(PitchCard->GetWidgetFromName(
				TEXT("PlayerCardFrame")))->GetBrushColor().Equals(
					Style.GetColor(
						EFMCodexPlayerUIColorRole::GoalkeeperCardFrame)));

	UFMCodexInteractionPanelWidget* Interaction =
		Screen->GetInteractionPanel();
	if (Interaction == nullptr)
	{
		return false;
	}
	Interaction->TakeWidget();
	FFMCodexUMGInteractionViewModel Action;
	Action.Category = EFMCodexUMGInteractionCategory::ContinueResolution;
	Action.KickerLabel = TEXT("PLAYER ACTION");
	Action.ExpectedActorLabel = TEXT("PLAYER A TO ACT");
	Action.TitleLabel = TEXT("CHOOSE NEXT ACTION");
	Action.bCanContinue = true;
	Action.bCanDecline = true;
	Action.bCanResolveNoLegal = true;
	Action.PrimaryActionLabel = TEXT("CONTINUE");
	Action.DeclineActionLabel = TEXT("DECLINE");
	Action.NoLegalActionLabel = TEXT("NO LEGAL CHOICE");
	Interaction->RefreshFromPresentation(Action);
	auto ButtonTint = [](const UButton* Button)
	{
		return Button->GetStyle().Normal.TintColor.GetSpecifiedColor();
	};
	const UButton* PrimaryButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionContinueButton")));
	const UButton* DeclineButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionDeclineButton")));
	const UButton* SecondaryButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionNoLegalButton")));
	TestTrue(TEXT("Interaction primary secondary and decline styles are distinct"),
		PrimaryButton != nullptr && DeclineButton != nullptr
			&& SecondaryButton != nullptr
			&& !ButtonTint(PrimaryButton).Equals(ButtonTint(SecondaryButton))
			&& !ButtonTint(PrimaryButton).Equals(ButtonTint(DeclineButton)));
	Action.Category = EFMCodexUMGInteractionCategory::SelectOneOnOneShot;
	Action.bCanContinue = false;
	Action.bCanDecline = false;
	Action.bCanResolveNoLegal = false;
	Action.OneOnOneChoices = {
		{ EFMCodexUMGOneOnOneChoice::ChipShot, TEXT("CHIP SHOT") },
		{ EFMCodexUMGOneOnOneChoice::DirectShot, TEXT("DIRECT SHOT") }
	};
	Interaction->RefreshFromPresentation(Action);
	TestEqual(TEXT("OneOnOne choices retain styled typed options"),
		Interaction->GetRenderedOptionWidgets().Num(), 2);
	Action.KickerLabel = TEXT("SYSTEM RESOLUTION");
	Action.ExpectedActorLabel = TEXT("SYSTEM RESOLUTION");
	Interaction->RefreshFromPresentation(Action);
	TestTrue(TEXT("Interaction system mode has dedicated visual treatment"),
		Cast<UBorder>(Interaction->GetWidgetFromName(
			TEXT("InteractionActionHeader")))->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)));

	UFMCodexResolutionPanelWidget* Resolution =
		Screen->GetResolutionPanel();
	if (Resolution == nullptr)
	{
		return false;
	}
	Resolution->TakeWidget();
	FFMCodexUMGResolutionViewModel Result;
	Result.bVisible = true;
	Result.bTerminal = true;
	Result.StepLabel = TEXT("FINISHING FORMULA");
	Result.StepSummaryLabel = TEXT("Authoritative comparison accepted");
	Result.DiceResults = { { TEXT("ATTACK"), TEXT("Finishing roll"), 6 } };
	Result.ComparisonEvidence = {
		{ TEXT("ATTACK"), TEXT("11") },
		{ TEXT("DEFENSE"), TEXT("8") }
	};
	Result.DecisionLabel = TEXT("ATTACK WINS");
	Result.TerminalLabel = TEXT("GOAL");
	Resolution->RefreshFromPresentation(Result);
	TestTrue(TEXT("Resolution styles Dice Comparison Decision and terminal result"),
		Resolution->GetRenderedDiceWidgets().Num() == 1
			&& Resolution->GetRenderedComparisonCount() == 2
			&& Resolution->GetWidgetFromName(TEXT("ResolutionVersusLabel"))
				!= nullptr
			&& Resolution->GetWidgetFromName(TEXT("ResultIconAssetHook"))
				!= nullptr
			&& Cast<UBorder>(Resolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion")))->GetBrushColor().Equals(
					Style.GetColor(EFMCodexPlayerUIColorRole::Success)));
	UFMCodexDiceResultWidget* Die = Resolution->GetRenderedDiceWidgets()[0];
	Die->TakeWidget();
	TestTrue(TEXT("Dice reads as a bounded result object"),
		Die->GetWidgetFromName(TEXT("DiceFaceAssetHook")) != nullptr
			&& Cast<UTextBlock>(Die->GetWidgetFromName(
				TEXT("DiceRawD6Value")))->GetText().ToString() == TEXT("[ 6 ]"));
	for (const TCHAR* TerminalLabel : {
		TEXT("MISS"), TEXT("NO GOAL"), TEXT("IMMEDIATE MISS"),
		TEXT("OFFSIDE"), TEXT("OUT OF PLAY"),
		TEXT("DEFENDER STOPPED ATTACK") })
	{
		Result.TerminalLabel = TerminalLabel;
		Resolution->RefreshFromPresentation(Result);
		TestTrue(FString::Printf(TEXT("%s receives failure emphasis"),
			TerminalLabel),
			Cast<UBorder>(Resolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion")))->GetBrushColor().Equals(
					Style.GetColor(EFMCodexPlayerUIColorRole::Danger)));
	}
	Result.bRejected = true;
	Result.DecisionLabel = TEXT("COMMAND REJECTED");
	Result.ErrorLabel = TEXT("Authoritative state unchanged");
	Resolution->RefreshFromPresentation(Result);
	TestTrue(TEXT("Rejected command is distinct from successful resolution"),
		Resolution->GetWidgetFromName(TEXT("ResolutionRejectedRegion"))
			->GetVisibility() == ESlateVisibility::Visible
			&& Resolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy"))->GetVisibility()
					== ESlateVisibility::Collapsed);

	const TArray<uint8> StateBeforeStyleRefresh =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	FFMCodexUMGMatchScreenViewModel HandoffPresentation =
		Screen->GetPresentation();
	HandoffPresentation.Handoff.bVisible = true;
	HandoffPresentation.Handoff.TitleLabel = TEXT("PASS CONTROL");
	HandoffPresentation.Handoff.NextPlayerLabel = TEXT("Next Player: Player B");
	HandoffPresentation.Handoff.ReadyLabel = TEXT("Confirm the handoff, then press Ready");
	Screen->RefreshFromPresentation(HandoffPresentation);
	TestTrue(TEXT("Styled handoff remains presentation-only and blocks interaction"),
		StateBeforeStyleRefresh == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& !Screen->GetInteractionPanel()->GetIsEnabled()
			&& Cast<UTextBlock>(Screen->GetWidgetFromName(
				TEXT("HotSeatHandoffText")))->GetText().ToString()
					== TEXT("PASS CONTROL")
			&& Cast<UTextBlock>(Screen->GetWidgetFromName(
				TEXT("HotSeatNextPlayerText")))->GetText().ToString().Contains(
					TEXT("Player B")));

	FString StyleHeader;
	FString StyleSource;
	FString RootSource;
	FString HeaderSource;
	FString PitchSource;
	FString SlotSource;
	FString CardSource;
	FString InteractionSource;
	FString OptionSource;
	FString ResolutionSource;
	FString DiceSource;
	FString ControllerSource;
	TestTrue(TEXT("Visual style boundary sources load"),
		LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIStyle.h"), StyleHeader)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIStyle.cpp"), StyleSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"), RootSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"), HeaderSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"), PitchSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"), SlotSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"), CardSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"), InteractionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.cpp"), OptionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"), ResolutionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.cpp"), DiceSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"), ControllerSource));
	const FString StyleSources = StyleHeader + StyleSource;
	const FString StyledWidgetSources = RootSource + HeaderSource + PitchSource
		+ SlotSource + CardSource + InteractionSource + OptionSource
		+ ResolutionSource + DiceSource;
	TestTrue(TEXT("Shared style owns no gameplay authority"),
		!StyleSources.Contains(TEXT("FMatchPlayState"))
			&& !StyleSources.Contains(TEXT("AuthoritativeSession"))
			&& !StyleSources.Contains(TEXT("HostGameMode"))
			&& !StyleSources.Contains(TEXT("D6Provider"))
			&& !StyleSources.Contains(TEXT("FormulaResolver"))
			&& !StyleSources.Contains(TEXT("Legality"))
			&& !StyleSources.Contains(TEXT("RouteThreshold"))
			&& !StyleSources.Contains(TEXT("RandRange"))
			&& !StyleSources.Contains(TEXT("RandomStream")));
	TestTrue(TEXT("Every major UMG structure consumes one shared visual style"),
		RootSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& HeaderSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& PitchSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& SlotSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& CardSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& InteractionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& OptionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& ResolutionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& DiceSource.Contains(TEXT("FMCodexPlayerUIStyle.h")));
	TestTrue(TEXT("Asset integration hooks remain bounded to visual Widgets"),
		StyledWidgetSources.Contains(TEXT("CardFrameAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("PortraitAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("SkillIconHook"))
			&& StyledWidgetSources.Contains(TEXT("RoleIconHook"))
			&& StyledWidgetSources.Contains(TEXT("CrestAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("PitchBackgroundAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("ResultIconAssetHook")));
	TestTrue(TEXT("Style introduces no generic dispatch or gameplay calculations"),
		!StyleSources.Contains(TEXT("ExecuteCommandByName"))
			&& !StyleSources.Contains(TEXT("CommandDispatcher"))
			&& !StyleSources.Contains(TEXT("ScoreResolver"))
			&& !StyleSources.Contains(TEXT("CurrentAttackingPlayer"))
			&& !StyleSources.Contains(TEXT("ExpectedActingPlayer")));
	TestTrue(TEXT("Typed Controller and Slate developer surface remain owned"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface"))
			&& ControllerSource.Contains(TEXT("SubmitCarrier"))
			&& ControllerSource.Contains(TEXT("ContinueResolution"))
			&& !ControllerSource.Contains(TEXT("ExecuteCommandByName")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexValidatedPlayerCardArtPilotIntegrationTest,
	"FMCodex.LocalPlay.ControlSurface.35.ValidatedPlayerCardArtPilotIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexValidatedPlayerCardArtPilotIntegrationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIAssetReferences& AssetReferences =
		FFMCodexPlayerUIAssetReferences::Get();
	const FFMCodexPlayerUICardArtReferences PilotArt =
		AssetReferences.ResolveCardArt(AssetReferences.GetPilotCardId());
	const FFMCodexPlayerUICardArtReferences MissingArt =
		AssetReferences.ResolveCardArt(TEXT("Demo.Missing.Cosmetic"));
	TestTrue(TEXT("Pilot catalog uses one deterministic presentation mapping"),
		AssetReferences.GetPilotCardId() == FName(TEXT("Demo.A.Outfield.01"))
			&& PilotArt.ArtIdentity == AssetReferences.GetPilotArtIdentity()
			&& !PilotArt.CardFrame.IsNull()
			&& !PilotArt.Portrait.IsNull()
			&& MissingArt.ArtIdentity.IsNone()
			&& MissingArt.CardFrame.IsNull()
			&& MissingArt.Portrait.IsNull());

	UTexture2D* FrameTexture = PilotArt.CardFrame.LoadSynchronous();
	UTexture2D* PortraitTexture = PilotArt.Portrait.LoadSynchronous();
	TestTrue(TEXT("Imported pilot packages load as substantial Texture2D assets"),
		FrameTexture != nullptr && PortraitTexture != nullptr
			&& FrameTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& PortraitTexture->GetImportedSize() == FIntPoint(1024, 1536));
	if (FrameTexture == nullptr || PortraitTexture == nullptr)
	{
		return false;
	}

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Pilot art integration Host exists"), Host);
	TestNotNull(TEXT("Pilot art integration Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Pilot art integration screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel Card;
	Card.CardId = AssetReferences.GetPilotCardId();
	Card.IdentityLabel = TEXT("Pilot Prototype Player");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW / MF");
	Card.RarityLabel = TEXT("Pilot");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	auto MakeCardWidget = [Screen, &Card](
		const EFMCodexPlayerCardPresentationMode Mode)
	{
		UFMCodexPlayerCardWidget* Widget =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		if (Widget != nullptr)
		{
			Widget->RefreshFromPresentation(Card, Mode);
			Widget->TakeWidget();
		}
		return Widget;
	};
	UFMCodexPlayerCardWidget* PitchCompactCard = MakeCardWidget(
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	UFMCodexPlayerCardWidget* InteractionChoiceCard = MakeCardWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestNotNull(TEXT("PitchCompact pilot Card constructs"), PitchCompactCard);
	TestNotNull(TEXT("InteractionChoice pilot Card constructs"),
		InteractionChoiceCard);
	if (PitchCompactCard == nullptr || InteractionChoiceCard == nullptr)
	{
		return false;
	}

	auto HasBoundPilotBrushes = [FrameTexture, PortraitTexture](
		const UFMCodexPlayerCardWidget& Widget)
	{
		const UImage* FrameImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("CardFrameAssetImage")));
		const UImage* PortraitImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("PortraitAssetImage")));
		return FrameImage != nullptr && PortraitImage != nullptr
			&& FrameImage->GetBrush().GetResourceObject() == FrameTexture
			&& PortraitImage->GetBrush().GetResourceObject() == PortraitTexture
			&& FrameImage->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& PortraitImage->GetVisibility()
				== ESlateVisibility::HitTestInvisible;
	};
	TestTrue(TEXT("PitchCompact binds real frame and portrait brush resources"),
		HasBoundPilotBrushes(*PitchCompactCard)
			&& PitchCompactCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchCompact);
	TestTrue(TEXT("InteractionChoice binds the same art identity and resources"),
		HasBoundPilotBrushes(*InteractionChoiceCard)
			&& InteractionChoiceCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& PitchCompactCard->GetResolvedArtIdentity()
				== InteractionChoiceCard->GetResolvedArtIdentity()
			&& PitchCompactCard->GetResolvedCardFrameTexture()
				== InteractionChoiceCard->GetResolvedCardFrameTexture()
			&& PitchCompactCard->GetResolvedPortraitTexture()
				== InteractionChoiceCard->GetResolvedPortraitTexture());
	TestTrue(TEXT("Pilot art supplements and preserves card semantics"),
		PitchCompactCard->GetPresentation().CardId == Card.CardId
			&& PitchCompactCard->GetRenderedSkillCount() == 5
			&& PitchCompactCard->GetRenderedAttributeCount() == 4
			&& PitchCompactCard->GetRenderedStatusBadgeCount() == 1
			&& InteractionChoiceCard->GetRenderedSkillCount() == 5
			&& InteractionChoiceCard->GetRenderedAttributeCount() == 10
			&& InteractionChoiceCard->GetRenderedStatusBadgeCount() == 1);

	FFMCodexUMGCardViewModel MissingCard = Card;
	MissingCard.CardId = TEXT("Demo.Missing.Cosmetic");
	UFMCodexPlayerCardWidget* FallbackCard =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	if (FallbackCard == nullptr)
	{
		return false;
	}
	FallbackCard->RefreshFromPresentation(
		MissingCard, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	FallbackCard->TakeWidget();
	const UImage* MissingFrameImage = Cast<UImage>(
		FallbackCard->GetWidgetFromName(TEXT("CardFrameAssetImage")));
	const UImage* MissingPortraitImage = Cast<UImage>(
		FallbackCard->GetWidgetFromName(TEXT("PortraitAssetImage")));
	TestTrue(TEXT("Missing frame safely restores standard style surface"),
		FallbackCard->GetResolvedCardFrameTexture() == nullptr
			&& MissingFrameImage != nullptr
			&& MissingFrameImage->GetVisibility() == ESlateVisibility::Collapsed
			&& FallbackCard->GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("Missing portrait safely restores placeholder"),
		FallbackCard->GetResolvedPortraitTexture() == nullptr
			&& MissingPortraitImage != nullptr
			&& MissingPortraitImage->GetVisibility()
				== ESlateVisibility::Collapsed
			&& FallbackCard->GetWidgetFromName(TEXT("PortraitPlaceholderText"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& FallbackCard->GetPresentation().CardId == MissingCard.CardId);

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		Region.RegionLabel = RegionIndex == 0
			? TEXT("PLAYER B HALF") : TEXT("PLAYER A HALF");
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Pilot.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.SlotLabel = FString::Printf(TEXT("POSITION %d"), SlotIndex + 1);
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied)
			{
				Slot.Card = Card;
			}
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	const TArray<uint8> StateBeforePresentation =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	Pitch->RefreshFromPitchPresentation(PitchPresentation);
	TestTrue(TEXT("Pilot art preserves all 10 canonical Pitch slots"),
		Pitch->GetRenderedSlotWidgets().Num() == 10);
	UFMCodexPitchSlotWidget* PilotSlot =
		Pitch->GetRenderedSlotWidgets().IsEmpty()
			? nullptr : Pitch->GetRenderedSlotWidgets()[0];
	if (PilotSlot != nullptr)
	{
		PilotSlot->TakeWidget();
	}
	UFMCodexPlayerCardWidget* PilotPitchCard = PilotSlot == nullptr
		? nullptr : PilotSlot->GetCardWidget();
	TestTrue(TEXT("Pitch slot uses the centralized pilot art identity"),
		PilotPitchCard != nullptr
			&& PilotPitchCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchCompact
			&& PilotPitchCard->GetResolvedArtIdentity()
				== InteractionChoiceCard->GetResolvedArtIdentity()
			&& HasBoundPilotBrushes(*PilotPitchCard));
	TestTrue(TEXT("Cosmetic Pitch enrichment cannot mutate authority"),
		StateBeforePresentation
			== SerializeState(Host->GetMatchSnapshot().Snapshot));

	Screen->RequestStartNewMatch();
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(
		AssetReferences.GetPilotCardId(), NAME_None);
	TestTrue(TEXT("Rejected typed command preserves State handoff and pilot art"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& StateBeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Controller->IsAwaitingHotSeatHandoff()
			&& Screen->GetPresentation().Handoff.bVisible
			&& HasBoundPilotBrushes(*InteractionChoiceCard)
			&& InteractionChoiceCard->GetPresentation().CardId == Card.CardId);

	FString AssetReferenceHeader;
	FString AssetReferenceSource;
	FString CardSource;
	FString AuthoritySources;
	FString SourcePart;
	TestTrue(TEXT("Pilot presentation boundary sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.h"),
			AssetReferenceHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp"),
				AssetReferenceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource));
	for (const TCHAR* Path : {
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.h"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.cpp"),
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
		TEXT("Source/FMCodex/CoreRules/MatchPlayState.h") })
	{
		if (LoadProductionSource(Path, SourcePart))
		{
			AuthoritySources += SourcePart;
		}
		else
		{
			AddError(FString::Printf(TEXT("Could not audit %s"), Path));
		}
	}
	TestTrue(TEXT("Soft asset paths are centralized in UI presentation"),
		AssetReferenceHeader.Contains(TEXT("TSoftObjectPtr<UTexture2D>"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Cards/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Portraits/"))
			&& !CardSource.Contains(TEXT("/Game/UI/")));
	TestTrue(TEXT("Authority State Host Session and D6 provider have zero UI refs"),
		!AuthoritySources.Contains(TEXT("/Game/UI/"))
			&& !AuthoritySources.Contains(TEXT("TSoftObjectPtr<UTexture2D>"))
			&& !AuthoritySources.Contains(TEXT("LoadObject<UTexture2D>"))
			&& !AuthoritySources.Contains(TEXT("LoadSynchronous")));
	TestTrue(TEXT("Card asset layer stays presentation-only"),
		!CardSource.Contains(TEXT("FMatchPlayState"))
			&& !CardSource.Contains(TEXT("AuthoritativeSession"))
			&& !CardSource.Contains(TEXT("D6Provider"))
			&& !CardSource.Contains(TEXT("FormulaResolver"))
			&& !CardSource.Contains(TEXT("Legality"))
			&& CardSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& CardSource.Contains(TEXT("CardFrameAssetHook"))
			&& CardSource.Contains(TEXT("PortraitAssetHook")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexGoldenSampleVisualDirectionTest,
	"FMCodex.LocalPlay.ControlSurface.36.GoldenSampleVisualDirection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexGoldenSampleVisualDirectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	const FFMCodexPlayerUICardArtReferences Golden =
		References.ResolveCardArt(References.GetGoldenSampleCardId());
	const FFMCodexPlayerUICardArtReferences Missing =
		References.ResolveCardArt(TEXT("Demo.Missing.GoldenSample"));
	TestTrue(TEXT("Golden Sample uses one deterministic presentation identity"),
		References.GetGoldenSampleCardId() == FName(TEXT("Demo.A.Outfield.02"))
			&& References.GetGoldenSampleArtIdentity()
				== FName(TEXT("GoldenSample.PlayerCard.01"))
			&& Golden.ArtIdentity == References.GetGoldenSampleArtIdentity()
			&& !Golden.CardFrame.IsNull()
			&& !Golden.Portrait.IsNull()
			&& !Golden.RoleIcon.IsNull()
			&& !Golden.LongShotSkillIcon.IsNull());
	TestTrue(TEXT("Unknown CardId has all four cosmetic fallbacks"),
		Missing.ArtIdentity.IsNone()
			&& Missing.CardFrame.IsNull()
			&& Missing.Portrait.IsNull()
			&& Missing.RoleIcon.IsNull()
			&& Missing.LongShotSkillIcon.IsNull());

	UTexture2D* FrameTexture = Golden.CardFrame.LoadSynchronous();
	UTexture2D* PortraitTexture = Golden.Portrait.LoadSynchronous();
	UTexture2D* RoleIconTexture = Golden.RoleIcon.LoadSynchronous();
	UTexture2D* SkillIconTexture = Golden.LongShotSkillIcon.LoadSynchronous();
	TestTrue(TEXT("All four Golden Sample packages load with expected dimensions"),
		FrameTexture != nullptr && PortraitTexture != nullptr
			&& RoleIconTexture != nullptr && SkillIconTexture != nullptr
			&& FrameTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& PortraitTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& RoleIconTexture->GetImportedSize() == FIntPoint(1254, 1254)
			&& SkillIconTexture->GetImportedSize() == FIntPoint(1254, 1254)
			&& !RoleIconTexture->CompressionNoAlpha
			&& !SkillIconTexture->CompressionNoAlpha);
	if (FrameTexture == nullptr || PortraitTexture == nullptr
		|| RoleIconTexture == nullptr || SkillIconTexture == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Forward role has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Role(TEXT("FW / MF")).ToString(),
		FString(TEXT("前锋 / 中场")));
	TestEqual(TEXT("Long Shot has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Skill(TEXT("Long Shot")).ToString(),
		FString(TEXT("远射")));
	TestEqual(TEXT("Attribute has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Attribute(TEXT("SHO 5")).ToString(),
		FString(TEXT("射门 5")));
	TestEqual(TEXT("Status has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Status(TEXT("AVAILABLE")).ToString(),
		FString(TEXT("可用")));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Golden Sample Host exists"), Host);
	TestNotNull(TEXT("Golden Sample Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen = Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Golden Sample screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel Card;
	Card.CardId = References.GetGoldenSampleCardId();
	Card.IdentityLabel = TEXT("Golden Sample Forward");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW / MF");
	Card.RarityLabel = TEXT("Pilot");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	auto MakeWidget = [Screen, &Card](
		const EFMCodexPlayerCardPresentationMode Mode,
		const FFMCodexUMGCardViewModel* Override = nullptr)
	{
		UFMCodexPlayerCardWidget* Widget =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		if (Widget != nullptr)
		{
			Widget->RefreshFromPresentation(
				Override == nullptr ? Card : *Override, Mode);
			Widget->TakeWidget();
		}
		return Widget;
	};
	const TArray<uint8> StateBeforeWidgets =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	UFMCodexPlayerCardWidget* PitchCompact = MakeWidget(
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	UFMCodexPlayerCardWidget* InteractionChoice = MakeWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestNotNull(TEXT("Golden Sample PitchCompact constructs"), PitchCompact);
	TestNotNull(TEXT("Golden Sample InteractionChoice constructs"),
		InteractionChoice);
	if (PitchCompact == nullptr || InteractionChoice == nullptr)
	{
		return false;
	}

	auto HasFourBrushes = [FrameTexture, PortraitTexture, RoleIconTexture,
		SkillIconTexture](const UFMCodexPlayerCardWidget& Widget)
	{
		const UImage* Frame = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("CardFrameAssetImage")));
		const UImage* Portrait = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("PortraitAssetImage")));
		const UImage* RoleIcon = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("RoleIconAssetImage")));
		const UImage* SkillIcon = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("SkillIconAssetImage0")));
		return Frame != nullptr && Portrait != nullptr && RoleIcon != nullptr
			&& SkillIcon != nullptr
			&& Frame->GetBrush().GetResourceObject() == FrameTexture
			&& Portrait->GetBrush().GetResourceObject() == PortraitTexture
			&& RoleIcon->GetBrush().GetResourceObject() == RoleIconTexture
			&& SkillIcon->GetBrush().GetResourceObject() == SkillIconTexture
			&& Frame->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Portrait->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& RoleIcon->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& SkillIcon->GetVisibility() == ESlateVisibility::HitTestInvisible;
	};
	TestTrue(TEXT("PitchCompact binds all four Golden Sample resources"),
		HasFourBrushes(*PitchCompact)
			&& PitchCompact->GetRenderedSkillCount() == 5
			&& PitchCompact->GetRenderedAttributeCount() == 4);
	TestTrue(TEXT("InteractionChoice binds all four Golden Sample resources"),
		HasFourBrushes(*InteractionChoice)
			&& InteractionChoice->GetRenderedSkillCount() == 5
			&& InteractionChoice->GetRenderedAttributeCount() == 10
			&& InteractionChoice->GetRenderedStatusBadgeCount() == 1);
	TestTrue(TEXT("Golden Sample uses the same art identity in both modes"),
		PitchCompact->GetResolvedArtIdentity()
			== References.GetGoldenSampleArtIdentity()
			&& PitchCompact->GetResolvedArtIdentity()
				== InteractionChoice->GetResolvedArtIdentity()
			&& PitchCompact->GetResolvedRoleIconTexture()
				== RoleIconTexture
			&& InteractionChoice->GetResolvedLongShotSkillIconTexture()
				== SkillIconTexture);

	const UTextBlock* RoleText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("CardRole")));
	const UTextBlock* SkillText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("SkillIdentity0")));
	const UTextBlock* AttributeText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("AttributeValue0")));
	const UTextBlock* StatusText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("StatusBadgeLabel0")));
	TestTrue(TEXT("Golden Sample renders live Chinese role Skill attribute status"),
		RoleText != nullptr && RoleText->GetText().ToString() == TEXT("前锋 / 中场")
			&& SkillText != nullptr && SkillText->GetText().ToString().Contains(TEXT("远射"))
			&& AttributeText != nullptr && AttributeText->GetText().ToString() == TEXT("射门 5")
			&& StatusText != nullptr && StatusText->GetText().ToString() == TEXT("可用"));
	TestTrue(TEXT("UMG-only Golden Sample refresh cannot mutate authority"),
		StateBeforeWidgets == SerializeState(Host->GetMatchSnapshot().Snapshot));

	FFMCodexUMGCardViewModel MissingCard = Card;
	MissingCard.CardId = TEXT("Demo.Missing.GoldenSample");
	UFMCodexPlayerCardWidget* Fallback = MakeWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice, &MissingCard);
	if (Fallback == nullptr)
	{
		return false;
	}
	const UImage* MissingFrame = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("CardFrameAssetImage")));
	const UImage* MissingPortrait = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("PortraitAssetImage")));
	const UImage* MissingRole = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("RoleIconAssetImage")));
	const UImage* MissingSkill = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("SkillIconAssetImage0")));
	const UTextBlock* MissingRoleText = Cast<UTextBlock>(
		Fallback->GetWidgetFromName(TEXT("CardRole")));
	const UTextBlock* MissingSkillText = Cast<UTextBlock>(
		Fallback->GetWidgetFromName(TEXT("SkillIdentity0")));
	TestTrue(TEXT("Frame and portrait fallbacks remain visible and semantic"),
		MissingFrame != nullptr
			&& MissingFrame->GetVisibility() == ESlateVisibility::Collapsed
			&& MissingPortrait != nullptr
			&& MissingPortrait->GetVisibility() == ESlateVisibility::Collapsed
			&& Fallback->GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Fallback->GetWidgetFromName(TEXT("PortraitPlaceholderText"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("Missing role and Skill icons preserve Chinese text fallbacks"),
		MissingRole != nullptr
			&& MissingRole->GetVisibility() == ESlateVisibility::Collapsed
			&& MissingSkill != nullptr
			&& MissingSkill->GetVisibility() == ESlateVisibility::Collapsed
			&& MissingRoleText != nullptr
			&& MissingRoleText->GetText().ToString() == TEXT("前锋 / 中场")
			&& MissingSkillText != nullptr
			&& MissingSkillText->GetText().ToString().Contains(TEXT("远射")));

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Golden.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied) Slot.Card = Card;
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	Screen->GetPitchWidget()->RefreshFromPitchPresentation(PitchPresentation);
	TestEqual(TEXT("Golden Sample preserves the 10-slot Pitch hierarchy"),
		Screen->GetPitchWidget()->GetRenderedSlotWidgets().Num(), 10);

	FString ArtDirection;
	FString AssetReferenceSource;
	FString CardSource;
	FString TextSource;
	TestTrue(TEXT("Golden Sample visual contracts are checked in"),
		LoadProductionSource(TEXT("Docs/Visual/Visual_Art_Direction_v1.md"),
			ArtDirection)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp"),
				AssetReferenceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.cpp"),
				TextSource));
	TestTrue(TEXT("Art direction freezes four language-neutral prompt specs"),
		ArtDirection.Contains(TEXT("Exact prompt — Card Frame"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Player Portrait"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Forward Role Icon"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Long Shot Skill Icon"))
			&& ArtDirection.Contains(TEXT("no readable text")));
	TestTrue(TEXT("Golden asset paths stay centralized and presentation-only"),
		AssetReferenceSource.Contains(TEXT("/Game/UI/Cards/GoldenSample/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Portraits/GoldenSample/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Icons/GoldenSample/"))
			&& !CardSource.Contains(TEXT("/Game/UI/"))
			&& TextSource.Contains(TEXT("LOCTEXT"))
			&& !TextSource.Contains(TEXT("FMatchPlayState"))
			&& !TextSource.Contains(TEXT("AuthoritativeSession"))
			&& !TextSource.Contains(TEXT("D6Provider")));
	for (const TCHAR* RelativePath : {
		TEXT("ArtSource/UI/GoldenSample/Cards/T_Golden_CardFrame_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Portraits/T_Golden_PlayerPortrait_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Icons/T_Golden_Role_Forward_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Icons/T_Golden_Skill_LongShot_01.png") })
	{
		TestTrue(FString::Printf(TEXT("Golden source exists: %s"), RelativePath),
			FPaths::FileExists(FPaths::Combine(FPaths::ProjectDir(), RelativePath)));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexFiveSlotDragDropDeploymentIntegrationTest,
	"FMCodex.LocalPlay.ControlSurface.37.FiveSlotDragDropDeploymentIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexFiveSlotDragDropDeploymentIntegrationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	const FFMCodexLocalMatchDemoConfiguration Configuration =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const TArray<FMatchPlayDeploymentSlotDefinition>& CanonicalSlots =
		Configuration.OpeningInput.DeploymentSlotCatalog.Slots;
	int32 NearPlayerACount = 0;
	int32 NearPlayerBCount = 0;
	TSet<FName> CanonicalSlotIds;
	for (const FMatchPlayDeploymentSlotDefinition& SlotDefinition
		: CanonicalSlots)
	{
		NearPlayerACount += SlotDefinition.NeutralSide
			== EMatchPlayNeutralSlotSide::NearPlayerA ? 1 : 0;
		NearPlayerBCount += SlotDefinition.NeutralSide
			== EMatchPlayNeutralSlotSide::NearPlayerB ? 1 : 0;
		CanonicalSlotIds.Add(SlotDefinition.SlotId);
	}
	TestTrue(TEXT("Canonical demo owns exactly five slots per physical half"),
		CanonicalSlots.Num() == 10
			&& NearPlayerACount == 5
			&& NearPlayerBCount == 5
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.01"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.05"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.01"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.05"))
			&& !CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.06"))
			&& !CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.06")));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Drag/drop integration Host exists"), Host);
	TestNotNull(TEXT("Drag/drop integration Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Drag/drop integration Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}
	Screen->RequestBeginOrdinaryAttack();
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}

	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	TestNotNull(TEXT("Drag/drop integration Pitch exists"), Pitch);
	TestNotNull(TEXT("Drag/drop integration Panel exists"), Panel);
	if (Pitch == nullptr || Panel == nullptr)
	{
		return false;
	}
	const FFMCodexUMGMatchScreenViewModel& DeploymentPresentation =
		Screen->GetPresentation();
	TestTrue(TEXT("Deployment hand and pitch expose bounded product DTOs"),
		DeploymentPresentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::Deploy
			&& !DeploymentPresentation.Interaction.DeploymentChoices.IsEmpty()
			&& Panel->GetRenderedCandidateCardWidgets().Num()
				== DeploymentPresentation.Interaction.DeploymentChoices.Num()
			&& Panel->GetRenderedOptionWidgets().IsEmpty()
			&& Pitch->GetPresentation().Num() == 2
			&& Pitch->GetPresentation()[0].Slots.Num() == 5
			&& Pitch->GetPresentation()[1].Slots.Num() == 5
			&& Pitch->GetRenderedSlotWidgets().Num() == 10);

	bool bCanonicalSingleRows = Pitch->GetRenderedSlotWidgets().Num() == 10;
	for (int32 Index = 0;
		bCanonicalSingleRows && Index < Pitch->GetRenderedSlotWidgets().Num();
		++Index)
	{
		const UFMCodexPitchSlotWidget* SlotWidget =
			Pitch->GetRenderedSlotWidgets()[Index];
		const UUniformGridSlot* GridSlot = SlotWidget == nullptr
			? nullptr : Cast<UUniformGridSlot>(SlotWidget->Slot);
		bCanonicalSingleRows = GridSlot != nullptr
			&& GridSlot->GetRow() == 0
			&& GridSlot->GetColumn() == Index % 5;
	}
	TestTrue(TEXT("Rendered board is one row of five slots per half"),
		bCanonicalSingleRows);

	auto FindChoice = [](const FFMCodexUMGInteractionViewModel& Interaction,
		const bool bGoalkeeper)
		-> const FFMCodexUMGDeploymentChoiceViewModel*
	{
		return Interaction.DeploymentChoices.FindByPredicate(
			[bGoalkeeper](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
			{
				return Candidate.bGoalkeeper == bGoalkeeper
					&& !Candidate.Destinations.IsEmpty();
			});
	};
	auto FindHandCard = [](UFMCodexInteractionPanelWidget& InteractionPanel,
		const FName CardId) -> UFMCodexPlayerCardWidget*
	{
		for (UFMCodexPlayerCardWidget* CardWidget
			: InteractionPanel.GetRenderedCandidateCardWidgets())
		{
			if (CardWidget != nullptr
				&& CardWidget->GetDeploymentDragCardId() == CardId)
			{
				return CardWidget;
			}
		}
		return nullptr;
	};
	auto FindPitchSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName SlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == SlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};

	const FFMCodexUMGDeploymentChoiceViewModel* OrdinaryChoiceSource =
		FindChoice(Screen->GetPresentation().Interaction, false);
	TestNotNull(TEXT("Authoritative presentation exposes ordinary hand card"),
		OrdinaryChoiceSource);
	if (OrdinaryChoiceSource == nullptr)
	{
		return false;
	}
	const FFMCodexUMGDeploymentChoiceViewModel OrdinaryChoice =
		*OrdinaryChoiceSource;
	UFMCodexPlayerCardWidget* OrdinaryCard =
		FindHandCard(*Panel, OrdinaryChoice.CardId);
	TestNotNull(TEXT("Ordinary presentation card is a real drag source"),
		OrdinaryCard);
	if (OrdinaryCard == nullptr)
	{
		return false;
	}

	const TArray<uint8> StateBeforeCancel =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FString CommandBeforeCancel = Controller->GetLastDiagnostic().CommandName;
	UFMCodexDeploymentDragDropOperation* CancelledOperation =
		OrdinaryCard->BeginDeploymentDrag();
	TestNotNull(TEXT("Mouse drag creates typed deployment operation"),
		CancelledOperation);
	if (CancelledOperation == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Drag payload contains presentation identity only"),
		CancelledOperation->CardId == OrdinaryChoice.CardId
			&& !CancelledOperation->bGoalkeeper
			&& CancelledOperation->CardPresentation.CardId
				== OrdinaryChoice.CardId
			&& Pitch->GetActiveDeploymentCardId() == OrdinaryChoice.CardId);
	CancelledOperation->DragCancelled(FPointerEvent());
	TestTrue(TEXT("Cancelled drag emits zero command and zero State mutation"),
		Pitch->GetActiveDeploymentCardId().IsNone()
			&& Controller->GetLastDiagnostic().CommandName == CommandBeforeCancel
			&& StateBeforeCancel
				== SerializeState(Host->GetMatchSnapshot().Snapshot));

	UFMCodexDeploymentDragDropOperation* OrdinaryOperation =
		OrdinaryCard->BeginDeploymentDrag();
	if (OrdinaryOperation == nullptr)
	{
		return false;
	}
	UFMCodexPitchSlotWidget* OrdinaryTarget = FindPitchSlot(
		*Pitch, OrdinaryChoice.Destinations[0].SlotId);
	TestNotNull(TEXT("Every ordinary destination maps to a rendered drop target"),
		OrdinaryTarget);
	if (OrdinaryTarget == nullptr)
	{
		return false;
	}
	const int32 PlacementsBeforeOrdinary =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.DeploymentPlacements.Num();
	TestTrue(TEXT("Ordinary destination is projected valid without rule queries"),
		OrdinaryTarget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Valid
			&& OrdinaryTarget->CanAcceptDeploymentOperation(OrdinaryOperation));
	const bool bOrdinaryDropAccepted =
		OrdinaryTarget->TryHandleDeploymentDrop(OrdinaryOperation);
	TestTrue(TEXT("Ordinary drop follows typed authoritative command chain"),
		bOrdinaryDropAccepted
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("DeployOrdinary")
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.Num() == PlacementsBeforeOrdinary + 1
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.ContainsByPredicate(
					[&OrdinaryChoice](
						const FMatchPlayDeploymentPlacement& Placement)
					{
						return Placement.CardId == OrdinaryChoice.CardId
							&& Placement.SlotId
								== OrdinaryChoice.Destinations[0].SlotId;
					}));
	OrdinaryOperation->Drop(FPointerEvent());
	if (Controller->IsAwaitingHotSeatHandoff())
	{
		Screen->RequestReady();
	}

	const FFMCodexUMGDeploymentDestinationViewModel* StaleDestination =
		OrdinaryChoice.Destinations.FindByPredicate(
			[&OrdinaryChoice](
				const FFMCodexUMGDeploymentDestinationViewModel& Destination)
			{
				return Destination.SlotId
					!= OrdinaryChoice.Destinations[0].SlotId;
			});
	TestNotNull(TEXT("Ordinary fixture retains an empty stale destination"),
		StaleDestination);
	if (StaleDestination == nullptr)
	{
		return false;
	}
	Pitch->BeginDeploymentDrag(
		OrdinaryChoice.CardId, { OrdinaryChoice });
	UFMCodexPitchSlotWidget* StaleTarget =
		FindPitchSlot(*Pitch, StaleDestination->SlotId);
	TestNotNull(TEXT("Stale presentation projects a rendered drop target"),
		StaleTarget);
	if (StaleTarget == nullptr)
	{
		return false;
	}
	const TArray<uint8> StateBeforeStaleDrop =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const bool bStaleDropLocallyAccepted =
		StaleTarget->TryHandleDeploymentDrop(OrdinaryOperation);
	TestTrue(TEXT("Stale rendered target still emits its typed intent"),
		bStaleDropLocallyAccepted);
	TestEqual(TEXT("Stale typed intent remains ordinary deployment"),
		Controller->GetLastDiagnostic().CommandName,
		FString(TEXT("DeployOrdinary")));
	TestFalse(TEXT("Authority rejects the stale ordinary deployment"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Stale authoritative rejection preserves State"),
		StateBeforeStaleDrop
			== SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestTrue(TEXT("Stale rejection refreshes failure and clears drag projection"),
		Screen->GetPresentation().Resolution.bRejected
			&& Pitch->GetActiveDeploymentCardId().IsNone());

	const FFMCodexUMGDeploymentChoiceViewModel* GoalkeeperChoiceSource =
		FindChoice(Screen->GetPresentation().Interaction, true);
	TestNotNull(TEXT("Next authoritative hand exposes goalkeeper choice"),
		GoalkeeperChoiceSource);
	if (GoalkeeperChoiceSource == nullptr)
	{
		return false;
	}
	const FFMCodexUMGDeploymentChoiceViewModel GoalkeeperChoice =
		*GoalkeeperChoiceSource;
	UFMCodexPlayerCardWidget* GoalkeeperCard =
		FindHandCard(*Panel, GoalkeeperChoice.CardId);
	TestNotNull(TEXT("Goalkeeper presentation card is a real drag source"),
		GoalkeeperCard);
	if (GoalkeeperCard == nullptr)
	{
		return false;
	}
	UFMCodexDeploymentDragDropOperation* GoalkeeperOperation =
		GoalkeeperCard->BeginDeploymentDrag();
	if (GoalkeeperOperation == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Goalkeeper drag retains presentation identity and type"),
		GoalkeeperOperation->CardId == GoalkeeperChoice.CardId
			&& GoalkeeperOperation->bGoalkeeper);

	UFMCodexPitchSlotWidget* InvalidTarget = nullptr;
	UFMCodexPitchSlotWidget* OccupiedTarget = nullptr;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		if (SlotWidget == nullptr)
		{
			continue;
		}
		if (SlotWidget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Invalid)
		{
			InvalidTarget = SlotWidget;
		}
		if (SlotWidget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Occupied)
		{
			OccupiedTarget = SlotWidget;
		}
	}
	TestNotNull(TEXT("Goalkeeper projection marks non-destination invalid"),
		InvalidTarget);
	TestNotNull(TEXT("Goalkeeper projection preserves occupied target state"),
		OccupiedTarget);
	if (InvalidTarget == nullptr)
	{
		return false;
	}
	const TArray<uint8> StateBeforeInvalidDrop =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FString CommandBeforeInvalidDrop =
		Controller->GetLastDiagnostic().CommandName;
	TestTrue(TEXT("Invalid drop emits zero command and zero State mutation"),
		!InvalidTarget->TryHandleDeploymentDrop(GoalkeeperOperation)
			&& Controller->GetLastDiagnostic().CommandName
				== CommandBeforeInvalidDrop
			&& StateBeforeInvalidDrop
				== SerializeState(Host->GetMatchSnapshot().Snapshot));

	UFMCodexPitchSlotWidget* GoalkeeperTarget = FindPitchSlot(
		*Pitch, GoalkeeperChoice.Destinations[0].SlotId);
	TestNotNull(TEXT("Goalkeeper destination maps to a rendered drop target"),
		GoalkeeperTarget);
	if (GoalkeeperTarget == nullptr)
	{
		return false;
	}
	const int32 PlacementsBeforeGoalkeeper =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.DeploymentPlacements.Num();
	const bool bGoalkeeperDropAccepted =
		GoalkeeperTarget->TryHandleDeploymentDrop(GoalkeeperOperation);
	TestTrue(TEXT("Goalkeeper drop routes slot-only typed authoritative command"),
		bGoalkeeperDropAccepted
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("DeployGoalkeeper")
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.Num() == PlacementsBeforeGoalkeeper + 1
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.ContainsByPredicate(
					[&GoalkeeperChoice](
						const FMatchPlayDeploymentPlacement& Placement)
					{
						return Placement.CardId == GoalkeeperChoice.CardId
							&& Placement.SlotId
								== GoalkeeperChoice.Destinations[0].SlotId;
					}));
	GoalkeeperOperation->Drop(FPointerEvent());

	FFMCodexUMGPitchSlotViewModel ProjectionSlot;
	ProjectionSlot.SlotId = TEXT("Projection.Valid");
	FFMCodexUMGDeploymentChoiceViewModel ProjectionChoice;
	ProjectionChoice.CardId = TEXT("Projection.Card");
	ProjectionChoice.Destinations.Add(
		{ ProjectionSlot.SlotId, TEXT("Projection destination") });
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bValidProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Valid;
	ProjectionSlot.SlotId = TEXT("Projection.Invalid");
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bInvalidProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Invalid;
	ProjectionSlot.bOccupied = true;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bOccupiedProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Occupied;
	ProjectionSlot.bOccupied = false;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, TEXT("Projection.Unknown"), { ProjectionChoice });
	const bool bUnavailableProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Unavailable;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, NAME_None, { ProjectionChoice });
	TestTrue(TEXT("Presentation projector covers all deployment target states"),
		bValidProjection && bInvalidProjection && bOccupiedProjection
			&& bUnavailableProjection
			&& ProjectionSlot.DeploymentTargetState
				== EFMCodexUMGDeploymentTargetState::Neutral);

	FString DragOperationHeader;
	FString CardWidgetSource;
	FString PitchSlotSource;
	FString PitchWidgetSource;
	FString ScreenSource;
	FString ControllerSource;
	FString HostSource;
	FString SessionSource;
	TestTrue(TEXT("Deployment drag/drop authority boundary sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexDeploymentDragDropOperation.h"),
			DragOperationHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				PitchSlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"),
				PitchWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ScreenSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
				HostSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
				SessionSource));
	const FString DragDropSources = DragOperationHeader + CardWidgetSource
		+ PitchSlotSource + PitchWidgetSource;
	TestTrue(TEXT("Drag/drop layer contains no gameplay authority or legality"),
		!DragDropSources.Contains(TEXT("FMatchPlayState"))
			&& !DragDropSources.Contains(TEXT("AuthoritativeSession"))
			&& !DragDropSources.Contains(TEXT("LocalMatchHost"))
			&& !DragDropSources.Contains(TEXT("RelativeDeploymentZoneResolver"))
			&& !DragDropSources.Contains(TEXT("Legality"))
			&& !DragDropSources.Contains(TEXT("Formula")));
	TestTrue(TEXT("Screen preserves explicit ordinary and slot-only GK routing"),
		ScreenSource.Contains(TEXT("RequestDeployOrdinary(CardId, SlotId)"))
			&& ScreenSource.Contains(TEXT("RequestDeployGoalkeeper(SlotId)"))
			&& !ScreenSource.Contains(TEXT("RequestDeployGoalkeeper(CardId"))
			&& ControllerSource.Contains(TEXT("Host->DeployOrdinary(Request)"))
			&& ControllerSource.Contains(TEXT("Host->DeployGoalkeeper(Request)"))
			&& HostSource.Contains(
				TEXT("AuthoritativeSession.DeployGoalkeeper(Request)")));
	FString SessionCountSource = SessionSource;
	const int32 SerializedEntrypointCount = SessionCountSource.ReplaceInline(
		TEXT("ExecuteSerialized<"), TEXT(""), ESearchCase::CaseSensitive);
	TestEqual(TEXT("Authoritative Session serialized entrypoint count is unchanged"),
		SerializedEntrypointCount, 42);

	return true;
}

#endif
