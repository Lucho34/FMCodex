#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkNearFreeKickTestFixture.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexRollReelWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "HAL/IConsoleManager.h"
namespace FMCodexCornerTheaterResolutionTests
{
using namespace FMCodexSetPieceSelectionTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
bool Shown(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{const auto* W=S->GetWidgetFromName(N);return W&&W->GetVisibility()!=ESlateVisibility::Collapsed&&W->GetVisibility()!=ESlateVisibility::Hidden;}
FString Text(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{if(auto* R=Cast<URichTextBlock>(S->GetWidgetFromName(N)))return R->GetText().ToString();return CastChecked<UTextBlock>(S->GetWidgetFromName(N))->GetText().ToString();}
void Click(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{CastChecked<UButton>(S->GetWidgetFromName(N))->OnClicked.Broadcast();}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerResolution,"FMCodex.LocalPlay.ResolutionTheater.Corner.Resolution",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerResolution::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* P:{TEXT("A.HighRapidGoal"),TEXT("A.LowRapidGoal"),TEXT("B.LowSwitchRapidNoGoal"),TEXT("A.HighKeeperTie"),TEXT("A.HighNormalGoal"),TEXT("B.LowNormalNoGoal"),TEXT("A.AttackZero"),TEXT("B.DefenseZero")}){N.Add(P);C.Add(P);}}
bool FCornerResolution::RunTest(const FString& P)
{
 const bool Tie=P.Contains(TEXT("Tie")),Normal=P.Contains(TEXT("Normal"));
 const bool Win=!P.Contains(TEXT("NoGoal"))&&!P.Contains(TEXT("AttackZero"))&&!Tie;
 const bool Low=P.Contains(TEXT("Low")),ZeroA=P.Contains(TEXT("AttackZero")),ZeroD=P.Contains(TEXT("DefenseZero"));
 FUIFixture F(P.StartsWith(TEXT("B")));Access::SetPiecePresentation(*F.Mode,false);
 auto* A=F.Attacker();auto* D=F.Defender();auto* S=A->GetPlayerMatchScreen();auto* W=D->GetPlayerMatchScreen();
 F.Entropy->Word=8;S->RequestRollTacticalPoints();F.Settle();F.Entropy->Word=0;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);F.Settle();
 auto Lock=[&](auto* PC,int32 Count)
 {
  auto* Screen=PC->GetPlayerMatchScreen();const auto Options=PC->GetOwnerView().SetPiece.CornerOptions;
  for(int32 I=0;I<Count;++I)CastChecked<UFMCodexCardRackWidget>(Screen->GetWidgetFromName(TEXT("TheaterTakers")))->OnCardSelectionRequested.Broadcast(Options[I]);
  Click(Screen,TEXT("TheaterContinue"));if(Count<3)Click(Screen,TEXT("TheaterContinue"));
  TestEqual(TEXT("Real typed lock accepted"),F.Backend(PC).LastCode,Code::Accepted);
 };
 Lock(A,ZeroA?0:Low?1:3);F.Entropy->Word=3;Lock(D,ZeroD?0:Low||Tie?3:1);
 if(!ZeroA&&!ZeroD)
 {
  TestTrue(TEXT("Both viewers stay in Theater draw"),Shown(S,TEXT("TheaterParticipantDraw"))&&Shown(W,TEXT("TheaterParticipantDraw")));
  TestFalse(TEXT("No wait viewer draw CTA"),Shown(W,TEXT("TheaterPrimaryBounds")));
  TestEqual(TEXT("Three-person range belongs to the candidate"),Text(Low?W:S,Low?TEXT("TheaterDrawDefense0Range"):TEXT("TheaterDrawAttack0Range")),FString(TEXT("1号位\n掷点 1–2")));
  TestTrue(TEXT("Mapping explains one die and locked positions"),Text(S,TEXT("TheaterDetail")).Contains(TEXT("同一掷点确定双方实际球员；号位表示锁定顺序")));
  TestEqual(TEXT("Single candidate keeps full D6 range"),Text(S,Low?TEXT("TheaterDrawAttack0Range"):Tie?TEXT("TheaterDrawAttack0Range"):TEXT("TheaterDrawDefense0Range")),FString(Tie?TEXT("1号位\n掷点 1–2"):TEXT("1号位\n掷点 1–6")));
  auto* Gate=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.CornerResolution"));Gate->Set(0,ECVF_SetByCode);A->RefreshPlayerFacingUI();
  TestFalse(TEXT("Resolution fallback restores legacy"),Shown(S,TEXT("ResolutionTheater")));Gate->Set(1,ECVF_SetByCode);A->RefreshPlayerFacingUI();
  auto Roll=[&](auto* PC,Kind K,int32 D6)
  {
   const auto Score=S->GetMatchHeader()->GetPresentation();F.Entropy->Word=D6-1;Click(PC->GetPlayerMatchScreen(),TEXT("TheaterContinue"));
   TestEqual(TEXT("Theater forwards typed command"),F.Backend(PC).Last.IntentKind,K);TestEqual(TEXT("Typed command accepted"),F.Backend(PC).LastCode,Code::Accepted);
   for(auto* Screen:{S,W})
   {
    Screen->PauseInlineFormulaRevealTimerForTesting();TestTrue(TEXT("Both viewers reveal"),Screen->IsInlineFormulaRevealInputBlocked());
    TestFalse(TEXT("No early Outcome"),Shown(Screen,TEXT("TheaterOutcome")));
    TestEqual(TEXT("Score A gated"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Score.PlayerAScoreLabel);
    TestEqual(TEXT("Score B gated"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Score.PlayerBScoreLabel);
    if(K==Kind::RequestCornerParticipantSelectionRoll)
    {
     TestTrue(TEXT("Single standalone draw"),Shown(Screen,TEXT("TheaterDrawReelHost"))&&!Shown(Screen,TEXT("TheaterRoll")));
     TestFalse(TEXT("Future method stays hidden"),Shown(Screen,TEXT("TheaterChoices")));
     TestEqual(TEXT("Future selection remains un-emphasized"),Screen->GetWidgetFromName(TEXT("TheaterDrawAttack0Frame"))->GetRenderOpacity(),1.f);
    }
    if(K==Kind::RequestCornerRouteRoll)
    {TestFalse(TEXT("No future formula during route"),Shown(Screen,TEXT("TheaterAttackValue")));TestTrue(TEXT("Known identities persist through route"),Shown(Screen,TEXT("TheaterDefensePeople")));}
   }
   const auto Phase=S->GetInlineFormulaRevealPhase();A->RefreshPlayerFacingUI();TestEqual(TEXT("Repeated safe View does not restart motion"),S->GetInlineFormulaRevealPhase(),Phase);
   if(K==Kind::RequestCornerParticipantSelectionRoll && Tie)
   {
    // Step the existing clock to first disclosed highlight, then prove it survives
    // the old 1.27-second readable window on both independent viewer surfaces.
    for(auto* Screen:{S,W})
    {
     for(int32 I=0;I<40 && Screen->GetInlineFormulaSurface()->GetPresentation().RouteResultLabel.IsEmpty();++I)
      Screen->AdvanceInlineFormulaRevealForTesting(.1f);
     TestTrue(TEXT("Participant result was disclosed"),!Screen->GetInlineFormulaSurface()->GetPresentation().RouteResultLabel.IsEmpty());
     Screen->AdvanceInlineFormulaRevealForTesting(2.2f);
     TestTrue(TEXT("Participant highlight readable for at least 2.2 seconds"),Shown(Screen,TEXT("TheaterParticipantDraw"))&&Screen->IsInlineFormulaRevealInputBlocked());
     const auto Held=Screen->GetInlineFormulaRevealPhase();A->RefreshPlayerFacingUI();
     TestEqual(TEXT("Repeated View preserves extended hold"),Screen->GetInlineFormulaRevealPhase(),Held);
     Screen->AdvanceInlineFormulaRevealForTesting(.3f);
     TestFalse(TEXT("Shared reveal exits after 2.4 readable seconds"),Screen->IsInlineFormulaRevealInputBlocked());
    }
   }
   F.Settle();
  };
  Roll(A,Kind::RequestCornerParticipantSelectionRoll,Tie?1:4);
  TestTrue(TEXT("Both selected actual identities"),!S->GetPresentation().SetPiece.CornerRunner.IsNone()&&!S->GetPresentation().SetPiece.CornerHelper.IsNone());
  TestTrue(TEXT("Shared choice family shown"),Shown(S,TEXT("TheaterChoices")));
  TestFalse(TEXT("Corner route choice removes information bar allocation"),S->GetWidgetFromName(TEXT("TheaterInfoBar"))->GetParent()->GetVisibility()!=ESlateVisibility::Collapsed);
  TestFalse(TEXT("Old method choice family is hidden"),Shown(S,TEXT("TheaterNearMethods")));
  TestEqual(TEXT("High route button"),Text(S,TEXT("TheaterHighLabel")),FString(TEXT("高球")));
  TestEqual(TEXT("Low route button"),Text(S,TEXT("TheaterLowLabel")),FString(TEXT("低球")));
  TestFalse(TEXT("Wait viewer cannot choose"),CastChecked<UButton>(W->GetWidgetFromName(TEXT("TheaterHigh")))->GetIsEnabled());
  TestEqual(TEXT("Goalkeeper explicitly participates"),Text(S,TEXT("TheaterDefenseRole1")),FString(TEXT("门将")));
  // Low case intentionally chooses High then switches, testing authority route rather than UI inference.
  Click(S,Low&&Normal?TEXT("TheaterLow"):TEXT("TheaterHigh"));TestEqual(TEXT("Typed route intent"),F.Backend(A).Last.IntentKind,Kind::SubmitCornerIntent);
  TestTrue(TEXT("Route mapping before roll"),Text(S,TEXT("TheaterDetail")).Contains(TEXT("1–4")));
  Roll(A,Kind::RequestCornerRouteRoll,Low&&!Normal?6:1);
  TestEqual(TEXT("Authority chose expected route"),S->GetPresentation().SetPiece.CornerActualRoute,Low?EMatchPlayCornerRouteIntent::Low:EMatchPlayCornerRouteIntent::High);
  const auto& Formula=S->GetInlineFormulaSurface()->GetPresentation();
  TestEqual(TEXT("Route-specific Formula title"),Text(S,TEXT("TheaterTitle")),FString(Low?TEXT("角球 · 低球"):TEXT("角球 · 高球")));
  TestFalse(TEXT("No old Low terminology in displayed route hint"),Formula.RollHelperLabel.Contains(TEXT("低平球")));
  TestEqual(TEXT("One real attacker"),Formula.AttackRow.Participants.Num(),1);TestEqual(TEXT("Helper and keeper"),Formula.DefenseRow.Participants.Num(),2);
  const auto* Tip=CastChecked<UBorder>(S->GetWidgetFromName(TEXT("TheaterDefenseBaseHover")));
  TestTrue(TEXT("Fixed defense modifier explained"),CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Tip->GetToolTip())->GetContent())->GetContent())->GetText().ToString().Contains(TEXT("防守加成 2")));
  if(!Tie)TestTrue(TEXT("Actual count modifier explained"),Text(S,TEXT("TheaterReasonSecondary")).Contains(TEXT("+3")));
  TestTrue(TEXT("Route-specific defense attributes"),CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Tip->GetToolTip())->GetContent())->GetContent())->GetText().ToString().Contains(Low?TEXT("门将反应"):TEXT("门将制空")));
  int32 AttackD6=Win?6:1,DefenseD6=Win?1:6;
  if(Tie||Normal)
  {
   bool Found=false;
   for(int32 X=1;X<=6&&!Found;++X)for(int32 Y=1;Y<=6&&!Found;++Y)
   {
    if((X==6&&Y<=2)||(Y==6&&X<=2))continue;
    const float AX=Formula.AttackRow.KnownNonRollSubtotal+X,DY=Formula.DefenseRow.KnownNonRollSubtotal+Y;
    if(Tie?AX==DY:Win?AX>DY:AX<DY){AttackD6=X;DefenseD6=Y;Found=true;}
   }
   if(!TestTrue(TEXT("Fixture can represent canonical requested comparison"),Found))return false;
  }
  Roll(A,Kind::RequestCornerAttackRoll,AttackD6);
  const FString Settled=Text(S,TEXT("TheaterAttackRollValue"));
  F.Entropy->Word=DefenseD6-1;Click(W,TEXT("TheaterContinue"));
  for(auto* Screen:{S,W})
  {
   Screen->PauseInlineFormulaRevealTimerForTesting();TestEqual(TEXT("Settled attack stays static during defense"),Text(Screen,TEXT("TheaterAttackRollValue")),Settled);
   TestFalse(TEXT("Attack reel does not reanimate"),Shown(Screen,TEXT("TheaterAttackReelHost")));
   TestTrue(TEXT("Defense uses inline operand"),Shown(Screen,TEXT("TheaterDefenseReelHost")));
   TestFalse(TEXT("Outcome waits for final hold"),Shown(Screen,TEXT("TheaterOutcome")));
  }
  F.Settle();TestTrue(TEXT("Projected authoritative win reason"),Text(S,TEXT("TheaterReasonPrimary")).Contains(Tie?TEXT("门将"):Normal?TEXT("最终"):TEXT("压制")));
  const auto& Final=S->GetInlineFormulaSurface()->GetPresentation();
  const FString Name=Final.AttackRow.Participants[0].PlayerName;
  TestEqual(TEXT("Natural Corner narrative follows authoritative result"),Final.ContestLabel,
   Name+(Low?TEXT("接角球低球攻门"):TEXT("接角球高球攻门"))+(Win?TEXT("得分！"):TEXT("未能得分。")));
  TestEqual(TEXT("Wire keeps its existing authored text"),S->GetPresentation().InlineFormula.OutcomeText.Keyword.ToString(),FString(Win?TEXT("破门"):TEXT("未能得分")));
 }
 for(auto* Screen:{S,W})
 {
  TestTrue(TEXT("Production Outcome owns terminal"),Shown(Screen,TEXT("ResolutionTheater"))&&Shown(Screen,TEXT("TheaterOutcome")));
  TestTrue(TEXT("Canonical reason visible"),!Text(Screen,TEXT("TheaterReasonPrimary")).IsEmpty());
  TestFalse(TEXT("Full Card remains planning-only"),Shown(Screen,TEXT("TheaterTakerInspector")));
  if(ZeroA||ZeroD){TestFalse(TEXT("Zero branch has no formula or reel"),Shown(Screen,TEXT("TheaterAttackValue"))||Screen->IsInlineFormulaRevealInputBlocked());TestFalse(TEXT("No fabricated defender"),Shown(Screen,TEXT("TheaterDefensePanelBounds")));}
 }
 TestEqual(TEXT("Authoritative result"),A->GetOwnerView().Terminal.Outcome,Win?EFMCodexNetworkTerminalOutcome::Goal:EFMCodexNetworkTerminalOutcome::NoGoal);
 Click(S,TEXT("TheaterContinue"));TestEqual(TEXT("Advance through shared lifecycle"),F.Backend(A).Last.IntentKind,Kind::AdvanceAfterTerminal);
 TestFalse(TEXT("Return to board"),Shown(S,TEXT("ResolutionTheater")));return true;
}
}
#endif
