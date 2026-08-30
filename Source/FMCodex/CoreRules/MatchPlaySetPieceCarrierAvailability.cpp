#include "MatchPlaySetPieceCarrierAvailability.h"

namespace MatchPlaySetPieceCarrierAvailability
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

	const FCardUsageState& GetUsage(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	void Fail(
		FMatchPlaySetPieceCarrierAvailabilityResult& Result,
		const EMatchPlaySetPieceCarrierAvailabilityErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySetPieceCarrierAvailabilityResult
FMatchPlaySetPieceCarrierAvailability::Query(
	const FMatchPlayState& State,
	const FMatchPlaySetPieceCarrierAvailabilityRequest& Request)
{
	using namespace MatchPlaySetPieceCarrierAvailability;

	FMatchPlaySetPieceCarrierAvailabilityResult Result;
	Result.Request = Request;
	if (!State.RuntimeState.bIsInitialized)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Set Piece Carrier availability requires initialized match play state."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode::NoCurrentAttack,
			TEXT("Set Piece Carrier availability requires a current attack."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::InvalidAttackSequence,
			TEXT("AttackSequence must be positive."));
		return Result;
	}
	if (Request.AttackSequence != State.CurrentAttack.AttackSequence)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::AttackSequenceMismatch,
			TEXT("AttackSequence does not match the current attack."));
		return Result;
	}
	if (!IsPlayer(State.RuntimeState.CurrentAttackingPlayer))
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	if (State.CurrentAttack.RouteKind
		!= EMatchPlayCurrentAttackRouteKind::SetPiece)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode::WrongRoute,
			TEXT("Carrier availability is only valid for the SetPiece route."));
		return Result;
	}

	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
	if (!Result.RouteValidationResult.bIsCanonical)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::InvalidRouteState,
			Result.RouteValidationResult.ErrorMessage);
		return Result;
	}

	Result.SetPieceType = State.CurrentAttack.SetPieceRoute.SelectedType;
	if (!IsCarrierType(Result.SetPieceType))
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode
				::UnsupportedSetPieceType,
			TEXT("Only Short Free Kick, Long Free Kick, and Penalty use Carrier availability."));
		return Result;
	}
	if (GetStage(State.CurrentAttack.SetPieceRoute)
		!= EMatchPlaySetPieceCarrierRouteStage::AwaitingCarrier)
	{
		Fail(Result,
			EMatchPlaySetPieceCarrierAvailabilityErrorCode::WrongStage,
			TEXT("Set Piece Carrier availability requires AwaitingCarrier stage."));
		return Result;
	}

	Result.AttackingSide = State.RuntimeState.CurrentAttackingPlayer;
	const FCardUsageState& Usage = GetUsage(
		State.CardUsageState,
		Result.AttackingSide);
	Result.LegalCarrierCardIds.Reserve(Usage.AvailableCardIds.Num());
	for (const FName CardId : Usage.AvailableCardIds)
	{
		FMatchPlaySetPieceParticipantEligibilityRequest EligibilityRequest;
		EligibilityRequest.ExpectedOwnerSide = Result.AttackingSide;
		EligibilityRequest.CardId = CardId;
		EligibilityRequest.Role =
			EMatchPlaySetPieceParticipantRole::Carrier;
		FMatchPlaySetPieceParticipantEligibilityResult Eligibility =
			FMatchPlaySetPieceParticipantEligibility::Evaluate(
				State,
				EligibilityRequest);
		Result.ParticipantEligibilityResults.Add(Eligibility);
		if (Eligibility.bIsEligible)
		{
			Result.LegalCarrierCardIds.Add(CardId);
			continue;
		}
		if (Eligibility.ErrorCode
			!= EMatchPlaySetPieceParticipantEligibilityErrorCode
				::GoalkeeperNotEligible)
		{
			Fail(Result,
				EMatchPlaySetPieceCarrierAvailabilityErrorCode
					::ParticipantEligibilityFailed,
				Eligibility.ErrorMessage);
			return Result;
		}
	}

	Result.bHasLegalCarrier = !Result.LegalCarrierCardIds.IsEmpty();
	Result.bSuccess = true;
	return Result;
}
