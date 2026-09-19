#include "FMCodexRollReelWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexRollPresentationSurface.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// Focused access to cosmetic projection only; real PIE below uses typed commands.
struct FFMCodexRollCosmeticTestAccess
{
	static void Begin(UFMCodexLocalMatchScreenWidget& Screen, int32 Maximum, int32 Final, int64 Event = 17)
	{
		Screen.CancelInlineFormulaReveal();
		FFMCodexCrossRollRevealIdentity Identity;
		Identity.Kind = Maximum == 12 ? EFMCodexUMGCrossRollRevealKind::TacticalPoint
			: EFMCodexUMGCrossRollRevealKind::InitialRoute;
		Identity.AttackSequence = Event;
		Identity.OwnerSide = EInitialTurnOrderPlayer::PlayerA;
		Identity.ContestId = TEXT("RollCosmetic.Focus"); Identity.RollSequenceIndex = 0;
		Screen.BeginInlineFormulaReveal(Identity, false);
		Screen.PauseInlineFormulaRevealTimerForTesting();
		Screen.RollRevealDomainMinimum = 1; Screen.RollRevealDomainMaximum = Maximum;
		Screen.RollRevealAuthoritativeRawValue = Final;
		Screen.bInlineFormulaAuthorityResultAvailable = true;
	}
	static FFMCodexUMGRollReelViewModel At(UFMCodexLocalMatchScreenWidget& Screen, float Time)
	{
		Screen.InlineFormulaRevealPhaseElapsed = Time;
		return Screen.BuildActiveRollReelPresentation();
	}
	static void Capture(UFMCodexLocalMatchScreenWidget& Screen) { Screen.BeginInlineFormulaFinalCapture(); }
	static void SetAvailable(UFMCodexLocalMatchScreenWidget& Screen, bool bAvailable)
	{ Screen.bInlineFormulaAuthorityResultAvailable = bAvailable; }
	static void Refresh(UFMCodexLocalMatchScreenWidget& Screen, float Time)
	{ Screen.InlineFormulaRevealPhaseElapsed = Time; Screen.RefreshActiveRollReelVisuals(); }
	static void Hide(UFMCodexLocalMatchScreenWidget& Screen)
	{ Screen.CancelInlineFormulaReveal(); Screen.RefreshVisuals(); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollActivationContinuityTest,
	"FMCodex.LocalPlay.RollPresentation.ActivationContinuity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollActivationContinuityTest::RunTest(const FString&)
{
	using Access = FFMCodexRollCosmeticTestAccess;
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	Access::Begin(*Screen, 12, 9);
	auto* Frame = CastChecked<UFMCodexRollPresentationSurface>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface")));
	auto* Title = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealTitle"));
	auto* State = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealState"));
	auto* Detail = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult"));
	auto* Reel = Screen->GetTacticalPointRollReel();
	Access::Refresh(*Screen, 0);
	TestTrue(TEXT("Command feedback begins with the shell, before chamber and footer content"),
		Frame->GetRenderOpacity() > 0 && Frame->GetRenderOpacity() < 1
		&& Title->GetRenderOpacity() == 0 && Reel->GetRenderOpacity() == 0 && State->GetRenderOpacity() == 0);
	Access::Refresh(*Screen, .06f);
	TestTrue(TEXT("Title, chamber and status assemble in order without a second clock"),
		Title->GetRenderOpacity() > Reel->GetRenderOpacity() && Reel->GetRenderOpacity() > State->GetRenderOpacity()
		&& Reel->GetRenderOpacity() > 0 && State->GetRenderTransform().Translation.Y > 0);
	const float BeforeRefresh = Reel->GetRenderOpacity();
	Access::Refresh(*Screen, .06f);
	TestEqual(TEXT("Duplicate refresh cannot restart activation"), Reel->GetRenderOpacity(), BeforeRefresh);
	TestEqual(TEXT("Activation never changes the reveal phase"), Screen->GetInlineFormulaRevealPhase(), EFMCodexUMGInlineFormulaRevealPhase::Cycling);
	for (float Time : {.18f, .80f, 4.83f})
	{
		Access::Refresh(*Screen, Time);
		TestTrue(TEXT("Activation ends inside the existing fast segment and cannot replay during a late result"),
			Frame->GetActivationProgress() == 1 && Frame->GetRenderOpacity() == 1
			&& Reel->GetRenderOpacity() == 1 && State->GetRenderOpacity() == 1 && Detail->GetRenderOpacity() == 1
			&& Reel->GetRenderTransform().Scale.Equals(FVector2D(1))
			&& Reel->GetRenderTransform().Translation.IsNearlyZero() && State->GetRenderTransform().Translation.IsNearlyZero());
	}
	Access::Refresh(*Screen, .06f); Access::Hide(*Screen);
	TestTrue(TEXT("Interrupted entrance clears all transforms and activation before reuse"),
		Frame->GetVisibility() == ESlateVisibility::Collapsed && Frame->GetActivationProgress() == 1
		&& Frame->GetRenderTransform().Scale.Equals(FVector2D(1))
		&& Reel->GetRenderOpacity() == 1 && Reel->GetRenderTransform().Scale.Equals(FVector2D(1))
		&& Title->GetRenderTransform().Translation.IsNearlyZero() && Detail->GetRenderTransform().Translation.IsNearlyZero());
	Access::Begin(*Screen, 12, 6, 18); Access::Refresh(*Screen, 0);
	TestTrue(TEXT("A legitimate new event can assemble again without a prior number"),
		Frame->GetActivationProgress() == 0 && Reel->GetRenderOpacity() == 0 && !Reel->GetPresentation().bAuthoritativeValue);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollResultIndependentCyclingTest,
	"FMCodex.LocalPlay.RollPresentation.ResultIndependentCycling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollResultIndependentCyclingTest::RunTest(const FString&)
{
	using Access = FFMCodexRollCosmeticTestAccess;
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	auto SameStrip = [](const FFMCodexUMGRollReelViewModel& A, const FFMCodexUMGRollReelViewModel& B)
	{
		return A.PreviousValue == B.PreviousValue && A.CenterValue == B.CenterValue && A.NextValue == B.NextValue
			&& FMath::IsNearlyEqual(A.ContinuousPositionCells, B.ContinuousPositionCells)
			&& FMath::IsNearlyEqual(A.ScrollAlpha, B.ScrollAlpha);
	};
	for (int32 Maximum : {6, 12})
	{
		TArray<FFMCodexUMGRollReelViewModel> Baseline;
		for (int32 Final = 1; Final <= Maximum; ++Final)
		{
			Access::Begin(*Screen, Maximum, Final);
			int32 ConsecutiveSteps = 0, OrderedSteps = 0;
			FFMCodexUMGRollReelViewModel Previous;
			for (int32 Frame = 0; Frame < 260; ++Frame)
			{
				const auto Reel = Access::At(*Screen, Frame*.005f);
				if (Final == 1) Baseline.Add(Reel);
				else TestTrue(TEXT("Every pre-settling label/position is independent of the final result"), SameStrip(Baseline[Frame], Reel));
				TestTrue(TEXT("Shuffled labels stay within the consumer domain"),
					Reel.PreviousValue >= 1 && Reel.PreviousValue <= Maximum
					&& Reel.CenterValue >= 1 && Reel.CenterValue <= Maximum
					&& Reel.NextValue >= 1 && Reel.NextValue <= Maximum && !Reel.bAuthoritativeValue);
				if (Frame > 0 && FMath::FloorToInt(Reel.ContinuousPositionCells) != FMath::FloorToInt(Previous.ContinuousPositionCells))
				{
					TestEqual(TEXT("Cell crossings preserve the incoming digit instead of flashing unrelated labels"), Reel.CenterValue, Previous.NextValue);
					++ConsecutiveSteps;
					OrderedSteps += Reel.CenterValue == Previous.CenterValue % Maximum + 1;
				}
				Previous = Reel;
			}
			TestTrue(TEXT("Visible progression is not the old ordered count-up"), ConsecutiveSteps >= 4 && OrderedSteps < ConsecutiveSteps);
			const auto BeforeCapture = Access::At(*Screen, 1.30f);
			Access::Capture(*Screen);
			TestTrue(TEXT("Capture does not relabel any currently visible slot"), SameStrip(BeforeCapture, Access::At(*Screen, 0)));
			const auto Captured = Access::At(*Screen, .12f);
			TestTrue(TEXT("Every D6/D12 result lands exactly in the unchanged capture window"),
				Captured.CenterValue == Final && FMath::IsNearlyZero(Captured.ScrollAlpha));
			Screen->AdvanceInlineFormulaRevealForTesting(.04f);
			const auto Held = Access::At(*Screen, 0);
			TestTrue(TEXT("ResultHold exposes only the accepted final value"), Held.bStaticResult && Held.bAuthoritativeValue && Held.CenterValue == Final);
		}
		Access::Begin(*Screen, Maximum, 1, 18);
		bool bDifferentEvent = false;
		for (int32 Frame = 0; Frame < Baseline.Num(); ++Frame)
			bDifferentEvent |= !SameStrip(Baseline[Frame], Access::At(*Screen, Frame*.005f));
		TestTrue(TEXT("A legitimate new event receives a different deterministic cosmetic order"), bDifferentEvent);
		Access::Begin(*Screen, Maximum, Maximum);
		Access::SetAvailable(*Screen, false);
		const auto Waiting = Access::At(*Screen, 4.83f);
		Access::Capture(*Screen);
		TestEqual(TEXT("Missing authority never starts capture"), Screen->GetInlineFormulaRevealPhase(), EFMCodexUMGInlineFormulaRevealPhase::Cycling);
		Access::SetAvailable(*Screen, true); Access::Capture(*Screen);
		TestTrue(TEXT("Late authority keeps the current moving strip at capture entry"), SameStrip(Waiting, Access::At(*Screen, 0)));
		TestEqual(TEXT("Late capture lands on authority without a whole-domain chase"), Access::At(*Screen, .12f).CenterValue, Maximum);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollPresentationReuseTest,
	"FMCodex.LocalPlay.RollPresentation.ChamberReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollPresentationReuseTest::RunTest(const FString&)
{
	auto* Reel = NewObject<UFMCodexRollReelWidget>();
	Reel->TakeWidget();
	const UTextBlock* Center = Reel->GetCenterDigitWidget();
	auto* Chamber = Cast<UFMCodexRollPresentationSurface>(Reel->GetWidgetFromName(TEXT("RollReelClippedWindow")));
	if (!TestNotNull(TEXT("Shared chamber uses roll-only procedural frame"), Chamber)) return false;
	auto* Bounds = CastChecked<USizeBox>(Reel->GetWidgetFromName(TEXT("RollReelBounds")));
	TestTrue(TEXT("Compact consumers retain their 68x72 layout"), Bounds->GetWidthOverride() == 68 && Bounds->GetHeightOverride() == 72);
	for (int32 Maximum : {6, 12})
	{
		Reel->SetExpandedChamber(Maximum == 12);
		TestEqual(TEXT("Only the main modal expands its internal chamber"), Bounds->GetWidthOverride(), Maximum == 12 ? 96.0f : 68.0f);
		FFMCodexUMGRollReelViewModel Model;
		Model.bVisible = true; Model.bMoving = true; Model.bShowNeighborDigits = true;
		Model.DomainMinimum = 1; Model.DomainMaximum = Maximum;
		Model.PreviousValue = Maximum-1; Model.CenterValue = Maximum; Model.NextValue = 1;
		Model.ScrollAlpha = .35f;
		Reel->RefreshFromPresentation(Model);
		TestTrue(TEXT("Reused D6/D12 chamber is clipped and moving"),
			Reel->HasClippedWindow() && Reel->GetRenderedChildCount() == 3
			&& Reel->GetVisibleNeighborDigitCount() == 2 && Reel->GetCenterVerticalOffset() < 0);
		TestEqual(TEXT("No stale gold lock survives a new roll"), Chamber->GetLockEmphasis(), 0.0f);
		Model.bMoving = false; Model.bShowNeighborDigits = false;
		Model.bStaticResult = true; Model.bAuthoritativeValue = true; Model.bResultHold = true;
		Reel->RefreshFromPresentation(Model);
		TestTrue(TEXT("Final projected value retains the same stable center widget"),
			Reel->GetCenterDigitWidget() == Center && Reel->IsStaticResultTileVisible()
			&& Center->GetText().ToString() == FString::FromInt(Maximum)
			&& Reel->GetCenterRenderOpacity() == 1 && Reel->GetCenterRenderScale() == 1
			&& Reel->GetCenterVerticalOffset() == 0);
		TestEqual(TEXT("Result activates the gold lock line"), Chamber->GetLockEmphasis(), 1.0f);
		Reel->RefreshFromPresentation({});
		TestEqual(TEXT("Hide clears lock decoration before reuse"), Chamber->GetLockEmphasis(), 0.0f);
		TestTrue(TEXT("Hidden resets text, neighbors, opacity and transform before reuse"),
			Reel->GetVisibility() == ESlateVisibility::Collapsed && Center->GetText().IsEmpty()
			&& Reel->GetVisibleNeighborDigitCount() == 0 && Reel->GetCenterRenderOpacity() == 0
			&& Reel->GetCenterVerticalOffset() == 0 && Reel->GetCenterRenderScale() == 1);
	}
	return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"

namespace
{
class FStartRollPresentationPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		// Per-session copy: never persist or replace the user's editor settings.
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(
			GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(Params);
		return true;
	}
};

class FRollPresentationPIEPath final : public IAutomationLatentCommand
{
public:
	explicit FRollPresentationPIEPath(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds()-Started > 35)
		{
			Test->AddError(TEXT("Roll presentation PIE path timed out")); return true;
		}
		auto* Controller = GEditor && GEditor->PlayWorld
			? Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController()) : nullptr;
		auto* Screen = Controller ? Controller->GetPlayerMatchScreen() : nullptr;
		if (!Screen) return false;
		if (!bStartedMatch)
		{
			Screen->RequestStartNewMatch();
			bStartedMatch = true; MatchStarted = FPlatformTime::Seconds();
			return false;
		}
		// Let the actual newly constructed match layout render before the click.
		if (FPlatformTime::Seconds()-MatchStarted < 1.0) return false;
		if (!bStartedRoll)
		{
			FFMCodexLocalDevRollOverrideRequest Override;
			Override.Target = EFMCodexLocalDevRollTarget::FullD12;
			Override.Value = 9;
			if (!Controller->SetLocalDevRollOverride(Override).bSuccess)
			{ Test->AddError(TEXT("Existing DEV D12 provider seam rejected override")); return true; }
			Test->TestFalse(TEXT("Restart clears prior reveal"), Screen->IsInlineFormulaRevealInputBlocked());
			Screen->RequestRollTacticalPoints();
			if (!Controller->GetLastDiagnostic().bHostSuccess)
			{ Test->AddError(TEXT("PIE typed Full D12 request failed")); return true; }
			auto* EntryFrame = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"));
			Test->TestTrue(TEXT("Real Enter uses the current Cycling clock"), EntryFrame && EntryFrame->GetRenderOpacity() < 1);
			bStartedRoll = true; RollStarted = FPlatformTime::Seconds();
			RollGameStarted = GEditor->PlayWorld->GetTimeSeconds();
			return false;
		}
		auto* Reel = bTypeRoll ? Screen->GetInlineFormulaSurface()->GetRollReelWidget() : Screen->GetTacticalPointRollReel();
		auto* Frame = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"));
		if (!Reel || !Frame) { Test->AddError(TEXT("PIE modal missing")); return true; }
		const auto Phase = Screen->GetInlineFormulaRevealPhase();
		const int32 PhaseIndex = static_cast<int32>(Phase);
		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling)
		{
			const int32 Cell = FMath::FloorToInt(Reel->GetPresentation().ContinuousPositionCells);
			if (Cell != LastCell)
			{
				Test->AddInfo(FString::Printf(TEXT("ROLL_VISIBLE domain=%s cell=%d previous=%d center=%d next=%d"),
					bTypeRoll ? TEXT("D6") : TEXT("D12"), Cell, Reel->GetPresentation().PreviousValue,
					Reel->GetPresentation().CenterValue, Reel->GetPresentation().NextValue));
				LastCell = Cell;
			}
		}
		if (PhaseIndex != LastPhase)
		{
			Test->AddInfo(FString::Printf(TEXT("ROLL_PIE domain=%s phase=%d wall=%.3f game=%.3f center=%d scale=%.3f opacity=%.3f"),
				bTypeRoll ? TEXT("D6") : TEXT("D12"), PhaseIndex, FPlatformTime::Seconds()-RollStarted, GEditor->PlayWorld->GetTimeSeconds()-RollGameStarted,
				Reel->GetPresentation().CenterValue, Reel->GetCenterRenderScale(), Frame->GetRenderOpacity()));
			LastPhase = PhaseIndex;
		}
		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling)
		{
			bCycling = true;
			if (!bTypeRoll && !bEntryShot && GEditor->PlayWorld->GetTimeSeconds()-RollGameStarted < .18f)
			{
				Capture(TEXT("Activation")); bEntryShot = true;
			}
			if (!bCycleShot && FPlatformTime::Seconds()-RollStarted > .55)
			{
				Capture(bTypeRoll ? TEXT("D6_Cycling") : TEXT("Cycling"));
				bCycleShot = true;
			}
		}
		else if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Settling)
		{
			if (!bTypeRoll && !bSettling) Capture(TEXT("Settling"));
			bSettling = true;
			Test->TestTrue(TEXT("Modal remains visible through real settling"), bTypeRoll || Frame->GetVisibility() != ESlateVisibility::Collapsed);
		}
		else if (Phase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
		{
			if (!bHeld)
			{
				Test->TestTrue(TEXT("PIE result is the provider-accepted value, with no moving neighbors"),
					Reel->IsStaticResultTileVisible() && Reel->GetPresentation().bAuthoritativeValue
					&& Reel->GetPresentation().CenterValue == (bTypeRoll ? 5 : 9));
				HoldStarted = GEditor->PlayWorld->GetTimeSeconds();
				bHeld = true;
			}
			if (!bResultShot && GEditor->PlayWorld->GetTimeSeconds()-HoldStarted > .25f)
			{
				Capture(bTypeRoll ? TEXT("D6_Result") : TEXT("Result")); bResultShot = true;
				if (!bTypeRoll)
				{
					auto* Footer = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult")));
					auto* State = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealState")));
					Test->TestTrue(TEXT("Runtime result module separates status from the disclosed business mapping"),
						Footer && Footer->GetFont().Size == 16 && Footer->GetText().ToString().Contains(TEXT("定位球"))
						&& State && State->GetFont().Size == 20 && State->GetFont().TypefaceFontName == TEXT("Bold"));
				}
			}
		}
		else if (!Screen->IsInlineFormulaRevealInputBlocked())
		{
			Test->TestTrue(TEXT("Natural PIE clock visited Cycling, Settling and ResultHold"), bCycling && bSettling && bHeld);
			if (bTypeRoll)
			{
				Test->TestTrue(TEXT("Legitimate successor D6 exits with no prior digit or lock state"),
					Reel->GetCenterDigitWidget()->GetText().IsEmpty()
					&& Reel->GetCenterRenderOpacity() == 0 && Reel->GetCenterRenderScale() == 1);
				return true;
			}
			Test->TestTrue(TEXT("Exit clears the modal and prior number"),
				Frame->GetVisibility() == ESlateVisibility::Collapsed && Reel->GetCenterDigitWidget()->GetText().IsEmpty()
				&& Frame->GetRenderOpacity() == 1 && Frame->GetRenderTransform().Scale.Equals(FVector2D(1)));
			// A new typed event in the same match, not the known same-sequence restart.
			FFMCodexLocalDevRollOverrideRequest Override;
			Override.Target = EFMCodexLocalDevRollTarget::SetPieceType; Override.Value = 5;
			if (!Controller->SetLocalDevRollOverride(Override).bSuccess)
			{ Test->AddError(TEXT("DEV type D6 seam rejected override")); return true; }
			Screen->RequestContinueResolution();
			if (!Controller->GetLastDiagnostic().bHostSuccess)
			{ Test->AddError(TEXT("Legitimate typed Set Piece successor failed")); return true; }
			bTypeRoll = true; bCycling = bSettling = bHeld = bCycleShot = bResultShot = false; LastPhase = LastCell = -1;
			RollStarted = FPlatformTime::Seconds(); RollGameStarted = GEditor->PlayWorld->GetTimeSeconds();
		}
		return false;
	}
private:
	void Capture(const TCHAR* State)
	{
		const FString Directory = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_6D"));
		IFileManager::Get().MakeDirectory(*Directory, true);
		// Read the current Slate state synchronously: a queued screenshot can miss
		// the 0.16s settling window. No clock pause or synthetic presentation state.
		auto Window = GEditor->PlayWorld->GetGameViewport()->GetWindow();
		TArray<FColor> Pixels;
		FIntVector Size = FIntVector::ZeroValue;
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size))
		{ Test->AddError(TEXT("Current-phase Slate capture failed")); return; }
		TArray64<uint8> PNG;
		FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
		const FString Path = Directory/FString::Printf(TEXT("RollPresentation_%s.png"), State);
		Test->TestTrue(TEXT("Current-phase runtime screenshot saved"), FFileHelper::SaveArrayToFile(PNG, *Path));
		Test->AddInfo(FString::Printf(TEXT("ROLL_CAPTURE %s %dx%d game=%.3f"), State, Size.X, Size.Y,
			GEditor->PlayWorld->GetTimeSeconds()-RollGameStarted));
	}
	FAutomationTestBase* Test;
	double Started = FPlatformTime::Seconds(), RollStarted = 0, MatchStarted = 0;
	double RollGameStarted = 0, HoldStarted = 0;
	int32 LastPhase = -1, LastCell = -1;
	bool bTypeRoll = false, bResultShot = false;
	bool bStartedMatch = false, bEntryShot = false;
	bool bStartedRoll = false, bCycling = false, bSettling = false, bHeld = false, bCycleShot = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollPresentationPIETest,
	"FMCodex.PIE.RollPresentation.ProductionCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexRollPresentationPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartRollPresentationPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FRollPresentationPIEPath(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
