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
	None, InitialD12, Deployment, SetPieceTypeRoll, TerminalPendingAdvance
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
