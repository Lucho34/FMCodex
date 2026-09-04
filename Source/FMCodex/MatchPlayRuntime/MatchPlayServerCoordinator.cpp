#include "MatchPlayServerCoordinator.h"

#include "../CoreRules/MatchEndResolver.h"
#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionAvailability.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionAvailability.h"
#include "../CoreRules/MatchPlaySetPieceCarrierAvailability.h"

namespace MatchPlayServerCoordinator
{
	constexpr int32 MaximumAutomaticSteps = 32;

	bool IsPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	void Stop(
		FMatchPlayServerCoordinatorResult& Result,
		const EMatchPlayServerCoordinatorStopReason Reason)
	{
		Result.bSuccess = true;
		Result.StopReason = Reason;
	}

	void Fail(
		FMatchPlayServerCoordinatorResult& Result,
		const EMatchPlayServerCoordinatorStopReason Reason,
		const FString& ErrorMessage)
	{
		Result.bSuccess = false;
		Result.StopReason = Reason;
		Result.ErrorMessage = ErrorMessage;
	}

	template <typename TResult>
	bool RecordInternalResult(
		FMatchPlayServerCoordinatorResult& CoordinatorResult,
		const TResult& CommandResult)
	{
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope =
			CommandResult.RuntimeEnvelope;
		FMatchPlayServerCoordinatorStep Step;
		Step.CommandKind = Envelope.CommandKind;
		Step.bStateAdvanced = Envelope.bStateAdvanced;
		Step.FailureDisposition = Envelope.FailureDisposition;
		Step.ErrorMessage = Envelope.ErrorMessage;
		CoordinatorResult.Steps.Add(MoveTemp(Step));
		CoordinatorResult.bStateAdvanced |= Envelope.bStateAdvanced;
		if (Envelope.bAccepted && Envelope.bDomainSuccess)
		{
			return true;
		}

		Fail(
			CoordinatorResult,
			EMatchPlayServerCoordinatorStopReason::InternalActionFailed,
			Envelope.ErrorMessage.IsEmpty()
				? TEXT("Server-internal authoritative action failed.")
				: Envelope.ErrorMessage);
		return false;
	}

	ESkillRuleType PreparedActionType(
		const FMatchPlayCurrentAttackState& Attack)
	{
		return Attack.bHasSelectedAction
			? Attack.SelectedAction.ActionType
			: Attack.ActionPreparation.ActionType;
	}
}

FMatchPlayServerCoordinator::FMatchPlayServerCoordinator(
	FMatchPlayAuthoritativeSession& InAuthoritativeSession,
	const FSkillRuleSnapshotSet& InSkillRuleSet)
	: AuthoritativeSession(InAuthoritativeSession)
	, SkillRuleSet(InSkillRuleSet)
{
}

FMatchPlayServerCoordinatorResult
FMatchPlayServerCoordinator::AdvanceToStableState()
{
	using namespace MatchPlayServerCoordinator;

	FMatchPlayServerCoordinatorResult Result;
	for (int32 StepIndex = 0; StepIndex < MaximumAutomaticSteps; ++StepIndex)
	{
		const FMatchPlayState State = AuthoritativeSession.GetStateSnapshot();
		if (!State.RuntimeState.bIsInitialized)
		{
			Stop(Result, EMatchPlayServerCoordinatorStopReason::NotInitialized);
			return Result;
		}

		const FMatchEndResolveResult MatchEnd =
			FMatchEndResolver::ResolveMatchEnd(State.RuntimeState);
		if (!MatchEnd.bSuccess)
		{
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				MatchEnd.ErrorMessage);
			return Result;
		}
		if (MatchEnd.bIsMatchEnded)
		{
			Stop(Result, EMatchPlayServerCoordinatorStopReason::MatchEnded);
			return Result;
		}
		if (!State.bHasCurrentAttack)
		{
			Stop(Result,
				EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
			return Result;
		}

		const FMatchPlayCurrentAttackState& Attack = State.CurrentAttack;
		if (Attack.LifecycleState
			== EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance)
		{
			Stop(Result,
				EMatchPlayServerCoordinatorStopReason::TerminalPendingAdvance);
			return Result;
		}

		if (Attack.RouteKind == EMatchPlayCurrentAttackRouteKind::SendingOff)
		{
			if (Attack.SendingOffRoute.Stage
				== EMatchPlaySendingOffRouteStage::AwaitingResolution)
			{
				FMatchPlaySendingOffResolutionRequest Request;
				Request.AttackSequence = Attack.AttackSequence;
				if (!RecordInternalResult(
					Result,
					AuthoritativeSession.ResolveSendingOff(Request)))
				{
					return Result;
				}
				continue;
			}
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				TEXT("Active SendingOff route has no automatic or player wait state."));
			return Result;
		}

		if (Attack.RouteKind == EMatchPlayCurrentAttackRouteKind::SetPiece)
		{
			const FMatchPlaySetPieceRouteState& Route = Attack.SetPieceRoute;
			if (Route.Stage == EMatchPlaySetPieceRouteStage::AwaitingTypeRoll)
			{
				Stop(Result,
					EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
				return Result;
			}
			if (Route.Stage != EMatchPlaySetPieceRouteStage::TypeResolved)
			{
				Fail(Result,
					EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
					TEXT("Set-piece route is not in a canonical wait state."));
				return Result;
			}

			EMatchPlaySetPieceCarrierRouteStage CarrierStage =
				EMatchPlaySetPieceCarrierRouteStage::None;
			switch (Route.SelectedType)
			{
			case ESetPieceSelectedType::ShortFreeKick:
				CarrierStage = Route.ShortFreeKick.Stage;
				break;
			case ESetPieceSelectedType::LongFreeKick:
				CarrierStage = Route.LongFreeKick.Stage;
				break;
			case ESetPieceSelectedType::Penalty:
				CarrierStage = Route.Penalty.Stage;
				break;
			case ESetPieceSelectedType::Corner:
				Stop(Result,
					EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
				return Result;
			default:
				Fail(Result,
					EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
					TEXT("Resolved set piece has no selected type."));
				return Result;
			}

			if (CarrierStage == EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
			{
				FMatchPlaySetPieceCarrierAvailabilityRequest AvailabilityRequest;
				AvailabilityRequest.AttackSequence = Attack.AttackSequence;
				const FMatchPlaySetPieceCarrierAvailabilityResult Availability =
					FMatchPlaySetPieceCarrierAvailability::Query(
						State, AvailabilityRequest);
				if (!Availability.bSuccess)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("Carrier availability query failed."));
					return Result;
				}
				if (Availability.bHasLegalCarrier)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}

				const EInitialTurnOrderPlayer Attacker =
					State.RuntimeState.CurrentAttackingPlayer;
				if (!IsPlayerSide(Attacker))
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("No-legal set-piece continuation requires a valid attacker."));
					return Result;
				}
				if (Route.SelectedType == ESetPieceSelectedType::ShortFreeKick)
				{
					FMatchPlayShortFreeKickNoLegalCarrierRequest Request;
					Request.AttackSequence = Attack.AttackSequence;
					Request.RequestingSide = Attacker;
					if (!RecordInternalResult(Result,
						AuthoritativeSession.ResolveNoLegalSetPieceCarrier(Request)))
					{
						return Result;
					}
				}
				else if (Route.SelectedType
					== ESetPieceSelectedType::LongFreeKick)
				{
					FMatchPlayLongFreeKickNoLegalCarrierRequest Request;
					Request.AttackSequence = Attack.AttackSequence;
					Request.RequestingSide = Attacker;
					if (!RecordInternalResult(Result,
						AuthoritativeSession.ResolveNoLegalSetPieceCarrier(Request)))
					{
						return Result;
					}
				}
				else
				{
					FMatchPlayPenaltyNoLegalCarrierRequest Request;
					Request.AttackSequence = Attack.AttackSequence;
					Request.RequestingSide = Attacker;
					if (!RecordInternalResult(Result,
						AuthoritativeSession.ResolveNoLegalSetPieceCarrier(Request)))
					{
						return Result;
					}
				}
				continue;
			}

			Stop(Result,
				EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
			return Result;
		}

		if (Attack.RouteKind != EMatchPlayCurrentAttackRouteKind::Ordinary)
		{
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				TEXT("Current attack has no supported authoritative route."));
			return Result;
		}

		if (Attack.Phase == EMatchPlayCurrentAttackPhase::Deployment)
		{
			Stop(Result,
				EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
			return Result;
		}

		if (!Attack.bHasResolutionSession)
		{
			const EInitialTurnOrderPlayer Attacker =
				State.RuntimeState.CurrentAttackingPlayer;
			const EInitialTurnOrderPlayer Defender =
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? EInitialTurnOrderPlayer::PlayerB
					: EInitialTurnOrderPlayer::PlayerA;
			const int64 Sequence = Attack.AttackSequence;
			switch (Attack.SelectionStage)
			{
			case EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier:
			{
				const auto Availability =
					FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
						State, Sequence, Attacker);
				if (!Availability.bQuerySucceeded)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("Carrier availability query failed."));
					return Result;
				}
				if (Availability.bCanSelectAnyCarrier)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.ResolveNoLegalCarrier()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::AwaitingMarker:
			{
				const auto Availability =
					FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
						State, Sequence, Defender);
				if (!Availability.bQuerySucceeded)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("Marker availability query failed."));
					return Result;
				}
				if (Availability.bCanSelectAnyMarker)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.ResolveNoLegalMarker()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::AwaitingSkill:
			{
				const auto Availability =
					FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
						State, Sequence, Attacker, SkillRuleSet);
				if (!Availability.bQuerySucceeded)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("Skill availability query failed."));
					return Result;
				}
				if (Availability.bCanSelectAnySkill)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.ResolveNoLegalSkill()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::AwaitingRunner:
			{
				const auto Availability =
					FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
						State, Sequence, Attacker);
				if (!Availability.bQuerySucceeded)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("Runner availability query failed."));
					return Result;
				}
				if (Availability.bCanSelectAnyRunner)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.ResolveNoLegalRunner()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::AwaitingHelper:
			{
				const auto Availability =
					FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
						State, Sequence, Defender);
				if (!Availability.bQuerySucceeded)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						Availability.ErrorMessage);
					return Result;
				}
				if (Availability.bCanSelectAnyHelper)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.ResolveNoLegalHelper()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::ReadyForResolution:
			{
				const ESkillRuleType ActionType = PreparedActionType(Attack);
				if (ActionType == ESkillRuleType::Cross
					|| ActionType == ESkillRuleType::ThroughBall
					|| ActionType == ESkillRuleType::PassControl)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (ActionType != ESkillRuleType::LongShot
					&& ActionType != ESkillRuleType::CutInsideShot)
				{
					Fail(Result,
						EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
						TEXT("ReadyForResolution has no supported action type."));
					return Result;
				}
				if (!RecordInternalResult(
					Result, AuthoritativeSession.BeginResolutionSession()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayCurrentAttackSelectionStage::AwaitingBranchIntent:
				Stop(Result,
					EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
				return Result;
			default:
				Fail(Result,
					EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
					TEXT("Ordinary selection has no supported stable state."));
				return Result;
			}
		}

		const FMatchPlayCurrentAttackResolutionSession& Session =
			Attack.ResolutionSession;
		if (Session.Stage == EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
		{
			const ESkillRuleType ActionType = Session.Bundle.Binding.ActionType;
			if (ActionType == ESkillRuleType::LongShot
				|| ActionType == ESkillRuleType::CutInsideShot)
			{
				if (!RecordInternalResult(
					Result,
					AuthoritativeSession.ResolveIntentDeterminedRoute()))
				{
					return Result;
				}
				continue;
			}
			if (ActionType == ESkillRuleType::Cross
				|| ActionType == ESkillRuleType::ThroughBall
				|| ActionType == ESkillRuleType::PassControl)
			{
				Stop(Result,
					EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
				return Result;
			}
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				TEXT("AwaitingRoute has no supported action type."));
			return Result;
		}
		if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved
			|| !Session.bHasActualBranch)
		{
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				TEXT("Resolution Session has no canonical route state."));
			return Result;
		}

		const FMatchPlayCurrentAttackPostRouteRollProgressResult Progress =
			FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
		if (!Progress.bIsCanonical)
		{
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				Progress.ErrorMessage);
			return Result;
		}

		if (Session.ThroughBallOneOnOneShotChoice
			!= EMatchPlayThroughBallOneOnOneShotChoice::None)
		{
			if (Session.PostRouteRollProgress.Phase
					== EMatchPlayCurrentAttackPostRouteRollPhase::PrimaryBranch
				|| !Progress.bContractComplete)
			{
				Stop(Result,
					EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
				return Result;
			}
			if (!RecordInternalResult(Result,
				AuthoritativeSession.ApplyThroughBallTerminalResolution()))
			{
				return Result;
			}
			continue;
		}

		if (!Progress.bContractComplete)
		{
			Stop(Result,
				EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
			return Result;
		}

		switch (Session.ActualBranch.ActionType)
		{
		case ESkillRuleType::Cross:
			if (!RecordInternalResult(Result,
				AuthoritativeSession.ApplyCrossTerminalResolution()))
			{
				return Result;
			}
			continue;
		case ESkillRuleType::PassControl:
			if (!RecordInternalResult(Result,
				AuthoritativeSession.ApplyPassControlTerminalResolution()))
			{
				return Result;
			}
			continue;
		case ESkillRuleType::LongShot:
		case ESkillRuleType::CutInsideShot:
			if (!RecordInternalResult(Result,
				AuthoritativeSession.ApplyShotTerminalResolution()))
			{
				return Result;
			}
			continue;
		case ESkillRuleType::ThroughBall:
			switch (Session.ActualBranch.ThroughBall)
			{
			case EMatchPlayThroughBallActualBranch::Feet:
				if (!RecordInternalResult(Result,
					AuthoritativeSession.ApplyThroughBallTerminalResolution()))
				{
					return Result;
				}
				continue;
			case EMatchPlayThroughBallActualBranch::AntiOffside:
			{
				const auto Decision =
					AuthoritativeSession.ResolveThroughBallAntiOffsideDecision();
				if (!RecordInternalResult(Result, Decision))
				{
					return Result;
				}
				if (Decision.OrchestrationResult.OutcomeResult.Decision
					== EThroughBallAntiOffsideOutcomeDecision::OneOnOneRequired)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(Result,
					AuthoritativeSession.ApplyThroughBallTerminalResolution()))
				{
					return Result;
				}
				continue;
			}
			case EMatchPlayThroughBallActualBranch::BehindDefense:
			{
				const auto Formula =
					AuthoritativeSession.ResolveThroughBallBehindDefenseP1Formula();
				if (!RecordInternalResult(Result, Formula))
				{
					return Result;
				}
				if (Formula.OrchestrationResult.FormulaExecutionResult.Decision
					== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision
						::OneOnOneRequired)
				{
					Stop(Result,
						EMatchPlayServerCoordinatorStopReason::WaitingForPlayerIntent);
					return Result;
				}
				if (!RecordInternalResult(Result,
					AuthoritativeSession.ApplyThroughBallTerminalResolution()))
				{
					return Result;
				}
				continue;
			}
			default:
				Fail(Result,
					EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
					TEXT("ThroughBall has no supported actual branch."));
				return Result;
			}
		default:
			Fail(Result,
				EMatchPlayServerCoordinatorStopReason::InvalidAuthoritativeState,
				TEXT("Resolved ordinary attack has no supported action family."));
			return Result;
		}
	}

	Fail(Result,
		EMatchPlayServerCoordinatorStopReason::SafetyLimitReached,
		TEXT("Server coordinator exceeded its deterministic automatic-step limit."));
	return Result;
}
