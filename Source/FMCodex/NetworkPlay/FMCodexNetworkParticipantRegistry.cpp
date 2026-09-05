#include "FMCodexNetworkParticipantRegistry.h"

#include "FMCodexNetworkMatchPlayerController.h"
#include "FMCodexNetworkMatchPlayerState.h"

FFMCodexNetworkAdmissionResult FFMCodexNetworkParticipantRegistry::Admit(
	AFMCodexNetworkMatchPlayerController* Controller,
	AFMCodexNetworkMatchPlayerState* PlayerState)
{
	FFMCodexNetworkAdmissionResult Result;
	if (!IsValid(Controller) || !IsValid(PlayerState))
	{
		Result.Error = EFMCodexNetworkAdmissionError::InvalidParticipant;
		return Result;
	}

	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		FParticipantRecord& Existing = RecordForSide(Side);
		if (Existing.Controller.Get() == Controller)
		{
			Result.bAccepted = true;
			Result.bAlreadyAdmitted = true;
			Result.AssignedSide = Side;
			return Result;
		}
	}

	const EInitialTurnOrderPlayer AvailableSide = !PlayerA.bEverAssigned
		? EInitialTurnOrderPlayer::PlayerA
		: !PlayerB.bEverAssigned
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::None;
	if (AvailableSide == EInitialTurnOrderPlayer::None)
	{
		Result.Error = EFMCodexNetworkAdmissionError::MatchFull;
		return Result;
	}

	FParticipantRecord& Record = RecordForSide(AvailableSide);
	Record.bEverAssigned = true;
	Record.bConnected = true;
	Record.Controller = Controller;
	Record.PlayerState = PlayerState;
	Result.bAccepted = true;
	Result.AssignedSide = AvailableSide;
	return Result;
}

void FFMCodexNetworkParticipantRegistry::MarkDisconnected(
	AController* Controller)
{
	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		FParticipantRecord& Record = RecordForSide(Side);
		if (Record.Controller.Get() == Controller)
		{
			Record.bConnected = false;
			Record.Controller.Reset();
			Record.PlayerState.Reset();
			return;
		}
	}
}

bool FFMCodexNetworkParticipantRegistry::HasBothParticipants() const
{
	return PlayerA.bConnected && PlayerB.bConnected;
}

bool FFMCodexNetworkParticipantRegistry::HasReservedBothSides() const
{
	return PlayerA.bEverAssigned && PlayerB.bEverAssigned;
}

int32 FFMCodexNetworkParticipantRegistry::GetConnectedParticipantCount() const
{
	return (PlayerA.bConnected ? 1 : 0) + (PlayerB.bConnected ? 1 : 0);
}

EInitialTurnOrderPlayer FFMCodexNetworkParticipantRegistry::ResolveSide(
	const AController* Controller) const
{
	if (PlayerA.Controller.Get() == Controller)
	{
		return EInitialTurnOrderPlayer::PlayerA;
	}
	if (PlayerB.Controller.Get() == Controller)
	{
		return EInitialTurnOrderPlayer::PlayerB;
	}
	return EInitialTurnOrderPlayer::None;
}

AFMCodexNetworkMatchPlayerController*
FFMCodexNetworkParticipantRegistry::FindController(
	const EInitialTurnOrderPlayer Side) const
{
	return RecordForSide(Side).Controller.Get();
}

FFMCodexNetworkParticipantRegistry::FParticipantRecord&
FFMCodexNetworkParticipantRegistry::RecordForSide(
	const EInitialTurnOrderPlayer Side)
{
	check(Side == EInitialTurnOrderPlayer::PlayerA
		|| Side == EInitialTurnOrderPlayer::PlayerB);
	return Side == EInitialTurnOrderPlayer::PlayerA ? PlayerA : PlayerB;
}

const FFMCodexNetworkParticipantRegistry::FParticipantRecord&
FFMCodexNetworkParticipantRegistry::RecordForSide(
	const EInitialTurnOrderPlayer Side) const
{
	check(Side == EInitialTurnOrderPlayer::PlayerA
		|| Side == EInitialTurnOrderPlayer::PlayerB);
	return Side == EInitialTurnOrderPlayer::PlayerA ? PlayerA : PlayerB;
}
