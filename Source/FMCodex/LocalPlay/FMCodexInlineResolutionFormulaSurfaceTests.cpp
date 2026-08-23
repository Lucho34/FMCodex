#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"

#include "Components/WrapBox.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FMCodexInlineResolutionFormulaSurfaceTests
{
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using EParticipantRole = EMatchPlayResolutionParticipantRole;
	using ERollPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using ETermKind = EMatchPlayResolutionFormulaTermKind;

	void AddRosterCard(
		FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const FString& Name)
	{
		FFMCodexLocalMatchCardView Card;
		Card.Side = Side;
		Card.CardId = CardId;
		Card.DisplayLabel = Name;
		(Side == EInitialTurnOrderPlayer::PlayerA
			? View.PlayerACardRoster : View.PlayerBCardRoster).Add(Card);
	}

	FMatchPlayResolutionFormulaTermFact AttributeTerm(
		const FName TermId,
		const EParticipantRole Role,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const EAttribute Attribute,
		const float SourceValue,
		const float Multiplier)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TermId;
		Result.Kind = Role == EParticipantRole::Goalkeeper
			? ETermKind::GoalkeeperContribution : ETermKind::Attribute;
		Result.ParticipantRole = Role;
		Result.Side = Side;
		Result.CardId = CardId;
		Result.Attribute = Attribute;
		Result.SourceValue = SourceValue;
		Result.Multiplier = Multiplier;
		Result.bResolved = true;
		Result.Contribution = SourceValue * Multiplier;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact RollTerm(
		const FName TermId,
		const int32 SequenceIndex,
		const bool bResolved,
		const int32 RawD6)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TermId;
		Result.Kind = ETermKind::RawRoll;
		Result.RollSequenceIndex = SequenceIndex;
		Result.bResolved = bResolved;
		Result.SourceValue = bResolved ? RawD6 : 0;
		Result.Contribution = Result.SourceValue;
		return Result;
	}

	FMatchPlayResolutionFormulaTermFact FixedTerm(const float Value)
	{
		FMatchPlayResolutionFormulaTermFact Result;
		Result.TermId = TEXT("Defense.FixedBonus");
		Result.Kind = ETermKind::FixedModifier;
		Result.SourceValue = Value;
		Result.Contribution = Value;
		Result.bResolved = true;
		return Result;
	}

	FMatchPlayResolutionRollFact RollFact(
		const int32 SequenceIndex,
		const ERollPurpose Purpose,
		const bool bResolved,
		const int32 RawD6,
		const EInitialTurnOrderPlayer Side)
	{
		FMatchPlayResolutionRollFact Result;
		Result.SequenceIndex = SequenceIndex;
		Result.OperandId = Purpose == ERollPurpose::PrimaryAttack
			? TEXT("PrimaryAttackD6") : TEXT("PrimaryDefenseD6");
		Result.PostRoutePurpose = Purpose;
		Result.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Result.OwningSide = Side;
		Result.bResolved = bResolved;
		Result.RawD6 = bResolved ? RawD6 : 0;
		return Result;
	}

	FMatchPlayCurrentAttackResolutionFactProjection MakeCrossHighFacts(
		const bool bHelper,
		const bool bGoalkeeper,
		const bool bAttackRollResolved,
		const bool bDefenseRollResolved)
	{
		const FName CarrierId(TEXT("Fixture.Carrier"));
		const FName RunnerId(TEXT("Fixture.Runner"));
		const FName MarkerId(TEXT("Fixture.Marker"));
		const FName HelperId(TEXT("Fixture.Helper"));
		const FName GoalkeeperId(TEXT("Fixture.Goalkeeper"));
		FMatchPlayCurrentAttackResolutionFactProjection Facts;
		Facts.bSuccess = true;
		Facts.bHasFacts = true;
		Facts.AttackSequence = 1;
		Facts.ActionType = ESkillRuleType::Cross;
		Facts.bHasActualBranch = true;
		Facts.ActualBranch.ActionType = ESkillRuleType::Cross;
		Facts.ActualBranch.Cross = EMatchPlayCrossActualBranch::High;
		Facts.Participants.Add({ EParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId });
		Facts.Participants.Add({ EParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA, RunnerId });
		Facts.Participants.Add({ EParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId });
		if (bHelper)
		{
			Facts.Participants.Add({ EParticipantRole::Helper,
				EInitialTurnOrderPlayer::PlayerB, HelperId });
		}
		if (bGoalkeeper)
		{
			Facts.Participants.Add({ EParticipantRole::Goalkeeper,
				EInitialTurnOrderPlayer::PlayerB, GoalkeeperId });
		}

		Facts.Rolls.Add(RollFact(1, ERollPurpose::PrimaryAttack,
			bAttackRollResolved, 4, EInitialTurnOrderPlayer::PlayerA));
		Facts.Rolls.Add(RollFact(2, ERollPurpose::PrimaryDefense,
			bDefenseRollResolved, 3, EInitialTurnOrderPlayer::PlayerB));
		Facts.bHasPendingRoll = !bAttackRollResolved || !bDefenseRollResolved;
		Facts.NextPendingRollSequenceIndex = !bAttackRollResolved ? 1
			: !bDefenseRollResolved ? 2 : INDEX_NONE;

		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = TEXT("Cross.High");
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = bAttackRollResolved && bDefenseRollResolved
			? EMatchPlayResolutionFormulaApplication::Applied
			: EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.RowId = TEXT("Cross.High.Attack");
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms.Add(AttributeTerm(
			TEXT("Carrier.PrimaryHalf"), EParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId,
			EAttribute::Passing, 5.0f, 0.5f));
		Contest.AttackRow.Terms.Add(AttributeTerm(
			TEXT("Runner.PrimaryHalf"), EParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA, RunnerId,
			EAttribute::Strength, 4.0f, 0.5f));
		Contest.AttackRow.Terms.Add(RollTerm(
			TEXT("PrimaryAttackD6"), 1, bAttackRollResolved, 4));
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 4.5f;
		if (bAttackRollResolved)
		{
			Contest.AttackRow.bFinalValueResolved = true;
			Contest.AttackRow.FinalValue = 8.5f;
		}

		Contest.DefenseRow.RowId = TEXT("Cross.High.Defense");
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms.Add(AttributeTerm(
			TEXT("Marker.PrimaryHalf"), EParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId,
			EAttribute::Tackling, 5.0f, 0.5f));
		if (bHelper)
		{
			Contest.DefenseRow.Terms.Add(AttributeTerm(
				TEXT("Helper.PrimaryHalf"), EParticipantRole::Helper,
				EInitialTurnOrderPlayer::PlayerB, HelperId,
				EAttribute::Strength, 4.0f, 0.5f));
		}
		Contest.DefenseRow.Terms.Add(RollTerm(
			TEXT("PrimaryDefenseD6"), 2, bDefenseRollResolved, 3));
		Contest.DefenseRow.Terms.Add(FixedTerm(2.0f));
		if (bGoalkeeper)
		{
			Contest.DefenseRow.Terms.Add(AttributeTerm(
				TEXT("Goalkeeper.ActiveHalf"), EParticipantRole::Goalkeeper,
				EInitialTurnOrderPlayer::PlayerB, GoalkeeperId,
				EAttribute::GoalkeeperAerial, 5.0f, 0.5f));
			Contest.bGoalkeeperParticipated = true;
		}
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = bHelper && bGoalkeeper
			? 9.0f : bHelper ? 6.5f : bGoalkeeper ? 7.0f : 4.5f;
		if (bAttackRollResolved && bDefenseRollResolved)
		{
			Contest.DefenseRow.bFinalValueResolved = true;
			Contest.DefenseRow.FinalValue = bHelper && bGoalkeeper
				? 12.0f : bHelper ? 9.5f : bGoalkeeper ? 10.0f : 7.5f;
			Contest.bHasResolvedFormula = true;
		}
		Facts.FormulaContests.Add(Contest);
		return Facts;
	}

	FFMCodexLocalMatchInteractionView MakeInteraction()
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ElectiveBranchIntent = EMatchPlayElectiveBranchIntent::CrossHigh;
		View.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::ContinueResolution;
		View.ContinueActionLabel = TEXT("Continue - Resolve Cross Plan");
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.Carrier"), TEXT("萨卡"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.Runner"), TEXT("哈弗茨"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.Marker"), TEXT("萨利巴"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.Helper"), TEXT("赖斯"));
		AddRosterCard(View, EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.Goalkeeper"), TEXT("拉亚"));
		return View;
	}

	FFMCodexUMGMatchScreenViewModel BuildPresentation(
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts)
	{
		FFMCodexLocalMatchInteractionView View = MakeInteraction();
		View.ResolutionFacts = Facts;
		const FMatchPlayResolutionRollFact* AttackRoll = Facts.Rolls.FindByPredicate(
			[](const FMatchPlayResolutionRollFact& Roll)
			{
				return Roll.PostRoutePurpose == ERollPurpose::PrimaryAttack;
			});
		const FMatchPlayResolutionRollFact* DefenseRoll = Facts.Rolls.FindByPredicate(
			[](const FMatchPlayResolutionRollFact& Roll)
			{
				return Roll.PostRoutePurpose == ERollPurpose::PrimaryDefense;
			});
		if (AttackRoll != nullptr && !AttackRoll->bResolved)
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::RollCrossHighAttack;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.ContinueActionLabel = TEXT("进攻方掷点");
		}
		else if (DefenseRoll != nullptr && !DefenseRoll->bResolved)
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::RollCrossHighDefense;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
			View.ContinueActionLabel = TEXT("防守方掷点");
		}
		else
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ContinueResolution;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
			View.ContinueActionLabel = TEXT("继续结算");
		}
		FFMCodexLocalMatchResolutionFeedback Feedback;
		Feedback.bVisible = true;
		Feedback.StepTitle = TEXT("Route Resolved");
		Feedback.ResolutionFacts = Facts;
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Feedback, FString(), EInitialTurnOrderPlayer::PlayerA);
	}

	const FFMCodexUMGInlineFormulaTermViewModel* FindTerm(
		const FFMCodexUMGInlineFormulaRowViewModel& Row,
		const EFMCodexUMGInlineFormulaTermKind Kind,
		const int32 RollSequenceIndex = INDEX_NONE)
	{
		return Row.Terms.FindByPredicate(
			[Kind, RollSequenceIndex](
				const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind == Kind
					&& (RollSequenceIndex == INDEX_NONE
						|| Term.RollSequenceIndex == RollSequenceIndex);
			});
	}

	bool HasParticipant(
		const FFMCodexUMGInlineFormulaRowViewModel& Row,
		const FString& Role,
		const FString& Name)
	{
		return Row.Participants.ContainsByPredicate(
			[&Role, &Name](
				const FFMCodexUMGInlineFormulaParticipantViewModel& Participant)
			{
				return Participant.RoleLabel == Role
					&& Participant.PlayerName == Name;
			});
	}

	FString FlattenVisibleText(
		const FFMCodexUMGInlineFormulaSurfaceViewModel& Surface)
	{
		FString Result = Surface.ContestLabel + Surface.StatusLabel
			+ Surface.AttackRow.SideLabel + Surface.DefenseRow.SideLabel
			+ Surface.AttackRow.KnownNonRollSubtotalLabel
			+ Surface.DefenseRow.KnownNonRollSubtotalLabel
			+ Surface.AttackRow.FinalValueLabel
			+ Surface.DefenseRow.FinalValueLabel
			+ Surface.ContinueActionLabel;
		for (const FFMCodexUMGInlineFormulaRowViewModel* Row
			: { &Surface.AttackRow, &Surface.DefenseRow })
		{
			for (const auto& Participant : Row->Participants)
			{
				Result += Participant.RoleLabel + Participant.PlayerName;
			}
			for (const auto& Term : Row->Terms)
			{
				Result += Term.DisplayLabel + Term.AttributeLabel;
			}
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInlineResolutionFormulaSurfaceTest,
	"FMCodex.LocalPlay.InlineFormula.CrossHighGoldenPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInlineResolutionFormulaSurfaceTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceTests;

	const FFMCodexUMGMatchScreenViewModel Pending = BuildPresentation(
		MakeCrossHighFacts(true, true, false, false));
	const auto& PendingSurface = Pending.InlineFormula;
	TestTrue(TEXT("Cross High arithmetic surface activates and suppresses legacy"),
		PendingSurface.bVisible
			&& PendingSurface.bSuppressLegacyResolution
			&& Pending.Resolution.bVisible);
	TestTrue(TEXT("Context and row labels are Chinese-first"),
		PendingSurface.ContestLabel == TEXT("高球传中")
			&& PendingSurface.AttackRow.SideLabel == TEXT("进攻")
			&& PendingSurface.DefenseRow.SideLabel == TEXT("防守")
			&& PendingSurface.StatusLabel == TEXT("等待进攻方掷点")
			&& PendingSurface.ContinueActionLabel == TEXT("进攻方掷点")
			&& PendingSurface.bAttackRowActive
			&& !PendingSurface.bDefenseRowActive);
	TestTrue(TEXT("Projected preferred participant identities are role-labeled"),
		HasParticipant(PendingSurface.AttackRow, TEXT("持球"), TEXT("萨卡"))
			&& HasParticipant(PendingSurface.AttackRow, TEXT("跑位"), TEXT("哈弗茨"))
			&& HasParticipant(PendingSurface.DefenseRow, TEXT("盯人"), TEXT("萨利巴"))
			&& HasParticipant(PendingSurface.DefenseRow, TEXT("协防"), TEXT("赖斯"))
			&& HasParticipant(PendingSurface.DefenseRow, TEXT("门将"), TEXT("拉亚")));
	TestTrue(TEXT("Known structured operands expose source and multiplier truth"),
		PendingSurface.AttackRow.Terms.Num() == 3
			&& PendingSurface.AttackRow.Terms[0].DisplayLabel
				== TEXT("传球 5 ×0.5")
			&& PendingSurface.AttackRow.Terms[1].DisplayLabel
				== TEXT("力量 4 ×0.5")
			&& PendingSurface.DefenseRow.Terms.Num() == 5
			&& PendingSurface.DefenseRow.Terms.Last().DisplayLabel
				== TEXT("制空 5 ×0.5"));
	const auto* PendingAttackD6 = FindTerm(
		PendingSurface.AttackRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* PendingDefenseD6 = FindTerm(
		PendingSurface.DefenseRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Known subtotals and pending attack roll are projected"),
		PendingAttackD6 != nullptr && PendingDefenseD6 != nullptr
			&& PendingSurface.AttackRow.KnownNonRollSubtotalLabel
				== TEXT("基础值 4.5")
			&& PendingSurface.DefenseRow.KnownNonRollSubtotalLabel
				== TEXT("基础值 9")
			&& PendingAttackD6->DisplayLabel == TEXT("掷点 ?")
			&& PendingAttackD6->bNextPendingRoll
			&& PendingDefenseD6->DisplayLabel == TEXT("掷点 ?")
			&& !PendingDefenseD6->bNextPendingRoll
			&& PendingSurface.AttackRow.FinalValueLabel == TEXT("?")
			&& PendingSurface.DefenseRow.FinalValueLabel == TEXT("?"));

	const auto OneRoll = BuildPresentation(
		MakeCrossHighFacts(true, true, true, false)).InlineFormula;
	const auto* ResolvedAttackD6 = FindTerm(
		OneRoll.AttackRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* StillPendingDefenseD6 = FindTerm(
		OneRoll.DefenseRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Attack settles without auto-resolving defense"),
		ResolvedAttackD6 != nullptr && StillPendingDefenseD6 != nullptr
			&& ResolvedAttackD6->bResolved
			&& ResolvedAttackD6->RawD6 == 4
			&& ResolvedAttackD6->DisplayLabel == TEXT("掷点 4")
			&& !ResolvedAttackD6->bNextPendingRoll
			&& StillPendingDefenseD6->bNextPendingRoll
			&& OneRoll.AttackRow.bFinalValueResolved
			&& OneRoll.AttackRow.FinalValueLabel == TEXT("8.5")
			&& !OneRoll.DefenseRow.bFinalValueResolved
			&& OneRoll.StatusLabel == TEXT("等待防守方掷点")
			&& OneRoll.ContinueActionLabel == TEXT("防守方掷点")
			&& !OneRoll.bAttackRowActive
			&& OneRoll.bDefenseRowActive);

	const auto Resolved = BuildPresentation(
		MakeCrossHighFacts(true, true, true, true)).InlineFormula;
	const auto* FinalAttackD6 = FindTerm(
		Resolved.AttackRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* FinalDefenseD6 = FindTerm(
		Resolved.DefenseRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Both authoritative raw rolls and projected finals render exactly"),
		FinalAttackD6 != nullptr && FinalDefenseD6 != nullptr
			&& FinalAttackD6->RawD6 == 4
			&& FinalDefenseD6->RawD6 == 3
			&& Resolved.AttackRow.bFinalValueResolved
			&& Resolved.AttackRow.FinalValue == 8.5f
			&& Resolved.AttackRow.FinalValueLabel == TEXT("8.5")
			&& Resolved.DefenseRow.bFinalValueResolved
			&& Resolved.DefenseRow.FinalValue == 12.0f
			&& Resolved.DefenseRow.FinalValueLabel == TEXT("12")
			&& Resolved.StatusLabel == TEXT("双方掷点已完成")
			&& Resolved.ContinueActionLabel == TEXT("继续结算"));

	const auto OptionalAbsent = BuildPresentation(
		MakeCrossHighFacts(false, false, false, false)).InlineFormula;
	TestTrue(TEXT("Absent Helper and inactive GK omit participants and terms"),
		OptionalAbsent.DefenseRow.Participants.Num() == 1
			&& OptionalAbsent.DefenseRow.Participants[0].RoleLabel == TEXT("盯人")
			&& OptionalAbsent.DefenseRow.Terms.Num() == 3
			&& !FlattenVisibleText(OptionalAbsent).Contains(TEXT("协防"))
			&& !FlattenVisibleText(OptionalAbsent).Contains(TEXT("门将")));

	FMatchPlayCurrentAttackResolutionFactProjection Unsupported =
		MakeCrossHighFacts(true, true, false, false);
	Unsupported.ActualBranch.Cross = EMatchPlayCrossActualBranch::Low;
	Unsupported.FormulaContests[0].ContestId = TEXT("Cross.Low");
	const FFMCodexUMGMatchScreenViewModel UnsupportedPresentation =
		BuildPresentation(Unsupported);
	TestTrue(TEXT("Unsupported route keeps legacy Resolution behavior"),
		!UnsupportedPresentation.InlineFormula.bVisible
			&& !UnsupportedPresentation.InlineFormula.bSuppressLegacyResolution
			&& UnsupportedPresentation.Resolution.bVisible);

	FMatchPlayCurrentAttackResolutionFactProjection InitialRoute =
		MakeCrossHighFacts(true, true, false, false);
	InitialRoute.bHasActualBranch = false;
	InitialRoute.FormulaContests.Reset();
	const FFMCodexUMGMatchScreenViewModel InitialRoutePresentation =
		BuildPresentation(InitialRoute);
	TestTrue(TEXT("Cross BranchSelection does not become arithmetic formula"),
		!InitialRoutePresentation.InlineFormula.bVisible
			&& InitialRoutePresentation.Resolution.bVisible);

	FFMCodexLocalMatchInteractionView RejectedView = MakeInteraction();
	RejectedView.ResolutionFacts = MakeCrossHighFacts(
		true, true, false, false);
	FFMCodexLocalMatchResolutionFeedback RejectedFeedback;
	RejectedFeedback.bVisible = true;
	RejectedFeedback.bRejected = true;
	RejectedFeedback.ResolutionFacts = RejectedView.ResolutionFacts;
	const auto RejectedPresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			RejectedView, RejectedFeedback, FString());
	TestTrue(TEXT("Exceptional rejection remains on legacy error surface"),
		!RejectedPresentation.InlineFormula.bVisible
			&& !RejectedPresentation.InlineFormula.bSuppressLegacyResolution
			&& RejectedPresentation.Resolution.bVisible);

	const FString VisibleText = FlattenVisibleText(PendingSurface);
	TestTrue(TEXT("Covered player-facing surface leaks no internal identifiers"),
		!VisibleText.Contains(TEXT("Fixture."))
			&& !VisibleText.Contains(TEXT("CardId"))
			&& !VisibleText.Contains(TEXT("PlayerKey"))
			&& !VisibleText.Contains(TEXT("Cross.High"))
			&& !VisibleText.Contains(TEXT("ArithmeticContest"))
			&& !VisibleText.Contains(TEXT("GoalkeeperContribution"))
			&& !VisibleText.Contains(TEXT("PrimaryAttack")));

	UFMCodexInlineResolutionFormulaSurfaceWidget* Widget =
		NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>(
			GetTransientPackage());
	TestNotNull(TEXT("Generic inline formula Widget can be constructed"), Widget);
	if (Widget != nullptr)
	{
		Widget->TakeWidget();
		Widget->RefreshFromPresentation(PendingSurface);
		Widget->RefreshFromPresentation(PendingSurface);
		UWrapBox* AttackTerms = Cast<UWrapBox>(Widget->GetWidgetFromName(
			TEXT("InlineFormulaAttackTerms")));
		UWrapBox* DefenseTerms = Cast<UWrapBox>(Widget->GetWidgetFromName(
			TEXT("InlineFormulaDefenseTerms")));
		TestTrue(TEXT("Widget renders structured rows and one highlighted pending term"),
			Widget->GetRenderedAttackTermCount() == 3
				&& Widget->GetRenderedDefenseTermCount() == 5
				&& Widget->GetRenderedPendingTermCount() == 1
				&& Widget->GetWidgetFromName(
					TEXT("InlineFormulaSurfaceFrame")) != nullptr
				&& Widget->GetWidgetFromName(
					TEXT("InlineFormulaContinueButton")) != nullptr);
		TestTrue(TEXT("Repeated presentation refresh reuses formula items without duplication"),
			AttackTerms != nullptr && DefenseTerms != nullptr
				&& AttackTerms->GetChildrenCount() == 3
				&& DefenseTerms->GetChildrenCount() == 5);
	}

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Production Match Screen can be constructed"), Screen);
	if (Screen != nullptr)
	{
		Screen->TakeWidget();
		Screen->RefreshFromPresentation(Pending);
		TestTrue(TEXT("Pitch-owned surface replaces only legacy overlay"),
			Screen->GetWidgetFromName(TEXT("PitchPresentationLayers")) != nullptr
				&& Screen->GetInlineFormulaSurface() != nullptr
				&& Screen->GetInlineFormulaSurface()->GetPresentation().bVisible
				&& !Screen->IsLegacyResolutionOverlayVisible());
		Screen->RefreshFromPresentation(UnsupportedPresentation);
		TestTrue(TEXT("Legacy overlay returns for unsupported Resolution state"),
			Screen->GetInlineFormulaSurface() != nullptr
				&& !Screen->GetInlineFormulaSurface()->GetPresentation().bVisible
				&& Screen->IsLegacyResolutionOverlayVisible());
	}

	FString WidgetSource;
	const FString WidgetPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexInlineResolutionFormulaSurfaceWidget.cpp"));
	TestTrue(TEXT("Widget source is readable for presentation-boundary guard"),
		FFileHelper::LoadFileToString(WidgetSource, *WidgetPath));
	TestTrue(TEXT("Widget contains no formula recomputation or RNG/provider call"),
		!WidgetSource.Contains(TEXT("FormulaResolver"))
			&& !WidgetSource.Contains(TEXT("FMath::Rand"))
			&& !WidgetSource.Contains(TEXT("D6Provider"))
			&& !WidgetSource.Contains(TEXT("SourceValue *"))
			&& !WidgetSource.Contains(TEXT("Contribution +"))
			&& !WidgetSource.Contains(TEXT("FinalValue =")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInlineResolutionFormulaDiceRevealTest,
	"FMCodex.LocalPlay.InlineFormula.CrossHighManualRollStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInlineResolutionFormulaDiceRevealTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceTests;
	FFMCodexLocalMatchInteractionView PreRouteView = MakeInteraction();
	PreRouteView.ContinueActionLabel = TEXT("判定高球传中路线");
	FFMCodexLocalMatchResolutionFeedback LegacyPreRouteFeedback;
	LegacyPreRouteFeedback.bVisible = true;
	LegacyPreRouteFeedback.StepTitle = TEXT("Resolution Started");
	LegacyPreRouteFeedback.ContinuationSummary =
		TEXT("Continue - Resolve Route");
	const FFMCodexUMGMatchScreenViewModel PreRoutePresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			PreRouteView,
			LegacyPreRouteFeedback,
			FString(),
			EInitialTurnOrderPlayer::PlayerA);
	UFMCodexLocalMatchScreenWidget* PreRouteScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	PreRouteScreen->TakeWidget();
	PreRouteScreen->RefreshFromPresentation(PreRoutePresentation);
	TestTrue(TEXT("Covered pre-route step suppresses the legacy English overlay"),
		PreRoutePresentation.InlineFormula.bSuppressLegacyResolution
			&& !PreRouteScreen->IsLegacyResolutionOverlayVisible()
			&& PreRoutePresentation.Interaction.PrimaryActionLabel
				== TEXT("判定高球传中路线"));

	const FFMCodexUMGMatchScreenViewModel ManualPending = BuildPresentation(
		MakeCrossHighFacts(true, true, false, false));
	const FFMCodexUMGMatchScreenViewModel ManualAttackSettled = BuildPresentation(
		MakeCrossHighFacts(true, true, true, false));
	const FFMCodexUMGMatchScreenViewModel ManualCompleted = BuildPresentation(
		MakeCrossHighFacts(true, true, true, true));
	UFMCodexLocalMatchScreenWidget* ManualScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Manual roll fixture screen can be constructed"), ManualScreen);
	if (ManualScreen == nullptr)
	{
		return false;
	}
	ManualScreen->TakeWidget();
	ManualScreen->RefreshFromPresentation(ManualPending);
	TestTrue(TEXT("Pre-roll is direct authority UI with no local reveal gate"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !ManualScreen->IsInlineFormulaRevealInputBlocked()
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.StatusLabel == TEXT("等待进攻方掷点"));
	ManualScreen->RefreshFromPresentation(ManualAttackSettled);
	const auto& AttackOnly =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	const auto* AttackOnlyRoll = FindTerm(
		AttackOnly.AttackRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* PendingDefenseRoll = FindTerm(
		AttackOnly.DefenseRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Attack-only state remains readable until defender acts"),
		AttackOnlyRoll != nullptr && AttackOnlyRoll->RawD6 == 4
			&& AttackOnly.AttackRow.bFinalValueResolved
			&& PendingDefenseRoll != nullptr && !PendingDefenseRoll->bResolved
			&& !AttackOnly.DefenseRow.bFinalValueResolved
			&& AttackOnly.ContinueActionLabel == TEXT("防守方掷点")
			&& !ManualScreen->IsLegacyResolutionOverlayVisible());
	ManualScreen->RefreshFromPresentation(ManualCompleted);
	const auto& ManualFinal =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Completed comparison comes directly from authority facts"),
		ManualFinal.AttackRow.bFinalValueResolved
			&& ManualFinal.DefenseRow.bFinalValueResolved
			&& ManualFinal.StatusLabel == TEXT("双方掷点已完成")
			&& ManualFinal.ContinueActionLabel == TEXT("继续结算")
			&& !ManualScreen->IsInlineFormulaRevealInputBlocked());
	return true;
}

#endif
