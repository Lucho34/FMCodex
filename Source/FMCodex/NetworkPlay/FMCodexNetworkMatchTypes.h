#pragma once

#include "CoreMinimal.h"

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

/**
 * Small owner-only transport DTO. It is projected from an already filtered
 * InteractionView and intentionally has no card, nomination, deck or raw-state
 * fields.
 */
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
