#pragma once

#include "CoreMinimal.h"

#include "DeckValidator.h"
#include "MatchStateTypes.h"

#include "MatchInitializer.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchInitializationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Initialization")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Initialization")
	FDeckValidationResult PlayerADeckValidation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Initialization")
	FDeckValidationResult PlayerBDeckValidation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Initialization")
	FMatchState InitialMatchState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Initialization")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FMatchInitializer final
{
public:
	static FMatchInitializationResult InitializeMatch(
		const TArray<FPlayerCardData>& PlayerADeck,
		const TArray<FPlayerCardData>& PlayerBDeck);
};