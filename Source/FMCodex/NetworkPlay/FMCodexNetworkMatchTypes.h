#pragma once

#include "CoreMinimal.h"
#include "FMCodexNetworkDeploymentPayload.h"

#include "../CoreRules/InitialTurnOrderResolver.h"

#include "FMCodexNetworkMatchTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFMCodexNetworkPlay, Log, All);

UENUM(BlueprintType)
enum class EFMCodexNetworkBootstrapState : uint8
{
	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
	MatchReady UMETA(DisplayName = "Match Ready"),
	ParticipantDisconnected UMETA(DisplayName = "Participant Disconnected"),
	MatchEnded UMETA(DisplayName = "Match Ended"),
	BootstrapFailed UMETA(DisplayName = "Bootstrap Failed")
};

UENUM(BlueprintType)
enum class EFMCodexNetworkClientInteractionState : uint8
{
	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
	WaitingForOwnInitialActionPoint
		UMETA(DisplayName = "Waiting For Own Initial Action Point"),
	WaitingForOpponentInitialActionPoint
		UMETA(DisplayName = "Waiting For Opponent Initial Action Point"),
	WaitingForOwnIntent UMETA(DisplayName = "Waiting For Own Intent"),
	WaitingForOpponentIntent UMETA(DisplayName = "Waiting For Opponent Intent"),
	MatchEnded UMETA(DisplayName = "Match Ended")
};

/** Already disclosed entry facts only, never full route state. */
UENUM(BlueprintType)
enum class EFMCodexNetworkEntryBranch : uint8
{
	None, SendingOff, Ordinary, SetPiece
};
UENUM(BlueprintType)
enum class EFMCodexNetworkEntryWait : uint8
{
	None, InitialD12, Deployment, SetPieceTypeRoll, TerminalPendingAdvance, CarrierSelection, MarkerSelection, RunnerSelection, SkillSelection, HelperSelection, BranchIntentSelection, PassControlRouteRoll, ThroughBallRouteRoll, CrossRouteRoll, LongShotDirectAttackRoll, LongShotDeadCornerRoll, CutInsideDirectAttackRoll, CutInsideDeadCornerRoll, CrossAttackRoll, PassControlAttackRoll, ThroughBallFeetAttackRoll, ThroughBallBehindDefenseAttackRoll, ThroughBallAntiOffsideAttackRoll
};


/** A safe offered player action, not a client-supplied route result. */
UENUM(BlueprintType)
enum class EFMCodexNetworkInitialRouteAction : uint8
{
	None, Cross, PassControl, ThroughBall
};

/** Only the disclosed initial route fact. No contest, Formula or terminal data. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkInitialRouteFact
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 D6 = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	ESkillRuleType ActionType = ESkillRuleType::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EMatchPlayCrossActualBranch Cross = EMatchPlayCrossActualBranch::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EMatchPlayPassControlActualBranch PassControl = EMatchPlayPassControlActualBranch::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EMatchPlayThroughBallActualBranch ThroughBall = EMatchPlayThroughBallActualBranch::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText RouteLabel;
};

/** Team/content identity is deliberately separate from the connected player. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkTeamIdentity
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FName TeamId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FString TeamDisplayName;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkParticipantPublicIdentity
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bAssigned = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bConnected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EInitialTurnOrderPlayer GameplaySide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FString PlayerDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkTeamIdentity Team;
};

/** One offered legal choice; labels are presentation only, never command identity. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkDeploymentOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkDeployOrdinaryPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText SlotLabel;
};

/** At most one offered goalkeeper slot. The card identity is chosen by Session. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkGoalkeeperOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkDeployGoalkeeperPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText SlotLabel;
};

/** One canonical Carrier identity and its existing player-facing name. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkCarrierOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitCarrierPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
};

/** One canonical Marker identity and its existing player-facing name. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkMarkerOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitMarkerPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
};

/** One canonical Runner identity and its existing player-facing name. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkRunnerOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitRunnerPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
};

/** One canonical Helper identity and its existing player-facing name. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkHelperOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitHelperPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText CardLabel;
};

/** One canonical Skill identity and its existing player-facing name. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkBranchOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitBranchIntentPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText BranchLabel;
};

USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkSkillOption
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSubmitSkillPayload Choice;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FText SkillLabel;
};

/** Public accepted placement, projected from State via the safe viewer view. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkDeploymentSummary
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EInitialTurnOrderPlayer Side = EInitialTurnOrderPlayer::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkDeploymentOption Placement;
};

/** Owner-safe bounded choices plus public facts; no raw State, hand or deck. */
USTRUCT(BlueprintType)
struct FMCODEX_API FFMCodexNetworkClientViewSnapshot
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FGuid MatchInstanceId;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 ViewRevision = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EInitialTurnOrderPlayer ViewerSide = EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EFMCodexNetworkBootstrapState BootstrapState =
		EFMCodexNetworkBootstrapState::WaitingForPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EFMCodexNetworkClientInteractionState InteractionState =
		EFMCodexNetworkClientInteractionState::WaitingForPlayers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bMatchInitialized = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bMatchEnded = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int64 AttackSequence = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EInitialTurnOrderPlayer CurrentAttackingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EInitialTurnOrderPlayer ExpectedActingSide =
		EInitialTurnOrderPlayer::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 PlayerAScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 PlayerBScore = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 PlayerAMaxAttackOpportunities = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 PlayerBMaxAttackOpportunities = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 DisclosedInitialD12 = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EFMCodexNetworkEntryBranch EntryBranch = EFMCodexNetworkEntryBranch::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EFMCodexNetworkEntryWait EntryWait = EFMCodexNetworkEntryWait::None;

	static constexpr int32 MaxDeploymentOptions = 3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkDeploymentOption> DeploymentOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	int32 DeploymentCount = 0;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkDeploymentSummary LastDeployment;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bCanDeployGoalkeeper = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkGoalkeeperOption GoalkeeperOption;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkDeploymentSummary GoalkeeperDeployment;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bCanFinishDeployment = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bPlayerADeploymentFinished = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bPlayerBDeploymentFinished = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bDeploymentComplete = false;

	// Canonical deck: 20 cards, exactly one GK; at most 19 distinct legal Carriers.
	static constexpr int32 MaxCarrierOptions = 19;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkCarrierOption> CarrierOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bCarrierOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkCarrierOption SelectedCarrier;

	// Markers are unique deployed non-GK defenders: <=19 per valid 20-card/one-GK deck.
	// Same physical area further restricts this set (at most 4 on the prototype five-slot half).
	static constexpr int32 MaxMarkerOptions = 19;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkMarkerOption> MarkerOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bMarkerOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkMarkerOption SelectedMarker;
	// Runner excludes the unique GK and frozen Carrier: <=18 per valid deck.
	// The ten shared prototype slots further bound legal Runner choices to eight.
	static constexpr int32 MaxRunnerOptions = 18;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkRunnerOption> RunnerOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bRunnerOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkRunnerOption SelectedRunner;
	// Helper excludes the unique GK and frozen Marker: <=18 per valid defending deck.
	// Same Runner half has five shared slots: at most four Helpers when Carrier/Marker are elsewhere.
	static constexpr int32 MaxHelperOptions = 18;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkHelperOption> HelperOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bHelperOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkHelperOption SelectedHelper;
	// Canonical validated Carrier has <=3 SkillIds; current prototype has <=2 simultaneously legal.
	static constexpr int32 MaxSkillOptions = 3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkSkillOption> SkillOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bSkillOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkSkillOption SelectedSkill;
	/** Every canonical elective family currently offers exactly two branches. */
	static constexpr int32 MaxBranchOptions = 2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	TArray<FFMCodexNetworkBranchOption> BranchOptions;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	bool bBranchOptionsUnavailable = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkBranchOption SelectedBranch;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	EFMCodexNetworkInitialRouteAction InitialRouteAction = EFMCodexNetworkInitialRouteAction::None;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network Play")
	FFMCodexNetworkInitialRouteFact InitialRoute;
};

struct FFMCodexLocalMatchInteractionView;

class FMCODEX_API FFMCodexNetworkClientViewSnapshotFactory final
{
public:
	static FFMCodexNetworkClientViewSnapshot BuildWaiting(
		const FGuid& MatchInstanceId,
		int32 ViewRevision,
		EInitialTurnOrderPlayer ViewerSide,
		EFMCodexNetworkBootstrapState BootstrapState);

	static FFMCodexNetworkClientViewSnapshot Build(
		const FFMCodexLocalMatchInteractionView& SafeViewerView,
		const FGuid& MatchInstanceId,
		int32 ViewRevision,
		EInitialTurnOrderPlayer ViewerSide,
		EFMCodexNetworkBootstrapState BootstrapState);
};
