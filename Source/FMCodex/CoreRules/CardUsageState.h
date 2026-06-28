#pragma once

#include "CoreMinimal.h"

#include "CardUsageState.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FCardUsageState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	TArray<FName> AvailableCardIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	TArray<FName> UsedCardIds;
};
