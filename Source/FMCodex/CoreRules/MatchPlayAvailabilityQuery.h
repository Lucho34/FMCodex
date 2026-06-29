#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayAvailabilityQuery.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayAvailabilityErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	InvalidPlayer UMETA(DisplayName = "Invalid Player"),
	NotCurrentAttacker UMETA(DisplayName = "Not Current Attacker"),
	NoRemainingAttackOpportunity
		UMETA(DisplayName = "No Remaining Attack Opportunity"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used"),
	InvalidCardUsageState UMETA(DisplayName = "Invalid Card Usage State"),
	CardValidationFailed UMETA(DisplayName = "Card Validation Failed"),
	MatchAlreadyEndedOrNoAttackRemaining
		UMETA(DisplayName = "Match Already Ended Or No Attack Remaining")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAvailabilityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	bool bCanPlay = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	int32 PlayerRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	EPlayCardResolveErrorCode UnderlyingPlayCardErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	EMatchPlayAvailabilityErrorCode ErrorCode =
		EMatchPlayAvailabilityErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Availability")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAvailabilityQuery final
{
public:
	static FMatchPlayAvailabilityResult QueryCanPlayCard(
		const FMatchPlayState& MatchPlayState,
		EInitialTurnOrderPlayer PlayerSide,
		FName CardId);
};
