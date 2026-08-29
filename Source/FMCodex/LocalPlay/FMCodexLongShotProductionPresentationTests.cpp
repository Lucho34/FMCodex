#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"

namespace FMCodexLongShotProductionPresentationTests
{
	FFMCodexLocalMatchInteractionView BaseLongShotView(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bCurrentAttackActive = true;
		View.AttackSequence = 7;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.PresentedActionType = ESkillRuleType::LongShot;
		View.InteractionCategory = Category;
		View.ActionLabel = TEXT("Long Shot");
		return View;
	}

	void AddLongShotBranch(
		FFMCodexLocalMatchInteractionView& View,
		const EMatchPlayLongShotActualBranch Branch)
	{
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		View.ResolutionFacts.bSuccess = true;
		View.ResolutionFacts.bHasFacts = true;
		View.ResolutionFacts.AttackSequence = View.AttackSequence;
		View.ResolutionFacts.ActionType = ESkillRuleType::LongShot;
		View.ResolutionFacts.bHasActualBranch = true;
		View.ResolutionFacts.ActualBranch.ActionType = ESkillRuleType::LongShot;
		View.ResolutionFacts.ActualBranch.LongShot = Branch;
	}

	FMatchPlayResolutionRollFact DeadRoll(
		const int32 Sequence,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose,
		const int32 RawD6,
		const bool bResolved)
	{
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = Sequence;
		Roll.PostRoutePurpose = Purpose;
		Roll.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
		Roll.OwningSide = EInitialTurnOrderPlayer::PlayerA;
		Roll.bResolved = bResolved;
		Roll.RawD6 = RawD6;
		return Roll;
	}

	FMatchPlayResolutionRollFact FormulaRoll(
		const int32 Sequence,
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose,
		const EInitialTurnOrderPlayer Side,
		const int32 RawD6,
		const bool bResolved)
	{
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = Sequence;
		Roll.PostRoutePurpose = Purpose;
		Roll.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Roll.OwningSide = Side;
		Roll.bResolved = bResolved;
		Roll.RawD6 = RawD6;
		return Roll;
	}

	FMatchPlayResolutionFormulaTermFact RawTerm(
		const int32 Sequence, const bool bResolved, const int32 RawD6)
	{
		FMatchPlayResolutionFormulaTermFact Term;
		Term.Kind = EMatchPlayResolutionFormulaTermKind::RawRoll;
		Term.RollSequenceIndex = Sequence;
		Term.bResolved = bResolved;
		Term.SourceValue = bResolved ? RawD6 : 0;
		Term.Contribution = Term.SourceValue;
		return Term;
	}

	FMatchPlayResolutionFormulaTermFact AttributeTerm(
		const EMatchPlayResolutionFormulaAttribute Attribute,
		const float Value)
	{
		FMatchPlayResolutionFormulaTermFact Term;
		Term.Kind = EMatchPlayResolutionFormulaTermKind::Attribute;
		Term.Attribute = Attribute;
		Term.SourceValue = Value;
		Term.Contribution = Value;
		return Term;
	}

	void AddDirectContest(
		FFMCodexLocalMatchInteractionView& View,
		const bool bAttackResolved,
		const int32 AttackD6,
		const bool bDefenseResolved,
		const int32 DefenseD6)
	{
		AddLongShotBranch(View, EMatchPlayLongShotActualBranch::DirectShot);
		View.ResolutionFacts.Rolls = {
			FormulaRoll(0, EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack,
				EInitialTurnOrderPlayer::PlayerA, AttackD6, bAttackResolved),
			FormulaRoll(1, EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense,
				EInitialTurnOrderPlayer::PlayerB, DefenseD6, bDefenseResolved)
		};
		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = TEXT("LongShot.DirectShot");
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms = {
			AttributeTerm(EMatchPlayResolutionFormulaAttribute::LongShot, 4.0f),
			RawTerm(0, bAttackResolved, AttackD6)
		};
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 4.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved ? 4.0f + AttackD6 : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms = {
			AttributeTerm(EMatchPlayResolutionFormulaAttribute::Tackling, 3.0f),
			RawTerm(1, bDefenseResolved, DefenseD6)
		};
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = 3.0f;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved ? 3.0f + DefenseD6 : 0.0f;
		View.ResolutionFacts.FormulaContests = { Contest };
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLongShotProductionBranchOwnershipTest,
	"FMCodex.LocalPlay.LongShotProduction.01.BranchOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLongShotProductionBranchOwnershipTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLongShotProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView View = BaseLongShotView(
		EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch);
	View.BranchIntentOptions = {
		EMatchPlayElectiveBranchIntent::DirectShot,
		EMatchPlayElectiveBranchIntent::DeadCorner
	};
	const FFMCodexUMGMatchScreenViewModel Screen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString());
	TestTrue(TEXT("LongShot production shell is visible"),
		Screen.LongShotResolution.bVisible);
	TestTrue(TEXT("Production shell suppresses generic resolution"),
		Screen.LongShotResolution.bSuppressLegacyResolution);
	TestEqual(TEXT("Branch stage is explicit"),
		Screen.LongShotResolution.Stage,
		EFMCodexUMGLongShotStage::BranchChoice);
	TestEqual(TEXT("Both authority-projected choices are central"),
		Screen.LongShotResolution.BranchChoices.Num(), 2);
	if (Screen.LongShotResolution.BranchChoices.Num() == 2)
	{
		TestEqual(TEXT("Direct primary copy is exact"),
			Screen.LongShotResolution.BranchChoices[0].Label,
			FString(TEXT("直接射门")));
		TestEqual(TEXT("Direct secondary copy is exact"),
			Screen.LongShotResolution.BranchChoices[0].SecondaryLabel,
			FString(TEXT("（看远射、抢断、门将站位）")));
		TestEqual(TEXT("Dead-corner primary copy is exact"),
			Screen.LongShotResolution.BranchChoices[1].Label,
			FString(TEXT("射向死角")));
		TestEqual(TEXT("Dead-corner secondary copy is exact"),
			Screen.LongShotResolution.BranchChoices[1].SecondaryLabel,
			FString(TEXT("（只看两枚掷点）")));
	}
	TestEqual(TEXT("Typed category remains LongShot-specific"),
		Screen.Interaction.Category,
		EFMCodexUMGInteractionCategory::SelectLongShotBranch);

	UFMCodexLongShotResolutionSurfaceWidget* Surface =
		NewObject<UFMCodexLongShotResolutionSurfaceWidget>(
			GetTransientPackage());
	Surface->TakeWidget();
	Surface->RefreshFromPresentation(Screen.LongShotResolution);
	const auto& Widgets = Surface->GetBranchChoiceWidgets();
	TestEqual(TEXT("LongShot renders two horizontal option widgets"),
		Widgets.Num(), 2);
	if (Widgets.Num() == 2)
	{
		UFMCodexInteractionOptionWidget* Direct = Widgets[0];
		UFMCodexInteractionOptionWidget* Dead = Widgets[1];
		Direct->TakeWidget();
		Dead->TakeWidget();
		UButton* DirectButton = Cast<UButton>(
			Direct->GetWidgetFromName(TEXT("InteractionOptionButton")));
		UButton* DeadButton = Cast<UButton>(
			Dead->GetWidgetFromName(TEXT("InteractionOptionButton")));
		UTextBlock* DirectPrimary = Cast<UTextBlock>(
			Direct->GetWidgetFromName(TEXT("InteractionOptionLabel")));
		UTextBlock* DirectSecondary = Cast<UTextBlock>(
			Direct->GetWidgetFromName(TEXT("InteractionOptionSecondaryLabel")));
		UTextBlock* DeadSecondary = Cast<UTextBlock>(
			Dead->GetWidgetFromName(TEXT("InteractionOptionSecondaryLabel")));
		USizeBox* DirectBounds = Cast<USizeBox>(
			Direct->GetWidgetFromName(TEXT("InteractionOptionBounds")));
		USizeBox* DeadBounds = Cast<USizeBox>(
			Dead->GetWidgetFromName(TEXT("InteractionOptionBounds")));
		TestTrue(TEXT("Both complete two-line options are clickable"),
			DirectButton != nullptr && DirectButton->GetIsEnabled()
				&& DeadButton != nullptr && DeadButton->GetIsEnabled());
		TestTrue(TEXT("Both secondary lines are visible without wrapping"),
			DirectPrimary != nullptr && !DirectPrimary->GetAutoWrapText()
				&& DirectSecondary != nullptr
				&& DirectSecondary->GetVisibility()
					== ESlateVisibility::HitTestInvisible
				&& !DirectSecondary->GetAutoWrapText()
				&& DeadSecondary != nullptr
				&& DeadSecondary->GetVisibility()
					== ESlateVisibility::HitTestInvisible
				&& !DeadSecondary->GetAutoWrapText());
		TestTrue(TEXT("Choice geometry is equal and stable"),
			DirectBounds != nullptr && DeadBounds != nullptr
				&& DirectBounds->GetWidthOverride()
					== DeadBounds->GetWidthOverride()
				&& DirectBounds->GetHeightOverride()
					== DeadBounds->GetHeightOverride());
		if (DirectButton == nullptr || DeadButton == nullptr)
		{
			AddError(TEXT("LongShot branch option buttons were not constructed."));
			return false;
		}

		Surface->ResetBranchDispatchForTesting();
		TestTrue(TEXT("Static branch choices bind no hover behavior"),
			!DirectButton->OnHovered.IsBound()
				&& !DirectButton->OnUnhovered.IsBound()
				&& !DeadButton->OnHovered.IsBound()
				&& !DeadButton->OnUnhovered.IsBound()
				&& Surface->GetBranchDispatchCountForTesting() == 0);
		DirectButton->OnClicked.Broadcast();
		TestTrue(TEXT("Direct click dispatches exactly one typed intent"),
			Surface->GetBranchDispatchCountForTesting() == 1
				&& Surface->GetLastBranchDispatchForTesting()
					== EFMCodexUMGBranchIntent::DirectShot);
		Surface->ResetBranchDispatchForTesting();
		DeadButton->OnClicked.Broadcast();
		TestTrue(TEXT("Dead-corner click dispatches exactly one typed intent"),
			Surface->GetBranchDispatchCountForTesting() == 1
				&& Surface->GetLastBranchDispatchForTesting()
					== EFMCodexUMGBranchIntent::DeadCorner);
	}

	FFMCodexLocalMatchResolutionFeedback Rejected;
	Rejected.bVisible = true;
	Rejected.bRejected = true;
	const FFMCodexUMGMatchScreenViewModel Diagnostic =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Rejected, TEXT("rejected"));
	TestFalse(TEXT("Rejection releases generic diagnostic ownership"),
		Diagnostic.LongShotResolution.bSuppressLegacyResolution);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLongShotProductionDeadCornerFactsTest,
	"FMCodex.LocalPlay.LongShotProduction.02.DeadCornerFactsAndCTA",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLongShotProductionDeadCornerFactsTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLongShotProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView Pending = BaseLongShotView(
		EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner);
	AddLongShotBranch(Pending, EMatchPlayLongShotActualBranch::DeadCorner);
	Pending.ContinueActionLabel = TEXT("进攻方掷远射双骰");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	Pending.ResolutionFacts.Rolls = {
		DeadRoll(0, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA,
			0, false),
		DeadRoll(1, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB,
			0, false)
	};
	const FFMCodexUMGMatchScreenViewModel PendingScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Pending, FFMCodexLocalMatchResolutionFeedback(), FString());
	TestEqual(TEXT("DeadCorner exposes exactly one typed player command"),
		PendingScreen.Interaction.Category,
		EFMCodexUMGInteractionCategory::RollLongShotDeadCorner);
	TestTrue(TEXT("DeadCorner central surface claims the command"),
		PendingScreen.LongShotResolution.PrimaryAction.Claims(
			PendingScreen.Interaction.PrimaryAction));
	TestEqual(TEXT("DeadCorner hint uses canonical compact wording"),
		PendingScreen.LongShotResolution.OutcomeHintLabel,
		FString(TEXT("合计 11–12：进球 ｜ 2–10：未进")));
	TestEqual(TEXT("DeadCorner reveal begins with pair A"),
		PendingScreen.Interaction.CrossRollRevealKind,
		EFMCodexUMGCrossRollRevealKind::LongShotDeadCornerA);

	FFMCodexLocalMatchInteractionView Terminal = Pending;
	Terminal.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
	Terminal.ContinueActionLabel = TEXT("下一回合");
	Terminal.bTerminalPendingAdvance = true;
	Terminal.ResolutionFacts.bHasPendingRoll = false;
	Terminal.ResolutionFacts.NextPendingRollSequenceIndex = INDEX_NONE;
	Terminal.ResolutionFacts.Rolls[0] = DeadRoll(
		0, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA, 5, true);
	Terminal.ResolutionFacts.Rolls[1] = DeadRoll(
		1, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB, 6, true);
	FMatchPlayResolutionDecisionFact Decision;
	Decision.DecisionId = TEXT("DeadCorner.Outcome");
	Decision.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
	Decision.RollSequenceIndices = { 0, 1 };
	Decision.bResolved = true;
	Decision.Outcome = EMatchPlayResolutionDecisionOutcome::Goal;
	Terminal.ResolutionFacts.Decisions = { Decision };
	const FFMCodexUMGMatchScreenViewModel TerminalScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Terminal, FFMCodexLocalMatchResolutionFeedback(), FString());
	TestTrue(TEXT("Completed snapshot reconstructs first die"),
		TerminalScreen.LongShotResolution.bDeadCornerAVisible);
	TestEqual(TEXT("First die remains authoritative"),
		TerminalScreen.LongShotResolution.DeadCornerA, 5);
	TestTrue(TEXT("Completed snapshot reconstructs second die"),
		TerminalScreen.LongShotResolution.bDeadCornerBVisible);
	TestEqual(TEXT("Second die remains authoritative"),
		TerminalScreen.LongShotResolution.DeadCornerB, 6);
	TestTrue(TEXT("Authority outcome projects shared narrative"),
		TerminalScreen.LongShotResolution.bNarrativeAvailable);
	TestEqual(TEXT("Terminal CTA is resolution-local"),
		TerminalScreen.LongShotResolution.ContinueActionLabel,
		FString(TEXT("下一回合")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLongShotProductionDirectFormulaAndGateTest,
	"FMCodex.LocalPlay.LongShotProduction.03.DirectFormulaAndImmediateMiss",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLongShotProductionDirectFormulaAndGateTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLongShotProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView Pending = BaseLongShotView(
		EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack);
	AddDirectContest(Pending, false, 0, false, 0);
	Pending.ContinueActionLabel = TEXT("进攻方掷远射点数");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	const FFMCodexUMGMatchScreenViewModel PendingScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Pending, FFMCodexLocalMatchResolutionFeedback(), FString());
	TestTrue(TEXT("Direct uses the shared authoritative formula surface"),
		PendingScreen.LongShotResolution.Formula.bVisible);
	TestTrue(TEXT("Direct formula claims its typed attack roll"),
		PendingScreen.LongShotResolution.Formula.PrimaryAction.Claims(
			PendingScreen.Interaction.PrimaryAction));
	TestTrue(TEXT("Direct hint exposes its authority gate"),
		PendingScreen.LongShotResolution.OutcomeHintLabel.Contains(TEXT("1–2")));

	FFMCodexLocalMatchInteractionView Miss = Pending;
	Miss.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
	Miss.ContinueActionLabel = TEXT("下一回合");
	Miss.bTerminalPendingAdvance = true;
	AddDirectContest(Miss, true, 2, false, 0);
	Miss.ResolutionFacts.bHasPendingRoll = false;
	FMatchPlayResolutionFormulaContestFact& Contest =
		Miss.ResolutionFacts.FormulaContests[0];
	Contest.Application =
		EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate;
	FMatchPlayResolutionDecisionFact Decision;
	Decision.DecisionId = TEXT("LongShot.DirectShot.Outcome");
	Decision.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
	Decision.RollSequenceIndices = { 0, 1 };
	Decision.bResolved = true;
	Decision.Outcome = EMatchPlayResolutionDecisionOutcome::ImmediateMiss;
	Miss.ResolutionFacts.Decisions = { Decision };
	const FFMCodexUMGMatchScreenViewModel MissScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Miss, FFMCodexLocalMatchResolutionFeedback(), FString());
	TestFalse(TEXT("1-2 skips formula rows instead of inventing defense"),
		MissScreen.LongShotResolution.Formula.bShowFormulaRows);
	TestTrue(TEXT("Immediate miss uses shared narrative"),
		MissScreen.LongShotResolution.Formula.bNarrativeAvailable);
	TestEqual(TEXT("Immediate miss title is player-facing"),
		MissScreen.LongShotResolution.Formula.ResultTitle,
		FString(TEXT("射门偏出")));
	TestTrue(TEXT("Fresh terminal snapshot owns Next Round"),
		MissScreen.LongShotResolution.Formula.PrimaryAction.Claims(
			MissScreen.Interaction.PrimaryAction));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLongShotProductionSurfaceExclusivityTest,
	"FMCodex.LocalPlay.LongShotProduction.04.SurfaceExclusivityAndDiagnostic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLongShotProductionSurfaceExclusivityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLongShotProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView View = BaseLongShotView(
		EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch);
	View.BranchIntentOptions = {
		EMatchPlayElectiveBranchIntent::DirectShot,
		EMatchPlayElectiveBranchIntent::DeadCorner
	};
	FFMCodexUMGMatchScreenViewModel Production =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString());
	Production.Resolution.bVisible = true;
	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Production);
	TestFalse(TEXT("Production ownership collapses generic resolution root"),
		Screen->IsLegacyResolutionOverlayVisible());
	TestTrue(TEXT("Production surface remains visible"),
		Screen->GetLongShotResolutionSurface() != nullptr
			&& Screen->GetLongShotResolutionSurface()->GetPresentation().bVisible);
	TestEqual(TEXT("Central branch choice collapses lower duplicate"),
		Screen->GetInteractionPanel()->GetVisibility(),
		ESlateVisibility::Collapsed);

	FFMCodexLocalMatchResolutionFeedback Rejected;
	Rejected.bVisible = true;
	Rejected.bRejected = true;
	FFMCodexUMGMatchScreenViewModel Diagnostic =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Rejected, TEXT("rejected"));
	Screen->RefreshFromPresentation(Diagnostic);
	TestTrue(TEXT("Rejection restores generic diagnostic root"),
		Screen->IsLegacyResolutionOverlayVisible());
	TestFalse(TEXT("Rejection collapses production root"),
		Screen->GetLongShotResolutionSurface()->GetPresentation().bVisible);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLongShotProductionDeadCornerSequentialRevealTest,
	"FMCodex.LocalPlay.LongShotProduction.05.DeadCornerSequentialReveal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLongShotProductionDeadCornerSequentialRevealTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLongShotProductionPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView Pending = BaseLongShotView(
		EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner);
	AddLongShotBranch(Pending, EMatchPlayLongShotActualBranch::DeadCorner);
	Pending.ContinueActionLabel = TEXT("进攻方掷远射双骰");
	Pending.ResolutionFacts.bHasPendingRoll = true;
	Pending.ResolutionFacts.NextPendingRollSequenceIndex = 0;
	Pending.ResolutionFacts.Rolls = {
		DeadRoll(0, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA,
			0, false),
		DeadRoll(1, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB,
			0, false)
	};
	const FFMCodexUMGMatchScreenViewModel PendingScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Pending, FFMCodexLocalMatchResolutionFeedback(), FString());

	FFMCodexLocalMatchInteractionView Terminal = Pending;
	Terminal.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal;
	Terminal.ContinueActionLabel = TEXT("下一回合");
	Terminal.bTerminalPendingAdvance = true;
	Terminal.ResolutionFacts.bHasPendingRoll = false;
	Terminal.ResolutionFacts.NextPendingRollSequenceIndex = INDEX_NONE;
	Terminal.ResolutionFacts.Rolls[0] = DeadRoll(
		0, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA, 5, true);
	Terminal.ResolutionFacts.Rolls[1] = DeadRoll(
		1, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB, 6, true);
	FMatchPlayResolutionDecisionFact Decision;
	Decision.DecisionId = TEXT("DeadCorner.Outcome");
	Decision.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
	Decision.RollSequenceIndices = { 0, 1 };
	Decision.bResolved = true;
	Decision.Outcome = EMatchPlayResolutionDecisionOutcome::Goal;
	Terminal.ResolutionFacts.Decisions = { Decision };
	const FFMCodexUMGMatchScreenViewModel TerminalScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Terminal, FFMCodexLocalMatchResolutionFeedback(), FString());

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
	TestTrue(TEXT("First hold discloses pair A"), First.bDeadCornerAVisible);
	TestFalse(TEXT("First hold does not leak pair B"), First.bDeadCornerBVisible);

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& SecondRolling =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second reveal retains pair A"),
		SecondRolling.bDeadCornerAVisible);
	TestFalse(TEXT("Second roll stays covered before hold"),
		SecondRolling.bDeadCornerBVisible);
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& SecondHeld =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second hold discloses pair B"),
		SecondHeld.bDeadCornerBVisible);
	TestTrue(TEXT("Second hold exposes authoritative reel result"),
		SecondHeld.RollReel.bAuthoritativeValue
			&& SecondHeld.RollReel.bResultHold);

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& Settled =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Settled terminal reconstructs both dice"),
		Settled.bDeadCornerAVisible && Settled.bDeadCornerBVisible);
	TestTrue(TEXT("Settled terminal discloses narrative"),
		Settled.bNarrativeAvailable);
	TestEqual(TEXT("No historical pair replay remains active"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settled);
	return true;
}

#endif
