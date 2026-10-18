#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexResolutionPanelWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

namespace
{
bool DisplayedAndEnabled(const UWidget* Widget)
{
	if (!Widget) return false;
	for (const UWidget* Current = Widget; Current; Current = Current->GetParent())
		if (!Current->IsVisible() || !Current->GetIsEnabled()) return false;
	return true;
}
UButton* Button(UUserWidget* Owner, const TCHAR* Name)
{
	if (!DisplayedAndEnabled(Owner)) return nullptr;
	auto* Found = Cast<UButton>(Owner->GetWidgetFromName(Name));
	return DisplayedAndEnabled(Found) ? Found : nullptr;
}
void HideResult(UUserWidget* Owner, const TCHAR* Name)
{
	if (Owner)
		if (auto* Widget = Owner->GetWidgetFromName(Name); Widget && Widget->IsVisible())
			Widget->SetVisibility(ESlateVisibility::Hidden); // Keep the existing CTA geometry.
}
}

UButton* UFMCodexLocalMatchScreenWidget::GetNextRoundButton() const
{
	if (TheaterMotion.bActive)
		return Button(const_cast<UFMCodexLocalMatchScreenWidget*>(this), TEXT("TheaterContinue"));
	if (DoesSetPieceOwnCurrentPrimaryAction())
		return DisplayedAndEnabled(SetPiecePrimaryButton) ? SetPiecePrimaryButton.Get() : nullptr;
	if (DoesThroughBallOwnCurrentPrimaryAction())
	{
		if (auto* Found = Button(ThroughBallResolutionSurface, TEXT("ThroughBallPrimaryActionButton"))) return Found;
		return Button(ThroughBallResolutionSurface->GetFormulaSurface(), TEXT("InlineFormulaContinueButton"));
	}
	if (DoesLongShotOwnCurrentPrimaryAction())
	{
		if (auto* Found = Button(LongShotResolutionSurface, TEXT("LongShotPrimaryActionButton"))) return Found;
		return Button(LongShotResolutionSurface->GetFormulaSurface(), TEXT("InlineFormulaContinueButton"));
	}
	if (DoesInlineFormulaOwnCurrentPrimaryAction())
		return Button(InlineFormulaSurface, TEXT("InlineFormulaContinueButton"));
	if (auto* Found = Button(ResolutionPanel, TEXT("ResolutionContinueButton"))) return Found;
	return Button(InteractionPanel, TEXT("InteractionContinueButton"));
}

bool UFMCodexLocalMatchScreenWidget::IsNextRoundReady() const
{
	using C = EFMCodexUMGInteractionCategory;
	return MatchBackend && !IsScreenRequestPending() && !bDeploymentTacticalReferenceOpen
		&& Presentation.Header.AttackSequence > 0
		&& Presentation.Interaction.Category == C::AdvanceAfterTerminal
		&& Presentation.Interaction.PrimaryAction.bAvailable
		&& Presentation.Interaction.PrimaryAction.Category == C::AdvanceAfterTerminal
		&& !Presentation.bActionWaitPromptReadOnly
		&& !Presentation.Header.bMatchEnded && !Presentation.Interaction.bMatchEnded && !Presentation.FullTime.bVisible
		&& !(Presentation.Resolution.bVisible && Presentation.Resolution.bNonBlockingNotification)
		&& !IsInlineFormulaRevealInputBlocked() && !TacticalScene.IsAnimating()
		&& !TacticalScene.Celebration.IsActive() && GetNextRoundButton();
}

void UFMCodexLocalMatchScreenWidget::AdvanceNextRoundCountdown(float DeltaSeconds)
{
	bool bReady = IsNextRoundReady();
#if !UE_BUILD_SHIPPING
	// Guided lessons retain their explicit learner-controlled continuation.
	if (MatchController && MatchController->GetGuidedLesson1()) bReady = false;
#endif
	if (MatchBackend)
	{
		auto& Dispatch = MatchBackend->NextRoundDispatch;
		Dispatch.Observe(Presentation.Header.AttackSequence);
		bReady &= Dispatch.Sequence == Presentation.Header.AttackSequence && !Dispatch.bSubmitted && !Dispatch.bAutoAttempted;
	}
	const bool bWasActive = NextRoundCountdown.Sequence != 0;
	const bool bExpired = NextRoundCountdown.Tick(bReady, Presentation.Header.AttackSequence,
		Presentation.Interaction.ExpectedActorLabel, Presentation.Header.CurrentPhaseLabel, DeltaSeconds);
	if (bExpired)
	{
		RequestContinueResolution(); // Same typed Continue -> AdvanceAfterTerminal path as a click.
		return;
	}
	if (bWasActive && !bReady && DeltaSeconds > 0.f)
		RefreshVisuals(); // Restore copy/narrative after cancellation, without rebuilding widgets.
	else ApplyNextRoundCountdown();
}

void UFMCodexLocalMatchScreenWidget::ApplyNextRoundCountdown()
{
	if (Presentation.Interaction.Category != EFMCodexUMGInteractionCategory::AdvanceAfterTerminal) return;
	if (auto* Continue = GetNextRoundButton())
		if (auto* Label = TheaterMotion.bActive ? Cast<UTextBlock>(GetWidgetFromName(TEXT("TheaterContinueLabel"))) : Cast<UTextBlock>(Continue->GetChildAt(0)))
		{
			const auto Text = NextRoundCountdown.bCounting
				? FText::Format(NSLOCTEXT("FMCodexNextRound", "Countdown", "下一回合（{0}）"), FText::AsNumber(NextRoundCountdown.Remaining()))
				: FText::FromString(Presentation.Interaction.PrimaryAction.Label);
			if (!Label->GetText().EqualTo(Text)) Label->SetText(Text);
			Label->SetJustification(ETextJustify::Center);
		}
	if (!NextRoundCountdown.bCounting && !TacticalScene.Celebration.IsActive()) return;
	// Retire only result narration, retaining the final arithmetic and action in place.
	for (const TCHAR* Name : {TEXT("CrossResultPlate"), TEXT("TheaterOutcome"), TEXT("TheaterOutcomeDivider")})
		HideResult(this, Name);
	HideResult(InlineFormulaSurface, TEXT("InlineFormulaOutcomeHeading"));
	HideResult(ThroughBallResolutionSurface, TEXT("ThroughBallOutcomeNarrative"));
	HideResult(LongShotResolutionSurface, TEXT("LongShotNarrative"));
	if (ThroughBallResolutionSurface) HideResult(ThroughBallResolutionSurface->GetFormulaSurface(), TEXT("InlineFormulaOutcomeHeading"));
	if (LongShotResolutionSurface) HideResult(LongShotResolutionSurface->GetFormulaSurface(), TEXT("InlineFormulaOutcomeHeading"));
	HideResult(ResolutionPanel, TEXT("ResolutionStepRegion"));
}
