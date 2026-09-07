#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
using namespace FMCodexNearFreeKickTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexNearUI,"FMCodex.NetworkPlay.NearFreeKick.SharedUI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexNearUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* M:{TEXT("Direct"),TEXT("Angled")})
 for(const TCHAR* G:{TEXT("Goal"),TEXT("NoGoal")})for(const TCHAR* End:{TEXT("Next"),TEXT("Final")})
 {const auto P=FString::Printf(TEXT("%s.%s.%s.%s"),S,M,G,End);N.Add(P);C.Add(P);}
}
bool FFMCodexNearUI::RunTest(const FString& P)
{
 const bool Angled=P.Contains(TEXT("Angled")),Goal=!P.Contains(TEXT("NoGoal")),Final=P.EndsWith(TEXT("Final"));
 FUIFixture F(P.StartsWith(TEXT("B")),Final);Access::SetPiecePresentation(*F.Mode,false);
 for(int32 Turn=0;Turn<(Final?2:1);++Turn)
 {
  auto* Actor=F.Attacker();auto* Other=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* W=Other->GetPlayerMatchScreen();
  F.Entropy->Word=8;S->RequestRollTacticalPoints();if(!TestEqual(TEXT("Real Full D12 screen intent"),F.Backend(Actor).LastCode,Code::Accepted))return false;F.Settle();
  TestTrue(TEXT("General UI supports common type entry"),S->GetPresentation().SetPiece.bSelectionSupported);
  F.Entropy->Word=4;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
  const FName Card=Eligible(F);S->DevSetPieceAction(TEXT("SetPieceTaker"),Card);S->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);
  S->DevSetPieceAction(Angled?TEXT("NearAngled"):TEXT("NearDirect"),NAME_None);
  if(!TestEqual(TEXT("Method callback accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
  const auto Before=Actor->GetOwnerView();
  TestTrue(TEXT("Near enabled outside DEV selection milestone"),S->GetPresentation().SetPiece.bSelectionSupported);
  for(auto* Screen:{S,W})
  {
   TestTrue(TEXT("Original shared Inline Formula shell"),Screen->GetInlineFormulaSurface()->GetPresentation().bVisible);
   TestTrue(TEXT("Both Near surfaces name canonical current actor"),Screen->GetInlineFormulaSurface()->GetPresentation().StatusLabel.Contains(Actor->GetOwnerView().ViewerSide==Side::PlayerA?TEXT("当前操作：玩家 A"):TEXT("当前操作：玩家 B")));
   TestEqual(TEXT("Direct has Formula rows, Angled outcome only"),Screen->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows,!Angled);
   TestEqual(TEXT("No duplicate lower Continue"),Screen->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionContinueButton"))->GetVisibility(),ESlateVisibility::Collapsed);
  }
  TestFalse(TEXT("Waiting viewer cannot submit"),W->GetPresentation().Interaction.PrimaryAction.bAvailable);
  auto SubmitRoll=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 A,int32 B)
  {
   const auto H1=S->GetMatchHeader()->GetPresentation(),H2=W->GetMatchHeader()->GetPresentation();
   F.Entropy->PendingWords={uint32(A-1)};if(K==Pair)F.Entropy->PendingWords.Add(uint32(B-1));
   if(!Continue(*this,F,PC,K))return false;
   const int32 Sends=F.Backend(PC).Sends;PC->GetPlayerMatchScreen()->RequestContinueResolution();TestEqual(TEXT("Reel blocks second click"),F.Backend(PC).Sends,Sends);
   for(auto* Viewer:{Actor,Other})
   {
    auto* Screen=Viewer->GetPlayerMatchScreen();Screen->PauseInlineFormulaRevealTimerForTesting();
    TestTrue(TEXT("Both viewers play accepted Reel"),Screen->IsInlineFormulaRevealInputBlocked());
    TestTrue(TEXT("Reel prompt retains roll owner instead of next actor"),Screen->GetInlineFormulaSurface()->GetPresentation().StatusLabel.Contains(PC->GetOwnerView().ViewerSide==Side::PlayerA?TEXT("当前操作：玩家 A"):TEXT("当前操作：玩家 B")));
    const auto& H=Viewer==Actor?H1:H2;
    TestEqual(TEXT("No early A score"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,H.PlayerAScoreLabel);
    TestEqual(TEXT("No early B score"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,H.PlayerBScoreLabel);
    const auto Phase=Screen->GetInlineFormulaRevealPhase();Viewer->RefreshPlayerFacingUI();TestEqual(TEXT("Duplicate View retains phase"),Screen->GetInlineFormulaRevealPhase(),Phase);
    Screen->AdvanceInlineFormulaRevealForTesting(0.01f);TestTrue(TEXT("Elapsed 10ms cannot finish hold"),Screen->IsInlineFormulaRevealInputBlocked());
    bool SawA=false,SawB=false,SawHold=false;
    for(int32 Tick=0;Tick<40&&Screen->IsInlineFormulaRevealInputBlocked();++Tick)
    {
     const auto& D=Screen->GetInlineFormulaSurface()->GetPresentation();
     SawHold|=Screen->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
     if(K==Pair)
     {
      SawA|=D.DiceOwnerLabel==TEXT("第一枚掷点");SawB|=D.DiceOwnerLabel==TEXT("第二枚掷点");
      if(Screen->GetInlineFormulaRevealPhase()!=EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
       TestEqual(TEXT("Pair helper through cycling and settling"),D.RollHelperLabel,FFMCodexPlayerUIPresentationText::SetPieceCompactOutcomeHint(Type::ShortFreeKick).ToString());
      if(D.DiceOwnerLabel==TEXT("第二枚掷点"))TestTrue(TEXT("A has appeared before B"),SawA);
     }
     if(!D.bNarrativeAvailable)
     {
      TestEqual(TEXT("A score gated through Formula/Narrative suspense"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,H.PlayerAScoreLabel);
      TestEqual(TEXT("B score gated through Formula/Narrative suspense"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,H.PlayerBScoreLabel);
     }
     TestFalse(TEXT("No next action CTA during reveal"),D.PrimaryAction.bVisible);
     Screen->AdvanceInlineFormulaRevealForTesting(0.5f);
    }
    TestTrue(TEXT("Real elapsed ResultHold observed"),SawHold);
    if(K==Pair){TestTrue(TEXT("Pair A then B used same reveal chain"),SawA&&SawB);TestTrue(TEXT("Pair helper hidden at final result"),Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel.IsEmpty());}
    TestFalse(TEXT("Reveal completed"),Screen->IsInlineFormulaRevealInputBlocked());
   }
   return true;
  };
  if(!SubmitRoll(Actor,Angled?Pair:Attack,Goal?6:1,Goal?3:1))return false;
  if(!Angled)
  {
   TestTrue(TEXT("Defense gets CTA after attack reveal"),W->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   TestFalse(TEXT("Attack viewer waits for defender"),S->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   if(!SubmitRoll(Other,Defense,Goal?1:6,1))return false;
  }
  TestEqual(TEXT("Canonical final outcome"),Actor->GetOwnerView().Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
  for(auto* Screen:{S,W})
  {
   TestEqual(TEXT("A score catches up at narrative"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
   TestEqual(TEXT("B score catches up at narrative"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
   TestTrue(TEXT("Terminal narrative from safe projection"),Screen->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
  }
  // A skipped publication must still consume both identities exactly once.
  if(Turn==0)
  {
   auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());auto Slate=Late->TakeWidget();
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Before,false));
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Actor->GetOwnerView(),false));Late->PauseInlineFormulaRevealTimerForTesting();
   TestTrue(TEXT("Coalesced accepted dice enter queue"),Late->IsInlineFormulaRevealInputBlocked());bool SawFirst=false,SawSecond=false;
   for(int32 I=0;I<24&&Late->IsInlineFormulaRevealInputBlocked();++I)
   {
    const auto& D=Late->GetInlineFormulaSurface()->GetPresentation();
    const bool First=D.DiceOwnerLabel==(Angled?TEXT("第一枚掷点"):TEXT("进攻方掷点"));
    const bool Second=D.DiceOwnerLabel==(Angled?TEXT("第二枚掷点"):TEXT("防守方掷点"));
    SawFirst|=First;SawSecond|=Second;
    if(First){TestFalse(TEXT("Coalesced first event cannot show terminal narrative"),D.bNarrativeAvailable);if(!Angled)TestFalse(TEXT("Coalesced defense total stays hidden during attack"),D.DefenseRow.bFinalValueResolved);}
    if(Second)TestTrue(TEXT("Coalesced events remain chronologically ordered"),SawFirst);
    Late->AdvanceInlineFormulaRevealForTesting(0.5f);
   }
   TestTrue(TEXT("Every coalesced identity was actually revealed"),SawFirst&&SawSecond);
   TestFalse(TEXT("Coalesced dice finish without replay"),Late->IsInlineFormulaRevealInputBlocked());
  }
  const int32 RecoveryBefore=Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount();
  if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
  const bool Ended=Final&&Turn==1;
  for(auto* Screen:{S,W})
  {
   TestEqual(TEXT("Canonical shared Full-Time"),Screen->GetPresentation().FullTime.bVisible,Ended);
   TestFalse(TEXT("Old Near shell clears on Advance"),Screen->GetPresentation().SetPiece.bVisible);
  }
  if(Ended)
  {TestEqual(TEXT("No Recovery RNG after final opportunity"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),RecoveryBefore);TestFalse(TEXT("No final D12 CTA"),S->GetPresentation().Interaction.bCanRollTacticalPoints||W->GetPresentation().Interaction.bCanRollTacticalPoints);}
  else
  {
   TestTrue(TEXT("Canonical Recovery fact"),F.Attacker()->GetOwnerView().Recovery.SourceAttackSequence>0);
   TestTrue(TEXT("Shared nonblocking Recovery presentation"),F.Attacker()->GetPlayerMatchScreen()->GetPresentation().Resolution.bNonBlockingNotification);
   TestTrue(TEXT("Next D12 actor is prior defender"),F.Attacker()==Other);
  }
 }
 if(!Final){F.Entropy->Word=8;auto* PC=F.Attacker();PC->GetPlayerMatchScreen()->RequestRollTacticalPoints();TestEqual(TEXT("Next D12 shared callback accepted"),F.Backend(PC).LastCode,Code::Accepted);}
 return true;
}
#endif
