#include "PlayCardResolver.h"

namespace PlayCardResolver
{
	void SetError(
		FPlayCardResolveResult& Result,
		const EPlayCardResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsInvalidCardUsageStateError(
		const ECardUsageResolveErrorCode ErrorCode)
	{
		return ErrorCode == ECardUsageResolveErrorCode::InvalidCardUsageState
			|| ErrorCode == ECardUsageResolveErrorCode::DuplicateCardInAvailable
			|| ErrorCode == ECardUsageResolveErrorCode::DuplicateCardInUsed
			|| ErrorCode
				== ECardUsageResolveErrorCode::CardExistsInBothAvailableAndUsed;
	}

	EPlayCardResolveErrorCode MapCardUsageError(
		const ECardUsageResolveErrorCode ErrorCode)
	{
		if (IsInvalidCardUsageStateError(ErrorCode))
		{
			return EPlayCardResolveErrorCode::InvalidMatchCardUsageState;
		}

		switch (ErrorCode)
		{
		case ECardUsageResolveErrorCode::InvalidCardId:
			return EPlayCardResolveErrorCode::InvalidCardId;
		case ECardUsageResolveErrorCode::CardNotAvailable:
			return EPlayCardResolveErrorCode::CardNotAvailable;
		case ECardUsageResolveErrorCode::CardAlreadyUsed:
			return EPlayCardResolveErrorCode::CardAlreadyUsed;
		default:
			return EPlayCardResolveErrorCode::CardUsageResolveFailed;
		}
	}
}

FPlayCardResolveResult FPlayCardResolver::PlayCard(
	const FMatchCardUsageState& MatchCardUsageState,
	const EInitialTurnOrderPlayer PlayerSide,
	const FName CardId)
{
	FPlayCardResolveResult Result;
	Result.UpdatedMatchCardUsageState = MatchCardUsageState;
	Result.PlayerSide = PlayerSide;
	Result.CardId = CardId;

	if (PlayerSide != EInitialTurnOrderPlayer::PlayerA
		&& PlayerSide != EInitialTurnOrderPlayer::PlayerB)
	{
		PlayCardResolver::SetError(
			Result,
			EPlayCardResolveErrorCode::InvalidPlayerSide,
			TEXT("PlayerSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (CardId.IsNone())
	{
		PlayCardResolver::SetError(
			Result,
			EPlayCardResolveErrorCode::InvalidCardId,
			TEXT("CardId cannot be None."));
		return Result;
	}

	const FCardUsageState& SelectedCardUsageState =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? MatchCardUsageState.PlayerACardUsageState
			: MatchCardUsageState.PlayerBCardUsageState;
	Result.CardUsageResult =
		FCardUsageResolver::UseCard(SelectedCardUsageState, CardId);

	if (!Result.CardUsageResult.bSuccess)
	{
		PlayCardResolver::SetError(
			Result,
			PlayCardResolver::MapCardUsageError(
				Result.CardUsageResult.ErrorCode),
			FString::Printf(
				TEXT("Card usage resolution failed for %s: %s"),
				PlayerSide == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PlayerA")
					: TEXT("PlayerB"),
				*Result.CardUsageResult.ErrorMessage));
		return Result;
	}

	FCardUsageState& UpdatedSelectedCardUsageState =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			: Result.UpdatedMatchCardUsageState.PlayerBCardUsageState;
	UpdatedSelectedCardUsageState =
		Result.CardUsageResult.UpdatedCardUsageState;
	Result.bSuccess = true;
	return Result;
}
