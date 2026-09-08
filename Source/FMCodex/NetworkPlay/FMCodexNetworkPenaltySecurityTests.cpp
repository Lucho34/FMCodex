#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPenaltyTestFixture.h"
namespace FMCodexPenaltyAutomation
{
using namespace FMCodexPenaltyTests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPenaltySecurity,"FMCodex.NetworkPlay.Penalty.Security",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexPenaltySecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 K=51;K<=53;++K)
 {const auto P=FString::Printf(TEXT("%s.%d"),S,K);N.Add(P);C.Add(P);}
}
bool FFMCodexPenaltySecurity::RunTest(const FString& P)
{
 const Kind K=static_cast<Kind>(FCString::Atoi(*P.Mid(2)));FFixture F(P.StartsWith(TEXT("B")));
 auto Reject=[&](AFMCodexNetworkMatchPlayerController* PC,const Envelope& E,Code Expected)
 {const FBoundary B(F);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);TestEqual(TEXT("Exact rejection"),Ack.Code,Expected);TestEqual(TEXT("Unchanged ACK revision"),Ack.ViewRevision,B.Revision);B.VerifyFailure(*this,F);};
 Reject(F.Attacker(),Request(F,F.Attacker(),K),Code::InvalidPhase);
 if(!TestTrue(TEXT("Canonical method"),PreparePenalty(F,K==Panenka)))return false;
 Reject(F.Attacker(),Request(F,F.Attacker(),K==Panenka?Attack:Panenka),Code::InvalidPhase);
 if(K==Defense&& !TestEqual(TEXT("Attack prefix"),Roll(F,Attack,6),Code::Accepted))return false;
 auto* PC=K==Defense?F.Defender():F.Attacker();auto* Other=PC==F.A?F.B:F.A;
 Reject(Other,Request(F,Other,K),Code::WrongSide);
 auto E=Request(F,PC,K);auto Bad=E;Bad.MatchInstanceId=FGuid::NewGuid();Reject(PC,Bad,Code::MatchMismatch);
 Bad=E;Bad.ExpectedAttackSequence+=1;Reject(PC,Bad,Code::StaleAttackSequence);
 Bad=E;Bad.PenaltyMethod=EMatchPlayPenaltyMethod::Panenka;Reject(PC,Bad,Code::InvalidPayload);
 Reject(F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(),E,Code::NotParticipant);
 E=Request(F,PC,K); // Earlier semantic rejections consume their request IDs.
 const FBoundary B(F);const auto OldA=F.A->GetOwnerView(),OldB=F.B->GetOwnerView();F.Entropy->FailOnCall=F.Entropy->Calls+1;
 TestEqual(TEXT("Provider failure rejects atomically"),Send(F,PC,E),Code::AuthorityRejected);B.VerifyFailure(*this,F,1);
 TestTrue(TEXT("A disclosure unchanged"),FFMCodexNetworkClientViewSnapshot::StaticStruct()->CompareScriptStruct(&OldA,&F.A->GetOwnerView(),0));
 TestTrue(TEXT("B disclosure unchanged"),FFMCodexNetworkClientViewSnapshot::StaticStruct()->CompareScriptStruct(&OldB,&F.B->GetOwnerView(),0));
 F.Entropy->FailOnCall=0;Reject(PC,E,Code::DuplicateOrAlreadyResolved);
 FFMCodexNetworkIntentClientState Client;const auto Retry=Request(F,PC,K);
 if(!TestTrue(TEXT("Fresh typed retry uses common pending"),BeginFresh(F,Client,PC,Retry,E)))return false;
 Envelope Parallel;TestFalse(TEXT("One shared pending"),Client.BeginSetPiece(PC->GetOwnerView(),K,Parallel));
 F.Entropy->Word=K==Defense?0:5;
 const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);TestEqual(TEXT("Retry accepted"),Ack.Code,Code::Accepted);
 if(P.StartsWith(TEXT("A"))) {Client.ObserveAck(Ack);TestTrue(TEXT("ACK alone waits"),Client.IsPending());Client.ObserveView(PC->GetOwnerView());}
 else {Client.ObserveView(PC->GetOwnerView());TestTrue(TEXT("View alone waits"),Client.IsPending());Client.ObserveAck(Ack);}
 TestFalse(TEXT("Matching ACK and View clear"),Client.IsPending());
 Client.ObserveAck(Ack);Client.ObserveView(PC->GetOwnerView());TestFalse(TEXT("Duplicate observations cannot recreate pending"),Client.IsPending());
 Reject(PC,E,Code::DuplicateOrAlreadyResolved);Reject(PC,Request(F,PC,K),Code::InvalidPhase);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexPenaltyWire,"FMCodex.NetworkPlay.Penalty.ClosedWire",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexPenaltyWire::RunTest(const FString&)
{
 TestEqual(TEXT("Previous tag stable"),int32(Kind::ResolveLongFreeKickPowerRoll),50);
 for(auto K:{Attack,Defense,Panenka})
 {
  Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.RequestId=31;E.ExpectedAttackSequence=2;E.IntentKind=K;
  TestEqual(TEXT("Empty shape accepted"),E.ValidatePayloadShape(),Code::None);
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Out(Writer,false);Envelope::StaticStruct()->SerializeItem(Out,&E,nullptr);
  Envelope Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive In(Reader,false);Envelope::StaticStruct()->SerializeItem(In,&Copy,nullptr);
  TestTrue(TEXT("Reflected wire roundtrip"),Envelope::StaticStruct()->CompareScriptStruct(&E,&Copy,0));
 }
 for(auto K:{EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectAttackRoll,EMatchPlayAuthoritativeCommandKind::ResolvePenaltyDirectDefenseRoll,EMatchPlayAuthoritativeCommandKind::ResolvePenaltyPanenkaRoll})
 {
  TestEqual(TEXT("Authority classifies PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(K),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
  FFixture F;const FBoundary B(F);const auto R=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,FMatchPlaySetPieceTypeRollRequest{}));
  TestEqual(TEXT("Exact typed request variant required"),R.ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);B.VerifyFailure(*this,F);
 }
 return true;
}
}
#endif
