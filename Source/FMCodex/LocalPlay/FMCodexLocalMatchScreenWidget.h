#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexLocalMatchScreenWidget.generated.h"

class AFMCodexLocalMatchPlayerController;
class UBorder;
class UButton;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexLocalMatchScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMatchController(
		AFMCodexLocalMatchPlayerController* InController);
	void ClearMatchController();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGMatchScreenViewModel& InPresentation);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestStartNewMatch();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestBeginOrdinaryAttack();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestDeployOrdinary(FName CardId, FName SlotId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestFinishDeployment();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestReady();

	const FFMCodexUMGMatchScreenViewModel& GetPresentation() const;
	AFMCodexLocalMatchPlayerController* GetMatchController() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshPitch();
	void RefreshInteraction();

	UFUNCTION()
	void HandleStartNewMatchClicked();

	UFUNCTION()
	void HandleBeginOrdinaryAttackClicked();

	UFUNCTION()
	void HandleFinishDeploymentClicked();

	UFUNCTION()
	void HandleReadyClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGMatchScreenViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<AFMCodexLocalMatchPlayerController> MatchController;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> MainScreen;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> PitchBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> InteractionTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> InteractionActionsText;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> CandidateCardsBody;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartNewMatchButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> BeginOrdinaryAttackButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> FinishDeploymentButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResolutionText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HandoffOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HandoffText;
};
