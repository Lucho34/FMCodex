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

	void AddSelectionOption(
		TArray<FFMCodexLocalMatchSelectionOption>& Options,
		const EInitialTurnOrderPlayer Side,
		const FName Id,
		const FString& Suffix = FString())
	{
		FFMCodexLocalMatchSelectionOption Option;
		Option.Side = Side;
		Option.Id = Id;
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
				View.DeploymentGroups.Add({
					Side, CardId, false, Availability.LegalSlotIds });
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
				View.DeploymentGroups.Add({
					Side,
					GoalkeeperId,
					true,
					GoalkeeperAvailability.LegalSlotIds });
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
					Rule.bSuccess
						? StaticEnum<ESkillRuleType>()->GetNameStringByValue(
							static_cast<int64>(Rule.Snapshot.SkillType))
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

	if (Attack.Phase == EMatchPlayCurrentAttackPhase::Deployment)
	{
		Result.MajorPhase = EFMCodexLocalMatchMajorPhase::Deployment;
		Result.InteractionCategory =
			EFMCodexLocalMatchInteractionCategory::Deploy;
		Result.ExpectedActingPlayer = Attack.CurrentLegalDeploymentSide;
		Result.bHumanInteraction = true;
		BuildDeploymentOptions(Snapshot, Result);
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
				EFMCodexLocalMatchRollGroup::PostRoute,
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
