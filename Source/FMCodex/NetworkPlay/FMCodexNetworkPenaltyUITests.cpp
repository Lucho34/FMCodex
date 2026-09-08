#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPenaltyTestFixture.h"
namespace FMCodexPenaltyAutomation
{
using namespace FMCodexPenaltyTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPenaltyUI,"FMCodex.NetworkPlay.Penalty.SharedUI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexPenaltyUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* P:{TEXT("A.Direct.Goal.Next"),TEXT("B.Direct.NoGoal.Final"),TEXT("B.Panenka.Goal.Next"),TEXT("A.Panenka.NoGoal.Final")}) {N.Add(P);C.Add(P);}
}
bool FFMCodexPenaltyUI::RunTest(const FString& P)
{
 const bool Chip=P.Contains(TEXT("Panenka")),Goal=!P.Contains(TEXT("NoGoal")),Final=P.EndsWith(TEXT("Final"));
 FUIFixture F(P.StartsWith(TEXT("B")),Final);Access::SetPiecePresentation(*F.Mode,false);
 for(int32 Turn=0;Turn<(Final?2:1);++Turn)
 {
  auto* Actor=F.Attacker();auto* Other=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* W=Other->GetPlayerMatchScreen();
  F.Entropy->Word=8;S->RequestRollTacticalPoints();if(!TestEqual(TEXT("Real Full D12 screen intent"),F.Backend(Actor).LastCode,Code::Accepted))return false;F.Settle();
  TestTrue(TEXT("General UI supports common type entry"),S->GetPresentation().SetPiece.bSelectionSupported);
  F.Entropy->Word=5;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
  const FName Card=Eligible(F);S->DevSetPieceAction(TEXT("SetPieceTaker"),Card);S->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);
  S->DevSetPieceAction(Chip?TEXT("PenaltyPanenka"):TEXT("PenaltyDirect"),NAME_None);
  if(!TestEqual(TEXT("Method callback accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
  const auto Before=Actor->GetOwnerView();
  // Compare the actual Network result boxes against existing Local authority projection,
  // without repeating Penalty attribute/modifier arithmetic in the test or presentation.
  auto CheckDirectTotals = [&]()
  {
   const auto State=Access::Session(*F.Mode).GetStateSnapshot();
   const auto Local=FFMCodexLocalMatchInteractionViewBuilder::Build(State,Access::CallerRules(*F.Mode));
   const auto LocalFormula=FFMCodexLocalMatchUMGPresentationBuilder::Build(Local,{},FString()).InlineFormula;
   TestTrue(TEXT("Canonical known subtotals available"),Local.bHasSetPieceAttackKnownSubtotal&&Local.bHasSetPieceDefenseKnownSubtotal);
   for(auto* Screen:{S,W})for(int32 RowIndex=0;RowIndex<2;++RowIndex)
   {
    auto* Surface=Screen->GetInlineFormulaSurface();
    const auto& Row=RowIndex==0?Surface->GetPresentation().AttackRow:Surface->GetPresentation().DefenseRow;
    const auto& LocalRow=RowIndex==0?LocalFormula.AttackRow:LocalFormula.DefenseRow;
    const bool HasRoll=RowIndex==0?Local.bHasSetPieceAttackD6:Local.bHasSetPieceDefenseD6;
    const float Current=RowIndex==0?Local.SetPieceAttackCurrentTotal:Local.SetPieceDefenseCurrentTotal;
    TestTrue(TEXT("Known subtotal/current total is displayed before final Formula"),Row.bDisplayedResultResolved);
    TestEqual(TEXT("Displayed value comes from canonical Local current total"),Row.DisplayedResult,Current);
    TestEqual(TEXT("Local and Network result labels agree"),Row.DisplayedResultLabel,LocalRow.DisplayedResultLabel);
    TestEqual(TEXT("Known subtotal is not marked as a resolved final Formula"),Row.bFinalValueResolved,Local.bHasSetPieceFormula);
    const auto* Die=Row.Terms.FindByPredicate([](const auto& T){return T.Kind==EFMCodexUMGInlineFormulaTermKind::RawRoll;});
    if(TestNotNull(TEXT("Actual roll term retained"),Die))
    {
     TestEqual(TEXT("Only accepted/revealed roll becomes known"),Die->bResolved,HasRoll);
     if(!HasRoll){TestEqual(TEXT("Unresolved roll label stays unknown"),Die->DisplayLabel,FString(TEXT("掷点 ?")));TestEqual(TEXT("No future raw die"),Die->RawD6,0);}
    }
    const auto* Text=Cast<UTextBlock>(Surface->GetWidgetFromName(RowIndex==0?TEXT("InlineFormulaAttackFinalValue"):TEXT("InlineFormulaDefenseFinalValue")));
    if(TestNotNull(TEXT("Rendered right-side result box"),Text))TestEqual(TEXT("Actual rendered text matches Local"),Text->GetText().ToString(),LocalRow.DisplayedResultLabel);
   }
  };
  if(!Chip)CheckDirectTotals();
  TestTrue(TEXT("Penalty enabled outside DEV selection milestone"),S->GetPresentation().SetPiece.bSelectionSupported);
  for(auto* Screen:{S,W})
  {
   TestTrue(TEXT("Original shared Inline Formula shell"),Screen->GetInlineFormulaSurface()->GetPresentation().bVisible);
   TestTrue(TEXT("Both Penalty surfaces name canonical current actor"),Screen->GetInlineFormulaSurface()->GetPresentation().StatusLabel.Contains(Actor->GetOwnerView().ViewerSide==Side::PlayerA?TEXT("当前操作：玩家 A"):TEXT("当前操作：玩家 B")));
   TestEqual(TEXT("Direct has Formula rows, Chip outcome only"),Screen->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows,!Chip);
   TestEqual(TEXT("No duplicate lower Continue"),Screen->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionContinueButton"))->GetVisibility(),ESlateVisibility::Collapsed);
  }
  TestFalse(TEXT("Waiting viewer cannot submit"),W->GetPresentation().Interaction.PrimaryAction.bAvailable);
  auto SubmitRoll=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 A)
  {
   const auto H1=S->GetMatchHeader()->GetPresentation(),H2=W->GetMatchHeader()->GetPresentation();
   F.Entropy->PendingWords={uint32(A-1)};
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
    bool SawHold=false;
    for(int32 Tick=0;Tick<40&&Screen->IsInlineFormulaRevealInputBlocked();++Tick)
    {
     const auto& D=Screen->GetInlineFormulaSurface()->GetPresentation();
     SawHold|=Screen->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::ResultHold;
     if(K==Panenka && Screen->GetInlineFormulaRevealPhase()!=EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
      TestEqual(TEXT("Single-die Panenka outcome hint"),D.RollHelperLabel,FFMCodexPlayerUIPresentationText::SetPieceCompactOutcomeHint(Type::Penalty).ToString());
     if(!D.bNarrativeAvailable)
     {
      TestEqual(TEXT("A score gated through Formula/Narrative suspense"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,H.PlayerAScoreLabel);
      TestEqual(TEXT("B score gated through Formula/Narrative suspense"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,H.PlayerBScoreLabel);
     }
     TestFalse(TEXT("No next action CTA during reveal"),D.PrimaryAction.bVisible);
     Screen->AdvanceInlineFormulaRevealForTesting(0.5f);
    }
    TestTrue(TEXT("Real elapsed ResultHold observed"),SawHold);

    if(K==Panenka)TestTrue(TEXT("Panenka hint clears at result"),Screen->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel.IsEmpty());
    TestFalse(TEXT("Reveal completed"),Screen->IsInlineFormulaRevealInputBlocked());
   }
   return true;
  };
  if(!SubmitRoll(Actor,Chip?Panenka:Attack,Chip?(Goal?2:1):(Goal?6:1)))return false;
  if(!Chip)
  {
   CheckDirectTotals();
   TestTrue(TEXT("Defense gets CTA after attack reveal"),W->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   TestFalse(TEXT("Attack viewer waits for defender"),S->GetInlineFormulaSurface()->GetPresentation().PrimaryAction.bVisible);
   if(!SubmitRoll(Other,Defense,Goal?1:6))return false;
  }
  if(!Chip)
  {
   CheckDirectTotals();
   // A persisted terminal with a hidden defense die retains its public facts,
   // but existing security gating withdraws the presentation context entirely.
   const auto State=Access::Session(*F.Mode).GetStateSnapshot();
   for(auto Viewer:{Side::PlayerA,Side::PlayerB})
   {
    FFMCodexLocalMatchViewerDisclosure D;D.bRevealInitialActionPointRoll=D.bRevealSetPieceTypeRoll=true;D.RevealedContestD6Count=1;D.bRevealTerminalOutcome=true;
    const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Access::CallerRules(*F.Mode),Viewer,D);
    const auto Model=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Viewer);
    TestFalse(TEXT("Hidden defense current total remains redacted"),Safe.bHasSetPieceDefenseCurrentTotal);
    TestTrue(TEXT("Public non-roll facts survive redaction"),Safe.bHasSetPieceAttackKnownSubtotal&&Safe.bHasSetPieceDefenseKnownSubtotal);
    TestFalse(TEXT("Subtotal fallback cannot restore a security-hidden terminal surface"),Model.InlineFormula.bVisible);
    TestFalse(TEXT("Subtotal fallback does not resolve a final comparison"),Model.InlineFormula.AttackRow.bFinalValueResolved||Model.InlineFormula.DefenseRow.bFinalValueResolved);
    TestFalse(TEXT("No hidden die, Formula, winner or outcome"),Safe.bHasSetPieceDefenseD6||Safe.bHasSetPieceFormula||Safe.bHasSetPieceOutcome);
    TestFalse(TEXT("No terminal narrative"),Model.InlineFormula.bNarrativeAvailable);
    TestTrue(TEXT("No early scorer/history"),Safe.SetPieceGoalScorerCardId.IsNone()&&Safe.GoalHistory.Num()==Before.PublicGoalHistory.Num());
    TestEqual(TEXT("No early score A"),Safe.PlayerAScore,Before.PlayerAScore);TestEqual(TEXT("No early score B"),Safe.PlayerBScore,Before.PlayerBScore);
   }
  }
  TestEqual(TEXT("Canonical final outcome"),Actor->GetOwnerView().Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
  for(auto* Screen:{S,W})
  {
   TestEqual(TEXT("A score catches up at narrative"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
   TestEqual(TEXT("B score catches up at narrative"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
   TestTrue(TEXT("Terminal narrative from safe projection"),Screen->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
  }
  // A skipped publication must still consume both identities exactly once.
  if(Turn==0&&!Chip)
  {
   auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());auto Slate=Late->TakeWidget();
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Before,false));
   Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Actor->GetOwnerView(),false));Late->PauseInlineFormulaRevealTimerForTesting();
   TestTrue(TEXT("Coalesced accepted dice enter queue"),Late->IsInlineFormulaRevealInputBlocked());bool SawFirst=false,SawSecond=false;
   for(int32 I=0;I<24&&Late->IsInlineFormulaRevealInputBlocked();++I)
   {
    const auto& D=Late->GetInlineFormulaSurface()->GetPresentation();
    const bool First=D.DiceOwnerLabel==(TEXT("进攻方掷点"));
    const bool Second=D.DiceOwnerLabel==(TEXT("防守方掷点"));
    SawFirst|=First;SawSecond|=Second;
    if(First){TestFalse(TEXT("Coalesced first event cannot show terminal narrative"),D.bNarrativeAvailable);if(!Chip)TestFalse(TEXT("Coalesced defense total stays hidden during attack"),D.DefenseRow.bFinalValueResolved);}
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
   TestFalse(TEXT("Old Penalty shell clears on Advance"),Screen->GetPresentation().SetPiece.bVisible);
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
}
#endif
