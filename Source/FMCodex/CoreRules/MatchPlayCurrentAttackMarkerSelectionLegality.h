#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.h"
#include "MatchPlayDeploymentPhysicalAreaMatchQuery.h"

#include "MatchPlayCurrentAttackMarkerSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackMarkerSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FName MarkerCardId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackMarkerSelectionErrorCode : uint8
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
	InvalidMarkerCardId UMETA(DisplayName = "Invalid Marker Card Id"),
	InvalidFrozenCarrierCardId
		UMETA(DisplayName = "Invalid Frozen Carrier Card Id"),
	FrozenCarrierNotDeployed
		UMETA(DisplayName = "Frozen Carrier Not Deployed"),
	FrozenCarrierDeploymentAmbiguous
		UMETA(DisplayName = "Frozen Carrier Deployment Ambiguous"),
	MarkerNotDeployed UMETA(DisplayName = "Marker Not Deployed"),
	MarkerDeploymentAmbiguous
		UMETA(DisplayName = "Marker Deployment Ambiguous"),
	MarkerSnapshotQueryFailed
		UMETA(DisplayName = "Marker Snapshot Query Failed"),
	PhysicalAreaQueryFailed
		UMETA(DisplayName = "Physical Area Query Failed"),
	MarkerNotInCarrierPhysicalArea
		UMETA(DisplayName = "Marker Not In Carrier Physical Area"),
	MarkerIsGoalkeeper UMETA(DisplayName = "Marker Is Goalkeeper")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackMarkerSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FMatchPlayCurrentAttackMarkerSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	EMatchPlayCurrentAttackMarkerSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FName FrozenCarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	int32 MatchingFrozenCarrierPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	int32 MatchingMarkerPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FMatchPlayDeploymentPlacement FrozenCarrierPlacement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FMatchPlayDeploymentPlacement MarkerPlacement;

	FMatchPlayCardSnapshotAuthorityQueryResult MarkerSnapshotQueryResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FMatchPlayDeploymentPhysicalAreaMatchResult PhysicalAreaMatchResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Marker Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackMarkerSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackMarkerSelectionRequest& Request);
};
