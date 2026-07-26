#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlaySkillParticipantRequirementQuery.h"
#include "SkillRuleSnapshotQuery.h"

#include "MatchPlayCurrentAttackSkillSelectionLegality.generated.h"

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
	InvalidCurrentAttackActionPoint
		UMETA(DisplayName = "Invalid Current Attack Action Point"),
	ActionPointOutsideSkillRange
		UMETA(DisplayName = "Action Point Outside Skill Range")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSkillSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayCurrentAttackSkillSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FName FrozenCarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FName FrozenMarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	int32 MatchingFrozenCarrierPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	int32 MatchingFrozenMarkerPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlayDeploymentPlacement FrozenMarkerPlacement;

	FMatchPlayCardSnapshotAuthorityQueryResult CarrierSnapshotQueryResult;
	FSkillRuleSnapshotQueryResult SkillRuleQueryResult;
	FSkillRuleSnapshot ResolvedSkillRule;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	ESkillRuleType ResolvedActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FMatchPlaySkillParticipantRequirementResult
		ParticipantRequirementResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Skill Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackSkillSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayCurrentAttackSkillSelectionRequest& Request);
};
