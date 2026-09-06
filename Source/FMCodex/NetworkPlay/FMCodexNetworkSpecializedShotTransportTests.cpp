#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSpecializedShotTestFixture.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSpecializedShotMatrix,"FMCodex.NetworkPlay.SpecializedShotTransport.Matrix",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSpecializedShotMatrix::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* Family:{TEXT("LongShot"),TEXT("CutInside")})
 {
  for(int32 GK=0;GK<=1;++GK)for(int32 A=1;A<=6;++A)for(int32 D=1;D<=(A<=2?1:6);++D)
  {auto P=FString::Printf(TEXT("%s.%s.Direct.%d.%d.%d"),S,Family,A,D,GK);N.Add(P);C.Add(P);}
  for(int32 A=1;A<=6;++A)for(int32 B=1;B<=6;++B)
  {auto P=FString::Printf(TEXT("%s.%s.DeadCorner.%d.%d.0"),S,Family,A,B);N.Add(P);C.Add(P);}
 }
}
bool FFMCodexSpecializedShotMatrix::RunTest(const FString& P)
{
 using namespace FMCodexSpecializedShotTests;
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const bool B=Parts[0]==TEXT("B"),Cut=Parts[1]==TEXT("CutInside"),Dead=Parts[2]==TEXT("DeadCorner"),GK=Parts[5]==TEXT("1");
 const int32 AD6=FCString::Atoi(*Parts[3]),DD6=FCString::Atoi(*Parts[4]);FShotFixture F(B),Local(B);
 if(!TestTrue(TEXT("Same canonical frozen branch on both paths"),F.ReadyBranch(Cut,Dead,GK)&&Local.ReadyBranch(Cut,Dead,GK)))return false;
 TestEqual(TEXT("Shot choice has no initial route die"),F.Attacker()->GetOwnerView().InitialRoute.D6,0);
 auto Step=[&](Kind K,int32 A,int32 D)
 {
  auto* PC=IsDefense(K)?F.Defender():F.Attacker();auto E=F.For(K,PC);const auto Intent=Canonical(E,PC->GetOwnerView().ViewerSide);const FFrozen Before(F);
  TestEqual(TEXT("Only canonical PlayerIntent exposed"),FMatchPlayAuthoritativeCommandClassification::OriginOf(Intent.CommandKind),EMatchPlayAuthoritativeCommandOrigin::PlayerIntent);
  F.Dice(A,D);Local.Dice(A,D);const auto Oracle=Access::Runtime(*Local.Mode).SubmitPlayerIntent(Intent);const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
  F.Entropy->PendingWords.Reset();Local.Entropy->PendingWords.Reset();
  if(!TestTrue(TEXT("Typed canonical port and connection transport accept"),Oracle.bSuccess&&Ack.Code==Code::Accepted))return false;
  const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Records=State.CurrentAttack.ResolutionSession.PostRouteRollProgress.RollRecords;
  TestTrue(TEXT("Full authoritative State equals canonical oracle"),SameState(State,Access::Session(*Local.Mode).GetStateSnapshot()));
  const int32 Draws=IsPaired(K)?2:1;const bool Terminal=State.CurrentAttack.LifecycleState==EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance;
  TestEqual(TEXT("Exact entropy attempts"),F.Entropy->Calls,Before.Entropy+Draws);
  TestEqual(TEXT("Exact post-route provider draws"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+Draws);
  TestEqual(TEXT("One Coordinator pass"),F.Calls(),Before.Coordinator+1);TestEqual(TEXT("One stable revision"),Ack.ViewRevision,Before.Revision+1);
  TestEqual(TEXT("LongShot internal terminal vs CutInside atomic terminal"),Oracle.CoordinatorResult.Steps.Num(),int32(Terminal&&!Cut));
  for(auto* Viewer:{F.A,F.B})
  {
   const auto& V=Viewer->GetOwnerView();TestEqual(TEXT("Accepted safe prefix count"),V.AcceptedContestRolls.Num(),Records.Num());
   for(int32 I=0;I<V.AcceptedContestRolls.Num()&&I<Records.Num();++I)
   {TestEqual(TEXT("Exact accepted die"),V.AcceptedContestRolls[I].D6,Records[I].RawD6);TestEqual(TEXT("Stable sequence"),V.AcceptedContestRolls[I].SequenceIndex,I);}
  }
  const FFrozen After(F);TestEqual(TEXT("Same ID duplicate cannot reroll"),F.Mode->SubmitConnectionPlayerIntent(PC,E).Code,Code::DuplicateOrAlreadyResolved);After.Verify(*this,F);
  TestEqual(TEXT("Fresh ID cannot reroll frozen step"),F.Mode->SubmitConnectionPlayerIntent(PC,F.For(K,PC)).Code,Code::AuthorityRejected);After.Verify(*this,F);
  AddInfo(FString::Printf(TEXT("STEP %s Kind=%d Dice=%d,%d Draws=%d Prefix=%d Internal=%d Terminal=%d"),*P,int32(K),A,D,Draws,Records.Num(),Oracle.CoordinatorResult.Steps.Num(),int32(Terminal)));return true;
 };
 if(!Step(Dead?PairKind(Cut):AttackKind(Cut),AD6,DD6))return false;
 if(!Dead&&AD6>=3)
 {
  TestEqual(TEXT("Attack-only expected defender"),F.Attacker()->GetOwnerView().ExpectedActingSide,F.Defender()->GetOwnerView().ViewerSide);
  TestFalse(TEXT("Attack-only has no resolved Formula"),F.Attacker()->GetOwnerView().Contest.bFormulaResolved);
  TestEqual(TEXT("Attack-only has no terminal"),F.Attacker()->GetOwnerView().Terminal.Outcome,Outcome::None);
  if(!Step(DefenseKind(Cut),DD6,1))return false;
 }
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto V=F.Attacker()->GetOwnerView();const bool Goal=V.Terminal.Outcome==Outcome::Goal;
 TestEqual(TEXT("Canonical terminal persisted"),State.CurrentAttack.LifecycleState,EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
 TestTrue(TEXT("Correct actor can Advance"),V.bCanAdvance);TestFalse(TEXT("Other actor cannot Advance"),F.Defender()->GetOwnerView().bCanAdvance);
 if(Dead)TestEqual(TEXT("DeadCorner canonical sum threshold"),Goal,AD6+DD6>=11);
 if(!Dead&&AD6<=2)TestEqual(TEXT("ImmediateMiss distinct from resolved Formula Miss"),V.Terminal.Outcome,Outcome::ImmediateMiss);
 TestEqual(TEXT("Formula only after real two-sided Direct"),V.Contest.bFormulaResolved,!Dead&&AD6>=3);
 TestEqual(TEXT("Exactly one history entry iff Goal"),State.GoalHistory.Num(),int32(Goal));TestEqual(TEXT("Public history count coherent"),V.PublicGoalHistory.Num(),State.GoalHistory.Num());
 if(Goal)TestEqual(TEXT("Persisted Carrier scorer"),V.Terminal.Goal.ScorerCardId,State.CurrentAttack.SelectedAction.CarrierCardId);
 else TestTrue(TEXT("No phantom scorer"),V.Terminal.Goal.ScorerCardId.IsNone());
 const auto Hidden=F.Safe(2,false);TestTrue(TEXT("Independent permission hides current history"),Hidden.GoalHistory.IsEmpty());TestEqual(TEXT("Hidden score A"),Hidden.PlayerAScore,0);TestEqual(TEXT("Hidden score B"),Hidden.PlayerBScore,0);
 const auto HiddenWire=FFMCodexNetworkClientViewSnapshotFactory::Build(Hidden,F.Mode->GetMatchInstanceId(),1,Side::PlayerA,EFMCodexNetworkBootstrapState::MatchReady);
 TestEqual(TEXT("Hidden terminal absent on wire"),HiddenWire.Terminal.Outcome,Outcome::None);
 if(Dead)TestTrue(TEXT("Paired branch never fabricates Formula"),!F.Safe(2).ResolutionFacts.FormulaContests.ContainsByPredicate([](const auto& C){return C.bHasResolvedFormula;}));
 auto* Next=F.Defender();const int32 Post=Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount();
 TestEqual(TEXT("Shared explicit Advance"),F.Roll(Kind::AdvanceAfterTerminal).Code,Code::Accepted);TestEqual(TEXT("No extra shot RNG at Advance"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post);
 TestFalse(TEXT("Prior attack cleared"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);TestTrue(TEXT("Ownership changes to prior defender"),F.Attacker()==Next);
 TestTrue(TEXT("Canonical Recovery"),Next->GetOwnerView().Recovery.SourceAttackSequence>0);F.Entropy->Word=3;F.RequestedKind=Kind::RequestInitialActionPointRoll;
 TestEqual(TEXT("Next Full D12 accepts"),F.Mode->SubmitConnectionPlayerIntent(Next,F.Request(Next)).Code,Code::Accepted);return true;
}
#endif
