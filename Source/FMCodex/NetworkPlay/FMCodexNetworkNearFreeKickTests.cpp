#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
using namespace FMCodexNearFreeKickTests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNearMatrix,"FMCodex.NetworkPlay.NearFreeKick.OutcomeMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexNearMatrix::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")}) for(const TCHAR* M:{TEXT("Direct"),TEXT("Angled")})
 for(int32 A=1;A<=6;++A) for(int32 B=1;B<=6;++B)
 {const auto P=FString::Printf(TEXT("%s.%s.%d.%d"),S,M,A,B);N.Add(P);C.Add(P);}
}
bool FFMCodexNearMatrix::RunTest(const FString& P)
{
 TArray<FString> Parts; P.ParseIntoArray(Parts,TEXT("."));
 const bool Angled=Parts[1]==TEXT("Angled");const int32 A=FCString::Atoi(*Parts[2]),B=FCString::Atoi(*Parts[3]);
 FFixture F(Parts[0]==TEXT("B"));if(!TestTrue(TEXT("Canonical Full D12/type/taker/method"),PrepareNear(F,Angled)))return false;
 const auto Actor=F.Attacker()->GetOwnerView().ViewerSide;const FName TakerId=F.Attacker()->GetOwnerView().SetPiece.TakerCardId;
 const FBoundary Before(F);bool Goal=A+B>=9;
 if(!Angled)
 {
  if(!TestEqual(TEXT("Every attack D6 accepted"),Roll(F,Attack,A),Code::Accepted))return false;
  const auto State=Access::Session(*F.Mode).GetStateSnapshot();const auto& Near=State.CurrentAttack.SetPieceRoute.ShortFreeKick;
  TestEqual(TEXT("All attack classes still require defender"),Near.Stage,Stage::DirectAwaitingDefenseRoll);
  TestFalse(TEXT("No early Formula"),Near.bHasFormulaResolution);NoScoreChange(*this,Before.State,State);
  TestEqual(TEXT("Defender owns next input"),F.Defender()->GetOwnerView().ExpectedActingSide,F.Defender()->GetOwnerView().ViewerSide);
  for(auto* PC:{F.A,F.B}) {const auto& V=PC->GetOwnerView();TestEqual(TEXT("One public accepted die"),V.AcceptedContestRolls.Num(),1);TestEqual(TEXT("No future defense die"),V.Contest.DefenseD6,0);TestEqual(TEXT("No future outcome"),V.Terminal.Outcome,Outcome::None);}
  if(!TestEqual(TEXT("Defense accepted"),Roll(F,Defense,B),Code::Accepted))return false;
  const auto& Carrier=Before.State.CurrentAttack.SetPieceRoute.ShortFreeKick.Carrier.Snapshot;
  const auto& Cards=Actor==Side::PlayerA?Before.State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards:Before.State.CardSnapshotAuthority.PlayerACardSnapshots.Cards;
  const auto* GK=Cards.FindByPredicate([](const auto& C){return C.bIsGoalkeeper;});if(!TestNotNull(TEXT("Canonical unique GK"),GK))return false;
  const float AttackTotal=FMath::Max(Carrier.Attributes.Shooting,Carrier.Attributes.Passing)+A;
  const float DefenseTotal=GK->GoalkeeperAttributes.Handling+1+B;
  Goal=A==6&&B<=2?true:B==6&&A<=2?false:AttackTotal>DefenseTotal;
  const auto Final=Access::Session(*F.Mode).GetStateSnapshot();const auto& Formula=Final.CurrentAttack.SetPieceRoute.ShortFreeKick.FormulaResolution;
  TestEqual(TEXT("Authoritative attack total"),Formula.AttackerFinalValue,AttackTotal);
  TestEqual(TEXT("Authoritative handling +1 defense total"),Formula.DefenderFinalValue,DefenseTotal);
  TestEqual(TEXT("No ordinary deployment attack modifier"),Formula.AttackerTacticalPlayerModifier,0.f);
  TestEqual(TEXT("No ordinary deployment defense modifier"),Formula.DefenderTacticalPlayerModifier,0.f);
  TestEqual(TEXT("Finishing Formula persisted"),Formula.FormulaType,EFormulaType::Finishing);
  if(AttackTotal==DefenseTotal)TestEqual(TEXT("GK wins exact tie"),Formula.WinReason,EFormulaWinReason::DefenderWinsGoalkeeperTie);
  if((A==6&&B<=2)||(B==6&&A<=2))TestEqual(TEXT("Canonical fast suppression"),Formula.WinReason,EFormulaWinReason::FastSuppression);
 }
 else if(!TestEqual(TEXT("One atomic paired intent"),Roll(F,Pair,A,B),Code::Accepted))return false;
 const auto Final=Access::Session(*F.Mode).GetStateSnapshot();const auto& Near=Final.CurrentAttack.SetPieceRoute.ShortFreeKick;
 TestEqual(TEXT("Canonical persisted terminal"),Final.CurrentAttack.LifecycleState,EMatchPlayCurrentAttackLifecycleState::TerminalPendingAdvance);
 TestEqual(TEXT("Outcome independent expected rule"),Near.GameplayOutcome,Goal?EMatchPlayShortFreeKickGameplayOutcome::Goal:EMatchPlayShortFreeKickGameplayOutcome::NoGoal);
 TestEqual(TEXT("Only Direct runs Formula"),Near.bHasFormulaResolution,!Angled);
 TestEqual(TEXT("Exactly two private draws"),F.Entropy->Calls,Before.EntropyCalls+2);
 TestEqual(TEXT("Exactly two post-route calls"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Before.Post+2);
 TestEqual(TEXT("One coordinator pass per accepted intent"),F.Calls(),Before.CoordinatorCalls+(Angled?1:2));
 TestEqual(TEXT("One publication per accepted intent"),Access::Revision(*F.Mode),Before.Revision+(Angled?1:2));
 TestEqual(TEXT("No entry draw during resolution"),Access::Runtime(*F.Mode).GetEntryProviderInvocationCount(),Before.EntryCalls);
 TestEqual(TEXT("No recovery before Advance"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Before.Recovery);
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();TestEqual(TEXT("Both accepted dice public"),V.AcceptedContestRolls.Num(),2);
  TestEqual(TEXT("Safe canonical terminal"),V.Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
  TestEqual(TEXT("Exact scorer only on Goal"),V.Terminal.Goal.ScorerCardId,Goal?TakerId:NAME_None);
  TestEqual(TEXT("One GoalHistory fact at most"),V.PublicGoalHistory.Num(),int32(Goal));
  TestEqual(TEXT("Advance owner only"),V.bCanAdvance,PC==F.Attacker());
  TestTrue(TEXT("Shared production surface available"),V.Presentation.InlineFormula.bVisible);
  TestEqual(TEXT("Angled outcome only, no fabricated arithmetic rows"),V.Presentation.InlineFormula.bShowFormulaRows,!Angled);
  TestEqual(TEXT("Type plus two unique reveal events"),V.Presentation.ResolvedRolls.Num(),3);
 }
 const FBoundary Terminal(F);TestEqual(TEXT("Fresh final reroll rejected"),Roll(F,Angled?Pair:Defense,A,B),Code::InvalidPhase);Terminal.VerifyFailure(*this,F);
 // Every disclosure prefix is tested against an already persisted terminal, independently of UI suspense.
 for(int32 Count=0;Count<=2;++Count)for(bool Reveal:{false,true})for(auto Viewer:{Side::PlayerA,Side::PlayerB})
 {
  FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=D.bRevealSetPieceTypeRoll=true;D.RevealedContestD6Count=Count;D.bRevealTerminalOutcome=Reveal;
  const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Final,Access::CallerRules(*F.Mode),Viewer,D);
  const auto V=FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),1,Viewer,EFMCodexNetworkBootstrapState::MatchReady);
  const auto M=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Viewer);
  if(Count<2||!Reveal)
  {TestFalse(TEXT("Hidden terminal Formula winner absent"),Safe.bHasSetPieceFormula);TestFalse(TEXT("Hidden outcome absent"),Safe.bHasSetPieceOutcome);TestEqual(TEXT("Hidden score absent"),Safe.PlayerAScore+Safe.PlayerBScore,0);TestTrue(TEXT("Hidden scorer history absent"),Safe.GoalHistory.IsEmpty());TestEqual(TEXT("Hidden terminal DTO absent"),V.Terminal.Outcome,Outcome::None);TestFalse(TEXT("Hidden terminal CTA absent"),V.bCanAdvance);}
  TestEqual(TEXT("No hidden accepted event exported"),M.ResolvedRolls.Num(),1+(Angled?(Count>=2?2:0):Count));
 }
 const int64 Sequence=Final.CurrentAttack.AttackSequence;
 if(!TestTrue(TEXT("Existing AdvanceAfterTerminal accepted"),F.Send(F.Attacker(),Kind::AdvanceAfterTerminal)))return false;
 const auto Advanced=Access::Session(*F.Mode).GetStateSnapshot();const auto& Usage=Actor==Side::PlayerA?Advanced.CardUsageState.PlayerACardUsageState:Advanced.CardUsageState.PlayerBCardUsageState;
 const auto& Recovery=F.Attacker()->GetOwnerView().Recovery;
 TestTrue(TEXT("Consumed taker is Used or canonically recovered to Available"),Usage.UsedCardIds.Contains(TakerId)||(Usage.AvailableCardIds.Contains(TakerId)&&Recovery.Cards.ContainsByPredicate([&](const auto& C){return C.OwnerSide==Actor&&C.CardId==TakerId;})));
 TestEqual(TEXT("Exactly next opportunity"),F.Attacker()->GetOwnerView().AttackSequence,Sequence+1);
 TestTrue(TEXT("No stale Near events"),F.Attacker()->GetOwnerView().Presentation.ResolvedRolls.IsEmpty());
 TestEqual(TEXT("Score/history persist across Advance"),Advanced.GoalHistory.Num(),int32(Goal));
 if(!TestTrue(TEXT("Next real D12 reaches type wait"),Entry(F)))return false;
 auto Stale=Request(F,F.Attacker(),Angled?Pair:Attack);Stale.ExpectedAttackSequence=Sequence;const FBoundary Next(F);
 TestEqual(TEXT("Old attack sequence cannot act in N+1"),Send(F,F.Attacker(),Stale),Code::StaleAttackSequence);Next.VerifyFailure(*this,F);
 return true;
}
#endif
