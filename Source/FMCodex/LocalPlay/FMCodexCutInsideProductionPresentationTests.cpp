#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#if WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#endif
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexTacticalScene.h"
#include "FMCodexMatchShellStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
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
				EAttribute::Control, 6.0f, 0.5f),
			RawTerm(0, bAttackResolved, AttackD6)
		};
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 7.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved ? 7.0f + AttackD6 : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms = {
			Attribute(TEXT("Marker.Defense"), EParticipant::Marker,
				EInitialTurnOrderPlayer::PlayerB, MarkerId,
				EAttribute::Defense, 5.0f),
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
	void FinishSpatialBeat(UFMCodexLocalMatchScreenWidget& Screen)
	{
		// These existing tests own reel/operand assertions. Spatial elapsed time
		// is covered separately by BroadcastTheater and natural Local PIE.
		for(int I=0;I<4 && (Screen.GetTacticalScene().IsAnimating() || Screen.GetTacticalScene().Celebration.IsActive());++I)
			Screen.SkipTacticalScene();
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
	TestFalse(TEXT("Unselected CutInside stays on the normal tactical board"),Screen.LongShotResolution.bVisible);
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
			FString(TEXT("（射门 / 控球 vs 防守 / 速度）")));
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
	TestTrue(TEXT("Interrupted choice can recover in the original tactical dock"),
		MatchScreen->GetInteractionPanel()->GetVisibility()==ESlateVisibility::Visible
		&& MatchScreen->GetInteractionPanel()->GetPresentation().BranchChoices.Num()==2);
	for (const auto Skill : {ESkillRuleType::LongShot})
	{
		auto Variant=View; Variant.PresentedActionType=Skill;
		Variant.InteractionCategory=Skill==ESkillRuleType::LongShot?ECategory::SelectLongShotBranch:ECategory::SelectBranchIntent;
		auto Choice=Build(Variant);
		MatchScreen->RefreshFromPresentation(Choice);
		auto Text=[MatchScreen](const TCHAR* Name){return CastChecked<UTextBlock>(MatchScreen->GetWidgetFromName(Name))->GetText().ToString();};
		TestEqual(TEXT("Shared attack context uses canonical selected Carrier"),Text(TEXT("TheaterAttackName0")),FString(TEXT("萨卡")));
		TestEqual(TEXT("Context keeps ordinary Carrier role"),Text(TEXT("TheaterAttackRole0")),FString(TEXT("持球")));
		TestTrue(TEXT("Direct summary keeps attributes and miss range without modifier or GK exposition"),
			Text(TEXT("TheaterNearDirectHint")).Contains(Skill==ESkillRuleType::LongShot?TEXT("射门"):TEXT("控球 × 0.5"))
			&& Text(TEXT("TheaterNearDirectHint")).Contains((Skill==ESkillRuleType::LongShot?TEXT("对抗盯人：防守\n进攻掷点 1–2：射门偏出"):TEXT("对抗盯人：防守 × 0.5 + 速度 × 0.5\n进攻掷点 1–2：射门偏出")))
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
#if WITH_EDITOR && !UE_BUILD_SHIPPING
	using namespace FMCodexPlayerFacingOrdinaryUITests;
	// Reuse the accepted Cross transport harness. This is adapter/authority
	// evidence with delayed ACK/View, not a second real Host/Remote PIE run.
	for (const bool Direct : {true,false})
	{
		FUIFixture F(!Direct,false,false);
		if (!TestTrue(TEXT("Canonical CutInside tactical choice"),F.SkillFixture(false,ESkillRuleType::CutInsideShot))) return false;
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
		const auto* Choice=Model.Interaction.SelectionChoices.FindByPredicate([](const auto& X){return X.SkillType==ESkillRuleType::CutInsideShot;});
		if (!TestNotNull(TEXT("Safe CutInside option"),Choice)) return false;
		const auto& Options=S->GetInteractionPanel()->GetRenderedOptionWidgets();
		int32 ExpectedOptions=Model.Interaction.SelectionChoices.Num();
		for (const auto& C:Model.Interaction.SelectionChoices)
			if (C.SkillType==ESkillRuleType::Cross || C.SkillType==ESkillRuleType::CutInsideShot) ++ExpectedOptions;
		TestEqual(TEXT("Only variant skills expand; other choices remain"),Options.Num(),ExpectedOptions);
		TestEqual(TEXT("Existing decline remains available"),S->GetInteractionPanel()->GetPresentation().bCanDecline,Model.Interaction.bCanDecline);
		FUnchanged BeforeHover(F);
		UButton* Clicked=nullptr;
		for (bool InspectDirect:{true,false})
		{
			const FString Label=InspectDirect?TEXT("内切（直接射门）"):TEXT("内切（射向死角）");
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
			Direct?EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack:EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner);
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
	FinishSpatialBeat(*Screen);
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
	TestEqual(TEXT("First landed die clears in-progress broadcast copy"),Text(TEXT("TheaterStatus")),FString(TEXT("掷点已落定")));

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	const auto& SecondRolling =
		Screen->GetLongShotResolutionSurface()->GetPresentation();
	TestTrue(TEXT("Second CutInside reveal retains pair A"),
		SecondRolling.bDeadCornerAVisible);
	TestFalse(TEXT("Second CutInside reveal does not leak pair B"),
		SecondRolling.bDeadCornerBVisible);
	TestTrue(TEXT("Modern second die moves while first remains landed"),Text(TEXT("TheaterPairARollValue"))==TEXT("6") && B->GetPresentation().bMoving);
	TestEqual(TEXT("Sum stays hidden during second motion"),Text(TEXT("TheaterPairTotal")),FString(TEXT("?")));
	TestEqual(TEXT("Second moving die restores reveal copy"),Text(TEXT("TheaterStatus")),FString(TEXT("正在揭示掷点")));
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
	TestEqual(TEXT("Second landed die also clears in-progress copy"),Text(TEXT("TheaterStatus")),FString(TEXT("掷点已落定")));
	TestTrue(TEXT("CutInside second hold shows arithmetic and clears target"),
		SecondHeld.PairedRollResultLabel == TEXT("D6 6 + D6 5 = 11")
			&& SecondHeld.OutcomeHintLabel.IsEmpty());

	Screen->AdvanceInlineFormulaRevealForTesting(3.0f);
	FinishSpatialBeat(*Screen);
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
	FinishSpatialBeat(*Screen);
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
	TestTrue(TEXT("Base hover explains projected Shooting and Control coefficients"),
		CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("TheaterAttackBaseHover")))->GetToolTip()!=nullptr
		&& Text(TEXT("TheaterAttackCutBaseSummary")).Contains(TEXT("射门")) && Text(TEXT("TheaterAttackCutBaseSummary")).Contains(TEXT("控球"))
		&& Text(TEXT("TheaterAttackCutBaseSummary")).Contains(TEXT("0.5")));
	TestTrue(TEXT("Only actual Marker and GK appear on defense"),Model.LongShotResolution.Formula.DefenseRow.Participants.Num()==2
		&& Text(TEXT("TheaterDefenseCutBaseSummary")).Contains(TEXT("手控球")));
	TestTrue(TEXT("Stepwise ordinary tooltip uses supplied subtotal, not a local sum"),Text(TEXT("TheaterAttackCutBaseSummary")).Contains(TEXT("最终基础值\t17.25")) && Text(TEXT("TheaterAttackCutBaseSummary")).Contains(TEXT("射门贡献：8 × 0.5\t= 4")));
	auto TraitView=Pending;
	auto& TraitRow=TraitView.ResolutionFacts.FormulaContests[0].DefenseRow;
	TraitRow.Terms={Attribute(TEXT("Marker.DefenseHalf"),EParticipant::Marker,EInitialTurnOrderPlayer::PlayerB,MarkerId,EAttribute::Defense,5.f,.5f),
		Attribute(TEXT("Marker.SpeedHalf"),EParticipant::Marker,EInitialTurnOrderPlayer::PlayerB,MarkerId,EAttribute::Speed,4.f,.5f),Fixed(2.f),RawTerm(1,false,0)};
	auto& Op=TraitRow.Terms[0].AttributeOperand;Op.bValid=true;Op.CardId=MarkerId;Op.Role=EParticipant::Marker;Op.Attribute=EAttribute::Defense;
	Op.BaseValue=4;Op.TraitId=TEXT("Trait.CutInsideStopper");Op.Rank=EPlayerTraitRank::B;Op.Bonus=1;Op.EffectiveValue=5;
	TraitRow.KnownNonRollSubtotal=6.5f;
	TraitView.ResolutionFacts.FormulaContests[0].bGoalkeeperParticipated=false;
	// A distinct fixture owns a distinct screen; the live screen deliberately caches
	// displayed facts for the current attack's reveal identity.
	auto* TraitScreen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TraitScreen->TakeWidget();TraitScreen->RefreshFromPresentation(Build(TraitView));
	FinishSpatialBeat(*TraitScreen);
	auto* TraitHover=TraitScreen->GetWidgetFromName(TEXT("TheaterDefenseBaseHover"));
	if(!TestNotNull(TEXT("Valid trait fixture enters the formula theater"),TraitHover))return false;
	const auto Detail=FindObjectChecked<UTextBlock>(TraitHover->GetOuter(),TEXT("TheaterDefenseCutBaseSummary"))->GetText().ToString();
	TestTrue(TEXT("Trait adjustment is before weighting"),Detail.Contains(TEXT("原始防守：4")) && Detail.Contains(TEXT("内切封锁 B：防守 +1 → 5")) && Detail.Contains(TEXT("防守贡献：(4 +1) × 0.5\t= 2.5")));
	TestTrue(TEXT("Defense fixed bonus and final supplied subtotal are explicit"),Detail.Contains(TEXT("防守加成\t+2")) && Detail.Contains(TEXT("最终基础值\t6.5")));
	TestTrue(TEXT("Unknown sources are never invented"),!Detail.Contains(TEXT("其他权威贡献")));
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
	TestEqual(TEXT("CutInside broadcast describes the landed hold"),Text(TEXT("TheaterStatus")),FString(TEXT("掷点已落定")));
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
	FinishSpatialBeat(*Screen);
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
	Screen->RefreshFromPresentation(Build(Pending)); FinishSpatialBeat(*Screen); Screen->BeginPendingCrossRollRevealForTesting();
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
			TestEqual(TEXT("Visible broadcast status agrees with landed die"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterStatus")))->GetText().ToString(),FString(TEXT("掷点已落定")));
		}
		else TestFalse(TEXT("Before authoritative landing no landed status leaks"),Status.Contains(TEXT("已落定")));
	}
	TestTrue(TEXT("Boundary sampling reached the landed state"),bSawLandedStatus);
	Screen->ForceLayoutPrepass();
	TestEqual(TEXT("ImmediateMiss preserves compact spatial silhouette allocation"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterAttackSilhouetteBounds")))->GetHeightOverride(),108.f);
	TestTrue(TEXT("ImmediateMiss retains the attack card allocation"),Screen->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetDesiredSize().Equals(OriginalCardSize,.1f));
	for (const TCHAR* Name : {TEXT("TheaterDefensePanelBounds"),TEXT("TheaterAttackBaseHover"),TEXT("TheaterAttackResultColumn")})
		TestFalse(TEXT("Skipped Formula never paints candidate defense/Base/Final"),Shown(Name));
	TestEqual(TEXT("No VS comparison"),Screen->GetWidgetFromName(TEXT("TheaterVS"))->GetParent()->GetVisibility(),ESlateVisibility::Collapsed);
	TestFalse(TEXT("No duplicate standalone die after landing"),Shown(TEXT("TheaterRoll")));
	TestTrue(TEXT("Same modern roll lands on the actual miss die"),AttackReel->GetPresentation().bAuthoritativeValue && AttackReel->GetPresentation().CenterValue==2);
	TestTrue(TEXT("Actual hold retains the landed accent"),AttackReel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor().Equals(FMCodexRollPresentationStyle::AuthoritativeLandedValue()));
	TestFalse(TEXT("ResultHold still owns the final CTA"),Shown(TEXT("TheaterPrimaryBounds")));
	Screen->AdvanceInlineFormulaRevealForTesting(3.f);
	FinishSpatialBeat(*Screen);
	const auto Model=Build(Terminal);
	TestTrue(TEXT("Existing immediate-miss narrative remains exact"),Model.LongShotResolution.Formula.NarrativeHeadline==TEXT("萨卡内切后射门偏出。") && Shown(TEXT("TheaterOutcome")));
	const auto& Reason=Model.LongShotResolution.Formula.ResolutionReasonLabel;
	TestTrue(TEXT("Skipped-path explanation does not claim Winner/stamina/GK"),Reason.Contains(TEXT("不进行攻防比较")) && !Reason.Contains(TEXT("体力")) && !Reason.Contains(TEXT("门将")) && !Reason.Contains(TEXT("获胜")));
	TestEqual(TEXT("No winner badge on the failed attack"),Screen->GetWidgetFromName(TEXT("TheaterAttackBadge"))->GetVisibility(),ESlateVisibility::Hidden);
	TestTrue(TEXT("Original terminal CTA available after the one roll"),Shown(TEXT("TheaterPrimaryBounds")));
	TestTrue(TEXT("Skipped contest uses compact result instead of empty duel"),Shown(TEXT("CutInsideMissBounds")) && !Shown(TEXT("TheaterDuel")));
	TestEqual(TEXT("Compact summary copies disclosed die"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("CutInsideMissDie")))->GetText().ToString(),FString(TEXT("2")));
	Screen->RefreshFromPresentation(Model);
	TestFalse(TEXT("Repeated View does not manufacture a defense event"),Screen->IsInlineFormulaRevealInputBlocked());
	TestFalse(TEXT("Resolution does not open Full Card"),Screen->IsDetailOverlayVisible());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCutInsideBroadcastTest,
	"FMCodex.LocalPlay.CutInsideProduction.BroadcastTheater",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCutInsideBroadcastTest::RunTest(const FString&)
{
	using namespace FMCodexCutInsideProductionPresentationTests;
	using namespace FMCodexTacticalScene;
	auto View=BaseView(ECategory::RollCutInsideShotDirectAttack);
	AddDirectFacts(View,false,0,false,0);
	// Deliberately differs from the fixture terms: the UI must consume this
	// authority-supplied subtotal instead of calculating it from attributes.
	View.ResolutionFacts.FormulaContests[0].AttackRow.KnownNonRollSubtotal=17.25f;
	auto Model=Build(View);
	const auto Facts=Project(Model.LongShotResolution,Model.Header,true);
	TestTrue(TEXT("CutInside uses distinct broadcast mode with existing Direct method"),Facts.bActive && Facts.bCutInside && Facts.Method==EMethod::Direct);
	TestEqual(TEXT("Only safe Carrier, Marker and participating GK"),Facts.Participants.Num(),3);
	for(const auto& P:Facts.Participants)
		TestTrue(TEXT("No fabricated Runner or Helper"),P.Role==EParticipant::Carrier || P.Role==EParticipant::Marker || P.Role==EParticipant::Goalkeeper);
	TestEqual(TEXT("Safe Carrier identity"),Facts.Participants[0].CardId,CarrierId);
	TestEqual(TEXT("Undisclosed result stays unknown"),Facts.Outcome,EOutcome::None);
	TestEqual(TEXT("Attacker retains typed manual command"),Model.LongShotResolution.Formula.PrimaryAction.Action.Category,EFMCodexUMGInteractionCategory::RollCutInsideShotDirectAttack);
	auto NoKeeper=View;
	NoKeeper.ResolutionFacts.Participants.RemoveAll([](const auto& P){return P.Role==EParticipant::Goalkeeper;});
	NoKeeper.ResolutionFacts.FormulaContests[0].DefenseRow.Terms.RemoveAll([](const auto& T){return T.ParticipantRole==EParticipant::Goalkeeper;});
	NoKeeper.ResolutionFacts.FormulaContests[0].bGoalkeeperParticipated=false;
	NoKeeper.PlayerBCardRoster.Last().bGoalkeeper=true;
	const auto NoKeeperModel=Build(NoKeeper);
	const auto ContextFacts=Project(NoKeeperModel.LongShotResolution,NoKeeperModel.Header,true);
	const auto* ContextKeeper=ContextFacts.Participants.FindByPredicate([](const auto& P){return P.Role==EParticipant::Goalkeeper;});
	TestTrue(TEXT("Public goalkeeper identity is visible without formula participation"),ContextKeeper && ContextKeeper->CardId==GoalkeeperId && !ContextKeeper->bFormulaActive);
	NoKeeper.PlayerBCardRoster.Last().bGoalkeeper=false;
	const auto MissingKeeper=Build(NoKeeper);
	TestEqual(TEXT("Missing safe identity cannot fabricate a goalkeeper"),Project(MissingKeeper.LongShotResolution,MissingKeeper.Header,true).Participants.Num(),2);
	FState State; State.Sync(Facts);
	TestTrue(TEXT("Both broadcast consumers share the goal-relative keeper anchor"),CutInsideParticipantAnchor(State,EParticipant::Goalkeeper).Equals(CrossParticipantAnchor(State,EParticipant::Goalkeeper)));
	const auto WideStart=CutInsideParticipantAnchor(State,EParticipant::Carrier);
	TestTrue(TEXT("Carrier begins on the upper wing beside the penalty area"),WideStart.X>700.f && WideStart.Y<210.f);
	TestTrue(TEXT("Marker covers the inside of the upper wing"),CutInsideParticipantAnchor(State,EParticipant::Marker).Y>WideStart.Y);
	TestTrue(TEXT("Direct aim is goal center, not an outcome"),CutInsideAimTarget(EMethod::Direct).Equals(CrossGoalAnchor));
	TestTrue(TEXT("DeadCorner intended aim is inside the far post"),IsInsideGoal(CutInsideAimTarget(EMethod::DeadCorner),EMethod::CrossHigh) && CutInsideAimTarget(EMethod::DeadCorner).X>1250.f);
	State.Tick(1.1f);
	State.CrossEntrance.Elapsed=CrossIntroSeconds; // Unit fixture, real painted readiness covered by PIE.
	TestTrue(TEXT("Actual manual phase owns restrained idle motion"),State.IsCutInsideIdle());
	for(int I=0;I<60;++I)State.Tick(.02f);
	const auto Carrier=CutInsideParticipantAnchor(State,EParticipant::Carrier);
	TestTrue(TEXT("Carrier cuts down and inward from the upper wing"),Carrier.X>WideStart.X && Carrier.Y>WideStart.Y && Carrier.Y<315.f);
	TestTrue(TEXT("Ball remains at Carrier foot before shot"),(CutInsideBall(State)-Carrier).Equals(FVector2D(48,24),.01));
	State.Tick(.6f);
	TestTrue(TEXT("Manual waiting visibly moves without shooting"),(CutInsideParticipantAnchor(State,EParticipant::Carrier)-Carrier).Size()>5);
	auto Rolling=Model.LongShotResolution;Rolling.Formula.bDiceRevealVisible=true;
	State.Sync(Project(Rolling,Model.Header,true));
	const auto HeldBall=CutInsideBall(State);const float Clock=State.CutInsideIdleTime;
	State.Tick(.5f);
	TestTrue(TEXT("Roll v2 continues dribble motion"),!CutInsideBall(State).Equals(HeldBall));
	TestTrue(TEXT("Roll v2 advances idle clock"),State.CutInsideIdleTime>Clock);
	TestEqual(TEXT("No future shot while roll owns disclosure"),State.Facts.Outcome,EOutcome::None);
	State.Sync(Facts);State.Tick(.3f);
	TestTrue(TEXT("Next manual wait resumes the same loop"),State.IsCutInsideIdle() && State.CutInsideIdleTime>Clock);
	State.Sync(Facts);TestTrue(TEXT("Duplicate views do not reset motion"),State.CutInsideMotion>0.f);
	auto Defense=BaseView(ECategory::RollCutInsideShotDirectDefense);AddDirectFacts(Defense,true,4,false,0);
	TestEqual(TEXT("Defender retains typed manual command"),Build(Defense).LongShotResolution.Formula.PrimaryAction.Action.Category,EFMCodexUMGInteractionCategory::RollCutInsideShotDirectDefense);
	auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());Screen->TakeWidget();
	Screen->RefreshFromPresentation(Model);Screen->SkipTacticalScene();Screen->SkipTacticalScene();
	auto Text=[Screen](const TCHAR* N){return CastChecked<UTextBlock>(Screen->GetWidgetFromName(N))->GetText().ToString();};
	auto Shown=[Screen](const TCHAR* N){return Screen->GetWidgetFromName(N)->IsVisible();};
	TestEqual(TEXT("Formula displays authority subtotal"),Text(TEXT("TheaterAttackFinalNumber")),FString(TEXT("17.25")));
	TestEqual(TEXT("Attacker action uses accepted broadcast copy"),Text(TEXT("TheaterContinueLabel")),FString(TEXT("实施战术")));
	TestTrue(TEXT("Existing detail hover remains accessible"),Screen->GetWidgetFromName(TEXT("TheaterAttackBaseHover"))->GetToolTip()!=nullptr);
	TestEqual(TEXT("Same accepted lower band height"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("CrossBroadcastBounds")))->GetHeightOverride(),200.f);
	TestEqual(TEXT("Same accepted primary CTA width"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterPrimaryBounds")))->GetWidthOverride(),260.f);
	TestEqual(TEXT("Broadcast footer is visible"),Screen->GetWidgetFromName(TEXT("CrossBroadcastBounds"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
	TestEqual(TEXT("No route-die UI"),Screen->GetWidgetFromName(TEXT("CrossRouteContextBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
	TestEqual(TEXT("No legacy central panel"),Screen->GetWidgetFromName(TEXT("TheaterTopBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
	TestTrue(TEXT("Real Screen opts into spatial CutInside"),Screen->GetTacticalScene().Facts.bActive && Screen->GetTacticalScene().Facts.bCutInside);
	const auto A=CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("TheaterAttackPanel")))->Background;
	const auto D=CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("TheaterDefensePanel")))->Background;
	TestTrue(TEXT("Neutral identical fills with fine team-owned borders"),A.TintColor==D.TintColor && A.OutlineSettings.Width==1.f && D.OutlineSettings.Width==1.f);
	TestTrue(TEXT("Attack border follows owner"),A.OutlineSettings.Color.GetSpecifiedColor().Equals(FMCodexMatchShellStyle::HeaderAccent(Model.Header.LeftAttackTurnTracker.PrimarySideColor)));
	auto Reversed=Model;
	Reversed.LongShotResolution.SpatialAttackingSide=EInitialTurnOrderPlayer::PlayerB;
	Reversed.LongShotResolution.Formula.AttackRow.Side=EInitialTurnOrderPlayer::PlayerB;
	Reversed.LongShotResolution.Formula.DefenseRow.Side=EInitialTurnOrderPlayer::PlayerA;
	Screen->RefreshFromPresentation(Reversed);
	TestTrue(TEXT("Player B attack border uses B color"),CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("TheaterAttackPanel")))->Background.OutlineSettings.Color.GetSpecifiedColor().Equals(FMCodexMatchShellStyle::HeaderAccent(Model.Header.RightAttackTurnTracker.PrimarySideColor)));
	FState ReverseState;ReverseState.Sync(Project(Reversed.LongShotResolution,Reversed.Header,true));
	TestEqual(TEXT("Camera still follows attacking goal for Player B"),ReverseState.Facts.Participants[0].Side,EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Both teams share coherent role staging"),CutInsideParticipantAnchor(ReverseState,EParticipant::Marker).X>CutInsideParticipantAnchor(ReverseState,EParticipant::Carrier).X);
	FitCrossViewport(*Screen->WidgetTree,true,FVector2D(2048,1080));
	const float DesignHeight=CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterDesignBounds")))->GetHeightOverride();
	TestTrue(TEXT("Broadcast fills non-16:9 viewport without side gutters"),FMath::IsNearlyEqual(1600.f/DesignHeight,2048.f/1080.f));
	TestEqual(TEXT("Pitch and footer fill same Cross geometry"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterSpatialLane")))->GetHeightOverride()+200.f,DesignHeight);
	for(const auto Outcome:{EOutcome::Goal,EOutcome::ImmediateMiss,EOutcome::DefensiveSuccess})
	{
		auto Terminal=BaseView(ECategory::AdvanceAfterTerminal);Terminal.bTerminalPendingAdvance=true;Terminal.ContinueActionLabel=TEXT("下一回合");
		AddDirectFacts(Terminal,true,Outcome==EOutcome::ImmediateMiss?2:4,Outcome!=EOutcome::ImmediateMiss,3,Outcome==EOutcome::Goal?EFormulaWinner::Attacker:EFormulaWinner::Defender);
		if(Outcome==EOutcome::ImmediateMiss)Terminal.ResolutionFacts.FormulaContests[0].Application=EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate;
		AddDecision(Terminal,TEXT("CutInsideShot.DirectShot.Outcome"),EMatchPlayResolutionRollSemantics::ArithmeticContest,
			Outcome==EOutcome::Goal?EMatchPlayResolutionDecisionOutcome::Goal:Outcome==EOutcome::ImmediateMiss?EMatchPlayResolutionDecisionOutcome::ImmediateMiss:EMatchPlayResolutionDecisionOutcome::Miss);
		auto TerminalModel=Build(Terminal);
		TestFalse(TEXT("CutInside broadcast omits redundant comparison explanation"),TerminalModel.LongShotResolution.Formula.ResolutionReasonLabel.Contains(TEXT("本次公式按照总值大小比较")));
		// Use the same reflected safe model sent through existing Network view DTOs.
		TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
		FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Save,&TerminalModel,nullptr);
		FFMCodexUMGMatchScreenViewModel Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,true);
		FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Load,&Copy,nullptr);
		TestFalse(TEXT("CutInside facts survive existing safe DTO serialization"),Save.IsError() || Load.IsError());
		const auto Final=Project(Copy.LongShotResolution,Copy.Header,true);
		TestEqual(TEXT("Actual authoritative CutInside decision drives spatial outcome"),Final.Outcome,Outcome);
		auto StillRolling=Copy.LongShotResolution;StillRolling.Formula.bDiceRevealVisible=true;
		TestEqual(TEXT("Safe disclosed fact still respects visible reel gate"),Project(StillRolling,Copy.Header,true).Outcome,EOutcome::None);
		FState End=State;End.Sync(Final);
		auto Gated=Model.LongShotResolution;Gated.Formula.bNarrativeAvailable=true;End.Gate(Gated);
		TestFalse(TEXT("Outcome flight gates final narrative"),Gated.Formula.bNarrativeAvailable);
		End.Tick(.6f);
		TestFalse(TEXT("CutInside movement is still underway after the old quick burst"),End.IsOutcomeVisualHold());
		TestTrue(TEXT("Ball remains attached during the slower cut"),(CutInsideBall(End)-CutInsideParticipantAnchor(End,EParticipant::Carrier)).Equals(FVector2D(48,24),.01f));
		End.Tick(CutInsideOutcomeActionSeconds-.6f);
		TestTrue(TEXT("Final ball remains readable before handoff"),End.IsOutcomeVisualHold());
		const auto Ball=CutInsideBall(End);
		TestEqual(TEXT("Only typed Goal reaches goalmouth"),IsInsideGoal(Ball,EMethod::CrossHigh),Outcome==EOutcome::Goal);
		End.Tick(CrossOutcomeHoldSeconds+.01f);
		TestEqual(TEXT("Spatial sequence hands off to existing result"),End.Phase,EPhase::ResultHold);
		TestEqual(TEXT("Only Goal starts existing celebration"),End.Celebration.IsActive(),Outcome==EOutcome::Goal);
		if(Outcome==EOutcome::Goal)End.Tick(FMCodexGoalCelebration::Duration);
		End.Sync(Final);TestFalse(TEXT("Duplicate result does not replay celebration"),End.Celebration.IsActive());
	}
	State.Sync(FFacts{});TestFalse(TEXT("Exit stops idle"),State.IsCutInsideIdle());
	TestEqual(TEXT("Exit clears movement"),State.CutInsideMotion,0.f);
	auto Dead=BaseView(ECategory::RollCutInsideShotDeadCorner);AddBranch(Dead,EMatchPlayCutInsideShotActualBranch::DeadCorner);
	Dead.ResolutionFacts.Rolls={Roll(0,EPostPurpose::PairedAttackA,EMatchPlayResolutionRollSemantics::OutcomeDecision,EInitialTurnOrderPlayer::PlayerA,false,0),
		Roll(1,EPostPurpose::PairedAttackB,EMatchPlayResolutionRollSemantics::OutcomeDecision,EInitialTurnOrderPlayer::PlayerA,false,0)};
	Dead.ResolutionFacts.Participants.RemoveAll([](const auto& P){return P.Role==EParticipant::Goalkeeper;});
	Dead.PlayerBCardRoster.Last().bGoalkeeper=true;
	auto DeadModel=Build(Dead);Screen->RefreshFromPresentation(DeadModel);
	const auto DeadFacts=Project(DeadModel.LongShotResolution,DeadModel.Header,true);
	TestTrue(TEXT("DeadCorner also shows public contextual goalkeeper"),DeadFacts.Participants.ContainsByPredicate([](const auto& P){return P.Role==EParticipant::Goalkeeper && P.CardId==GoalkeeperId && !P.bFormulaActive;}));
	FState DeadLoop;DeadLoop.Sync(DeadFacts);DeadLoop.Tick(1.1f);DeadLoop.CrossEntrance.Elapsed=CrossIntroSeconds;
	for(int I=0;I<30;++I)DeadLoop.Tick(.02f);
	auto DeadRolling=DeadModel.LongShotResolution;DeadRolling.bDiceRevealVisible=true;DeadRolling.bNarrativeAvailable=true;
	DeadLoop.Sync(Project(DeadRolling,DeadModel.Header,true));
	const auto DeadBall=CutInsideBall(DeadLoop);DeadLoop.Tick(.5f);
	TestTrue(TEXT("Paired roll continues dribble without revealing final outcome"),DeadLoop.IsCutInsideIdle() && !CutInsideBall(DeadLoop).Equals(DeadBall) && DeadLoop.Facts.Outcome==EOutcome::None);
	TestEqual(TEXT("DeadCorner retains its existing manual paired intent"),DeadModel.LongShotResolution.PrimaryAction.Action.Category,EFMCodexUMGInteractionCategory::RollCutInsideShotDeadCorner);
	TestEqual(TEXT("DeadCorner has no fabricated defense formula"),Screen->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
	TestEqual(TEXT("DeadCorner reuses accepted pair widget"),Screen->GetWidgetFromName(TEXT("TheaterPair"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
	TestEqual(TEXT("DeadCorner compact card width"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TheaterAttackPanelBounds")))->GetWidthOverride(),520.f);
	TestEqual(TEXT("DeadCorner identity no longer creates a wide gap"),CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("CrossBroadcastIdentity")))->GetWidthOverride(),160.f);
	TestEqual(TEXT("Entire DeadCorner group is centered by its container"),CastChecked<UBorder>(Screen->GetWidgetFromName(TEXT("CrossBroadcastBand")))->GetHorizontalAlignment(),HAlign_Center);
	TestFalse(TEXT("Compact miss summary does not leak into DeadCorner"),Shown(TEXT("CutInsideMissBounds")));
	TestTrue(TEXT("Dice rule shares the title row"),Screen->GetWidgetFromName(TEXT("CutInsidePairRule"))->GetParent()==Screen->GetWidgetFromName(TEXT("TheaterAttackSide"))->GetParent());
	TestEqual(TEXT("Static rule names two D6"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("CutInsidePairRule")))->GetText().ToString(),FString(TEXT("掷 2 枚六面骰，点数相加")));
	TestEqual(TEXT("Goal threshold uses catalog range"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("CutInsidePairGoalRule")))->GetText().ToString(),FString(TEXT("总和 11–12：进球")));
	TestEqual(TEXT("No premature pair total"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterPairTotal")))->GetText().ToString(),FString(TEXT("?")));
	Dead.InteractionCategory=ECategory::AdvanceAfterTerminal;Dead.bTerminalPendingAdvance=true;
	Dead.ResolutionFacts.Rolls[0].bResolved=Dead.ResolutionFacts.Rolls[1].bResolved=true;
	Dead.ResolutionFacts.Rolls[0].RawD6=Dead.ResolutionFacts.Rolls[1].RawD6=5;
	AddDecision(Dead,TEXT("DeadCorner.Outcome"),EMatchPlayResolutionRollSemantics::OutcomeDecision,EMatchPlayResolutionDecisionOutcome::Miss);
	DeadModel=Build(Dead);FState DeadState;DeadState.Sync(Project(DeadModel.LongShotResolution,DeadModel.Header,true));
	TestEqual(TEXT("DeadCorner safe miss reaches result state"),DeadState.Phase,EPhase::ResultHold);
	TestTrue(TEXT("DeadCorner miss stays outside goal without a fabricated save"),CutInsideBall(DeadState).Equals(FVector2D(1300,410)));
	Dead.ResolutionFacts.Decisions[0].Outcome=EMatchPlayResolutionDecisionOutcome::Goal;
	Dead.ResolutionFacts.Rolls[0].RawD6=6;DeadModel=Build(Dead);
	DeadState.Sync(Project(DeadModel.LongShotResolution,DeadModel.Header,true));
	TestTrue(TEXT("Disclosed DeadCorner Goal reaches upper visible goalmouth"),IsInsideGoal(CutInsideBall(DeadState),EMethod::CrossHigh));
	// Same widget tree returns to the unchanged LongShot consumer.
	auto Choice=BaseView(ECategory::SelectBranchIntent);Choice.SelectedCarrierCardId=CarrierId;Choice.SelectedMarkerCardId=MarkerId;
	Choice.BranchIntentOptions={EMatchPlayElectiveBranchIntent::DirectShot,EMatchPlayElectiveBranchIntent::DeadCorner};
	auto ChoiceModel=Build(Choice);Screen->RefreshFromPresentation(ChoiceModel);
	TestFalse(TEXT("CutInside has no unselected theater"),ChoiceModel.LongShotResolution.bVisible || Screen->GetTacticalScene().Facts.bActive);
	ChoiceModel.LongShotResolution.SkillType=ESkillRuleType::LongShot;ChoiceModel.LongShotResolution.bVisible=true;Screen->RefreshFromPresentation(ChoiceModel);
	TestFalse(TEXT("LongShot never inherits CutInside broadcast"),Screen->GetTacticalScene().IsBroadcast() || Shown(TEXT("CrossBroadcastBounds")));
	TestEqual(TEXT("LongShot restores its methods home"),Screen->GetWidgetFromName(TEXT("TheaterNearMethods"))->GetParent()->GetFName(),FName(TEXT("TheaterMethodsHome")));
	return true;
}

#endif
