#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"

#include "MatchPlayMarkerNoSelectionGoal.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalMarkerRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Marker No-selection Goal")
	int64 AttackSequence = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayMarkerDeclineRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Marker No-selection Goal")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Marker No-selection Goal")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

UENUM(BlueprintType)
enum class EMatchPlayResolveNoLegalMarkerErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	MarkerAvailabilityFailed
		UMETA(DisplayName = "Marker Availability Failed"),
	LegalMarkerExists UMETA(DisplayName = "Legal Marker Exists"),
	GoalProjectionCreationFailed
		UMETA(DisplayName = "Goal Projection Creation Failed"),
	CompletionFailed UMETA(DisplayName = "Completion Failed")
};

UENUM(BlueprintType)
enum class EMatchPlayMarkerDeclineErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	AttackSequenceMismatch UMETA(DisplayName = "Attack Sequence Mismatch"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	RequestingSideIsNotCurrentDefender
		UMETA(DisplayName = "Requesting Side Is Not Current Defender"),
	MarkerAvailabilityFailed
		UMETA(DisplayName = "Marker Availability Failed"),
	NoLegalMarkerToDecline
		UMETA(DisplayName = "No Legal Marker To Decline"),
	GoalProjectionCreationFailed
		UMETA(DisplayName = "Goal Projection Creation Failed"),
	CompletionFailed UMETA(DisplayName = "Completion Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalMarkerResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayResolveNoLegalMarkerRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
		MarkerAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayMarkerNoSelectionGoalProjection GoalProjection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	EMatchPlayResolveNoLegalMarkerErrorCode ErrorCode =
		EMatchPlayResolveNoLegalMarkerErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayMarkerDeclineResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayMarkerDeclineRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
		MarkerAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayMarkerNoSelectionGoalProjection GoalProjection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	EMatchPlayMarkerDeclineErrorCode ErrorCode =
		EMatchPlayMarkerDeclineErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Marker No-selection Goal")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayResolveNoLegalMarker final
{
public:
	static FMatchPlayResolveNoLegalMarkerResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayResolveNoLegalMarkerRequest& Request);
};

class FMCODEX_API FMatchPlayMarkerDecline final
{
public:
	static FMatchPlayMarkerDeclineResult Decline(
		const FMatchPlayState& BeforeState,
		const FMatchPlayMarkerDeclineRequest& Request);
};
