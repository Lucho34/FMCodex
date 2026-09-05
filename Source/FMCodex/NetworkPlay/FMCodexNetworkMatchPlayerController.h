#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexNetworkMatchTypes.h"
#include "FMCodexNetworkPlayerIntent.h"

#include "FMCodexNetworkMatchPlayerController.generated.h"

class STextBlock;
class SWidget;

UCLASS()
class FMCODEX_API AFMCodexNetworkMatchPlayerController final
	: public APlayerController
{
	GENERATED_BODY()

public:
	AFMCodexNetworkMatchPlayerController();

	void SetOwnerViewOnServer(
		const FFMCodexNetworkClientViewSnapshot& InOwnerView);
	const FFMCodexNetworkClientViewSnapshot& GetOwnerView() const;
	void RefreshNetworkBootstrapUI();
	/** Same owner command used by the DEV button and runtime validation. Inert in Shipping. */
	UFUNCTION(Exec)
	void DevRequestInitialActionPointRoll();
	bool CanRequestInitialActionPointRoll() const;
	/** Fresh-run negative wire probe; no UI consumer, inert outside automation builds. */
	UFUNCTION(Exec)
	void DevProbeWrongSideInitialD12();
	UFUNCTION(Exec)
	void DevDeployOrdinary();
	UFUNCTION(Exec)
	void DevProbeInvalidDeploymentCard();
	bool CanDeployOrdinary() const;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;

private:
#if WITH_DEV_AUTOMATION_TESTS
	friend struct FFMCodexNetworkBootstrapUIRefreshTestAccess;
	friend struct FFMCodexNetworkIntentTestAccess;
	friend struct FFMCodexNetworkDeploymentTestAccess;
#endif

	UFUNCTION(Server, Reliable)
	void ServerSubmitPlayerIntent(const FFMCodexNetworkPlayerIntentEnvelope& Envelope);
	UFUNCTION(Client, Reliable)
	void ClientReceivePlayerIntentAck(const FFMCodexNetworkPlayerIntentAck& Ack);
	UFUNCTION()
	void OnRep_OwnerView();

	FFMCodexNetworkIntentClientState IntentClientState;

	void SubmitDeploymentChoice(const FFMCodexNetworkDeployOrdinaryPayload& Choice);
	void InitializeDeveloperStatusUI();
	FText BuildStatusText() const;

	UPROPERTY(ReplicatedUsing = OnRep_OwnerView)
	FFMCodexNetworkClientViewSnapshot OwnerView;

#if !UE_BUILD_SHIPPING
	TSharedPtr<SWidget> StatusViewportWidget;
	TSharedPtr<STextBlock> StatusText;
#endif
};
