#pragma once

#include "CoreMinimal.h"

#include "MatchPlayStatusQuery.h"

#include "MatchPlayTurnGuard.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayTurnGuardErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchStateNotInitialized UMETA(DisplayName = "Match State Not Initialized"),
	NoRemainingAttackOpportunities
		UMETA(DisplayName = "No Remaining Attack Opportunities"),
	CurrentAttackerHasNoRemainingAttackOpportunity
		UMETA(DisplayName = "Current Attacker Has No Remaining Attack Opportunity"),
	CurrentAttackerHasNoAvailableCards
		UMETA(DisplayName = "Current Attacker Has No Available Cards")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayTurnGuardResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	bool bCanSubmitAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	bool bShouldWaitForExternalAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	bool bNoRemainingAttackOpportunities = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	EInitialTurnOrderPlayer CurrentAttacker =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	int32 CurrentAttackerRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	int32 PlayerARemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	int32 PlayerBRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	int32 CurrentAttackerAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	EMatchPlayTurnGuardErrorCode ErrorCode =
		EMatchPlayTurnGuardErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Turn Guard")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayTurnGuard final
{
public:
	static FMatchPlayTurnGuardResult QueryCanSubmitAttackRequest(
		const FMatchPlayState& MatchPlayState);
};

