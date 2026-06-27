#pragma once

#include "CoreMinimal.h"

#include "CoreRuleEnums.h"
#include "PlayerCardTypes.h"

#include "DeckValidator.generated.h"

UENUM(BlueprintType)
enum class EDeckValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidCardCount UMETA(DisplayName = "Invalid Card Count"),
	NoGoalkeeper UMETA(DisplayName = "No Goalkeeper"),
	MultipleGoalkeepers UMETA(DisplayName = "Multiple Goalkeepers"),
	GoalkeeperHasMixedPositionTypes UMETA(DisplayName = "Goalkeeper Has Mixed Position Types"),
	NonGoalkeeperContainsGoalkeeperPosition
		UMETA(DisplayName = "Non-Goalkeeper Contains Goalkeeper Position")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FDeckValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Deck")
	bool bIsValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Deck")
	TArray<EDeckValidationErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Deck")
	TArray<FString> ErrorMessages;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Deck")
	int32 InitialDeckRarityScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Deck")
	FString GoalkeeperCardId;
};

class FMCODEX_API FDeckValidator final
{
public:
	static FDeckValidationResult ValidateDeck(const TArray<FPlayerCardData>& Deck);
	static int32 GetRarityScore(ECardRarity Rarity);
};