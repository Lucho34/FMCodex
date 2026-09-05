#include "FMCodexNetworkMatchGameState.h"

#include "FMCodexNetworkMatchPlayerController.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AFMCodexNetworkMatchGameState::AFMCodexNetworkMatchGameState()
{
	bReplicates = true;
}

void AFMCodexNetworkMatchGameState::SetBootstrapStateOnServer(
	const FGuid& InMatchInstanceId,
	const EFMCodexNetworkBootstrapState InBootstrapState,
	const FFMCodexNetworkParticipantPublicIdentity& InPlayerA,
	const FFMCodexNetworkParticipantPublicIdentity& InPlayerB)
{
	check(HasAuthority());
	MatchInstanceId = InMatchInstanceId;
	BootstrapState = InBootstrapState;
	PlayerAIdentity = InPlayerA;
	PlayerBIdentity = InPlayerB;
	ForceNetUpdate();
	NotifyLocalController();
}

const FGuid& AFMCodexNetworkMatchGameState::GetMatchInstanceId() const
{
	return MatchInstanceId;
}

EFMCodexNetworkBootstrapState
AFMCodexNetworkMatchGameState::GetBootstrapState() const
{
	return BootstrapState;
}

const FFMCodexNetworkParticipantPublicIdentity&
AFMCodexNetworkMatchGameState::GetPlayerAIdentity() const
{
	return PlayerAIdentity;
}

const FFMCodexNetworkParticipantPublicIdentity&
AFMCodexNetworkMatchGameState::GetPlayerBIdentity() const
{
	return PlayerBIdentity;
}

void AFMCodexNetworkMatchGameState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFMCodexNetworkMatchGameState, MatchInstanceId);
	DOREPLIFETIME(AFMCodexNetworkMatchGameState, BootstrapState);
	DOREPLIFETIME(AFMCodexNetworkMatchGameState, PlayerAIdentity);
	DOREPLIFETIME(AFMCodexNetworkMatchGameState, PlayerBIdentity);
}

void AFMCodexNetworkMatchGameState::OnRep_PublicBootstrap()
{
	NotifyLocalController();
}

void AFMCodexNetworkMatchGameState::NotifyLocalController() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
		It; ++It)
	{
		AFMCodexNetworkMatchPlayerController* Controller = Cast<
			AFMCodexNetworkMatchPlayerController>(It->Get());
		if (Controller != nullptr && Controller->IsLocalController())
		{
			Controller->RefreshNetworkBootstrapUI();
		}
	}
}
