#pragma once

#include "CoreMinimal.h"
#include "MatchEndResolver.h"

#include "GoalResolver.generated.h"

UENUM(BlueprintType)
enum class EGoalResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	InvalidScoringSide UMETA(DisplayName = "Invalid Scoring Side"),
	MatchAlreadyEnded UMETA(DisplayName = "Match Already Ended")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FGoalResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	FMatchRuntimeState UpdatedRuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	EInitialTurnOrderPlayer ScoringSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	int32 PlayerAScoreBefore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	int32 PlayerBScoreBefore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	int32 PlayerAScoreAfter = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	int32 PlayerBScoreAfter = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	EGoalResolveErrorCode ErrorCode = EGoalResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Goal")
	FString ErrorMessage;
};

class FMCODEX_API FGoalResolver final
{
public:
	static FGoalResolveResult RecordGoal(
		const FMatchRuntimeState& RuntimeState,
		EInitialTurnOrderPlayer ScoringSide);
};
