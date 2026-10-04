#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "ImageUtils.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SScrollBox.h"

#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexLocalDevRollOverride.h"

namespace
{
TSharedPtr<SWindow> ExplainerWindow;
TSharedPtr<SScrollBox> FindExplainerScroll(const TSharedRef<SWidget>& Widget)
{
 if(Widget->GetTypeAsString()==TEXT("SScrollBox")) return StaticCastSharedRef<SScrollBox>(Widget);
 auto* Children=Widget->GetChildren();
 for(int32 I=0;I<Children->Num();++I) if(auto Found=FindExplainerScroll(Children->GetChildAt(I))) return Found;
 return nullptr;
}
class FStartExplainerPIE : public IAutomationLatentCommand
{
public:
 bool Update() override
 {
    auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
    Settings->NewWindowWidth=1600; Settings->NewWindowHeight=900;
    Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
    ExplainerWindow=SNew(SWindow).Title(FText::FromString(TEXT("Tactic Explainer PIE")))
      .ClientSize(FVector2D(1600,900)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
      .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
    FSlateApplication::Get().AddWindow(ExplainerWindow.ToSharedRef());
    FRequestPlaySessionParams P; P.EditorPlaySettings=Settings; P.CustomPIEWindow=ExplainerWindow;
    GEditor->RequestPlaySession(P); return true;
 }
};
class FPlayExplainerPIE : public IAutomationLatentCommand
{
public:
 explicit FPlayExplainerPIE(FAutomationTestBase* In):T(In){}
 bool Update() override
 {
    const double Now=FPlatformTime::Seconds();
    // Keep visual captures free of unrelated hover cards, including after window resize.
    if (ExplainerWindow.IsValid())
    {
       auto& App=FSlateApplication::Get();
       const FVector2D Before=App.GetCursorPos();
       const FVector2D At=ExplainerWindow->GetPositionInScreen()+FVector2D(800,45);
       App.SetCursorPos(At);
       App.ProcessMouseMoveEvent(FPointerEvent(0,At,Before,TSet<FKey>(),EKeys::Invalid,0,FModifierKeysState()),false);
    }
    if(Now-Started>100) { T->AddError(FString::Printf(TEXT("Shell PIE timeout step %d"),Step)); return true; }
    if(!GEditor->PlayWorld || Now-Changed<1.2) return false;
    auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
    auto* S=C?C->GetPlayerMatchScreen():nullptr;
    if(!S || S->IsInlineFormulaRevealInputBlocked()) return false;
    const auto& V=C->GetInteractionView();
    auto Next=[&](){ ++Step; Changed=Now; };
    auto Click=[&](const TCHAR* Name,bool Dock=false){
       auto* W=Dock?S->GetInteractionPanel()->GetWidgetFromName(Name):S->GetWidgetFromName(Name);
       auto* B=Cast<UButton>(W);
       if(T->TestNotNull(TEXT("Existing semantic CTA"),B) && T->TestTrue(TEXT("CTA is visible and enabled"),B->IsVisible() && B->GetIsEnabled())) B->OnClicked.Broadcast();
    };
    auto Pin=[&](EFMCodexLocalDevRollTarget Target,int32 Value){
       FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
       T->TestTrue(TEXT("Host-owned DEV provider accepts one roll"),C->SetLocalDevRollOverride(R).bSuccess);
    };
    auto* W=S->GetTacticExplainer();
    switch(Step)
    {
    case 0: S->RequestStartNewMatch(); Next(); break;
    case 1: Pin(EFMCodexLocalDevRollTarget::FullD12,3); Click(TEXT("InteractionTacticalPointRollButton"),true); Next(); break;
    case 2:
       Click(TEXT("DeploymentTacticalReferenceEntryButton"),true);
       T->TestTrue(TEXT("Actual production entry opens modal"),S->IsDeploymentTacticalReferenceOpen());
       if(FParse::Param(FCommandLine::Get(),TEXT("ExplainerCutLayoutRetry")))
       { W->SelectTactic(TEXT("CutInside")); W->ShowDetails(true); Step=20; Changed=Now; break; }
       W->SelectTactic(TEXT("Cross")); W->SelectRoute(TEXT("Cross.Low")); Next(); break;
    case 3:
       if(auto Pane=FindExplainerScroll(W->TakeWidget())) T->TestTrue(TEXT("Accepted overview including renamed CTA fits"),Pane->GetScrollOffsetOfEnd()<1.f);
       else T->AddError(TEXT("Right content pane missing"));
       Overview=W->CollectPlayerFacingText(); W->ShowDetails(true); Next(); break;
    case 4:
       CheckFit(W); Capture(TEXT("01_CrossLow_Calculation_1600.png"));
       W->ShowDetails(false); T->TestEqual(TEXT("Return restores accepted overview"),W->CollectPlayerFacingText(),Overview);
       W->SelectTactic(TEXT("CutInside")); W->ShowDetails(true); Next(); break;
    case 5:
       CheckFit(W); Capture(TEXT("02_CutInside_Calculation_1600.png"));
       W->SelectTactic(TEXT("ThroughBall")); W->SelectRoute(TEXT("ThroughBall.AntiOffside")); W->ShowDetails(true); Next(); break;
    case 6:
       CheckFit(W); Capture(TEXT("03_AntiOffside_Calculation_1600.png"));
       W->SelectTactic(TEXT("NearFreeKick")); W->ShowDetails(true); Next(); break;
    case 7:
       CheckFit(W); Capture(TEXT("04_NearFK_Calculation_1600.png"));
       ExplainerWindow->Resize(FVector2D(1920,1080)); Next(); break;
    case 8:
       CheckFit(W); Capture(TEXT("05_NearFK_Calculation_1920.png")); W->OnClose.Broadcast(); Next(); break;
    case 20:
       CheckFit(W); Capture(TEXT("02_CutInside_Calculation_1600.png"));
       W->ShowDetails(false); T->TestTrue(TEXT("Return reaches overview CTA"),W->CollectPlayerFacingText().Contains(TEXT("查看计算方式")));
       W->OnClose.Broadcast(); Step=9; Changed=Now; break;
    case 9:
       T->TestFalse(TEXT("Close removes modal"),S->IsDeploymentTacticalReferenceOpen());
       T->TestEqual(TEXT("No navigation action spends TP"),V.ActionPoint,3);
       T->TestEqual(TEXT("No navigation action advances deployment"),V.InteractionCategory,EFMCodexLocalMatchInteractionCategory::Deploy);
       T->AddInfo(FParse::Param(FCommandLine::Get(),TEXT("ExplainerCutLayoutRetry"))
        ?TEXT("CALCULATION_PIE_RETRY: CutInside Direct only, final 1600 layout, return/close, unchanged TP/deployment.")
        :TEXT("CALCULATION_PIE: one Local session; Cross Low, CutInside Direct, AntiOffside, Near FK Direct; return/close; 1600 and quick 1920."));
       return true;
    }
    return false;
 }
private:
 void CheckFit(UFMCodexTacticExplainerWidget* W)
 {
    if(auto Pane=FindExplainerScroll(W->TakeWidget())) T->TestTrue(FString::Printf(TEXT("%s calculation fits fixed right pane (extent %.1f)"),*W->GetRouteId().ToString(),Pane->GetScrollOffsetOfEnd()),Pane->GetScrollOffsetOfEnd()<1.f);
    else T->AddError(TEXT("Calculation pane missing"));
 }
 void Capture(const TCHAR* Name)
 {
    auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
    TArray<FColor> Pixels; FIntVector Size=FIntVector::ZeroValue;
    if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)){T->AddError(TEXT("Actual PIE screenshot unavailable"));return;}
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_22B/CalculationView/PIE"); IFileManager::Get().MakeDirectory(*Dir,true);
    TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
    T->TestTrue(TEXT("Actual shell screenshot saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
 }
 FString Overview;
 FAutomationTestBase* T; int32 Step=0;
 double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplainerPIETest,"FMCodex.PIE.TacticExplainer.Commercialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplainerPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartExplainerPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayExplainerPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}
#endif
