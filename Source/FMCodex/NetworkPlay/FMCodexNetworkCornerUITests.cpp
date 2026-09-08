#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
namespace FMCodexCornerNetworkUI
{
using namespace FMCodexSetPieceSelectionTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
using Outcome = EFMCodexNetworkTerminalOutcome;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerUI,"FMCodex.NetworkPlay.Corner.SharedUI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* P:{TEXT("A.OpposedGoal.Next"),TEXT("B.AttackZero.Next"),TEXT("B.DefenseZero.Final")}){N.Add(P);C.Add(P);}}
bool FCornerUI::RunTest(const FString& P)
{
 const bool ZeroA=P.Contains(TEXT("AttackZero")),ZeroD=P.Contains(TEXT("DefenseZero")),Final=P.EndsWith(TEXT("Final"));
 FUIFixture F(P.StartsWith(TEXT("B")),Final);Access::SetPiecePresentation(*F.Mode,false);
 for(int32 Turn=0;Turn<(Final?2:1);++Turn)
 {
  auto* Actor=F.Attacker();auto* Other=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* W=Other->GetPlayerMatchScreen();
  F.Entropy->Word=8;S->RequestRollTacticalPoints();if(!TestEqual(TEXT("Shared Full D12"),F.Backend(Actor).LastCode,Code::Accepted))return false;F.Settle();
  F.Entropy->Word=0;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
  TestTrue(TEXT("Corner generally enabled without fixture dependency"),S->GetPresentation().SetPiece.bSelectionSupported);
  auto LockUI=[&](AFMCodexNetworkMatchPlayerController* PC,int32 Count)
  {
   auto* Screen=PC->GetPlayerMatchScreen();const auto Options=PC->GetOwnerView().SetPiece.CornerOptions;const int32 Before=F.Backend(PC).Sends;
   if(!TestTrue(TEXT("Authority supplies candidate choices"),Options.Num()>=Count))return false;
   for(int32 I=Count-1;I>=0;--I)Screen->DevSetPieceAction(TEXT("CornerCandidate"),Options[I]);
   if(Count==3){Screen->DevSetPieceAction(TEXT("CornerCandidate"),Options[1]);Screen->DevSetPieceAction(TEXT("CornerCandidate"),Options[1]);}
   TestEqual(TEXT("Draft select/remove/reorder never sends RPC"),F.Backend(PC).Sends,Before);
   Screen->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);
   if(Count<3)
   {
    TestEqual(TEXT("Underfull candidate confirmation does not send"),F.Backend(PC).Sends,Before);
    Screen->DevSetPieceAction(TEXT("CornerReturn"),NAME_None);
    TestEqual(TEXT("Return is presentation-only"),F.Backend(PC).Sends,Before);
    Screen->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);Screen->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);
   }
   TestEqual(TEXT("One typed lock after actual confirmation"),F.Backend(PC).Sends,Before+1);
   TestEqual(TEXT("Lock accepted"),F.Backend(PC).LastCode,Code::Accepted);
   return F.Backend(PC).LastCode==Code::Accepted;
  };
  if(!LockUI(Actor,ZeroA?0:ZeroD?2:3))return false;
  TestTrue(TEXT("Defender gets own action and sealed opponent acknowledgement"),W->GetPresentation().SetPiece.bCornerDraft&&W->GetPresentation().SetPiece.CornerAttackers.IsEmpty());
  TestFalse(TEXT("Attacker waits without duplicate lock CTA"),S->GetPresentation().Interaction.PrimaryAction.bAvailable);
  F.Entropy->Word=3;if(!LockUI(Other,ZeroD?0:ZeroA?2:1))return false;
  if(!ZeroA&&!ZeroD)
  {
   const auto BeforeSelection=Actor->GetOwnerView();
   auto RollUI=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 D6)
   {
    const auto HeaderA=S->GetMatchHeader()->GetPresentation(),HeaderB=W->GetMatchHeader()->GetPresentation();
    F.Entropy->Word=D6-1;if(!Continue(*this,F,PC,K))return false;
    for(auto* Screen:{S,W})
    {
     Screen->PauseInlineFormulaRevealTimerForTesting();TestTrue(TEXT("Both viewers play real accepted Corner Reel"),Screen->IsInlineFormulaRevealInputBlocked());
     const auto& Header=Screen==S?HeaderA:HeaderB;
     TestEqual(TEXT("No early score A"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Header.PlayerAScoreLabel);
     TestEqual(TEXT("No early score B"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Header.PlayerBScoreLabel);
     if(K==Kind::RequestCornerParticipantSelectionRoll)
     {TestEqual(TEXT("Selected-player choice CTA waits for reveal"),Screen->GetWidgetFromName(TEXT("CornerHighIntent"))->GetVisibility(),ESlateVisibility::Collapsed);}
    }
    const int32 Sends=F.Backend(PC).Sends;PC->GetPlayerMatchScreen()->RequestContinueResolution();TestEqual(TEXT("Repeat click blocked while revealing"),F.Backend(PC).Sends,Sends);
    const auto Phase=S->GetInlineFormulaRevealPhase();Actor->RefreshPlayerFacingUI();TestEqual(TEXT("Repeated View does not restart reveal"),S->GetInlineFormulaRevealPhase(),Phase);
    F.Settle();return true;
   };
   if(!RollUI(Actor,Kind::RequestCornerParticipantSelectionRoll,4))return false;
   const auto Published=Actor->GetOwnerView().SetPiece;
   TestFalse(TEXT("Runner and Helper selected from public lists"),Published.CornerRunner.IsNone()||Published.CornerHelper.IsNone());
   TestEqual(TEXT("Attacker chooses high through existing button callback"),S->GetWidgetFromName(TEXT("CornerHighIntent"))->GetVisibility(),ESlateVisibility::Visible);
   TestEqual(TEXT("Defender sees no route CTA"),W->GetWidgetFromName(TEXT("CornerHighIntent"))->GetVisibility(),ESlateVisibility::Collapsed);
   S->DevSetPieceAction(TEXT("CornerHigh"),NAME_None);TestEqual(TEXT("Typed Corner intent"),F.Backend(Actor).Last.IntentKind,Kind::SubmitCornerIntent);
   if(!RollUI(Actor,Kind::RequestCornerRouteRoll,1))return false;
   for(auto* Screen:{S,W})TestTrue(TEXT("Canonical known subtotal before attack"),Screen->GetInlineFormulaSurface()->GetPresentation().AttackRow.bDisplayedResultResolved&&Screen->GetInlineFormulaSurface()->GetPresentation().DefenseRow.bDisplayedResultResolved);
   if(!RollUI(Actor,Kind::RequestCornerAttackRoll,6))return false;
   TestTrue(TEXT("Defender receives real next CTA"),W->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   TestFalse(TEXT("Attacker waits after attack"),S->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   if(!RollUI(Other,Kind::RequestCornerDefenseRoll,1))return false;
   // Coalesced publications must queue all four distinct Corner events, not reveal the terminal at selection.
   auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());auto Slate=Late->TakeWidget();
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeSelection,false));
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Actor->GetOwnerView(),false));Late->PauseInlineFormulaRevealTimerForTesting();
   TestTrue(TEXT("Coalesced Corner enters reveal queue"),Late->IsInlineFormulaRevealInputBlocked());
   TestFalse(TEXT("No terminal narrative during coalesced participant reveal"),Late->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
   for(int32 I=0;I<80&&Late->IsInlineFormulaRevealInputBlocked();++I)Late->AdvanceInlineFormulaRevealForTesting(.5f);
   TestFalse(TEXT("Every accepted event settles"),Late->IsInlineFormulaRevealInputBlocked());
  }
  for(auto* Screen:{S,W})
  {
   TestEqual(TEXT("Correct zero/normal outcome"),Actor->GetOwnerView().Terminal.Outcome,ZeroA?Outcome::NoGoal:Outcome::Goal);
   TestTrue(TEXT("Shared canonical terminal narrative"),Screen->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
   TestEqual(TEXT("A score catches up at result"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
   TestEqual(TEXT("B score catches up at result"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
   if(ZeroA||ZeroD){TestFalse(TEXT("Zero branch has no fake Reel"),Screen->IsInlineFormulaRevealInputBlocked());TestFalse(TEXT("Zero branch has no fake Formula"),Screen->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows);}
   TestEqual(TEXT("No duplicate lower Continue"),Screen->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionContinueButton"))->GetVisibility(),ESlateVisibility::Collapsed);
  }
  const int32 Recovery=Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount();if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
  const bool Ended=Final&&Turn==1;
  for(auto* Screen:{S,W}){TestEqual(TEXT("Shared MatchEnd surface"),Screen->GetPresentation().FullTime.bVisible,Ended);TestFalse(TEXT("Corner shell clears on Advance"),Screen->GetPresentation().SetPiece.bVisible);}
  if(Ended){TestEqual(TEXT("No final Recovery RNG"),Access::Runtime(*F.Mode).GetRecoveryProviderInvocationCount(),Recovery);TestFalse(TEXT("No final D12"),S->GetPresentation().Interaction.bCanRollTacticalPoints||W->GetPresentation().Interaction.bCanRollTacticalPoints);}
  else{if(ZeroA)TestTrue(TEXT("No participant consumed, no fabricated recovery notice"),F.Attacker()->GetOwnerView().Recovery.Cards.IsEmpty());else TestTrue(TEXT("Canonical Recovery is nonblocking"),F.Attacker()->GetPlayerMatchScreen()->GetPresentation().Resolution.bNonBlockingNotification);TestTrue(TEXT("Next attacker is previous defender"),F.Attacker()==Other);}
 }
 return true;
}
}
#endif
