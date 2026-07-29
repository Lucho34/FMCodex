#pragma once

#include "CoreMinimal.h"

#include "MatchPlayState.h"

#include "MatchPlayCurrentAttackSelectionStateValidator.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSelectionStateValidationErrorCode : uint8
{
	None UMETA(DisplayName = "None"),
	UnsupportedCurrentAttackPhase
		UMETA(DisplayName = "Unsupported Current Attack Phase"),
	UnsupportedSelectionStage
		UMETA(DisplayName = "Unsupported Selection Stage"),
	SelectionStageDoesNotMatchPhase
		UMETA(DisplayName = "Selection Stage Does Not Match Phase"),
	UnexpectedPreparationCarrier
		UMETA(DisplayName = "Unexpected Preparation Carrier"),
	MissingPreparationCarrier
		UMETA(DisplayName = "Missing Preparation Carrier"),
	SelectedActionUnexpectedlyPresent
		UMETA(DisplayName = "Selected Action Unexpectedly Present"),
	SelectedActionPayloadNotEmpty
		UMETA(DisplayName = "Selected Action Payload Not Empty"),
	DuplicateCarrierAuthority
		UMETA(DisplayName = "Duplicate Carrier Authority"),
	UnexpectedPreparationMarker
		UMETA(DisplayName = "Unexpected Preparation Marker"),
	MissingPreparationMarker
		UMETA(DisplayName = "Missing Preparation Marker"),
	UnexpectedPreparationSkill
		UMETA(DisplayName = "Unexpected Preparation Skill"),
	MissingPreparationSkill
		UMETA(DisplayName = "Missing Preparation Skill"),
	UnexpectedPreparationActionType
		UMETA(DisplayName = "Unexpected Preparation Action Type"),
	MissingPreparationActionType
		UMETA(DisplayName = "Missing Preparation Action Type"),
	UnexpectedPreparationRunner
		UMETA(DisplayName = "Unexpected Preparation Runner"),
	MissingPreparationRunner
		UMETA(DisplayName = "Missing Preparation Runner"),
	ResolutionDeploymentNotComplete
		UMETA(DisplayName = "Resolution Deployment Not Complete"),
	ResolutionLegalDeploymentSideNotCleared
		UMETA(DisplayName = "Resolution Legal Deployment Side Not Cleared"),
	InvalidResolutionActionPoint
		UMETA(DisplayName = "Invalid Resolution Action Point"),
	PreparationAndSelectedActionCoexist
		UMETA(DisplayName = "Preparation And Selected Action Coexist"),
	MissingSelectedAction
		UMETA(DisplayName = "Missing Selected Action"),
	MissingSelectedActionCarrier
		UMETA(DisplayName = "Missing Selected Action Carrier"),
	MissingSelectedActionMarker
		UMETA(DisplayName = "Missing Selected Action Marker"),
	MissingSelectedActionSkill
		UMETA(DisplayName = "Missing Selected Action Skill"),
	MissingSelectedActionActionType
		UMETA(DisplayName = "Missing Selected Action Action Type"),
	MissingSelectedActionRunner
		UMETA(DisplayName = "Missing Selected Action Runner"),
	UnexpectedSelectedActionRunner
		UMETA(DisplayName = "Unexpected Selected Action Runner"),
	MissingSelectedActionHelper
		UMETA(DisplayName = "Missing Selected Action Helper"),
	UnexpectedSelectedActionHelper
		UMETA(DisplayName = "Unexpected Selected Action Helper"),
	ParticipantRequirementResolutionFailed
		UMETA(DisplayName = "Participant Requirement Resolution Failed"),
	ActionTypeDoesNotMatchSelectionStage
		UMETA(DisplayName = "Action Type Does Not Match Selection Stage")
};

USTRUCT(BlueprintType)
struct FMCODEX_API
	FMatchPlayCurrentAttackSelectionStateValidationResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	bool bIsCanonical = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	EMatchPlayCurrentAttackSelectionStateValidationErrorCode ErrorCode =
		EMatchPlayCurrentAttackSelectionStateValidationErrorCode::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Selection State")
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCurrentAttackSelectionStateValidator final
{
public:
	static FMatchPlayCurrentAttackSelectionStateValidationResult Validate(
		const FMatchPlayCurrentAttackState& CurrentAttack);
};
