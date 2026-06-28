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

FPlayCardValidationResult FPlayCardResolver::ValidateCanPlayCard(
	const FMatchCardUsageState& MatchCardUsageState,
	const EInitialTurnOrderPlayer PlayerSide,
	const FName CardId)
{
	FPlayCardValidationResult Result;
	Result.PlayerSide = PlayerSide;
	Result.CardId = CardId;

	if (PlayerSide != EInitialTurnOrderPlayer::PlayerA
		&& PlayerSide != EInitialTurnOrderPlayer::PlayerB)
	{
		Result.ErrorCode =
			EPlayCardResolveErrorCode::InvalidPlayerSide;
		Result.ErrorMessage =
			TEXT("PlayerSide must be PlayerA or PlayerB.");
		return Result;
	}

	if (CardId.IsNone())
	{
		Result.ErrorCode =
			EPlayCardResolveErrorCode::InvalidCardId;
		Result.ErrorMessage = TEXT("CardId cannot be None.");
		return Result;
	}

	const FCardUsageState& SelectedCardUsageState =
		PlayerSide == EInitialTurnOrderPlayer::PlayerA
			? MatchCardUsageState.PlayerACardUsageState
			: MatchCardUsageState.PlayerBCardUsageState;
	const FCardUsageResolveResult CardUsageResult =
		FCardUsageResolver::UseCard(
			SelectedCardUsageState,
			CardId);
	if (!CardUsageResult.bSuccess)
	{
		Result.ErrorCode =
			PlayCardResolver::MapCardUsageError(
				CardUsageResult.ErrorCode);
		Result.CardUsageErrorCode =
			CardUsageResult.ErrorCode;
		Result.CardUsageErrorMessage =
			CardUsageResult.ErrorMessage;
		Result.ErrorMessage = FString::Printf(
			TEXT("Card usage validation failed for %s: %s"),
			PlayerSide == EInitialTurnOrderPlayer::PlayerA
				? TEXT("PlayerA")
				: TEXT("PlayerB"),
			*CardUsageResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	return Result;
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

	const FPlayCardValidationResult ValidationResult =
		ValidateCanPlayCard(
			MatchCardUsageState,
			PlayerSide,
			CardId);
	if (!ValidationResult.bSuccess)
	{
		PlayCardResolver::SetError(
			Result,
			ValidationResult.ErrorCode,
			ValidationResult.ErrorMessage);

		if (ValidationResult.CardUsageErrorCode
			!= ECardUsageResolveErrorCode::None)
		{
			const FCardUsageState& SelectedCardUsageState =
				PlayerSide == EInitialTurnOrderPlayer::PlayerA
					? MatchCardUsageState.PlayerACardUsageState
					: MatchCardUsageState.PlayerBCardUsageState;
			Result.CardUsageResult.UpdatedCardUsageState =
				SelectedCardUsageState;
			Result.CardUsageResult.CardId = CardId;
			Result.CardUsageResult.ErrorCode =
				ValidationResult.CardUsageErrorCode;
			Result.CardUsageResult.ErrorMessage =
				ValidationResult.CardUsageErrorMessage;
		}
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
