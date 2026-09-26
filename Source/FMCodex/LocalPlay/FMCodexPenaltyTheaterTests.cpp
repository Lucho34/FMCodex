#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkPenaltyTestFixture.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexRollReelWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "HAL/IConsoleManager.h"

namespace FMCodexPenaltyTheaterTests
{
using namespace FMCodexPenaltyTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
bool Shown(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
 const auto* W=S->GetWidgetFromName(Name);
 return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;
}
FString Text(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();}
FString Tip(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
 auto* Hover=CastChecked<UBorder>(S->GetWidgetFromName(Name));
 return CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Hover->GetToolTip())->GetContent())->GetContent())->GetText().ToString();
}
void Click(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{CastChecked<UButton>(S->GetWidgetFromName(Name))->OnClicked.Broadcast();}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FPenaltyTheaterLifecycle,"FMCodex.LocalPlay.ResolutionTheater.Penalty.Lifecycle",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FPenaltyTheaterLifecycle::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
 for (const TCHAR* SideName:{TEXT("A"),TEXT("B")})
  for (const TCHAR* Branch:{TEXT("DirectGoal"),TEXT("DirectSave"),TEXT("PanenkaGoal"),TEXT("PanenkaMiss")})
  {const FString P=FString(SideName)+TEXT(".")+Branch;Names.Add(P);Commands.Add(P);}
}
bool FPenaltyTheaterLifecycle::RunTest(const FString& P)
{
 using namespace FMCodexPenaltyTheaterTests;
 const bool Chip=P.Contains(TEXT("Panenka")),Goal=P.Contains(TEXT("Goal"));
 const int32 AttackDie=Chip?(Goal?2:1):(Goal?6:1);
 FUIFixture F(P.StartsWith(TEXT("B")));Access::SetPiecePresentation(*F.Mode,false);
 auto* Actor=F.Attacker();auto* Other=F.Defender();
 auto* S=Actor->GetPlayerMatchScreen();auto* W=Other->GetPlayerMatchScreen();
 F.Entropy->Word=8;S->RequestRollTacticalPoints();F.Settle();
 TestFalse(TEXT("Type D6 stays Legacy"),Shown(S,TEXT("ResolutionTheater")));
 F.Entropy->Word=5;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);
 for(auto* Screen:{S,W})
 {
  Screen->PauseInlineFormulaRevealTimerForTesting();
  TestFalse(TEXT("Future Penalty cannot bypass Type hold"),Shown(Screen,TEXT("ResolutionTheater")));
 }
 F.Settle();
 for(auto* Screen:{S,W})TestTrue(TEXT("Both viewers enter Theater after disclosure"),Shown(Screen,TEXT("ResolutionTheater")));
 TestEqual(TEXT("Penalty title"),Text(S,TEXT("TheaterTitle")),FString(TEXT("点球")));
 TestEqual(TEXT("No selection subtitle"),Text(S,TEXT("TheaterSubtitle")),FString(TEXT("选择主罚球员")));
 TestEqual(TEXT("Selection names the modified goalkeeper attribute"),Text(S,TEXT("TheaterDetail")),FString(TEXT("常规点球：取射门 / 传球较高值，对抗门将预判（门将预判 -3）")));
 TestEqual(TEXT("Panenka explains one die"),Text(S,TEXT("TheaterReasonSecondary")),FString(TEXT("勺子点球：掷一枚骰子；1 射失，2–6 进球")));
 auto* RuleA=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterDetail")));
 auto* RuleB=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonSecondary")));
 TestTrue(TEXT("Selection rules have peer fonts"),RuleA->GetFont()==RuleB->GetFont());
 TestTrue(TEXT("Selection rules have peer brightness"),RuleA->GetColorAndOpacity()==RuleB->GetColorAndOpacity());
 TestTrue(TEXT("Selection rules share the same text column"),RuleA->GetParent()==RuleB->GetParent());
 auto* Rack=CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));
 TestEqual(TEXT("Candidate pool comes from authority"),Rack->GetPresentation().Cells.Num(),Actor->GetOwnerView().SetPiece.TakerOptions.Num());
 TestTrue(TEXT("No fabricated per-method eligibility"),Actor->GetOwnerView().SetPiece.NearTakerEligibility.IsEmpty());
 TestFalse(TEXT("Waiting viewer has no candidate controls"),Shown(W,TEXT("TheaterTakerBounds")));
 TestFalse(TEXT("No confirm before draft"),Shown(S,TEXT("TheaterPrimaryBounds")));
 const FName Taker=Eligible(F);
 Rack->OnCardSelectionRequested.Broadcast(Taker);
 const int32 Sends=F.Backend(Actor).Sends;
 TestTrue(TEXT("Taker draft has not mutated authority"),Actor->GetOwnerView().SetPiece.TakerCardId.IsNone());
 TestTrue(TEXT("Selected subtitle owns identity"),Text(S,TEXT("TheaterSubtitle")).StartsWith(TEXT("已选主罚球员：")));
 TestFalse(TEXT("Rule bar is not selected status"),Text(S,TEXT("TheaterDetail")).Contains(TEXT("已选")));
 auto* Draft=Rack->GetRenderedCardWidgets().FindByPredicate([Taker](const auto& Card){return Card->GetPresentation().CardId==Taker;})->Get();
 TestTrue(TEXT("Planning hover reuses Full Card"),Draft->RequestFullCardDetailHover());
 TestTrue(TEXT("Full Card visible during planning"),Shown(S,TEXT("TheaterTakerFullCard")));
 Click(S,TEXT("TheaterContinue"));
 TestEqual(TEXT("One explicit confirmation intent"),F.Backend(Actor).Sends,Sends+1);
 TestFalse(TEXT("Method screen clears inspector"),Shown(S,TEXT("TheaterTakerInspector")));
 TestEqual(TEXT("Direct method wording"),Text(S,TEXT("TheaterNearDirectLabel")),FString(TEXT("常规点球")));
 TestEqual(TEXT("Panenka method wording"),Text(S,TEXT("TheaterNearCombinationLabel")),FString(TEXT("勺子点球")));
 TestEqual(TEXT("Method explains the same -3 attribute adjustment"),Text(S,TEXT("TheaterNearDirectHint")),FString(TEXT("取射门 / 传球较高值\n对抗门将预判（门将预判 -3）")));
 TestTrue(TEXT("Method hints remain peer typography"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearDirectHint")))->GetFont()==CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearCombinationHint")))->GetFont());
 TestEqual(TEXT("Direct icon depicts a penalty spot shot"),CastChecked<UFMCodexMatchFlowDiagram>(S->GetWidgetFromName(TEXT("TheaterNearDirectDiagram")))->GetDiagram(),EFMCodexFlowDiagram::PenaltyDirect);
 TestEqual(TEXT("Panenka icon depicts a central chip"),CastChecked<UFMCodexMatchFlowDiagram>(S->GetWidgetFromName(TEXT("TheaterNearCombinationDiagram")))->GetDiagram(),EFMCodexFlowDiagram::PenaltyChip);
 TestTrue(TEXT("Peer title typography"),CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearDirectLabel")))->GetFont()==CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterNearCombinationLabel")))->GetFont());
 const FString TakerName=Actor->GetOwnerView().SetPiece.TakerLabel.ToString();
 Click(S,Chip?TEXT("TheaterNearCombination"):TEXT("TheaterNearDirect"));
 if(!TestEqual(TEXT("Existing typed method adapter accepted"),F.Backend(Actor).LastCode,Code::Accepted))return false;
 for(auto* Screen:{S,W})
 {
  TestEqual(TEXT("Known taker continuous"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
  TestEqual(TEXT("Only Direct has defending participant"),Screen->GetPresentation().InlineFormula.DefenseRow.Participants.Num(),Chip?0:1);
  TestEqual(TEXT("No fake Panenka defender"),Shown(Screen,TEXT("TheaterDefensePanelBounds")),!Chip);
  TestFalse(TEXT("No execution inspector"),Shown(Screen,TEXT("TheaterTakerInspector")));
 }
 if(!Chip)
 {
  FFMCodexLocalMatchViewerDisclosure Disclosure;
  Disclosure.bRevealInitialActionPointRoll=true;Disclosure.bRevealSetPieceTypeRoll=true;
  const auto Safe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(Access::Session(*F.Mode).GetStateSnapshot(),
   Access::CallerRules(*F.Mode),Actor->GetOwnerView().ViewerSide,Disclosure);
  TestFalse(TEXT("Direct projects the canonical goalkeeper ID"),Safe.PenaltyFormulaGoalkeeperCardId.IsNone());
  TestTrue(TEXT("Type-hidden view withholds goalkeeper ID"),Access::Safe(*F.Mode,Actor->GetOwnerView().ViewerSide,true).PenaltyFormulaGoalkeeperCardId.IsNone());
  TestTrue(TEXT("Derived base explains max attribute"),Tip(S,TEXT("TheaterAttackBaseHover")).Contains(TEXT("取较高")));
  const FString DefenseTip=Tip(S,TEXT("TheaterDefenseBaseHover"));
  TestTrue(TEXT("Derived defense base explains anticipation and exact -3"),DefenseTip.Contains(TEXT("预判"))&&DefenseTip.Contains(TEXT("点球防守调整 -3")));
  TestEqual(TEXT("Rendered defense base consumes safe subtotal"),Text(S,TEXT("TheaterDefenseNumber")),FText::AsNumber(Safe.SetPieceDefenseKnownSubtotal).ToString());
 }
 else
 {
  TestEqual(TEXT("Panenka has exactly one projected roll term"),S->GetPresentation().InlineFormula.AttackRow.Terms.Num(),1);
  auto* Operands=CastChecked<UHorizontalBox>(S->GetWidgetFromName(TEXT("TheaterPair")));
  for(int32 I=1;I<Operands->GetChildrenCount();++I)TestEqual(TEXT("No fake arithmetic or second operand"),Operands->GetChildAt(I)->GetVisibility(),ESlateVisibility::Collapsed);
 }
 const auto Before=S->GetPresentation().Header;
 auto RollAndObserve=[&](AFMCodexNetworkMatchPlayerController* PC,bool DefenseTurn,int32 Die)
 {
  const FString SettledTotal=Text(S,TEXT("TheaterAttackFinalNumber"));
  F.Entropy->PendingWords={uint32(Die-1)};
  Click(PC->GetPlayerMatchScreen(),TEXT("TheaterContinue"));
  if(!TestEqual(TEXT("Roll uses existing intent"),F.Backend(PC).LastCode,Code::Accepted))return false;
  for(auto* Screen:{S,W})
  {
   Screen->PauseInlineFormulaRevealTimerForTesting();
   const auto Phase=Screen->GetInlineFormulaRevealPhase();
   Screen->RefreshFromPresentation(Screen->GetPresentation());
   TestEqual(TEXT("Repeated safe view does not restart reveal"),Screen->GetInlineFormulaRevealPhase(),Phase);
   for(int32 I=0;I<100 && Screen->IsInlineFormulaRevealInputBlocked();++I)
   {
    TestFalse(TEXT("No CTA during reveal"),Shown(Screen,TEXT("TheaterPrimaryBounds")));
    if(!Shown(Screen,TEXT("TheaterOutcome")))
    {
     TestEqual(TEXT("No early A score"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Before.PlayerAScoreLabel);
     TestEqual(TEXT("No early B score"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Before.PlayerBScoreLabel);
    }
    TestEqual(TEXT("Duplicate lower roll status stays hidden"),Screen->GetWidgetFromName(TEXT("TheaterStatus"))->GetVisibility(),ESlateVisibility::Hidden);
    if(!Chip)
    {
     TestEqual(TEXT("Attack reel follows reveal owner"),Shown(Screen,TEXT("TheaterAttackReelHost")),!DefenseTurn);
     TestEqual(TEXT("Defense reel follows reveal owner"),Shown(Screen,TEXT("TheaterDefenseReelHost")),DefenseTurn);
     if(DefenseTurn)
     {
      TestEqual(TEXT("Settled attack die never reanimates"),Text(Screen,TEXT("TheaterAttackRollValue")),FString::FromInt(AttackDie));
      TestEqual(TEXT("Settled attack total remains stable"),Text(Screen,TEXT("TheaterAttackFinalNumber")),SettledTotal);
      TestEqual(TEXT("Settled attack remains final"),Text(Screen,TEXT("TheaterAttackValueLabel")),FString(TEXT("最终值")));
     }
    }
    else
    {
     TestTrue(TEXT("Single Panenka operand owns the reel"),Shown(Screen,TEXT("TheaterPairAReelHost")));
     const auto& D=Screen->GetInlineFormulaSurface()->GetPresentation();
     if(D.RevealPhase!=EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
      TestFalse(TEXT("No future Panenka operand disclosure"),D.AttackRow.Terms[0].bResolved);
    }
    Screen->AdvanceInlineFormulaRevealForTesting(.25f);
   }
   TestFalse(TEXT("Reveal completes"),Screen->IsInlineFormulaRevealInputBlocked());
  }
  return true;
 };
 if(!RollAndObserve(Actor,false,AttackDie))return false;
 if(!Chip)
 {
  TestTrue(TEXT("Defender alone gets next CTA"),Shown(W,TEXT("TheaterPrimaryBounds"))&&!Shown(S,TEXT("TheaterPrimaryBounds")));
  if(!RollAndObserve(Other,true,Goal?1:6))return false;
 }
 TestEqual(TEXT("Outcome is canonical"),Actor->GetOwnerView().Terminal.Outcome,Goal?Outcome::Goal:Outcome::NoGoal);
 for(auto* Screen:{S,W})
 {
  TestTrue(TEXT("Outcome visible inside Theater"),Shown(Screen,TEXT("TheaterOutcome")));
  TestTrue(TEXT("Authority reason supplied"),!Screen->GetPresentation().InlineFormula.ResolutionReasonLabel.IsEmpty());
  TestEqual(TEXT("Participant continuous into result"),Text(Screen,TEXT("TheaterAttackName0")),TakerName);
  TestEqual(TEXT("Score reaches final projection with visible Outcome"),Screen->GetMatchHeader()->GetDisplayedScoreLabel(),Screen->GetPresentation().Header.ScoreLabel);
 }
 if(Chip)
 {
  TestEqual(TEXT("Single landed Panenka value"),Text(S,TEXT("TheaterPairARollValue")),FString::FromInt(AttackDie));
  for(auto* Screen:{S,W})
  {
   TestEqual(TEXT("Canonical Panenka headline retained"),Screen->GetPresentation().InlineFormula.NarrativeHeadline,TakerName+(Goal?TEXT("勺子点球命中！"):TEXT("勺子点球未能命中。")));
   TestEqual(TEXT("Panenka reason describes the authoritative outcome"),Text(Screen,TEXT("TheaterDetail")),FString::Printf(TEXT("掷点 %d：%s"),AttackDie,Goal?TEXT("进球"):TEXT("射失")));
   TestEqual(TEXT("Panenka reason names its rule"),Text(Screen,TEXT("TheaterReasonSecondary")),FString(TEXT("勺子点球规则：1 射失，2–6 进球")));
   TestTrue(TEXT("Resolved reason uses the shared primary slot"),Shown(Screen,TEXT("TheaterReasonPrimary")));
   TestEqual(TEXT("Single context header"),Text(Screen,TEXT("TheaterTitle")),FString(TEXT("点球 · 勺子点球")));
  }
 }
 Click(S,TEXT("TheaterContinue"));
 for(auto* Screen:{S,W})TestFalse(TEXT("Next returns to board"),Shown(Screen,TEXT("ResolutionTheater")));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPenaltyTheaterScope,"FMCodex.LocalPlay.ResolutionTheater.Penalty.Scope",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPenaltyTheaterScope::RunTest(const FString&)
{
 using namespace FMCodexResolutionTheaterPrototype;
 auto* Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.Penalty"));
 if(!TestNotNull(TEXT("Development fallback available"),Mode))return false;
 TestEqual(TEXT("Development defaults ON"),Mode->GetInt(),1);
 FFMCodexUMGMatchScreenViewModel P;P.SetPiece.bVisible=true;P.SetPiece.Type=ESetPieceSelectedType::Penalty;
 TestTrue(TEXT("Penalty enters Theater"),WantsTheater(P,{}));
 Mode->Set(0,ECVF_SetByCode);TestFalse(TEXT("Fallback retains old path"),WantsTheater(P,{}));Mode->Set(1,ECVF_SetByCode);
 P.SetPiece.Type=ESetPieceSelectedType::Corner;TestFalse(TEXT("Corner is not migrated"),WantsTheater(P,{}));
 return true;
}
#endif
