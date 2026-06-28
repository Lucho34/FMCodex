#pragma once

#include "CoreMinimal.h"
#include "AttackCardPlayFlow.h"
#include "FormulaResolver.h"

#include "FormulaAttackFlow.generated.h"

UENUM(BlueprintType)
enum class EFormulaAttackFlowErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	InvalidMatchCardUsageState
		UMETA(DisplayName = "Invalid Match Card Usage State"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	MatchAlreadyEnded UMETA(DisplayName = "Match Already Ended"),
	PlayCardValidationFailed
		UMETA(DisplayName = "Play Card Validation Failed"),
	FormulaResolveFailed UMETA(DisplayName = "Formula Resolve Failed"),
	AttackCardPlayFlowFailed
		UMETA(DisplayName = "Attack Card Play Flow Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFormulaAttackFlowResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FMatchRuntimeState UpdatedRuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FMatchCardUsageState UpdatedMatchCardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FName PlayedCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	EInitialTurnOrderPlayer ActingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FFormulaResolutionResult FormulaResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	EPlayCardResolveErrorCode PlayCardValidationErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FAttackCardPlayFlowResult AttackCardPlayResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	EFormulaAttackFlowErrorCode ErrorCode =
		EFormulaAttackFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula Attack")
	FString ErrorMessage;
};

class FMCODEX_API FFormulaAttackFlow final
{
public:
	static FFormulaAttackFlowResult ResolveFormulaAttack(
		const FMatchRuntimeState& RuntimeState,
		const FMatchCardUsageState& MatchCardUsageState,
		FName CardId,
		const FFormulaResolverInput& FormulaInput);
};
