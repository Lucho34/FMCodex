#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"

#include "MatchPlayCurrentAttackResolutionBinding.generated.h"

class FMatchPlayValidatedResolutionPreparationAccess;

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackResolutionBindingErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	SelectionNotComplete UMETA(DisplayName = "Selection Not Complete")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionBindingResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	int64 RequestedAttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	EMatchPlayCurrentAttackResolutionBindingErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolutionBindingErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FMatchPlayCurrentAttackReadyValidationResult
		ReadyValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FMatchPlayCurrentAttackResolutionBindingValue Binding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackResolutionBinding final
{
public:
	static FMatchPlayCurrentAttackResolutionBindingResult Query(
		const FMatchPlayState& MatchPlayState,
		int64 AttackSequence);

private:
	friend class FMatchPlayValidatedResolutionPreparationAccess;

	static bool PopulateFromSuccessfulReadyValidation(
		const FMatchPlayState& MatchPlayState,
		int64 AttackSequence,
		const FMatchPlayCurrentAttackReadyValidationResult&
			ReadyValidationResult,
		FMatchPlayCurrentAttackResolutionBindingResult& OutResult);
};
