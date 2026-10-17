#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexDeploymentTacticalGuide.h"
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "Components/Button.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "ImageUtils.h"
#include "HAL/FileManager.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"

namespace
{
TSharedPtr<SWindow> GuideWindow;
class FStartGuidePIE : public IAutomationLatentCommand
{
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1920; Settings->NewWindowHeight=1080;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
  GuideWindow=SNew(SWindow).Title(FText::FromString(TEXT("Deployment Guide PIE")))
   .ClientSize(FVector2D(1920,1080)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
   .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
  FSlateApplication::Get().AddWindow(GuideWindow.ToSharedRef());
  FRequestPlaySessionParams P; P.EditorPlaySettings=Settings; P.CustomPIEWindow=GuideWindow;
  GEditor->RequestPlaySession(P); return true;
 }
};
class FPlayGuidePIE : public IAutomationLatentCommand
{
public:
 explicit FPlayGuidePIE(FAutomationTestBase* In):T(In){}
 bool Update() override
 {
  const double Now=FPlatformTime::Seconds();
  if(Now-Started>90) {T->AddError(FString::Printf(TEXT("Guide PIE timeout step %d"),Step));return true;}
  if(!GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C?C->GetPlayerMatchScreen():nullptr; if(!S) return false;
  auto* G=S->GetInteractionPanel()->GetTacticalGuide();
  if(Step==2)
  {
   if(G->GetTactics().IsEmpty()) bSawHidden=true;
   else if(G->GetEntranceProgress()<1.f) bSawFade=true;
  }
  if(Now-Changed<.6) return false;
  auto Next=[&](){++Step;Changed=Now;};
  auto Chip=[&](FName Id){auto B=G->GetChip(Id);if(T->TestTrue(TEXT("Real chip mounted"),B.IsValid()))
   {T->AddInfo(FString::Printf(TEXT("Chip %s step %d: enabled %d, at %s"),*Id.ToString(),Step,B->IsEnabled(),*Center(B->GetCachedGeometry()).ToString()));Click(B->GetCachedGeometry());
    T->AddInfo(FString::Printf(TEXT("Selection immediately after click: %s"),*G->GetSelection().ToString()));}};
  switch(Step)
  {
  case 0:S->RequestStartNewMatch();Next();break;
  case 1:
  {
   FFMCodexLocalDevRollOverrideRequest R;R.Target=EFMCodexLocalDevRollTarget::FullD12;R.Value=4;
   T->TestTrue(TEXT("DEV provider sets normal TP"),C->SetLocalDevRollOverride(R).bSuccess);
   S->RequestRollTacticalPoints();Next();break;
  }
  case 2:
   if(G->GetTactics().IsEmpty() || G->GetEntranceProgress()<1.f) return false;
   T->TestTrue(TEXT("Actual result boundary hides then fades chips"),bSawHidden && bSawFade);
   Expected=G->GetTactics();
   Pointer(Center(G->GetChip(TEXT("LongShot"))->GetCachedGeometry()));Next();break;
  case 3:
   T->TestTrue(TEXT("Real hover does not open"),G->GetSelection().IsNone());Chip(TEXT("LongShot"));Next();break;
  case 4:
   T->TestEqual(TEXT("Real click opens its section"),G->GetSelection(),FName(TEXT("LongShot")));
   Pointer(GuideWindow->GetPositionInScreen()+FVector2D(900,60));Next();break;
  case 5:
   T->TestEqual(TEXT("Pointer departure preserves panel"),G->GetSelection(),FName(TEXT("LongShot")));
   Chip(TEXT("Cross"));Next();break;
  case 6:
  {
   T->TestEqual(TEXT("Real different click replaces content"),G->GetExplainer()->GetTacticId(),FName(TEXT("Cross")));
   const auto Panel=G->GetDetailGeometry();const auto Window=GuideWindow->GetContent()->GetCachedGeometry();
   const auto Pos=Window.AbsoluteToLocal(Panel.GetAbsolutePosition());const auto End=Window.AbsoluteToLocal(Panel.LocalToAbsolute(Panel.GetLocalSize()));
   T->TestTrue(TEXT("Anchored detail fits viewport"),Pos.X>=0 && Pos.Y>=0 && End.X<=Window.GetLocalSize().X+1 && End.Y<=Window.GetLocalSize().Y+1);
   Capture();Chip(TEXT("Cross"));Next();break;
  }
  case 7:
   T->TestTrue(TEXT("Real same click closes"),G->GetSelection().IsNone());Chip(TEXT("LongShot"));Next();break;
  case 8:
   FSlateApplication::Get().ProcessKeyDownEvent(FKeyEvent(EKeys::Escape,FModifierKeysState(),0,false,0,0));
   T->TestTrue(TEXT("Real Esc closes"),G->GetSelection().IsNone());Next();break;
  case 9:
  {
   T->TestEqual(TEXT("Information preserves TP"),C->GetInteractionView().ActionPoint,4);
   const auto Options=C->GetInteractionView().DeploymentOptions;
   const auto* O=Options.FindByPredicate([](const auto& X){return !X.bGoalkeeper;});
   if(T->TestNotNull(TEXT("Real deployment choice"),O)) S->RequestDeployOrdinary(O->CardId,O->SlotId);
   Next();break;
  }
  case 10:
   T->TestTrue(TEXT("Real hand-to-field union retained"),G->GetTactics()==Expected);
   T->TestEqual(TEXT("Deployment never restarts cue"),G->GetEntranceProgress(),1.f);
   Chip(TEXT("Cross"));Next();break;
  case 11:
  {
   FinishingSide=C->GetInteractionView().ExpectedActingPlayer;
   auto* B=CastChecked<UButton>(S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionFinishDeploymentButton")));
   Click(B->GetCachedGeometry());Next();break;
  }
  case 12:
   T->TestTrue(TEXT("Outside Finish click closes and reaches gameplay"),G->GetSelection().IsNone()
    && (FinishingSide==EInitialTurnOrderPlayer::PlayerA?C->GetInteractionView().bPlayerADeploymentFinished:C->GetInteractionView().bPlayerBDeploymentFinished));
   // Finish the other deployment via the existing intent; no full match is needed.
   S->RequestFinishDeployment();Next();break;
  case 13:
   T->TestTrue(TEXT("Leaving deployment clears chips and panel"),G->GetTactics().IsEmpty() && G->GetSelection().IsNone());
   return true;
  }
  return false;
 }
private:
 FVector2D Center(const FGeometry& G){return G.LocalToAbsolute(G.GetLocalSize()*.5f);}
 void Pointer(FVector2D At,int32 Button=0)
 {
  auto& App=FSlateApplication::Get();const auto Before=App.GetCursorPos();App.SetCursorPos(At);
  TSet<FKey> Keys;if(Button==1)Keys.Add(EKeys::LeftMouseButton);
  FPointerEvent E(0,At,Before,Keys,Button?EKeys::LeftMouseButton:EKeys::Invalid,0,FModifierKeysState());
  if(Button==1)App.ProcessMouseButtonDownEvent(nullptr,E);else if(Button==2)App.ProcessMouseButtonUpEvent(E);else App.ProcessMouseMoveEvent(E,false);
 }
 void Click(const FGeometry& G){const auto At=Center(G);Pointer(At);Pointer(At,1);Pointer(At,2);}
 void Capture()
 {
  TArray<FColor> Pixels;FIntVector Size;
  if(!T->TestTrue(TEXT("Real guide screenshot"),FSlateApplication::Get().TakeScreenshot(GuideWindow->GetContent(),Pixels,Size)))return;
  const auto Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_30E");IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  T->TestTrue(TEXT("Screenshot saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/TEXT("Guide.png"))));
 }
 FAutomationTestBase* T;int32 Step=0;bool bSawHidden=false,bSawFade=false;
 TArray<FName> Expected;double Started=FPlatformTime::Seconds(),Changed=0;
 EInitialTurnOrderPlayer FinishingSide=EInitialTurnOrderPlayer::None;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGuidePIETest,"FMCodex.PIE.DeploymentTacticalGuide",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FGuidePIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartGuidePIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayGuidePIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
