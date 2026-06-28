#include "CardUsageResolver.h"

namespace CardUsageResolver
{
	void SetError(
		FCardUsageResolveResult& Result,
		const ECardUsageResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool ValidateCardList(
		FCardUsageResolveResult& Result,
		const TArray<FName>& CardIds,
		const ECardUsageResolveErrorCode DuplicateErrorCode,
		const TCHAR* ListName,
		TSet<FName>& OutUniqueCardIds)
	{
		for (const FName CardId : CardIds)
		{
			if (CardId.IsNone())
			{
				SetError(
					Result,
					ECardUsageResolveErrorCode::InvalidCardUsageState,
					FString::Printf(
						TEXT("%s contains an invalid CardId."),
						ListName));
				return false;
			}

			if (OutUniqueCardIds.Contains(CardId))
			{
				SetError(
					Result,
					DuplicateErrorCode,
					FString::Printf(
						TEXT("%s contains duplicate CardId '%s'."),
						ListName,
						*CardId.ToString()));
				return false;
			}

			OutUniqueCardIds.Add(CardId);
		}

		return true;
	}

	bool ValidateState(
		FCardUsageResolveResult& Result,
		const FCardUsageState& CardUsageState)
	{
		TSet<FName> AvailableCardIds;
		if (!ValidateCardList(
			Result,
			CardUsageState.AvailableCardIds,
			ECardUsageResolveErrorCode::DuplicateCardInAvailable,
			TEXT("AvailableCardIds"),
			AvailableCardIds))
		{
			return false;
		}

		TSet<FName> UsedCardIds;
		if (!ValidateCardList(
			Result,
			CardUsageState.UsedCardIds,
			ECardUsageResolveErrorCode::DuplicateCardInUsed,
			TEXT("UsedCardIds"),
			UsedCardIds))
		{
			return false;
		}

		for (const FName CardId : AvailableCardIds)
		{
			if (UsedCardIds.Contains(CardId))
			{
				SetError(
					Result,
					ECardUsageResolveErrorCode::CardExistsInBothAvailableAndUsed,
					FString::Printf(
						TEXT("CardId '%s' exists in both AvailableCardIds and UsedCardIds."),
						*CardId.ToString()));
				return false;
			}
		}

		return true;
	}
}

FCardUsageResolveResult FCardUsageResolver::UseCard(
	const FCardUsageState& CardUsageState,
	const FName CardId)
{
	FCardUsageResolveResult Result;
	Result.UpdatedCardUsageState = CardUsageState;
	Result.CardId = CardId;

	if (CardId.IsNone())
	{
		CardUsageResolver::SetError(
			Result,
			ECardUsageResolveErrorCode::InvalidCardId,
			TEXT("CardId cannot be None."));
		return Result;
	}

	if (!CardUsageResolver::ValidateState(Result, CardUsageState))
	{
		return Result;
	}

	if (CardUsageState.UsedCardIds.Contains(CardId))
	{
		CardUsageResolver::SetError(
			Result,
			ECardUsageResolveErrorCode::CardAlreadyUsed,
			FString::Printf(
				TEXT("CardId '%s' has already been used."),
				*CardId.ToString()));
		return Result;
	}

	const int32 AvailableIndex =
		CardUsageState.AvailableCardIds.IndexOfByKey(CardId);
	if (AvailableIndex == INDEX_NONE)
	{
		CardUsageResolver::SetError(
			Result,
			ECardUsageResolveErrorCode::CardNotAvailable,
			FString::Printf(
				TEXT("CardId '%s' is not available."),
				*CardId.ToString()));
		return Result;
	}

	Result.UpdatedCardUsageState.AvailableCardIds.RemoveAt(AvailableIndex);
	Result.UpdatedCardUsageState.UsedCardIds.Add(CardId);
	Result.bSuccess = true;
	return Result;
}
