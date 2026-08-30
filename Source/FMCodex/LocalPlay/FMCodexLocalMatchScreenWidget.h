#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexLocalMatchScreenWidget.generated.h"

class AFMCodexLocalMatchPlayerController;
class UBorder;
class UButton;
class UCanvasPanel;
class UFMCodexCardRackWidget;
class UFMCodexInteractionPanelWidget;
class UFMCodexInlineResolutionFormulaSurfaceWidget;
class UFMCodexLongShotResolutionSurfaceWidget;
class UFMCodexMatchHeaderWidget;
class UFMCodexPitchWidget;
class UFMCodexPlayerCardWidget;
class UFMCodexResolutionPanelWidget;
class UFMCodexRollReelWidget;
class UFMCodexSelectionFeedbackToastWidget;
class UFMCodexTacticalDetailPanelWidget;
class UFMCodexThroughBallResolutionSurfaceWidget;
class UImage;
class USizeBox;
class UTextBlock;
class UTexture2D;
class UUniformGridPanel;
class UWidget;
class UVerticalBox;
class SWidget;

/** Stable presentation identity for one covered Cross D6. */
struct FFMCodexCrossRollRevealIdentity
{
	EFMCodexUMGCrossRollRevealKind Kind =
		EFMCodexUMGCrossRollRevealKind::None;
	int64 AttackSequence = 0;
	FName ContestId = NAME_None;
	int32 RollSequenceIndex = INDEX_NONE;
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;

	bool IsValid() const
	{
		return Kind != EFMCodexUMGCrossRollRevealKind::None
			&& AttackSequence > 0
			&& !ContestId.IsNone()
			&& RollSequenceIndex != INDEX_NONE
			&& OwnerSide != EInitialTurnOrderPlayer::None;
	}

	bool operator==(const FFMCodexCrossRollRevealIdentity& Other) const
	{
		return Kind == Other.Kind
			&& AttackSequence == Other.AttackSequence
			&& ContestId == Other.ContestId
			&& RollSequenceIndex == Other.RollSequenceIndex
			&& OwnerSide == Other.OwnerSide;
	}

	FString StableKey() const
	{
		return FString::Printf(TEXT("%lld|%d|%s|%d|%d"),
			AttackSequence,
			static_cast<int32>(Kind),
			*ContestId.ToString(),
			RollSequenceIndex,
			static_cast<int32>(OwnerSide));
	}
};

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
	UFMCodexThroughBallResolutionSurfaceWidget*
		GetThroughBallResolutionSurface() const;
	UFMCodexLongShotResolutionSurfaceWidget*
		GetLongShotResolutionSurface() const;
	UFMCodexResolutionPanelWidget* GetResolutionPanel() const;
	bool IsLegacyResolutionOverlayVisible() const;
	EFMCodexUMGInlineFormulaRevealPhase GetInlineFormulaRevealPhase() const;
	bool IsInlineFormulaRevealInputBlocked() const;
	UFMCodexRollReelWidget* GetTacticalPointRollReel() const;
#if WITH_DEV_AUTOMATION_TESTS
	void AdvanceInlineFormulaRevealForTesting(float DeltaSeconds);
	void PauseInlineFormulaRevealTimerForTesting();
	void BeginPendingCrossRollRevealForTesting();
	void BeginPendingTacticalPointRevealForTesting();
	void ResetPrimaryActionDispatchForTesting();
	int32 GetPrimaryActionDispatchCountForTesting() const;
	EFMCodexUMGInteractionCategory
		GetLastPrimaryActionDispatchForTesting() const;
#endif
	UFMCodexSelectionFeedbackToastWidget* GetSelectionFeedbackToast() const;
	UFMCodexTacticalDetailPanelWidget* GetTacticalDetailPanel() const;
	bool IsDeploymentTacticalReferenceOpen() const;
	ESkillRuleType GetDeploymentTacticalReferenceSkillType() const;
	void OpenDeploymentTacticalReference();
	void SelectDeploymentTacticalReference(ESkillRuleType SkillType);
	void CloseDeploymentTacticalReference();
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
	void UpdateInlineFormulaRevealState(
		const FFMCodexUMGMatchScreenViewModel& InPresentation);
	FFMCodexUMGInlineFormulaSurfaceViewModel
		BuildDisplayedInlineFormula() const;
	FFMCodexUMGThroughBallResolutionViewModel
		BuildDisplayedThroughBallResolution() const;
	FFMCodexUMGLongShotResolutionViewModel
		BuildDisplayedLongShotResolution() const;
	FFMCodexUMGMatchHeaderViewModel BuildDisplayedHeader() const;
	bool CanRevealTacticalPointDependentPresentation() const;
	FFMCodexUMGCardRackViewModel BuildDisplayedHandRack(
		const FFMCodexUMGCardRackViewModel& Source) const;
	FFMCodexUMGRollReelViewModel BuildActiveRollReelPresentation() const;
	void AdvanceInlineFormulaReveal(float DeltaSeconds, bool bForceFullRefresh);
	void BeginInlineFormulaFinalCapture();
	void RefreshActiveRollReelVisuals();
	void HandleInlineFormulaRevealTimer();
	void StartInlineFormulaRevealTimer();
	void StopInlineFormulaRevealTimer();
	void ResetInlineFormulaRevealState();
	FFMCodexCrossRollRevealIdentity PendingCrossRollIdentity(
		const FFMCodexUMGMatchScreenViewModel& InPresentation) const;
	void ObservePendingCrossRoll(
		const FFMCodexUMGMatchScreenViewModel& InPresentation);
	void BeginInlineFormulaReveal(
		const FFMCodexCrossRollRevealIdentity& Identity,
		bool bRequestInFlight);
	void CancelInlineFormulaReveal();
	bool TryReadAuthoritativeRawRoll(
		const FFMCodexUMGMatchScreenViewModel& InPresentation,
		const FFMCodexCrossRollRevealIdentity& Identity,
		int32& OutRawValue,
		int32& OutDomainMinimum,
		int32& OutDomainMaximum) const;
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
	void ScheduleTacticalDetailDismiss();
	void CancelTacticalDetailDismiss();
	void CompleteTacticalDetailDismiss();
	void HandleTacticalDetailPointerEntered();
	void HandleTacticalDetailPointerLeft();
	void HideTacticalDetail();

	UFUNCTION()
	void HandleDeploymentTacticalReferenceRequested();

	UFUNCTION()
	void HandleDeploymentReferenceLongShotClicked();

	UFUNCTION()
	void HandleDeploymentReferenceCutInsideClicked();

	UFUNCTION()
	void HandleDeploymentReferencePassControlClicked();

	UFUNCTION()
	void HandleDeploymentReferenceCrossClicked();

	UFUNCTION()
	void HandleDeploymentReferenceThroughBallClicked();

	UFUNCTION()
	void HandleDeploymentReferenceCloseClicked();

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
	void HandleTacticalDetailRequested(FName SkillId);

	UFUNCTION()
	void HandleTacticalDetailDismissed(FName SkillId);

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
	void HandleInlineFormulaContinueRequested();

	UFUNCTION()
	void HandleThroughBallContinueRequested();

	UFUNCTION()
	void HandleLongShotContinueRequested();

	UFUNCTION()
	void HandleLongShotBranchRequested(EFMCodexUMGBranchIntent Intent);

	bool DoesInlineFormulaOwnCurrentPrimaryAction() const;
	bool DoesThroughBallOwnCurrentPrimaryAction() const;
	bool DoesLongShotOwnCurrentPrimaryAction() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGMatchScreenViewModel Presentation;

	/** Last fully resolved authoritative surface; display staging only copies it. */
	FFMCodexUMGInlineFormulaSurfaceViewModel CachedResolvedInlineFormula;
	/** Last disclosed authority surface used to ignore stale pending rebuilds. */
	FFMCodexUMGInlineFormulaSurfaceViewModel LastDisclosedInlineFormula;
	/** Exact authority-built ThroughBall shell retained through an outer reel. */
	FFMCodexUMGThroughBallResolutionViewModel CachedResolvedThroughBall;
	/** Exact authority-built LongShot/CutInside shell retained through a reel. */
	FFMCodexUMGLongShotResolutionViewModel CachedResolvedLongShot;
	/** Header before a live Tactical Point roll, retained until disclosure. */
	FFMCodexUMGMatchHeaderViewModel CachedPreRollHeader;

	EFMCodexUMGInlineFormulaRevealPhase InlineFormulaRevealPhase =
		EFMCodexUMGInlineFormulaRevealPhase::None;
	float InlineFormulaRevealPhaseElapsed = 0.0f;
	float RollRevealCaptureStartPositionCells = 0.0f;
	float RollRevealCaptureDistanceCells = 0.0f;
	int32 RollRevealSequenceOffsetCells = 0;
	FTimerHandle InlineFormulaRevealTimerHandle;
	FTimerHandle TacticalDetailDismissTimerHandle;
	FFMCodexCrossRollRevealIdentity ObservedPendingCrossRoll;
	FFMCodexCrossRollRevealIdentity ActiveCrossRollReveal;
	TSet<FString> SettledCrossRollRevealKeys;
	bool bInlineFormulaAuthorityResultAvailable = false;
	int32 RollRevealAuthoritativeRawValue = 0;
	int32 RollRevealDomainMinimum = 1;
	int32 RollRevealDomainMaximum = 6;
	int32 CachedTacticalPointFinalValue = 0;
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

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexTacticalDetailPanelWidget> TacticalDetailPanel;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> TacticalDetailSurface;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DeploymentTacticalReferenceControls;

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
	TSubclassOf<UFMCodexThroughBallResolutionSurfaceWidget>
		ThroughBallResolutionSurfaceWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexThroughBallResolutionSurfaceWidget>
		ThroughBallResolutionSurface;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Resolution Presentation")
	TSubclassOf<UFMCodexLongShotResolutionSurfaceWidget>
		LongShotResolutionSurfaceWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexLongShotResolutionSurfaceWidget>
		LongShotResolutionSurface;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> TacticalPointRevealSurface;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexRollReelWidget> TacticalPointRollReel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TacticalPointRevealTitle;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TacticalPointRevealResult;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Resolution Presentation")
	TSubclassOf<UFMCodexResolutionPanelWidget> ResolutionPanelWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexResolutionPanelWidget> ResolutionPanel;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ResolutionOverlay;

	UPROPERTY(Transient)
	TObjectPtr<USizeBox> ResolutionBounds;

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
	bool bDeploymentTacticalReferenceOpen = false;
	ESkillRuleType DeploymentTacticalReferenceSkillType = ESkillRuleType::None;
	FName ActiveTacticalDetailSkillId = NAME_None;
	bool bTacticalCardHoverOrFocus = false;
	bool bTacticalDetailPointerInside = false;

#if WITH_DEV_AUTOMATION_TESTS
	int32 PrimaryActionDispatchCountForTesting = 0;
	EFMCodexUMGInteractionCategory LastPrimaryActionDispatchForTesting =
		EFMCodexUMGInteractionCategory::None;
#endif
};
