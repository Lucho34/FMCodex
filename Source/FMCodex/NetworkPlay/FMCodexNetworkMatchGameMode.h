#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FMCodexNetworkMatchRuntime.h"
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

protected:
	virtual void BeginPlay() override;

private:
#if WITH_DEV_AUTOMATION_TESTS
	friend class FFMCodexNetworkPlayerDisplayNameFallbackTest;
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
	static int32 GenerateServerSeed(const FGuid& MatchId);

	FFMCodexNetworkParticipantRegistry ParticipantRegistry;
	FFMCodexNetworkBootstrapConfiguration BootstrapConfiguration;
	TUniquePtr<FFMCodexNetworkMatchRuntime> MatchRuntime;
	FGuid MatchInstanceId;
	FFMCodexNetworkParticipantPublicIdentity PlayerAIdentity;
	FFMCodexNetworkParticipantPublicIdentity PlayerBIdentity;
	int32 ViewRevision = 0;
	bool bBootstrapAttempted = false;
};
