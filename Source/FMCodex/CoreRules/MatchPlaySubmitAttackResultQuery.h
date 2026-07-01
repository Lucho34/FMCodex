#pragma once

#include "CoreMinimal.h"

#include "MatchPlaySubmitAttackFacade.h"

#include "MatchPlaySubmitAttackResultQuery.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySubmitAttackScoreView
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	int32 PlayerAScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	int32 PlayerBScore = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySubmitAttackResultView
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	bool bSubmittedSuccessfully = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	bool bWasRejectedBeforeExecution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	bool bAttackStepFailed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	bool bScoreChanged = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	EMatchPlaySubmitAttackFacadeCode SubmitCode =
		EMatchPlaySubmitAttackFacadeCode::SubmissionRejected;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	FString Reason;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	FMatchPlaySubmitAttackScoreView BeforeScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	FMatchPlaySubmitAttackScoreView AfterScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	EInitialTurnOrderPlayer BeforeCurrentAttacker =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	EInitialTurnOrderPlayer AfterCurrentAttacker =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Submit Attack Result")
	FMatchPlayExecutionSummary ExecutionSummary;
};

class FMCODEX_API FMatchPlaySubmitAttackResultQuery final
{
public:
	static FMatchPlaySubmitAttackResultView BuildView(
		const FMatchPlaySubmitAttackFacadeResult& Result);
};
