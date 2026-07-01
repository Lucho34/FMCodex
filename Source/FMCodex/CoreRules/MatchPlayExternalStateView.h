#pragma once

#include "CoreMinimal.h"

#include "MatchPlayLoopReadiness.h"
#include "MatchPlayStatusQuery.h"

#include "MatchPlayExternalStateView.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayExternalStateView
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	bool bIsRuntimeInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 HomeScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 AwayScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	bool bIsMatchFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	bool bIsWaitingForExternalAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	bool bCanSubmitAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	EMatchPlayLoopReadinessCode ReadinessCode =
		EMatchPlayLoopReadinessCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	FString StatusSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	FString CannotSubmitReason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 HomeRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 AwayRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 HomeAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 HomeUsedCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 AwayAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	int32 AwayUsedCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	TArray<FName> HomeAvailableCardIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	TArray<FName> HomeUsedCardIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	TArray<FName> AwayAvailableCardIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	TArray<FName> AwayUsedCardIds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	FMatchPlayStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External State View")
	FMatchPlayLoopReadinessResult LoopReadinessResult;
};

class FMCODEX_API FMatchPlayExternalStateViewQuery final
{
public:
	static FMatchPlayExternalStateView BuildView(
		const FMatchPlayState& State);
};
