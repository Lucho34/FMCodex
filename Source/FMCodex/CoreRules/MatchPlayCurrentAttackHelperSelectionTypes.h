#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.h"

#include "MatchPlayCurrentAttackHelperSelectionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackHelperSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Helper Selection")
	FName HelperCardId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackHelperSelectionErrorCode : uint8
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
	InvalidFrozenCarrierCardId
		UMETA(DisplayName = "Invalid Frozen Carrier Card Id"),
	InvalidFrozenMarkerCardId
		UMETA(DisplayName = "Invalid Frozen Marker Card Id"),
	InvalidFrozenSkillId
		UMETA(DisplayName = "Invalid Frozen Skill Id"),
	InvalidFrozenActionType
		UMETA(DisplayName = "Invalid Frozen Action Type"),
	InvalidFrozenRunnerCardId
		UMETA(DisplayName = "Invalid Frozen Runner Card Id"),
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
	DuplicateDeploymentPlacement
		UMETA(DisplayName = "Duplicate Deployment Placement"),
	FrozenCarrierNotDeployed
		UMETA(DisplayName = "Frozen Carrier Not Deployed"),
	FrozenCarrierDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Carrier Deployment Ambiguous"),
	FrozenMarkerNotDeployed
		UMETA(DisplayName = "Frozen Marker Not Deployed"),
	FrozenMarkerDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Marker Deployment Ambiguous"),
	FrozenRunnerNotDeployed
		UMETA(DisplayName = "Frozen Runner Not Deployed"),
	FrozenRunnerDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Runner Deployment Ambiguous"),
	InvalidAttackingSnapshotSet
		UMETA(DisplayName = "Invalid Attacking Snapshot Set"),
	InvalidDefendingSnapshotSet
		UMETA(DisplayName = "Invalid Defending Snapshot Set"),
	GlobalContextFailed UMETA(DisplayName = "Global Context Failed"),
	InvalidHelperCardId UMETA(DisplayName = "Invalid Helper Card Id"),
	HelperNotDeployed UMETA(DisplayName = "Helper Not Deployed"),
	HelperDeploymentAmbiguous
		UMETA(DisplayName = "Helper Deployment Ambiguous"),
	HelperSnapshotQueryFailed
		UMETA(DisplayName = "Helper Snapshot Query Failed"),
	HelperIsGoalkeeper UMETA(DisplayName = "Helper Is Goalkeeper"),
	HelperMatchesMarker UMETA(DisplayName = "Helper Matches Marker"),
	PhysicalAreaQueryFailed
		UMETA(DisplayName = "Physical Area Query Failed"),
	HelperNotInRunnerPhysicalArea
		UMETA(DisplayName = "Helper Not In Runner Physical Area"),
	UnsupportedHelperActionType
		UMETA(DisplayName = "Unsupported Helper Action Type")
};
