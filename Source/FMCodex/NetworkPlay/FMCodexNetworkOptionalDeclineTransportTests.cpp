#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkOptionalDeclineTestFixture.h"
#include "FMCodexNetworkThroughBallConditionalTestFixture.h"
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOptionalDeclineTransport,"FMCodex.NetworkPlay.OptionalDeclineTransport.Matrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOptionalDeclineTransport::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* K:{TEXT("Runner"),TEXT("Helper"),TEXT("Skill")})
 for(const TCHAR* Case:{TEXT("ValidReplay"),TEXT("WrongSide"),TEXT("WrongPhase"),TEXT("AlreadySelected"),TEXT("WrongMatch"),TEXT("Window"),TEXT("StaleNextAttack"),TEXT("Nonparticipant"),TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("RejectPending"),TEXT("Payload")})
 {const auto P=FString::Printf(TEXT("%s.%s.%s"),S,K,Case);N.Add(P);C.Add(P);}
}
bool FFMCodexOptionalDeclineTransport::RunTest(const FString& P)
{
 using namespace FMCodexOptionalDeclineTests;
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const Kind K=Command(Parts[1]);const FString Case=Parts[2];
 FFixture F((Parts[0]==TEXT("B"))!=(K==Kind::DeclineHelper));if(!TestTrue(TEXT("Canonical setup offers voluntary decline"),Prepare(F,K)))return false;
 auto* PC=Actor(F,K);auto* Other=PC==F.A?F.B:F.A;const auto Offered=PC->GetOwnerView();
 TestEqual(TEXT("Owner-safe action exact"),Offered.DeclineAction,Action(K));TestEqual(TEXT("Waiting view filters decline"),Other->GetOwnerView().DeclineAction,Decline::None);
 TestTrue(TEXT("At least one canonical legal option"),K==Kind::DeclineRunner?!Offered.RunnerOptions.IsEmpty():K==Kind::DeclineHelper?!Offered.HelperOptions.IsEmpty():!Offered.SkillOptions.IsEmpty());
 auto Send=[&](AFMCodexNetworkMatchPlayerController* Who,const Envelope& E){return F.Mode->SubmitConnectionPlayerIntent(Who,E);};
 auto Reject=[&](AFMCodexNetworkMatchPlayerController* Who,const Envelope& E,Code Expected){FUnchanged B(F);auto A=Send(Who,E);TestEqual(TEXT("Exact rejection ACK"),A.Code,Expected);TestEqual(TEXT("Rejection receipt holds revision"),A.ViewRevision,B.Revision);B.Verify(*this,F);};
 Envelope E=Request(F,PC,K);
 if(Case==TEXT("Payload"))
 {
  auto Bad=E;Bad.Skill.SkillId=TEXT("Injected");Reject(PC,Bad,Code::InvalidPayload);
  Bad=E;Bad.Runner.RunnerCardId=TEXT("Injected");Reject(PC,Bad,Code::InvalidPayload);
  Bad=E;Bad.IntentKind=static_cast<Kind>(255);Reject(PC,Bad,Code::NotPlayerIntent);
 }

 if(Case==TEXT("WrongSide")){E=Request(F,Other,K);Reject(Other,E,Code::WrongSide);return true;}
 if(Case==TEXT("WrongPhase")){E.IntentKind=K==Kind::DeclineRunner?Kind::DeclineHelper:Kind::DeclineRunner;Reject(PC,E,Code::InvalidPhase);return true;}
 if(Case==TEXT("Nonparticipant")){Reject(F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(),E,Code::NotParticipant);return true;}
 if(Case==TEXT("WrongMatch")){E.MatchInstanceId=FGuid::NewGuid();Reject(PC,E,Code::MatchMismatch);E.MatchInstanceId=Offered.MatchInstanceId;}
 if(Case==TEXT("Window"))
 {
  auto Huge=E;Huge.RequestId+=1024;Reject(PC,Huge,Code::InvalidPayload);
  auto Zero=E;Zero.RequestId=0;Reject(PC,Zero,Code::InvalidPayload);
 }
 if(Case==TEXT("AlreadySelected"))
 {
  const bool OK=K==Kind::DeclineRunner?F.Send(PC,Kind::SubmitRunner,{},{},{},Offered.RunnerOptions[0].Choice)
   :K==Kind::DeclineHelper?F.Send(PC,Kind::SubmitHelper,{},{},{},{},Offered.HelperOptions[0].Choice)
   :F.Send(PC,Kind::SubmitSkill,{},{},{},{},{},Offered.SkillOptions[0].Choice);
  if(!TestTrue(TEXT("Positive choice remains available"),OK))return false;
  E=Request(F,PC,K);Reject(PC,E,Code::InvalidPhase);return true;
 }
 if(Case==TEXT("StaleNextAttack"))
 {
  if(K==Kind::DeclineRunner&&!TestTrue(TEXT("Progress Runner"),F.Send(F.Attacker(),Kind::SubmitRunner,{},{},{},F.Attacker()->GetOwnerView().RunnerOptions[0].Choice)))return false;
  if(K!=Kind::DeclineSkill&&!TestTrue(TEXT("Progress Helper"),F.Send(F.Defender(),Kind::SubmitHelper,{},{},{},{},F.Defender()->GetOwnerView().HelperOptions[0].Choice)))return false;
  auto End=Request(F,F.Attacker(),Kind::DeclineSkill);TestEqual(TEXT("Close N canonically"),Send(F.Attacker(),End).Code,Code::Accepted);
  if(!TestTrue(TEXT("Reach corresponding wait N+1 canonically"),Prepare(F,K)))return false;
  auto* Next=Actor(F,K);auto Stale=Request(F,Next,K);Stale.ExpectedAttackSequence=Offered.AttackSequence;Reject(Next,Stale,Code::StaleAttackSequence);return true;
 }
 if(Case==TEXT("AckFirst")||Case==TEXT("ViewFirst")||Case==TEXT("RejectPending"))
 {
  FFMCodexNetworkIntentClientState Client;
  if(!TestTrue(TEXT("Generic client begins exact decline"),Client.BeginDecline(Offered,K,E)))return false;
  Envelope Duplicate;TestFalse(TEXT("Single pending blocks duplicate"),Client.BeginDecline(Offered,K,Duplicate));
  if(Case==TEXT("RejectPending"))
  {
   const FUnchanged B(F);auto Bad=E;Bad.IntentKind=K==Kind::DeclineRunner?Kind::DeclineHelper:Kind::DeclineRunner;
   const auto Ack=Send(PC,Bad);TestEqual(TEXT("Rejection correlated"),Ack.Code,Code::InvalidPhase);Client.ObserveAck(Ack);TestFalse(TEXT("Rejected pending clears"),Client.IsPending());B.Verify(*this,F);
   TestTrue(TEXT("Latest safe view restores action"),Client.BeginDecline(PC->GetOwnerView(),K,Duplicate));return true;
  }
  const FUnchanged B(F);const auto Ack=Send(PC,E);TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);Adopted(*this,F,K,B);
  if(Case==TEXT("AckFirst")){Client.ObserveAck(Ack);TestTrue(TEXT("ACK waits for new View"),Client.IsPending());Client.ObserveView(PC->GetOwnerView());}
  else{Client.ObserveView(PC->GetOwnerView());TestTrue(TEXT("View waits for matching ACK"),Client.IsPending());Client.ObserveAck(Ack);}
  TestFalse(TEXT("Generic pending completes on both"),Client.IsPending());return true;
 }
 const FUnchanged Before(F);const auto Ack=Send(PC,E);TestEqual(TEXT("Legal decline accepted"),Ack.Code,Code::Accepted);Adopted(*this,F,K,Before);
 Reject(PC,E,Code::DuplicateOrAlreadyResolved);
 E=Request(F,PC,K);Reject(PC,E,Code::InvalidPhase);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOptionalDeclineWire,"FMCodex.NetworkPlay.OptionalDeclineTransport.ClosedWire",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexOptionalDeclineWire::RunTest(const FString&)
{
 using namespace FMCodexOptionalDeclineTests;
 for(auto K:{Kind::DeclineRunner,Kind::DeclineHelper,Kind::DeclineSkill})for(int32 Mask=0;Mask<512;++Mask)
 {
  Envelope E;E.IntentKind=K;if(Mask&1)E.Deployment.CardId=TEXT("Card");if(Mask&2)E.Goalkeeper.SlotId=TEXT("Slot");if(Mask&4)E.Carrier.CarrierCardId=TEXT("Card");
  if(Mask&8)E.Marker.MarkerCardId=TEXT("Card");if(Mask&16)E.Runner.RunnerCardId=TEXT("Card");if(Mask&32)E.Helper.HelperCardId=TEXT("Card");if(Mask&64)E.Skill.SkillId=TEXT("Skill");
  if(Mask&128)E.Branch.Intent=EMatchPlayElectiveBranchIntent::DirectShot;if(Mask&256)E.OneOnOneChoice=EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
  TestEqual(TEXT("Closed empty union rejects every nonempty member combination"),E.ValidatePayloadShape(),Mask?Code::InvalidPayload:Code::None);
 }
 for(int32 Value=39;Value<256;++Value){Envelope E;E.IntentKind=static_cast<Kind>(Value);TestEqual(TEXT("Unallowlisted/internal wire tags fail closed"),E.ValidatePayloadShape(),Code::NotPlayerIntent);}
 for(auto K:{EMatchPlayAuthoritativeCommandKind::ResolveNoLegalRunner,EMatchPlayAuthoritativeCommandKind::ResolveNoLegalHelper,EMatchPlayAuthoritativeCommandKind::ResolveNoLegalSkill})
  TestEqual(TEXT("No legal remains server internal"),FMatchPlayAuthoritativeCommandClassification::OriginOf(K),EMatchPlayAuthoritativeCommandOrigin::ServerInternalAction);
 TestEqual(TEXT("Append-only tag Runner"),int32(Kind::DeclineRunner),36);TestEqual(TEXT("Append-only tag Helper"),int32(Kind::DeclineHelper),37);TestEqual(TEXT("Append-only tag Skill"),int32(Kind::DeclineSkill),38);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexDeclineOtherWaits,"FMCodex.NetworkPlay.OptionalDeclineTransport.OtherWaitsAndTypedBoundary",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexDeclineOtherWaits::RunTest(const FString&)
{
 using namespace FMCodexOptionalDeclineTests;
 for(bool OneOnOne:{false,true})
 {
  FMCodexThroughBallConditionalTests::FConditionalFixture F;
  const bool Ready=OneOnOne?F.Entry():Access::Runtime(*F.Mode).PrepareOrdinaryTerminalMilestone(true,false);
  if(!TestTrue(TEXT("Canonical distinct non-selection wait"),Ready))return false;Access::Publish(*F.Mode);
  for(auto K:{Kind::DeclineRunner,Kind::DeclineHelper,Kind::DeclineSkill})
  {
   const FUnchanged Before(F);auto E=Request(F,F.Attacker(),K);
   TestEqual(TEXT("Terminal/OneOnOne cannot accept decline"),F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),E).Code,Code::InvalidPhase);Before.Verify(*this,F);
  }
 }
 for(auto K:{Kind::DeclineRunner,Kind::DeclineHelper,Kind::DeclineSkill})
 {
  FFixture F;if(!TestTrue(TEXT("Voluntary choice setup"),Prepare(F,K)))return false;
  const FUnchanged Before(F);
  const auto C=K==Kind::DeclineRunner?EMatchPlayAuthoritativeCommandKind::DeclineRunner:K==Kind::DeclineHelper?EMatchPlayAuthoritativeCommandKind::DeclineHelper:EMatchPlayAuthoritativeCommandKind::DeclineSkill;
  const auto R=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(C,FMatchPlayAuthoritativeSubmitRunnerRequest{}));
  TestFalse(TEXT("Typed HostPort rejects mismatched variant"),R.bSuccess);TestEqual(TEXT("Exact typed mismatch"),R.ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);Before.Verify(*this,F);
 }
 return true;
}
#endif
