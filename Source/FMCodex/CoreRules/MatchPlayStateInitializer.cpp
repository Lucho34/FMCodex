#include "MatchPlayStateInitializer.h"

FMatchPlayStateInitializeResult
FMatchPlayStateInitializer::InitializeMatchPlayState(
	const FMatchRuntimeState& RuntimeState,
	const TArray<FName>& PlayerACardIds,
	const TArray<FName>& PlayerBCardIds,
	const FMatchPlayDeploymentSlotCatalog& DeploymentSlotCatalog)
{
	FMatchPlayStateInitializeResult Result;

	const FMatchPlayDeploymentSlotCatalogValidationResult
		DeploymentSlotCatalogValidationResult =
			FMatchPlayDeploymentSlotCatalogValidator::Validate(
				DeploymentSlotCatalog);
	if (!DeploymentSlotCatalogValidationResult.bSuccess)
	{
		Result.ErrorCode = EMatchPlayStateInitializeErrorCode
			::DeploymentSlotCatalogValidationFailed;
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode =
			DeploymentSlotCatalogValidationResult.ErrorCode;
		Result.ErrorMessage =
			DeploymentSlotCatalogValidationResult.ErrorMessage;
		return Result;
	}

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
		CardUsageResult.InitializedCardUsageState,
		DeploymentSlotCatalog);
	Result.bSuccess = true;
	return Result;
}
