#pragma once

#include "CoreMinimal.h"

#include "MatchPlayExternalStateView.h"

#include "MatchPlayExternalMatchSetupView.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayExternalMatchSetupView
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	bool bSuccessfullyReadInitializedState = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 HomeAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 HomeUsedCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 AwayAvailableCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 AwayUsedCardCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	EInitialTurnOrderPlayer InitialAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 InitialHomeScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 InitialAwayScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 InitialHomeRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	int32 InitialAwayRemainingAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	bool bIsMatchFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	bool bIsWaitingForExternalAttackRequest = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	FString SetupSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play External Match Setup View")
	FMatchPlayExternalStateView StateView;
};

class FMCODEX_API FMatchPlayExternalMatchSetupViewQuery final
{
public:
	static FMatchPlayExternalMatchSetupView BuildView(
		const FMatchPlayState& InitializedState);
};
