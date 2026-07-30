#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackBranchIntentSelectionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackBranchIntentSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Branch Intent Selection")
	EMatchPlayElectiveBranchIntent Intent =
		EMatchPlayElectiveBranchIntent::None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackBranchIntentSelectionErrorCode : uint8
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
	UnsupportedElectiveIntentActionType
		UMETA(DisplayName = "Unsupported Elective Intent Action Type"),
	InvalidDeploymentPlacement
		UMETA(DisplayName = "Invalid Deployment Placement"),
	DuplicateDeploymentCard
		UMETA(DisplayName = "Duplicate Deployment Card"),
	DuplicateDeploymentSlot
		UMETA(DisplayName = "Duplicate Deployment Slot"),
	CarrierDeploymentInvalid
		UMETA(DisplayName = "Carrier Deployment Invalid"),
	MarkerDeploymentInvalid
		UMETA(DisplayName = "Marker Deployment Invalid"),
	RunnerDeploymentInvalid
		UMETA(DisplayName = "Runner Deployment Invalid"),
	CarrierSnapshotQueryFailed
		UMETA(DisplayName = "Carrier Snapshot Query Failed"),
	MarkerSnapshotQueryFailed
		UMETA(DisplayName = "Marker Snapshot Query Failed"),
	RunnerSnapshotQueryFailed
		UMETA(DisplayName = "Runner Snapshot Query Failed"),
	RequiredParticipantIsGoalkeeper
		UMETA(DisplayName = "Required Participant Is Goalkeeper"),
	RunnerMatchesCarrier UMETA(DisplayName = "Runner Matches Carrier"),
	RunnerMissingRequiredPositionType
		UMETA(DisplayName = "Runner Missing Required Position Type"),
	HelperAuthorityFailed UMETA(DisplayName = "Helper Authority Failed"),
	GlobalContextFailed UMETA(DisplayName = "Global Context Failed"),
	InvalidIntent UMETA(DisplayName = "Invalid Intent"),
	IntentActionTypeMismatch
		UMETA(DisplayName = "Intent Action Type Mismatch")
};
