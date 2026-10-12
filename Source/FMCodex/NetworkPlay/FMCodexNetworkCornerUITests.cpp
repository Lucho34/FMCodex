#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkNearFreeKickTestFixture.h"
#include "../LocalPlay/FMCodexResolutionTheaterPrototype.h"
#include "Components/Button.h"
namespace FMCodexCornerNetworkUI
{
using namespace FMCodexSetPieceSelectionTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
using Outcome = EFMCodexNetworkTerminalOutcome;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPublicCardsSealedNominations,
 "FMCodex.NetworkPlay.Corner.PublicCardsAndSealedNominations",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPublicCardsSealedNominations::RunTest(const FString&)
{
 // In-process production controller/presentation harness, not a real remote connection.
 for (bool BFirst:{false,true})
 {
  FUIFixture F(BFirst);Access::SetPiecePresentation(*F.Mode,false);
  auto* Actor=F.Attacker();auto* Other=F.Defender();
  F.Entropy->Word=8;Actor->GetPlayerMatchScreen()->RequestRollTacticalPoints();F.Settle();
  F.Entropy->Word=0;Actor->GetPlayerMatchScreen()->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
  const auto Options=Actor->GetOwnerView().SetPiece.CornerOptions;
  if (!TestTrue(TEXT("Real Corner supplies three nominees"),Options.Num()>=3)) return false;
  for (int32 I=0;I<3;++I) Actor->GetPlayerMatchScreen()->DevSetPieceAction(TEXT("CornerCandidate"),Options[I]);
  Actor->GetPlayerMatchScreen()->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);
  if (!TestEqual(TEXT("Real nomination accepted"),F.Backend(Actor).LastCode,Code::Accepted)) return false;
  const auto State=Access::Session(*F.Mode).GetStateSnapshot();
  TestEqual(TEXT("Authority contains sealed nominations"),State.CurrentAttack.SetPieceRoute.Corner.AttackerNominees.Num(),3);
  for (auto* PC:{F.A,F.B})
  {
   const auto& Owner=PC->GetOwnerView();const auto& M=PC->GetPlayerMatchScreen()->GetPresentation();
   TestEqual(TEXT("Full public hands in both network screens"),M.LocalRack.Cells.Num()+M.OpponentRack.Cells.Num(),40);
   for (const auto* Rack:{&M.LocalRack,&M.OpponentRack}) for (const auto& Cell:Rack->Cells)
   {
    const auto& C=Cell.Card;
    const auto* Snapshot=State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
    if (!Snapshot) Snapshot=State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
    if (!TestNotNull(TEXT("Network card comes from authoritative snapshot"),Snapshot)) continue;
    TestEqual(TEXT("Network own/opposing Ranked count"),C.RankedTraits.Num(),Snapshot->RankedTraits.Num());
    for (int32 I=0;I<FMath::Min(C.RankedTraits.Num(),Snapshot->RankedTraits.Num());++I)
    {
     TestEqual(TEXT("Network public Trait identity"),C.RankedTraits[I].TraitId,Snapshot->RankedTraits[I].TraitId);
     TestEqual(TEXT("Network public Trait grade"),C.RankedTraits[I].Rank,Snapshot->RankedTraits[I].Rank);
    }
    TestTrue(TEXT("Network Binary presence preserved"),C.BinaryTraits==Snapshot->BinaryTraits);
   }
   TestTrue(TEXT("Future participant/route/contest dice absent"),Owner.SetPiece.CornerSharedD6==0 && Owner.SetPiece.CornerRouteD6==0 && Owner.AcceptedContestRolls.IsEmpty());
   TestEqual(TEXT("No future outcome"),Owner.Terminal.Outcome,Outcome::None);
  }
  TestTrue(TEXT("Opponent sees cards but not sealed attacker nominations"),Other->GetPlayerMatchScreen()->GetPresentation().SetPiece.CornerAttackers.IsEmpty());
  TestTrue(TEXT("Defender still owns nomination input"),Other->GetPlayerMatchScreen()->GetPresentation().SetPiece.bCornerDraft);
  TestFalse(TEXT("Public cards do not grant the waiting actor a command"),Actor->GetPlayerMatchScreen()->GetPresentation().Interaction.PrimaryAction.bAvailable);
 }
 return true;
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerUI,"FMCodex.NetworkPlay.Corner.SharedUI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* P:{TEXT("A.OpposedGoal.Next"),TEXT("B.AttackZero.Next"),TEXT("B.DefenseZero.Final")}){N.Add(P);C.Add(P);}}
bool FCornerUI::RunTest(const FString& P)
{
 const bool ZeroA=P.Contains(TEXT("AttackZero")),ZeroD=P.Contains(TEXT("DefenseZero")),Final=P.EndsWith(TEXT("Final"));
 const bool Theater=FMCodexResolutionTheaterPrototype::IsCornerResolutionEnabled();
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
   const TCHAR* HighButton=Theater?TEXT("TheaterHigh"):TEXT("CornerHighIntent");
   TestEqual(TEXT("Attacker chooses high through the active surface"),S->GetWidgetFromName(HighButton)->GetVisibility(),ESlateVisibility::Visible);
   if(Theater)TestFalse(TEXT("Waiting viewer cannot choose a route"),CastChecked<UButton>(W->GetWidgetFromName(HighButton))->GetIsEnabled());
   else TestEqual(TEXT("Defender sees no route CTA"),W->GetWidgetFromName(HighButton)->GetVisibility(),ESlateVisibility::Collapsed);
   CastChecked<UButton>(S->GetWidgetFromName(HighButton))->OnClicked.Broadcast();TestEqual(TEXT("Typed Corner intent"),F.Backend(Actor).Last.IntentKind,Kind::SubmitCornerIntent);
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
