#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSetPieceSelectionTestFixture.h"
using namespace FMCodexSetPieceSelectionTests;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSetPieceWire,"FMCodex.NetworkPlay.SetPieceSelection.ClosedWire",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexSetPieceWire::RunTest(const FString&)
{
 for(int32 Tag=40;Tag<=44;++Tag)for(int32 Mask=0;Mask<8192;++Mask)
 {
  Envelope E;E.IntentKind=static_cast<Kind>(Tag);
  if(Mask&1)E.Deployment.CardId=TEXT("Card");if(Mask&2)E.Goalkeeper.SlotId=TEXT("Slot");if(Mask&4)E.Carrier.CarrierCardId=TEXT("Card");if(Mask&8)E.Marker.MarkerCardId=TEXT("Card");
  if(Mask&16)E.Runner.RunnerCardId=TEXT("Card");if(Mask&32)E.Helper.HelperCardId=TEXT("Card");if(Mask&64)E.Skill.SkillId=TEXT("Skill");if(Mask&128)E.Branch.Intent=EMatchPlayElectiveBranchIntent::DirectShot;if(Mask&256)E.OneOnOneChoice=EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
  if(Mask&512)E.SetPieceCardId=TEXT("Taker");if(Mask&1024)E.NearMethod=EMatchPlayShortFreeKickMethod::Direct;if(Mask&2048)E.LongMethod=EMatchPlayLongFreeKickMethod::Power;if(Mask&4096)E.PenaltyMethod=EMatchPlayPenaltyMethod::Panenka;
  const int32 Allowed=Tag==40?0:Tag==41?512:Tag==42?1024:Tag==43?2048:4096;
  if(!TestEqual(TEXT("Closed union exhaustive mixed-member rejection"),E.ValidatePayloadShape(),Mask==Allowed?Code::None:Code::InvalidPayload))return false;
 }
 for(int32 Tag=1;Tag<=39;++Tag)for(int32 Field=0;Field<4;++Field)
 {Envelope E;E.IntentKind=static_cast<Kind>(Tag);if(Field==0)E.SetPieceCardId=TEXT("Injected");if(Field==1)E.NearMethod=EMatchPlayShortFreeKickMethod::Direct;if(Field==2)E.LongMethod=EMatchPlayLongFreeKickMethod::Power;if(Field==3)E.PenaltyMethod=EMatchPlayPenaltyMethod::Panenka;TestEqual(TEXT("Legacy commands reject all new members"),E.ValidatePayloadShape(),Code::InvalidPayload);}
 for(int32 Tag=40;Tag<=44;++Tag)
 {
  Envelope E;E.MatchInstanceId=FGuid::NewGuid();E.RequestId=123;E.ExpectedAttackSequence=4;E.IntentKind=static_cast<Kind>(Tag);
  if(Tag==41)E.SetPieceCardId=TEXT("Prototype.Arsenal.MartinOdegaard");if(Tag==42)E.NearMethod=EMatchPlayShortFreeKickMethod::Angled;if(Tag==43)E.LongMethod=EMatchPlayLongFreeKickMethod::Power;if(Tag==44)E.PenaltyMethod=EMatchPlayPenaltyMethod::Panenka;
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Out(Writer,false);Envelope::StaticStruct()->SerializeItem(Out,&E,nullptr);
  Envelope Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive In(Reader,false);Envelope::StaticStruct()->SerializeItem(In,&Copy,nullptr);
  TestTrue(TEXT("Exact reflected typed envelope round trip"),Envelope::StaticStruct()->CompareScriptStruct(&E,&Copy,0));
 }
 for(int32 Value=45;Value<256;++Value){Envelope E;E.IntentKind=static_cast<Kind>(Value);TestEqual(TEXT("Unknown/internal tags remain closed"),E.ValidatePayloadShape(),Code::NotPlayerIntent);}
 for(int32 Value=0;Value<256;++Value)if(Value!=1&&Value!=2)
 {Envelope E;E.IntentKind=Kind::SubmitShortFreeKickMethod;E.NearMethod=static_cast<EMatchPlayShortFreeKickMethod>(Value);TestEqual(TEXT("Unknown Near enum"),E.ValidatePayloadShape(),Code::InvalidPayload);E={};E.IntentKind=Kind::SubmitLongFreeKickMethod;E.LongMethod=static_cast<EMatchPlayLongFreeKickMethod>(Value);TestEqual(TEXT("Unknown Long enum"),E.ValidatePayloadShape(),Code::InvalidPayload);E={};E.IntentKind=Kind::SubmitPenaltyMethod;E.PenaltyMethod=static_cast<EMatchPlayPenaltyMethod>(Value);TestEqual(TEXT("Unknown Penalty enum"),E.ValidatePayloadShape(),Code::InvalidPayload);}
 Envelope LongId;LongId.IntentKind=Kind::SubmitSetPieceCarrier;LongId.SetPieceCardId=FName(*FString::ChrN(129,TCHAR('X')));TestEqual(TEXT("Oversized ID"),LongId.ValidatePayloadShape(),Code::InvalidPayload);
 for(auto K:{EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll,EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier,EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod,EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod,EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod})
 {TestEqual(TEXT("Genuine existing PlayerIntent classification"),FMatchPlayAuthoritativeCommandClassification::OriginOf(K),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);FFixture F;const FUnchanged Before(F);auto R=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,FMatchPlayAuthoritativeSubmitRunnerRequest{}));TestEqual(TEXT("HostPort exact variant mismatch"),R.ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);Before.Verify(*this,F);}
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceIllegal,"FMCodex.NetworkPlay.SetPieceSelection.IllegalTakerAndEligibility",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceIllegal::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D6:{3,5,6}){auto P=FString::Printf(TEXT("%s.%d"),S,D6);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceIllegal::RunTest(const FString& P)
{
 FFixture F(P.StartsWith(TEXT("B")));const int32 D6=FCString::Atoi(*P.Right(1));if(!TestTrue(TEXT("Canonical taker wait"),Prepare(F,Kind::SubmitSetPieceCarrier,D6)))return false;
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Own=F.Attacker()==F.A?State.CardSnapshotAuthority.PlayerACardSnapshots.Cards:State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;const auto& Opp=F.Attacker()==F.A?State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards:State.CardSnapshotAuthority.PlayerACardSnapshots.Cards;
 TArray<FName> Invalid={TEXT("Nonexistent"),Opp[0].CardId};for(const auto& Card:Own)if(Card.bIsGoalkeeper)Invalid.Add(Card.CardId);
 for(auto Id:Invalid){auto E=Request(F,F.Attacker(),Kind::SubmitSetPieceCarrier);E.SetPieceCardId=Id;const FUnchanged B(F);TestEqual(TEXT("Domain rejects illegal/opponent/GK taker"),Send(F,F.Attacker(),E),Code::AuthorityRejected);B.Verify(*this,F);}
 FFMCodexLocalMatchViewerDisclosure Disclosure=FFMCodexLocalMatchViewerDisclosure::FullyDisclosed();
 auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),F.Attacker()->GetOwnerView().ViewerSide,Disclosure);
 const auto Expected=Safe.LegalSetPieceCardIds;const auto Bounded=FFMCodexSetPieceSelectionPresentation::Build(Safe,Safe.ExpectedActingPlayer);TestTrue(TEXT("Complete canonical ordered set"),Expected==Bounded.TakerOptions&&Expected.Num()==19);
 for(int32 Mutation=0;Mutation<4;++Mutation)
 {auto Bad=Safe;if(Mutation==0)Bad.LegalSetPieceCardIds.Add(Expected[0]);if(Mutation==1)Bad.LegalSetPieceCardIds[1]=Bad.LegalSetPieceCardIds[0];if(Mutation==2)Bad.LegalSetPieceCardIds[0]=TEXT("Unknown");if(Mutation==3)Bad.LegalSetPieceCardIds[0]=Opp[0].CardId;auto V=FFMCodexSetPieceSelectionPresentation::Build(Bad,Bad.ExpectedActingPlayer);TestTrue(TEXT("Overflow/duplicates/invalid IDs fail closed, no truncation"),V.bOptionsUnavailable&&V.TakerOptions.IsEmpty());}
 if(D6==5)
 {
  if(!TestTrue(TEXT("Ineligible taker still legal for Direct"),Taker(F,false)))return false;
  TestTrue(TEXT("Angled condition projected from frozen authority"),F.Attacker()->GetOwnerView().SetPiece.NearMethods==TArray<EMatchPlayShortFreeKickMethod>{EMatchPlayShortFreeKickMethod::Direct});
  const FUnchanged Before(F);TestEqual(TEXT("Forged Angled rejects"),Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitShortFreeKickMethod,true)),Code::AuthorityRejected);Before.Verify(*this,F);
  TestEqual(TEXT("Direct remains available"),Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::SubmitShortFreeKickMethod)),Code::Accepted);
 }
 return true;
}
class FSetPieceEntryProvider final : public IMatchPlayAttackEntryRollProvider
{
public:int32 Value=5,Calls=0;bool Fail=false;
 virtual FMatchPlayAttackEntryRollProviderResult RollD12(EMatchPlayAttackEntryRollPurpose)override{FMatchPlayAttackEntryRollProviderResult R;R.bSuccess=true;R.RawRoll=9;return R;}
 virtual FMatchPlayAttackEntryRollProviderResult RollD6(EMatchPlayAttackEntryRollPurpose)override{++Calls;FMatchPlayAttackEntryRollProviderResult R;R.bSuccess=!Fail;R.RawRoll=Value;return R;}
 virtual FMatchPlayAttackEntrySelectionProviderResult SelectUniformIndex(EMatchPlayAttackEntryRollPurpose,int32)override{return {};}
};
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceNoTaker,"FMCodex.NetworkPlay.SetPieceSelection.ReconstructedUsageAndProvider",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceNoTaker::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D6:{3,5,6})for(const TCHAR* Case:{TEXT("NoTaker"),TEXT("Used"),TEXT("Ejected"),TEXT("Failure"),TEXT("Malformed")}){auto P=FString::Printf(TEXT("%s.%d.%s"),S,D6,Case);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceNoTaker::RunTest(const FString& P)
{
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const int32 D6=FCString::Atoi(*Parts[1]);const FString Case=Parts[2];FSetPieceEntryProvider Provider;Provider.Value=D6;
 FFixture F(Parts[0]==TEXT("B"));Access::SetPiecePresentation(*F.Mode);if(!TestTrue(TEXT("Canonical D12 before reconstructed usage fixture"),Entry(F)))return false;
 auto State=Access::Session(*F.Mode).GetStateSnapshot();auto& Usage=F.Attacker()==F.A?State.CardUsageState.PlayerACardUsageState:State.CardUsageState.PlayerBCardUsageState;const auto& Cards=F.Attacker()==F.A?State.CardSnapshotAuthority.PlayerACardSnapshots.Cards:State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;FName Forbidden;
 // Valid reconstructed availability only; no route, winner, roll or Formula is forced.
 if(Case==TEXT("NoTaker")||Case==TEXT("Used")||Case==TEXT("Ejected"))for(const auto& Card:Cards)if(!Card.bIsGoalkeeper)
 {Forbidden=Card.CardId;Usage.AvailableCardIds.Remove(Card.CardId);if(Case==TEXT("Ejected"))Usage.EjectedCardIds.Add(Card.CardId);else Usage.UsedCardIds.Add(Card.CardId);if(Case!=TEXT("NoTaker"))break;}
 Access::ReconstructEntry(*F.Mode,State,Provider);Provider.Fail=Case==TEXT("Failure");if(Case==TEXT("Malformed"))Provider.Value=7;
 const FUnchanged Before(F);const auto Ack=Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll));
 if(Provider.Fail||Provider.Value==7)
 {TestEqual(TEXT("Provider rejects atomically"),Ack,Code::AuthorityRejected);TestEqual(TEXT("One provider call only"),Provider.Calls,1);Before.Verify(*this,F);return true;}
 TestEqual(TEXT("Genuine type draw after reconstruction"),Ack,Code::Accepted);TestEqual(TEXT("One type provider call"),Provider.Calls,1);
 const auto After=Access::Session(*F.Mode).GetStateSnapshot();NoScoreChange(*this,Before.State,After);
 if(Case==TEXT("NoTaker"))
 {
  TestEqual(TEXT("Canonical server internal NoGoal terminal"),After.CurrentAttack.LifecycleState,EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
  for(auto* PC:{F.A,F.B}){const auto& V=PC->GetOwnerView();TestTrue(TEXT("No impossible taker wait"),V.SetPiece.TakerOptions.IsEmpty()&&V.SetPiece.bNoTakerNoGoal);TestEqual(TEXT("Safe terminal NoGoal"),V.Terminal.Outcome,EFMCodexNetworkTerminalOutcome::NoGoal);TestEqual(TEXT("Existing advance belongs to actor"),V.bCanAdvance,PC==F.Attacker());}
  const int64 OldSequence=F.Attacker()->GetOwnerView().AttackSequence;
  if(!TestTrue(TEXT("Existing Advance continues no-taker terminal canonically"),F.Send(F.Attacker(),Kind::AdvanceAfterTerminal)))return false;
  if(!TestTrue(TEXT("Next attacker reaches genuine type wait"),Entry(F)))return false;
  auto E=Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll);E.ExpectedAttackSequence=OldSequence;const FUnchanged B(F);TestEqual(TEXT("Old request at same wait N+1 is stale"),Send(F,F.Attacker(),E),Code::StaleAttackSequence);B.Verify(*this,F);
  // The next attacker has its own available roster. Reach each matching wait through canonical selection.
  if(!TestTrue(TEXT("Actual N+1 type result"),TypeRoll(F,D6)))return false;
  auto OldTaker=Request(F,F.Attacker(),Kind::SubmitSetPieceCarrier);OldTaker.ExpectedAttackSequence=OldSequence;
  const FUnchanged BeforeTaker(F);TestEqual(TEXT("Old N taker at real N+1 taker wait is stale"),Send(F,F.Attacker(),OldTaker),Code::StaleAttackSequence);BeforeTaker.Verify(*this,F);
  if(!TestTrue(TEXT("Actual N+1 taker frozen"),Taker(F)))return false;
  auto OldMethod=Request(F,F.Attacker(),MethodKind(D6));OldMethod.ExpectedAttackSequence=OldSequence;
  const FUnchanged BeforeMethod(F);TestEqual(TEXT("Old N method at real N+1 matching method wait is stale"),Send(F,F.Attacker(),OldMethod),Code::StaleAttackSequence);BeforeMethod.Verify(*this,F);

 }
 else
 {TestFalse(TEXT("Used/ejected omitted from complete choices"),F.Attacker()->GetOwnerView().SetPiece.TakerOptions.Contains(Forbidden));auto E=Request(F,F.Attacker(),Kind::SubmitSetPieceCarrier);E.SetPieceCardId=Forbidden;const FUnchanged B(F);TestEqual(TEXT("Used/ejected cannot be forged"),Send(F,F.Attacker(),E),Code::AuthorityRejected);B.Verify(*this,F);}
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSetPieceOtherPhases,"FMCodex.NetworkPlay.SetPieceSelection.OtherWaitsAndSecureFailure",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexSetPieceOtherPhases::RunTest(const FString&)
{
 for(int32 Phase=0;Phase<3;++Phase)
 {
  FFixture F;Access::SetPiecePresentation(*F.Mode);
  if(Phase==1){F.Entropy->Word=3;if(!TestTrue(TEXT("Ordinary deployment"),F.Send(F.Attacker(),Kind::RequestInitialActionPointRoll)))return false;}
  if(Phase==2){if(!TestTrue(TEXT("Ordinary terminal"),Access::Runtime(*F.Mode).PrepareOrdinaryTerminalMilestone(true,false)))return false;Access::Publish(*F.Mode);}
  for(int32 Tag=40;Tag<=44;++Tag){auto E=Request(F,F.Attacker(),static_cast<Kind>(Tag));const FUnchanged B(F);TestEqual(TEXT("Set-piece cannot cross InitialD12/Deployment/ordinary terminal"),Send(F,F.Attacker(),E),Code::InvalidPhase);B.Verify(*this,F);}
 }
 FFixture F;if(!TestTrue(TEXT("Secure provider type wait"),Prepare(F,Kind::RequestSetPieceTypeRoll)))return false;
 const FUnchanged B(F);F.Entropy->bFail=true;TestEqual(TEXT("Secure entropy failure rejects type draw"),Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::RequestSetPieceTypeRoll)),Code::AuthorityRejected);
 TestTrue(TEXT("Failed secure draw adopts nothing"),SameState(B.State,Access::Session(*F.Mode).GetStateSnapshot()));TestEqual(TEXT("One attempted entropy call"),F.Entropy->Calls,B.EntropyCalls+1);TestEqual(TEXT("One entry-provider call"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),B.EntryCalls+1);TestEqual(TEXT("No coordinator"),F.Calls(),B.CoordinatorCalls);TestEqual(TEXT("No new View"),Access::Revision(*F.Mode),B.Revision);
 return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceTakerReplay,"FMCodex.NetworkPlay.SetPieceSelection.TakerReplayAndSharedNamespace",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceTakerReplay::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D6:{3,5,6}){auto P=FString::Printf(TEXT("%s.%d"),S,D6);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceTakerReplay::RunTest(const FString& P)
{
 FFixture F(P.StartsWith(TEXT("B")));const int32 D6=FCString::Atoi(*P.Right(1));if(!TestTrue(TEXT("Canonical taker wait"),Prepare(F,Kind::SubmitSetPieceCarrier,D6)))return false;
 auto* PC=F.Attacker();
 for(auto K:{Kind::SubmitShortFreeKickMethod,Kind::SubmitLongFreeKickMethod,Kind::SubmitPenaltyMethod})
 {auto E=Request(F,PC,K);const FUnchanged Before(F);TestEqual(TEXT("Every method rejects before frozen taker"),Send(F,PC,E),Code::InvalidPhase);Before.Verify(*this,F);}
 auto E=Request(F,PC,Kind::SubmitSetPieceCarrier);const FUnchanged Before(F);const FName Alternative=PC->GetOwnerView().SetPiece.TakerOptions[1];
 TestEqual(TEXT("Typed taker accepted"),Send(F,PC,E),Code::Accepted);const auto State=Access::Session(*F.Mode).GetStateSnapshot();
 const auto& SP=State.CurrentAttack.SetPieceRoute;const auto& Carrier=D6==5?SP.ShortFreeKick.Carrier:D6==6?SP.Penalty.Carrier:SP.LongFreeKick.Carrier;
 TestTrue(TEXT("Frozen authoritative taker identity"),Carrier.bIsBound&&Carrier.CardId==E.SetPieceCardId&&Carrier.OwnerSide==PC->GetOwnerView().ViewerSide);
 TestEqual(TEXT("Taker exact one coordinator"),F.Calls(),Before.CoordinatorCalls+1);TestEqual(TEXT("Taker exact one publication"),Access::Revision(*F.Mode),Before.Revision+1);TestEqual(TEXT("Taker zero RNG"),F.Entropy->Calls,Before.EntropyCalls);NoScoreChange(*this,Before.State,State);
 const FUnchanged After(F);TestEqual(TEXT("Taker duplicate cannot adopt twice"),Send(F,PC,E),Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
 auto Fresh=Request(F,PC,Kind::SubmitSetPieceCarrier);Fresh.SetPieceCardId=Alternative;TestEqual(TEXT("Fresh ID cannot replace frozen taker"),Send(F,PC,Fresh),Code::InvalidPhase);After.Verify(*this,F);
 auto Method=Request(F,PC,MethodKind(D6));Method.RequestId=E.RequestId;TestEqual(TEXT("Method shares already-consumed taker namespace"),Send(F,PC,Method),Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
 return true;
}

#endif
