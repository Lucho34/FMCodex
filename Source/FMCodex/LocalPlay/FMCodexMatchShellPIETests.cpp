#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
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

#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexLocalDevRollOverride.h"

namespace
{
TSharedPtr<SWindow> ShellWindow;
class FStartShellPIE : public IAutomationLatentCommand
{
public:
 bool Update() override
 {
    auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
    Settings->NewWindowWidth=1600; Settings->NewWindowHeight=900;
    Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
    ShellWindow=SNew(SWindow).Title(FText::FromString(TEXT("Production Match Shell PIE")))
      .ClientSize(FVector2D(1600,900)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
      .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
    FSlateApplication::Get().AddWindow(ShellWindow.ToSharedRef());
    FRequestPlaySessionParams P; P.EditorPlaySettings=Settings; P.CustomPIEWindow=ShellWindow;
    GEditor->RequestPlaySession(P); return true;
 }
};
class FPlayShellPIE : public IAutomationLatentCommand
{
public:
 explicit FPlayShellPIE(FAutomationTestBase* In):T(In),bMicroPolish(FParse::Param(FCommandLine::Get(),TEXT("MatchShellMicroPolish"))){}
 bool Update() override
 {
    const double Now=FPlatformTime::Seconds();
    // Keep visual captures free of unrelated hover cards, including after window resize.
    if (ShellWindow.IsValid())
    {
       auto& App=FSlateApplication::Get();
       const FVector2D Before=App.GetCursorPos();
       const FVector2D At=ShellWindow->GetPositionInScreen()+FVector2D(800,45);
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
    auto Deploy=[&](FName Card){
       const auto* O=V.DeploymentOptions.FindByPredicate([&](const auto& X){ return !X.bGoalkeeper && X.CardId==Card && X.SlotId.ToString().Contains(Forward); });
       if(!T->TestNotNull(TEXT("Real legal forward deployment"),O)) return false;
       const FName Slot=O->SlotId; S->RequestDeployOrdinary(Card,Slot); return true;
    };
    switch(Step)
    {
    case 0: S->RequestStartNewMatch(); Next(); break;
    case 1:
       T->TestFalse(TEXT("Pre-roll resource hidden"),S->GetMatchHeader()->GetWidgetFromName(TEXT("LeftTacticalPointChip"))->IsVisible() || S->GetMatchHeader()->GetWidgetFromName(TEXT("RightTacticalPointChip"))->IsVisible());
       Capture(TEXT("01_BeforeTP_1600.png")); Pin(EFMCodexLocalDevRollTarget::FullD12,3);
       Click(TEXT("InteractionTacticalPointRollButton"),true); Next(); break;
    case 2:
       T->TestEqual(TEXT("Production TP result"),V.ActionPoint,3);
       Capture(TEXT("02_AfterTP_1600.png"));
       Carrier=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.Arsenal.MartinOdegaard"):TEXT("Prototype.ManchesterCity.PhilFoden");
       Defender=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.ManchesterCity.JohnStones"):TEXT("Prototype.Arsenal.GabrielMagalhaes");
       Forward=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("NearB"):TEXT("NearA");
       if(!Deploy(Carrier)) return true; Next(); break;
    case 3:
       // Choose from live legal options if the production defender catalog uses another key.
       if(!V.DeploymentOptions.ContainsByPredicate([&](const auto& X){return X.CardId==Defender;}))
          for(const auto& O:V.DeploymentOptions) if(!O.bGoalkeeper && O.SlotId.ToString().Contains(Forward)){ Defender=O.CardId; break; }
       if(!Deploy(Defender)) return true; Next(); break;
    case 4:
       if(bMicroPolish)
       {
          auto* Dock=S->GetInteractionPanel();
          auto* Hint=Dock->GetWidgetFromName(TEXT("DeploymentHandInstruction"));
          T->TestTrue(TEXT("Actual deployment has no hint or reserved region"),CastChecked<UTextBlock>(Hint)->GetText().IsEmpty()
             && Hint->GetVisibility()==ESlateVisibility::Collapsed
             && Dock->GetWidgetFromName(TEXT("InteractionCandidateRegion"))->GetVisibility()==ESlateVisibility::Collapsed);
          const FString Actor=CastChecked<UTextBlock>(Dock->GetWidgetFromName(TEXT("InteractionExpectedActor")))->GetText().ToString();
          T->TestTrue(TEXT("Actual concise player operation context"),Actor==TEXT("玩家 A 操作") || Actor==TEXT("玩家 B 操作"));
          T->TestEqual(TEXT("Actual deployment phase title"),CastChecked<UTextBlock>(Dock->GetWidgetFromName(TEXT("InteractionActionTitle")))->GetText().ToString(),FString(TEXT("部署球员")));
          T->TestTrue(TEXT("Actual deployment retains both actions"),Dock->GetWidgetFromName(TEXT("DeploymentTacticalReferenceEntryButton"))->IsVisible()
             && Dock->GetWidgetFromName(TEXT("InteractionFinishDeploymentButton"))->IsVisible());
       }
       Capture(TEXT("03_Deployment_1600.png"));
       ShellWindow->Resize(FVector2D(1920,1080)); Next(); break;
    case 5:
       Capture(TEXT("04_Deployment_1920.png"));
       if(bMicroPolish)
       {
          T->AddInfo(FString::Printf(TEXT("MATCH_SHELL_MICRO_PIE elapsed=%.1fs; one Local session; pre/post TP and deployment; 1920 resize only"),Now-Started));
          return true;
       }
       ShellWindow->Resize(FVector2D(1600,900)); Next(); break;
    case 6:
       if(V.InteractionCategory==EFMCodexLocalMatchInteractionCategory::Deploy)
       { Click(TEXT("InteractionFinishDeploymentButton"),true); Changed=Now; }
       else Next();
       break;
    case 7:
       using Cat=EFMCodexLocalMatchInteractionCategory;
       if(V.InteractionCategory==Cat::SelectCarrier){ Capture(TEXT("05_Carrier_1600.png")); S->RequestSubmitCarrier(Carrier); }
       else if(V.InteractionCategory==Cat::SelectMarker) S->RequestSubmitMarker(Defender);
       else if(V.InteractionCategory==Cat::SelectRunner || V.InteractionCategory==Cat::SelectHelper)
       { if(V.bCanResolveNoLegalChoice) S->RequestResolveNoLegalSelection(); else S->RequestDeclineSelection(); }
       else if(V.InteractionCategory==Cat::SelectSkill)
       {
          Capture(TEXT("06_TacticalChoice_1600.png"));
          const auto* O=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([](const auto& X){return X.SkillType==ESkillRuleType::LongShot && X.bEnabled;});
          if(!T->TestNotNull(TEXT("Production LongShot choice"),O)) return true;
          const auto Id=O->OptionId; S->RequestSubmitSkill(Id); Next();
       }
       else { T->AddError(TEXT("Unexpected role state")); return true; }
       Changed=Now; break;
    case 8:
       Click(TEXT("TheaterNearDirect")); Next(); break;
    case 9:
       Pin(EFMCodexLocalDevRollTarget::LongShotDirectAttack,5); Click(TEXT("TheaterContinue")); Next(); break;
    case 10:
       Pin(EFMCodexLocalDevRollTarget::LongShotDirectDefense,3); Click(TEXT("TheaterContinue")); Next(); break;
    case 11:
       T->TestTrue(TEXT("Natural resolution reaches next-round boundary"),V.bTerminalPendingAdvance);
       T->TestFalse(TEXT("No duplicate dock roll action in Theater"),S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionTacticalPointRollButton"))->IsVisible());
       Click(TEXT("TheaterContinue")); Next(); break;
    case 12:
       T->TestTrue(TEXT("Next round returns to production TP action"),V.bTacticalPointRollReady);
       T->AddInfo(FString::Printf(TEXT("MATCH_SHELL_PIE elapsed=%.1fs; TP/deployment/roles/tactic/method/resolution/result/next; 1600x900 and 1920x1080"),Now-Started));
       return true;
    }
    return false;
 }
private:
 void Capture(const TCHAR* Name)
 {
    auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
    TArray<FColor> Pixels; FIntVector Size=FIntVector::ZeroValue;
    if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)){T->AddError(TEXT("Actual PIE screenshot unavailable"));return;}
    const FString Dir=FPaths::ProjectSavedDir()/(bMicroPolish?TEXT("Stage8_22A/MicroPolish/PIE"):TEXT("Stage8_22A/PIE")); IFileManager::Get().MakeDirectory(*Dir,true);
    TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
    T->TestTrue(TEXT("Actual shell screenshot saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
 }
 FAutomationTestBase* T; int32 Step=0; FName Carrier,Defender; FString Forward;
 bool bMicroPolish=false;
 double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMatchShellPIETest,"FMCodex.PIE.MatchShell.Commercialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMatchShellPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartShellPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayShellPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand()); return true;
}
#endif
