#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayFinishDeployment.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayFinishDeploymentErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	CurrentAttackNotInDeployment
		UMETA(DisplayName = "Current Attack Not In Deployment"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	InvalidCurrentLegalDeploymentSide
		UMETA(DisplayName = "Invalid Current Legal Deployment Side"),
	RequestingSideNotCurrentLegalDeploymentSide
		UMETA(DisplayName = "Requesting Side Not Current Legal Deployment Side"),
	InvalidDeploymentFinishedState
		UMETA(DisplayName = "Invalid Deployment Finished State"),
	RequestingSideAlreadyFinished
		UMETA(DisplayName = "Requesting Side Already Finished")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayFinishDeploymentResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	EMatchPlayFinishDeploymentErrorCode ErrorCode =
		EMatchPlayFinishDeploymentErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Finish Deployment")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayFinishDeployment final
{
public:
	static FMatchPlayFinishDeploymentResult Finish(
		const FMatchPlayState& BeforeState,
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);
};
