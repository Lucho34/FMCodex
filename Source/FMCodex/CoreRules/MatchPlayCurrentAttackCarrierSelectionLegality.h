#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.h"

#include "MatchPlayCurrentAttackCarrierSelectionLegality.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackCarrierSelectionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	int64 AttackSequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	FName CarrierCardId = NAME_None;
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackCarrierSelectionErrorCode : uint8
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
	InvalidSelectionState UMETA(DisplayName = "Invalid Selection State"),
	WrongSelectionStage UMETA(DisplayName = "Wrong Selection Stage"),
	InvalidRequestingSide UMETA(DisplayName = "Invalid Requesting Side"),
	RequestingSideIsNotCurrentAttacker
		UMETA(DisplayName = "Requesting Side Is Not Current Attacker"),
	InvalidCarrierCardId UMETA(DisplayName = "Invalid Carrier Card Id"),
	CarrierNotDeployed UMETA(DisplayName = "Carrier Not Deployed"),
	CarrierDeploymentAmbiguous
		UMETA(DisplayName = "Carrier Deployment Ambiguous"),
	CarrierSnapshotLookupFailed
		UMETA(DisplayName = "Carrier Snapshot Lookup Failed"),
	CarrierIsGoalkeeper UMETA(DisplayName = "Carrier Is Goalkeeper")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionLegalityResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	bool bIsLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	FMatchPlayCurrentAttackCarrierSelectionRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	EMatchPlayCurrentAttackCarrierSelectionErrorCode ErrorCode =
		EMatchPlayCurrentAttackCarrierSelectionErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	FMatchPlayCurrentAttackSelectionStateValidationResult
		SelectionStateValidationResult;

	EMatchPlayCardSnapshotAuthorityQueryErrorCode
		UnderlyingSnapshotAuthorityQueryErrorCode =
			EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	int32 MatchingCarrierPlacementCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Carrier Selection")
	FString ErrorMessage;
};

class FMCODEX_API
	FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator final
{
public:
	static FMatchPlayCurrentAttackCarrierSelectionLegalityResult Evaluate(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCurrentAttackCarrierSelectionRequest& Request);
};
