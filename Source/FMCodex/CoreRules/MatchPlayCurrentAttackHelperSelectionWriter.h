#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackHelperSelectionLegality.h"
#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#include "MatchPlayCurrentAttackHelperSelectionWriter.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackHelperSelectionWriterErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	LegalityFailed UMETA(DisplayName = "Legality Failed"),
	SelectionStateValidationFailed
		UMETA(DisplayName = "Selection State Validation Failed"),
	ReadyValidationFailed UMETA(DisplayName = "Ready Validation Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionWriterResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayCurrentAttackHelperSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayCurrentAttackHelperSelectionLegalityResult LegalityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayCurrentAttackReadyValidationResult ReadyValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	EMatchPlayCurrentAttackHelperSelectionWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionWriterErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Helper Selection Writer")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackHelperSelectionWriter final
{
public:
	static FMatchPlayCurrentAttackHelperSelectionWriterResult Select(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackHelperSelectionRequest& Request);
};
