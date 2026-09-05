#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"

#include "FMCodexNetworkMatchTypes.h"

#include "FMCodexNetworkMatchPlayerState.generated.h"

UCLASS()
class FMCODEX_API AFMCodexNetworkMatchPlayerState final : public APlayerState
{
	GENERATED_BODY()

public:
	AFMCodexNetworkMatchPlayerState();

	void SetNetworkIdentityOnServer(
		EInitialTurnOrderPlayer InGameplaySide,
		const FFMCodexNetworkTeamIdentity& InTeam,
		const FString& InPlayerDisplayName);

	bool IsGameplayParticipant() const;
	EInitialTurnOrderPlayer GetGameplaySide() const;
	const FFMCodexNetworkTeamIdentity& GetTeamIdentity() const;
	const FString& GetNetworkPlayerDisplayName() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_NetworkIdentity();

	void NotifyOwningController() const;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkIdentity)
	bool bGameplayParticipant = false;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkIdentity)
	EInitialTurnOrderPlayer GameplaySide = EInitialTurnOrderPlayer::None;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkIdentity)
	FFMCodexNetworkTeamIdentity TeamIdentity;

	UPROPERTY(ReplicatedUsing = OnRep_NetworkIdentity)
	FString NetworkPlayerDisplayName;
};
