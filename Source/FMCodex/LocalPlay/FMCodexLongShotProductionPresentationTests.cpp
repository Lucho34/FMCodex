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
#include "Misc/ScopeExit.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Internationalization/Text.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexFormulaTooltipTestUtils.h"
#include "FMCodexTacticalScene.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#if WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#endif


namespace FMCodexLongShotProductionPresentationTests
{
	FFMCodexLocalMatchInteractionView BaseLongShotView(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
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
			AttributeTerm(EMatchPlayResolutionFormulaAttribute::Shooting, 4.0f),
			RawTerm(0, bAttackResolved, AttackD6)
		};
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 4.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved ? 4.0f + AttackD6 : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms = {
			AttributeTerm(EMatchPlayResolutionFormulaAttribute::Defense, 3.0f),
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
	TestFalse(TEXT("Unselected LongShot stays on the board"),Screen.LongShotResolution.bVisible);
	TestEqual(TEXT("Authority still exposes both original methods"),Screen.Interaction.BranchChoices.Num(),2);
	auto* Reopened=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());Reopened->TakeWidget();Reopened->RefreshFromPresentation(Screen);
	TestFalse(TEXT("Rebuilt branch choice has no pitch theater"),Reopened->GetTacticalScene().Facts.bActive);
	TestTrue(TEXT("Branch retry dock remains visible"),Reopened->GetInteractionPanel()->GetVisibility()==ESlateVisibility::Visible);
	const auto& RetryOptions=Reopened->GetInteractionPanel()->GetRenderedOptionWidgets();
	TestEqual(TEXT("Two dock choices on reconstruction"),RetryOptions.Num(),2);
	for(const auto& Option:RetryOptions)TestNotNull(TEXT("Restored choice has contextual hover"),Option->GetToolTip());

	FFMCodexLocalMatchResolutionFeedback Rejected;
	Rejected.bVisible = true;
	Rejected.bRejected = true;
	const FFMCodexUMGMatchScreenViewModel Diagnostic =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Rejected, TEXT("rejected"));
	TestFalse(TEXT("Rejection releases generic diagnostic ownership"),
		Diagnostic.LongShotResolution.bSuppressLegacyResolution);
#if WITH_EDITOR && !UE_BUILD_SHIPPING
	using namespace FMCodexPlayerFacingOrdinaryUITests;
	// Reuse the accepted Cross transport harness. This is adapter/authority
	// evidence with delayed ACK/View, not a second real Host/Remote PIE run.
	for (const bool Direct : {true,false})
	{
		FUIFixture F(!Direct,false,false);
		if (!TestTrue(TEXT("Canonical LongShot tactical choice"),F.SkillFixture(false,ESkillRuleType::LongShot))) return false;
		auto* PC=F.Attacker(); auto* S=PC->GetPlayerMatchScreen();
		struct FDeferredBackend : FBackend
		{
			FFMCodexNetworkPlayerIntentAck Ack;
			Submission SubmitScreenIntent(const FFMCodexMatchScreenRequest& R) override
			{
				if (!FFMCodexNetworkMatchScreenActions::Begin(R,PC->GetOwnerView(),F->Client(PC),Last)) return Submission::Rejected;
				++Sends; return Submission::Queued;
			}
			void DeliverView() { Ack=F->Mode->SubmitConnectionPlayerIntent(PC,Last); F->Client(PC).ObserveView(PC->GetOwnerView()); PC->RefreshPlayerFacingUI(); }
			void DeliverAck()
			{
				F->Client(PC).ObserveAck(Ack); PC->RefreshPlayerFacingUI();
				if (Ack.Code!=Code::Accepted) PC->GetPlayerMatchScreen()->NotifyScreenRequestRejected();
			}
		} Backend;
		Backend.F=&F; Backend.PC=PC; S->SetMatchBackend(&Backend);
		const auto Model=S->GetPresentation();
		const auto* Choice=Model.Interaction.SelectionChoices.FindByPredicate([](const auto& X){return X.SkillType==ESkillRuleType::LongShot;});
		if (!TestNotNull(TEXT("Safe LongShot option"),Choice)) return false;
		const auto& Options=S->GetInteractionPanel()->GetRenderedOptionWidgets();
		int32 ExpectedOptions=Model.Interaction.SelectionChoices.Num();
		for (const auto& C:Model.Interaction.SelectionChoices)
			if (C.SkillType==ESkillRuleType::Cross || C.SkillType==ESkillRuleType::CutInsideShot || C.SkillType==ESkillRuleType::LongShot) ++ExpectedOptions;
		TestEqual(TEXT("Only variant skills expand; other choices remain"),Options.Num(),ExpectedOptions);
		TestEqual(TEXT("Existing decline remains available"),S->GetInteractionPanel()->GetPresentation().bCanDecline,Model.Interaction.bCanDecline);
		FUnchanged BeforeHover(F);
		UButton* Clicked=nullptr;
		for (bool InspectDirect:{true,false})
		{
			const FString Label=InspectDirect?TEXT("远射（直接射门）"):TEXT("远射（射向死角）");
			const auto* Option=Options.FindByPredicate([&](const auto& O){return O->GetLabel()==Label;});
			if (!TestNotNull(TEXT("Explicit method button"),Option)) return false;
			auto* Button=CastChecked<UButton>((*Option)->GetWidgetFromName(TEXT("InteractionOptionButton")));
			Button->OnHovered.Broadcast();
			const auto& Detail=S->GetTacticalDetailPanel()->GetPresentation();
			TestEqual(TEXT("Only the hovered canonical branch"),Detail.Branches.Num(),1);
			TestEqual(TEXT("Hover title matches method"),Detail.DisplayName,Label);
			const auto Copy=S->GetTacticalDetailPanel()->CollectPlayerFacingText();
			TestFalse(TEXT("Other method excluded from hover"),Copy.Contains(InspectDirect?TEXT("射向死角"):TEXT("直接射门")));
			TestTrue(TEXT("Existing method-specific rule explanation"),Copy.Contains(InspectDirect?TEXT("进攻掷点 1–2：射门偏出"):TEXT("总和 11–12")));
			if (InspectDirect==Direct) Clicked=Button;
			Button->OnUnhovered.Broadcast();
		}
		BeforeHover.Verify(*this,F); TestEqual(TEXT("Hover sends no commands"),Backend.Sends,0);
		TestEqual(TEXT("Cross high hover still filters one table"),FFMCodexTacticalDetailPresentationBuilder::BuildCrossChoice(EFMCodexUMGBranchIntent::CrossHigh).Branches.Num(),1);
		TestEqual(TEXT("Cross low hover still names its own branch"),FFMCodexTacticalDetailPresentationBuilder::BuildCrossChoice(EFMCodexUMGBranchIntent::CrossLow).DisplayName,FString(TEXT("低球传中")));
		const int32 EntropyBefore=F.Entropy->Calls;
		Clicked->OnClicked.Broadcast();
		TestEqual(TEXT("Only original Skill sent first"),Backend.Last.IntentKind,Kind::SubmitSkill);
		TestFalse(TEXT("Selection clears hover"),S->GetTacticalDetailPanel()->GetPresentation().bValid);
		S->RequestCrossTactic(Choice->OptionId,Direct?EFMCodexUMGBranchIntent::DirectShot:EFMCodexUMGBranchIntent::DeadCorner);
		S->AdvanceCrossFlow(5); TestEqual(TEXT("Pending duplicate click cannot send another Skill"),Backend.Sends,1);
		Backend.DeliverView(); S->AdvanceCrossFlow(5);
		TestEqual(TEXT("View without ACK cannot submit method"),Backend.Sends,1);
		TestFalse(TEXT("No theater flash before method is authorized"),S->GetPresentation().LongShotResolution.bVisible);
		TestTrue(TEXT("Queued selection suppresses redundant dock choices"),S->GetInteractionPanel()->GetPresentation().BranchChoices.IsEmpty());
		Backend.DeliverAck(); S->AdvanceCrossFlow(0);
		TestEqual(TEXT("Original Branch follows confirmed Skill"),Backend.Last.IntentKind,Kind::SubmitBranchIntent);
		TestEqual(TEXT("Exact existing method identity"),Backend.Last.Branch.Intent,Direct?Branch::DirectShot:Branch::DeadCorner);
		auto* Rebuilt=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());Rebuilt->TakeWidget();
		Rebuilt->SetMatchBackend(&Backend);Rebuilt->RefreshFromPresentation(S->GetPresentation());
		Rebuilt->AdvanceCrossFlow(5);Rebuilt->SetMatchBackend(nullptr);
		S->RefreshFromPresentation(S->GetPresentation());S->AdvanceCrossFlow(5);
		TestEqual(TEXT("Reconstruction and repeated view do not duplicate method"),Backend.Sends,2);
		if (!Direct)
		{
			// A real validation rejection must clear the early draft and expose the
			// existing manual retry controls, without changing the gameplay phase.
			++Backend.Last.ExpectedAttackSequence; // Keep positive: zero is InvalidPayload, not a stale sequence.
			Backend.DeliverView();Backend.DeliverAck();
			TestEqual(TEXT("Stale method rejected by common authority boundary"),Backend.Ack.Code,Code::StaleAttackSequence);
			TestEqual(TEXT("Rejected draft is cleared"),Backend.CrossPresentationFlow.Draft,EFMCodexUMGBranchIntent::None);
			TestEqual(TEXT("Existing method choices available for retry"),S->GetInteractionPanel()->GetPresentation().BranchChoices.Num(),2);
			TestTrue(TEXT("Rejected selection restores a visible retry dock"),S->GetInteractionPanel()->IsVisible());
			S->RequestSubmitBranchIntent(EFMCodexUMGBranchIntent::DeadCorner);
		}
		Backend.DeliverView();Backend.DeliverAck();S->AdvanceCrossFlow(0);
		TestEqual(TEXT("Existing method accepted"),Backend.Ack.Code,Code::Accepted);
		TestTrue(TEXT("Selected method enters existing theater"),S->GetPresentation().LongShotResolution.bVisible);
		TestEqual(TEXT("No second in-theater method chooser"),S->GetWidgetFromName(TEXT("TheaterNearMethods"))->GetVisibility(),ESlateVisibility::Collapsed);
		TestEqual(TEXT("Correct manual dice phase"),S->GetPresentation().Interaction.Category,
			Direct?EFMCodexUMGInteractionCategory::RollLongShotDirectAttack:EFMCodexUMGInteractionCategory::RollLongShotDeadCorner);
		const int32 Sends=Backend.Sends;
		S->SkipTacticalScene();S->SkipTacticalScene();S->AdvanceCrossFlow(10);
		TestEqual(TEXT("Waiting never automatically rolls final dice"),Backend.Sends,Sends);
		TestEqual(TEXT("Skill and method consume no gameplay RNG"),F.Entropy->Calls,EntropyBefore);
		TestEqual(TEXT("Leaving selection clears draft"),Backend.CrossPresentationFlow.Draft,EFMCodexUMGBranchIntent::None);
		S->SetMatchBackend(nullptr);
	}
#endif
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
		FString(TEXT("两枚点数总和达到 11 或以上：进球")));
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
	TestEqual(TEXT("Direct hint states only the early out"),
		PendingScreen.LongShotResolution.OutcomeHintLabel,
		FString(TEXT("1–2：射门偏出")));
	FFMCodexLocalMatchInteractionView Defense = Pending;
	Defense.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense;
	AddDirectContest(Defense, true, 4, false, 0);
	TestTrue(TEXT("Direct helper clears when defense becomes pending"),
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Defense, FFMCodexLocalMatchResolutionFeedback(), FString())
			.LongShotResolution.OutcomeHintLabel.IsEmpty());
	UFMCodexLongShotResolutionSurfaceWidget* DirectSurface =
		NewObject<UFMCodexLongShotResolutionSurfaceWidget>(
			GetTransientPackage());
	DirectSurface->TakeWidget();
	DirectSurface->RefreshFromPresentation(
		PendingScreen.LongShotResolution);
	TestTrue(TEXT("Resolved Direct renders one semantic branch heading"),
		DirectSurface->GetWidgetFromName(TEXT("LongShotProductionBranch"))
			->GetVisibility() == ESlateVisibility::Collapsed
			&& Cast<UTextBlock>(DirectSurface->GetWidgetFromName(
				TEXT("LongShotProductionStage")))->GetText().ToString()
				== TEXT("直接射门"));

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
	TestTrue(TEXT("Immediate miss clears the early-out helper"),
		MissScreen.LongShotResolution.OutcomeHintLabel.IsEmpty());
	TestTrue(TEXT("Fresh terminal snapshot owns Next Round"),
		MissScreen.LongShotResolution.Formula.PrimaryAction.Claims(
			MissScreen.Interaction.PrimaryAction));
	auto* Shared=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());Shared->TakeWidget();
	Shared->RefreshFromPresentation(PendingScreen);Shared->SkipTacticalScene();Shared->SkipTacticalScene();
	TestTrue(TEXT("LongShot uses broadcast coordinates"),Shared->GetTacticalScene().IsBroadcast());
	TestEqual(TEXT("Same navy footer height"),CastChecked<USizeBox>(Shared->GetWidgetFromName(TEXT("CrossBroadcastBounds")))->GetHeightOverride(),200.f);
	TestEqual(TEXT("Broadcast identifies LongShot"),CastChecked<UTextBlock>(Shared->GetWidgetFromName(TEXT("CrossBroadcastType")))->GetText().ToString(),FString(TEXT("远射")));
	TestFalse(TEXT("No redundant in-theater selector"),Shared->GetWidgetFromName(TEXT("TheaterNearMethods"))->GetVisibility()!=ESlateVisibility::Collapsed);
	const auto* Hover=Shared->GetWidgetFromName(TEXT("TheaterAttackBaseHover"));
	const auto Tip=FMCodexFormulaTooltipTestUtils::Text(Hover->GetToolTip());
	TestTrue(TEXT("Stage I tooltip retains LongShot context and supplied subtotal"),Tip.Contains(TEXT("远射直接射门")) && Tip.Contains(TEXT("最终基础值 4")));
	Shared->RefreshFromPresentation(MissScreen);
	TestTrue(TEXT("Immediate miss still waits for mandatory attack reveal"),Shared->IsInlineFormulaRevealInputBlocked());
	Shared->PauseInlineFormulaRevealTimerForTesting();
	Shared->AdvanceInlineFormulaRevealForTesting(1.8f);
	Shared->AdvanceInlineFormulaRevealForTesting(3.f);
	for(int I=0;I<3;++I)Shared->SkipTacticalScene();
	TestTrue(TEXT("Immediate miss uses compact result card"),Shared->GetWidgetFromName(TEXT("CutInsideMissBounds"))->GetVisibility()!=ESlateVisibility::Collapsed);
	TestFalse(TEXT("Immediate miss never offers a defense comparison"),Shared->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()!=ESlateVisibility::Collapsed);
	TestFalse(TEXT("Stable result has no reveal-in-progress copy"),CastChecked<UTextBlock>(Shared->GetWidgetFromName(TEXT("TheaterStatus")))->GetText().ToString().Contains(TEXT("正在揭示")));

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
		EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack);
	AddDirectContest(View,false,0,false,0);
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
	Screen->SkipTacticalScene();Screen->SkipTacticalScene();
	TestFalse(TEXT("Production ownership collapses generic resolution root"),
		Screen->IsLegacyResolutionOverlayVisible());
	TestTrue(TEXT("Production surface remains visible"),
		Screen->GetLongShotResolutionSurface() != nullptr
			&& Screen->GetLongShotResolutionSurface()->GetPresentation().bVisible);
	TestEqual(TEXT("Manual roll in theater collapses the duplicate dock"),
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
	for(const bool Goal:{true,false})
	{
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
		0, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA, Goal?5:2, true);
	Terminal.ResolutionFacts.Rolls[1] = DeadRoll(
		1, EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB, Goal?6:3, true);
	FMatchPlayResolutionDecisionFact Decision;
	Decision.DecisionId = TEXT("DeadCorner.Outcome");
	Decision.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
	Decision.RollSequenceIndices = { 0, 1 };
	Decision.bResolved = true;
	Decision.Outcome = Goal?EMatchPlayResolutionDecisionOutcome::Goal:EMatchPlayResolutionDecisionOutcome::Miss;
	Terminal.ResolutionFacts.Decisions = { Decision };
	const FFMCodexUMGMatchScreenViewModel TerminalScreen =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Terminal, FFMCodexLocalMatchResolutionFeedback(), FString());

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(PendingScreen);
	// This headless widget has no NativeTick. Finish only the local spatial
	// setup/intent; neither call may advance the mandatory paired reveal.
	Screen->SkipTacticalScene();
	Screen->SkipTacticalScene();
	Screen->BeginPendingCrossRollRevealForTesting();
	Screen->RefreshFromPresentation(TerminalScreen);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& First =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("First hold discloses pair A"), First.bDeadCornerAVisible);
	TestFalse(TEXT("First hold does not leak pair B"), First.bDeadCornerBVisible);

	Screen->AdvanceInlineFormulaRevealForTesting(2.23f); // Total 4.03: first final die budget not complete.
	TestEqual(TEXT("No decisive flight before 4.04 seconds per die"),Screen->GetTacticalScene().Facts.Outcome,FMCodexTacticalScene::EOutcome::None);
	Screen->AdvanceInlineFormulaRevealForTesting(.77f);
	const auto& SecondRolling =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second reveal retains pair A"),
		SecondRolling.bDeadCornerAVisible);
	TestFalse(TEXT("Second roll stays covered before hold"),
		SecondRolling.bDeadCornerBVisible);
	TestTrue(TEXT("Second roll keeps first die and target, not the total"),
		SecondRolling.PairedRollResultLabel == (Goal?TEXT("第一枚 D6：5"):TEXT("第一枚 D6：2"))
			&& SecondRolling.OutcomeHintLabel
				== TEXT("两枚点数总和达到 11 或以上：进球")
			&& !SecondRolling.PrimaryAction.bVisible);
	Screen->AdvanceInlineFormulaRevealForTesting(1.8f);
	const auto& SecondHeld =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second hold discloses pair B"),
		SecondHeld.bDeadCornerBVisible);
	TestTrue(TEXT("Second hold replaces target with accepted arithmetic"),
		SecondHeld.PairedRollResultLabel == (Goal?TEXT("D6 5 + D6 6 = 11"):TEXT("D6 2 + D6 3 = 5"))
			&& SecondHeld.OutcomeHintLabel.IsEmpty());
	TestTrue(TEXT("Second hold exposes authoritative reel result"),
		SecondHeld.RollReel.bAuthoritativeValue
			&& SecondHeld.RollReel.bResultHold);

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	TestEqual(TEXT("Settled pair hands off to local ball flight"),
		Screen->GetTacticalScene().Phase, FMCodexTacticalScene::EPhase::Outcome);
	TestFalse(TEXT("Terminal narrative waits for local ball flight"),
		Screen->GetLongShotResolutionSurface()->GetPresentation().bNarrativeAvailable);
	Screen->SkipTacticalScene();
	const auto& Settled =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Settled terminal reconstructs both dice"),
		Settled.bDeadCornerAVisible && Settled.bDeadCornerBVisible);
	TestTrue(TEXT("Settled terminal discloses narrative"),
		Settled.bNarrativeAvailable);
	TestEqual(TEXT("No historical pair replay remains active"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settled);
	}
	return true;
}

#if WITH_EDITOR && !UE_BUILD_SHIPPING
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexShotFinalTextTest,
 "FMCodex.LocalPlay.LongShotProduction.06.FinalVisibleText",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexShotFinalTextTest::RunTest(const FString&)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 // Reuse the in-process authority/action harness; no synthetic correct CTA is
 // injected into the view, and no independent Host/Remote run is claimed.
 auto Shown=[](const UWidget* W)
 {
  for(;W;W=W->GetParent())if(!W->IsVisible())return false;
  return true;
 };
 auto& Loc=FTextLocalizationManager::Get();
 const bool WasPreview=Loc.IsGameLocalizationPreviewEnabled();
 const FString PreviousPreview=Loc.GetConfiguredGameLocalizationPreviewLanguage();
 ON_SCOPE_EXIT {if(WasPreview)Loc.EnableGameLocalizationPreview(PreviousPreview);else Loc.DisableGameLocalizationPreview();Loc.WaitForAsyncTasks();};
 for(const bool English:{false,true})
 {
 Loc.EnableGameLocalizationPreview(English?TEXT("en"):TEXT("zh-Hans"));
 Loc.WaitForAsyncTasks();
 for(const auto Family:{ESkillRuleType::LongShot,ESkillRuleType::CutInsideShot,ESkillRuleType::Cross})
 {
  const bool Cut=Family==ESkillRuleType::CutInsideShot, Cross=Family==ESkillRuleType::Cross;
  FUIFixture F(Cut,false,false);
  if(!TestTrue(TEXT("Reach canonical skill selection"),F.SkillFixture(false,Family)))return false;
  auto* Actor=F.Attacker();auto* Defender=F.Defender();
  auto* S=Actor->GetPlayerMatchScreen();auto* D=Defender->GetPlayerMatchScreen();
  const auto* Choice=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate(
   [&](const auto& X){return X.SkillType==Family && X.bEnabled;});
  if(!TestNotNull(TEXT("Authorized shot skill"),Choice))return false;
  S->RequestSubmitSkill(Choice->OptionId);
  if(!TestEqual(TEXT("Skill accepted before branch"),F.Backend(Actor).LastCode,Code::Accepted))return false;
  if(!Cut && !Cross)
  {
   const auto& Options=S->GetInteractionPanel()->GetRenderedOptionWidgets();
   if(!TestEqual(TEXT("Original branch options retained"),Options.Num(),2))return false;
   auto* Hint=CastChecked<UTextBlock>(Options[0]->GetWidgetFromName(TEXT("InteractionOptionSecondaryLabel")));
   TestTrue(TEXT("Direct hint is actually visible"),Shown(Hint));
   TestEqual(TEXT("Rendered principal-attribute hint"),Hint->GetText().ToString(),FString(TEXT("（射门 vs 防守）")));
   TestEqual(TEXT("Other branch hint unchanged"),Options[1]->GetSecondaryLabel(),FString(TEXT("（只看两枚掷点）")));
  }
  S->RequestSubmitBranchIntent(Cross?EFMCodexUMGBranchIntent::CrossHigh:EFMCodexUMGBranchIntent::DirectShot);
  if(!TestEqual(TEXT("Original direct branch accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
  for(auto* Screen:{S,D}){Screen->SkipTacticalScene();Screen->SkipTacticalScene();}
  if(Cross)
  {
   F.Entropy->Word=1; S->RequestContinueResolution(); // Authoritative high-cross route.
   if(!TestEqual(TEXT("Cross route accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
   F.Settle();
   for(auto* Screen:{S,D}){Screen->SkipTacticalScene();Screen->SkipTacticalScene();}
  }
  F.Entropy->Word=2; // Canonical attack D6=3 reaches the defense phase.
  S->RequestContinueResolution();
  if(!TestEqual(TEXT("Manual attack roll accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
  F.Settle();
  for(auto* PC:{Actor,Defender})
  {
   PC->RefreshPlayerFacingUI();
   PC->GetPlayerMatchScreen()->SkipTacticalScene();
   PC->GetPlayerMatchScreen()->SkipTacticalScene();
  }
  const auto Category=Cross?EFMCodexUMGInteractionCategory::RollCrossDefense:Cut?EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense
   :EFMCodexUMGInteractionCategory::RollLongShotDirectDefense;
  auto* Button=CastChecked<UButton>(D->GetWidgetFromName(TEXT("TheaterContinue")));
  auto* Label=CastChecked<UTextBlock>(D->GetWidgetFromName(TEXT("TheaterContinueLabel")));
  TestEqual(TEXT("Distinct typed defense category retained"),D->GetPresentation().Interaction.PrimaryAction.Category,Category);
  TestEqual(TEXT("Authority assigns operation to defender"),Defender->GetOwnerView().ExpectedActingSide,Defender->GetOwnerView().ViewerSide);
  TestTrue(TEXT("Final defense button and label are visible and enabled"),Shown(Button) && Shown(Label) && Button->GetIsEnabled());
  TestEqual(TEXT("Final rendered defense CTA after all refreshes"),Label->GetText().ToString(),FString(English?TEXT("Defender Rolls"):TEXT("防守方掷点")));
  TestFalse(TEXT("Non-owner cannot operate the defense CTA"),Shown(S->GetWidgetFromName(TEXT("TheaterContinue"))) && S->GetWidgetFromName(TEXT("TheaterContinue"))->GetIsEnabled());
  auto CheckIdentity=[&](const FText& Text,const TCHAR* Namespace,const TCHAR* Key)
  {
   const auto N=FTextInspector::GetNamespace(Text); const auto K=FTextInspector::GetKey(Text);
   TestTrue(TEXT("Final control retains stable namespace/key"),N.IsSet() && K.IsSet() && N.GetValue()==Namespace && K.GetValue()==Key);
  };
  CheckIdentity(Label->GetText(),TEXT("FMCodexPlayerUIPresentationText"),TEXT("ShotDefenseRollAction"));
  // Faithful display-state projection: use the production-created formula, change
  // only pending/reel flags. Expected labels are never seeded into the fixture.
  const auto Base=Cross?D->GetInlineFormulaSurface()->GetPresentation():D->GetLongShotResolutionSurface()->GetPresentation().Formula;
  for(int32 State=0;State<(Cross?2:3);++State)
  {
   auto Display=Base;
   Display.bDiceRevealVisible=State!=0;
   Display.PrimaryAction.bVisible=State==0;
   Display.RollReel.bStaticResult=Display.RollReel.bAuthoritativeValue=State==2;
   FMCodexResolutionTheaterPrototype::FTakerInspection Inspection;
   FMCodexResolutionTheaterPrototype::Refresh(*D->WidgetTree,D->GetPresentation(),Display,D->GetPresentation().Header,State==0,Inspection);
   auto* Status=CastChecked<UTextBlock>(D->GetWidgetFromName(TEXT("TheaterStatus")));
   const TCHAR* ZH[]={TEXT("操作已提交，等待确认"),TEXT("正在揭示掷点"),TEXT("掷点已落定")};
   const TCHAR* EN[]={TEXT("Action submitted. Awaiting confirmation"),TEXT("Revealing dice"),TEXT("Dice settled")};
   const TCHAR* Keys[]={TEXT("PendingRequest"),TEXT("CrossRevealing"),TEXT("CutRollLanded")};
   TestTrue(TEXT("Projected status actually visible"),Shown(Status));
   TestEqual(TEXT("Localized final status after production refresh"),Status->GetText().ToString(),FString(English?EN[State]:ZH[State]));
   CheckIdentity(Status->GetText(),TEXT("FMCodexResolutionTheater"),Keys[State]);
   TestFalse(TEXT("Pending/reveal cannot expose usable CTA"),Shown(Button) && Button->GetIsEnabled());
  }
  // Restore through the complete screen refresh before clicking the real button.
  Defender->RefreshPlayerFacingUI(); D->SkipTacticalScene();D->SkipTacticalScene();
  TestEqual(TEXT("Stable state restores localized CTA"),Label->GetText().ToString(),FString(English?TEXT("Defender Rolls"):TEXT("防守方掷点")));
  TestTrue(TEXT("Stable owner action restored"),Shown(Button) && Button->GetIsEnabled());
  const int32 Before=F.Backend(Defender).Sends;
  Button->OnClicked.Broadcast();
  TestEqual(TEXT("Visible button sends one request"),F.Backend(Defender).Sends,Before+1);
  TestEqual(TEXT("Button retains family-specific typed intent"),F.Backend(Defender).Last.IntentKind,
   Cross?Kind::CrossHighDefenseRoll:Cut?Kind::CutInsideShotDirectDefenseRoll:Kind::LongShotDirectDefenseRoll);
  TestEqual(TEXT("Defender request accepted by authority"),F.Backend(Defender).LastCode,Code::Accepted);
 }
 }
 return true;
}
#endif

#endif
