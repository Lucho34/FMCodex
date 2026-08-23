#pragma once

#include "CoreMinimal.h"

#include "InitialTurnOrderResolver.h"

#include "MatchPlayCurrentAttackSkillSelectionTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSkillSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FName SkillId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSkillSelectionErrorCode : uint8
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
	InvalidSkillId UMETA(DisplayName = "Invalid Skill Id"),
	InvalidFrozenCarrierCardId
		UMETA(DisplayName = "Invalid Frozen Carrier Card Id"),
	InvalidFrozenMarkerCardId
		UMETA(DisplayName = "Invalid Frozen Marker Card Id"),
	FrozenCarrierNotDeployed
		UMETA(DisplayName = "Frozen Carrier Not Deployed"),
	FrozenCarrierDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Carrier Deployment Ambiguous"),
	FrozenMarkerNotDeployed
		UMETA(DisplayName = "Frozen Marker Not Deployed"),
	FrozenMarkerDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Marker Deployment Ambiguous"),
	CarrierSnapshotQueryFailed
		UMETA(DisplayName = "Carrier Snapshot Query Failed"),
	DuplicateCarrierSkillId
		UMETA(DisplayName = "Duplicate Carrier Skill Id"),
	CarrierDoesNotOwnSkill
		UMETA(DisplayName = "Carrier Does Not Own Skill"),
	InvalidSkillRuleSet
		UMETA(DisplayName = "Invalid Skill Rule Set"),
	SkillRuleNotFound UMETA(DisplayName = "Skill Rule Not Found"),
	SkillRuleAmbiguous UMETA(DisplayName = "Skill Rule Ambiguous"),
	UnsupportedSkillRuleType
		UMETA(DisplayName = "Unsupported Skill Rule Type"),
	ParticipantRequirementResolutionFailed
		UMETA(DisplayName = "Participant Requirement Resolution Failed"),
	PreparedRunnerIncompatibleWithSkill
		UMETA(DisplayName = "Prepared Runner Incompatible With Skill"),
	InvalidCurrentAttackActionPoint
		UMETA(DisplayName = "Invalid Current Attack Action Point"),
	ActionPointOutsideSkillRange
		UMETA(DisplayName = "Action Point Outside Skill Range"),
	DeferredActionTypeMismatch
		UMETA(DisplayName = "Deferred Action Type Mismatch")
};
