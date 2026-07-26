#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSelectionStateValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	UnsupportedCurrentAttackPhase
		UMETA(DisplayName = "Unsupported Current Attack Phase"),
	UnsupportedSelectionStage
		UMETA(DisplayName = "Unsupported Selection Stage"),
	SelectionStageDoesNotMatchPhase
		UMETA(DisplayName = "Selection Stage Does Not Match Phase"),
	UnexpectedPreparationCarrier
		UMETA(DisplayName = "Unexpected Preparation Carrier"),
	MissingPreparationCarrier
		UMETA(DisplayName = "Missing Preparation Carrier"),
	SelectedActionUnexpectedlyPresent
		UMETA(DisplayName = "Selected Action Unexpectedly Present"),
	SelectedActionPayloadNotEmpty
		UMETA(DisplayName = "Selected Action Payload Not Empty"),
	DuplicateCarrierAuthority
		UMETA(DisplayName = "Duplicate Carrier Authority"),
	UnexpectedPreparationMarker
		UMETA(DisplayName = "Unexpected Preparation Marker"),
	MissingPreparationMarker
		UMETA(DisplayName = "Missing Preparation Marker")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackSelectionStateValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	bool bIsCanonical = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	EMatchPlayCurrentAttackSelectionStateValidationErrorCode ErrorCode =
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackSelectionStateValidator final
{
public:
	static FMatchPlayCurrentAttackSelectionStateValidationResult Validate(
		const FMatchPlayCurrentAttackState& CurrentAttack);
};
