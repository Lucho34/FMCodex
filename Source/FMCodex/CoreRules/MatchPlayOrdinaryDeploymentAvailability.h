#pragma once

#include "CoreMinimal.h"

#include "MatchPlayOrdinaryDeploymentLegality.h"

#include "MatchPlayOrdinaryDeploymentAvailability.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayOrdinaryDeploymentAvailabilityErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	CatalogEnumerationFailed
		UMETA(DisplayName = "Catalog Enumeration Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOrdinaryDeploymentSlotAvailability
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	FMatchPlayOrdinaryDeploymentLegalityResult LegalityResult;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOrdinaryDeploymentAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	bool bQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	bool bCanDeployToAnySlot = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	TArray<FName> LegalSlotIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	TArray<FMatchPlayOrdinaryDeploymentSlotAvailability> SlotResults;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	bool bHasFirstBlockingLegalityResult = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	FMatchPlayOrdinaryDeploymentLegalityResult
		FirstBlockingLegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	EMatchPlayDeploymentSlotCatalogValidationErrorCode
		UnderlyingCatalogValidationErrorCode =
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	EMatchPlayOrdinaryDeploymentAvailabilityErrorCode ErrorCode =
		EMatchPlayOrdinaryDeploymentAvailabilityErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment Availability")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayOrdinaryDeploymentAvailability final
{
public:
	static FMatchPlayOrdinaryDeploymentAvailabilityResult Query(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide,
		FName CardId);
};
