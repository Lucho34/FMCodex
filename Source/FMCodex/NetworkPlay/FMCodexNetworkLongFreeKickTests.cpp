#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkLongFreeKickTestFixture.h"
namespace FMCodexLongFreeKickAutomation
{
using namespace FMCodexLongFreeKickTests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexLongMatrix,"FMCodex.NetworkPlay.LongFreeKick.OutcomeMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexLongMatrix::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")}) for(const TCHAR* M:{TEXT("Direct"),TEXT("Power")})
 for(int32 A=1;A<=6;++A) for(int32 B=1;B<=6;++B)
 {if(M==FString(TEXT("Direct"))&&A<=2&&B!=1)continue;const auto P=FString::Printf(TEXT("%s.%s.%d.%d"),S,M,A,B);N.Add(P);C.Add(P);}
}
bool FFMCodexLongMatrix::RunTest(const FString& P)
{
 TArray<FString> Parts; P.ParseIntoArray(Parts,TEXT("."));
 const bool Power=Parts[1]==TEXT("Power");const int32 A=FCString::Atoi(*Parts[2]),B=FCString::Atoi(*Parts[3]);
 FFixture F(Parts[0]==TEXT("B"));if(!TestTrue(TEXT("Canonical Full D12/type/taker/method"),PrepareLong(F,Power)))return false;
 const auto Actor=F.Attacker()->GetOwnerView().ViewerSide;const FName TakerId=F.Attacker()->GetOwnerView().SetPiece.TakerCardId;
 const bool Early=!Power&&A<=2;const int32 Draws=Early?1:2;
 const FBoundary Before(F);bool Goal=Power&&A+B>=11;
 if(!Power)
 {
  if(!TestEqual(TEXT("Every attack D6 accepted"),Roll(F,Attack,A),Code::Accepted))return false;
  const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Long=State.CurrentAttack.SetPieceRoute.LongFreeKick;
  TestEqual(TEXT("Exact early/defender split"),Long.Stage,Early?Stage::Terminal:Stage::DirectAwaitingDefenseRoll);
  if(Early)
  {
   TestFalse(TEXT("Early NoGoal has no Formula"),Long.bHasFormulaResolution);
   TestFalse(TEXT("Early NoGoal has no defense die"),Long.bHasDefenseD6);NoScoreChange(*this,Before.State,State);
  }
  else
  {
  TestFalse(TEXT("No early Formula"),Long.bHasFormulaResolution);NoScoreChange(*this,Before.State,State);
  TestEqual(TEXT("Defender owns next input"),F.Defender()->GetOwnerView().ExpectedActingSide,F.Defender()->GetOwnerView().ViewerSide);
  for(auto* PC:{F.A,F.B}) {const auto& V=PC->GetOwnerView();TestEqual(TEXT("One public accepted die"),V.AcceptedContestRolls.Num(),1);TestEqual(TEXT("No future defense die"),V.Contest.DefenseD6,0);TestEqual(TEXT("No future outcome"),V.Terminal.Outcome,Outcome::None);}
  if(!TestEqual(TEXT("Defense accepted"),Roll(F,Defense,B),Code::Accepted))return false;
  const auto& Carrier=Before.State.CurrentAttack.SetPieceRoute.LongFreeKick.Carrier.Snapshot;
  const auto& Cards=Actor==Side::PlayerA?Before.State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards:Before.State.CardSnapshotAuthority.PlayerACardSnapshots.Cards;
  const auto* GK=Cards.FindByPredicate([](const auto& C){return C.bIsGoalkeeper;});if(!TestNotNull(TEXT("Canonical unique GK"),GK))return false;
  const float AttackTotal=Carrier.Attributes.LongShot+A;
  const float DefenseTotal=GK->GoalkeeperAttributes.Positioning+2+B;
  Goal=A==6&&B<=2?true:B==6&&A<=2?false:AttackTotal>DefenseTotal;
  const auto Final=Access::Session(*F.Mode).GetStateSnapshot();const auto& Formula=Final.CurrentAttack.SetPieceRoute.LongFreeKick.FormulaResolution;
  TestEqual(TEXT("Authoritative attack total"),Formula.AttackerFinalValue,AttackTotal);
  TestEqual(TEXT("Authoritative positioning +2 defense total"),Formula.DefenderFinalValue,DefenseTotal);
  TestEqual(TEXT("No ordinary deployment attack modifier"),Formula.AttackerTacticalPlayerModifier,0.f);
  TestEqual(TEXT("No ordinary deployment defense modifier"),Formula.DefenderTacticalPlayerModifier,0.f);
  TestEqual(TEXT("Finishing Formula persisted"),Formula.FormulaType,EFormulaType::Finishing);
  if(AttackTotal==DefenseTotal&&!((A==6&&B<=2)||(B==6&&A<=2)))TestEqual(TEXT("GK wins exact tie"),Formula.WinReason,EFormulaWinReason::DefenderWinsGoalkeeperTie);
  if((A==6&&B<=2)||(B==6&&A<=2))TestEqual(TEXT("Canonical fast suppression"),Formula.WinReason,EFormulaWinReason::FastSuppression);
  }
 }
 else if(!TestEqual(TEXT("One atomic paired intent"),Roll(F,Pair,A,B),Code::Accepted))return false;
 const auto Final=Access::Session(*F.Mode).GetStateSnapshot();const auto& Long=Final.CurrentAttack.SetPieceRoute.LongFreeKick;
 TestEqual(TEXT("Canonical persisted terminal"),Final.CurrentAttack.LifecycleState,EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
 TestEqual(TEXT("Outcome independent expected rule"),Long.GameplayOutcome,Goal?EMatchPlayLongFreeKickGameplayOutcome::Goal:EMatchPlayLongFreeKickGameplayOutcome::NoGoal);
 TestEqual(TEXT("Only Direct runs Formula"),Long.bHasFormulaResolution,!Power&&!Early);
 TestEqual(TEXT("Exact branch private draws"),F.Entropy->Calls,Before.EntropyCalls+Draws);
 TestEqual(TEXT("Exact branch post-route calls"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+Draws);
 TestEqual(TEXT("One coordinator pass per accepted intent"),F.Calls(),Before.CoordinatorCalls+(Power||Early?1:2));
 TestEqual(TEXT("One publication per accepted intent"),Access::Revision(*F.Mode),Before.Revision+(Power||Early?1:2));
 TestEqual(TEXT("No entry draw during resolution"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),Before.EntryCalls);
 TestEqual(TEXT("No recovery before Advance"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Before.Recovery);
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();TestEqual(TEXT("Exact accepted dice prefix"),V.AcceptedContestRolls.Num(),Draws);
  TestEqual(TEXT("Safe canonical terminal"),V.Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
  TestEqual(TEXT("Exact scorer only on Goal"),V.Terminal.Goal.ScorerCardId,Goal?TakerId:NAME_None);
  TestEqual(TEXT("One GoalHistory fact at most"),V.PublicGoalHistory.Num(),int32(Goal));
  TestEqual(TEXT("Advance owner only"),V.bCanAdvance,PC==F.Attacker());
  TestTrue(TEXT("Shared production surface available"),V.Presentation.InlineFormula.bVisible);
  TestEqual(TEXT("Power outcome only, Direct retains its attack context"),V.Presentation.InlineFormula.bShowFormulaRows,!Power);
  TestEqual(TEXT("Early terminal has no defense comparison row"),V.Presentation.InlineFormula.bShowDefenseRow,!Power&&!Early);
  TestEqual(TEXT("Type plus exact unique roll events"),V.Presentation.ResolvedRolls.Num(),1+Draws);
 }
 const FBoundary Terminal(F);TestEqual(TEXT("Fresh final reroll rejected"),Roll(F,Power?Pair:Early?Attack:Defense,A,B),Code::InvalidPhase);Terminal.VerifyFailure(*this,F);
 // Every disclosure prefix is tested against an already persisted terminal, independently of UI suspense.
 for(int32 Count=0;Count<=2;++Count)for(bool Reveal:{false,true})for(auto Viewer:{Side::PlayerA,Side::PlayerB})
 {
  FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=D.bRevealSetPieceTypeRoll=true;D.RevealedContestD6Count=Count;D.bRevealTerminalOutcome=Reveal;
  const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Final,Access::CallerRules(*F.Mode),Viewer,D);
  const auto V=FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),1,Viewer,EFMCodexNetworkBootstrapState::MatchReady);
  const auto M=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Viewer);
  if(Count<Draws||!Reveal)
  {TestFalse(TEXT("Hidden terminal Formula winner absent"),Safe.bHasSetPieceFormula);TestFalse(TEXT("Hidden outcome absent"),Safe.bHasSetPieceOutcome);TestEqual(TEXT("Hidden score absent"),Safe.PlayerAScore+Safe.PlayerBScore,0);TestTrue(TEXT("Hidden scorer history absent"),Safe.GoalHistory.IsEmpty());TestEqual(TEXT("Hidden terminal DTO absent"),V.Terminal.Outcome,Outcome::None);TestFalse(TEXT("Hidden terminal CTA absent"),V.bCanAdvance);}
  TestEqual(TEXT("No hidden accepted event exported"),M.ResolvedRolls.Num(),1+(Power?(Count>=2?2:0):FMath::Min(Count,Draws)));
 }
 const int64 Sequence=Final.CurrentAttack.AttackSequence;
 if(!TestTrue(TEXT("Existing AdvanceAfterTerminal accepted"),F.Send(F.Attacker(),Kind::AdvanceAfterTerminal)))return false;
 const auto Advanced=Access::Session(*F.Mode).GetStateSnapshot();const auto& Usage=Actor==Side::PlayerA?Advanced.CardUsageState.PlayerACardUsageState:Advanced.CardUsageState.PlayerBCardUsageState;
 const auto& Recovery=F.Attacker()->GetOwnerView().Recovery;
 TestTrue(TEXT("Consumed taker is Used or canonically recovered to Available"),Usage.UsedCardIds.Contains(TakerId)||(Usage.AvailableCardIds.Contains(TakerId)&&Recovery.Cards.ContainsByPredicate([&](const auto& C){return C.OwnerSide==Actor&&C.CardId==TakerId;})));
 TestEqual(TEXT("Exactly next opportunity"),F.Attacker()->GetOwnerView().AttackSequence,Sequence+1);
 TestTrue(TEXT("No stale Long events"),F.Attacker()->GetOwnerView().Presentation.ResolvedRolls.IsEmpty());
 TestEqual(TEXT("Score/history persist across Advance"),Advanced.GoalHistory.Num(),int32(Goal));
 if(!TestTrue(TEXT("Next real D12 reaches type wait"),Entry(F)))return false;
 auto Stale=Request(F,F.Attacker(),Power?Pair:Attack);Stale.ExpectedAttackSequence=Sequence;const FBoundary Next(F);
 TestEqual(TEXT("Old attack sequence cannot act in N+1"),Send(F,F.Attacker(),Stale),Code::StaleAttackSequence);Next.VerifyFailure(*this,F);
 return true;
}
}
#endif
