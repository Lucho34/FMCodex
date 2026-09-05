#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "FMCodexNetworkMatchTypes.h"

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

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRep_PlayerState() override;

private:
#if WITH_DEV_AUTOMATION_TESTS
	friend struct FFMCodexNetworkBootstrapUIRefreshTestAccess;
#endif

	UFUNCTION()
	void OnRep_OwnerView();

	void InitializeDeveloperStatusUI();
	FText BuildStatusText() const;

	UPROPERTY(ReplicatedUsing = OnRep_OwnerView)
	FFMCodexNetworkClientViewSnapshot OwnerView;

#if !UE_BUILD_SHIPPING
	TSharedPtr<SWidget> StatusViewportWidget;
	TSharedPtr<STextBlock> StatusText;
#endif
};
