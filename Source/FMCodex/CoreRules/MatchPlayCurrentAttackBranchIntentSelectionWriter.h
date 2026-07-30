#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBranchIntentSelectionLegality.h"
#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"

#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
	: uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed"),
	ReadyValidationFailed UMETA(DisplayName = "Ready Validation Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FMatchPlayCurrentAttackBranchIntentSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FMatchPlayCurrentAttackBranchIntentSelectionLegalityResult
		LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FMatchPlayCurrentAttackReadyValidationResult
		ReadyValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackBranchIntentSelectionWriterErrorCode
				::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackBranchIntentSelectionWriterResult
		Select(
			const FMatchPlayState& BeforeState,
			const FMatchPlayCurrentAttackBranchIntentSelectionRequest&
				Request);
};
