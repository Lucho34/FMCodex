#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexTacticalScene.h"
#include "FMCodexMatchShellStyle.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "Engine/Texture2D.h"
#include "FMCodexTacticalDetailPanelWidget.h"
#include "FMCodexTacticalDetailPresentation.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPitchWidget.h"
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#include "Blueprint/WidgetTree.h"
#include "Components/NativeWidgetHost.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/FontMeasure.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
using namespace FMCodexTacticalScene;
using namespace FMCodexPlayerFacingOrdinaryUITests;
namespace
{
using Role=EMatchPlayResolutionParticipantRole;
FFMCodexUMGMatchScreenViewModel SafeModel(FUIFixture& F)
{
 // Use the production owner-safe adapter, which explicitly preserves pending
 // ordinary Formula descriptors. The older Access::Safe helper intentionally
 // defaults that disclosure opt-in off and thus removes pending contests.
 return F.Attacker()->GetPlayerMatchScreen()->GetPresentation();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossSceneFactsTest,"FMCodex.LocalPlay.TacticalScene.CrossSceneFactsAndIsolation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossSceneFactsTest::RunTest(const FString&)
{
 for(bool BFirst:{false,true}) for(bool Keeper:{false,true})
 {
  FUIFixture F(BFirst);
  if(!TestTrue(TEXT("Real selected Cross roles and canonical route"),F.ReachRoute(ESkillRuleType::Cross,2,true,false,Keeper)
   && F.Send(F.Attacker(),Kind::CrossInitialRouteRoll)))return false;
  FUnchanged Before(F);
  auto M=SafeModel(F);
  if(!TestTrue(TEXT("Published safe model contains pending Cross Formula"),M.InlineFormula.bVisible))return false;
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
  FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Save,&M,nullptr);
  FFMCodexUMGMatchScreenViewModel Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,true);
  FFMCodexUMGMatchScreenViewModel::StaticStruct()->SerializeItem(Load,&Copy,nullptr);
  TestFalse(TEXT("Safe reflected Cross DTO roundtrip"),Load.IsError());
  // Deliberately non-default colors: identity follows the configured side,
  // including the inactive defending keeper, not a hard-coded attack palette.
  Copy.Header.LeftAttackTurnTracker.PrimarySideColor=FLinearColor(.32f,.65f,.12f);
  Copy.Header.RightAttackTurnTracker.PrimarySideColor=FLinearColor(.62f,.18f,.7f);
  const auto Facts=ProjectCross(Copy.InlineFormula,Copy.Header,true);
  TestTrue(TEXT("Cross has actual four roles plus defending roster GK"),Facts.bActive && Facts.Participants.Num()==5);
  for(const auto* Row:{&Copy.InlineFormula.AttackRow,&Copy.InlineFormula.DefenseRow}) for(const auto& P:Row->Participants)
  {
   const auto* Spatial=Facts.Participants.FindByPredicate([&](const auto& X){return X.Role==P.Role;});
   TestTrue(TEXT("Each formula identity, name and side survives unchanged"),Spatial && Spatial->CardId==P.CardId && Spatial->Side==Row->Side && Spatial->Name.ToString()==P.PlayerName);
  }
  const auto* GK=Facts.Participants.FindByPredicate([](const auto& P){return P.Role==Role::Goalkeeper;});
  TestTrue(TEXT("Roster GK is not automatically Formula-active"),GK && GK->CardId==Copy.InlineFormula.SpatialGoalkeeper.CardId && GK->bFormulaActive==Keeper);
  auto Miss=Copy.InlineFormula;Miss.SpatialOutcome=EMatchPlayResolutionDecisionOutcome::Miss;
  Miss.bNarrativeAvailable=true;Miss.bDiceRevealVisible=false;
  FState Neutral;Neutral.Sync(ProjectCross(Miss,Copy.Header,true));
  TestEqual(TEXT("Aggregate Cross defense result stays generic even with active GK"),Neutral.Facts.Outcome,EOutcome::DefensiveSuccess);
  const auto NeutralEnd=CrossOutcomeBall(Neutral,1.f);
  Miss.NarrativeHeadline=TEXT("门将扑救；协防拦截；盯人抢断");
  FState Wording;Wording.Sync(ProjectCross(Miss,Copy.Header,true));
  TestTrue(TEXT("Narrative dramatization cannot select a causal actor or alter ball path"),CrossOutcomeBall(Wording,1.f).Equals(NeutralEnd));
  TestFalse(TEXT("No invented goalkeeper possession"),NeutralEnd.Equals(CrossParticipantAnchor(Neutral,Role::Goalkeeper),60.f));
  Miss.bDiceRevealVisible=true;
  TestEqual(TEXT("Typed result still waits for existing visible reveal"),ProjectCross(Miss,Copy.Header,true).Outcome,EOutcome::None);
  for(const auto& P:Facts.Participants)
  {
   const auto& Tracker=P.Side==Copy.Header.LeftPlayerSide?Copy.Header.LeftAttackTurnTracker:Copy.Header.RightAttackTurnTracker;
   TestTrue(TEXT("Actual A/B side accent survives attacker swap and GK activation"),P.Accent.Equals(FMCodexMatchShellStyle::HeaderAccent(Tracker.PrimarySideColor)));
  }
  for(float Low:{0.f,1.f})
  {
   FState S;S.Sync(Facts);S.CrossLowBlend=Low;S.Phase=EPhase::Intent;S.Elapsed=0;
   const auto Carrier=CrossParticipantAnchor(S,Role::Carrier),RunnerStart=CrossParticipantAnchor(S,Role::Runner),HelperStart=CrossParticipantAnchor(S,Role::Helper);
   S.Elapsed=CrossIntentSeconds;
   const auto Runner=CrossParticipantAnchor(S,Role::Runner),Helper=CrossParticipantAnchor(S,Role::Helper),Zone=CrossReceiveZone(Low);
   TestTrue(TEXT("Crosser stays on far wide channel, distinct from central receiver"),Carrier.Y<Runner.Y-100 && Carrier.X<GoalAnchor.X);
   TestTrue(TEXT("Marker pressures crosser, not receiver"),(CrossParticipantAnchor(S,Role::Marker)-Carrier).Size()<(CrossParticipantAnchor(S,Role::Marker)-Runner).Size());
   TestTrue(TEXT("Receiver and helper converge on the same penalty-area zone"),
    (Runner+FVector2D(0,32)-Zone).Size()<(RunnerStart+FVector2D(0,32)-Zone).Size()
    && (Helper+FVector2D(0,32)-Zone).Size()<(HelperStart+FVector2D(0,32)-Zone).Size());
   TestTrue(TEXT("High delivers forward across box; Low cuts back from deeper byline"),Low==0?Carrier.X<Zone.X:Carrier.X>Zone.X);
   TestTrue(TEXT("Helper contests from goal side, receiver attacks from box entry"),Runner.X<Zone.X && Helper.X>Zone.X);
  }
  // Missing roles in a safe presentation fixture must stay absent, never become
  // replacements inferred from the roster, a total or a screen-space location.
  Copy.InlineFormula.DefenseRow.Participants.RemoveAll([](const auto& P){return P.Role==Role::Helper || P.Role==Role::Goalkeeper;});
  Copy.InlineFormula.SpatialGoalkeeper={};
  auto Missing=ProjectCross(Copy.InlineFormula,Copy.Header,true);
  TestEqual(TEXT("No fabricated Helper or GK"),Missing.Participants.Num(),3);
  const auto Hidden=FFMCodexLocalMatchUMGPresentationBuilder::Build(Access::Safe(*F.Mode,F.Attacker()->GetOwnerView().ViewerSide,false),{},FString());
  TestFalse(TEXT("Withheld context cannot leak spatial roles"),ProjectCross(Hidden.InlineFormula,Hidden.Header,true).bActive);
  TestFalse(TEXT("Guided/fallback disallow is honored"),ProjectCross(M.InlineFormula,M.Header,false).bActive);
  Before.Verify(*this,F);
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossPreviewTest,"FMCodex.LocalPlay.TacticalScene.CrossRoutePreview",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossPreviewTest::RunTest(const FString&)
{
 // Exclude the unrelated synthetic PassControl label from this UI fixture.
 FUIFixture F(false,false,false);
 if(!TestTrue(TEXT("Real Cross method choice"),F.ReachBranch(ESkillRuleType::Cross,true)))return false;
 F.Settle();auto* Screen=F.Attacker()->GetPlayerMatchScreen();
 FState S;const auto M=SafeModel(F);S.Sync(ProjectCross(M.InlineFormula,M.Header,true));
 TestEqual(TEXT("Choice preview has four selected roles plus GK"),S.Facts.Participants.Num(),5);
 TestEqual(TEXT("Preview is read-only"),S.Phase,EPhase::Preview);
 FUnchanged Before(F);const int Sends=F.Backend(F.Attacker()).Sends;
 auto* Host=CastChecked<UNativeWidgetHost>(Screen->GetWidgetFromName(TEXT("TacticalScene")));
 const auto Persistent=Host->GetContent();
 auto* High=CastChecked<UButton>(Screen->GetWidgetFromName(TEXT("TheaterHigh")));
 auto* Low=CastChecked<UButton>(Screen->GetWidgetFromName(TEXT("TheaterLow")));
 High->TakeWidget()->OnMouseEnter(High->GetCachedGeometry(),FPointerEvent());UpdatePreview(*Screen->WidgetTree,S);S.Tick(.15f);
 TestEqual(TEXT("High hover chooses high preview"),S.PreviewMethod,EMethod::CrossHigh);
 S.PulseTime=PI*.5f;
 const auto HighCarrier=CrossParticipantAnchor(S,Role::Carrier);
 S.PulseTime=0;const auto HighStart=CrossParticipantAnchor(S,Role::Runner);
 S.PulseTime=PI*.5f;const auto HighTravel=(CrossParticipantAnchor(S,Role::Runner)-HighStart).Size();
 // This headless tree is not mounted in a focusable window. Exercise pointer
 // hover here; the real PIE window separately verifies keyboard focus.
 High->TakeWidget()->OnMouseLeave(FPointerEvent());Low->TakeWidget()->OnMouseEnter(Low->GetCachedGeometry(),FPointerEvent());UpdatePreview(*Screen->WidgetTree,S);S.Tick(.075f);
 TestTrue(TEXT("Mid-transition interpolates without a jump"),S.CrossLowBlend>0 && S.CrossLowBlend<1);
 S.Tick(.075f);S.PulseTime=PI*.5f;
 TestEqual(TEXT("Low hover preview completes in 150ms"),S.CrossLowBlend,1.f);
 const auto LowRunner=CrossParticipantAnchor(S,Role::Runner);
 TestTrue(TEXT("Low crosser advances down wide flank toward byline"),CrossParticipantAnchor(S,Role::Carrier).X>HighCarrier.X+200);
 S.PulseTime=0;const auto LowStart=CrossParticipantAnchor(S,Role::Runner);
 S.PulseTime=PI*.5f;
 TestTrue(TEXT("Low receiver accelerates farther into box than short High positioning"),(LowRunner-LowStart).Size()>HighTravel*1.5f);
 TestTrue(TEXT("High control lofts above delivery; low control stays in sweep corridor"),
  CrossDeliveryControl(0).Y<HighCarrier.Y && CrossDeliveryControl(1).Y<CrossParticipantAnchor(S,Role::Carrier).Y
  && CrossDeliveryControl(1).Y>CrossReceiveZone(1).Y);
 TestTrue(TEXT("Low receiving zone cuts back instead of running beyond crosser"),CrossReceiveZone(1).X<CrossParticipantAnchor(S,Role::Carrier).X);
 RefreshSurface(*Screen->WidgetTree,S,[](){});
 TestTrue(TEXT("Same Slate scene and portrait cache"),Persistent==Host->GetContent());
 TestFalse(TEXT("Preview skip cannot select a route"),S.Skip());
 TestEqual(TEXT("Preview submits no typed intent"),F.Backend(F.Attacker()).Sends,Sends);
 Before.Verify(*this,F);
 auto Pending=M.InlineFormula;Pending.bVisible=true;Pending.ContestId=TEXT("Cross.Route");Pending.SpatialCrossIntent=EMatchPlayElectiveBranchIntent::CrossHigh;
 S.Sync(ProjectCross(Pending,M.Header,true));S.Preview(EMethod::CrossLow);
 TestEqual(TEXT("Pending route retains committed intent, ignores hover"),S.PreviewMethod,EMethod::CrossHigh);
 Pending.ContestId=TEXT("Cross.Low");S.Sync(ProjectCross(Pending,M.Header,true));S.Tick(.15f);
 TestEqual(TEXT("Only disclosed actual route replaces flipped intention"),S.Facts.Method,EMethod::CrossLow);
 TestEqual(TEXT("No stale high blend after actual low"),S.CrossLowBlend,1.f);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossStoryboardTest,"FMCodex.LocalPlay.TacticalScene.CrossStoryboardAndOutcome",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossStoryboardTest::RunTest(const FString&)
{
 for(bool High:{true,false})
 {
  FUIFixture F;
  if(!TestTrue(TEXT("Canonical Cross pending Formula"),F.ReachRoute(ESkillRuleType::Cross,2,High,false,true)
   && F.Send(F.Attacker(),Kind::CrossInitialRouteRoll)))return false;
  auto M=SafeModel(F);FUnchanged Before(F);
  if(!TestTrue(TEXT("Owner-safe adapter retains pending Formula"),M.InlineFormula.bVisible))return false;
  for(auto Outcome:{EMatchPlayResolutionDecisionOutcome::Goal,EMatchPlayResolutionDecisionOutcome::Miss})
  {
   auto V=M.InlineFormula;auto Preview=V;Preview.ContestId=TEXT("Cross.Setup");
   FState S;S.Sync(ProjectCross(Preview,M.Header,true));S.Tick(.3f);
   const auto Last=CrossParticipantAnchor(S,Role::Runner);
   S.Sync(ProjectCross(V,M.Header,true));
   TestEqual(TEXT("Setup precedes Formula"),S.Phase,EPhase::Setup);
   TestTrue(TEXT("Preview positions survive commit"),Last.Equals(CrossParticipantAnchor(S,Role::Runner)));
   S.Tick(.2f);S.Sync(ProjectCross(V,M.Header,true));TestEqual(TEXT("Duplicate view preserves clock"),S.Elapsed,.2f);
   S.Skip();TestEqual(TEXT("One skip advances only setup"),S.Phase,EPhase::Intent);
   S.Tick(CrossIntentSeconds);TestEqual(TEXT("Intent waits for existing Formula"),S.Phase,EPhase::FormulaHold);
   const auto DeliveryBefore=CrossIntentBall(S);
   TestFalse(TEXT("Attempted delivery stops short of the receiving zone"),DeliveryBefore.Equals(CrossReceiveZone(S.CrossLowBlend)));
   TestFalse(TEXT("Cannot skip authority/reel"),S.Skip());
   V.bDefenseRowActive=true;V.bAttackRowActive=false;
   V.SpatialOutcome=Outcome;V.bNarrativeAvailable=true;V.bDiceRevealVisible=true;V.NarrativeHeadline=TEXT("权威叙事");
   S.Sync(ProjectCross(V,M.Header,true));TestEqual(TEXT("No outcome before mandatory reveal"),S.Facts.Outcome,EOutcome::None);
   V.bDiceRevealVisible=false;S.Sync(ProjectCross(V,M.Header,true));
   TestTrue(TEXT("Outcome begins at the held delivery position, including its first frame"),CrossOutcomeBall(S,0.f).Equals(DeliveryBefore));
   TestEqual(TEXT("Revealed typed fact begins outcome"),S.Phase,EPhase::Outcome);
   TestEqual(TEXT("No inferred save/header; typed result only"),S.Facts.Outcome,Outcome==EMatchPlayResolutionDecisionOutcome::Goal?EOutcome::Goal:EOutcome::DefensiveSuccess);
   auto Gated=V;S.Gate(Gated);TestFalse(TEXT("Headline waits for arrival"),Gated.bNarrativeAvailable);
   TestEqual(TEXT("Formula subtotal unchanged"),Gated.AttackRow.KnownNonRollSubtotalLabel,V.AttackRow.KnownNonRollSubtotalLabel);
   TestTrue(TEXT("Both outcomes enter the real receiving corridor before continuation"),CrossOutcomeBall(S,CrossDeliveryFraction).Equals(CrossReceiveZone(S.CrossLowBlend)));
   S.Tick(CrossOutcomeActionSeconds);
   TestTrue(TEXT("Action completes into brief visual hold, not terminal CTA"),S.Phase==EPhase::Outcome && S.IsCrossOutcomeVisualHold());
   TestEqual(TEXT("Action freezes at its final frame"),S.CrossOutcomeActionProgress(),1.f);
   const auto End=CrossOutcomeBall(S,1.f),Receiver=CrossParticipantAnchor(S,Role::Runner);
   TestEqual(TEXT("Only Goal finishes inside goal"),IsInsideGoal(End,S.Facts.Method),Outcome==EMatchPlayResolutionDecisionOutcome::Goal);
   S.Tick(CrossOutcomeHoldSeconds*.5f);
   TestTrue(TEXT("Ball and receiver remain settled throughout visual hold"),S.IsCrossOutcomeVisualHold() && CrossOutcomeBall(S,1.f).Equals(End) && CrossParticipantAnchor(S,Role::Runner).Equals(Receiver));
   Gated=V;S.Gate(Gated);TestFalse(TEXT("Final visual hold still gates headline"),Gated.bNarrativeAvailable);
   S.Tick(CrossOutcomeHoldSeconds*.5f+.001f);TestEqual(TEXT("Short visual hold hands off to original result flow"),S.Phase,EPhase::ResultHold);
   TestTrue(TEXT("Goal endpoint inside goal"),IsInsideGoal(OutcomeTarget(EOutcome::Goal,S.Facts.Method),S.Facts.Method));
   S.Sync(ProjectCross(V,M.Header,true));TestEqual(TEXT("Repeated snapshot does not replay"),S.Phase,EPhase::ResultHold);
   FState Rebuilt;Rebuilt.Sync(ProjectCross(V,M.Header,true));TestEqual(TEXT("Resolved rebuild snaps to result"),Rebuilt.Phase,EPhase::ResultHold);
   TestTrue(TEXT("Resolved High/Low rebuild has correct final geometry immediately"),CrossOutcomeBall(Rebuilt,1.f).Equals(End) && CrossParticipantAnchor(Rebuilt,Role::Runner).Equals(Receiver));
   FState Accelerated=S;Accelerated.Phase=EPhase::Outcome;Accelerated.Elapsed=.2f;
   TestTrue(TEXT("Outcome skip is responsive"),Accelerated.Skip());
   TestTrue(TEXT("Skip preserves final spatial state without replay"),Accelerated.Phase==EPhase::ResultHold && CrossOutcomeBall(Accelerated,1.f).Equals(End));
   TestFalse(TEXT("Result skip never continues gameplay"),S.Skip());
  }
  Before.Verify(*this,F);
 }
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCrossBroadcastFlowTest,"FMCodex.LocalPlay.TacticalScene.CrossBroadcastFlow",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCrossBroadcastFlowTest::RunTest(const FString&)
{
 auto* Background=LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/TacticalScene/T_CrossBroadcast_Stadium.T_CrossBroadcast_Stadium"));
 if(!TestNotNull(TEXT("Authored Cross environment is imported at its production path"),Background))return false;
 // GetSizeX/Y may return the square placeholder while editor texture compilation
 // is asynchronous. This assertion concerns the authored plate, whose source
 // dimensions are available immediately; real rendering is covered by PIE.
 TestTrue(TEXT("Environment retains valid wide reference composition"),Background->Source.IsValid()
  && Background->Source.GetSizeY()>0 && Background->Source.GetSizeX()>Background->Source.GetSizeY()*2);
 TestEqual(TEXT("Unbranched Cross retains both canonical branches"),FFMCodexTacticalDetailPresentationBuilder::Build(ESkillRuleType::Cross).Branches.Num(),2);
 // One representative High and Low; the second swaps the attacking side.
 for (bool High : {true,false})
 {
  // Production catalog only: the optional synthetic PassControl fixture creates
  // an unrelated overlong Hand Micro skill label before this Cross test starts.
  FUIFixture F(!High,false,false);
  if (!TestTrue(TEXT("Post-participant canonical Skill phase"),F.SkillFixture())) return false;
  auto* PC=F.Attacker(); auto* S=PC->GetPlayerMatchScreen(); auto* Waiting=F.Defender()->GetPlayerMatchScreen();
  struct FDeferredBackend : FBackend
  {
   FFMCodexNetworkPlayerIntentAck Ack;
   Submission SubmitScreenIntent(const FFMCodexMatchScreenRequest& R) override
   {
    if (!FFMCodexNetworkMatchScreenActions::Begin(R,PC->GetOwnerView(),F->Client(PC),Last)) return Submission::Rejected;
    ++Sends; return Submission::Queued;
   }
   void DeliverView() { Ack=F->Mode->SubmitConnectionPlayerIntent(PC,Last); F->Client(PC).ObserveView(PC->GetOwnerView()); PC->RefreshPlayerFacingUI(); }
   void DeliverAck() { F->Client(PC).ObserveAck(Ack); PC->RefreshPlayerFacingUI(); }
  } Backend;
  Backend.F=&F; Backend.PC=PC; S->SetMatchBackend(&Backend);
  const auto Model=S->GetPresentation();
  const auto BoardCards=S->GetLocalRackWidget()->GetRenderedCardWidgets();
  const auto BoardSlots=S->GetPitchWidget()->GetRenderedSlotWidgets();
  const auto* Choice=Model.Interaction.SelectionChoices.FindByPredicate([](const auto& X){return X.SkillType==ESkillRuleType::Cross;});
  if (!TestNotNull(TEXT("Cross safe option"),Choice)) return false;
  const auto& Options=S->GetInteractionPanel()->GetRenderedOptionWidgets();
  TestEqual(TEXT("Cross expands to two; unrelated choices remain"),Options.Num(),Model.Interaction.SelectionChoices.Num()+1);
  TestEqual(TEXT("Voluntary decline preserved"),S->GetInteractionPanel()->GetPresentation().bCanDecline,Model.Interaction.bCanDecline);
  const auto* Clicked=Options.FindByPredicate([High](const auto& O){return O->GetLabel()==(High?TEXT("传中（高球）"):TEXT("传中（低球）"));});
  if (!TestNotNull(TEXT("Direct variant choice"),Clicked)) return false;
  auto* ChoiceButton=CastChecked<UButton>((*Clicked)->GetWidgetFromName(TEXT("InteractionOptionButton")));
  TestNotNull(TEXT("Theater is prepared before the click"),S->GetWidgetFromName(TEXT("ResolutionTheater")));
  ChoiceButton->OnHovered.Broadcast();
  const auto& Detail=S->GetTacticalDetailPanel()->GetPresentation();
  TestEqual(TEXT("Explicit branch hover is a single canonical table"),Detail.Branches.Num(),1);
  TestEqual(TEXT("Explicit branch title"),Detail.DisplayName,FString(High?TEXT("高球传中"):TEXT("低球传中")));
  const FString Copy=S->GetTacticalDetailPanel()->CollectPlayerFacingText();
  TestFalse(TEXT("Unused branch is absent"),Copy.Contains(High?TEXT("低球"):TEXT("高球")));
  TestTrue(TEXT("Route intent never promises the actual route"),Copy.Contains(TEXT("实际路线由路线掷点判定")));
  TestTrue(TEXT("Chosen receiver attribute"),Copy.Contains(High?TEXT("力量"):TEXT("速度")));
  TestTrue(TEXT("Chosen keeper attribute"),Copy.Contains(High?TEXT("制空"):TEXT("反应")));
  ChoiceButton->OnClicked.Broadcast();
  TestEqual(TEXT("Only original Skill sent first"),Backend.Last.IntentKind,Kind::SubmitSkill);
  S->AdvanceCrossFlow(5); TestEqual(TEXT("Pending Skill blocks draft dispatch"),Backend.Sends,1);
  Backend.DeliverView(); S->AdvanceCrossFlow(5);
  TestEqual(TEXT("View without ACK cannot submit Branch"),Backend.Sends,1);
  Backend.DeliverAck(); S->AdvanceCrossFlow(0);
  TestEqual(TEXT("Original Branch follows confirmed Skill"),Backend.Last.IntentKind,Kind::SubmitBranchIntent);
  TestEqual(TEXT("Selected intent preserved"),Backend.Last.Branch.Intent,High?Branch::CrossHigh:Branch::CrossLow);
  TestEqual(TEXT("No repeated High/Low decision"),S->GetWidgetFromName(TEXT("TheaterChoices"))->GetVisibility(),ESlateVisibility::Collapsed);
  Backend.DeliverView(); Backend.DeliverAck();
  TestTrue(TEXT("Covered hand retains its existing widgets across Cross view and ACK"),S->GetLocalRackWidget()->GetRenderedCardWidgets()==BoardCards);
  TestTrue(TEXT("Covered pitch retains its existing slot widgets"),S->GetPitchWidget()->GetRenderedSlotWidgets()==BoardSlots);
  TestEqual(TEXT("Central route notice cannot reveal the future route"),S->GetWidgetFromName(TEXT("CrossRouteNotice"))->GetVisibility(),ESlateVisibility::Collapsed);
  TestEqual(TEXT("Both route cards stay neutral before disclosure"),
   CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossRouteHighTitle")))->GetColorAndOpacity().GetSpecifiedColor(),
   CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossRouteLowTitle")))->GetColorAndOpacity().GetSpecifiedColor());
  const int32 EntropyBefore=F.Entropy->Calls; F.Entropy->Word=1;
  S->AdvanceCrossFlow(5); TestEqual(TEXT("Elapsed phase time without a rendered scene sends nothing"),Backend.Sends,2);
  auto& Entrance=const_cast<FState&>(S->GetTacticalScene()).CrossEntrance;
  // Simulate paint notifications only in this unit fixture; the separate PIE
  // observes actual Slate paint and real elapsed time through the same clock.
  const FVector2D ViewSize(1600,700);
  Entrance.Painted(GFrameCounter-1,ViewSize,false); Entrance.Painted(GFrameCounter,ViewSize,false);
  Entrance.Tick(.1f,GFrameCounter); S->AdvanceCrossFlow(.1f);
  TestEqual(TEXT("Unready assets do not start the route"),Backend.Sends,2);
  Entrance={}; Entrance.Painted(GFrameCounter-1,ViewSize,true); Entrance.Painted(GFrameCounter,ViewSize,true);
  Entrance.Tick(3.f,GFrameCounter); S->AdvanceCrossFlow(3.f);
  TestEqual(TEXT("Entry hitch cannot consume the readable pause"),Backend.Sends,2);
  Entrance.Tick(.05f,GFrameCounter);
  for(int I=0;I<7;++I)Entrance.Tick(.05f,GFrameCounter);
  S->AdvanceCrossFlow(.05f);TestEqual(TEXT("Intro completion alone cannot trigger"),Backend.Sends,2);
  for(int I=0;I<21;++I)Entrance.Tick(.05f,GFrameCounter);
  S->AdvanceCrossFlow(.05f);TestEqual(TEXT("Post-intro readable hold is mandatory"),Backend.Sends,2);
  Entrance.Tick(.06f,GFrameCounter);
  Waiting->AdvanceCrossFlow(3); TestEqual(TEXT("Waiting side never rolls"),F.Backend(F.Defender()).Sends,0);
  S->AdvanceCrossFlow(.02f); TestEqual(TEXT("Only intermediate initial route auto-fires"),Backend.Last.IntentKind,Kind::CrossInitialRouteRoll);
  S->AdvanceCrossFlow(5); TestEqual(TEXT("Repeated tick sends once"),Backend.Sends,3);
  auto* Rebuilt=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());
  Rebuilt->SetMatchBackend(&Backend); auto Slate=Rebuilt->TakeWidget(); Rebuilt->RefreshFromPresentation(S->GetPresentation());
  Rebuilt->AdvanceCrossFlow(5); TestEqual(TEXT("Reconstruction retains dispatch identity"),Backend.Sends,3);
  Backend.DeliverView(); Backend.DeliverAck();
  S->PauseInlineFormulaRevealTimerForTesting(); S->AdvanceInlineFormulaRevealForTesting(1.5f);
  TestTrue(TEXT("Existing Roll hold keeps the disclosed route readable"),S->IsInlineFormulaRevealInputBlocked());
  TestEqual(TEXT("Route beat keeps formula hidden"),S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
  TestEqual(TEXT("High canonical range follows intent in plain language"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossRouteHighRange")))->GetText().ToString(),FString(High?TEXT("掷出 1–4"):TEXT("掷出 5–6")));
  TestEqual(TEXT("Low canonical range follows intent in plain language"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossRouteLowRange")))->GetText().ToString(),FString(High?TEXT("掷出 5–6"):TEXT("掷出 1–4")));
  const auto RangeColor=[&](const TCHAR* Name){return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetColorAndOpacity().GetSpecifiedColor();};
  TestTrue(TEXT("Only the safely disclosed route receives emphasis"),RangeColor(TEXT("CrossRouteHighTitle"))!=RangeColor(TEXT("CrossRouteLowTitle")));
  TestEqual(TEXT("Identity switches to the actual disclosed route"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossBroadcastType")))->GetText().ToString(),FString(S->GetPresentation().InlineFormula.ContestId==TEXT("Cross.High")?TEXT("高球传中"):TEXT("低球传中")));
  TestTrue(TEXT("Route notification displays only the disclosed result"),S->GetWidgetFromName(TEXT("CrossRouteNotice"))->GetVisibility()!=ESlateVisibility::Collapsed);
  TestEqual(TEXT("Route notification names the actual route"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossRouteNoticeTitle")))->GetText().ToString(),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("CrossBroadcastType")))->GetText().ToString());
  S->AdvanceInlineFormulaRevealForTesting(2.4f); TestTrue(TEXT("Cross route remains readable for the extended 2.5 second hold"),S->IsInlineFormulaRevealInputBlocked());
  S->RefreshFromPresentation(S->GetPresentation()); // Duplicate safe View must not restart the reading beat.
  S->AdvanceInlineFormulaRevealForTesting(.1f); F.Settle();
  TestFalse(TEXT("Existing hold releases without another stacked pause"),S->IsInlineFormulaRevealInputBlocked());
  TestEqual(TEXT("Route notification leaves with route reading"),S->GetWidgetFromName(TEXT("CrossRouteNotice"))->GetVisibility(),ESlateVisibility::Collapsed);
  S->SkipTacticalScene(); S->SkipTacticalScene();
  TestEqual(TEXT("Authority consumes exactly one route RNG draw"),F.Entropy->Calls,EntropyBefore+1);
  S->AdvanceCrossFlow(10); TestEqual(TEXT("Attack final stays manual"),Backend.Sends,3);
  TestEqual(TEXT("Attack label"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterContinueLabel")))->GetText().ToString(),FString(TEXT("实施战术")));
  TestEqual(TEXT("Redundant contest subtitle is collapsed"),S->GetWidgetFromName(TEXT("CrossBroadcastPhase"))->GetVisibility(),ESlateVisibility::Collapsed);
  TestTrue(TEXT("Button fits its existing action region"),CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterPrimaryBounds")))->GetWidthOverride()
   <=CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterBottomBounds")))->GetWidthOverride());
  auto* Row=CastChecked<UHorizontalBox>(S->GetWidgetFromName(TEXT("CrossBroadcastRow")));
  TArray<UWidget*> VisibleChildren;
  for(auto* Child:Row->GetAllChildren())
   if(Child->GetVisibility()!=ESlateVisibility::Collapsed)VisibleChildren.Add(Child);
  TestEqual(TEXT("Identity, comparison and action are the only visible row groups"),VisibleChildren.Num(),3);
  TestTrue(TEXT("Tactical identity lives in left footer"),Row->GetChildAt(0)==S->GetWidgetFromName(TEXT("CrossBroadcastIdentity")));
  TestTrue(TEXT("Action group is far right"),!VisibleChildren.IsEmpty() && VisibleChildren.Last()==S->GetWidgetFromName(TEXT("TheaterBottomBounds")));
  TestEqual(TEXT("Broadcast header visible over pitch"),S->GetWidgetFromName(TEXT("CrossBroadcastHeader"))->GetVisibility(),ESlateVisibility::HitTestInvisible);
  TestEqual(TEXT("Original large title collapsed"),S->GetWidgetFromName(TEXT("TheaterTopBounds"))->GetVisibility(),ESlateVisibility::Collapsed);
  for(const auto* Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
  {
   const auto Name=[&](const TCHAR* Suffix){return FName(*(FString(Prefix)+Suffix));};
   TestEqual(TEXT("Full identities remain on pitch, duplicate footer roster collapsed"),S->GetWidgetFromName(Name(TEXT("People")))->GetVisibility(),ESlateVisibility::Collapsed);
   TestEqual(TEXT("Cross formula has a bounded visible panel"),CastChecked<UBorder>(S->GetWidgetFromName(Name(TEXT("Panel"))))->GetBrushColor(),FLinearColor::White);
   TestEqual(TEXT("Current value caption preserved"),CastChecked<UTextBlock>(S->GetWidgetFromName(Name(TEXT("ValueLabel"))))->GetText().ToString(),FString(TEXT("当前值")));
   TestEqual(TEXT("Equals sign preserved"),CastChecked<UTextBlock>(S->GetWidgetFromName(Name(TEXT("Equal"))))->GetText().ToString(),FString(TEXT("=")));
  }
  TestEqual(TEXT("Attack RHS remains authoritative displayed subtotal"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterAttackFinalNumber")))->GetText().ToString(),S->GetPresentation().InlineFormula.AttackRow.DisplayedResultLabel);
  TestEqual(TEXT("Old oversized prompt removed"),S->GetWidgetFromName(TEXT("TheaterInfoBar"))->GetParent()->GetVisibility(),ESlateVisibility::Collapsed);
  TestEqual(TEXT("Solid lower background"),CastChecked<UBorder>(S->GetWidgetFromName(TEXT("CrossBroadcastBand")))->Background.GetTint(FWidgetStyle()).A,1.f);
  TestFalse(TEXT("No premature outcome"),S->GetTacticalScene().Facts.Outcome!=EOutcome::None);
  const auto& Header=S->GetPresentation().Header;
  for(bool Attack:{true,false})
  {
   const auto Side=Attack?S->GetPresentation().InlineFormula.AttackRow.Side:S->GetPresentation().InlineFormula.DefenseRow.Side;
   const auto Expected=FMCodexMatchShellStyle::HeaderAccent(Side==Header.LeftPlayerSide?Header.LeftAttackTurnTracker.PrimarySideColor:Header.RightAttackTurnTracker.PrimarySideColor);
   TestEqual(TEXT("Footer color matches the same owner's scoreboard/portrait accent"),CastChecked<UBorder>(S->GetWidgetFromName(Attack?TEXT("TheaterAttackAccent"):TEXT("TheaterDefenseAccent")))->GetBrushColor(),Expected);
   const auto& Brush=CastChecked<UBorder>(S->GetWidgetFromName(Attack?TEXT("TheaterAttackPanel"):TEXT("TheaterDefensePanel")))->Background;
   TestEqual(TEXT("Both interiors use neutral navy without a team tint"),Brush.GetTint(FWidgetStyle()),FMCodexMatchShellStyle::Color(5,20,32));
   TestEqual(TEXT("Complete outline follows actual team, including B attack"),Brush.OutlineSettings.Color.GetSpecifiedColor(),Expected);
   TestEqual(TEXT("Team outline remains thin"),Brush.OutlineSettings.Width,1.f);
   TestEqual(TEXT("Thick identity stripe is removed"),S->GetWidgetFromName(Attack?TEXT("TheaterAttackAccent"):TEXT("TheaterDefenseAccent"))->GetVisibility(),ESlateVisibility::Collapsed);
   TestTrue(TEXT("Formula heading identifies actual owner"),CastChecked<UTextBlock>(S->GetWidgetFromName(Attack?TEXT("TheaterAttackSide"):TEXT("TheaterDefenseSide")))->GetText().ToString().Contains(Side==EInitialTurnOrderPlayer::PlayerA?TEXT("玩家 A"):TEXT("玩家 B")));
  }
  for(const auto Size:{FVector2D(1920,1080),FVector2D(2048,1080),FVector2D(1280,960)})
  {
   FitCrossViewport(*S->WidgetTree,true,Size);
   const float Height=CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterDesignBounds")))->GetHeightOverride();
   TestTrue(TEXT("Cross design aspect exactly fills usable viewport width"),FMath::IsNearlyEqual(1600.f/Height,float(Size.X/Size.Y),.001f));
   TestEqual(TEXT("Pitch and footer consume the full height"),CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterSpatialLane")))->GetHeightOverride()+200.f,Height);
  }
  FitCrossViewport(*S->WidgetTree,false,FVector2D(2048,1080));
  TestEqual(TEXT("Other theaters retain fixed original design"),CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterDesignBounds")))->GetHeightOverride(),900.f);
  auto Idle=S->GetTacticalScene(); Idle.Tick(.2f); // Settle the existing route-position blend after fixture Skip.
  TestTrue(TEXT("Manual waiting enables visible guide motion"),Idle.IsCrossIdle());
  const auto Origin=CrossBallStart(Idle),Crosser=CrossParticipantAnchor(Idle,Role::Carrier),Zone=CrossReceiveZone(Idle.CrossLowBlend);
  TestTrue(TEXT("Ball originates adjacent to actual Carrier role"),(Origin-Crosser).Size()<55.f);
  TestTrue(TEXT("Wide crosser stays separated from receiving contest and goalkeeper"),(Crosser-Zone).Size()>300.f && (Crosser-CrossParticipantAnchor(Idle,Role::Goalkeeper)).Size()>190.f);
  TestTrue(TEXT("Receiver is the closest attacking role to the contest"),(CrossParticipantAnchor(Idle,Role::Runner)-Zone).Size()<(Crosser-Zone).Size());
  if(!High)TestTrue(TEXT("Low cutback originates from near flank, not goalmouth"),Origin.Y>Zone.Y && Crosser.X>Zone.X);
  const auto BallBefore=CrossIntentBall(Idle),RunnerBefore=CrossParticipantAnchor(Idle,Role::Runner);
  const float DashBefore=Idle.CrossIdleDashOffset(),BreathBefore=Idle.CrossIdleBreath(); Idle.Tick(1.3f);
  TestTrue(TEXT("Waiting visibly replays the attempted pass and receiver approach"),
   (CrossIntentBall(Idle)-BallBefore).Size()>40.f && (CrossParticipantAnchor(Idle,Role::Runner)-RunnerBefore).Size()>50.f);
  TestTrue(TEXT("Rendered dash offset and target radius change perceptibly"),FMath::Abs(Idle.CrossIdleDashOffset()-DashBefore)>10.f && FMath::Abs(Idle.CrossIdleBreath()-BreathBefore)>.2f);
  // Sample the actual moving pass against portrait and label geometry, including
  // the shorter field in a wide viewport. Both routes use real projected names.
  auto Loop=Idle; Loop.CrossIdleTime=0; Loop.CrossIdleSettle=0;
  bool Clear=true,CloseMarker=true,Unresolved=true;
  for(int Frame=0;Frame<60;++Frame)
  {
   const auto Mark=CrossParticipantAnchor(Loop,Role::Marker),Carrier=CrossParticipantAnchor(Loop,Role::Carrier);
   CloseMarker&=(Mark-Carrier).Size()<180.f && (Mark-Carrier).Size()<(Mark-CrossParticipantAnchor(Loop,Role::Runner)).Size();
   const auto Ball=CrossIntentBall(Loop);
   Unresolved&=Loop.Phase==EPhase::FormulaHold && Loop.Facts.Outcome==EOutcome::None
    && (Ball-Zone).Size()>90.f && !IsInsideGoal(Ball,Loop.Facts.Method);
   if(Loop.CrossWaitingBallOpacity()>.05f)for(float YScale:{1.f,.92f})for(const auto& Participant:Loop.Facts.Participants)
   {
    auto Anchor=CrossParticipantAnchor(Loop,Participant.Role);Anchor.Y*=YScale;
    const FVector2D PaintedBall(Ball.X,Ball.Y*YScale);
    const float NameWidth=FMath::Clamp(float(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(
     Participant.Name,FMCodexMatchShellStyle::Font(18,true)).X)+24.f,94.f,220.f);
    auto ClearsRect=[&](FVector2D Min,FVector2D Max)
    {return (PaintedBall-FVector2D(FMath::Clamp(PaintedBall.X,Min.X,Max.X),FMath::Clamp(PaintedBall.Y,Min.Y,Max.Y))).Size()>13.5f;};
    Clear&=(PaintedBall-Anchor).Size()>47.5f
     && ClearsRect(Anchor+FVector2D(-NameWidth*.5f,39),Anchor+FVector2D(NameWidth*.5f,66))
     && ClearsRect(Anchor+FVector2D(-32,69),Anchor+FVector2D(32,92));
   }
   Loop.Tick(CrossIdleCycleSeconds/60.f);
  }
  TestTrue(TEXT("Looping ball clears portrait, name and role plaques"),Clear);
  TestTrue(TEXT("Marker follows the nearby crosser throughout High/Low rehearsal"),CloseMarker);
  TestTrue(TEXT("Repeated preparation never reaches a receiving touch or result"),Unresolved);
  auto WrapBefore=Idle,WrapAfter=Idle;WrapBefore.CrossIdleTime=CrossIdleCycleSeconds-.001f;WrapAfter.CrossIdleTime=.001f;
  TestTrue(TEXT("Loop seam preserves the ball and player pose"),(CrossIntentBall(WrapBefore)-CrossIntentBall(WrapAfter)).Size()<1.f
   && (CrossParticipantAnchor(WrapBefore,Role::Runner)-CrossParticipantAnchor(WrapAfter,Role::Runner)).Size()<1.f);
  WrapBefore.CrossIdleTime=.7999f;WrapAfter.CrossIdleTime=.8001f;
  TestTrue(TEXT("Replay resets the ball only while invisible"),WrapBefore.CrossWaitingBallOpacity()<.001f && WrapAfter.CrossWaitingBallOpacity()<.001f);
  const float ExistingClock=Idle.CrossIdleTime; Idle.Sync(Idle.Facts);
  TestEqual(TEXT("Repeated safe view cannot restart idle cycle"),Idle.CrossIdleTime,ExistingClock);
  auto Revealing=S->GetPresentation().InlineFormula; Revealing.bDiceRevealVisible=true;
  const auto AtRoll=CrossIntentBall(Idle),RunnerAtRoll=CrossParticipantAnchor(Idle,Role::Runner);
  Idle.Sync(ProjectCross(Revealing,Header,true));
  TestTrue(TEXT("Clicking roll does not snap the ball or player"),CrossIntentBall(Idle).Equals(AtRoll) && CrossParticipantAnchor(Idle,Role::Runner).Equals(RunnerAtRoll));
  Idle.Tick(.25f);
  TestTrue(TEXT("Dice reveal continues the existing loop"),Idle.IsCrossIdle());
  TestTrue(TEXT("Reveal advances elapsed loop time"),FMath::IsNearlyEqual(Idle.CrossIdleTime,FMath::Fmod(ExistingClock+.25f,CrossIdleCycleSeconds)));
  const auto RollingBall=CrossIntentBall(Idle),RollingRunner=CrossParticipantAnchor(Idle,Role::Runner);Idle.Tick(1.f);
  TestTrue(TEXT("Ball and player continue moving during the roll"),!CrossIntentBall(Idle).Equals(RollingBall) && !CrossParticipantAnchor(Idle,Role::Runner).Equals(RollingRunner));
  const float BeforeWait=Idle.CrossIdleTime;
  Revealing.bDiceRevealVisible=false;Idle.Sync(ProjectCross(Revealing,Header,true));
  TestEqual(TEXT("Next manual wait preserves the loop without a restart"),Idle.CrossIdleTime,BeforeWait);
  Revealing.bNarrativeAvailable=true;Revealing.bDiceRevealVisible=true;Revealing.SpatialOutcome=EMatchPlayResolutionDecisionOutcome::Goal;
  Idle.Sync(ProjectCross(Revealing,Header,true));Idle.Tick(.2f);
  TestTrue(TEXT("Final reveal hold keeps rehearsing without exposing the result"),Idle.IsCrossIdle() && Idle.Facts.Outcome==EOutcome::None);
  Revealing.bDiceRevealVisible=false;Idle.Sync(ProjectCross(Revealing,Header,true));
  TestTrue(TEXT("Only displayed result hands the scene to the outcome animation"),!Idle.IsCrossIdle() && Idle.Phase==EPhase::Outcome && Idle.Facts.Outcome==EOutcome::Goal);
  Idle.Sync({});Idle.Tick(.1f);TestFalse(TEXT("Hidden scene cannot keep idle alive"),Idle.IsCrossIdle());
  TestEqual(TEXT("Teardown clears replay time"),Idle.CrossIdleTime,0.f);
  for(const auto Outcome:{EOutcome::Goal,EOutcome::DefensiveSuccess})
  {
   auto Result=S->GetTacticalScene(); auto Facts=Result.Facts; Facts.Outcome=Outcome;Facts.bManualWaiting=false;Result.Sync(Facts);
   TestFalse(TEXT("Outcome owns scene instead of idle"),Result.IsCrossIdle());
   Result.Tick(1.20f);TestTrue(TEXT("Cross field action continues past old duration"),Result.Phase==EPhase::Outcome && Result.CrossOutcomeActionProgress()<1.f);
   Result.Tick(.25f);TestTrue(TEXT("Cross action completes at 1.44 seconds before original hold"),Result.IsCrossOutcomeVisualHold());
   auto Gated=S->GetPresentation().InlineFormula;Gated.bNarrativeAvailable=true;Result.Gate(Gated);
   TestFalse(TEXT("Original handoff stays gated through field hold"),Gated.bNarrativeAvailable);
   Result.Tick(.30f);TestEqual(TEXT("Both outcomes release at 1.74 seconds"),Result.Phase,EPhase::ResultHold);
   TestEqual(TEXT("Only the disclosed Cross Goal starts the existing celebration"),Result.Celebration.IsActive(),Outcome==EOutcome::Goal);
  }
  TestEqual(TEXT("Compact footer"),CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("CrossBroadcastBounds")))->GetHeightOverride(),200.f);
  TestEqual(TEXT("Result plate waits for outcome"),S->GetWidgetFromName(TEXT("CrossResultPlate"))->GetVisibility(),ESlateVisibility::Collapsed);
  CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
  TestEqual(TEXT("Manual attacker typed die"),Backend.Last.IntentKind,High?Kind::CrossHighAttackRoll:Kind::CrossLowAttackRoll);
  Backend.DeliverView(); Backend.DeliverAck(); F.Settle(); Waiting->SkipTacticalScene(); Waiting->SkipTacticalScene();
  S->AdvanceCrossFlow(10); Waiting->AdvanceCrossFlow(10);
  TestEqual(TEXT("Defender final stays manual"),F.Backend(F.Defender()).Sends,0);
  TestFalse(TEXT("Attacker cannot roll for defender"),S->GetWidgetFromName(TEXT("TheaterContinue"))->GetIsEnabled());
  TestEqual(TEXT("Defense label"),CastChecked<UTextBlock>(Waiting->GetWidgetFromName(TEXT("TheaterContinueLabel")))->GetText().ToString(),FString(TEXT("防守方掷点")));
  TestTrue(TEXT("Actual defender manual wait also enables idle"),Waiting->GetTacticalScene().IsCrossIdle());
  if (!Continue(*this,F,F.Defender(),High?Kind::CrossHighDefenseRoll:Kind::CrossLowDefenseRoll)) return false;
  F.Settle(); S->SkipTacticalScene(); Waiting->SkipTacticalScene();
  TestTrue(TEXT("Final result still comes from disclosed authority"),S->GetTacticalScene().Facts.Outcome!=EOutcome::None);
  TestEqual(TEXT("Result plate appears with disclosed narrative"),S->GetWidgetFromName(TEXT("CrossResultPlate"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
  TestTrue(TEXT("Result has high-contrast backing"),CastChecked<UBorder>(S->GetWidgetFromName(TEXT("CrossResultPlate")))->Background.GetTint(FWidgetStyle()).A>.9f);
  // Presentation-only consumer switch: no second gameplay scenario/suite.
  auto Other=S->GetPresentation(); auto OtherFormula=Other.InlineFormula; OtherFormula.ContestId=TEXT("LongShot.DirectShot");
  FMCodexResolutionTheaterPrototype::FTakerInspection Inspection;
  FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,Other,OtherFormula,Other.Header,false,Inspection);
  TestTrue(TEXT("Other tactics regain the original live layout home"),S->GetWidgetFromName(TEXT("TheaterDuel"))->GetParent()==S->GetWidgetFromName(TEXT("TheaterDuelHome")));
  TestEqual(TEXT("Other tactics retain original panel allocation"),CastChecked<USizeBox>(S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds")))->GetWidthOverride(),600.f);
  TestEqual(TEXT("Other tactics hide Cross overlay"),S->GetWidgetFromName(TEXT("CrossBroadcastHeader"))->GetVisibility(),ESlateVisibility::Collapsed);
  TestEqual(TEXT("Other tactics restore participant list"),S->GetWidgetFromName(TEXT("TheaterAttackPeople"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
  const auto LegacyFont=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterTitle")))->GetFont();
  const auto RestoredFont=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterContinueLabel")))->GetFont();
  TestTrue(TEXT("Other tactics restore their actual font asset"),LegacyFont.FontObject==RestoredFont.FontObject && LegacyFont.CompositeFont==RestoredFont.CompositeFont);
  // ThroughBall's independent-event consumer has an early return; it must also
  // restore the reused controls after a Cross, without executing another tactic.
  FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,S->GetPresentation(),S->GetPresentation().InlineFormula,Other.Header,false,Inspection);
  Other.ThroughBallResolution.bVisible=true;
  Other.ThroughBallResolution.Stage=EFMCodexUMGThroughBallStage::InitialRoute;
  Other.ThroughBallResolution.bNarrativeAvailable=false;
  FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,Other,OtherFormula,Other.Header,false,Inspection);
  TestTrue(TEXT("Independent event regains original action home"),S->GetWidgetFromName(TEXT("TheaterBottomBounds"))->GetParent()==S->GetWidgetFromName(TEXT("TheaterBottomHome")));
  TestEqual(TEXT("Independent event hides Cross header"),S->GetWidgetFromName(TEXT("CrossBroadcastHeader"))->GetVisibility(),ESlateVisibility::Collapsed);
  // Returning to a board consumer immediately renders the latest safe model;
  // deferred hidden widgets must never resurrect an old roster on exit.
  auto BoardReturn=Model; BoardReturn.LocalRack.Cells.Reset();
  S->RefreshFromPresentation(BoardReturn);
  TestTrue(TEXT("Cross exit refreshes the latest safe roster"),S->GetLocalRackWidget()->GetPresentation().Cells.IsEmpty());
  TestTrue(TEXT("Cross exit removes stale rendered cards"),S->GetLocalRackWidget()->GetRenderedCardWidgets().IsEmpty());
  Rebuilt->ResetPresentationSession(); TestFalse(TEXT("New match clears auto-roll identity"),Backend.CrossPresentationFlow.bRouteSubmitted);
  Rebuilt->SetMatchBackend(nullptr); S->SetMatchBackend(&F.Backend(PC));
 }
 return true;
}
#endif
