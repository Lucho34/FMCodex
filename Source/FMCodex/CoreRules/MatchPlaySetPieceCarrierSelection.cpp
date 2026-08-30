#include "MatchPlaySetPieceCarrierSelection.h"

namespace MatchPlaySetPieceCarrierSelection
{
	bool IsPlayer(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsCarrierType(const ESetPieceSelectedType Type)
	{
		return Type == ESetPieceSelectedType::ShortFreeKick
			|| Type == ESetPieceSelectedType::LongFreeKick
			|| Type == ESetPieceSelectedType::Penalty;
	}

	EMatchPlaySetPieceCarrierRouteStage GetStage(
		const FMatchPlaySetPieceRouteState& Route)
	{
		switch (Route.SelectedType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			return Route.ShortFreeKick.Stage;
		case ESetPieceSelectedType::LongFreeKick:
			return Route.LongFreeKick.Stage;
		case ESetPieceSelectedType::Penalty:
			return Route.Penalty.Stage;
		default:
			return EMatchPlaySetPieceCarrierRouteStage::None;
		}
	}

	void BindCarrier(
		FMatchPlaySetPieceRouteState& Route,
		const FMatchPlaySetPieceParticipantBinding& Binding)
	{
		switch (Route.SelectedType)
		{
		case ESetPieceSelectedType::ShortFreeKick:
			Route.ShortFreeKick.Carrier = Binding;
			Route.ShortFreeKick.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod;
			break;
		case ESetPieceSelectedType::LongFreeKick:
			Route.LongFreeKick.Carrier = Binding;
			Route.LongFreeKick.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod;
			break;
		case ESetPieceSelectedType::Penalty:
			Route.Penalty.Carrier = Binding;
			Route.Penalty.Stage =
				EMatchPlaySetPieceCarrierRouteStage::AwaitingMethod;
			break;
		default:
			break;
		}
	}

	void Fail(
		FMatchPlaySetPieceCarrierSelectionResult& Result,
		const EMatchPlaySetPieceCarrierSelectionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySetPieceCarrierSelectionResult
FMatchPlaySetPieceCarrierSelection::Submit(
	const FMatchPlayState& BeforeState,
	const FMatchPlaySetPieceCarrierSelectionRequest& Request)
{
	using namespace MatchPlaySetPieceCarrierSelection;

	FMatchPlaySetPieceCarrierSelectionResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.Request = Request;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Set Piece Carrier selection requires initialized match play state."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode::NoCurrentAttack,
			TEXT("Set Piece Carrier selection requires a current attack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.AttackSequence <= 0)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack requires a positive AttackSequence."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::InvalidRequestAttackSequence,
			TEXT("Request AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence != BeforeState.CurrentAttack.AttackSequence)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Request AttackSequence does not match the current attack."));
		return Result;
	}
	if (BeforeState.CurrentAttack.RouteKind
		!= EMatchPlayCurrentAttackRouteKind::SetPiece)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode::WrongRoute,
			TEXT("Carrier selection is only valid for the SetPiece route."));
		return Result;
	}

	Result.BeforeRouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(BeforeState);
	if (!Result.BeforeRouteValidationResult.bIsCanonical)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode::InvalidRouteState,
			Result.BeforeRouteValidationResult.ErrorMessage);
		return Result;
	}

	Result.SetPieceType = BeforeState.CurrentAttack.SetPieceRoute.SelectedType;
	if (!IsCarrierType(Result.SetPieceType))
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::UnsupportedSetPieceType,
			TEXT("Only Short Free Kick, Long Free Kick, and Penalty accept a Carrier."));
		return Result;
	}
	if (GetStage(BeforeState.CurrentAttack.SetPieceRoute)
		!= EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode::WrongStage,
			TEXT("Set Piece Carrier selection requires AwaitingCarrier stage."));
		return Result;
	}
	if (!IsPlayer(BeforeState.RuntimeState.CurrentAttackingPlayer))
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	if (!IsPlayer(Request.RequestingSide))
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}
	if (Request.RequestingSide
		!= BeforeState.RuntimeState.CurrentAttackingPlayer)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::RequestingSideNotCurrentAttacker,
			TEXT("Only the current attacking player may submit the Set Piece Carrier."));
		return Result;
	}

	FMatchPlaySetPieceParticipantEligibilityRequest EligibilityRequest;
	EligibilityRequest.ExpectedOwnerSide = Request.RequestingSide;
	EligibilityRequest.CardId = Request.CardId;
	EligibilityRequest.Role = EMatchPlaySetPieceParticipantRole::Carrier;
	Result.EligibilityResult =
		FMatchPlaySetPieceParticipantEligibility::Evaluate(
			BeforeState,
			EligibilityRequest);
	if (!Result.EligibilityResult.bIsEligible)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::ParticipantNotEligible,
			Result.EligibilityResult.ErrorMessage);
		return Result;
	}

	BindCarrier(
		Result.AfterState.CurrentAttack.SetPieceRoute,
		Result.EligibilityResult.Binding);
	Result.AfterRouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(Result.AfterState);
	if (!Result.AfterRouteValidationResult.bIsCanonical)
	{
		Result.AfterState = BeforeState;
		Fail(Result,
			EMatchPlaySetPieceCarrierSelectionErrorCode
				::InvalidCandidateRouteState,
			Result.AfterRouteValidationResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
