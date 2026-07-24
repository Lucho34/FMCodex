#pragma once

#include "CoreMinimal.h"

#include "MatchPlayGoalkeeperDeploymentLegality.h"

#include "MatchPlayGoalkeeperDeploymentAvailability.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	CatalogEnumerationFailed
		UMETA(DisplayName = "Catalog Enumeration Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperDeploymentSlotAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	FMatchPlayGoalkeeperDeploymentLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperDeploymentAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	bool bCanDeployToAnySlot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	TArray<FName> LegalSlotIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	TArray<FMatchPlayGoalkeeperDeploymentSlotAvailability> SlotResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	bool bHasFirstBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	FMatchPlayGoalkeeperDeploymentLegalityResult
		FirstBlockingLegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	EMatchPlayDeploymentSlotCatalogValidationErrorCode
		UnderlyingCatalogValidationErrorCode =
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode ErrorCode =
		EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment Availability")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayGoalkeeperDeploymentAvailability final
{
public:
	static FMatchPlayGoalkeeperDeploymentAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide,
		FName CardId);
};
