#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayGoalkeeperDeploymentLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperDeploymentRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	FName CardId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	FName SlotId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayGoalkeeperDeploymentErrorCode : uint8
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
	RequestingSideIsNotDefender
		UMETA(DisplayName = "Requesting Side Is Not Defender"),
	InvalidCardId UMETA(DisplayName = "Invalid Card Id"),
	InvalidSlotId UMETA(DisplayName = "Invalid Slot Id"),
	CardSnapshotLookupFailed
		UMETA(DisplayName = "Card Snapshot Lookup Failed"),
	CardIsNotGoalkeeper UMETA(DisplayName = "Card Is Not Goalkeeper"),
	CardUsageValidationFailed
		UMETA(DisplayName = "Card Usage Validation Failed"),
	CardNotAvailable UMETA(DisplayName = "Card Not Available"),
	CardAlreadyUsed UMETA(DisplayName = "Card Already Used"),
	GoalkeeperUsageStateQueryFailed
		UMETA(DisplayName = "Goalkeeper Usage State Query Failed"),
	InvalidGoalkeeperUsageState
		UMETA(DisplayName = "Invalid Goalkeeper Usage State"),
	GoalkeeperAlreadyActivatedThisAttack
		UMETA(DisplayName = "Goalkeeper Already Activated This Attack"),
	GoalkeeperAlreadyUsedThisMatch
		UMETA(DisplayName = "Goalkeeper Already Used This Match"),
	SlotCatalogLookupFailed
		UMETA(DisplayName = "Slot Catalog Lookup Failed"),
	SlotNotFound UMETA(DisplayName = "Slot Not Found"),
	SlotAlreadyOccupied UMETA(DisplayName = "Slot Already Occupied"),
	RelativeZoneResolutionFailed
		UMETA(DisplayName = "Relative Zone Resolution Failed"),
	GoalkeeperSlotNotInDefenderBackfield
		UMETA(DisplayName = "Goalkeeper Slot Not In Defender Backfield")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayGoalkeeperDeploymentLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	FMatchPlayGoalkeeperDeploymentRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EMatchPlayGoalkeeperDeploymentErrorCode ErrorCode =
		EMatchPlayGoalkeeperDeploymentErrorCode::None;

	EMatchPlayCardSnapshotAuthorityQueryErrorCode
		UnderlyingSnapshotAuthorityQueryErrorCode =
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EPlayCardResolveErrorCode UnderlyingPlayCardErrorCode =
		EPlayCardResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	ECardUsageResolveErrorCode UnderlyingCardUsageErrorCode =
		ECardUsageResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EMatchPlayGoalkeeperUsageErrorCode UnderlyingGoalkeeperUsageErrorCode =
		EMatchPlayGoalkeeperUsageErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EMatchPlayDeploymentSlotCatalogQueryErrorCode
		UnderlyingSlotCatalogQueryErrorCode =
			EMatchPlayDeploymentSlotCatalogQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EMatchPlayRelativeDeploymentZoneResolveErrorCode
		UnderlyingRelativeZoneResolutionErrorCode =
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	EMatchPlayRelativeDeploymentZone ResolvedRelativeZone =
		EMatchPlayRelativeDeploymentZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	int32 MatchingCurrentGoalkeeperPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Goalkeeper Deployment")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayGoalkeeperDeploymentLegalityEvaluator final
{
public:
	static FMatchPlayGoalkeeperDeploymentLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayGoalkeeperDeploymentRequest& Request);
};
