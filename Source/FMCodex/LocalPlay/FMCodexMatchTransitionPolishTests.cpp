#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexResolutionPanelWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexMatchShellStyle.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
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
#include "Slate/WidgetRenderer.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
void CheckCopyLabels(FAutomationTestBase& Test, const TSharedRef<SWidget>& Widget, int32& Count)
{
 if (Widget->GetTypeAsString() == TEXT("SHorizontalBox"))
 {
  auto* Children = Widget->GetChildren();
  if (Children->Num() == 2 && Children->GetChildAt(0)->GetTypeAsString() == TEXT("STextBlock"))
  {
   const auto Title = StaticCastSharedRef<STextBlock>(Children->GetChildAt(0));
   if (Title->GetText().ToString() == TEXT("前置判定") || Title->GetText().ToString() == TEXT("结果"))
   {
    ++Count;
    const auto& G = Title->GetCachedGeometry();
    Test.TestTrue(TEXT("Chinese copy label receives its full desired width"), G.GetLocalSize().X + .5f >= Title->GetDesiredSize().X);
    const auto End = G.LocalToAbsolute(FVector2D(G.GetLocalSize().X,0));
    const auto Start = Children->GetChildAt(1)->GetCachedGeometry().GetAbsolutePosition();
    Test.TestTrue(TEXT("Copy body starts after the complete label with a visible gap"), Start.X > End.X + 4.f);
   }
  }
 }
 auto* Children = Widget->GetChildren();
 for (int32 I=0; I<Children->Num(); ++I) CheckCopyLabels(Test, Children->GetChildAt(I), Count);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTransitionWidgetTest,"FMCodex.LocalPlay.MatchTransitions.LayoutAndReuse",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTransitionWidgetTest::RunTest(const FString&)
{
 auto* Renderer = new FWidgetRenderer(true);
 auto* Explainer = NewObject<UFMCodexTacticExplainerWidget>();
 auto Slate = Explainer->TakeWidget();
 Explainer->SelectTactic(TEXT("CutInside")); Explainer->ShowDetails(true);
 for (const FVector2D Size : {FVector2D(1200,700),FVector2D(1440,840)})
 {
  auto* Target = Renderer->DrawWidget(Slate,Size);
  Renderer->DrawWidget(Target,Slate,Size,0.f);
  int32 Count=0; CheckCopyLabels(*this,Slate,Count);
  TestEqual(TEXT("Both precondition and result labels are checked"),Count,2);
 }
 auto* Detail = NewObject<UFMCodexTacticalDetailPanelWidget>();
 auto DetailSlate = Detail->TakeWidget();
 Detail->RefreshFromPresentation(FFMCodexTacticalDetailPresentationBuilder::Build(ESkillRuleType::Cross));
 auto* Target = Renderer->DrawWidget(DetailSlate,FVector2D(780,430));
 Renderer->DrawWidget(Target,DetailSlate,FVector2D(780,430),0.f);
 for (int32 Branch=0; Branch<2; ++Branch)
 {
  auto* Heading=Detail->GetWidgetFromName(*FString::Printf(TEXT("TacticalDetailBranchTitle%d"),Branch));
  auto* Last=Detail->GetWidgetFromName(*FString::Printf(TEXT("TacticalDetailAttribute%d_4"),Branch));
  TestTrue(TEXT("Both Cross columns retain full heading and last goalkeeper row"),
   Heading->GetCachedGeometry().GetLocalSize().Y >= Heading->GetDesiredSize().Y-.5f
   && Last->GetCachedGeometry().GetLocalSize().Y >= Last->GetDesiredSize().Y-.5f);
 }
 auto* Recovery = NewObject<UFMCodexResolutionPanelWidget>(); Recovery->TakeWidget();
 FFMCodexUMGResolutionViewModel P; P.bVisible=true; P.bNonBlockingNotification=true;
 P.StepLabel=TEXT("球员返回手牌"); P.StepSummaryLabel=TEXT("玩家A · 亚历山大·阿诺德\n玩家B · 萨卡\n无分隔符的原文");
 Recovery->RefreshFromPresentation(P);
 auto* Rows=CastChecked<UVerticalBox>(Recovery->GetWidgetFromName(TEXT("RecoverySummaryRows")));
 TestEqual(TEXT("Notification preserves canonical line count"),Rows->GetChildrenCount(),3);
 auto* First=CastChecked<UHorizontalBox>(Rows->GetChildAt(0));
 auto* Name=CastChecked<UTextBlock>(First->GetChildAt(1));
 TestEqual(TEXT("Display name with middle dot is preserved in full"),Name->GetText().ToString(),FString(TEXT("亚历山大·阿诺德")));
 TestEqual(TEXT("Only the canonical name receives mint emphasis"),Name->GetColorAndOpacity().GetSpecifiedColor(),FMCodexMatchShellStyle::Mint());
 auto* Unknown=CastChecked<UHorizontalBox>(Rows->GetChildAt(2));
 TestEqual(TEXT("Unknown format is kept intact"),CastChecked<UTextBlock>(Unknown->GetChildAt(0))->GetText().ToString(),FString(TEXT("无分隔符的原文")));
 P.StepLabel=TEXT("进球"); Recovery->RefreshFromPresentation(P);
 TestTrue(TEXT("Unrelated system notification does not inherit recovery formatting"),Rows->GetChildrenCount()==0 && Rows->GetVisibility()==ESlateVisibility::Collapsed
  && CastChecked<UBorder>(Recovery->GetWidgetFromName(TEXT("ResolutionPanelFrame")))->Background.DrawAs!=ESlateBrushDrawType::RoundedBox);
 BeginCleanup(Renderer); return true;
}

namespace
{
TSharedPtr<SWindow> TransitionWindow;
class FStartTransitionPIE : public IAutomationLatentCommand
{
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1600; Settings->NewWindowHeight=900;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone); Settings->SetPlayNumberOfClients(1);
  TransitionWindow=SNew(SWindow).Title(FText::FromString(TEXT("Match Transition Polish PIE")))
   .ClientSize(FVector2D(1600,900)).AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0))
   .SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
  FSlateApplication::Get().AddWindow(TransitionWindow.ToSharedRef());
  FRequestPlaySessionParams P; P.EditorPlaySettings=Settings; P.CustomPIEWindow=TransitionWindow;
  GEditor->RequestPlaySession(P); return true;
 }
};
class FPlayTransitionPIE : public IAutomationLatentCommand
{
public:
 explicit FPlayTransitionPIE(FAutomationTestBase* In):T(In),bFinalPolish(FParse::Param(FCommandLine::Get(),TEXT("MatchTransitionsFinalPolish"))){}
 bool Update() override
 {
  const double Now=FPlatformTime::Seconds();
  if (Now-Started>100) {T->AddError(FString::Printf(TEXT("Transition PIE timeout step %d"),Step)); return true;}
  if (!GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C?C->GetPlayerMatchScreen():nullptr; if(!S) return false;
  // Observe the real reveal before the input gate releases; never force a phase.
  if (Step==15 && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
  {
   const auto& P=S->GetInlineFormulaSurface()->GetPresentation();
   if (!bTypeCaptured && !P.RouteResultLabel.IsEmpty())
   {
    T->TestEqual(TEXT("Natural type result uses authoritative five"),P.RouteResultLabel,FString(TEXT("掷点 5 → 近距离任意球")));
    T->TestFalse(TEXT("No premature type continuation CTA"),P.PrimaryAction.bVisible);
    Capture(TEXT("05_TypeResult.png")); bTypeCaptured=true;
   }
  }
  if (S->IsInlineFormulaRevealInputBlocked() || Now-Changed<(Step==100?.15:Step==12?.05:.7)) return false;
  const auto& V=C->GetInteractionView();
  auto Next=[&](){++Step;Changed=Now;};
  auto Pin=[&](EFMCodexLocalDevRollTarget Target,int32 Value){FFMCodexLocalDevRollOverrideRequest R;R.Target=Target;R.Value=Value;T->TestTrue(TEXT("Existing DEV provider accepts override"),C->SetLocalDevRollOverride(R).bSuccess);};
  auto Click=[&](const TCHAR* Name,bool Dock=false){auto* B=Cast<UButton>(Dock?S->GetInteractionPanel()->GetWidgetFromName(Name):S->GetWidgetFromName(Name));
   if(T->TestNotNull(TEXT("Existing semantic CTA"),B)&&T->TestTrue(TEXT("CTA remains visible/enabled"),B->IsVisible()&&B->GetIsEnabled()))B->OnClicked.Broadcast();};
  auto Deploy=[&](FName Card){const auto* O=V.DeploymentOptions.FindByPredicate([&](const auto& X){return !X.bGoalkeeper&&X.CardId==Card&&X.SlotId.ToString().Contains(Forward);});
   if(!T->TestNotNull(TEXT("Live legal forward deployment"),O))return false; const FName Slot=O->SlotId;S->RequestDeployOrdinary(Card,Slot);return true;};
  switch(Step)
  {
  case 0:S->RequestStartNewMatch();Next();break;
  case 1:Pin(EFMCodexLocalDevRollTarget::FullD12,bFinalPolish?4:3);Click(TEXT("InteractionTacticalPointRollButton"),true);Next();break;
  case 2:
   Click(TEXT("DeploymentTacticalReferenceEntryButton"),true);
   if(bFinalPolish){ExplainerRoot=S->GetTacticExplainer()->TakeWidget();Step=100;Changed=Now;break;}
   S->GetTacticExplainer()->SelectTactic(TEXT("CutInside"));S->GetTacticExplainer()->ShowDetails(true);Next();break;
  case 100:
   {
    auto* W=S->GetTacticExplainer();
    const TCHAR* Families[]={TEXT("LongShot"),TEXT("CutInside"),TEXT("Cross"),TEXT("ThroughBall"),TEXT("Corner"),TEXT("NearFreeKick")};
    if(SwitchIndex<6)
    {
     W->SelectTactic(Families[SwitchIndex]);
     T->TestTrue(TEXT("Rapid navigation retains mounted modal shell"),W->TakeWidget()==ExplainerRoot);
     T->TestTrue(TEXT("Complete new overview is available immediately"),!W->IsShowingDetails() && W->CollectPlayerFacingText().Contains(TEXT("查看计算方式")));
     ++SwitchIndex;Changed=Now;break;
    }
    if(SwitchIndex==6){W->SelectTactic(TEXT("Cross"));W->SelectRoute(TEXT("Cross.High"));++SwitchIndex;Changed=Now;break;}
    if(SwitchIndex==7){Capture(TEXT("06_CrossHigh.png"));W->SelectRoute(TEXT("Cross.Low"));++SwitchIndex;Changed=Now;break;}
    if(SwitchIndex==8){Capture(TEXT("07_CrossLow.png"));W->ShowDetails(true);++SwitchIndex;Changed=Now;break;}
    if(SwitchIndex==9)
    {
     W->SelectTactic(TEXT("NearFreeKick"));
     T->TestFalse(TEXT("Top-level change returns calculation to overview"),W->IsShowingDetails());
     W->SelectTactic(TEXT("CutInside"));W->ShowDetails(true);Step=3;Changed=Now;break;
    }
   }
   break;
  case 3:
   {int32 Count=0;CheckCopyLabels(*T,S->GetTacticExplainer()->TakeWidget(),Count);T->TestEqual(TEXT("Real PIE checks both copy headings"),Count,2);}
   Capture(TEXT("01_CalculationLabels.png"));S->GetTacticExplainer()->OnClose.Broadcast();
   Carrier=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.Arsenal.MartinOdegaard"):TEXT("Prototype.ManchesterCity.PhilFoden");
   Defender=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.ManchesterCity.JohnStones"):TEXT("Prototype.Arsenal.GabrielMagalhaes");
   Forward=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("NearB"):TEXT("NearA");
   if(bFinalPolish)
   {
    Carrier=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.Arsenal.BukayoSaka"):TEXT("Prototype.ManchesterCity.RayanAitNouri");
    Runner=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA?TEXT("Prototype.Arsenal.ViktorGyokeres"):TEXT("Prototype.ManchesterCity.ErlingHaaland");
   }
   if(!Deploy(Carrier))return true;Next();break;
  case 4:
   if(!Deploy(Defender))return true;
   if(bFinalPolish)
   {
    if(!Deploy(Runner))return true;
    const auto* HelperOption=V.DeploymentOptions.FindByPredicate([&](const auto& O){return !O.bGoalkeeper && O.SlotId.ToString().Contains(Forward);});
    if(!T->TestNotNull(TEXT("Live second defender deployment"),HelperOption))return true;
    const FName Helper=HelperOption->CardId;if(!Deploy(Helper))return true;
   }
   Next();break;
  case 5:
   if(V.InteractionCategory==EFMCodexLocalMatchInteractionCategory::Deploy){Click(TEXT("InteractionFinishDeploymentButton"),true);Changed=Now;}else Next();break;
  case 6:
   using Cat=EFMCodexLocalMatchInteractionCategory;
   if(V.InteractionCategory==Cat::SelectCarrier)S->RequestSubmitCarrier(Carrier);
   else if(V.InteractionCategory==Cat::SelectMarker)S->RequestSubmitMarker(Defender);
   else if(V.InteractionCategory==Cat::SelectRunner||V.InteractionCategory==Cat::SelectHelper)
   {if(bFinalPolish && V.InteractionCategory==Cat::SelectRunner)S->RequestSubmitRunner(Runner);
    else if(V.bCanResolveNoLegalChoice)S->RequestResolveNoLegalSelection();else S->RequestDeclineSelection();}
   else if(V.InteractionCategory==Cat::SelectSkill)
   {
    const auto* O=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([&](const auto& X){return X.SkillType==(bFinalPolish?ESkillRuleType::CutInsideShot:ESkillRuleType::LongShot)&&X.bEnabled;});
    if(!T->TestNotNull(TEXT("Legal ordinary shot choice"),O))return true;Skill=O->OptionId;
    if(bFinalPolish)
    {
     for(const TCHAR* Name : {TEXT("传中"),TEXT("内切")})
      T->TestTrue(TEXT("Both requested tactics remain available"),S->GetPresentation().Interaction.SelectionChoices.ContainsByPredicate([&](const auto& X){return X.Label==Name&&X.bEnabled;}));
     for(const auto& Option:S->GetInteractionPanel()->GetRenderedOptionWidgets())
     {
      auto* Subtitle=CastChecked<UTextBlock>(Option->GetWidgetFromName(TEXT("InteractionOptionSecondaryLabel")));
      T->TestTrue(TEXT("Actual tactic buttons are name only"),Subtitle->GetText().IsEmpty()&&Subtitle->GetVisibility()==ESlateVisibility::Collapsed);
     }
    }
    for(const auto& Option:S->GetInteractionPanel()->GetRenderedOptionWidgets()) if(Option->GetLabel()==O->Label)
     CastChecked<UButton>(Option->GetWidgetFromName(TEXT("InteractionOptionButton")))->OnHovered.Broadcast();
    Next();
   }
   else {T->AddError(TEXT("Unexpected role state"));return true;} Changed=Now;break;
  case 7:
   T->TestTrue(TEXT("Actual tactic hover shows canonical compact summary"),S->GetTacticalDetailPanel()->IsVisible());
   Capture(TEXT("02_TacticalChoiceAndPreview.png"));S->RequestSubmitSkill(Skill);Next();break;
  case 8:Click(TEXT("TheaterNearDirect"));Next();break;
  case 9:Pin(bFinalPolish?EFMCodexLocalDevRollTarget::CutInsideShotDirectAttack:EFMCodexLocalDevRollTarget::LongShotDirectAttack,5);Click(TEXT("TheaterContinue"));Next();break;
  case 10:Pin(bFinalPolish?EFMCodexLocalDevRollTarget::CutInsideShotDirectDefense:EFMCodexLocalDevRollTarget::LongShotDirectDefense,3);Click(TEXT("TheaterContinue"));Next();break;
  case 11:T->TestTrue(TEXT("Natural terminal boundary"),V.bTerminalPendingAdvance);Click(TEXT("TheaterContinue"));RecoveryStart=GEditor->PlayWorld->GetTimeSeconds();RecoveryNames=S->GetPresentation().Resolution.StepSummaryLabel;Next();break;
  case 12:
   if(bFinalPolish)
   {
    const float Elapsed=GEditor->PlayWorld->GetTimeSeconds()-RecoveryStart;
    const auto& P=S->GetPresentation().Resolution;
    if(Elapsed<2.45f)
    {
     T->TestTrue(TEXT("Returned names remain visible and nonblocking throughout readable hold"),P.bVisible&&P.bNonBlockingNotification&&P.StepSummaryLabel==RecoveryNames&&V.bTacticalPointRollReady);
     if(Elapsed>2.2f)bLateRecovery=true;
     if(!bRecoveryCaptured){Capture(TEXT("03_ReturnToHand.png"));bRecoveryCaptured=true;}
    }
    else if(Elapsed>2.65f)
    {
     T->TestTrue(TEXT("Natural timer completes the longer hold without blocking progression"),bLateRecovery&&!P.bVisible&&!C->IsRecoveryNotificationDismissScheduledForTesting()&&V.bTacticalPointRollReady);
     T->AddInfo(FString::Printf(TEXT("FINAL_MICRO_PIE readable hold sampled through >2.2s; natural expiry by %.3fs; six tactics and Cross routes; one Local session."),Elapsed));return true;
    }
    Changed=Now;break;
   }
   T->TestTrue(TEXT("Natural recovery remains nonblocking and next TP available"),S->GetPresentation().Resolution.bNonBlockingNotification && V.bTacticalPointRollReady);
   T->TestEqual(TEXT("Natural recovery title"),S->GetPresentation().Resolution.StepLabel,FString(TEXT("球员返回手牌")));
   Capture(TEXT("03_ReturnToHand.png"));Next();break;
  case 13:Pin(EFMCodexLocalDevRollTarget::FullD12,9);Click(TEXT("InteractionTacticalPointRollButton"),true);Next();break;
  case 14:
   T->TestEqual(TEXT("Next natural attack reaches type selection"),V.InteractionCategory,EFMCodexLocalMatchInteractionCategory::RollSetPieceType);
   Capture(TEXT("04_TypeSelection.png"));Pin(EFMCodexLocalDevRollTarget::SetPieceType,5);
   CastChecked<UButton>(S->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaContinueButton")))->OnClicked.Broadcast();Next();break;
  case 15:
   T->TestTrue(TEXT("Actual result hold was observed"),bTypeCaptured);
   T->TestEqual(TEXT("Existing handoff follows hold"),V.InteractionCategory,EFMCodexLocalMatchInteractionCategory::SelectSetPieceCarrier);
   T->AddInfo(TEXT("One Local PIE: calculation labels, tactics/hover, natural recovery, next-attack type selection and reveal/handoff."));return true;
  }
  return false;
 }
private:
 void Capture(const TCHAR* Name)
 {
  auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();TArray<FColor> Pixels;FIntVector Size=FIntVector::ZeroValue;
  if(!Window||!FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)){T->AddError(TEXT("PIE capture unavailable"));return;}
  const FString Dir=FPaths::ProjectSavedDir()/(bFinalPolish?TEXT("Stage8_22BC/FinalPolish/PIE"):TEXT("Stage8_22C/PIE"));IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
  T->TestTrue(TEXT("Actual PIE capture saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/Name)));
 }
 FAutomationTestBase* T;int32 Step=0;FName Carrier,Defender,Skill,Runner;FString Forward,RecoveryNames;bool bTypeCaptured=false;
 bool bFinalPolish=false,bLateRecovery=false,bRecoveryCaptured=false;int32 SwitchIndex=0;float RecoveryStart=0;TSharedPtr<SWidget> ExplainerRoot;
 double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTransitionPIETest,"FMCodex.PIE.MatchTransitions.Commercialization",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FTransitionPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartTransitionPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayTransitionPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
