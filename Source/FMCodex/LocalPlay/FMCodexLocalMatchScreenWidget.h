#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexLocalMatchScreenWidget.generated.h"

class AFMCodexLocalMatchPlayerController;
class UBorder;
class UFMCodexInteractionPanelWidget;
class UFMCodexPitchWidget;
class UFMCodexPlayerCardWidget;
class UTextBlock;
class UVerticalBox;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexLocalMatchScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexLocalMatchScreenWidget(
		const FObjectInitializer& ObjectInitializer);

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
	void RequestDeployGoalkeeper(FName SlotId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestFinishDeployment();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitCarrier(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitMarker(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitSkill(FName SkillId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitRunner(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitHelper(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestDeclineSelection();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestResolveNoLegalSelection();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitBranchIntent(EFMCodexUMGBranchIntent Intent);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestSubmitOneOnOneChoice(EFMCodexUMGOneOnOneChoice Choice);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestContinueResolution();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Intent")
	void RequestReady();

	const FFMCodexUMGMatchScreenViewModel& GetPresentation() const;
	AFMCodexLocalMatchPlayerController* GetMatchController() const;
	UFMCodexPitchWidget* GetPitchWidget() const;
	UFMCodexInteractionPanelWidget* GetInteractionPanel() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetRenderedCandidateCardWidgets() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UFUNCTION()
	void HandleStartNewMatchClicked();

	UFUNCTION()
	void HandleBeginOrdinaryAttackClicked();

	UFUNCTION()
	void HandleDeployOrdinaryRequested(FName CardId, FName SlotId);

	UFUNCTION()
	void HandleDeployGoalkeeperRequested(FName SlotId);

	UFUNCTION()
	void HandleFinishDeploymentClicked();

	UFUNCTION()
	void HandleCarrierRequested(FName CardId);

	UFUNCTION()
	void HandleMarkerRequested(FName CardId);

	UFUNCTION()
	void HandleSkillRequested(FName SkillId);

	UFUNCTION()
	void HandleRunnerRequested(FName CardId);

	UFUNCTION()
	void HandleHelperRequested(FName CardId);

	UFUNCTION()
	void HandleDeclineRequested();

	UFUNCTION()
	void HandleNoLegalRequested();

	UFUNCTION()
	void HandleBranchRequested(EFMCodexUMGBranchIntent Intent);

	UFUNCTION()
	void HandleOneOnOneRequested(EFMCodexUMGOneOnOneChoice Choice);

	UFUNCTION()
	void HandleContinueRequested();

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

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Pitch Presentation")
	TSubclassOf<UFMCodexPitchWidget> PitchWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPitchWidget> PitchWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Interaction Presentation")
	TSubclassOf<UFMCodexInteractionPanelWidget> InteractionPanelWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexInteractionPanelWidget> InteractionPanel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ResolutionText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HandoffOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HandoffText;
};
