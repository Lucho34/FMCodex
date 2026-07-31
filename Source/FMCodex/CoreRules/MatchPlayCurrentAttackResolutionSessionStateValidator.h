#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackResolutionSessionStateValidator.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
	: uint8
{
	None UMETA(DisplayName = "None"),
	AbsentSessionHasPayload
		UMETA(DisplayName = "Absent Session Has Payload"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	InvalidSessionAttackSequence
		UMETA(DisplayName = "Invalid Session Attack Sequence"),
	SessionAttackSequenceMismatch
		UMETA(DisplayName = "Session Attack Sequence Mismatch"),
	InvalidResolutionStage
		UMETA(DisplayName = "Invalid Resolution Stage"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	MissingSelectedAction UMETA(DisplayName = "Missing Selected Action"),
	NonDefaultActionPreparation
		UMETA(DisplayName = "Non-Default Action Preparation"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	InvalidRuntimeAttackingPlayer
		UMETA(DisplayName = "Invalid Runtime Attacking Player"),
	RuntimeAttackerMismatch
		UMETA(DisplayName = "Runtime Attacker Mismatch"),
	RuntimeDefenderMismatch
		UMETA(DisplayName = "Runtime Defender Mismatch"),
	InvalidSessionBundle UMETA(DisplayName = "Invalid Session Bundle")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bIsCanonical = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackResolutionSessionStateValidator final
{
public:
	static
		FMatchPlayCurrentAttackResolutionSessionStateValidationResult
	Validate(
		const FMatchPlayState& State,
		const FMatchPlayCurrentAttackResolutionSession*
			ProposedSession = nullptr);
};
