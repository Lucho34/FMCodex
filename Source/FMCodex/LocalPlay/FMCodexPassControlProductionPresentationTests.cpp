#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"

#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexPassControlProductionPresentationTests
{
	using ECategory = EFMCodexLocalMatchInteractionCategory;
	using EBranch = EMatchPlayPassControlActualBranch;
	using EPostPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using ETermKind = EMatchPlayResolutionFormulaTermKind;
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EParticipant = EMatchPlayResolutionParticipantRole;

	const FName CarrierId(TEXT("Player.PassControl.Carrier"));
	const FName RunnerId(TEXT("Player.PassControl.Runner"));
	const FName MarkerId(TEXT("Player.PassControl.Marker"));
	const FName HelperId(TEXT("Player.PassControl.Helper"));
	const FName GoalkeeperId(TEXT("Player.PassControl.Goalkeeper"));

	void AddRosterCard(
		FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const FString& DisplayName)
	{
		FFMCodexLocalMatchCardView Card;
		Card.Side = Side;
		Card.CardId = CardId;
		Card.DisplayLabel = DisplayName;
		(Side == EInitialTurnOrderPlayer::PlayerA
			? View.PlayerACardRoster : View.PlayerBCardRoster).Add(Card);
	}

	FFMCodexLocalMatchInteractionView BaseView(const ECategory Category)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.AttackSequence = 23;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = Category == ECategory::RollPassControlDefense
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
		View.PresentedActionType = ESkillRuleType::PassControl;
		View.InteractionCategory = Category;
		View.ActionLabel = TEXT("Pass Control");
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerA,
			CarrierId, TEXT("萨卡"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerA,
			RunnerId, TEXT("厄德高"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			MarkerId, TEXT("萨利巴"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			HelperId, TEXT("赖斯"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			GoalkeeperId, TEXT("拉亚"));
		return View;
	}

	FMatchPlayResolutionRollFact RouteRoll(
		const bool bResolved, const int32 RawD6)
	{
		FMatchPlayResolutionRollFact Result;
		Result.SequenceIndex = 0;
		Result.bInitialRoute = true;
		Result.Semantics = EMatchPlayResolutionRollSemantics::BranchSelection;
		Result.OwningSide = EInitialTurnOrderPlayer::PlayerA;
		Result.bResolved = bResolved;
		Result.RawD6 = bResolved ? RawD6 : 0;
		return Result;
	}

	FMatchPlayResolutionRollFact ContestRoll(
		const int32 Sequence,
		const EPostPurpose Purpose,
		const EInitialTurnOrderPlayer Side,
		const bool bResolved,
		const int32 RawD6)
	{
		FMatchPlayResolutionRollFact Result;
		Result.SequenceIndex = Sequence;
		Result.PostRoutePurpose = Purpose;
		Result.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Result.OwningSide = Side;
		Result.bConditionallyRequired = Purpose == EPostPurpose::PrimaryDefense;
		Result.bResolved = bResolved;
		Result.RawD6 = bResolved ? RawD6 : 0;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact Attribute(
		const FName TermId,
		const EParticipant Role,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const EAttribute AttributeId,
		const float Source,
		const ETermKind Kind = ETermKind::Attribute)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TermId;
		Result.Kind = Kind;
		Result.ParticipantRole = Role;
		Result.Side = Side;
		Result.CardId = CardId;
		Result.Attribute = AttributeId;
		Result.SourceValue = Source;
		Result.Multiplier = 0.5f;
		Result.Contribution = Source * 0.5f;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact RawTerm(
		const int32 Sequence, const bool bResolved, const int32 RawD6)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = Sequence == 1
			? FName(TEXT("PrimaryAttackD6"))
			: FName(TEXT("PrimaryDefenseD6"));
		Result.Kind = ETermKind::RawRoll;
		Result.RollSequenceIndex = Sequence;
		Result.bResolved = bResolved;
		Result.SourceValue = bResolved ? RawD6 : 0;
		Result.Contribution = Result.SourceValue;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact Fixed(const float Value)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TEXT("Defense.FixedBonus");
		Result.Kind = ETermKind::FixedModifier;
		Result.SourceValue = Value;
		Result.Contribution = Value;
		return Result;
	}

	FName ContestId(const EBranch Branch)
	{
		switch (Branch)
		{
		case EBranch::PassAdvance: return TEXT("PassControl.PassAdvance");
		case EBranch::DribbleAdvance: return TEXT("PassControl.DribbleAdvance");
		case EBranch::RunAdvance: return TEXT("PassControl.RunAdvance");
		default: return NAME_None;
		}
	}

	int32 RouteD6(const EBranch Branch)
	{
		return Branch == EBranch::PassAdvance ? 2
			: Branch == EBranch::DribbleAdvance ? 4 : 6;
	}

	void AddContestFacts(
		FFMCodexLocalMatchInteractionView& View,
		const EBranch Branch,
		const bool bAttackResolved,
		const int32 AttackD6,
		const bool bDefenseResolved,
		const int32 DefenseD6,
		const EFormulaWinner Winner = EFormulaWinner::None,
		const bool bHasHelper = true,
		const bool bHasGoalkeeper = false,
		const bool bEqualFinalValues = false)
	{
		auto& Facts = View.ResolutionFacts;
		Facts.bSuccess = true;
		Facts.bHasFacts = true;
		Facts.AttackSequence = View.AttackSequence;
		Facts.ActionType = ESkillRuleType::PassControl;
		Facts.bHasActualBranch = true;
		Facts.ActualBranch.ActionType = ESkillRuleType::PassControl;
		Facts.ActualBranch.PassControl = Branch;
		Facts.Participants = {
			{ EParticipant::Carrier, EInitialTurnOrderPlayer::PlayerA, CarrierId },
			{ EParticipant::Runner, EInitialTurnOrderPlayer::PlayerA, RunnerId },
			{ EParticipant::Marker, EInitialTurnOrderPlayer::PlayerB, MarkerId }
		};
		if (bHasHelper)
		{
			Facts.Participants.Add(
				{ EParticipant::Helper, EInitialTurnOrderPlayer::PlayerB, HelperId });
		}
		if (bHasGoalkeeper)
		{
			Facts.Participants.Add({ EParticipant::Goalkeeper,
				EInitialTurnOrderPlayer::PlayerB, GoalkeeperId });
		}
		Facts.Rolls = {
			RouteRoll(true, RouteD6(Branch)),
			ContestRoll(1, EPostPurpose::PrimaryAttack,
				EInitialTurnOrderPlayer::PlayerA, bAttackResolved, AttackD6),
			ContestRoll(2, EPostPurpose::PrimaryDefense,
				EInitialTurnOrderPlayer::PlayerB, bDefenseResolved, DefenseD6)
		};
		Facts.bHasPendingRoll = !bAttackResolved || !bDefenseResolved;
		Facts.NextPendingRollSequenceIndex = !bAttackResolved ? 1
			: !bDefenseResolved ? 2 : INDEX_NONE;

		const EAttribute CarrierAttribute = Branch == EBranch::PassAdvance
			? EAttribute::Passing
			: Branch == EBranch::DribbleAdvance
				? EAttribute::Dribbling : EAttribute::OffBall;
		const EAttribute RunnerAttribute = Branch == EBranch::RunAdvance
			? EAttribute::Dribbling : EAttribute::Passing;
		const EAttribute MarkerAttribute = Branch == EBranch::RunAdvance
			? EAttribute::Marking : EAttribute::Tackling;

		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = ContestId(Branch);
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms = {
			Attribute(TEXT("Carrier.PrimaryHalf"), EParticipant::Carrier,
				EInitialTurnOrderPlayer::PlayerA, CarrierId, CarrierAttribute, 8.0f),
			Attribute(TEXT("Runner.PrimaryHalf"), EParticipant::Runner,
				EInitialTurnOrderPlayer::PlayerA, RunnerId, RunnerAttribute, 6.0f),
			RawTerm(1, bAttackResolved, AttackD6)
		};
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 7.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved
			? (bEqualFinalValues ? 12.0f : 7.0f + AttackD6) : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms = {
			Attribute(TEXT("Marker.PrimaryHalf"), EParticipant::Marker,
				EInitialTurnOrderPlayer::PlayerB, MarkerId, MarkerAttribute, 6.0f)
		};
		if (bHasHelper)
		{
			Contest.DefenseRow.Terms.Add(Attribute(
				TEXT("Helper.PrimaryHalf"), EParticipant::Helper,
				EInitialTurnOrderPlayer::PlayerB, HelperId,
				EAttribute::Marking, 4.0f));
		}
		Contest.DefenseRow.Terms.Add(RawTerm(2, bDefenseResolved, DefenseD6));
		Contest.DefenseRow.Terms.Add(Fixed(2.0f));
		if (bHasGoalkeeper)
		{
			Contest.DefenseRow.Terms.Add(Attribute(
				TEXT("Goalkeeper.HandlingHalf"), EParticipant::Goalkeeper,
				EInitialTurnOrderPlayer::PlayerB, GoalkeeperId,
				EAttribute::GoalkeeperHandling, 8.0f,
				ETermKind::GoalkeeperContribution));
			Contest.bGoalkeeperParticipated = true;
		}
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = bHasHelper ? 7.0f : 5.0f;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved
			? (bEqualFinalValues ? 12.0f
				: Contest.DefenseRow.KnownNonRollSubtotal + DefenseD6) : 0.0f;
		if (bAttackResolved && bDefenseResolved && Winner != EFormulaWinner::None)
		{
			Contest.Application = EMatchPlayResolutionFormulaApplication::Applied;
			Contest.bHasResolvedFormula = true;
			Contest.ResolvedResult.FormulaType = EFormulaType::Finishing;
			Contest.ResolvedResult.Winner = Winner;
			Contest.ResolvedResult.bIsGoal = Winner == EFormulaWinner::Attacker;
			Contest.ResolvedResult.bAttackEnded = true;
			Contest.ResolvedResult.bContinueResolution = false;
		}
		Facts.FormulaContests = { Contest };
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = FName(*FString::Printf(
			TEXT("%s.Outcome"), *Contest.ContestId.ToString()));
		Decision.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Decision.RollSequenceIndices = { 1, 2 };
		Decision.bResolved = Contest.bHasResolvedFormula;
		Decision.Outcome = Decision.bResolved
			? (Contest.ResolvedResult.bIsGoal
				? EMatchPlayResolutionDecisionOutcome::Goal
				: EMatchPlayResolutionDecisionOutcome::Miss)
			: EMatchPlayResolutionDecisionOutcome::None;
		Facts.Decisions = { Decision };
	}

	FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString());
	}

	bool HasAttribute(
		const FFMCodexUMGInlineFormulaRowViewModel& Row,
		const FString& AttributeLabel,
		const FString& Contributor)
	{
		return Row.Terms.ContainsByPredicate(
			[&](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.AttributeLabel == AttributeLabel
					&& Term.ContributorDisplayName == Contributor
					&& Term.Multiplier == 0.5f;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPassControlProductionRoutePendingTest,
	"FMCodex.LocalPlay.PassControlProduction.01.RoutePending",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPassControlProductionRoutePendingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPassControlProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView View = BaseView(ECategory::RollPassControlRoute);
	View.ContinueActionLabel = TEXT("判定控球推进路线");
	View.ResolutionFacts.bSuccess = true;
	View.ResolutionFacts.bHasFacts = true;
	View.ResolutionFacts.AttackSequence = View.AttackSequence;
	View.ResolutionFacts.ActionType = ESkillRuleType::PassControl;
	View.ResolutionFacts.Rolls = { RouteRoll(false, 0) };
	View.ResolutionFacts.bHasPendingRoll = true;
	View.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	const FFMCodexUMGMatchScreenViewModel Presentation = Build(View);
	const auto& Surface = Presentation.LongShotResolution;
	TestTrue(TEXT("PassControl route pending owns the central production surface"),
		Surface.bVisible && Surface.bSuppressLegacyResolution
			&& Surface.SkillType == ESkillRuleType::PassControl);
	TestEqual(TEXT("PassControl title is Chinese-first"), Surface.TitleLabel,
		FString(TEXT("控球推进")));
	TestEqual(TEXT("Route stage describes random route determination"),
		Surface.StageLabel, FString(TEXT("判定推进方式")));
	TestTrue(TEXT("Route is one typed central CTA"),
		Surface.PrimaryAction.Claims(Presentation.Interaction.PrimaryAction)
			&& Presentation.Interaction.PrimaryAction.Label == TEXT("判定推进方式"));
	TestTrue(TEXT("Route reveal uses its own shared-reel identity"),
		Presentation.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::PassControlInitialRoute
			&& Presentation.Interaction.CrossRollContestId
				== FName(TEXT("PassControl.Route")));
	TestTrue(TEXT("Random routes are not exposed as choices"),
		Surface.BranchChoices.IsEmpty() && !Surface.Formula.bVisible);

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Presentation);
	TestEqual(TEXT("Central route ownership suppresses the lower duplicate"),
		Screen->GetInteractionPanel()->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("Normal route state suppresses diagnostic resolution"),
		Screen->GetWidgetFromName(TEXT("ResolutionPresentationLayer"))->GetVisibility(),
		ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPassControlProductionRouteFormulaMatrixTest,
	"FMCodex.LocalPlay.PassControlProduction.02.RouteFormulaMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPassControlProductionRouteFormulaMatrixTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPassControlProductionPresentationTests;
	(void)Parameters;
	struct FCase
	{
		EBranch Branch;
		const TCHAR* RouteLabel;
		const TCHAR* CarrierAttribute;
		const TCHAR* RunnerAttribute;
		const TCHAR* MarkerAttribute;
	};
	const TArray<FCase> Cases = {
		{ EBranch::PassAdvance, TEXT("传球推进"), TEXT("传球"), TEXT("传球"),
			TEXT("抢断") },
		{ EBranch::DribbleAdvance, TEXT("盘带推进"), TEXT("盘带"), TEXT("传球"),
			TEXT("抢断") },
		{ EBranch::RunAdvance, TEXT("跑动推进"), TEXT("跑位"), TEXT("盘带"),
			TEXT("盯防") }
	};
	for (const FCase& Case : Cases)
	{
		FFMCodexLocalMatchInteractionView View = BaseView(ECategory::RollPassControlAttack);
		View.ContinueActionLabel = TEXT("进攻方掷点");
		AddContestFacts(View, Case.Branch, false, 0, false, 0);
		const FFMCodexUMGMatchScreenViewModel Presentation = Build(View);
		const auto& Surface = Presentation.LongShotResolution;
		const auto& Formula = Surface.Formula;
		TestTrue(FString::Printf(TEXT("%s route retains raw D6 and authoritative name"),
			Case.RouteLabel),
			Surface.BranchLabel.Contains(FString::FromInt(RouteD6(Case.Branch)))
				&& Surface.BranchLabel.Contains(Case.RouteLabel));
		TestEqual(FString::Printf(TEXT("%s contest id"), Case.RouteLabel),
			Formula.ContestId, ContestId(Case.Branch));
		TestTrue(FString::Printf(TEXT("%s maps Carrier authority fact"), Case.RouteLabel),
			HasAttribute(Formula.AttackRow, Case.CarrierAttribute, TEXT("萨卡")));
		TestTrue(FString::Printf(TEXT("%s maps Runner authority fact"), Case.RouteLabel),
			HasAttribute(Formula.AttackRow, Case.RunnerAttribute, TEXT("厄德高")));
		TestTrue(FString::Printf(TEXT("%s maps Marker authority fact"), Case.RouteLabel),
			HasAttribute(Formula.DefenseRow, Case.MarkerAttribute, TEXT("萨利巴")));
		TestTrue(FString::Printf(TEXT("%s maps optional Helper fact"), Case.RouteLabel),
			HasAttribute(Formula.DefenseRow, TEXT("盯防"), TEXT("赖斯")));
		TestTrue(FString::Printf(TEXT("%s attack owns the typed central CTA"),
			Case.RouteLabel),
			Formula.PrimaryAction.Claims(Presentation.Interaction.PrimaryAction)
				&& Formula.ContinueActionLabel == TEXT("进攻方掷点"));

		FFMCodexLocalMatchInteractionView Terminal = BaseView(
			ECategory::AdvanceAfterTerminal);
		Terminal.ContinueActionLabel = TEXT("下一回合");
		Terminal.bTerminalPendingAdvance = true;
		AddContestFacts(Terminal, Case.Branch, true, 4, true, 3,
			EFormulaWinner::Attacker);
		const auto TerminalPresentation = Build(Terminal);
		const auto& TerminalFormula = TerminalPresentation.LongShotResolution.Formula;
		TestTrue(FString::Printf(TEXT("%s uses centralized Runner-goal Narrative"),
			Case.RouteLabel),
			TerminalFormula.bNarrativeAvailable
				&& TerminalFormula.NarrativeHeadline.Contains(Case.RouteLabel)
				&& TerminalFormula.NarrativeHeadline.Contains(TEXT("厄德高破门"))
				&& !TerminalFormula.NarrativeHeadline.Contains(CarrierId.ToString()));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPassControlProductionSequentialAndTerminalTest,
	"FMCodex.LocalPlay.PassControlProduction.03.SequentialAndTerminal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPassControlProductionSequentialAndTerminalTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPassControlProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView AttackOnly = BaseView(
		ECategory::RollPassControlDefense);
	AttackOnly.ContinueActionLabel = TEXT("防守方掷点");
	AddContestFacts(AttackOnly, EBranch::PassAdvance, true, 4, false, 0);
	const FFMCodexUMGMatchScreenViewModel AttackOnlyPresentation = Build(AttackOnly);
	const auto& AttackOnlyFormula =
		AttackOnlyPresentation.LongShotResolution.Formula;
	TestTrue(TEXT("Attack-only snapshot exposes the authoritative attack raw roll"),
		AttackOnlyFormula.AttackRow.bFinalValueResolved
			&& AttackOnlyFormula.AttackRow.Terms.ContainsByPredicate(
				[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
				{
					return Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll
						&& Term.bResolved && Term.RawD6 == 4;
				}));
	TestFalse(TEXT("Attack-only snapshot does not fabricate defense"),
		AttackOnlyFormula.DefenseRow.bFinalValueResolved
			|| AttackOnlyFormula.DefenseRow.Terms.ContainsByPredicate(
				[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
				{
					return Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll
						&& Term.bResolved;
				}));
	TestTrue(TEXT("Attack-only snapshot exposes no final result or Narrative"),
		!AttackOnlyFormula.bNarrativeAvailable
			&& AttackOnlyFormula.ResultTitle.IsEmpty()
			&& AttackOnlyFormula.PrimaryAction.Claims(
				AttackOnlyPresentation.Interaction.PrimaryAction)
			&& AttackOnlyFormula.ContinueActionLabel == TEXT("防守方掷点"));

	FFMCodexLocalMatchInteractionView Terminal = BaseView(
		ECategory::AdvanceAfterTerminal);
	Terminal.ContinueActionLabel = TEXT("下一回合");
	Terminal.bTerminalPendingAdvance = true;
	AddContestFacts(Terminal, EBranch::RunAdvance, true, 4, true, 3,
		EFormulaWinner::Defender, false, true, true);
	const FFMCodexUMGMatchScreenViewModel TerminalPresentation = Build(Terminal);
	const auto& Formula = TerminalPresentation.LongShotResolution.Formula;
	TestTrue(TEXT("Authoritative tie winner is rendered without local stamina math"),
		Formula.AttackRow.FinalValue == Formula.DefenseRow.FinalValue
			&& Formula.ResultTitle == TEXT("防守成功")
			&& Formula.NarrativeHeadline.Contains(TEXT("跑动推进")));
	TestFalse(TEXT("Absent Helper produces no fake or blank participant row"),
		Formula.DefenseRow.Participants.ContainsByPredicate(
			[](const FFMCodexUMGInlineFormulaParticipantViewModel& Participant)
			{
				return Participant.RoleLabel == TEXT("协防")
					|| Participant.PlayerName.IsEmpty();
			}));
	TestTrue(TEXT("Active GK is rendered only from authoritative Handling fact"),
		HasAttribute(Formula.DefenseRow, TEXT("手控球"), TEXT("拉亚")));
	TestTrue(TEXT("Terminal Formula owns one explicit NextRound"),
		Formula.PrimaryAction.Claims(TerminalPresentation.Interaction.PrimaryAction)
			&& Formula.ContinueActionLabel == TEXT("下一回合"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPassControlProductionTypedRoutingContractTest,
	"FMCodex.LocalPlay.PassControlProduction.04.TypedRoutingContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPassControlProductionTypedRoutingContractTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	FString ScreenSource;
	const FString ScreenPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"));
	TestTrue(TEXT("Screen routing source is readable"),
		FFileHelper::LoadFileToString(ScreenSource, *ScreenPath));
	TestTrue(TEXT("Route dispatches typed PassControl Controller request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollPassControlRoute:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollPassControlRoute();")));
	TestTrue(TEXT("Attack dispatches typed PassControl Controller request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollPassControlAttack:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollPassControlAttack();")));
	TestTrue(TEXT("Defense dispatches typed PassControl Controller request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollPassControlDefense:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollPassControlDefense();")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexPassControlProductionNarrativeDedupAndNamingTest,
	"FMCodex.LocalPlay.PassControlProduction.05.NarrativeDedupAndNaming",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexPassControlProductionNarrativeDedupAndNamingTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexPassControlProductionPresentationTests;
	(void)Parameters;
	struct FCase
	{
		EBranch Branch;
		const TCHAR* RouteLabel;
	};
	for (const FCase& Case : {
		FCase{ EBranch::PassAdvance, TEXT("传球推进") },
		FCase{ EBranch::DribbleAdvance, TEXT("盘带推进") },
		FCase{ EBranch::RunAdvance, TEXT("跑动推进") } })
	{
		FFMCodexLocalMatchInteractionView Goal = BaseView(
			ECategory::AdvanceAfterTerminal);
		Goal.ContinueActionLabel = TEXT("下一回合");
		Goal.bTerminalPendingAdvance = true;
		AddContestFacts(Goal, Case.Branch, true, 6, true, 1,
			EFormulaWinner::Attacker);
		const auto GoalPresentation = Build(Goal);
		const auto& GoalSurface = GoalPresentation.LongShotResolution;
		const auto& GoalFormula = GoalSurface.Formula;
		TestTrue(FString::Printf(TEXT("%s Goal keeps Carrier Runner route and scorer"),
			Case.RouteLabel),
			GoalFormula.bNarrativeAvailable
				&& GoalFormula.NarrativeHeadline.Contains(TEXT("萨卡"))
				&& GoalFormula.NarrativeHeadline.Contains(TEXT("厄德高"))
				&& GoalFormula.NarrativeHeadline.Contains(Case.RouteLabel)
				&& GoalFormula.NarrativeHeadline.Contains(TEXT("厄德高破门")));
		TestTrue(FString::Printf(TEXT("%s Goal has one prose owner and compact status"),
			Case.RouteLabel),
			GoalSurface.StageLabel == Case.RouteLabel
				&& GoalSurface.StageLabel != GoalFormula.NarrativeHeadline
				&& GoalFormula.ContestLabel == GoalFormula.NarrativeHeadline
				&& GoalFormula.StatusLabel
					== FString::Printf(TEXT("%s · 进球"), Case.RouteLabel)
				&& GoalFormula.StatusLabel != GoalFormula.NarrativeHeadline
				&& !GoalSurface.bNarrativeAvailable
				&& GoalSurface.NarrativeHeadline.IsEmpty());

		FFMCodexLocalMatchInteractionView Defense = BaseView(
			ECategory::AdvanceAfterTerminal);
		Defense.ContinueActionLabel = TEXT("下一回合");
		Defense.bTerminalPendingAdvance = true;
		AddContestFacts(Defense, Case.Branch, true, 1, true, 6,
			EFormulaWinner::Defender, false);
		const auto DefensePresentation = Build(Defense);
		const auto& DefenseSurface = DefensePresentation.LongShotResolution;
		const auto& DefenseFormula = DefenseSurface.Formula;
		TestTrue(FString::Printf(TEXT("%s defense keeps both attackers and Marker verb"),
			Case.RouteLabel),
			DefenseFormula.ResultTitle == TEXT("防守成功")
				&& DefenseFormula.NarrativeHeadline.Contains(TEXT("萨卡"))
				&& DefenseFormula.NarrativeHeadline.Contains(TEXT("厄德高"))
				&& DefenseFormula.NarrativeHeadline.Contains(Case.RouteLabel)
				&& DefenseFormula.NarrativeHeadline.Contains(TEXT("萨利巴"))
				&& DefenseFormula.NarrativeHeadline.Contains(TEXT("抢断"))
				&& !DefenseFormula.NarrativeHeadline.Contains(CarrierId.ToString())
				&& !DefenseFormula.NarrativeHeadline.Contains(RunnerId.ToString()));
		TestTrue(FString::Printf(TEXT("%s defense prevents duplicated complete prose"),
			Case.RouteLabel),
			DefenseSurface.StageLabel == Case.RouteLabel
				&& DefenseSurface.StageLabel != DefenseFormula.NarrativeHeadline
				&& DefenseFormula.ContestLabel == DefenseFormula.NarrativeHeadline
				&& DefenseFormula.StatusLabel
					== FString::Printf(TEXT("%s · 防守成功"), Case.RouteLabel)
				&& DefenseFormula.StatusLabel != DefenseFormula.NarrativeHeadline
				&& DefenseFormula.PrimaryAction.Claims(
					DefensePresentation.Interaction.PrimaryAction)
				&& DefenseFormula.ContinueActionLabel == TEXT("下一回合"));
	}
	return true;
}

#endif
