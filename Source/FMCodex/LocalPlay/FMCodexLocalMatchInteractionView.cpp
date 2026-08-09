#include "FMCodexLocalMatchInteractionView.h"

#include "../CoreRules/MatchEndResolver.h"
#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentAvailability.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentAvailability.h"
#include "../CoreRules/SkillRuleSnapshotQuery.h"

namespace FMCodexLocalMatchInteractionView
{
	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: Side == EInitialTurnOrderPlayer::PlayerB
				? EInitialTurnOrderPlayer::PlayerA
				: EInitialTurnOrderPlayer::None;
	}

	const TArray<FName>& AvailableCards(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			: State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	}

	FName GoalkeeperCardId(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return FName(*(Side == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState.GoalkeeperCardId
			: State.RuntimeState.PlayerBState.GoalkeeperCardId));
	}

	const FCardUsageState& CardUsage(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	FString JoinLabels(const TArray<FString>& Labels)
	{
		return FString::Join(Labels, TEXT(" / "));
	}

	FString SkillLabel(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FName SkillId)
	{
		FSkillRuleSnapshotQueryInput Input;
		Input.SkillId = SkillId;
		const auto Query = FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSet, Input);
		return Query.bSuccess
			? FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Query.Snapshot.SkillType)
			: FString::Printf(TEXT("Skill %s"), *SkillId.ToString());
	}

	FFMCodexLocalMatchSlotView MakeSlotView(
		const FMatchPlayState& State,
		const FName SlotId,
		const EInitialTurnOrderPlayer EvaluatedSide)
	{
		FFMCodexLocalMatchSlotView View;
		View.SlotId = SlotId;
		const auto Slot = FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			State.DeploymentSlotCatalog, SlotId);
		if (Slot.bSuccess)
		{
			View.NeutralSide = Slot.SlotDefinition.NeutralSide;
		}
		const auto Zone = FMatchPlayRelativeDeploymentZoneResolver::Resolve(
			State.DeploymentSlotCatalog,
			SlotId,
			State.RuntimeState.CurrentAttackingPlayer,
			EvaluatedSide);
		if (Zone.bSuccess)
		{
			View.RelativeZone = Zone.RelativeZone;
		}
		View.Label = FString::Printf(
			TEXT("%s | %s | %s"),
			*SlotId.ToString(),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(View.NeutralSide),
			*FFMCodexLocalMatchInteractionViewBuilder::ToString(View.RelativeZone));
		return View;
	}

	FFMCodexLocalMatchCardView MakeCardView(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		FFMCodexLocalMatchCardView View;
		View.Side = Side;
		View.CardId = CardId;
		View.DisplayLabel = FString::Printf(TEXT("Card %s"), *CardId.ToString());
		const auto Card =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				State.CardSnapshotAuthority, Side, CardId);
		if (!Card.bSuccess)
		{
			return View;
		}

		View.bGoalkeeper = Card.Snapshot.bIsGoalkeeper;
		TArray<FString> Positions;
		for (const EPlayerPositionType Position : Card.Snapshot.PositionTypes)
		{
			Positions.Add(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Position));
		}
		View.PositionLabel = JoinLabels(Positions);
		const FPlayerAttributes& A = Card.Snapshot.Attributes;
		View.AttributeSummary = FString::Printf(
			TEXT("SHO %d | PAS %d | DRI %d | SPD %d | MRK %d | TKL %d | STA %d | LS %d"),
			A.Shooting, A.Passing, A.Dribbling, A.Speed,
			A.Marking, A.Tackling, A.Stamina, A.LongShot);
		if (Card.Snapshot.bHasGoalkeeperAttributes)
		{
			const FGoalkeeperAttributes& G = Card.Snapshot.GoalkeeperAttributes;
			View.GoalkeeperAttributeSummary = FString::Printf(
				TEXT("HAN %d | POS %d | REF %d | AER %d | ANT %d | 1v1 %d"),
				G.Handling, G.Positioning, G.Reflex,
				G.Aerial, G.Anticipation, G.OneOnOne);
		}
		for (const FName SkillId : Card.Snapshot.SkillIds)
		{
			View.SkillLabels.Add(SkillLabel(SkillRuleSet, SkillId));
		}

		const FCardUsageState& Usage = CardUsage(State, Side);
		View.bAvailable = Usage.AvailableCardIds.Contains(CardId);
		View.bUsed = Usage.UsedCardIds.Contains(CardId);
		View.bGoalkeeperUsedThisMatch = View.bGoalkeeper
			&& (Side == EInitialTurnOrderPlayer::PlayerA
				? State.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed
				: State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed);

		if (State.bHasCurrentAttack)
		{
			for (const FMatchPlayDeploymentPlacement& Placement
				: State.CurrentAttack.DeploymentPlacements)
			{
				if (Placement.PlayerSide == Side
					&& Placement.CardId == CardId)
				{
					View.bDeployed = true;
					View.SlotId = Placement.SlotId;
					const auto Slot = MakeSlotView(State, Placement.SlotId, Side);
					View.NeutralSide = Slot.NeutralSide;
					View.RelativeZone = Slot.RelativeZone;
					break;
				}
			}
			View.bGoalkeeperActivatedThisAttack = View.bGoalkeeper
				&& View.bDeployed
				&& State.CurrentAttack.bCurrentDefenseGoalkeeperActivated
				&& Side != State.RuntimeState.CurrentAttackingPlayer;
		}
		return View;
	}

	void BuildPitchRegions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		for (const EMatchPlayNeutralSlotSide Side : {
			EMatchPlayNeutralSlotSide::NearPlayerA,
			EMatchPlayNeutralSlotSide::NearPlayerB })
		{
			FFMCodexLocalMatchPitchRegionView Region;
			Region.NeutralSide = Side;
			Region.Label =
				FFMCodexLocalMatchInteractionViewBuilder::ToString(Side);
			for (const FMatchPlayDeploymentSlotDefinition& Slot
				: State.DeploymentSlotCatalog.Slots)
			{
				if (Slot.NeutralSide == Side)
				{
					Region.CanonicalSlotIds.Add(Slot.SlotId);
				}
			}
			if (State.bHasCurrentAttack)
			{
				for (const FMatchPlayDeploymentPlacement& Placement
					: State.CurrentAttack.DeploymentPlacements)
				{
					const auto Slot =
						FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
							State.DeploymentSlotCatalog, Placement.SlotId);
					if (Slot.bSuccess
						&& Slot.SlotDefinition.NeutralSide == Side)
					{
						Region.DeployedCards.Add(MakeCardView(
							State,
							SkillRuleSet,
							Placement.PlayerSide,
							Placement.CardId));
					}
				}
			}
			View.PitchRegions.Add(MoveTemp(Region));
		}
	}

	void AddSelectionOption(
		TArray<FFMCodexLocalMatchSelectionOption>& Options,
		const EInitialTurnOrderPlayer Side,
		const FName Id,
		const FName RelatedCardId,
		const FString& Suffix = FString())
	{
		FFMCodexLocalMatchSelectionOption Option;
		Option.Side = Side;
		Option.Id = Id;
		Option.RelatedCardId = RelatedCardId;
		Option.Label = Suffix.IsEmpty()
			? Id.ToString()
			: FString::Printf(TEXT("%s (%s)"), *Id.ToString(), *Suffix);
		Options.Add(MoveTemp(Option));
	}

	FString RollPurpose(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryAttack:
			return TEXT("Primary Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PrimaryDefense:
			return TEXT("Primary Defense");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackA:
			return TEXT("Paired Attack A");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::PairedAttackB:
			return TEXT("Paired Attack B");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::BehindDefenseP2Defense:
			return TEXT("Behind Defense P2 Defense");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneChipShotAttack:
			return TEXT("One-on-One Chip Shot Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack:
			return TEXT("One-on-One Direct Shot Attack");
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense:
			return TEXT("One-on-One Direct Shot Defense");
		default:
			return TEXT("Unknown Post-Route Roll");
		}
	}

	EFMCodexLocalMatchRollGroup RollGroup(
		const EMatchPlayCurrentAttackPostRouteRollPurpose Purpose)
	{
		switch (Purpose)
		{
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneChipShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotAttack:
		case EMatchPlayCurrentAttackPostRouteRollPurpose::OneOnOneDirectShotDefense:
			return EFMCodexLocalMatchRollGroup::OneOnOne;
		default:
			return EFMCodexLocalMatchRollGroup::PostRoute;
		}
	}

	bool RequiresOneOnOneChoice(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet)
	{
		const FMatchPlayCurrentAttackResolutionSession& Session =
			State.CurrentAttack.ResolutionSession;
		if (Session.ThroughBallOneOnOneShotChoice
			!= EMatchPlayThroughBallOneOnOneShotChoice::None
			|| !Session.bHasActualBranch
			|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall)
		{
			return false;
		}

		if (Session.ActualBranch.ThroughBall
			== EMatchPlayThroughBallActualBranch::AntiOffside)
		{
			FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
				Request;
			Request.AttackSequence = State.CurrentAttack.AttackSequence;
			const auto Replay =
				FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator
					::Resolve(State, Request, &SkillRuleSet, nullptr);
			return Replay.bSuccess
				&& Replay.ProviderCallCount == 0
				&& Replay.OutcomeResult.Decision
					== EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired;
		}

		if (Session.ActualBranch.ThroughBall
			== EMatchPlayThroughBallActualBranch::BehindDefense)
		{
			const auto Replay =
				FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator
					::Resolve(State, &SkillRuleSet, nullptr);
			return Replay.bSuccess
				&& Replay.ProviderCallCount == 0
				&& Replay.QueryResult.Decision
					== EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired;
		}
		return false;
	}

	void BuildDeploymentOptions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
		const EInitialTurnOrderPlayer Side = Attack.CurrentLegalDeploymentSide;
		for (const FName CardId : AvailableCards(State, Side))
		{
			const auto Availability =
				FMatchPlayOrdinaryDeploymentAvailability::Query(
					State, Attack.AttackSequence, Side, CardId);
			if (!Availability.bQuerySucceeded)
			{
				View.Diagnostic = Availability.ErrorMessage;
				continue;
			}
			if (Availability.LegalSlotIds.Num() > 0)
			{
				FFMCodexLocalMatchDeploymentGroup Group;
				Group.Side = Side;
				Group.CardId = CardId;
				Group.LegalSlotIds = Availability.LegalSlotIds;
				Group.Card = MakeCardView(
					State, SkillRuleSet, Side, CardId);
				for (const FName SlotId : Availability.LegalSlotIds)
				{
					Group.LegalSlots.Add(MakeSlotView(State, SlotId, Side));
				}
				View.DeploymentGroups.Add(MoveTemp(Group));
			}
			for (const FName SlotId : Availability.LegalSlotIds)
			{
				View.DeploymentOptions.Add({ Side, CardId, SlotId, false });
			}
		}

		const FName GoalkeeperId = GoalkeeperCardId(State, Side);
		const auto GoalkeeperAvailability =
			FMatchPlayGoalkeeperDeploymentAvailability::Query(
				State, Attack.AttackSequence, Side, GoalkeeperId);
		if (GoalkeeperAvailability.bQuerySucceeded)
		{
			if (GoalkeeperAvailability.LegalSlotIds.Num() > 0)
			{
				FFMCodexLocalMatchDeploymentGroup Group;
				Group.Side = Side;
				Group.CardId = GoalkeeperId;
				Group.bGoalkeeper = true;
				Group.LegalSlotIds = GoalkeeperAvailability.LegalSlotIds;
				Group.Card = MakeCardView(
					State, SkillRuleSet, Side, GoalkeeperId);
				for (const FName SlotId : GoalkeeperAvailability.LegalSlotIds)
				{
					Group.LegalSlots.Add(MakeSlotView(State, SlotId, Side));
				}
				View.DeploymentGroups.Add(MoveTemp(Group));
			}
			for (const FName SlotId : GoalkeeperAvailability.LegalSlotIds)
			{
				View.DeploymentOptions.Add({
					Side, GoalkeeperId, SlotId, true });
			}
		}
	}

	void BuildSelectionOptions(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		const int64 Sequence = Attack.AttackSequence;

		switch (Attack.SelectionStage)
		{
		case EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectCarrier;
			View.ExpectedActingPlayer = Attacker;
			const auto Availability =
				FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
					State, Sequence, Attacker);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Attacker,
						Candidate.CarrierCardId,
						Candidate.CarrierCardId);
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyCarrier;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingMarker:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectMarker;
			View.ExpectedActingPlayer = Defender;
			View.bCanDecline = true;
			const auto Availability =
				FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
					State, Sequence, Defender);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Defender,
						Candidate.MarkerCardId,
						Candidate.MarkerCardId);
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyMarker;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingSkill:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectSkill;
			View.ExpectedActingPlayer = Attacker;
			View.bCanDecline = true;
			const auto Availability =
				FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
					State, Sequence, Attacker, SkillRuleSet);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (!Candidate.LegalityResult.bIsLegal)
				{
					continue;
				}
				FSkillRuleSnapshotQueryInput QueryInput;
				QueryInput.SkillId = Candidate.SkillId;
				const FSkillRuleSnapshotQueryResult Rule =
					FSkillRuleSnapshotQuery::FindBySkillId(
						SkillRuleSet, QueryInput);
				AddSelectionOption(
					View.SelectionOptions,
					Attacker,
					Candidate.SkillId,
					Attack.ActionPreparation.CarrierCardId,
					Rule.bSuccess
						? FFMCodexLocalMatchInteractionViewBuilder::ToString(
							Rule.Snapshot.SkillType)
						: TEXT("Unknown Skill"));
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnySkill;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingRunner:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectRunner;
			View.ExpectedActingPlayer = Attacker;
			View.bCanDecline = true;
			const auto Availability =
				FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
					State, Sequence, Attacker);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bIsLegal)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Attacker,
						Candidate.RunnerCardId,
						Candidate.RunnerCardId);
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyRunner;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingHelper:
		{
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectHelper;
			View.ExpectedActingPlayer = Defender;
			View.bCanDecline = true;
			const auto Availability =
				FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
					State, Sequence, Defender);
			for (const auto& Candidate : Availability.Candidates)
			{
				if (Candidate.LegalityResult.bSuccess)
				{
					AddSelectionOption(
						View.SelectionOptions,
						Defender,
						Candidate.HelperCardId,
						Candidate.HelperCardId);
				}
			}
			View.bCanResolveNoLegalChoice = Availability.bQuerySucceeded
				&& !Availability.bCanSelectAnyHelper;
			break;
		}
		case EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent:
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectBranchIntent;
			View.ExpectedActingPlayer = Attacker;
			if (Attack.ActionPreparation.ActionType == ESkillRuleType::Cross)
			{
				View.BranchIntentOptions = {
					EMatchPlayElectiveBranchIntent::CrossHigh,
					EMatchPlayElectiveBranchIntent::CrossLow
				};
			}
			else if (Attack.ActionPreparation.ActionType
					== ESkillRuleType::LongShot
				|| Attack.ActionPreparation.ActionType
					== ESkillRuleType::CutInsideShot)
			{
				View.BranchIntentOptions = {
					EMatchPlayElectiveBranchIntent::DirectShot,
					EMatchPlayElectiveBranchIntent::DeadCorner
				};
			}
			break;
		default:
			View.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::ContinueResolution;
			break;
		}
	}

	void AttachSelectionCardViews(
		const FMatchPlayState& State,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		FFMCodexLocalMatchInteractionView& View)
	{
		for (FFMCodexLocalMatchSelectionOption& Option
			: View.SelectionOptions)
		{
			if (Option.RelatedCardId.IsNone())
			{
				continue;
			}
			Option.Card = MakeCardView(
				State,
				SkillRuleSet,
				Option.Side,
				Option.RelatedCardId);
			Option.bHasCard = !Option.Card.CardId.IsNone();
		}
	}
}

FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch()
{
	FFMCodexLocalMatchInteractionView Result;
	Result.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	return Result;
}

FFMCodexLocalMatchInteractionView
FFMCodexLocalMatchInteractionViewBuilder::Build(
	const FMatchPlayState& Snapshot,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	using namespace FMCodexLocalMatchInteractionView;

	if (!Snapshot.RuntimeState.bIsInitialized)
	{
		return BuildNoActiveMatch();
	}

	FFMCodexLocalMatchInteractionView Result;
	Result.bMatchActive = true;
	Result.PlayerAScore = Snapshot.RuntimeState.PlayerAState.Score;
	Result.PlayerBScore = Snapshot.RuntimeState.PlayerBState.Score;
	Result.CurrentAttackingPlayer =
		Snapshot.RuntimeState.CurrentAttackingPlayer;

	const FMatchEndResolveResult MatchEnd =
		FMatchEndResolver::ResolveMatchEnd(Snapshot.RuntimeState);
	Result.bMatchEnded = MatchEnd.bSuccess && MatchEnd.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		const FMatchResultResolveResult MatchResult =
			FMatchResultResolver::ResolveMatchResult(Snapshot.RuntimeState);
		Result.MatchResult = MatchResult.ResultType;
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Complete;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::MatchEnded;
		return Result;
	}

	Result.bCurrentAttackActive = Snapshot.bHasCurrentAttack;
	if (!Snapshot.bHasCurrentAttack)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::BetweenAttacks;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::BeginAttack;
		Result.ExpectedActingPlayer = Result.CurrentAttackingPlayer;
		Result.bHumanInteraction = true;
		return Result;
	}

	const FMatchPlayCurrentAttackState& Attack = Snapshot.CurrentAttack;
	Result.AttackSequence = Attack.AttackSequence;
	Result.ActionPoint = Attack.ActionPoint;
	Result.SelectionStage = Attack.SelectionStage;
	Result.CurrentLegalDeploymentSide = Attack.CurrentLegalDeploymentSide;
	Result.DeploymentPlacements = Attack.DeploymentPlacements;
	BuildPitchRegions(Snapshot, SkillRuleSet, Result);

	if (Attack.Phase == EMatchPlayCurrentAttackPhase::Deployment)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Deployment;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::Deploy;
		Result.ExpectedActingPlayer = Attack.CurrentLegalDeploymentSide;
		Result.bHumanInteraction = true;
		BuildDeploymentOptions(Snapshot, SkillRuleSet, Result);
		return Result;
	}

	if (Attack.bHasResolutionSession)
	{
		const FMatchPlayCurrentAttackResolutionSession& Session =
			Attack.ResolutionSession;
		for (const FMatchPlayCurrentAttackResolutionRollRecord& Roll
			: Session.InitialRouteRollRecords)
		{
			Result.AcceptedRolls.Add({
				EFMCodexLocalMatchRollGroup::InitialRoute,
				TEXT("Initial Route"),
				Roll.RawD6 });
		}
		for (const FMatchPlayCurrentAttackPostRouteRollRecord& Roll
			: Session.PostRouteRollProgress.RollRecords)
		{
			Result.AcceptedRolls.Add({
				RollGroup(Roll.Purpose),
				RollPurpose(Roll.Purpose),
				Roll.RawD6 });
		}

		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Resolution;
		if (Session.Stage
				== EMatchPlayCurrentAttackResolutionStage::RouteResolved
			&& RequiresOneOnOneChoice(Snapshot, SkillRuleSet))
		{
			Result.InteractionCategory =
				EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot;
			Result.ExpectedActingPlayer = Result.CurrentAttackingPlayer;
			Result.bHumanInteraction = true;
			Result.OneOnOneOptions = {
				EMatchPlayThroughBallOneOnOneShotChoice::ChipShot,
				EMatchPlayThroughBallOneOnOneShotChoice::DirectShot
			};
			return Result;
		}
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::ContinueResolution;
		if (Session.Stage
			== EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
		{
			Result.ContinueActionLabel = TEXT("Continue - Resolve Route");
		}
		else
		{
			const auto Progress =
				FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
			Result.ContinueActionLabel = Progress.bIsCanonical
				&& Progress.bContractComplete
					? TEXT("Continue - Apply Formula / Result")
					: TEXT("Continue - Resolve Post-route Step");
		}
		return Result;
	}

	Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Selection;
	Result.bHumanInteraction = Attack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::ReadyForResolution;
	BuildSelectionOptions(Snapshot, SkillRuleSet, Result);
	AttachSelectionCardViews(Snapshot, SkillRuleSet, Result);
	if (Result.InteractionCategory
		== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
	{
		Result.ContinueActionLabel = TEXT("Continue - Begin Resolution");
	}
	return Result;
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EFMCodexLocalMatchMajorPhase Phase)
{
	switch (Phase)
	{
	case EFMCodexLocalMatchMajorPhase::NoActiveMatch: return TEXT("No Active Match");
	case EFMCodexLocalMatchMajorPhase::BetweenAttacks: return TEXT("Between Attacks");
	case EFMCodexLocalMatchMajorPhase::Deployment: return TEXT("Deployment");
	case EFMCodexLocalMatchMajorPhase::Selection: return TEXT("Selection");
	case EFMCodexLocalMatchMajorPhase::Resolution: return TEXT("Resolution");
	case EFMCodexLocalMatchMajorPhase::Complete: return TEXT("Complete");
	default: return TEXT("Unknown");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EFMCodexLocalMatchInteractionCategory Category)
{
	switch (Category)
	{
	case EFMCodexLocalMatchInteractionCategory::None: return TEXT("None");
	case EFMCodexLocalMatchInteractionCategory::StartMatch: return TEXT("Start Match");
	case EFMCodexLocalMatchInteractionCategory::BeginAttack: return TEXT("Begin Attack");
	case EFMCodexLocalMatchInteractionCategory::Deploy: return TEXT("Deploy / Finish Deployment");
	case EFMCodexLocalMatchInteractionCategory::SelectCarrier: return TEXT("Select Carrier");
	case EFMCodexLocalMatchInteractionCategory::SelectMarker: return TEXT("Select Marker");
	case EFMCodexLocalMatchInteractionCategory::SelectSkill: return TEXT("Select Skill");
	case EFMCodexLocalMatchInteractionCategory::SelectRunner: return TEXT("Select Runner");
	case EFMCodexLocalMatchInteractionCategory::SelectHelper: return TEXT("Select Helper");
	case EFMCodexLocalMatchInteractionCategory::SelectBranchIntent: return TEXT("Select Branch Intent");
	case EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot: return TEXT("Select One-on-One Shot");
	case EFMCodexLocalMatchInteractionCategory::ContinueResolution: return TEXT("Continue Resolution");
	case EFMCodexLocalMatchInteractionCategory::AttackComplete: return TEXT("Attack Complete");
	case EFMCodexLocalMatchInteractionCategory::MatchEnded: return TEXT("Match Ended");
	default: return TEXT("Unknown");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EInitialTurnOrderPlayer Player)
{
	switch (Player)
	{
	case EInitialTurnOrderPlayer::PlayerA: return TEXT("Player A");
	case EInitialTurnOrderPlayer::PlayerB: return TEXT("Player B");
	default: return TEXT("None");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchResultType Result)
{
	switch (Result)
	{
	case EMatchResultType::HomeWin: return TEXT("Player A Win");
	case EMatchResultType::AwayWin: return TEXT("Player B Win");
	case EMatchResultType::Draw: return TEXT("Draw");
	default: return TEXT("Not Ended");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EPlayerPositionType Position)
{
	switch (Position)
	{
	case EPlayerPositionType::Attack: return TEXT("Forward");
	case EPlayerPositionType::Midfield: return TEXT("Midfielder");
	case EPlayerPositionType::Defense: return TEXT("Defender");
	case EPlayerPositionType::Goalkeeper: return TEXT("Goalkeeper");
	default: return TEXT("Unknown Role");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchPlayNeutralSlotSide Side)
{
	switch (Side)
	{
	case EMatchPlayNeutralSlotSide::NearPlayerA:
		return TEXT("Player A Half");
	case EMatchPlayNeutralSlotSide::NearPlayerB:
		return TEXT("Player B Half");
	default:
		return TEXT("Unknown Field Region");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const EMatchPlayRelativeDeploymentZone Zone)
{
	switch (Zone)
	{
	case EMatchPlayRelativeDeploymentZone::Forward: return TEXT("Forward");
	case EMatchPlayRelativeDeploymentZone::Midfield: return TEXT("Midfield");
	case EMatchPlayRelativeDeploymentZone::Backfield: return TEXT("Backfield");
	default: return TEXT("Unknown Zone");
	}
}

FString FFMCodexLocalMatchInteractionViewBuilder::ToString(
	const ESkillRuleType SkillType)
{
	switch (SkillType)
	{
	case ESkillRuleType::LongShot: return TEXT("Long Shot");
	case ESkillRuleType::CutInsideShot: return TEXT("Cut Inside");
	case ESkillRuleType::PassControl: return TEXT("Pass Control");
	case ESkillRuleType::Cross: return TEXT("Cross");
	case ESkillRuleType::ThroughBall: return TEXT("Through Ball");
	default: return TEXT("Unknown Skill");
	}
}
