#pragma once

#include "CoreMinimal.h"
#include "AttackOpportunityResolver.h"
#include "GoalResolver.h"
#include "MatchResultResolver.h"

#include "AttackResolutionFlow.generated.h"

UENUM(BlueprintType)
enum class EAttackResolutionFlowErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidRuntimeState UMETA(DisplayName = "Invalid Runtime State"),
	MatchAlreadyEnded UMETA(DisplayName = "Match Already Ended"),
	NoAttackOpportunity UMETA(DisplayName = "No Attack Opportunity"),
	GoalResolveFailed UMETA(DisplayName = "Goal Resolve Failed"),
	AttackOpportunityResolveFailed
		UMETA(DisplayName = "Attack Opportunity Resolve Failed"),
	MatchEndResolveFailed UMETA(DisplayName = "Match End Resolve Failed"),
	MatchResultResolveFailed UMETA(DisplayName = "Match Result Resolve Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FAttackResolutionFlowResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	FMatchRuntimeState UpdatedRuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	bool bGoalScored = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	EInitialTurnOrderPlayer ScoringSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	EMatchResultType MatchResultType = EMatchResultType::NotEnded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	EAttackResolutionFlowErrorCode ErrorCode =
		EAttackResolutionFlowErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Resolution")
	FString ErrorMessage;
};

class FMCODEX_API FAttackResolutionFlow final
{
public:
	static FAttackResolutionFlowResult ResolveAttack(
		const FMatchRuntimeState& RuntimeState,
		bool bGoalScored);
};
