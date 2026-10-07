#include "FMCodexLocalMatchPlayerController.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexGuidedLesson1Focus.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FMCodexGuidedLesson1Entry"
namespace
{
FAutoConsoleCommandWithWorld StartLesson(TEXT("fm.Tutorial.Lesson1"), TEXT("Start/reset Guided Match Lesson 1 in Local PIE/Development."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (auto* PC = World ? Cast<AFMCodexLocalMatchPlayerController>(World->GetFirstPlayerController()) : nullptr)
			PC->StartGuidedLesson1();
		else UE_LOG(LogTemp, Warning, TEXT("fm.Tutorial.Lesson1 requires the LocalMatchHostGameMode and Local player controller."));
	}));
FAutoConsoleCommandWithWorld ExitLesson(TEXT("fm.Tutorial.Exit"), TEXT("Exit Guided Match and start a fresh normal Local match."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		if (auto* PC = World ? Cast<AFMCodexLocalMatchPlayerController>(World->GetFirstPlayerController()) : nullptr)
			PC->ExitGuidedLesson1();
	}));
}

FFMCodexGuidedLesson1* AFMCodexLocalMatchPlayerController::GetGuidedLesson1() const
{
	const auto* Host = FindLocalMatchHost();
	return Host ? Host->GetGuidedLesson1() : nullptr;
}
void AFMCodexLocalMatchPlayerController::RemoveGuidedLesson1Overlay()
{
	if (GetWorld()) GetWorld()->GetTimerManager().ClearTimer(GuidedLesson1Timer);
	if (GuidedLesson1Overlay && GetWorld() && GetWorld()->GetGameViewport())
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(GuidedLesson1Overlay.ToSharedRef());
	GuidedLesson1Overlay.Reset();
	if (DevRollOverrideViewportWidget) DevRollOverrideViewportWidget->SetVisibility(EVisibility::SelfHitTestInvisible);
}
void AFMCodexLocalMatchPlayerController::ResetGuidedLesson1Presentation()
{
	if (auto* L = GetGuidedLesson1()) L->CancelExitConfirmation();
	CancelRecoveryNotificationDismiss(); ResetSetPieceDraft();
	ResolutionFeedback = {}; LastDiagnostic = {};
	// Replace the presentation session, including hover, drag, pending and reveal caches.
	if (PlayerMatchScreen)
	{
		PlayerMatchScreen->ResetPresentationSession();
		PlayerMatchScreen->ClearMatchController();
		PlayerMatchScreen->RemoveFromParent(); PlayerMatchScreen = nullptr;
	}
	RefreshPresentation(); InitializePlayerFacingUI();
}
void AFMCodexLocalMatchPlayerController::StartGuidedLesson1()
{
	auto* Host = FindLocalMatchHost();
	if (!Host || !Host->StartGuidedLesson1()) return;
	RemoveGuidedLesson1Overlay(); ResetGuidedLesson1Presentation();
	if (DevRollOverrideViewportWidget) DevRollOverrideViewportWidget->SetVisibility(EVisibility::Collapsed);
	GuidedLesson1Overlay = FMCodexLesson1Focus::Build(this);
	if (GetWorld()->GetGameViewport()) GetWorld()->GetGameViewport()->AddViewportWidgetContent(GuidedLesson1Overlay.ToSharedRef(), 160);
	GuidedLesson1LastTime = GetWorld()->GetTimeSeconds();
	GetWorld()->GetTimerManager().SetTimer(GuidedLesson1Timer, this,
		&AFMCodexLocalMatchPlayerController::TickGuidedLesson1, .1f, true);
}
void AFMCodexLocalMatchPlayerController::GuidedLesson1Primary()
{
	auto* L = GetGuidedLesson1();
	if (!L || L->IsExitConfirmationOpen() || (PlayerMatchScreen && !FFMCodexGuidedLesson1::CanProgress(InteractionView, PlayerMatchScreen->GetTacticalScene(), PlayerMatchScreen->IsInlineFormulaRevealInputBlocked()))) return;
	if (L->GetStep() == EFMCodexLesson1Step::Complete) { ExitGuidedLesson1(); return; }
	L->Primary(); RefreshPlayerMatchScreen();
}
void AFMCodexLocalMatchPlayerController::ExitGuidedLesson1()
{
	if (!GetGuidedLesson1()) return;
	RemoveGuidedLesson1Overlay();
	StartNewDemoMatch(); ResetGuidedLesson1Presentation();
}
void AFMCodexLocalMatchPlayerController::TickGuidedLesson1()
{
	auto* L = GetGuidedLesson1();
	if (!L || !PlayerMatchScreen) { RemoveGuidedLesson1Overlay(); return; }
	const float Now = GetWorld()->GetTimeSeconds();
	const bool bReady = FFMCodexGuidedLesson1::CanProgress(InteractionView, PlayerMatchScreen->GetTacticalScene(), PlayerMatchScreen->IsInlineFormulaRevealInputBlocked());
	const auto* Detail = PlayerMatchScreen->GetDetailOverlayCard();
	const bool bInspected = bReady && Detail && L->InspectCard(Detail->GetPresentation().CardId,
		PlayerMatchScreen->IsDetailOverlayVisible() && Detail->FindAttributePresentationWidget(TEXT("SHO")) && Detail->FindSkillPresentationWidget(L->Skill())
		&& (!L->IsComparison() || Detail->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier"))));
	bool bChanged = L->Update(InteractionView, bReady, Now-GuidedLesson1LastTime) || bInspected;
	if (bReady && L->GetStep()==EFMCodexLesson1Step::FormulaHover
		&& PlayerMatchScreen->GetTacticalScene().Phase==FMCodexTacticalScene::EPhase::FormulaHold)
	{
		if (auto* Value=PlayerMatchScreen->GetWidgetFromName(TEXT("TheaterAttackBaseHover")); Value && Value->IsHovered() && Value->GetToolTip())
		{
			const auto Tip=Value->GetToolTip()->GetCachedWidget();
			const auto Window=Tip.IsValid()?FSlateApplication::Get().FindWidgetWindow(Tip.ToSharedRef()):TSharedPtr<SWindow>();
			bChanged |= L->InspectFormula(Window.IsValid() && Window->IsVisible());
		}
	}
	GuidedLesson1LastTime = Now;
	if (L->IsCheckpointDue())
	{
		if (FindLocalMatchHost()->RebuildLesson1DeploymentCheckpoint()) ResetGuidedLesson1Presentation();
		return;
	}
	if (bChanged) RefreshPlayerMatchScreen();
	if (!bReady || !L->IsOpponentActionDue()) return;
	const auto BeforeCategory = InteractionView.InteractionCategory;
	const auto BeforePlacements = InteractionView.DeploymentPlacements.Num();
	const auto Action = L->OpponentAction();
	TGuardValue<bool> ScriptScope(L->bDispatchingOpponent, true);
	switch (Action.Kind)
	{
	case EFMCodexMatchScreenIntent::DeployOrdinary: PlayerMatchScreen->RequestDeployOrdinary(Action.OptionId, Action.SlotId); break;
	case EFMCodexMatchScreenIntent::FinishDeployment: PlayerMatchScreen->RequestFinishDeployment(); break;
	case EFMCodexMatchScreenIntent::Marker: PlayerMatchScreen->RequestSubmitMarker(Action.OptionId); break;
	case EFMCodexMatchScreenIntent::Continue: PlayerMatchScreen->RequestContinueResolution(); break;
	default: break;
	}
	// Deployment waits for the lesson proxy's painted arrival. Only this original
	// production command places a card; visual arrival never fabricates board state.
	if (BeforeCategory != InteractionView.InteractionCategory || BeforePlacements != InteractionView.DeploymentPlacements.Num()
		|| (Action.Kind == EFMCodexMatchScreenIntent::FinishDeployment && InteractionView.bPlayerBDeploymentFinished))
	{
		UE_LOG(LogTemp,Display,TEXT("LESSON1_OPPONENT accepted gesture=%d placements=%d->%d"),static_cast<int32>(Action.Kind),BeforePlacements,InteractionView.DeploymentPlacements.Num());
		L->OpponentActionSubmitted(); RefreshPlayerMatchScreen();
	}
}
#undef LOCTEXT_NAMESPACE
#endif
