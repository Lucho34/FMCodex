#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
#include "FMCodexNetworkOptionalDeclineTestFixture.h"
#include "../CoreRules/MatchPlayRecovery.h"
#include "TimerManager.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexMarkerOtherPhases,"FMCodex.NetworkPlay.MarkerDeclineTransport.OtherPhasesAndPrecedence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexMarkerOtherPhases::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* Side:{TEXT("A"),TEXT("B")})for(const TCHAR* Phase:{TEXT("Deployment"),TEXT("Runner"),TEXT("Helper"),TEXT("Skill"),TEXT("BothAbsent")})
 {auto P=FString::Printf(TEXT("%s.%s"),Side,Phase);N.Add(P);C.Add(P);}
}
bool FFMCodexMarkerOtherPhases::RunTest(const FString& P)
{
 using namespace FMCodexOptionalDeclineTests;
 FFixture F(P.StartsWith(TEXT("B")));auto* Attack=F.Attacker();auto* Defense=F.Defender();
 if(P.EndsWith(TEXT("Deployment"))||P.EndsWith(TEXT("BothAbsent")))
 {
  F.Entropy->Word=3;if(!TestTrue(TEXT("Ordinary entry"),F.Send(Attack,Kind::RequestInitialActionPointRoll)))return false;
  if(P.EndsWith(TEXT("BothAbsent")))
  {
   if(!TestTrue(TEXT("Empty attacker deployment accepted"),F.Send(Attack,Kind::FinishDeployment)))return false;
   const auto Entropy=F.Entropy->Calls;
   if(!TestTrue(TEXT("Empty defender deployment accepted"),F.Send(Defense,Kind::FinishDeployment)))return false;
   const auto& State=Access::Session(*F.Mode).GetStateSnapshot();
   TestFalse(TEXT("Attacker absence resolves without waiting for Marker"),State.bHasCurrentAttack);
   TestTrue(TEXT("Both absent is NoGoal, no scorer/history"),State.GoalHistory.IsEmpty()&&State.RuntimeState.PlayerAState.Score==0&&State.RuntimeState.PlayerBState.Score==0);
   TestEqual(TEXT("No closure RNG"),F.Entropy->Calls,Entropy);
   for(auto* PC:{F.A,F.B})TestTrue(TEXT("No phantom Goal or client deadlock"),PC->GetOwnerView().PublicGoalHistory.IsEmpty()&&PC->GetOwnerView().EntryWait==EFMCodexNetworkEntryWait::InitialD12&&PC->GetOwnerView().DeclineAction==Decline::None);
  }
 }
 else if(!TestTrue(TEXT("Different selection stage"),Prepare(F,P.EndsWith(TEXT("Runner"))?Kind::DeclineRunner:P.EndsWith(TEXT("Helper"))?Kind::DeclineHelper:Kind::DeclineSkill)))return false;
 const FUnchanged Before(F);const auto E=Request(F,Defense,Kind::DeclineMarker);
 TestEqual(TEXT("Marker request rejects outside Marker"),F.Mode->SubmitConnectionPlayerIntent(Defense,E).Code,Code::InvalidPhase);Before.Verify(*this,F);
 return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNetworkRecoveryFeedback,"FMCodex.NetworkPlay.MarkerRecoveryUI.RecoveryHandsAndNotification",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexNetworkRecoveryFeedback::GetTests(TArray<FString>& N,TArray<FString>& C) const
{N={TEXT("A.Next"),TEXT("B.Next"),TEXT("A.Final"),TEXT("B.Final")};C=N;}
bool FFMCodexNetworkRecoveryFeedback::RunTest(const FString& P)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 const bool Final=P.EndsWith(TEXT("Final"));FUIFixture F(P.StartsWith(TEXT("B"))!=Final,Final);
 if(!TestTrue(TEXT("Canonical Cross setup"),F.SkillFixture(Final)))return false;
 auto* Actor=F.Attacker();auto* Defense=F.Defender();auto* S=Actor->GetPlayerMatchScreen();
 S->RequestSubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));S->RequestSubmitBranchIntent(EFMCodexUMGBranchIntent::CrossLow);
 F.Entropy->Word=4;if(!Continue(*this,F,Actor,Kind::CrossInitialRouteRoll))return false;F.Settle();
 F.Entropy->Word=5;if(!Continue(*this,F,Actor,Kind::CrossHighAttackRoll))return false;F.Settle();
 F.Entropy->Word=0;if(!Continue(*this,F,Defense,Kind::CrossHighDefenseRoll))return false;F.Settle();
 const int64 Sequence=Actor->GetOwnerView().AttackSequence;
 const int32 Provider=Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount();
 if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
 const auto Advance=F.Backend(Actor).Last;
 const auto State=Access::Session(*F.Mode).GetStateSnapshot();
 TestEqual(TEXT("Canonical two returns or final zero"),State.LastRecoveryFact.ReturnedCards.Num(),Final?0:2);
 TestEqual(TEXT("Recovery provider exactly once unless final"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Provider+(Final?0:1));
 for(auto* PC:{F.A,F.B})
 {
  const auto& V=PC->GetOwnerView();const auto& M=PC->GetPlayerMatchScreen()->GetPresentation();
  TestEqual(TEXT("Shared nonblocking Recovery visible"),M.Resolution.bVisible,!Final);
  TestEqual(TEXT("Final retains Full Time"),M.FullTime.bVisible,Final);
  TestEqual(TEXT("Safe ordered recovery count"),V.Recovery.Cards.Num(),State.LastRecoveryFact.ReturnedCards.Num());
  FString Expected;
  for(int32 I=0;I<V.Recovery.Cards.Num();++I)
  {
   const auto& Card=V.Recovery.Cards[I];const auto& Fact=State.LastRecoveryFact.ReturnedCards[I];
   TestTrue(TEXT("Safe ordered identity equals authority"),Card.OwnerSide==Fact.OwnerSide&&Card.CardId==Fact.CardId);
   if(I)Expected+=TEXT("\n");Expected+=FFMCodexPlayerUIPresentationText::RecoveryEntry(Card.OwnerSide,Card.CardLabel.ToString()).ToString();
  }
  if(!Final){TestTrue(TEXT("Same Local nonblocking feedback"),M.Resolution.bNonBlockingNotification);TestEqual(TEXT("Exact shared Chinese recovery rows"),M.Resolution.StepSummaryLabel,Expected);}
  for(bool Local:{true,false})
  {
   const auto& Rack=Local?M.LocalRack:M.OpponentRack;
   const auto Owner=Local?V.ViewerSide:V.ViewerSide==Side::PlayerA?Side::PlayerB:Side::PlayerA;
   const auto& Usage=Owner==Side::PlayerA?State.CardUsageState.PlayerACardUsageState:State.CardUsageState.PlayerBCardUsageState;
   TSet<FName> Ids;
   for(const auto& Cell:Rack.Cells)
   {
    const auto& Card=Cell.Card;if(Card.CardId.IsNone())continue;
    TestFalse(TEXT("No duplicate hand card"),Ids.Contains(Card.CardId));Ids.Add(Card.CardId);
    TestEqual(TEXT("Both viewers agree available card"),Card.bAvailable,Usage.AvailableCardIds.Contains(Card.CardId));
    TestEqual(TEXT("Both viewers agree Used card"),Card.bUsed,Usage.UsedCardIds.Contains(Card.CardId));
   }
  }
 }
 const FUnchanged Before(F);
 TestEqual(TEXT("Duplicate Advance deduped"),F.Mode->SubmitConnectionPlayerIntent(Actor,Advance).Code,Code::DuplicateOrAlreadyResolved);Before.Verify(*this,F);
 TestEqual(TEXT("Duplicate never reruns Recovery"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Provider+(Final?0:1));
 if(!Final)
 {
  // Activate timers pending since the synchronous fixture, then measure elapsed time.
  ++GFrameCounter;F.World->GetTimerManager().Tick(0.f);
  // A duplicate View between elapsed intervals must not restart the deadline.
  ++GFrameCounter;F.World->GetTimerManager().Tick(1.1f);
  for(auto* PC:{F.A,F.B}){PC->RefreshPlayerFacingUI();TestTrue(TEXT("Notification alive before two seconds"),PC->GetPlayerMatchScreen()->GetPresentation().Resolution.bVisible);}
  ++GFrameCounter;F.World->GetTimerManager().Tick(1.1f);
  for(auto* PC:{F.A,F.B}){PC->RefreshPlayerFacingUI();TestFalse(TEXT("Repeated refresh does not restart elapsed notification"),PC->GetPlayerMatchScreen()->GetPresentation().Resolution.bVisible);}
  TestEqual(TEXT("Post-recovery next sequence"),F.Attacker()->GetOwnerView().AttackSequence,Sequence+1);
  F.Attacker()->GetPlayerMatchScreen()->RequestRollTacticalPoints();TestEqual(TEXT("Updated hand enters next D12"),F.Backend(F.Attacker()).LastCode,Code::Accepted);
 }
 return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNetworkRecoverySmallPool,"FMCodex.NetworkPlay.MarkerRecoveryUI.ZeroOneOrderedProjection",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexNetworkRecoverySmallPool::GetTests(TArray<FString>& N,TArray<FString>& C) const
{N={TEXT("0"),TEXT("1")};C=N;}
bool FFMCodexNetworkRecoverySmallPool::RunTest(const FString& P)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 FFixture F;auto State=Access::Session(*F.Mode).GetStateSnapshot();const int32 Count=FCString::Atoi(*P);
 // Canonical 0/1 Used-pool fixture, resolved by the production resolver without a provider.
 if(Count)
 {
  auto& Usage=State.CardUsageState.PlayerBCardUsageState;
  const auto* Card=State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.FindByPredicate([](const auto& C){return !C.bIsGoalkeeper;});
  if(!TestNotNull(TEXT("Non-GK fixture"),Card))return false;
  Usage.AvailableCardIds.Remove(Card->CardId);Usage.UsedCardIds.Add(Card->CardId);
 }
 const auto Recovered=FMatchPlayRecoveryResolver::Resolve(State.CardUsageState,State.CardSnapshotAuthority,1,nullptr);
 if(!TestTrue(TEXT("Canonical small pool resolves without RNG"),Recovered.bSuccess))return false;
 State.CardUsageState=Recovered.UpdatedCardUsageState;State.LastRecoveryFact=Recovered.RecoveryFact;
 State.RuntimeState.PlayerAState.UsedAttackCount=1;
 for(auto Side:{Side::PlayerA,Side::PlayerB})
 {
  auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),Side);
  auto V=FFMCodexNetworkClientViewSnapshotFactory::Build(Safe,F.Mode->GetMatchInstanceId(),1,Side,EFMCodexNetworkBootstrapState::MatchReady);
  V.Presentation=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Side);
  const auto M=FFMCodexNetworkMatchPresentationAdapter::Read(V,false);
  TestEqual(TEXT("Exact safe small-pool count"),V.Recovery.Cards.Num(),Count);
  TestEqual(TEXT("Empty returns have no empty notification"),M.Resolution.bVisible,Count==1);
  if(Count)TestTrue(TEXT("Actual owner/name kept"),M.Resolution.StepSummaryLabel.Contains(V.Recovery.Cards[0].CardLabel.ToString()));
 }
 return true;
}
#endif
