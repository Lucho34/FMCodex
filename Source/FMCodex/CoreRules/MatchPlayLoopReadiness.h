#pragma once

#include "CoreMinimal.h"

#include "MatchPlayTurnGuard.h"

#include "MatchPlayLoopReadiness.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayLoopReadinessCode : uint8
{
	None UMETA(DisplayName = "None"),
	ReadyForExternalAttackRequest
		UMETA(DisplayName = "Ready For External Attack Request"),
	MatchStateNotInitialized UMETA(DisplayName = "Match State Not Initialized"),
	NoRemainingAttackOpportunities
		UMETA(DisplayName = "No Remaining Attack Opportunities"),
	CurrentAttackerHasNoRemainingAttackOpportunity
		UMETA(DisplayName = "Current Attacker Has No Remaining Attack Opportunity"),
	CurrentAttackerHasNoAvailableCards
		UMETA(DisplayName = "Current Attacker Has No Available Cards")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayLoopReadinessResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	bool bCanAcceptExternalAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	bool bShouldWaitForExternalAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	bool bShouldEnterAutomaticLoop = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	EMatchPlayLoopReadinessCode Code =
		EMatchPlayLoopReadinessCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Loop Readiness")
	FMatchPlayTurnGuardResult TurnGuardResult;
};

class FMCODEX_API FMatchPlayLoopReadiness final
{
public:
	static FMatchPlayLoopReadinessResult Evaluate(
		const FMatchPlayState& State);
};

