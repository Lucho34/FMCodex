#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
using namespace FMCodexNearFreeKickTests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNearSecurity,"FMCodex.NetworkPlay.NearFreeKick.Security",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexNearSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 K=45;K<=47;++K)
 for(const TCHAR* Case:{TEXT("WrongSide"),TEXT("Nonparticipant"),TEXT("Match"),TEXT("Sequence"),TEXT("Window"),TEXT("Payload"),TEXT("OtherMethod"),TEXT("OtherFamily"),TEXT("OtherPhase"),TEXT("Duplicate"),TEXT("ProviderFirst"),TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("RejectedPending")})
 {const auto P=FString::Printf(TEXT("%s.%d.%s"),S,K,Case);N.Add(P);C.Add(P);}
 for(const TCHAR* S:{TEXT("A"),TEXT("B")}){const auto P=FString::Printf(TEXT("%s.47.ProviderSecond"),S);N.Add(P);C.Add(P);}
}
bool FFMCodexNearSecurity::RunTest(const FString& P)
{
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const Kind K=static_cast<Kind>(FCString::Atoi(*Parts[1]));const auto Case=Parts[2];
 FFixture F(Parts[0]==TEXT("B"));
 if(Case==TEXT("OtherPhase")) {if(!TestTrue(TEXT("Ordinary deployment"),F.Send(F.Attacker(),Kind::RequestInitialActionPointRoll)))return false;}
 else if(Case==TEXT("OtherFamily")) {if(!TestTrue(TEXT("Actual Long method wait"),Prepare(F,Kind::SubmitLongFreeKickMethod,3)))return false;Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitLongFreeKickMethod));}
 else
 {
  if(!TestTrue(TEXT("Canonical Near method"),PrepareNear(F,Case==TEXT("OtherMethod")?K!=Pair:K==Pair)))return false;
  if(K==Defense&&Case!=TEXT("OtherMethod"))if(!TestEqual(TEXT("Canonical attack before defense"),Roll(F,Attack,3),Code::Accepted))return false;
 }
 auto* PC=K==Defense?F.Defender():F.Attacker();auto E=Request(F,PC,K);
 auto Reject=[&](AFMCodexNetworkMatchPlayerController* Who,const Envelope& Bad,Code CodeExpected)
 {const FBoundary B(F);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(Who,Bad);TestEqual(TEXT("Exact rejection code"),Ack.Code,CodeExpected);TestEqual(TEXT("ACK carries unchanged revision"),Ack.ViewRevision,B.Revision);B.VerifyFailure(*this,F);};
 if(Case==TEXT("WrongSide")){auto* Other=PC==F.A?F.B:F.A;Reject(Other,Request(F,Other,K),Code::WrongSide);return true;}
 if(Case==TEXT("Nonparticipant")){Reject(F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(),E,Code::NotParticipant);return true;}
 if(Case==TEXT("OtherMethod")||Case==TEXT("OtherFamily")||Case==TEXT("OtherPhase")){Reject(PC,E,Code::InvalidPhase);return true;}
 if(Case==TEXT("Match")){auto Bad=E;Bad.MatchInstanceId=FGuid::NewGuid();Reject(PC,Bad,Code::MatchMismatch);TestEqual(TEXT("Normal ID still usable"),Send(F,PC,E),Code::Accepted);return true;}
 if(Case==TEXT("Sequence")){E.ExpectedAttackSequence+=1;Reject(PC,E,Code::StaleAttackSequence);return true;}
 if(Case==TEXT("Window"))
 {
  auto Bad=E;for(auto Id:{int64(0),int64(-1),MAX_int64,E.RequestId+1024}){Bad.RequestId=Id;Reject(PC,Bad,Code::InvalidPayload);}
  TestEqual(TEXT("Huge IDs did not poison shared namespace"),Send(F,PC,E),Code::Accepted);return true;
 }
 if(Case==TEXT("Payload"))
 {
  for(int32 I=0;I<14;++I)
  {
   auto Bad=E;
   switch(I)
   {
   case 0:Bad.Deployment.CardId=TEXT("Injected");break;case 1:Bad.Goalkeeper.SlotId=TEXT("Injected");break;
   case 2:Bad.Carrier.CarrierCardId=TEXT("Injected");break;case 3:Bad.Marker.MarkerCardId=TEXT("Injected");break;
   case 4:Bad.Runner.RunnerCardId=TEXT("Injected");break;case 5:Bad.Helper.HelperCardId=TEXT("Injected");break;
   case 6:Bad.Skill.SkillId=TEXT("Injected");break;case 7:Bad.Branch.Intent=EMatchPlayElectiveBranchIntent::DirectShot;break;
   case 8:Bad.OneOnOneChoice=EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;break;
   case 9:Bad.SetPieceCardId=TEXT("Injected");break;case 10:Bad.NearMethod=EMatchPlayShortFreeKickMethod::Direct;break;
   case 11:Bad.LongMethod=EMatchPlayLongFreeKickMethod::Power;break;case 12:Bad.PenaltyMethod=EMatchPlayPenaltyMethod::Panenka;break;
   case 13:Bad.IntentKind=static_cast<Kind>(255);break;
   }
   Reject(PC,Bad,I==13?Code::NotPlayerIntent:Code::InvalidPayload);
  }
  TestEqual(TEXT("Malformed shapes did not consume ID"),Send(F,PC,E),Code::Accepted);return true;
 }
 if(Case==TEXT("Duplicate"))
 {TestEqual(TEXT("First accepted"),Send(F,PC,E),Code::Accepted);Reject(PC,E,Code::DuplicateOrAlreadyResolved);Reject(PC,Request(F,PC,K),Code::InvalidPhase);return true;}
 if(Case.StartsWith(TEXT("Provider")))
 {
  const int32 Draws=Case==TEXT("ProviderSecond")?2:1;const FBoundary B(F);
  const auto OldA=F.A->GetOwnerView(),OldB=F.B->GetOwnerView();F.Entropy->FailOnCall=F.Entropy->Calls+Draws;
  TestEqual(TEXT("Provider failure rejects"),Send(F,PC,E),Code::AuthorityRejected);B.VerifyFailure(*this,F,Draws);
  TestTrue(TEXT("A disclosure unchanged on failure"),FFMCodexNetworkClientViewSnapshot::StaticStruct()->CompareScriptStruct(&OldA,&F.A->GetOwnerView(),0));
  TestTrue(TEXT("B disclosure unchanged on failure"),FFMCodexNetworkClientViewSnapshot::StaticStruct()->CompareScriptStruct(&OldB,&F.B->GetOwnerView(),0));
  F.Entropy->FailOnCall=0;TestEqual(TEXT("Failed ID remains consumed"),Send(F,PC,E),Code::DuplicateOrAlreadyResolved);
  TestEqual(TEXT("Fresh typed retry works without partial state"),Send(F,PC,Request(F,PC,K)),Code::Accepted);return true;
 }
 FFMCodexNetworkIntentClientState Client;
 if(!TestTrue(TEXT("Common BeginSetPiece delegates shared pending"),BeginFresh(F,Client,PC,E,E)))return false;
 Envelope Other;TestFalse(TEXT("No parallel request"),Client.BeginSetPiece(PC->GetOwnerView(),K,Other));
 if(Case==TEXT("RejectedPending"))
 {const FBoundary B(F);E.NearMethod=EMatchPlayShortFreeKickMethod::Direct;const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);Client.ObserveAck(Ack);TestFalse(TEXT("Rejected pending clears"),Client.IsPending());B.VerifyFailure(*this,F);return true;}
 const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);TestEqual(TEXT("Typed request accepted"),Ack.Code,Code::Accepted);
 if(Case==TEXT("AckFirst")){Client.ObserveAck(Ack);TestTrue(TEXT("ACK needs View revision"),Client.IsPending());Client.ObserveView(PC->GetOwnerView());}
 else{Client.ObserveView(PC->GetOwnerView());TestTrue(TEXT("View needs matching ACK"),Client.IsPending());Client.ObserveAck(Ack);}
 TestFalse(TEXT("Pending clears on both"),Client.IsPending());Client.ObserveAck(Ack);Client.ObserveView(PC->GetOwnerView());TestFalse(TEXT("Duplicates cannot recreate pending"),Client.IsPending());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNearWire,"FMCodex.NetworkPlay.NearFreeKick.ClosedWire",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexNearWire::RunTest(const FString&)
{
 TestEqual(TEXT("Existing last tag stable"),int32(Kind::SubmitPenaltyMethod),44);
 for(auto K:{Attack,Defense,Pair})
 {
  Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.RequestId=31;E.ExpectedAttackSequence=2;E.IntentKind=K;
  TestEqual(TEXT("Empty shape accepted"),E.ValidatePayloadShape(),Code::None);
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Out(Writer,false);Envelope::StaticStruct()->SerializeItem(Out,&E,nullptr);
  Envelope Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive In(Reader,false);Envelope::StaticStruct()->SerializeItem(In,&Copy,nullptr);
  TestTrue(TEXT("Reflected wire roundtrip"),Envelope::StaticStruct()->CompareScriptStruct(&E,&Copy,0));
 }
 for(auto K:{EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectAttackRoll,EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickDirectDefenseRoll,EMatchPlayAuthoritativeCommandKind::ResolveShortFreeKickAngledRoll})
 {
  TestEqual(TEXT("Existing authority classifies PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(K),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
  FFixture F;const FBoundary B(F);auto R=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,FMatchPlaySetPieceTypeRollRequest{}));
  TestEqual(TEXT("Exact HostPort variant required"),R.ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);B.VerifyFailure(*this,F);
 }
 return true;
}
#endif
