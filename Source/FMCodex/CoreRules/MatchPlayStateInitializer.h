#pragma once

#include "CoreMinimal.h"

#include "MatchCardUsageInitializer.h"
#include "MatchPlayState.h"

#include "MatchPlayStateInitializer.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayStateInitializeErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	CardUsageInitializationFailed
		UMETA(DisplayName = "Card Usage Initialization Failed"),
	DeploymentSlotCatalogValidationFailed
		UMETA(DisplayName = "Deployment Slot Catalog Validation Failed"),
	CardSnapshotAuthorityInitializationFailed
		UMETA(DisplayName = "Card Snapshot Authority Initialization Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayStateInitializeResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayState MatchPlayState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EMatchPlayStateInitializeErrorCode ErrorCode =
		EMatchPlayStateInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EMatchCardUsageInitializeErrorCode UnderlyingCardUsageErrorCode =
		EMatchCardUsageInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EMatchPlayDeploymentSlotCatalogValidationErrorCode
		UnderlyingDeploymentSlotCatalogValidationErrorCode =
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EMatchPlayCardSnapshotAuthorityBuildErrorCode
		UnderlyingCardSnapshotAuthorityBuildErrorCode =
			EMatchPlayCardSnapshotAuthorityBuildErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EInitialTurnOrderPlayer UnderlyingCardSnapshotAuthorityFailingPlayerSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EDeckValidationErrorCode UnderlyingDeckValidationErrorCode =
		EDeckValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	EPlayerCardRuleSnapshotValidationErrorCode
		UnderlyingPlayerCardRuleSnapshotValidationErrorCode =
			EPlayerCardRuleSnapshotValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayStateInitializer final
{
public:
	static FMatchPlayStateInitializeResult InitializeMatchPlayState(
		const FMatchRuntimeState& RuntimeState,
		const TArray<FPlayerCardData>& PlayerADeck,
		const TArray<FPlayerCardData>& PlayerBDeck,
		const FMatchPlayDeploymentSlotCatalog& DeploymentSlotCatalog);
};
