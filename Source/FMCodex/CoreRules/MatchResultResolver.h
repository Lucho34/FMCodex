#pragma once

#include "CoreMinimal.h"
#include "MatchEndResolver.h"

#include "MatchResultResolver.generated.h"

UENUM(BlueprintType)
enum class EMatchResultType : uint8
{
	NotEnded UMETA(DisplayName = "Not Ended"),
	HomeWin UMETA(DisplayName = "Home Win"),
	AwayWin UMETA(DisplayName = "Away Win"),
	Draw UMETA(DisplayName = "Draw")
};

UENUM(BlueprintType)
enum class EMatchResultResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	MatchNotEnded UMETA(DisplayName = "Match Not Ended")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchResultResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	bool bIsMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	EMatchResultType ResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	int32 HomeScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	int32 AwayScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	EMatchResultResolveErrorCode ErrorCode =
		EMatchResultResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Result")
	FString ErrorMessage;
};

class FMCODEX_API FMatchResultResolver final
{
public:
	static FMatchResultResolveResult ResolveMatchResult(
		const FMatchRuntimeState& RuntimeState);
};
