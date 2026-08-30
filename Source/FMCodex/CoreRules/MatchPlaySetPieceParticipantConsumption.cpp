#include "MatchPlaySetPieceParticipantConsumption.h"

namespace MatchPlaySetPieceParticipantConsumption
{
	void Fail(
		FMatchPlaySetPieceParticipantConsumptionResult& Result,
		const EMatchPlaySetPieceParticipantConsumptionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySetPieceParticipantConsumptionResult
FMatchPlaySetPieceParticipantConsumption::Extract(
	const FMatchPlayState& State)
{
	using namespace MatchPlaySetPieceParticipantConsumption;

	FMatchPlaySetPieceParticipantConsumptionResult Result;
	if (!State.bHasCurrentAttack)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode::NoCurrentAttack,
			TEXT("Set Piece participant consumption requires a current attack."));
		return Result;
	}
	if (State.CurrentAttack.RouteKind
		!= EMatchPlayCurrentAttackRouteKind::SetPiece)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode::WrongRoute,
			TEXT("Set Piece participant consumption requires the SetPiece route."));
		return Result;
	}

	Result.RouteValidationResult =
		FMatchPlayCurrentAttackRouteStateValidator::Validate(State);
	if (!Result.RouteValidationResult.bIsCanonical)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode
				::InvalidRouteState,
			Result.RouteValidationResult.ErrorMessage);
		return Result;
	}
	if (State.CurrentAttack.SetPieceRoute.SelectedType
		!= ESetPieceSelectedType::ShortFreeKick)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode
				::UnsupportedSetPieceType,
			TEXT("Only the production Short Free Kick route currently exposes consumable Set Piece participants."));
		return Result;
	}

	const FMatchPlayShortFreeKickRouteState& Short =
		State.CurrentAttack.SetPieceRoute.ShortFreeKick;
	if (Short.Stage != EMatchPlaySetPieceCarrierRouteStage::Terminal)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode
				::InvalidShortFreeKickTerminal,
			TEXT("Short Free Kick participants may be consumed only from a canonical terminal route."));
		return Result;
	}
	if (!Short.bNoLegalCarrier)
	{
		FMatchPlaySetPieceParticipantToConsume Participant;
		Participant.OwnerSide = Short.Carrier.OwnerSide;
		Participant.CardId = Short.Carrier.CardId;
		Result.Participants.Add(Participant);
	}

	Result.bSuccess = true;
	return Result;
}
