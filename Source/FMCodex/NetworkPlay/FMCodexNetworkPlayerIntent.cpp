#include "FMCodexNetworkPlayerIntent.h"

bool FFMCodexNetworkIntentLedger::Consume(const FGuid& ServerMatch,
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
	if (!ServerMatch.IsValid() || Envelope.MatchInstanceId != ServerMatch || Envelope.RequestId <= 0)
	{
		return false;
	}
	if (Match != ServerMatch)
	{
		Match = ServerMatch;
		HighestRequestId = 0;
	}
	if (Envelope.RequestId <= HighestRequestId)
	{
		return false;
	}
	HighestRequestId = Envelope.RequestId;
	return true;
}

void FFMCodexNetworkIntentClientState::ObserveView(const FFMCodexNetworkClientViewSnapshot& View)
{
	if (!View.MatchInstanceId.IsValid())
	{
		return;
	}
	if (Match != View.MatchInstanceId)
	{
		Match = View.MatchInstanceId;
		PendingRequestId = 0;
		SeenViewRevision = 0;
		LastAck = {};
		// IDs stay monotonic for this controller's lifetime, including new matches.
	}
	SeenViewRevision = FMath::Max(SeenViewRevision, View.ViewRevision);
	CompleteIfReady();
}

bool FFMCodexNetworkIntentClientState::Begin(const FFMCodexNetworkClientViewSnapshot& View,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	ObserveView(View);
	if (IsPending() || !Match.IsValid() || NextRequestId == MAX_int64
		|| View.ViewRevision < SeenViewRevision || !View.bMatchInitialized
		|| View.BootstrapState != EFMCodexNetworkBootstrapState::MatchReady
		|| View.InteractionState != EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint
		|| View.ViewerSide == EInitialTurnOrderPlayer::None
		|| View.ExpectedActingSide != View.ViewerSide || View.AttackSequence <= 0)
	{
		return false;
	}
	OutEnvelope.MatchInstanceId = Match;
	OutEnvelope.RequestId = NextRequestId++;
	OutEnvelope.ExpectedAttackSequence = View.AttackSequence;
	OutEnvelope.IntentKind = EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll;
	PendingRequestId = OutEnvelope.RequestId;
	return true;
}

bool FFMCodexNetworkIntentClientState::ObserveAck(const FFMCodexNetworkPlayerIntentAck& Ack)
{
	if (!IsPending() || Ack.MatchInstanceId != Match || Ack.RequestId != PendingRequestId
		|| Ack.Code == EFMCodexNetworkIntentAckCode::None || Ack.ViewRevision < 0
		|| (LastAck.MatchInstanceId == Match && LastAck.RequestId == PendingRequestId))
	{
		return false;
	}
	LastAck = Ack;
	CompleteIfReady();
	return true;
}

void FFMCodexNetworkIntentClientState::CompleteIfReady()
{
	if (IsPending() && LastAck.MatchInstanceId == Match && LastAck.RequestId == PendingRequestId
		&& (LastAck.Code != EFMCodexNetworkIntentAckCode::Accepted
			|| SeenViewRevision >= LastAck.ViewRevision))
	{
		PendingRequestId = 0;
	}
}
