#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"
namespace FMCodexOptionalDeclineTests
{
using namespace FMCodexNetworkInitialRouteTests;
using Decline = EFMCodexNetworkDeclineAction;
inline Decline Action(Kind K) { return K == Kind::DeclineRunner ? Decline::Runner : K == Kind::DeclineHelper ? Decline::Helper : Decline::Skill; }
inline Kind Command(const FString& S) { return S == TEXT("Runner") ? Kind::DeclineRunner : S == TEXT("Helper") ? Kind::DeclineHelper : Kind::DeclineSkill; }
inline AFMCodexNetworkMatchPlayerController* Actor(FFixture& F,Kind K) { return K == Kind::DeclineHelper ? F.Defender() : F.Attacker(); }
inline bool Prepare(FFixture& F,Kind K,bool Final=false)
{
 if(Final)
 {
  F.Entropy->Word=0;
  if(!F.Send(F.Attacker(),Kind::RequestInitialActionPointRoll))return false;
  FMatchPlayAuthoritativeAdvanceAfterTerminalRequest R;
  R.AttackSequence=F.Attacker()->GetOwnerView().AttackSequence;R.RequestingSide=F.Attacker()->GetOwnerView().ViewerSide;
  if(!Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal,R)).bSuccess)return false;
  Access::Publish(*F.Mode);
 }
 F.Entropy->Word=3;
 if(!Access::Runtime(*F.Mode).PrepareInitialRouteMilestone(ESkillRuleType::LongShot,Action(K)))return false;
 Access::Publish(*F.Mode);return true;
}
inline Envelope Request(FFixture& F,AFMCodexNetworkMatchPlayerController* PC,Kind K)
{ auto E=F.Request(PC);E.IntentKind=K;return E; }
inline void Adopted(FAutomationTestBase& T,FFixture& F,Kind K,const FUnchanged& Before)
{
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();
 T.TestEqual(TEXT("Accepted decline coordinates exactly once"),F.Calls(),Before.CoordinatorCalls+1);
 T.TestEqual(TEXT("Accepted decline publishes exactly once"),Access::Revision(*F.Mode),Before.Revision+1);
 T.TestEqual(TEXT("Decline draws zero entropy across all providers"),F.Entropy->Calls,Before.EntropyCalls);
 T.TestEqual(TEXT("Score A unchanged"),State.RuntimeState.PlayerAState.Score,Before.State.RuntimeState.PlayerAState.Score);
 T.TestEqual(TEXT("Score B unchanged"),State.RuntimeState.PlayerBState.Score,Before.State.RuntimeState.PlayerBState.Score);
 T.TestEqual(TEXT("No phantom scorer/history"),State.GoalHistory.Num(),Before.State.GoalHistory.Num());
 if(K==Kind::DeclineSkill)
 {
  T.TestFalse(TEXT("Skill decline directly completes attack"),State.bHasCurrentAttack);
  T.TestTrue(TEXT("Skill decline changes attacking side"),State.RuntimeState.CurrentAttackingPlayer!=Before.State.RuntimeState.CurrentAttackingPlayer);
  for(auto* PC:{F.A,F.B}) {T.TestFalse(TEXT("No invented terminal Advance"),PC->GetOwnerView().bCanAdvance);T.TestTrue(TEXT("Old participant identities removed"),PC->GetOwnerView().SelectedRunner.Choice.RunnerCardId.IsNone()&&PC->GetOwnerView().SelectedHelper.Choice.HelperCardId.IsNone());}
 }
 else
 {
  T.TestTrue(TEXT("Participant decline keeps attack"),State.bHasCurrentAttack);
  const auto& A=State.CurrentAttack;
  T.TestEqual(TEXT("Reaches genuine Skill selection"),A.SelectionStage,EMatchPlayCurrentAttackSelectionStage::AwaitingSkill);
  T.TestTrue(TEXT("Helper formally absent"),!A.ActionPreparation.bHasHelper&&A.ActionPreparation.HelperCardId.IsNone());
  T.TestTrue(TEXT("Skill remains authority pending"),A.ActionPreparation.SkillId.IsNone()&&A.ActionPreparation.ActionType==ESkillRuleType::None);
  T.TestEqual(TEXT("Runner absent only for Runner decline"),A.ActionPreparation.RunnerCardId.IsNone(),K==Kind::DeclineRunner);
  T.TestTrue(TEXT("Canonical LongShot remains available after decline"),F.Attacker()->GetOwnerView().SkillOptions.ContainsByPredicate([](const auto& O){return O.Choice.SkillId==FName(TEXT("Canonical.Skill.LongShot.3.5"));}));
  T.TestEqual(TEXT("Next actor owns genuine Skill decline"),F.Attacker()->GetOwnerView().DeclineAction,Decline::Skill);
  if(K==Kind::DeclineRunner)for(const auto& O:F.Attacker()->GetOwnerView().SkillOptions)
   T.TestFalse(TEXT("No Runner-dependent tactic reconstructed by client"),O.Choice.SkillId.ToString().Contains(TEXT("Cross"))||O.Choice.SkillId.ToString().Contains(TEXT("PassControl"))||O.Choice.SkillId.ToString().Contains(TEXT("ThroughBall")));
 }
}
}
#endif
