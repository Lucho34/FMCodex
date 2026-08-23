#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexLocalMatchScreenWidget.generated.h"

class AFMCodexLocalMatchPlayerController;
class UBorder;
class UCanvasPanel;
class UFMCodexCardRackWidget;
class UFMCodexInteractionPanelWidget;
class UFMCodexInlineResolutionFormulaSurfaceWidget;
class UFMCodexMatchHeaderWidget;
class UFMCodexPitchWidget;
class UFMCodexPlayerCardWidget;
class UFMCodexResolutionPanelWidget;
class UFMCodexSelectionFeedbackToastWidget;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UWidget;
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
	void RequestRollTacticalPoints();

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
	const FFMCodexUMGMatchScreenViewModel& GetPresentation() const;
	AFMCodexLocalMatchPlayerController* GetMatchController() const;
	UFMCodexMatchHeaderWidget* GetMatchHeader() const;
	UFMCodexPitchWidget* GetPitchWidget() const;
	UFMCodexInteractionPanelWidget* GetInteractionPanel() const;
	UFMCodexInlineResolutionFormulaSurfaceWidget*
		GetInlineFormulaSurface() const;
	UFMCodexResolutionPanelWidget* GetResolutionPanel() const;
	bool IsLegacyResolutionOverlayVisible() const;
	UFMCodexSelectionFeedbackToastWidget* GetSelectionFeedbackToast() const;
	UFMCodexCardRackWidget* GetLocalRackWidget() const;
	UFMCodexCardRackWidget* GetOpponentRackWidget() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetRenderedCandidateCardWidgets() const;
	UFMCodexPlayerCardWidget* GetDetailOverlayCard() const;
	bool IsDetailOverlayVisible() const;
	bool IsDetailOverlayHitTestInvisible() const;
	FVector2D GetDetailOverlayPosition() const;
	EFMCodexUMGCardInteractionState GetInteractionState() const;
	EFMCodexUMGCardInteractionState GetLastCompletedDragState() const;
	int32 GetFullCardProductionReviewCardCount() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetFullCardProductionReviewCards() const;
	bool IsFullCardProductionReviewVisible() const;
	static FVector2D GetCanonicalFullCardDimensions();
	static FVector2D CalculateDetailOverlayPosition(
		const FVector2D& SourcePosition,
		const FVector2D& SourceSize,
		const FVector2D& ViewportSize,
		bool bOpenTowardRight);

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void HandleDeploymentDragStarted(FName CardId, bool bGoalkeeper);
	void HandleDeploymentDragFinished();
	void HandlePitchDeploymentDropped(
		FName CardId, FName SlotId, bool bGoalkeeper);
	void BindDetailHoverSources();
	void HandleDetailHoverRequested(UFMCodexPlayerCardWidget* SourceCard);
	void HandleDetailHoverDismissed(UFMCodexPlayerCardWidget* SourceCard);
	void ShowDetailOverlay(UFMCodexPlayerCardWidget* SourceCard);
	void HideDetailOverlay();
	void PositionDetailOverlay(UFMCodexPlayerCardWidget* SourceCard);
	void RefreshFullCardProductionReviewSurface();

	UFUNCTION()
	void HandleStartNewMatchClicked();

	UFUNCTION()
	void HandleTacticalPointRollClicked();

	UFUNCTION()
	void HandleDeployOrdinaryRequested(FName CardId, FName SlotId);

	UFUNCTION()
	void HandleDeployGoalkeeperRequested(FName SlotId);

	UFUNCTION()
	void HandleFinishDeploymentClicked();

	UFUNCTION()
	void HandleCarrierRequested(FName CardId);
	void HandleOnPitchSelectionRequested(
		EFMCodexUMGOnPitchSelectionIntent Intent, FName OptionId);
	void HandleSelectionFeedbackRequested(FName CardId);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGMatchScreenViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<AFMCodexLocalMatchPlayerController> MatchController;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> MainScreen;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> DetailOverlayCanvas;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPlayerCardWidget> DetailOverlayCard;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPlayerCardWidget> DetailHoverSource;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> LocalRackBounds;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> PitchBounds;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> OpponentRackBounds;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexCardRackWidget> LocalRackWidget;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexCardRackWidget> OpponentRackWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Header Presentation")
	TSubclassOf<UFMCodexMatchHeaderWidget> MatchHeaderWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexMatchHeaderWidget> MatchHeader;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Pitch Presentation")
	TSubclassOf<UFMCodexPitchWidget> PitchWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPitchWidget> PitchWidget;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexSelectionFeedbackToastWidget> SelectionFeedbackToast;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Interaction Presentation")
	TSubclassOf<UFMCodexInteractionPanelWidget> InteractionPanelWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexInteractionPanelWidget> InteractionPanel;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Resolution Presentation")
	TSubclassOf<UFMCodexInlineResolutionFormulaSurfaceWidget>
		InlineFormulaSurfaceWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexInlineResolutionFormulaSurfaceWidget>
		InlineFormulaSurface;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Resolution Presentation")
	TSubclassOf<UFMCodexResolutionPanelWidget> ResolutionPanelWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexResolutionPanelWidget> ResolutionPanel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ResolutionOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> HandMicroProductionReviewSurface;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> HandMicroProductionReviewBounds;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> HandMicroProductionReviewPortraitsPage;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> HandMicroProductionReviewTypographyPage;

	UPROPERTY(Transient)
	TObjectPtr<UWidget> HandMicroProductionReviewLayoutPage;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> FullCardProductionReviewBounds;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> FullCardProductionReviewGrid;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexPlayerCardWidget>>
		FullCardProductionReviewCards;

	FVector2D DetailOverlayPosition = FVector2D::ZeroVector;
	EFMCodexUMGCardInteractionState InteractionState =
		EFMCodexUMGCardInteractionState::Default;
	EFMCodexUMGCardInteractionState LastCompletedDragState =
		EFMCodexUMGCardInteractionState::Default;
	bool bDeploymentDragActive = false;
	bool bDeploymentDropSubmitted = false;
};
