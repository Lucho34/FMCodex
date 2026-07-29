#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackHelperParticipantAuthority.h"
#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchPlaySkillParticipantRequirementQuery.h"

#include "MatchPlayCurrentAttackReadyForResolutionValidator.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackReadyValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	MatchPlayStateNotInitialized
		UMETA(DisplayName = "Match Play State Not Initialized"),
	NoCurrentAttack UMETA(DisplayName = "No Current Attack"),
	InvalidCurrentAttackSequence
		UMETA(DisplayName = "Invalid Current Attack Sequence"),
	CurrentAttackNotInResolution
		UMETA(DisplayName = "Current Attack Not In Resolution"),
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	InvalidCurrentAttackingPlayer
		UMETA(DisplayName = "Invalid Current Attacking Player"),
	InvalidCurrentDefendingPlayer
		UMETA(DisplayName = "Invalid Current Defending Player"),
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
	RunnerPhysicalAreaResolutionFailed
		UMETA(DisplayName = "Runner Physical Area Resolution Failed"),
	RunnerNotInAttackingForwardArea
		UMETA(DisplayName = "Runner Not In Attacking Forward Area"),
	HelperAuthorityFailed UMETA(DisplayName = "Helper Authority Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackReadyValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	EMatchPlayCurrentAttackReadyValidationErrorCode ErrorCode =
		EMatchPlayCurrentAttackReadyValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	FMatchPlaySkillParticipantRequirementResult
		ParticipantRequirementResult;

	FMatchPlayCardSnapshotAuthorityQueryResult
		CarrierSnapshotQueryResult;
	FMatchPlayCardSnapshotAuthorityQueryResult
		MarkerSnapshotQueryResult;
	FMatchPlayCardSnapshotAuthorityQueryResult
		RunnerSnapshotQueryResult;
	FMatchPlayCurrentAttackHelperParticipantAuthorityResult
		HelperAuthorityResult;
	FMatchPlayRelativeDeploymentZoneResolveResult
		RunnerRelativeZoneResolveResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Ready")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackReadyForResolutionValidator final
{
public:
	static FMatchPlayCurrentAttackReadyValidationResult Validate(
		const FMatchPlayState& State);
};
