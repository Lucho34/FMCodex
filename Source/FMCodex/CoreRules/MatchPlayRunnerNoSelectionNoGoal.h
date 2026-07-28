#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"

#include "MatchPlayRunnerNoSelectionNoGoal.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalRunnerRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	int64 AttackSequence = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRunnerDeclineRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

UENUM(BlueprintType)
enum class EMatchPlayRunnerNoSelectionNoGoalErrorCode : uint8
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
	RequestingSideIsNotCurrentAttacker
		UMETA(DisplayName = "Requesting Side Is Not Current Attacker"),
	InvalidFrozenCarrierCardId
		UMETA(DisplayName = "Invalid Frozen Carrier Card Id"),
	InvalidFrozenMarkerCardId
		UMETA(DisplayName = "Invalid Frozen Marker Card Id"),
	InvalidFrozenSkillId
		UMETA(DisplayName = "Invalid Frozen Skill Id"),
	ParticipantRequirementResolutionFailed
		UMETA(DisplayName = "Participant Requirement Resolution Failed"),
	ParticipantRequirementMismatch
		UMETA(DisplayName = "Participant Requirement Mismatch"),
	InvalidDeploymentPlacement
		UMETA(DisplayName = "Invalid Deployment Placement"),
	DuplicateDeploymentCard
		UMETA(DisplayName = "Duplicate Deployment Card"),
	DuplicateDeploymentSlot
		UMETA(DisplayName = "Duplicate Deployment Slot"),
	FrozenCarrierNotDeployed
		UMETA(DisplayName = "Frozen Carrier Not Deployed"),
	FrozenCarrierDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Carrier Deployment Ambiguous"),
	FrozenMarkerNotDeployed
		UMETA(DisplayName = "Frozen Marker Not Deployed"),
	FrozenMarkerDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Marker Deployment Ambiguous"),
	InvalidAttackingSnapshotSet
		UMETA(DisplayName = "Invalid Attacking Snapshot Set"),
	InvalidSlotCatalog UMETA(DisplayName = "Invalid Slot Catalog"),
	AvailabilityQueryFailed
		UMETA(DisplayName = "Availability Query Failed"),
	LegalRunnerExists UMETA(DisplayName = "Legal Runner Exists"),
	NoLegalRunnerToDecline
		UMETA(DisplayName = "No Legal Runner To Decline"),
	InvalidCapability UMETA(DisplayName = "Invalid Capability"),
	InvalidCapabilitySourceReason
		UMETA(DisplayName = "Invalid Capability Source Reason"),
	CompletionFailed UMETA(DisplayName = "Completion Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayResolveNoLegalRunnerResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayResolveNoLegalRunnerRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult
		RunnerAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalReason Reason =
		EMatchPlayRunnerNoSelectionNoGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalSource Source =
		EMatchPlayRunnerNoSelectionNoGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalErrorCode ErrorCode =
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayRunnerDeclineResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayRunnerDeclineRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayState BeforeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayState AfterState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult
		RunnerAvailabilityResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalReason Reason =
		EMatchPlayRunnerNoSelectionNoGoalReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalSource Source =
		EMatchPlayRunnerNoSelectionNoGoalSource::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FMatchPlayCurrentAttackCompletionResult CompletionResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	EMatchPlayRunnerNoSelectionNoGoalErrorCode ErrorCode =
		EMatchPlayRunnerNoSelectionNoGoalErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Runner No-selection NoGoal")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayResolveNoLegalRunner final
{
public:
	static FMatchPlayResolveNoLegalRunnerResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlayResolveNoLegalRunnerRequest& Request);
};

class FMCODEX_API FMatchPlayRunnerDecline final
{
public:
	static FMatchPlayRunnerDeclineResult Decline(
		const FMatchPlayState& BeforeState,
		const FMatchPlayRunnerDeclineRequest& Request);
};
