#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkContestTestFixture.h"
namespace FMCodexSpecializedShotTests
{
using namespace FMCodexNetworkInitialRouteTests;
using Command = EMatchPlayAuthoritativeCommandKind;
using Outcome = EFMCodexNetworkTerminalOutcome;
inline const TArray<Kind>& Commands()
{
 static const TArray<Kind> K={Kind::LongShotDirectAttackRoll,Kind::LongShotDirectDefenseRoll,Kind::LongShotDeadCornerRoll,
  Kind::CutInsideShotDirectAttackRoll,Kind::CutInsideShotDirectDefenseRoll,Kind::CutInsideShotDeadCornerRoll};return K;
}
inline bool IsCut(Kind K){return K==Kind::CutInsideShotDirectAttackRoll||K==Kind::CutInsideShotDirectDefenseRoll||K==Kind::CutInsideShotDeadCornerRoll;}
inline bool IsDefense(Kind K){return K==Kind::LongShotDirectDefenseRoll||K==Kind::CutInsideShotDirectDefenseRoll;}
inline bool IsPaired(Kind K){return K==Kind::LongShotDeadCornerRoll||K==Kind::CutInsideShotDeadCornerRoll;}
inline Kind AttackKind(bool Cut){return Cut?Kind::CutInsideShotDirectAttackRoll:Kind::LongShotDirectAttackRoll;}
inline Kind DefenseKind(bool Cut){return Cut?Kind::CutInsideShotDirectDefenseRoll:Kind::LongShotDirectDefenseRoll;}
inline Kind PairKind(bool Cut){return Cut?Kind::CutInsideShotDeadCornerRoll:Kind::LongShotDeadCornerRoll;}
inline FMatchPlayPlayerIntent Canonical(const Envelope& E,Side Player)
{
 auto Make=[&](auto R,Command C){R.AttackSequence=E.ExpectedAttackSequence;R.RequestingSide=Player;return FMatchPlayPlayerIntent::Create(C,R);};
 switch(E.IntentKind)
 {
 case Kind::LongShotDirectAttackRoll:return Make(FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest{},Command::ResolveLongShotDirectAttackRoll);
 case Kind::LongShotDirectDefenseRoll:return Make(FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest{},Command::ResolveLongShotDirectDefenseRoll);
 case Kind::LongShotDeadCornerRoll:return Make(FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest{},Command::ResolveLongShotDeadCornerRoll);
 case Kind::CutInsideShotDirectAttackRoll:return Make(FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest{},Command::ResolveCutInsideShotDirectAttackRoll);
 case Kind::CutInsideShotDirectDefenseRoll:return Make(FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest{},Command::ResolveCutInsideShotDirectDefenseRoll);
 default:return Make(FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest{},Command::ResolveCutInsideShotDeadCornerRoll);
 }
}
struct FShotFixture : FFixture
{
 using FFixture::FFixture;
 bool ReadyBranch(bool Cut=false,bool Dead=false,bool GK=false)
 {
  Entropy->Word=3;
  if(!ReachBranch(Cut?ESkillRuleType::CutInsideShot:ESkillRuleType::LongShot,GK))return false;
  Payload P;P.Intent=Dead?Branch::DeadCorner:Branch::DirectShot;
  return Send(Attacker(),Kind::SubmitBranchIntent,{},{},{},{},{},{},P);
 }
 Envelope For(Kind K,AFMCodexNetworkMatchPlayerController* PC=nullptr)
 {RequestedKind=K;return Request(PC?PC:IsDefense(K)?Defender():Attacker());}
 void Dice(int32 FirstD6,int32 SecondD6=1){Entropy->Word=FirstD6-1;Entropy->PendingWords={uint32(FirstD6-1),uint32(SecondD6-1)};}
 FFMCodexNetworkPlayerIntentAck Roll(Kind K,int32 FirstD6=6,int32 SecondD6=1)
 {
  Dice(FirstD6,SecondD6);auto* PC=IsDefense(K)?Defender():Attacker();Envelope E;auto& C=Client(PC);
  const bool Began=K==Kind::AdvanceAfterTerminal?C.BeginAdvance(PC->GetOwnerView(),E):C.BeginOrdinaryContest(PC->GetOwnerView(),K,E);
  if(!Began||E.RequestId<Next(PC))
  {
   if(Began){FFMCodexNetworkPlayerIntentAck Rejected;Rejected.MatchInstanceId=E.MatchInstanceId;Rejected.RequestId=E.RequestId;Rejected.Code=Code::InvalidPayload;C.ObserveAck(Rejected);}
   const auto Ack=Mode->SubmitConnectionPlayerIntent(PC,For(K,PC));Entropy->PendingWords.Reset();return Ack;
  }
  Next(PC)=E.RequestId+1;const auto Ack=Mode->SubmitConnectionPlayerIntent(PC,E);C.ObserveView(PC->GetOwnerView());C.ObserveAck(Ack);Entropy->PendingWords.Reset();return Ack;
 }
 bool Ready(Kind K){return ReadyBranch(IsCut(K),IsPaired(K))&&(!IsDefense(K)||Roll(AttackKind(IsCut(K)),3).Code==Code::Accepted);}
 FFMCodexLocalMatchInteractionView Safe(int32 Count,bool Terminal=true,bool Route=true,bool Pending=true)
 {
  FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=true;D.bRevealRouteRoll=Route;
  D.RevealedContestD6Count=Count;D.bRevealTerminalOutcome=Terminal;D.bPreservePendingOrdinaryFormula=Pending;
  return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Access::Session(*Mode).GetStateSnapshot(),Access::CallerRules(*Mode),Side::PlayerA,D);
 }
};
}
#endif
