#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackExecutor.h"
#include "MatchPlayExecutionSummary.h"

#include "MatchPlayAttackStep.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayAttackStepErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	ExecutionFailed UMETA(DisplayName = "Execution Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAttackStepResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Step")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Step")
	FMatchPlayAttackExecutionResult ExecutionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Step")
	FMatchPlayExecutionSummary ExecutionSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Step")
	EMatchPlayAttackStepErrorCode ErrorCode =
		EMatchPlayAttackStepErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Step")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAttackStep final
{
public:
	static FMatchPlayAttackStepResult ExecuteAttackStep(
		const FMatchPlayState& BeforeMatchPlayState,
		const FMatchPlayAttackRequest& AttackRequest);
};

