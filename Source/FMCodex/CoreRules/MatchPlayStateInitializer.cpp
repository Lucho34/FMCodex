#include "MatchPlayStateInitializer.h"

FMatchPlayStateInitializeResult
FMatchPlayStateInitializer::InitializeMatchPlayState(
	const FMatchRuntimeState& RuntimeState,
	const TArray<FName>& PlayerACardIds,
	const TArray<FName>& PlayerBCardIds)
{
	FMatchPlayStateInitializeResult Result;

	const FMatchCardUsageInitializeResult CardUsageResult =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			PlayerACardIds,
			PlayerBCardIds);
	if (!CardUsageResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayStateInitializeErrorCode::CardUsageInitializationFailed;
		Result.UnderlyingCardUsageErrorCode = CardUsageResult.ErrorCode;
		Result.ErrorMessage = CardUsageResult.ErrorMessage;
		return Result;
	}

	Result.MatchPlayState = FMatchPlayState::Create(
		RuntimeState,
		CardUsageResult.InitializedCardUsageState);
	Result.bSuccess = true;
	return Result;
}
