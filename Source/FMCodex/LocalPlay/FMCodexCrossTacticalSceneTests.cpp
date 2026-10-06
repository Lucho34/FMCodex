#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexTacticalScene.h"
#include "FMCodexMatchShellStyle.h"
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#include "Blueprint/WidgetTree.h"
#include "Components/NativeWidgetHost.h"
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
   TestTrue(TEXT("Marker pressures crosser, not receiver"),(CrossParticipantAnchor(S,Role::Marker)-Carrier).Size()<180);
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
 FUIFixture F;
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
  CrossDeliveryControl(0).Y<HighCarrier.Y && CrossDeliveryControl(1).Y>CrossParticipantAnchor(S,Role::Carrier).Y
  && CrossDeliveryControl(1).Y<CrossReceiveZone(1).Y);
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
   TestFalse(TEXT("Cannot skip authority/reel"),S.Skip());
   V.bDefenseRowActive=true;V.bAttackRowActive=false;
   V.SpatialOutcome=Outcome;V.bNarrativeAvailable=true;V.bDiceRevealVisible=true;V.NarrativeHeadline=TEXT("权威叙事");
   S.Sync(ProjectCross(V,M.Header,true));TestEqual(TEXT("No outcome before mandatory reveal"),S.Facts.Outcome,EOutcome::None);
   V.bDiceRevealVisible=false;S.Sync(ProjectCross(V,M.Header,true));
   TestEqual(TEXT("Revealed typed fact begins outcome"),S.Phase,EPhase::Outcome);
   TestEqual(TEXT("No inferred save/header; typed result only"),S.Facts.Outcome,Outcome==EMatchPlayResolutionDecisionOutcome::Goal?EOutcome::Goal:EOutcome::DefensiveSuccess);
   auto Gated=V;S.Gate(Gated);TestFalse(TEXT("Headline waits for arrival"),Gated.bNarrativeAvailable);
   TestEqual(TEXT("Formula subtotal unchanged"),Gated.AttackRow.KnownNonRollSubtotalLabel,V.AttackRow.KnownNonRollSubtotalLabel);
   TestTrue(TEXT("Both outcomes enter the real receiving corridor before continuation"),CrossOutcomeBall(S,CrossDeliveryFraction).Equals(CrossReceiveZone(S.CrossLowBlend)));
   S.Tick(CrossOutcomeActionSeconds);
   TestTrue(TEXT("Action completes into brief visual hold, not terminal CTA"),S.Phase==EPhase::Outcome && S.IsCrossOutcomeVisualHold());
   TestEqual(TEXT("Action freezes at its final frame"),S.CrossOutcomeActionProgress(),1.f);
   const auto End=CrossOutcomeBall(S,1.f),Receiver=CrossParticipantAnchor(S,Role::Runner);
   TestEqual(TEXT("Only Goal finishes inside goal"),IsInsideGoal(End),Outcome==EMatchPlayResolutionDecisionOutcome::Goal);
   S.Tick(CrossOutcomeHoldSeconds*.5f);
   TestTrue(TEXT("Ball and receiver remain settled throughout visual hold"),S.IsCrossOutcomeVisualHold() && CrossOutcomeBall(S,1.f).Equals(End) && CrossParticipantAnchor(S,Role::Runner).Equals(Receiver));
   Gated=V;S.Gate(Gated);TestFalse(TEXT("Final visual hold still gates headline"),Gated.bNarrativeAvailable);
   S.Tick(CrossOutcomeHoldSeconds*.5f+.001f);TestEqual(TEXT("Short visual hold hands off to original result flow"),S.Phase,EPhase::ResultHold);
   TestTrue(TEXT("Goal endpoint inside goal"),IsInsideGoal(OutcomeTarget(EOutcome::Goal,S.Facts.Method)));
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
#endif
