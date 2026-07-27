#include "MatchPlayMarkerNoSelectionGoal.h"

#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"

namespace MatchPlayMarkerNoSelectionGoalImplementation
{
	enum class ECommonValidationError : uint8
	{
		None,
		MatchPlayStateNotInitialized,
		NoCurrentAttack,
		InvalidCurrentAttackSequence,
		AttackSequenceMismatch,
		CurrentAttackNotInResolution,
		InvalidCurrentAttackingPlayer,
		InvalidCurrentDefendingPlayer,
		InvalidSelectionState,
		WrongSelectionStage
	};

	struct FCommonValidationResult
	{
		bool bSuccess = false;
		ECommonValidationError Error =
			ECommonValidationError::None;
		EInitialTurnOrderPlayer Defender =
			EInitialTurnOrderPlayer::None;
		FMatchPlayCurrentAttackSelectionStateValidationResult
			SelectionStateValidationResult;
		FString ErrorMessage;
	};

	bool IsMarkerGoalPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetMarkerGoalDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			return EInitialTurnOrderPlayer::PlayerB;
		}
		if (Attacker == EInitialTurnOrderPlayer::PlayerB)
		{
			return EInitialTurnOrderPlayer::PlayerA;
		}
		return EInitialTurnOrderPlayer::None;
	}

	FCommonValidationResult ValidateCommon(
		const FMatchPlayState& BeforeState,
		const int64 RequestedAttackSequence)
	{
		FCommonValidationResult Result;
		if (!BeforeState.RuntimeState.bIsInitialized)
		{
			Result.Error =
				ECommonValidationError::MatchPlayStateNotInitialized;
			Result.ErrorMessage =
				TEXT("Match play state must be initialized before resolving marker no-selection.");
			return Result;
		}
		if (!BeforeState.bHasCurrentAttack)
		{
			Result.Error = ECommonValidationError::NoCurrentAttack;
			Result.ErrorMessage =
				TEXT("Marker no-selection requires an active current attack.");
			return Result;
		}

		const FMatchPlayCurrentAttackState& CurrentAttack =
			BeforeState.CurrentAttack;
		if (CurrentAttack.AttackSequence <= 0)
		{
			Result.Error =
				ECommonValidationError::InvalidCurrentAttackSequence;
			Result.ErrorMessage =
				TEXT("Current attack sequence must be greater than zero.");
			return Result;
		}
		if (RequestedAttackSequence != CurrentAttack.AttackSequence)
		{
			Result.Error =
				ECommonValidationError::AttackSequenceMismatch;
			Result.ErrorMessage =
				TEXT("Request sequence does not match the current attack.");
			return Result;
		}
		if (CurrentAttack.Phase
			!= EMatchPlayCurrentAttackPhase::Resolution)
		{
			Result.Error =
				ECommonValidationError::CurrentAttackNotInResolution;
			Result.ErrorMessage =
				TEXT("Current attack must be in Resolution phase.");
			return Result;
		}

		const EInitialTurnOrderPlayer Attacker =
			BeforeState.RuntimeState.CurrentAttackingPlayer;
		if (!IsMarkerGoalPlayer(Attacker))
		{
			Result.Error =
				ECommonValidationError::InvalidCurrentAttackingPlayer;
			Result.ErrorMessage =
				TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB.");
			return Result;
		}
		Result.Defender = GetMarkerGoalDefender(Attacker);
		if (!IsMarkerGoalPlayer(Result.Defender))
		{
			Result.Error =
				ECommonValidationError::InvalidCurrentDefendingPlayer;
			Result.ErrorMessage =
				TEXT("Current defending player could not be derived.");
			return Result;
		}

		Result.SelectionStateValidationResult =
			FMatchPlayCurrentAttackSelectionStateValidator::Validate(
				CurrentAttack);
		if (!Result.SelectionStateValidationResult.bIsCanonical)
		{
			Result.Error =
				ECommonValidationError::InvalidSelectionState;
			Result.ErrorMessage =
				Result.SelectionStateValidationResult.ErrorMessage;
			return Result;
		}
		if (CurrentAttack.SelectionStage
			!= EMatchPlayCurrentAttackSelectionStage::AwaitingMarker)
		{
			Result.Error =
				ECommonValidationError::WrongSelectionStage;
			Result.ErrorMessage =
				TEXT("Marker no-selection requires AwaitingMarker stage.");
			return Result;
		}

		Result.bSuccess = true;
		return Result;
	}

	EMatchPlayResolveNoLegalMarkerErrorCode MapNoLegalError(
		const ECommonValidationError Error)
	{
		switch (Error)
		{
		case ECommonValidationError::MatchPlayStateNotInitialized:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::MatchPlayStateNotInitialized;
		case ECommonValidationError::NoCurrentAttack:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::NoCurrentAttack;
		case ECommonValidationError::InvalidCurrentAttackSequence:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::InvalidCurrentAttackSequence;
		case ECommonValidationError::AttackSequenceMismatch:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::AttackSequenceMismatch;
		case ECommonValidationError::CurrentAttackNotInResolution:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::CurrentAttackNotInResolution;
		case ECommonValidationError::InvalidCurrentAttackingPlayer:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::InvalidCurrentAttackingPlayer;
		case ECommonValidationError::InvalidCurrentDefendingPlayer:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::InvalidCurrentDefendingPlayer;
		case ECommonValidationError::InvalidSelectionState:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::InvalidSelectionState;
		case ECommonValidationError::WrongSelectionStage:
			return EMatchPlayResolveNoLegalMarkerErrorCode
				::WrongSelectionStage;
		case ECommonValidationError::None:
		default:
			return EMatchPlayResolveNoLegalMarkerErrorCode::None;
		}
	}

	EMatchPlayMarkerDeclineErrorCode MapDeclineError(
		const ECommonValidationError Error)
	{
		switch (Error)
		{
		case ECommonValidationError::MatchPlayStateNotInitialized:
			return EMatchPlayMarkerDeclineErrorCode
				::MatchPlayStateNotInitialized;
		case ECommonValidationError::NoCurrentAttack:
			return EMatchPlayMarkerDeclineErrorCode::NoCurrentAttack;
		case ECommonValidationError::InvalidCurrentAttackSequence:
			return EMatchPlayMarkerDeclineErrorCode
				::InvalidCurrentAttackSequence;
		case ECommonValidationError::AttackSequenceMismatch:
			return EMatchPlayMarkerDeclineErrorCode
				::AttackSequenceMismatch;
		case ECommonValidationError::CurrentAttackNotInResolution:
			return EMatchPlayMarkerDeclineErrorCode
				::CurrentAttackNotInResolution;
		case ECommonValidationError::InvalidCurrentAttackingPlayer:
			return EMatchPlayMarkerDeclineErrorCode
				::InvalidCurrentAttackingPlayer;
		case ECommonValidationError::InvalidCurrentDefendingPlayer:
			return EMatchPlayMarkerDeclineErrorCode
				::InvalidCurrentDefendingPlayer;
		case ECommonValidationError::InvalidSelectionState:
			return EMatchPlayMarkerDeclineErrorCode
				::InvalidSelectionState;
		case ECommonValidationError::WrongSelectionStage:
			return EMatchPlayMarkerDeclineErrorCode
				::WrongSelectionStage;
		case ECommonValidationError::None:
		default:
			return EMatchPlayMarkerDeclineErrorCode::None;
		}
	}

	int32 CountDefenderPlacements(
		const FMatchPlayState& State,
		const EInitialTurnOrderPlayer Defender)
	{
		int32 Count = 0;
		for (const FMatchPlayDeploymentPlacement& Placement :
			State.CurrentAttack.DeploymentPlacements)
		{
			if (Placement.PlayerSide == Defender)
			{
				++Count;
			}
		}
		return Count;
	}
}

FMatchPlayResolveNoLegalMarkerResult
FMatchPlayResolveNoLegalMarker::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayResolveNoLegalMarkerRequest& Request)
{
	using namespace MatchPlayMarkerNoSelectionGoalImplementation;

	FMatchPlayResolveNoLegalMarkerResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	const FCommonValidationResult Validation =
		ValidateCommon(BeforeState, Request.AttackSequence);
	Result.SelectionStateValidationResult =
		Validation.SelectionStateValidationResult;
	if (!Validation.bSuccess)
	{
		Result.ErrorCode = MapNoLegalError(Validation.Error);
		Result.ErrorMessage = Validation.ErrorMessage;
		return Result;
	}

	Result.MarkerAvailabilityResult =
		FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Validation.Defender);
	if (!Result.MarkerAvailabilityResult.bQuerySucceeded
		|| Result.MarkerAvailabilityResult
			.bHasGlobalBlockingLegalityResult)
	{
		Result.ErrorCode =
			EMatchPlayResolveNoLegalMarkerErrorCode
				::MarkerAvailabilityFailed;
		Result.ErrorMessage =
			Result.MarkerAvailabilityResult
				.bHasGlobalBlockingLegalityResult
				? Result.MarkerAvailabilityResult
					.GlobalBlockingLegalityResult.ErrorMessage
				: TEXT("Marker availability query failed.");
		return Result;
	}
	if (Result.MarkerAvailabilityResult.bCanSelectAnyMarker)
	{
		Result.ErrorCode =
			EMatchPlayResolveNoLegalMarkerErrorCode
				::LegalMarkerExists;
		Result.ErrorMessage =
			TEXT("Cannot resolve no legal marker while a legal marker exists.");
		return Result;
	}

	const EMatchPlayMarkerNoSelectionGoalReason Reason =
		CountDefenderPlacements(BeforeState, Validation.Defender) == 0
			? EMatchPlayMarkerNoSelectionGoalReason
				::DefenderHasNoDeployedPlayers
			: EMatchPlayMarkerNoSelectionGoalReason
				::NoLegalMarker;
	const EMatchPlayMarkerNoSelectionGoalSource Source =
		EMatchPlayMarkerNoSelectionGoalSource::ResolveNoLegalMarker;
	Result.Reason = Reason;
	Result.Source = Source;
	const FMatchPlayMarkerNoSelectionGoalCapability Capability(
		Request.AttackSequence,
		Reason,
		Source,
		Result.MarkerAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteMarkerGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayResolveNoLegalMarkerErrorCode
				::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayResolveNoLegalMarkerErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}

FMatchPlayMarkerDeclineResult FMatchPlayMarkerDecline::Decline(
	const FMatchPlayState& BeforeState,
	const FMatchPlayMarkerDeclineRequest& Request)
{
	using namespace MatchPlayMarkerNoSelectionGoalImplementation;

	FMatchPlayMarkerDeclineResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	const FCommonValidationResult Validation =
		ValidateCommon(BeforeState, Request.AttackSequence);
	Result.SelectionStateValidationResult =
		Validation.SelectionStateValidationResult;
	if (!Validation.bSuccess)
	{
		Result.ErrorCode = MapDeclineError(Validation.Error);
		Result.ErrorMessage = Validation.ErrorMessage;
		return Result;
	}
	if (!IsMarkerGoalPlayer(Request.RequestingSide))
	{
		Result.ErrorCode =
			EMatchPlayMarkerDeclineErrorCode::InvalidRequestingSide;
		Result.ErrorMessage =
			TEXT("RequestingSide must be PlayerA or PlayerB.");
		return Result;
	}
	if (Request.RequestingSide != Validation.Defender)
	{
		Result.ErrorCode =
			EMatchPlayMarkerDeclineErrorCode
				::RequestingSideIsNotCurrentDefender;
		Result.ErrorMessage =
			TEXT("Only the current defender may decline marker selection.");
		return Result;
	}

	Result.MarkerAvailabilityResult =
		FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide);
	if (!Result.MarkerAvailabilityResult.bQuerySucceeded
		|| Result.MarkerAvailabilityResult
			.bHasGlobalBlockingLegalityResult)
	{
		Result.ErrorCode =
			EMatchPlayMarkerDeclineErrorCode
				::MarkerAvailabilityFailed;
		Result.ErrorMessage =
			Result.MarkerAvailabilityResult
				.bHasGlobalBlockingLegalityResult
				? Result.MarkerAvailabilityResult
					.GlobalBlockingLegalityResult.ErrorMessage
				: TEXT("Marker availability query failed.");
		return Result;
	}
	if (!Result.MarkerAvailabilityResult.bCanSelectAnyMarker)
	{
		Result.ErrorCode =
			EMatchPlayMarkerDeclineErrorCode
				::NoLegalMarkerToDecline;
		Result.ErrorMessage =
			TEXT("Decline requires at least one legal marker; use the no-legal-marker entry instead.");
		return Result;
	}

	Result.Reason =
		EMatchPlayMarkerNoSelectionGoalReason::MarkerDeclined;
	Result.Source =
		EMatchPlayMarkerNoSelectionGoalSource::DeclineMarker;
	const FMatchPlayMarkerNoSelectionGoalCapability Capability(
		Request.AttackSequence,
		Result.Reason,
		Result.Source,
		Result.MarkerAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteMarkerGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayMarkerDeclineErrorCode::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayMarkerDeclineErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
