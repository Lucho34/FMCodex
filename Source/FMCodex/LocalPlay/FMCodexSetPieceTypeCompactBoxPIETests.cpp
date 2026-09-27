#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexRollReelWidget.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"

namespace
{
class FStartTypeCompactBoxPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams Params; Params.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(Params);
		return true;
	}
};

// Typed commands and the existing server-owned DEV provider only. The production
// timer advances naturally; this driver never advances or pauses reveal time.
class FTypeCompactBoxPIE final : public IAutomationLatentCommand
{
public:
	explicit FTypeCompactBoxPIE(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds() - Started > 90)
		{ Test->AddError(TEXT("Type CompactBox PIE timed out")); return true; }
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* Controller = Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* Screen = Controller ? Controller->GetPlayerMatchScreen() : nullptr;
		if (!Screen) return false;
		const float Now = GEditor->PlayWorld->GetTimeSeconds();
		if (Step == 0)
		{
			Screen->RequestStartNewMatch();
			if (!Override(*Controller, EFMCodexLocalDevRollTarget::FullD12, 9)) return true;
			Screen->RequestRollTacticalPoints(); Step = 1; return false;
		}
		if (Step == 1)
		{
			if (Screen->IsInlineFormulaRevealInputBlocked())
			{
				CheckRollColor(*Screen->GetTacticalPointRollReel());
				if (Screen->GetTacticalPointRollReel()->GetPresentation().bStaticResult) bHeroLanded = true;
				Test->TestEqual(TEXT("Hero explanatory text keeps its neutral color"),
					CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult")))->GetColorAndOpacity().GetSpecifiedColor(), FLinearColor(.76f,.85f,.93f,1));
				return false;
			}
			if (!Test->TestEqual(TEXT("Full D12 naturally reaches the type request"),
				Controller->GetInteractionView().InteractionCategory, EFMCodexLocalMatchInteractionCategory::RollSetPieceType)) return true;
			Test->TestTrue(TEXT("Existing Local read-only reference remains"),
				Screen->GetInlineFormulaSurface()->GetPresentation().TacticalPlayerSummaryLabel.Contains(TEXT("1–2：角球")));
			if (!Override(*Controller, EFMCodexLocalDevRollTarget::SetPieceType, 5)) return true;
			Screen->RequestContinueResolution(); TypeStart = Now; Step = 2; return false;
		}
		auto* Surface = Screen->GetInlineFormulaSurface();
		const auto& P = Surface->GetPresentation();
		if (Screen->IsInlineFormulaRevealInputBlocked())
		{
			CheckRollColor(*Surface->GetRollReelWidget());
			Test->TestEqual(TEXT("Actual Type consumer is CompactBox"), Surface->GetRollReelWidget()->GetVisualVariant(), EFMCodexRollVisualVariant::CompactBox);
			Test->TestFalse(TEXT("No next-action CTA during Type reveal"), P.PrimaryAction.bVisible);
			const auto* EarlyTaker = Screen->GetWidgetFromName(TEXT("TheaterTakerBounds"));
			Test->TestTrue(TEXT("Theater cannot take over early"), !EarlyTaker || EarlyTaker->GetVisibility() == ESlateVisibility::Collapsed);
			const auto Phase = Screen->GetInlineFormulaRevealPhase();
			if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling) bCycling = true;
			if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Settling)
			{
				// Legacy would still cycle until 1.30 seconds.
				if (Now - TypeStart < 1.25f) bModernCapture = true;
				Test->TestEqual(TEXT("Modern capture has no legacy scale bounce"), P.RollReel.LandingScale, 1.f);
			}
			if (Phase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
			{
				if (HoldStart < 0) HoldStart = Now;
				Test->TestTrue(TEXT("Authoritative five lands in the original motion path"), P.RollReel.bStaticResult && P.RollReel.CenterValue == 5);
				if (!P.RouteResultLabel.IsEmpty() && !bDisclosed)
				{
					Test->TestEqual(TEXT("Type text comes from canonical result"), P.RouteResultLabel, FString(TEXT("掷点 5 → 近距离任意球")));
					const auto ResultColor = CastChecked<UTextBlock>(Surface->GetWidgetFromName(TEXT("InlineFormulaStatus")))->GetColorAndOpacity().GetSpecifiedColor();
					Test->TestTrue(TEXT("Type result text retains its neutral treatment"), ResultColor.B > ResultColor.R);
					bDisclosed = true; DisclosedAt = Now;
					Controller->RefreshPresentation();
					Test->TestEqual(TEXT("Refresh does not restart the real reveal"), Screen->GetInlineFormulaRevealPhase(), Phase);
					Capture();
				}
			}
			return false;
		}
		Test->TestTrue(TEXT("Real PIE observed cycling, modern capture, landed value and disclosed hold"), bCycling && bModernCapture && bDisclosed);
		Test->TestTrue(TEXT("Same PIE also observes Hero authoritative landed accent"), bHeroLanded);
		// Sampling can miss one timer tick at either edge; the focused test checks
		// the exact .18 + 2.40 boundary without screenshot/readback latency.
		Test->TestTrue(TEXT("Natural clock preserves readable ResultHold"), HoldStart >= 0 && Now - HoldStart >= 2.45f && Now - DisclosedAt >= 2.25f);
		const auto* Taker = Screen->GetWidgetFromName(TEXT("TheaterTakerBounds"));
		Test->TestTrue(TEXT("Near FK Theater takes over after Type hold"), Taker && Taker->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
		Test->TestEqual(TEXT("Canonical next action remains taker selection"), Controller->GetInteractionView().InteractionCategory, EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier);
		Controller->RefreshPresentation();
		Test->TestFalse(TEXT("Completed Type cannot replay or leave a stale reel"), Screen->IsInlineFormulaRevealInputBlocked() || Surface->GetPresentation().bDiceRevealVisible);
		Test->AddInfo(FString::Printf(TEXT("TYPE_COMPACTBOX_PIE world=%s modernCapture=%d value=5 holdObserved=%.3fs readableObserved=%.3fs handoff=NearFK"),
			*GEditor->PlayWorld->GetPathName(), bModernCapture, Now - HoldStart, Now - DisclosedAt));
		return true;
	}
private:
	void CheckRollColor(const UFMCodexRollReelWidget& Reel)
	{
		const auto Color = Reel.GetCenterDigitWidget()->GetColorAndOpacity().GetSpecifiedColor();
		if (Reel.GetPresentation().bStaticResult && Reel.GetPresentation().bAuthoritativeValue)
			Test->TestEqual(TEXT("Real authoritative landed glyph is exact EED7A6"), Color, FLinearColor::FromSRGBColor(FColor(238,215,166)));
		else
			Test->TestTrue(TEXT("Real pre-landed glyph stays neutral, never warm"), Color.B > Color.R);
	}
	bool Override(AFMCodexLocalMatchPlayerController& Controller, EFMCodexLocalDevRollTarget Target, int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request; Request.Target = Target; Request.Value = Value;
		return Test->TestTrue(TEXT("Existing DEV provider accepts the requested roll"), Controller.SetLocalDevRollOverride(Request).bSuccess);
	}
	void Capture()
	{
		auto Window = GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Window.IsValid()) { Test->AddError(TEXT("No real PIE window")); return; }
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		if (!FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(), Pixels, Size))
		{ Test->AddError(TEXT("PIE screenshot failed")); return; }
		const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Stage8_16_1"));
		IFileManager::Get().MakeDirectory(*Dir, true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNG);
		Test->TestTrue(TEXT("Live Type ResultHold evidence saved"), FFileHelper::SaveArrayToFile(PNG, *(Dir / TEXT("TypeCompactBoxHold.png"))));
	}
	FAutomationTestBase* Test;
	double Started = FPlatformTime::Seconds();
	int32 Step = 0;
	float TypeStart = 0, HoldStart = -1, DisclosedAt = 0;
	bool bCycling = false, bModernCapture = false, bDisclosed = false, bHeroLanded = false;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexTypeCompactBoxPIETest,
	"FMCodex.PIE.SetPieceType.CompactBoxNearFreeKick",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexTypeCompactBoxPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTypeCompactBoxPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FTypeCompactBoxPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
