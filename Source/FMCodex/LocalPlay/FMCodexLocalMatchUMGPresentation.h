#pragma once

#include "CoreMinimal.h"

#include "FMCodexLocalMatchUMGPresentation.generated.h"

struct FFMCodexLocalMatchCardView;
struct FFMCodexLocalMatchInteractionView;
struct FFMCodexLocalMatchResolutionFeedback;

UENUM(BlueprintType)
enum class EFMCodexUMGInteractionCategory : uint8
{
	None,
	StartMatch,
	BeginAttack,
	Deploy,
	SelectCarrier,
	SelectMarker,
	SelectSkill,
	SelectRunner,
	SelectHelper,
	SelectBranchIntent,
	SelectOneOnOneShot,
	ContinueResolution,
	AttackComplete,
	MatchEnded
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

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGCardViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString IdentityLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString OwnerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Card")
	FString RoleLabel;

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
	FString CurrentAttackerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString ExpectedActorLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	FString MatchStatusLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Header")
	bool bMatchEnded = false;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGBranchChoiceViewModel> BranchChoices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	TArray<FFMCodexUMGOneOnOneChoiceViewModel> OneOnOneChoices;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanStartNewMatch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanBeginOrdinaryAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanFinishDeployment = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanDecline = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanResolveNoLegal = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Interaction")
	bool bCanContinue = false;

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
	FString StepLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	TArray<FString> DiceLabels;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString DecisionLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString TerminalLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Resolution")
	FString ErrorLabel;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGHandoffViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Handoff")
	bool bVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Handoff")
	FString TitleLabel = TEXT("PASS CONTROL");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Handoff")
	FString NextPlayerLabel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Handoff")
	FString ReadyLabel = TEXT("Ready");
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexUMGMatchScreenViewModel
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGMatchHeaderViewModel Header;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	TArray<FFMCodexUMGPitchRegionViewModel> PitchRegions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGInteractionViewModel Interaction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGResolutionViewModel Resolution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Local Match|Screen")
	FFMCodexUMGHandoffViewModel Handoff;

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
		bool bAwaitingHandoff,
		const FString& PendingPlayerLabel);
};
