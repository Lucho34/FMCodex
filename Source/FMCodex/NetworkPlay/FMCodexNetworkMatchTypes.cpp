#include "FMCodexNetworkMatchTypes.h"

#include "../LocalPlay/FMCodexLocalMatchInteractionView.h"
#include "../LocalPlay/FMCodexPlayerUIPresentationText.h"

DEFINE_LOG_CATEGORY(LogFMCodexNetworkPlay);
#define LOCTEXT_NAMESPACE "FMCodexNetworkDeploymentProjection"

namespace FMCodexNetworkMatchTypes
{
	FText CardLabel(const FFMCodexLocalMatchCardView& Card)
	{
		return Card.DisplayLabel.IsEmpty() || Card.DisplayLabel.StartsWith(TEXT("Card "))
			|| Card.DisplayLabel == TEXT("UNKNOWN CARD")
			? LOCTEXT("PlayerFallback", "球员") : FText::FromString(Card.DisplayLabel);
	}
	FText SlotLabel(const FFMCodexLocalMatchInteractionView& View, FName SlotId)
	{
		int32 Index = 0;
		for (const auto& Region : View.PitchRegions)
		{
			for (const auto& Slot : Region.Slots)
			{
				++Index;
				if (Slot.SlotId == SlotId)
				{
					// Stable catalog order is a display label only. Never parse IDs or infer legality.
					return FText::Format(LOCTEXT("Slot", "场地槽位 {0}"), FText::AsNumber(Index));
				}
			}
		}
		return LOCTEXT("SlotFallback", "场地槽位");
	}
	void ProjectDeployment(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		Result.bPlayerADeploymentFinished = View.bPlayerADeploymentFinished;
		Result.bPlayerBDeploymentFinished = View.bPlayerBDeploymentFinished;
		Result.bDeploymentComplete = View.bDeploymentComplete;
		if (Result.EntryWait == EFMCodexNetworkEntryWait::Deployment
			&& View.bHumanInteraction && View.CurrentLegalDeploymentSide == Result.ViewerSide
			&& Result.ViewerSide != EInitialTurnOrderPlayer::None)
		{
			Result.bCanFinishDeployment = View.bCanFinishDeployment;
			// Unique goalkeeper, one bounded offered slot; all canonical legal slots remain accepted.
			for (const auto& Group : View.DeploymentGroups)
			{
				if (!Group.bGoalkeeper || Group.Side != Result.ViewerSide) { continue; }
				for (const FName SlotId : Group.LegalSlotIds)
				{
					FFMCodexNetworkDeployGoalkeeperPayload Choice;
					Choice.SlotId = SlotId;
					if (!Choice.IsValidShape()) { continue; }
					Result.GoalkeeperOption.Choice = Choice;
					Result.GoalkeeperOption.CardLabel = CardLabel(Group.Card);
					Result.GoalkeeperOption.SlotLabel = SlotLabel(View, SlotId);
					Result.bCanDeployGoalkeeper = true;
					break;
				}
				if (Result.bCanDeployGoalkeeper) { break; }
			}
			for (const auto& Group : View.DeploymentGroups)
			{
				if (Group.bGoalkeeper || Group.Side != Result.ViewerSide) { continue; }
				for (const FName SlotId : Group.LegalSlotIds)
				{
					if (Result.DeploymentOptions.Num() == FFMCodexNetworkClientViewSnapshot::MaxDeploymentOptions) { break; }
					FFMCodexNetworkDeploymentOption Option;
					Option.Choice.CardId = Group.CardId;
					Option.Choice.SlotId = SlotId;
					if (!Option.Choice.IsValidShape()) { continue; }
					Option.CardLabel = CardLabel(Group.Card);
					Option.SlotLabel = SlotLabel(View, SlotId);
					Result.DeploymentOptions.Add(MoveTemp(Option));
				}
				if (Result.DeploymentOptions.Num() == FFMCodexNetworkClientViewSnapshot::MaxDeploymentOptions) { break; }
			}
		}
		for (const auto& Placed : View.DeploymentPlacements)
		{
			const auto& PublicRoster = Placed.PlayerSide == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster;
			const auto* Card = PublicRoster.FindByPredicate([&](const auto& C) { return C.CardId == Placed.CardId; });
			if (Card && Card->bGoalkeeper && Card->bGoalkeeperActivatedThisAttack)
			{
				Result.GoalkeeperDeployment.Side = Placed.PlayerSide;
				auto& Public = Result.GoalkeeperDeployment.Placement;
				Public.Choice.CardId = Placed.CardId;
				Public.Choice.SlotId = Placed.SlotId;
				Public.CardLabel = CardLabel(*Card);
				Public.SlotLabel = SlotLabel(View, Placed.SlotId);
				break;
			}
		}
		Result.DeploymentCount = View.DeploymentPlacements.Num();
		if (Result.DeploymentCount == 0) { return; }
		const auto& Last = View.DeploymentPlacements.Last();
		Result.LastDeployment.Side = Last.PlayerSide;
		auto& Placement = Result.LastDeployment.Placement;
		Placement.Choice.CardId = Last.CardId;
		Placement.Choice.SlotId = Last.SlotId;
		Placement.CardLabel = LOCTEXT("PlayerFallback", "球员");
		Placement.SlotLabel = SlotLabel(View, Last.SlotId);
		const auto& Roster = Last.PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? View.PlayerACardRoster : View.PlayerBCardRoster;
		for (const auto& Card : Roster)
		{
			if (Card.CardId == Last.CardId) { Placement.CardLabel = CardLabel(Card); break; }
		}
	}
	// Transport representation only: participant consumers already receive canonical legality.
	template<typename TOption, typename TCopyOption>
	void CopyCompleteSelectionOptions(const FFMCodexLocalMatchInteractionView& View,
		EInitialTurnOrderPlayer Viewer, int32 Bound, TArray<TOption>& Options,
		bool& Unavailable, TCopyOption CopyOption)
	{
		if (View.SelectionOptions.Num() > Bound) { Unavailable = true; return; }
		TSet<FName> Seen;
		for (const auto& Source : View.SelectionOptions)
		{
			TOption Option;
			const FName Identity = CopyOption(Option, Source);
			if (Source.Side != Viewer || !Option.Choice.IsValidShape() || Seen.Contains(Identity))
			{
				Options.Reset(); Unavailable = true; return;
			}
			Seen.Add(Identity);
			Options.Add(MoveTemp(Option)); // Entire safe set, preserving canonical order.
		}
	}
	void ProjectMarker(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		FFMCodexNetworkSubmitMarkerPayload Choice;
		Choice.MarkerCardId = View.SelectedMarkerCardId;
		if (Choice.IsValidShape())
		{
			Result.SelectedMarker.Choice = Choice;
			const auto& Roster = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerBCardRoster : View.PlayerACardRoster;
			const auto* Card = Roster.FindByPredicate([&](const auto& C) { return C.CardId == Choice.MarkerCardId; });
			Result.SelectedMarker.CardLabel = Card ? CardLabel(*Card) : LOCTEXT("PlayerFallback", "球员");
		}
		if (View.InteractionCategory != EFMCodexLocalMatchInteractionCategory::SelectMarker
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		CopyCompleteSelectionOptions(View, Result.ViewerSide,
			FFMCodexNetworkClientViewSnapshot::MaxMarkerOptions, Result.MarkerOptions,
			Result.bMarkerOptionsUnavailable, [](auto& Option, const auto& Source)
			{
				Option.CardLabel = Source.bHasCard ? CardLabel(Source.Card) : LOCTEXT("PlayerFallback", "球员");
				return Option.Choice.MarkerCardId = Source.RelatedCardId;
			});
	}

	FText BranchLabel(EMatchPlayElectiveBranchIntent Intent)
	{
		// Presentation mapping only; canonical legality and order come from the safe view.
		switch (Intent)
		{
		case EMatchPlayElectiveBranchIntent::DirectShot: return FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Direct Shot"));
		case EMatchPlayElectiveBranchIntent::DeadCorner: return FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Dead Corner"));
		case EMatchPlayElectiveBranchIntent::CrossHigh: return FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Cross High"));
		case EMatchPlayElectiveBranchIntent::CrossLow: return FFMCodexPlayerUIPresentationText::MatchScreenLabel(TEXT("Cross Low"));
		default: return FText::GetEmpty();
		}
	}
	void ProjectBranch(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		FFMCodexNetworkSubmitBranchIntentPayload Selected; Selected.Intent = View.ElectiveBranchIntent;
		if (Selected.IsValidShape())
		{
			Result.SelectedBranch.Choice = Selected;
			Result.SelectedBranch.BranchLabel = BranchLabel(Selected.Intent);
		}
		if (Result.EntryWait != EFMCodexNetworkEntryWait::BranchIntentSelection
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		if (View.BranchIntentOptions.Num() > FFMCodexNetworkClientViewSnapshot::MaxBranchOptions)
		{ Result.bBranchOptionsUnavailable = true; return; }
		TSet<EMatchPlayElectiveBranchIntent> Seen;
		for (const auto Intent : View.BranchIntentOptions)
		{
			FFMCodexNetworkBranchOption Option; Option.Choice.Intent = Intent;
			if (!Option.Choice.IsValidShape() || Seen.Contains(Intent))
			{
				Result.BranchOptions.Reset(); Result.bBranchOptionsUnavailable = true; return;
			}
			Seen.Add(Intent); Option.BranchLabel = BranchLabel(Intent);
			Result.BranchOptions.Add(MoveTemp(Option));
		}
	}

	void ProjectInitialRoute(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		if (View.bHumanInteraction && Result.ViewerSide != EInitialTurnOrderPlayer::None
			&& View.ExpectedActingPlayer == Result.ViewerSide)
		{
			switch (View.InteractionCategory)
			{
			case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
				Result.InitialRouteAction = EFMCodexNetworkInitialRouteAction::Cross; break;
			case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute:
				Result.InitialRouteAction = EFMCodexNetworkInitialRouteAction::PassControl; break;
			case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute:
				Result.InitialRouteAction = EFMCodexNetworkInitialRouteAction::ThroughBall; break;
			default: break;
			}
		}
		// Both inputs have already passed BuildForViewer's route-reveal redaction.
		if (!View.ResolutionFacts.bHasActualBranch) { return; }
		const FFMCodexLocalMatchRollView* Roll = nullptr;
		for (const auto& Candidate : View.AcceptedRolls)
		{
			if (Candidate.Group != EFMCodexLocalMatchRollGroup::InitialRoute) { continue; }
			if (Roll != nullptr || Candidate.RawD6 < 1 || Candidate.RawD6 > 6) { return; }
			Roll = &Candidate;
		}
		if (!Roll) { return; }
		const auto& Actual = View.ResolutionFacts.ActualBranch;
		FFMCodexNetworkInitialRouteFact Fact;
		Fact.D6 = Roll->RawD6;
		Fact.ActionType = Actual.ActionType;
		switch (Actual.ActionType)
		{
		case ESkillRuleType::Cross:
			if (Actual.Cross != EMatchPlayCrossActualBranch::High && Actual.Cross != EMatchPlayCrossActualBranch::Low) { return; }
			Fact.Cross = Actual.Cross;
			Fact.RouteLabel = BranchLabel(Actual.Cross == EMatchPlayCrossActualBranch::High
				? EMatchPlayElectiveBranchIntent::CrossHigh : EMatchPlayElectiveBranchIntent::CrossLow);
			break;
		case ESkillRuleType::PassControl:
			Fact.PassControl = Actual.PassControl;
			switch (Actual.PassControl)
			{
			case EMatchPlayPassControlActualBranch::PassAdvance: Fact.RouteLabel = FFMCodexPlayerUIPresentationText::ResolutionContest(TEXT("PassControl.PassAdvance")); break;
			case EMatchPlayPassControlActualBranch::DribbleAdvance: Fact.RouteLabel = FFMCodexPlayerUIPresentationText::ResolutionContest(TEXT("PassControl.DribbleAdvance")); break;
			case EMatchPlayPassControlActualBranch::RunAdvance: Fact.RouteLabel = FFMCodexPlayerUIPresentationText::ResolutionContest(TEXT("PassControl.RunAdvance")); break;
			default: return;
			}
			break;
		case ESkillRuleType::ThroughBall:
			if (Actual.ThroughBall != EMatchPlayThroughBallActualBranch::Feet
				&& Actual.ThroughBall != EMatchPlayThroughBallActualBranch::BehindDefense
				&& Actual.ThroughBall != EMatchPlayThroughBallActualBranch::AntiOffside) { return; }
			Fact.ThroughBall = Actual.ThroughBall;
			Fact.RouteLabel = FFMCodexPlayerUIPresentationText::ThroughBallRoute(Actual.ThroughBall);
			break;
		default: return;
		}
		Result.InitialRoute = MoveTemp(Fact);
	}

	FName OrdinaryContestId(const FFMCodexNetworkClientViewSnapshot& View)
	{
		const auto& R = View.InitialRoute;
		if (R.ActionType == ESkillRuleType::Cross)
		{
			if (R.Cross == EMatchPlayCrossActualBranch::High) return TEXT("Cross.High");
			if (R.Cross == EMatchPlayCrossActualBranch::Low) return TEXT("Cross.Low");
		}
		if (R.ActionType == ESkillRuleType::PassControl)
		{
			switch (R.PassControl)
			{
			case EMatchPlayPassControlActualBranch::PassAdvance: return TEXT("PassControl.PassAdvance");
			case EMatchPlayPassControlActualBranch::DribbleAdvance: return TEXT("PassControl.DribbleAdvance");
			case EMatchPlayPassControlActualBranch::RunAdvance: return TEXT("PassControl.RunAdvance");
			default: break;
			}
		}
		if (R.ActionType == ESkillRuleType::ThroughBall && R.ThroughBall == EMatchPlayThroughBallActualBranch::Feet)
			return TEXT("ThroughBall.Feet");
		return NAME_None;
	}

	void ProjectOrdinaryContest(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		using Purpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
		using Side = EInitialTurnOrderPlayer;
		const bool ThroughBall = Result.InitialRoute.ActionType == ESkillRuleType::ThroughBall;
		if (OrdinaryContestId(Result).IsNone() && !ThroughBall) return;
		TArray<FFMCodexNetworkAcceptedContestRoll> Rolls;
		FFMCodexNetworkContestFact Fact;
		for (const auto& Roll : View.ResolutionFacts.Rolls)
		{
			if (Roll.bInitialRoute) continue;
			const bool Attack = Roll.PostRoutePurpose == Purpose::PrimaryAttack
				|| Roll.PostRoutePurpose == Purpose::OneOnOneDirectShotAttack || Roll.PostRoutePurpose == Purpose::OneOnOneChipShotAttack;
			const bool Defense = Roll.PostRoutePurpose == Purpose::PrimaryDefense || Roll.PostRoutePurpose == Purpose::OneOnOneDirectShotDefense;
			if ((!Attack && !Defense) || !Roll.bResolved || Roll.RawD6 < 1 || Roll.RawD6 > 6
				|| Roll.SequenceIndex != Rolls.Num() + 1 || Rolls.Num() >= (ThroughBall ? 4 : 2)
				|| Roll.OwningSide != (Attack ? Result.CurrentAttackingSide
					: Result.CurrentAttackingSide == Side::PlayerA ? Side::PlayerB : Side::PlayerA)) return;
			auto& Accepted = Rolls.AddDefaulted_GetRef();
			Accepted.Purpose = Roll.PostRoutePurpose; Accepted.SequenceIndex = Roll.SequenceIndex;
			Accepted.D6 = Roll.RawD6; Accepted.OwnerSide = Roll.OwningSide;
			if (Roll.PostRoutePurpose == Purpose::PrimaryAttack) Fact.AttackD6 = Roll.RawD6;
			if (Roll.PostRoutePurpose == Purpose::PrimaryDefense) Fact.DefenseD6 = Roll.RawD6;
		}
		const FName Primary = OrdinaryContestId(Result);
		Fact.bFormulaResolved = !Primary.IsNone() && View.ResolutionFacts.FormulaContests.ContainsByPredicate(
			[&](const auto& F) { return F.ContestId == Primary && F.bHasResolvedFormula; });
		Result.Contest = Fact; Result.AcceptedContestRolls = MoveTemp(Rolls);
		if (!View.bHumanInteraction || Result.ViewerSide == Side::None || View.ExpectedActingPlayer != Result.ViewerSide) return;
		using C = EFMCodexLocalMatchInteractionCategory;
		using A = EFMCodexNetworkContestAction;
		const bool High = Result.InitialRoute.Cross == EMatchPlayCrossActualBranch::High;
		switch (View.InteractionCategory)
		{
		case C::RollCrossAttack: Result.ContestAction = High ? A::CrossHighAttackRoll : A::CrossLowAttackRoll; break;
		case C::RollCrossDefense: Result.ContestAction = High ? A::CrossHighDefenseRoll : A::CrossLowDefenseRoll; break;
		case C::RollPassControlAttack: Result.ContestAction = A::PassControlAttackRoll; break;
		case C::RollPassControlDefense: Result.ContestAction = A::PassControlDefenseRoll; break;
		case C::RollThroughBallFeetAttack: Result.ContestAction = A::ThroughBallFeetAttackRoll; break;
		case C::RollThroughBallFeetDefense: Result.ContestAction = A::ThroughBallFeetDefenseRoll; break;
		case C::RollThroughBallBehindDefenseAttack: Result.ContestAction = A::ThroughBallBehindDefenseP1AttackRoll; break;
		case C::RollThroughBallBehindDefenseDefense: Result.ContestAction = A::ThroughBallBehindDefenseP1DefenseRoll; break;
		case C::RollThroughBallAntiOffsideAttack: Result.ContestAction = A::ThroughBallAntiOffsideAttackRoll; break;
		case C::RollThroughBallOneOnOneDirectShotAttack: Result.ContestAction = A::ThroughBallOneOnOneDirectShotAttackRoll; break;
		case C::RollThroughBallOneOnOneDirectShotDefense: Result.ContestAction = A::ThroughBallOneOnOneDirectShotDefenseRoll; break;
		case C::RollThroughBallOneOnOneChipShotAttack: Result.ContestAction = A::ThroughBallOneOnOneChipShotAttackRoll; break;
		case C::SelectOneOnOneShot:
			if (View.OneOnOneOptions.Num() != 2 || View.OneOnOneOptions[0] == View.OneOnOneOptions[1]) break;
			for (auto Option : View.OneOnOneOptions)
				if (Option != EMatchPlayThroughBallOneOnOneShotChoice::DirectShot && Option != EMatchPlayThroughBallOneOnOneShotChoice::ChipShot) return;
			Result.OneOnOneOptions = View.OneOnOneOptions;
			break;
		default: break;
		}
	}

	void ProjectPublicLifecycle(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		using Side = EInitialTurnOrderPlayer;
		auto ValidSide = [](Side Player) { return Player == Side::PlayerA || Player == Side::PlayerB; };
		if (!ValidSide(Result.ViewerSide)) { return; }
		Result.bGoalHistoryUnavailable = View.GoalHistory.Num() > FFMCodexNetworkClientViewSnapshot::MaxPublicGoals;
		int64 PreviousSequence = 0;
		for (const auto& Goal : View.GoalHistory)
		{
			if (Result.bGoalHistoryUnavailable || Goal.AttackSequence <= PreviousSequence || !ValidSide(Goal.ScoringSide))
			{
				Result.bGoalHistoryUnavailable = true;
				Result.PublicGoalHistory.Reset();
				break;
			}
			PreviousSequence = Goal.AttackSequence;
			FFMCodexNetworkPublicGoal Public;
			Public.AttackSequence = Goal.AttackSequence;
			Public.ScoringSide = Goal.ScoringSide;
			Public.ScorerCardId = Goal.ScorerCardId;
			Public.bSystemAward = Goal.bSystemAward;
			if (Goal.bSystemAward)
			{
				Public.ScorerLabel = LOCTEXT("AwardedGoal", "规则判定进球");
			}
			else
			{
				const auto& Roster = Goal.ScoringSide == Side::PlayerA ? View.PlayerACardRoster : View.PlayerBCardRoster;
				const auto* Card = Roster.FindByPredicate([&](const auto& C) { return C.CardId == Goal.ScorerCardId; });
				Public.ScorerLabel = Card ? CardLabel(*Card) : LOCTEXT("ScorerFallback", "进球球员");
			}
			Result.PublicGoalHistory.Add(MoveTemp(Public));
		}
		if (View.bMatchEnded)
		{
			switch (View.MatchResult)
			{
			case EMatchResultType::HomeWin: Result.MatchResult = EFMCodexNetworkMatchResult::PlayerAWins; break;
			case EMatchResultType::AwayWin: Result.MatchResult = EFMCodexNetworkMatchResult::PlayerBWins; break;
			case EMatchResultType::Draw: Result.MatchResult = EFMCodexNetworkMatchResult::Draw; break;
			default: break;
			}
			return;
		}
		if (View.bHasRecoveryFact && View.RecoverySourceAttackSequence > 0 && View.RecoveryPresentationEntries.Num() <= 2)
		{
			FFMCodexNetworkRecoveryFact Recovery;
			Recovery.SourceAttackSequence = View.RecoverySourceAttackSequence;
			for (const auto& Card : View.RecoveryPresentationEntries)
			{
				if (!ValidSide(Card.OwnerSide) || Card.CardId.IsNone()
					|| Recovery.Cards.ContainsByPredicate([&](const auto& C) { return C.OwnerSide == Card.OwnerSide && C.CardId == Card.CardId; }))
				{
					Recovery = {};
					break;
				}
				FFMCodexNetworkRecoveredCard Entry;
				Entry.OwnerSide = Card.OwnerSide; Entry.CardId = Card.CardId;
				Entry.CardLabel = Card.PlayerDisplayName.IsEmpty() ? LOCTEXT("PlayerFallback", "球员") : FText::FromString(Card.PlayerDisplayName);
				Recovery.Cards.Add(MoveTemp(Entry));
			}
			Result.Recovery = MoveTemp(Recovery);
		}
		if (!View.bTerminalPendingAdvance || Result.bGoalHistoryUnavailable) return;
		const FName Primary = OrdinaryContestId(Result);
		const FName PrimaryDecision = Primary.IsNone() ? NAME_None : FName(*FString::Printf(TEXT("%s.Outcome"), *Primary.ToString()));
		const bool ThroughBall = Result.InitialRoute.ActionType == ESkillRuleType::ThroughBall;
		const FMatchPlayResolutionDecisionFact* Terminal = nullptr;
		for (const auto& Decision : View.ResolutionFacts.Decisions)
		{
			const bool Supported = (!PrimaryDecision.IsNone() && Decision.DecisionId == PrimaryDecision) || (ThroughBall
				&& (Decision.DecisionId == TEXT("ThroughBall.BehindDefense.P1.Outcome")
					|| Decision.DecisionId == TEXT("ThroughBall.AntiOffside.Outcome")
					|| Decision.DecisionId == TEXT("ThroughBall.OneOnOne.DirectShot.Outcome")
					|| Decision.DecisionId == TEXT("ThroughBall.OneOnOne.ChipShot.Outcome")));
			if (!Supported || !Decision.bResolved || Decision.Outcome == EMatchPlayResolutionDecisionOutcome::OneOnOneRequired) continue;
			if (Terminal) return;
			Terminal = &Decision;
		}
		if (!Terminal) return;
		if (Terminal->Semantics == EMatchPlayResolutionRollSemantics::ArithmeticContest
			&& !View.ResolutionFacts.FormulaContests.ContainsByPredicate([&](const auto& F)
			{
				return FName(*FString::Printf(TEXT("%s.Outcome"), *F.ContestId.ToString())) == Terminal->DecisionId
					&& (F.bHasResolvedFormula || F.Application == EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate);
			})) return;
		const auto* Goal = Result.PublicGoalHistory.FindByPredicate([&](const auto& G) { return G.AttackSequence == Result.AttackSequence; });
		if (Terminal->Outcome == EMatchPlayResolutionDecisionOutcome::Goal)
		{
			if (!Goal || Goal->ScoringSide != Result.CurrentAttackingSide || Goal->ScorerCardId.IsNone() || Goal->bSystemAward) { return; }
			Result.Terminal.Outcome = EFMCodexNetworkTerminalOutcome::Goal;
			Result.Terminal.Goal = *Goal;
		}
		else if (Terminal->Outcome == EMatchPlayResolutionDecisionOutcome::Miss)
		{
			if (Goal) { return; }
			Result.Terminal.Outcome = EFMCodexNetworkTerminalOutcome::NoGoal;
		}
		else if (ThroughBall && !Goal)
		{
			switch (Terminal->Outcome)
			{
			case EMatchPlayResolutionDecisionOutcome::OutOfPlay: Result.Terminal.Outcome = EFMCodexNetworkTerminalOutcome::OutOfPlay; break;
			case EMatchPlayResolutionDecisionOutcome::DefenderStoppedAttack: Result.Terminal.Outcome = EFMCodexNetworkTerminalOutcome::DefenderStoppedAttack; break;
			case EMatchPlayResolutionDecisionOutcome::Offside: Result.Terminal.Outcome = EFMCodexNetworkTerminalOutcome::Offside; break;
			default: return;
			}
		}
		else { return; }
		Result.bCanAdvance = View.InteractionCategory == EFMCodexLocalMatchInteractionCategory::AdvanceAfterTerminal
			&& View.bHumanInteraction && View.ExpectedActingPlayer == Result.ViewerSide;
	}

	FText SkillLabel(ESkillRuleType Type)
	{
		switch (Type)
		{
		case ESkillRuleType::LongShot:
		case ESkillRuleType::CutInsideShot:
		case ESkillRuleType::PassControl:
		case ESkillRuleType::Cross:
		case ESkillRuleType::ThroughBall:
			return FFMCodexPlayerUIPresentationText::Skill(FFMCodexLocalMatchInteractionViewBuilder::ToString(Type));
		default: return LOCTEXT("SkillFallback", "战术");
		}
	}
	void ProjectSkill(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		FFMCodexNetworkSubmitSkillPayload Choice; Choice.SkillId = View.SelectedSkillId;
		if (Choice.IsValidShape())
		{
			Result.SelectedSkill.Choice = Choice;
			Result.SelectedSkill.SkillLabel = SkillLabel(View.PresentedActionType);
		}
		if (View.InteractionCategory != EFMCodexLocalMatchInteractionCategory::SelectSkill
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		CopyCompleteSelectionOptions(View, Result.ViewerSide,
			FFMCodexNetworkClientViewSnapshot::MaxSkillOptions, Result.SkillOptions,
			Result.bSkillOptionsUnavailable, [](auto& Option, const auto& Source)
			{
				Option.SkillLabel = SkillLabel(Source.SkillType);
				return Option.Choice.SkillId = Source.Id;
			});
	}
	void ProjectHelper(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		FFMCodexNetworkSubmitHelperPayload Choice;
		Choice.HelperCardId = View.SelectedHelperCardId;
		if (Choice.IsValidShape())
		{
			Result.SelectedHelper.Choice = Choice;
			const auto& Roster = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerBCardRoster : View.PlayerACardRoster;
			const auto* Card = Roster.FindByPredicate([&](const auto& C) { return C.CardId == Choice.HelperCardId; });
			Result.SelectedHelper.CardLabel = Card ? CardLabel(*Card) : LOCTEXT("PlayerFallback", "球员");
		}
		if (View.InteractionCategory != EFMCodexLocalMatchInteractionCategory::SelectHelper
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		CopyCompleteSelectionOptions(View, Result.ViewerSide,
			FFMCodexNetworkClientViewSnapshot::MaxHelperOptions, Result.HelperOptions,
			Result.bHelperOptionsUnavailable, [](auto& Option, const auto& Source)
			{
				Option.CardLabel = Source.bHasCard ? CardLabel(Source.Card) : LOCTEXT("PlayerFallback", "球员");
				return Option.Choice.HelperCardId = Source.RelatedCardId;
			});
	}
	void ProjectRunner(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		FFMCodexNetworkSubmitRunnerPayload Choice;
		Choice.RunnerCardId = View.SelectedRunnerCardId;
		if (Choice.IsValidShape())
		{
			Result.SelectedRunner.Choice = Choice;
			const auto& Roster = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
				? View.PlayerACardRoster : View.PlayerBCardRoster;
			const auto* Card = Roster.FindByPredicate([&](const auto& C) { return C.CardId == Choice.RunnerCardId; });
			Result.SelectedRunner.CardLabel = Card ? CardLabel(*Card) : LOCTEXT("PlayerFallback", "球员");
		}
		if (View.InteractionCategory != EFMCodexLocalMatchInteractionCategory::SelectRunner
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		CopyCompleteSelectionOptions(View, Result.ViewerSide,
			FFMCodexNetworkClientViewSnapshot::MaxRunnerOptions, Result.RunnerOptions,
			Result.bRunnerOptionsUnavailable, [](auto& Option, const auto& Source)
			{
				Option.CardLabel = Source.bHasCard ? CardLabel(Source.Card) : LOCTEXT("PlayerFallback", "球员");
				return Option.Choice.RunnerCardId = Source.RelatedCardId;
			});
	}
	void ProjectCarrier(const FFMCodexLocalMatchInteractionView& View,
		FFMCodexNetworkClientViewSnapshot& Result)
	{
		if (Result.EntryBranch != EFMCodexNetworkEntryBranch::Ordinary) { return; }
		if (!View.SelectedCarrierCardId.IsNone())
		{
			FFMCodexNetworkSubmitCarrierPayload Choice;
			Choice.CarrierCardId = View.SelectedCarrierCardId;
			if (Choice.IsValidShape())
			{
				Result.SelectedCarrier.Choice = Choice;
				const auto& Roster = View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
				const auto* Card = Roster.FindByPredicate([&](const auto& C) { return C.CardId == Choice.CarrierCardId; });
				Result.SelectedCarrier.CardLabel = Card ? CardLabel(*Card) : LOCTEXT("PlayerFallback", "球员");
			}
		}
		if (View.InteractionCategory != EFMCodexLocalMatchInteractionCategory::SelectCarrier
			|| !View.bHumanInteraction || Result.ViewerSide == EInitialTurnOrderPlayer::None
			|| View.ExpectedActingPlayer != Result.ViewerSide) { return; }
		CopyCompleteSelectionOptions(View, Result.ViewerSide,
			FFMCodexNetworkClientViewSnapshot::MaxCarrierOptions, Result.CarrierOptions,
			Result.bCarrierOptionsUnavailable, [](auto& Option, const auto& Source)
			{
				Option.CardLabel = Source.bHasCard ? CardLabel(Source.Card) : LOCTEXT("PlayerFallback", "球员");
				return Option.Choice.CarrierCardId = Source.RelatedCardId;
			});
	}
	EFMCodexNetworkClientInteractionState SelectInteractionState(
		const FFMCodexLocalMatchInteractionView& View,
		const EInitialTurnOrderPlayer ViewerSide)
	{
		if (View.bMatchEnded)
		{
			return EFMCodexNetworkClientInteractionState::MatchEnded;
		}
		if (View.ExpectedActingPlayer == EInitialTurnOrderPlayer::None)
		{
			return EFMCodexNetworkClientInteractionState
				::WaitingForOpponentIntent;
		}
		const bool bOwnAction = View.ExpectedActingPlayer == ViewerSide;
		if (View.bTacticalPointRollReady)
		{
			return bOwnAction
				? EFMCodexNetworkClientInteractionState
					::WaitingForOwnInitialActionPoint
				: EFMCodexNetworkClientInteractionState
					::WaitingForOpponentInitialActionPoint;
		}
		return bOwnAction
			? EFMCodexNetworkClientInteractionState::WaitingForOwnIntent
			: EFMCodexNetworkClientInteractionState::WaitingForOpponentIntent;
	}
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkClientViewSnapshotFactory::BuildWaiting(
	const FGuid& MatchInstanceId,
	const int32 ViewRevision,
	const EInitialTurnOrderPlayer ViewerSide,
	const EFMCodexNetworkBootstrapState BootstrapState)
{
	FFMCodexNetworkClientViewSnapshot Result;
	Result.MatchInstanceId = MatchInstanceId;
	Result.ViewRevision = ViewRevision;
	Result.ViewerSide = ViewerSide;
	Result.BootstrapState = BootstrapState;
	Result.InteractionState =
		EFMCodexNetworkClientInteractionState::WaitingForPlayers;
	return Result;
}

FFMCodexNetworkClientViewSnapshot
FFMCodexNetworkClientViewSnapshotFactory::Build(
	const FFMCodexLocalMatchInteractionView& SafeViewerView,
	const FGuid& MatchInstanceId,
	const int32 ViewRevision,
	const EInitialTurnOrderPlayer ViewerSide,
	const EFMCodexNetworkBootstrapState BootstrapState)
{
	FFMCodexNetworkClientViewSnapshot Result;
	Result.MatchInstanceId = MatchInstanceId;
	Result.ViewRevision = ViewRevision;
	Result.ViewerSide = ViewerSide;
	Result.BootstrapState = BootstrapState;
	Result.bMatchInitialized = SafeViewerView.bMatchActive
		|| SafeViewerView.bMatchEnded;
	Result.bMatchEnded = SafeViewerView.bMatchEnded;
	Result.AttackSequence = SafeViewerView.AttackSequence;
	Result.CurrentAttackingSide =
		SafeViewerView.CurrentAttackingPlayer;
	Result.ExpectedActingSide = SafeViewerView.ExpectedActingPlayer;
	Result.PlayerAScore = SafeViewerView.PlayerAScore;
	Result.PlayerBScore = SafeViewerView.PlayerBScore;
	Result.PlayerAMaxAttackOpportunities =
		SafeViewerView.PlayerAMaxAttackTurns;
	Result.PlayerBMaxAttackOpportunities =
		SafeViewerView.PlayerBMaxAttackTurns;
	Result.DisclosedInitialD12 = SafeViewerView.RawInitialD12;
	switch (SafeViewerView.RouteKind)
	{
	case EMatchPlayCurrentAttackRouteKind::SendingOff:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::SendingOff;
		break;
	case EMatchPlayCurrentAttackRouteKind::Ordinary:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::Ordinary;
		break;
	case EMatchPlayCurrentAttackRouteKind::SetPiece:
		Result.EntryBranch = EFMCodexNetworkEntryBranch::SetPiece;
		break;
	default: break;
	}
	if (SafeViewerView.bTacticalPointRollReady)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::InitialD12;
	}
	else if (SafeViewerView.bTerminalPendingAdvance)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::TerminalPendingAdvance;
	}
	else if (SafeViewerView.RouteKind == EMatchPlayCurrentAttackRouteKind::SetPiece
		&& SafeViewerView.SetPieceStage == EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::SetPieceTypeRoll;
	}
	else if (SafeViewerView.MajorPhase == EFMCodexLocalMatchMajorPhase::Deployment)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::Deployment;
	}
	if (SafeViewerView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectCarrier)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::CarrierSelection;
	}
	if (SafeViewerView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectMarker)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::MarkerSelection;
	}
	if (SafeViewerView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectRunner)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::RunnerSelection;
	}
	if (SafeViewerView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectSkill)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::SkillSelection;
	}
	if (SafeViewerView.InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectHelper)
	{
		Result.EntryWait = EFMCodexNetworkEntryWait::HelperSelection;
	}

	switch (SafeViewerView.InteractionCategory)
	{
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent:
	case EFMCodexLocalMatchInteractionCategory::SelectLongShotBranch:
		Result.EntryWait = EFMCodexNetworkEntryWait::BranchIntentSelection; break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlRoute:
		Result.EntryWait = EFMCodexNetworkEntryWait::PassControlRouteRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallInitialRoute:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallRouteRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossDefense:
		Result.EntryWait = EFMCodexNetworkEntryWait::CrossDefenseRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::CrossAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlDefense:
		Result.EntryWait = EFMCodexNetworkEntryWait::PassControlDefenseRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallBehindDefenseDefense:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallBehindDefenseDefenseRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneDirectShotAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallOneOnOneDirectAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneDirectShotDefense:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallOneOnOneDirectDefenseRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallOneOnOneChipShotAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallOneOnOneChipRoll; break;
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallOneOnOneChoice; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetDefense:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallFeetDefenseRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollPassControlAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::PassControlAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallFeetAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallFeetAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallBehindDefenseAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallBehindDefenseAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollThroughBallAntiOffsideAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::ThroughBallAntiOffsideAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollCrossRoute:
		Result.EntryWait = EFMCodexNetworkEntryWait::CrossRouteRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDirectAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::LongShotDirectAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollLongShotDeadCorner:
		Result.EntryWait = EFMCodexNetworkEntryWait::LongShotDeadCornerRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDirectAttack:
		Result.EntryWait = EFMCodexNetworkEntryWait::CutInsideDirectAttackRoll; break;
	case EFMCodexLocalMatchInteractionCategory::RollCutInsideShotDeadCorner:
		Result.EntryWait = EFMCodexNetworkEntryWait::CutInsideDeadCornerRoll; break;
	default: break;
	}
	FMCodexNetworkMatchTypes::ProjectDeployment(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectCarrier(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectMarker(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectRunner(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectHelper(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectSkill(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectBranch(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectInitialRoute(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectOrdinaryContest(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectPublicLifecycle(SafeViewerView, Result);
	Result.InteractionState =
		FMCodexNetworkMatchTypes::SelectInteractionState(
			SafeViewerView,
			ViewerSide);
	return Result;
}
#undef LOCTEXT_NAMESPACE
