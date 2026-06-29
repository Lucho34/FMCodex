#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackExecutor.h"
#include "MatchPlayStatusQuery.h"

#include "MatchPlayExecutionSummary.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayExecutionSummary
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bExecutionSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bAttackResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	EInitialTurnOrderPlayer RequestPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	FMatchPlayStatus BeforeStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bHasAfterStatus = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	FMatchPlayStatus AfterStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bValidationFailed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	bool bAttackFlowFailed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	EMatchPlayAttackExecutionErrorCode ErrorCode =
		EMatchPlayAttackExecutionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Execution Summary")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayExecutionSummaryBuilder final
{
public:
	static FMatchPlayExecutionSummary BuildSummary(
		const FMatchPlayState& BeforeMatchPlayState,
		const FMatchPlayAttackExecutionResult& ExecutionResult);
};
