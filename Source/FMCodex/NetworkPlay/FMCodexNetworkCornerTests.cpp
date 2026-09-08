#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
#include "../CoreRules/MatchPlayCornerResolution.h"
namespace FMCodexCornerNetworkTests
{
using namespace FMCodexSetPieceSelectionTests;
using FBoundary = FMCodexNearFreeKickTests::FBoundary;
using Outcome = EFMCodexNetworkTerminalOutcome;
using CornerStage = EMatchPlaySetPieceCornerRouteStage;
using Route = EMatchPlayCornerRouteIntent;
constexpr Kind AttackerList = Kind::SubmitCornerAttackerNominations, DefenderList = Kind::SubmitCornerDefenderNominations;
constexpr Kind Select = Kind::RequestCornerParticipantSelectionRoll, Intent = Kind::SubmitCornerIntent;
constexpr Kind RouteRoll = Kind::RequestCornerRouteRoll, Attack = Kind::RequestCornerAttackRoll, Defense = Kind::RequestCornerDefenseRoll;
bool Enter(FFixture& F)
{ Access::Runtime(*F.Mode).EnablePlayerFacingPresentation(); Access::SetPiecePresentation(*F.Mode,false); return Entry(F)&&TypeRoll(F,1); }
Envelope Nomination(FFixture& F,bool Def,int32 Count)
{
 auto* PC=Def?F.Defender():F.Attacker();auto E=Request(F,PC,Def?DefenderList:AttackerList);
 const auto& Options=PC->GetOwnerView().SetPiece.CornerOptions;
 for(int32 I=Count-1;I>=0;--I)if(Options.IsValidIndex(I))E.CornerCandidateIds.Add(Options[I]);
 return E;
}
Code SubmitRoll(FFixture& F,Kind K,int32 D6)
{auto* PC=K==Defense?F.Defender():F.Attacker();F.Entropy->Word=D6-1;return Send(F,PC,Request(F,PC,K));}
bool Lock(FFixture& F,int32 A,int32 D)
{return Send(F,F.Attacker(),Nomination(F,false,A))==Code::Accepted&&Send(F,F.Defender(),Nomination(F,true,D))==Code::Accepted;}
bool RouteReady(FFixture& F,Route R=Route::High)
{if(SubmitRoll(F,Select,4)!=Code::Accepted)return false;auto E=Request(F,F.Attacker(),Intent);E.CornerIntent=R;return Send(F,F.Attacker(),E)==Code::Accepted&&SubmitRoll(F,RouteRoll,1)==Code::Accepted;}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerContract,"FMCodex.NetworkPlay.Corner.Contract",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerContract::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* P:{TEXT("A.High.Goal"),TEXT("B.Low.NoGoal")}){N.Add(P);C.Add(P);}}
bool FCornerContract::RunTest(const FString& P)
{
 const bool Goal=P.StartsWith(TEXT("A"));FFixture F(!Goal);if(!TestTrue(TEXT("Canonical Corner entry"),Enter(F)))return false;
 const FBoundary Before(F);auto A=Nomination(F,false,3);
 TestEqual(TEXT("Ordered attacker lock"),Send(F,F.Attacker(),A),Code::Accepted);
 TestTrue(TEXT("Defender cannot read sealed identities/order"),F.Defender()->GetOwnerView().SetPiece.CornerAttackers.IsEmpty()&&F.Defender()->GetOwnerView().SetPiece.CornerAttackerRollLabels.IsEmpty());
 TestTrue(TEXT("Owner retains exact order"),F.Attacker()->GetOwnerView().SetPiece.CornerAttackers==A.CornerCandidateIds);
 auto D=Nomination(F,true,1);TestEqual(TEXT("Defender lock"),Send(F,F.Defender(),D),Code::Accepted);
 TestEqual(TEXT("Locking consumes no RNG"),F.Entropy->Calls,Before.EntropyCalls);
 for(auto* PC:{F.A,F.B}){const auto& V=PC->GetOwnerView();TestTrue(TEXT("Both lists released after defender lock"),V.SetPiece.CornerAttackers==A.CornerCandidateIds&&V.SetPiece.CornerDefenders==D.CornerCandidateIds);TestTrue(TEXT("No selected player or die before explicit click"),V.SetPiece.CornerRunner.IsNone()&&V.SetPiece.CornerHelper.IsNone()&&V.SetPiece.CornerSharedD6==0);}
 if(!TestTrue(TEXT("Participant click, route choice and route click"),RouteReady(F,Goal?Route::High:Route::Low)))return false;
 const auto Selected=Access::Session(*F.Mode).GetStateSnapshot();const auto& Corner=Selected.CurrentAttack.SetPieceRoute.Corner;
 TestEqual(TEXT("Shared D6 selects ordered second attacker"),Corner.Runner.CardId,A.CornerCandidateIds[1]);TestEqual(TEXT("Same D6 selects sole defender"),Corner.Helper.CardId,D.CornerCandidateIds[0]);
 TestEqual(TEXT("Count advantage from authority"),Corner.CandidateBonus,3);TestEqual(TEXT("Bonus belongs to attacker"),Corner.CandidateBonusSide,Selected.RuntimeState.CurrentAttackingPlayer);
 const auto Preview=FMatchPlayCornerResolution::QueryFormulaPreview(Selected);
 for(auto* PC:{F.A,F.B}){const auto& M=PC->GetOwnerView().Presentation.InlineFormula;TestTrue(TEXT("Both unknown dice have canonical known subtotals"),M.AttackRow.bDisplayedResultResolved&&M.DefenseRow.bDisplayedResultResolved);TestEqual(TEXT("Canonical attack subtotal"),M.AttackRow.DisplayedResult,Preview.AttackKnownSubtotal);TestEqual(TEXT("Canonical defense subtotal"),M.DefenseRow.DisplayedResult,Preview.DefenseKnownSubtotal);}
 TestEqual(TEXT("Attacker roll"),SubmitRoll(F,Attack,Goal?6:1),Code::Accepted);
 for(auto* PC:{F.A,F.B}){const auto& V=PC->GetOwnerView();TestEqual(TEXT("Only attack accepted"),V.AcceptedContestRolls.Num(),1);TestEqual(TEXT("No future terminal"),V.Terminal.Outcome,Outcome::None);TestEqual(TEXT("Defender owns next roll"),V.ExpectedActingSide,F.Defender()->GetOwnerView().ViewerSide);}
 TestEqual(TEXT("Defender roll"),SubmitRoll(F,Defense,Goal?1:6),Code::Accepted);
 const auto Final=Access::Session(*F.Mode).GetStateSnapshot();
 TestEqual(TEXT("Exactly four private draws"),F.Entropy->Calls,Before.EntropyCalls+4);
 TestEqual(TEXT("Seven accepted typed calls/publications"),Access::Revision(*F.Mode),Before.Revision+7);
 TestEqual(TEXT("Seven shared coordinator calls"),F.Calls(),Before.CoordinatorCalls+7);
 TestEqual(TEXT("Canonical Finishing suppression"),Final.CurrentAttack.SetPieceRoute.Corner.FormulaResolution.WinReason,EFormulaWinReason::FastSuppression);
 for(auto* PC:{F.A,F.B}){const auto& V=PC->GetOwnerView();TestEqual(TEXT("Correct viewer-safe outcome"),V.Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);TestEqual(TEXT("Five actual reveal events"),V.Presentation.ResolvedRolls.Num(),5);TestEqual(TEXT("Scorer is selected runner"),V.Terminal.Goal.ScorerCardId,Goal?Corner.Runner.CardId:NAME_None);}
 // Inspect persisted truth with incomplete security permissions, separately from presentation suspense.
 for(int32 Gate=0;Gate<4;++Gate)
 {
  FFMCodexLocalMatchViewerDisclosure X;X.bRevealInitialActionPointRoll=X.bRevealSetPieceTypeRoll=true;X.bRevealParticipantSelectionRoll=Gate>0;X.bRevealRouteRoll=Gate>1;X.RevealedContestD6Count=Gate>2?2:1;X.bRevealTerminalOutcome=Gate<3;
  auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Final,Access::CallerRules(*F.Mode),Final.RuntimeState.CurrentAttackingPlayer,X);
  auto M=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Final.RuntimeState.CurrentAttackingPlayer);
  TestFalse(TEXT("Incomplete permission hides Formula/outcome/Advance"),Safe.bHasSetPieceFormula||Safe.bHasSetPieceOutcome||Safe.bTerminalPendingAdvance);
  TestTrue(TEXT("No future scorer/history"),Safe.SetPieceGoalScorerCardId.IsNone()&&Safe.GoalHistory.IsEmpty());TestFalse(TEXT("No terminal narrative"),M.InlineFormula.bNarrativeAvailable);
  if(Gate<2)TestFalse(TEXT("No subtotal derived from hidden selected players/route"),Safe.bHasSetPieceAttackKnownSubtotal||Safe.bHasSetPieceDefenseKnownSubtotal);
 }
 auto* Actor=F.Attacker();auto Advance=Request(F,Actor,Kind::AdvanceAfterTerminal);TestEqual(TEXT("Canonical advance"),Send(F,Actor,Advance),Code::Accepted);
 const FBoundary Advanced(F);TestEqual(TEXT("Duplicate cannot consume/recover twice"),Send(F,Actor,Advance),Code::DuplicateOrAlreadyResolved);Advanced.VerifyFailure(*this,F);
 const auto ActorSide=Selected.RuntimeState.CurrentAttackingPlayer;
 const auto& Usage=ActorSide==Side::PlayerA?Advanced.State.CardUsageState.PlayerACardUsageState:Advanced.State.CardUsageState.PlayerBCardUsageState;
 for(FName Id:A.CornerCandidateIds)if(Id!=Corner.Runner.CardId)TestTrue(TEXT("Unselected nominees stay available"),Usage.AvailableCardIds.Contains(Id));
 TestTrue(TEXT("Actual participant consumed or recovered"),Usage.UsedCardIds.Contains(Corner.Runner.CardId)||F.Attacker()->GetOwnerView().Recovery.Cards.ContainsByPredicate([&](const auto& C){return C.CardId==Corner.Runner.CardId&&C.OwnerSide==ActorSide;}));
 TestTrue(TEXT("Old Corner events clear"),F.Attacker()->GetOwnerView().Presentation.ResolvedRolls.IsEmpty());
 if(!TestTrue(TEXT("Next genuine Full D12"),Entry(F)))return false;
 auto Old=Request(F,F.Attacker(),Attack);Old.ExpectedAttackSequence=Final.CurrentAttack.AttackSequence;const FBoundary Next(F);TestEqual(TEXT("Stale old attack rejected"),Send(F,F.Attacker(),Old),Code::StaleAttackSequence);Next.VerifyFailure(*this,F);
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerZero,"FMCodex.NetworkPlay.Corner.Zero",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerZero::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* P:{TEXT("AttackZero"),TEXT("BothZero"),TEXT("DefenseZeroOne"),TEXT("DefenseZeroThree")}){N.Add(P);C.Add(P);}}
bool FCornerZero::RunTest(const FString& P)
{
 const int32 A=P.StartsWith(TEXT("Defense"))?(P.EndsWith(TEXT("One"))?1:3):0,D=P==TEXT("AttackZero")?2:0;
 FFixture F; if(!TestTrue(TEXT("Entry"),Enter(F)))return false;const FBoundary Before(F);F.Entropy->Word=3;
 auto Nom=Nomination(F,false,A);TestEqual(TEXT("Attack lock never early-terminates"),Send(F,F.Attacker(),Nom),Code::Accepted);TestEqual(TEXT("Defense still chooses"),F.Defender()->GetOwnerView().EntryWait,Wait::CornerDefenderNominations);
 auto Def=Nomination(F,true,D);TestEqual(TEXT("Defense lock resolves zero branch"),Send(F,F.Defender(),Def),Code::Accepted);
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& C=State.CurrentAttack.SetPieceRoute.Corner;
 TestEqual(TEXT("Only multi-candidate automatic scorer draws"),F.Entropy->Calls,Before.EntropyCalls+(A>1?1:0));
 TestFalse(TEXT("No fake shared/route/contest dice or Formula"),C.bHasSharedParticipantD6||C.bHasRouteD6||C.bHasAttackD6||C.bHasDefenseD6||C.bHasFormulaResolution);
 TestTrue(TEXT("No fake defender"),C.Helper.CardId.IsNone());
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();TestEqual(TEXT("Zero precedence outcome"),V.Terminal.Outcome,A>0?Outcome::Goal:Outcome::NoGoal);
  TestEqual(TEXT("Only genuine type Reel"),V.Presentation.ResolvedRolls.Num(),1);TestTrue(TEXT("No public contest"),V.AcceptedContestRolls.IsEmpty());
  TestFalse(TEXT("No Formula rows"),V.Presentation.InlineFormula.bShowFormulaRows);TestTrue(TEXT("Existing terminal narrative"),V.Presentation.InlineFormula.bNarrativeAvailable);
  if(A>0){TestEqual(TEXT("Real scorer selected canonically"),V.Terminal.Goal.ScorerCardId,Nom.CornerCandidateIds[A>1?1:0]);TestEqual(TEXT("Safe automatic Runner for narrative"),V.SetPiece.CornerRunner,V.Terminal.Goal.ScorerCardId);TestFalse(TEXT("Real scorer, not system award"),V.Terminal.Goal.bSystemAward);}
 }
 auto Duplicate=Def;const FBoundary Done(F);TestEqual(TEXT("Duplicate defense lock never redraws scorer"),Send(F,F.Defender(),Duplicate),Code::DuplicateOrAlreadyResolved);Done.VerifyFailure(*this,F);
 if(A>0)
 {
  FFMCodexLocalMatchViewerDisclosure X;X.bRevealInitialActionPointRoll=X.bRevealSetPieceTypeRoll=true;
  auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),Side::PlayerB,X);
  TestTrue(TEXT("Unpublished automatic scorer remains private"),Safe.CornerRunner.CardId.IsNone()&&Safe.SetPieceGoalScorerCardId.IsNone()&&Safe.GoalHistory.IsEmpty());
 }
 TestEqual(TEXT("Zero branch uses shared Advance"),Send(F,F.Attacker(),Request(F,F.Attacker(),Kind::AdvanceAfterTerminal)),Code::Accepted);
 TestTrue(TEXT("Next opportunity available"),F.Attacker()->GetOwnerView().EntryWait==Wait::InitialD12);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCornerSecurity,"FMCodex.NetworkPlay.Corner.SecurityAndWire",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCornerSecurity::RunTest(const FString&)
{
 FFixture F;if(!TestTrue(TEXT("Entry"),Enter(F)))return false;
 auto Reject=[&](AFMCodexNetworkMatchPlayerController* PC,Envelope E,Code Expected){const FBoundary B(F);TestEqual(TEXT("Typed boundary rejects"),Send(F,PC,E),Expected);B.VerifyFailure(*this,F);};
 auto* Visitor=F.World->SpawnActor<AFMCodexNetworkMatchPlayerController>();Reject(Visitor,Nomination(F,false,1),Code::NotParticipant);
 auto E=Nomination(F,false,2);E.CornerCandidateIds[1]=E.CornerCandidateIds[0];Reject(F.Attacker(),E,Code::AuthorityRejected);
 E=Nomination(F,false,1);E.CornerCandidateIds[0]=TEXT("UnknownCard");Reject(F.Attacker(),E,Code::AuthorityRejected);
 E=Nomination(F,false,1);E.CornerCandidateIds.SetNum(4);Reject(F.Attacker(),E,Code::InvalidPayload);
 E=Nomination(F,false,1);E.MatchInstanceId=FGuid::NewGuid();Reject(F.Attacker(),E,Code::MatchMismatch);
 E=Nomination(F,false,1);E.ExpectedAttackSequence+=1;Reject(F.Attacker(),E,Code::StaleAttackSequence);
 E=Nomination(F,false,1);E.RequestId=F.Next(F.Attacker())+1025;Reject(F.Attacker(),E,Code::InvalidPayload);
 for(Kind K:{AttackerList,DefenderList,Select,Intent,RouteRoll,Attack,Defense})
 {
  Envelope Wire;Wire.IntentKind=K;if(K==Intent)Wire.CornerIntent=Route::Low;
  TestEqual(TEXT("Exact legal payload, including empty nominations"),Wire.ValidatePayloadShape(),Code::None);
  if(K==AttackerList||K==DefenderList)Wire.CornerCandidateIds={TEXT("Second"),TEXT("First")};
  auto Mixed=Wire;Mixed.PenaltyMethod=EMatchPlayPenaltyMethod::Direct;TestEqual(TEXT("Mixed family fails closed"),Mixed.ValidatePayloadShape(),Code::InvalidPayload);
  TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Out(Writer,false);Envelope::StaticStruct()->SerializeItem(Out,&Wire,nullptr);
  Envelope Copy;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive In(Reader,false);Envelope::StaticStruct()->SerializeItem(In,&Copy,nullptr);TestTrue(TEXT("Reflected RPC roundtrip"),Envelope::StaticStruct()->CompareScriptStruct(&Wire,&Copy,0));
  auto* Owner=K==DefenderList||K==Defense?F.Defender():F.Attacker();auto* Other=Owner==F.A?F.B:F.A;
  auto Wrong=Request(F,Other,K);Wrong.CornerCandidateIds=Wire.CornerCandidateIds;Wrong.CornerIntent=Wire.CornerIntent;
  Reject(Other,Wrong,K==AttackerList?Code::WrongSide:Code::InvalidPhase);
 }
 for(int32 Tag=1;Tag<=53;++Tag){Envelope Old;Old.IntentKind=static_cast<Kind>(Tag);Old.CornerIntent=Route::High;TestEqual(TEXT("Every previous intent rejects Corner payload"),Old.ValidatePayloadShape(),Code::InvalidPayload);}
 for(int32 Tag=61;Tag<256;++Tag){Envelope Bad;Bad.IntentKind=static_cast<Kind>(Tag);TestEqual(TEXT("Unknown/internal commands are not admitted"),Bad.ValidatePayloadShape(),Code::NotPlayerIntent);}
 for(auto K:{EMatchPlayAuthoritativeCommandKind::SubmitCornerAttackerNominations,EMatchPlayAuthoritativeCommandKind::SubmitCornerDefenderNominations,
  EMatchPlayAuthoritativeCommandKind::RequestCornerParticipantSelectionRoll,EMatchPlayAuthoritativeCommandKind::SubmitCornerIntent,
  EMatchPlayAuthoritativeCommandKind::RequestCornerRouteRoll,EMatchPlayAuthoritativeCommandKind::RequestCornerAttackRoll,EMatchPlayAuthoritativeCommandKind::RequestCornerDefenseRoll})
 {
  TestEqual(TEXT("Existing authority classification is PlayerIntent"),FMatchPlayAuthoritativeCommandClassification::OriginOf(K),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
  const FBoundary B(F);auto Result=Access::Runtime(*F.Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(K,FMatchPlaySetPieceTypeRollRequest{}));
  TestEqual(TEXT("Shared HostPort requires exact Corner variant"),Result.ErrorCode,EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch);B.VerifyFailure(*this,F);
 }
 // Same shared client ledger supports ACK before View, with no separate Corner retry authority.
 FFMCodexNetworkIntentClientState Client;Envelope Pending;
 while(Client.BeginCorner(F.Attacker()->GetOwnerView(),AttackerList,Pending,{} )&&Pending.RequestId<F.Next(F.Attacker()))
 {FFMCodexNetworkPlayerIntentAck Skip;Skip.MatchInstanceId=Pending.MatchInstanceId;Skip.RequestId=Pending.RequestId;Skip.Code=Code::InvalidPayload;Client.ObserveAck(Skip);}
 TestTrue(TEXT("Corner pending created"),Client.IsPending());auto Ack=F.Mode->SubmitConnectionPlayerIntent(F.Attacker(),Pending);TestEqual(TEXT("Empty list genuine intent"),Ack.Code,Code::Accepted);
 Client.ObserveAck(Ack);TestTrue(TEXT("ACK waits for safe View revision"),Client.IsPending());Client.ObserveView(F.Attacker()->GetOwnerView());TestFalse(TEXT("ACK and View release pending"),Client.IsPending());
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCornerProvider,"FMCodex.NetworkPlay.Corner.ProviderAtomicity",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCornerProvider::RunTest(const FString&)
{
 for(bool Automatic:{false,true})
 {
  FFixture F;if(!Enter(F))return false;
  TestEqual(TEXT("Attack nomination"),Send(F,F.Attacker(),Nomination(F,false,3)),Code::Accepted);
  if(!Automatic)TestEqual(TEXT("Defense nomination"),Send(F,F.Defender(),Nomination(F,true,2)),Code::Accepted);
  const FBoundary Before(F);F.Entropy->FailOnCall=F.Entropy->Calls+1;
  auto* PC=Automatic?F.Defender():F.Attacker();auto E=Automatic?Nomination(F,true,0):Request(F,PC,Select);
  TestEqual(TEXT("Private provider failure rejects atomically"),Send(F,PC,E),Code::AuthorityRejected);Before.VerifyFailure(*this,F,1);
  F.Entropy->FailOnCall=INDEX_NONE;F.Entropy->Word=3;E.RequestId=F.Next(PC)++;
  TestEqual(TEXT("Fresh retry consumes one real draw"),Send(F,PC,E),Code::Accepted);
  const FBoundary Done(F);TestEqual(TEXT("Successful duplicate cannot redraw"),Send(F,PC,E),Code::DuplicateOrAlreadyResolved);Done.VerifyFailure(*this,F);
 }
 return true;
}
}
#endif
