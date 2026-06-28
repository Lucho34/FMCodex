#include "MatchInitializer.h"

namespace MatchInitializer
{
	const FName PlayerAId(TEXT("PlayerA"));
	const FName PlayerBId(TEXT("PlayerB"));

	void AppendValidationErrors(
		const TCHAR* PlayerLabel,
		const FDeckValidationResult& Validation,
		TArray<FString>& ErrorMessages)
	{
		for (const FString& ValidationError : Validation.ErrorMessages)
		{
			ErrorMessages.Add(FString::Printf(
				TEXT("%s: %s"),
				PlayerLabel,
				*ValidationError));
		}
	}

	void AddPlayerState(FMatchState& MatchState, const FName PlayerId)
	{
		FPlayerMatchState& PlayerState = MatchState.PlayerStates.AddDefaulted_GetRef();
		PlayerState.PlayerId = PlayerId;
	}
}

FMatchInitializationResult FMatchInitializer::InitializeMatch(
	const TArray<FPlayerCardData>& PlayerADeck,
	const TArray<FPlayerCardData>& PlayerBDeck)
{
	FMatchInitializationResult Result;
	Result.PlayerADeckValidation = FDeckValidator::ValidateDeck(PlayerADeck);
	Result.PlayerBDeckValidation = FDeckValidator::ValidateDeck(PlayerBDeck);

	if (!Result.PlayerADeckValidation.bIsValid)
	{
		MatchInitializer::AppendValidationErrors(
			TEXT("PlayerA"),
			Result.PlayerADeckValidation,
			Result.ErrorMessages);
	}

	if (!Result.PlayerBDeckValidation.bIsValid)
	{
		MatchInitializer::AppendValidationErrors(
			TEXT("PlayerB"),
			Result.PlayerBDeckValidation,
			Result.ErrorMessages);
	}

	if (!Result.PlayerADeckValidation.bIsValid || !Result.PlayerBDeckValidation.bIsValid)
	{
		return Result;
	}

	Result.InitialMatchState.InitialDeckRarityScores.Add(
		MatchInitializer::PlayerAId,
		Result.PlayerADeckValidation.InitialDeckRarityScore);
	Result.InitialMatchState.InitialDeckRarityScores.Add(
		MatchInitializer::PlayerBId,
		Result.PlayerBDeckValidation.InitialDeckRarityScore);

	Result.InitialMatchState.InitialGoalkeeperCardIds.Add(
		MatchInitializer::PlayerAId,
		FName(*Result.PlayerADeckValidation.GoalkeeperCardId));
	Result.InitialMatchState.InitialGoalkeeperCardIds.Add(
		MatchInitializer::PlayerBId,
		FName(*Result.PlayerBDeckValidation.GoalkeeperCardId));

	MatchInitializer::AddPlayerState(Result.InitialMatchState, MatchInitializer::PlayerAId);
	MatchInitializer::AddPlayerState(Result.InitialMatchState, MatchInitializer::PlayerBId);

	Result.bSuccess = true;
	return Result;
}