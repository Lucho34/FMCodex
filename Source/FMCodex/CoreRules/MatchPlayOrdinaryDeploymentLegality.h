#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayOrdinaryDeploymentLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOrdinaryDeploymentRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Ordinary Deployment")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Ordinary Deployment")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Ordinary Deployment")
	FName CardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Ordinary Deployment")
	FName SlotId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayOrdinaryDeploymentErrorCode : uint8
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
		UMETA(DisplayName = "Requesting Side Already Finished"),
	InvalidCardId UMETA(DisplayName = "Invalid Card Id"),
	InvalidSlotId UMETA(DisplayName = "Invalid Slot Id"),
	CardSnapshotLookupFailed
		UMETA(DisplayName = "Card Snapshot Lookup Failed"),
	CardUsageValidationFailed
		UMETA(DisplayName = "Card Usage Validation Failed"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used"),
	CardAlreadyPlacedBySide
		UMETA(DisplayName = "Card Already Placed By Side"),
	GoalkeeperNotAllowed UMETA(DisplayName = "Goalkeeper Not Allowed"),
	SlotCatalogLookupFailed
		UMETA(DisplayName = "Slot Catalog Lookup Failed"),
	SlotNotFound UMETA(DisplayName = "Slot Not Found"),
	SlotAlreadyOccupied UMETA(DisplayName = "Slot Already Occupied"),
	RelativeZoneResolutionFailed
		UMETA(DisplayName = "Relative Zone Resolution Failed"),
	PositionNotEligibleForZone
		UMETA(DisplayName = "Position Not Eligible For Zone")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayOrdinaryDeploymentLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	FMatchPlayOrdinaryDeploymentRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	EMatchPlayOrdinaryDeploymentErrorCode ErrorCode =
		EMatchPlayOrdinaryDeploymentErrorCode::None;

	// The existing query-only error enum intentionally remains plain C++.
	EMatchPlayCardSnapshotAuthorityQueryErrorCode
		UnderlyingSnapshotAuthorityQueryErrorCode =
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	EPlayCardResolveErrorCode UnderlyingPlayCardErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	ECardUsageResolveErrorCode UnderlyingCardUsageErrorCode =
		ECardUsageResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	EMatchPlayDeploymentSlotCatalogQueryErrorCode
		UnderlyingSlotCatalogQueryErrorCode =
			EMatchPlayDeploymentSlotCatalogQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	EMatchPlayRelativeDeploymentZoneResolveErrorCode
		UnderlyingRelativeZoneResolutionErrorCode =
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	EMatchPlayRelativeDeploymentZone ResolvedRelativeZone =
		EMatchPlayRelativeDeploymentZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Ordinary Deployment")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayOrdinaryDeploymentLegalityEvaluator final
{
public:
	static FMatchPlayOrdinaryDeploymentLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayOrdinaryDeploymentRequest& Request);
};
