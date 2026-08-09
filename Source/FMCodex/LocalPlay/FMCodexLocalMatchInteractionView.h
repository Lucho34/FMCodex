#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayState.h"
#include "../CoreRules/MatchResultResolver.h"

enum class EFMCodexLocalMatchMajorPhase : uint8
{
	NoActiveMatch,
	BetweenAttacks,
	Deployment,
	Selection,
	Resolution,
	Complete
};

enum class EFMCodexLocalMatchInteractionCategory : uint8
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

struct FMCODEX_API FFMCodexLocalMatchDeploymentOption
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FName SlotId = NAME_None;
	bool bGoalkeeper = false;
};

struct FMCODEX_API FFMCodexLocalMatchSlotView
{
	FName SlotId = NAME_None;
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
	EMatchPlayRelativeDeploymentZone RelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
	FString Label;
};

struct FMCODEX_API FFMCodexLocalMatchCardView
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FString DisplayLabel;
	FString PositionLabel;
	FString AttributeSummary;
	FString GoalkeeperAttributeSummary;
	TArray<FString> SkillLabels;
	bool bGoalkeeper = false;
	bool bAvailable = false;
	bool bUsed = false;
	bool bDeployed = false;
	bool bGoalkeeperUsedThisMatch = false;
	bool bGoalkeeperActivatedThisAttack = false;
	FName SlotId = NAME_None;
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
	EMatchPlayRelativeDeploymentZone RelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
};

struct FMCODEX_API FFMCodexLocalMatchDeploymentGroup
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	bool bGoalkeeper = false;
	TArray<FName> LegalSlotIds;
	FFMCodexLocalMatchCardView Card;
	TArray<FFMCodexLocalMatchSlotView> LegalSlots;
};

struct FMCODEX_API FFMCodexLocalMatchSelectionOption
{
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	FName Id = NAME_None;
	FName RelatedCardId = NAME_None;
	FString Label;
	bool bHasCard = false;
	FFMCodexLocalMatchCardView Card;
};

struct FMCODEX_API FFMCodexLocalMatchPitchRegionView
{
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
	FString Label;
	TArray<FName> CanonicalSlotIds;
	TArray<FFMCodexLocalMatchCardView> DeployedCards;
};

enum class EFMCodexLocalMatchRollGroup : uint8
{
	InitialRoute,
	PostRoute,
	OneOnOne
};

struct FMCODEX_API FFMCodexLocalMatchRollView
{
	EFMCodexLocalMatchRollGroup Group =
		EFMCodexLocalMatchRollGroup::InitialRoute;
	FString Purpose;
	int32 RawD6 = 0;
};

struct FMCODEX_API FFMCodexLocalMatchInteractionView
{
	bool bMatchActive = false;
	bool bMatchEnded = false;
	EMatchResultType MatchResult = EMatchResultType::NotEnded;
	int32 PlayerAScore = 0;
	int32 PlayerBScore = 0;
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer ExpectedActingPlayer =
		EInitialTurnOrderPlayer::None;
	bool bCurrentAttackActive = false;
	int64 AttackSequence = 0;
	int32 ActionPoint = 0;
	EFMCodexLocalMatchMajorPhase MajorPhase =
		EFMCodexLocalMatchMajorPhase::NoActiveMatch;
	EMatchPlayCurrentAttackSelectionStage SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::None;
	EInitialTurnOrderPlayer CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None;
	EFMCodexLocalMatchInteractionCategory InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	bool bHumanInteraction = false;
	bool bCanDecline = false;
	bool bCanResolveNoLegalChoice = false;
	FString Diagnostic;
	FString ActionLabel;
	FString ActualBranchLabel;
	FString OneOnOneChoiceLabel;
	TArray<FMatchPlayDeploymentPlacement> DeploymentPlacements;
	TArray<FFMCodexLocalMatchDeploymentOption> DeploymentOptions;
	TArray<FFMCodexLocalMatchDeploymentGroup> DeploymentGroups;
	TArray<FFMCodexLocalMatchSelectionOption> SelectionOptions;
	TArray<FFMCodexLocalMatchPitchRegionView> PitchRegions;
	TArray<EMatchPlayElectiveBranchIntent> BranchIntentOptions;
	TArray<EMatchPlayThroughBallOneOnOneShotChoice> OneOnOneOptions;
	TArray<FFMCodexLocalMatchRollView> AcceptedRolls;
	FString ContinueActionLabel = TEXT("Continue Resolution");
};

class FMCODEX_API FFMCodexLocalMatchInteractionViewBuilder final
{
public:
	static FFMCodexLocalMatchInteractionView BuildNoActiveMatch();

	static FFMCodexLocalMatchInteractionView Build(
		const FMatchPlayState& Snapshot,
		const FSkillRuleSnapshotSet& SkillRuleSet);

	static FString ToString(EFMCodexLocalMatchMajorPhase Phase);
	static FString ToString(EFMCodexLocalMatchInteractionCategory Category);
	static FString ToString(EInitialTurnOrderPlayer Player);
	static FString ToString(EMatchResultType Result);
	static FString ToString(EPlayerPositionType Position);
	static FString ToString(EMatchPlayNeutralSlotSide Side);
	static FString ToString(EMatchPlayRelativeDeploymentZone Zone);
	static FString ToString(ESkillRuleType SkillType);
	static FString ToString(
		const FMatchPlayCurrentAttackActualBranch& ActualBranch);
	static FString ToString(
		EMatchPlayThroughBallOneOnOneShotChoice Choice);
};
