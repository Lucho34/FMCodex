#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionNormalizedParticipantValues.h"
#include "MatchPlayCurrentAttackResolutionBinding.h"

#include "MatchPlayBoundActionParticipantNormalizationTypes.generated.h"

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionParticipantNormalizationRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int64 AttackSequence = 0;
};

UENUM(BlueprintType)
enum class EMatchPlayBoundActionParticipantSnapshotQueryErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	InvalidPlayerSide UMETA(DisplayName = "Invalid Player Side"),
	InvalidCardId UMETA(DisplayName = "Invalid Card Id"),
	SnapshotValidationFailed UMETA(DisplayName = "Snapshot Validation Failed"),
	SnapshotNotFound UMETA(DisplayName = "Snapshot Not Found")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionNormalizedParticipant
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bIsPresent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bSnapshotQueryAttempted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bSnapshotQuerySucceeded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EMatchPlayBoundActionParticipantSnapshotQueryErrorCode
		SnapshotQueryErrorCode =
			EMatchPlayBoundActionParticipantSnapshotQueryErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FString SnapshotQueryErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipantValues Values;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionNormalizedParticipantBundle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayCurrentAttackResolutionBindingValue Binding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipant Carrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipant Marker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bHasRunner = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipant Runner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipant Helper;
};

UENUM(BlueprintType)
enum class EMatchPlayBoundActionParticipantNormalizationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	BindingFailed UMETA(DisplayName = "Binding Failed"),
	UnsupportedActionType UMETA(DisplayName = "Unsupported Action Type"),
	InvalidBoundParticipants UMETA(DisplayName = "Invalid Bound Participants"),
	InvalidAttackingPlayer UMETA(DisplayName = "Invalid Attacking Player"),
	InvalidDefendingPlayer UMETA(DisplayName = "Invalid Defending Player"),
	CarrierSnapshotQueryFailed
		UMETA(DisplayName = "Carrier Snapshot Query Failed"),
	MarkerSnapshotQueryFailed
		UMETA(DisplayName = "Marker Snapshot Query Failed"),
	MissingRequiredRunner UMETA(DisplayName = "Missing Required Runner"),
	UnexpectedRunner UMETA(DisplayName = "Unexpected Runner"),
	RunnerSnapshotQueryFailed
		UMETA(DisplayName = "Runner Snapshot Query Failed"),
	InvalidHelperPresence UMETA(DisplayName = "Invalid Helper Presence"),
	HelperSnapshotQueryFailed
		UMETA(DisplayName = "Helper Snapshot Query Failed"),
	InvalidNumericSource UMETA(DisplayName = "Invalid Numeric Source"),
	BundleConstructionFailed UMETA(DisplayName = "Bundle Construction Failed")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayBoundActionParticipantNormalizationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	bool bSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionParticipantNormalizationRequest Request;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EMatchPlayBoundActionParticipantNormalizationErrorCode ErrorCode =
		EMatchPlayBoundActionParticipantNormalizationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FString ErrorMessage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayCurrentAttackResolutionBindingResult BindingResult;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Bound Action Participant Normalization")
	FMatchPlayBoundActionNormalizedParticipantBundle Bundle;
};
