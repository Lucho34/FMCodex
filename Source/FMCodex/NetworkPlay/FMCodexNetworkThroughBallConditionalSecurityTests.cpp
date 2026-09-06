#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkThroughBallConditionalTestFixture.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexThroughBallConditionalSecurity,"FMCodex.NetworkPlay.ThroughBallConditionalTransport.Security",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexThroughBallConditionalSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(auto K:FMCodexThroughBallConditionalTests::Commands())
 for(const TCHAR* Test:{TEXT("Side"),TEXT("Phase"),TEXT("Match"),TEXT("Sequence"),TEXT("HugeId"),TEXT("Internal"),TEXT("Nonparticipant"),TEXT("Payload"),TEXT("Choice"),TEXT("Provider"),TEXT("AckFirst"),TEXT("ViewFirst")})
 {if(K==EFMCodexNetworkPlayerIntentKind::SubmitThroughBallOneOnOneShotChoice&&FString(Test)==TEXT("Provider"))continue;
 auto P=FString::Printf(TEXT("%s.%d.%s"),S,int32(K),Test);N.Add(P);C.Add(P);}
}
bool FFMCodexThroughBallConditionalSecurity::RunTest(const FString& P)
{
 using namespace FMCodexThroughBallConditionalTests;TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));
 auto K=static_cast<Kind>(FCString::Atoi(*Parts[1]));const auto Case=Parts[2];FConditionalFixture F(Parts[0]==TEXT("B"));
 if(Case!=TEXT("Phase")&&!TestTrue(TEXT("Exact legal canonical checkpoint"),F.Ready(K)))return false;
 auto* PC=IsDefense(K)?F.Defender():F.Attacker();const auto Choice=K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None;
 auto E=F.For(K,Choice,PC);auto Expected=Code::AuthorityRejected;
 if(Case==TEXT("AckFirst")||Case==TEXT("ViewFirst"))
 {
  auto& Client=F.Client(PC);Client.ObserveView(PC->GetOwnerView());
  bool Began=K==Kind::SubmitThroughBallOneOnOneShotChoice?Client.BeginOneOnOne(PC->GetOwnerView(),Choice,E):Client.BeginOrdinaryContest(PC->GetOwnerView(),K,E);
  if(!TestTrue(TEXT("Common pending begins"),Began))return false;
  auto Old=PC->GetOwnerView();auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);auto New=PC->GetOwnerView();TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);
  if(Case==TEXT("AckFirst")){Client.ObserveAck(Ack);TestTrue(TEXT("ACK waits for newer view"),Client.IsPending());Client.ObserveView(Old);TestTrue(TEXT("Old view cannot release"),Client.IsPending());Client.ObserveView(New);}
  else{Client.ObserveView(New);TestTrue(TEXT("View waits for ACK"),Client.IsPending());Client.ObserveAck(Ack);}
  TestFalse(TEXT("Both release"),Client.IsPending());Client.ObserveAck(Ack);Client.ObserveView(Old);TestFalse(TEXT("Duplicates do not reopen pending"),Client.IsPending());return true;
 }
 if(Case==TEXT("Side")){PC=IsDefense(K)?F.Attacker():F.Defender();E.RequestId=F.Next(PC)++;}
 if(Case==TEXT("Match")){E.MatchInstanceId=FGuid::NewGuid();E.RequestId=MAX_int64;Expected=Code::MatchMismatch;}
 if(Case==TEXT("Sequence")){++E.ExpectedAttackSequence;Expected=Code::StaleAttackSequence;}
 if(Case==TEXT("HugeId")){E.RequestId=MAX_int64;Expected=Code::InvalidPayload;}
 if(Case==TEXT("Internal")){E.IntentKind=static_cast<Kind>(255);Expected=Code::NotPlayerIntent;}
 if(Case==TEXT("Nonparticipant")){PC=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Expected=Code::NotParticipant;}
 if(Case==TEXT("Payload")){E.Skill.SkillId=TEXT("CannotInjectRules");Expected=Code::InvalidPayload;}
 if(Case==TEXT("Choice")){E.OneOnOneChoice=static_cast<Shot>(255);Expected=Code::InvalidPayload;}
 F.Entropy->bFail=Case==TEXT("Provider");const FFrozen Before(F);auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
 TestEqual(TEXT("Exact common ACK"),Ack.Code,Expected);Before.Verify(*this,F,F.Entropy->bFail);
 AddInfo(FString::Printf(TEXT("REJECT %s ACK=%d State=0 RNG=%d Coordinator=0 Revision=0"),*P,int32(Ack.Code),int32(F.Entropy->bFail)));
 if(Case==TEXT("Provider")||Case==TEXT("HugeId"))
 {
  F.Entropy->bFail=false;const FFrozen RetryBefore(F);
  TestEqual(TEXT("Explicit next legal request after rejection accepts"),F.Roll(K,6,Choice).Code,Code::Accepted);
  const int32 Draws=K==Kind::SubmitThroughBallOneOnOneShotChoice?0:1;
  TestEqual(TEXT("Retry consumes only the requested die"),F.Entropy->Calls,RetryBefore.Entropy+Draws);
  TestEqual(TEXT("Retry uses the same post-route provider"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),RetryBefore.Post+Draws);
  TestEqual(TEXT("Retry coordinates once"),F.Calls(),RetryBefore.Coordinator+1);
  TestEqual(TEXT("Retry publishes once"),Access::Revision(*F.Mode),RetryBefore.Revision+1);
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallConditionalWire,"FMCodex.NetworkPlay.ThroughBallConditionalTransport.ClosedWire",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallConditionalWire::RunTest(const FString&)
{
 using namespace FMCodexThroughBallConditionalTests;
 for(auto K:Commands())for(int32 Mask=0;Mask<512;++Mask)
 {
  Envelope E;E.IntentKind=K;
  if(Mask&1)E.Deployment.CardId=TEXT("Card");if(Mask&2)E.Goalkeeper.SlotId=TEXT("Slot");
  if(Mask&4)E.Carrier.CarrierCardId=TEXT("Card");if(Mask&8)E.Marker.MarkerCardId=TEXT("Card");
  if(Mask&16)E.Runner.RunnerCardId=TEXT("Card");if(Mask&32)E.Helper.HelperCardId=TEXT("Card");
  if(Mask&64)E.Skill.SkillId=TEXT("Skill");if(Mask&128)E.Branch.Intent=Branch::CrossHigh;
  if(Mask&256)E.OneOnOneChoice=Shot::DirectShot;
  const bool Valid=K==Kind::SubmitThroughBallOneOnOneShotChoice?Mask==256:Mask==0;
  TestEqual(TEXT("All nine-member mixed masks closed"),E.ValidatePayloadShape(),Valid?Code::None:Code::InvalidPayload);
 }
 for(int32 K=1;K<=22;++K){Envelope E;E.IntentKind=static_cast<Kind>(K);E.OneOnOneChoice=Shot::ChipShot;TestEqual(TEXT("New payload forbidden on old kinds"),E.ValidatePayloadShape(),Code::InvalidPayload);}
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallConditionalExclusions,"FMCodex.NetworkPlay.ThroughBallConditionalTransport.WrongRoutesBranchesAndStale",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallConditionalExclusions::RunTest(const FString&)
{
 using namespace FMCodexThroughBallConditionalTests;
 for(bool B:{false,true})
 {
  for(auto Family:{ESkillRuleType::Cross,ESkillRuleType::PassControl,ESkillRuleType::ThroughBall})
  {
   FConditionalFixture F(B);F.Entropy->Word=5;
   if(!TestTrue(TEXT("Genuine wrong family / Feet checkpoint"),F.ReachRoute(Family,1)&&F.Send(F.Attacker(),Family==ESkillRuleType::Cross?Kind::CrossInitialRouteRoll:Family==ESkillRuleType::PassControl?Kind::PassControlInitialRouteRoll:Kind::ThroughBallInitialRouteRoll)))return false;
   for(auto K:Commands()){const FFrozen Before(F);TestEqual(TEXT("Wrong family/route rejects"),F.Roll(K,6,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
  }
  for(auto Family:{ESkillRuleType::LongShot,ESkillRuleType::CutInsideShot,ESkillRuleType::ThroughBall})
  {
   FConditionalFixture F(B);F.Entropy->Word=Family==ESkillRuleType::ThroughBall?5:3;
   if(!TestTrue(FString::Printf(TEXT("Genuine family %d pre-route checkpoint, actor B=%d"),int32(Family),int32(B)),F.ReachBranch(Family)))return false;
   for(auto K:Commands()){const FFrozen Before(F);TestEqual(TEXT("Wrong family or pre-route command rejects"),F.Roll(K,6,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
  }
  for(bool Anti:{false,true})
  {
   FConditionalFixture F(B);if(!F.Route(Anti?5:3)||F.Roll(Anti?Kind::ThroughBallAntiOffsideAttackRoll:Kind::ThroughBallBehindDefenseP1AttackRoll,1).Code!=Code::Accepted)return false;
   TestTrue(TEXT("Canonical direct terminal reached"),F.Attacker()->GetOwnerView().bCanAdvance);
   for(auto K:Commands()){const FFrozen Before(F);TestEqual(TEXT("No conditional or choice command after OutOfPlay/Offside"),F.Roll(K,6,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
  }
  for(bool Anti:{false,true})
  {
   FConditionalFixture F(B);if(!F.Route(Anti?5:3))return false;
   for(auto K:Commands())if(K!=(Anti?Kind::ThroughBallAntiOffsideAttackRoll:Kind::ThroughBallBehindDefenseP1AttackRoll))
   {const FFrozen Before(F);TestEqual(TEXT("Wrong route/order before first conditional roll"),F.Roll(K,6,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
  }

  {
   FConditionalFixture F(B);if(!F.Entry())return false;
   const auto Prior=Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.ResolutionSession.PostRouteRollProgress;
   for(auto K:{Kind::ThroughBallOneOnOneDirectShotAttackRoll,Kind::ThroughBallOneOnOneDirectShotDefenseRoll,Kind::ThroughBallOneOnOneChipShotAttackRoll})
   {const FFrozen Before(F);TestEqual(TEXT("OneOnOne is offered but no branch was chosen"),F.Roll(K).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
   TestEqual(TEXT("Canonical choice still accepts after rejected premature rolls"),F.Roll(Kind::SubmitThroughBallOneOnOneShotChoice,6,Shot::DirectShot).Code,Code::Accepted);
   const auto After=Access::Session(*F.Mode).GetStateSnapshot().CurrentAttack.ResolutionSession.PostRouteRollProgress;
   TestTrue(TEXT("Choice preserves every prior roll and its phase byte-for-byte"),FMatchPlayCurrentAttackPostRouteRollProgress::StaticStruct()->CompareScriptStruct(&Prior,&After,0));
  }
  for(bool Chip:{false,true})
  {
   FConditionalFixture F(B);if(!F.Entry()||F.Roll(Kind::SubmitThroughBallOneOnOneShotChoice,6,Chip?Shot::ChipShot:Shot::DirectShot).Code!=Code::Accepted)return false;
   for(auto K:Commands())if(K!=(Chip?Kind::ThroughBallOneOnOneChipShotAttackRoll:Kind::ThroughBallOneOnOneDirectShotAttackRoll))
   {const FFrozen Before(F);TestEqual(TEXT("Wrong OneOnOne branch/order or prior phase"),F.Roll(K,6,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None).Code,Code::AuthorityRejected);Before.Verify(*this,F);}
   TArray<Envelope> Held;for(auto K:Commands())Held.Add(F.For(K,K==Kind::SubmitThroughBallOneOnOneShotChoice?Shot::DirectShot:Shot::None));
   if(F.Roll(Chip?Kind::ThroughBallOneOnOneChipShotAttackRoll:Kind::ThroughBallOneOnOneDirectShotAttackRoll).Code!=Code::Accepted)return false;
   if(!Chip&&F.Roll(Kind::ThroughBallOneOnOneDirectShotDefenseRoll,1).Code!=Code::Accepted)return false;
   if(F.Roll(Kind::AdvanceAfterTerminal).Code!=Code::Accepted)return false;
   F.Entropy->Word=5;auto* NextActor=F.Attacker();F.RequestedKind=Kind::RequestInitialActionPointRoll;
   if(!TestEqual(TEXT("Real next Full D12 after shared Advance"),F.Mode->SubmitConnectionPlayerIntent(NextActor,F.Request(NextActor)).Code,Code::Accepted))return false;
   for(auto E:Held){auto* PC=IsDefense(E.IntentKind)?F.Defender():F.Attacker();E.RequestId=F.Next(PC)++;const FFrozen Before(F);TestEqual(TEXT("Held prior attack commands reject in real N+1"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::StaleAttackSequence);Before.Verify(*this,F);}
  }
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexThroughBallConditionalPrivacy,"FMCodex.NetworkPlay.ThroughBallConditionalTransport.Disclosure",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexThroughBallConditionalPrivacy::RunTest(const FString&)
{
 using namespace FMCodexThroughBallConditionalTests;
 for(bool Anti:{false,true})
 {
  FConditionalFixture F;if(!F.Entry(Anti))return false;auto Hidden=F.Safe(0,false);
  TestTrue(TEXT("Hidden accepted conditional rolls remove decisions"),Hidden.ResolutionFacts.Decisions.IsEmpty());
  TestTrue(TEXT("Hidden conditional outcome cannot leak OneOnOne capability"),Hidden.OneOnOneOptions.IsEmpty());
  TestEqual(TEXT("Hidden result cannot expose next actor action"),Hidden.InteractionCategory,EFMCodexLocalMatchInteractionCategory::None);
  auto Open=F.Safe(Anti?1:2,false);TestEqual(TEXT("Full accepted prefix permits real OneOnOne choice"),Open.OneOnOneOptions.Num(),2);
  auto HiddenRoute=F.Safe(4,false,true,false);
  TestTrue(TEXT("An undisclosed accepted route cannot leak conditional choice"),HiddenRoute.OneOnOneOptions.IsEmpty());
  TestTrue(TEXT("Undisclosed accepted route removes derived Formula"),HiddenRoute.ResolutionFacts.FormulaContests.IsEmpty());
  TestEqual(TEXT("Undisclosed accepted route removes dependent action"),HiddenRoute.InteractionCategory,EFMCodexLocalMatchInteractionCategory::None);
 }
 return true;
}
#endif
