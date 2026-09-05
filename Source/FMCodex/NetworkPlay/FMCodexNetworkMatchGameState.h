#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "FMCodexNetworkMatchTypes.h"

#include "FMCodexNetworkMatchGameState.generated.h"

UCLASS()
class FMCODEX_API AFMCodexNetworkMatchGameState final : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFMCodexNetworkMatchGameState();

	void SetBootstrapStateOnServer(
		const FGuid& InMatchInstanceId,
		EFMCodexNetworkBootstrapState InBootstrapState,
		const FFMCodexNetworkParticipantPublicIdentity& InPlayerA,
		const FFMCodexNetworkParticipantPublicIdentity& InPlayerB);

	const FGuid& GetMatchInstanceId() const;
	EFMCodexNetworkBootstrapState GetBootstrapState() const;
	const FFMCodexNetworkParticipantPublicIdentity& GetPlayerAIdentity() const;
	const FFMCodexNetworkParticipantPublicIdentity& GetPlayerBIdentity() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_PublicBootstrap();

	void NotifyLocalController() const;

	UPROPERTY(ReplicatedUsing = OnRep_PublicBootstrap)
	FGuid MatchInstanceId;

	UPROPERTY(ReplicatedUsing = OnRep_PublicBootstrap)
	EFMCodexNetworkBootstrapState BootstrapState =
		EFMCodexNetworkBootstrapState::WaitingForPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_PublicBootstrap)
	FFMCodexNetworkParticipantPublicIdentity PlayerAIdentity;

	UPROPERTY(ReplicatedUsing = OnRep_PublicBootstrap)
	FFMCodexNetworkParticipantPublicIdentity PlayerBIdentity;
};
