#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayBeginOrdinaryAttack.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayBeginOrdinaryAttackErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	CurrentAttackAlreadyActive
		UMETA(DisplayName = "Current Attack Already Active"),
	InvalidPlayerAAttackCountState
		UMETA(DisplayName = "Invalid Player A Attack Count State"),
	InvalidPlayerBAttackCountState
		UMETA(DisplayName = "Invalid Player B Attack Count State"),
	MatchAlreadyEnded UMETA(DisplayName = "Match Already Ended"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	CurrentAttackerHasNoRemainingAttackOpportunity
		UMETA(DisplayName = "Current Attacker Has No Remaining Attack Opportunity"),
	InvalidActionPoint UMETA(DisplayName = "Invalid Action Point")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBeginOrdinaryAttackResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	int32 ActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	EMatchPlayBeginOrdinaryAttackErrorCode ErrorCode =
		EMatchPlayBeginOrdinaryAttackErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Begin Ordinary Attack")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayBeginOrdinaryAttack final
{
public:
	static FMatchPlayBeginOrdinaryAttackResult Begin(
		const FMatchPlayState& BeforeState,
		int32 ActionPoint);
};
