#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/InitialTurnOrderResolver.h"

class AFMCodexNetworkMatchPlayerController;
class AFMCodexNetworkMatchPlayerState;

enum class EFMCodexNetworkAdmissionError : uint8
{
	None,
	InvalidParticipant,
	MatchFull
};

struct FMCODEX_API FFMCodexNetworkAdmissionResult
{
	bool bAccepted = false;
	bool bAlreadyAdmitted = false;
	EInitialTurnOrderPlayer AssignedSide = EInitialTurnOrderPlayer::None;
	EFMCodexNetworkAdmissionError Error =
		EFMCodexNetworkAdmissionError::None;
};

/** Server-only connection-to-side registry. Reserved sides are never recycled. */
class FMCODEX_API FFMCodexNetworkParticipantRegistry final
{
public:
	FFMCodexNetworkAdmissionResult Admit(
		AFMCodexNetworkMatchPlayerController* Controller,
		AFMCodexNetworkMatchPlayerState* PlayerState);

	void MarkDisconnected(AController* Controller);

	bool HasBothParticipants() const;
	bool HasReservedBothSides() const;
	int32 GetConnectedParticipantCount() const;
	EInitialTurnOrderPlayer ResolveSide(const AController* Controller) const;
	AFMCodexNetworkMatchPlayerController* FindController(
		EInitialTurnOrderPlayer Side) const;

private:
	struct FParticipantRecord
	{
		bool bEverAssigned = false;
		bool bConnected = false;
		TWeakObjectPtr<AFMCodexNetworkMatchPlayerController> Controller;
		TWeakObjectPtr<AFMCodexNetworkMatchPlayerState> PlayerState;
	};

	FParticipantRecord& RecordForSide(EInitialTurnOrderPlayer Side);
	const FParticipantRecord& RecordForSide(
		EInitialTurnOrderPlayer Side) const;

	FParticipantRecord PlayerA;
	FParticipantRecord PlayerB;
};
