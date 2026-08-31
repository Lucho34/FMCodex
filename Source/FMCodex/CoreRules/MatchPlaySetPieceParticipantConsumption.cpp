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
	const ESetPieceSelectedType SelectedType =
		State.CurrentAttack.SetPieceRoute.SelectedType;
	if (SelectedType != ESetPieceSelectedType::ShortFreeKick
		&& SelectedType != ESetPieceSelectedType::LongFreeKick
		&& SelectedType != ESetPieceSelectedType::Penalty
		&& SelectedType != ESetPieceSelectedType::Corner)
	{
		Fail(Result,
			EMatchPlaySetPieceParticipantConsumptionErrorCode
				::UnsupportedSetPieceType,
			TEXT("Only production Set Piece routes expose consumable participants."));
		return Result;
	}
	if (SelectedType == ESetPieceSelectedType::Corner)
	{
		const FMatchPlayCornerRouteState& Corner =
			State.CurrentAttack.SetPieceRoute.Corner;
		if (Corner.Stage != EMatchPlaySetPieceCornerRouteStage::Terminal)
		{
			Fail(Result,
				EMatchPlaySetPieceParticipantConsumptionErrorCode
					::InvalidCornerTerminal,
				TEXT("Corner participants may be consumed only from a canonical terminal route."));
			return Result;
		}

		// Shortage terminals consume nobody. Normal terminals consume only the
		// shared-D6 selected Runner and Helper; non-selected nominees and GK stay Available.
		if (!Corner.AttackerNominees.IsEmpty()
			&& !Corner.DefenderNominees.IsEmpty())
		{
			FMatchPlaySetPieceParticipantToConsume Runner;
			Runner.OwnerSide = Corner.Runner.OwnerSide;
			Runner.CardId = Corner.Runner.CardId;
			Result.Participants.Add(Runner);

			FMatchPlaySetPieceParticipantToConsume Helper;
			Helper.OwnerSide = Corner.Helper.OwnerSide;
			Helper.CardId = Corner.Helper.CardId;
			Result.Participants.Add(Helper);
		}
		Result.bSuccess = true;
		return Result;
	}

	FMatchPlaySetPieceParticipantBinding Carrier;
	bool bNoLegalCarrier = false;
	if (SelectedType == ESetPieceSelectedType::ShortFreeKick)
	{
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
		Carrier = Short.Carrier;
		bNoLegalCarrier = Short.bNoLegalCarrier;
	}
	else if (SelectedType == ESetPieceSelectedType::LongFreeKick)
	{
		const FMatchPlayLongFreeKickRouteState& Long =
			State.CurrentAttack.SetPieceRoute.LongFreeKick;
		if (Long.Stage != EMatchPlaySetPieceCarrierRouteStage::Terminal)
		{
			Fail(Result,
				EMatchPlaySetPieceParticipantConsumptionErrorCode
					::InvalidLongFreeKickTerminal,
				TEXT("Long Free Kick participants may be consumed only from a canonical terminal route."));
			return Result;
		}
		Carrier = Long.Carrier;
		bNoLegalCarrier = Long.bNoLegalCarrier;
	}
	else
	{
		const FMatchPlayPenaltyRouteState& Penalty =
			State.CurrentAttack.SetPieceRoute.Penalty;
		if (Penalty.Stage != EMatchPlaySetPieceCarrierRouteStage::Terminal)
		{
			Fail(Result,
				EMatchPlaySetPieceParticipantConsumptionErrorCode
					::InvalidPenaltyTerminal,
				TEXT("Penalty participants may be consumed only from a canonical terminal route."));
			return Result;
		}
		Carrier = Penalty.Carrier;
		bNoLegalCarrier = Penalty.bNoLegalCarrier;
	}
	if (!bNoLegalCarrier)
	{
		FMatchPlaySetPieceParticipantToConsume Participant;
		Participant.OwnerSide = Carrier.OwnerSide;
		Participant.CardId = Carrier.CardId;
		Result.Participants.Add(Participant);
	}

	Result.bSuccess = true;
	return Result;
}
