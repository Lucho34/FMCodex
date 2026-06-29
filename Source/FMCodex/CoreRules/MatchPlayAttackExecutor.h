#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackFlow.h"
#include "MatchPlayAttackRequest.h"

#include "MatchPlayAttackExecutor.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayAttackExecutionErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	RequestValidationFailed UMETA(DisplayName = "Request Validation Failed"),
	AttackFlowFailed UMETA(DisplayName = "Attack Flow Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAttackExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	FMatchPlayState UpdatedMatchPlayState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	FMatchPlayAttackRequest AttackRequest;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	FMatchPlayAttackRequestValidationResult ValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	FMatchPlayAttackFlowResult AttackFlowResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	bool bAttackResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	EMatchPlayAttackRequestValidationErrorCode
		UnderlyingRequestValidationErrorCode =
			EMatchPlayAttackRequestValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	EMatchPlayAttackFlowErrorCode UnderlyingMatchPlayAttackFlowErrorCode =
		EMatchPlayAttackFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	EMatchPlayAttackExecutionErrorCode ErrorCode =
		EMatchPlayAttackExecutionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack Executor")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAttackExecutor final
{
public:
	static FMatchPlayAttackExecutionResult ExecuteAttackRequest(
		const FMatchPlayState& CurrentMatchPlayState,
		const FMatchPlayAttackRequest& AttackRequest);
};
