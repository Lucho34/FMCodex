#include "MatchCardUsageInitializer.h"

namespace MatchCardUsageInitializer
{
	bool ValidateCardIds(
		const TArray<FName>& CardIds,
		const TCHAR* PlayerName,
		const EMatchCardUsageInitializeErrorCode InvalidCardIdError,
		const EMatchCardUsageInitializeErrorCode DuplicateCardIdError,
		FMatchCardUsageInitializeResult& OutResult)
	{
		TSet<FName> UniqueCardIds;
		for (int32 Index = 0; Index < CardIds.Num(); ++Index)
		{
			const FName CardId = CardIds[Index];
			if (CardId.IsNone())
			{
				OutResult.ErrorCode = InvalidCardIdError;
				OutResult.ErrorMessage = FString::Printf(
					TEXT("%s CardId at index %d cannot be None."),
					PlayerName,
					Index);
				return false;
			}

			if (UniqueCardIds.Contains(CardId))
			{
				OutResult.ErrorCode = DuplicateCardIdError;
				OutResult.ErrorMessage = FString::Printf(
					TEXT("%s contains duplicate CardId '%s'."),
					PlayerName,
					*CardId.ToString());
				return false;
			}

			UniqueCardIds.Add(CardId);
		}

		return true;
	}
}

FMatchCardUsageInitializeResult
FMatchCardUsageInitializer::InitializeMatchCardUsageState(
	const TArray<FName>& PlayerACardIds,
	const TArray<FName>& PlayerBCardIds)
{
	FMatchCardUsageInitializeResult Result;

	if (!MatchCardUsageInitializer::ValidateCardIds(
		PlayerACardIds,
		TEXT("PlayerA"),
		EMatchCardUsageInitializeErrorCode::InvalidPlayerACardId,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerACardId,
		Result))
	{
		return Result;
	}

	if (!MatchCardUsageInitializer::ValidateCardIds(
		PlayerBCardIds,
		TEXT("PlayerB"),
		EMatchCardUsageInitializeErrorCode::InvalidPlayerBCardId,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerBCardId,
		Result))
	{
		return Result;
	}

	Result.InitializedCardUsageState.PlayerACardUsageState.AvailableCardIds =
		PlayerACardIds;
	Result.InitializedCardUsageState.PlayerACardUsageState.UsedCardIds.Reset();
	Result.InitializedCardUsageState.PlayerBCardUsageState.AvailableCardIds =
		PlayerBCardIds;
	Result.InitializedCardUsageState.PlayerBCardUsageState.UsedCardIds.Reset();
	Result.bSuccess = true;
	return Result;
}
