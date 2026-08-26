#include "FMCodexLocalMatchScreenWidget.h"

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexFullCardDiagnostics.h"
#include "FMCodexHandMicroDiagnostics.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexResolutionPanelWidget.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexSelectionFeedbackToastWidget.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace FMCodexLocalMatchScreenWidget
{
	// Stage 6.13.1.4.8C.5 presentation tuning. These values never drive authority.
	constexpr float RevealHoldTickInterval = 0.04f;
	constexpr float FastPhaseDuration = 0.45f;
	constexpr float MainDecelerationEndTime = 1.05f;
	constexpr float FinalSlowEndTime = 1.30f;
	constexpr float CaptureSettleDuration = 0.16f;
	constexpr float CaptureOvershootPixels = 3.0f;
	constexpr float CaptureScale = 1.08f;
	constexpr float CaptureTargetEntryAlpha = 0.45f;
	constexpr float NeighborFadeStartAlpha = CaptureTargetEntryAlpha;
	constexpr float FormulaDisclosureDelay = 0.18f;
	constexpr float NarrativeDisclosureDelay = 0.38f;
	constexpr float FormulaReadableResultHoldDuration = 2.40f;
	constexpr float TacticalPointReadableResultHoldDuration = 2.40f;
	constexpr float FormulaResultHoldDuration =
		FormulaDisclosureDelay + FormulaReadableResultHoldDuration;
	constexpr float TacticalPointResultHoldDuration =
		FormulaDisclosureDelay + TacticalPointReadableResultHoldDuration;
	constexpr float RouteResultHoldDuration = 1.45f;
	constexpr float FastVelocityCellsPerSecond = 12.5f;
	constexpr float MainDecelerationEndVelocityCellsPerSecond = 4.5f;
	constexpr float FinalSlowVelocityCellsPerSecond = 2.0f;

	bool IsPerFrameMotionPhase(
		const EFMCodexUMGInlineFormulaRevealPhase Phase)
	{
		return Phase == EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight
			|| Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling
			|| Phase == EFMCodexUMGInlineFormulaRevealPhase::Settling;
	}

	const FFMCodexUMGInlineFormulaTermViewModel* FindRawRoll(
		const FFMCodexUMGInlineFormulaRowViewModel& Row)
	{
		return Row.Terms.FindByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll;
			});
	}

	FFMCodexUMGInlineFormulaTermViewModel* FindRawRoll(
		FFMCodexUMGInlineFormulaRowViewModel& Row)
	{
		return Row.Terms.FindByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll;
			});
	}

	const FFMCodexUMGInlineFormulaSurfaceViewModel& ActiveFormula(
		const FFMCodexUMGMatchScreenViewModel& Presentation)
	{
		return Presentation.ThroughBallResolution.Formula.bVisible
			? Presentation.ThroughBallResolution.Formula
			: Presentation.InlineFormula;
	}

	float CosmeticReelPosition(const float Elapsed)
	{
		const float SafeElapsed = FMath::Max(0.0f, Elapsed);
		if (SafeElapsed <= FastPhaseDuration)
		{
			return SafeElapsed * FastVelocityCellsPerSecond;
		}

		const float FastDistance =
			FastPhaseDuration * FastVelocityCellsPerSecond;
		const float MainDuration =
			MainDecelerationEndTime - FastPhaseDuration;
		const float MainElapsed = FMath::Min(
			SafeElapsed - FastPhaseDuration, MainDuration);
		const float MainAlpha = MainDuration > 0.0f
			? MainElapsed / MainDuration : 1.0f;
		// Velocity uses a squared tail: continuous at both boundaries, with a
		// stronger early slowdown and a visibly longer slow tail than C.2.
		const float MainTailIntegral =
			(1.0f - FMath::Pow(1.0f - MainAlpha, 3.0f)) / 3.0f;
		float Position = FastDistance
			+ MainDecelerationEndVelocityCellsPerSecond * MainElapsed
			+ (FastVelocityCellsPerSecond
				- MainDecelerationEndVelocityCellsPerSecond)
				* MainDuration * MainTailIntegral;
		if (SafeElapsed <= MainDecelerationEndTime)
		{
			return Position;
		}

		const float FinalSlowDuration =
			FinalSlowEndTime - MainDecelerationEndTime;
		const float FinalSlowElapsed = FMath::Min(
			SafeElapsed - MainDecelerationEndTime, FinalSlowDuration);
		const float FinalSlowAlpha = FinalSlowDuration > 0.0f
			? FinalSlowElapsed / FinalSlowDuration : 1.0f;
		const float FinalSlowTailIntegral =
			(1.0f - FMath::Pow(1.0f - FinalSlowAlpha, 3.0f)) / 3.0f;
		Position += FinalSlowVelocityCellsPerSecond * FinalSlowElapsed
			+ (MainDecelerationEndVelocityCellsPerSecond
				- FinalSlowVelocityCellsPerSecond)
				* FinalSlowDuration * FinalSlowTailIntegral;
		if (SafeElapsed > FinalSlowEndTime)
		{
			// Network-safe awaiting motion stays slow and continuous.
			Position += (SafeElapsed - FinalSlowEndTime)
				* FinalSlowVelocityCellsPerSecond;
		}
		return Position;
	}

	int32 WrapDomainValue(
		const int32 Value,
		const int32 Minimum,
		const int32 Maximum)
	{
		const int32 Count = FMath::Max(1, Maximum - Minimum + 1);
		return Minimum + ((Value - Minimum) % Count + Count) % Count;
	}

	int32 CosmeticCycleStep(const float Elapsed)
	{
		return FMath::FloorToInt(CosmeticReelPosition(Elapsed));
	}

	int32 PlannedSequenceOffset(
		const int32 AuthoritativeValue,
		const int32 DomainMinimum,
		const int32 DomainMaximum)
	{
		const int32 DomainCount = FMath::Max(
			1, DomainMaximum - DomainMinimum + 1);
		const int32 TargetIndex = WrapDomainValue(
			AuthoritativeValue, DomainMinimum, DomainMaximum) - DomainMinimum;
		const int32 PlannedTargetStep = FMath::CeilToInt(
			CosmeticReelPosition(FinalSlowEndTime));
		const int32 WrappedPlannedStep =
			((PlannedTargetStep % DomainCount) + DomainCount) % DomainCount;
		return (TargetIndex - WrappedPlannedStep + DomainCount) % DomainCount;
	}

	FFMCodexUMGRollReelViewModel BuildReelPresentation(
		const EFMCodexUMGInlineFormulaRevealPhase Phase,
		const float PhaseElapsed,
		const int32 DomainMinimum,
		const int32 DomainMaximum,
		const int32 AuthoritativeValue,
		const float CaptureStartPositionCells,
		const float CaptureDistanceCells,
		const int32 SequenceOffsetCells)
	{
		FFMCodexUMGRollReelViewModel Result;
		Result.bVisible = true;
		Result.DomainMinimum = DomainMinimum;
		Result.DomainMaximum = DomainMaximum;
		Result.bMoving = IsPerFrameMotionPhase(Phase);
		Result.bResultHold = Phase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
		Result.bShowNeighborDigits = Result.bMoving;
		Result.bStaticResult = Result.bResultHold
			|| Phase == EFMCodexUMGInlineFormulaRevealPhase::Settled;

		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold
			|| Phase == EFMCodexUMGInlineFormulaRevealPhase::Settled)
		{
			Result.CenterValue = AuthoritativeValue;
			Result.PreviousValue = WrapDomainValue(
				AuthoritativeValue - 1, DomainMinimum, DomainMaximum);
			Result.NextValue = WrapDomainValue(
				AuthoritativeValue + 1, DomainMinimum, DomainMaximum);
			Result.bAuthoritativeValue = true;
			Result.NeighborFadeAlpha = 1.0f;
			Result.ContinuousPositionCells =
				CaptureStartPositionCells + CaptureDistanceCells;
			return Result;
		}

		float Position = CosmeticReelPosition(PhaseElapsed);
		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Settling)
		{
			const float Alpha = FMath::Clamp(
				PhaseElapsed / CaptureSettleDuration, 0.0f, 1.0f);
			if (Alpha < CaptureTargetEntryAlpha)
			{
				const float EntryAlpha = Alpha / CaptureTargetEntryAlpha;
				const float SmoothedEntry = EntryAlpha * EntryAlpha
					* (3.0f - 2.0f * EntryAlpha);
				Position = CaptureStartPositionCells
					+ CaptureDistanceCells * SmoothedEntry;
			}
			else
			{
				Position = CaptureStartPositionCells + CaptureDistanceCells;
				const float LandingAlpha = (Alpha - CaptureTargetEntryAlpha)
					/ (1.0f - CaptureTargetEntryAlpha);
				const float LandingPulse = FMath::Square(
					FMath::Sin(LandingAlpha * PI));
				Result.LandingOffsetY =
					-CaptureOvershootPixels * LandingPulse;
				Result.LandingScale = 1.0f
					+ (CaptureScale - 1.0f) * LandingPulse;
			}
			const float NeighborAlpha = FMath::Clamp(
				(Alpha - NeighborFadeStartAlpha)
					/ (1.0f - NeighborFadeStartAlpha),
				0.0f, 1.0f);
			Result.NeighborFadeAlpha = NeighborAlpha * NeighborAlpha
				* (3.0f - 2.0f * NeighborAlpha);
		}
		const int32 Step = FMath::FloorToInt(Position);
		Result.ContinuousPositionCells = Position;
		Result.CenterValue = WrapDomainValue(
			DomainMinimum + Step + SequenceOffsetCells,
			DomainMinimum, DomainMaximum);
		Result.PreviousValue = WrapDomainValue(
			Result.CenterValue - 1, DomainMinimum, DomainMaximum);
		Result.NextValue = WrapDomainValue(
			Result.CenterValue + 1, DomainMinimum, DomainMaximum);
		Result.ScrollAlpha = FMath::Frac(Position);
		return Result;
	}

	void StageRowForReveal(
		FFMCodexUMGInlineFormulaRowViewModel& Row,
		const bool bActive,
		const bool bSettling,
		const int32 VisibleD6)
	{
		if (FFMCodexUMGInlineFormulaTermViewModel* Roll = FindRawRoll(Row))
		{
		Roll->bResolved = bSettling;
		Roll->RawD6 = bSettling ? VisibleD6 : 0;
		Roll->DisplayLabel = bSettling
			? FString::Printf(TEXT("掷点 %d"), VisibleD6)
			: FString(TEXT("掷点 ?"));
			Roll->bNextPendingRoll = bActive;
		}
		Row.bFinalValueResolved = false;
		Row.FinalValue = 0.0f;
		Row.FinalValueLabel = TEXT("?");
		Row.bDisplayedResultResolved = Row.bKnownNonRollSubtotalResolved;
		Row.bDisplayedResultIsFinalValue = false;
		Row.DisplayedResult = Row.KnownNonRollSubtotal;
		Row.DisplayedResultLabel = Row.bKnownNonRollSubtotalResolved
			? FString::SanitizeFloat(Row.KnownNonRollSubtotal)
			: FString(TEXT("?"));
	}

	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Text = FString())
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(Text));
		Result->SetAutoWrapText(true);
		return Result;
	}

	UBorder* MakeRegion(UWidgetTree& Tree, const FName Name)
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		FFMCodexPlayerUIStyle::Get().ApplyBorder(
			*Result, EFMCodexPlayerUIColorRole::PanelBackground,
			FFMCodexPlayerUIStyle::Get().GetOuterPadding());
		return Result;
	}

	UButton* MakeButton(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Label)
	{
		UButton* Result = Tree.ConstructWidget<UButton>(
			UButton::StaticClass(), Name);
		UTextBlock* LabelText = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Label"))), Label);
		LabelText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LabelText, EFMCodexPlayerUITextRole::Status);
		Result->AddChild(LabelText);
		FFMCodexPlayerUIStyle::Get().ApplyButton(
			*Result, EFMCodexPlayerUIActionRole::Primary);
		return Result;
	}
}

UFMCodexLocalMatchScreenWidget::UFMCodexLocalMatchScreenWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MatchHeaderWidgetClass = UFMCodexMatchHeaderWidget::StaticClass();
	PitchWidgetClass = UFMCodexPitchWidget::StaticClass();
	InteractionPanelWidgetClass = UFMCodexInteractionPanelWidget::StaticClass();
	InlineFormulaSurfaceWidgetClass =
		UFMCodexInlineResolutionFormulaSurfaceWidget::StaticClass();
	ThroughBallResolutionSurfaceWidgetClass =
		UFMCodexThroughBallResolutionSurfaceWidget::StaticClass();
	ResolutionPanelWidgetClass = UFMCodexResolutionPanelWidget::StaticClass();
}

void UFMCodexLocalMatchScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexLocalMatchScreenWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexLocalMatchScreenWidget::NativeDestruct()
{
	StopInlineFormulaRevealTimer();
	CancelTacticalDetailDismiss();
	HideTacticalDetail();
	HideDetailOverlay();
	if (SelectionFeedbackToast != nullptr)
	{
		SelectionFeedbackToast->DismissFeedback();
	}
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	bDeploymentDragActive = false;
	bDeploymentDropSubmitted = false;
	InteractionState = EFMCodexUMGCardInteractionState::Default;
	Super::NativeDestruct();
}

void UFMCodexLocalMatchScreenWidget::SetMatchController(
	AFMCodexLocalMatchPlayerController* InController)
{
	MatchController = InController;
}

void UFMCodexLocalMatchScreenWidget::ClearMatchController()
{
	MatchController = nullptr;
}

void UFMCodexLocalMatchScreenWidget::RefreshFromPresentation(
	const FFMCodexUMGMatchScreenViewModel& InPresentation)
{
	HideDetailOverlay();
	// Any authoritative presentation rebuild invalidates the transient hover
	// source. The newly rendered tactical cards can publish a fresh identity.
	HideTacticalDetail();
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	const bool bLeavingFeedbackSelection =
		Presentation.Interaction.Category
			!= InPresentation.Interaction.Category
		&& (Presentation.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectMarker
			|| Presentation.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectRunner
			|| Presentation.Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectHelper);
	UpdateInlineFormulaRevealState(InPresentation);
	Presentation = InPresentation;
	if (bLeavingFeedbackSelection && SelectionFeedbackToast != nullptr)
	{
		SelectionFeedbackToast->DismissFeedback();
	}
	if (!bDeploymentDragActive)
	{
		InteractionState = EFMCodexUMGCardInteractionState::Default;
	}
	RefreshVisuals();
}

const FFMCodexUMGMatchScreenViewModel&
UFMCodexLocalMatchScreenWidget::GetPresentation() const
{
	return Presentation;
}

AFMCodexLocalMatchPlayerController*
UFMCodexLocalMatchScreenWidget::GetMatchController() const
{
	return MatchController;
}

UFMCodexMatchHeaderWidget*
UFMCodexLocalMatchScreenWidget::GetMatchHeader() const
{
	return MatchHeader;
}

UFMCodexPitchWidget* UFMCodexLocalMatchScreenWidget::GetPitchWidget() const
{
	return PitchWidget;
}

UFMCodexInteractionPanelWidget*
UFMCodexLocalMatchScreenWidget::GetInteractionPanel() const
{
	return InteractionPanel;
}

UFMCodexSelectionFeedbackToastWidget*
UFMCodexLocalMatchScreenWidget::GetSelectionFeedbackToast() const
{
	return SelectionFeedbackToast;
}

UFMCodexTacticalDetailPanelWidget*
UFMCodexLocalMatchScreenWidget::GetTacticalDetailPanel() const
{
	return TacticalDetailPanel;
}

bool UFMCodexLocalMatchScreenWidget::
IsDeploymentTacticalReferenceOpen() const
{
	return bDeploymentTacticalReferenceOpen;
}

ESkillRuleType UFMCodexLocalMatchScreenWidget::
GetDeploymentTacticalReferenceSkillType() const
{
	return DeploymentTacticalReferenceSkillType;
}

void UFMCodexLocalMatchScreenWidget::OpenDeploymentTacticalReference()
{
	if (TacticalDetailPanel == nullptr
		|| DeploymentTacticalReferenceControls == nullptr
		|| Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::Deploy)
	{
		return;
	}
	bDeploymentTacticalReferenceOpen = true;
	DeploymentTacticalReferenceControls->SetVisibility(
		ESlateVisibility::Visible);
	SelectDeploymentTacticalReference(ESkillRuleType::LongShot);
}

void UFMCodexLocalMatchScreenWidget::SelectDeploymentTacticalReference(
	const ESkillRuleType SkillType)
{
	if (!bDeploymentTacticalReferenceOpen || TacticalDetailPanel == nullptr
		|| Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::Deploy)
	{
		return;
	}
	const FFMCodexUMGTacticalDetailViewModel Detail =
		FFMCodexTacticalDetailPresentationBuilder::Build(SkillType);
	if (!Detail.bValid)
	{
		return;
	}
	CancelTacticalDetailDismiss();
	DeploymentTacticalReferenceSkillType = SkillType;
	TacticalDetailPanel->RefreshFromPresentation(Detail);
	TacticalDetailPanel->SetVisibility(ESlateVisibility::Visible);
}

void UFMCodexLocalMatchScreenWidget::CloseDeploymentTacticalReference()
{
	bDeploymentTacticalReferenceOpen = false;
	DeploymentTacticalReferenceSkillType = ESkillRuleType::None;
	if (DeploymentTacticalReferenceControls != nullptr)
	{
		DeploymentTacticalReferenceControls->SetVisibility(
			ESlateVisibility::Collapsed);
	}
	HideTacticalDetail();
}

UFMCodexResolutionPanelWidget*
UFMCodexLocalMatchScreenWidget::GetResolutionPanel() const
{
	return ResolutionPanel;
}

UFMCodexInlineResolutionFormulaSurfaceWidget*
UFMCodexLocalMatchScreenWidget::GetInlineFormulaSurface() const
{
	return InlineFormulaSurface;
}

UFMCodexThroughBallResolutionSurfaceWidget*
UFMCodexLocalMatchScreenWidget::GetThroughBallResolutionSurface() const
{
	return ThroughBallResolutionSurface;
}

bool UFMCodexLocalMatchScreenWidget::IsLegacyResolutionOverlayVisible() const
{
	return ResolutionOverlay != nullptr
		&& ResolutionOverlay->GetVisibility() != ESlateVisibility::Collapsed;
}

EFMCodexUMGInlineFormulaRevealPhase
UFMCodexLocalMatchScreenWidget::GetInlineFormulaRevealPhase() const
{
	return InlineFormulaRevealPhase;
}

bool UFMCodexLocalMatchScreenWidget::IsInlineFormulaRevealInputBlocked() const
{
	return InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight
		|| InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
		|| InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Settling
		|| InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
}

UFMCodexRollReelWidget*
UFMCodexLocalMatchScreenWidget::GetTacticalPointRollReel() const
{
	return TacticalPointRollReel;
}

#if WITH_DEV_AUTOMATION_TESTS
void UFMCodexLocalMatchScreenWidget::AdvanceInlineFormulaRevealForTesting(
	const float DeltaSeconds)
{
	AdvanceInlineFormulaReveal(DeltaSeconds, true);
}

void UFMCodexLocalMatchScreenWidget::PauseInlineFormulaRevealTimerForTesting()
{
	StopInlineFormulaRevealTimer();
}

void UFMCodexLocalMatchScreenWidget::BeginPendingCrossRollRevealForTesting()
{
	BeginInlineFormulaReveal(PendingCrossRollIdentity(Presentation), true);
}

void UFMCodexLocalMatchScreenWidget
	::BeginPendingTacticalPointRevealForTesting()
{
	const FFMCodexCrossRollRevealIdentity Identity =
		PendingCrossRollIdentity(Presentation);
	if (Identity.Kind == EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		BeginInlineFormulaReveal(Identity, true);
	}
}

void UFMCodexLocalMatchScreenWidget::ResetPrimaryActionDispatchForTesting()
{
	PrimaryActionDispatchCountForTesting = 0;
	LastPrimaryActionDispatchForTesting =
		EFMCodexUMGInteractionCategory::None;
}

int32 UFMCodexLocalMatchScreenWidget
	::GetPrimaryActionDispatchCountForTesting() const
{
	return PrimaryActionDispatchCountForTesting;
}

EFMCodexUMGInteractionCategory UFMCodexLocalMatchScreenWidget
	::GetLastPrimaryActionDispatchForTesting() const
{
	return LastPrimaryActionDispatchForTesting;
}
#endif

UFMCodexCardRackWidget*
UFMCodexLocalMatchScreenWidget::GetLocalRackWidget() const
{
	return LocalRackWidget;
}

UFMCodexCardRackWidget*
UFMCodexLocalMatchScreenWidget::GetOpponentRackWidget() const
{
	return OpponentRackWidget;
}

const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
UFMCodexLocalMatchScreenWidget::GetRenderedCandidateCardWidgets() const
{
	static const TArray<TObjectPtr<UFMCodexPlayerCardWidget>> Empty;
	return InteractionPanel != nullptr
		? InteractionPanel->GetRenderedCandidateCardWidgets() : Empty;
}

UFMCodexPlayerCardWidget*
UFMCodexLocalMatchScreenWidget::GetDetailOverlayCard() const
{
	return DetailOverlayCard;
}

bool UFMCodexLocalMatchScreenWidget::IsDetailOverlayVisible() const
{
	return DetailOverlayCard != nullptr
		&& DetailOverlayCard->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UFMCodexLocalMatchScreenWidget::IsDetailOverlayHitTestInvisible() const
{
	return DetailOverlayCanvas != nullptr && DetailOverlayCard != nullptr
		&& DetailOverlayCanvas->GetVisibility()
			== ESlateVisibility::HitTestInvisible
		&& DetailOverlayCard->GetVisibility()
			== ESlateVisibility::HitTestInvisible;
}

FVector2D UFMCodexLocalMatchScreenWidget::GetDetailOverlayPosition() const
{
	return DetailOverlayPosition;
}

EFMCodexUMGCardInteractionState
UFMCodexLocalMatchScreenWidget::GetInteractionState() const
{
	return InteractionState;
}

EFMCodexUMGCardInteractionState
UFMCodexLocalMatchScreenWidget::GetLastCompletedDragState() const
{
	return LastCompletedDragState;
}

int32 UFMCodexLocalMatchScreenWidget::GetFullCardProductionReviewCardCount() const
{
	return FullCardProductionReviewCards.Num();
}

const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
UFMCodexLocalMatchScreenWidget::GetFullCardProductionReviewCards() const
{
	return FullCardProductionReviewCards;
}

bool UFMCodexLocalMatchScreenWidget::IsFullCardProductionReviewVisible() const
{
	return FullCardProductionReviewBounds != nullptr
		&& FullCardProductionReviewBounds->GetVisibility()
			!= ESlateVisibility::Collapsed;
}

FVector2D UFMCodexLocalMatchScreenWidget::GetCanonicalFullCardDimensions()
{
	return FVector2D(360.0f, 540.0f);
}

FVector2D UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
	const FVector2D& SourcePosition,
	const FVector2D& SourceSize,
	const FVector2D& ViewportSize,
	const bool bOpenTowardRight)
{
	constexpr float Gap = 16.0f;
	constexpr float ViewportMargin = 12.0f;
	const FVector2D FullCardSize = GetCanonicalFullCardDimensions();
	const float DesiredX = bOpenTowardRight
		? SourcePosition.X + SourceSize.X + Gap
		: SourcePosition.X - FullCardSize.X - Gap;
	const float DesiredY = SourcePosition.Y
		+ (SourceSize.Y - FullCardSize.Y) * 0.5f;
	const float MaximumX = FMath::Max(
		ViewportMargin, ViewportSize.X - FullCardSize.X - ViewportMargin);
	const float MaximumY = FMath::Max(
		ViewportMargin, ViewportSize.Y - FullCardSize.Y - ViewportMargin);
	return FVector2D(
		FMath::Clamp(DesiredX, ViewportMargin, MaximumX),
		FMath::Clamp(DesiredY, ViewportMargin, MaximumY));
}

void UFMCodexLocalMatchScreenWidget::RequestStartNewMatch()
{
	if (MatchController != nullptr)
	{
		MatchController->StartNewDemoMatch();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestRollTacticalPoints()
{
	if (MatchController == nullptr || IsInlineFormulaRevealInputBlocked())
	{
		return;
	}
	const FFMCodexCrossRollRevealIdentity RequestedIdentity =
		PendingCrossRollIdentity(Presentation);
	if (RequestedIdentity.Kind
		== EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		BeginInlineFormulaReveal(RequestedIdentity, true);
	}
	MatchController->RollDemoTacticalPoints();
	if (!MatchController->GetLastDiagnostic().bHostSuccess)
	{
		CancelInlineFormulaReveal();
		ObservePendingCrossRoll(Presentation);
		RefreshVisuals();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	if (MatchController != nullptr && !IsInlineFormulaRevealInputBlocked())
	{
		MatchController->DeployOrdinary(CardId, SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployGoalkeeper(
	const FName SlotId)
{
	if (MatchController != nullptr && !IsInlineFormulaRevealInputBlocked())
	{
		MatchController->DeployGoalkeeper(SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestFinishDeployment()
{
	if (bDeploymentTacticalReferenceOpen)
	{
		CloseDeploymentTacticalReference();
	}
	if (MatchController != nullptr && !IsInlineFormulaRevealInputBlocked())
	{
		MatchController->FinishDeployment();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitCarrier(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitCarrier(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitMarker(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitMarker(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitSkill(const FName SkillId)
{
	HideTacticalDetail();
	if (MatchController != nullptr)
	{
		MatchController->SubmitSkill(SkillId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitRunner(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitRunner(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitHelper(const FName CardId)
{
	if (MatchController != nullptr)
	{
		MatchController->SubmitHelper(CardId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeclineSelection()
{
	if (Presentation.Interaction.Category
		== EFMCodexUMGInteractionCategory::SelectSkill)
	{
		HideTacticalDetail();
	}
	if (MatchController != nullptr)
	{
		if (Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectSkill)
		{
			MatchController->AbandonCurrentTacticalSelection();
		}
		else
		{
			MatchController->DeclineCurrentSelection();
		}
	}
}

void UFMCodexLocalMatchScreenWidget::RequestResolveNoLegalSelection()
{
	if (Presentation.Interaction.Category
		== EFMCodexUMGInteractionCategory::SelectSkill)
	{
		HideTacticalDetail();
	}
	if (MatchController != nullptr)
	{
		if (Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectSkill)
		{
			MatchController->AbandonCurrentTacticalSelection();
		}
		else
		{
			MatchController->ResolveNoLegalCurrentSelection();
		}
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitBranchIntent(
	const EFMCodexUMGBranchIntent Intent)
{
	if (MatchController == nullptr)
	{
		return;
	}
	switch (Intent)
	{
	case EFMCodexUMGBranchIntent::DirectShot:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::DirectShot);
		break;
	case EFMCodexUMGBranchIntent::DeadCorner:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::DeadCorner);
		break;
	case EFMCodexUMGBranchIntent::CrossHigh:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::CrossHigh);
		break;
	case EFMCodexUMGBranchIntent::CrossLow:
		MatchController->SubmitBranchIntent(
			EMatchPlayElectiveBranchIntent::CrossLow);
		break;
	default:
		break;
	}
}

void UFMCodexLocalMatchScreenWidget::RequestSubmitOneOnOneChoice(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	if (MatchController == nullptr)
	{
		return;
	}
	if (Choice == EFMCodexUMGOneOnOneChoice::ChipShot)
	{
		MatchController->SubmitOneOnOneShotChoice(
			EMatchPlayThroughBallOneOnOneShotChoice::ChipShot);
	}
	else if (Choice == EFMCodexUMGOneOnOneChoice::DirectShot)
	{
		MatchController->SubmitOneOnOneShotChoice(
			EMatchPlayThroughBallOneOnOneShotChoice::DirectShot);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestContinueResolution()
{
	if (IsInlineFormulaRevealInputBlocked() || MatchController == nullptr)
	{
		return;
	}
#if WITH_DEV_AUTOMATION_TESTS
	++PrimaryActionDispatchCountForTesting;
	LastPrimaryActionDispatchForTesting = Presentation.Interaction.Category;
#endif
	const FFMCodexCrossRollRevealIdentity RequestedIdentity =
		PendingCrossRollIdentity(Presentation);
	if (RequestedIdentity.IsValid()
		&& !SettledCrossRollRevealKeys.Contains(
			RequestedIdentity.StableKey()))
	{
		BeginInlineFormulaReveal(RequestedIdentity, true);
	}
	switch (Presentation.Interaction.Category)
	{
	case EFMCodexUMGInteractionCategory::RollCrossAttack:
		MatchController->RollCrossAttack();
		break;
	case EFMCodexUMGInteractionCategory::RollCrossDefense:
		MatchController->RollCrossDefense();
		break;
	case EFMCodexUMGInteractionCategory::CompleteCrossAndAdvance:
		MatchController->CompleteCrossAndAdvance();
		break;
	case EFMCodexUMGInteractionCategory::RollThroughBallFeetAttack:
		MatchController->RollThroughBallFeetAttack();
		break;
	case EFMCodexUMGInteractionCategory::RollThroughBallFeetDefense:
		MatchController->RollThroughBallFeetDefense();
		break;
	case EFMCodexUMGInteractionCategory::CompleteThroughBallFeetAndAdvance:
		MatchController->CompleteThroughBallFeetAndAdvance();
		break;
	case EFMCodexUMGInteractionCategory::ApplyCrossTerminalResolution:
		MatchController->ApplyCrossTerminalResolution();
		break;
	case EFMCodexUMGInteractionCategory
		::ApplyThroughBallFeetTerminalResolution:
		MatchController->ApplyThroughBallFeetTerminalResolution();
		break;
	case EFMCodexUMGInteractionCategory::AdvanceAfterTerminal:
		MatchController->AdvanceAfterTerminal();
		break;
	default:
		MatchController->ContinueResolution();
		break;
	}
	if (RequestedIdentity.IsValid()
		&& !MatchController->GetLastDiagnostic().bHostSuccess)
	{
		CancelInlineFormulaReveal();
		ObservePendingCrossRoll(Presentation);
		RefreshVisuals();
	}
}

void UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked()
{
	RequestStartNewMatch();
}

void UFMCodexLocalMatchScreenWidget::HandleTacticalPointRollClicked()
{
	RequestRollTacticalPoints();
}

void UFMCodexLocalMatchScreenWidget::HandleDeployOrdinaryRequested(
	const FName CardId,
	const FName SlotId)
{
	RequestDeployOrdinary(CardId, SlotId);
}

void UFMCodexLocalMatchScreenWidget::HandleDeployGoalkeeperRequested(
	const FName SlotId)
{
	RequestDeployGoalkeeper(SlotId);
}

void UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked()
{
	RequestFinishDeployment();
}

void UFMCodexLocalMatchScreenWidget::HandleCarrierRequested(const FName CardId)
{
	RequestSubmitCarrier(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleOnPitchSelectionRequested(
	const EFMCodexUMGOnPitchSelectionIntent Intent,
	const FName OptionId)
{
	const bool bMatchesCarrierPrompt =
		Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectCarrier
		&& Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitCarrier;
	const bool bMatchesMarkerPrompt =
		Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectMarker
		&& Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitMarker;
	const bool bMatchesRunnerPrompt =
		Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectRunner
		&& Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitRunner;
	const bool bMatchesHelperPrompt =
		Presentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::SelectHelper
		&& Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitHelper;
	if (!Presentation.Interaction.bUseOnPitchPlayerSelection
		|| (!bMatchesCarrierPrompt && !bMatchesMarkerPrompt
			&& !bMatchesRunnerPrompt && !bMatchesHelperPrompt)
		|| OptionId.IsNone())
	{
		return;
	}
	const bool bProjectedCandidate = Presentation.PitchRegions.ContainsByPredicate(
		[Intent, OptionId](const FFMCodexUMGPitchRegionViewModel& Region)
		{
			return Region.Slots.ContainsByPredicate(
				[Intent, OptionId](const FFMCodexUMGPitchSlotViewModel& PitchSlot)
				{
					return PitchSlot.bSelectableForCurrentPrompt
						&& PitchSlot.OnPitchSelectionIntent == Intent
						&& PitchSlot.OnPitchSelectionOptionId == OptionId;
				});
		});
	if (bProjectedCandidate)
	{
		HideDetailOverlay();
		if (SelectionFeedbackToast != nullptr)
		{
			SelectionFeedbackToast->DismissFeedback();
		}
		if (Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitCarrier)
		{
			RequestSubmitCarrier(OptionId);
		}
		else if (Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitMarker)
		{
			RequestSubmitMarker(OptionId);
		}
		else if (Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitRunner)
		{
			RequestSubmitRunner(OptionId);
		}
		else if (Intent == EFMCodexUMGOnPitchSelectionIntent::SubmitHelper)
		{
			RequestSubmitHelper(OptionId);
		}
	}
}

void UFMCodexLocalMatchScreenWidget::HandleSelectionFeedbackRequested(
	const FName CardId)
{
	if ((Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectMarker
		&& Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectRunner
		&& Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectHelper)
		|| SelectionFeedbackToast == nullptr || CardId.IsNone())
	{
		return;
	}
	const FFMCodexUMGPitchSlotViewModel* FeedbackSlot = nullptr;
	for (const FFMCodexUMGPitchRegionViewModel& Region
		: Presentation.PitchRegions)
	{
		FeedbackSlot = Region.Slots.FindByPredicate(
			[CardId](const FFMCodexUMGPitchSlotViewModel& PitchSlotView)
			{
				return PitchSlotView.bOccupied
					&& PitchSlotView.Card.CardId == CardId
					&& PitchSlotView.SelectionFeedbackReason
						!= EFMCodexUMGSelectionFeedbackReason::None
					&& !PitchSlotView.SelectionFeedbackLabel.IsEmpty();
			});
		if (FeedbackSlot != nullptr)
		{
			break;
		}
	}
	if (FeedbackSlot != nullptr)
	{
		SelectionFeedbackToast->ShowFeedback(
			FeedbackSlot->SelectionFeedbackReason,
			FeedbackSlot->SelectionFeedbackLabel);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleMarkerRequested(const FName CardId)
{
	RequestSubmitMarker(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleSkillRequested(const FName SkillId)
{
	RequestSubmitSkill(SkillId);
}

void UFMCodexLocalMatchScreenWidget::HandleTacticalDetailRequested(
	const FName SkillId)
{
	if (TacticalDetailPanel == nullptr || SkillId.IsNone()
		|| Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectSkill)
	{
		return;
	}
	const FFMCodexUMGSelectionChoiceViewModel* Choice =
		Presentation.Interaction.SelectionChoices.FindByPredicate(
			[SkillId](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.OptionId == SkillId
					&& Candidate.SkillType != ESkillRuleType::None;
			});
	if (Choice == nullptr)
	{
		return;
	}
	const FFMCodexUMGTacticalDetailViewModel Detail =
		FFMCodexTacticalDetailPresentationBuilder::Build(Choice->SkillType);
	if (!Detail.bValid)
	{
		return;
	}
	CancelTacticalDetailDismiss();
	bTacticalCardHoverOrFocus = true;
	ActiveTacticalDetailSkillId = SkillId;
	TacticalDetailPanel->RefreshFromPresentation(Detail);
	TacticalDetailPanel->SetVisibility(ESlateVisibility::Visible);
}

void UFMCodexLocalMatchScreenWidget::HandleTacticalDetailDismissed(
	const FName SkillId)
{
	if (SkillId == ActiveTacticalDetailSkillId)
	{
		bTacticalCardHoverOrFocus = false;
		ScheduleTacticalDetailDismiss();
	}
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentTacticalReferenceRequested()
{
	OpenDeploymentTacticalReference();
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferenceLongShotClicked()
{
	SelectDeploymentTacticalReference(ESkillRuleType::LongShot);
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferenceCutInsideClicked()
{
	SelectDeploymentTacticalReference(ESkillRuleType::CutInsideShot);
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferencePassControlClicked()
{
	SelectDeploymentTacticalReference(ESkillRuleType::PassControl);
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferenceCrossClicked()
{
	SelectDeploymentTacticalReference(ESkillRuleType::Cross);
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferenceThroughBallClicked()
{
	SelectDeploymentTacticalReference(ESkillRuleType::ThroughBall);
}

void UFMCodexLocalMatchScreenWidget::
HandleDeploymentReferenceCloseClicked()
{
	CloseDeploymentTacticalReference();
}

void UFMCodexLocalMatchScreenWidget::HandleTacticalDetailPointerEntered()
{
	bTacticalDetailPointerInside = true;
	CancelTacticalDetailDismiss();
}

void UFMCodexLocalMatchScreenWidget::HandleTacticalDetailPointerLeft()
{
	bTacticalDetailPointerInside = false;
	ScheduleTacticalDetailDismiss();
}

void UFMCodexLocalMatchScreenWidget::ScheduleTacticalDetailDismiss()
{
	if (bDeploymentTacticalReferenceOpen
		|| bTacticalCardHoverOrFocus || bTacticalDetailPointerInside
		|| TacticalDetailPanel == nullptr
		|| TacticalDetailPanel->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}
	CancelTacticalDetailDismiss();
	if (UWorld* World = GetWorld())
	{
		TacticalDetailDismissTimerHandle = World->GetTimerManager()
			.SetTimerForNextTick(FTimerDelegate::CreateUObject(
				this,
				&UFMCodexLocalMatchScreenWidget::CompleteTacticalDetailDismiss));
	}
	else
	{
		CompleteTacticalDetailDismiss();
	}
}

void UFMCodexLocalMatchScreenWidget::CancelTacticalDetailDismiss()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TacticalDetailDismissTimerHandle);
	}
	TacticalDetailDismissTimerHandle.Invalidate();
}

void UFMCodexLocalMatchScreenWidget::CompleteTacticalDetailDismiss()
{
	TacticalDetailDismissTimerHandle.Invalidate();
	if (!bTacticalCardHoverOrFocus && !bTacticalDetailPointerInside)
	{
		HideTacticalDetail();
	}
}

void UFMCodexLocalMatchScreenWidget::HideTacticalDetail()
{
	CancelTacticalDetailDismiss();
	ActiveTacticalDetailSkillId = NAME_None;
	bTacticalCardHoverOrFocus = false;
	bTacticalDetailPointerInside = false;
	if (TacticalDetailPanel != nullptr)
	{
		TacticalDetailPanel->ClearPresentation();
		TacticalDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleRunnerRequested(const FName CardId)
{
	RequestSubmitRunner(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleHelperRequested(const FName CardId)
{
	RequestSubmitHelper(CardId);
}

void UFMCodexLocalMatchScreenWidget::HandleDeclineRequested()
{
	RequestDeclineSelection();
}

void UFMCodexLocalMatchScreenWidget::HandleNoLegalRequested()
{
	RequestResolveNoLegalSelection();
}

void UFMCodexLocalMatchScreenWidget::HandleBranchRequested(
	const EFMCodexUMGBranchIntent Intent)
{
	RequestSubmitBranchIntent(Intent);
}

void UFMCodexLocalMatchScreenWidget::HandleOneOnOneRequested(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	RequestSubmitOneOnOneChoice(Choice);
}

void UFMCodexLocalMatchScreenWidget::HandleContinueRequested()
{
	RequestContinueResolution();
}

void UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted(
	const FName CardId,
	const bool bGoalkeeper)
{
	if (PitchWidget == nullptr || Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::Deploy)
	{
		return;
	}
	const bool bPresentedChoice =
		Presentation.Interaction.DeploymentChoices.ContainsByPredicate(
			[CardId, bGoalkeeper](
				const FFMCodexUMGDeploymentChoiceViewModel& Choice)
			{
				return Choice.CardId == CardId
					&& Choice.bGoalkeeper == bGoalkeeper;
			});
	if (bPresentedChoice)
	{
		if (bDeploymentTacticalReferenceOpen)
		{
			CloseDeploymentTacticalReference();
		}
		HideDetailOverlay();
		bDeploymentDragActive = true;
		bDeploymentDropSubmitted = false;
		InteractionState = EFMCodexUMGCardInteractionState::Dragging;
		PitchWidget->BeginDeploymentDrag(
			CardId, Presentation.Interaction.DeploymentChoices);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished()
{
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	HideDetailOverlay();
	LastCompletedDragState = bDeploymentDropSubmitted
		? EFMCodexUMGCardInteractionState::DropSuccess
		: EFMCodexUMGCardInteractionState::DropCancelled;
	InteractionState = LastCompletedDragState;
	bDeploymentDragActive = false;
	bDeploymentDropSubmitted = false;
}

void UFMCodexLocalMatchScreenWidget::HandlePitchDeploymentDropped(
	const FName CardId,
	const FName SlotId,
	const bool bGoalkeeper)
{
	HideDetailOverlay();
	if (PitchWidget != nullptr)
	{
		PitchWidget->EndDeploymentDrag();
	}
	if (bGoalkeeper)
	{
		RequestDeployGoalkeeper(SlotId);
	}
	else
	{
		RequestDeployOrdinary(CardId, SlotId);
	}
	const FString ExpectedCommand = bGoalkeeper
		? TEXT("DeployGoalkeeper") : TEXT("DeployOrdinary");
	bDeploymentDropSubmitted = MatchController != nullptr
		&& MatchController->GetLastDiagnostic().CommandName == ExpectedCommand
		&& MatchController->GetLastDiagnostic().bHostSuccess;
	InteractionState = bDeploymentDropSubmitted
		? EFMCodexUMGCardInteractionState::DropSuccess
		: EFMCodexUMGCardInteractionState::DropCancelled;
}

void UFMCodexLocalMatchScreenWidget::BindDetailHoverSources()
{
	auto BindCard = [this](UFMCodexPlayerCardWidget* Card)
	{
		if (Card == nullptr)
		{
			return;
		}
		Card->OnDetailHoverRequested.AddUObject(
			this, &UFMCodexLocalMatchScreenWidget::HandleDetailHoverRequested);
		Card->OnDetailHoverDismissed.AddUObject(
			this, &UFMCodexLocalMatchScreenWidget::HandleDetailHoverDismissed);
	};
	if (LocalRackWidget != nullptr)
	{
		for (UFMCodexPlayerCardWidget* Card
			: LocalRackWidget->GetRenderedCardWidgets())
		{
			BindCard(Card);
		}
	}
	if (OpponentRackWidget != nullptr)
	{
		for (UFMCodexPlayerCardWidget* Card
			: OpponentRackWidget->GetRenderedCardWidgets())
		{
			BindCard(Card);
		}
	}
}

void UFMCodexLocalMatchScreenWidget::HandleDetailHoverRequested(
	UFMCodexPlayerCardWidget* SourceCard)
{
	if (!bDeploymentDragActive)
	{
		ShowDetailOverlay(SourceCard);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleDetailHoverDismissed(
	UFMCodexPlayerCardWidget* SourceCard)
{
	if (DetailHoverSource == SourceCard)
	{
		HideDetailOverlay();
	}
}

void UFMCodexLocalMatchScreenWidget::ShowDetailOverlay(
	UFMCodexPlayerCardWidget* SourceCard)
{
	if (SourceCard == nullptr || DetailOverlayCard == nullptr
		|| !SourceCard->CanExposeFullCardDetail())
	{
		return;
	}
	DetailHoverSource = SourceCard;
	DetailOverlayCard->RefreshFromPresentation(
		SourceCard->GetPresentation(),
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	DetailOverlayCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	PositionDetailOverlay(SourceCard);
	InteractionState = EFMCodexUMGCardInteractionState::Hover;
}

void UFMCodexLocalMatchScreenWidget::HideDetailOverlay()
{
	DetailHoverSource = nullptr;
	if (DetailOverlayCard != nullptr)
	{
		DetailOverlayCard->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (!bDeploymentDragActive
		&& InteractionState == EFMCodexUMGCardInteractionState::Hover)
	{
		InteractionState = EFMCodexUMGCardInteractionState::Default;
	}
}

void UFMCodexLocalMatchScreenWidget::PositionDetailOverlay(
	UFMCodexPlayerCardWidget* SourceCard)
{
	if (SourceCard == nullptr || DetailOverlayCard == nullptr)
	{
		return;
	}
	const FGeometry ScreenGeometry = GetCachedGeometry();
	const FGeometry SourceGeometry = SourceCard->GetCachedGeometry();
	FVector2D ViewportSize = ScreenGeometry.GetLocalSize();
	if (ViewportSize.X <= 0.0f || ViewportSize.Y <= 0.0f)
	{
		ViewportSize = FVector2D(1920.0f, 1080.0f);
	}
	const FVector2D SourcePosition = ScreenGeometry.AbsoluteToLocal(
		SourceGeometry.LocalToAbsolute(FVector2D::ZeroVector));
	FVector2D SourceSize = SourceGeometry.GetLocalSize();
	if (SourceSize.X <= 0.0f || SourceSize.Y <= 0.0f)
	{
		SourceSize = SourceCard->GetConfiguredDimensions();
	}
	const bool bOpenTowardRight = SourcePosition.X + SourceSize.X * 0.5f
		<= ViewportSize.X * 0.5f;
	DetailOverlayPosition = CalculateDetailOverlayPosition(
		SourcePosition, SourceSize, ViewportSize, bOpenTowardRight);
	if (UCanvasPanelSlot* OverlaySlot = Cast<UCanvasPanelSlot>(
		DetailOverlayCard->Slot))
	{
		OverlaySlot->SetPosition(DetailOverlayPosition);
		OverlaySlot->SetSize(GetCanonicalFullCardDimensions());
		OverlaySlot->SetAutoSize(false);
	}
}

void UFMCodexLocalMatchScreenWidget::BuildWidgetTree()
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("MatchScreenRoot"));
	WidgetTree->RootWidget = Root;
	UBorder* ScreenBackground = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchScreenStyleBackground"));
	FFMCodexPlayerUIStyle::Get().ApplyBorder(
		*ScreenBackground, EFMCodexPlayerUIColorRole::ScreenBackground,
		FMargin(0.0f));
	if (UOverlaySlot* BackgroundSlot = Root->AddChildToOverlay(ScreenBackground))
	{
		BackgroundSlot->SetHorizontalAlignment(HAlign_Fill);
		BackgroundSlot->SetVerticalAlignment(VAlign_Fill);
	}

	MainScreen = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("GoldenLayoutMatchScreen"));
	USizeBox* ResponsiveBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ResponsiveGoldenLayoutBounds"));
	ResponsiveBounds->SetWidthOverride(1920.0f);
	ResponsiveBounds->AddChild(MainScreen);
	if (UOverlaySlot* MainSlot = Root->AddChildToOverlay(ResponsiveBounds))
	{
		MainSlot->SetHorizontalAlignment(HAlign_Center);
		MainSlot->SetVerticalAlignment(VAlign_Center);
	}

	USizeBox* HeaderBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("BroadcastMatchHeaderRegion"));
	HeaderBounds->SetHeightOverride(80.0f);
	UBorder* HeaderRegion = MakeRegion(*WidgetTree, TEXT("MatchHeaderRegion"));
	UClass* ResolvedHeaderClass = MatchHeaderWidgetClass != nullptr
		? MatchHeaderWidgetClass.Get() : UFMCodexMatchHeaderWidget::StaticClass();
	MatchHeader = WidgetTree->ConstructWidget<UFMCodexMatchHeaderWidget>(
		ResolvedHeaderClass, TEXT("DedicatedMatchHeaderWidget"));
	HeaderRegion->AddChild(MatchHeader);
	HeaderBounds->AddChild(HeaderRegion);
	MainScreen->AddChildToVerticalBox(HeaderBounds);

	USizeBox* MainAreaBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("GoldenLayoutMainMatchArea"));
	MainAreaBounds->SetHeightOverride(880.0f);
	UHorizontalBox* MainArea = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RackPitchRackHierarchy"));
	MainAreaBounds->AddChild(MainArea);
	LocalRackBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("LocalPlayerCardRackRegion"));
	LocalRackBounds->SetWidthOverride(422.0f);
	LocalRackWidget = WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
		UFMCodexCardRackWidget::StaticClass(), TEXT("PersistentLocalCardRack"));
	LocalRackWidget->OnCardDragStarted.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted);
	LocalRackWidget->OnCardDragFinished.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished);
	LocalRackBounds->AddChild(LocalRackWidget);
	MainArea->AddChildToHorizontalBox(LocalRackBounds);

	PitchBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("CentralPitchRegion"));
	PitchBounds->SetWidthOverride(1076.0f);
	UBorder* PitchRegion = MakeRegion(*WidgetTree, TEXT("FootballCardFieldRegion"));
	UClass* ResolvedPitchClass = PitchWidgetClass != nullptr
		? PitchWidgetClass.Get() : UFMCodexPitchWidget::StaticClass();
	PitchWidget = WidgetTree->ConstructWidget<UFMCodexPitchWidget>(
		ResolvedPitchClass, TEXT("DedicatedFootballPitchWidget"));
	PitchWidget->OnDeploymentDropped.AddUObject(
		this,
		&UFMCodexLocalMatchScreenWidget::HandlePitchDeploymentDropped);
	PitchWidget->OnCardDetailHoverRequested.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDetailHoverRequested);
	PitchWidget->OnCardDetailHoverDismissed.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDetailHoverDismissed);
	PitchWidget->OnOnPitchSelectionRequested.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleOnPitchSelectionRequested);
	PitchWidget->OnSelectionFeedbackRequested.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleSelectionFeedbackRequested);
	UOverlay* PitchPresentationLayers = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("PitchPresentationLayers"));
	if (UOverlaySlot* PitchLayerSlot =
		PitchPresentationLayers->AddChildToOverlay(PitchWidget))
	{
		PitchLayerSlot->SetHorizontalAlignment(HAlign_Fill);
		PitchLayerSlot->SetVerticalAlignment(VAlign_Fill);
	}
	UClass* ResolvedInlineFormulaClass =
		InlineFormulaSurfaceWidgetClass != nullptr
			? InlineFormulaSurfaceWidgetClass.Get()
			: UFMCodexInlineResolutionFormulaSurfaceWidget::StaticClass();
	InlineFormulaSurface = WidgetTree->ConstructWidget<
		UFMCodexInlineResolutionFormulaSurfaceWidget>(
			ResolvedInlineFormulaClass, TEXT("InlineResolutionFormulaSurface"));
	InlineFormulaSurface->OnContinueRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleContinueRequested);
	if (UOverlaySlot* FormulaLayerSlot =
		PitchPresentationLayers->AddChildToOverlay(InlineFormulaSurface))
	{
		FormulaLayerSlot->SetHorizontalAlignment(HAlign_Center);
		FormulaLayerSlot->SetVerticalAlignment(VAlign_Center);
		FormulaLayerSlot->SetPadding(FMargin(28.0f));
	}
	UClass* ResolvedThroughBallClass =
		ThroughBallResolutionSurfaceWidgetClass != nullptr
			? ThroughBallResolutionSurfaceWidgetClass.Get()
			: UFMCodexThroughBallResolutionSurfaceWidget::StaticClass();
	ThroughBallResolutionSurface = WidgetTree->ConstructWidget<
		UFMCodexThroughBallResolutionSurfaceWidget>(
			ResolvedThroughBallClass,
			TEXT("ThroughBallProductionResolutionSurface"));
	ThroughBallResolutionSurface->OnContinueRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleContinueRequested);
	if (UOverlaySlot* ThroughBallLayerSlot =
		PitchPresentationLayers->AddChildToOverlay(
			ThroughBallResolutionSurface))
	{
		ThroughBallLayerSlot->SetHorizontalAlignment(HAlign_Center);
		ThroughBallLayerSlot->SetVerticalAlignment(VAlign_Center);
		ThroughBallLayerSlot->SetPadding(FMargin(28.0f));
	}

	TacticalPointRevealSurface = MakeRegion(
		*WidgetTree, TEXT("TacticalPointRollRevealSurface"));
	TacticalPointRevealSurface->SetPadding(FMargin(22.0f, 16.0f));
	TacticalPointRevealSurface->SetVisibility(ESlateVisibility::Collapsed);
	UVerticalBox* TacticalPointRevealBody =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("TacticalPointRollRevealHierarchy"));
	TacticalPointRevealTitle = MakeText(
		*WidgetTree, TEXT("TacticalPointRollRevealTitle"), TEXT("战术点掷点"));
	TacticalPointRevealTitle->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*TacticalPointRevealTitle, EFMCodexPlayerUITextRole::ActionTitle);
	TacticalPointRevealBody->AddChildToVerticalBox(TacticalPointRevealTitle);
	TacticalPointRollReel = WidgetTree->ConstructWidget<UFMCodexRollReelWidget>(
		UFMCodexRollReelWidget::StaticClass(), TEXT("TacticalPointRollReel"));
	if (UVerticalBoxSlot* ReelSlot =
		TacticalPointRevealBody->AddChildToVerticalBox(TacticalPointRollReel))
	{
		ReelSlot->SetHorizontalAlignment(HAlign_Center);
		ReelSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 8.0f));
	}
	TacticalPointRevealResult = MakeText(
		*WidgetTree, TEXT("TacticalPointRollRevealResult"));
	TacticalPointRevealResult->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*TacticalPointRevealResult, EFMCodexPlayerUITextRole::SectionHeading);
	TacticalPointRevealBody->AddChildToVerticalBox(TacticalPointRevealResult);
	TacticalPointRevealSurface->AddChild(TacticalPointRevealBody);
	if (UOverlaySlot* TacticalPointLayerSlot =
		PitchPresentationLayers->AddChildToOverlay(TacticalPointRevealSurface))
	{
		TacticalPointLayerSlot->SetHorizontalAlignment(HAlign_Center);
		TacticalPointLayerSlot->SetVerticalAlignment(VAlign_Center);
		TacticalPointLayerSlot->SetPadding(FMargin(28.0f));
	}
	PitchRegion->AddChild(PitchPresentationLayers);
	PitchBounds->AddChild(PitchRegion);
	if (UHorizontalBoxSlot* PitchSlot = MainArea->AddChildToHorizontalBox(PitchBounds))
	{
		PitchSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		PitchSlot->SetHorizontalAlignment(HAlign_Fill);
		PitchSlot->SetVerticalAlignment(VAlign_Fill);
	}
	OpponentRackBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("OpponentCardRackRegion"));
	OpponentRackBounds->SetWidthOverride(422.0f);
	OpponentRackWidget = WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
		UFMCodexCardRackWidget::StaticClass(), TEXT("PersistentOpponentCardRack"));
	OpponentRackBounds->AddChild(OpponentRackWidget);
	MainArea->AddChildToHorizontalBox(OpponentRackBounds);
	MainScreen->AddChildToVerticalBox(MainAreaBounds);

	USizeBox* DockBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ContextActionDockRegion"));
	DockBounds->SetHeightOverride(120.0f);
	DockBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UBorder* InteractionRegion = MakeRegion(
		*WidgetTree, TEXT("CurrentInteractionRegion"));
	UClass* ResolvedInteractionClass = InteractionPanelWidgetClass != nullptr
		? InteractionPanelWidgetClass.Get()
		: UFMCodexInteractionPanelWidget::StaticClass();
	InteractionPanel =
		WidgetTree->ConstructWidget<UFMCodexInteractionPanelWidget>(
			ResolvedInteractionClass, TEXT("DedicatedInteractionPanelWidget"));
	InteractionPanel->OnStartMatchRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked);
	InteractionPanel->OnTacticalPointRollRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleTacticalPointRollClicked);
	InteractionPanel->OnDeployOrdinaryRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeployOrdinaryRequested);
	InteractionPanel->OnDeployGoalkeeperRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeployGoalkeeperRequested);
	InteractionPanel->OnFinishDeploymentRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked);
	InteractionPanel->OnDeploymentTacticalReferenceRequested.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentTacticalReferenceRequested);
	InteractionPanel->OnCarrierRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleCarrierRequested);
	InteractionPanel->OnMarkerRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleMarkerRequested);
	InteractionPanel->OnSkillRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleSkillRequested);
	InteractionPanel->OnTacticalDetailRequested.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleTacticalDetailRequested);
	InteractionPanel->OnTacticalDetailDismissed.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleTacticalDetailDismissed);
	InteractionPanel->OnRunnerRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleRunnerRequested);
	InteractionPanel->OnHelperRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleHelperRequested);
	InteractionPanel->OnDeclineRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeclineRequested);
	InteractionPanel->OnNoLegalRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleNoLegalRequested);
	InteractionPanel->OnBranchRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleBranchRequested);
	InteractionPanel->OnOneOnOneRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleOneOnOneRequested);
	InteractionPanel->OnContinueRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleContinueRequested);
	InteractionPanel->OnDeploymentDragStarted.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragStarted);
	InteractionPanel->OnDeploymentDragFinished.AddUObject(
		this, &UFMCodexLocalMatchScreenWidget::HandleDeploymentDragFinished);
	InteractionRegion->AddChild(InteractionPanel);
	DockBounds->AddChild(InteractionRegion);
	MainScreen->AddChildToVerticalBox(DockBounds);

	SelectionFeedbackToast =
		WidgetTree->ConstructWidget<UFMCodexSelectionFeedbackToastWidget>(
			UFMCodexSelectionFeedbackToastWidget::StaticClass(),
			TEXT("SelectionFeedbackToast"));
	SelectionFeedbackToast->SetVisibility(ESlateVisibility::Collapsed);
	if (UOverlaySlot* ToastSlot = Root->AddChildToOverlay(
		SelectionFeedbackToast))
	{
		ToastSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 132.0f));
		ToastSlot->SetHorizontalAlignment(HAlign_Center);
		ToastSlot->SetVerticalAlignment(VAlign_Bottom);
	}

	TacticalDetailSurface = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SharedTacticalDetailSurface"));
	USizeBox* ReferenceControlsBounds =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			TEXT("DeploymentTacticalReferenceControlsBounds"));
	ReferenceControlsBounds->SetWidthOverride(780.0f);
	DeploymentTacticalReferenceControls =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			TEXT("DeploymentTacticalReferenceControls"));
	FFMCodexPlayerUIStyle::Get().ApplyBorder(
		*DeploymentTacticalReferenceControls,
		EFMCodexPlayerUIColorRole::PanelRaised, FMargin(8.0f, 6.0f));
	DeploymentTacticalReferenceControls->SetVisibility(
		ESlateVisibility::Collapsed);
	UHorizontalBox* ReferenceSelector =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			TEXT("DeploymentTacticalReferenceSelector"));
	UTextBlock* ReferenceLabel = MakeText(
		*WidgetTree, TEXT("DeploymentTacticalReferenceLabel"),
		FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			TEXT("TACTICAL REFERENCE")).ToString());
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*ReferenceLabel, EFMCodexPlayerUITextRole::SectionHeading);
	if (UHorizontalBoxSlot* LabelSlot =
		ReferenceSelector->AddChildToHorizontalBox(ReferenceLabel))
	{
		LabelSlot->SetPadding(FMargin(0.0f, 0.0f, 6.0f, 0.0f));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	auto AddReferenceButton = [this, ReferenceSelector](
		UButton* Button, const float Width, const float LeftGap = 2.0f)
	{
		USizeBox* ButtonBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), FName(*FString::Printf(
				TEXT("%sBounds"), *Button->GetName())));
		ButtonBounds->SetWidthOverride(Width);
		ButtonBounds->SetHeightOverride(38.0f);
		ButtonBounds->AddChild(Button);
		if (UHorizontalBoxSlot* ButtonSlot =
			ReferenceSelector->AddChildToHorizontalBox(ButtonBounds))
		{
			ButtonSlot->SetPadding(FMargin(LeftGap, 0.0f, 2.0f, 0.0f));
			ButtonSlot->SetVerticalAlignment(VAlign_Center);
		}
	};
	auto MakeReferenceButton = [this](
		const FName Name, const ESkillRuleType SkillType)
	{
		const FFMCodexUMGTacticalDetailViewModel Detail =
			FFMCodexTacticalDetailPresentationBuilder::Build(SkillType);
		UButton* Button = MakeButton(*WidgetTree, Name, Detail.DisplayName);
		if (UTextBlock* ButtonLabel = Cast<UTextBlock>(Button->GetChildAt(0)))
		{
			ButtonLabel->SetAutoWrapText(false);
			ButtonLabel->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*ButtonLabel, EFMCodexPlayerUITextRole::SectionHeading);
		}
		FFMCodexPlayerUIStyle::Get().ApplyButton(
			*Button, EFMCodexPlayerUIActionRole::Secondary);
		return Button;
	};
	UButton* LongShotButton = MakeReferenceButton(
		TEXT("DeploymentReferenceLongShotButton"), ESkillRuleType::LongShot);
	LongShotButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferenceLongShotClicked);
	AddReferenceButton(LongShotButton, 74.0f);
	UButton* CutInsideButton = MakeReferenceButton(
		TEXT("DeploymentReferenceCutInsideButton"),
		ESkillRuleType::CutInsideShot);
	CutInsideButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferenceCutInsideClicked);
	AddReferenceButton(CutInsideButton, 74.0f);
	UButton* PassControlButton = MakeReferenceButton(
		TEXT("DeploymentReferencePassControlButton"),
		ESkillRuleType::PassControl);
	PassControlButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferencePassControlClicked);
	AddReferenceButton(PassControlButton, 108.0f);
	UButton* CrossButton = MakeReferenceButton(
		TEXT("DeploymentReferenceCrossButton"), ESkillRuleType::Cross);
	CrossButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferenceCrossClicked);
	AddReferenceButton(CrossButton, 74.0f);
	UButton* ThroughBallButton = MakeReferenceButton(
		TEXT("DeploymentReferenceThroughBallButton"),
		ESkillRuleType::ThroughBall);
	ThroughBallButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferenceThroughBallClicked);
	AddReferenceButton(ThroughBallButton, 74.0f);
	USpacer* ReferenceCloseSpacer = WidgetTree->ConstructWidget<USpacer>(
		USpacer::StaticClass(), TEXT("DeploymentReferenceCloseSpacer"));
	if (UHorizontalBoxSlot* SpacerSlot =
		ReferenceSelector->AddChildToHorizontalBox(ReferenceCloseSpacer))
	{
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	UButton* CloseReferenceButton = MakeButton(
		*WidgetTree, TEXT("DeploymentReferenceCloseButton"),
		FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			TEXT("CLOSE TACTICAL REFERENCE")).ToString());
	FFMCodexPlayerUIStyle::Get().ApplyButton(
		*CloseReferenceButton, EFMCodexPlayerUIActionRole::Decline);
	if (UTextBlock* CloseLabel = Cast<UTextBlock>(
		CloseReferenceButton->GetChildAt(0)))
	{
		CloseLabel->SetAutoWrapText(false);
		CloseLabel->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*CloseLabel, EFMCodexPlayerUITextRole::SectionHeading);
	}
	CloseReferenceButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleDeploymentReferenceCloseClicked);
	AddReferenceButton(CloseReferenceButton, 146.0f, 8.0f);
	DeploymentTacticalReferenceControls->AddChild(ReferenceSelector);
	ReferenceControlsBounds->AddChild(DeploymentTacticalReferenceControls);
	if (UVerticalBoxSlot* ControlsSlot =
		TacticalDetailSurface->AddChildToVerticalBox(ReferenceControlsBounds))
	{
		ControlsSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 9.0f));
		ControlsSlot->SetHorizontalAlignment(HAlign_Center);
	}

	TacticalDetailPanel =
		WidgetTree->ConstructWidget<UFMCodexTacticalDetailPanelWidget>(
			UFMCodexTacticalDetailPanelWidget::StaticClass(),
			TEXT("SharedTacticalDetailPanel"));
	TacticalDetailPanel->SetVisibility(ESlateVisibility::Collapsed);
	TacticalDetailPanel->OnDetailPointerEntered.AddUObject(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleTacticalDetailPointerEntered);
	TacticalDetailPanel->OnDetailPointerLeft.AddUObject(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleTacticalDetailPointerLeft);
	if (UVerticalBoxSlot* PanelSlot =
		TacticalDetailSurface->AddChildToVerticalBox(TacticalDetailPanel))
	{
		PanelSlot->SetHorizontalAlignment(HAlign_Center);
	}
	if (UOverlaySlot* DetailSlot =
		Root->AddChildToOverlay(TacticalDetailSurface))
	{
		DetailSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 132.0f));
		DetailSlot->SetHorizontalAlignment(HAlign_Center);
		DetailSlot->SetVerticalAlignment(VAlign_Bottom);
	}

	DetailOverlayCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("FullCardDetailOverlayCanvas"));
	DetailOverlayCanvas->SetVisibility(ESlateVisibility::HitTestInvisible);
	DetailOverlayCard = WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
		UFMCodexPlayerCardWidget::StaticClass(), TEXT("TransientFullCardDetail"));
	DetailOverlayCard->SetVisibility(ESlateVisibility::Collapsed);
	if (UCanvasPanelSlot* DetailCardSlot =
		DetailOverlayCanvas->AddChildToCanvas(DetailOverlayCard))
	{
		DetailCardSlot->SetPosition(FVector2D::ZeroVector);
		DetailCardSlot->SetSize(GetCanonicalFullCardDimensions());
	}
	if (UOverlaySlot* DetailCanvasSlot =
		Root->AddChildToOverlay(DetailOverlayCanvas))
	{
		DetailCanvasSlot->SetHorizontalAlignment(HAlign_Fill);
		DetailCanvasSlot->SetVerticalAlignment(VAlign_Fill);
	}

#if !UE_BUILD_SHIPPING
	if (FMCodexHandMicroDiagnostics::IsProductionReviewEnabled())
	{
		HandMicroProductionReviewSurface = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("HandMicroProductionReviewSurface"));
		HandMicroProductionReviewSurface->SetBrushColor(
			FLinearColor::FromSRGBColor(FColor(0x08, 0x16, 0x20)));
		HandMicroProductionReviewSurface->SetPadding(FMargin(16.0f, 12.0f));
		HandMicroProductionReviewSurface->SetClipping(
			EWidgetClipping::ClipToBounds);
		UOverlay* ReviewPageStack = WidgetTree->ConstructWidget<UOverlay>(
			UOverlay::StaticClass(), TEXT("HandMicroProductionReviewPageStack"));
		HandMicroProductionReviewSurface->AddChild(ReviewPageStack);

		struct FProductionReviewCase
		{
			FName CardId;
			FString Name;
			FString Position;
			FString Slug;
			FString Rarity;
			bool bGoalkeeper = false;
		};
		const TArray<FProductionReviewCase> ProductionCases = {
			{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("拉亚"), TEXT("GK"),
				TEXT("Raya"), TEXT("Continental"), true },
			{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("萨利巴"), TEXT("D"),
				TEXT("Saliba"), TEXT("Continental") },
			{ TEXT("Prototype.Arsenal.DeclanRice"), TEXT("赖斯"), TEXT("M/D"),
				TEXT("Rice"), TEXT("Continental") },
			{ TEXT("Prototype.Arsenal.GabrielMartinelli"), TEXT("马丁内利"), TEXT("A"),
				TEXT("Martinelli"), TEXT("National") },
			{ TEXT("Prototype.Arsenal.GabrielMagalhaes"), TEXT("加布里埃尔"), TEXT("D"),
				TEXT("Gabriel"), TEXT("Continental") },
			{ TEXT("Prototype.Arsenal.MikelMerino"), TEXT("梅里诺"), TEXT("M/A"),
				TEXT("Merino"), TEXT("National") },
			{ TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("萨卡"), TEXT("A/M"),
				TEXT("Saka"), TEXT("World Class") },
			{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("厄德高"), TEXT("M/A"),
				TEXT("Odegaard"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
				TEXT("多纳鲁马"), TEXT("GK"), TEXT("Donnarumma"),
				TEXT("Continental"), true },
			{ TEXT("Prototype.ManchesterCity.RubenDias"), TEXT("迪亚斯"), TEXT("D"),
				TEXT("Dias"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.Rodri"), TEXT("罗德里"), TEXT("M/D"),
				TEXT("Rodri"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"), TEXT("格瓦迪奥尔"), TEXT("D"),
				TEXT("Gvardiol"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.BernardoSilva"), TEXT("贝尔纳多"), TEXT("M/A"),
				TEXT("Bernardo"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.JeremyDoku"), TEXT("多库"), TEXT("A"),
				TEXT("Doku"), TEXT("National") },
			{ TEXT("Prototype.ManchesterCity.PhilFoden"), TEXT("福登"), TEXT("A/M"),
				TEXT("Foden"), TEXT("Continental") },
			{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), TEXT("哈兰德"), TEXT("A"),
				TEXT("Haaland"), TEXT("World Class") }
		};
		auto MakeReviewCard = [this](const FProductionReviewCase& ReviewCase,
			const FString& Prefix) -> UFMCodexPlayerCardWidget*
		{
			FFMCodexUMGCardViewModel CardModel;
			CardModel.CardId = ReviewCase.CardId;
			CardModel.IdentityLabel = ReviewCase.Name;
			CardModel.OwnerLabel = TEXT("Production Review");
			CardModel.RoleLabel = ReviewCase.Position;
			CardModel.RarityLabel = ReviewCase.Rarity;
			CardModel.bGoalkeeper = ReviewCase.bGoalkeeper;
			UFMCodexPlayerCardWidget* Card =
				WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
					UFMCodexPlayerCardWidget::StaticClass(),
					FName(*(Prefix + ReviewCase.Slug + TEXT("Card"))));
			Card->RefreshFromPresentation(
				CardModel, EFMCodexPlayerCardPresentationMode::HandMicro);
			return Card;
		};

		UVerticalBox* PortraitsPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewPortraitsPage"));
		HandMicroProductionReviewPortraitsPage = PortraitsPage;
		UTextBlock* PortraitsTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewPortraitsTitle"),
			TEXT("HAND MICRO PRODUCTION PORTRAITS — APPROVED 16"));
		PortraitsTitle->SetAutoWrapText(false);
		PortraitsTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*PortraitsTitle, EFMCodexPlayerUITextRole::Status);
		PortraitsPage->AddChildToVerticalBox(PortraitsTitle);
		UUniformGridPanel* PortraitsGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(),
				TEXT("HandMicroProductionReviewPortraitsGrid"));
		PortraitsGrid->SetSlotPadding(FMargin(5.0f, 3.0f));
		PortraitsPage->AddChildToVerticalBox(PortraitsGrid);
		ReviewPageStack->AddChildToOverlay(PortraitsPage);
		for (int32 Index = 0; Index < ProductionCases.Num(); ++Index)
		{
			UFMCodexPlayerCardWidget* Card = MakeReviewCard(
				ProductionCases[Index], TEXT("ProductionPortrait"));
			if (UUniformGridSlot* ReviewGridSlot =
				PortraitsGrid->AddChildToUniformGrid(Card, Index / 2, Index % 2))
			{
				ReviewGridSlot->SetHorizontalAlignment(HAlign_Center);
				ReviewGridSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		UVerticalBox* TypographyPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewTypographyPage"));
		HandMicroProductionReviewTypographyPage = TypographyPage;
		UTextBlock* TypographyTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewTypographyTitle"),
			TEXT("HAND MICRO PRODUCTION NAME STRESS — 16 SHRINK-ONLY TO 12"));
		TypographyTitle->SetAutoWrapText(false);
		TypographyTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*TypographyTitle, EFMCodexPlayerUITextRole::Status);
		TypographyPage->AddChildToVerticalBox(TypographyTitle);
		UUniformGridPanel* TypographyGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(),
				TEXT("HandMicroProductionReviewTypographyGrid"));
		TypographyGrid->SetSlotPadding(FMargin(5.0f, 4.0f));
		TypographyPage->AddChildToVerticalBox(TypographyGrid);
		ReviewPageStack->AddChildToOverlay(TypographyPage);
		const TArray<FProductionReviewCase> TypographyCases = {
			ProductionCases[0], ProductionCases[1], ProductionCases[3],
			ProductionCases[4],
			{ TEXT("Visual.HandMicro.Kvaratskhelia"), TEXT("克瓦拉茨赫利亚"),
				TEXT("A/M/D"), TEXT("Stress"), TEXT("Continental") }
		};
		for (int32 Index = 0; Index < TypographyCases.Num(); ++Index)
		{
			UFMCodexPlayerCardWidget* Card = MakeReviewCard(
				TypographyCases[Index], TEXT("ProductionTypography"));
			if (UUniformGridSlot* ReviewGridSlot =
				TypographyGrid->AddChildToUniformGrid(Card, Index, 0))
			{
				ReviewGridSlot->SetHorizontalAlignment(HAlign_Center);
				ReviewGridSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		UVerticalBox* LayoutPage = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("HandMicroProductionReviewLayoutPage"));
		HandMicroProductionReviewLayoutPage = LayoutPage;
		UTextBlock* LayoutTitle = MakeText(*WidgetTree,
			TEXT("HandMicroProductionReviewLayoutTitle"),
			TEXT("HAND MICRO PRODUCTION LAYOUT — 2x10 / GHOST / NO PAGING"));
		LayoutTitle->SetAutoWrapText(false);
		LayoutTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LayoutTitle, EFMCodexPlayerUITextRole::Status);
		LayoutPage->AddChildToVerticalBox(LayoutTitle);
		FFMCodexUMGCardRackViewModel ReviewRackModel;
		ReviewRackModel.SideLabel = TEXT("Production Review");
		ReviewRackModel.bLocalRack = true;
		ReviewRackModel.ColumnCount = 2;
		ReviewRackModel.RowCount = 10;
		for (int32 Index = 0; Index < 20; ++Index)
		{
			FFMCodexUMGCardRackCellViewModel& Cell =
				ReviewRackModel.Cells.AddDefaulted_GetRef();
			Cell.StableIndex = Index;
			Cell.bPlayed = Index >= 4;
			const FProductionReviewCase& ReviewCase =
				ProductionCases[Index % ProductionCases.Num()];
			Cell.Card.CardId = ReviewCase.CardId;
			Cell.Card.IdentityLabel = ReviewCase.Name;
			Cell.Card.OwnerLabel = TEXT("Production Review");
			Cell.Card.RoleLabel = ReviewCase.Position;
			Cell.Card.RarityLabel = ReviewCase.Rarity;
			Cell.Card.bGoalkeeper = ReviewCase.bGoalkeeper;
		}
		UFMCodexCardRackWidget* ReviewRack =
			WidgetTree->ConstructWidget<UFMCodexCardRackWidget>(
				UFMCodexCardRackWidget::StaticClass(),
				TEXT("HandMicroProductionReviewRack"));
		ReviewRack->RefreshFromPresentation(ReviewRackModel);
		LayoutPage->AddChildToVerticalBox(ReviewRack);
		ReviewPageStack->AddChildToOverlay(LayoutPage);

		HandMicroProductionReviewBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), TEXT("HandMicroProductionReviewBounds"));
		HandMicroProductionReviewBounds->SetWidthOverride(560.0f);
		HandMicroProductionReviewBounds->SetHeightOverride(660.0f);
		HandMicroProductionReviewBounds->AddChild(
			HandMicroProductionReviewSurface);
	}
	// Construct the review surface in non-shipping builds even when the CVar is
	// currently off. This lets a developer toggle it during a live PIE session;
	// RefreshFullCardProductionReviewSurface owns its visibility.
	{
		FullCardProductionReviewBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), TEXT("FullCardProductionReviewBounds"));
		FullCardProductionReviewBounds->SetWidthOverride(780.0f);
		FullCardProductionReviewBounds->SetHeightOverride(610.0f);
		FullCardProductionReviewBounds->SetVisibility(
			ESlateVisibility::Collapsed);
		UBorder* FullCardReviewSurface = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), TEXT("FullCardProductionReviewSurface"));
		FullCardReviewSurface->SetBrushColor(
			FLinearColor::FromSRGBColor(FColor(0x04, 0x0E, 0x16)));
		FullCardReviewSurface->SetPadding(FMargin(12.0f, 8.0f));
		UVerticalBox* ReviewBody = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), TEXT("FullCardProductionReviewBody"));
		UTextBlock* ReviewTitle = MakeText(*WidgetTree,
			TEXT("FullCardProductionReviewTitle"),
			TEXT("IN-MATCH FULL CARD — DRAFT PRODUCTION REVIEW"));
		ReviewTitle->SetAutoWrapText(false);
		ReviewTitle->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*ReviewTitle, EFMCodexPlayerUITextRole::Status);
		ReviewBody->AddChildToVerticalBox(ReviewTitle);
		FullCardProductionReviewGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(),
				TEXT("FullCardProductionReviewGrid"));
		FullCardProductionReviewGrid->SetSlotPadding(FMargin(5.0f, 3.0f));
		ReviewBody->AddChildToVerticalBox(FullCardProductionReviewGrid);
		FullCardReviewSurface->AddChild(ReviewBody);
		FullCardProductionReviewBounds->AddChild(FullCardReviewSurface);
	}
#endif
	ResolutionOverlay = MakeRegion(
		*WidgetTree, TEXT("ResolutionPresentationLayer"));
	USizeBox* ResolutionBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ResolutionResultRegion"));
	ResolutionBounds->SetWidthOverride(720.0f);
	ResolutionBounds->SetMaxDesiredHeight(640.0f);
	UClass* ResolvedResolutionClass = ResolutionPanelWidgetClass != nullptr
		? ResolutionPanelWidgetClass.Get()
		: UFMCodexResolutionPanelWidget::StaticClass();
	ResolutionPanel =
		WidgetTree->ConstructWidget<UFMCodexResolutionPanelWidget>(
			ResolvedResolutionClass, TEXT("DedicatedResolutionPanelWidget"));
	ResolutionPanel->OnContinueRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleContinueRequested);
	ResolutionBounds->AddChild(ResolutionPanel);
	if (UBorderSlot* ResolutionSlot = Cast<UBorderSlot>(
		ResolutionOverlay->AddChild(ResolutionBounds)))
	{
		ResolutionSlot->SetHorizontalAlignment(HAlign_Center);
		ResolutionSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UOverlaySlot* OverlaySlot = Root->AddChildToOverlay(ResolutionOverlay))
	{
		OverlaySlot->SetHorizontalAlignment(HAlign_Fill);
		OverlaySlot->SetVerticalAlignment(VAlign_Fill);
	}

#if !UE_BUILD_SHIPPING
	// Keep the opt-in development review above full-screen presentation layers.
	if (HandMicroProductionReviewBounds != nullptr)
	{
		if (UOverlaySlot* ReviewSlot = Root->AddChildToOverlay(
			HandMicroProductionReviewBounds))
		{
			ReviewSlot->SetHorizontalAlignment(HAlign_Center);
			ReviewSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
	if (FullCardProductionReviewBounds != nullptr)
	{
		if (UOverlaySlot* ReviewSlot = Root->AddChildToOverlay(
			FullCardProductionReviewBounds))
		{
			ReviewSlot->SetHorizontalAlignment(HAlign_Center);
			ReviewSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
#endif
}

void UFMCodexLocalMatchScreenWidget::RefreshFullCardProductionReviewSurface()
{
#if UE_BUILD_SHIPPING
	return;
#else
	if (FullCardProductionReviewBounds == nullptr
		|| FullCardProductionReviewGrid == nullptr)
	{
		return;
	}
	const bool bReviewEnabled =
		FMCodexFullCardDiagnostics::IsProductionReviewEnabled();
	FullCardProductionReviewBounds->SetVisibility(bReviewEnabled
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (!bReviewEnabled)
	{
		return;
	}
	FullCardProductionReviewGrid->ClearChildren();
	FullCardProductionReviewCards.Reset();
	TArray<const FFMCodexUMGCardViewModel*> Candidates;
	auto CollectCandidates = [&Candidates](
		const FFMCodexUMGCardRackViewModel& Rack, const bool bPrototypeOnly)
	{
		for (const FFMCodexUMGCardRackCellViewModel& Cell : Rack.Cells)
		{
			if (!Cell.Card.CardId.IsNone()
				&& (!bPrototypeOnly || Cell.Card.CardId.ToString().StartsWith(
					TEXT("Prototype."))))
			{
				Candidates.Add(&Cell.Card);
			}
		}
	};
	CollectCandidates(Presentation.LocalRack, true);
	CollectCandidates(Presentation.OpponentRack, true);
	if (Candidates.IsEmpty())
	{
		CollectCandidates(Presentation.LocalRack, false);
		CollectCandidates(Presentation.OpponentRack, false);
	}
	TArray<const FFMCodexUMGCardViewModel*> Selected;
	TArray<FFMCodexUMGCardViewModel> ReviewOnlyModels;
	ReviewOnlyModels.Reserve(2);
	auto AddUnique = [&Selected](const FFMCodexUMGCardViewModel* Candidate)
	{
		if (Selected.Num() < 2 && Candidate != nullptr
			&& !Selected.ContainsByPredicate(
			[Candidate](const FFMCodexUMGCardViewModel* Existing)
			{
				return Existing != nullptr
					&& Existing->CardId == Candidate->CardId;
			}))
		{
			Selected.Add(Candidate);
		}
	};
	auto FindCandidate = [&Candidates](const FName CardId)
		-> const FFMCodexUMGCardViewModel*
	{
		const FFMCodexUMGCardViewModel* const* Match =
			Candidates.FindByPredicate(
				[CardId](const FFMCodexUMGCardViewModel* Candidate)
				{
					return Candidate != nullptr
						&& Candidate->CardId == CardId;
				});
		return Match == nullptr ? nullptr : *Match;
	};
	// Two true-size cards fit comfortably at 1920x1080. Five bounded review
	// pages cover all six current conformance-rollout Hero Bust portraits, one
	// frozen comparison pair, and the presentation-only three-Skill stress DTO
	// without
	// shrinking the frozen 360x540 production card.
	const int32 ReviewPage =
		FMCodexFullCardDiagnostics::GetProductionReviewPage();
	if (ReviewPage == 0)
	{
		AddUnique(FindCandidate(
			TEXT("Prototype.Arsenal.WilliamSaliba")));
		AddUnique(FindCandidate(
			TEXT("Prototype.Arsenal.MartinOdegaard")));
	}
	else if (ReviewPage == 1)
	{
		AddUnique(FindCandidate(TEXT("Prototype.Arsenal.DeclanRice")));
		AddUnique(FindCandidate(
			TEXT("Prototype.ManchesterCity.ErlingHaaland")));
	}
	else if (ReviewPage == 2)
	{
		AddUnique(FindCandidate(
			TEXT("Prototype.ManchesterCity.PhilFoden")));
		AddUnique(FindCandidate(
			TEXT("Prototype.ManchesterCity.RubenDias")));
	}
	else if (ReviewPage == 3)
	{
		AddUnique(FindCandidate(TEXT("Prototype.Arsenal.BukayoSaka")));
		AddUnique(FindCandidate(
			TEXT("Prototype.ManchesterCity.Rodri")));
	}
	else
	{
		const FFMCodexUMGCardViewModel* StressBase = FindCandidate(
			TEXT("Prototype.ManchesterCity.Rodri"));
		if (StressBase != nullptr)
		{
			FFMCodexUMGCardViewModel StressCard = *StressBase;
			StressCard.Skills.Reset();
			StressCard.SkillLabels.Reset();
			StressCard.DeveloperReferenceLabel =
				TEXT("FullCardReview.ThreeSkillStress");
			TSet<FString> AddedSkillLabels;
			for (const FFMCodexUMGCardViewModel* Candidate : Candidates)
			{
				if (Candidate == nullptr)
				{
					continue;
				}
				for (const FFMCodexUMGSkillViewModel& Skill : Candidate->Skills)
				{
					if (StressCard.Skills.Num() >= 3)
					{
						break;
					}
					if (!Skill.CanonicalLabel.IsEmpty()
						&& Skill.MinTriggerActionPoint > 0
						&& Skill.MaxTriggerActionPoint
							>= Skill.MinTriggerActionPoint
						&& !AddedSkillLabels.Contains(Skill.CanonicalLabel))
					{
						StressCard.Skills.Add(Skill);
						AddedSkillLabels.Add(Skill.CanonicalLabel);
					}
				}
				if (StressCard.Skills.Num() >= 3)
				{
					break;
				}
			}
			ReviewOnlyModels.Add(MoveTemp(StressCard));
			AddUnique(&ReviewOnlyModels.Last());
		}
		AddUnique(FindCandidate(
			TEXT("Prototype.Arsenal.GabrielMagalhaes")));
	}
	if (UTextBlock* ReviewTitle = Cast<UTextBlock>(
		GetWidgetFromName(TEXT("FullCardProductionReviewTitle"))))
	{
		ReviewTitle->SetText(FText::FromString(FString::Printf(
			TEXT("IN-MATCH FULL CARD — 360x540 VISUAL REVIEW — PAGE %d/5"),
			ReviewPage + 1)));
	}
	const FFMCodexUMGCardViewModel* ShortName = nullptr;
	const FFMCodexUMGCardViewModel* LongName = nullptr;
	const FFMCodexUMGCardViewModel* Goalkeeper = nullptr;
	for (const FFMCodexUMGCardViewModel* Candidate : Candidates)
	{
		if (Candidate == nullptr)
		{
			continue;
		}
		if (ShortName == nullptr
			|| Candidate->IdentityLabel.Len() < ShortName->IdentityLabel.Len())
		{
			ShortName = Candidate;
		}
		if (LongName == nullptr
			|| Candidate->IdentityLabel.Len() > LongName->IdentityLabel.Len())
		{
			LongName = Candidate;
		}
		if (Goalkeeper == nullptr && Candidate->bGoalkeeper)
		{
			Goalkeeper = Candidate;
		}
	}
	AddUnique(ShortName);
	AddUnique(LongName);
	AddUnique(Goalkeeper);
	for (const FFMCodexUMGCardViewModel* Candidate : Candidates)
	{
		if (Selected.Num() >= 2)
		{
			break;
		}
		const bool bAddsRarity = Selected.IsEmpty()
			|| !Selected.ContainsByPredicate(
				[Candidate](const FFMCodexUMGCardViewModel* Existing)
				{
					return Candidate != nullptr && Existing != nullptr
						&& Existing->RarityLabel == Candidate->RarityLabel;
				});
		if (bAddsRarity)
		{
			AddUnique(Candidate);
		}
	}
	for (const FFMCodexUMGCardViewModel* Candidate : Candidates)
	{
		if (Selected.Num() >= 2)
		{
			break;
		}
		AddUnique(Candidate);
	}
	for (int32 Index = 0; Index < Selected.Num(); ++Index)
	{
		UFMCodexPlayerCardWidget* Card =
			WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
				UFMCodexPlayerCardWidget::StaticClass(),
				FName(*FString::Printf(TEXT("FullCardProductionReviewCard%d"),
					Index)));
		Card->RefreshFromPresentation(*Selected[Index],
			EFMCodexPlayerCardPresentationMode::InteractionChoice);
		Card->TakeWidget();
		if (UUniformGridSlot* GridSlot =
			FullCardProductionReviewGrid->AddChildToUniformGrid(Card, 0, Index))
		{
			GridSlot->SetHorizontalAlignment(HAlign_Center);
			GridSlot->SetVerticalAlignment(VAlign_Center);
		}
		FullCardProductionReviewCards.Add(Card);
}
#endif
}

void UFMCodexLocalMatchScreenWidget::UpdateInlineFormulaRevealState(
	const FFMCodexUMGMatchScreenViewModel& InPresentation)
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (ActiveCrossRollReveal.IsValid())
	{
		if (InPresentation.Header.AttackSequence
			!= ActiveCrossRollReveal.AttackSequence
			|| InPresentation.Resolution.bRejected)
		{
			CancelInlineFormulaReveal();
			ObservePendingCrossRoll(InPresentation);
			return;
		}

		int32 RawValue = 0;
		int32 DomainMinimum = 1;
		int32 DomainMaximum = 6;
		if (TryReadAuthoritativeRawRoll(
			InPresentation, ActiveCrossRollReveal, RawValue,
			DomainMinimum, DomainMaximum))
		{
			if (!bInlineFormulaAuthorityResultAvailable
				&& InlineFormulaRevealPhaseElapsed <= 0.05f)
			{
				RollRevealSequenceOffsetCells = PlannedSequenceOffset(
					RawValue, DomainMinimum, DomainMaximum);
			}
			// Cache only the exact authority-built presentation. No arithmetic is
			// reconstructed in this widget.
			CachedResolvedInlineFormula = ActiveFormula(InPresentation);
			bInlineFormulaAuthorityResultAvailable = true;
			RollRevealAuthoritativeRawValue = RawValue;
			RollRevealDomainMinimum = DomainMinimum;
			RollRevealDomainMaximum = DomainMaximum;
			if (ActiveCrossRollReveal.Kind
				== EFMCodexUMGCrossRollRevealKind::TacticalPoint)
			{
				CachedTacticalPointFinalValue =
					InPresentation.Header.CurrentAttackerTacticalPoints;
			}
			if (InlineFormulaRevealPhase
				== EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight)
			{
				InlineFormulaRevealPhase =
					EFMCodexUMGInlineFormulaRevealPhase::Cycling;
			}
			if (InlineFormulaRevealPhase
					== EFMCodexUMGInlineFormulaRevealPhase::Cycling
				&& InlineFormulaRevealPhaseElapsed >= FinalSlowEndTime)
			{
				BeginInlineFormulaFinalCapture();
			}
			StartInlineFormulaRevealTimer();
		}
		return;
	}

	if (ObservedPendingCrossRoll.IsValid())
	{
		if (InPresentation.Header.AttackSequence
			!= ObservedPendingCrossRoll.AttackSequence)
		{
			ObservedPendingCrossRoll = {};
		}
		else
		{
			int32 RawValue = 0;
			int32 DomainMinimum = 1;
			int32 DomainMaximum = 6;
			if (TryReadAuthoritativeRawRoll(
				InPresentation, ObservedPendingCrossRoll, RawValue,
				DomainMinimum, DomainMaximum))
			{
				const FFMCodexCrossRollRevealIdentity ResolvedIdentity =
					ObservedPendingCrossRoll;
				BeginInlineFormulaReveal(ResolvedIdentity, false);
				CachedResolvedInlineFormula = ActiveFormula(InPresentation);
				bInlineFormulaAuthorityResultAvailable = true;
				RollRevealAuthoritativeRawValue = RawValue;
				RollRevealDomainMinimum = DomainMinimum;
				RollRevealDomainMaximum = DomainMaximum;
				RollRevealSequenceOffsetCells = PlannedSequenceOffset(
					RawValue, DomainMinimum, DomainMaximum);
				if (ResolvedIdentity.Kind
					== EFMCodexUMGCrossRollRevealKind::TacticalPoint)
				{
					CachedTacticalPointFinalValue =
						InPresentation.Header.CurrentAttackerTacticalPoints;
				}
				return;
			}

			// A command refresh can project the next pending side before its
			// feedback copy exposes the accepted result. Hold the old identity.
			const FFMCodexCrossRollRevealIdentity Candidate =
				PendingCrossRollIdentity(InPresentation);
			if (Candidate.IsValid()
				&& Candidate == ObservedPendingCrossRoll)
			{
				InlineFormulaRevealPhase =
					EFMCodexUMGInlineFormulaRevealPhase::IdlePending;
			}
			return;
		}
	}

	ObservePendingCrossRoll(InPresentation);
}

FFMCodexUMGInlineFormulaSurfaceViewModel
UFMCodexLocalMatchScreenWidget::BuildDisplayedInlineFormula() const
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (ActiveCrossRollReveal.Kind
		== EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		return Presentation.InlineFormula;
	}
	if (ActiveCrossRollReveal.Kind
		== EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute)
	{
		return Presentation.InlineFormula;
	}
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Source =
		ActiveFormula(Presentation);
	if (!IsInlineFormulaRevealInputBlocked()
		|| !ActiveCrossRollReveal.IsValid())
	{
		const FFMCodexCrossRollRevealIdentity Candidate =
			PendingCrossRollIdentity(Presentation);
		if (Candidate.IsValid()
			&& SettledCrossRollRevealKeys.Contains(Candidate.StableKey())
			&& LastDisclosedInlineFormula.bVisible)
		{
			return LastDisclosedInlineFormula;
		}
		return Source;
	}

	FFMCodexUMGInlineFormulaSurfaceViewModel Result =
		bInlineFormulaAuthorityResultAvailable
			? CachedResolvedInlineFormula : Source;
	Result.bVisible = true;
	Result.bSuppressLegacyResolution = true;
	Result.RevealPhase = InlineFormulaRevealPhase;
	Result.bDiceRevealVisible = true;
	Result.bDiceRolling = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight
		|| InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling
		|| InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Settling;
	Result.ActiveRollSequenceIndex =
		ActiveCrossRollReveal.RollSequenceIndex;
	Result.RevealAnimationFrame = CosmeticCycleStep(
		InlineFormulaRevealPhaseElapsed);
	const bool bHolding = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
	const bool bSettling = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::Settling;
	const bool bAuthoritativeVisible = bHolding;
	Result.RollReel = BuildActiveRollReelPresentation();
	const int32 VisibleD6 = bAuthoritativeVisible
		? RollRevealAuthoritativeRawValue
		: Result.RollReel.CenterValue;
	Result.DiceFaceLabel = FString::FromInt(VisibleD6);
	Result.DiceOwnerLabel =
		ActiveCrossRollReveal.Kind
			== EFMCodexUMGCrossRollRevealKind::Defense
				? TEXT("防守方掷点") : TEXT("进攻方掷点");
	// Retain the exact claim while the reveal gate hides its button. This keeps
	// the lower panel from leaking the next authoritative action early.
	Result.PrimaryAction.bVisible = false;
	Result.bCanContinue = false;
	Result.ContinueActionLabel.Empty();
	const bool bFormulaDisclosed = bHolding
		&& InlineFormulaRevealPhaseElapsed >= FormulaDisclosureDelay;
	const bool bNarrativeDisclosed =
		ActiveCrossRollReveal.Kind
			== EFMCodexUMGCrossRollRevealKind::Defense
		&& bHolding
		&& InlineFormulaRevealPhaseElapsed >= NarrativeDisclosureDelay;
	if (!bNarrativeDisclosed)
	{
		Result.bNarrativeAvailable = false;
		Result.NarrativeHeadline.Empty();
		Result.ResultSubtitle.Empty();
		// Authority aliases a terminal Narrative into these two visible labels.
		// Restore the neutral contest hierarchy until Presentation has disclosed
		// the Defense formula and completed the short Narrative transition.
		Result.ContestLabel = FFMCodexPlayerUIPresentationText
			::ResolutionContest(Result.ContestId).ToString();
		Result.StatusLabel = bHolding
			? TEXT("权威结果已落定")
			: bSettling ? TEXT("权威掷点落定中") : TEXT("号码滚动中");
	}

	if (ActiveCrossRollReveal.Kind
		== EFMCodexUMGCrossRollRevealKind::InitialRoute)
	{
		Result.ContestId = TEXT("Cross.Route");
		Result.ContestLabel = TEXT("传中路线判定");
		if (!bHolding)
		{
			Result.RouteResultLabel.Empty();
		}
		Result.TacticalPlayerSummaryLabel.Empty();
		Result.bShowFormulaRows = false;
		Result.bAttackRowActive = false;
		Result.bDefenseRowActive = false;
		return Result;
	}

	Result.bShowFormulaRows = true;
	const bool bAttack = ActiveCrossRollReveal.Kind
		== EFMCodexUMGCrossRollRevealKind::Attack;
	Result.bAttackRowActive = bAttack;
	Result.bDefenseRowActive = !bAttack;
	if (!bFormulaDisclosed)
	{
		StageRowForReveal(
			bAttack ? Result.AttackRow : Result.DefenseRow,
			!bHolding,
			bAuthoritativeVisible,
			VisibleD6);
	}
	return Result;
}

FFMCodexUMGThroughBallResolutionViewModel
UFMCodexLocalMatchScreenWidget::BuildDisplayedThroughBallResolution() const
{
	using namespace FMCodexLocalMatchScreenWidget;
	FFMCodexUMGThroughBallResolutionViewModel Result =
		Presentation.ThroughBallResolution;
	if (Result.Formula.bVisible)
	{
		Result.Formula = BuildDisplayedInlineFormula();
	}
	if (ActiveCrossRollReveal.Kind
		!= EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute
		|| !IsInlineFormulaRevealInputBlocked())
	{
		return Result;
	}

	Result.bVisible = true;
	Result.bSuppressLegacyResolution = true;
	Result.Stage = EFMCodexUMGThroughBallStage::InitialRoute;
	Result.StageLabel = FFMCodexPlayerUIPresentationText
		::ThroughBallInitialRouteStage().ToString();
	Result.RevealPhase = InlineFormulaRevealPhase;
	Result.bDiceRevealVisible = true;
	Result.RollReel = BuildActiveRollReelPresentation();
	Result.bInitialRouteRollAwaitingInput = false;
	// The route surface still owns the exact action during its reveal; only the
	// button visibility is transient presentation state.
	Result.PrimaryAction.bVisible = false;
	Result.bCanContinue = false;
	Result.ContinueActionLabel.Empty();
	const bool bHolding = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
	const bool bSettling = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::Settling;
	Result.bRouteRevealComplete = bHolding;
	if (!bHolding)
	{
		Result.RouteLabel.Empty();
		Result.RouteResultLabel.Empty();
		Result.StatusLabel = bSettling
			? TEXT("路线掷点落定中") : TEXT("号码滚动中");
	}
	else
	{
		Result.StatusLabel = TEXT("路线判定完成");
	}
	Result.ActionPromptLabel.Empty();
	return Result;
}

FFMCodexUMGMatchHeaderViewModel
UFMCodexLocalMatchScreenWidget::BuildDisplayedHeader() const
{
	if (ActiveCrossRollReveal.Kind
		!= EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		return Presentation.Header;
	}
	const bool bResourceDisclosed = InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
		&& InlineFormulaRevealPhaseElapsed
			>= FMCodexLocalMatchScreenWidget::FormulaDisclosureDelay;
	return bResourceDisclosed ? Presentation.Header : CachedPreRollHeader;
}

FFMCodexUMGRollReelViewModel
UFMCodexLocalMatchScreenWidget::BuildActiveRollReelPresentation() const
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (!ActiveCrossRollReveal.IsValid()
		|| !IsInlineFormulaRevealInputBlocked())
	{
		return {};
	}
	return BuildReelPresentation(
		InlineFormulaRevealPhase,
		InlineFormulaRevealPhaseElapsed,
		RollRevealDomainMinimum,
		RollRevealDomainMaximum,
		RollRevealAuthoritativeRawValue,
		RollRevealCaptureStartPositionCells,
		RollRevealCaptureDistanceCells,
		RollRevealSequenceOffsetCells);
}

void UFMCodexLocalMatchScreenWidget::AdvanceInlineFormulaReveal(
	float DeltaSeconds,
	const bool bForceFullRefresh)
{
	using namespace FMCodexLocalMatchScreenWidget;
	const EFMCodexUMGInlineFormulaRevealPhase PreviousPhase =
		InlineFormulaRevealPhase;
	const float PreviousPhaseElapsed = InlineFormulaRevealPhaseElapsed;
	DeltaSeconds = FMath::Max(0.0f, DeltaSeconds);
	while (DeltaSeconds > 0.0f && IsInlineFormulaRevealInputBlocked())
	{
		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Cycling;
		}

		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Cycling)
		{
			const float Remaining = FMath::Max(
				0.0f,
				FinalSlowEndTime - InlineFormulaRevealPhaseElapsed);
			const float Consumed = FMath::Min(DeltaSeconds, Remaining);
			InlineFormulaRevealPhaseElapsed += Consumed;
			DeltaSeconds -= Consumed;
			if (InlineFormulaRevealPhaseElapsed < FinalSlowEndTime)
			{
				break;
			}
			if (!bInlineFormulaAuthorityResultAvailable)
			{
				// Network-safe wait: keep cosmetic cycling, never settle a guess.
				InlineFormulaRevealPhaseElapsed += DeltaSeconds;
				DeltaSeconds = 0.0f;
				break;
			}
			BeginInlineFormulaFinalCapture();
			continue;
		}

		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Settling)
		{
			const float Remaining = FMath::Max(
				0.0f, CaptureSettleDuration - InlineFormulaRevealPhaseElapsed);
			const float Consumed = FMath::Min(DeltaSeconds, Remaining);
			InlineFormulaRevealPhaseElapsed += Consumed;
			DeltaSeconds -= Consumed;
			if (InlineFormulaRevealPhaseElapsed < CaptureSettleDuration)
			{
				break;
			}
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
			InlineFormulaRevealPhaseElapsed = 0.0f;
			continue;
		}

		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
		{
			const float HoldDuration = ActiveCrossRollReveal.Kind
					== EFMCodexUMGCrossRollRevealKind::InitialRoute
				|| ActiveCrossRollReveal.Kind
					== EFMCodexUMGCrossRollRevealKind
						::ThroughBallInitialRoute
					? RouteResultHoldDuration
					: ActiveCrossRollReveal.Kind
						== EFMCodexUMGCrossRollRevealKind::TacticalPoint
							? TacticalPointResultHoldDuration
							: FormulaResultHoldDuration;
			const float Remaining = FMath::Max(
				0.0f, HoldDuration - InlineFormulaRevealPhaseElapsed);
			const float Consumed = FMath::Min(DeltaSeconds, Remaining);
			InlineFormulaRevealPhaseElapsed += Consumed;
			DeltaSeconds -= Consumed;
			if (InlineFormulaRevealPhaseElapsed < HoldDuration)
			{
				break;
			}
			SettledCrossRollRevealKeys.Add(
				ActiveCrossRollReveal.StableKey());
			LastDisclosedInlineFormula = ActiveFormula(Presentation);
			ActiveCrossRollReveal = {};
			ObservedPendingCrossRoll = {};
			CachedResolvedInlineFormula = {};
			CachedPreRollHeader = {};
			bInlineFormulaAuthorityResultAvailable = false;
			RollRevealAuthoritativeRawValue = 0;
			RollRevealDomainMinimum = 1;
			RollRevealDomainMaximum = 6;
			RollRevealCaptureStartPositionCells = 0.0f;
			RollRevealCaptureDistanceCells = 0.0f;
			RollRevealSequenceOffsetCells = 0;
			CachedTacticalPointFinalValue = 0;
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Settled;
			InlineFormulaRevealPhaseElapsed = 0.0f;
			StopInlineFormulaRevealTimer();
			ObservePendingCrossRoll(Presentation);
		}
	}
	const bool bPhaseChanged = PreviousPhase != InlineFormulaRevealPhase;
	const bool bFormulaDisclosureChanged =
		PreviousPhase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold
		&& InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
		&& PreviousPhaseElapsed < FormulaDisclosureDelay
		&& InlineFormulaRevealPhaseElapsed >= FormulaDisclosureDelay;
	const bool bNarrativeDisclosureChanged =
		PreviousPhase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold
		&& InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold
		&& PreviousPhaseElapsed < NarrativeDisclosureDelay
		&& InlineFormulaRevealPhaseElapsed >= NarrativeDisclosureDelay;
	if (bForceFullRefresh || bPhaseChanged || bFormulaDisclosureChanged
		|| bNarrativeDisclosureChanged)
	{
		RefreshVisuals();
	}
	else if (IsPerFrameMotionPhase(InlineFormulaRevealPhase))
	{
		RefreshActiveRollReelVisuals();
	}
}

void UFMCodexLocalMatchScreenWidget::BeginInlineFormulaFinalCapture()
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (InlineFormulaRevealPhase
			!= EFMCodexUMGInlineFormulaRevealPhase::Cycling
		|| !bInlineFormulaAuthorityResultAvailable)
	{
		return;
	}
	RollRevealCaptureStartPositionCells = CosmeticReelPosition(
		InlineFormulaRevealPhaseElapsed);
	const int32 DomainCount = FMath::Max(
		1, RollRevealDomainMaximum - RollRevealDomainMinimum + 1);
	const int32 TargetIndex = WrapDomainValue(
		RollRevealAuthoritativeRawValue,
		RollRevealDomainMinimum,
		RollRevealDomainMaximum) - RollRevealDomainMinimum;
	int32 TargetStep = FMath::CeilToInt(
		RollRevealCaptureStartPositionCells);
	const int32 WrappedStep =
		(((TargetStep + RollRevealSequenceOffsetCells) % DomainCount)
			+ DomainCount)
		% DomainCount;
	TargetStep += (TargetIndex - WrappedStep + DomainCount) % DomainCount;
	if (TargetStep - RollRevealCaptureStartPositionCells < 0.35f)
	{
		TargetStep += DomainCount;
	}
	RollRevealCaptureDistanceCells =
		TargetStep - RollRevealCaptureStartPositionCells;
	InlineFormulaRevealPhase =
		EFMCodexUMGInlineFormulaRevealPhase::Settling;
	InlineFormulaRevealPhaseElapsed = 0.0f;
}

void UFMCodexLocalMatchScreenWidget::RefreshActiveRollReelVisuals()
{
	if (!ActiveCrossRollReveal.IsValid()
		|| !IsInlineFormulaRevealInputBlocked())
	{
		return;
	}
	const FFMCodexUMGRollReelViewModel Reel =
		BuildActiveRollReelPresentation();
	if (ActiveCrossRollReveal.Kind
		== EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		if (TacticalPointRollReel != nullptr)
		{
			TacticalPointRollReel->RefreshFromPresentation(Reel);
		}
	}
	else if (ActiveCrossRollReveal.Kind
			== EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute
		&& ThroughBallResolutionSurface != nullptr
		&& ThroughBallResolutionSurface->GetRollReelWidget() != nullptr)
	{
		ThroughBallResolutionSurface->GetRollReelWidget()
			->RefreshFromPresentation(Reel);
	}
	else if (Presentation.ThroughBallResolution.Formula.bVisible
		&& ThroughBallResolutionSurface != nullptr
		&& ThroughBallResolutionSurface->GetFormulaSurface() != nullptr
		&& ThroughBallResolutionSurface->GetFormulaSurface()
			->GetRollReelWidget() != nullptr)
	{
		ThroughBallResolutionSurface->GetFormulaSurface()->GetRollReelWidget()
			->RefreshFromPresentation(Reel);
	}
	else if (InlineFormulaSurface != nullptr
		&& InlineFormulaSurface->GetRollReelWidget() != nullptr)
	{
		InlineFormulaSurface->GetRollReelWidget()
			->RefreshFromPresentation(Reel);
	}
}

void UFMCodexLocalMatchScreenWidget::HandleInlineFormulaRevealTimer()
{
	using namespace FMCodexLocalMatchScreenWidget;
	InlineFormulaRevealTimerHandle.Invalidate();
	const UWorld* World = GetWorld();
	const float DeltaSeconds =
		IsPerFrameMotionPhase(InlineFormulaRevealPhase)
			? World != nullptr ? World->GetDeltaSeconds() : 0.0f
			: RevealHoldTickInterval;
	AdvanceInlineFormulaReveal(DeltaSeconds, false);
	StartInlineFormulaRevealTimer();
}

void UFMCodexLocalMatchScreenWidget::StartInlineFormulaRevealTimer()
{
	if (!IsInlineFormulaRevealInputBlocked())
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(
			InlineFormulaRevealTimerHandle))
		{
			if (FMCodexLocalMatchScreenWidget::IsPerFrameMotionPhase(
				InlineFormulaRevealPhase))
			{
				InlineFormulaRevealTimerHandle = World->GetTimerManager()
					.SetTimerForNextTick(
						this,
						&UFMCodexLocalMatchScreenWidget
							::HandleInlineFormulaRevealTimer);
			}
			else
			{
				World->GetTimerManager().SetTimer(
					InlineFormulaRevealTimerHandle,
					this,
					&UFMCodexLocalMatchScreenWidget
						::HandleInlineFormulaRevealTimer,
					FMCodexLocalMatchScreenWidget::RevealHoldTickInterval,
					false);
			}
		}
	}
}

void UFMCodexLocalMatchScreenWidget::StopInlineFormulaRevealTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InlineFormulaRevealTimerHandle);
	}
}

void UFMCodexLocalMatchScreenWidget::ResetInlineFormulaRevealState()
{
	StopInlineFormulaRevealTimer();
	CachedResolvedInlineFormula = {};
	LastDisclosedInlineFormula = {};
	InlineFormulaRevealPhase = EFMCodexUMGInlineFormulaRevealPhase::None;
	InlineFormulaRevealPhaseElapsed = 0.0f;
	RollRevealCaptureStartPositionCells = 0.0f;
	RollRevealCaptureDistanceCells = 0.0f;
	RollRevealSequenceOffsetCells = 0;
	ObservedPendingCrossRoll = {};
	ActiveCrossRollReveal = {};
	SettledCrossRollRevealKeys.Reset();
	bInlineFormulaAuthorityResultAvailable = false;
	RollRevealAuthoritativeRawValue = 0;
	RollRevealDomainMinimum = 1;
	RollRevealDomainMaximum = 6;
	CachedTacticalPointFinalValue = 0;
	CachedPreRollHeader = {};
}

FFMCodexCrossRollRevealIdentity
UFMCodexLocalMatchScreenWidget::PendingCrossRollIdentity(
	const FFMCodexUMGMatchScreenViewModel& InPresentation) const
{
	FFMCodexCrossRollRevealIdentity Result;
	Result.Kind = InPresentation.Interaction.CrossRollRevealKind;
	Result.AttackSequence = InPresentation.Header.AttackSequence;
	Result.ContestId = InPresentation.Interaction.CrossRollContestId;
	Result.RollSequenceIndex =
		InPresentation.Interaction.CrossRollSequenceIndex;
	Result.OwnerSide = InPresentation.Interaction.CrossRollOwnerSide;
	return Result;
}

void UFMCodexLocalMatchScreenWidget::ObservePendingCrossRoll(
	const FFMCodexUMGMatchScreenViewModel& InPresentation)
{
	const FFMCodexCrossRollRevealIdentity Candidate =
		PendingCrossRollIdentity(InPresentation);
	if (Candidate.IsValid()
		&& !SettledCrossRollRevealKeys.Contains(Candidate.StableKey()))
	{
		ObservedPendingCrossRoll = Candidate;
		InlineFormulaRevealPhase =
			EFMCodexUMGInlineFormulaRevealPhase::IdlePending;
	}
	else
	{
		ObservedPendingCrossRoll = {};
		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::IdlePending)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::None;
		}
	}
}

void UFMCodexLocalMatchScreenWidget::BeginInlineFormulaReveal(
	const FFMCodexCrossRollRevealIdentity& Identity,
	const bool bRequestInFlight)
{
	if (ActiveCrossRollReveal.IsValid()
		|| !Identity.IsValid()
		|| SettledCrossRollRevealKeys.Contains(Identity.StableKey()))
	{
		return;
	}
	ActiveCrossRollReveal = Identity;
	ObservedPendingCrossRoll = {};
	CachedResolvedInlineFormula = {};
	CachedPreRollHeader = Presentation.Header;
	bInlineFormulaAuthorityResultAvailable = false;
	RollRevealAuthoritativeRawValue = 0;
	RollRevealDomainMinimum = Identity.Kind
		== EFMCodexUMGCrossRollRevealKind::TacticalPoint ? 2 : 1;
	RollRevealDomainMaximum = Identity.Kind
		== EFMCodexUMGCrossRollRevealKind::TacticalPoint ? 8 : 6;
	CachedTacticalPointFinalValue = 0;
	InlineFormulaRevealPhase = bRequestInFlight
		? EFMCodexUMGInlineFormulaRevealPhase::RequestInFlight
		: EFMCodexUMGInlineFormulaRevealPhase::Cycling;
	InlineFormulaRevealPhaseElapsed = 0.0f;
	RollRevealCaptureStartPositionCells = 0.0f;
	RollRevealCaptureDistanceCells = 0.0f;
	RollRevealSequenceOffsetCells = 0;
	StartInlineFormulaRevealTimer();
	RefreshVisuals();
}

void UFMCodexLocalMatchScreenWidget::CancelInlineFormulaReveal()
{
	StopInlineFormulaRevealTimer();
	ActiveCrossRollReveal = {};
	CachedResolvedInlineFormula = {};
	CachedPreRollHeader = {};
	bInlineFormulaAuthorityResultAvailable = false;
	RollRevealAuthoritativeRawValue = 0;
	RollRevealDomainMinimum = 1;
	RollRevealDomainMaximum = 6;
	CachedTacticalPointFinalValue = 0;
	InlineFormulaRevealPhase = EFMCodexUMGInlineFormulaRevealPhase::None;
	InlineFormulaRevealPhaseElapsed = 0.0f;
	RollRevealCaptureStartPositionCells = 0.0f;
	RollRevealCaptureDistanceCells = 0.0f;
	RollRevealSequenceOffsetCells = 0;
}

bool UFMCodexLocalMatchScreenWidget::TryReadAuthoritativeRawRoll(
	const FFMCodexUMGMatchScreenViewModel& InPresentation,
	const FFMCodexCrossRollRevealIdentity& Identity,
	int32& OutRawValue,
	int32& OutDomainMinimum,
	int32& OutDomainMaximum) const
{
	using ERollPurpose = EMatchPlayCurrentAttackPostRouteRollPurpose;
	using ERollSemantics = EMatchPlayResolutionRollSemantics;
	if (!Identity.IsValid())
	{
		return false;
	}
	if (Identity.Kind == EFMCodexUMGCrossRollRevealKind::TacticalPoint)
	{
		const int32 TacticalPoints =
			InPresentation.Header.CurrentAttackerTacticalPoints;
		if (InPresentation.Header.AttackSequence != Identity.AttackSequence
			|| InPresentation.Interaction.bCanRollTacticalPoints
			|| TacticalPoints < 2 || TacticalPoints > 8)
		{
			return false;
		}
		// Current production's authoritative random object is directly 2..8;
		// BeginOrdinaryAttack stores that same value as the Tactical Point resource.
		OutRawValue = TacticalPoints;
		OutDomainMinimum = 2;
		OutDomainMaximum = 8;
		return true;
	}
	const FMatchPlayCurrentAttackResolutionFactProjection& Facts =
		InPresentation.Resolution.FormulaFacts;
	if (!Facts.bSuccess || !Facts.bHasFacts
		|| Facts.AttackSequence != Identity.AttackSequence)
	{
		return false;
	}

	if (Identity.Kind != EFMCodexUMGCrossRollRevealKind::InitialRoute
		&& Identity.Kind
			!= EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute
		&& !Facts.FormulaContests.ContainsByPredicate(
			[&Identity](
				const FMatchPlayResolutionFormulaContestFact& Contest)
			{
				return Contest.ContestId == Identity.ContestId;
			}))
	{
		return false;
	}

	const FMatchPlayResolutionRollFact* Roll = Facts.Rolls.FindByPredicate(
		[&Identity](const FMatchPlayResolutionRollFact& Candidate)
		{
			if (!Candidate.bResolved
				|| Candidate.SequenceIndex != Identity.RollSequenceIndex
				|| Candidate.OwningSide != Identity.OwnerSide)
			{
				return false;
			}
			switch (Identity.Kind)
			{
			case EFMCodexUMGCrossRollRevealKind::InitialRoute:
			case EFMCodexUMGCrossRollRevealKind::ThroughBallInitialRoute:
				return Candidate.bInitialRoute
					&& Candidate.Semantics
						== ERollSemantics::BranchSelection;
			case EFMCodexUMGCrossRollRevealKind::Attack:
				return Candidate.Semantics
						== ERollSemantics::ArithmeticContest
					&& Candidate.PostRoutePurpose
						== ERollPurpose::PrimaryAttack;
			case EFMCodexUMGCrossRollRevealKind::Defense:
				return Candidate.Semantics
						== ERollSemantics::ArithmeticContest
					&& Candidate.PostRoutePurpose
						== ERollPurpose::PrimaryDefense;
			default:
				return false;
			}
		});
	if (Roll == nullptr || Roll->RawD6 < 1 || Roll->RawD6 > 6)
	{
		return false;
	}
	OutRawValue = Roll->RawD6;
	OutDomainMinimum = 1;
	OutDomainMaximum = 6;
	return true;
}

void UFMCodexLocalMatchScreenWidget::RefreshVisuals()
{
	if (MatchHeader == nullptr)
	{
		return;
	}
	LocalRackBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::RackWidth);
	PitchBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::PitchWidth);
	OpponentRackBounds->SetWidthOverride(FMCodexHandMicroDiagnostics::RackWidth);
#if !UE_BUILD_SHIPPING
	if (HandMicroProductionReviewBounds != nullptr)
	{
		const int32 ReviewPage =
			FMCodexHandMicroDiagnostics::GetProductionReviewPage();
		if (HandMicroProductionReviewPortraitsPage != nullptr)
		{
			HandMicroProductionReviewPortraitsPage->SetVisibility(ReviewPage == 0
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroProductionReviewTypographyPage != nullptr)
		{
			HandMicroProductionReviewTypographyPage->SetVisibility(ReviewPage == 1
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		if (HandMicroProductionReviewLayoutPage != nullptr)
		{
			HandMicroProductionReviewLayoutPage->SetVisibility(ReviewPage == 2
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
		HandMicroProductionReviewBounds->SetHeightOverride(
			ReviewPage == 0 ? 660.0f : ReviewPage == 1 ? 430.0f : 830.0f);
		HandMicroProductionReviewBounds->SetVisibility(
			FMCodexHandMicroDiagnostics::IsProductionReviewEnabled()
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
	}
	RefreshFullCardProductionReviewSurface();
#endif
	MatchHeader->RefreshFromPresentation(BuildDisplayedHeader());
	LocalRackWidget->RefreshFromPresentation(Presentation.LocalRack);
	OpponentRackWidget->RefreshFromPresentation(Presentation.OpponentRack);
	PitchWidget->RefreshFromPitchPresentation(Presentation.PitchRegions);
	const FFMCodexUMGInlineFormulaSurfaceViewModel DisplayedInlineFormula =
		BuildDisplayedInlineFormula();
	InlineFormulaSurface->RefreshFromPresentation(
		Presentation.ThroughBallResolution.Formula.bVisible
			? Presentation.InlineFormula : DisplayedInlineFormula);
	const FFMCodexUMGThroughBallResolutionViewModel DisplayedThroughBall =
		BuildDisplayedThroughBallResolution();
	ThroughBallResolutionSurface->RefreshFromPresentation(DisplayedThroughBall);
	const bool bTacticalPointRevealVisible =
		ActiveCrossRollReveal.Kind
			== EFMCodexUMGCrossRollRevealKind::TacticalPoint
		&& IsInlineFormulaRevealInputBlocked();
	TacticalPointRevealSurface->SetVisibility(bTacticalPointRevealVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	if (bTacticalPointRevealVisible)
	{
		const FFMCodexUMGRollReelViewModel Reel =
			BuildActiveRollReelPresentation();
		TacticalPointRollReel->RefreshFromPresentation(Reel);
		const bool bHolding = InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
		const bool bSettling = InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Settling;
		TacticalPointRevealTitle->SetText(FText::FromString(
			bHolding ? TEXT("战术点结果")
				: bSettling ? TEXT("战术点掷点落定")
					: TEXT("战术点号码滚动中")));
		const bool bResourceDisclosed = bHolding
			&& InlineFormulaRevealPhaseElapsed
				>= FMCodexLocalMatchScreenWidget::FormulaDisclosureDelay;
		TacticalPointRevealResult->SetText(FText::FromString(
			bResourceDisclosed
				? FString::Printf(TEXT("掷点 %d  →  战术点 %d"),
					RollRevealAuthoritativeRawValue,
					CachedTacticalPointFinalValue)
				: bHolding
					? FString::Printf(TEXT("掷点 %d"),
						RollRevealAuthoritativeRawValue)
					: bSettling
						? FString(TEXT("掷点落定中"))
						: FString(TEXT("等待权威结果"))));
	}
	BindDetailHoverSources();
	InteractionPanel->RefreshFromPresentation(Presentation.Interaction);
	const bool bDeploymentContext = Presentation.Interaction.Category
		== EFMCodexUMGInteractionCategory::Deploy;
	if (bDeploymentTacticalReferenceOpen && !bDeploymentContext)
	{
		CloseDeploymentTacticalReference();
	}
	else if (Presentation.Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectSkill
		&& !bDeploymentTacticalReferenceOpen)
	{
		HideTacticalDetail();
	}
	const FFMCodexUMGPrimaryActionViewModel& PrimaryAction =
		Presentation.Interaction.PrimaryAction;
	const bool bCentralSurfaceClaimsPrimaryAction =
		DisplayedInlineFormula.PrimaryAction.Claims(PrimaryAction)
		|| DisplayedThroughBall.PrimaryAction.Claims(PrimaryAction)
		|| DisplayedThroughBall.Formula.PrimaryAction.Claims(PrimaryAction);
	InteractionPanel->SetVisibility(
		bCentralSurfaceClaimsPrimaryAction
			? ESlateVisibility::Collapsed
			: ESlateVisibility::Visible);
	InteractionPanel->SetInteractionBlocked(
		IsInlineFormulaRevealInputBlocked());
	ResolutionPanel->RefreshFromPresentation(Presentation.Resolution);
	ResolutionOverlay->SetVisibility(Presentation.Resolution.bVisible
		&& !DisplayedInlineFormula.bSuppressLegacyResolution
		&& !DisplayedThroughBall.bSuppressLegacyResolution
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

}
