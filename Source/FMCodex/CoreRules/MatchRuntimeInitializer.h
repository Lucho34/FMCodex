#pragma once

#include "CoreMinimal.h"

#include "MatchRuntimeStateTypes.h"

#include "MatchRuntimeInitializer.generated.h"

UENUM(BlueprintType)
enum class EMatchRuntimeInitializeErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	OpeningResultFailed UMETA(DisplayName = "Opening Result Failed"),
	InvalidFirstPlayer UMETA(DisplayName = "Invalid First Player")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchRuntimeInitializeResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	FMatchRuntimeState RuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	TArray<EMatchRuntimeInitializeErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Runtime")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FMatchRuntimeInitializer final
{
public:
	static FMatchRuntimeInitializeResult InitializeRuntimeState(
		const FMatchOpeningResolveResult& OpeningResult);
};
