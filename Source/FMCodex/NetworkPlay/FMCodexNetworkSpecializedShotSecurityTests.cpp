#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSpecializedShotTestFixture.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSpecializedShotSecurity,"FMCodex.NetworkPlay.SpecializedShotTransport.Security",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSpecializedShotSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(auto K:FMCodexSpecializedShotTests::Commands())
 for(const TCHAR* Case:{TEXT("Side"),TEXT("Phase"),TEXT("Match"),TEXT("Sequence"),TEXT("HugeId"),TEXT("Internal"),TEXT("Nonparticipant"),TEXT("Payload"),TEXT("ProviderFirst"),TEXT("ProviderSecond"),TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("StaleNext")})
 {
  if(FString(Case)==TEXT("ProviderSecond")&&!FMCodexSpecializedShotTests::IsPaired(K))continue;
  auto P=FString::Printf(TEXT("%s.%d.%s"),S,int32(K),Case);N.Add(P);C.Add(P);
 }
}
bool FFMCodexSpecializedShotSecurity::RunTest(const FString& P)
{
 using namespace FMCodexSpecializedShotTests;TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));
 const auto K=static_cast<Kind>(FCString::Atoi(*Parts[1]));const auto Case=Parts[2];FShotFixture F(Parts[0]==TEXT("B"));
 if(Case!=TEXT("Phase")&&!TestTrue(TEXT("Exact canonical checkpoint"),F.Ready(K)))return false;
 auto* PC=IsDefense(K)?F.Defender():F.Attacker();auto E=F.For(K,PC);auto Expected=Code::AuthorityRejected;
 if(Case==TEXT("StaleNext"))
 {
  if(!IsDefense(K)&&F.Roll(K,6,6).Code!=Code::Accepted)return false;
  if(!IsPaired(K)&&F.Roll(DefenseKind(IsCut(K)),1).Code!=Code::Accepted)return false;
  if(F.Roll(Kind::AdvanceAfterTerminal).Code!=Code::Accepted)return false;
  if(!TestTrue(TEXT("Actual corresponding state at next attack"),F.Ready(K)))return false;
  PC=IsDefense(K)?F.Defender():F.Attacker();E.RequestId=F.Next(PC)++;const FFrozen Before(F);
  TestEqual(TEXT("Old sequence rejected at real matching checkpoint"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::StaleAttackSequence);Before.Verify(*this,F);return true;
 }
 if(Case==TEXT("AckFirst")||Case==TEXT("ViewFirst"))
 {
  auto& Client=F.Client(PC);Client.ObserveView(PC->GetOwnerView());if(!TestTrue(TEXT("Shared pending begins"),Client.BeginOrdinaryContest(PC->GetOwnerView(),K,E)))return false;
  const auto Old=PC->GetOwnerView();F.Dice(6,6);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);const auto New=PC->GetOwnerView();
  TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);
  if(Case==TEXT("AckFirst")){Client.ObserveAck(Ack);TestTrue(TEXT("ACK awaits View"),Client.IsPending());Client.ObserveView(Old);TestTrue(TEXT("Old View cannot release"),Client.IsPending());Client.ObserveView(New);}
  else{Client.ObserveView(New);TestTrue(TEXT("View awaits own ACK"),Client.IsPending());Client.ObserveAck(Ack);}
  TestFalse(TEXT("Both release pending"),Client.IsPending());Client.ObserveAck(Ack);Client.ObserveView(Old);TestFalse(TEXT("Duplicates do not reopen"),Client.IsPending());return true;
 }
 if(Case==TEXT("Side")){PC=IsDefense(K)?F.Attacker():F.Defender();E.RequestId=F.Next(PC)++;}
 if(Case==TEXT("Match")){E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
 if(Case==TEXT("Sequence")){++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
 if(Case==TEXT("HugeId")){E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
 if(Case==TEXT("Internal")){E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
 if(Case==TEXT("Nonparticipant")){PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
 if(Case==TEXT("Payload")){E.Skill.SkillId=TEXT("ForgedSkill");E.Branch.Intent=Branch::DeadCorner;Expected=Code::InvalidPayload;}
 const int32 FailedAttempts=Case==TEXT("ProviderFirst")?1:Case==TEXT("ProviderSecond")?2:0;
 const FFrozen Before(F);F.Dice(6,6);if(FailedAttempts)F.Entropy->FailOnCall=Before.Entropy+FailedAttempts;
 const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);TestEqual(TEXT("Common rejection ACK"),Ack.Code,Expected);
 TestTrue(TEXT("No complete or half-pair State adoption"),SameState(Before.State,Access::Session(*F.Mode).GetStateSnapshot()));
 TestEqual(TEXT("No publication on reject"),Access::Revision(*F.Mode),Before.Revision);TestEqual(TEXT("No Coordinator on reject"),F.Calls(),Before.Coordinator);
 TestEqual(TEXT("No fallback entropy"),F.Entropy->Calls,Before.Entropy+FailedAttempts);TestEqual(TEXT("Exact failed provider attempts"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+FailedAttempts);
 TestEqual(TEXT("Other route RNG unchanged"),Access::Runtime(*F.Mode).GetInitialRouteProviderInvocationCount(),Before.Initial);
 if(FailedAttempts||Case==TEXT("HugeId"))
 {
  F.Entropy->FailOnCall=0;const FFrozen Retry(F);TestEqual(TEXT("Explicit legal retry accepts"),F.Roll(K,6,6).Code,Code::Accepted);
  TestEqual(TEXT("Retry uses complete fresh roll contract"),F.Entropy->Calls,Retry.Entropy+(IsPaired(K)?2:1));
  TestEqual(TEXT("Retry coordinates once"),F.Calls(),Retry.Coordinator+1);TestEqual(TEXT("Retry publishes once"),Access::Revision(*F.Mode),Retry.Revision+1);
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSpecializedShotClosedWire,"FMCodex.NetworkPlay.SpecializedShotTransport.ClosedWire",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexSpecializedShotClosedWire::RunTest(const FString&)
{
 using namespace FMCodexSpecializedShotTests;
 for(auto K:Commands())for(int32 Mask=0;Mask<512;++Mask)
 {
  Envelope E;E.IntentKind=K;
  if(Mask&1)E.Deployment.CardId=TEXT("Card");if(Mask&2)E.Goalkeeper.SlotId=TEXT("Slot");if(Mask&4)E.Carrier.CarrierCardId=TEXT("Card");
  if(Mask&8)E.Marker.MarkerCardId=TEXT("Card");if(Mask&16)E.Runner.RunnerCardId=TEXT("Card");if(Mask&32)E.Helper.HelperCardId=TEXT("Card");
  if(Mask&64)E.Skill.SkillId=TEXT("Skill");if(Mask&128)E.Branch.Intent=Branch::DirectShot;
  if(Mask&256)E.OneOnOneChoice=EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
  TestEqual(TEXT("All nine payload members must be empty"),E.ValidatePayloadShape(),Mask==0?Code::None:Code::InvalidPayload);
 }
 TestEqual(TEXT("Existing wire value preserved"),int32(Kind::SubmitThroughBallOneOnOneShotChoice),29);
 TestEqual(TEXT("New typed kind starts at 30"),int32(Kind::LongShotDirectAttackRoll),30);
 TestEqual(TEXT("Final typed kind is 35"),int32(Kind::CutInsideShotDeadCornerRoll),35);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSpecializedShotExclusions,"FMCodex.NetworkPlay.SpecializedShotTransport.WrongFamiliesBranchesAndDisclosure",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexSpecializedShotExclusions::RunTest(const FString&)
{
 using namespace FMCodexSpecializedShotTests;
 auto RejectAll=[&](FShotFixture& F,const TArray<Kind>& Allowed)
 {for(auto K:Commands())if(!Allowed.Contains(K)){const FFrozen Before(F);TestEqual(TEXT("Wrong family/branch/order rejects"),F.Roll(K).Code,Code::AuthorityRejected);Before.Verify(*this,F);}};
 for(bool B:{false,true})
 {
  for(auto Family:{ESkillRuleType::Cross,ESkillRuleType::PassControl,ESkillRuleType::ThroughBall})
  {
   FShotFixture F(B);F.Entropy->Word=5;
   if(!TestTrue(TEXT("Canonical wrong-family route state"),F.ReachRoute(Family,1)&&F.Send(F.Attacker(),Family==ESkillRuleType::Cross?Kind::CrossInitialRouteRoll:Family==ESkillRuleType::PassControl?Kind::PassControlInitialRouteRoll:Kind::ThroughBallInitialRouteRoll)))return false;
   RejectAll(F,{});
  }
  for(bool Cut:{false,true})for(bool Dead:{false,true})
  {
   FShotFixture F(B);F.Entropy->Word=3;if(!F.ReachBranch(Cut?ESkillRuleType::CutInsideShot:ESkillRuleType::LongShot))return false;RejectAll(F,{});
   Payload Choice;Choice.Intent=Dead?Branch::DeadCorner:Branch::DirectShot;if(!F.Send(F.Attacker(),Kind::SubmitBranchIntent,{},{},{},{},{},{},Choice))return false;
   RejectAll(F,{Dead?PairKind(Cut):AttackKind(Cut)});
   if(F.Roll(Dead?PairKind(Cut):AttackKind(Cut),6,6).Code!=Code::Accepted)return false;
   if(!Dead)
   {
    RejectAll(F,{DefenseKind(Cut)});const auto Hidden=F.Safe(0,true);
    TestEqual(TEXT("Hidden conditional Attack cannot reveal Defense CTA"),Hidden.InteractionCategory,EFMCodexLocalMatchInteractionCategory::None);
    if(F.Roll(DefenseKind(Cut),1).Code!=Code::Accepted)return false;
   }
   RejectAll(F,{});
   for(int32 Count=0;Count<2;++Count)
   {
    const auto Hidden=F.Safe(Count,true);TestTrue(TEXT("Hidden roll conceals dependent terminal history"),Hidden.GoalHistory.IsEmpty());
    TestFalse(TEXT("Hidden roll cannot permit Advance"),Hidden.bTerminalPendingAdvance);
    TestTrue(TEXT("Hidden roll cannot reveal Formula result"),Hidden.ResolutionFacts.FormulaContests.IsEmpty());
   }
   const auto HiddenRoute=F.Safe(2,true,false);TestFalse(TEXT("Hidden route conceals dependent terminal"),HiddenRoute.bTerminalPendingAdvance);
  }
 }
 return true;
}
#endif
