#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexNetworkMatchTypes.h"
#include "FMCodexNetworkPlayerIntent.h"

#include "FMCodexNetworkMatchPlayerController.generated.h"

class STextBlock;
class SWidget;
class SVerticalBox;

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
	UFUNCTION(Exec)
	void DevDeployGoalkeeper();
	UFUNCTION(Exec)
	void DevFinishDeployment();
	UFUNCTION(Exec)
	void DevProbeInvalidGoalkeeperSlot();
	bool CanDeployGoalkeeper() const;
	bool CanFinishDeployment() const;
	UFUNCTION(Exec)
	void DevSubmitCarrier(FName CarrierCardId);
	UFUNCTION(Exec)
	void DevProbeInvalidCarrier();
	bool CanSubmitCarrier() const;
	UFUNCTION(Exec)
	void DevSubmitMarker(FName MarkerCardId);
	UFUNCTION(Exec)
	void DevProbeInvalidMarker();
	UFUNCTION(Exec)
	void DevSubmitRunner(FName RunnerCardId);
	UFUNCTION(Exec)
	void DevProbeInvalidRunner();
	UFUNCTION(Exec)
	void DevSubmitHelper(FName HelperCardId);
	UFUNCTION(Exec)
	void DevProbeInvalidHelper();
	UFUNCTION(Exec)
	void DevSubmitSkill(FName SkillId);
	UFUNCTION(Exec)
	void DevProbeInvalidSkill();
	bool CanSubmitMarker() const;
	bool CanSubmitRunner() const;
	bool CanSubmitHelper() const;
	bool CanSubmitSkill() const;
	UFUNCTION(Exec)
	void DevSubmitBranchIntent(int32 Intent);
	UFUNCTION(Exec)
	void DevProbeInvalidBranch();
	bool CanSubmitBranch() const;
	bool CanRequestInitialRoute() const;
	UFUNCTION(Exec)
	void DevRequestInitialRoute();
	UFUNCTION(Exec)
	void DevProbeWrongRouteFamily();
	void SubmitInitialRoute(EFMCodexNetworkPlayerIntentKind Kind);

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
	friend struct FFMCodexNetworkDeploymentCompletionTestAccess;
#endif

	UFUNCTION(Server, Reliable)
	void ServerSubmitPlayerIntent(const FFMCodexNetworkPlayerIntentEnvelope& Envelope);
	UFUNCTION(Client, Reliable)
	void ClientReceivePlayerIntentAck(const FFMCodexNetworkPlayerIntentAck& Ack);
	UFUNCTION()
	void OnRep_OwnerView();

	FFMCodexNetworkIntentClientState IntentClientState;

	void SubmitMarkerChoice(const FFMCodexNetworkSubmitMarkerPayload& Choice);
	void SubmitRunnerChoice(const FFMCodexNetworkSubmitRunnerPayload& Choice);
	void SubmitHelperChoice(const FFMCodexNetworkSubmitHelperPayload& Choice);
	void SubmitBranchChoice(const FFMCodexNetworkSubmitBranchIntentPayload& Choice);
	void SubmitSkillChoice(const FFMCodexNetworkSubmitSkillPayload& Choice);
	void SubmitCarrierChoice(const FFMCodexNetworkSubmitCarrierPayload& Choice);
	void SubmitDeploymentChoice(const FFMCodexNetworkDeployOrdinaryPayload& Choice);
	void SubmitDeploymentCompletion(EFMCodexNetworkPlayerIntentKind Kind);
	void InitializeDeveloperStatusUI();
	FText BuildStatusText() const;

	UPROPERTY(ReplicatedUsing = OnRep_OwnerView)
	FFMCodexNetworkClientViewSnapshot OwnerView;

#if !UE_BUILD_SHIPPING
	TSharedPtr<SWidget> StatusViewportWidget;
	TSharedPtr<STextBlock> StatusText;
	TSharedPtr<SVerticalBox> ParticipantChoices;
#endif
};
