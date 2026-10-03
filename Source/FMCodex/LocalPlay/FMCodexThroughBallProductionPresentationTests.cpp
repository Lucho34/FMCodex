#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "FMCodexResolutionTheaterPrototype.h"

#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
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
		const EFormulaWinner Winner = EFormulaWinner::None,
		const float AttackerTacticalPlayerModifier = 0.0f,
		const float DefenderTacticalPlayerModifier = 0.0f)
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
		auto TacticalPlayerTerm = [](const FName TermId,
			const EInitialTurnOrderPlayer Side, const float Modifier)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.TermId = TermId;
			Term.Kind = ETermKind::TacticalPlayerAdvantage;
			Term.Side = Side;
			Term.SourceValue = Modifier;
			Term.Contribution = Modifier;
			Term.bResolved = true;
			return Term;
		};
		Facts.AttackerTacticalPlayerModifier = AttackerTacticalPlayerModifier;
		Facts.DefenderTacticalPlayerModifier = DefenderTacticalPlayerModifier;

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
		if (!FMath::IsNearlyZero(AttackerTacticalPlayerModifier))
		{
			Contest.AttackRow.Terms.Add(TacticalPlayerTerm(
				TEXT("TacticalPlayer.Advantage.Attack"),
				EInitialTurnOrderPlayer::PlayerA,
				AttackerTacticalPlayerModifier));
		}
		Contest.AttackRow.Terms.Add(RollTerm(1, bAttackResolved, 5));
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal =
			4.5f + AttackerTacticalPlayerModifier;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved
			? 9.5f + AttackerTacticalPlayerModifier : 0.0f;
		Contest.DefenseRow.RowId = TEXT("ThroughBall.Feet.Defense");
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms.Add(AttributeTerm(
			TEXT("Marker.Defense"),
			EMatchPlayResolutionParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId,
			EMatchPlayResolutionFormulaAttribute::Defense, 5.5f));
		if (!FMath::IsNearlyZero(DefenderTacticalPlayerModifier))
		{
			Contest.DefenseRow.Terms.Add(TacticalPlayerTerm(
				TEXT("TacticalPlayer.Advantage.Defense"),
				EInitialTurnOrderPlayer::PlayerB,
				DefenderTacticalPlayerModifier));
		}
		Contest.DefenseRow.Terms.Add(RollTerm(2, bDefenseResolved, 3));
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal =
			5.5f + DefenderTacticalPlayerModifier;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved
			? 8.5f + DefenderTacticalPlayerModifier : 0.0f;
		Contest.bHasResolvedFormula = bAttackResolved && bDefenseResolved;
		if (Contest.bHasResolvedFormula && Winner != EFormulaWinner::None)
		{
			Contest.ResolvedResult.FormulaType = EFormulaType::Finishing;
			Contest.ResolvedResult.WinReason=EFormulaWinReason::HigherFinalValue;
			Contest.ResolvedResult.AttackerFinalValue=Contest.AttackRow.FinalValue;
			Contest.ResolvedResult.DefenderFinalValue=Contest.DefenseRow.FinalValue;
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

	FMatchPlayCurrentAttackResolutionFactProjection MakeBehindFormulaFacts(
		const bool bAttackResolved,
		const bool bDefenseResolved,
		const EMatchPlayResolutionDecisionOutcome Outcome =
			EMatchPlayResolutionDecisionOutcome::None,
		const bool bHasHelper = true,
		const int64 AttackSequence = 41,
		const int32 AttackD6 = 3,
		const int32 DefenseD6 = 2)
	{
		using ETermKind = EMatchPlayResolutionFormulaTermKind;
		using ERollPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
		FMatchPlayCurrentAttackResolutionFactProjection Facts = MakeFacts(
			EMatchPlayThroughBallActualBranch::BehindDefense, 4,
			AttackSequence);
		const FName CarrierId(TEXT("Fixture.Behind.Carrier"));
		const FName RunnerId(TEXT("Fixture.Behind.Runner"));
		const FName MarkerId(TEXT("Fixture.Behind.Marker"));
		const FName HelperId(TEXT("Fixture.Behind.Helper"));
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId });
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA, RunnerId });
		Facts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId });
		if (bHasHelper)
		{
			Facts.Participants.Add({
				EMatchPlayResolutionParticipantRole::Helper,
				EInitialTurnOrderPlayer::PlayerB, HelperId });
		}

		auto AddRoll = [&Facts](
			const int32 SequenceIndex,
			const ERollPurpose Purpose,
			const EInitialTurnOrderPlayer Side,
			const bool bResolved,
			const int32 RawD6,
			const bool bConditional)
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
			Roll.bConditionallyRequired = bConditional;
			Roll.bResolved = bResolved;
			Roll.RawD6 = bResolved ? RawD6 : 0;
			Facts.Rolls.Add(Roll);
		};
		AddRoll(1, ERollPurpose::PrimaryAttack,
			EInitialTurnOrderPlayer::PlayerA, bAttackResolved, AttackD6, false);
		AddRoll(2, ERollPurpose::PrimaryDefense,
			EInitialTurnOrderPlayer::PlayerB, bDefenseResolved, DefenseD6, true);
		Facts.bHasPendingRoll = !bAttackResolved || (!bDefenseResolved
			&& Outcome != EMatchPlayResolutionDecisionOutcome::OutOfPlay);
		Facts.NextPendingRollSequenceIndex = !bAttackResolved
			? 1 : Facts.bHasPendingRoll ? 2 : INDEX_NONE;

		auto AttributeTerm = [](
			const FName TermId,
			const EMatchPlayResolutionParticipantRole Role,
			const EInitialTurnOrderPlayer Side,
			const FName CardId,
			const EMatchPlayResolutionFormulaAttribute Attribute,
			const float SourceValue)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.TermId = TermId;
			Term.Kind = ETermKind::Attribute;
			Term.ParticipantRole = Role;
			Term.Side = Side;
			Term.CardId = CardId;
			Term.Attribute = Attribute;
			Term.SourceValue = SourceValue;
			Term.Multiplier = 0.5f;
			Term.Contribution = SourceValue * 0.5f;
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
		Contest.ContestId = TEXT("ThroughBall.BehindDefense.P1");
		Contest.FormulaType = EFormulaType::Transition;
		Contest.Application = Outcome
			== EMatchPlayResolutionDecisionOutcome::OutOfPlay
				? EMatchPlayResolutionFormulaApplication
					::SkippedByAuthoritativeGate
				: bAttackResolved && bDefenseResolved
					? EMatchPlayResolutionFormulaApplication::Applied
					: EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.RowId = TEXT("ThroughBall.BehindDefense.P1.Attack");
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms.Add(AttributeTerm(
			TEXT("Carrier.PrimaryHalf"),
			EMatchPlayResolutionParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA, CarrierId,
			EMatchPlayResolutionFormulaAttribute::Passing, 8.0f));
		Contest.AttackRow.Terms.Add(AttributeTerm(
			TEXT("Runner.PrimaryHalf"),
			EMatchPlayResolutionParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA, RunnerId,
			EMatchPlayResolutionFormulaAttribute::Speed, 6.0f));
		Contest.AttackRow.Terms.Add(RollTerm(1, bAttackResolved, AttackD6));
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 7.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved
			? 7.0f + AttackD6 : 0.0f;
		Contest.DefenseRow.RowId = TEXT("ThroughBall.BehindDefense.P1.Defense");
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms.Add(AttributeTerm(
			TEXT("Marker.PrimaryHalf"),
			EMatchPlayResolutionParticipantRole::Marker,
			EInitialTurnOrderPlayer::PlayerB, MarkerId,
			EMatchPlayResolutionFormulaAttribute::Defense, 8.0f));
		if (bHasHelper)
		{
			Contest.DefenseRow.Terms.Add(AttributeTerm(
				TEXT("Helper.PrimaryHalf"),
				EMatchPlayResolutionParticipantRole::Helper,
				EInitialTurnOrderPlayer::PlayerB, HelperId,
				EMatchPlayResolutionFormulaAttribute::Speed, 6.0f));
		}
		Contest.DefenseRow.Terms.Add(RollTerm(
			2, bDefenseResolved, DefenseD6));
		FMatchPlayResolutionFormulaTermFact Fixed;
		Fixed.TermId = TEXT("Defense.FixedBonus");
		Fixed.Kind = ETermKind::FixedModifier;
		Fixed.SourceValue = 1.0f;
		Fixed.Contribution = 1.0f;
		Contest.DefenseRow.Terms.Add(Fixed);
		const float DefenseSubtotal = bHasHelper ? 8.0f : 5.0f;
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = DefenseSubtotal;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved
			? DefenseSubtotal + DefenseD6 : 0.0f;
		Contest.bHasResolvedFormula = bAttackResolved && bDefenseResolved;
		if (Contest.bHasResolvedFormula)
		{
			Contest.ResolvedResult.FormulaType = EFormulaType::Transition;
			Contest.ResolvedResult.WinReason=EFormulaWinReason::FastSuppression;
			Contest.ResolvedResult.Winner = Outcome
				== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired
					? EFormulaWinner::Attacker : EFormulaWinner::Defender;
		}
		Facts.FormulaContests.Add(Contest);

		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = TEXT(
			"ThroughBall.BehindDefense.P1.Outcome");
		Decision.Semantics =
			EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Decision.RollSequenceIndices = { 1, 2 };
		Decision.bResolved = Outcome
			!= EMatchPlayResolutionDecisionOutcome::None;
		Decision.Outcome = Outcome;
		Facts.Decisions.Add(Decision);
		return Facts;
	}

	FFMCodexLocalMatchInteractionView MakeBehindView(
		const bool bAttackResolved,
		const bool bDefenseResolved,
		const EMatchPlayResolutionDecisionOutcome Outcome =
			EMatchPlayResolutionDecisionOutcome::None,
		const bool bHasHelper = true,
		const int64 AttackSequence = 41,
		const int32 AttackD6 = 3,
		const int32 DefenseD6 = 2)
	{
		FFMCodexLocalMatchInteractionView View = MakeView();
		View.AttackSequence = AttackSequence;
		View.ResolutionFacts = MakeBehindFormulaFacts(
			bAttackResolved, bDefenseResolved, Outcome, bHasHelper,
			AttackSequence, AttackD6, DefenseD6);
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
			TEXT("Fixture.Behind.Carrier"), TEXT("厄德高"));
		AddRosterCard(EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.Behind.Runner"), TEXT("哈兰德"));
		AddRosterCard(EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.Behind.Marker"), TEXT("萨利巴"));
		if (bHasHelper)
		{
			AddRosterCard(EInitialTurnOrderPlayer::PlayerB,
				TEXT("Fixture.Behind.Helper"), TEXT("赖斯"));
		}
		if (!bAttackResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseAttack;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.bThroughBallBehindDefenseAttackRollPending = true;
			View.ContinueActionLabel = TEXT("进攻方掷点");
		}
		else if (!bDefenseResolved
			&& Outcome != EMatchPlayResolutionDecisionOutcome::OutOfPlay)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallBehindDefenseDefense;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
			View.bThroughBallBehindDefenseDefenseRollPending = true;
			View.ContinueActionLabel = TEXT("防守方掷点");
		}
		else if (Outcome
			== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::SelectOneOnOneShot;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.OneOnOneOptions = {
				EMatchPlayThroughBallOneOnOneShotChoice::DirectShot,
				EMatchPlayThroughBallOneOnOneShotChoice::ChipShot };
		}
		else
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::AdvanceAfterTerminal;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
			View.bTerminalPendingAdvance = true;
			View.ContinueActionLabel = TEXT("下一回合");
		}
		return View;
	}

	void AddOneOnOneRoster(FFMCodexLocalMatchInteractionView& View)
	{
		auto Add = [&View](const EInitialTurnOrderPlayer Side,
			const FName CardId, const FString& Name)
		{
			FFMCodexLocalMatchCardView Card;
			Card.Side = Side;
			Card.CardId = CardId;
			Card.DisplayLabel = Name;
			(Side == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster).Add(Card);
		};
		Add(EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.OneOnOne.Carrier"), TEXT("厄德高"));
		Add(EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.OneOnOne.Runner"), TEXT("哈兰德"));
		Add(EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.OneOnOne.Goalkeeper"), TEXT("阿利松"));
	}

	void AddOneOnOneParticipants(
		FMatchPlayCurrentAttackResolutionFactProjection& Facts)
	{
		Facts.Participants.Add({ EMatchPlayResolutionParticipantRole::Carrier,
			EInitialTurnOrderPlayer::PlayerA,
			FName(TEXT("Fixture.OneOnOne.Carrier")) });
		Facts.Participants.Add({ EMatchPlayResolutionParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA,
			FName(TEXT("Fixture.OneOnOne.Runner")) });
		Facts.Participants.Add({ EMatchPlayResolutionParticipantRole::Goalkeeper,
			EInitialTurnOrderPlayer::PlayerB,
			FName(TEXT("Fixture.OneOnOne.Goalkeeper")) });
	}

	FFMCodexLocalMatchInteractionView MakeAntiView(
		const bool bResolved,
		const EMatchPlayResolutionDecisionOutcome Outcome =
			EMatchPlayResolutionDecisionOutcome::None,
		const int32 RawD6 = 0)
	{
		FFMCodexLocalMatchInteractionView View = MakeView();
		View.ResolutionFacts = MakeFacts(
			EMatchPlayThroughBallActualBranch::AntiOffside, 6);
		AddOneOnOneParticipants(View.ResolutionFacts);
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = 1;
		Roll.OperandId = TEXT("AntiOffsideD6");
		Roll.PostRoutePurpose =
			EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack;
		Roll.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
		Roll.OwningSide = EInitialTurnOrderPlayer::PlayerA;
		Roll.bResolved = bResolved;
		Roll.RawD6 = bResolved ? RawD6 : 0;
		View.ResolutionFacts.Rolls.Add(Roll);
		View.ResolutionFacts.bHasPendingRoll = !bResolved;
		View.ResolutionFacts.NextPendingRollSequenceIndex =
			bResolved ? INDEX_NONE : 1;
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = TEXT("ThroughBall.AntiOffside.Outcome");
		Decision.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
		Decision.RollSequenceIndices = { 1 };
		Decision.bResolved = bResolved;
		Decision.Outcome = bResolved
			? Outcome : EMatchPlayResolutionDecisionOutcome::None;
		View.ResolutionFacts.Decisions.Add(Decision);
		AddOneOnOneRoster(View);
		if (!bResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallAntiOffsideAttack;
			View.bThroughBallAntiOffsideAttackRollPending = true;
			View.ContinueActionLabel = TEXT("进攻方掷点");
		}
		else if (Outcome
			== EMatchPlayResolutionDecisionOutcome::OneOnOneRequired)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::SelectOneOnOneShot;
			View.OneOnOneOptions = {
				EMatchPlayThroughBallOneOnOneShotChoice::DirectShot,
				EMatchPlayThroughBallOneOnOneShotChoice::ChipShot };
		}
		else
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::AdvanceAfterTerminal;
			View.bTerminalPendingAdvance = true;
			View.ContinueActionLabel = TEXT("下一回合");
		}
		return View;
	}

	FFMCodexLocalMatchInteractionView MakeChipView(
		const bool bResolved,
		const EMatchPlayResolutionDecisionOutcome Outcome =
			EMatchPlayResolutionDecisionOutcome::None,
		const int32 RawD6 = 0)
	{
		FFMCodexLocalMatchInteractionView View = MakeAntiView(true,
			EMatchPlayResolutionDecisionOutcome::OneOnOneRequired, 6);
		View.OneOnOneOptions.Reset();
		View.OneOnOneChoiceLabel = TEXT("Chip Shot");
		FMatchPlayResolutionRollFact Roll;
		Roll.SequenceIndex = 2;
		Roll.OperandId = TEXT("OneOnOneChipShotD6");
		Roll.PostRoutePurpose = EMatchPlayCurrentAttackPostRouteRollPurpose
			::OneOnOneChipShotAttack;
		Roll.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
		Roll.OwningSide = EInitialTurnOrderPlayer::PlayerA;
		Roll.bResolved = bResolved;
		Roll.RawD6 = bResolved ? RawD6 : 0;
		View.ResolutionFacts.Rolls.Add(Roll);
		View.ResolutionFacts.bHasPendingRoll = !bResolved;
		View.ResolutionFacts.NextPendingRollSequenceIndex =
			bResolved ? INDEX_NONE : 2;
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = TEXT("ThroughBall.OneOnOne.ChipShot.Outcome");
		Decision.Semantics = EMatchPlayResolutionRollSemantics::OutcomeDecision;
		Decision.RollSequenceIndices = { 2 };
		Decision.bResolved = bResolved;
		Decision.Outcome = bResolved
			? Outcome : EMatchPlayResolutionDecisionOutcome::None;
		View.ResolutionFacts.Decisions.Add(Decision);
		View.InteractionCategory = bResolved
			? EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal
			: EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack;
		View.bThroughBallOneOnOneChipShotAttackRollPending = !bResolved;
		View.bTerminalPendingAdvance = bResolved;
		View.ContinueActionLabel = bResolved ? TEXT("下一回合")
			: TEXT("进攻方掷挑射点数");
		return View;
	}

	FFMCodexLocalMatchInteractionView MakeDirectView(
		const bool bAttackResolved,
		const bool bDefenseResolved,
		const EMatchPlayResolutionDecisionOutcome Outcome =
			EMatchPlayResolutionDecisionOutcome::None)
	{
		using ETermKind = EMatchPlayResolutionFormulaTermKind;
		using ERollPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
		FFMCodexLocalMatchInteractionView View = MakeView();
		View.ResolutionFacts = MakeFacts(
			EMatchPlayThroughBallActualBranch::BehindDefense, 4);
		AddOneOnOneParticipants(View.ResolutionFacts);
		View.OneOnOneChoiceLabel = TEXT("Direct Shot");
		const bool bDefenderWins = Outcome
			== EMatchPlayResolutionDecisionOutcome::Miss;
		const int32 AttackRawD6 = bDefenderWins ? 1 : 6;
		const int32 DefenseRawD6 = bDefenderWins ? 6 : 2;
		auto AddRoll = [&View](const int32 Index, const ERollPurpose Purpose,
			const EInitialTurnOrderPlayer Side, const bool bResolved,
			const int32 RawD6)
		{
			FMatchPlayResolutionRollFact Roll;
			Roll.SequenceIndex = Index;
			Roll.PostRoutePurpose = Purpose;
			Roll.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
			Roll.OwningSide = Side;
			Roll.bResolved = bResolved;
			Roll.RawD6 = bResolved ? RawD6 : 0;
			View.ResolutionFacts.Rolls.Add(Roll);
		};
		AddRoll(3, ERollPurpose::OneOnOneDirectShotAttack,
			EInitialTurnOrderPlayer::PlayerA, bAttackResolved, AttackRawD6);
		AddRoll(4, ERollPurpose::OneOnOneDirectShotDefense,
			EInitialTurnOrderPlayer::PlayerB, bDefenseResolved, DefenseRawD6);
		View.ResolutionFacts.bHasPendingRoll =
			!bAttackResolved || !bDefenseResolved;
		View.ResolutionFacts.NextPendingRollSequenceIndex = !bAttackResolved
			? 3 : !bDefenseResolved ? 4 : INDEX_NONE;
		auto Attribute = [](const FName Id,
			const EMatchPlayResolutionParticipantRole Role,
			const EInitialTurnOrderPlayer Side, const FName CardId,
			const EMatchPlayResolutionFormulaAttribute AttributeId,
			const float Value)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.TermId = Id;
			Term.Kind = ETermKind::Attribute;
			Term.ParticipantRole = Role;
			Term.Side = Side;
			Term.CardId = CardId;
			Term.Attribute = AttributeId;
			Term.SourceValue = Value;
			Term.Contribution = Value;
			Term.bResolved = true;
			return Term;
		};
		auto RollTerm = [](const int32 Index, const bool bResolved,
			const int32 RawD6)
		{
			FMatchPlayResolutionFormulaTermFact Term;
			Term.Kind = ETermKind::RawRoll;
			Term.RollSequenceIndex = Index;
			Term.bResolved = bResolved;
			Term.SourceValue = bResolved ? RawD6 : 0;
			Term.Contribution = Term.SourceValue;
			return Term;
		};
		FMatchPlayResolutionFormulaContestFact Contest;
		Contest.ContestId = TEXT("ThroughBall.OneOnOne.DirectShot");
		Contest.FormulaType = EFormulaType::Finishing;
		Contest.Application = bAttackResolved && bDefenseResolved
			? EMatchPlayResolutionFormulaApplication::Applied
			: EMatchPlayResolutionFormulaApplication::Pending;
		Contest.AttackRow.Side = EInitialTurnOrderPlayer::PlayerA;
		Contest.AttackRow.Terms.Add(Attribute(TEXT("Runner.Shooting"),
			EMatchPlayResolutionParticipantRole::Runner,
			EInitialTurnOrderPlayer::PlayerA,
			TEXT("Fixture.OneOnOne.Runner"),
			EMatchPlayResolutionFormulaAttribute::Shooting, 8.0f));
		FMatchPlayResolutionFormulaTermFact Fixed;
		Fixed.TermId = TEXT("Attack.FixedBonus");
		Fixed.Kind = ETermKind::FixedModifier;
		Fixed.SourceValue = 1.0f;
		Fixed.Contribution = 1.0f;
		Fixed.bResolved = true;
		Contest.AttackRow.Terms.Add(Fixed);
		FMatchPlayResolutionFormulaTermFact Tactical;
		Tactical.TermId = TEXT("TacticalPlayer.Advantage.Attack");
		Tactical.Kind = ETermKind::TacticalPlayerAdvantage;
		Tactical.Side = EInitialTurnOrderPlayer::PlayerA;
		Tactical.SourceValue = 1.0f;
		Tactical.Contribution = 1.0f;
		Tactical.bResolved = true;
		Contest.AttackRow.Terms.Add(Tactical);
		View.ResolutionFacts.AttackerTacticalPlayerModifier = 1.0f;
		Contest.AttackRow.Terms.Add(RollTerm(3, bAttackResolved, AttackRawD6));
		Contest.AttackRow.bKnownNonRollSubtotalResolved = true;
		Contest.AttackRow.KnownNonRollSubtotal = 10.0f;
		Contest.AttackRow.bFinalValueResolved = bAttackResolved;
		Contest.AttackRow.FinalValue = bAttackResolved
			? 10.0f + AttackRawD6 : 0.0f;
		Contest.DefenseRow.Side = EInitialTurnOrderPlayer::PlayerB;
		Contest.DefenseRow.Terms.Add(Attribute(TEXT("Goalkeeper.OneOnOneBase"),
			EMatchPlayResolutionParticipantRole::Goalkeeper,
			EInitialTurnOrderPlayer::PlayerB,
			TEXT("Fixture.OneOnOne.Goalkeeper"),
			EMatchPlayResolutionFormulaAttribute::GoalkeeperOneOnOne, 7.0f));
		Contest.DefenseRow.Terms.Add(RollTerm(4, bDefenseResolved, DefenseRawD6));
		Contest.DefenseRow.bKnownNonRollSubtotalResolved = true;
		Contest.DefenseRow.KnownNonRollSubtotal = 7.0f;
		Contest.DefenseRow.bFinalValueResolved = bDefenseResolved;
		Contest.DefenseRow.FinalValue = bDefenseResolved
			? 7.0f + DefenseRawD6 : 0.0f;
		Contest.bHasResolvedFormula = bAttackResolved && bDefenseResolved;
		if (Contest.bHasResolvedFormula)
		{
			Contest.ResolvedResult.WinReason=EFormulaWinReason::FastSuppression;
			Contest.ResolvedResult.Winner = Outcome
				== EMatchPlayResolutionDecisionOutcome::Goal
					? EFormulaWinner::Attacker : EFormulaWinner::Defender;
			Contest.ResolvedResult.bAttackEnded = true;
		}
		View.ResolutionFacts.FormulaContests.Add(Contest);
		FMatchPlayResolutionDecisionFact Decision;
		Decision.DecisionId = TEXT(
			"ThroughBall.OneOnOne.DirectShot.Outcome");
		Decision.Semantics = EMatchPlayResolutionRollSemantics::ArithmeticContest;
		Decision.RollSequenceIndices = { 3, 4 };
		Decision.bResolved = Outcome
			!= EMatchPlayResolutionDecisionOutcome::None;
		Decision.Outcome = Outcome;
		View.ResolutionFacts.Decisions.Add(Decision);
		AddOneOnOneRoster(View);
		if (!bAttackResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotAttack;
			View.bThroughBallOneOnOneDirectShotAttackRollPending = true;
			View.ContinueActionLabel = TEXT("进攻方掷单刀射门点数");
		}
		else if (!bDefenseResolved)
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::RollThroughBallOneOnOneDirectShotDefense;
			View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
			View.bThroughBallOneOnOneDirectShotDefenseRollPending = true;
			View.ContinueActionLabel = TEXT("防守方掷单刀防守点数");
		}
		else
		{
			View.InteractionCategory = EFMCodexLocalMatchInteractionCategory
				::AdvanceAfterTerminal;
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
			EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute;
		View.ContinueActionLabel = TEXT("判定直塞路线");
		View.SelectedCarrierCardId=TEXT("Fixture.Context.Carrier");
		View.SelectedRunnerCardId=TEXT("Fixture.Context.Runner");
		auto& Region=View.PitchRegions.AddDefaulted_GetRef();
		Region.NeutralSide=EMatchPlayNeutralSlotSide::NearPlayerA;
		for (bool bCarrier:{true,false})
		{
			auto& Slot=Region.Slots.AddDefaulted_GetRef(); Slot.bOccupied=true;
			Slot.Card.CardId=bCarrier?View.SelectedCarrierCardId:View.SelectedRunnerCardId;
			Slot.Card.Side=EInitialTurnOrderPlayer::PlayerA;
			Slot.Card.DisplayLabel=bCarrier?TEXT("厄德高"):TEXT("哈兰德");
		}
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

	void CheckTheater(FAutomationTestBase& Test, UFMCodexLocalMatchScreenWidget& Screen, bool bOutcome=false)
	{
		Test.TestTrue(TEXT("ThroughBall has one modern outer surface and denies Full Card"),
			Screen.GetWidgetFromName(TEXT("ResolutionTheater"))->GetVisibility()!=ESlateVisibility::Collapsed
			&& Screen.GetThroughBallResolutionSurface()->GetVisibility()==ESlateVisibility::Collapsed
			&& !Screen.IsLegacyResolutionOverlayVisible() && !Screen.IsDetailOverlayVisible());
		if (bOutcome) Test.TestTrue(TEXT("Shared Outcome is painted in Theater"),
			Screen.GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()!=ESlateVisibility::Collapsed
			&& !CastChecked<URichTextBlock>(Screen.GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().IsEmpty());
	}
	void CheckFormula(FAutomationTestBase& Test, UFMCodexLocalMatchScreenWidget& Screen)
	{
		CheckTheater(Test,Screen);
		const auto& P=Screen.GetThroughBallResolutionSurface()->GetPresentation().Formula;
		for (bool bAttack:{true,false})
		{
			const FString Prefix=bAttack?TEXT("TheaterAttack"):TEXT("TheaterDefense");
			const auto& Row=bAttack?P.AttackRow:P.DefenseRow;
			Test.TestEqual(TEXT("Projected Base uses production numeric grammar"),
				CastChecked<UTextBlock>(Screen.GetWidgetFromName(FName(*(Prefix+TEXT("Number")))))->GetText().ToString(), FText::AsNumber(Row.KnownNonRollSubtotal).ToString());
			Test.TestEqual(TEXT("Formula operands opt into TheaterInline"),
				CastChecked<UFMCodexRollReelWidget>(Screen.GetWidgetFromName(FName(*(Prefix+TEXT("Reel")))))->GetVisualVariant(),EFMCodexRollVisualVariant::TheaterInline);
			for (int32 I=0;I<Row.Participants.Num();++I)
			{
				const auto* Name=Cast<UTextBlock>(Screen.GetWidgetFromName(FName(*(Prefix+FString::Printf(TEXT("Name%d"),I)))));
				Test.TestNotNull(TEXT("Projected participant has a name slot"),Name);
				Test.TestEqual(TEXT("Theater shows only projected Formula participants"),
					Name?Name->GetText().ToString():FString(),Row.Participants[I].PlayerName);
			}
		}
	}
	void CheckMergedFormula(FAutomationTestBase& Test, const FFMCodexUMGMatchScreenViewModel& Preview,
		FFMCodexUMGMatchScreenViewModel Future)
	{
		const FName Contest=Preview.ThroughBallResolution.Formula.ContestId;
		for (const auto& Fact:Future.Resolution.FormulaFacts.Rolls)
		{
			if (Fact.bInitialRoute || !Fact.bResolved || Fact.Semantics!=EMatchPlayResolutionRollSemantics::ArithmeticContest) continue;
			auto& Event=Future.ResolvedRolls.AddDefaulted_GetRef();
			Event.Kind=Fact.OwningSide==Preview.ThroughBallResolution.Formula.AttackRow.Side?EFMCodexUMGCrossRollRevealKind::Attack:EFMCodexUMGCrossRollRevealKind::Defense;
			Event.ContestId=Contest; Event.AttackSequence=Future.Header.AttackSequence;
			Event.SequenceIndex=Fact.SequenceIndex; Event.OwnerSide=Fact.OwningSide; Event.RawD6=Fact.RawD6;
		}
		auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget(); Screen->RefreshFromPresentation(Preview); Screen->RefreshFromPresentation(Future);
		Screen->PauseInlineFormulaRevealTimerForTesting(); Screen->AdvanceInlineFormulaRevealForTesting(1.85f);
		const auto& P=Screen->GetThroughBallResolutionSurface()->GetPresentation().Formula;
		Test.TestTrue(TEXT("Merged terminal keeps defense and outcome hidden during attack hold"),
			P.bAttackRowActive && !P.DefenseRow.bFinalValueResolved && !P.bNarrativeAvailable
			&& P.DefenseRow.Terms.ContainsByPredicate([](const auto& Term){return Term.Kind==EFMCodexUMGInlineFormulaTermKind::RawRoll && !Term.bResolved;})
			&& Screen->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()==ESlateVisibility::Collapsed);
		const auto Phase=Screen->GetInlineFormulaRevealPhase(); Screen->RefreshFromPresentation(Future);
		Screen->PauseInlineFormulaRevealTimerForTesting();
		Test.TestEqual(TEXT("Merged repeated View does not replay current reveal"),Screen->GetInlineFormulaRevealPhase(),Phase);
		Screen->AdvanceInlineFormulaRevealForTesting(2.21f);
		Test.TestTrue(TEXT("Queued defense gets its own reveal after attack hold"),
			Screen->IsInlineFormulaRevealInputBlocked() && Screen->GetThroughBallResolutionSurface()->GetPresentation().Formula.bDefenseRowActive);
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
			&& PreRoute.ThroughBallResolution.PrimaryAction.Claims(
				PreRoute.Interaction.PrimaryAction)
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
	FFMCodexLocalMatchInteractionView ReadySelectionRoute = PreRouteView;
	ReadySelectionRoute.MajorPhase = EFMCodexLocalMatchMajorPhase::Selection;
	const FFMCodexUMGMatchScreenViewModel ReadySelectionPresentation =
		Build(ReadySelectionRoute);
	TestTrue(TEXT("ReadyForResolution route pending is already central-owned"),
		ReadySelectionPresentation.ThroughBallResolution.bVisible
			&& ReadySelectionPresentation.ThroughBallResolution.PrimaryAction.Claims(
				ReadySelectionPresentation.Interaction.PrimaryAction)
			&& ReadySelectionPresentation.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute);

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
			TEXT("身后球"), TEXT("身后球") },
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

	const FFMCodexLocalMatchInteractionView OneOnOneView = MakeBehindView(
		true, true, EMatchPlayResolutionDecisionOutcome::OneOnOneRequired,
		false, 41, 6, 1);
	const FFMCodexUMGMatchScreenViewModel OneOnOne = Build(OneOnOneView);
	TestTrue(TEXT("BehindDefense success exposes exactly two typed shot choices"),
		OneOnOne.ThroughBallResolution.Stage
			== EFMCodexUMGThroughBallStage::OneOnOneChoice
			&& OneOnOne.ThroughBallResolution.OneOnOneChoices.Num() == 2
			&& OneOnOne.ThroughBallResolution.StageLabel
				== TEXT("选择单刀方式")
			&& OneOnOne.ThroughBallResolution.ActionPromptLabel.IsEmpty()
			&& OneOnOne.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("OneOnOne compact choice help comes from localized presentation copy"),
		OneOnOne.Interaction.OneOnOneChoices.Num() == 2
			&& OneOnOne.Interaction.OneOnOneChoices[0].SecondaryLabel
				== FFMCodexPlayerUIPresentationText
					::ThroughBallDirectChoiceHint().ToString()
			&& OneOnOne.Interaction.OneOnOneChoices[1].SecondaryLabel
				== FFMCodexPlayerUIPresentationText
					::ThroughBallChipChoiceHint().ToString()
			&& OneOnOne.ThroughBallResolution.OneOnOneChoices[0]
				.SecondaryLabel == TEXT("（看射门、门将单刀）")
			&& OneOnOne.ThroughBallResolution.OneOnOneChoices[1]
				.SecondaryLabel == TEXT("（只看掷点）"));
	TestTrue(TEXT("Behind OneOnOne parent exclusively owns route context"),
		OneOnOne.ThroughBallResolution.RouteResultLabel
			== TEXT("路线掷点 4 → 判定为身后球")
			&& OneOnOne.ThroughBallResolution.Formula.bParentOwnsRouteContext);
	const FFMCodexUMGMatchScreenViewModel AntiOneOnOne = Build(MakeAntiView(
		true, EMatchPlayResolutionDecisionOutcome::OneOnOneRequired, 6));
	TestTrue(TEXT("Anti success shares OneOnOne layout and parent route ownership"),
		AntiOneOnOne.ThroughBallResolution.Stage
			== EFMCodexUMGThroughBallStage::OneOnOneChoice
			&& AntiOneOnOne.ThroughBallResolution.OneOnOneChoices.Num() == 2
			&& AntiOneOnOne.ThroughBallResolution.RouteResultLabel
				== TEXT("路线掷点 6 → 判定为反越位")
			&& AntiOneOnOne.ThroughBallResolution.Formula
				.bParentOwnsRouteContext);
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
		// This semantic/layout check starts from the already-settled snapshot;
		// the dedicated reveal tests cover route-to-choice disclosure timing.
		Screen = NewObject<UFMCodexLocalMatchScreenWidget>(
			GetTransientPackage());
		Screen->TakeWidget();
		Screen->RefreshFromPresentation(OneOnOne);
		TestTrue(TEXT("Normal ThroughBall hides generic resolution debug surface"),
			Screen->GetThroughBallResolutionSurface() != nullptr
				&& Screen->GetThroughBallResolutionSurface()
					->GetPresentation().bVisible
				&& Screen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Collapsed
				&& !Screen->IsLegacyResolutionOverlayVisible());
		UFMCodexThroughBallResolutionSurfaceWidget* ThroughBallSurface =
			Screen->GetThroughBallResolutionSurface();
		const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>& CentralChoices =
			ThroughBallSurface->GetOneOnOneChoiceWidgets();
		UFMCodexInteractionOptionWidget* DirectOption =
			CentralChoices.IsValidIndex(0) ? CentralChoices[0] : nullptr;
		UFMCodexInteractionOptionWidget* ChipOption =
			CentralChoices.IsValidIndex(1) ? CentralChoices[1] : nullptr;
		if (DirectOption != nullptr) DirectOption->TakeWidget();
		if (ChipOption != nullptr) ChipOption->TakeWidget();
		UHorizontalBox* ShotRow = Cast<UHorizontalBox>(
			ThroughBallSurface->GetWidgetFromName(
				TEXT("ThroughBallOneOnOneChoiceRow")));
		USizeBox* DirectBounds = DirectOption != nullptr
			? Cast<USizeBox>(DirectOption->GetWidgetFromName(
				TEXT("InteractionOptionBounds"))) : nullptr;
		USizeBox* ChipBounds = ChipOption != nullptr
			? Cast<USizeBox>(ChipOption->GetWidgetFromName(
				TEXT("InteractionOptionBounds"))) : nullptr;
		UTextBlock* DirectLabel = DirectOption != nullptr
			? Cast<UTextBlock>(DirectOption->GetWidgetFromName(
				TEXT("InteractionOptionLabel"))) : nullptr;
		UTextBlock* ChipLabel = ChipOption != nullptr
			? Cast<UTextBlock>(ChipOption->GetWidgetFromName(
				TEXT("InteractionOptionLabel"))) : nullptr;
		UTextBlock* DirectSecondary = DirectOption != nullptr
			? Cast<UTextBlock>(DirectOption->GetWidgetFromName(
				TEXT("InteractionOptionSecondaryLabel"))) : nullptr;
		UTextBlock* ChipSecondary = ChipOption != nullptr
			? Cast<UTextBlock>(ChipOption->GetWidgetFromName(
				TEXT("InteractionOptionSecondaryLabel"))) : nullptr;
		UButton* DirectButton = DirectOption != nullptr
			? Cast<UButton>(DirectOption->GetWidgetFromName(
				TEXT("InteractionOptionButton"))) : nullptr;
		UButton* ChipButton = ChipOption != nullptr
			? Cast<UButton>(ChipOption->GetWidgetFromName(
				TEXT("InteractionOptionButton"))) : nullptr;
		UTextBlock* ParentRouteContext = Cast<UTextBlock>(
			ThroughBallSurface->GetWidgetFromName(
				TEXT("ThroughBallInitialRouteResult")));
		UTextBlock* NestedRouteContext = Cast<UTextBlock>(
			ThroughBallSurface->GetFormulaSurface()->GetWidgetFromName(
				TEXT("InlineFormulaRouteResult")));
		Screen->ForceLayoutPrepass();
		TestEqual(TEXT("Central OneOnOne renders exactly two choices"),
			CentralChoices.Num(), 2);
		TestNotNull(TEXT("Central Direct choice exists"), DirectOption);
		TestNotNull(TEXT("Central Chip choice exists"), ChipOption);
		TestNotNull(TEXT("Central OneOnOne horizontal row exists"), ShotRow);
		TestTrue(TEXT("OneOnOne choices are ordered on one horizontal row"),
			DirectOption != nullptr && ChipOption != nullptr && ShotRow != nullptr
				&& ShotRow->GetChildrenCount() == 2
				&& ShotRow->GetChildAt(0) == DirectOption
				&& ShotRow->GetChildAt(1) == ChipOption);
		TestTrue(TEXT("Central OneOnOne labels are complete"),
			DirectOption != nullptr && ChipOption != nullptr
				&& DirectOption->GetLabel() == TEXT("直接射门")
				&& DirectOption->GetSecondaryLabel()
					== TEXT("（看射门、门将单刀）")
				&& ChipOption->GetLabel() == TEXT("挑射")
				&& ChipOption->GetSecondaryLabel() == TEXT("（只看掷点）"));
		TestTrue(TEXT("Both OneOnOne choices have stable clickable geometry"),
			DirectBounds != nullptr && ChipBounds != nullptr
				&& DirectBounds->GetWidthOverride() >= 200.0f
				&& ChipBounds->GetWidthOverride()
					== DirectBounds->GetWidthOverride()
				&& DirectBounds->GetHeightOverride() >= 60.0f
				&& ChipBounds->GetHeightOverride()
					== DirectBounds->GetHeightOverride()
				&& DirectButton != nullptr && DirectButton->GetIsEnabled()
				&& ChipButton != nullptr && ChipButton->GetIsEnabled());
		TestTrue(TEXT("OneOnOne primary and secondary labels are readable two-line copy"),
			DirectBounds != nullptr && DirectLabel != nullptr
				&& !DirectLabel->GetAutoWrapText()
				&& DirectLabel->GetTextOverflowPolicy()
					== ETextOverflowPolicy::Clip
				&& DirectSecondary != nullptr
				&& DirectSecondary->GetVisibility()
					== ESlateVisibility::HitTestInvisible
				&& !DirectSecondary->GetAutoWrapText()
				&& DirectSecondary->GetText().ToString()
					== TEXT("（看射门、门将单刀）")
				&& DirectSecondary->GetDesiredSize().X
					< DirectBounds->GetWidthOverride()
				&& DirectSecondary->GetFont().Size < DirectLabel->GetFont().Size
				&& ChipBounds != nullptr && ChipLabel != nullptr
				&& !ChipLabel->GetAutoWrapText()
				&& ChipLabel->GetTextOverflowPolicy()
					== ETextOverflowPolicy::Clip
				&& ChipSecondary != nullptr
				&& ChipSecondary->GetVisibility()
					== ESlateVisibility::HitTestInvisible
				&& !ChipSecondary->GetAutoWrapText()
				&& ChipSecondary->GetText().ToString() == TEXT("（只看掷点）")
				&& ChipSecondary->GetDesiredSize().X
					< ChipBounds->GetWidthOverride()
				&& ChipSecondary->GetFont().Size < ChipLabel->GetFont().Size);
		TestTrue(TEXT("Deprecated OneOnOne detail reserve is absent"),
			ThroughBallSurface->GetWidgetFromName(
				TEXT("ThroughBallOneOnOneDetailBounds")) == nullptr
				&& ThroughBallSurface->GetWidgetFromName(
					TEXT("ThroughBallOneOnOneDetailPanel")) == nullptr);
		TestTrue(TEXT("Parent route context renders once and nested copy collapses"),
			ParentRouteContext != nullptr
				&& ParentRouteContext->GetVisibility()
					== ESlateVisibility::SelfHitTestInvisible
				&& NestedRouteContext != nullptr
				&& NestedRouteContext->GetVisibility()
					== ESlateVisibility::Collapsed);
		ThroughBallSurface->ResetOneOnOneDispatchForTesting();
		if (DirectButton != nullptr) DirectButton->OnHovered.Broadcast();
		Screen->RefreshFromPresentation(OneOnOne);
		const auto& DirectRefreshChoices =
			ThroughBallSurface->GetOneOnOneChoiceWidgets();
		TestTrue(TEXT("Direct hover has no detail consumer or gameplay dispatch"),
			DirectRefreshChoices.IsValidIndex(1)
				&& DirectRefreshChoices[0] == DirectOption
				&& DirectRefreshChoices[1] == ChipOption
				&& Screen->GetTacticalDetailPanel()->GetVisibility()
					== ESlateVisibility::Collapsed
				&& ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 0);
		if (ChipButton != nullptr) ChipButton->OnHovered.Broadcast();
		Screen->RefreshFromPresentation(OneOnOne);
		const auto& ChipRefreshChoices =
			ThroughBallSurface->GetOneOnOneChoiceWidgets();
		TestTrue(TEXT("Chip hover has no detail consumer or gameplay dispatch"),
			ChipRefreshChoices.IsValidIndex(1)
				&& ChipRefreshChoices[0] == DirectOption
				&& ChipRefreshChoices[1] == ChipOption
				&& Screen->GetTacticalDetailPanel()->GetVisibility()
					== ESlateVisibility::Collapsed
				&& ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 0);

		ThroughBallSurface->ResetOneOnOneDispatchForTesting();
		if (DirectButton != nullptr) DirectButton->OnHovered.Broadcast();
		if (DirectButton != nullptr) DirectButton->OnClicked.Broadcast();
		TestTrue(TEXT("Direct remains clickable after hover and dispatches once"),
			ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 1
				&& ThroughBallSurface->GetLastOneOnOneDispatchForTesting()
					== EFMCodexUMGOneOnOneChoice::DirectShot);
		if (DirectButton != nullptr) DirectButton->OnUnhovered.Broadcast();
		ThroughBallSurface->ResetOneOnOneDispatchForTesting();
		if (ChipButton != nullptr) ChipButton->OnHovered.Broadcast();
		if (ChipButton != nullptr) ChipButton->OnClicked.Broadcast();
		TestTrue(TEXT("Chip remains clickable after hover and dispatches once"),
			ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 1
				&& ThroughBallSurface->GetLastOneOnOneDispatchForTesting()
					== EFMCodexUMGOneOnOneChoice::ChipShot);

		Screen->RefreshFromPresentation(AntiOneOnOne);
		ThroughBallSurface = Screen->GetThroughBallResolutionSurface();
		const auto& AntiChoices = ThroughBallSurface->GetOneOnOneChoiceWidgets();
		UFMCodexInteractionOptionWidget* AntiDirect =
			AntiChoices.IsValidIndex(0) ? AntiChoices[0] : nullptr;
		UFMCodexInteractionOptionWidget* AntiChip =
			AntiChoices.IsValidIndex(1) ? AntiChoices[1] : nullptr;
		if (AntiDirect != nullptr) AntiDirect->TakeWidget();
		if (AntiChip != nullptr) AntiChip->TakeWidget();
		UButton* AntiDirectButton = AntiDirect != nullptr
			? Cast<UButton>(AntiDirect->GetWidgetFromName(
				TEXT("InteractionOptionButton"))) : nullptr;
		UButton* AntiChipButton = AntiChip != nullptr
			? Cast<UButton>(AntiChip->GetWidgetFromName(
				TEXT("InteractionOptionButton"))) : nullptr;
		ThroughBallSurface->ResetOneOnOneDispatchForTesting();
		if (AntiDirectButton != nullptr) AntiDirectButton->OnHovered.Broadcast();
		if (AntiChipButton != nullptr) AntiChipButton->OnHovered.Broadcast();
		Screen->RefreshFromPresentation(AntiOneOnOne);
		TestTrue(TEXT("Anti source shares stable microcopy choices with no hover consumer"),
			AntiDirect != nullptr && AntiChip != nullptr
				&& ThroughBallSurface->GetOneOnOneChoiceWidgets().Num() == 2
				&& ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 0
				&& ThroughBallSurface->GetOneOnOneChoiceWidgets()[0] == AntiDirect
				&& ThroughBallSurface->GetOneOnOneChoiceWidgets()[1] == AntiChip
				&& AntiDirect->GetSecondaryLabel()
					== TEXT("（看射门、门将单刀）")
				&& AntiChip->GetSecondaryLabel() == TEXT("（只看掷点）")
				&& Screen->GetTacticalDetailPanel()->GetVisibility()
					== ESlateVisibility::Collapsed);
		if (AntiDirectButton != nullptr) AntiDirectButton->OnHovered.Broadcast();
		if (AntiDirectButton != nullptr) AntiDirectButton->OnClicked.Broadcast();
		if (AntiChipButton != nullptr) AntiChipButton->OnHovered.Broadcast();
		if (AntiChipButton != nullptr) AntiChipButton->OnClicked.Broadcast();
		TestTrue(TEXT("Anti source shares both typed clicks"),
			AntiDirectButton != nullptr && AntiChipButton != nullptr
				&& ThroughBallSurface->GetOneOnOneDispatchCountForTesting() == 2
				&& ThroughBallSurface->GetLastOneOnOneDispatchForTesting()
					== EFMCodexUMGOneOnOneChoice::ChipShot);
		const FFMCodexUMGMatchScreenViewModel RejectedRoute =
			Build(PreRouteView, true);
		Screen->RefreshFromPresentation(RejectedRoute);
		TestTrue(TEXT("Rejected central route recovers its lower action and diagnostic"),
			Screen->IsLegacyResolutionOverlayVisible()
				&& Screen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Visible
				&& RejectedRoute.Interaction.PrimaryAction.bAvailable
				&& !RejectedRoute.ThroughBallResolution.PrimaryAction.Claims(
					RejectedRoute.Interaction.PrimaryAction)
				&& Screen->GetThroughBallResolutionSurface()
					->GetOneOnOneChoiceWidgets().IsEmpty());
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
	TestTrue(TEXT("Independent pre-roll has no large rule chamber"),Screen->GetWidgetFromName(TEXT("TheaterEventRule"))==nullptr);
	TestNull(TEXT("Context card has no decorative die placeholder"),Screen->GetWidgetFromName(TEXT("TheaterEventPending")));
	TestTrue(TEXT("Compact die shares Cross route's dedicated action lane"),
		Screen->GetWidgetFromName(TEXT("TheaterTacticalEvent"))->GetParent()==Screen->GetWidgetFromName(TEXT("TheaterRoll"))->GetParent());
	TestEqual(TEXT("Pre-roll preserves empty die allocation beneath the CTA"),
		Screen->GetWidgetFromName(TEXT("TheaterEventReelHost"))->GetVisibility(),ESlateVisibility::Hidden);
	CheckTheater(*this,*Screen);
	TestEqual(TEXT("Route shows projected Carrier"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackName0")))->GetText().ToString(),FString(TEXT("厄德高")));
	TestEqual(TEXT("Route shows projected Runner"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackName1")))->GetText().ToString(),FString(TEXT("哈兰德")));
	TestTrue(TEXT("InitialRoute enters full Theater before any roll; old modal is absent"),
		Screen->GetWidgetFromName(TEXT("ResolutionTheater")) != nullptr
		&& Screen->GetWidgetFromName(TEXT("TheaterTacticalEvent"))->GetVisibility() == ESlateVisibility::SelfHitTestInvisible
		&& Screen->GetThroughBallResolutionSurface()->GetVisibility() == ESlateVisibility::Collapsed
		&& CastChecked<UButton>(Screen->GetWidgetFromName(TEXT("TheaterContinue")))->GetIsEnabled()
		&& Screen->GetWidgetFromName(TEXT("TheaterDuel"))->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
	TestEqual(TEXT("Route pre-roll gives the canonical readonly reference"),
		CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(),
		FString(TEXT("1–2：脚下球　｜　3–4：身后球　｜　5–6：反越位")));
	Screen->RefreshFromPresentation(Resolved);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexThroughBallResolutionSurfaceWidget* Surface =
		Screen->GetThroughBallResolutionSurface();
	UFMCodexRollReelWidget* Reel = Cast<UFMCodexRollReelWidget>(Screen->GetWidgetFromName(TEXT("TheaterEventReel")));
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
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !Screen->IsLegacyResolutionOverlayVisible());

	TestEqual(TEXT("InitialRoute opts into production CompactBox"), Reel->GetVisualVariant(), EFMCodexRollVisualVariant::CompactBox);
	TestTrue(TEXT("Cycling glyph is neutral"), Reel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor().B
		> Reel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor().R);
	Screen->AdvanceInlineFormulaRevealForTesting(0.92f);
	TestEqual(TEXT("Existing v2 enters continuous capture before Legacy would settle"),
		Screen->GetInlineFormulaRevealPhase(),
		EFMCodexUMGInlineFormulaRevealPhase::Settling);
	Screen->AdvanceInlineFormulaRevealForTesting(0.54f);
	TestEqual(TEXT("Authoritative landed glyph uses shared EED7A6"),
		Reel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor(), FLinearColor::FromSRGBColor(FColor(238,215,166)));
	TestTrue(TEXT("Authority raw 4 lands before BehindDefense is disclosed"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			&& Reel->GetPresentation().CenterValue == 4
			&& Reel->GetPresentation().bAuthoritativeValue
			&& Reel->GetPresentation().bStaticResult
			&& Surface->GetPresentation().RouteLabel == TEXT("身后球")
			&& Surface->GetPresentation().RouteResultLabel
				== TEXT("路线掷点 4 → 判定为身后球")
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

	// Real downstream models, including a coalesced accepted Anti success. The
	// old route-only fixture cannot detect embedded Formula or choice leakage.
	for (auto Future : {Build(MakeFeetView(false,false)), Build(MakeBehindView(false,false)),
		Build(MakeAntiView(false)), Build(MakeAntiView(true, EMatchPlayResolutionDecisionOutcome::OneOnOneRequired, 6))})
	{
		if (!Future.ThroughBallResolution.OneOnOneChoices.IsEmpty())
		{
			// A coalesced Network view carries the accepted chronological prefix.
			// Local builder-only fixtures omit it because Local observes each action.
			for (const auto& Fact : Future.Resolution.FormulaFacts.Rolls)
			{
				if (!Fact.bResolved) continue;
				auto& Event = Future.ResolvedRolls.AddDefaulted_GetRef();
				Event.Kind = Fact.bInitialRoute ? EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute : EFMCodexUMGCrossRollRevealKind::Attack;
				Event.ContestId = Fact.bInitialRoute ? TEXT("ThroughBall.Route") : TEXT("ThroughBall.AntiOffside");
				Event.AttackSequence = Future.Header.AttackSequence;
				Event.SequenceIndex = Fact.SequenceIndex; Event.OwnerSide = Fact.OwningSide; Event.RawD6 = Fact.RawD6;
			}
		}
		auto* Gated = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Gated->TakeWidget(); Gated->RefreshFromPresentation(Pending); Gated->RefreshFromPresentation(Future);
		Gated->PauseInlineFormulaRevealTimerForTesting();
		auto* Outer = Gated->GetThroughBallResolutionSurface();
		for (const float Delta : {0.f, .92f, .54f, 1.44f})
		{
			Gated->AdvanceInlineFormulaRevealForTesting(Delta);
			const auto Phase = Gated->GetInlineFormulaRevealPhase();
			Gated->RefreshFromPresentation(Future); Gated->PauseInlineFormulaRevealTimerForTesting();
			TestEqual(TEXT("Repeated route refresh never restarts its phase"), Gated->GetInlineFormulaRevealPhase(), Phase);
			const auto& Shown = Outer->GetPresentation();
			TestTrue(TEXT("Full route Theater stays primary through coalesced future facts"),
				Outer->GetVisibility() == ESlateVisibility::Collapsed
				&& CastChecked<UTextBlock>(Gated->GetWidgetFromName(TEXT("TheaterSubtitle")))->GetText().ToString() == TEXT("路线判定")
				&& !CastChecked<UTextBlock>(Gated->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString().Contains(TEXT("形成单刀"))
				&& Gated->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetVisibility() == ESlateVisibility::Collapsed);
			TestTrue(TEXT("The entire current route hold excludes downstream UI"),
				Shown.Stage == EFMCodexUMGThroughBallStage::InitialRoute && !Shown.Formula.bVisible
				&& !Shown.OutcomeRollHint.bVisible && Shown.OneOnOneChoices.IsEmpty()
				&& !Shown.bNarrativeAvailable && !Shown.PrimaryAction.bVisible
				&& Outer->GetFormulaSurface()->GetVisibility() == ESlateVisibility::Collapsed
				&& Outer->GetOneOnOneChoiceWidgets().IsEmpty());
		}
		Gated->AdvanceInlineFormulaRevealForTesting(.02f);
		TestTrue(TEXT("Only completed route hold releases the next event"), Outer->GetPresentation().Stage != EFMCodexUMGThroughBallStage::InitialRoute);
		if (Future.ThroughBallResolution.Route == EFMCodexUMGThroughBallRoute::AntiOffside)
		{
			TestTrue(TEXT("Anti now owns its canonical condition, not OneOnOne choices"),
				Outer->GetPresentation().Stage == EFMCodexUMGThroughBallStage::AntiOffsideCheck
				&& Outer->GetPresentation().OutcomeRollHint.bVisible && Outer->GetOneOnOneChoiceWidgets().IsEmpty());
			TestEqual(TEXT("Second event also selects CompactBox"), Outer->GetRollReelWidget()->GetVisualVariant(), EFMCodexRollVisualVariant::CompactBox);
			TestEqual(TEXT("Second event stays in Theater with its own subtitle"),
				CastChecked<UTextBlock>(Gated->GetWidgetFromName(TEXT("TheaterSubtitle")))->GetText().ToString(), FString(TEXT("反越位判定")));
			if (!Future.ThroughBallResolution.OneOnOneChoices.IsEmpty())
			{
				TestTrue(TEXT("Coalesced second event has its own reveal lifetime"), Gated->IsInlineFormulaRevealInputBlocked());
				Gated->AdvanceInlineFormulaRevealForTesting(4.1f);
				TestEqual(TEXT("Only the second completed hold releases both original choices"), Outer->GetOneOnOneChoiceWidgets().Num(), 2);
				TestEqual(TEXT("OneOnOne retains Theater ownership"), Outer->GetVisibility(), ESlateVisibility::Collapsed);
			}
		}
		else
		{
			TestEqual(TEXT("Formula handoff retains its existing contest"), Outer->GetPresentation().Formula.ContestId, Future.ThroughBallResolution.Formula.ContestId);
			TestEqual(TEXT("Production Formula reel is TheaterInline"), CastChecked<UFMCodexRollReelWidget>(Gated->GetWidgetFromName(TEXT("TheaterAttackReel")))->GetVisualVariant(), EFMCodexRollVisualVariant::TheaterInline);
			TestEqual(TEXT("Formula retains Theater ownership"), Outer->GetVisibility(), ESlateVisibility::Collapsed);
		}
	}
	for (const auto& Other : {Build(MakeFeetView(false,false)), Build(MakeBehindView(false,false)),
		Build(MakeChipView(false)), Build(MakeDirectView(false,false)),
		Build(MakeAntiView(true, EMatchPlayResolutionDecisionOutcome::Offside, 1)),
		Build(MakeAntiView(true, EMatchPlayResolutionDecisionOutcome::OneOnOneRequired, 6))})
		TestTrue(TEXT("Explicit ThroughBall stages retain full Theater continuity"),
			FMCodexResolutionTheaterPrototype::WantsTheater(Other,Other.InlineFormula));
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
				== TEXT("防守 5.5")
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
			&& Preview.ThroughBallResolution.Formula.PrimaryAction.Claims(
				Preview.Interaction.PrimaryAction)
			&& Preview.Interaction.bCanContinue
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
				== TEXT("路线掷点 2 → 判定为脚下球")
			&& AttackComplete.ThroughBallResolution.Formula.PrimaryAction.Claims(
				AttackComplete.Interaction.PrimaryAction));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Feet production screen can be constructed"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Preview);
	CheckFormula(*this,*Screen);
	UFMCodexThroughBallResolutionSurfaceWidget* Surface =
		Screen->GetThroughBallResolutionSurface();
	UFMCodexInlineResolutionFormulaSurfaceWidget* FormulaSurface =
		Surface != nullptr ? Surface->GetFormulaSurface() : nullptr;
	TestTrue(TEXT("Feet owns the central shared formula CTA without lower duplicate"),
		Surface != nullptr && FormulaSurface != nullptr
			&& FormulaSurface->GetPresentation().bVisible
			&& FormulaSurface->GetPresentation().bCanContinue
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
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
				== TEXT("防守方掷点")
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

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
	TestTrue(TEXT("Feet terminal slot claims the unchanged typed NextRound"),
		Terminal.ThroughBallResolution.Formula.PrimaryAction.Claims(
			Terminal.Interaction.PrimaryAction)
			&& Terminal.Interaction.PrimaryAction.bAvailable
			&& Terminal.Interaction.PrimaryAction.Category
				== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal);
	Screen->RefreshFromPresentation(Terminal);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Accepted defense roll gates terminal result and NextRound"),
		Screen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& FormulaSurface->GetPresentation().bDiceRevealVisible
			&& !FormulaSurface->GetPresentation().bCanContinue
			&& FormulaSurface->GetPresentation().ResultTitle.IsEmpty()
			&& FormulaSurface->GetPresentation().ResultSubtitle.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(1.30f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.16f);
	Screen->AdvanceInlineFormulaRevealForTesting(0.18f);
	TestTrue(TEXT("Defense formula discloses before the concise result"),
		FormulaSurface->GetPresentation().DefenseRow.bFinalValueResolved
			&& FMath::IsNearlyEqual(
				FormulaSurface->GetPresentation().DefenseRow.FinalValue, 8.5f)
			&& FormulaSurface->GetPresentation().ResultTitle.IsEmpty()
			&& FormulaSurface->GetPresentation().ResultSubtitle.IsEmpty()
			&& !FormulaSurface->GetPresentation().bCanContinue);
	Screen->AdvanceInlineFormulaRevealForTesting(0.20f);
	TestTrue(TEXT("Authority winner maps to Chinese result during hold"),
		FormulaSurface->GetPresentation().StatusLabel
			== TEXT("脚下球 · 进球")
			&& FormulaSurface->GetPresentation().ResultTitle == TEXT("进球")
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
	CheckFormula(*this,*TerminalReconstructed); CheckTheater(*this,*TerminalReconstructed,true);
	TestTrue(TEXT("Feet result explains authoritative Formula reason"),!Terminal.ThroughBallResolution.Formula.ResolutionReasonLabel.IsEmpty());
	CheckMergedFormula(*this,Preview,Terminal);
	const FFMCodexUMGInlineFormulaSurfaceViewModel& RebuiltFormula =
		TerminalReconstructed->GetThroughBallResolutionSurface()
			->GetFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Fresh TerminalPendingAdvance renders truth without replay"),
		TerminalReconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !TerminalReconstructed->IsInlineFormulaRevealInputBlocked()
			&& RebuiltFormula.AttackRow.bFinalValueResolved
			&& RebuiltFormula.DefenseRow.bFinalValueResolved
			&& RebuiltFormula.StatusLabel == TEXT("脚下球 · 进球")
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
				== TEXT("厄德高直塞被萨利巴抢断。")
			&& DefenderNarrative.ResultTitle == TEXT("防守成功")
			&& DefenderNarrative.ResultSubtitle
				== TEXT("脚下球 · 防守成功"));

	bool bFeetMarkerObserved = false;
	bool bFeetHelperObserved = false;
	for (int64 Sequence = 1; Sequence <= 32; ++Sequence)
	{
		FFMCodexLocalMatchInteractionView BothDefenders = MakeFeetView(
			true, true, true, EFormulaWinner::Defender);
		BothDefenders.ResolutionFacts.AttackSequence = Sequence;
		BothDefenders.ResolutionFacts.Participants.Add({
			EMatchPlayResolutionParticipantRole::Helper,
			EInitialTurnOrderPlayer::PlayerB, TEXT("Fixture.Feet.Helper") });
		FFMCodexLocalMatchCardView HelperCard;
		HelperCard.Side = EInitialTurnOrderPlayer::PlayerB;
		HelperCard.CardId = TEXT("Fixture.Feet.Helper");
		HelperCard.DisplayLabel = TEXT("赖斯");
		BothDefenders.PlayerBCardRoster.Add(HelperCard);
		const FFMCodexUMGInlineFormulaSurfaceViewModel Candidate =
			Build(BothDefenders).ThroughBallResolution.Formula;
		bFeetMarkerObserved |= Candidate.DefensiveNarrativePerformer
			== EFMCodexUMGCrossDefensiveNarrativePerformer::Marker
			&& Candidate.NarrativeHeadline
				== TEXT("厄德高直塞被萨利巴抢断。");
		bFeetHelperObserved |= Candidate.DefensiveNarrativePerformer
			== EFMCodexUMGCrossDefensiveNarrativePerformer::Helper
			&& Candidate.NarrativeHeadline
				== TEXT("哈兰德前插被赖斯拦截。");
	}
	TestTrue(TEXT("Feet production migration uses stable Marker/Helper choice"),
		bFeetMarkerObserved && bFeetHelperObserved);

	FFMCodexLocalMatchInteractionView GoalkeeperOnly = MakeFeetView(
		true, true, true, EFormulaWinner::Defender);
	GoalkeeperOnly.ResolutionFacts.Participants.RemoveAll(
		[](const FMatchPlayResolutionParticipantFact& Participant)
		{
			return Participant.Role
				== EMatchPlayResolutionParticipantRole::Marker;
		});
	GoalkeeperOnly.ResolutionFacts.Participants.Add({
		EMatchPlayResolutionParticipantRole::Goalkeeper,
		EInitialTurnOrderPlayer::PlayerB, TEXT("Fixture.Feet.Goalkeeper") });
	FFMCodexLocalMatchCardView GoalkeeperCard;
	GoalkeeperCard.Side = EInitialTurnOrderPlayer::PlayerB;
	GoalkeeperCard.CardId = TEXT("Fixture.Feet.Goalkeeper");
	GoalkeeperCard.DisplayLabel = TEXT("阿利松");
	GoalkeeperOnly.PlayerBCardRoster.Add(GoalkeeperCard);
	const FFMCodexUMGInlineFormulaSurfaceViewModel GoalkeeperFallback =
		Build(GoalkeeperOnly).ThroughBallResolution.Formula;
	TestTrue(TEXT("Feet aggregate defense never promotes GK into the narrative"),
		GoalkeeperFallback.NarrativeHeadline == TEXT("直塞被防守方化解。")
			&& !GoalkeeperFallback.NarrativeHeadline.Contains(TEXT("阿利松"))
			&& !GoalkeeperFallback.NarrativeHeadline.Contains(TEXT("扑")));

	FFMCodexLocalMatchInteractionView MissingNarrativeFacts = MakeFeetView(
		true, true, true, EFormulaWinner::Attacker);
	MissingNarrativeFacts.ResolutionFacts.Participants.Reset();
	const FFMCodexUMGInlineFormulaSurfaceViewModel FallbackNarrative =
		Build(MissingNarrativeFacts).ThroughBallResolution.Formula;
	TestTrue(TEXT("Feet terminal narrative has a concise insufficient-facts fallback"),
		FallbackNarrative.bNarrativeAvailable
			&& FallbackNarrative.NarrativeHeadline
				== TEXT("直塞形成进球！"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionTacticalPlayerFormulaContractTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.TacticalPlayerFormulaContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionTacticalPlayerFormulaContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexThroughBallProductionPresentationTests;
	(void)Parameters;
	auto FindTacticalTerm = [](
		const FFMCodexUMGInlineFormulaRowViewModel& Row)
	{
		return Row.Terms.FindByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.DisplayLabel.StartsWith(TEXT("战术球员 "));
			});
	};

	const FFMCodexUMGInlineFormulaSurfaceViewModel Zero =
		Build(MakeFeetView(false, false)).ThroughBallResolution.Formula;
	TestTrue(TEXT("Authoritative +0 remains omitted from both Feet rows"),
		FindTacticalTerm(Zero.AttackRow) == nullptr
			&& FindTacticalTerm(Zero.DefenseRow) == nullptr);

	FFMCodexLocalMatchInteractionView PlusOneView = MakeFeetView(false, false);
	PlusOneView.ResolutionFacts = MakeFeetFormulaFacts(
		false, false, EFormulaWinner::None, 1.0f, 0.0f);
	PlusOneView.ResolutionFacts.AttackerTacticalPlayerCount = 4;
	PlusOneView.ResolutionFacts.DefenderTacticalPlayerCount = 2;
	const FFMCodexUMGInlineFormulaSurfaceViewModel PlusOne =
		Build(PlusOneView).ThroughBallResolution.Formula;
	const FFMCodexUMGInlineFormulaTermViewModel* PlusOneTerm =
		FindTacticalTerm(PlusOne.AttackRow);
	TestTrue(TEXT("Feet projects authoritative Tactical Player +1 as its own term"),
		PlusOneTerm != nullptr
			&& PlusOneTerm->DisplayLabel == TEXT("战术球员 +1")
			&& FMath::IsNearlyEqual(PlusOneTerm->Contribution, 1.0f)
			&& FMath::IsNearlyEqual(
				PlusOne.AttackRow.KnownNonRollSubtotal, 5.5f)
			&& PlusOne.TacticalPlayerSummaryLabel.IsEmpty());

	FFMCodexLocalMatchInteractionView PlusTwoView = MakeFeetView(false, false);
	PlusTwoView.ResolutionFacts = MakeFeetFormulaFacts(
		false, false, EFormulaWinner::None, 2.0f, 0.0f);
	PlusTwoView.ResolutionFacts.AttackerTacticalPlayerCount = 5;
	PlusTwoView.ResolutionFacts.DefenderTacticalPlayerCount = 2;
	const FFMCodexUMGInlineFormulaSurfaceViewModel PlusTwo =
		Build(PlusTwoView).ThroughBallResolution.Formula;
	const FFMCodexUMGInlineFormulaTermViewModel* PlusTwoTerm =
		FindTacticalTerm(PlusTwo.AttackRow);
	TestTrue(TEXT("Feet projects authoritative Tactical Player +2 without count text"),
		PlusTwoTerm != nullptr
			&& PlusTwoTerm->DisplayLabel == TEXT("战术球员 +2")
			&& !PlusTwoTerm->DisplayLabel.Contains(TEXT("×"))
			&& FMath::IsNearlyEqual(PlusTwoTerm->Contribution, 2.0f)
			&& FMath::IsNearlyEqual(
				PlusTwo.AttackRow.KnownNonRollSubtotal, 6.5f));

	FFMCodexLocalMatchInteractionView BehindView = MakeBehindView(false, false);
	BehindView.ResolutionFacts.AttackerTacticalPlayerCount = 5;
	BehindView.ResolutionFacts.DefenderTacticalPlayerCount = 2;
	BehindView.ResolutionFacts.AttackerTacticalPlayerModifier = 2.0f;
	const FFMCodexUMGInlineFormulaSurfaceViewModel Behind =
		Build(BehindView).ThroughBallResolution.Formula;
	TestTrue(TEXT("BehindDefense Transition does not invent a finishing modifier"),
		Behind.bVisible
			&& FindTacticalTerm(Behind.AttackRow) == nullptr
			&& FindTacticalTerm(Behind.DefenseRow) == nullptr);

	FString PresentationSource;
	TestTrue(TEXT("UMG presentation source is readable for formula boundary"),
		FFileHelper::LoadFileToString(
			PresentationSource,
			*FPaths::Combine(FPaths::ProjectDir(),
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"))));
	TestTrue(TEXT("UMG maps Formula terms and contains no Tactical Player tier math"),
		!PresentationSource.Contains(TEXT("ModifierForCountAdvantage"))
			&& !PresentationSource.Contains(
				TEXT("AttackerTacticalPlayerModifier"))
			&& !PresentationSource.Contains(
				TEXT("DefenderTacticalPlayerModifier")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionBehindGoldenPathTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.BehindGoldenPathRevealNarrativeAndReconstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionBehindGoldenPathTest::RunTest(
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
		MakeBehindView(false, false));
	const FFMCodexUMGInlineFormulaSurfaceViewModel& PreviewFormula =
		Preview.ThroughBallResolution.Formula;
	TestEqual(TEXT("Behind P1 attack exposes only the early-out helper"),
		PreviewFormula.RollHelperLabel, FString(TEXT("1–2：传球出界")));
	TestTrue(TEXT("Behind P1 defense clears the early-out helper"),
		Build(MakeBehindView(true, false))
			.ThroughBallResolution.Formula.RollHelperLabel.IsEmpty());
	TestTrue(TEXT("Behind route hands off to the production first-stage owner"),
		Preview.ThroughBallResolution.bVisible
			&& Preview.ThroughBallResolution.bSuppressLegacyResolution
			&& Preview.ThroughBallResolution.Route
				== EFMCodexUMGThroughBallRoute::BehindDefense
			&& Preview.ThroughBallResolution.RouteLabel == TEXT("身后球")
			&& Preview.ThroughBallResolution.Stage
				== EFMCodexUMGThroughBallStage::BehindDefenseFirstStage
			&& Preview.ThroughBallResolution.StageLabel == TEXT("身后球")
			&& PreviewFormula.bVisible
			&& PreviewFormula.ContestId
				== TEXT("ThroughBall.BehindDefense.P1")
			&& PreviewFormula.ContestLabel == TEXT("身后球对抗")
			&& PreviewFormula.RouteResultLabel
				== TEXT("路线掷点 4 → 判定为身后球")
			&& PreviewFormula.PrimaryAction.Claims(
				Preview.Interaction.PrimaryAction)
			&& PreviewFormula.ContinueActionLabel == TEXT("进攻方掷点")
			&& Preview.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::Attack
			&& Preview.Interaction.CrossRollContestId
				== TEXT("ThroughBall.BehindDefense.P1")
			&& Preview.Interaction.CrossRollSequenceIndex == 1
			&& Preview.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerA
			&& !Preview.InlineFormula.bVisible);
	TestTrue(TEXT("Behind formula reuses named authoritative contributors"),
		PreviewFormula.AttackRow.Terms.Num() == 3
			&& PreviewFormula.DefenseRow.Terms.Num() == 4
			&& PreviewFormula.AttackRow.Terms[0].ContributorDisplayName
				== TEXT("厄德高")
			&& PreviewFormula.AttackRow.Terms[1].ContributorDisplayName
				== TEXT("哈兰德")
			&& PreviewFormula.DefenseRow.Terms[0].ContributorDisplayName
				== TEXT("萨利巴")
			&& PreviewFormula.DefenseRow.Terms[1].ContributorDisplayName
				== TEXT("赖斯")
			&& RawRoll(PreviewFormula.AttackRow) != nullptr
			&& RawRoll(PreviewFormula.DefenseRow) != nullptr);
	const FFMCodexUMGMatchScreenViewModel RejectedPreview = Build(
		MakeBehindView(false, false), true);
	UFMCodexLocalMatchScreenWidget* RejectedScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	RejectedScreen->TakeWidget();
	RejectedScreen->RefreshFromPresentation(RejectedPreview);
	TestTrue(TEXT("Behind rejection restores diagnostic and lower typed recovery"),
		RejectedScreen->IsLegacyResolutionOverlayVisible()
			&& RejectedScreen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& RejectedScreen->GetThroughBallResolutionSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& RejectedScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Visible
			&& !RejectedPreview.ThroughBallResolution
				.Formula.PrimaryAction.Claims(
					RejectedPreview.Interaction.PrimaryAction)
			&& !RejectedPreview.ThroughBallResolution.Formula.bVisible
			&& RejectedPreview.Interaction.PrimaryAction.bAvailable);

	const FFMCodexUMGMatchScreenViewModel OutOfPlay = Build(MakeBehindView(
		true, false, EMatchPlayResolutionDecisionOutcome::OutOfPlay,
		true, 41, 1));
	TestTrue(TEXT("Behind P1 terminal clears the early-out helper"),
		OutOfPlay.ThroughBallResolution.Formula.RollHelperLabel.IsEmpty());
	TestTrue(TEXT("Authority OutOfPlay maps without a defense formula path"),
		OutOfPlay.ThroughBallResolution.Formula.bNarrativeAvailable
			&& !OutOfPlay.ThroughBallResolution.Formula.bShowFormulaRows
			&& OutOfPlay.ThroughBallResolution.Formula.ResultTitle
				== TEXT("传球出界")
			&& OutOfPlay.ThroughBallResolution.Formula.NarrativeHeadline
				== TEXT("厄德高直塞传出界外。")
			&& OutOfPlay.ThroughBallResolution.Formula.PrimaryAction.Claims(
				OutOfPlay.Interaction.PrimaryAction)
			&& OutOfPlay.Interaction.Category
				== EFMCodexUMGInteractionCategory::AdvanceAfterTerminal
			&& OutOfPlay.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::None);

	UFMCodexLocalMatchScreenWidget* OutScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TestNotNull(TEXT("Behind OutOfPlay reveal screen constructs"), OutScreen);
	if (OutScreen == nullptr)
	{
		return false;
	}
	OutScreen->TakeWidget();
	OutScreen->RefreshFromPresentation(Preview);
	CheckFormula(*this,*OutScreen);
	OutScreen->RefreshFromPresentation(OutOfPlay);
	OutScreen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexInlineResolutionFormulaSurfaceWidget* OutFormulaWidget =
		OutScreen->GetThroughBallResolutionSurface()->GetFormulaSurface();
	TestTrue(TEXT("Attack reel starts with terminal action and narrative gated"),
		OutScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& OutScreen->IsInlineFormulaRevealInputBlocked()
			&& OutFormulaWidget->GetPresentation().bDiceRevealVisible
			&& OutFormulaWidget->GetPresentation().bShowFormulaRows
			&& !OutFormulaWidget->GetPresentation().bNarrativeAvailable
			&& !OutFormulaWidget->GetPresentation().bCanContinue
			&& OutScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	OutScreen->AdvanceInlineFormulaRevealForTesting(1.30f);
	OutScreen->AdvanceInlineFormulaRevealForTesting(0.16f);
	TestTrue(TEXT("OutOfPlay reel lands on authority 1 before narrative"),
		OutFormulaWidget->GetPresentation().RollReel.CenterValue == 1
			&& OutFormulaWidget->GetPresentation().RollReel.bAuthoritativeValue
			&& OutFormulaWidget->GetPresentation().ResultTitle.IsEmpty());
	OutScreen->AdvanceInlineFormulaRevealForTesting(0.38f);
	TestTrue(TEXT("Behind landed early-end suppresses unexecuted comparison and no fake defense"),
		!OutFormulaWidget->GetPresentation().bShowFormulaRows
		&& OutScreen->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()==ESlateVisibility::Collapsed
		&& OutScreen->GetWidgetFromName(TEXT("TheaterAttackBaseHover"))->GetVisibility()==ESlateVisibility::Hidden
		&& CastChecked<UTextBlock>(OutScreen->GetWidgetFromName(TEXT("TheaterAttackRollValue")))->GetText().ToString()==TEXT("1"));
	TestTrue(TEXT("OutOfPlay result and shared narrative disclose during hold"),
		OutFormulaWidget->GetPresentation().ResultTitle == TEXT("传球出界")
			&& OutFormulaWidget->GetPresentation().NarrativeHeadline
				== TEXT("厄德高直塞传出界外。")
			&& !OutFormulaWidget->GetPresentation().bCanContinue);
	OutScreen->AdvanceInlineFormulaRevealForTesting(2.22f);
	TestTrue(TEXT("OutOfPlay hold releases only typed NextRound"),
		OutScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Settled
			&& OutFormulaWidget->GetPresentation().bCanContinue
			&& OutFormulaWidget->GetPresentation().ContinueActionLabel
				== TEXT("下一回合")
			&& OutScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

	const FFMCodexUMGMatchScreenViewModel AttackOnly = Build(
		MakeBehindView(true, false));
	UFMCodexLocalMatchScreenWidget* AttackScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	AttackScreen->TakeWidget();
	AttackScreen->RefreshFromPresentation(Preview);
	AttackScreen->RefreshFromPresentation(AttackOnly);
	AttackScreen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexInlineResolutionFormulaSurfaceWidget* AttackFormulaWidget =
		AttackScreen->GetThroughBallResolutionSurface()->GetFormulaSurface();
	TestTrue(TEXT("Attack 3 reveal keeps the already-projected defense CTA hidden"),
		AttackScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& !AttackFormulaWidget->GetPresentation().bCanContinue
			&& AttackScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	AttackScreen->AdvanceInlineFormulaRevealForTesting(1.30f);
	AttackScreen->AdvanceInlineFormulaRevealForTesting(0.16f);
	AttackScreen->AdvanceInlineFormulaRevealForTesting(0.18f);
	const FFMCodexUMGInlineFormulaTermViewModel* DisclosedAttackRoll =
		RawRoll(AttackFormulaWidget->GetPresentation().AttackRow);
	TestTrue(TEXT("Attack 3 discloses authoritative row without an outcome"),
		DisclosedAttackRoll != nullptr && DisclosedAttackRoll->bResolved
			&& DisclosedAttackRoll->RawD6 == 3
			&& AttackFormulaWidget->GetPresentation()
				.AttackRow.bFinalValueResolved
			&& FMath::IsNearlyEqual(
				AttackFormulaWidget->GetPresentation().AttackRow.FinalValue,
				10.0f)
			&& AttackFormulaWidget->GetPresentation().ResultTitle.IsEmpty());
	AttackScreen->AdvanceInlineFormulaRevealForTesting(2.40f);
	TestTrue(TEXT("Defense CTA appears only after attack reveal settles"),
		AttackScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& AttackFormulaWidget->GetPresentation().bCanContinue
			&& AttackFormulaWidget->GetPresentation().ContinueActionLabel
				== TEXT("防守方掷点")
			&& AttackOnly.Interaction.CrossRollRevealKind
				== EFMCodexUMGCrossRollRevealKind::Defense
			&& AttackOnly.Interaction.CrossRollSequenceIndex == 2
			&& AttackOnly.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerB);

	UFMCodexLocalMatchScreenWidget* AttackReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	AttackReconstructed->TakeWidget();
	AttackReconstructed->RefreshFromPresentation(AttackOnly);
	TestTrue(TEXT("Fresh attack-only snapshot does not replay historical attack"),
		AttackReconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& !AttackReconstructed->IsInlineFormulaRevealInputBlocked()
			&& AttackReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation()
					.AttackRow.bFinalValueResolved
			&& AttackReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation()
					.ContinueActionLabel == TEXT("防守方掷点"));

	const FFMCodexUMGMatchScreenViewModel DefenderStopped = Build(
		MakeBehindView(true, true,
			EMatchPlayResolutionDecisionOutcome::DefenderStoppedAttack,
			true, 41, 3, 6));
	UFMCodexLocalMatchScreenWidget* DefenseScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	DefenseScreen->TakeWidget();
	DefenseScreen->RefreshFromPresentation(AttackOnly);
	DefenseScreen->RefreshFromPresentation(DefenderStopped);
	DefenseScreen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexInlineResolutionFormulaSurfaceWidget* DefenseFormulaWidget =
		DefenseScreen->GetThroughBallResolutionSurface()->GetFormulaSurface();
	TestTrue(TEXT("Defense reel gates final formula narrative and NextRound"),
		DefenseScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& DefenseFormulaWidget->GetPresentation().ResultTitle.IsEmpty()
			&& !DefenseFormulaWidget->GetPresentation().bCanContinue);
	DefenseScreen->AdvanceInlineFormulaRevealForTesting(1.30f);
	DefenseScreen->AdvanceInlineFormulaRevealForTesting(0.16f);
	DefenseScreen->AdvanceInlineFormulaRevealForTesting(0.18f);
	TestTrue(TEXT("Defense formula discloses authoritative final values first"),
		FMath::IsNearlyEqual(
			DefenseFormulaWidget->GetPresentation().AttackRow.FinalValue,
			10.0f)
			&& FMath::IsNearlyEqual(
				DefenseFormulaWidget->GetPresentation().DefenseRow.FinalValue,
				14.0f)
			&& DefenseFormulaWidget->GetPresentation().ResultTitle.IsEmpty());
	DefenseScreen->AdvanceInlineFormulaRevealForTesting(0.20f);
	const FString DefenderNarrative =
		DefenseFormulaWidget->GetPresentation().NarrativeHeadline;
	TestTrue(TEXT("DefenderStopped uses shared deterministic Marker or Helper prose"),
		DefenseFormulaWidget->GetPresentation().ResultTitle
			== TEXT("进攻被阻断")
			&& (DefenderNarrative == TEXT("厄德高的身后球被萨利巴抢断。")
				|| DefenderNarrative == TEXT("哈兰德前插被赖斯拦截。"))
			&& DefenseFormulaWidget->GetPresentation().ResultSubtitle
				== TEXT("身后球 · 进攻被阻断")
			&& !DefenseFormulaWidget->GetPresentation().bCanContinue);
	TestTrue(TEXT("Dense terminal retains only its existing Formula participant cards"),
		DefenseScreen->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& DefenseScreen->GetWidgetFromName(TEXT("TheaterTacticalEvent"))->GetVisibility()==ESlateVisibility::Collapsed
		&& CastChecked<UHorizontalBox>(DefenseScreen->GetWidgetFromName(TEXT("TheaterAttackPeople")))->GetChildrenCount()==2);
	DefenseScreen->AdvanceInlineFormulaRevealForTesting(2.22f);
	TestTrue(TEXT("DefenderStopped hold releases NextRound"),
		DefenseFormulaWidget->GetPresentation().bCanContinue
			&& DefenseFormulaWidget->GetPresentation().ContinueActionLabel
				== TEXT("下一回合"));

	const FFMCodexUMGInlineFormulaSurfaceViewModel NoHelper = Build(
		MakeBehindView(true, true,
			EMatchPlayResolutionDecisionOutcome::DefenderStoppedAttack,
			false, 42, 3, 6)).ThroughBallResolution.Formula;
	TestTrue(TEXT("Helper-absent formula and narrative remain complete"),
		NoHelper.DefenseRow.Terms.Num() == 3
			&& NoHelper.NarrativeHeadline
				== TEXT("厄德高的身后球被萨利巴抢断。")
			&& !NoHelper.NarrativeHeadline.Contains(TEXT("门将")));

	const FFMCodexUMGMatchScreenViewModel OneOnOne = Build(MakeBehindView(
		true, true, EMatchPlayResolutionDecisionOutcome::OneOnOneRequired,
		false, 43, 6, 1));
	UFMCodexLocalMatchScreenWidget* OneOnOneScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	OneOnOneScreen->TakeWidget();
	const FFMCodexUMGMatchScreenViewModel OneOnOneDefensePending = Build(
		MakeBehindView(true, false,
			EMatchPlayResolutionDecisionOutcome::None, false, 43, 6));
	OneOnOneScreen->RefreshFromPresentation(OneOnOneDefensePending);
	OneOnOneScreen->RefreshFromPresentation(OneOnOne);
	OneOnOneScreen->PauseInlineFormulaRevealTimerForTesting();
	UFMCodexInlineResolutionFormulaSurfaceWidget* OneOnOneFormulaWidget =
		OneOnOneScreen->GetThroughBallResolutionSurface()->GetFormulaSurface();
	TestTrue(TEXT("OneOnOne choices cannot leak during the defense reel"),
		OneOnOneScreen->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
			&& OneOnOneScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& OneOnOneFormulaWidget->GetPresentation().ResultTitle.IsEmpty());
	OneOnOneScreen->AdvanceInlineFormulaRevealForTesting(1.30f);
	OneOnOneScreen->AdvanceInlineFormulaRevealForTesting(0.16f);
	OneOnOneScreen->AdvanceInlineFormulaRevealForTesting(0.38f);
	TestTrue(TEXT("OneOnOne creation narrative appears before choices"),
		OneOnOneFormulaWidget->GetPresentation().ResultTitle
			== TEXT("形成单刀")
			&& OneOnOneFormulaWidget->GetPresentation().NarrativeHeadline
				== TEXT("厄德高送出身后球，哈兰德形成单刀！")
			&& !OneOnOneFormulaWidget->GetPresentation().ResultTitle.Contains(
				TEXT("进球"))
			&& !OneOnOneFormulaWidget->GetPresentation().NarrativeHeadline
				.Contains(TEXT("破门"))
			&& !OneOnOneFormulaWidget->GetPresentation().bCanContinue
			&& OneOnOneScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	OneOnOneScreen->AdvanceInlineFormulaRevealForTesting(3.00f);
	TestTrue(TEXT("Intermediate Narrative retains the full readable duration"),
		OneOnOneScreen->IsInlineFormulaRevealInputBlocked()
		&& OneOnOneScreen->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearMethods"))->GetVisibility()==ESlateVisibility::Collapsed);
	OneOnOneScreen->RefreshFromPresentation(OneOnOne); OneOnOneScreen->PauseInlineFormulaRevealTimerForTesting();
	OneOnOneScreen->AdvanceInlineFormulaRevealForTesting(.11f);
	CheckTheater(*this,*OneOnOneScreen);
	TestEqual(TEXT("Direct choice helper is concise"),CastChecked<UTextBlock>(OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearDirectHint")))->GetText().ToString(),FString(TEXT("比较射门与门将单刀")));
	TestEqual(TEXT("Chip helper spans its own button with no horizontal offset"),CastChecked<UHorizontalBoxSlot>(OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearCombinationHint"))->Slot)->GetPadding(),FMargin(0));
	TestEqual(TEXT("Choice helper has a compact line allocation"),CastChecked<USizeBox>(OneOnOneScreen->GetWidgetFromName(TEXT("TheaterDirectExplanationBounds")))->GetHeightOverride(),26.f);
	TestTrue(TEXT("Modern OneOnOne has two legal methods and no terminal CTA"),
		OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearMethods"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& CastChecked<UButton>(OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearDirect")))->GetIsEnabled()
		&& CastChecked<UButton>(OneOnOneScreen->GetWidgetFromName(TEXT("TheaterNearCombination")))->GetIsEnabled()
		&& OneOnOneScreen->GetWidgetFromName(TEXT("TheaterPrimaryBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
	CheckMergedFormula(*this,Preview,DefenderStopped);
	TestTrue(TEXT("Shot choices appear only after the narrative hold"),
		!OneOnOneScreen->IsInlineFormulaRevealInputBlocked()
			&& OneOnOneScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& OneOnOneScreen->GetThroughBallResolutionSurface()
				->GetPresentation().OneOnOneChoices.Num() == 2
			&& OneOnOne.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectOneOnOneShot
			&& OneOnOne.Interaction.OneOnOneChoices.Num() == 2
			&& !OneOnOne.Interaction.PrimaryAction.bAvailable
			&& OneOnOneFormulaWidget->GetPresentation()
				.ContinueActionLabel.IsEmpty());

	UFMCodexLocalMatchScreenWidget* OneOnOneReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	OneOnOneReconstructed->TakeWidget();
	OneOnOneReconstructed->RefreshFromPresentation(OneOnOne);
	TestTrue(TEXT("Fresh OneOnOne snapshot rebuilds result and legal choices without replay"),
		OneOnOneReconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& !OneOnOneReconstructed->IsInlineFormulaRevealInputBlocked()
			&& OneOnOneReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().ResultTitle
					== TEXT("形成单刀")
			&& OneOnOneReconstructed->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& OneOnOneReconstructed->GetThroughBallResolutionSurface()
				->GetPresentation().OneOnOneChoices.Num() == 2
			&& OneOnOneReconstructed->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !OneOnOneReconstructed->IsLegacyResolutionOverlayVisible());

	UFMCodexLocalMatchScreenWidget* TerminalReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	TerminalReconstructed->TakeWidget();
	TerminalReconstructed->RefreshFromPresentation(OutOfPlay);
	CheckTheater(*this,*TerminalReconstructed,true);
	TestTrue(TEXT("Early-end identifies its real attack operand and explains the range"),
		TerminalReconstructed->GetWidgetFromName(TEXT("TheaterAttackRollCaption"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& CastChecked<UTextBlock>(TerminalReconstructed->GetWidgetFromName(TEXT("TheaterReasonSecondary")))->GetText().ToString()==TEXT("进攻掷点 1 落入 1–2，本次不进行防守比较"));
	TestTrue(TEXT("Fresh terminal snapshot rebuilds result and NextRound without replay"),
		TerminalReconstructed->GetInlineFormulaRevealPhase()
			== EFMCodexUMGInlineFormulaRevealPhase::None
			&& TerminalReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().ResultTitle
					== TEXT("传球出界")
			&& TerminalReconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().bCanContinue
			&& TerminalReconstructed->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !TerminalReconstructed->IsLegacyResolutionOverlayVisible());

	const FString ProductText = Preview.ThroughBallResolution.TitleLabel
		+ Preview.ThroughBallResolution.RouteLabel
		+ Preview.ThroughBallResolution.StageLabel
		+ PreviewFormula.ContestLabel + PreviewFormula.StatusLabel;
	TestTrue(TEXT("Behind production contains no engineering or P2 vocabulary"),
		!ProductText.Contains(TEXT("BehindDefense"))
			&& !ProductText.Contains(TEXT("P1"))
			&& !ProductText.Contains(TEXT("P2"))
			&& !ProductText.Contains(TEXT("PrimaryAttack"))
			&& !ProductText.Contains(TEXT("STEP"))
			&& !ProductText.Contains(TEXT("POST-ROUTE")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionAntiAndChipGoldenPathsTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.AntiAndChipGoldenPaths",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionAntiAndChipGoldenPathsTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	using namespace FMCodexThroughBallProductionPresentationTests;
	const FFMCodexUMGMatchScreenViewModel Pending = Build(MakeAntiView(false));
	TestEqual(TEXT("Anti pending UMG category"), Pending.Interaction.Category,
		EFMCodexUMGInteractionCategory::RollThroughBallAntiOffsideAttack);
	TestEqual(TEXT("Anti pending player CTA"),
		Pending.Interaction.PrimaryAction.Label, FString(TEXT("掷点判定越位")));
	TestEqual(TEXT("Anti pending reveal contest"),
		Pending.Interaction.CrossRollContestId,
		FName(TEXT("ThroughBall.AntiOffside")));
	TestEqual(TEXT("Anti pending reveal sequence"),
		Pending.Interaction.CrossRollSequenceIndex, 1);
	TestTrue(TEXT("Anti pending central surface owns CTA"),
		Pending.ThroughBallResolution.PrimaryAction.Claims(
			Pending.Interaction.PrimaryAction));
	TestTrue(TEXT("Anti pending shows canonical outcome ranges only"),
		Pending.ThroughBallResolution.OutcomeRollHint.bVisible
			&& Pending.ThroughBallResolution.OutcomeRollHint.BranchId
				== TEXT("ThroughBall.AntiOffside")
			&& Pending.ThroughBallResolution.OutcomeRollHint.Entries.Num() == 2
			&& Pending.ThroughBallResolution.OutcomeRollHint.DisplayLabel
				== TEXT("1–5：越位　｜　6：反越位成功")
			&& !Pending.ThroughBallResolution.Formula.bVisible);
	TestEqual(TEXT("Anti route result remains persistent"),
		Pending.ThroughBallResolution.RouteResultLabel,
		FString(TEXT("路线掷点 6 → 判定为反越位")));
	const FFMCodexUMGMatchScreenViewModel RejectedPending = Build(
		MakeAntiView(false), true);
	TestTrue(TEXT("Rejected Anti request keeps diagnostics and releases central claim"),
		RejectedPending.Resolution.bVisible
			&& RejectedPending.Resolution.bRejected
			&& !RejectedPending.ThroughBallResolution.PrimaryAction.bClaimsAction);
	UFMCodexLocalMatchScreenWidget* RejectedScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	RejectedScreen->TakeWidget();
	RejectedScreen->RefreshFromPresentation(RejectedPending);
	TestTrue(TEXT("Rejected Anti request has no fake reel and restores lower action"),
		!RejectedScreen->IsInlineFormulaRevealInputBlocked()
			&& RejectedScreen->IsLegacyResolutionOverlayVisible()
			&& RejectedScreen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& RejectedScreen->GetThroughBallResolutionSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& RejectedScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Visible);

	const FFMCodexUMGMatchScreenViewModel Offside = Build(MakeAntiView(true,
		EMatchPlayResolutionDecisionOutcome::Offside, 1));
	TestTrue(TEXT("Anti 1 is terminal Offside without formula or shot choices"),
		Offside.ThroughBallResolution.bNarrativeAvailable
			&& Offside.ThroughBallResolution.ResultTitle == TEXT("越位")
			&& Offside.ThroughBallResolution.NarrativeHeadline
				== TEXT("厄德高送出直塞，哈兰德越位。")
			&& !Offside.ThroughBallResolution.Formula.bVisible
			&& Offside.ThroughBallResolution.OneOnOneChoices.IsEmpty()
			&& Offside.ThroughBallResolution.PrimaryAction.Claims(
				Offside.Interaction.PrimaryAction)
			&& Offside.Interaction.PrimaryAction.Label == TEXT("下一回合")
			&& Offside.ThroughBallResolution.ActionPromptLabel.IsEmpty()
			&& !Offside.ThroughBallResolution.OutcomeRollHint.bVisible);

	const FFMCodexUMGMatchScreenViewModel Success = Build(MakeAntiView(true,
		EMatchPlayResolutionDecisionOutcome::OneOnOneRequired, 6));
	TestTrue(TEXT("Anti 6 is non-terminal and enters the shared central choice"),
		Success.ThroughBallResolution.ResultTitle == TEXT("形成单刀")
			&& Success.ThroughBallResolution.NarrativeHeadline
				== TEXT("厄德高送出直塞，哈兰德反越位成功，形成单刀！")
			&& Success.ThroughBallResolution.Stage
				== EFMCodexUMGThroughBallStage::OneOnOneChoice
			&& Success.ThroughBallResolution.OneOnOneChoices.Num() == 2
			&& !Success.Interaction.PrimaryAction.bAvailable
			&& !Success.ThroughBallResolution.ResultTitle.Contains(TEXT("进球")));

	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Screen->TakeWidget();
	Screen->RefreshFromPresentation(Pending);
	TestEqual(TEXT("Anti keeps passer before runner"),CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterAttackName0")))->GetText().ToString(),FString(TEXT("厄德高")));
	TestTrue(TEXT("Anti uses the dedicated roll lane outside player context"),
		Screen->GetWidgetFromName(TEXT("TheaterTacticalEvent"))->GetParent()==Screen->GetWidgetFromName(TEXT("TheaterActionLane"))
		&& Screen->GetWidgetFromName(TEXT("TheaterEventPending"))==nullptr);
	TestTrue(TEXT("Anti pre-roll enters full Theater with no old modal"),
		Screen->GetThroughBallResolutionSurface()->GetVisibility()
			== ESlateVisibility::Collapsed
			&& Screen->GetWidgetFromName(TEXT("TheaterTacticalEvent")) != nullptr
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !Screen->IsLegacyResolutionOverlayVisible());
	Screen->RefreshFromPresentation(Success);
	Screen->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Anti reel gates result narrative choice and lower duplicate"),
		Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetThroughBallResolutionSurface()->GetPresentation()
				.ResultTitle.IsEmpty()
			&& Screen->GetThroughBallResolutionSurface()->GetPresentation()
				.OneOnOneChoices.IsEmpty()
			&& Screen->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	TestEqual(TEXT("Anti is an independent CompactBox consumer"),
		Screen->GetThroughBallResolutionSurface()->GetRollReelWidget()->GetVisualVariant(), EFMCodexRollVisualVariant::CompactBox);
	TestTrue(TEXT("Anti keeps its own rule during motion"), Screen->GetThroughBallResolutionSurface()->GetPresentation().OutcomeRollHint.bVisible);
	TestTrue(TEXT("Anti's visible canonical range names the OneOnOne destination"),
		CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString().Contains(TEXT("6：形成单刀")));
	Screen->AdvanceInlineFormulaRevealForTesting(.92f);
	TestEqual(TEXT("Anti reuses v2 capture timing"), Screen->GetInlineFormulaRevealPhase(), EFMCodexUMGInlineFormulaRevealPhase::Settling);
	Screen->AdvanceInlineFormulaRevealForTesting(.54f);
	TestTrue(TEXT("Anti keeps rule rather than revealing a result before the narrative gate"),
		CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString().Contains(TEXT("6：形成单刀"))
		&& Screen->GetThroughBallResolutionSurface()->GetPresentation().ResultTitle.IsEmpty());
	TestEqual(TEXT("Anti authoritative six lands in the shared slot"),
		Screen->GetThroughBallResolutionSurface()->GetRollReelWidget()->GetPresentation().CenterValue, 6);
	Screen->AdvanceInlineFormulaRevealForTesting(0.38f);
	TestEqual(TEXT("Anti hold communicates the gated meaning instead of landed status"),
		CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(), FString(TEXT("形成单刀")));
	TestTrue(TEXT("Anti formation narrative discloses before choices"),
		Screen->GetThroughBallResolutionSurface()->GetPresentation().ResultTitle
			== TEXT("形成单刀")
			&& Screen->GetThroughBallResolutionSurface()->GetPresentation()
				.OneOnOneChoices.IsEmpty());
	Screen->AdvanceInlineFormulaRevealForTesting(2.22f);
	TestTrue(TEXT("Anti hold releases the same central two-choice owner"),
		!Screen->IsInlineFormulaRevealInputBlocked()
			&& Screen->GetThroughBallResolutionSurface()->GetPresentation()
				.OneOnOneChoices.Num() == 2
			&& Screen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	CheckTheater(*this,*Screen);
	auto Waiting=Success;
	Waiting.ThroughBallResolution.OneOnOneChoices.Reset(); Waiting.Interaction.OneOnOneChoices.Reset();
	Waiting.bMirrorActionWaitPrompt=Waiting.bActionWaitPromptReadOnly=true;
	Waiting.ActionWaitActorText=FText::FromString(TEXT("等待玩家 A 操作"));
	Waiting.ActionWaitActionText=FText::FromString(TEXT("选择单刀方式"));
	auto* WaitScreen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	WaitScreen->TakeWidget(); WaitScreen->RefreshFromPresentation(Waiting);
	TestTrue(TEXT("Waiting viewer sees explicit read-only modern choice"),
		!CastChecked<UButton>(WaitScreen->GetWidgetFromName(TEXT("TheaterNearDirect")))->GetIsEnabled()
		&& !CastChecked<UButton>(WaitScreen->GetWidgetFromName(TEXT("TheaterNearCombination")))->GetIsEnabled()
		&& CastChecked<UTextBlock>(WaitScreen->GetWidgetFromName(TEXT("TheaterStatus")))->GetText().ToString().Contains(TEXT("等待玩家 A")));

	UFMCodexLocalMatchScreenWidget* Reconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Reconstructed->TakeWidget();
	Reconstructed->RefreshFromPresentation(Offside);
	TestTrue(TEXT("Fresh Anti terminal reconstruction does not replay"),
		!Reconstructed->IsInlineFormulaRevealInputBlocked()
			&& Reconstructed->GetThroughBallResolutionSurface()->GetPresentation()
				.ResultTitle == TEXT("越位")
			&& Reconstructed->GetThroughBallResolutionSurface()->GetPresentation()
				.PrimaryAction.bVisible
			&& Reconstructed->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !Reconstructed->IsLegacyResolutionOverlayVisible());
	for (const int32 D6 : {1,5})
	{
		auto* Failure = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Failure->TakeWidget(); Failure->RefreshFromPresentation(Pending);
		Failure->RefreshFromPresentation(Build(MakeAntiView(true, EMatchPlayResolutionDecisionOutcome::Offside, D6)));
		Failure->PauseInlineFormulaRevealTimerForTesting();
		auto* Outer = Failure->GetThroughBallResolutionSurface();
		TestFalse(TEXT("Offside never creates a Formula"), Outer->GetPresentation().Formula.bVisible);
		Failure->AdvanceInlineFormulaRevealForTesting(1.46f);
		TestEqual(TEXT("Failure lands the accepted independent die"), Outer->GetRollReelWidget()->GetPresentation().CenterValue, D6);
		TestEqual(TEXT("Unfavorable landed die also uses EED7A6"), Outer->GetRollReelWidget()->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor(), FLinearColor::FromSRGBColor(FColor(238,215,166)));
		TestFalse(TEXT("Terminal CTA stays gated during hold"), Outer->GetPresentation().PrimaryAction.bVisible);
		Failure->AdvanceInlineFormulaRevealForTesting(.38f);
		TestEqual(TEXT("Unfavorable Anti hold also uses the gated semantic result"),
			CastChecked<UTextBlock>(Failure->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString(), FString(TEXT("越位")));
		Failure->AdvanceInlineFormulaRevealForTesting(2.21f);
		CheckTheater(*this,*Failure,true);
		TestTrue(TEXT("Sparse Offside keeps a single compact passer/runner card"),
			Failure->GetWidgetFromName(TEXT("TheaterDuel"))->GetVisibility()!=ESlateVisibility::Collapsed
			&& Failure->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()==ESlateVisibility::Collapsed
			&& CastChecked<UTextBlock>(Failure->GetWidgetFromName(TEXT("TheaterAttackName0")))->GetText().ToString()==TEXT("厄德高")
			&& CastChecked<UTextBlock>(Failure->GetWidgetFromName(TEXT("TheaterAttackName1")))->GetText().ToString()==TEXT("哈兰德"));
		TestEqual(TEXT("Offside uses Shared Outcome inside Theater"), Outer->GetVisibility(), ESlateVisibility::Collapsed);
		TestTrue(TEXT("Failure hands off to original Offside Outcome and NextRound"),
			Outer->GetPresentation().ResultTitle == TEXT("越位") && Outer->GetPresentation().PrimaryAction.bVisible
			&& Outer->GetOneOnOneChoiceWidgets().IsEmpty() && !Outer->GetPresentation().Formula.bVisible);
	}

	const FFMCodexUMGMatchScreenViewModel ChipPending = Build(
		MakeChipView(false));
	const FFMCodexUMGMatchScreenViewModel ChipMiss = Build(MakeChipView(true,
		EMatchPlayResolutionDecisionOutcome::Miss, 1));
	const FFMCodexUMGMatchScreenViewModel ChipGoal = Build(MakeChipView(true,
		EMatchPlayResolutionDecisionOutcome::Goal, 6));
	// The dormant fallback keeps its old skin; normal production Chip uses Theater below.
	auto* ReusedSurface = Screen->GetThroughBallResolutionSurface();
	ReusedSurface->RefreshFromPresentation(Pending.ThroughBallResolution);
	ReusedSurface->RefreshFromPresentation(ChipPending.ThroughBallResolution);
	TestEqual(TEXT("Chip remains Legacy after CompactBox host reuse"), ReusedSurface->GetRollReelWidget()->GetVisualVariant(), EFMCodexRollVisualVariant::Legacy);
	TestTrue(TEXT("Chip pending is one shared outcome-only reel"),
		ChipPending.Interaction.Category
			== EFMCodexUMGInteractionCategory
				::RollThroughBallOneOnOneChipShotAttack
			&& ChipPending.Interaction.PrimaryAction.Label == TEXT("挑射掷点")
			&& ChipPending.Interaction.CrossRollContestId
				== TEXT("ThroughBall.OneOnOne.ChipShot")
			&& ChipPending.ThroughBallResolution.StageLabel == TEXT("挑射")
			&& ChipPending.ThroughBallResolution.OutcomeRollHint.bVisible
			&& ChipPending.ThroughBallResolution.OutcomeRollHint.BranchId
				== TEXT("ThroughBall.OneOnOneChip")
			&& ChipPending.ThroughBallResolution.OutcomeRollHint.DisplayLabel
				== TEXT("1–3：挑射未进　｜　4–6：进球")
			&& !ChipPending.ThroughBallResolution.Formula.bVisible);
	TestTrue(TEXT("Chip miss and goal use shared narrative without goalkeeper"),
		ChipMiss.ThroughBallResolution.ResultTitle == TEXT("挑射未进")
			&& ChipMiss.ThroughBallResolution.NarrativeHeadline
				== TEXT("哈兰德挑射未能得分。")
			&& ChipGoal.ThroughBallResolution.ResultTitle == TEXT("进球")
			&& ChipGoal.ThroughBallResolution.NarrativeHeadline
				== TEXT("哈兰德挑射破门！")
			&& !ChipMiss.ThroughBallResolution.NarrativeHeadline.Contains(
				TEXT("门将"))
			&& !ChipGoal.ThroughBallResolution.Formula.bVisible
			&& ChipGoal.ThroughBallResolution.PrimaryAction.Claims(
				ChipGoal.Interaction.PrimaryAction)
			&& ChipGoal.ThroughBallResolution.ActionPromptLabel.IsEmpty()
			&& !ChipGoal.ThroughBallResolution.OutcomeRollHint.bVisible);
	UFMCodexLocalMatchScreenWidget* ChipReconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	ChipReconstructed->TakeWidget();
	ChipReconstructed->RefreshFromPresentation(ChipGoal);
	CheckTheater(*this,*ChipReconstructed,true);
	for (const auto& Terminal:{ChipMiss,ChipGoal})
	{
		auto* Chip=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Chip->TakeWidget(); Chip->RefreshFromPresentation(ChipPending);
		TestTrue(TEXT("Chip independent die also mounts outside its Runner card"),
			Chip->GetWidgetFromName(TEXT("TheaterTacticalEvent"))->GetParent()==Chip->GetWidgetFromName(TEXT("TheaterActionLane"))
			&& Chip->GetWidgetFromName(TEXT("TheaterEventPending"))==nullptr);
		TestTrue(TEXT("Chip lower bar owns the useful rule range"),
			CastChecked<UTextBlock>(Chip->GetWidgetFromName(TEXT("TheaterDetail")))->GetText().ToString().Contains(TEXT("4–6：进球"))
			&& Chip->GetWidgetFromName(TEXT("TheaterEventRule"))==nullptr);
		Chip->RefreshFromPresentation(Terminal);
		Chip->PauseInlineFormulaRevealTimerForTesting();
		auto* Reel=CastChecked<UFMCodexRollReelWidget>(Chip->GetWidgetFromName(TEXT("TheaterEventReel")));
		TestEqual(TEXT("Production Chip is one CompactBox"),Reel->GetVisualVariant(),EFMCodexRollVisualVariant::CompactBox);
		TestTrue(TEXT("Chip never creates Formula, paired sum or defense"),!Chip->GetThroughBallResolutionSurface()->GetPresentation().Formula.bVisible
			&& Chip->GetWidgetFromName(TEXT("TheaterPair"))->GetVisibility()==ESlateVisibility::Collapsed
			&& Chip->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()==ESlateVisibility::Collapsed);
		Chip->AdvanceInlineFormulaRevealForTesting(.92f);
		TestEqual(TEXT("Chip uses continuous v2 capture"),Chip->GetInlineFormulaRevealPhase(),EFMCodexUMGInlineFormulaRevealPhase::Settling);
		Chip->AdvanceInlineFormulaRevealForTesting(.54f);
		TestEqual(TEXT("Chip authoritative landing is EED7A6"),Reel->GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor(),FLinearColor::FromSRGBColor(FColor(238,215,166)));
		Chip->AdvanceInlineFormulaRevealForTesting(2.61f); CheckTheater(*this,*Chip,true);
	}
	TestTrue(TEXT("Fresh Chip terminal has one exclusive production root"),
		ChipReconstructed->GetThroughBallResolutionSurface()->GetVisibility()
			== ESlateVisibility::Collapsed
			&& ChipReconstructed->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !ChipReconstructed->IsLegacyResolutionOverlayVisible());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexThroughBallProductionDirectGoldenPathTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.DirectGoldenPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexThroughBallProductionDirectGoldenPathTest::RunTest(
	const FString& Parameters)
{
	(void)Parameters;
	using namespace FMCodexThroughBallProductionPresentationTests;
	const FFMCodexUMGMatchScreenViewModel Preview = Build(
		MakeDirectView(false, false));
	const FFMCodexUMGInlineFormulaSurfaceViewModel& PreviewFormula =
		Preview.ThroughBallResolution.Formula;
	TestTrue(TEXT("Direct preview is the authoritative shared formula"),
		Preview.ThroughBallResolution.RouteLabel == TEXT("单刀")
			&& Preview.ThroughBallResolution.StageLabel == TEXT("直接射门")
			&& PreviewFormula.bVisible
			&& PreviewFormula.ContestId
				== TEXT("ThroughBall.OneOnOne.DirectShot")
			&& PreviewFormula.ContestLabel == TEXT("直接射门")
			&& PreviewFormula.bParentOwnsContestHeading
			&& PreviewFormula.bParentOwnsRouteContext
			&& !Preview.ThroughBallResolution.OutcomeRollHint.bVisible
			&& PreviewFormula.PrimaryAction.Claims(
				Preview.Interaction.PrimaryAction)
			&& PreviewFormula.ContinueActionLabel == TEXT("进攻方掷点")
			&& PreviewFormula.AttackRow.Terms.ContainsByPredicate(
				[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
				{
					return Term.DisplayLabel == TEXT("战术球员 +1");
				})
			&& Preview.Interaction.CrossRollSequenceIndex == 3
			&& Preview.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerA);
	UFMCodexThroughBallResolutionSurfaceWidget* PreviewSurface =
		NewObject<UFMCodexThroughBallResolutionSurfaceWidget>(
			GetTransientPackage());
	PreviewSurface->TakeWidget();
	PreviewSurface->RefreshFromPresentation(Preview.ThroughBallResolution);
	UTextBlock* NestedContest = Cast<UTextBlock>(
		PreviewSurface->GetFormulaSurface()->GetWidgetFromName(
			TEXT("InlineFormulaContestHeading")));
	UTextBlock* NestedRoute = Cast<UTextBlock>(
		PreviewSurface->GetFormulaSurface()->GetWidgetFromName(
			TEXT("InlineFormulaRouteResult")));
	TestTrue(TEXT("Direct parent owns one main title and one route context"),
		NestedContest != nullptr
			&& NestedContest->GetVisibility() == ESlateVisibility::Collapsed
			&& NestedRoute != nullptr
			&& NestedRoute->GetVisibility() == ESlateVisibility::Collapsed);

	const FFMCodexUMGMatchScreenViewModel AttackOnly = Build(
		MakeDirectView(true, false));
	TestTrue(TEXT("Direct attack-only exposes no winner and only defense roll"),
		AttackOnly.ThroughBallResolution.Formula.AttackRow.bFinalValueResolved
			&& !AttackOnly.ThroughBallResolution.Formula
				.DefenseRow.bFinalValueResolved
			&& AttackOnly.ThroughBallResolution.Formula.ResultTitle.IsEmpty()
			&& AttackOnly.ThroughBallResolution.Formula.ContinueActionLabel
				== TEXT("防守方掷点")
			&& AttackOnly.Interaction.CrossRollSequenceIndex == 4
			&& AttackOnly.Interaction.CrossRollOwnerSide
				== EInitialTurnOrderPlayer::PlayerB);
	UFMCodexLocalMatchScreenWidget* Reconstructed =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	Reconstructed->TakeWidget();
	Reconstructed->RefreshFromPresentation(AttackOnly);
	TestTrue(TEXT("Fresh Direct attack-only reconstruction never replays attack"),
		!Reconstructed->IsInlineFormulaRevealInputBlocked()
			&& Reconstructed->GetInlineFormulaRevealPhase()
				== EFMCodexUMGInlineFormulaRevealPhase::IdlePending
			&& Reconstructed->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !Reconstructed->IsLegacyResolutionOverlayVisible()
			&& Reconstructed->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation()
					.AttackRow.bFinalValueResolved);

	const FFMCodexUMGMatchScreenViewModel Goal = Build(MakeDirectView(
		true, true, EMatchPlayResolutionDecisionOutcome::Goal));
	const FFMCodexUMGMatchScreenViewModel Save = Build(MakeDirectView(
		true, true, EMatchPlayResolutionDecisionOutcome::Miss));
	TestTrue(TEXT("Direct final uses Goal and explicit goalkeeper-save presentation"),
		Goal.ThroughBallResolution.Formula.ResultTitle == TEXT("进球")
			&& Goal.ThroughBallResolution.Formula.NarrativeHeadline
				== TEXT("哈兰德单刀破门！")
			&& Save.ThroughBallResolution.Formula.ResultTitle == TEXT("扑救成功")
			&& Save.ThroughBallResolution.Formula.NarrativeHeadline
				== TEXT("哈兰德单刀射门被阿利松扑出！")
			&& Save.ThroughBallResolution.Formula.PrimaryAction.Claims(
				Save.Interaction.PrimaryAction));

	UFMCodexLocalMatchScreenWidget* DefenseReveal =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	DefenseReveal->TakeWidget();
	DefenseReveal->RefreshFromPresentation(AttackOnly);
	DefenseReveal->RefreshFromPresentation(Save);
	DefenseReveal->PauseInlineFormulaRevealTimerForTesting();
	TestTrue(TEXT("Direct defense reel gates save narrative and NextRound"),
		DefenseReveal->IsInlineFormulaRevealInputBlocked()
			&& DefenseReveal->GetInlineFormulaSurface()->GetVisibility()
				== ESlateVisibility::Collapsed
			&& !DefenseReveal->IsLegacyResolutionOverlayVisible()
			&& DefenseReveal->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().ResultTitle.IsEmpty()
			&& DefenseReveal->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);
	DefenseReveal->AdvanceInlineFormulaRevealForTesting(1.30f);
	DefenseReveal->AdvanceInlineFormulaRevealForTesting(0.16f);
	DefenseReveal->AdvanceInlineFormulaRevealForTesting(0.38f);
	TestTrue(TEXT("Direct save narrative discloses only after final formula"),
		DefenseReveal->GetThroughBallResolutionSurface()
			->GetFormulaSurface()->GetPresentation().ResultTitle
				== TEXT("扑救成功")
			&& !DefenseReveal->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().bCanContinue);
	DefenseReveal->AdvanceInlineFormulaRevealForTesting(2.22f);
	CheckTheater(*this,*DefenseReveal,true); CheckFormula(*this,*DefenseReveal);
	TestTrue(TEXT("Direct uses authority reason"),Save.ThroughBallResolution.Formula.ResolutionReasonLabel.Contains(TEXT("快速压制")));
	CheckMergedFormula(*this,Preview,Save);
	// A low Direct attack is a new accepted operand, never LongShot ImmediateMiss.
	auto LowView=MakeDirectView(true,false,EMatchPlayResolutionDecisionOutcome::Miss);
	LowView.ResolutionFacts.Decisions[0].bResolved=false;
	LowView.ResolutionFacts.Decisions[0].Outcome=EMatchPlayResolutionDecisionOutcome::None;
	const auto Low=Build(LowView);
	auto* LowScreen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	LowScreen->TakeWidget(); LowScreen->RefreshFromPresentation(Preview); LowScreen->RefreshFromPresentation(Low);
	LowScreen->PauseInlineFormulaRevealTimerForTesting(); LowScreen->AdvanceInlineFormulaRevealForTesting(4.05f);
	CheckFormula(*this,*LowScreen);
	TestTrue(TEXT("Direct attack one retains real defense CTA and GK without terminal shortcut"),
		LowScreen->GetThroughBallResolutionSurface()->GetPresentation().Formula.bShowFormulaRows
		&& LowScreen->GetThroughBallResolutionSurface()->GetPresentation().Formula.ResultTitle.IsEmpty()
		&& LowScreen->GetWidgetFromName(TEXT("TheaterDefensePanelBounds"))->GetVisibility()!=ESlateVisibility::Collapsed
		&& Low.Interaction.CrossRollSequenceIndex==4
		&& Low.ThroughBallResolution.Formula.DefenseRow.Participants.Num()==1
		&& Low.ThroughBallResolution.Formula.DefenseRow.Participants[0].PlayerName==TEXT("阿利松"));
	TestTrue(TEXT("Direct terminal hold releases central NextRound"),
		DefenseReveal->GetThroughBallResolutionSurface()
			->GetFormulaSurface()->GetPresentation().bCanContinue
			&& DefenseReveal->GetThroughBallResolutionSurface()
				->GetFormulaSurface()->GetPresentation().ContinueActionLabel
					== TEXT("下一回合"));
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexAntiOffsideFailureReasonTest,
	"FMCodex.LocalPlay.ThroughBallProductionPresentation.AntiOffsideFailureReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexAntiOffsideFailureReasonTest::RunTest(const FString&)
{
	using namespace FMCodexThroughBallProductionPresentationTests;
	struct FCase { int32 First, Second; bool bSuccess; };
	for (const FCase Case : {FCase{4,0,false}, FCase{4,3,false}, FCase{3,2,false}, FCase{6,0,true}, FCase{3,6,true}})
	{
		const int32 First = Case.First, Second = Case.Second;
		const bool bPair = Second > 0, bSuccess = Case.bSuccess;
		auto View = MakeAntiView(true, bSuccess ? EMatchPlayResolutionDecisionOutcome::OneOnOneRequired
			: EMatchPlayResolutionDecisionOutcome::Offside, First);
		View.ResolutionFacts.Rolls.Last().AntiOffsideSecondD6 = Second;
		const auto Before = View.ResolutionFacts;
		const auto Model = Build(View);
		const auto& P = Model.ThroughBallResolution;
		const FString ExpectedReason = bPair ? TEXT("两次判定均未掷出 6，因此越位。")
			: TEXT("反越位判定未掷出 6，因此越位。");
		TestEqual(TEXT("Projected second die preserves authoritative pair/absence"), P.AntiOffsideSecondD6, Second);
		TestEqual(TEXT("Authority outcome unchanged"), P.ResultTitle, FString(bSuccess ? TEXT("形成单刀") : TEXT("越位")));
		TestTrue(TEXT("Presentation leaves authoritative facts unchanged"), FMatchPlayCurrentAttackResolutionFactProjection::StaticStruct()->CompareScriptStruct(&Before, &View.ResolutionFacts, 0));
		if (bSuccess)
		{
			TestFalse(TEXT("No failure reason on single six or second-die six success"), P.OutcomeRollDetail.Contains(TEXT("因此越位")));
		}
		else
		{
			const FString ExpectedRoll = bPair ? FString::Printf(TEXT("反越位专家 · D6：%d、%d"), First, Second)
				: FString::Printf(TEXT("D6：%d"), First);
			TestEqual(TEXT("Exact roll disclosure then subordinate reason"), P.OutcomeRollDetail, ExpectedRoll + TEXT("\n") + ExpectedReason);
			TestEqual(TEXT("Trait attribution only for the authoritative pair"), P.OutcomeRollDetail.Contains(TEXT("反越位专家")), bPair);
			TestEqual(TEXT("Existing main headline preserved"), P.NarrativeHeadline, FString(TEXT("厄德高送出直塞，哈兰德越位。")));
		}
		auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		Screen->TakeWidget(); Screen->RefreshFromPresentation(Model);
		auto* Reason = CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterReasonSecondary")));
		if (!bSuccess)
		{
			TestEqual(TEXT("One existing secondary explanation slot displays approved sentence"), Reason->GetText().ToString(), ExpectedReason);
			TestTrue(TEXT("Failure reason is visible and smaller than roll detail"), Reason->GetVisibility() != ESlateVisibility::Collapsed && Reason->GetFont().Size == 14);
			const FString ExpectedMarkup = (bPair ? FString(TEXT("反越位专家 · ")) : FString())
				+ FString::Printf(TEXT("D6：<Value>%d</>"), First)
				+ (bPair ? FString::Printf(TEXT("、<Value>%d</>"), Second) : FString());
			TestEqual(TEXT("Dice label stays plain; only disclosed operands use the existing Value style"),
				CastChecked<URichTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().ToString(), ExpectedMarkup);
			TestFalse(TEXT("Reason is not duplicated in roll disclosure"), CastChecked<URichTextBlock>(Screen->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().ToString().Contains(TEXT("因此越位")));
			const auto* Fallback = CastChecked<UTextBlock>(Screen->GetThroughBallResolutionSurface()->GetWidgetFromName(TEXT("OutcomeDetail")));
			TestEqual(TEXT("Fallback reuses existing detail slot"), Fallback->GetText().ToString(), P.OutcomeRollDetail);
		}
		else
			TestFalse(TEXT("No failure reason leaks into successful choice page"), Reason->GetText().ToString().Contains(TEXT("因此越位")));
	}
	return true;
}

#endif
