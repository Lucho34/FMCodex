#pragma once

#include "CoreMinimal.h"
#include "../CoreRules/InitialTurnOrderResolver.h"
#include "../CoreRules/MatchPlayCurrentAttackResolutionFactProjection.h"

#include "FMCodexLocalMatchUMGPresentation.generated.h"

struct FFMCodexLocalMatchCardView;
struct FFMCodexLocalMatchInteractionView;
struct FFMCodexLocalMatchResolutionFeedback;

UENUM(BlueprintType)
enum class EFMCodexUMGInteractionCategory : uint8
{
	None,
	StartMatch,
	TacticalPointRoll,
	Deploy,
	SelectCarrier,
	SelectMarker,
	SelectSkill,
	SelectRunner,
	SelectHelper,
	SelectBranchIntent,
	SelectOneOnOneShot,
	RollCrossAttack,
	RollCrossDefense,
	CompleteCrossAndAdvance,
	ContinueResolution,
	AttackComplete,
	MatchEnded
};

UENUM(BlueprintType)
enum class EFMCodexUMGAttackTurnStepState : uint8
{
	Used,
	Current,
	Remaining
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGAttackTurnStepViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 AttackIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	EFMCodexUMGAttackTurnStepState State =
		EFMCodexUMGAttackTurnStepState::Remaining;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGAttackTurnTrackerViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 MaxAttackTurns = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 UsedAttackTurns = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 CurrentAttackIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bCurrentAttackingSide = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FLinearColor PrimarySideColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	TArray<FFMCodexUMGAttackTurnStepViewModel> Steps;
};

UENUM(BlueprintType)
enum class EFMCodexUMGBranchIntent : uint8
{
	None,
	DirectShot,
	DeadCorner,
	CrossHigh,
	CrossLow
};

UENUM(BlueprintType)
enum class EFMCodexUMGOneOnOneChoice : uint8
{
	None,
	ChipShot,
	DirectShot
};

UENUM(BlueprintType)
enum class EFMCodexUMGDeploymentTargetState : uint8
{
	Neutral,
	Valid,
	Invalid,
	Occupied,
	Unavailable
};

/** Presentation-only states for the Match Screen card interaction loop. */
UENUM(BlueprintType)
enum class EFMCodexUMGCardInteractionState : uint8
{
	Default,
	Hover,
	OnPitchSelectable,
	DragSource,
	Dragging,
	DragOverLegalSlot,
	DropSuccess,
	DropCancelled,
	Deployed,
	Ghost
};

/** Explicit presentation intent carried by an authoritative on-pitch candidate. */
UENUM(BlueprintType)
enum class EFMCodexUMGOnPitchSelectionIntent : uint8
{
	None,
	SubmitCarrier,
	SubmitMarker,
	SubmitRunner,
	SubmitHelper
};

/** Exactly one optional current-attack role may be attached to a Pitch Mini. */
UENUM(BlueprintType)
enum class EFMCodexUMGSelectedRole : uint8
{
	None,
	Carrier,
	Runner,
	Marker,
	Helper
};

/** Player feedback reasons already resolved outside UMG. */
UENUM(BlueprintType)
enum class EFMCodexUMGSelectionFeedbackReason : uint8
{
	None,
	MarkerWrongPhysicalArea,
	RunnerIsGoalkeeper,
	RunnerMatchesCarrier,
	RunnerMissingRequiredPositionType,
	RunnerNotInAttackingForwardArea,
	HelperIsGoalkeeper,
	HelperMatchesMarker,
	HelperWrongPhysicalArea
};

/** Presentation-only football landmark treatment for a physical Half. */
UENUM(BlueprintType)
enum class EFMCodexUMGPitchVisualRole : uint8
{
	Midfield,
	Forward,
	Backfield
};

/** Already-resolved structural ownership marker for a deployed Pitch Mini. */
UENUM(BlueprintType)
enum class EFMCodexUMGPitchMiniOwnershipEdge : uint8
{
	None,
	Left,
	Right
};

/**
 * Replaceable presentation palette for the two match sides. The prototype
 * defaults intentionally live above UMG widgets so later team selection can
 * supply club-derived colors without adding club rules to card rendering.
 */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGSidePrimaryColors
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "Local Match|Pitch Mini")
	FLinearColor PlayerAPrimaryColor = FLinearColor::FromSRGBColor(
		FColor(0x4F, 0x78, 0x92));

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "Local Match|Pitch Mini")
	FLinearColor PlayerBPrimaryColor = FLinearColor::FromSRGBColor(
		FColor(0xA4, 0x47, 0x4F));
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGAttributeViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString CanonicalLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGSkillViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString CanonicalLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 MinTriggerActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 MaxTriggerActionPoint = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGCardViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString IdentityLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString EnglishIdentityLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString NationalityLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString ClubLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString OwnerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString RoleLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 OverallRating = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bHasOverallRating = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString BirthDate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 HeightCm = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 WeightKg = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FFMCodexUMGAttributeViewModel> AttributeValues;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FFMCodexUMGSkillViewModel> Skills;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FFMCodexUMGSkillViewModel> EligibleTacticalSkills;

	/** Resolved by InteractionView: empty for the current defending side. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FFMCodexUMGSkillViewModel> PitchMiniVisibleTacticalSkills;

	/** Already resolved outside UMG; legal values are 0..2. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	int32 PitchMiniTacticalMatchCount = 0;

	/** Already resolved outside UMG; the Widget only renders this state. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bHasPitchMiniTacticalMatch = false;

	/** Authoritative current-attack role; only populated on Pitch Slot cards. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	EFMCodexUMGSelectedRole SelectedRole = EFMCodexUMGSelectedRole::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString SelectedRoleLabel;

	/** Presentation-resolved side color; never inferred from club data in UMG. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FLinearColor PitchMiniOwnershipAccentColor = FLinearColor::Transparent;

	/** Self uses the left rail; opponent uses the right rail. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	EFMCodexUMGPitchMiniOwnershipEdge PitchMiniOwnershipAccentEdge =
		EFMCodexUMGPitchMiniOwnershipEdge::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bHasPitchMiniOwnershipAccent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString PlayerFacingSerialLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FString> SkillLabels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString SkillSummaryLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString CompactAttributeSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString FullAttributeSummary;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	TArray<FString> StatusLabels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString StatusSummaryLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString RarityLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString DeveloperReferenceLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bAvailable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bUsed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	bool bDeployed = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGCardRackCellViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	int32 StableIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	bool bPlayed = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	bool bDeploymentDraggable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	bool bGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	FFMCodexUMGCardViewModel Card;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGCardRackViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	FString SideLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	bool bLocalRack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	bool bHasTacticalPlayerCount = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	int32 TacticalPlayerCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	FString TacticalPlayerCountLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	int32 ColumnCount = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	int32 RowCount = 10;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Rack")
	TArray<FFMCodexUMGCardRackCellViewModel> Cells;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGPitchSlotViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString SlotLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString PhysicalHalfLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString PlayerARelativeZoneLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString PlayerBRelativeZoneLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	bool bOccupied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	EFMCodexUMGDeploymentTargetState DeploymentTargetState =
		EFMCodexUMGDeploymentTargetState::Neutral;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FName DeploymentTargetCardId = NAME_None;

	/** Structural selectability projected for the current player prompt. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	bool bSelectableForCurrentPrompt = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FName OnPitchSelectionOptionId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	EFMCodexUMGOnPitchSelectionIntent OnPitchSelectionIntent =
		EFMCodexUMGOnPitchSelectionIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	EFMCodexUMGSelectionFeedbackReason SelectionFeedbackReason =
		EFMCodexUMGSelectionFeedbackReason::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString SelectionFeedbackLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FFMCodexUMGCardViewModel Card;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGPitchRegionViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString RegionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString ZoneContextLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString PhysicalHalfLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FString TacticalRegionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	EFMCodexUMGPitchVisualRole VisualRole =
		EFMCodexUMGPitchVisualRole::Midfield;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	FText VisualRoleLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	bool bLocalFacingLane = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	int32 VisualLaneIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	bool bCurrentAttackingSide = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Pitch")
	TArray<FFMCodexUMGPitchSlotViewModel> Slots;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGMatchHeaderViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString PlayerALabel = TEXT("Player A");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString PlayerBLabel = TEXT("Player B");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString ScoreLabel = TEXT("0 - 0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString LeftPlayerLabel = TEXT("Player A");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString RightPlayerLabel = TEXT("Player B");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString LeftScoreLabel = TEXT("0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString RightScoreLabel = TEXT("0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString TurnLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString CurrentAttackerTacticalPointsLabel;

	/** Canonical authoritative phase label, localized only when rendered. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString CurrentPhaseLabel;

	/** Fully projected side ownership for the current attack's TP resource. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bShowLeftTacticalPointChip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bShowRightTacticalPointChip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 LeftTacticalPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 RightTacticalPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FFMCodexUMGAttackTurnTrackerViewModel LeftAttackTurnTracker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FFMCodexUMGAttackTurnTrackerViewModel RightAttackTurnTracker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 CurrentAttackerAttackIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 CurrentAttackerMaxAttackTurns = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bTacticalPointRollReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	int32 CurrentAttackerTacticalPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bHasCurrentAttacker = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bCurrentAttackerOnLeft = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString PlayerAScoreLabel = TEXT("0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString PlayerBScoreLabel = TEXT("0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString CurrentAttackerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString ExpectedActorLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString AttackerStatusLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString ActorStatusLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString MatchResultLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString MatchStatusLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bMatchActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bAttackActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bHumanAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bSystemResolution = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGDeploymentDestinationViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString Label;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGDeploymentChoiceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FFMCodexUMGCardViewModel Card;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGDeploymentDestinationViewModel> Destinations;
};

/**
 * Presentation-only projection of authoritative deployment choices onto pitch
 * slots. It does not query gameplay state or calculate deployment legality.
 */
class FMCODEX_API FFMCodexUMGDeploymentTargetProjector final
{
public:
	static void ProjectSlot(
		FFMCodexUMGPitchSlotViewModel& Slot,
		FName DraggedCardId,
		const TArray<FFMCodexUMGDeploymentChoiceViewModel>& Choices);
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGSelectionChoiceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FName OptionId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString Label;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bHasCard = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FFMCodexUMGCardViewModel Card;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGBranchChoiceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	EFMCodexUMGBranchIntent Intent = EFMCodexUMGBranchIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString Label;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGOneOnOneChoiceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	EFMCodexUMGOneOnOneChoice Choice = EFMCodexUMGOneOnOneChoice::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString Label;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGInteractionViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	EFMCodexUMGInteractionCategory Category =
		EFMCodexUMGInteractionCategory::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString KickerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString TitleLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString ClassificationLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString CategoryLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString ExpectedActorLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString ActionPointLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FString> LegalActionLabels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGCardViewModel> CandidateCards;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGDeploymentChoiceViewModel> DeploymentChoices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGSelectionChoiceViewModel> SelectionChoices;

	/** The normal player-facing selection surface is the deployed pitch card. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bUseOnPitchPlayerSelection = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString OnPitchSelectionHintLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGBranchChoiceViewModel> BranchChoices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGOneOnOneChoiceViewModel> OneOnOneChoices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanStartNewMatch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanRollTacticalPoints = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bHasActingSidePrimaryColor = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FLinearColor ActingSidePrimaryColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanFinishDeployment = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanDecline = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanResolveNoLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanContinue = false;

	/** The Cross terminal action remains typed, but its only button is inline. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bPrimaryActionOwnedByInlineFormula = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bSystemResolution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString PrimaryActionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString DeclineActionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString NoLegalActionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString BranchSectionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	FString EmptyStateLabel;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGDiceResultViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString ContextLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString PurposeLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	int32 RawD6 = 0;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGComparisonEvidenceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString HeadingLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString EvidenceLabel;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGResolutionViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	bool bVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	bool bRejected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	bool bTerminal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	bool bCanContinue = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString ContinueActionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString StepLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString StepSummaryLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString RouteLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	TArray<FFMCodexUMGDiceResultViewModel> DiceResults;

	/** Structured, read-only facts for the future inline Formula surface. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FMatchPlayCurrentAttackResolutionFactProjection FormulaFacts;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	TArray<FString> DiceLabels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	TArray<FFMCodexUMGComparisonEvidenceViewModel> ComparisonEvidence;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString DecisionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString ContinuationLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString TerminalLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString ErrorLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString EmptyStateLabel = TEXT("Waiting for an authoritative result.");
};

UENUM(BlueprintType)
enum class EFMCodexUMGInlineFormulaTermKind : uint8
{
	Attribute,
	RawRoll,
	FixedModifier
};

/** Local presentation phase only; never represents a gameplay-resolution state. */
UENUM(BlueprintType)
enum class EFMCodexUMGInlineFormulaRevealPhase : uint8
{
	None,
	Pending,
	AttackReveal,
	AttackSettled,
	DefenseReveal,
	Completed
};

UENUM(BlueprintType)
enum class EFMCodexUMGCrossDefensiveNarrativePerformer : uint8
{
	None,
	Marker,
	Helper
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGInlineFormulaParticipantViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString RoleLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString PlayerName;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGInlineFormulaTermViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	EFMCodexUMGInlineFormulaTermKind Kind =
		EFMCodexUMGInlineFormulaTermKind::Attribute;

	/** Complete player-facing text for this one structured operand. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString DisplayLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString AttributeLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float SourceValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float Multiplier = 1.0f;

	/** Projected contribution retained for truth/audit; Widget does not sum it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float Contribution = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	int32 RollSequenceIndex = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	int32 RawD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bResolved = false;

	/** True only when this operand matches NextPendingRollSequenceIndex. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bNextPendingRoll = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGInlineFormulaRowViewModel
{
	GENERATED_BODY()

	/** Authoritative row owner retained for stable reveal identity. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString SideLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	TArray<FFMCodexUMGInlineFormulaParticipantViewModel> Participants;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	TArray<FFMCodexUMGInlineFormulaTermViewModel> Terms;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bKnownNonRollSubtotalResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float KnownNonRollSubtotal = 0.0f;

	/** Projection-formatted known subtotal; Widget never sums Terms. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString KnownNonRollSubtotalLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bFinalValueResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float FinalValue = 0.0f;

	/** Presentation-formatted authoritative FinalValue, or "?". */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString FinalValueLabel = TEXT("?");

	/**
	 * Main result-box source selected by Presentation without arithmetic:
	 * FinalValue when resolved, otherwise KnownNonRollSubtotal when resolved.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bDisplayedResultResolved = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bDisplayedResultIsFinalValue = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	float DisplayedResult = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString DisplayedResultLabel = TEXT("?");
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGInlineFormulaSurfaceViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bVisible = false;

	/** Explicit screen-level replacement condition for the legacy overlay. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bSuppressLegacyResolution = false;

	/** Canonical fact identity. This is never derived from localized display text. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FName ContestId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString ContestLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString StatusLabel;

	/** Structured terminal narrative derived from authoritative winner facts. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bNarrativeAvailable = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bNarrativeAttackSuccess = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	EFMCodexUMGCrossDefensiveNarrativePerformer DefensiveNarrativePerformer =
		EFMCodexUMGCrossDefensiveNarrativePerformer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString NarrativeHeadline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString ResultSubtitle;

	/** Authoritative initial-route D6 and enum outcome, formatted by presentation. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString RouteResultLabel;

	/** Canonical deployment/position matches from the facts DTO. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString TacticalPlayerSummaryLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bShowFormulaRows = true;

	/** Legacy display field retained for serialized compatibility; manual roll flow leaves it None. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	EFMCodexUMGInlineFormulaRevealPhase RevealPhase =
		EFMCodexUMGInlineFormulaRevealPhase::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bDiceRevealVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bDiceRolling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString DiceOwnerLabel;

	/** "D6" while rolling, then the exact authoritative RawD6. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString DiceFaceLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	int32 ActiveRollSequenceIndex = INDEX_NONE;

	/** Deterministic visual frame. It is not, and never supplies, a dice value. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	int32 RevealAnimationFrame = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bAttackRowActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bDefenseRowActive = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FFMCodexUMGInlineFormulaRowViewModel AttackRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FFMCodexUMGInlineFormulaRowViewModel DefenseRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	bool bCanContinue = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Inline Formula")
	FString ContinueActionLabel;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGMatchScreenViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGMatchHeaderViewModel Header;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGCardRackViewModel LocalRack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGCardRackViewModel OpponentRack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FString LocalPlayerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	TArray<FFMCodexUMGPitchRegionViewModel> PitchRegions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGInteractionViewModel Interaction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGResolutionViewModel Resolution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGInlineFormulaSurfaceViewModel InlineFormula;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FString DiagnosticLabel;
};

class FMCODEX_API FFMCodexLocalMatchUMGPresentationBuilder final
{
public:
	static FFMCodexUMGCardViewModel BuildCard(
		const FFMCodexLocalMatchCardView& CardView);

	static FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexLocalMatchResolutionFeedback& ResolutionFeedback,
		const FString& DiagnosticMessage,
		EInitialTurnOrderPlayer LocalViewerSide =
			EInitialTurnOrderPlayer::PlayerA);

	static FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& InteractionView,
		const FFMCodexLocalMatchResolutionFeedback& ResolutionFeedback,
		const FString& DiagnosticMessage,
		EInitialTurnOrderPlayer LocalViewerSide,
		const FFMCodexUMGSidePrimaryColors& SidePrimaryColors);
};
