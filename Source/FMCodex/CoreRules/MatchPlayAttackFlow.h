#pragma once

#include "CoreMinimal.h"

#include "FormulaAttackFlow.h"
#include "MatchPlayState.h"

#include "MatchPlayAttackFlow.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayAttackFlowErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	FormulaAttackFlowFailed UMETA(DisplayName = "Formula Attack Flow Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayAttackFlowResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	FMatchPlayState UpdatedMatchPlayState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	FName PlayedCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	EInitialTurnOrderPlayer ActingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	bool bAttackResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	FFormulaResolutionResult FormulaResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	EFormulaAttackFlowErrorCode UnderlyingFormulaAttackFlowErrorCode =
		EFormulaAttackFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	EMatchPlayAttackFlowErrorCode ErrorCode =
		EMatchPlayAttackFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play Attack")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayAttackFlow final
{
public:
	static FMatchPlayAttackFlowResult ResolveMatchPlayAttack(
		const FMatchPlayState& CurrentMatchPlayState,
		FName CardId,
		const FFormulaResolverInput& FormulaInput);
};
