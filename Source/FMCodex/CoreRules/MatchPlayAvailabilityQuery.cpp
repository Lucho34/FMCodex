#include "MatchPlayAvailabilityQuery.h"

namespace MatchPlayAvailabilityQuery
{
	void SetError(
		FMatchPlayAvailabilityResult& Result,
		const EMatchPlayAvailabilityErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsAttackCountStateValid(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount >= 0
			&& PlayerState.UsedAttackCount >= 0
			&& PlayerState.UsedAttackCount
				<= PlayerState.TotalAttackCount;
	}

	EMatchPlayAvailabilityErrorCode MapPlayCardError(
		const EPlayCardResolveErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EPlayCardResolveErrorCode::InvalidPlayerSide:
			return EMatchPlayAvailabilityErrorCode::InvalidPlayer;
		case EPlayCardResolveErrorCode::InvalidCardId:
			return EMatchPlayAvailabilityErrorCode::InvalidCardId;
		case EPlayCardResolveErrorCode::CardNotAvailable:
			return EMatchPlayAvailabilityErrorCode::CardNotAvailable;
		case EPlayCardResolveErrorCode::CardAlreadyUsed:
			return EMatchPlayAvailabilityErrorCode::CardAlreadyUsed;
		case EPlayCardResolveErrorCode::InvalidMatchCardUsageState:
			return EMatchPlayAvailabilityErrorCode::InvalidCardUsageState;
		default:
			return EMatchPlayAvailabilityErrorCode::CardValidationFailed;
		}
	}
}

FMatchPlayAvailabilityResult
FMatchPlayAvailabilityQuery::QueryCanPlayCard(
	const FMatchPlayState& MatchPlayState,
	const EInitialTurnOrderPlayer PlayerSide,
	const FName CardId)
{
	FMatchPlayAvailabilityResult Result;
	Result.PlayerSide = PlayerSide;
	Result.CardId = CardId;

	if (!MatchPlayAvailabilityQuery::IsPlayer(PlayerSide))
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			EMatchPlayAvailabilityErrorCode::InvalidPlayer,
			TEXT("PlayerSide must be PlayerA or PlayerB."));
		return Result;
	}

	const FMatchRuntimeState& RuntimeState = MatchPlayState.RuntimeState;
	if (!RuntimeState.bIsInitialized
		|| !MatchPlayAvailabilityQuery::IsPlayer(
			RuntimeState.CurrentAttackingPlayer)
		|| !MatchPlayAvailabilityQuery::IsAttackCountStateValid(
			RuntimeState.PlayerAState)
		|| !MatchPlayAvailabilityQuery::IsAttackCountStateValid(
			RuntimeState.PlayerBState))
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			EMatchPlayAvailabilityErrorCode::InvalidRuntimeState,
			TEXT("Runtime state must be initialized with valid players and attack counts."));
		return Result;
	}

	const int32 PlayerARemainingAttackCount =
		RuntimeState.PlayerAState.TotalAttackCount
		- RuntimeState.PlayerAState.UsedAttackCount;
	const int32 PlayerBRemainingAttackCount =
		RuntimeState.PlayerBState.TotalAttackCount
		- RuntimeState.PlayerBState.UsedAttackCount;
	Result.PlayerRemainingAttackCount =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? PlayerARemainingAttackCount
			: PlayerBRemainingAttackCount;

	if (PlayerARemainingAttackCount == 0
		&& PlayerBRemainingAttackCount == 0)
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			EMatchPlayAvailabilityErrorCode
				::MatchAlreadyEndedOrNoAttackRemaining,
			TEXT("Neither player has a remaining attack opportunity."));
		return Result;
	}

	if (RuntimeState.CurrentAttackingPlayer != PlayerSide)
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			EMatchPlayAvailabilityErrorCode::NotCurrentAttacker,
			TEXT("Only the current attacking player can play a card."));
		return Result;
	}

	if (Result.PlayerRemainingAttackCount == 0)
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			EMatchPlayAvailabilityErrorCode
				::NoRemainingAttackOpportunity,
			TEXT("The queried player has no remaining attack opportunity."));
		return Result;
	}

	const FPlayCardValidationResult ValidationResult =
		FPlayCardResolver::ValidateCanPlayCard(
			MatchPlayState.CardUsageState,
			PlayerSide,
			CardId);
	Result.UnderlyingPlayCardErrorCode = ValidationResult.ErrorCode;
	if (!ValidationResult.bSuccess)
	{
		MatchPlayAvailabilityQuery::SetError(
			Result,
			MatchPlayAvailabilityQuery::MapPlayCardError(
				ValidationResult.ErrorCode),
			ValidationResult.ErrorMessage);
		return Result;
	}

	Result.bCanPlay = true;
	return Result;
}
