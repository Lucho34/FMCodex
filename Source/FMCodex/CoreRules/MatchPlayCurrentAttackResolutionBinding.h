#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackReadyForResolutionValidator.h"

#include "MatchPlayCurrentAttackResolutionBinding.generated.h"

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
struct FMCODEX_API FMatchPlayCurrentAttackResolutionBindingValue
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName HelperCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	EMatchPlayElectiveBranchIntent ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::None;
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
};
