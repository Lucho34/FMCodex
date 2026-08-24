#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#include "FMCodexCardRackWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexRollReelWidget.h"

#include "Components/TextBlock.h"
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
		const bool bDefenseRollResolved,
		const EFormulaWinner Winner = EFormulaWinner::None,
		const EMatchPlayCrossActualBranch Branch =
			EMatchPlayCrossActualBranch::High,
		const int64 AttackSequence = 1)
	{
		const FName CarrierId(TEXT("Fixture.Carrier"));
		const FName RunnerId(TEXT("Fixture.Runner"));
		const FName MarkerId(TEXT("Fixture.Marker"));
		const FName HelperId(TEXT("Fixture.Helper"));
		const FName GoalkeeperId(TEXT("Fixture.Goalkeeper"));
		FMatchPlayCurrentAttackResolutionFactProjection Facts;
		Facts.bSuccess = true;
		Facts.bHasFacts = true;
		Facts.AttackSequence = AttackSequence;
		Facts.ActionType = ESkillRuleType::Cross;
		Facts.bHasActualBranch = true;
		Facts.ActualBranch.ActionType = ESkillRuleType::Cross;
		Facts.ActualBranch.Cross = Branch;
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
		Facts.TacticalPlayers.Add({ EInitialTurnOrderPlayer::PlayerA,
			CarrierId, EMatchPlayRelativeDeploymentZone::Forward });
		Facts.TacticalPlayers.Add({ EInitialTurnOrderPlayer::PlayerB,
			MarkerId, EMatchPlayRelativeDeploymentZone::Backfield });

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
		RouteRoll.RawD6 = Branch == EMatchPlayCrossActualBranch::High ? 5 : 2;
		Facts.Rolls.Add(RouteRoll);
		Facts.Rolls.Add(RollFact(1, ERollPurpose::PrimaryAttack,
			bAttackRollResolved, 4, EInitialTurnOrderPlayer::PlayerA));
		Facts.Rolls.Add(RollFact(2, ERollPurpose::PrimaryDefense,
			bDefenseRollResolved, 3, EInitialTurnOrderPlayer::PlayerB));
		Facts.bHasPendingRoll = !bAttackRollResolved || !bDefenseRollResolved;
		Facts.NextPendingRollSequenceIndex = !bAttackRollResolved ? 1
			: !bDefenseRollResolved ? 2 : INDEX_NONE;

		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = Branch == EMatchPlayCrossActualBranch::High
			? FName(TEXT("Cross.High")) : FName(TEXT("Cross.Low"));
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = bAttackRollResolved && bDefenseRollResolved
			? EMatchPlayResolutionFormulaApplication::Applied
			: EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.RowId = Branch == EMatchPlayCrossActualBranch::High
			? FName(TEXT("Cross.High.Attack"))
			: FName(TEXT("Cross.Low.Attack"));
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

		Contest.DefenseRow.RowId = Branch == EMatchPlayCrossActualBranch::High
			? FName(TEXT("Cross.High.Defense"))
			: FName(TEXT("Cross.Low.Defense"));
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
			if (Winner != EFormulaWinner::None)
			{
				Contest.ResolvedResult.FormulaType = EFormulaType::Finishing;
				Contest.ResolvedResult.Winner = Winner;
				Contest.ResolvedResult.bIsGoal =
					Winner == EFormulaWinner::Attacker;
				Contest.ResolvedResult.bAttackEnded = true;
				Contest.ResolvedResult.bContinueResolution = false;
			}
		}
		Facts.FormulaContests.Add(Contest);
		return Facts;
	}

	FFMCodexLocalMatchInteractionView MakeInteraction()
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.AttackSequence = 1;
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
		const FMatchPlayCurrentAttackResolutionFactProjection& Facts,
		const EInitialTurnOrderPlayer LocalViewerSide =
			EInitialTurnOrderPlayer::PlayerA,
		const bool bIncludeRosterNames = true)
	{
		FFMCodexLocalMatchInteractionView View = MakeInteraction();
		View.ElectiveBranchIntent = Facts.bHasActualBranch
			&& Facts.ActualBranch.Cross == EMatchPlayCrossActualBranch::Low
				? EMatchPlayElectiveBranchIntent::CrossLow
				: EMatchPlayElectiveBranchIntent::CrossHigh;
		if (!bIncludeRosterNames)
		{
			View.PlayerACardRoster.Reset();
			View.PlayerBCardRoster.Reset();
		}
		View.ResolutionFacts = Facts;
		View.AttackSequence = Facts.AttackSequence;
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
				EFMCodexLocalMatchInteractionCategory::RollCrossAttack;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.ContinueActionLabel = TEXT("进攻方掷点");
		}
		else if (DefenseRoll != nullptr && !DefenseRoll->bResolved)
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::RollCrossDefense;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
			View.ContinueActionLabel = TEXT("防守方掷点");
		}
		else
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::CompleteCrossAndAdvance;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.bCrossFormulaComplete = true;
			View.bCrossTerminalActionAvailable = true;
			View.ContinueActionLabel = TEXT("下一回合");
		}
		FFMCodexLocalMatchResolutionFeedback Feedback;
		Feedback.bVisible = true;
		Feedback.StepTitle = TEXT("Route Resolved");
		Feedback.ResolutionFacts = Facts;
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, Feedback, FString(), LocalViewerSide);
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
			+ Surface.RouteResultLabel + Surface.TacticalPlayerSummaryLabel
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
	TestTrue(TEXT("Route result stays narrow and omits tactical-player names"),
		PendingSurface.RouteResultLabel
			== TEXT("路线掷点 5 → 判定为高球传中")
			&& PendingSurface.TacticalPlayerSummaryLabel.IsEmpty());
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
			&& PendingSurface.DefenseRow.FinalValueLabel == TEXT("?")
			&& PendingSurface.AttackRow.bDisplayedResultResolved
			&& !PendingSurface.AttackRow.bDisplayedResultIsFinalValue
			&& PendingSurface.AttackRow.DisplayedResult == 4.5f
			&& PendingSurface.AttackRow.DisplayedResultLabel == TEXT("4.5")
			&& PendingSurface.DefenseRow.bDisplayedResultResolved
			&& !PendingSurface.DefenseRow.bDisplayedResultIsFinalValue
			&& PendingSurface.DefenseRow.DisplayedResult == 9.0f
			&& PendingSurface.DefenseRow.DisplayedResultLabel == TEXT("9"));

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
			&& OneRoll.AttackRow.bDisplayedResultIsFinalValue
			&& OneRoll.AttackRow.DisplayedResultLabel == TEXT("8.5")
			&& !OneRoll.DefenseRow.bFinalValueResolved
			&& !OneRoll.DefenseRow.bDisplayedResultIsFinalValue
			&& OneRoll.DefenseRow.DisplayedResultLabel == TEXT("9")
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
			&& Resolved.AttackRow.bDisplayedResultIsFinalValue
			&& Resolved.AttackRow.DisplayedResultLabel == TEXT("8.5")
			&& Resolved.DefenseRow.bFinalValueResolved
			&& Resolved.DefenseRow.FinalValue == 12.0f
			&& Resolved.DefenseRow.FinalValueLabel == TEXT("12")
			&& Resolved.DefenseRow.bDisplayedResultIsFinalValue
			&& Resolved.DefenseRow.DisplayedResultLabel == TEXT("12")
			&& Resolved.StatusLabel == TEXT("双方掷点已完成")
			&& Resolved.ContinueActionLabel == TEXT("下一回合"));

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
	TestTrue(TEXT("Cross Low gets the same pending dual-roll surface"),
		UnsupportedPresentation.InlineFormula.bVisible
			&& UnsupportedPresentation.InlineFormula.bSuppressLegacyResolution
			&& UnsupportedPresentation.InlineFormula.bShowFormulaRows
			&& UnsupportedPresentation.InlineFormula.StatusLabel
				== TEXT("等待进攻方掷点")
			&& UnsupportedPresentation.InlineFormula.AttackRow
				.bKnownNonRollSubtotalResolved
			&& UnsupportedPresentation.InlineFormula.DefenseRow
				.bKnownNonRollSubtotalResolved
			&& !UnsupportedPresentation.InlineFormula.AttackRow
				.bFinalValueResolved
			&& !UnsupportedPresentation.InlineFormula.DefenseRow
				.bFinalValueResolved
			&& UnsupportedPresentation.InlineFormula.RouteResultLabel
				== TEXT("路线掷点 5 → 判定为低球传中")
			&& UnsupportedPresentation.Resolution.bVisible);
	FMatchPlayCurrentAttackResolutionFactProjection SettledLow = Unsupported;
	for (FMatchPlayResolutionRollFact& Roll : SettledLow.Rolls)
	{
		if (!Roll.bInitialRoute)
		{
			Roll.bResolved = true;
		}
	}
	SettledLow.FormulaContests[0].AttackRow.bFinalValueResolved = true;
	SettledLow.FormulaContests[0].DefenseRow.bFinalValueResolved = true;
	const auto SettledLowSurface = BuildPresentation(SettledLow).InlineFormula;
	TestTrue(TEXT("Settled Cross Low remains on the same formula surface"),
		SettledLowSurface.bVisible
			&& SettledLowSurface.bShowFormulaRows
			&& SettledLowSurface.ContestId == TEXT("Cross.Low")
			&& SettledLowSurface.ContestLabel == TEXT("低球传中"));

	FMatchPlayCurrentAttackResolutionFactProjection NoTactical =
		MakeCrossHighFacts(true, true, false, false);
	NoTactical.TacticalPlayers.Reset();
	const auto NoTacticalSurface = BuildPresentation(NoTactical).InlineFormula;
	TestTrue(TEXT("Missing tactical players do not fabricate or hide the surface"),
		NoTacticalSurface.bVisible
			&& NoTacticalSurface.TacticalPlayerSummaryLabel.IsEmpty());

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
		const UTextBlock* AttackMainResult = Cast<UTextBlock>(
			Widget->GetWidgetFromName(TEXT("InlineFormulaAttackFinalValue")));
		const UTextBlock* DefenseMainResult = Cast<UTextBlock>(
			Widget->GetWidgetFromName(TEXT("InlineFormulaDefenseFinalValue")));
		TestTrue(TEXT("Pre-roll main result boxes show authoritative known subtotals"),
			AttackMainResult != nullptr && DefenseMainResult != nullptr
				&& AttackMainResult->GetText().ToString() == TEXT("4.5")
				&& DefenseMainResult->GetText().ToString() == TEXT("9"));
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
		TestTrue(TEXT("Cross Low pending formula also replaces the legacy overlay"),
			Screen->GetInlineFormulaSurface() != nullptr
				&& Screen->GetInlineFormulaSurface()->GetPresentation().bVisible
				&& Screen->GetInlineFormulaSurface()->GetPresentation()
					.bShowFormulaRows
				&& !Screen->IsLegacyResolutionOverlayVisible());
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
			&& !WidgetSource.Contains(TEXT("FinalValue ="))
			&& !WidgetSource.Contains(TEXT("KnownNonRollSubtotal +"))
			&& !WidgetSource.Contains(TEXT("DisplayedResult =")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCrossResultNarrativePresentationTest,
	"FMCodex.LocalPlay.InlineFormula.CrossResultNarrativeAndStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCrossResultNarrativePresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceTests;

	const FFMCodexUMGMatchScreenViewModel Pending = BuildPresentation(
		MakeCrossHighFacts(true, true, true, false));
	TestTrue(TEXT("Narrative remains hidden before the complete authoritative result"),
		!Pending.InlineFormula.bNarrativeAvailable
			&& Pending.InlineFormula.StatusLabel == TEXT("等待防守方掷点"));

	// The fixture's displayed 8.5 attack is below its displayed 12 defense.
	// An attacker winner therefore proves that presentation follows the
	// authoritative resolved result instead of re-comparing UI values.
	const FFMCodexUMGMatchScreenViewModel AttackWin = BuildPresentation(
		MakeCrossHighFacts(true, true, true, true,
			EFormulaWinner::Attacker));
	TestTrue(TEXT("Authoritative attacker winner owns the completed narrative"),
		AttackWin.InlineFormula.bNarrativeAvailable
			&& AttackWin.InlineFormula.bNarrativeAttackSuccess
			&& AttackWin.InlineFormula.NarrativeHeadline
				== TEXT("萨卡传中，哈弗茨破门！")
			&& AttackWin.InlineFormula.ResultSubtitle
				== TEXT("高球传中 · 进攻成功")
			&& AttackWin.InlineFormula.ContestLabel
				== AttackWin.InlineFormula.NarrativeHeadline
			&& AttackWin.InlineFormula.StatusLabel
				== AttackWin.InlineFormula.ResultSubtitle);
	TestTrue(TEXT("Completed Cross CTA has one presentation owner"),
		AttackWin.InlineFormula.bCanContinue
			&& AttackWin.InlineFormula.ContinueActionLabel == TEXT("下一回合")
			&& AttackWin.Interaction.bPrimaryActionOwnedByInlineFormula
			&& !AttackWin.Interaction.bCanContinue
			&& AttackWin.Interaction.PrimaryActionLabel.IsEmpty());

	const FFMCodexUMGMatchScreenViewModel MarkerDefense = BuildPresentation(
		MakeCrossHighFacts(false, false, true, true,
			EFormulaWinner::Defender));
	TestTrue(TEXT("Marker-only defense uses carrier interception wording"),
		MarkerDefense.InlineFormula.DefensiveNarrativePerformer
			== EFMCodexUMGCrossDefensiveNarrativePerformer::Marker
			&& MarkerDefense.InlineFormula.NarrativeHeadline
				== TEXT("萨卡传中被萨利巴破坏")
			&& MarkerDefense.InlineFormula.ResultSubtitle
				== TEXT("高球传中 · 防守成功"));

	FFMCodexUMGMatchScreenViewModel HelperDefense;
	int64 HelperSequence = 0;
	for (int64 Sequence = 1; Sequence <= 32; ++Sequence)
	{
		const FFMCodexUMGMatchScreenViewModel Candidate = BuildPresentation(
			MakeCrossHighFacts(true, false, true, true,
				EFormulaWinner::Defender,
				EMatchPlayCrossActualBranch::High,
				Sequence));
		if (Candidate.InlineFormula.DefensiveNarrativePerformer
			== EFMCodexUMGCrossDefensiveNarrativePerformer::Helper)
		{
			HelperDefense = Candidate;
			HelperSequence = Sequence;
			break;
		}
	}
	TestTrue(TEXT("Immutable contest identity can deterministically select helper"),
		HelperSequence > 0
			&& HelperDefense.InlineFormula.NarrativeHeadline
				== TEXT("哈弗茨抢点被赖斯破坏"));
	if (HelperSequence > 0)
	{
		const auto Rebuilt = BuildPresentation(MakeCrossHighFacts(
			true, false, true, true, EFormulaWinner::Defender,
			EMatchPlayCrossActualBranch::High, HelperSequence));
		const auto OppositeViewer = BuildPresentation(MakeCrossHighFacts(
			true, false, true, true, EFormulaWinner::Defender,
			EMatchPlayCrossActualBranch::High, HelperSequence),
			EInitialTurnOrderPlayer::PlayerB);
		TestTrue(TEXT("Defensive presenter is stable across rebuilds and viewers"),
			Rebuilt.InlineFormula.NarrativeHeadline
				== HelperDefense.InlineFormula.NarrativeHeadline
				&& OppositeViewer.InlineFormula.NarrativeHeadline
					== HelperDefense.InlineFormula.NarrativeHeadline
				&& OppositeViewer.InlineFormula.DefensiveNarrativePerformer
					== EFMCodexUMGCrossDefensiveNarrativePerformer::Helper);
	}

	const auto LowAttackWin = BuildPresentation(MakeCrossHighFacts(
		true, true, true, true, EFormulaWinner::Attacker,
		EMatchPlayCrossActualBranch::Low)).InlineFormula;
	TestTrue(TEXT("Cross Low uses the same result narrative with route subtitle"),
		LowAttackWin.NarrativeHeadline == TEXT("萨卡传中，哈弗茨破门！")
			&& LowAttackWin.ResultSubtitle
				== TEXT("低球传中 · 进攻成功"));

	const auto AttackFallback = BuildPresentation(MakeCrossHighFacts(
		false, false, true, true, EFormulaWinner::Attacker),
		EInitialTurnOrderPlayer::PlayerA, false).InlineFormula;
	const auto DefenseFallback = BuildPresentation(MakeCrossHighFacts(
		false, false, true, true, EFormulaWinner::Defender),
		EInitialTurnOrderPlayer::PlayerA, false).InlineFormula;
	TestTrue(TEXT("Missing safe display names use localized generic fallbacks"),
		AttackFallback.NarrativeHeadline == TEXT("传中进攻成功")
			&& DefenseFallback.NarrativeHeadline
				== TEXT("传中被防守方破坏")
			&& !FlattenVisibleText(AttackFallback).Contains(TEXT("Fixture."))
			&& !FlattenVisibleText(DefenseFallback).Contains(TEXT("Fixture.")));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Completed narrative screen can be constructed"), Screen);
	if (Screen != nullptr)
	{
		Screen->TakeWidget();
		Screen->RefreshFromPresentation(AttackWin);
		TestTrue(TEXT("Bottom InteractionPanel duplicate terminal CTA is collapsed"),
			Screen->GetInteractionPanel() != nullptr
				&& Screen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Collapsed
				&& Screen->GetInlineFormulaSurface()->GetPresentation()
					.ContinueActionLabel == TEXT("下一回合"));
	}

	FFMCodexLocalMatchInteractionView CountView = MakeInteraction();
	CountView.bHasTacticalPlayerCounts = true;
	CountView.PlayerATacticalPlayerCount = 4;
	CountView.PlayerBTacticalPlayerCount = 2;
	FFMCodexLocalMatchResolutionFeedback EmptyFeedback;
	const FFMCodexUMGMatchScreenViewModel PlayerAView =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			CountView, EmptyFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	const FFMCodexUMGMatchScreenViewModel PlayerBView =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			CountView, EmptyFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Tactical counts map through local and opponent identity"),
		PlayerAView.LocalRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×4")
			&& PlayerAView.OpponentRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×2")
			&& PlayerBView.LocalRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×2")
			&& PlayerBView.OpponentRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×4"));
	CountView.PlayerATacticalPlayerCount = 0;
	CountView.PlayerBTacticalPlayerCount = 0;
	const FFMCodexUMGMatchScreenViewModel ZeroCountView =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			CountView, EmptyFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("Zero remains an explicit meaningful rack status"),
		ZeroCountView.LocalRack.bHasTacticalPlayerCount
			&& ZeroCountView.OpponentRack.bHasTacticalPlayerCount
			&& ZeroCountView.LocalRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×0")
			&& ZeroCountView.OpponentRack.TacticalPlayerCountLabel
				== TEXT("战术球员 ×0"));
	UFMCodexLocalMatchScreenWidget* CountScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	if (CountScreen != nullptr)
	{
		CountScreen->TakeWidget();
		CountScreen->RefreshFromPresentation(PlayerAView);
		const UTextBlock* LocalCount = Cast<UTextBlock>(
			CountScreen->GetLocalRackWidget()->GetWidgetFromName(
				TEXT("TacticalPlayerCountStatus")));
		const UTextBlock* OpponentCount = Cast<UTextBlock>(
			CountScreen->GetOpponentRackWidget()->GetWidgetFromName(
				TEXT("TacticalPlayerCountStatus")));
		TestTrue(TEXT("Both compact rack status labels render without layout duplication"),
			LocalCount != nullptr && OpponentCount != nullptr
				&& LocalCount->GetText().ToString() == TEXT("战术球员 ×4")
				&& OpponentCount->GetText().ToString()
					== TEXT("战术球员 ×2"));
	}

	FString PresentationSource;
	const FString PresentationPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"));
	TestTrue(TEXT("Presentation source is readable for RNG boundary guard"),
		FFileHelper::LoadFileToString(PresentationSource, *PresentationPath));
	TestTrue(TEXT("Narrative presentation consumes no gameplay or local RNG"),
		!PresentationSource.Contains(TEXT("FMath::Rand"))
			&& !PresentationSource.Contains(TEXT("FRandomStream"))
			&& !PresentationSource.Contains(TEXT("RandRange"))
			&& !PresentationSource.Contains(TEXT("D6Provider")));
	return true;
}

#if 0 // Superseded by the unified reel contract below.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInlineResolutionFormulaDiceRevealTest,
	"FMCodex.LocalPlay.DiceCycling.CrossCoveredRolls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInlineResolutionFormulaDiceRevealTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceTests;
	FFMCodexLocalMatchInteractionView PreRouteView = MakeInteraction();
	PreRouteView.ContinueActionLabel = TEXT("判定传中路线");
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
	TestTrue(TEXT("Covered pre-route step exposes typed route identity"),
		PreRoutePresentation.InlineFormula.bSuppressLegacyResolution
			&& !PreRouteScreen->IsLegacyResolutionOverlayVisible()
			&& PreRoutePresentation.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::InitialRoute
			&& PreRoutePresentation.Interaction.CrossRollContestId
				== TEXT("Cross.Route")
			&& PreRoutePresentation.Interaction.CrossRollSequenceIndex == 0
			&& PreRoutePresentation.Interaction.PrimaryActionLabel
				== TEXT("判定传中路线"));

	const FFMCodexUMGMatchScreenViewModel ManualPending = BuildPresentation(
		MakeCrossHighFacts(true, true, false, false));
	const FFMCodexUMGMatchScreenViewModel ManualAttackSettled = BuildPresentation(
		MakeCrossHighFacts(true, true, true, false));
	const FFMCodexUMGMatchScreenViewModel ManualCompleted = BuildPresentation(
		MakeCrossHighFacts(
			true, true, true, true, EFormulaWinner::Attacker));
	UFMCodexLocalMatchScreenWidget* ManualScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Manual roll fixture screen can be constructed"), ManualScreen);
	if (ManualScreen == nullptr)
	{
		return false;
	}
	ManualScreen->TakeWidget();
	ManualScreen->RefreshFromPresentation(PreRoutePresentation);
	ManualScreen->RefreshFromPresentation(ManualPending);
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	const auto& RouteCycling =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Route live transition starts one inline cosmetic cycle"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& ManualScreen->IsInlineFormulaRevealInputBlocked()
			&& RouteCycling.bVisible
			&& RouteCycling.ContestId == TEXT("Cross.Route")
			&& RouteCycling.RouteResultLabel.IsEmpty()
			&& !RouteCycling.bShowFormulaRows
			&& RouteCycling.bDiceRolling
			&& RouteCycling.DiceFaceLabel == TEXT("1")
			&& !RouteCycling.bCanContinue
			&& ManualScreen->GetPresentation().InlineFormula.RouteResultLabel
				== TEXT("路线掷点 5 → 判定为高球传中"));
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.60f);
	const FString SlowerFace = ManualScreen->GetInlineFormulaSurface()
		->GetPresentation().DiceFaceLabel;
	TestTrue(TEXT("Cosmetic D6 advances deterministically before settle"),
		SlowerFace != TEXT("1")
			&& FCString::Atoi(*SlowerFace) >= 1
			&& FCString::Atoi(*SlowerFace) <= 6);
	ManualScreen->RefreshFromPresentation(ManualPending);
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.30f);
	const auto& RouteSettling =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Route settle shows exact authority D6 but still hides branch"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& RouteSettling.DiceFaceLabel == TEXT("5")
			&& !RouteSettling.bDiceRolling
			&& RouteSettling.RouteResultLabel.IsEmpty()
			&& !RouteSettling.bCanContinue);
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.20f);
	const auto& RouteSettled =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Route outcome and attack action appear only after settle"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& !ManualScreen->IsInlineFormulaRevealInputBlocked()
			&& RouteSettled.RouteResultLabel
				== TEXT("路线掷点 5 → 判定为高球传中")
			&& RouteSettled.ContinueActionLabel == TEXT("进攻方掷点"));

	// Begin request-in-flight before the authority value exists. Crossing the
	// minimum duration must wait safely and must not settle a cosmetic face.
	ManualScreen->BeginPendingCrossRollRevealForTesting();
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	ManualScreen->AdvanceInlineFormulaRevealForTesting(1.20f);
	const FString WaitingFace = ManualScreen->GetInlineFormulaSurface()
		->GetPresentation().DiceFaceLabel;
	ManualScreen->BeginPendingCrossRollRevealForTesting();
	TestTrue(TEXT("Delayed authority waits in cycle and duplicate begin is ignored"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& ManualScreen->IsInlineFormulaRevealInputBlocked()
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.DiceFaceLabel == WaitingFace);
	ManualScreen->RefreshFromPresentation(ManualAttackSettled);
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	const auto& AttackSettling =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	const auto* AttackSettlingRoll = FindTerm(
		AttackSettling.AttackRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* PendingDefenseRoll = FindTerm(
		AttackSettling.DefenseRow,
		EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Attack settle gates FinalValue and defender action"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& AttackSettlingRoll != nullptr
			&& AttackSettlingRoll->RawD6 == 4
			&& AttackSettlingRoll->bResolved
			&& !AttackSettling.AttackRow.bFinalValueResolved
			&& !AttackSettling.AttackRow.bDisplayedResultIsFinalValue
			&& FMath::IsNearlyEqual(
				AttackSettling.AttackRow.DisplayedResult,
				AttackSettling.AttackRow.KnownNonRollSubtotal)
			&& PendingDefenseRoll != nullptr && !PendingDefenseRoll->bResolved
			&& AttackSettling.ContinueActionLabel.IsEmpty()
			&& ManualScreen->GetPresentation().InlineFormula.AttackRow
				.bFinalValueResolved
			&& !ManualScreen->IsLegacyResolutionOverlayVisible());
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.10f);
	ManualScreen->RefreshFromPresentation(ManualAttackSettled);
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.11f);
	const auto& AttackOnly =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Repeated rebuild does not restart accepted attack settle"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& AttackOnly.AttackRow.bFinalValueResolved
			&& AttackOnly.ContinueActionLabel == TEXT("防守方掷点"));
	ManualScreen->RefreshFromPresentation(ManualPending);
	TestTrue(TEXT("Stale attack-pending rebuild cannot replay or downgrade"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.AttackRow.bFinalValueResolved
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("防守方掷点"));
	ManualScreen->RefreshFromPresentation(ManualAttackSettled);

	ManualScreen->BeginPendingCrossRollRevealForTesting();
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.35f);
	ManualScreen->RefreshFromPresentation(ManualCompleted);
	ManualScreen->PauseInlineFormulaRevealTimerForTesting();
	const auto& DefenseCycling =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Defense cycle hides terminal narrative and CTA"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& !DefenseCycling.bNarrativeAvailable
			&& DefenseCycling.NarrativeHeadline.IsEmpty()
			&& !DefenseCycling.DefenseRow.bFinalValueResolved
			&& DefenseCycling.ContinueActionLabel.IsEmpty());
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.55f);
	TestTrue(TEXT("Defense reaches exact authoritative settle after minimum cycle"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.DiceFaceLabel == TEXT("3"));
	ManualScreen->AdvanceInlineFormulaRevealForTesting(0.20f);
	const auto& ManualFinal =
		ManualScreen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Defense settle releases authoritative narrative and terminal CTA"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& ManualFinal.AttackRow.bFinalValueResolved
			&& ManualFinal.DefenseRow.bFinalValueResolved
			&& ManualFinal.bNarrativeAvailable
			&& ManualFinal.ContinueActionLabel == TEXT("下一回合")
			&& !ManualScreen->IsInlineFormulaRevealInputBlocked());
	ManualScreen->RefreshFromPresentation(ManualCompleted);
	TestTrue(TEXT("Repeated completed rebuild never replays settled defense"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& !ManualScreen->IsInlineFormulaRevealInputBlocked());
	ManualScreen->RefreshFromPresentation(ManualAttackSettled);
	TestTrue(TEXT("Stale defense-pending rebuild keeps disclosed final truth"),
		ManualScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.DefenseRow.bFinalValueResolved
			&& ManualScreen->GetInlineFormulaSurface()->GetPresentation()
				.bNarrativeAvailable);

	UFMCodexLocalMatchScreenWidget* ReconstructedScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	ReconstructedScreen->TakeWidget();
	ReconstructedScreen->RefreshFromPresentation(ManualCompleted);
	TestTrue(TEXT("Already-resolved first observation renders settled immediately"),
		ReconstructedScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !ReconstructedScreen->IsInlineFormulaRevealInputBlocked()
			&& ReconstructedScreen->GetInlineFormulaSurface()->GetPresentation()
				.DefenseRow.bFinalValueResolved);

	UFMCodexLocalMatchScreenWidget* RejectedScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	RejectedScreen->TakeWidget();
	RejectedScreen->RefreshFromPresentation(ManualPending);
	RejectedScreen->BeginPendingCrossRollRevealForTesting();
	FFMCodexUMGMatchScreenViewModel RejectedPending = ManualPending;
	RejectedPending.Resolution.bRejected = true;
	RejectedScreen->RefreshFromPresentation(RejectedPending);
	TestTrue(TEXT("Rejected request cancels reveal and restores pending truth"),
		RejectedScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& !RejectedScreen->IsInlineFormulaRevealInputBlocked()
			&& RejectedScreen->GetInlineFormulaSurface()->GetPresentation()
				.AttackRow.bFinalValueResolved == false
			&& RejectedScreen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("进攻方掷点"));

	const FFMCodexUMGMatchScreenViewModel LowPending = BuildPresentation(
		MakeCrossHighFacts(false, false, false, false,
			EFormulaWinner::None, EMatchPlayCrossActualBranch::Low, 2));
	const FFMCodexUMGMatchScreenViewModel LowAttack = BuildPresentation(
		MakeCrossHighFacts(false, false, true, false,
			EFormulaWinner::None, EMatchPlayCrossActualBranch::Low, 2));
	const FFMCodexUMGMatchScreenViewModel LowCompleted = BuildPresentation(
		MakeCrossHighFacts(false, false, true, true,
			EFormulaWinner::Defender, EMatchPlayCrossActualBranch::Low, 2));
	FFMCodexLocalMatchInteractionView LowPreRouteView = MakeInteraction();
	LowPreRouteView.AttackSequence = 2;
	LowPreRouteView.ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::CrossLow;
	LowPreRouteView.ContinueActionLabel = TEXT("判定传中路线");
	const FFMCodexUMGMatchScreenViewModel LowPreRoute =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			LowPreRouteView, LegacyPreRouteFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	UFMCodexLocalMatchScreenWidget* LowScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	LowScreen->TakeWidget();
	LowScreen->RefreshFromPresentation(LowPreRoute);
	LowScreen->RefreshFromPresentation(LowPending);
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	LowScreen->AdvanceInlineFormulaRevealForTesting(1.10f);
	TestTrue(TEXT("Cross Low route uses the same reveal and settles to Low"),
		LowScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& LowScreen->GetInlineFormulaSurface()->GetPresentation().ContestId
				== TEXT("Cross.Low")
			&& LowScreen->GetInlineFormulaSurface()->GetPresentation()
				.RouteResultLabel
					== TEXT("路线掷点 2 → 判定为低球传中"));
	LowScreen->RefreshFromPresentation(LowAttack);
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Cross Low attack live transition cycles"),
		LowScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& LowScreen->IsInlineFormulaRevealInputBlocked());
	LowScreen->AdvanceInlineFormulaRevealForTesting(1.10f);
	LowScreen->RefreshFromPresentation(LowCompleted);
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Cross Low defense live transition cycles"),
		LowScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& !LowScreen->GetInlineFormulaSurface()->GetPresentation()
				.bNarrativeAvailable);
	LowScreen->AdvanceInlineFormulaRevealForTesting(1.10f);
	const auto& LowFinal =
		LowScreen->GetInlineFormulaSurface()->GetPresentation();
	const auto* LowAttackRoll = FindTerm(
		LowFinal.AttackRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* LowDefenseRoll = FindTerm(
		LowFinal.DefenseRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Cross Low terminal truth releases exact RawD6 only after settle"),
		LowScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& LowFinal.bNarrativeAvailable
			&& LowAttackRoll != nullptr && LowAttackRoll->RawD6 == 4
			&& LowDefenseRoll != nullptr && LowDefenseRoll->RawD6 == 3);

	FString ScreenSource;
	const FString ScreenPath = FPaths::Combine(
		FPaths::ProjectDir(),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"));
	TestTrue(TEXT("Reveal source is readable for RNG boundary guard"),
		FFileHelper::LoadFileToString(ScreenSource, *ScreenPath));
	TestTrue(TEXT("Cosmetic cycling contains no RNG or authority D6 call"),
		!ScreenSource.Contains(TEXT("FMath::Rand"))
			&& !ScreenSource.Contains(TEXT("RandRange"))
			&& !ScreenSource.Contains(TEXT("FRandomStream"))
			&& !ScreenSource.Contains(TEXT("RollD6")));
	return true;
}
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUnifiedRollReelRevealTest,
	"FMCodex.LocalPlay.RollReel.UnifiedCoveredRolls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUnifiedRollReelRevealTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceTests;
	(void)Parameters;

	FFMCodexLocalMatchResolutionFeedback LegacyPreRouteFeedback;
	LegacyPreRouteFeedback.bVisible = true;
	LegacyPreRouteFeedback.StepTitle = TEXT("Resolution Started");
	LegacyPreRouteFeedback.ContinuationSummary =
		TEXT("Continue - Resolve Route");
	FFMCodexLocalMatchInteractionView PreRouteView = MakeInteraction();
	PreRouteView.ContinueActionLabel = TEXT("判定传中路线");
	const FFMCodexUMGMatchScreenViewModel PreRoutePresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			PreRouteView, LegacyPreRouteFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	const FFMCodexUMGMatchScreenViewModel ManualPending = BuildPresentation(
		MakeCrossHighFacts(true, true, false, false));
	const FFMCodexUMGMatchScreenViewModel ManualAttackSettled = BuildPresentation(
		MakeCrossHighFacts(true, true, true, false));
	const FFMCodexUMGMatchScreenViewModel ManualCompleted = BuildPresentation(
		MakeCrossHighFacts(true, true, true, true, EFormulaWinner::Attacker));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Unified reel fixture screen can be constructed"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(PreRoutePresentation);
	Screen->RefreshFromPresentation(ManualPending);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	const auto& RouteStart =
		Screen->GetInlineFormulaSurface()->GetPresentation();
	UFMCodexRollReelWidget* FormulaReel =
		Screen->GetInlineFormulaSurface()->GetRollReelWidget();
	TestTrue(TEXT("Route reveal starts a clipped three-number vertical reel"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& RouteStart.RollReel.bVisible
			&& RouteStart.RollReel.DomainMinimum == 1
			&& RouteStart.RollReel.DomainMaximum == 6
			&& RouteStart.RouteResultLabel.IsEmpty()
			&& !RouteStart.bShowFormulaRows
			&& FormulaReel != nullptr
			&& FormulaReel->HasClippedWindow()
			&& FormulaReel->GetStripDigitCount() == 3
			&& FormulaReel->GetRenderedChildCount() == 3);
	const float InitialOffset = FormulaReel != nullptr
		? FormulaReel->GetCenterVerticalOffset() : 0.0f;
	Screen->AdvanceInlineFormulaRevealForTesting(0.016f);
	const float FirstFramePosition = FormulaReel != nullptr
		? FormulaReel->GetPresentation().ContinuousPositionCells : 0.0f;
	Screen->AdvanceInlineFormulaRevealForTesting(0.016f);
	const float SecondFramePosition = FormulaReel != nullptr
		? FormulaReel->GetPresentation().ContinuousPositionCells : 0.0f;
	TestTrue(TEXT("Adjacent rendered frames advance continuous spatial offset"),
		FormulaReel != nullptr
			&& !FMath::IsNearlyEqual(
				FormulaReel->GetCenterVerticalOffset(), InitialOffset)
			&& FirstFramePosition > 0.0f
			&& SecondFramePosition > FirstFramePosition
			&& FormulaReel->GetRenderedChildCount() == 3);
	const float EarlyStart = SecondFramePosition;
	Screen->AdvanceInlineFormulaRevealForTesting(0.10f);
	const float EarlyEnd = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	const float EarlyVelocity = (EarlyEnd - EarlyStart) / 0.10f;
	Screen->AdvanceInlineFormulaRevealForTesting(0.418f);
	const float MainStart = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	Screen->AdvanceInlineFormulaRevealForTesting(0.10f);
	const float MainEnd = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	const float MainVelocity = (MainEnd - MainStart) / 0.10f;
	Screen->AdvanceInlineFormulaRevealForTesting(0.45f);
	const float LateStart = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	Screen->AdvanceInlineFormulaRevealForTesting(0.10f);
	const float LateEnd = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	const float LateVelocity = (LateEnd - LateStart) / 0.10f;
	TestTrue(TEXT("C.3 lowers fast speed and makes both slowdown stages visible"),
		EarlyVelocity < 14.0f && EarlyVelocity > 11.0f
			&& EarlyVelocity > MainVelocity
			&& MainVelocity > LateVelocity
			&& LateVelocity > 0.0f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.10f);
	TestTrue(TEXT("1.30-second cycling enters continuous target capture"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.RollReel.bMoving
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.RollReel.bAuthoritativeValue
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.RouteResultLabel.IsEmpty());
	const float CaptureStart = FormulaReel->GetPresentation()
		.ContinuousPositionCells;
	const UTextBlock* StableCenterDigit = FormulaReel->GetCenterDigitWidget();
	const FLinearColor StableReelFrameColor =
		FormulaReel->GetFrameBrushColor();
	Screen->AdvanceInlineFormulaRevealForTesting(0.08f);
	const float EarlyTransitionNeighborOpacity =
		FormulaReel->GetMaximumNeighborRenderOpacity();
	TestTrue(TEXT("Final capture glides spatially instead of replacing center text"),
		FormulaReel->GetPresentation().ContinuousPositionCells > CaptureStart
			&& !FormulaReel->GetPresentation().bAuthoritativeValue
			&& FormulaReel->GetCenterDigitWidget() == StableCenterDigit
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor)
			&& FormulaReel->GetCenterRenderOpacity() > 0.95f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.036f);
	const float PeakLandingOffset = FormulaReel->GetCenterVerticalOffset();
	const float PeakLandingScale = FormulaReel->GetCenterRenderScale();
	TestTrue(TEXT("Landing applies one restrained three-pixel and 1.08 scale lock"),
		PeakLandingOffset <= -2.5f && PeakLandingOffset >= -3.1f
			&& PeakLandingScale >= 1.07f && PeakLandingScale <= 1.081f
			&& FormulaReel->GetMaximumNeighborRenderOpacity()
				< EarlyTransitionNeighborOpacity
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor)
			&& FormulaReel->GetCenterRenderOpacity() > 0.99f);
	const float PeakNeighborOpacity =
		FormulaReel->GetMaximumNeighborRenderOpacity();
	Screen->AdvanceInlineFormulaRevealForTesting(0.02f);
	TestTrue(TEXT("Landing returns toward center without a second bounce"),
		FormulaReel->GetCenterVerticalOffset() < 0.0f
			&& FormulaReel->GetCenterVerticalOffset() > PeakLandingOffset
			&& FormulaReel->GetCenterRenderScale() < PeakLandingScale
			&& FormulaReel->GetMaximumNeighborRenderOpacity()
				< PeakNeighborOpacity
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor));
	Screen->RefreshFromPresentation(ManualPending);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("DTO rebuild during settle preserves the fixed reel style"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor)
			&& FormulaReel->GetCenterDigitWidget() == StableCenterDigit);
	Screen->AdvanceInlineFormulaRevealForTesting(0.02f);
	TestTrue(TEXT("Settling visually completes before ResultHold state swap"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& FormulaReel->GetMaximumNeighborRenderOpacity() < 0.02f
			&& FormulaReel->GetPresentation().NeighborFadeAlpha > 0.98f
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor)
			&& FormulaReel->GetCenterRenderOpacity() > 0.99f
			&& FMath::Abs(FormulaReel->GetCenterVerticalOffset()) < 0.10f
			&& FMath::Abs(FormulaReel->GetCenterRenderScale() - 1.0f) < 0.01f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.004f);
	TestTrue(TEXT("Route enters a 1.45-second result hold before next action"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.RouteResultLabel
					== TEXT("路线掷点 5 → 判定为高球传中")
			&& FormulaReel->GetPresentation().CenterValue == 5
			&& FormulaReel->GetPresentation().bStaticResult
			&& FormulaReel->GetVisibleNeighborDigitCount() == 0
			&& FormulaReel->IsStaticResultTileVisible()
			&& FormulaReel->GetCenterDigitWidget() == StableCenterDigit
			&& FormulaReel->GetCenterRenderOpacity() > 0.99f
			&& FormulaReel->GetFrameBrushColor().Equals(
				StableReelFrameColor)
			&& FMath::IsNearlyZero(FormulaReel->GetCenterVerticalOffset())
			&& FMath::IsNearlyEqual(
				FormulaReel->GetCenterRenderScale(), 1.0f)
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel.IsEmpty());
	Screen->RefreshFromPresentation(ManualPending);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("ResultHold rebuild cannot restore faded reel neighbors"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& FormulaReel->GetVisibleNeighborDigitCount() == 0
			&& FormulaReel->GetCenterDigitWidget() == StableCenterDigit);
	Screen->AdvanceInlineFormulaRevealForTesting(1.43f);
	TestTrue(TEXT("Route remains blocked until the complete hold elapses"),
		Screen->IsInlineFormulaRevealInputBlocked());
	Screen->AdvanceInlineFormulaRevealForTesting(0.02f);
	TestTrue(TEXT("Route releases the already-authoritative attack action"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("进攻方掷点"));

	// Begin without an authority result, then receive the accepted value late.
	Screen->BeginPendingCrossRollRevealForTesting();
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(1.35f);
	const int32 WaitingCenter = Screen->GetInlineFormulaSurface()
		->GetPresentation().RollReel.CenterValue;
	Screen->BeginPendingCrossRollRevealForTesting();
	TestTrue(TEXT("Delayed authority loops safely and duplicate begin is ignored"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.RollReel.CenterValue == WaitingCenter);
	Screen->RefreshFromPresentation(ManualAttackSettled);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Late accepted attack value starts exact authority capture"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.RollReel.bMoving
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.RollReel.bAuthoritativeValue);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestTrue(TEXT("Attack result hold initially withholds derived final value"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.AttackRow.bFinalValueResolved
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(0.17f);
	TestTrue(TEXT("Formula disclosure waits approximately 0.18 seconds"),
		!Screen->GetInlineFormulaSurface()->GetPresentation()
			.AttackRow.bFinalValueResolved);
	Screen->AdvanceInlineFormulaRevealForTesting(0.02f);
	TestTrue(TEXT("Formula updates inside hold while defender CTA stays gated"),
		Screen->GetInlineFormulaSurface()->GetPresentation()
			.AttackRow.bFinalValueResolved
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel.IsEmpty()
			&& Screen->IsInlineFormulaRevealInputBlocked());
	Screen->RefreshFromPresentation(ManualAttackSettled);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Rebuild during accepted hold does not restart the reel"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold);
	Screen->AdvanceInlineFormulaRevealForTesting(2.37f);
	TestTrue(TEXT("Attack remains blocked through the readable-result window"),
		Screen->IsInlineFormulaRevealInputBlocked());
	Screen->AdvanceInlineFormulaRevealForTesting(0.03f);
	TestTrue(TEXT("Attack releases defender after 2.4 readable seconds"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("防守方掷点"));
	Screen->RefreshFromPresentation(ManualPending);
	TestTrue(TEXT("Stale pending rebuild cannot replay or downgrade attack truth"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.AttackRow.bFinalValueResolved);
	Screen->RefreshFromPresentation(ManualAttackSettled);

	Screen->BeginPendingCrossRollRevealForTesting();
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(0.35f);
	Screen->RefreshFromPresentation(ManualCompleted);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	Screen->AdvanceInlineFormulaRevealForTesting(0.95f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestTrue(TEXT("Defense hold starts before terminal narrative disclosure"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.bNarrativeAvailable
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContestLabel != ManualCompleted.InlineFormula.NarrativeHeadline
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.DefenseRow.bFinalValueResolved
			&& Screen->IsInlineFormulaRevealInputBlocked());
	Screen->AdvanceInlineFormulaRevealForTesting(0.21f);
	TestTrue(TEXT("Defense FinalValue discloses before terminal Narrative"),
		Screen->GetInlineFormulaSurface()->GetPresentation()
			.DefenseRow.bFinalValueResolved
			&& !Screen->GetInlineFormulaSurface()->GetPresentation()
				.bNarrativeAvailable
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel.IsEmpty()
			&& Screen->IsInlineFormulaRevealInputBlocked());
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestFalse(TEXT("Narrative remains hidden through its short transition"),
		Screen->GetInlineFormulaSurface()->GetPresentation()
			.bNarrativeAvailable);
	Screen->AdvanceInlineFormulaRevealForTesting(0.03f);
	TestTrue(TEXT("Terminal Narrative discloses only after the formula gate"),
		Screen->GetInlineFormulaSurface()->GetPresentation()
			.bNarrativeAvailable
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContestLabel
					== ManualCompleted.InlineFormula.NarrativeHeadline
			&& Screen->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(2.17f);
	TestTrue(TEXT("Terminal CTA remains blocked through readable result hold"),
		Screen->IsInlineFormulaRevealInputBlocked());
	Screen->AdvanceInlineFormulaRevealForTesting(0.03f);
	const auto& ManualFinal =
		Screen->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Defense releases terminal truth only after full hold"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& !Screen->IsInlineFormulaRevealInputBlocked()
			&& ManualFinal.AttackRow.bFinalValueResolved
			&& ManualFinal.DefenseRow.bFinalValueResolved
			&& ManualFinal.bNarrativeAvailable
			&& ManualFinal.ContinueActionLabel == TEXT("下一回合"));
	Screen->RefreshFromPresentation(ManualCompleted);
	TestTrue(TEXT("Repeated completed rebuild never replays settled defense"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& !Screen->IsInlineFormulaRevealInputBlocked());

	UFMCodexLocalMatchScreenWidget* Reconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Reconstructed->TakeWidget();
	Reconstructed->RefreshFromPresentation(ManualCompleted);
	TestTrue(TEXT("Already-resolved first observation renders without replay"),
		Reconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !Reconstructed->IsInlineFormulaRevealInputBlocked()
			&& Reconstructed->GetInlineFormulaSurface()->GetPresentation()
				.DefenseRow.bFinalValueResolved);

	UFMCodexLocalMatchScreenWidget* Rejected =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Rejected->TakeWidget();
	Rejected->RefreshFromPresentation(ManualPending);
	Rejected->BeginPendingCrossRollRevealForTesting();
	FFMCodexUMGMatchScreenViewModel RejectedPending = ManualPending;
	RejectedPending.Resolution.bRejected = true;
	Rejected->RefreshFromPresentation(RejectedPending);
	TestTrue(TEXT("Rejected request cancels the reel and restores pending truth"),
		Rejected->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& !Rejected->IsInlineFormulaRevealInputBlocked()
			&& Rejected->GetInlineFormulaSurface()->GetPresentation()
				.ContinueActionLabel == TEXT("进攻方掷点"));

	// Tactical Points use their production 2..8 random object, not a D6 skin.
	FFMCodexLocalMatchInteractionView TacticalPendingView = MakeInteraction();
	TacticalPendingView.bCurrentAttackActive = false;
	TacticalPendingView.bTacticalPointRollReady = true;
	TacticalPendingView.ActionPoint = 0;
	TacticalPendingView.MajorPhase = EFMCodexLocalMatchMajorPhase::BetweenAttacks;
	TacticalPendingView.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::TacticalPointRoll;
	TacticalPendingView.bHumanInteraction = true;
	FFMCodexLocalMatchInteractionView TacticalResolvedView = TacticalPendingView;
	TacticalResolvedView.bCurrentAttackActive = true;
	TacticalResolvedView.bTacticalPointRollReady = false;
	TacticalResolvedView.ActionPoint = 6;
	TacticalResolvedView.MajorPhase = EFMCodexLocalMatchMajorPhase::Deployment;
	TacticalResolvedView.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::Deploy;
	const FFMCodexLocalMatchResolutionFeedback EmptyFeedback;
	const auto TacticalPending =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			TacticalPendingView, EmptyFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	const auto TacticalResolved =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			TacticalResolvedView, EmptyFeedback, FString(),
			EInitialTurnOrderPlayer::PlayerA);
	UFMCodexLocalMatchScreenWidget* TacticalScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TacticalScreen->TakeWidget();
	TacticalScreen->RefreshFromPresentation(TacticalPending);
	TacticalScreen->BeginPendingTacticalPointRevealForTesting();
	TacticalScreen->PauseInlineFormulaRevealTimerForTesting();
	TacticalScreen->RefreshFromPresentation(TacticalResolved);
	TacticalScreen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexRollReelWidget* TacticalReel =
		TacticalScreen->GetTacticalPointRollReel();
	TestTrue(TEXT("Tactical Point reveal uses one clipped 2..8 reel"),
		TacticalScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& TacticalScreen->IsInlineFormulaRevealInputBlocked()
			&& TacticalReel != nullptr
			&& TacticalReel->GetPresentation().DomainMinimum == 2
			&& TacticalReel->GetPresentation().DomainMaximum == 8
			&& TacticalReel->HasClippedWindow()
			&& TacticalReel->GetRenderedChildCount() == 3
			&& !TacticalScreen->GetMatchHeader()->GetPresentation()
				.bShowLeftTacticalPointChip);
	TacticalScreen->AdvanceInlineFormulaRevealForTesting(1.30f);
	TacticalScreen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestTrue(TEXT("Tactical raw result equals authoritative final resource"),
		TacticalScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& TacticalReel->GetPresentation().CenterValue == 6
			&& TacticalReel->GetPresentation().bAuthoritativeValue
			&& TacticalReel->GetPresentation().bStaticResult
			&& TacticalReel->GetVisibleNeighborDigitCount() == 0
			&& TacticalReel->IsStaticResultTileVisible()
			&& !TacticalScreen->GetMatchHeader()->GetPresentation()
				.bShowLeftTacticalPointChip);
	TacticalScreen->AdvanceInlineFormulaRevealForTesting(0.21f);
	TestTrue(TEXT("Tactical resource discloses after settle but remains gated"),
		TacticalScreen->GetMatchHeader()->GetPresentation()
			.bShowLeftTacticalPointChip
			&& TacticalScreen->GetMatchHeader()->GetPresentation()
				.CurrentAttackerTacticalPoints == 6
			&& TacticalScreen->IsInlineFormulaRevealInputBlocked());
	TacticalScreen->AdvanceInlineFormulaRevealForTesting(2.35f);
	TestTrue(TEXT("Deployment remains blocked through readable resource hold"),
		TacticalScreen->IsInlineFormulaRevealInputBlocked());
	TacticalScreen->AdvanceInlineFormulaRevealForTesting(0.03f);
	TestTrue(TEXT("Tactical deployment releases after 2.4 readable seconds"),
		!TacticalScreen->IsInlineFormulaRevealInputBlocked()
			&& TacticalScreen->GetPresentation().Interaction.Category
				== EFMCodexUMGInteractionCategory::Deploy);

	// Cross Low is covered by the same state machine and exact RawD6 facts.
	const auto LowPending = BuildPresentation(MakeCrossHighFacts(
		false, false, false, false, EFormulaWinner::None,
		EMatchPlayCrossActualBranch::Low, 2));
	const auto LowAttack = BuildPresentation(MakeCrossHighFacts(
		false, false, true, false, EFormulaWinner::None,
		EMatchPlayCrossActualBranch::Low, 2));
	const auto LowCompleted = BuildPresentation(MakeCrossHighFacts(
		false, false, true, true, EFormulaWinner::Defender,
		EMatchPlayCrossActualBranch::Low, 2));
	UFMCodexLocalMatchScreenWidget* LowScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	LowScreen->TakeWidget();
	LowScreen->RefreshFromPresentation(LowPending);
	LowScreen->BeginPendingCrossRollRevealForTesting();
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	LowScreen->RefreshFromPresentation(LowAttack);
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	LowScreen->AdvanceInlineFormulaRevealForTesting(4.20f);
	LowScreen->BeginPendingCrossRollRevealForTesting();
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	LowScreen->RefreshFromPresentation(LowCompleted);
	LowScreen->PauseInlineFormulaRevealTimerForTesting();
	LowScreen->AdvanceInlineFormulaRevealForTesting(4.20f);
	const auto& LowFinal =
		LowScreen->GetInlineFormulaSurface()->GetPresentation();
	const auto* LowAttackRoll = FindTerm(
		LowFinal.AttackRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 1);
	const auto* LowDefenseRoll = FindTerm(
		LowFinal.DefenseRow, EFMCodexUMGInlineFormulaTermKind::RawRoll, 2);
	TestTrue(TEXT("Cross Low shares terminal reveal without changing RawD6"),
		LowFinal.bNarrativeAvailable
			&& LowAttackRoll != nullptr && LowAttackRoll->RawD6 == 4
			&& LowDefenseRoll != nullptr && LowDefenseRoll->RawD6 == 3);

	FString ScreenSource;
	FString ReelSource;
	FString TacticalProviderSource;
	FString HostSource;
	TestTrue(TEXT("Reveal implementation sources are readable"),
		FFileHelper::LoadFileToString(ScreenSource, *FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp")))
		&& FFileHelper::LoadFileToString(ReelSource, *FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexRollReelWidget.cpp")))
		&& FFileHelper::LoadFileToString(TacticalProviderSource, *FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.cpp")))
		&& FFileHelper::LoadFileToString(HostSource, *FPaths::Combine(
			FPaths::ProjectDir(),
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"))));
	TestTrue(TEXT("Cosmetic reel consumes no gameplay RNG"),
		!ScreenSource.Contains(TEXT("FMath::Rand"))
			&& !ScreenSource.Contains(TEXT("RandRange"))
			&& !ScreenSource.Contains(TEXT("FRandomStream"))
			&& !ScreenSource.Contains(TEXT("RollD6"))
			&& !ReelSource.Contains(TEXT("FMath::Rand"))
			&& !ReelSource.Contains(TEXT("RandRange"))
			&& !ReelSource.Contains(TEXT("FRandomStream")));
	TestTrue(TEXT("Active motion is scheduled per rendered frame, not by 40ms steps"),
		ScreenSource.Contains(TEXT("SetTimerForNextTick"))
			&& ScreenSource.Contains(TEXT("GetDeltaSeconds"))
			&& ScreenSource.Contains(TEXT("RefreshActiveRollReelVisuals")));
	TestTrue(TEXT("Tactical production authority remains one direct 2..8 roll"),
		TacticalProviderSource.Contains(TEXT("RandomStream.RandRange(2, 8)"))
			&& HostSource.Contains(TEXT("RollOrdinaryTacticalPoint()"))
			&& HostSource.Contains(TEXT("BeginOrdinaryAttack(")));
	return true;
}

#endif
