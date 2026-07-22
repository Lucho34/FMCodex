#include "MatchPlayStateInitializer.h"

FMatchPlayStateInitializeResult
FMatchPlayStateInitializer::InitializeMatchPlayState(
	const FMatchRuntimeState& RuntimeState,
	const TArray<FPlayerCardData>& PlayerADeck,
	const TArray<FPlayerCardData>& PlayerBDeck,
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

	const FMatchPlayCardSnapshotAuthorityBuildResult AuthorityBuildResult =
		FMatchPlayCardSnapshotAuthorityBuilder::Build(
			PlayerADeck,
			PlayerBDeck);
	if (!AuthorityBuildResult.bSuccess)
	{
		Result.ErrorCode = EMatchPlayStateInitializeErrorCode
			::CardSnapshotAuthorityInitializationFailed;
		Result.UnderlyingCardSnapshotAuthorityBuildErrorCode =
			AuthorityBuildResult.ErrorCode;
		Result.UnderlyingCardSnapshotAuthorityFailingPlayerSide =
			AuthorityBuildResult.FailingPlayerSide;
		Result.UnderlyingDeckValidationErrorCode =
			AuthorityBuildResult.UnderlyingDeckValidationErrorCode;
		Result.UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
			AuthorityBuildResult
				.UnderlyingPlayerCardRuleSnapshotValidationErrorCode;
		Result.ErrorMessage = AuthorityBuildResult.ErrorMessage;
		return Result;
	}

	TArray<FName> DerivedPlayerACardIds;
	DerivedPlayerACardIds.Reserve(AuthorityBuildResult.CardSnapshotAuthority
		.PlayerACardSnapshots.Cards.Num());
	for (const FPlayerCardRuleSnapshot& Snapshot :
		AuthorityBuildResult.CardSnapshotAuthority
			.PlayerACardSnapshots.Cards)
	{
		DerivedPlayerACardIds.Add(Snapshot.CardId);
	}

	TArray<FName> DerivedPlayerBCardIds;
	DerivedPlayerBCardIds.Reserve(AuthorityBuildResult.CardSnapshotAuthority
		.PlayerBCardSnapshots.Cards.Num());
	for (const FPlayerCardRuleSnapshot& Snapshot :
		AuthorityBuildResult.CardSnapshotAuthority
			.PlayerBCardSnapshots.Cards)
	{
		DerivedPlayerBCardIds.Add(Snapshot.CardId);
	}

	const FMatchCardUsageInitializeResult CardUsageResult =
		FMatchCardUsageInitializer::InitializeMatchCardUsageState(
			DerivedPlayerACardIds,
			DerivedPlayerBCardIds);
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
		DeploymentSlotCatalog,
		AuthorityBuildResult.CardSnapshotAuthority);
	Result.bSuccess = true;
	return Result;
}
