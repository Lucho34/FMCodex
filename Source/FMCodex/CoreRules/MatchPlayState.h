#pragma once

#include "CoreMinimal.h"

#include "MatchPlayBoundActionNormalizedParticipantValues.h"
#include "MatchPlayCardSnapshotAuthority.h"
#include "MatchPlayCurrentAttackActualBranchTypes.h"
#include "MatchPlayCurrentAttackPostRouteRollTypes.h"
#include "MatchPlayCurrentAttackResolutionRollTypes.h"
#include "MatchPlayDeploymentSlotCatalog.h"
#include "MatchPlayGoalkeeperUsageState.h"
#include "MatchPlayRecovery.h"
#include "MatchRuntimeStateTypes.h"
#include "PlayCardResolver.h"
#include "FormulaResolver.h"
#include "SetPieceTypeSelectionQuery.h"
#include "SkillRuleSnapshot.h"

#include "MatchPlayState.generated.h"

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackPhase : uint8
{
	Deployment UMETA(DisplayName = "Deployment"),
	Resolution UMETA(DisplayName = "Resolution"),
	RoutePending UMETA(DisplayName = "Route Pending")
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackRouteKind : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Ordinary = 1 UMETA(DisplayName = "Ordinary"),
	SendingOff = 2 UMETA(DisplayName = "Sending Off"),
	SetPiece = 3 UMETA(DisplayName = "Set Piece")
};

UENUM(BlueprintType)
enum class EMatchPlaySendingOffRouteStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingResolution = 1 UMETA(DisplayName = "Awaiting Resolution"),
	Resolved = 2 UMETA(DisplayName = "Resolved")
};

UENUM(BlueprintType)
enum class EMatchPlaySendingOffSelectionOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	NoEligibleCandidate = 1 UMETA(DisplayName = "No Eligible Candidate"),
	CardEjected = 2 UMETA(DisplayName = "Card Ejected")
};

UENUM(BlueprintType)
enum class EMatchPlaySendingOffGameplayOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	NoGoal = 1 UMETA(DisplayName = "No Goal")
};

UENUM(BlueprintType)
enum class EMatchPlaySetPieceRouteStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingTypeRoll = 1 UMETA(DisplayName = "Awaiting Type Roll"),
	TypeResolved = 2 UMETA(DisplayName = "Type Resolved")
};

UENUM(BlueprintType)
enum class EMatchPlaySetPieceCarrierRouteStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingCarrier = 1 UMETA(DisplayName = "Awaiting Carrier"),
	AwaitingMethod = 2 UMETA(DisplayName = "Awaiting Method"),
	DirectAwaitingAttackRoll = 3
		UMETA(DisplayName = "Direct Awaiting Attack Roll"),
	DirectAwaitingDefenseRoll = 4
		UMETA(DisplayName = "Direct Awaiting Defense Roll"),
	AngledAwaitingRoll = 5 UMETA(DisplayName = "Angled Awaiting Roll"),
	Terminal = 6 UMETA(DisplayName = "Terminal"),
	PowerAwaitingRoll = 7 UMETA(DisplayName = "Power Awaiting Roll"),
	PanenkaAwaitingRoll = 8 UMETA(DisplayName = "Panenka Awaiting Roll")
};

UENUM(BlueprintType)
enum class EMatchPlayShortFreeKickMethod : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Direct = 1 UMETA(DisplayName = "Direct"),
	Angled = 2 UMETA(DisplayName = "Angled")
};

UENUM(BlueprintType)
enum class EMatchPlayShortFreeKickGameplayOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Goal = 1 UMETA(DisplayName = "Goal"),
	NoGoal = 2 UMETA(DisplayName = "No Goal")
};

UENUM(BlueprintType)
enum class EMatchPlayLongFreeKickMethod : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Direct = 1 UMETA(DisplayName = "Direct"),
	Power = 2 UMETA(DisplayName = "Power")
};

UENUM(BlueprintType)
enum class EMatchPlayLongFreeKickGameplayOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Goal = 1 UMETA(DisplayName = "Goal"),
	NoGoal = 2 UMETA(DisplayName = "No Goal")
};

UENUM(BlueprintType)
enum class EMatchPlayPenaltyMethod : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Direct = 1 UMETA(DisplayName = "Direct"),
	Panenka = 2 UMETA(DisplayName = "Panenka")
};

UENUM(BlueprintType)
enum class EMatchPlayPenaltyGameplayOutcome : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Goal = 1 UMETA(DisplayName = "Goal"),
	NoGoal = 2 UMETA(DisplayName = "No Goal")
};

UENUM(BlueprintType)
enum class EMatchPlaySetPieceCornerRouteStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingAttackerNominations = 1
		UMETA(DisplayName = "Awaiting Attacker Nominations")
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackLifecycleState : uint8
{
	Active = 0 UMETA(DisplayName = "Active"),
	TerminalPendingAdvance = 1
		UMETA(DisplayName = "Terminal Pending Advance")
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackSelectionStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingCarrier = 1 UMETA(DisplayName = "Awaiting Carrier"),
	AwaitingMarker = 2 UMETA(DisplayName = "Awaiting Marker"),
	AwaitingSkill = 3 UMETA(DisplayName = "Awaiting Skill"),
	AwaitingRunner = 4 UMETA(DisplayName = "Awaiting Runner"),
	ReadyForResolution = 5 UMETA(DisplayName = "Ready For Resolution"),
	AwaitingHelper = 6 UMETA(DisplayName = "Awaiting Helper"),
	AwaitingBranchIntent = 7
		UMETA(DisplayName = "Awaiting Branch Intent")
};

UENUM(BlueprintType)
enum class EMatchPlayElectiveBranchIntent : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	DirectShot = 1 UMETA(DisplayName = "Direct Shot"),
	DeadCorner = 2 UMETA(DisplayName = "Dead Corner"),
	CrossHigh = 3 UMETA(DisplayName = "Cross High"),
	CrossLow = 4 UMETA(DisplayName = "Cross Low")
};

UENUM(BlueprintType)
enum class EMatchPlayCurrentAttackResolutionStage : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	AwaitingRoute = 1 UMETA(DisplayName = "Awaiting Route"),
	RouteResolved = 2 UMETA(DisplayName = "Route Resolved")
};

UENUM(BlueprintType)
enum class EMatchPlayThroughBallOneOnOneShotChoice : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	ChipShot = 1 UMETA(DisplayName = "Chip Shot"),
	DirectShot = 2 UMETA(DisplayName = "Direct Shot")
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayDeploymentPlacement
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EInitialTurnOrderPlayer PlayerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SlotId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackActionPreparationState
{
	GENERATED_BODY()

	/**
	 * Authority-only marker for participant-first preparation. The historical
	 * name is retained for compatibility; it is not Cross-specific and does not
	 * imply that SkillId or ActionType has already been selected.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bSkillSelectionDeferred = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName HelperCardId = NAME_None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackSelectedAction
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FName HelperCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayElectiveBranchIntent ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionBindingValue
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName CarrierCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName MarkerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName SkillId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	ESkillRuleType ActionType = ESkillRuleType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName RunnerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	FName HelperCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack Resolution Binding")
	EMatchPlayElectiveBranchIntent ElectiveBranchIntent =
		EMatchPlayElectiveBranchIntent::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionSessionParticipant
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bIsPresent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayBoundActionNormalizedParticipantValues Values;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionSessionBundle
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionBindingValue Binding;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EInitialTurnOrderPlayer CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EInitialTurnOrderPlayer CurrentDefendingPlayer =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSessionParticipant Carrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSessionParticipant Marker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bHasRunner = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSessionParticipant Runner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bHasHelper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSessionParticipant Helper;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackResolutionSession
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayCurrentAttackResolutionStage Stage =
		EMatchPlayCurrentAttackResolutionStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSessionBundle Bundle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bHasActualBranch = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackActualBranch ActualBranch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	TArray<FMatchPlayCurrentAttackResolutionRollRecord>
		InitialRouteRollRecords;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackPostRouteRollProgress PostRouteRollProgress;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	EMatchPlayThroughBallOneOnOneShotChoice ThroughBallOneOnOneShotChoice =
		EMatchPlayThroughBallOneOnOneShotChoice::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySendingOffRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Sending Off")
	EMatchPlaySendingOffRouteStage Stage =
		EMatchPlaySendingOffRouteStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Sending Off")
	EMatchPlaySendingOffSelectionOutcome SelectionOutcome =
		EMatchPlaySendingOffSelectionOutcome::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Sending Off")
	FName EjectedCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Sending Off")
	EMatchPlaySendingOffGameplayOutcome GameplayOutcome =
		EMatchPlaySendingOffGameplayOutcome::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySetPieceParticipantBinding
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	bool bIsBound = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FName CardId = NAME_None;

	/** Frozen match-authoritative snapshot accepted with the participant identity. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FPlayerCardRuleSnapshot Snapshot;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayShortFreeKickRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	EMatchPlaySetPieceCarrierRouteStage Stage =
		EMatchPlaySetPieceCarrierRouteStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	FMatchPlaySetPieceParticipantBinding Carrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	EMatchPlayShortFreeKickMethod Method =
		EMatchPlayShortFreeKickMethod::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bHasAttackD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	int32 AttackD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bHasDefenseD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	int32 DefenseD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bHasAngledD6Pair = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	int32 AngledD6A = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	int32 AngledD6B = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bHasFormulaResolution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	FFormulaResolutionResult FormulaResolution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	EMatchPlayShortFreeKickGameplayOutcome GameplayOutcome =
		EMatchPlayShortFreeKickGameplayOutcome::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bHasGoalScorer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	FName GoalScorerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Short Free Kick")
	bool bNoLegalCarrier = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayLongFreeKickRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	EMatchPlaySetPieceCarrierRouteStage Stage =
		EMatchPlaySetPieceCarrierRouteStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	FMatchPlaySetPieceParticipantBinding Carrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	EMatchPlayLongFreeKickMethod Method =
		EMatchPlayLongFreeKickMethod::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bHasAttackD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	int32 AttackD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bHasDefenseD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	int32 DefenseD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bHasPowerD6Pair = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	int32 PowerD6A = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	int32 PowerD6B = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bHasFormulaResolution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	FFormulaResolutionResult FormulaResolution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	EMatchPlayLongFreeKickGameplayOutcome GameplayOutcome =
		EMatchPlayLongFreeKickGameplayOutcome::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bHasGoalScorer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	FName GoalScorerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Long Free Kick")
	bool bNoLegalCarrier = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayPenaltyRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	EMatchPlaySetPieceCarrierRouteStage Stage =
		EMatchPlaySetPieceCarrierRouteStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	FMatchPlaySetPieceParticipantBinding Carrier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	EMatchPlayPenaltyMethod Method = EMatchPlayPenaltyMethod::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bHasAttackD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	int32 AttackD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bHasDefenseD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	int32 DefenseD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bHasPanenkaD6 = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	int32 PanenkaD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bHasFormulaResolution = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	FFormulaResolutionResult FormulaResolution;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	EMatchPlayPenaltyGameplayOutcome GameplayOutcome =
		EMatchPlayPenaltyGameplayOutcome::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bHasGoalScorer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	FName GoalScorerCardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Penalty")
	bool bNoLegalCarrier = false;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCornerRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece|Corner")
	EMatchPlaySetPieceCornerRouteStage Stage =
		EMatchPlaySetPieceCornerRouteStage::None;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlaySetPieceRouteState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	EMatchPlaySetPieceRouteStage Stage =
		EMatchPlaySetPieceRouteStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	bool bHasTypeRoll = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	int32 RawTypeD6 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	ESetPieceSelectedType SelectedType = ESetPieceSelectedType::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FMatchPlayShortFreeKickRouteState ShortFreeKick;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FMatchPlayLongFreeKickRouteState LongFreeKick;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FMatchPlayPenaltyRouteState Penalty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Set Piece")
	FMatchPlayCornerRouteState Corner;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayCurrentAttackState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayCurrentAttackLifecycleState LifecycleState =
		EMatchPlayCurrentAttackLifecycleState::Active;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayCurrentAttackPhase Phase =
		EMatchPlayCurrentAttackPhase::Deployment;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	int32 ActionPoint = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	int32 RawInitialD12 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayCurrentAttackRouteKind RouteKind =
		EMatchPlayCurrentAttackRouteKind::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlaySendingOffRouteState SendingOffRoute;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlaySetPieceRouteState SetPieceRoute;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EMatchPlayCurrentAttackSelectionStage SelectionStage =
		EMatchPlayCurrentAttackSelectionStage::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlayCurrentAttackActionPreparationState ActionPreparation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	EInitialTurnOrderPlayer CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bAttackerDeploymentFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bDefenderDeploymentFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	TArray<FMatchPlayDeploymentPlacement> DeploymentPlacements;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bCurrentDefenseGoalkeeperActivated = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	bool bHasSelectedAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Current Attack")
	FMatchPlayCurrentAttackSelectedAction SelectedAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	bool bHasResolutionSession = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play|Resolution Session")
	FMatchPlayCurrentAttackResolutionSession ResolutionSession;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FMatchPlayState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchRuntimeState RuntimeState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchCardUsageState CardUsageState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayDeploymentSlotCatalog DeploymentSlotCatalog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayPerSideCardSnapshotAuthority CardSnapshotAuthority;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayGoalkeeperUsageState GoalkeeperUsageState;

	/** Latest successful non-final advance event; never consulted for legality. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayLastRecoveryFact LastRecoveryFact;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	bool bHasCurrentAttack = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Core Rules|Match Play")
	FMatchPlayCurrentAttackState CurrentAttack;

private:
	friend class FMatchPlayStateInitializer;

	static FMatchPlayState Create(
		const FMatchRuntimeState& InRuntimeState,
		const FMatchCardUsageState& InCardUsageState,
		const FMatchPlayDeploymentSlotCatalog& InDeploymentSlotCatalog,
		const FMatchPlayPerSideCardSnapshotAuthority&
			InCardSnapshotAuthority,
		const FMatchPlayGoalkeeperUsageState& InGoalkeeperUsageState)
	{
		FMatchPlayState Result;
		Result.RuntimeState = InRuntimeState;
		Result.CardUsageState = InCardUsageState;
		Result.DeploymentSlotCatalog = InDeploymentSlotCatalog;
		Result.CardSnapshotAuthority = InCardSnapshotAuthority;
		Result.GoalkeeperUsageState = InGoalkeeperUsageState;
		return Result;
	}
};
