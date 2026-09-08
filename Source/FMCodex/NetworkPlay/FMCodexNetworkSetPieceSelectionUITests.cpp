#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkSetPieceSelectionTestFixture.h"
#include "../LocalPlay/FMCodexCardRackWidget.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSetPieceSelectionUI,"FMCodex.NetworkPlay.SetPieceSelection.SharedUI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSetPieceSelectionUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(int32 D6:{1,3,5,6})for(int32 Method:{1,2}){auto P=FString::Printf(TEXT("%s.%d.%d"),S,D6,Method);N.Add(P);C.Add(P);}}
bool FFMCodexSetPieceSelectionUI::RunTest(const FString& P)
{
 using namespace FMCodexSetPieceSelectionTests;
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));const int32 D6=FCString::Atoi(*Parts[1]);const bool Alt=Parts[2]==TEXT("2");FUIFixture F(Parts[0]==TEXT("B"));Access::SetPiecePresentation(*F.Mode);
 auto* PC=F.Attacker();auto* Other=F.Defender();auto* S=PC->GetPlayerMatchScreen();auto* W=Other->GetPlayerMatchScreen();
 const auto Visible=[&](UFMCodexLocalMatchScreenWidget* Screen,const TCHAR* Name)
 {
  const auto* Widget=Screen->GetWidgetFromName(Name);TestNotNull(Name,Widget);
  // A collapsed owner surface hides its retained children after the Free kick handoff.
  for(const UWidget* Current=Widget;Current;Current=Current->GetParent())
   if(Current->GetVisibility()==ESlateVisibility::Collapsed||Current->GetVisibility()==ESlateVisibility::Hidden)return false;
  return Widget!=nullptr;
 };
 F.Entropy->Word=8;S->RequestRollTacticalPoints();if(!TestEqual(TEXT("Full D12 shared action"),F.Backend(PC).LastCode,Code::Accepted))return false;F.Settle();
 TestTrue(TEXT("Type CTA from authority"),S->GetPresentation().SetPiece.bCanRollType);TestFalse(TEXT("Waiting type CTA absent"),W->GetPresentation().SetPiece.bCanRollType);
 const FUnchanged Before(F);F.Entropy->Word=D6-1;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);
 if(!TestEqual(TEXT("Shared type callback generated typed request"),F.Backend(PC).Last.IntentKind,Kind::RequestSetPieceTypeRoll)||!TestEqual(TEXT("Type accepted"),F.Backend(PC).LastCode,Code::Accepted))return false;
 for(auto* Viewer:{PC,Other})
 {
  auto* Screen=Viewer->GetPlayerMatchScreen();Screen->PauseInlineFormulaRevealTimerForTesting();
  TestTrue(TEXT("Both viewers reuse type Reel"),Screen->IsInlineFormulaRevealInputBlocked());
  TestFalse(TEXT("Next type/taker/method panel hidden during reel"),Visible(Screen,TEXT("SetPieceProductionResolutionSurface")));
  const auto Phase=Screen->GetInlineFormulaRevealPhase();Viewer->RefreshPlayerFacingUI();TestEqual(TEXT("Duplicate View does not restart phase"),Screen->GetInlineFormulaRevealPhase(),Phase);
  Screen->AdvanceInlineFormulaRevealForTesting(0.4f);TestTrue(TEXT("Elapsed time cannot skip the reveal"),Screen->IsInlineFormulaRevealInputBlocked());
 }
 F.Settle();
 for(auto* Screen:{S,W})
 {TestFalse(TEXT("Shared Reel and hold completed"),Screen->IsInlineFormulaRevealInputBlocked());TestTrue(TEXT("Existing set-piece surface visible"),Visible(Screen,TEXT("SetPieceProductionResolutionSurface")));}
 const auto Phase=S->GetInlineFormulaRevealPhase();Access::Publish(*F.Mode);TestEqual(TEXT("Repeated accepted type View does not replay completed Reel"),S->GetInlineFormulaRevealPhase(),Phase);
 TestFalse(TEXT("Waiting viewer has method CTA"),Visible(W,TEXT("ShortDirectMethod")));TestFalse(TEXT("Waiting viewer has primary CTA"),Visible(W,TEXT("SetPieceProductionPrimaryAction")));
 if(D6<=2)
 {TestTrue(TEXT("Corner is a clear read-only boundary"),S->GetPresentation().ActionWaitActionText.ToString().Contains(TEXT("暂未开放")));TestFalse(TEXT("No Corner nominations"),Visible(S,TEXT("SetPieceProductionPrimaryAction")));return true;}
 const FName Card=Eligible(F);const int32 Sends=F.Backend(PC).Sends;
 S->DevSetPieceAction(TEXT("SetPieceTaker"),Card);TestEqual(TEXT("Hand click is draft only"),F.Backend(PC).Sends,Sends);TestTrue(TEXT("No early taker adoption"),PC->GetOwnerView().SetPiece.TakerCardId.IsNone());
 TestTrue(TEXT("Explicit confirmation uses existing button"),Visible(S,TEXT("SetPieceProductionPrimaryAction")));
 S->DevSetPieceAction(TEXT("SetPieceConfirm"),NAME_None);if(!TestEqual(TEXT("Confirm typed taker accepted"),F.Backend(PC).LastCode,Code::Accepted))return false;
 TestEqual(TEXT("Exact taker CardId only"),F.Backend(PC).Last.SetPieceCardId,Card);TestEqual(TEXT("Canonical taker frozen"),PC->GetOwnerView().SetPiece.TakerCardId,Card);
 const auto* Status=Cast<UTextBlock>(S->GetWidgetFromName(TEXT("SetPieceProductionStatus")));TestTrue(TEXT("Central current actor explicit"),Status->GetText().ToString().Contains(Parts[0]==TEXT("B")?TEXT("玩家 B"):TEXT("玩家 A")));
 TestFalse(TEXT("No lower Continue duplicates method surface"),S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionContinueButton"))->GetVisibility()!=ESlateVisibility::Collapsed);
 TestEqual(TEXT("Acting dock collapses under central method ownership"),S->GetInteractionPanel()->GetVisibility(),ESlateVisibility::Collapsed);
 TestEqual(TEXT("Waiting dock retains concise actor only"),W->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionActionTitle"))->GetVisibility(),ESlateVisibility::Collapsed);
 const FName Action=D6==5?(Alt?FName(TEXT("NearAngled")):FName(TEXT("NearDirect"))):D6==6?(Alt?FName(TEXT("PenaltyPanenka")):FName(TEXT("PenaltyDirect"))):(Alt?FName(TEXT("LongPower")):FName(TEXT("LongDirect")));
 S->DevSetPieceAction(Action,NAME_None);TestEqual(TEXT("Method typed request"),F.Backend(PC).Last.IntentKind,MethodKind(D6));TestEqual(TEXT("Method accepted"),F.Backend(PC).LastCode,Code::Accepted);
 for(auto* Screen:{S,W})
 {
  TestFalse(TEXT("No decisive-roll CTA"),Visible(Screen,TEXT("SetPieceProductionPrimaryAction")));
  for(const TCHAR* Name:{TEXT("ShortDirectMethod"),TEXT("ShortAngledMethod"),TEXT("LongDirectMethod"),TEXT("LongPowerMethod"),TEXT("PenaltyDirectMethod"),TEXT("PenaltyPanenkaMethod")})TestFalse(TEXT("No stale method CTA"),Visible(Screen,Name));
  const auto* Detail=Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceProductionDetail")));if(D6==6) TestTrue(TEXT("Clear unsupported continuation"),Detail->GetText().ToString().Contains(TEXT("暂未开放")));
  else
  {
   TestFalse(TEXT("Free kick handoff collapses the entire method surface"),Visible(Screen,TEXT("SetPieceProductionResolutionSurface")));
   TestTrue(TEXT("Free kick shared resolution enabled"),Screen->GetPresentation().InlineFormula.bVisible);
  }
 }
 TestEqual(TEXT("Only type consumed entropy"),F.Entropy->Calls,Before.EntropyCalls+1);
 Access::SetPiecePresentation(*F.Mode,false);const auto General=FFMCodexNetworkMatchPresentationAdapter::Read(PC->GetOwnerView(),false);TestEqual(TEXT("Only complete free kicks are enabled generally"),General.SetPiece.bSelectionSupported,D6==3||D6==5);
 return true;
}
#endif
