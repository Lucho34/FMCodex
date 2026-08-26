#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexThroughBallProductionPresentationTests
{
	FFMCodexLocalMatchInteractionView MakeView();

	FMatchPlayCurrentAttackResolutionFactProjection MakeFacts(
		const EMatchPlayThroughBallActualBranch Route,
		const int32 RawD6,
		const int64 AttackSequence = 41)
	{
		FMatchPlayCurrentAttackResolutionFactProjection Facts;
		Facts.bSuccess = true;
		Facts.bHasFacts = true;
		Facts.AttackSequence = AttackSequence;
		Facts.ActionType = ESkillRuleType::ThroughBall;
		Facts.bHasActualBranch = true;
		Facts.ActualBranch.ActionType = ESkillRuleType::ThroughBall;
		Facts.ActualBranch.ThroughBall = Route;

		FMatchPlayResolutionRollFact RouteRoll;
		RouteRoll.SequenceIndex = 0;
		RouteRoll.OperandId = TEXT("InitialRouteD6");
		RouteRoll.bInitialRoute = true;
		RouteRoll.InitialPurpose =
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
		RouteRoll.Semantics =
			EMatchPlayResolutionRollSemantics::BranchSelection;
		RouteRoll.OwningSide = EInitialTurnOrderPlayer::PlayerA;
		RouteRoll.bResolved = true;
		RouteRoll.RawD6 = RawD6;
		Facts.Rolls.Add(RouteRoll);
		return Facts;
	}

	FMatchPlayCurrentAttackResolutionFactProjection MakeFeetFormulaFacts(
		const bool bAttackResolved,
		const bool bDefenseResolved,
		const EFormulaWinner Winner = EFormulaWinner::None)
	{
		using ETermKind = EMatchPlayResolutionFormulaTermKind;
		using ERollPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
		FMatchPlayCurrentAttackResolutionFactProjection Facts = MakeFacts(
			EMatchPlayThroughBallActualBranch::Feet, 2);
		const FName CarrierId(TEXT("Fixture.Feet.Carrier"));
		const FName RunnerId(TEXT("Fixture.Feet.Runner"));
		const FName MarkerId(TEXT("Fixture.Feet.Marker"));
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId });
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA, RunnerId });
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId });

		auto AddRoll = [&Facts](
			const int32 SequenceIndex,
			const ERollPurpose Purpose,
			const EInitialTurnOrderPlayer Side,
			const bool bResolved,
			const int32 RawD6)
		{
			FMatchPlayResolutionRollFact Roll;
			Roll.SequenceIndex = SequenceIndex;
			Roll.OperandId = Purpose == ERollPurpose::PrimaryAttack
				? FName(TEXT("PrimaryAttackD6"))
				: FName(TEXT("PrimaryDefenseD6"));
			Roll.PostRoutePurpose = Purpose;
			Roll.Semantics =
				EMatchPlayResolutionRollSemantics::ArithmeticContest;
			Roll.OwningSide = Side;
			Roll.bResolved = bResolved;
			Roll.RawD6 = bResolved ? RawD6 : 0;
			Facts.Rolls.Add(Roll);
		};
		AddRoll(1, ERollPurpose::PrimaryAttack,
			EInitialTurnOrderPlayer::PlayerA, bAttackResolved, 5);
		AddRoll(2, ERollPurpose::PrimaryDefense,
			EInitialTurnOrderPlayer::PlayerB, bDefenseResolved, 3);
		Facts.bHasPendingRoll = !bAttackResolved || !bDefenseResolved;
		Facts.NextPendingRollSequenceIndex = !bAttackResolved
			? 1 : !bDefenseResolved ? 2 : INDEX_NONE;

		auto AttributeTerm = [](
			const FName TermId,
			const EMatchPlayResolutionParticipantRole Role,
			const EInitialTurnOrderPlayer Side,
			const FName CardId,
			const EMatchPlayResolutionFormulaAttribute Attribute,
			const float Value)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.TermId = TermId;
			Term.Kind = ETermKind::Attribute;
			Term.ParticipantRole = Role;
			Term.Side = Side;
			Term.CardId = CardId;
			Term.Attribute = Attribute;
			Term.SourceValue = Value;
			Term.Multiplier = 1.0f;
			Term.Contribution = Value;
			Term.bResolved = true;
			return Term;
		};
		auto RollTerm = [](const int32 SequenceIndex,
			const bool bResolved, const int32 RawD6)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.TermId = SequenceIndex == 1
				? FName(TEXT("PrimaryAttackD6"))
				: FName(TEXT("PrimaryDefenseD6"));
			Term.Kind = ETermKind::RawRoll;
			Term.RollSequenceIndex = SequenceIndex;
			Term.bResolved = bResolved;
			Term.SourceValue = bResolved ? RawD6 : 0;
			Term.Contribution = Term.SourceValue;
			return Term;
		};

		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = TEXT("ThroughBall.Feet");
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = bAttackResolved && bDefenseResolved
			? EMatchPlayResolutionFormulaApplication::Applied
			: EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.RowId = TEXT("ThroughBall.Feet.Attack");
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms.Add(AttributeTerm(
			TEXT("Carrier.Passing"),
			EMatchPlayResolutionParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId,
			EMatchPlayResolutionFormulaAttribute::Passing, 4.5f));
		Contest.AttackRow.Terms.Add(RollTerm(1, bAttackResolved, 5));
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 4.5f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved ? 9.5f : 0.0f;
		Contest.DefenseRow.RowId = TEXT("ThroughBall.Feet.Defense");
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms.Add(AttributeTerm(
			TEXT("Marker.Tackling"),
			EMatchPlayResolutionParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId,
			EMatchPlayResolutionFormulaAttribute::Tackling, 5.5f));
		Contest.DefenseRow.Terms.Add(RollTerm(2, bDefenseResolved, 3));
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = 5.5f;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved ? 8.5f : 0.0f;
		Contest.bHasResolvedFormula = bAttackResolved && bDefenseResolved;
		if (Contest.bHasResolvedFormula && Winner != EFormulaWinner::None)
		{
			Contest.ResolvedResult.FormulaType = EFormulaType::Finishing;
			Contest.ResolvedResult.Winner = Winner;
			Contest.ResolvedResult.bAttackEnded = true;
			Contest.ResolvedResult.bContinueResolution = false;
		}
		Facts.FormulaContests.Add(Contest);
		return Facts;
	}

	FFMCodexLocalMatchInteractionView MakeFeetView(
		const bool bAttackResolved,
		const bool bDefenseResolved,
		const bool bTerminal = false,
		const EFormulaWinner TerminalWinner = EFormulaWinner::Attacker)
	{
		FFMCodexLocalMatchInteractionView View = MakeView();
		View.ResolutionFacts = MakeFeetFormulaFacts(
			bAttackResolved, bDefenseResolved,
			bTerminal ? TerminalWinner : EFormulaWinner::None);
		auto AddRosterCard = [&View](
			const EInitialTurnOrderPlayer Side,
			const FName CardId,
			const FString& DisplayLabel)
		{
			FFMCodexLocalMatchCardView Card;
			Card.Side = Side;
			Card.CardId = CardId;
			Card.DisplayLabel = DisplayLabel;
			(Side == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster).Add(Card);
		};
		AddRosterCard(EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.Feet.Carrier"), TEXT("厄德高"));
		AddRosterCard(EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.Feet.Runner"), TEXT("哈兰德"));
		AddRosterCard(EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.Feet.Marker"), TEXT("萨利巴"));
		if (!bAttackResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallFeetAttack;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.bThroughBallFeetAttackRollPending = true;
			View.ContinueActionLabel = TEXT("进攻方掷点");
		}
		else if (!bDefenseResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallFeetDefense;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
			View.bThroughBallFeetDefenseRollPending = true;
			View.ContinueActionLabel = TEXT("防守方掷点");
		}
		else
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::AdvanceAfterTerminal;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.bThroughBallFeetFormulaComplete = true;
			View.bTerminalPendingAdvance = true;
			View.ContinueActionLabel = TEXT("下一回合");
		}
		return View;
	}

	FFMCodexLocalMatchInteractionView MakeView()
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.AttackSequence = 41;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		View.PresentedActionType = ESkillRuleType::ThroughBall;
		View.ActionLabel = TEXT("Through Ball");
		View.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::ContinueResolution;
		View.ContinueActionLabel = TEXT("判定直塞路线");
		return View;
	}

	FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& View,
		const bool bRejected = false)
	{
		FFMCodexLocalMatchResolutionFeedback Feedback;
		Feedback.bVisible = true;
		Feedback.bRejected = bRejected;
		Feedback.StepTitle = TEXT("POST-ROUTE ENGINEERING STEP");
		Feedback.StepSummary = TEXT("CONTINUES INTERNAL STATE");
		Feedback.ErrorMessage = bRejected
			? TEXT("Authoritative rejection") : FString();
		Feedback.ResolutionFacts = View.ResolutionFacts;
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Feedback, FString(), EInitialTurnOrderPlayer::PlayerA);
	}

	FString Flatten(
		const FFMCodexUMGThroughBallResolutionViewModel& Surface)
	{
		FString Result = Surface.TitleLabel + Surface.RouteLabel
			+ Surface.StageLabel + Surface.StatusLabel
			+ Surface.RouteResultLabel + Surface.ActionPromptLabel
			+ Surface.ContinueActionLabel;
		for (const FFMCodexUMGOneOnOneChoiceViewModel& Choice
			: Surface.OneOnOneChoices)
		{
			Result += Choice.Label;
		}
		return Result;
	}

	int32 CountOccurrences(const FString& Text, const FString& Needle)
	{
		int32 Count = 0;
		int32 SearchFrom = 0;
		while ((SearchFrom = Text.Find(
			Needle,
			ESearchCase::CaseSensitive,
			ESearchDir::FromStart,
			SearchFrom)) != INDEX_NONE)
		{
			++Count;
			SearchFrom += Needle.Len();
		}
		return Count;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionSemanticSurfaceTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.SemanticSurfaceAndDebugIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionSemanticSurfaceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexThroughBallProductionPresentationTests;
	(void)Parameters;

	const FFMCodexLocalMatchInteractionView PreRouteView = MakeView();
	const FFMCodexUMGMatchScreenViewModel PreRoute = Build(PreRouteView);
	TestTrue(TEXT("Typed ThroughBall route entry owns a production shell"),
		PreRoute.ThroughBallResolution.bVisible
			&& PreRoute.ThroughBallResolution.bSuppressLegacyResolution
			&& PreRoute.ThroughBallResolution.Stage
				== EFMCodexUMGThroughBallStage::InitialRoute
			&& PreRoute.ThroughBallResolution.StageLabel
				== TEXT("判定直塞路线")
			&& PreRoute.ThroughBallResolution.StatusLabel.IsEmpty()
			&& PreRoute.ThroughBallResolution.ActionPromptLabel.IsEmpty()
			&& PreRoute.ThroughBallResolution.bPrimaryActionOwnedBySurface
			&& PreRoute.ThroughBallResolution.bCanContinue
			&& PreRoute.ThroughBallResolution.ContinueActionLabel
				== TEXT("掷点判定路线")
			&& !PreRoute.ThroughBallResolution.Formula.bVisible
			&& PreRoute.ThroughBallResolution.Formula
				.RouteResultLabel.IsEmpty()
			&& PreRoute.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute
			&& PreRoute.Interaction.CrossRollContestId
				== TEXT("ThroughBall.Route")
			&& PreRoute.Interaction.CrossRollSequenceIndex == 0
			&& PreRoute.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Initial-route semantic instruction appears once"),
		CountOccurrences(
			Flatten(PreRoute.ThroughBallResolution), TEXT("判定直塞路线")),
		1);

	struct FRouteExpectation
	{
		EMatchPlayThroughBallActualBranch Canonical;
		int32 RawD6;
		EFMCodexUMGThroughBallRoute Route;
		EFMCodexUMGThroughBallStage Stage;
		const TCHAR* RouteLabel;
		const TCHAR* StageLabel;
	};
	const FRouteExpectation Expectations[] =
	{
		{ EMatchPlayThroughBallActualBranch::Feet, 1,
			EFMCodexUMGThroughBallRoute::Feet,
			EFMCodexUMGThroughBallStage::FeetContest,
			TEXT("脚下球"), TEXT("属性对抗") },
		{ EMatchPlayThroughBallActualBranch::BehindDefense, 4,
			EFMCodexUMGThroughBallRoute::BehindDefense,
			EFMCodexUMGThroughBallStage::BehindDefenseFirstStage,
			TEXT("身后球"), TEXT("第一阶段") },
		{ EMatchPlayThroughBallActualBranch::AntiOffside, 6,
			EFMCodexUMGThroughBallRoute::AntiOffside,
			EFMCodexUMGThroughBallStage::AntiOffsideCheck,
			TEXT("反越位"), TEXT("越位判定") }
	};
	for (const FRouteExpectation& Expected : Expectations)
	{
		FFMCodexLocalMatchInteractionView RouteView = MakeView();
		RouteView.ResolutionFacts = MakeFacts(
			Expected.Canonical, Expected.RawD6);
		const FFMCodexUMGThroughBallResolutionViewModel& Surface =
			Build(RouteView).ThroughBallResolution;
		TestTrue(FString::Printf(TEXT("%s projects canonical semantic route"),
			Expected.RouteLabel),
			Surface.Route == Expected.Route
				&& Surface.Stage == Expected.Stage
				&& Surface.RouteLabel == Expected.RouteLabel
				&& Surface.StageLabel == Expected.StageLabel
				&& Surface.bHasAuthoritativeInitialRouteRoll
				&& Surface.AuthoritativeInitialRouteD6 == Expected.RawD6
				&& Surface.RouteResultLabel.Contains(
					FString::FromInt(Expected.RawD6)));
	}

	FFMCodexLocalMatchInteractionView OneOnOneView = MakeView();
	OneOnOneView.ResolutionFacts = MakeFacts(
		EMatchPlayThroughBallActualBranch::BehindDefense, 3);
	OneOnOneView.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot;
	OneOnOneView.OneOnOneOptions =
	{
		EMatchPlayThroughBallOneOnOneShotChoice::DirectShot,
		EMatchPlayThroughBallOneOnOneShotChoice::ChipShot
	};
	const FFMCodexUMGMatchScreenViewModel OneOnOne = Build(OneOnOneView);
	TestTrue(TEXT("BehindDefense success exposes exactly two typed shot choices"),
		OneOnOne.ThroughBallResolution.Stage
			== EFMCodexUMGThroughBallStage::OneOnOneChoice
			&& OneOnOne.ThroughBallResolution.OneOnOneChoices.Num() == 2
			&& OneOnOne.ThroughBallResolution.ActionPromptLabel
				== TEXT("直接射门或挑射")
			&& OneOnOne.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("Production surface does not leak engineering diagnostics"),
		!Flatten(OneOnOne.ThroughBallResolution).Contains(TEXT("POST-ROUTE"))
			&& !Flatten(OneOnOne.ThroughBallResolution).Contains(
				TEXT("CONTINUES"))
			&& !Flatten(OneOnOne.ThroughBallResolution).Contains(
				TEXT("ENGINEERING")));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Production ThroughBall screen can be constructed"), Screen);
	if (Screen != nullptr)
	{
		Screen->TakeWidget();
		Screen->RefreshFromPresentation(PreRoute);
		TestTrue(TEXT("Central surface owns the only initial-route primary CTA"),
			Screen->GetThroughBallResolutionSurface() != nullptr
				&& Screen->GetThroughBallResolutionSurface()
					->GetPresentation().bCanContinue
				&& Screen->GetThroughBallResolutionSurface()->GetWidgetFromName(
					TEXT("ThroughBallPrimaryActionButton")) != nullptr
				&& Screen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Collapsed);
		Screen->RefreshFromPresentation(OneOnOne);
		TestTrue(TEXT("Normal ThroughBall hides generic resolution debug surface"),
			Screen->GetThroughBallResolutionSurface() != nullptr
				&& Screen->GetThroughBallResolutionSurface()
					->GetPresentation().bVisible
				&& !Screen->IsLegacyResolutionOverlayVisible());
		Screen->RefreshFromPresentation(Build(OneOnOneView, true));
		TestTrue(TEXT("Rejected authority result retains legacy diagnostic surface"),
			Screen->IsLegacyResolutionOverlayVisible());
	}

	FFMCodexLocalMatchInteractionView CrossView = MakeView();
	CrossView.PresentedActionType = ESkillRuleType::Cross;
	CrossView.ActionLabel = TEXT("Cross");
	const FFMCodexUMGMatchScreenViewModel Cross = Build(CrossView);
	TestFalse(TEXT("Non-ThroughBall flow does not activate the new shell"),
		Cross.ThroughBallResolution.bVisible);

	FString WidgetSource;
	const FString WidgetPath = FPaths::Combine(FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexThroughBallResolutionSurfaceWidget.cpp"));
	TestTrue(TEXT("Production widget source is readable for boundary guard"),
		FFileHelper::LoadFileToString(WidgetSource, *WidgetPath));
	TestTrue(TEXT("Thin widget reuses RollReel and owns no rules, RNG, or authority"),
		WidgetSource.Contains(TEXT("UFMCodexRollReelWidget::StaticClass"))
			&& !WidgetSource.Contains(TEXT("RawD6 <="))
			&& !WidgetSource.Contains(TEXT("RawD6 >="))
			&& !WidgetSource.Contains(TEXT("FMath::Rand"))
			&& !WidgetSource.Contains(TEXT("FRandomStream"))
			&& !WidgetSource.Contains(TEXT("RollD6"))
			&& !WidgetSource.Contains(TEXT("FormulaResolver"))
			&& !WidgetSource.Contains(TEXT("ActualBranch")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionSharedRevealTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.SharedRouteReelAndResync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionSharedRevealTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexThroughBallProductionPresentationTests;
	(void)Parameters;

	const FFMCodexUMGMatchScreenViewModel Pending = Build(MakeView());
	FFMCodexLocalMatchInteractionView ResolvedView = MakeView();
	ResolvedView.ResolutionFacts = MakeFacts(
		EMatchPlayThroughBallActualBranch::BehindDefense, 4);
	ResolvedView.ContinueActionLabel = TEXT("继续直塞结算");
	const FFMCodexUMGMatchScreenViewModel Resolved = Build(ResolvedView);

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Shared reveal fixture can be constructed"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Pending);
	Screen->RefreshFromPresentation(Resolved);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexThroughBallResolutionSurfaceWidget* Surface =
		Screen->GetThroughBallResolutionSurface();
	UFMCodexRollReelWidget* Reel = Surface != nullptr
		? Surface->GetRollReelWidget() : nullptr;
	TestTrue(TEXT("ThroughBall route uses the unified clipped D6 reel"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& Surface != nullptr && Surface->GetPresentation().bVisible
			&& Reel != nullptr && Reel->HasClippedWindow()
			&& Reel->GetRenderedChildCount() == 3
			&& Reel->GetPresentation().DomainMinimum == 1
			&& Reel->GetPresentation().DomainMaximum == 6
			&& Surface->GetPresentation().RouteLabel.IsEmpty()
			&& !Screen->GetInlineFormulaSurface()->GetPresentation().bVisible
			&& !Screen->IsLegacyResolutionOverlayVisible());

	Screen->AdvanceInlineFormulaRevealForTesting(1.30f);
	TestEqual(TEXT("Shared timing enters the existing settle phase"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settling);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestTrue(TEXT("Authority raw 4 lands before BehindDefense is disclosed"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& Reel->GetPresentation().CenterValue == 4
			&& Reel->GetPresentation().bAuthoritativeValue
			&& Reel->GetPresentation().bStaticResult
			&& Surface->GetPresentation().RouteLabel == TEXT("身后球")
			&& Surface->GetPresentation().RouteResultLabel
				== TEXT("掷点 4 → 身后球")
			&& Screen->IsInlineFormulaRevealInputBlocked());
	Screen->RefreshFromPresentation(Resolved);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestEqual(TEXT("Rebuild during result hold does not restart reveal"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::ResultHold);
	Screen->AdvanceInlineFormulaRevealForTesting(1.45f);
	TestTrue(TEXT("Settled identity releases once and repeated truth never replays"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& !Screen->IsInlineFormulaRevealInputBlocked()
			&& Surface->GetPresentation().RouteLabel == TEXT("身后球"));
	Screen->RefreshFromPresentation(Resolved);
	TestEqual(TEXT("Repeated completed presentation remains settled"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settled);

	UFMCodexLocalMatchScreenWidget* Reconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Reconstructed->TakeWidget();
	Reconstructed->RefreshFromPresentation(Resolved);
	TestTrue(TEXT("Already-resolved first observation renders without replay"),
		Reconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !Reconstructed->IsInlineFormulaRevealInputBlocked()
			&& Reconstructed->GetThroughBallResolutionSurface()
				->GetPresentation().RouteLabel == TEXT("身后球")
			&& !Reconstructed->IsLegacyResolutionOverlayVisible());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionFeetFormulaFlowTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.FeetFormulaRevealTerminalAndResync",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionFeetFormulaFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexThroughBallProductionPresentationTests;
	(void)Parameters;
	auto RawRoll = [](const FFMCodexUMGInlineFormulaRowViewModel& Row)
	{
		return Row.Terms.FindByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind
					== EFMCodexUMGInlineFormulaTermKind::RawRoll;
			});
	};

	const FFMCodexUMGMatchScreenViewModel Preview = Build(
		MakeFeetView(false, false));
	const FFMCodexUMGInlineFormulaSurfaceViewModel& PreviewFormula =
		Preview.ThroughBallResolution.Formula;
	const FFMCodexUMGInlineFormulaTermViewModel* PreviewAttackRoll =
		RawRoll(PreviewFormula.AttackRow);
	const FFMCodexUMGInlineFormulaTermViewModel* PreviewDefenseRoll =
		RawRoll(PreviewFormula.DefenseRow);
	TestTrue(TEXT("Feet preview composes the authoritative shared formula DTO"),
		Preview.ThroughBallResolution.bVisible
			&& Preview.ThroughBallResolution.Stage
				== EFMCodexUMGThroughBallStage::FeetContest
			&& PreviewFormula.bVisible
			&& PreviewFormula.ContestId == TEXT("ThroughBall.Feet")
			&& PreviewFormula.RouteResultLabel
				== TEXT("路线掷点 2 → 判定为脚下球")
			&& FMath::IsNearlyEqual(
				PreviewFormula.AttackRow.KnownNonRollSubtotal, 4.5f)
			&& FMath::IsNearlyEqual(
				PreviewFormula.DefenseRow.KnownNonRollSubtotal, 5.5f)
			&& PreviewAttackRoll != nullptr && !PreviewAttackRoll->bResolved
			&& PreviewAttackRoll->bNextPendingRoll
			&& PreviewDefenseRoll != nullptr && !PreviewDefenseRoll->bResolved
			&& PreviewFormula.AttackRow.Terms.Num() == 2
			&& PreviewFormula.AttackRow.Terms[0].ContributorDisplayName
				== TEXT("厄德高")
			&& PreviewFormula.AttackRow.Terms[0].DisplayLabel
				== TEXT("传球 4.5")
			&& PreviewFormula.DefenseRow.Terms.Num() == 2
			&& PreviewFormula.DefenseRow.Terms[0].ContributorDisplayName
				== TEXT("萨利巴")
			&& PreviewFormula.DefenseRow.Terms[0].DisplayLabel
				== TEXT("抢断 5.5")
			&& PreviewAttackRoll->ContributorDisplayName.IsEmpty()
			&& PreviewDefenseRoll->ContributorDisplayName.IsEmpty()
			&& PreviewFormula.bCanContinue
			&& PreviewFormula.ContinueActionLabel == TEXT("进攻方掷点")
			&& Preview.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::Attack
			&& Preview.Interaction.CrossRollContestId
				== TEXT("ThroughBall.Feet")
			&& Preview.Interaction.CrossRollSequenceIndex == 1
			&& Preview.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerA
			&& Preview.Interaction.bPrimaryActionOwnedByInlineFormula
			&& !Preview.Interaction.bCanContinue
			&& !Preview.InlineFormula.bVisible);

	const FFMCodexUMGMatchScreenViewModel AttackComplete = Build(
		MakeFeetView(true, false));
	TestTrue(TEXT("Attack truth projects a distinct defender reveal identity"),
		AttackComplete.Interaction.CrossRollRevealKind
			== EFMCodexUMGCrossRollRevealKind::Defense
			&& AttackComplete.Interaction.CrossRollContestId
				== TEXT("ThroughBall.Feet")
			&& AttackComplete.Interaction.CrossRollSequenceIndex == 2
			&& AttackComplete.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerB
			&& AttackComplete.ThroughBallResolution.Formula.RouteResultLabel
				== TEXT("路线掷点 2 → 判定为脚下球"));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Feet production screen can be constructed"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Preview);
	UFMCodexThroughBallResolutionSurfaceWidget* Surface =
		Screen->GetThroughBallResolutionSurface();
	UFMCodexInlineResolutionFormulaSurfaceWidget* FormulaSurface =
		Surface != nullptr ? Surface->GetFormulaSurface() : nullptr;
	TestTrue(TEXT("Feet owns the central shared formula CTA without lower duplicate"),
		Surface != nullptr && FormulaSurface != nullptr
			&& FormulaSurface->GetPresentation().bVisible
			&& FormulaSurface->GetPresentation().bCanContinue
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !Screen->IsLegacyResolutionOverlayVisible());

	Screen->RefreshFromPresentation(AttackComplete);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Accepted attack roll starts the shared reel and gates defense"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& FormulaSurface->GetPresentation().bDiceRevealVisible
			&& !FormulaSurface->GetPresentation().bCanContinue
			&& FormulaSurface->GetRollReelWidget() != nullptr
			&& FormulaSurface->GetRollReelWidget()->HasClippedWindow());
	Screen->AdvanceInlineFormulaRevealForTesting(1.30f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.18f);
	const FFMCodexUMGInlineFormulaTermViewModel* DisclosedAttackRoll =
		RawRoll(FormulaSurface->GetPresentation().AttackRow);
	TestTrue(TEXT("Attack result discloses authority RawD6 and FinalValue only after settle"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& DisclosedAttackRoll != nullptr && DisclosedAttackRoll->bResolved
			&& DisclosedAttackRoll->RawD6 == 5
			&& FormulaSurface->GetPresentation().AttackRow.bFinalValueResolved
			&& FMath::IsNearlyEqual(
				FormulaSurface->GetPresentation().AttackRow.FinalValue, 9.5f)
			&& !FormulaSurface->GetPresentation().bCanContinue);
	Screen->AdvanceInlineFormulaRevealForTesting(2.40f);
	TestTrue(TEXT("Attack stable reveal settles before defender CTA is exposed"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& !Screen->IsInlineFormulaRevealInputBlocked()
			&& FormulaSurface->GetPresentation().bCanContinue
			&& FormulaSurface->GetPresentation().ContinueActionLabel
				== TEXT("防守方掷点"));

	UFMCodexLocalMatchScreenWidget* AttackReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	AttackReconstructed->TakeWidget();
	AttackReconstructed->RefreshFromPresentation(AttackComplete);
	TestTrue(TEXT("Fresh attack-complete screen does not replay historical attack"),
		!AttackReconstructed->IsInlineFormulaRevealInputBlocked()
			&& AttackReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation()
				.AttackRow.bFinalValueResolved
			&& AttackReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("防守方掷点"));

	const FFMCodexUMGMatchScreenViewModel Terminal = Build(
		MakeFeetView(true, true, true));
	Screen->RefreshFromPresentation(Terminal);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Accepted defense roll gates terminal result and NextRound"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& FormulaSurface->GetPresentation().bDiceRevealVisible
			&& !FormulaSurface->GetPresentation().bCanContinue
			&& FormulaSurface->GetPresentation().ResultSubtitle.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(1.30f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.18f);
	TestTrue(TEXT("Defense formula discloses before the concise result"),
		FormulaSurface->GetPresentation().DefenseRow.bFinalValueResolved
			&& FMath::IsNearlyEqual(
				FormulaSurface->GetPresentation().DefenseRow.FinalValue, 8.5f)
			&& FormulaSurface->GetPresentation().ResultSubtitle.IsEmpty()
			&& !FormulaSurface->GetPresentation().bCanContinue);
	Screen->AdvanceInlineFormulaRevealForTesting(0.20f);
	TestTrue(TEXT("Authority winner maps to Chinese result during hold"),
		FormulaSurface->GetPresentation().StatusLabel
			== TEXT("脚下球 · 进攻成功")
			&& FormulaSurface->GetPresentation().ContestLabel
				== TEXT("厄德高直塞，哈兰德破门！")
			&& FormulaSurface->GetPresentation().bNarrativeAvailable
			&& FormulaSurface->GetPresentation().NarrativeHeadline
				== TEXT("厄德高直塞，哈兰德破门！")
			&& FormulaSurface->GetPresentation().RouteResultLabel
				== TEXT("路线掷点 2 → 判定为脚下球")
			&& !FormulaSurface->GetPresentation().bCanContinue);
	Screen->AdvanceInlineFormulaRevealForTesting(2.22f);
	TestTrue(TEXT("Terminal settles before the central NextRound CTA appears"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& FormulaSurface->GetPresentation().bCanContinue
			&& FormulaSurface->GetPresentation().ContinueActionLabel
				== TEXT("下一回合")
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

	UFMCodexLocalMatchScreenWidget* TerminalReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TerminalReconstructed->TakeWidget();
	TerminalReconstructed->RefreshFromPresentation(Terminal);
	const FFMCodexUMGInlineFormulaSurfaceViewModel& RebuiltFormula =
		TerminalReconstructed->GetThroughBallResolutionSurface()
			->GetFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Fresh TerminalPendingAdvance renders truth without replay"),
		TerminalReconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !TerminalReconstructed->IsInlineFormulaRevealInputBlocked()
			&& RebuiltFormula.AttackRow.bFinalValueResolved
			&& RebuiltFormula.DefenseRow.bFinalValueResolved
			&& RebuiltFormula.StatusLabel == TEXT("脚下球 · 进攻成功")
			&& RebuiltFormula.NarrativeHeadline
				== TEXT("厄德高直塞，哈兰德破门！")
			&& RebuiltFormula.RouteResultLabel
				== TEXT("路线掷点 2 → 判定为脚下球")
			&& RebuiltFormula.bCanContinue
			&& RebuiltFormula.ContinueActionLabel == TEXT("下一回合")
			&& Terminal.Interaction.Category
				== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal
			&& !TerminalReconstructed->IsLegacyResolutionOverlayVisible());

	const FFMCodexUMGInlineFormulaSurfaceViewModel DefenderNarrative =
		Build(MakeFeetView(
			true, true, true, EFormulaWinner::Defender))
				.ThroughBallResolution.Formula;
	TestTrue(TEXT("Feet defender winner maps to deterministic participant narrative"),
		DefenderNarrative.bNarrativeAvailable
			&& !DefenderNarrative.bNarrativeAttackSuccess
			&& DefenderNarrative.NarrativeHeadline
				== TEXT("厄德高直塞被萨利巴破坏")
			&& DefenderNarrative.ResultSubtitle
				== TEXT("脚下球 · 防守成功"));

	FFMCodexLocalMatchInteractionView MissingNarrativeFacts = MakeFeetView(
		true, true, true, EFormulaWinner::Attacker);
	MissingNarrativeFacts.ResolutionFacts.Participants.Reset();
	const FFMCodexUMGInlineFormulaSurfaceViewModel FallbackNarrative =
		Build(MissingNarrativeFacts).ThroughBallResolution.Formula;
	TestTrue(TEXT("Feet terminal narrative has a concise insufficient-facts fallback"),
		FallbackNarrative.bNarrativeAvailable
			&& FallbackNarrative.NarrativeHeadline
				== TEXT("脚下球进攻成功"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionFeetAuthorityCapabilityBoundaryTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.FeetAuthorityCapabilityBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionFeetAuthorityCapabilityBoundaryTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;

	const FString ProjectDir = FPaths::ProjectDir();
	FString FeetPlanSource;
	FString TerminalSource;
	FString HostHeader;
	FString ControllerSource;
	FString InteractionHeader;
	FString D6ProviderHeader;
	TestTrue(TEXT("Feet post-route plan authority source is readable"),
		FFileHelper::LoadFileToString(FeetPlanSource, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/CoreRules/MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator.cpp"))));
	TestTrue(TEXT("ThroughBall terminal authority source is readable"),
		FFileHelper::LoadFileToString(TerminalSource, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/CoreRules/MatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator.cpp"))));
	TestTrue(TEXT("Local host API source is readable"),
		FFileHelper::LoadFileToString(HostHeader, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"))));
	TestTrue(TEXT("Local controller routing source is readable"),
		FFileHelper::LoadFileToString(ControllerSource, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"))));
	TestTrue(TEXT("Local interaction contract source is readable"),
		FFileHelper::LoadFileToString(InteractionHeader, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.h"))));
	TestTrue(TEXT("Local D6 provider source is readable"),
		FFileHelper::LoadFileToString(D6ProviderHeader, *FPaths::Combine(
			ProjectDir,
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.h"))));

	TestTrue(TEXT("Feet authority limits each explicit roll command to one D6"),
		FeetPlanSource.Contains(
			TEXT("const int32 MaximumRollsThisCommand = bExplicitRollStep ? 1 : MAX_int32"))
			&& FeetPlanSource.Contains(
				TEXT("Result.ProviderCallCount < MaximumRollsThisCommand"))
			&& FeetPlanSource.Contains(TEXT("RollProvider->RollD6(Purpose)"))
			&& FeetPlanSource.Contains(TEXT("++Result.ProviderCallCount")));
	TestTrue(TEXT("Feet authority validates typed step and requesting side before RNG"),
		FeetPlanSource.Contains(TEXT("EError::WrongFeetRollStep"))
			&& FeetPlanSource.Contains(TEXT("EError::WrongRequestingSide"))
			&& FeetPlanSource.Find(TEXT("EError::WrongRequestingSide"))
				< FeetPlanSource.Find(TEXT("RollProvider->RollD6(Purpose)")));
	TestTrue(TEXT("Terminal apply regenerates the authoritative Feet formula"),
		TerminalSource.Contains(TEXT("++Result.FeetFormulaRegenerationCount"))
			&& TerminalSource.Contains(
				TEXT("FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator")));
	TestTrue(TEXT("Separate Feet attack-roll authority command is exposed"),
		HostHeader.Contains(TEXT("ResolveThroughBallFeetAttackRoll"))
			&& ControllerSource.Contains(
				TEXT("ResolveThroughBallFeetAttackRoll"))
			&& InteractionHeader.Contains(
				TEXT("RollThroughBallFeetAttack")));
	TestTrue(TEXT("Separate Feet defense-roll authority command is exposed"),
		HostHeader.Contains(TEXT("ResolveThroughBallFeetDefenseRoll"))
			&& ControllerSource.Contains(
				TEXT("ResolveThroughBallFeetDefenseRoll"))
			&& InteractionHeader.Contains(
				TEXT("RollThroughBallFeetDefense")));
	TestFalse(TEXT("Generic Controller continuation cannot call the atomic Feet API"),
		ControllerSource.Contains(
			TEXT("Host->ResolveThroughBallFeetPostRoutePlan()")));
	TestTrue(TEXT("Seeded provider has no one-shot developer override seam"),
		D6ProviderHeader.Contains(TEXT("FRandomStream RandomStream"))
			&& !D6ProviderHeader.Contains(
				TEXT("Override"), ESearchCase::CaseSensitive)
			&& !D6ProviderHeader.Contains(
				TEXT("Enqueue"), ESearchCase::CaseSensitive)
			&& !D6ProviderHeader.Contains(
				TEXT("ClearNextD6"), ESearchCase::CaseSensitive));
	return true;
}

#endif
