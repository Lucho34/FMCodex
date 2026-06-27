#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "CoreRuleEnums.h"
#include "MatchStateTypes.h"

#include "FormulaResolver.generated.h"

UENUM(BlueprintType)
enum class EFormulaWinner : uint8
{
	None UMETA(DisplayName = "None"),
	Attacker UMETA(DisplayName = "Attacker"),
	Defender UMETA(DisplayName = "Defender")
};

UENUM(BlueprintType)
enum class EFormulaWinReason : uint8
{
	None UMETA(DisplayName = "None"),
	HigherFinalValue UMETA(DisplayName = "Higher Final Value"),
	StaminaTieBreaker UMETA(DisplayName = "Stamina Tie Breaker"),
	DefenderWinsEqualStamina UMETA(DisplayName = "Defender Wins Equal Stamina"),
	DefenderWinsGoalkeeperTie UMETA(DisplayName = "Defender Wins Goalkeeper Tie"),
	FastSuppression UMETA(DisplayName = "Fast Suppression")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFormulaSideInput
{
	GENERATED_BODY()

	// Attribute subtotal prepared by the caller. Skill-specific assembly stays outside the resolver.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	float Modifier = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	int32 ComparePoint = 0;

	// Fast suppression is only eligible when this point came from a D6 comparison roll.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	bool bComparePointWasRolledOnD6 = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	TArray<int32> ParticipatingStamina;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFormulaResolverInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	EFormulaType FormulaType = EFormulaType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	FFormulaSideInput Attacker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	FFormulaSideInput Defender;

	// This only affects a tied formula result.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula")
	bool bGoalkeeperParticipated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula|Log")
	FGuid LogId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula|Log")
	int32 TurnIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula|Log")
	FName AttackerPlayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula|Log")
	FName DefenderPlayerId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Formula|Log")
	TArray<FName> InvolvedCardIds;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFormulaResolutionResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	EFormulaType FormulaType = EFormulaType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	float AttackerFinalValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	float DefenderFinalValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	EFormulaWinner Winner = EFormulaWinner::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	EFormulaWinReason WinReason = EFormulaWinReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	bool bIsGoal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	bool bAttackEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	bool bContinueResolution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	bool bFastSuppressionTriggered = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Formula")
	FMatchLogEntry MatchLogEntry;
};

UCLASS()
class FMCODEX_API UFormulaResolver final : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Core Rules|Formula")
	static FFormulaResolutionResult ResolveFormula(const FFormulaResolverInput& Input);

	UFUNCTION(BlueprintPure, Category = "Core Rules|Formula")
	static float RoundToOneDecimal(float Value);

	UFUNCTION(BlueprintPure, Category = "Core Rules|Formula")
	static float DivideByTwo(float Value);

	UFUNCTION(BlueprintPure, Category = "Core Rules|Formula")
	static float Average(float FirstValue, float SecondValue);

	UFUNCTION(BlueprintPure, Category = "Core Rules|Formula")
	static float CalculateGoalkeeperHalf(float GoalkeeperAttribute);
};
