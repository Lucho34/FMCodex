#include "MatchPlayRunnerNoSelectionNoGoal.h"

namespace MatchPlayRunnerNoSelectionNoGoalImplementation
{
	bool IsRunnerNoGoalPlayer(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	EMatchPlayRunnerNoSelectionNoGoalErrorCode
	MapAvailabilityError(
		const EMatchPlayCurrentAttackRunnerSelectionErrorCode
			ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::MatchPlayStateNotInitialized:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::MatchPlayStateNotInitialized;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::NoCurrentAttack:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::NoCurrentAttack;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidCurrentAttackSequence:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackSequence;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::AttackSequenceMismatch:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::AttackSequenceMismatch;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::CurrentAttackNotInResolution:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::CurrentAttackNotInResolution;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackingPlayer;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidCurrentDefendingPlayer:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidCurrentDefendingPlayer;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidSelectionState:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidSelectionState;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::WrongSelectionStage:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::WrongSelectionStage;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidRequestingSide:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidRequestingSide;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::RequestingSideIsNotCurrentAttacker;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidFrozenCarrierCardId:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidFrozenCarrierCardId;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidFrozenMarkerCardId:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidFrozenMarkerCardId;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidFrozenSkillId:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidFrozenSkillId;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::ParticipantRequirementResolutionFailed:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::ParticipantRequirementResolutionFailed;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::ParticipantRequirementMismatch:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::ParticipantRequirementMismatch;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidDeploymentPlacement:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidDeploymentPlacement;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::DuplicateDeploymentCard:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::DuplicateDeploymentCard;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::DuplicateDeploymentSlot:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::DuplicateDeploymentSlot;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::FrozenCarrierNotDeployed:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::FrozenCarrierNotDeployed;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::FrozenCarrierDeploymentAmbiguous:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::FrozenCarrierDeploymentAmbiguous;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::FrozenMarkerNotDeployed:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::FrozenMarkerNotDeployed;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::FrozenMarkerDeploymentAmbiguous:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::FrozenMarkerDeploymentAmbiguous;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidAttackingSnapshotSet:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidAttackingSnapshotSet;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode
			::InvalidSlotCatalog:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidSlotCatalog;
		case EMatchPlayCurrentAttackRunnerSelectionErrorCode::None:
		default:
			return EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::AvailabilityQueryFailed;
		}
	}

	void SetAvailabilityFailure(
		EMatchPlayRunnerNoSelectionNoGoalErrorCode& OutErrorCode,
		FString& OutErrorMessage,
		const FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult&
			Availability)
	{
		const EMatchPlayCurrentAttackRunnerSelectionErrorCode
			AuthorityError =
				Availability.GlobalContextResult.bSuccess
					? Availability.ErrorCode
					: Availability.GlobalContextResult.ErrorCode;
		OutErrorCode = MapAvailabilityError(AuthorityError);
		OutErrorMessage =
			Availability.ErrorMessage.IsEmpty()
				? TEXT("Runner availability query failed.")
				: Availability.ErrorMessage;
	}
}

FMatchPlayResolveNoLegalRunnerResult
FMatchPlayResolveNoLegalRunner::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayResolveNoLegalRunnerRequest& Request)
{
	using namespace MatchPlayRunnerNoSelectionNoGoalImplementation;

	FMatchPlayResolveNoLegalRunnerResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::MatchPlayStateNotInitialized;
		Result.ErrorMessage =
			TEXT("Match play state must be initialized before resolving no legal runner.");
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::NoCurrentAttack;
		Result.ErrorMessage =
			TEXT("Resolving no legal runner requires an active current attack.");
		return Result;
	}

	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsRunnerNoGoalPlayer(Attacker))
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackingPlayer;
		Result.ErrorMessage =
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB.");
		return Result;
	}

	Result.RunnerAvailabilityResult =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Attacker);
	if (!Result.RunnerAvailabilityResult.bQuerySucceeded
		|| !Result.RunnerAvailabilityResult
			.GlobalContextResult.bSuccess)
	{
		SetAvailabilityFailure(
			Result.ErrorCode,
			Result.ErrorMessage,
			Result.RunnerAvailabilityResult);
		return Result;
	}
	if (Result.RunnerAvailabilityResult.bCanSelectAnyRunner)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::LegalRunnerExists;
		Result.ErrorMessage =
			TEXT("Cannot resolve no legal runner while a legal runner exists.");
		return Result;
	}

	Result.Reason =
		EMatchPlayRunnerNoSelectionNoGoalReason::NoLegalRunner;
	Result.Source =
		EMatchPlayRunnerNoSelectionNoGoalSource
			::ResolveNoLegalRunner;
	const FMatchPlayRunnerNoSelectionNoGoalCapability Capability(
		FMatchPlayRunnerNoSelectionNoGoalCapability
			::FResolveNoLegalRunnerIssuerTag(),
		Request.AttackSequence,
		Result.RunnerAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteRunnerNoGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}

FMatchPlayRunnerDeclineResult FMatchPlayRunnerDecline::Decline(
	const FMatchPlayState& BeforeState,
	const FMatchPlayRunnerDeclineRequest& Request)
{
	using namespace MatchPlayRunnerNoSelectionNoGoalImplementation;

	FMatchPlayRunnerDeclineResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	Result.RunnerAvailabilityResult =
		FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide);
	if (!Result.RunnerAvailabilityResult.bQuerySucceeded
		|| !Result.RunnerAvailabilityResult
			.GlobalContextResult.bSuccess)
	{
		SetAvailabilityFailure(
			Result.ErrorCode,
			Result.ErrorMessage,
			Result.RunnerAvailabilityResult);
		return Result;
	}
	if (!Result.RunnerAvailabilityResult.bCanSelectAnyRunner)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::NoLegalRunnerToDecline;
		Result.ErrorMessage =
			TEXT("Decline requires at least one legal runner; use the no-legal-runner entry instead.");
		return Result;
	}

	Result.Reason =
		EMatchPlayRunnerNoSelectionNoGoalReason::RunnerDeclined;
	Result.Source =
		EMatchPlayRunnerNoSelectionNoGoalSource::RunnerDecline;
	const FMatchPlayRunnerNoSelectionNoGoalCapability Capability(
		FMatchPlayRunnerNoSelectionNoGoalCapability
			::FRunnerDeclineIssuerTag(),
		Request.AttackSequence,
		Result.RunnerAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteRunnerNoGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayRunnerNoSelectionNoGoalErrorCode
				::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
