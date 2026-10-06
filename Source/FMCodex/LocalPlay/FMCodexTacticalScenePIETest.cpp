#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverride.h"
#include "FMCodexRollReelWidget.h"
#include "Components/Button.h"
#include "Components/NativeWidgetHost.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"
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
  const int32 Width=FParse::Param(FCommandLine::Get(),TEXT("TacticalSceneLayoutOnly"))?1800:1600;
  Settings->NewWindowWidth=Width;Settings->NewWindowHeight=900;
  Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);Settings->SetPlayNumberOfClients(1);
  SpatialWindow=SNew(SWindow).Title(FText::FromString(TEXT("Tactical Scene Local PIE"))).ClientSize(FVector2D(Width,900))
   .AutoCenter(EAutoCenter::None).ScreenPosition(FVector2D(0,0)).SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false);
  FSlateApplication::Get().AddWindow(SpatialWindow.ToSharedRef());
  FRequestPlaySessionParams P;P.EditorPlaySettings=Settings;P.CustomPIEWindow=SpatialWindow;GEditor->RequestPlaySession(P);return true;
 }
};
class FPlaySpatialPIE final : public IAutomationLatentCommand
{
public:
 explicit FPlaySpatialPIE(FAutomationTestBase* In):T(In)
 {
  bGoalOnly=FParse::Param(FCommandLine::Get(),TEXT("TacticalSceneGoalOnly"));
  bLayoutOnly=FParse::Param(FCommandLine::Get(),TEXT("TacticalSceneLayoutOnly"));
  if(bGoalOnly)Round=1;
  else if(FParse::Param(FCommandLine::Get(),TEXT("TacticalSceneImmediateMissOnly")))Round=3;
 }
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
  if(State.Celebration.IsActive())
  {
   T->TestTrue(TEXT("Celebration follows Goal and existing result disclosure"),State.Phase==Phase::ResultHold
    && State.Facts.Outcome==FMCodexTacticalScene::EOutcome::Goal && !S->IsInlineFormulaRevealInputBlocked());
   SawCelebration=true;
   if(Round==1 && !CapturedCelebration && State.Celebration.Elapsed>.55f)
   {Capture(TEXT("04_GoalCelebration.png"));CapturedCelebration=true;}
   if(Round==2 && State.Celebration.Elapsed>.55f)
   {
    auto* Host=CastChecked<UNativeWidgetHost>(S->GetWidgetFromName(TEXT("TacticalScene")));
    const auto Before=C->GetInteractionView().InteractionCategory;
    const int Rolls=C->GetInteractionView().AcceptedRolls.Num();
    const auto Score=S->GetPresentation().Header.ScoreLabel;
    Host->GetContent()->OnMouseButtonDown(Host->GetCachedGeometry(),
     FPointerEvent(0,FVector2D::ZeroVector,FVector2D::ZeroVector,TSet<FKey>{EKeys::LeftMouseButton},EKeys::LeftMouseButton,0.f,FModifierKeysState()));
    T->TestFalse(TEXT("Click completes Goal celebration"),State.Celebration.IsActive());
    T->TestEqual(TEXT("Celebration skip does not continue gameplay"),C->GetInteractionView().InteractionCategory,Before);
    T->TestEqual(TEXT("Celebration skip does not roll"),C->GetInteractionView().AcceptedRolls.Num(),Rolls);
    T->TestEqual(TEXT("Celebration skip does not score again"),S->GetPresentation().Header.ScoreLabel,Score);CelebrationSkipped=true;
   }
  }
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
   if(State.IsOutcomeVisualHold())
   {
    T->TestEqual(TEXT("Action settles before final hold"),State.OutcomeActionProgress(),1.f);
    if(!SawVisualHold && Round==3)Capture(TEXT("06_ImmediateMissHold.png"));
    SawVisualHold=true;
   }
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
   }
   return false;
  }
  if(FPlatformTime::Seconds()-Changed<.1) return false;
  if(Step==0)
  {
   // Independent fixture matches reuse this PIE world, not the previous match's
   // presentation session / settled reveal identities.
   S->ResetPresentationSession();S->RequestStartNewMatch();Next();return false;
  }
  if(Step==1)
  {
   if(C->IsRecoveryNotificationDismissScheduledForTesting() || !S->GetPresentation().Interaction.bCanRollTacticalPoints)return false;
   if(!Roll(EFMCodexLocalDevRollTarget::FullD12,3))return true;S->RequestRollTacticalPoints();
   if(!T->TestEqual(TEXT("Real tactical roll enters deployment"),C->GetInteractionView().MajorPhase,EFMCodexLocalMatchMajorPhase::Deployment))return true;
   Next();return false;
  }
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
    T->TestEqual(TEXT("Method choice starts with Direct"),State.PreviewMethod,FMCodexTacticalScene::EMethod::Direct);
    PreviewSurface=Host->GetContent();PreviewRollCount=C->GetInteractionView().AcceptedRolls.Num();
    const auto& G=Host->GetCachedGeometry();const auto& RootG=S->GetCachedGeometry();
    PreviewPosition=RootG.AbsoluteToLocal(G.GetAbsolutePosition());
    PreviewSize=RootG.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()))-PreviewPosition;
    PreviewGeometryValid=true;
    PreviewMarker=FMCodexTacticalScene::LongShotParticipantAnchor(State,EMatchPlayResolutionParticipantRole::Marker);
    // Keep the real pointer off both buttons while exercising keyboard focus.
    FSlateApplication::Get().SetCursorPos(SpatialWindow->GetPositionInScreen()+FVector2D(10,10));
    PreviewStep=Round==0?1:8;Changed=FPlatformTime::Seconds()+.5;return false;
   }
   if(PreviewStep>0 && PreviewStep<8)
   {
    T->TestTrue(TEXT("Same Slate scene retained across previews"),PreviewSurface==Host->GetContent());
    T->TestEqual(TEXT("Hover/focus consumes no accepted roll"),C->GetInteractionView().AcceptedRolls.Num(),PreviewRollCount);
    T->TestEqual(TEXT("Hover/focus leaves authoritative method selection pending"),C->GetInteractionView().InteractionCategory,Category::SelectLongShotBranch);
    const bool ExpectCorner=PreviewStep==2 || PreviewStep==4 || PreviewStep==6;
    T->TestEqual(TEXT("Hover and keyboard preview route"),State.PreviewMethod,ExpectCorner?FMCodexTacticalScene::EMethod::DeadCorner:FMCodexTacticalScene::EMethod::Direct);
    T->TestTrue(TEXT("Preview blend completes without stale geometry"),FMath::IsNearlyEqual(State.CornerBlend,ExpectCorner?1.f:0.f));
    if(PreviewStep==1)
    {
     T->TestFalse(TEXT("Default preview is alive without taking a shot"),FMCodexTacticalScene::LongShotParticipantAnchor(State,EMatchPlayResolutionParticipantRole::Marker).Equals(PreviewMarker,.1f));
     Capture(TEXT("01_DirectPreview.png"));Corner->TakeWidget()->OnMouseEnter(Corner->GetCachedGeometry(),FPointerEvent());
    }
    if(PreviewStep==2)
    {
     Capture(TEXT("02_DeadCornerPreview.png"));Corner->TakeWidget()->OnMouseLeave(FPointerEvent());
     Direct->TakeWidget()->OnMouseEnter(Direct->GetCachedGeometry(),FPointerEvent());
    }
    if(PreviewStep==3)
    {
     Direct->TakeWidget()->OnMouseLeave(FPointerEvent());
     FSlateApplication::Get().SetKeyboardFocus(Corner->TakeWidget(),EFocusCause::Navigation);
    }
    if(PreviewStep==4)FSlateApplication::Get().SetKeyboardFocus(Direct->TakeWidget(),EFocusCause::Navigation);
    if(PreviewStep==5)Corner->TakeWidget()->OnMouseEnter(Corner->GetCachedGeometry(),FPointerEvent());
    if(PreviewStep==6)Corner->TakeWidget()->OnMouseLeave(FPointerEvent());
    ++PreviewStep;Changed=FPlatformTime::Seconds()+.2;return false;
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
   if(bLayoutOnly && !CapturedRoll)
   {
    if(RollReady==0){RollReady=FPlatformTime::Seconds();return false;}
    if(FPlatformTime::Seconds()-RollReady<.4)return false;
    const auto& Root=S->GetCachedGeometry();
    auto* Scene=S->GetWidgetFromName(TEXT("TacticalScene"));
    const auto& SG=Scene->GetCachedGeometry();
    const auto& Duel=S->GetWidgetFromName(TEXT("TheaterDuel"))->GetCachedGeometry();
    const auto& Button=S->GetWidgetFromName(TEXT("TheaterContinue"))->GetCachedGeometry();
    float LastRoleBottom=0;
    for(const auto& P:State.Facts.Participants)
     LastRoleBottom=FMath::Max(LastRoleBottom,float(Root.AbsoluteToLocal(SG.LocalToAbsolute(
      FMCodexTacticalScene::LongShotParticipantAnchor(State,P.Role)+FVector2D(0,82))).Y));
    const float CardsTop=Root.AbsoluteToLocal(Duel.GetAbsolutePosition()).Y;
    const float ButtonBottom=Root.AbsoluteToLocal(Button.LocalToAbsolute(Button.GetLocalSize())).Y;
    T->TestTrue(TEXT("Raised Formula cards remain below every participant label"),CardsTop>LastRoleBottom+16);
    T->TestTrue(TEXT("Roll CTA retains comfortable bottom breathing room"),Root.GetLocalSize().Y-ButtonBottom>40);
    auto* Reserve=CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterSpatialReserve")));
    auto* Lane=CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterSpatialLane")));
    T->TestTrue(TEXT("Committed stack rises without moving the pitch"),Reserve->GetHeightOverride()<Lane->GetHeightOverride());
    const auto Padding=CastChecked<UOverlaySlot>(S->GetWidgetFromName(TEXT("TheaterCompositionFit"))->Slot)->GetPadding();
    T->TestTrue(TEXT("Lift keeps original available control height and scale"),FMath::IsNearlyEqual(
     Reserve->GetHeightOverride()+Padding.Bottom,Lane->GetHeightOverride()));
    T->TestTrue(TEXT("Raised roll CTA remains actionable"),S->GetWidgetFromName(TEXT("TheaterContinue"))->GetIsEnabled());
    T->AddInfo(FString::Printf(TEXT("LAYOUT_ROLL roleBottom=%.1f cardsTop=%.1f buttonBottom=%.1f viewportHeight=%.1f"),
     LastRoleBottom,CardsTop,ButtonBottom,Root.GetLocalSize().Y));
    Capture(TEXT("03_AttackRoll.png"));CapturedRoll=true;
   }
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
   if(!Roll(EFMCodexLocalDevRollTarget::LongShotDirectAttack,Round==3?2:Round==0?4:6))return true;
   S->RequestContinueResolution();
   if(Round==3)
   {
    // Assert the reveal begins at dispatch, including after a fixture restart.
    // Do not advance its real clock or bypass the mandatory attack reveal.
    SawRoll=S->IsInlineFormulaRevealInputBlocked();
    T->TestTrue(TEXT("ImmediateMiss starts the mandatory attack reveal"),SawRoll);
    T->TestEqual(TEXT("ImmediateMiss ball waits for attack reveal"),State.Phase,Phase::FormulaHold);
    Step=7;Changed=FPlatformTime::Seconds();return false;
   }
   Next();return false;
  }
  if(Step==6)
  {
   if(!Roll(EFMCodexLocalDevRollTarget::LongShotDirectDefense,Round==0?6:1))return true;
   S->RequestContinueResolution();Next();return false;
  }
  if(Step==7)
  {
   if(State.Phase!=Phase::ResultHold)return false;
   if(State.Celebration.IsActive())return false;
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
   T->TestTrue(TEXT("Provider goes through actual failure / goal resolution"),State.Facts.Outcome==(Round==3?FMCodexTacticalScene::EOutcome::ImmediateMiss:Round==0?FMCodexTacticalScene::EOutcome::DefensiveSuccess:FMCodexTacticalScene::EOutcome::Goal));
   T->TestTrue(TEXT("Natural final hold observed"),SawVisualHold);
   T->TestEqual(TEXT("Only Goal celebrated"),SawCelebration,Round==1 || Round==2);
   if(Round==2)T->TestTrue(TEXT("DeadCorner celebration click skip verified"),CelebrationSkipped);
   const auto FinalMarker=FMCodexTacticalScene::LongShotParticipantAnchor(State,EMatchPlayResolutionParticipantRole::Marker);
   T->TestEqual(TEXT("Only Direct defense win ends in Marker pressure pose"),FinalMarker.Equals(FMCodexTacticalScene::MarkerAnchor),Round!=0);
   if(Round==0)T->TestFalse(TEXT("Aggregate defensive narrative does not assert a tackle/block/save"),
    S->GetPresentation().LongShotResolution.Formula.NarrativeHeadline.Contains(TEXT("抢断"))
    || S->GetPresentation().LongShotResolution.Formula.NarrativeHeadline.Contains(TEXT("扑救")));
   T->TestTrue(TEXT("Narrative is prominent at the top"),S->GetWidgetFromName(TEXT("TheaterOutcome"))->GetVisibility()!=ESlateVisibility::Collapsed
    && !CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterOutcome")))->GetText().IsEmpty());
   const auto Endpoint=FMCodexTacticalScene::OutcomeTarget(State.Facts.Outcome,State.Facts.Method);
   T->TestEqual(TEXT("Goal lands inside; failure outside"),FMCodexTacticalScene::IsInsideGoal(Endpoint),Round==1 || Round==2);
   if(bLayoutOnly)
   {
    const float BallBottom=Root.AbsoluteToLocal(SG.LocalToAbsolute(Endpoint+FVector2D(0,10))).Y;
    const float CardsTop=Root.AbsoluteToLocal(S->GetWidgetFromName(TEXT("TheaterDuel"))->GetCachedGeometry().GetAbsolutePosition()).Y;
    T->TestTrue(TEXT("Final failed-shot ball remains fully above the information cards"),CardsTop>BallBottom+8);
    T->AddInfo(FString::Printf(TEXT("LAYOUT_RESULT ballBottom=%.1f cardsTop=%.1f"),BallBottom,CardsTop));
   }
   if(Round==2)T->TestTrue(TEXT("Both procedural dice revealed naturally"),SawPairA && SawPairB);
   if(Round==2)Capture(TEXT("05_DeadCornerResult.png"));
   if(Round==0)Capture(bLayoutOnly?TEXT("04_DefensiveResult.png"):TEXT("03_DefensiveResult.png"));
   T->AddInfo(FString::Printf(TEXT("TACTICAL_SCENE_PIE round=%d carrier=%s participants=%d setup=%d intent=%d roll=%d outcome=%d result=1 geometryPhases=%d"),Round,*Carrier.ToString(),State.Facts.Participants.Num(),SawSetup,SawIntent,SawRoll,SawOutcome,CheckedGeometry.Num()));
   CompletedSequence=C->GetInteractionView().AttackSequence;
   S->RequestContinueResolution();Next();return false;
  }
  T->TestTrue(TEXT("Canonical continuation clears scene"),S->GetTacticalScene().Phase==Phase::Hidden);
  T->TestTrue(TEXT("Canonical continuation leaves completed attack"),!C->GetInteractionView().bCurrentAttackActive || C->GetInteractionView().AttackSequence>CompletedSequence);
  if(Round<3 && !bGoalOnly && !bLayoutOnly)
  {
   ++Round;RoundStart=FPlatformTime::Seconds();ResultReady=0;PreviewStep=0;
   PreviewGeometryValid=false;CheckedGeometry.Reset();
   // Direct failure -> opposite-side goal still verifies actual continuation.
   // The independent DeadCorner case starts with a fresh roster: recovery is
   // random and does not guarantee the first round's named carrier is returned.
   Step=Round>=2?0:1;KeeperDeployed=false;SawSetup=SawIntent=SawOutcome=SawRoll=SawVisualHold=SawCelebration=CapturedCelebration=CelebrationSkipped=false;
   Changed=FPlatformTime::Seconds();return false;
  }
  return true;
 }
private:
 void Capture(const TCHAR* Name)
 {
  auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();TArray<FColor> Pixels;FIntVector Size=FIntVector::ZeroValue;
  if(!Window || !FSlateApplication::Get().TakeScreenshot(Window->GetContent(),Pixels,Size)){T->AddError(TEXT("PIE capture failed"));return;}
  const FString Dir=FPaths::ProjectSavedDir()/(bLayoutOnly?TEXT("Stage8_24A/LayoutPolish/PIE")
   :bGoalOnly?TEXT("GoalCelebrationPolish/PIE"):TEXT("Stage8_24A/FinalPolish/PIE"));
  IFileManager::Get().MakeDirectory(*Dir,true);
  TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);FFileHelper::SaveArrayToFile(PNG,*(Dir/Name));
 }
 FAutomationTestBase* T;double RoundStart=FPlatformTime::Seconds(),Changed=0,ResultReady=0,RollReady=0;
 int Step=0,Round=0,PreviewStep=0,PreviewRollCount=0;FName Carrier;
 int64 CompletedSequence=INDEX_NONE;
 TSharedPtr<SWidget> PreviewSurface;
 FVector2D PreviewPosition,PreviewSize,PreviewMarker;
 TArray<FMCodexTacticalScene::EPhase> CheckedGeometry;
 bool bGoalOnly=false,bLayoutOnly=false,CapturedRoll=false,PreviewGeometryValid=false,SawVisualHold=false,SawCelebration=false,CapturedCelebration=false,CelebrationSkipped=false;
 bool SawPairA=false,SawPairB=false;
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
