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
#include "Widgets/Layout/SBox.h"

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "FMCodexTacticalResolutionNarrativePresentation.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "FMCodexOutcomePresentation.h"
#include "FMCodexMatchFlowPanel.h"
#include "Misc/AutomationTest.h"

namespace
{
class FStartOutcomeFamilyPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams P; P.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(P);
		return true;
	}
};


class FOutcomeFamilyPIE final : public IAutomationLatentCommand
{
public:
 explicit FOutcomeFamilyPIE(FAutomationTestBase* InTest) : Test(InTest) {}
 virtual bool Update() override
 {
  if (FPlatformTime::Seconds()-Started > 90) { Test->AddError(TEXT("Outcome PIE timed out")); return true; }
  if (!GEditor || !GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C ? C->GetPlayerMatchScreen() : nullptr;
  if (!S || FPlatformTime::Seconds()-Changed < .8) return false;
  if (Step==0) { S->RequestStartNewMatch(); Advance(); return false; }
  if (Step==1) { if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,9)) return true; S->RequestRollTacticalPoints(); Advance(); return false; }
  if (S->IsInlineFormulaRevealInputBlocked())
  {
   if (Step==6)
   {
    auto* F=S->GetInlineFormulaSurface();
    const auto& P=F->GetPresentation();
    if (!P.bNarrativeAvailable)
    {
     Test->TestTrue(TEXT("Hidden canonical narrative never enters outcome composition"),
      CastChecked<URichTextBlock>(F->GetWidgetFromName(TEXT("OutcomePrimary")))->GetText().IsEmpty());
     bObservedHidden=true;
    }
    Test->TestTrue(TEXT("Actual transition never restores the legacy frame"),
     CastChecked<UFMCodexMatchFlowPanel>(F->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled());
    if (P.bNarrativeAvailable && P.bDiceRevealVisible)
    {
     Test->TestTrue(TEXT("Actual ResultHold keeps neutral intermediate and suppresses final result"),
      CastChecked<URichTextBlock>(F->GetWidgetFromName(TEXT("OutcomePrimary")))->GetText().IsEmpty()
      && F->GetWidgetFromName(TEXT("OutcomeIntermediate"))->GetVisibility() != ESlateVisibility::Collapsed
      && F->GetWidgetFromName(TEXT("InlineFormulaContinueButton"))->GetParent()->GetVisibility() == ESlateVisibility::Collapsed
      && F->GetWidgetFromName(TEXT("InlineFormulaContestHeading"))->GetVisibility() == ESlateVisibility::Collapsed
      && CastChecked<UFMCodexMatchFlowPanel>(F->GetWidgetFromName(TEXT("InlineFormulaDiceRevealRegion")))->GetFormulaRole() == EFMCodexFormulaPanelRole::RollHost);
     if (!bCapturedHold)
     {
      Capture(F->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Outcome_Intermediate_After.png")); bCapturedHold=true;
      Test->AddInfo(FString::Printf(TEXT("OUTCOME_TRANSITION narrativeFact=1 reel=1 finalHeading=empty intermediate=1 action=0 phase=%d"), static_cast<int32>(P.RevealPhase)));
     }
    }
    Test->TestFalse(TEXT("Original reveal blocks next action"),P.PrimaryAction.bVisible);
   }
   return false;
  }
  if (Step==2)
  {
   if (!Test->TestTrue(TEXT("Natural type selection"),C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollSetPieceType)) return true;
   if (!Override(*C,EFMCodexLocalDevRollTarget::SetPieceType,3)) return true;
   S->RequestContinueResolution(); Advance(); return false;
  }
  if (Step==3)
  {
   const auto& V=C->GetInteractionView();
   if (V.LegalSetPieceCardIds.IsEmpty()) { Test->AddError(TEXT("No legal taker")); return true; }
   C->ToggleSetPieceDraftCard(V.LegalSetPieceCardIds[0]);
   CastChecked<UButton>(S->GetWidgetFromName(TEXT("SetPieceProductionPrimaryAction")))->OnClicked.Broadcast();
   Advance(); return false;
  }
  if (Step==4)
  {
   auto* B=CastChecked<UButton>(S->GetWidgetFromName(TEXT("LongPowerMethod")));
   if (!Test->TestTrue(TEXT("Original power choice enabled"),B->GetIsEnabled())) return true;
   B->OnClicked.Broadcast(); Advance(); return false;
  }
  if (Step==5)
  {
   if (!Override(*C,EFMCodexLocalDevRollTarget::LongFreeKickPowerA,3)
    || !Override(*C,EFMCodexLocalDevRollTarget::LongFreeKickPowerB,1)) return true;
   S->GetInlineFormulaSurface()->RequestContinue(); Advance(); return false;
  }
  if (Step==6)
  {
   auto* F=S->GetInlineFormulaSurface(); const auto& P=F->GetPresentation();
   Test->TestTrue(TEXT("Real provider and lifecycle reveal non-formula result"),bObservedHidden && bCapturedHold && P.bNarrativeAvailable && !P.bShowFormulaRows
    && C->GetInteractionView().bTerminalPendingAdvance && !C->GetInteractionView().bSetPieceGoal);
   Test->TestEqual(TEXT("Disclosed prose remains exact"),CastChecked<URichTextBlock>(F->GetWidgetFromName(TEXT("OutcomePrimary")))->GetText().ToString(),FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel,P.OutcomeText));
   Test->TestEqual(TEXT("Real paired detail uses disclosed structured dice"),CastChecked<UTextBlock>(F->GetWidgetFromName(TEXT("OutcomeDetail")))->GetText().ToString(),FString(TEXT("首次掷点 3 + 第二次掷点 1 = 4")));
   Test->TestEqual(TEXT("Real semantic segments preserve canonical sentence"),P.OutcomeText.ToText().ToString(),P.ContestLabel);
   Test->TestTrue(TEXT("Real non-goal has warm keyword only"),P.OutcomeText.Accent == EFMCodexOutcomeAccent::NoGoal);
   Capture(F->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Outcome_Final_NoGoal_After.png"));
   CastChecked<UButton>(F->GetWidgetFromName(TEXT("InlineFormulaContinueButton")))->OnClicked.Broadcast();
   Advance(); return false;
  }
  Test->TestTrue(TEXT("Unchanged button handler completes authoritative next-action handoff"),C->GetLastDiagnostic().bHostSuccess && !C->GetInteractionView().bTerminalPendingAdvance);
  return true;
 }
private:
 void Advance() { ++Step; Changed=FPlatformTime::Seconds(); }
 bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
 {
  FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
  return Test->TestTrue(TEXT("Existing DEV RNG provider accepts override"),C.SetLocalDevRollOverride(R).bSuccess);
 }

	void Save(const TArray<FColor>& Pixels,const FIntVector& Size,const TCHAR* File)
	{
		const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_7E_2"));
		IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
		Test->AddInfo(FString::Printf(TEXT("OUTCOME_CAPTURE %s %dx%d game=%.3f"),File,Size.X,Size.Y,GEditor->PlayWorld->GetTimeSeconds()));
	}
	void Capture(UWidget* Widget,const TCHAR* File)
	{
		// Existing live widget in the real PIE 2D window renderer. Temporarily frame
		// at native Slate size for readability; restore content and its parent.
		const auto Slate = Widget->TakeWidget(); Slate->SlatePrepass(1.f);
		const FVector2D NativeSize(FMath::CeilToInt(Widget->GetCachedGeometry().GetLocalSize().X),FMath::CeilToInt(Slate->GetDesiredSize().Y));
		auto Window = GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Window.IsValid()) { Test->AddError(TEXT("No PIE capture window")); return; }
		const auto OriginalContent = Window->GetContent();
		const auto OriginalParent = Slate->GetParentWidget();
		const FVector2D OriginalClientSize = Window->GetClientSizeInScreen();
		// A native outcome can exceed the small offscreen PIE client. Give the
		// capture enough space so the footer/CTA is not clipped by that window.
		Window->Resize(FVector2D(FMath::Max(1000.0,NativeSize.X+80),FMath::Max(700.0,NativeSize.Y+80)));
		Window->SetContent(SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[SNew(SBox).WidthOverride(NativeSize.X).HeightOverride(NativeSize.Y)[Slate]]);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		const bool bRead = FSlateApplication::Get().TakeScreenshot(Slate,Pixels,Size);
		Window->SetContent(OriginalContent);
		if (OriginalParent.IsValid()) Slate->AssignParentWidget(OriginalParent);
		Window->Resize(OriginalClientSize);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		Test->TestTrue(TEXT("Native capture includes full outcome height"),bRead && Size.Y >= FMath::FloorToInt(NativeSize.Y));
		if (bRead) Save(Pixels,Size,File); else Test->AddError(TEXT("Native PIE widget capture failed"));
	}

 FAutomationTestBase* Test;
 double Started=FPlatformTime::Seconds(),Changed=0;
 int32 Step=0;
 bool bObservedHidden=false, bCapturedHold=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOutcomeFamilyPIETest,
 "FMCodex.PIE.OutcomeFamily.LongFreeKickPower",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexOutcomeFamilyPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartOutcomeFamilyPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FOutcomeFamilyPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
