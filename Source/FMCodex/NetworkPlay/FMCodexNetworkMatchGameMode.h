#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FMCodexNetworkMatchRuntime.h"
#include "FMCodexNetworkPlayerIntent.h"
#include "FMCodexNetworkParticipantRegistry.h"

#include "FMCodexNetworkMatchGameMode.generated.h"

UCLASS()
class FMCODEX_API AFMCodexNetworkMatchGameMode final : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFMCodexNetworkMatchGameMode();
	virtual ~AFMCodexNetworkMatchGameMode() override;

	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	EInitialTurnOrderPlayer ResolveSideForController(
		const AController* Controller) const;
	const FGuid& GetMatchInstanceId() const;
	bool IsNetworkMatchInitialized() const;
	FFMCodexNetworkPlayerIntentAck SubmitConnectionPlayerIntent(
		AFMCodexNetworkMatchPlayerController* Controller,
		const FFMCodexNetworkPlayerIntentEnvelope& Envelope);

protected:
	virtual void BeginPlay() override;

private:
#if WITH_DEV_AUTOMATION_TESTS
	friend class FFMCodexNetworkPlayerDisplayNameFallbackTest;
	friend struct FFMCodexNetworkIntentTestAccess;
	friend struct FFMCodexNetworkDeploymentTestAccess;
	friend struct FFMCodexNetworkDeploymentCompletionTestAccess;
	friend struct FFMCodexNetworkCarrierTestAccess;
	friend struct FFMCodexNetworkMarkerTestAccess;
	friend struct FFMCodexNetworkRunnerTestAccess;
	friend struct FFMCodexNetworkHelperTestAccess;
	friend struct FFMCodexNetworkSkillTestAccess;
	friend struct FFMCodexNetworkBranchTestAccess;
	friend struct FFMCodexNetworkInitialRouteTestAccess;
#endif

	void EnsureMatchInstanceId();
	void EnsureBootstrapConfiguration();
	void TryInitializeNetworkMatch();
	void PublishParticipantState(
		EFMCodexNetworkBootstrapState NewState);
	void PublishOwnerViews(EFMCodexNetworkBootstrapState State);
	FFMCodexNetworkParticipantPublicIdentity BuildPublicIdentity(
		EInitialTurnOrderPlayer Side) const;
	static FString SelectPlayerDisplayName(
		const FString& Candidate,
		EInitialTurnOrderPlayer Side);

	FFMCodexNetworkParticipantRegistry ParticipantRegistry;
	FFMCodexNetworkBootstrapConfiguration BootstrapConfiguration;
	TUniquePtr<FFMCodexNetworkMatchRuntime> MatchRuntime;
	FGuid MatchInstanceId;
	FFMCodexNetworkParticipantPublicIdentity PlayerAIdentity;
	FFMCodexNetworkParticipantPublicIdentity PlayerBIdentity;
	int32 ViewRevision = 0;
	bool bBootstrapAttempted = false;
	bool bTransportFault = false;
	TMap<TWeakObjectPtr<AFMCodexNetworkMatchPlayerController>, FFMCodexNetworkIntentLedger> IntentLedgers;
};
