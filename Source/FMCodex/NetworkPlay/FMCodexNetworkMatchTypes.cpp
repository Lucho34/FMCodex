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
	default: break;
	}
	FMCodexNetworkMatchTypes::ProjectDeployment(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectCarrier(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectMarker(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectRunner(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectHelper(SafeViewerView, Result);
	FMCodexNetworkMatchTypes::ProjectSkill(SafeViewerView, Result);
	Result.InteractionState =
		FMCodexNetworkMatchTypes::SelectInteractionState(
			SafeViewerView,
			ViewerSide);
	return Result;
}
#undef LOCTEXT_NAMESPACE
