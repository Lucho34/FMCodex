#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "FMCodexRollReelWidget.h"
#include "Components/Button.h"
#include "Components/NativeWidgetHost.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
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
TSharedPtr<SWindow> SpatialWindow;
class FStartSpatialPIE final : public IAutomationLatentCommand
{
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  Settings->NewWindowWidth=1600;Settings->NewWindowHeight=900;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);Settings->SetPlayNumberOfClients(1);
  SpatialWindow=SNew(SWindow).Title(FText::FromString(TEXT("Tactical Scene Local PIE"))).ClientSize(FVector2D(1600,900))
   .AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0)).SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
  FSlateApplication::Get().AddWindow(SpatialWindow.ToSharedRef());
  FRequestPlaySessionParams P;P.EditorPlaySettings=Settings;P.CustomPIEWindow=SpatialWindow;GEditor->RequestPlaySession(P);return true;
 }
};
class FPlaySpatialPIE final : public IAutomationLatentCommand
{
public:
 explicit FPlaySpatialPIE(FAutomationTestBase* In):T(In){}
 bool Update() override
 {
  using Phase=FMCodexTacticalScene::EPhase;
  using Category=EFMCodexLocalMatchInteractionCategory;
  if(FPlatformTime::Seconds()-RoundStart>200){T->AddError(FString::Printf(TEXT("Spatial PIE timeout step=%d round=%d"),Step,Round));return true;}
  if(!GEditor || !GEditor->PlayWorld) return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C?C->GetPlayerMatchScreen():nullptr;
  if(!S) return false;
  auto Next=[&](){++Step;Changed=FPlatformTime::Seconds();};
  auto Roll=[&](EFMCodexLocalDevRollTarget Target,int Value){FFMCodexLocalDevRollOverrideRequest R;R.Target=Target;R.Value=Value;return T->TestTrue(TEXT("Server-owned deterministic provider"),C->SetLocalDevRollOverride(R).bSuccess);};
  const auto& State=S->GetTacticalScene();
  if(PreviewGeometryValid && State.Phase!=Phase::Hidden && State.Phase!=Phase::Preview
   && !CheckedGeometry.Contains(State.Phase))
  {
   const auto& G=S->GetWidgetFromName(TEXT("TacticalScene"))->GetCachedGeometry();
   const auto& RootG=S->GetCachedGeometry();
   const auto Position=RootG.AbsoluteToLocal(G.GetAbsolutePosition());
   const auto Size=RootG.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()))-Position;
   T->TestTrue(TEXT("Pitch origin stays fixed across method / roll / outcome / result"),Position.Equals(PreviewPosition,.5f));
   T->TestTrue(TEXT("Pitch scale stays fixed when lower controls change"),Size.Equals(PreviewSize,.5f));
   CheckedGeometry.Add(State.Phase);
  }
  if(State.Phase==Phase::Setup) SawSetup=true;
  if(State.Phase==Phase::Intent)
  {
   SawIntent=true;
   if(Round==0 && !Skipped)
   {
    auto* Host=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
    const auto Before=C->GetInteractionView().InteractionCategory;
    Host->GetContent()->OnKeyDown(Host->GetCachedGeometry(),FKeyEvent(EKeys::SpaceBar,FModifierKeysState(),0,false,0,0));
    T->TestEqual(TEXT("Space does not submit a gameplay intent"),C->GetInteractionView().InteractionCategory,Before);
    T->TestTrue(TEXT("Space advances only Intent"),S->GetTacticalScene().Phase==Phase::FormulaHold);Skipped=true;
   }
  }
  if(State.Phase==Phase::Outcome)
  {
   SawOutcome=true;
   T->TestTrue(TEXT("Outcome occurs after mandatory reveal"),!S->IsInlineFormulaRevealInputBlocked());
   T->TestTrue(TEXT("Result text waits for outcome"),S->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()==ESlateVisibility::Collapsed);
   if(Round==1 && !FlightCaptured && State.Progress()>.3f && State.Progress()<.75f)
   {Capture(TEXT("04_Goal_Flight.png"));FlightCaptured=true;}
  }
  if(S->IsInlineFormulaRevealInputBlocked())
  {
   if(Step==6 || Step==7)
   {
    SawRoll=true;
    T->TestTrue(TEXT("Ball waits for Roll v2"),State.Phase==Phase::FormulaHold);
   }
   if(Round==2 && Step==7)
   {
    auto* A=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterPairAReel")));
    auto* B=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterPairBReel")));
    SawPairA |= A->GetPresentation().bMoving;
    SawPairB |= B->GetPresentation().bMoving;
    T->TestTrue(TEXT("DeadCorner retains commercial ownership through sequential reveal"),S->GetWidgetFromName(TEXT("ResolutionTheater"))->GetVisibility()!=ESlateVisibility::Collapsed);
    T->TestFalse(TEXT("Pair has no attribute formula"),S->GetPresentation().LongShotResolution.Formula.bVisible);
    if(B->GetPresentation().bMoving && !PairCaptured){Capture(TEXT("05_DeadCorner_SecondDie.png"));PairCaptured=true;}
   }
   return false;
  }
  if(FPlatformTime::Seconds()-Changed<.1) return false;
  if(Step==0){S->RequestStartNewMatch();Next();return false;}
  if(Step==1){if(!Roll(EFMCodexLocalDevRollTarget::FullD12,3))return true;S->RequestRollTacticalPoints();Next();return false;}
  if(Step==2)
  {
   const auto& V=C->GetInteractionView();
   if(V.MajorPhase!=EFMCodexLocalMatchMajorPhase::Deployment){Next();return false;}
   const bool A=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA;
   Carrier=A?TEXT("Prototype.Arsenal.DeclanRice"):TEXT("Prototype.ManchesterCity.OmarMarmoush");
   const bool Attack=V.CurrentLegalDeploymentSide==V.CurrentAttackingPlayer;
   const bool CarrierPresent=V.DeploymentPlacements.ContainsByPredicate([&](const auto& P){return P.CardId==Carrier;});
   const FString Half=A?TEXT("NearA"):TEXT("NearB");
   const auto* O=V.DeploymentOptions.FindByPredicate([&](const auto& X)
   {return Round==0 && !Attack && !KeeperDeployed && X.bGoalkeeper;});
   if(O){const auto Slot=O->SlotId;S->RequestDeployGoalkeeper(Slot);KeeperDeployed=true;}
   else
   {
    O=V.DeploymentOptions.FindByPredicate([&](const auto& X){return !X.bGoalkeeper && X.SlotId.ToString().Contains(Half) && (!Attack || CarrierPresent || X.CardId==Carrier);});
    if(O){const auto Card=O->CardId,Slot=O->SlotId;S->RequestDeployOrdinary(Card,Slot);}
    else if(V.bCanFinishDeployment) S->RequestFinishDeployment();
    else {T->AddError(TEXT("No canonical deployment option"));return true;}
   }
   if(!T->TestTrue(TEXT("Normal deployment accepted"),C->GetLastDiagnostic().bHostSuccess))return true;
   Changed=FPlatformTime::Seconds();return false;
  }
  if(Step==3)
  {
   const auto& V=C->GetInteractionView();
   switch(V.InteractionCategory)
   {
   case Category::SelectCarrier:S->RequestSubmitCarrier(Carrier);break;
   case Category::SelectMarker:if(V.SelectionOptions.IsEmpty()){T->AddError(TEXT("No legal marker"));return true;}S->RequestSubmitMarker(V.SelectionOptions[0].Id);break;
   case Category::SelectRunner:case Category::SelectHelper:if(V.bCanResolveNoLegalChoice)S->RequestResolveNoLegalSelection();else S->RequestDeclineSelection();break;
   case Category::SelectSkill:
   {
    const auto* O=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([](const auto& X){return X.SkillType==ESkillRuleType::LongShot && X.bEnabled;});
    if(!T->TestNotNull(TEXT("Actual alternate production player has legal LongShot"),O))return true;
    S->RequestSubmitSkill(O->OptionId);Next();return false;
   }
   default:T->AddError(FString::Printf(TEXT("Unexpected role selection phase category=%d major=%d round=%d diagnostic=%s"),
    int(V.InteractionCategory),int(V.MajorPhase),Round,*C->GetLastDiagnostic().Message));return true;
   }
   Changed=FPlatformTime::Seconds();return false;
  }
  if(Step==4)
  {
   auto* Direct=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearDirect")));
   auto* Corner=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearCombination")));
   auto* Host=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
   if(PreviewStep==0)
   {
    T->TestEqual(TEXT("Preview precedes method commit"),State.Phase,Phase::Preview);
    PreviewSurface=Host->GetContent();PreviewRollCount=C->GetInteractionView().AcceptedRolls.Num();
    Direct->TakeWidget()->OnMouseEnter(Direct->GetCachedGeometry(),FPointerEvent());
    PreviewStep=1;Changed=FPlatformTime::Seconds();return false;
   }
   if(PreviewStep==1)
   {
    const auto& G=Host->GetCachedGeometry();const auto& RootG=S->GetCachedGeometry();
    PreviewPosition=RootG.AbsoluteToLocal(G.GetAbsolutePosition());
    PreviewSize=RootG.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()))-PreviewPosition;
    PreviewGeometryValid=true;
    T->TestTrue(TEXT("Hover previews Direct without committing"),State.PreviewMethod==FMCodexTacticalScene::EMethod::Direct);
    if(Round==0)Capture(TEXT("01_Direct_Preview.png"));
    Direct->TakeWidget()->OnMouseLeave(FPointerEvent());Corner->SetKeyboardFocus();
    PreviewStep=2;Changed=FPlatformTime::Seconds()+.1;return false;
   }
   if(PreviewStep==2)
   {
    T->TestTrue(TEXT("Keyboard focus previews corner"),State.PreviewMethod==FMCodexTacticalScene::EMethod::DeadCorner && State.CornerBlend>.99f);
    T->TestTrue(TEXT("Same Slate scene retained across previews"),PreviewSurface==Host->GetContent());
    T->TestEqual(TEXT("Hover/focus consumes no accepted roll"),C->GetInteractionView().AcceptedRolls.Num(),PreviewRollCount);
    T->TestEqual(TEXT("Hover/focus leaves authoritative method selection pending"),C->GetInteractionView().InteractionCategory,Category::SelectLongShotBranch);
    if(Round==0)Capture(TEXT("02_Corner_Preview.png"));
    // Synchronous screenshot readback stalls this frame. Commit on a later
    // frame so that capture cost is not mistaken for time after the click.
    PreviewStep=3;Changed=FPlatformTime::Seconds();return false;
   }
   (Round==2?Corner:Direct)->OnClicked.Broadcast();
   SawSetup=State.Phase==Phase::Setup;
   T->TestTrue(TEXT("Real method click starts the short setup beat"),SawSetup);
   const auto& SelectedFacts=C->GetInteractionView().ResolutionFacts;
   if(!T->TestTrue(*FString::Printf(TEXT("Real selected method safe projection: %s"),*SelectedFacts.ErrorMessage),
    SelectedFacts.bSuccess && SelectedFacts.bHasActualBranch))return true;
   if(Round==2)
   {
    T->TestEqual(TEXT("Real method remains DeadCorner"),SelectedFacts.ActualBranch.LongShot,EMatchPlayLongShotActualBranch::DeadCorner);
    T->TestTrue(TEXT("DeadCorner has no authoritative Formula"),SelectedFacts.FormulaContests.IsEmpty());
    T->TestEqual(TEXT("DeadCorner starts at paired A"),SelectedFacts.NextPendingRollSequenceIndex,0);
   }
   Next();return false;
  }
  if(Step==5)
  {
   if(State.Phase!=Phase::FormulaHold) return false;
   T->TestTrue(TEXT("Natural setup and intent observed"),SawSetup && SawIntent);
   T->TestEqual(TEXT("Actual carrier retained"),State.Facts.Participants[0].CardId,Carrier);
   T->TestEqual(TEXT("Spatial keeper always present"),State.Facts.Participants.Num(),3);
   const auto* GK=State.Facts.Participants.FindByPredicate([](const auto& P){return P.Role==EMatchPlayResolutionParticipantRole::Goalkeeper;});
   T->TestTrue(TEXT("GK Formula-active versus spatial-only distinction"),GK && GK->bFormulaActive==(Round==0));
   if(Round==2)
   {
    auto Observe=[&](const TCHAR* Moment)
    {
     const auto& V=C->GetInteractionView();const auto& F=V.ResolutionFacts;
     T->AddInfo(FString::Printf(TEXT("DEAD_FACTS %s active=%d action=%d intent=%d terminal=%d success=%d has=%d branch=%d rolls=%d decisions=%d next=%d error=%s"),
      Moment,V.bCurrentAttackActive,int(V.PresentedActionType),int(V.ElectiveBranchIntent),V.bTerminalPendingAdvance,F.bSuccess,F.bHasFacts,F.bHasActualBranch,F.Rolls.Num(),F.Decisions.Num(),F.NextPendingRollSequenceIndex,*F.ErrorMessage));
    };
    Observe(TEXT("before"));
    T->TestEqual(TEXT("Accepted method enters DeadCorner without STEP"),S->GetPresentation().LongShotResolution.Stage,EFMCodexUMGLongShotStage::DeadCorner);
    if(!Roll(EFMCodexLocalDevRollTarget::LongShotDeadCornerA,6) || !Roll(EFMCodexLocalDevRollTarget::LongShotDeadCornerB,5))return true;
    Capture(TEXT("05_DeadCorner_Entry.png"));
    CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
    Observe(TEXT("after"));
    const auto& CompletedFacts=C->GetInteractionView().ResolutionFacts;
    if(!T->TestTrue(TEXT("Real terminal facts succeed without an error"),CompletedFacts.bSuccess && CompletedFacts.ErrorMessage.IsEmpty()))return true;
    T->TestTrue(TEXT("Terminal still has no fake attribute Formula"),CompletedFacts.FormulaContests.IsEmpty());
    const auto* Decision=CompletedFacts.Decisions.FindByPredicate([](const auto& Fact){return Fact.DecisionId==TEXT("DeadCorner.Outcome");});
    T->TestTrue(TEXT("Real typed procedural Goal reaches safe terminal"),Decision && Decision->bResolved && Decision->Outcome==EMatchPlayResolutionDecisionOutcome::Goal);
    const auto& D=C->GetLastDiagnostic();
    T->AddInfo(FString::Printf(TEXT("DEAD_REQUEST host=%d accepted=%d command=%s message=%s phase=%d scene=%d"),D.bHostSuccess,D.bAuthoritativeAccepted,*D.CommandName,*D.Message,int(S->GetInlineFormulaRevealPhase()),int(S->GetTacticalScene().Phase)));
    if(!T->TestTrue(TEXT("Actual DeadCorner CTA submits accepted roll"),D.bHostSuccess && D.CommandName==TEXT("ResolveLongShotDeadCornerRoll"))) return true;
    if(!T->TestTrue(TEXT("Accepted procedural result retains safe commercial projection"),S->GetPresentation().LongShotResolution.bVisible)
     || !T->TestTrue(TEXT("Actual procedural result starts mandatory reveal"),S->IsInlineFormulaRevealInputBlocked())) return true;
    Step=7;Changed=FPlatformTime::Seconds();return false;
   }
   if(!Roll(EFMCodexLocalDevRollTarget::LongShotDirectAttack,Round==0?4:6))return true;
   S->RequestContinueResolution();Next();return false;
  }
  if(Step==6)
  {
   if(!Roll(EFMCodexLocalDevRollTarget::LongShotDirectDefense,Round==0?6:1))return true;
   S->RequestContinueResolution();Next();return false;
  }
  if(Step==7)
  {
   if(State.Phase!=Phase::ResultHold)return false;
   if(ResultReady==0){ResultReady=FPlatformTime::Seconds();return false;}
   if(FPlatformTime::Seconds()-ResultReady<.5) return false;
   const auto* Scene=S->GetWidgetFromName(TEXT("TacticalScene"));
   const auto& SG=Scene->GetCachedGeometry();
   const auto& TitleG=S->GetWidgetFromName(TEXT("TheaterOutcome"))->GetCachedGeometry();
   const auto& ButtonG=S->GetWidgetFromName(TEXT("TheaterContinue"))->GetCachedGeometry();
   T->AddInfo(FString::Printf(TEXT("SPATIAL_BOUNDS scene=%s size=%s titleBottom=%s buttonBottom=%s"),*SG.GetAbsolutePosition().ToString(),*SG.GetLocalSize().ToString(),
    *TitleG.LocalToAbsolute(TitleG.GetLocalSize()).ToString(),*ButtonG.LocalToAbsolute(ButtonG.GetLocalSize()).ToString()));
   T->TestTrue(TEXT("Field begins below title"),SG.GetAbsolutePosition().Y>=TitleG.LocalToAbsolute(TitleG.GetLocalSize()).Y-1);
   const auto& Root=S->GetCachedGeometry();
   T->TestTrue(TEXT("Continue remains inside viewport"),Root.AbsoluteToLocal(ButtonG.LocalToAbsolute(ButtonG.GetLocalSize())).Y<=Root.GetLocalSize().Y);
   T->TestTrue(TEXT("Natural roll and outcome observed"),SawRoll && SawOutcome);
   T->TestTrue(TEXT("Provider goes through actual failure / goal resolution"),State.Facts.Outcome==(Round==0?FMCodexTacticalScene::EOutcome::DefensiveSuccess:FMCodexTacticalScene::EOutcome::Goal));
   T->TestTrue(TEXT("Narrative is prominent at the top"),S->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()!=ESlateVisibility::Collapsed
    && !CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().IsEmpty());
   const auto Endpoint=FMCodexTacticalScene::OutcomeTarget(State.Facts.Outcome,State.Facts.Method);
   T->TestEqual(TEXT("Goal lands inside; failure outside"),FMCodexTacticalScene::IsInsideGoal(Endpoint),Round!=0);
   if(Round==2)T->TestTrue(TEXT("Both procedural dice revealed naturally"),SawPairA && SawPairB);
   Capture(Round==0?TEXT("03_Failure_1600.png"):Round==1?TEXT("04_Goal_1920.png"):TEXT("06_DeadCorner_Result.png"));
   T->AddInfo(FString::Printf(TEXT("TACTICAL_SCENE_PIE round=%d carrier=%s participants=%d setup=%d intent=%d roll=%d outcome=%d result=1 geometryPhases=%d"),Round,*Carrier.ToString(),State.Facts.Participants.Num(),SawSetup,SawIntent,SawRoll,SawOutcome,CheckedGeometry.Num()));
   CompletedSequence=C->GetInteractionView().AttackSequence;
   S->RequestContinueResolution();Next();return false;
  }
  T->TestTrue(TEXT("Canonical continuation clears scene"),S->GetTacticalScene().Phase==Phase::Hidden);
  T->TestTrue(TEXT("Canonical continuation leaves completed attack"),!C->GetInteractionView().bCurrentAttackActive || C->GetInteractionView().AttackSequence>CompletedSequence);
  if(Round<2)
  {
   ++Round;RoundStart=FPlatformTime::Seconds();ResultReady=0;PreviewStep=0;
   PreviewGeometryValid=false;CheckedGeometry.Reset();
   // Direct failure -> opposite-side goal still verifies actual continuation.
   // The independent DeadCorner case starts with a fresh roster: recovery is
   // random and does not guarantee the first round's named carrier is returned.
   Step=Round==2?0:1;KeeperDeployed=false;SawSetup=SawIntent=SawOutcome=SawRoll=false;
   SpatialWindow->Resize(FVector2D(1920,1080));Changed=FPlatformTime::Seconds();return false;
  }
  return true;
 }
private:
 void Capture(const TCHAR* Name)
 {
  auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();TArray<FColor> Pixels;FIntVector Size=FIntVector::ZeroValue;
  if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)){T->AddError(TEXT("PIE capture failed"));return;}
  const FString Dir=FPaths::ProjectSavedDir()/TEXT("Stage8_23A/BlockerFixPIE");IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);FFileHelper::SaveArrayToFile(PNG,*(Dir/Name));
 }
 FAutomationTestBase* T;double RoundStart=FPlatformTime::Seconds(),Changed=0,ResultReady=0;
 int Step=0,Round=0,PreviewStep=0,PreviewRollCount=0;FName Carrier;
 int64 CompletedSequence=INDEX_NONE;
 TSharedPtr<SWidget> PreviewSurface;
 FVector2D PreviewPosition,PreviewSize;
 TArray<FMCodexTacticalScene::EPhase> CheckedGeometry;
 bool PreviewGeometryValid=false,FlightCaptured=false;
 bool SawPairA=false,SawPairB=false,PairCaptured=false;
 bool KeeperDeployed=false,SawSetup=false,SawIntent=false,SawOutcome=false,SawRoll=false,Skipped=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpatialPIETest,"FMCodex.PIE.TacticalScene.LongShotCommercialFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSpatialPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartSpatialPIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlaySpatialPIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
