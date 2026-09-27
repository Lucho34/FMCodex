#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "../NetworkPlay/FMCodexNetworkNearFreeKickTestFixture.h"
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIStyle.h"
#include "Components/Button.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Engine/DataTable.h"
#include "HAL/IConsoleManager.h"

namespace FMCodexCornerTheaterSelectionTests
{
using namespace FMCodexSetPieceSelectionTests;
using namespace FMCodexPlayerFacingOrdinaryUITests;
bool Shown(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{const auto* W=S->GetWidgetFromName(N);return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;}
FString Text(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{if(const auto* Rich=Cast<URichTextBlock>(S->GetWidgetFromName(N)))return Rich->GetText().ToString();return CastChecked<UTextBlock>(S->GetWidgetFromName(N))->GetText().ToString();}
void Click(UFMCodexLocalMatchScreenWidget* S,const TCHAR* N)
{CastChecked<UButton>(S->GetWidgetFromName(N))->OnClicked.Broadcast();}
UFMCodexCardRackWidget* Rack(UFMCodexLocalMatchScreenWidget* S)
{return CastChecked<UFMCodexCardRackWidget>(S->GetWidgetFromName(TEXT("TheaterTakers")));}
FName Inspector(UFMCodexLocalMatchScreenWidget* S)
{return CastChecked<UFMCodexPlayerCardWidget>(S->GetWidgetFromName(TEXT("TheaterTakerFullCard")))->GetPresentation().CardId;}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FCornerTheaterSelection,"FMCodex.LocalPlay.ResolutionTheater.Corner.Selection",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FCornerTheaterSelection::GetTests(TArray<FString>& N,TArray<FString>& C) const
{for(const TCHAR* Side:{TEXT("A"),TEXT("B")}){N.Add(Side);C.Add(Side);}}
bool FCornerTheaterSelection::RunTest(const FString& P)
{
 using namespace FMCodexCornerTheaterSelectionTests;
 FUIFixture F(P==TEXT("B"));Access::SetPiecePresentation(*F.Mode,false);
 auto* A=F.Attacker();auto* D=F.Defender();auto* S=A->GetPlayerMatchScreen();auto* W=D->GetPlayerMatchScreen();
 F.Entropy->Word=8;S->RequestRollTacticalPoints();F.Settle();
 F.Entropy->Word=0;S->DevSetPieceAction(TEXT("SetPieceType"),NAME_None);
 for(auto* Screen:{S,W}){Screen->PauseInlineFormulaRevealTimerForTesting();TestFalse(TEXT("Type reveal owns legacy surface"),Shown(Screen,TEXT("ResolutionTheater")));}
 F.Settle();
 TestTrue(TEXT("Default Corner planning enabled"),FMCodexResolutionTheaterPrototype::IsCornerSelectionEnabled());
 TestTrue(TEXT("Both views enter Theater"),Shown(S,TEXT("ResolutionTheater"))&&Shown(W,TEXT("ResolutionTheater")));
 const auto Options=A->GetOwnerView().SetPiece.CornerOptions;
 if(!TestTrue(TEXT("Complete authoritative pool available"),Options.Num()>3))return false;
 TestEqual(TEXT("Theater preserves entire legal pool"),Rack(S)->GetPresentation().Cells.Num(),Options.Num());
 TestTrue(TEXT("Actor may explicitly choose zero"),Shown(S,TEXT("TheaterPrimaryBounds"))&&Text(S,TEXT("TheaterSelectionHint")).Contains(TEXT("不派进攻")));
 TestEqual(TEXT("Attack zero consequence belongs to attack zero lock"),Text(S,TEXT("TheaterSelectionHint")),FString(TEXT("可不派进攻候选；<Danger>若进攻方锁定 0 人，本次角球直接不进球</>")));
 TestEqual(TEXT("Attack planning attribute summary"),Text(S,TEXT("TheaterDetail")),FString(TEXT("高球看力量，低球看射门")));
 TestEqual(TEXT("Static count and unused-candidate summary"),Text(S,TEXT("TheaterReasonSecondary")),FString(TEXT("候选人数多 1 人最终点数 +2，多 2 人 +3；未选中参与角球球员不消耗")));
 auto* Hint=CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterSelectionHint")));
 const auto* Danger=Hint->GetTextStyleSet()->FindRow<FRichTextStyleRow>(TEXT("Danger"),TEXT("CornerTest"));
 if(!TestNotNull(TEXT("Underfilled semantic span exists"),Danger))return false;
 TestEqual(TEXT("Red span reuses product Danger token"),Danger->TextStyle.ColorAndOpacity.GetSpecifiedColor(),FFMCodexPlayerUIStyle::Get().GetColor(EFMCodexPlayerUIColorRole::Danger));
 TestNotEqual(TEXT("Normal copy keeps its own color"),Hint->GetDefaultTextStyle().ColorAndOpacity.GetSpecifiedColor(),Danger->TextStyle.ColorAndOpacity.GetSpecifiedColor());
 TestTrue(TEXT("Zero count shown"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("0 / 3")));
 TestFalse(TEXT("Wait viewer has no candidates or confirm"),Shown(W,TEXT("TheaterTakerBounds"))||Shown(W,TEXT("TheaterPrimaryBounds")));
 TestFalse(TEXT("Empty inspector"),Shown(S,TEXT("TheaterTakerFullCard")));
 const int32 Sends=F.Backend(A).Sends;
 Click(S,TEXT("TheaterContinue"));
 TestTrue(TEXT("Zero confirmation stays in Theater with back"),Shown(S,TEXT("TheaterSelectionReturnBounds"))&&Shown(S,TEXT("ResolutionTheater")));
 TestTrue(TEXT("Zero confirmation keeps explicit consequence and choice"),Text(S,TEXT("TheaterSelectionHint")).Contains(TEXT("若进攻方锁定 0 人，本次角球直接不进球</>；确认继续锁定，或返回补充")));
 TestEqual(TEXT("Zero first confirm sends nothing"),F.Backend(A).Sends,Sends);
 Click(S,TEXT("TheaterSelectionReturn"));
 auto Select=[&](int32 I){Rack(S)->OnCardSelectionRequested.Broadcast(Options[I]);};
 Select(0);TestTrue(TEXT("One count"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("1 / 3")));
 Click(S,TEXT("TheaterContinue"));
 TestEqual(TEXT("One confirmation colors only the warning phrase"),Text(S,TEXT("TheaterSelectionHint")),FString(TEXT("已选 1/3，<Danger>未选满 3 人</>；确认继续锁定，或返回补充")));
 Click(S,TEXT("TheaterSelectionReturn"));
 TestFalse(TEXT("Return removes confirmation warning"),Text(S,TEXT("TheaterSelectionHint")).Contains(TEXT("<Danger>")));
 Select(1);TestTrue(TEXT("Two count"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("2 / 3")));
 TestEqual(TEXT("Multi focus is most recently selected"),Inspector(S),Options[1]);
 auto* Hover=Rack(S)->GetRenderedCardWidgets()[2].Get();Hover->OnDetailHoverRequested.Broadcast(Hover);
 TestEqual(TEXT("Hover supersedes selected focus"),Inspector(S),Options[2]);
 Hover->OnDetailHoverDismissed.Broadcast(Hover);TestEqual(TEXT("Hover leave restores focus"),Inspector(S),Options[1]);
 Click(S,TEXT("TheaterContinue"));TestTrue(TEXT("Underfull confirms in place"),Shown(S,TEXT("TheaterSelectionReturnBounds")));
 TestEqual(TEXT("Two confirmation colors only the warning phrase"),Text(S,TEXT("TheaterSelectionHint")),FString(TEXT("已选 2/3，<Danger>未选满 3 人</>；确认继续锁定，或返回补充")));
 Select(2);TestTrue(TEXT("Confirmation freezes draft"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("2 / 3")));
 Click(S,TEXT("TheaterSelectionReturn"));Select(2);
 TestTrue(TEXT("Max count"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("3 / 3")));
 TestTrue(TEXT("Full state is ready without underfilled warning"),Text(S,TEXT("TheaterSelectionHint")).Contains(TEXT("已选满 3 人"))&&!Text(S,TEXT("TheaterSelectionHint")).Contains(TEXT("未选满")));
 auto* MaxHover=Rack(S)->GetRenderedCardWidgets()[3].Get();MaxHover->OnDetailHoverRequested.Broadcast(MaxHover);
 TestEqual(TEXT("Unselected candidate remains inspectable at max"),Inspector(S),Options[3]);
 MaxHover->OnDetailHoverDismissed.Broadcast(MaxHover);
 TestFalse(TEXT("Fourth remains visible but not selectable"),Rack(S)->GetPresentation().Cells[3].bSetPieceSelectable);
 Select(3);TestTrue(TEXT("Max callback cannot replace selection"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("3 / 3")));
 Select(1);TestEqual(TEXT("Deselect retains latest remaining focus"),Inspector(S),Options[2]);
 Select(2);TestEqual(TEXT("Deselect latest falls back to remaining"),Inspector(S),Options[0]);
 Select(0);TestFalse(TEXT("Removing all clears inspector"),Shown(S,TEXT("TheaterTakerFullCard")));
 Select(2);Select(0);Select(1);
 TestEqual(TEXT("Click order is explicit"),Rack(S)->GetPresentation().Cells[2].SetPieceSelectionOrder,1);
 TestEqual(TEXT("Draft and inspection never submit"),F.Backend(A).Sends,Sends);
 auto* Mode=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.CornerSelection"));
 Mode->Set(0,ECVF_SetByCode);A->RefreshPlayerFacingUI();TestFalse(TEXT("Development fallback"),Shown(S,TEXT("ResolutionTheater")));
 Mode->Set(1,ECVF_SetByCode);A->RefreshPlayerFacingUI();TestTrue(TEXT("Fallback round trip preserves draft"),Text(S,TEXT("TheaterSubtitle")).Contains(TEXT("3 / 3")));
 Click(S,TEXT("TheaterContinue"));
 TestEqual(TEXT("One typed nomination lock"),F.Backend(A).Sends,Sends+1);
 TestTrue(TEXT("Attack waiting retains its own locked identities"),Text(S,TEXT("TheaterSelectionMessageText")).Contains(TEXT("1. ")));
 TestTrue(TEXT("Defender projection contains no attack nominees"),D->GetOwnerView().SetPiece.CornerAttackers.IsEmpty());
 TestFalse(TEXT("Waiting attacker cannot submit twice"),Shown(S,TEXT("TheaterPrimaryBounds")));
 TestFalse(TEXT("Leaving actor planning clears Full Card"),Shown(S,TEXT("TheaterTakerInspector")));
 TestTrue(TEXT("Defense uses same selection family"),Shown(W,TEXT("TheaterTakerBounds"))&&Text(W,TEXT("TheaterSubtitle")).Contains(TEXT("选择防守")));
 TestEqual(TEXT("Defense zero copy preserves attacker-positive condition"),Text(W,TEXT("TheaterSelectionHint")),FString(TEXT("可不派防守候选；<Danger>若防守方锁定 0 人且进攻方有候选，对方直接进球</>")));
 TestEqual(TEXT("Defense low-ball attribute remains marking"),Text(W,TEXT("TheaterDetail")),FString(TEXT("高球看力量，低球看盯防")));
 Click(W,TEXT("TheaterContinue"));
 TestTrue(TEXT("Defense zero confirmation preserves its own consequence"),Text(W,TEXT("TheaterSelectionHint")).Contains(TEXT("若防守方锁定 0 人且进攻方有候选，对方直接进球</>；确认继续锁定，或返回补充")));
 Click(W,TEXT("TheaterSelectionReturn"));
 const auto DefenseOptions=D->GetOwnerView().SetPiece.CornerOptions;
 Rack(W)->OnCardSelectionRequested.Broadcast(DefenseOptions[0]);Click(W,TEXT("TheaterContinue"));Click(W,TEXT("TheaterContinue"));
 for(auto* PC:{A,D})
 {
  const auto& V=PC->GetOwnerView().SetPiece;auto* Screen=PC->GetPlayerMatchScreen();
  TestTrue(TEXT("Both locked lists publicly handed off"),V.CornerAttackers.Num()==3&&V.CornerDefenders.Num()==1&&V.CornerAttackers[0]==Options[2]);
  TestEqual(TEXT("Stops at shared D6 boundary"),V.CornerStage,EMatchPlaySetPieceCornerRouteStage::AwaitingParticipantSelectionRoll);
  TestEqual(TEXT("No participant roll submitted by confirmation"),V.CornerSharedD6,0);
  TestTrue(TEXT("Accepted planning continues into resolution Theater"),Shown(Screen,TEXT("ResolutionTheater")));
  TestFalse(TEXT("No Full Card beyond planning"),Shown(Screen,TEXT("TheaterTakerInspector")));
  TestTrue(TEXT("Public ordered lists move to Theater draw"),Shown(Screen,TEXT("TheaterParticipantDraw")));
 }
 return true;
}
#endif
