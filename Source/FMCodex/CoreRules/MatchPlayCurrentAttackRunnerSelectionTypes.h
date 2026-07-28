#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.h"

#include "MatchPlayCurrentAttackRunnerSelectionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackRunnerSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Runner Selection")
	FName RunnerCardId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackRunnerSelectionErrorCode : uint8
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
	GlobalContextFailed UMETA(DisplayName = "Global Context Failed"),
	InvalidRunnerCardId UMETA(DisplayName = "Invalid Runner Card Id"),
	RunnerNotDeployed UMETA(DisplayName = "Runner Not Deployed"),
	RunnerDeploymentAmbiguous
		UMETA(DisplayName = "Runner Deployment Ambiguous"),
	RunnerSnapshotQueryFailed
		UMETA(DisplayName = "Runner Snapshot Query Failed"),
	RunnerIsGoalkeeper UMETA(DisplayName = "Runner Is Goalkeeper"),
	RunnerMatchesCarrier UMETA(DisplayName = "Runner Matches Carrier"),
	RunnerMissingRequiredPositionType
		UMETA(DisplayName = "Runner Missing Required Position Type"),
	RunnerPhysicalAreaResolutionFailed
		UMETA(DisplayName = "Runner Physical Area Resolution Failed"),
	RunnerNotInAttackingForwardArea
		UMETA(DisplayName = "Runner Not In Attacking Forward Area"),
	UnsupportedRunnerActionType
		UMETA(DisplayName = "Unsupported Runner Action Type")
};
