#pragma once

#include "CoreMinimal.h"
#include "CardUsageResolver.h"
#include "InitialTurnOrderResolver.h"

#include "PlayCardResolver.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchCardUsageState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FCardUsageState PlayerACardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FCardUsageState PlayerBCardUsageState;
};

UENUM(BlueprintType)
enum class EPlayCardResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidMatchCardUsageState
		UMETA(DisplayName = "Invalid Match Card Usage State"),
	InvalidPlayerSide UMETA(DisplayName = "Invalid Player Side"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	CardUsageResolveFailed UMETA(DisplayName = "Card Usage Resolve Failed"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayCardValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	EPlayCardResolveErrorCode ErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	ECardUsageResolveErrorCode CardUsageErrorCode =
		ECardUsageResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FString CardUsageErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FPlayCardResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FMatchCardUsageState UpdatedMatchCardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FCardUsageResolveResult CardUsageResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	EPlayCardResolveErrorCode ErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Play Card")
	FString ErrorMessage;
};

class FMCODEX_API FPlayCardResolver final
{
public:
	static FPlayCardValidationResult ValidateCanPlayCard(
		const FMatchCardUsageState& MatchCardUsageState,
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId);

	static FPlayCardResolveResult PlayCard(
		const FMatchCardUsageState& MatchCardUsageState,
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId);
};
