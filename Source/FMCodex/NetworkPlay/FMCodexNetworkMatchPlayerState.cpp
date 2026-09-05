#include "FMCodexNetworkMatchPlayerState.h"

#include "FMCodexNetworkMatchPlayerController.h"

#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AFMCodexNetworkMatchPlayerState::AFMCodexNetworkMatchPlayerState()
{
	bReplicates = true;
}

void AFMCodexNetworkMatchPlayerState::SetNetworkIdentityOnServer(
	const EInitialTurnOrderPlayer InGameplaySide,
	const FFMCodexNetworkTeamIdentity& InTeam,
	const FString& InPlayerDisplayName)
{
	check(HasAuthority());
	bGameplayParticipant = InGameplaySide != EInitialTurnOrderPlayer::None;
	GameplaySide = InGameplaySide;
	TeamIdentity = InTeam;
	NetworkPlayerDisplayName = InPlayerDisplayName;
	SetPlayerName(InPlayerDisplayName);
	ForceNetUpdate();
	NotifyOwningController();
}

bool AFMCodexNetworkMatchPlayerState::IsGameplayParticipant() const
{
	return bGameplayParticipant;
}

EInitialTurnOrderPlayer
AFMCodexNetworkMatchPlayerState::GetGameplaySide() const
{
	return GameplaySide;
}

const FFMCodexNetworkTeamIdentity&
AFMCodexNetworkMatchPlayerState::GetTeamIdentity() const
{
	return TeamIdentity;
}

const FString&
AFMCodexNetworkMatchPlayerState::GetNetworkPlayerDisplayName() const
{
	return NetworkPlayerDisplayName;
}

void AFMCodexNetworkMatchPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFMCodexNetworkMatchPlayerState, bGameplayParticipant);
	DOREPLIFETIME(AFMCodexNetworkMatchPlayerState, GameplaySide);
	DOREPLIFETIME(AFMCodexNetworkMatchPlayerState, TeamIdentity);
	DOREPLIFETIME(AFMCodexNetworkMatchPlayerState, NetworkPlayerDisplayName);
}

void AFMCodexNetworkMatchPlayerState::OnRep_NetworkIdentity()
{
	NotifyOwningController();
}

void AFMCodexNetworkMatchPlayerState::NotifyOwningController() const
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
		if (Controller != nullptr && Controller->IsLocalController()
			&& Controller->PlayerState == this)
		{
			Controller->RefreshNetworkBootstrapUI();
		}
	}
}
