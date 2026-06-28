#pragma once

#include "CoreMinimal.h"
#include "AttackResolutionFlow.h"
#include "PlayCardResolver.h"

#include "AttackCardPlayFlow.generated.h"

UENUM(BlueprintType)
enum class EAttackCardPlayFlowErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	InvalidMatchCardUsageState
		UMETA(DisplayName = "Invalid Match Card Usage State"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	MatchAlreadyEnded UMETA(DisplayName = "Match Already Ended"),
	InvalidCardId UMETA(DisplayName = "Invalid Card ID"),
	PlayCardResolveFailed UMETA(DisplayName = "Play Card Resolve Failed"),
	AttackResolutionFlowFailed
		UMETA(DisplayName = "Attack Resolution Flow Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FAttackCardPlayFlowResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FMatchRuntimeState UpdatedRuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FMatchCardUsageState UpdatedMatchCardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FName PlayedCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	EInitialTurnOrderPlayer ActingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FPlayCardResolveResult PlayCardResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FAttackResolutionFlowResult AttackResolutionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	EAttackCardPlayFlowErrorCode ErrorCode =
		EAttackCardPlayFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Card Play")
	FString ErrorMessage;
};

class FMCODEX_API FAttackCardPlayFlow final
{
public:
	static FAttackCardPlayFlowResult ResolveAttackCardPlay(
		const FMatchRuntimeState& RuntimeState,
		const FMatchCardUsageState& MatchCardUsageState,
		FName CardId,
		bool bGoalScored);
};
