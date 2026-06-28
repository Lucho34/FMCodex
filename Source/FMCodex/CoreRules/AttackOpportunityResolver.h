#pragma once

#include "CoreMinimal.h"
#include "MatchRuntimeStateTypes.h"

#include "AttackOpportunityResolver.generated.h"

UENUM(BlueprintType)
enum class EAttackOpportunityResolveErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	RuntimeStateNotInitialized UMETA(DisplayName = "Runtime State Not Initialized"),
	InvalidCurrentAttackingPlayer UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidPlayerAAttackCountState UMETA(DisplayName = "Invalid Player A Attack Count State"),
	InvalidPlayerBAttackCountState UMETA(DisplayName = "Invalid Player B Attack Count State"),
	CurrentPlayerHasNoRemainingAttack
		UMETA(DisplayName = "Current Player Has No Remaining Attack"),
	NoRemainingAttackForBothPlayers
		UMETA(DisplayName = "No Remaining Attack For Both Players")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FAttackOpportunityResolveResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	bool bMatchHasRemainingAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	FMatchRuntimeState UpdatedRuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	EInitialTurnOrderPlayer ActingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	EInitialTurnOrderPlayer NextAttackingPlayer = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	TArray<EAttackOpportunityResolveErrorCode> ErrorCodes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Attack Opportunity")
	TArray<FString> ErrorMessages;
};

class FMCODEX_API FAttackOpportunityResolver final
{
public:
	static FAttackOpportunityResolveResult ConsumeCurrentAttackOpportunity(
		const FMatchRuntimeState& RuntimeState);
};
