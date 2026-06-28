#pragma once

#include "CoreMinimal.h"
#include "CardUsageState.h"

#include "CardUsageResolver.generated.h"

UENUM(BlueprintType)
enum class ECardUsageResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidCardUsageState UMETA(DisplayName = "Invalid Card Usage State"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used"),
	DuplicateCardInAvailable UMETA(DisplayName = "Duplicate Card In Available"),
	DuplicateCardInUsed UMETA(DisplayName = "Duplicate Card In Used"),
	CardExistsInBothAvailableAndUsed
		UMETA(DisplayName = "Card Exists In Both Available And Used")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FCardUsageResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	FCardUsageState UpdatedCardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	ECardUsageResolveErrorCode ErrorCode =
		ECardUsageResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	FString ErrorMessage;
};

class FMCODEX_API FCardUsageResolver final
{
public:
	static FCardUsageResolveResult UseCard(
		const FCardUsageState& CardUsageState,
		FName CardId);
};
