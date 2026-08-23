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
#include "FMCodexSelectionFeedbackToastWidget.h"

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
	constexpr float RevealTickInterval = 0.05f;
	constexpr float AttackRevealDuration = 0.65f;
	constexpr float AttackSettledDuration = 0.28f;
	constexpr float DefenseRevealDuration = 0.65f;

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

	bool IsFullyResolved(
		const FFMCodexUMGInlineFormulaSurfaceViewModel& Surface)
	{
		const FFMCodexUMGInlineFormulaTermViewModel* AttackRoll =
			FindRawRoll(Surface.AttackRow);
		const FFMCodexUMGInlineFormulaTermViewModel* DefenseRoll =
			FindRawRoll(Surface.DefenseRow);
		return AttackRoll != nullptr && DefenseRoll != nullptr
			&& AttackRoll->bResolved && DefenseRoll->bResolved
			&& Surface.AttackRow.bFinalValueResolved
			&& Surface.DefenseRow.bFinalValueResolved;
	}

	void StageRowAsPending(
		FFMCodexUMGInlineFormulaRowViewModel& Row,
		const bool bActive)
	{
		if (FFMCodexUMGInlineFormulaTermViewModel* Roll = FindRawRoll(Row))
		{
			Roll->bResolved = false;
			Roll->RawD6 = 0;
			Roll->DisplayLabel = TEXT("掷点 ?");
			Roll->bNextPendingRoll = bActive;
		}
		Row.bFinalValueResolved = false;
		Row.FinalValue = 0.0f;
		Row.FinalValueLabel = TEXT("?");
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
	// Cross High now exposes each accepted roll as an authoritative state.
	// The former local dual-roll reveal must never delay or fabricate that flow.
	ResetInlineFormulaRevealState();
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
	return false;
}

#if WITH_DEV_AUTOMATION_TESTS
void UFMCodexLocalMatchScreenWidget::AdvanceInlineFormulaRevealForTesting(
	const float DeltaSeconds)
{
	AdvanceInlineFormulaReveal(DeltaSeconds);
}

void UFMCodexLocalMatchScreenWidget::PauseInlineFormulaRevealTimerForTesting()
{
	StopInlineFormulaRevealTimer();
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
	if (MatchController != nullptr)
	{
		MatchController->RollDemoTacticalPoints();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	if (MatchController != nullptr)
	{
		MatchController->DeployOrdinary(CardId, SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployGoalkeeper(
	const FName SlotId)
{
	if (MatchController != nullptr)
	{
		MatchController->DeployGoalkeeper(SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestFinishDeployment()
{
	if (MatchController != nullptr)
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
	if (MatchController != nullptr)
	{
		MatchController->DeclineCurrentSelection();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestResolveNoLegalSelection()
{
	if (MatchController != nullptr)
	{
		MatchController->ResolveNoLegalCurrentSelection();
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
	default:
		MatchController->ContinueResolution();
		break;
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
	InteractionPanel->OnCarrierRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleCarrierRequested);
	InteractionPanel->OnMarkerRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleMarkerRequested);
	InteractionPanel->OnSkillRequested.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleSkillRequested);
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
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Surface =
		InPresentation.InlineFormula;
	if (!Surface.bVisible || Surface.ContestId != TEXT("Cross.High"))
	{
		ResetInlineFormulaRevealState();
		return;
	}

	const FFMCodexUMGInlineFormulaTermViewModel* AttackRoll =
		FindRawRoll(Surface.AttackRow);
	const FFMCodexUMGInlineFormulaTermViewModel* DefenseRoll =
		FindRawRoll(Surface.DefenseRow);
	if (AttackRoll == nullptr || DefenseRoll == nullptr
		|| AttackRoll->RollSequenceIndex == INDEX_NONE
		|| DefenseRoll->RollSequenceIndex == INDEX_NONE
		|| Surface.AttackRow.Side == EInitialTurnOrderPlayer::None
		|| Surface.DefenseRow.Side == EInitialTurnOrderPlayer::None)
	{
		ResetInlineFormulaRevealState();
		return;
	}

	const bool bSameIdentity = IsSameInlineFormulaRevealIdentity(
		InPresentation.Header.AttackSequence,
		Surface.ContestId,
		AttackRoll->RollSequenceIndex,
		Surface.AttackRow.Side,
		DefenseRoll->RollSequenceIndex,
		Surface.DefenseRow.Side);
	const bool bResolved = IsFullyResolved(Surface);

	if (!bSameIdentity)
	{
		StopInlineFormulaRevealTimer();
		SetInlineFormulaRevealIdentity(
			InPresentation.Header.AttackSequence,
			Surface.ContestId,
			AttackRoll->RollSequenceIndex,
			Surface.AttackRow.Side,
			DefenseRoll->RollSequenceIndex,
			Surface.DefenseRow.Side);
		InlineFormulaRevealPhaseElapsed = 0.0f;
		if (bResolved)
		{
			// A late join / reconstructed screen must never replay old dice.
			CachedResolvedInlineFormula = Surface;
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Completed;
		}
		else
		{
			CachedResolvedInlineFormula = {};
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Pending;
		}
		return;
	}

	if (bResolved)
	{
		// Always cache the latest exact authority copy; never rebuild arithmetic.
		CachedResolvedInlineFormula = Surface;
		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::Pending)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::AttackReveal;
			InlineFormulaRevealPhaseElapsed = 0.0f;
			StartInlineFormulaRevealTimer();
		}
		else if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::None)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Completed;
		}
		return;
	}

	// A delayed/stale pending rebuild cannot replay or downgrade this identity.
	if (InlineFormulaRevealPhase
		== EFMCodexUMGInlineFormulaRevealPhase::Completed
		|| IsInlineFormulaRevealInputBlocked())
	{
		return;
	}
	InlineFormulaRevealPhase = EFMCodexUMGInlineFormulaRevealPhase::Pending;
	InlineFormulaRevealPhaseElapsed = 0.0f;
}

FFMCodexUMGInlineFormulaSurfaceViewModel
UFMCodexLocalMatchScreenWidget::BuildDisplayedInlineFormula() const
{
	return Presentation.InlineFormula;
}

void UFMCodexLocalMatchScreenWidget::AdvanceInlineFormulaReveal(
	float DeltaSeconds)
{
	using namespace FMCodexLocalMatchScreenWidget;
	DeltaSeconds = FMath::Max(0.0f, DeltaSeconds);
	while (DeltaSeconds > 0.0f && IsInlineFormulaRevealInputBlocked())
	{
		float Duration = 0.0f;
		switch (InlineFormulaRevealPhase)
		{
		case EFMCodexUMGInlineFormulaRevealPhase::AttackReveal:
			Duration = AttackRevealDuration;
			break;
		case EFMCodexUMGInlineFormulaRevealPhase::AttackSettled:
			Duration = AttackSettledDuration;
			break;
		case EFMCodexUMGInlineFormulaRevealPhase::DefenseReveal:
			Duration = DefenseRevealDuration;
			break;
		default:
			break;
		}
		const float Remaining = FMath::Max(
			0.0f, Duration - InlineFormulaRevealPhaseElapsed);
		if (DeltaSeconds < Remaining)
		{
			InlineFormulaRevealPhaseElapsed += DeltaSeconds;
			DeltaSeconds = 0.0f;
			break;
		}
		DeltaSeconds -= Remaining;
		InlineFormulaRevealPhaseElapsed = 0.0f;
		if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::AttackReveal)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::AttackSettled;
		}
		else if (InlineFormulaRevealPhase
			== EFMCodexUMGInlineFormulaRevealPhase::AttackSettled)
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::DefenseReveal;
		}
		else
		{
			InlineFormulaRevealPhase =
				EFMCodexUMGInlineFormulaRevealPhase::Completed;
			StopInlineFormulaRevealTimer();
		}
	}
	RefreshVisuals();
}

void UFMCodexLocalMatchScreenWidget::HandleInlineFormulaRevealTimer()
{
	AdvanceInlineFormulaReveal(
		FMCodexLocalMatchScreenWidget::RevealTickInterval);
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
			World->GetTimerManager().SetTimer(
				InlineFormulaRevealTimerHandle,
				this,
				&UFMCodexLocalMatchScreenWidget
					::HandleInlineFormulaRevealTimer,
				FMCodexLocalMatchScreenWidget::RevealTickInterval,
				true);
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
	InlineFormulaRevealPhase = EFMCodexUMGInlineFormulaRevealPhase::None;
	InlineFormulaRevealPhaseElapsed = 0.0f;
	bInlineFormulaRevealIdentityObserved = false;
	InlineFormulaRevealAttackSequence = 0;
	InlineFormulaRevealContestId = NAME_None;
	InlineFormulaRevealAttackRollSequenceIndex = INDEX_NONE;
	InlineFormulaRevealAttackSide = EInitialTurnOrderPlayer::None;
	InlineFormulaRevealDefenseRollSequenceIndex = INDEX_NONE;
	InlineFormulaRevealDefenseSide = EInitialTurnOrderPlayer::None;
}

bool UFMCodexLocalMatchScreenWidget::IsSameInlineFormulaRevealIdentity(
	const int64 AttackSequence,
	const FName ContestId,
	const int32 AttackRollSequenceIndex,
	const EInitialTurnOrderPlayer AttackSide,
	const int32 DefenseRollSequenceIndex,
	const EInitialTurnOrderPlayer DefenseSide) const
{
	return bInlineFormulaRevealIdentityObserved
		&& InlineFormulaRevealAttackSequence == AttackSequence
		&& InlineFormulaRevealContestId == ContestId
		&& InlineFormulaRevealAttackRollSequenceIndex
			== AttackRollSequenceIndex
		&& InlineFormulaRevealAttackSide == AttackSide
		&& InlineFormulaRevealDefenseRollSequenceIndex
			== DefenseRollSequenceIndex
		&& InlineFormulaRevealDefenseSide == DefenseSide;
}

void UFMCodexLocalMatchScreenWidget::SetInlineFormulaRevealIdentity(
	const int64 AttackSequence,
	const FName ContestId,
	const int32 AttackRollSequenceIndex,
	const EInitialTurnOrderPlayer AttackSide,
	const int32 DefenseRollSequenceIndex,
	const EInitialTurnOrderPlayer DefenseSide)
{
	bInlineFormulaRevealIdentityObserved = true;
	InlineFormulaRevealAttackSequence = AttackSequence;
	InlineFormulaRevealContestId = ContestId;
	InlineFormulaRevealAttackRollSequenceIndex = AttackRollSequenceIndex;
	InlineFormulaRevealAttackSide = AttackSide;
	InlineFormulaRevealDefenseRollSequenceIndex = DefenseRollSequenceIndex;
	InlineFormulaRevealDefenseSide = DefenseSide;
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
	MatchHeader->RefreshFromPresentation(Presentation.Header);
	LocalRackWidget->RefreshFromPresentation(Presentation.LocalRack);
	OpponentRackWidget->RefreshFromPresentation(Presentation.OpponentRack);
	PitchWidget->RefreshFromPitchPresentation(Presentation.PitchRegions);
	const FFMCodexUMGInlineFormulaSurfaceViewModel DisplayedInlineFormula =
		BuildDisplayedInlineFormula();
	InlineFormulaSurface->RefreshFromPresentation(DisplayedInlineFormula);
	BindDetailHoverSources();
	InteractionPanel->RefreshFromPresentation(Presentation.Interaction);
	InteractionPanel->SetInteractionBlocked(
		IsInlineFormulaRevealInputBlocked());
	ResolutionPanel->RefreshFromPresentation(Presentation.Resolution);
	ResolutionOverlay->SetVisibility(Presentation.Resolution.bVisible
		&& !DisplayedInlineFormula.bSuppressLegacyResolution
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

}
