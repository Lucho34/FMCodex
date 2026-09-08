#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSetPieceSelectionTestFixture.h"
#include "../CoreRules/MatchPlaySetPieceTypeRoll.h"
using namespace FMCodexSetPieceSelectionTests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceTypes,"FMCodex.NetworkPlay.SetPieceSelection.TypeMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceTypes::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D12=9;D12<=12;++D12)for(int32 D6=1;D6<=6;++D6){auto P=FString::Printf(TEXT("%s.%d.%d"),S,D12,D6);N.Add(P);C.Add(P);} }
bool FFMCodexSetPieceTypes::RunTest(const FString& P)
{
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const int32 D12=FCString::Atoi(*Parts[1]),D6=FCString::Atoi(*Parts[2]);
 FFixture F(Parts[0]==TEXT("B"));Access::SetPiecePresentation(*F.Mode);
 if(!TestTrue(TEXT("Canonical Full D12 enters type wait"),Entry(F,D12)))return false;
 const FUnchanged B(F);auto E=Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll);
 for(auto* PC:{F.A,F.B}){TestEqual(TEXT("No future type die"),PC->GetOwnerView().SetPiece.TypeD6,0);TestEqual(TEXT("No future type"),PC->GetOwnerView().SetPiece.Type,Type::None);}
 F.Entropy->Word=D6-1;TestEqual(TEXT("Typed type accepted"),Send(F,F.Attacker(),E),Code::Accepted);
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& SP=State.CurrentAttack.SetPieceRoute;
 TestTrue(TEXT("One canonical type result"),SP.bHasTypeRoll&&SP.RawTypeD6==D6&&SP.SelectedType==ExpectedType(D6));
 TestEqual(TEXT("One private entry draw"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),B.EntryCalls+1);
 TestEqual(TEXT("One secure entropy draw"),F.Entropy->Calls,B.EntropyCalls+1);
 TestEqual(TEXT("No new D12"),Access::Runtime(*F.Mode).GetD12ProviderInvocationCount(),B.D12Calls);
 TestEqual(TEXT("One coordinator pass"),F.Calls(),B.CoordinatorCalls+1);TestEqual(TEXT("One publication"),Access::Revision(*F.Mode),B.Revision+1);
 NoScoreChange(*this,B.State,State);
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();TestEqual(TEXT("Safe type public to both"),V.SetPiece.Type,SP.SelectedType);TestEqual(TEXT("Safe accepted type die"),V.SetPiece.TypeD6,D6);
  TestTrue(TEXT("No future contest/formula/terminal"),V.AcceptedContestRolls.IsEmpty()&&V.Terminal.Outcome==EFMCodexNetworkTerminalOutcome::None&&!V.Presentation.InlineFormula.bVisible);
  TestEqual(TEXT("One accepted type event"),V.Presentation.ResolvedRolls.Num(),1);
  if(D6<=2)
  {TestEqual(TEXT("Corner stops at nomination boundary"),V.EntryWait,Wait::CornerSelectionBoundary);TestTrue(TEXT("Corner exposes no taker or methods"),V.SetPiece.TakerOptions.IsEmpty()&&V.SetPiece.NearMethods.IsEmpty()&&V.SetPiece.LongMethods.IsEmpty()&&V.SetPiece.PenaltyMethods.IsEmpty());}
  else {TestEqual(TEXT("Canonical taker wait"),V.EntryWait,Wait::SetPieceTakerSelection);TestEqual(TEXT("Complete canonical 19 takers only for actor"),V.SetPiece.TakerOptions.Num(),PC==F.Attacker()?19:0);}
 }
 if(D6<=2)TestEqual(TEXT("Actual Corner state remains sealed nomination wait"),SP.Corner.Stage,EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations);
 const FUnchanged After(F);TestEqual(TEXT("Same ID cannot reroll"),Send(F,F.Attacker(),E),Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
 E=Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll);TestEqual(TEXT("Fresh ID cannot reroll"),Send(F,F.Attacker(),E),Code::InvalidPhase);After.Verify(*this,F);
 FFMCodexLocalMatchViewerDisclosure Hidden;Hidden.bRevealInitialActionPointRoll=true;
 const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),F.Attacker()->GetOwnerView().ViewerSide,Hidden);
 const auto Redacted=FFMCodexSetPieceSelectionPresentation::Build(Safe,F.Attacker()->GetOwnerView().ViewerSide);
 TestTrue(TEXT("Security disclosure is independent of UI suspense"),Redacted.TypeD6==0&&Redacted.Type==Type::None&&Redacted.TakerOptions.IsEmpty()&&Redacted.TakerCardId.IsNone());
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceMethods,"FMCodex.NetworkPlay.SetPieceSelection.MethodMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceMethods::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D6:{3,5,6})for(int32 M:{1,2}){auto P=FString::Printf(TEXT("%s.%d.%d"),S,D6,M);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceMethods::RunTest(const FString& P)
{
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const int32 D6=FCString::Atoi(*Parts[1]);const bool Alt=Parts[2]==TEXT("2");
 FFixture F(Parts[0]==TEXT("B"));const auto K=MethodKind(D6);if(!TestTrue(TEXT("Canonical type and eligible taker"),Prepare(F,K,D6)))return false;
 const auto& Offered=F.Attacker()->GetOwnerView().SetPiece;
 TestTrue(TEXT("Only corresponding two typed methods"),D6==5?Offered.NearMethods.Num()==2&&Offered.LongMethods.IsEmpty()&&Offered.PenaltyMethods.IsEmpty():D6==6?Offered.PenaltyMethods.Num()==2&&Offered.NearMethods.IsEmpty()&&Offered.LongMethods.IsEmpty():Offered.LongMethods.Num()==2&&Offered.NearMethods.IsEmpty()&&Offered.PenaltyMethods.IsEmpty());
 for(auto OtherKind:{Kind::SubmitShortFreeKickMethod,Kind::SubmitLongFreeKickMethod,Kind::SubmitPenaltyMethod})if(OtherKind!=K)
 {const FUnchanged Unchanged(F);TestEqual(TEXT("Both foreign method families reject after taker"),Send(F,F.Attacker(),Request(F,F.Attacker(),OtherKind)),Code::InvalidPhase);Unchanged.Verify(*this,F);}
 const FUnchanged Before(F);const auto E=Request(F,F.Attacker(),K,Alt);TestEqual(TEXT("Exact method accepted"),Send(F,F.Attacker(),E),Code::Accepted);
 const auto A=Access::Session(*F.Mode).GetStateSnapshot();const auto& SP=A.CurrentAttack.SetPieceRoute;
 const Stage Actual=D6==5?SP.ShortFreeKick.Stage:D6==6?SP.Penalty.Stage:SP.LongFreeKick.Stage;
 const Stage Expected=!Alt?Stage::DirectAwaitingAttackRoll:D6==5?Stage::AngledAwaitingRoll:D6==6?Stage::PanenkaAwaitingRoll:Stage::PowerAwaitingRoll;
 TestEqual(TEXT("Stops at exact first missing decisive PlayerIntent"),Actual,Expected);
 TestEqual(TEXT("Specialized family retains RoutePending, not ordinary Resolution"),A.CurrentAttack.Phase,EMatchPlayCurrentAttackPhase::RoutePending);
 TestEqual(TEXT("Ordinary selection stage remains None"),A.CurrentAttack.SelectionStage,EMatchPlayCurrentAttackSelectionStage::None);
 TestEqual(TEXT("Expected actor stays attacker"),F.Attacker()->GetOwnerView().ExpectedActingSide,F.Attacker()->GetOwnerView().ViewerSide);
 TestEqual(TEXT("No method RNG"),F.Entropy->Calls,Before.EntropyCalls);TestEqual(TEXT("One coordinator pass"),F.Calls(),Before.CoordinatorCalls+1);TestEqual(TEXT("One stable publication"),Access::Revision(*F.Mode),Before.Revision+1);NoScoreChange(*this,Before.State,A);
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();auto M=FFMCodexNetworkMatchPresentationAdapter::Read(V,false);
  TestEqual(TEXT("Exact offered resolution step"),V.EntryWait,D6==5?(Alt?Wait::NearAngledRoll:Wait::NearDirectAttackRoll):D6<=4?(Alt?Wait::LongFreeKickPowerRoll:Wait::LongFreeKickDirectAttackRoll):Wait::SetPieceResolutionBoundary);
  if(D6!=6) { TestTrue(TEXT("Free kick reuses shared Formula surface"),M.InlineFormula.bVisible); TestEqual(TEXT("Free kick decisive action owner only"),M.Interaction.bCanContinue,PC==F.Attacker()); }
  else TestTrue(TEXT("Other decisive families stay gated"),!M.Interaction.PrimaryAction.bAvailable&&!M.Interaction.bCanContinue&&!M.InlineFormula.bVisible);
  TestTrue(TEXT("Selection options consumed"),M.SetPiece.TakerOptions.IsEmpty()&&M.SetPiece.NearMethods.IsEmpty()&&M.SetPiece.LongMethods.IsEmpty()&&M.SetPiece.PenaltyMethods.IsEmpty());
  TestTrue(TEXT("No fake result or score"),V.Terminal.Outcome==EFMCodexNetworkTerminalOutcome::None&&V.AcceptedContestRolls.IsEmpty());
 }
 const FUnchanged After(F);TestEqual(TEXT("Duplicate method"),Send(F,F.Attacker(),E),Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
 TestEqual(TEXT("Fresh same method cannot resubmit"),Send(F,F.Attacker(),Request(F,F.Attacker(),K,Alt)),Code::InvalidPhase);After.Verify(*this,F);
 TestEqual(TEXT("Fresh method cannot rewrite"),Send(F,F.Attacker(),Request(F,F.Attacker(),K,!Alt)),Code::InvalidPhase);After.Verify(*this,F);
 AddInfo(FString::Printf(TEXT("PostMethod D6=%d Method=%d Phase=%d SelectionStage=%d CarrierStage=%d Actor=%d Wait=%d"),D6,Alt?2:1,int32(A.CurrentAttack.Phase),int32(A.CurrentAttack.SelectionStage),int32(Actual),int32(F.Attacker()->GetOwnerView().ExpectedActingSide),int32(F.Attacker()->GetOwnerView().EntryWait)));
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceSecurity,"FMCodex.NetworkPlay.SetPieceSelection.SecurityMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceSecurity::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 K=40;K<=44;++K)for(const TCHAR* Case:{TEXT("WrongSide"),TEXT("WrongPhase"),TEXT("WrongMatch"),TEXT("Stale"),TEXT("Window"),TEXT("Nonparticipant"),TEXT("Payload"),TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("RejectedPending")}){auto P=FString::Printf(TEXT("%s.%d.%s"),S,K,Case);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceSecurity::RunTest(const FString& P)
{
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const Kind K=static_cast<Kind>(FCString::Atoi(*Parts[1]));const FString Case=Parts[2];const int32 D6=K==Kind::SubmitLongFreeKickMethod?3:K==Kind::SubmitPenaltyMethod?6:5;
 FFixture F(Parts[0]==TEXT("B"));if(!TestTrue(TEXT("Canonical requested wait"),Prepare(F,K,D6)))return false;auto* PC=F.Attacker();
 auto E=Request(F,PC,K);auto Reject=[&](AFMCodexNetworkMatchPlayerController* Who,const Envelope& Bad,Code Expected){const FUnchanged B(F);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(Who,Bad);TestEqual(TEXT("Exact rejection"),Ack.Code,Expected);TestEqual(TEXT("Rejection revision unchanged"),Ack.ViewRevision,B.Revision);B.Verify(*this,F);};
 if(Case==TEXT("WrongSide")){Reject(F.Defender(),E,Code::WrongSide);return true;}
 if(Case==TEXT("Nonparticipant")){Reject(F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>(),E,Code::NotParticipant);return true;}
 if(Case==TEXT("WrongMatch")){const auto Normal=E;E.MatchInstanceId=FGuid::NewGuid();Reject(PC,E,Code::MatchMismatch);TestEqual(TEXT("Wrong match does not consume current-match RequestId"),Send(F,PC,Normal),Code::Accepted);return true;}
 if(Case==TEXT("Stale")){E.ExpectedAttackSequence+=1;Reject(PC,E,Code::StaleAttackSequence);return true;}
 if(Case==TEXT("Window")){const auto Normal=E;auto Zero=E;Zero.RequestId=0;Reject(PC,Zero,Code::InvalidPayload);E.RequestId+=1024;Reject(PC,E,Code::InvalidPayload);TestEqual(TEXT("Huge ID leaves next normal request usable"),Send(F,PC,Normal),Code::Accepted);return true;}
 if(Case==TEXT("Payload")){E.Skill.SkillId=TEXT("Injected");Reject(PC,E,Code::InvalidPayload);return true;}
 if(Case==TEXT("WrongPhase")){auto Wrong=Request(F,PC,K==Kind::RequestSetPieceTypeRoll?Kind::SubmitSetPieceCarrier:K==Kind::SubmitSetPieceCarrier?Kind::SubmitShortFreeKickMethod:K==Kind::SubmitPenaltyMethod?Kind::SubmitLongFreeKickMethod:Kind::SubmitPenaltyMethod);Reject(PC,Wrong,Code::InvalidPhase);return true;}
 FFMCodexNetworkIntentClientState C;
 if(!TestTrue(TEXT("Common client begins exact typed family"),BeginFresh(F,C,PC,E,E)))return false;
 Envelope Duplicate;TestFalse(TEXT("One shared pending"),C.BeginSetPiece(PC->GetOwnerView(),K,Duplicate,E.SetPieceCardId,E.NearMethod,E.LongMethod,E.PenaltyMethod));
 if(Case==TEXT("RejectedPending"))
 {auto Bad=E;Bad.Skill.SkillId=TEXT("Injected");const FUnchanged B(F);auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,Bad);TestEqual(TEXT("Payload rejection"),Ack.Code,Code::InvalidPayload);C.ObserveAck(Ack);TestFalse(TEXT("Rejected pending clears"),C.IsPending());B.Verify(*this,F);return true;}
 F.Entropy->Word=4;const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);TestEqual(TEXT("Accepted"),Ack.Code,Code::Accepted);
 if(Case==TEXT("AckFirst")){C.ObserveAck(Ack);TestTrue(TEXT("ACK alone stays pending"),C.IsPending());C.ObserveView(PC->GetOwnerView());}
 else{C.ObserveView(PC->GetOwnerView());TestTrue(TEXT("View alone stays pending"),C.IsPending());C.ObserveAck(Ack);}
 TestFalse(TEXT("ACK and View clears"),C.IsPending());C.ObserveAck(Ack);C.ObserveView(PC->GetOwnerView());TestFalse(TEXT("Repeated observations remain clear"),C.IsPending());return true;
}
#endif
