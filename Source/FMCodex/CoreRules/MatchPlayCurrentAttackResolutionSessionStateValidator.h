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
	InvalidSessionAttackSequence
		UMETA(DisplayName = "Invalid Session Attack Sequence"),
	SessionAttackSequenceMismatch
		UMETA(DisplayName = "Session Attack Sequence Mismatch"),
	InvalidResolutionStage
		UMETA(DisplayName = "Invalid Resolution Stage"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
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
		const FMatchPlayCurrentAttackState& CurrentAttack,
		const FMatchPlayCurrentAttackResolutionSession*
			ProposedSession = nullptr);
};
