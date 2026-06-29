#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "MatchPlayActionPreview.h"

#include "MatchPlayAttackRequest.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAttackRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Attack Request")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Attack Request")
	FName CardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Attack Request")
	bool bHasExternalFormulaInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play Attack Request")
	FFormulaResolverInput FormulaInput;
};

UENUM(BlueprintType)
enum class EMatchPlayAttackRequestValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRequestPlayer UMETA(DisplayName = "Invalid Request Player"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	MissingFormulaInput UMETA(DisplayName = "Missing Formula Input"),
	CardNotPlayable UMETA(DisplayName = "Card Not Playable"),
	AvailabilityValidationFailed
		UMETA(DisplayName = "Availability Validation Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAttackRequestValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	bool bValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	FMatchPlayAttackRequest AttackRequest;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	FMatchPlayAvailabilityResult AvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	FMatchPlayActionPreviewResult ActionPreviewResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	bool bCanPlay = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	bool bHasExternalFormulaInput = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	EMatchPlayAttackRequestValidationErrorCode ErrorCode =
		EMatchPlayAttackRequestValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Request")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAttackRequestValidator final
{
public:
	static FMatchPlayAttackRequestValidationResult ValidateRequest(
		const FMatchPlayState& MatchPlayState,
		const FMatchPlayAttackRequest& AttackRequest);
};
