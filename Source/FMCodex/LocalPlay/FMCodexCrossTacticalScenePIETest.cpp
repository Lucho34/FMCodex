#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/NativeWidgetHost.h"
#include "Components/TextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
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
namespace
{
TSharedPtr<SWindow> CrossWindow;
class FStartCrossScenePIE final : public IAutomationLatentCommand
{
 bool Update() override
 {
  auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
  const int32 Width=FParse::Param(FCommandLine::Get(),TEXT("CrossLayoutRefinement"))?1720:1600;
  Settings->NewWindowWidth=Width;Settings->NewWindowHeight=900;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);Settings->SetPlayNumberOfClients(1);
  CrossWindow=SNew(SWindow).Title(FText::FromString(TEXT("Cross Tactical Scene Local PIE"))).ClientSize(FVector2D(Width,900))
   .AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0)).SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
  FSlateApplication::Get().AddWindow(CrossWindow.ToSharedRef());
  FRequestPlaySessionParams P;P.EditorPlaySettings=Settings;P.CustomPIEWindow=CrossWindow;GEditor->RequestPlaySession(P);return true;
 }
};
// One Local PIE world; actual typed UI actions, canonical roster, server-owned
// DEV dice only. No direct state writes, forced winners, or reveal-clock jumps.
class FPlayCrossScenePIE final : public IAutomationLatentCommand
{
public:
 explicit FPlayCrossScenePIE(FAutomationTestBase* Test,bool OutcomePolish=false,bool Broadcast=false):T(Test),bOutcomePolish(OutcomePolish),bBroadcast(Broadcast){}
 bool Update() override
 {
  using namespace FMCodexTacticalScene;
  using Category=EFMCodexLocalMatchInteractionCategory;
  using Role=EMatchPlayResolutionParticipantRole;
  const bool CrossRound=bOutcomePolish || Round<2;
  const bool HighRound=(Round==0 || (bOutcomePolish && Round==2))
   && !(bBroadcast && FParse::Param(FCommandLine::Get(),TEXT("CrossBroadcastLow")));
  const bool FailRound=bBroadcast?!FParse::Param(FCommandLine::Get(),TEXT("CrossBroadcastGoal"))
   :(bOutcomePolish?Round<2:Round==1);
  if(FPlatformTime::Seconds()-Started>220){T->AddError(FString::Printf(TEXT("Cross PIE timeout round=%d step=%d"),Round,Step));return true;}
  if(!GEditor || !GEditor->PlayWorld)return false;
  auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
  auto* S=C?C->GetPlayerMatchScreen():nullptr;if(!S)return false;
  const auto& V=C->GetInteractionView();const auto& Scene=S->GetTacticalScene();
  auto Next=[&](){++Step;Changed=FPlatformTime::Seconds();};
  if(Scene.Phase==EPhase::Setup)SawSetup=true;
  if(Scene.Phase==EPhase::Intent)
  {
   SawIntent=true;
   if(!bBroadcast && Round<2 && !Skipped && Scene.Progress()>.25f)
   {
    const auto Rolls=V.AcceptedRolls.Num();const auto CategoryBefore=V.InteractionCategory;
    auto* H=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
    H->GetContent()->OnKeyDown(H->GetCachedGeometry(),FKeyEvent(EKeys::SpaceBar,FModifierKeysState(),0,false,0,0));
    T->TestEqual(TEXT("Space only accelerates current spatial beat"),S->GetTacticalScene().Phase,EPhase::FormulaHold);
    T->TestEqual(TEXT("Space cannot submit gameplay"),V.InteractionCategory,CategoryBefore);
    T->TestEqual(TEXT("Space consumes no roll"),V.AcceptedRolls.Num(),Rolls);Skipped=true;
   }
  }
  if(Scene.Phase==EPhase::Outcome)
  {
   SawOutcome=true;
   if(bBroadcast && OutcomeStarted==0)OutcomeStarted=FPlatformTime::Seconds();
   if(bBroadcast && Scene.IsCrossOutcomeVisualHold())SawVisualHold=true;
   T->TestFalse(TEXT("Spatial outcome waits until reel completed"),S->IsInlineFormulaRevealInputBlocked());
   T->TestEqual(TEXT("Headline still hidden during ball arrival"),S->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility(),ESlateVisibility::Collapsed);
   // Hot-seat intentionally reorients the header when the acting side changes.
   // Check the displayed A/B scores, not the order of the context sentence.
   const auto& Header=S->GetMatchHeader()->GetPresentation();
   T->TestEqual(TEXT("Visible A score waits for spatial arrival"),Header.PlayerAScoreLabel,ScoreBeforeA);
   T->TestEqual(TEXT("Visible B score waits for spatial arrival"),Header.PlayerBScoreLabel,ScoreBeforeB);
   if(bBroadcast)
   {
    const bool LeftA=Header.LeftPlayerSide==EInitialTurnOrderPlayer::PlayerA;
    const FString Score=FString::Printf(TEXT("%s : %s"),*(LeftA?ScoreBeforeA:ScoreBeforeB),*(LeftA?ScoreBeforeB:ScoreBeforeA));
    T->TestEqual(TEXT("Broadcast scorebug also waits for spatial arrival"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossScoreValue")))->GetText().ToString(),Score);
    T->TestEqual(TEXT("Broadcast headline remains gated"),S->GetWidgetFromName(TEXT("CrossBroadcastOutcome"))->GetVisibility(),ESlateVisibility::Collapsed);
    T->TestEqual(TEXT("Broadcast reason remains gated"),S->GetWidgetFromName(TEXT("CrossBroadcastResultReason"))->GetVisibility(),ESlateVisibility::Collapsed);
   }
   if(bOutcomePolish)
   {
    if(Scene.IsCrossOutcomeVisualHold())
    {
     T->TestEqual(TEXT("Outcome action has settled before visual hold"),Scene.CrossOutcomeActionProgress(),1.f);
     T->TestTrue(TEXT("Readable final endpoint"),CrossOutcomeBall(Scene,1.f).Equals(OutcomeTarget(Scene.Facts.Outcome,Scene.Facts.Method)));
     if(!SawVisualHold && Round!=1)Capture(Round==0?TEXT("Outcome_HighDefenseHold.png"):TEXT("Outcome_GoalHold.png"));
     SawVisualHold=true;
    }
    if(Round==1 && !OutcomeSkipped && Scene.CrossOutcomeActionProgress()>.85f)
    {
     const int Rolls=V.AcceptedRolls.Num();const auto CategoryBefore=V.InteractionCategory;
     const auto End=CrossOutcomeBall(Scene,1.f);
     auto* H=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
     H->GetContent()->OnKeyDown(H->GetCachedGeometry(),FKeyEvent(EKeys::SpaceBar,FModifierKeysState(),0,false,0,0));
     T->TestEqual(TEXT("Outcome Space lands in final spatial state"),Scene.Phase,EPhase::ResultHold);
     T->TestTrue(TEXT("Outcome Space preserves ball endpoint"),CrossOutcomeBall(Scene,1.f).Equals(End));
     T->TestEqual(TEXT("Outcome Space does not submit continuation"),V.InteractionCategory,CategoryBefore);
     T->TestEqual(TEXT("Outcome Space consumes no roll"),V.AcceptedRolls.Num(),Rolls);OutcomeSkipped=true;
    }
   }
  }
  if(bBroadcast && Step==4)
  {
   if(BroadcastPaintTime==0 && Scene.CrossEntrance.StablePaints>=2)BroadcastPaintTime=FPlatformTime::Seconds();
   if(V.AcceptedRolls.Num()==BroadcastRolls)return false;
   T->TestTrue(TEXT("Actual ready paints precede the readable intro and hold"),BroadcastPaintTime>0 && FPlatformTime::Seconds()-BroadcastPaintTime>=CrossIntroSeconds+CrossIntroHoldSeconds-.1);
   T->TestTrue(TEXT("Real rendered scene clock completed before route"),Scene.CrossEntrance.Elapsed>=CrossIntroSeconds+CrossIntroHoldSeconds);
   T->AddInfo(FString::Printf(TEXT("CROSS_ENTRY ready_after_click=%.3fs entrance_clock=%.3fs ready_to_route_observed=%.3fs"),BroadcastPaintTime-BroadcastChoiceTime,Scene.CrossEntrance.Elapsed,FPlatformTime::Seconds()-BroadcastPaintTime));
   T->TestEqual(TEXT("Exactly one intermediate die"),V.AcceptedRolls.Num(),BroadcastRolls+1);
   // Narrow hitch diagnosis reuses this fixture and stops after the route;
   // it is not evidence for the later final-roll/result portion of the smoke.
   if(FParse::Param(FCommandLine::Get(),TEXT("CrossEntryOnly")))return true;
   Step=6;
  }
  if(bBroadcast && Step==6 && Scene.Facts.bRoutePending
   && S->IsInlineFormulaRevealInputBlocked() && S->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
  {
   if(RouteReadStart==0)
   {
    RouteReadStart=FPlatformTime::Seconds(); Capture(TEXT("Broadcast_RouteReadability.png"));
    T->TestEqual(TEXT("Both ranges remain visible during the original route result hold"),S->GetWidgetFromName(TEXT("CrossRouteContextBounds"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
    T->TestEqual(TEXT("No manual formula during route reading"),S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
    T->TestTrue(TEXT("Central route notice is visible during disclosed reading"),S->GetWidgetFromName(TEXT("CrossRouteNotice"))->GetVisibility()!=ESlateVisibility::Collapsed);
   }
  }
  if(bBroadcast && RouteReadStart>0 && RouteReadEnd==0 && !Scene.Facts.bRoutePending)RouteReadEnd=FPlatformTime::Seconds();
  if(S->IsInlineFormulaRevealInputBlocked())return false;
  if(FPlatformTime::Seconds()-Changed<.15)return false;
  if(Step==0){S->RequestStartNewMatch();Next();return false;}
  if(Step==1)
  {
   // The previous attack may still be displaying its timed recovery notice.
   // Wait for the normal handoff; do not dismiss it or jump a reveal clock.
   if(C->IsRecoveryNotificationDismissScheduledForTesting() || S->IsScreenRequestPending()
    || Scene.IsAnimating() || !S->GetPresentation().Interaction.bCanRollTacticalPoints)return false;
   if(!Roll(*C,EFMCodexLocalDevRollTarget::FullD12,CrossRound?4:3))return true;
   S->RequestRollTacticalPoints();
   if(!Accepted(*C))return true;
   if(!T->TestEqual(TEXT("Tactical roll enters canonical deployment"),V.MajorPhase,EFMCodexLocalMatchMajorPhase::Deployment))return true;
   Next();return false;
  }
  if(Step==2)
  {
   if(V.MajorPhase!=EFMCodexLocalMatchMajorPhase::Deployment)
   {
    if(!T->TestEqual(TEXT("Deployment hands off to selection"),V.MajorPhase,EFMCodexLocalMatchMajorPhase::Selection))return true;
    Next();return false;
   }
   const bool A=V.CurrentAttackingPlayer==EInitialTurnOrderPlayer::PlayerA;
   Carrier=!CrossRound?FName(TEXT("Prototype.Arsenal.DeclanRice")):FName(A?TEXT("Prototype.Arsenal.BukayoSaka"):TEXT("Prototype.ManchesterCity.RayanAitNouri"));
   Runner=A?TEXT("Prototype.Arsenal.ChristianNorgaard"):TEXT("Prototype.ManchesterCity.Rodri");
   // Prior Cross participants have already spent stamina in this same match.
   // Calafiori has canonical Cross at 4-5 TP; Rice's skills do not include Cross.
   // Deployment and subsequent skill selection still use real legal options.
   if(bOutcomePolish && Round==2){Carrier=TEXT("Prototype.Arsenal.RiccardoCalafiori");Runner=TEXT("Prototype.Arsenal.KaiHavertz");}
   const bool Attack=V.CurrentLegalDeploymentSide==V.CurrentAttackingPlayer;
   const FString Own=A?TEXT("NearA"):TEXT("NearB"),Forward=A?TEXT("NearB"):TEXT("NearA");
   if(Attack)
   {
    const bool HasCarrier=V.DeploymentPlacements.ContainsByPredicate([&](const auto& P){return P.CardId==Carrier;});
    const bool HasRunner=V.DeploymentPlacements.ContainsByPredicate([&](const auto& P){return P.CardId==Runner;});
    if(!HasCarrier){if(!Deploy(*C,*S,Own,Carrier))return true;}
    else if(CrossRound && !HasRunner){if(!Deploy(*C,*S,Forward,Runner))return true;}
    else S->RequestFinishDeployment();
   }
   else if(Round==0 && !Keeper)
   {
    const auto* O=V.DeploymentOptions.FindByPredicate([](const auto& X){return X.bGoalkeeper;});
    if(!T->TestNotNull(TEXT("Canonical GK activation option"),O))return true;
    const FName Slot=O->SlotId;S->RequestDeployGoalkeeper(Slot);Keeper=true;
   }
   else if(Defenders<(CrossRound?3:1))
   {if(!Deploy(*C,*S,Defenders==0?Own:Forward))return true;++Defenders;}
   else S->RequestFinishDeployment();
   if(!Accepted(*C))return true;Changed=FPlatformTime::Seconds();return false;
  }
  if(Step==3)
  {
   switch(V.InteractionCategory)
   {
   case Category::SelectCarrier:S->RequestSubmitCarrier(Carrier);break;
   case Category::SelectMarker:
    if(V.SelectionOptions.IsEmpty()){T->AddError(TEXT("Missing canonical Marker"));return true;}
    Marker=V.SelectionOptions[0].Id;S->RequestSubmitMarker(Marker);break;
   case Category::SelectRunner:
    if(CrossRound)S->RequestSubmitRunner(Runner);
    else if(V.bCanResolveNoLegalChoice)S->RequestResolveNoLegalSelection();else S->RequestDeclineSelection();break;
   case Category::SelectHelper:
    if(CrossRound)
    {
     if(V.SelectionOptions.IsEmpty()){T->AddError(TEXT("Missing canonical Helper"));return true;}
     Helper=V.SelectionOptions[0].Id;S->RequestSubmitHelper(Helper);
    }
    else if(V.bCanResolveNoLegalChoice)S->RequestResolveNoLegalSelection();else S->RequestDeclineSelection();break;
   case Category::SelectSkill:
   {
    const auto* O=V.SelectionOptions.FindByPredicate([&](const auto& X){return X.SkillType==(CrossRound?ESkillRuleType::Cross:ESkillRuleType::LongShot);});
    if(!T->TestNotNull(TEXT("Canonical requested skill"),O))return true;
    const FName Id=O->Id;
    if(bBroadcast)
    {
     if(!Roll(*C,EFMCodexLocalDevRollTarget::CrossRoute,2))return true;
     const auto* Button=S->GetInteractionPanel()->GetRenderedOptionWidgets().FindByPredicate([&](const auto& X){return X->GetLabel()==(HighRound?TEXT("传中（高球）"):TEXT("传中（低球）"));});
     if(!T->TestNotNull(TEXT("Direct Cross intent in original dock"),Button))return true;
     BroadcastRolls=V.AcceptedRolls.Num();BroadcastChoiceTime=FPlatformTime::Seconds();
     CastChecked<UButton>((*Button)->GetWidgetFromName(TEXT("InteractionOptionButton")))->OnClicked.Broadcast();
     T->AddInfo(FString::Printf(TEXT("CROSS_ENTRY click_handler=%.1fms"),(FPlatformTime::Seconds()-BroadcastChoiceTime)*1000));
     T->TestEqual(TEXT("No repeated branch choice"),S->GetWidgetFromName(TEXT("TheaterChoices"))->GetVisibility(),ESlateVisibility::Collapsed);
    }
    else S->RequestSubmitSkill(Id);
    Next();return false;
   }
   default:T->AddError(FString::Printf(TEXT("Unexpected selection category %d, round=%d, last=%s: %s"),int(V.InteractionCategory),Round,*C->GetLastDiagnostic().CommandName,*C->GetLastDiagnostic().Message));return true;
   }
   if(!Accepted(*C))return true;Changed=FPlatformTime::Seconds();return false;
  }
  if(Step==4)
  {
   if(!CrossRound){CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterNearDirect")))->OnClicked.Broadcast();Step=6;return false;}
   auto* High=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterHigh")));
   auto* Low=CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterLow")));
   // Outcome coverage selects the real branch directly. Hover/focus behavior
   // belongs to the existing commercial-flow test, not this follow-up's budget.
   if(bOutcomePolish)
   {
    (HighRound?High:Low)->OnClicked.Broadcast();if(!Accepted(*C))return true;Next();return false;
   }
   auto* Host=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
   if(Preview==0)
   {
    T->TestEqual(TEXT("Actual method choice has spatial preview"),Scene.Phase,EPhase::Preview);
    Persistent=Host->GetContent();PreviewRolls=V.AcceptedRolls.Num();PreviewSequence=V.AttackSequence;
    High->TakeWidget()->OnMouseEnter(High->GetCachedGeometry(),FPointerEvent());++Preview;Changed=FPlatformTime::Seconds()+.55;return false;
   }
   if(Preview==1)
   {
    T->TestEqual(TEXT("High pointer hover previews aerial route"),Scene.PreviewMethod,EMethod::CrossHigh);
    if(Round==0 && !bOutcomePolish)Capture(TEXT("01_HighPreview.png"));
    High->TakeWidget()->OnMouseLeave(FPointerEvent());Low->SetKeyboardFocus();++Preview;Changed=FPlatformTime::Seconds()+.55;return false;
   }
   if(Preview==2)
   {
    T->TestTrue(TEXT("Low focus previews speed route"),Scene.PreviewMethod==EMethod::CrossLow && Scene.CrossLowBlend>.99f);
    T->TestTrue(TEXT("Persistent scene survives High/Low"),Persistent==Host->GetContent());
    T->TestEqual(TEXT("Preview leaves gameplay selection pending"),V.InteractionCategory,Category::SelectBranchIntent);
    T->TestEqual(TEXT("Preview does not consume RNG"),V.AcceptedRolls.Num(),PreviewRolls);
    T->TestEqual(TEXT("Preview keeps attack identity"),V.AttackSequence,PreviewSequence);
    if(Round==0 && !bOutcomePolish)Capture(TEXT("02_LowPreview.png"));
    ++Preview;Changed=FPlatformTime::Seconds();return false;
   }
   (HighRound?High:Low)->OnClicked.Broadcast();if(!Accepted(*C))return true;Next();return false;
  }
  if(Step==5)
  {
   if(!Roll(*C,EFMCodexLocalDevRollTarget::CrossRoute,2))return true;
   CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();if(!Accepted(*C))return true;Next();return false;
  }
  if(Step==6)
  {
   if(Scene.Phase!=EPhase::FormulaHold)return false;
   if(bBroadcast)
   {
    if(ManualWait==0)
    {
     // Synchronous screenshot/frame stalls can lengthen wall time. Exact game-
     // clock completion is checked by CrossBroadcastFlow/PurposeBasedPacing;
     // here prove that the real visible reading interval is not cut short.
     T->TestTrue(TEXT("Cross route result remains readable for at least the requested hold"),RouteReadStart>0 && RouteReadEnd-RouteReadStart>=2.4);
     T->AddInfo(FString::Printf(TEXT("CROSS_ROUTE_READ hold=%.3fs"),RouteReadEnd-RouteReadStart));
     ManualWait=FPlatformTime::Seconds(); IdleStart=Scene.CrossIdleTime;
     IdleRunner=CrossParticipantAnchor(Scene,EMatchPlayResolutionParticipantRole::Runner);IdleBall=CrossIntentBall(Scene);
     Capture(TEXT("Broadcast_AttackManual.png"));return false;
    }
    const double Waited=FPlatformTime::Seconds()-ManualWait;
    if(!IdleMotionCaptured && Waited>=1.5)
    {
     T->TestTrue(TEXT("Real painted waiting state remains active"),Scene.IsCrossIdle() && Scene.CrossEntrance.RecentlyPainted(GFrameCounter));
     const float RunnerTravel=(CrossParticipantAnchor(Scene,EMatchPlayResolutionParticipantRole::Runner)-IdleRunner).Size();
     const float BallTravel=(CrossIntentBall(Scene)-IdleBall).Size();
     T->TestTrue(TEXT("PIE visibly replays player approach and ball delivery"),RunnerTravel>50.f && BallTravel>40.f);
     T->AddInfo(FString::Printf(TEXT("CROSS_REPLAY runner=%.1f ball=%.1f"),RunnerTravel,BallTravel));
     Capture(TEXT("Broadcast_AttackIdleMotion.png"));IdleMotionCaptured=true;
    }
    if(Waited<CrossIdleCycleSeconds*2+.4f)return false;
    // Resize invalidates cached child geometry for one Slate layout pass.
    // Measure the settled viewport, not a mixture of old and new allocations.
    if(NarrowStarted>0 && FPlatformTime::Seconds()-NarrowStarted<.4)return false;
    T->TestTrue(TEXT("Manual waiting animates the persistent scene"),Scene.IsCrossIdle() && Scene.CrossIdleTime!=IdleStart);
    const auto& ScreenGeometry=S->GetCachedGeometry();
    for(const auto Name:{TEXT("TheaterSpatialLane"),TEXT("CrossBroadcastBounds")})
    {
     const auto& Box=S->GetWidgetFromName(Name)->GetCachedGeometry();
     const auto Min=ScreenGeometry.AbsoluteToLocal(Box.LocalToAbsolute(FVector2D::ZeroVector));
     const auto Max=ScreenGeometry.AbsoluteToLocal(Box.LocalToAbsolute(Box.GetLocalSize()));
     T->TestTrue(TEXT("Cross field and footer use the entire usable width"),FMath::Abs(Min.X)<2 && FMath::Abs(Max.X-ScreenGeometry.GetLocalSize().X)<2);
    }
    T->TestEqual(TEXT("Natural wait cannot auto-roll final attack"),V.AcceptedRolls.Num(),BroadcastRolls+1);
    auto* Row=CastChecked<UHorizontalBox>(S->GetWidgetFromName(TEXT("CrossBroadcastRow")));
    const auto& G=Row->GetCachedGeometry();
    for(int I=0;I<Row->GetChildrenCount();++I)
    {
     const auto& Child=Row->GetChildAt(I)->GetCachedGeometry();
     const auto A=G.AbsoluteToLocal(Child.LocalToAbsolute(FVector2D::ZeroVector));
     const auto B=G.AbsoluteToLocal(Child.LocalToAbsolute(Child.GetLocalSize()));
     T->TestTrue(TEXT("Broadcast groups fit without clipping"),A.X>=-1 && A.Y>=-1 && B.X<=G.GetLocalSize().X+1 && B.Y<=G.GetLocalSize().Y+1);
    }
    T->TestEqual(TEXT("Manual attack CTA copy"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterContinueLabel")))->GetText().ToString(),FString(TEXT("实施战术")));
    const auto& ActionRegion=S->GetWidgetFromName(TEXT("TheaterBottomBounds"))->GetCachedGeometry();
    const auto& ButtonGeometry=S->GetWidgetFromName(TEXT("TheaterContinue"))->GetCachedGeometry();
    const auto& StatusGeometry=S->GetWidgetFromName(TEXT("TheaterStatus"))->GetCachedGeometry();
    const float ButtonCenter=ActionRegion.AbsoluteToLocal(ButtonGeometry.LocalToAbsolute(ButtonGeometry.GetLocalSize()*.5f)).X;
    const float StatusCenter=ActionRegion.AbsoluteToLocal(StatusGeometry.LocalToAbsolute(StatusGeometry.GetLocalSize()*.5f)).X;
    T->TestTrue(TEXT("Rendered button and actor hint share the action centerline"),FMath::Abs(ButtonCenter-StatusCenter)<1.f && FMath::Abs(ButtonCenter-ActionRegion.GetLocalSize().X*.5f)<1.f);
    T->TestTrue(TEXT("Rendered CTA fits the action region"),ButtonGeometry.GetLocalSize().X<=ActionRegion.GetLocalSize().X+1.f);
    T->AddInfo(FString::Printf(TEXT("CROSS_IDLE wait=%.2fs phase=%.3f dash=%.2f breath=%.3f action_centers=%.1f/%.1f"),Waited,Scene.CrossIdleTime,Scene.CrossIdleDashOffset(),Scene.CrossIdleBreath(),ButtonCenter,StatusCenter));
    for(const auto* Name:{TEXT("TheaterAttackDieWell"),TEXT("TheaterDefenseDieWell")})
    {
     const auto Size=S->GetWidgetFromName(Name)->GetCachedGeometry().GetLocalSize();
     T->TestTrue(TEXT("Pending die frame has real layout area"),Size.X>=60 && Size.Y>=60);
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("CrossLayoutRefinement")) && !NarrowCaptured)
    {
     if(NarrowStarted==0){CrossWindow->Resize(FVector2D(1280,960));NarrowStarted=FPlatformTime::Seconds();return false;}
     if(FPlatformTime::Seconds()-NarrowStarted<.4)return false;
     Capture(TEXT("Broadcast_NarrowManual.png"));NarrowCaptured=true;
    }
    if(FParse::Param(FCommandLine::Get(),TEXT("CrossLayoutOnly")))return true;
    ManualWait=0;
   }
   T->TestTrue(TEXT("Setup and intent ran naturally"),SawSetup && SawIntent);
   T->TestEqual(TEXT("Correct disclosed branch owns scene"),Scene.Facts.Method,CrossRound?(HighRound?EMethod::CrossHigh:EMethod::CrossLow):EMethod::Direct);
   if(CrossRound)
   {
    for(const auto Pair:{TPair<Role,FName>(Role::Carrier,Carrier),{Role::Runner,Runner},{Role::Marker,Marker},{Role::Helper,Helper}})
     T->TestTrue(TEXT("Four spatial roles are actual chosen identities"),Scene.Facts.Participants.ContainsByPredicate([&](const auto& P){return P.Role==Pair.Key && P.CardId==Pair.Value;}));
    const auto* GK=Scene.Facts.Participants.FindByPredicate([](const auto& P){return P.Role==Role::Goalkeeper;});
    T->TestTrue(TEXT("Real roster GK; only activated High GK contributes"),GK && GK->bFormulaActive==(Round==0));
    FString Terms;for(const auto* Row:{&S->GetPresentation().InlineFormula.AttackRow,&S->GetPresentation().InlineFormula.DefenseRow})
     for(const auto& Term:Row->Terms)Terms+=Term.DisplayLabel;
    T->TestTrue(TEXT("Authoritative route attributes remain visible"),Terms.Contains(HighRound?TEXT("力量"):TEXT("速度")) && Terms.Contains(TEXT("传球")) && Terms.Contains(TEXT("防守")));
    T->AddInfo(FString::Printf(TEXT("CROSS_FORMULA round=%d %s"),Round,*Terms));
   }
   if(!Roll(*C,CrossRound?(HighRound?EFMCodexLocalDevRollTarget::CrossHighAttack:EFMCodexLocalDevRollTarget::CrossLowAttack):EFMCodexLocalDevRollTarget::LongShotDirectAttack,FailRound?3:6))return true;
   S->RequestContinueResolution();if(!Accepted(*C))return true;Next();return false;
  }
  if(Step==7)
  {
   if(bBroadcast)
   {
    if(ManualWait==0){ManualWait=FPlatformTime::Seconds();return false;}
    if(FPlatformTime::Seconds()-ManualWait<CrossIdleCycleSeconds+1.f)return false;
    T->TestTrue(TEXT("Defender wait also paints an active idle loop"),Scene.IsCrossIdle() && Scene.CrossEntrance.RecentlyPainted(GFrameCounter));
    T->TestEqual(TEXT("Natural wait cannot auto-roll final defense"),V.AcceptedRolls.Num(),BroadcastRolls+2);
   }
   ScoreBeforeA=S->GetMatchHeader()->GetPresentation().PlayerAScoreLabel;
   ScoreBeforeB=S->GetMatchHeader()->GetPresentation().PlayerBScoreLabel;
   if(!Roll(*C,CrossRound?(HighRound?EFMCodexLocalDevRollTarget::CrossHighDefense:EFMCodexLocalDevRollTarget::CrossLowDefense):EFMCodexLocalDevRollTarget::LongShotDirectDefense,FailRound?6:1))return true;
   S->RequestContinueResolution();if(!Accepted(*C))return true;Next();return false;
  }
  if(Step==8)
  {
   if(Scene.Phase!=EPhase::ResultHold)return false;
   if(ResultReady==0){ResultReady=FPlatformTime::Seconds();return false;}
   if(FPlatformTime::Seconds()-ResultReady<.3)return false;
   if(bBroadcast && !FailRound)
   {
    if(!GoalCaptured)
    {
     if(Scene.Celebration.Elapsed<.55f)return false;
     T->TestTrue(TEXT("Natural Cross Goal plays the existing celebration after disclosure"),Scene.Celebration.IsActive() && Scene.Celebration.bConsumed);
     T->TestTrue(TEXT("Celebration is painted on the live field"),Scene.CrossEntrance.RecentlyPainted(GFrameCounter));
     T->TestEqual(TEXT("Narrative banner waits until the celebration finishes"),S->GetWidgetFromName(TEXT("CrossResultPlate"))->GetVisibility(),ESlateVisibility::Collapsed);
     Capture(TEXT("Broadcast_GoalCelebration.png"));GoalCaptured=true;return false;
    }
    if(Scene.Celebration.IsActive())return false;
    // The banner was collapsed during celebration. Let Slate lay it out and
    // paint before comparing cached parent/child geometry from the live frame.
    if(ResultBannerFrame==0){ResultBannerFrame=GFrameCounter;return false;}
    if(GFrameCounter<ResultBannerFrame+2)return false;
    T->TestTrue(TEXT("Celebration ends naturally after two seconds without another roll"),Scene.Celebration.Elapsed>=FMCodexGoalCelebration::CrossDuration);
    T->AddInfo(FString::Printf(TEXT("CROSS_GOAL_CELEBRATION duration=%.2fs"),Scene.Celebration.Elapsed));
   }
   T->TestTrue(TEXT("One outcome follows the mandatory Formula reveal"),SawOutcome);
   if(!bBroadcast && Round<2)T->TestTrue(TEXT("Skip verified for both routes"),Skipped);
   T->TestEqual(TEXT("Authoritative result maps to Goal or neutral Miss"),Scene.Facts.Outcome,FailRound?EOutcome::DefensiveSuccess:EOutcome::Goal);
   if(bOutcomePolish)T->TestTrue(TEXT("Natural visual hold or explicit Outcome skip observed"),Round==1?OutcomeSkipped:SawVisualHold);
   const auto& G=S->GetWidgetFromName(TEXT("TheaterContinue"))->GetCachedGeometry();
   T->TestTrue(TEXT("Continue remains within viewport"),S->GetCachedGeometry().AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize())).Y<=S->GetCachedGeometry().GetLocalSize().Y);
   if(bBroadcast)
   {
    T->TestTrue(TEXT("Natural field outcome includes the longer action and existing hold"),SawVisualHold && OutcomeStarted>0 && ResultReady-OutcomeStarted>=CrossOutcomeSeconds-.15);
    T->TestFalse(TEXT("Final result stops idle"),Scene.IsCrossIdle());
    T->AddInfo(FString::Printf(TEXT("CROSS_OUTCOME observed=%.3fs action=%.2fs hold=%.2fs"),ResultReady-OutcomeStarted,CrossOutcomeActionSeconds,CrossOutcomeHoldSeconds));
    const auto* Plate=CastChecked<UBorder>(S->GetWidgetFromName(TEXT("CrossResultPlate")));
    T->TestTrue(TEXT("Result plate visible with opaque navy treatment"),Plate->GetVisibility()!=ESlateVisibility::Collapsed && Plate->Background.GetTint(FWidgetStyle()).A>.9f);
    const auto PlateSize=Plate->GetCachedGeometry().GetLocalSize();
    T->TestFalse(TEXT("Result omits redundant ordinary comparison sentence"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossBroadcastResultReason")))->GetText().ToString().Contains(TEXT("本次公式按照总值大小比较")));
    T->TestTrue(TEXT("Result remains a compact banner"),PlateSize.X<=761 && PlateSize.Y<190);
    for(const auto Name:{TEXT("CrossBroadcastOutcome"),TEXT("CrossBroadcastResultReason")})
    {
     const auto& Child=S->GetWidgetFromName(Name)->GetCachedGeometry();
     const auto Max=Plate->GetCachedGeometry().AbsoluteToLocal(Child.LocalToAbsolute(Child.GetLocalSize()));
     T->TestTrue(TEXT("Result text fits its backing"),Max.X<=PlateSize.X && Max.Y<=PlateSize.Y);
    }
   }
   if(bOutcomePolish)Capture(Round==0?TEXT("Outcome_HighDefenseResult.png"):Round==1?TEXT("Outcome_LowDefenseResult.png"):TEXT("Outcome_GoalResult.png"));
   else if(Round<2)Capture(Round==0?TEXT("03_HighResult.png"):TEXT("04_LowResult.png"));
   T->AddInfo(FString::Printf(TEXT("CROSS_SCENE_PIE round=%d sequence=%lld setup=%d intent=%d skip=%d outcome=%d participants=%d"),Round,V.AttackSequence,SawSetup,SawIntent,Skipped,int(Scene.Facts.Outcome),Scene.Facts.Participants.Num()));
   Completed=V.AttackSequence;S->RequestContinueResolution();if(!Accepted(*C))return true;Next();return false;
  }
  T->TestEqual(TEXT("Continuation releases spatial scene"),Scene.Phase,EPhase::Hidden);
  T->TestTrue(TEXT("Canonical continuation advances exactly once"),!V.bCurrentAttackActive || V.AttackSequence==Completed+1);
  if(bBroadcast || ++Round==3)return true;
  Step=1;Preview=0;Defenders=0;Keeper=false;Skipped=SawSetup=SawIntent=SawOutcome=SawVisualHold=OutcomeSkipped=false;ResultReady=0;
  Started=Changed=FPlatformTime::Seconds();return false;
 }
private:
 bool Accepted(AFMCodexLocalMatchPlayerController& C)
 {return T->TestTrue(*FString::Printf(TEXT("Typed action accepted: %s %s"),*C.GetLastDiagnostic().CommandName,*C.GetLastDiagnostic().Message),C.GetLastDiagnostic().bHostSuccess);}
 bool Roll(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int Value)
 {FFMCodexLocalDevRollOverrideRequest R;R.Target=Target;R.Value=Value;return T->TestTrue(TEXT("Server-owned DEV provider"),C.SetLocalDevRollOverride(R).bSuccess);}
 bool Deploy(AFMCodexLocalMatchPlayerController& C,UFMCodexLocalMatchScreenWidget& S,const FString& Half,FName Card=NAME_None)
 {
  const auto* O=C.GetInteractionView().DeploymentOptions.FindByPredicate([&](const auto& X){return !X.bGoalkeeper && X.SlotId.ToString().Contains(Half) && (Card.IsNone() || X.CardId==Card);});
  if(!T->TestNotNull(*FString::Printf(TEXT("Canonical deployment option: %s / %s"),*Card.ToString(),*Half),O))return false;
  const FName Id=O->CardId,Slot=O->SlotId;S.RequestDeployOrdinary(Id,Slot);return Accepted(C);
 }
 void Capture(const TCHAR* Name)
 {
  auto W=GEditor->PlayWorld->GetGameViewport()->GetWindow();TArray<FColor> Pixels;FIntVector Size=FIntVector::ZeroValue;
  if(!W || !FSlateApplication::Get().TakeScreenshot(W->GetContent(),Pixels,Size)){T->AddError(TEXT("PIE capture failed"));return;}
  const FString Dir=FPaths::ProjectSavedDir()/(bBroadcast?TEXT("Stage8_30F/PIE"):TEXT("Stage8_23B/PIE"));IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);FFileHelper::SaveArrayToFile(PNG,*(Dir/Name));
 }
 FAutomationTestBase* T;int Round=0,Step=0,Preview=0,Defenders=0,PreviewRolls=0;
 double Started=FPlatformTime::Seconds(),Changed=0,ResultReady=0;
 bool Keeper=false,Skipped=false,SawSetup=false,SawIntent=false,SawOutcome=false;
 bool bOutcomePolish=false,SawVisualHold=false,OutcomeSkipped=false;
 bool bBroadcast=false;int32 BroadcastRolls=0;double BroadcastChoiceTime=0,BroadcastPaintTime=0,ManualWait=0,RouteReadStart=0,RouteReadEnd=0,NarrowStarted=0,OutcomeStarted=0;float IdleStart=0;bool NarrowCaptured=false,IdleMotionCaptured=false;
 FVector2D IdleRunner,IdleBall;
 bool GoalCaptured=false;
 uint64 ResultBannerFrame=0;
 int64 PreviewSequence=0,Completed=0;FName Carrier,Runner,Marker,Helper;FString ScoreBeforeA,ScoreBeforeB;TSharedPtr<SWidget> Persistent;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossScenePIETest,"FMCodex.PIE.TacticalScene.CrossHighLowCommercialFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossScenePIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartCrossScenePIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayCrossScenePIE(this)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossOutcomePIETest,"FMCodex.PIE.TacticalScene.CrossOutcomeReadabilityFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossOutcomePIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartCrossScenePIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayCrossScenePIE(this,true)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossBroadcastPIETest,"FMCodex.PIE.TacticalScene.CrossBroadcastFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossBroadcastPIETest::RunTest(const FString&)
{
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartCrossScenePIE()));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FPlayCrossScenePIE(this,false,true)));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
