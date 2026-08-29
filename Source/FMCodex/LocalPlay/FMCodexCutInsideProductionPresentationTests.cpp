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

namespace FMCodexCutInsideProductionPresentationTests
{
	using ECategory = EFMCodexLocalMatchInteractionCategory;
	using EPostPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using ETermKind = EMatchPlayResolutionFormulaTermKind;
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EParticipant = EMatchPlayResolutionParticipantRole;

	const FName CarrierId(TEXT("Player.CutInside.Carrier"));
	const FName MarkerId(TEXT("Player.CutInside.Marker"));
	const FName GoalkeeperId(TEXT("Player.CutInside.Goalkeeper"));

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
		View.AttackSequence = 19;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = Category
			== ECategory::RollCutInsideShotDirectDefense
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		View.PresentedActionType = ESkillRuleType::CutInsideShot;
		View.InteractionCategory = Category;
		View.ActionLabel = TEXT("Cut Inside");
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerA,
			CarrierId, TEXT("萨卡"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			MarkerId, TEXT("萨利巴"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			GoalkeeperId, TEXT("拉亚"));
		return View;
	}

	void AddBranch(
		FFMCodexLocalMatchInteractionView& View,
		const EMatchPlayCutInsideShotActualBranch Branch)
	{
		View.ResolutionFacts.bSuccess = true;
		View.ResolutionFacts.bHasFacts = true;
		View.ResolutionFacts.AttackSequence = View.AttackSequence;
		View.ResolutionFacts.ActionType = ESkillRuleType::CutInsideShot;
		View.ResolutionFacts.bHasActualBranch = true;
		View.ResolutionFacts.ActualBranch.ActionType =
			ESkillRuleType::CutInsideShot;
		View.ResolutionFacts.ActualBranch.CutInsideShot = Branch;
		View.ResolutionFacts.Participants = {
			{ EParticipant::Carrier, EInitialTurnOrderPlayer::PlayerA, CarrierId },
			{ EParticipant::Marker, EInitialTurnOrderPlayer::PlayerB, MarkerId },
			{ EParticipant::Goalkeeper, EInitialTurnOrderPlayer::PlayerB,
				GoalkeeperId }
		};
	}

	FMatchPlayResolutionRollFact Roll(
		const int32 Sequence,
		const EPostPurpose Purpose,
		const EMatchPlayResolutionRollSemantics Semantics,
		const EInitialTurnOrderPlayer Side,
		const bool bResolved,
		const int32 RawD6)
	{
		FMatchPlayResolutionRollFact Result;
		Result.SequenceIndex = Sequence;
		Result.PostRoutePurpose = Purpose;
		Result.Semantics = Semantics;
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
		const float Multiplier = 1.0f)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TermId;
		Result.Kind = Role == EParticipant::Goalkeeper
			? ETermKind::GoalkeeperContribution : ETermKind::Attribute;
		Result.ParticipantRole = Role;
		Result.Side = Side;
		Result.CardId = CardId;
		Result.Attribute = AttributeId;
		Result.SourceValue = Source;
		Result.Multiplier = Multiplier;
		Result.Contribution = Source * Multiplier;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact RawTerm(
		const int32 Sequence, const bool bResolved, const int32 RawD6)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = Sequence == 0
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

	void AddDirectFacts(
		FFMCodexLocalMatchInteractionView& View,
		const bool bAttackResolved,
		const int32 AttackD6,
		const bool bDefenseResolved,
		const int32 DefenseD6,
		const EFormulaWinner Winner = EFormulaWinner::None)
	{
		AddBranch(View, EMatchPlayCutInsideShotActualBranch::DirectShot);
		View.ResolutionFacts.Rolls = {
			Roll(0, EPostPurpose::PrimaryAttack,
				EMatchPlayResolutionRollSemantics::ArithmeticContest,
				EInitialTurnOrderPlayer::PlayerA, bAttackResolved, AttackD6),
			Roll(1, EPostPurpose::PrimaryDefense,
				EMatchPlayResolutionRollSemantics::ArithmeticContest,
				EInitialTurnOrderPlayer::PlayerB, bDefenseResolved, DefenseD6)
		};
		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = TEXT("CutInsideShot.DirectShot");
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms = {
			Attribute(TEXT("Carrier.ShootingHalf"), EParticipant::Carrier,
				EInitialTurnOrderPlayer::PlayerA, CarrierId,
				EAttribute::Shooting, 8.0f, 0.5f),
			Attribute(TEXT("Carrier.DribblingHalf"), EParticipant::Carrier,
				EInitialTurnOrderPlayer::PlayerA, CarrierId,
				EAttribute::Dribbling, 6.0f, 0.5f),
			RawTerm(0, bAttackResolved, AttackD6)
		};
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 7.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved ? 7.0f + AttackD6 : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms = {
			Attribute(TEXT("Marker.Tackling"), EParticipant::Marker,
				EInitialTurnOrderPlayer::PlayerB, MarkerId,
				EAttribute::Tackling, 5.0f),
			RawTerm(1, bDefenseResolved, DefenseD6),
			Fixed(2.0f),
			Attribute(TEXT("Goalkeeper.HandlingHalf"), EParticipant::Goalkeeper,
				EInitialTurnOrderPlayer::PlayerB, GoalkeeperId,
				EAttribute::GoalkeeperHandling, 8.0f, 0.5f)
		};
		Contest.bGoalkeeperParticipated = true;
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = 11.0f;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved
			? 11.0f + DefenseD6 : 0.0f;
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
		View.ResolutionFacts.FormulaContests = { Contest };
	}

	void AddDecision(
		FFMCodexLocalMatchInteractionView& View,
		const FName DecisionId,
		const EMatchPlayResolutionRollSemantics Semantics,
		const EMatchPlayResolutionDecisionOutcome Outcome)
	{
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = DecisionId;
		Decision.Semantics = Semantics;
		Decision.bResolved = true;
		Decision.Outcome = Outcome;
		View.ResolutionFacts.Decisions = { Decision };
	}

	FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCutInsideProductionBranchSurfaceTest,
	"FMCodex.LocalPlay.CutInsideProduction.01.BranchSurface",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCutInsideProductionBranchSurfaceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView View = BaseView(
		ECategory::SelectBranchIntent);
	View.BranchIntentOptions = {
		EMatchPlayElectiveBranchIntent::DirectShot,
		EMatchPlayElectiveBranchIntent::DeadCorner
	};
	const FFMCodexUMGMatchScreenViewModel Screen = Build(View);
	TestTrue(TEXT("CutInside reuses central production surface"),
		Screen.LongShotResolution.bVisible
			&& Screen.LongShotResolution.bSuppressLegacyResolution);
	TestEqual(TEXT("Surface preserves family identity"),
		Screen.LongShotResolution.SkillType, ESkillRuleType::CutInsideShot);
	TestEqual(TEXT("CutInside title is player-facing"),
		Screen.LongShotResolution.TitleLabel, FString(TEXT("内切")));
	TestEqual(TEXT("Two authority choices remain independent"),
		Screen.LongShotResolution.BranchChoices.Num(), 2);
	if (Screen.LongShotResolution.BranchChoices.Num() == 2)
	{
		const auto& Direct = Screen.LongShotResolution.BranchChoices[0];
		const auto& Dead = Screen.LongShotResolution.BranchChoices[1];
		TestEqual(TEXT("Direct branch copy"), Direct.Label,
			FString(TEXT("直接射门")));
		TestEqual(TEXT("DeadCorner branch copy"), Dead.Label,
			FString(TEXT("直射死角")));
		TestEqual(TEXT("Direct helper is the canonical compact comparison"),
			Direct.SecondaryLabel,
			FString(TEXT("（射门 / 盘带 vs 抢断）")));
		TestEqual(TEXT("DeadCorner compact helper"), Dead.SecondaryLabel,
			FString(TEXT("（只看两枚掷点）")));
	}
	TestEqual(TEXT("Real InteractionView category remains authoritative"),
		Screen.Interaction.Category,
		EFMCodexUMGInteractionCategory::SelectBranchIntent);
	TestFalse(TEXT("Branch selection exposes no roll CTA"),
		Screen.Interaction.PrimaryAction.bAvailable);

	UFMCodexLocalMatchScreenWidget* MatchScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	MatchScreen->TakeWidget();
	MatchScreen->RefreshFromPresentation(Screen);
	TestTrue(TEXT("Real CutInside branch category renders centrally only"),
		MatchScreen->GetLongShotResolutionSurface() != nullptr
			&& MatchScreen->GetLongShotResolutionSurface()->GetPresentation()
				.bVisible
			&& MatchScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCutInsideProductionDirectStatesTest,
	"FMCodex.LocalPlay.CutInsideProduction.02.DirectStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCutInsideProductionDirectStatesTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	(void)Parameters;

	FFMCodexLocalMatchInteractionView Pending = BaseView(
		ECategory::RollCutInsideShotDirectAttack);
	Pending.ContinueActionLabel = TEXT("进攻方掷内切射门点数");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	AddDirectFacts(Pending, false, 0, false, 0);
	const FFMCodexUMGMatchScreenViewModel PendingScreen = Build(Pending);
	TestEqual(TEXT("Direct Attack maps to typed UMG category"),
		PendingScreen.Interaction.Category,
		EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack);
	TestTrue(TEXT("Shared Formula surface owns Direct Attack"),
		PendingScreen.LongShotResolution.Formula.PrimaryAction.Claims(
			PendingScreen.Interaction.PrimaryAction));
	TestEqual(TEXT("Direct Attack uses the compact player CTA"),
		PendingScreen.Interaction.PrimaryAction.Label,
		FString(TEXT("进攻方掷点")));
	TestEqual(TEXT("CutInside gate hint uses the exact compact copy"),
		PendingScreen.LongShotResolution.OutcomeHintLabel,
		FString(TEXT("1–2：射门偏出｜3–6：进入攻防结算")));
	TestEqual(TEXT("Attack reveal identity is authoritative"),
		PendingScreen.Interaction.CrossRollContestId,
		FName(TEXT("CutInsideShot.DirectShot")));
	TestEqual(TEXT("Attack reveal begins on attack side"),
		PendingScreen.Interaction.CrossRollOwnerSide,
		EInitialTurnOrderPlayer::PlayerA);

	FFMCodexLocalMatchInteractionView AttackOnly = BaseView(
		ECategory::RollCutInsideShotDirectDefense);
	AttackOnly.ContinueActionLabel = TEXT("防守方掷防守点数");
	AttackOnly.ResolutionFacts.bHasPendingRoll = true;
	AttackOnly.ResolutionFacts.NextPendingRollSequenceIndex = 1;
	AddDirectFacts(AttackOnly, true, 4, false, 0);
	const FFMCodexUMGMatchScreenViewModel AttackOnlyScreen = Build(AttackOnly);
	const FFMCodexUMGMatchScreenViewModel RebuiltAttackOnly = Build(AttackOnly);
	TestEqual(TEXT("Direct Defense uses the compact player CTA"),
		AttackOnlyScreen.Interaction.PrimaryAction.Label,
		FString(TEXT("防守方掷点")));
	TestTrue(TEXT("Attack-only snapshot preserves real Attack value"),
		AttackOnlyScreen.LongShotResolution.Formula.AttackRow.bFinalValueResolved
			&& AttackOnlyScreen.LongShotResolution.Formula.AttackRow.FinalValue
				== 11.0f
			&& AttackOnlyScreen.LongShotResolution.Formula.AttackRow.Terms
				.ContainsByPredicate(
					[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
					{
						return Term.Kind
							== EFMCodexUMGInlineFormulaTermKind::RawRoll
							&& Term.bResolved && Term.RawD6 == 4;
					}));
	TestFalse(TEXT("Attack-only snapshot does not fabricate Defense"),
		AttackOnlyScreen.LongShotResolution.Formula.DefenseRow
			.bFinalValueResolved
			|| AttackOnlyScreen.LongShotResolution.Formula.DefenseRow.Terms
				.ContainsByPredicate(
					[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
					{
						return Term.Kind
							== EFMCodexUMGInlineFormulaTermKind::RawRoll
							&& Term.bResolved;
					}));
	TestFalse(TEXT("Attack-only snapshot has no final Narrative"),
		AttackOnlyScreen.LongShotResolution.Formula.bNarrativeAvailable);
	TestTrue(TEXT("Defender owns the reconstructed typed CTA"),
		AttackOnlyScreen.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerB
			&& AttackOnlyScreen.LongShotResolution.Formula.PrimaryAction.Claims(
				AttackOnlyScreen.Interaction.PrimaryAction));
	TestEqual(TEXT("Repeated reconstruction preserves raw Attack"),
		RebuiltAttackOnly.LongShotResolution.Formula.AttackRow.FinalValue,
		AttackOnlyScreen.LongShotResolution.Formula.AttackRow.FinalValue);

	FFMCodexLocalMatchInteractionView Immediate = BaseView(
		ECategory::AdvanceAfterTerminal);
	Immediate.ContinueActionLabel = TEXT("下一回合");
	Immediate.bTerminalPendingAdvance = true;
	AddDirectFacts(Immediate, true, 2, false, 0);
	Immediate.ResolutionFacts.FormulaContests[0].Application =
		EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate;
	AddDecision(Immediate, TEXT("CutInsideShot.DirectShot.Outcome"),
		EMatchPlayResolutionRollSemantics::ArithmeticContest,
		EMatchPlayResolutionDecisionOutcome::ImmediateMiss);
	const FFMCodexUMGMatchScreenViewModel ImmediateScreen = Build(Immediate);
	TestFalse(TEXT("ImmediateMiss shows no fabricated Formula rows"),
		ImmediateScreen.LongShotResolution.Formula.bShowFormulaRows);
	TestEqual(TEXT("ImmediateMiss uses centralized CutInside Narrative"),
		ImmediateScreen.LongShotResolution.Formula.NarrativeHeadline,
		FString(TEXT("萨卡内切后射门偏出。")));
	TestTrue(TEXT("ImmediateMiss terminal owns NextRound"),
		ImmediateScreen.LongShotResolution.Formula.PrimaryAction.Claims(
			ImmediateScreen.Interaction.PrimaryAction)
			&& ImmediateScreen.LongShotResolution.Formula.ContinueActionLabel
				== TEXT("下一回合"));

	FFMCodexLocalMatchInteractionView Complete = BaseView(
		ECategory::AdvanceAfterTerminal);
	Complete.ContinueActionLabel = TEXT("下一回合");
	Complete.bTerminalPendingAdvance = true;
	AddDirectFacts(Complete, true, 4, true, 1, EFormulaWinner::Attacker);
	AddDecision(Complete, TEXT("CutInsideShot.DirectShot.Outcome"),
		EMatchPlayResolutionRollSemantics::ArithmeticContest,
		EMatchPlayResolutionDecisionOutcome::Goal);
	const FFMCodexUMGMatchScreenViewModel CompleteScreen = Build(Complete);
	const auto& Formula = CompleteScreen.LongShotResolution.Formula;
	TestTrue(TEXT("Completed Direct keeps both authoritative rows"),
		Formula.bShowFormulaRows && Formula.AttackRow.bFinalValueResolved
			&& Formula.DefenseRow.bFinalValueResolved);
	TestTrue(TEXT("Formula exposes player-facing participants"),
		Formula.AttackRow.Participants.ContainsByPredicate(
			[](const FFMCodexUMGInlineFormulaParticipantViewModel& Participant)
			{
				return Participant.PlayerName == TEXT("萨卡");
			})
			&& Formula.DefenseRow.Participants.ContainsByPredicate(
				[](const FFMCodexUMGInlineFormulaParticipantViewModel& Participant)
				{
					return Participant.PlayerName == TEXT("拉亚");
				}));
	TestTrue(TEXT("Formula retains authoritative fixed +2"),
		Formula.DefenseRow.Terms.ContainsByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind == EFMCodexUMGInlineFormulaTermKind::FixedModifier
					&& Term.Contribution == 2.0f;
			}));
	TestTrue(TEXT("Formula retains only authoritative GK Handling x0.5"),
		Formula.DefenseRow.Terms.ContainsByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.AttributeLabel == TEXT("手控球")
					&& Term.ContributorDisplayName == TEXT("拉亚")
					&& Term.Multiplier == 0.5f;
			}));
	TestEqual(TEXT("Completed Direct uses CutInside Goal Narrative"),
		Formula.NarrativeHeadline, FString(TEXT("萨卡内切破门！")));
	TestTrue(TEXT("Completed Direct remains terminal"),
		Formula.PrimaryAction.Claims(CompleteScreen.Interaction.PrimaryAction));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCutInsideProductionDeadCornerStatesTest,
	"FMCodex.LocalPlay.CutInsideProduction.03.DeadCornerStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCutInsideProductionDeadCornerStatesTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView Pending = BaseView(
		ECategory::RollCutInsideShotDeadCorner);
	Pending.ContinueActionLabel = TEXT("进攻方掷内切死角双骰");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	AddBranch(Pending, EMatchPlayCutInsideShotActualBranch::DeadCorner);
	Pending.ResolutionFacts.Rolls = {
		Roll(0, EPostPurpose::PairedAttackA,
			EMatchPlayResolutionRollSemantics::OutcomeDecision,
			EInitialTurnOrderPlayer::PlayerA, false, 0),
		Roll(1, EPostPurpose::PairedAttackB,
			EMatchPlayResolutionRollSemantics::OutcomeDecision,
			EInitialTurnOrderPlayer::PlayerA, false, 0)
	};
	const FFMCodexUMGMatchScreenViewModel PendingScreen = Build(Pending);
	TestEqual(TEXT("DeadCorner is one typed paired-roll CTA"),
		PendingScreen.Interaction.Category,
		EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner);
	TestTrue(TEXT("Central outcome-only surface owns paired CTA"),
		PendingScreen.LongShotResolution.PrimaryAction.Claims(
			PendingScreen.Interaction.PrimaryAction));
	TestEqual(TEXT("DeadCorner uses the compact paired-roll CTA"),
		PendingScreen.Interaction.PrimaryAction.Label,
		FString(TEXT("掷两枚骰")));
	TestFalse(TEXT("DeadCorner never creates a Formula surface"),
		PendingScreen.LongShotResolution.Formula.bVisible);
	TestEqual(TEXT("CutInside pair begins with its own reveal identity"),
		PendingScreen.Interaction.CrossRollRevealKind,
		EFMCodexUMGCrossRollRevealKind::CutInsideShotDeadCornerA);

	FFMCodexLocalMatchInteractionView Terminal = Pending;
	Terminal.InteractionCategory = ECategory::AdvanceAfterTerminal;
	Terminal.ContinueActionLabel = TEXT("下一回合");
	Terminal.bTerminalPendingAdvance = true;
	Terminal.ResolutionFacts.bHasPendingRoll = false;
	Terminal.ResolutionFacts.NextPendingRollSequenceIndex = INDEX_NONE;
	Terminal.ResolutionFacts.Rolls[0] = Roll(
		0, EPostPurpose::PairedAttackA,
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EInitialTurnOrderPlayer::PlayerA, true, 6);
	Terminal.ResolutionFacts.Rolls[1] = Roll(
		1, EPostPurpose::PairedAttackB,
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EInitialTurnOrderPlayer::PlayerA, true, 5);
	AddDecision(Terminal, TEXT("DeadCorner.Outcome"),
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EMatchPlayResolutionDecisionOutcome::Goal);
	const FFMCodexUMGMatchScreenViewModel TerminalScreen = Build(Terminal);
	TestTrue(TEXT("Terminal reconstructs both committed dice"),
		TerminalScreen.LongShotResolution.bDeadCornerAVisible
			&& TerminalScreen.LongShotResolution.DeadCornerA == 6
			&& TerminalScreen.LongShotResolution.bDeadCornerBVisible
			&& TerminalScreen.LongShotResolution.DeadCornerB == 5);
	TestEqual(TEXT("DeadCorner uses centralized CutInside Narrative"),
		TerminalScreen.LongShotResolution.NarrativeHeadline,
		FString(TEXT("萨卡内切直射死角破门！")));
	TestTrue(TEXT("DeadCorner terminal owns NextRound"),
		TerminalScreen.LongShotResolution.PrimaryAction.Claims(
			TerminalScreen.Interaction.PrimaryAction));
	TestFalse(TEXT("Terminal DeadCorner remains outcome-only"),
		TerminalScreen.LongShotResolution.Formula.bVisible);

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(PendingScreen);
	TestEqual(TEXT("Central paired CTA suppresses lower duplicate"),
		Screen->GetInteractionPanel()->GetVisibility(),
		ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCutInsideProductionTypedRoutingContractTest,
	"FMCodex.LocalPlay.CutInsideProduction.04.TypedRoutingContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCutInsideProductionTypedRoutingContractTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	FString ScreenSource;
	const FString ScreenPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"));
	TestTrue(TEXT("Screen routing source is readable"),
		FFileHelper::LoadFileToString(ScreenSource, *ScreenPath));
	TestTrue(TEXT("Direct Attack dispatches typed Controller request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollCutInsideShotDirectAttack();")));
	TestTrue(TEXT("Direct Defense dispatches typed Controller request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollCutInsideShotDirectDefense();")));
	TestTrue(TEXT("DeadCorner dispatches one typed paired request"),
		ScreenSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner:"))
			&& ScreenSource.Contains(TEXT("MatchController->RollCutInsideShotDeadCorner();")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCutInsideProductionPairedRevealTest,
	"FMCodex.LocalPlay.CutInsideProduction.05.PairedReveal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCutInsideProductionPairedRevealTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView Pending = BaseView(
		ECategory::RollCutInsideShotDeadCorner);
	Pending.ContinueActionLabel = TEXT("进攻方掷内切死角双骰");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	AddBranch(Pending, EMatchPlayCutInsideShotActualBranch::DeadCorner);
	Pending.ResolutionFacts.Rolls = {
		Roll(0, EPostPurpose::PairedAttackA,
			EMatchPlayResolutionRollSemantics::OutcomeDecision,
			EInitialTurnOrderPlayer::PlayerA, false, 0),
		Roll(1, EPostPurpose::PairedAttackB,
			EMatchPlayResolutionRollSemantics::OutcomeDecision,
			EInitialTurnOrderPlayer::PlayerA, false, 0)
	};
	const FFMCodexUMGMatchScreenViewModel PendingScreen = Build(Pending);

	FFMCodexLocalMatchInteractionView Terminal = Pending;
	Terminal.InteractionCategory = ECategory::AdvanceAfterTerminal;
	Terminal.ContinueActionLabel = TEXT("下一回合");
	Terminal.bTerminalPendingAdvance = true;
	Terminal.ResolutionFacts.bHasPendingRoll = false;
	Terminal.ResolutionFacts.NextPendingRollSequenceIndex = INDEX_NONE;
	Terminal.ResolutionFacts.Rolls[0] = Roll(
		0, EPostPurpose::PairedAttackA,
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EInitialTurnOrderPlayer::PlayerA, true, 6);
	Terminal.ResolutionFacts.Rolls[1] = Roll(
		1, EPostPurpose::PairedAttackB,
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EInitialTurnOrderPlayer::PlayerA, true, 5);
	AddDecision(Terminal, TEXT("DeadCorner.Outcome"),
		EMatchPlayResolutionRollSemantics::OutcomeDecision,
		EMatchPlayResolutionDecisionOutcome::Goal);
	const FFMCodexUMGMatchScreenViewModel TerminalScreen = Build(Terminal);

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(PendingScreen);
	Screen->BeginPendingCrossRollRevealForTesting();
	Screen->RefreshFromPresentation(TerminalScreen);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& First =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("First CutInside hold discloses pair A"),
		First.bDeadCornerAVisible);
	TestFalse(TEXT("First CutInside hold keeps pair B covered"),
		First.bDeadCornerBVisible);

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& SecondRolling =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second CutInside reveal retains pair A"),
		SecondRolling.bDeadCornerAVisible);
	TestFalse(TEXT("Second CutInside reveal does not leak pair B"),
		SecondRolling.bDeadCornerBVisible);
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& SecondHeld =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second CutInside hold discloses pair B"),
		SecondHeld.bDeadCornerBVisible
			&& SecondHeld.RollReel.bAuthoritativeValue);

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& Settled =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Settled CutInside pair reconstructs terminal facts"),
		Settled.bDeadCornerAVisible && Settled.bDeadCornerBVisible
			&& Settled.bNarrativeAvailable);
	TestEqual(TEXT("CutInside pair does not replay after settlement"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settled);
	return true;
}

#endif
