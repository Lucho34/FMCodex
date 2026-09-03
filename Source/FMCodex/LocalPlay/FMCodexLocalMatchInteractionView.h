#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayState.h"
#include "../CoreRules/MatchPlayCurrentAttackResolutionFactProjection.h"
#include "../CoreRules/MatchResultResolver.h"
#include "FMCodexFullTimePresentation.h"

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
	/** Legacy compatibility only; production uses ApplyCrossTerminalResolution. */
	CompleteCrossAndAdvance,
	RollThroughBallFeetAttack,
	RollThroughBallFeetDefense,
	RollThroughBallAntiOffsideAttack,
	RollThroughBallOneOnOneChipShotAttack,
	RollThroughBallOneOnOneDirectShotAttack,
	RollThroughBallOneOnOneDirectShotDefense,
	RollThroughBallBehindDefenseAttack,
	RollThroughBallBehindDefenseDefense,
	/** Legacy compatibility only; production uses ApplyThroughBallFeetTerminalResolution. */
	CompleteThroughBallFeetAndAdvance,
	ContinueResolution,
	AttackComplete,
	MatchEnded,
	ApplyCrossTerminalResolution,
	ApplyThroughBallFeetTerminalResolution,
	AdvanceAfterTerminal,
	RollThroughBallInitialRoute,
	SelectLongShotBranch,
	RollLongShotDirectAttack,
	RollLongShotDirectDefense,
	RollLongShotDeadCorner,
	RollCutInsideShotDirectAttack,
	RollCutInsideShotDirectDefense,
	RollCutInsideShotDeadCorner,
	RollPassControlRoute,
	RollPassControlAttack,
	RollPassControlDefense,
	RollCrossRoute,
	ResolveSendingOff,
	RollSetPieceType,
	SelectSetPieceCarrier,
	ConfirmSetPieceCarrier,
	SelectSetPieceMethod,
	RollShortFreeKickDirectAttack,
	RollShortFreeKickDirectDefense,
	RollShortFreeKickAngled,
	RollLongFreeKickDirectAttack,
	RollLongFreeKickDirectDefense,
	RollLongFreeKickPower,
	RollPenaltyDirectAttack,
	RollPenaltyDirectDefense,
	RollPenaltyPanenka,
	DraftCornerAttacker,
	DraftCornerDefender,
	RollCornerParticipantSelection,
	SelectCornerIntent,
	RollCornerRoute,
	RollCornerAttack,
	RollCornerDefense
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
	FString EnglishDisplayLabel;
	FString NationalityLabel;
	FString ClubLabel;
	FString PositionLabel;
	FString CompactRoleLabel;
	int32 OverallRating = 0;
	bool bHasOverallRating = false;
	FString BirthDate;
	int32 HeightCm = 0;
	int32 WeightKg = 0;
	struct FAttribute
	{
		FString CanonicalLabel;
		int32 Value = 0;
	};
	struct FSkill
	{
		FName SkillId = NAME_None;
		FString CanonicalLabel;
		int32 MinTriggerActionPoint = 0;
		int32 MaxTriggerActionPoint = 0;
	};
	TArray<FAttribute> AttributeValues;
	TArray<FSkill> Skills;
	TArray<FSkill> EligibleTacticalSkills;
	/**
	 * Already-resolved Hand Micro visibility. Empty unless this available card
	 * belongs to the current attacking side and a CurrentAttack owns TP truth.
	 */
	TArray<FSkill> HandMicroVisibleTacticalSkills;
	/** Resolved Hand Micro count cue. Legal values are 0..2. */
	int32 HandMicroTacticalMatchCount = 0;
	/** True exactly when Hand Micro should render one or two tactical pips. */
	bool bHasHandMicroTacticalMatch = false;
	/**
	 * Already-resolved Pitch Mini visibility. InteractionView owns the
	 * attacking-side gate; presentation widgets must not infer ownership.
	 */
	TArray<FSkill> PitchMiniVisibleTacticalSkills;
	/**
	 * Resolved Pitch Mini count cue. Legal values are 0..2; presentation
	 * widgets must not derive this from Skills or gameplay state.
	 */
	int32 PitchMiniTacticalMatchCount = 0;
	/**
	 * Resolved Pitch Mini status signal. True only for a deployed card on the
	 * current attacking side with at least one TP-matching Tactical Skill.
	 */
	bool bHasPitchMiniTacticalMatch = false;
	FString PlayerFacingSerialLabel;
	FString AttributeSummary;
	FString GoalkeeperAttributeSummary;
	FString CompactAttributeSummary;
	TArray<FString> SkillLabels;
	FString SkillSummaryLabel;
	TArray<FString> StatusLabels;
	FString StatusSummaryLabel;
	FString RarityLabel;
	FString DeveloperReferenceLabel;
	int32 RackSortGroup = 3;
	bool bGoalkeeper = false;
	bool bAvailable = false;
	bool bUsed = false;
	bool bEjected = false;
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
	/** Populated only for authoritative Skill-rule choices. */
	ESkillRuleType SkillType = ESkillRuleType::None;
	FString Label;
	bool bHasCard = false;
	FFMCodexLocalMatchCardView Card;
};

/** Bounded, presentation-facing reasons copied from canonical legality. */
enum class EFMCodexLocalMatchSelectionFeedbackReason : uint8
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

struct FMCODEX_API FFMCodexLocalMatchSelectionFeedbackCandidate
{
	FName CardId = NAME_None;
	EFMCodexLocalMatchSelectionFeedbackReason Reason =
		EFMCodexLocalMatchSelectionFeedbackReason::None;
};

struct FMCODEX_API FFMCodexLocalMatchPitchSlotView
{
	FName SlotId = NAME_None;
	FString Label;
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
	EMatchPlayRelativeDeploymentZone PlayerARelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
	EMatchPlayRelativeDeploymentZone PlayerBRelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
	bool bOccupied = false;
	FFMCodexLocalMatchCardView Card;
};

struct FMCODEX_API FFMCodexLocalMatchPitchRegionView
{
	EMatchPlayNeutralSlotSide NeutralSide =
		EMatchPlayNeutralSlotSide::None;
	FString Label;
	FString ZoneContextLabel;
	EMatchPlayRelativeDeploymentZone PlayerARelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
	EMatchPlayRelativeDeploymentZone PlayerBRelativeZone =
		EMatchPlayRelativeDeploymentZone::None;
	bool bCurrentAttackingSide = false;
	TArray<FFMCodexLocalMatchPitchSlotView> Slots;
};

/** Data-driven projection of one authoritative returned-card fact entry. */
struct FMCODEX_API FFMCodexLocalMatchRecoveryPresentationEntry
{
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FText OwnerDisplayName;
	FString PlayerDisplayName;
	FString PresentationLine;
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
	FFMCodexFullTimePresentation FullTime;
	bool bMatchActive = false;
	bool bMatchEnded = false;
	EMatchResultType MatchResult = EMatchResultType::NotEnded;
	int32 PlayerAScore = 0;
	int32 PlayerBScore = 0;
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EInitialTurnOrderPlayer ExpectedActingPlayer =
		EInitialTurnOrderPlayer::None;
	int32 PlayerAMaxAttackTurns = 0;
	int32 PlayerAUsedAttackTurns = 0;
	int32 PlayerACurrentAttackIndex = 0;
	int32 PlayerBMaxAttackTurns = 0;
	int32 PlayerBUsedAttackTurns = 0;
	int32 PlayerBCurrentAttackIndex = 0;
	/** Canonical board-status counts; never derived from rendered Pitch cards. */
	bool bHasTacticalPlayerCounts = false;
	int32 PlayerATacticalPlayerCount = 0;
	int32 PlayerBTacticalPlayerCount = 0;
	bool bPlayerACurrentAttackTurn = false;
	bool bPlayerBCurrentAttackTurn = false;
	bool bTacticalPointRollReady = false;
	bool bCurrentAttackActive = false;
	int64 AttackSequence = 0;
	int32 ActionPoint = 0;
	int32 RawInitialD12 = 0;
	EMatchPlayCurrentAttackRouteKind RouteKind =
		EMatchPlayCurrentAttackRouteKind::None;
	EMatchPlaySetPieceRouteStage SetPieceStage =
		EMatchPlaySetPieceRouteStage::None;
	ESetPieceSelectedType SetPieceType = ESetPieceSelectedType::None;
	bool bHasSetPieceTypeRoll = false;
	int32 RawSetPieceTypeD6 = 0;
	EMatchPlaySetPieceCarrierRouteStage SetPieceCarrierStage =
		EMatchPlaySetPieceCarrierRouteStage::None;
	EMatchPlaySetPieceCornerRouteStage CornerStage =
		EMatchPlaySetPieceCornerRouteStage::None;
	TArray<FName> LegalSetPieceCardIds;
	FName DraftSetPieceCarrierCardId = NAME_None;
	TArray<FName> DraftCornerNomineeCardIds;
	bool bCornerLockConfirmationPending = false;
	TArray<FString> CornerAttackerNomineeRollLabels;
	TArray<FString> CornerDefenderNomineeRollLabels;
	bool bShortAngledEligible = false;
	FMatchPlaySetPieceParticipantBinding SetPieceCarrier;
	TArray<FMatchPlaySetPieceParticipantBinding> CornerAttackerNominees;
	TArray<FMatchPlaySetPieceParticipantBinding> CornerDefenderNominees;
	bool bCornerAttackerNominationsLocked = false;
	bool bCornerDefenderNominationsLocked = false;
	bool bHideCornerAttackerNomineeDetails = false;
	bool bHasCornerSharedParticipantD6 = false;
	int32 CornerSharedParticipantD6 = 0;
	FMatchPlaySetPieceParticipantBinding CornerRunner;
	FMatchPlaySetPieceParticipantBinding CornerHelper;
	EInitialTurnOrderPlayer CornerCandidateBonusSide =
		EInitialTurnOrderPlayer::None;
	int32 CornerCandidateBonus = 0;
	EMatchPlayCornerRouteIntent CornerIntendedRoute =
		EMatchPlayCornerRouteIntent::None;
	bool bHasCornerRouteD6 = false;
	int32 CornerRouteD6 = 0;
	EMatchPlayCornerRouteIntent CornerActualRoute =
		EMatchPlayCornerRouteIntent::None;
	bool bHasSetPieceAttackD6 = false;
	int32 SetPieceAttackD6 = 0;
	bool bHasSetPieceDefenseD6 = false;
	int32 SetPieceDefenseD6 = 0;
	/** Read-only formula inputs/totals projected from frozen authority state. */
	bool bHasSetPieceAttackKnownSubtotal = false;
	float SetPieceAttackKnownSubtotal = 0.0f;
	bool bHasSetPieceDefenseKnownSubtotal = false;
	float SetPieceDefenseKnownSubtotal = 0.0f;
	bool bHasSetPieceAttackCurrentTotal = false;
	float SetPieceAttackCurrentTotal = 0.0f;
	bool bHasSetPieceDefenseCurrentTotal = false;
	float SetPieceDefenseCurrentTotal = 0.0f;
	bool bHasSetPiecePairedD6 = false;
	int32 SetPiecePairedD6A = 0;
	int32 SetPiecePairedD6B = 0;
	/** Read-only projection of the canonical stored pair; never drives gameplay. */
	int32 SetPiecePairedD6Total = 0;
	bool bHasSetPieceFormula = false;
	FFormulaResolutionResult SetPieceFormula;
	bool bHasSetPieceOutcome = false;
	bool bSetPieceGoal = false;
	bool bSetPieceSystemGoal = false;
	bool bSetPieceNoLegalCarrier = false;
	FName SetPieceGoalScorerCardId = NAME_None;
	FName SendingOffEjectedCardId = NAME_None;
	EInitialTurnOrderPlayer SendingOffEjectedOwnerSide =
		EInitialTurnOrderPlayer::None;
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
	/** Bounded canonical reason key for a normal, non-rejection selection notice. */
	FString SelectionNotice;
	/** Typed presentation identity; player-facing code must not parse ActionLabel. */
	ESkillRuleType PresentedActionType = ESkillRuleType::None;
	FString ActionLabel;
	EMatchPlayElectiveBranchIntent ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::None;
	FString ActualBranchLabel;
	FString OneOnOneChoiceLabel;
	FString AttackDirectionLabel;
	TArray<FMatchPlayDeploymentPlacement> DeploymentPlacements;
	TArray<FFMCodexLocalMatchDeploymentOption> DeploymentOptions;
	TArray<FFMCodexLocalMatchDeploymentGroup> DeploymentGroups;
	TArray<FFMCodexLocalMatchSelectionOption> SelectionOptions;
	TArray<FFMCodexLocalMatchSelectionFeedbackCandidate>
		SelectionFeedbackCandidates;
	FName SelectedCarrierCardId = NAME_None;
	FName SelectedRunnerCardId = NAME_None;
	FName SelectedMarkerCardId = NAME_None;
	FName SelectedHelperCardId = NAME_None;
	TArray<FFMCodexLocalMatchPitchRegionView> PitchRegions;
	TArray<FFMCodexLocalMatchCardView> PlayerACardRoster;
	TArray<FFMCodexLocalMatchCardView> PlayerBCardRoster;
	bool bHasRecoveryFact = false;
	int64 RecoverySourceAttackSequence = 0;
	TArray<FFMCodexLocalMatchRecoveryPresentationEntry>
		RecoveryPresentationEntries;
	TArray<EMatchPlayElectiveBranchIntent> BranchIntentOptions;
	TArray<EMatchPlayThroughBallOneOnOneShotChoice> OneOnOneOptions;
	TArray<FFMCodexLocalMatchRollView> AcceptedRolls;
	FMatchPlayCurrentAttackResolutionFactProjection ResolutionFacts;
	bool bCrossAttackRollPending = false;
	bool bCrossDefenseRollPending = false;
	bool bCrossFormulaComplete = false;
	bool bCrossTerminalActionAvailable = false;
	bool bLongShotDirectAttackRollPending = false;
	bool bLongShotDirectDefenseRollPending = false;
	bool bLongShotDeadCornerRollPending = false;
	bool bCutInsideShotDirectAttackRollPending = false;
	bool bCutInsideShotDirectDefenseRollPending = false;
	bool bCutInsideShotDeadCornerRollPending = false;
	bool bThroughBallFeetAttackRollPending = false;
	bool bThroughBallFeetDefenseRollPending = false;
	bool bThroughBallAntiOffsideAttackRollPending = false;
	bool bThroughBallOneOnOneChipShotAttackRollPending = false;
	bool bThroughBallOneOnOneDirectShotAttackRollPending = false;
	bool bThroughBallOneOnOneDirectShotDefenseRollPending = false;
	bool bThroughBallBehindDefenseAttackRollPending = false;
	bool bThroughBallBehindDefenseDefenseRollPending = false;
	bool bThroughBallFeetFormulaComplete = false;
	bool bThroughBallFeetTerminalActionAvailable = false;
	bool bTerminalPendingAdvance = false;
	FString ContinueActionLabel = TEXT("继续结算");
};

struct FMCODEX_API FFMCodexLocalMatchScreenPresentation
{
	FString MatchStatusLabel;
	FString ActingStatusLabel;
	FString InteractionKicker;
	FString InteractionTitle;
	bool bSystemResolution = false;
};

class FMCODEX_API FFMCodexLocalMatchInteractionViewBuilder final
{
public:
	static FFMCodexLocalMatchInteractionView BuildNoActiveMatch();

	static FFMCodexLocalMatchInteractionView Build(
		const FMatchPlayState& Snapshot,
		const FSkillRuleSnapshotSet& SkillRuleSet);

	static FFMCodexLocalMatchScreenPresentation BuildScreenPresentation(
		const FFMCodexLocalMatchInteractionView& View);

	static TArray<FFMCodexLocalMatchCardView::FSkill>
	ProjectEligibleTacticalSkills(
		const TArray<FFMCodexLocalMatchCardView::FSkill>& StaticSkills,
		int32 CurrentTacticalPoint);

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
