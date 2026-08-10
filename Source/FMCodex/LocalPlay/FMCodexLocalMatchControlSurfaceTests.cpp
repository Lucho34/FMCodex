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

		for (int32 Step = 0; Step < 6; ++Step)
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

#endif
