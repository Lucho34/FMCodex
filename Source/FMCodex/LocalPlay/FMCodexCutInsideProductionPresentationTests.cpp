#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexRollPresentationStyle.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"

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
	View.SelectedCarrierCardId=CarrierId;
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
			FString(TEXT("射向死角")));
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
	for (const auto Skill : {ESkillRuleType::LongShot, ESkillRuleType::CutInsideShot})
	{
		auto Variant=View; Variant.PresentedActionType=Skill;
		Variant.InteractionCategory=Skill==ESkillRuleType::LongShot?ECategory::SelectLongShotBranch:ECategory::SelectBranchIntent;
		auto Choice=Build(Variant);
		MatchScreen->RefreshFromPresentation(Choice);
		auto Text=[MatchScreen](const TCHAR* Name){return CastChecked<UTextBlock>(MatchScreen->GetWidgetFromName(Name))->GetText().ToString();};
		TestEqual(TEXT("Shared attack context uses canonical selected Carrier"),Text(TEXT("TheaterAttackName0")),FString(TEXT("萨卡")));
		TestEqual(TEXT("Context keeps ordinary Carrier role"),Text(TEXT("TheaterAttackRole0")),FString(TEXT("持球")));
		TestTrue(TEXT("Direct summary keeps attributes and miss range without modifier or GK exposition"),
			Text(TEXT("TheaterNearDirectHint")).Contains(Skill==ESkillRuleType::LongShot?TEXT("远射"):TEXT("盘带 × 0.5"))
			&& Text(TEXT("TheaterNearDirectHint")).Contains(TEXT("对抗盯人：抢断\n进攻掷点 1–2：射门偏出"))
			&& !Text(TEXT("TheaterNearDirectHint")).Contains(TEXT("+ 2"))
			&& !Text(TEXT("TheaterNearDirectHint")).Contains(TEXT("门将")));
		TestEqual(TEXT("Pair explanation contains only the two-dice concept and success range"),
			Text(TEXT("TheaterNearCombinationHint")),FString(TEXT("进攻方依次掷两枚骰子\n总和 11–12：进球")));
		TestEqual(TEXT("Both shot branch consumers enter Theater"),MatchScreen->GetWidgetFromName(TEXT("ResolutionTheater"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
		TestEqual(TEXT("Legacy branch modal is hidden"),MatchScreen->GetLongShotResolutionSurface()->GetVisibility(),ESlateVisibility::Collapsed);
		TestTrue(TEXT("Both projected branch choices remain enabled"),MatchScreen->GetWidgetFromName(TEXT("TheaterNearDirect"))->GetIsEnabled()
			&& MatchScreen->GetWidgetFromName(TEXT("TheaterNearCombination"))->GetIsEnabled());
		Choice.LongShotResolution.BranchChoices.RemoveAll([](const auto& C){return C.Intent==EFMCodexUMGBranchIntent::DeadCorner;});
		MatchScreen->RefreshFromPresentation(Choice);
		TestFalse(TEXT("Absent safe choice stays disabled"),MatchScreen->GetWidgetFromName(TEXT("TheaterNearCombination"))->GetIsEnabled());
		TestTrue(TEXT("Unavailable strip names the disabled option without inventing a threshold"),Text(TEXT("TheaterDetail")).Contains(TEXT("当前不可选择射向死角")));
		Choice.LongShotResolution.BranchChoices.Empty(); Choice.bActionWaitPromptReadOnly=true;
		MatchScreen->RefreshFromPresentation(Choice);
		TestFalse(TEXT("Waiting viewer cannot execute either choice"),MatchScreen->GetWidgetFromName(TEXT("TheaterNearDirect"))->GetIsEnabled()
			|| MatchScreen->GetWidgetFromName(TEXT("TheaterNearCombination"))->GetIsEnabled());
		TestEqual(TEXT("Waiting strip explains the expected action"),Text(TEXT("TheaterDetail")),FString(TEXT("等待进攻方选择射门方式")));
	}
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
		FString(TEXT("1–2：射门偏出")));
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
	TestTrue(TEXT("Defense pending no longer displays the attack helper"),
		AttackOnlyScreen.LongShotResolution.OutcomeHintLabel.IsEmpty());
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
	TestTrue(TEXT("Immediate miss no longer displays the attack helper"),
		ImmediateScreen.LongShotResolution.OutcomeHintLabel.IsEmpty());
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
		FString(TEXT("萨卡内切射向死角破门！")));
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
	FString AdapterSource;
	TestTrue(TEXT("Local action adapter source is readable"),
		FFileHelper::LoadFileToString(AdapterSource, *FPaths::Combine(FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"))));
	const FString ScreenPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"));
	TestTrue(TEXT("Screen routing source is readable"),
		FFileHelper::LoadFileToString(ScreenSource, *ScreenPath));
	TestTrue(TEXT("Direct Attack dispatches typed Controller request"),
		AdapterSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack:"))
			&& AdapterSource.Contains(TEXT("RollCutInsideShotDirectAttack();")));
	TestTrue(TEXT("Direct Defense dispatches typed Controller request"),
		AdapterSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense:"))
			&& AdapterSource.Contains(TEXT("RollCutInsideShotDirectDefense();")));
	TestTrue(TEXT("DeadCorner dispatches one typed paired request"),
		AdapterSource.Contains(TEXT("case EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner:"))
			&& AdapterSource.Contains(TEXT("RollCutInsideShotDeadCorner();")));
	TestTrue(TEXT("Shared continuation reaches the Local backend"),
		ScreenSource.Contains(TEXT("SubmitScreenRequest(EFMCodexMatchScreenIntent::Continue)"))
		&& ScreenSource.Contains(TEXT("MatchBackend->SubmitScreenIntent(Request)")));
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
	auto* A=CastChecked<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(TEXT("TheaterPairAReel")));
	auto* B=CastChecked<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(TEXT("TheaterPairBReel")));
	auto Text=[Screen](const TCHAR* Name){return CastChecked<UTextBlock>(Screen->GetWidgetFromName(Name))->GetText().ToString();};
	TestTrue(TEXT("Pair reuses FK TheaterInline operands"),A->UsesTheaterInlineSkin() && B->UsesTheaterInlineSkin());
	TestEqual(TEXT("DeadCorner retains canonical attacking identity"),Text(TEXT("TheaterAttackName0")),FString(TEXT("萨卡")));
	TestEqual(TEXT("DeadCorner has no fake Formula"),Screen->GetWidgetFromName(TEXT("TheaterAttackValue"))->GetVisibility(),ESlateVisibility::Collapsed);
	TestEqual(TEXT("DeadCorner has no defense"),Screen->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
	TestFalse(TEXT("Second die is not shown during first motion"),B->GetPresentation().bVisible);
	TestEqual(TEXT("First motion keeps the goal condition visible"),Text(TEXT("TheaterReasonSecondary")),FString(TEXT("两枚掷点总和达到 11–12：进球")));
	TestEqual(TEXT("Goal hint is actually visible"),Screen->GetWidgetFromName(TEXT("TheaterReasonSecondary"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
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
	TestTrue(TEXT("Modern second die moves while first remains landed"),Text(TEXT("TheaterPairARollValue"))==TEXT("6") && B->GetPresentation().bMoving);
	TestEqual(TEXT("Sum stays hidden during second motion"),Text(TEXT("TheaterPairTotal")),FString(TEXT("?")));
	TestEqual(TEXT("Second motion retains the same goal condition"),Text(TEXT("TheaterReasonSecondary")),FString(TEXT("两枚掷点总和达到 11–12：进球")));
	TestTrue(TEXT("CutInside second roll retains first die and target only"),
		SecondRolling.PairedRollResultLabel == TEXT("第一枚 D6：6")
			&& SecondRolling.OutcomeHintLabel
				== TEXT("两枚点数总和达到 11 或以上：进球")
			&& !SecondRolling.PrimaryAction.bVisible);
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& SecondHeld =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second CutInside hold discloses pair B"),
		SecondHeld.bDeadCornerBVisible
			&& SecondHeld.RollReel.bAuthoritativeValue);
	TestTrue(TEXT("CutInside second hold shows arithmetic and clears target"),
		SecondHeld.PairedRollResultLabel == TEXT("D6 6 + D6 5 = 11")
			&& SecondHeld.OutcomeHintLabel.IsEmpty());

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& Settled =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Settled CutInside pair reconstructs terminal facts"),
		Settled.bDeadCornerAVisible && Settled.bDeadCornerBVisible
			&& Settled.bNarrativeAvailable);
	TestEqual(TEXT("CutInside pair does not replay after settlement"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settled);
	TestTrue(TEXT("Modern pair retains both real values and sum at Outcome"),Text(TEXT("TheaterPairARollValue"))==TEXT("6") && Text(TEXT("TheaterPairBRollValue"))==TEXT("5") && Text(TEXT("TheaterPairTotal"))==TEXT("11"));
	TestTrue(TEXT("Existing narrative and terminal action are in Theater"),Screen->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& Screen->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetVisibility()!=ESlateVisibility::Collapsed);
	TestEqual(TEXT("Theater renders the complete canonical paired narrative"),
		CastChecked<URichTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().ToString(),
		FString(TEXT("萨卡内切射向死角<Goal>破门</>！")));
	Screen->RefreshFromPresentation(TerminalScreen);
	TestFalse(TEXT("Terminal refresh does not replay either die"),Screen->IsInlineFormulaRevealInputBlocked());
	TestEqual(TEXT("Goal reason explains the disclosed success condition"),Text(TEXT("TheaterReasonSecondary")),FString(TEXT("总和达到 11–12，进球")));
	// The other outcome uses the same safe decision, not a UI sum comparison.
	for (const auto Skill:{ESkillRuleType::LongShot,ESkillRuleType::CutInsideShot})
	{
		auto Miss=Terminal;
		Miss.PresentedActionType=Skill; Miss.ResolutionFacts.ActionType=Skill;
		Miss.ResolutionFacts.ActualBranch.ActionType=Skill;
		Miss.ResolutionFacts.ActualBranch.LongShot=EMatchPlayLongShotActualBranch::DeadCorner;
		Miss.ResolutionFacts.Rolls[0].RawD6=5;
		Miss.ResolutionFacts.Decisions[0].Outcome=EMatchPlayResolutionDecisionOutcome::Miss;
		auto* Rebuilt=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Rebuilt->TakeWidget(); Rebuilt->RefreshFromPresentation(Build(Miss));
		TestEqual(TEXT("Both shot consumers explain why the disclosed miss failed"),
			CastChecked<UTextBlock>(Rebuilt->GetWidgetFromName(TEXT("TheaterReasonSecondary")))->GetText().ToString(),FString(TEXT("总和未达到 11–12，未进球")));
		TestEqual(TEXT("The original arithmetic remains readable"),
			CastChecked<UTextBlock>(Rebuilt->GetWidgetFromName(TEXT("TheaterPairTotal")))->GetText().ToString(),FString(TEXT("10")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDirectShotTheaterFormulaTest,
	"FMCodex.LocalPlay.DirectShotTheater.FormulaAndSequentialRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDirectShotTheaterFormulaTest::RunTest(const FString&)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	auto Pending = BaseView(ECategory::RollCutInsideShotDirectAttack);
	AddDirectFacts(Pending, false, 0, false, 0);
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	// A distinctive supplied subtotal proves the renderer does not sum the terms.
	Pending.ResolutionFacts.FormulaContests[0].AttackRow.KnownNonRollSubtotal = 17.25f;
	auto Model = Build(Pending);
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget(); Screen->RefreshFromPresentation(Model);
	auto Shown = [Screen](const TCHAR* Name) { auto* W=Screen->GetWidgetFromName(Name); return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden; };
	auto Text = [this, Screen](const TCHAR* Name)
	{
		// Native tooltip children belong to the tree but are not in its main hierarchy.
		auto* Value = FindObject<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackBaseHover"))->GetOuter(), Name);
		return TestNotNull(Name, Value) ? Value->GetText().ToString() : FString();
	};
	TestTrue(TEXT("Explicit DirectShot consumer enters production Theater"),Shown(TEXT("ResolutionTheater")));
	TestEqual(TEXT("Base is the supplied scalar"),Text(TEXT("TheaterAttackNumber")),FText::AsNumber(17.25f).ToString());
	TestEqual(TEXT("Pending RHS is the supplied current subtotal"),Text(TEXT("TheaterAttackFinalNumber")),Model.LongShotResolution.Formula.AttackRow.DisplayedResultLabel);
	TestTrue(TEXT("Unresolved grammar has one pending operand and current RHS"),Shown(TEXT("TheaterAttackPending")) && Text(TEXT("TheaterAttackValueLabel"))==TEXT("当前值"));
	TestTrue(TEXT("Base hover explains projected Shooting and Dribbling coefficients"),
		CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("TheaterAttackBaseHover")))->GetToolTip()!=nullptr
		&& Text(TEXT("TheaterAttackBaseExplanation" )).Contains(TEXT("射门")) && Text(TEXT("TheaterAttackBaseExplanation")).Contains(TEXT("盘带"))
		&& Text(TEXT("TheaterAttackBaseExplanation")).Contains(TEXT("0.5")));
	TestTrue(TEXT("Only actual Marker and GK appear on defense"),Model.LongShotResolution.Formula.DefenseRow.Participants.Num()==2
		&& Text(TEXT("TheaterDefenseBaseExplanation")).Contains(TEXT("手控球")));
	for (const TCHAR* Prefix : {TEXT("TheaterAttack"),TEXT("TheaterDefense")})
	{
		auto* Reel=CastChecked<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("Reel")))));
		TestEqual(TEXT("Both Formula operands use TheaterInline"),Reel->GetVisualVariant(),EFMCodexRollVisualVariant::TheaterInline);
		TestNull(TEXT("Roll never gains an underline"),Screen->GetWidgetFromName(FName(*(FString(Prefix)+TEXT("RollUnderline")))));
	}
	Screen->BeginPendingCrossRollRevealForTesting();
	auto AttackOnly=BaseView(ECategory::RollCutInsideShotDirectDefense);
	AddDirectFacts(AttackOnly,true,4,false,0);
	AttackOnly.ResolutionFacts.bHasPendingRoll=true; AttackOnly.ResolutionFacts.NextPendingRollSequenceIndex=1;
	Screen->RefreshFromPresentation(Build(AttackOnly)); Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(1.2f);
	TestEqual(TEXT("DirectShot uses v2 capture timing rather than Legacy Cycling"),Screen->GetInlineFormulaRevealPhase(),EFMCodexUMGInlineFormulaRevealPhase::Settling);
	Screen->AdvanceInlineFormulaRevealForTesting(.6f);
	TestEqual(TEXT("Normal DirectShot shares neutral landed status"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(),FString(TEXT("进攻方掷点已落定")));
	Screen->AdvanceInlineFormulaRevealForTesting(3.f);
	TestEqual(TEXT("Attack lands on accepted D6"),Text(TEXT("TheaterAttackRollValue")),FString(TEXT("4")));
	Screen->BeginPendingCrossRollRevealForTesting();
	auto Complete=BaseView(ECategory::AdvanceAfterTerminal); Complete.bTerminalPendingAdvance=true; Complete.ContinueActionLabel=TEXT("下一回合");
	AddDirectFacts(Complete,true,4,true,3,EFormulaWinner::Defender);
	auto& Resolved=Complete.ResolutionFacts.FormulaContests[0].ResolvedResult;
	Resolved.WinReason=EFormulaWinReason::HigherFinalValue; Resolved.AttackerFinalValue=11; Resolved.DefenderFinalValue=14;
	AddDecision(Complete,TEXT("CutInsideShot.DirectShot.Outcome"),EMatchPlayResolutionRollSemantics::ArithmeticContest,EMatchPlayResolutionDecisionOutcome::Miss);
	Screen->RefreshFromPresentation(Build(Complete)); Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(.3f);
	TestTrue(TEXT("Defense motion retains static accepted attack operand"),Shown(TEXT("TheaterAttackRollValue")) && !Shown(TEXT("TheaterAttackReelHost")) && Shown(TEXT("TheaterDefenseReelHost")));
	TestEqual(TEXT("Attack value is never replaced by a placeholder"),Text(TEXT("TheaterAttackRollValue")),FString(TEXT("4")));
	TestFalse(TEXT("Final Outcome cannot preempt the defense reveal"),Shown(TEXT("TheaterOutcome")) || Shown(TEXT("TheaterPrimaryBounds")));
	Screen->AdvanceInlineFormulaRevealForTesting(5.f);
	TestEqual(TEXT("Resolved RHS consumes supplied Final"),Text(TEXT("TheaterDefenseFinalNumber")),FString(TEXT("14")));
	TestTrue(TEXT("Reason comes from authoritative WinReason"),Build(Complete).LongShotResolution.Formula.ResolutionReasonLabel.Contains(TEXT("高于")));
	const auto Landed=FMCodexRollPresentationStyle::AuthoritativeLandedValue();
	TestTrue(TEXT("Static Roll alone uses shared landed accent"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackRollValue")))->GetColorAndOpacity().GetSpecifiedColor().Equals(Landed));
	TestFalse(TEXT("Formula RHS is not the Roll accent"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackFinalNumber")))->GetColorAndOpacity().GetSpecifiedColor().Equals(Landed));
	TestTrue(TEXT("Existing terminal narrative and CTA become visible"),Shown(TEXT("TheaterOutcome")) && Shown(TEXT("TheaterPrimaryBounds")));
	Screen->RefreshFromPresentation(Build(Complete));
	TestFalse(TEXT("Repeated terminal refresh never replays accepted rolls"),Screen->IsInlineFormulaRevealInputBlocked());
	// The same renderer formats a safe GK tie reason; it cannot infer it from totals.
	Resolved.WinReason=EFormulaWinReason::DefenderWinsGoalkeeperTie;
	TestTrue(TEXT("Projected GK tie has its own explanation"),Build(Complete).LongShotResolution.Formula.ResolutionReasonLabel.Contains(TEXT("门将参与")));
	for (const auto Id : {TEXT("PassControl.PassAdvance"),TEXT("ThroughBall.Feet"),TEXT("LongShot.DeadCorner"),TEXT("CutInsideShot.DeadCorner")})
	{
		auto Other=Model; Other.LongShotResolution.Formula.ContestId=Id;
		TestFalse(TEXT("Non-target consumer cannot opt in through shared host"),FMCodexResolutionTheaterPrototype::WantsTheater(Other,Other.LongShotResolution.Formula));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDirectShotTheaterImmediateMissTest,
	"FMCodex.LocalPlay.DirectShotTheater.ImmediateMiss",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexDirectShotTheaterImmediateMissTest::RunTest(const FString&)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	auto Pending=BaseView(ECategory::RollCutInsideShotDirectAttack); AddDirectFacts(Pending,false,0,false,0);
	Pending.ResolutionFacts.bHasPendingRoll=true; Pending.ResolutionFacts.NextPendingRollSequenceIndex=0;
	auto Terminal=BaseView(ECategory::AdvanceAfterTerminal); Terminal.bTerminalPendingAdvance=true; Terminal.ContinueActionLabel=TEXT("下一回合");
	AddDirectFacts(Terminal,true,2,false,0);
	Terminal.ResolutionFacts.FormulaContests[0].Application=EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate;
	AddDecision(Terminal,TEXT("CutInsideShot.DirectShot.Outcome"),EMatchPlayResolutionRollSemantics::ArithmeticContest,EMatchPlayResolutionDecisionOutcome::ImmediateMiss);
	auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage()); Screen->TakeWidget();
	Screen->RefreshFromPresentation(Build(Pending)); Screen->BeginPendingCrossRollRevealForTesting();
	Screen->ForceLayoutPrepass();
	const FVector2D OriginalCardSize=Screen->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetDesiredSize();
	auto* AttackReel=CastChecked<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(TEXT("TheaterAttackReel")));
	Screen->RefreshFromPresentation(Build(Terminal)); Screen->PauseInlineFormulaRevealTimerForTesting();
	auto Shown=[Screen](const TCHAR* Name) {auto* W=Screen->GetWidgetFromName(Name); return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;};
	TestEqual(TEXT("Accepted gate has exactly one resolved roll fact"),Terminal.ResolutionFacts.Rolls.FilterByPredicate([](const auto& R){return R.bResolved;}).Num(),1);
	TestTrue(TEXT("Immediate miss retains the SAME attack operand widget"),AttackReel==Screen->GetWidgetFromName(TEXT("TheaterAttackReel")) && AttackReel->UsesTheaterInlineSkin());
	Screen->AdvanceInlineFormulaRevealForTesting(.3f);
	TestTrue(TEXT("Single real attack reel remains visible"),Shown(TEXT("TheaterAttackReelHost")) && AttackReel->GetPresentation().bVisible);
	TestTrue(TEXT("Undisclosed miss keeps the same pending composition as normal attack"),Shown(TEXT("TheaterDefensePanelBounds")) && Shown(TEXT("TheaterAttackBaseHover")));
	TestFalse(TEXT("No duplicate standalone die during the attack"),Shown(TEXT("TheaterRoll")));
	TestFalse(TEXT("No early Outcome or CTA"),Shown(TEXT("TheaterOutcome")) || Shown(TEXT("TheaterPrimaryBounds")));
	bool bSawLandedStatus=false;
	for (int32 Tick=0;Tick<150;++Tick)
	{
		Screen->AdvanceInlineFormulaRevealForTesting(.01f);
		const FString Status=CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString();
		if (AttackReel->GetPresentation().bAuthoritativeValue)
		{
			bSawLandedStatus=true;
			TestEqual(TEXT("The first landed frame and entire hold cannot retain rolling copy"),Status,FString(TEXT("进攻方掷点已落定")));
		}
		else TestFalse(TEXT("Before authoritative landing no landed status leaks"),Status.Contains(TEXT("已落定")));
	}
	TestTrue(TEXT("Boundary sampling reached the landed state"),bSawLandedStatus);
	Screen->ForceLayoutPrepass();
	TestEqual(TEXT("ImmediateMiss does not resize the attack silhouette"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterAttackSilhouetteBounds")))->GetHeightOverride(),178.f);
	TestTrue(TEXT("ImmediateMiss retains the attack card allocation"),Screen->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetDesiredSize().Equals(OriginalCardSize,.1f));
	for (const TCHAR* Name : {TEXT("TheaterDefensePanelBounds"),TEXT("TheaterAttackBaseHover"),TEXT("TheaterAttackResultColumn")})
		TestFalse(TEXT("Skipped Formula never paints candidate defense/Base/Final"),Shown(Name));
	TestEqual(TEXT("No VS comparison"),Screen->GetWidgetFromName(TEXT("TheaterVS"))->GetParent()->GetVisibility(),ESlateVisibility::Collapsed);
	TestFalse(TEXT("No duplicate standalone die after landing"),Shown(TEXT("TheaterRoll")));
	TestTrue(TEXT("Same modern roll lands on the actual miss die"),AttackReel->GetPresentation().bAuthoritativeValue && AttackReel->GetPresentation().CenterValue==2);
	TestTrue(TEXT("Actual hold retains the landed accent"),AttackReel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor().Equals(FMCodexRollPresentationStyle::AuthoritativeLandedValue()));
	TestFalse(TEXT("ResultHold still owns the final CTA"),Shown(TEXT("TheaterPrimaryBounds")));
	Screen->AdvanceInlineFormulaRevealForTesting(3.f);
	const auto Model=Build(Terminal);
	TestTrue(TEXT("Existing immediate-miss narrative remains exact"),Model.LongShotResolution.Formula.NarrativeHeadline==TEXT("萨卡内切后射门偏出。") && Shown(TEXT("TheaterOutcome")));
	const auto& Reason=Model.LongShotResolution.Formula.ResolutionReasonLabel;
	TestTrue(TEXT("Skipped-path explanation does not claim Winner/stamina/GK"),Reason.Contains(TEXT("不进行攻防比较")) && !Reason.Contains(TEXT("体力")) && !Reason.Contains(TEXT("门将")) && !Reason.Contains(TEXT("获胜")));
	TestEqual(TEXT("No winner badge on the failed attack"),Screen->GetWidgetFromName(TEXT("TheaterAttackBadge"))->GetVisibility(),ESlateVisibility::Hidden);
	TestTrue(TEXT("Original terminal CTA available after the one roll"),Shown(TEXT("TheaterPrimaryBounds")));
	Screen->RefreshFromPresentation(Model);
	TestFalse(TEXT("Repeated View does not manufacture a defense event"),Screen->IsInlineFormulaRevealInputBlocked());
	TestFalse(TEXT("Resolution does not open Full Card"),Screen->IsDetailOverlayVisible());
	return true;
}

#endif
