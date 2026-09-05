#pragma once
#include "FMCodexNetworkMatchTypes.h"
#include "FMCodexNetworkPlayerIntent.generated.h"

/** Independent of the much larger authoritative command enum. */
UENUM()
enum class EFMCodexNetworkPlayerIntentKind : uint8
{
	None,
	RequestInitialActionPointRoll,
	DeployOrdinary,
	DeployGoalkeeper,
	FinishDeployment,
	SubmitCarrier,
	SubmitMarker,
	SubmitRunner,
	SubmitHelper
};

UENUM()
enum class EFMCodexNetworkIntentAckCode : uint8
{
	None, Accepted, MatchMismatch, NotParticipant, WrongSide,
	StaleAttackSequence, NotPlayerIntent, InvalidPayload, InvalidPhase,
	DuplicateOrAlreadyResolved, AuthorityRejected, InternalFailure
};

/** Closed kind-tagged members. Each intent requires exactly its own payload shape. */
USTRUCT()
struct FMCODEX_API FFMCodexNetworkPlayerIntentEnvelope
{
	GENERATED_BODY()
	UPROPERTY()
	FGuid MatchInstanceId;
	UPROPERTY()
	int64 RequestId = 0;
	UPROPERTY()
	int64 ExpectedAttackSequence = 0;
	UPROPERTY()
	EFMCodexNetworkPlayerIntentKind IntentKind = EFMCodexNetworkPlayerIntentKind::None;
	UPROPERTY()
	FFMCodexNetworkDeployOrdinaryPayload Deployment;
	UPROPERTY()
	FFMCodexNetworkDeployGoalkeeperPayload Goalkeeper;
	UPROPERTY()
	FFMCodexNetworkSubmitCarrierPayload Carrier;
	UPROPERTY()
	FFMCodexNetworkSubmitMarkerPayload Marker;
	UPROPERTY()
	FFMCodexNetworkSubmitRunnerPayload Runner;
	UPROPERTY()
	FFMCodexNetworkSubmitHelperPayload Helper;
	EFMCodexNetworkIntentAckCode ValidatePayloadShape() const;
};

/** Owner-targeted receipt. Gameplay facts travel exclusively in OwnerView. */
USTRUCT()
struct FMCODEX_API FFMCodexNetworkPlayerIntentAck
{
	GENERATED_BODY()
	UPROPERTY()
	FGuid MatchInstanceId;
	UPROPERTY()
	int64 RequestId = 0;
	UPROPERTY()
	EFMCodexNetworkIntentAckCode Code = EFMCodexNetworkIntentAckCode::None;
	UPROPERTY()
	int32 ViewRevision = 0;
};

/**
 * Constant-size ledger per admitted controller. Reliable ordered RPCs and one
 * pending request require increasing IDs, including retries. Only the server's
 * match can reset it; an old or fabricated envelope cannot reset the ledger.
 */
struct FMCODEX_API FFMCodexNetworkIntentLedger
{
	static constexpr int64 MaxForwardDelta = 1024;
	EFMCodexNetworkIntentAckCode Check(const FGuid& ServerMatch,
		const FFMCodexNetworkPlayerIntentEnvelope& Envelope) const;
	bool Consume(const FGuid& ServerMatch, const FFMCodexNetworkPlayerIntentEnvelope& Envelope);
private:
	FGuid Match;
	int64 HighestRequestId = 0;
};

/** Client-only correlation, never a gameplay authority or prediction model. */
struct FMCODEX_API FFMCodexNetworkIntentClientState
{
	void ObserveView(const FFMCodexNetworkClientViewSnapshot& View);
	bool Begin(const FFMCodexNetworkClientViewSnapshot& View,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginDeployment(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkDeployOrdinaryPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginGoalkeeper(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkDeployGoalkeeperPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginFinishDeployment(const FFMCodexNetworkClientViewSnapshot& View,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginCarrier(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkSubmitCarrierPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginMarker(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkSubmitMarkerPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginRunner(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkSubmitRunnerPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool BeginHelper(const FFMCodexNetworkClientViewSnapshot& View,
		const FFMCodexNetworkSubmitHelperPayload& Choice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	bool ObserveAck(const FFMCodexNetworkPlayerIntentAck& Ack);
	bool IsPending() const { return PendingRequestId != 0; }
	int64 GetPendingRequestId() const { return PendingRequestId; }
	const FFMCodexNetworkPlayerIntentAck& GetLastAck() const { return LastAck; }
private:
	bool BeginIntent(const FFMCodexNetworkClientViewSnapshot& View,
		EFMCodexNetworkPlayerIntentKind Kind, const FFMCodexNetworkDeployOrdinaryPayload& Choice,
		const FFMCodexNetworkDeployGoalkeeperPayload& GoalkeeperChoice,
		const FFMCodexNetworkSubmitCarrierPayload& CarrierChoice,
		const FFMCodexNetworkSubmitMarkerPayload& MarkerChoice,
		const FFMCodexNetworkSubmitRunnerPayload& RunnerChoice,
		const FFMCodexNetworkSubmitHelperPayload& HelperChoice,
		FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope);
	void CompleteIfReady();
	FGuid Match;
	int64 NextRequestId = 1;
	int64 PendingRequestId = 0;
	int32 SeenViewRevision = 0;
	FFMCodexNetworkPlayerIntentAck LastAck;
};
