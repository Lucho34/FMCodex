#include "FMCodexNetworkPlayerIntent.h"

EFMCodexNetworkIntentAckCode FFMCodexNetworkPlayerIntentEnvelope::ValidatePayloadShape() const
{
	using Code = EFMCodexNetworkIntentAckCode;
	switch (IntentKind)
	{
	case EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll:
	case EFMCodexNetworkPlayerIntentKind::FinishDeployment:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::DeployOrdinary:
		return Deployment.IsValidShape() && Goalkeeper.IsEmpty() && Carrier.IsEmpty() ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper:
		return Deployment.IsEmpty() && Goalkeeper.IsValidShape() && Carrier.IsEmpty() ? Code::None : Code::InvalidPayload;
	case EFMCodexNetworkPlayerIntentKind::SubmitCarrier:
		return Deployment.IsEmpty() && Goalkeeper.IsEmpty() && Carrier.IsValidShape() ? Code::None : Code::InvalidPayload;
	default:
		return Code::NotPlayerIntent;
	}
}
EFMCodexNetworkIntentAckCode FFMCodexNetworkIntentLedger::Check(
	const FGuid& ServerMatch, const FFMCodexNetworkPlayerIntentEnvelope& Envelope) const
{
	using Code = EFMCodexNetworkIntentAckCode;
	if (!ServerMatch.IsValid() || Envelope.MatchInstanceId != ServerMatch) { return Code::MatchMismatch; }
	if (Envelope.RequestId <= 0) { return Code::InvalidPayload; }
	const int64 HighWater = Match == ServerMatch ? HighestRequestId : 0;
	if (Envelope.RequestId <= HighWater) { return Code::DuplicateOrAlreadyResolved; }
	// Both operands are nonnegative and RequestId > HighWater: subtraction cannot overflow.
	if (Envelope.RequestId - HighWater > MaxForwardDelta) { return Code::InvalidPayload; }
	return Code::None;
}

bool FFMCodexNetworkIntentLedger::Consume(const FGuid& ServerMatch,
	const FFMCodexNetworkPlayerIntentEnvelope& Envelope)
{
	if (Check(ServerMatch, Envelope) != EFMCodexNetworkIntentAckCode::None)
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
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll, {}, {}, {}, OutEnvelope);
}

bool FFMCodexNetworkIntentClientState::BeginDeployment(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkDeployOrdinaryPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::DeployOrdinary, Choice, {}, {}, OutEnvelope);
}

bool FFMCodexNetworkIntentClientState::BeginGoalkeeper(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkDeployGoalkeeperPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper, {}, Choice, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginFinishDeployment(const FFMCodexNetworkClientViewSnapshot& View,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::FinishDeployment, {}, {}, {}, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginCarrier(const FFMCodexNetworkClientViewSnapshot& View,
	const FFMCodexNetworkSubmitCarrierPayload& Choice, FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	return BeginIntent(View, EFMCodexNetworkPlayerIntentKind::SubmitCarrier, {}, {}, Choice, OutEnvelope);
}
bool FFMCodexNetworkIntentClientState::BeginIntent(const FFMCodexNetworkClientViewSnapshot& View,
	EFMCodexNetworkPlayerIntentKind Kind, const FFMCodexNetworkDeployOrdinaryPayload& Choice,
	const FFMCodexNetworkDeployGoalkeeperPayload& GoalkeeperChoice,
	const FFMCodexNetworkSubmitCarrierPayload& CarrierChoice,
	FFMCodexNetworkPlayerIntentEnvelope& OutEnvelope)
{
	ObserveView(View);
	bool bActionable = false;
	switch (Kind)
	{
	case EFMCodexNetworkPlayerIntentKind::RequestInitialActionPointRoll:
		bActionable = View.InteractionState == EFMCodexNetworkClientInteractionState::WaitingForOwnInitialActionPoint;
		break;
	case EFMCodexNetworkPlayerIntentKind::DeployOrdinary:
		bActionable = View.EntryBranch == EFMCodexNetworkEntryBranch::Ordinary
			&& View.EntryWait == EFMCodexNetworkEntryWait::Deployment && !View.DeploymentOptions.IsEmpty();
		break;
	case EFMCodexNetworkPlayerIntentKind::DeployGoalkeeper:
		bActionable = View.bCanDeployGoalkeeper;
		break;
	case EFMCodexNetworkPlayerIntentKind::FinishDeployment:
		bActionable = View.bCanFinishDeployment;
		break;
	case EFMCodexNetworkPlayerIntentKind::SubmitCarrier:
		bActionable = View.EntryWait == EFMCodexNetworkEntryWait::CarrierSelection
			&& !View.bCarrierOptionsUnavailable && !View.CarrierOptions.IsEmpty();
		break;
	default: break;
	}
	FFMCodexNetworkPlayerIntentEnvelope Candidate;
	Candidate.IntentKind = Kind;
	Candidate.Deployment = Choice;
	Candidate.Goalkeeper = GoalkeeperChoice;
	Candidate.Carrier = CarrierChoice;
	if (IsPending() || !Match.IsValid() || NextRequestId == MAX_int64
		|| View.ViewRevision < SeenViewRevision || !View.bMatchInitialized
		|| View.BootstrapState != EFMCodexNetworkBootstrapState::MatchReady
		|| !bActionable || Candidate.ValidatePayloadShape() != EFMCodexNetworkIntentAckCode::None
		|| View.ViewerSide == EInitialTurnOrderPlayer::None
		|| View.ExpectedActingSide != View.ViewerSide || View.AttackSequence <= 0)
	{
		return false;
	}
	OutEnvelope = Candidate;
	OutEnvelope.MatchInstanceId = Match;
	OutEnvelope.RequestId = NextRequestId++;
	OutEnvelope.ExpectedAttackSequence = View.AttackSequence;
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
