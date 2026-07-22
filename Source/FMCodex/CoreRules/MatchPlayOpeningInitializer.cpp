#include "MatchPlayOpeningInitializer.h"

namespace MatchPlayOpeningInitializer
{
	FString MakeErrorMessage(
		const TArray<FString>& ErrorMessages,
		const TCHAR* FallbackMessage)
	{
		return ErrorMessages.IsEmpty()
			? FString(FallbackMessage)
			: FString::Join(ErrorMessages, TEXT(" | "));
	}
}

FMatchPlayOpeningInitializeResult
FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(
	const FMatchPlayOpeningInitializeInput& Input)
{
	FMatchPlayOpeningInitializeResult Result;

	const FMatchOpeningResolveResult OpeningResult =
		FMatchOpeningResolver::ResolveMatchOpening(Input.OpeningInput);
	if (!OpeningResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayOpeningInitializeErrorCode::OpeningResolveFailed;
		Result.UnderlyingOpeningErrorCodes = OpeningResult.ErrorCodes;
		Result.bRequiresTieBreakerReroll =
			OpeningResult.InitialTurnOrderResult.bRequiresTieBreakerReroll;
		Result.ErrorMessage = MatchPlayOpeningInitializer::MakeErrorMessage(
			OpeningResult.ErrorMessages,
			TEXT("Match opening resolution failed."));
		return Result;
	}

	const FMatchRuntimeInitializeResult RuntimeResult =
		FMatchRuntimeInitializer::InitializeRuntimeState(OpeningResult);
	if (!RuntimeResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayOpeningInitializeErrorCode::RuntimeInitializationFailed;
		Result.UnderlyingRuntimeInitializeErrorCodes =
			RuntimeResult.ErrorCodes;
		Result.ErrorMessage = MatchPlayOpeningInitializer::MakeErrorMessage(
			RuntimeResult.ErrorMessages,
			TEXT("Match runtime initialization failed."));
		return Result;
	}

	const FMatchPlayStateInitializeResult PlayStateResult =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			RuntimeResult.RuntimeState,
			Input.OpeningInput.PlayerADeck,
			Input.OpeningInput.PlayerBDeck,
			Input.DeploymentSlotCatalog);
	if (!PlayStateResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayOpeningInitializeErrorCode::PlayStateInitializationFailed;
		Result.UnderlyingPlayStateInitializeErrorCode =
			PlayStateResult.ErrorCode;
		Result.UnderlyingCardUsageErrorCode =
			PlayStateResult.UnderlyingCardUsageErrorCode;
		Result.UnderlyingDeploymentSlotCatalogValidationErrorCode =
			PlayStateResult
				.UnderlyingDeploymentSlotCatalogValidationErrorCode;
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode =
			PlayStateResult
				.UnderlyingCardSnapshotAuthorityBuildErrorCode;
		Result.UnderlyingCardSnapshotAuthorityFailingPlayerSide =
			PlayStateResult
				.UnderlyingCardSnapshotAuthorityFailingPlayerSide;
		Result.UnderlyingDeckValidationErrorCode =
			PlayStateResult.UnderlyingDeckValidationErrorCode;
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
			PlayStateResult
				.UnderlyingPlayerCardRuleSnapshotValidationErrorCode;
		Result.ErrorMessage = PlayStateResult.ErrorMessage;
		return Result;
	}

	Result.MatchPlayState = PlayStateResult.MatchPlayState;
	Result.bSuccess = true;
	return Result;
}
