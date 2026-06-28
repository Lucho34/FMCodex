#pragma once

#include "CoreMinimal.h"

#include "PlayCardResolver.h"

#include "MatchCardUsageInitializer.generated.h"

UENUM(BlueprintType)
enum class EMatchCardUsageInitializeErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidPlayerACardId UMETA(DisplayName = "Invalid PlayerA Card ID"),
	InvalidPlayerBCardId UMETA(DisplayName = "Invalid PlayerB Card ID"),
	DuplicatePlayerACardId UMETA(DisplayName = "Duplicate PlayerA Card ID"),
	DuplicatePlayerBCardId UMETA(DisplayName = "Duplicate PlayerB Card ID")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchCardUsageInitializeResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	FMatchCardUsageState InitializedCardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	EMatchCardUsageInitializeErrorCode ErrorCode =
		EMatchCardUsageInitializeErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Card Usage")
	FString ErrorMessage;
};

class FMCODEX_API FMatchCardUsageInitializer final
{
public:
	static FMatchCardUsageInitializeResult InitializeMatchCardUsageState(
		const TArray<FName>& PlayerACardIds,
		const TArray<FName>& PlayerBCardIds);
};
