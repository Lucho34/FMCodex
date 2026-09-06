#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
#include "FMCodexNetworkSpecializedShotTestFixture.h"
#include "../LocalPlay/FMCodexLongShotResolutionSurfaceWidget.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexSpecializedShotPlayerUI,"FMCodex.NetworkPlay.PlayerFacingSpecializedShotUI.GoldenPaths",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexSpecializedShotPlayerUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* Family:{TEXT("LongShot"),TEXT("CutInside")})
 for(const TCHAR* Path:{TEXT("Immediate1"),TEXT("Immediate2"),TEXT("DirectGoal"),TEXT("DirectMiss"),TEXT("DeadCornerGoal"),TEXT("DeadCornerMiss")})for(const TCHAR* End:{TEXT("Next"),TEXT("Final")})
 {auto P=FString::Printf(TEXT("%s.%s.%s.%s"),S,Family,Path,End);N.Add(P);C.Add(P);}
}
bool FFMCodexSpecializedShotPlayerUI::RunTest(const FString& P)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 using namespace FMCodexSpecializedShotTests;
 const bool B=P.StartsWith(TEXT("B")),Cut=P.Contains(TEXT("CutInside")),Dead=P.Contains(TEXT("DeadCorner")),Goal=P.Contains(TEXT("Goal")),Immediate=P.Contains(TEXT("Immediate")),Final=P.EndsWith(TEXT("Final"));
 const auto Family=Cut?ESkillRuleType::CutInsideShot:ESkillRuleType::LongShot;
 FUIFixture F(B!=Final,Final);if(!TestTrue(TEXT("Canonical setup stops at genuine skill choice"),F.SkillFixture(Final,Family)))return false;
 auto* Actor=F.Attacker();auto* Defense=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* D=Defense->GetPlayerMatchScreen();
 CheckBothPrompts(*this,F,TEXT("选择战术"));
 const auto* Skill=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([&](const auto& O){return O.SkillType==Family&&O.bEnabled;});
 if(!TestNotNull(TEXT("Complete shot capability enabled"),Skill))return false;
 S->RequestSubmitSkill(Skill->OptionId);TestEqual(TEXT("Shared skill callback accepted"),F.Backend(Actor).LastCode,Code::Accepted);
 CheckBothPrompts(*this,F,TEXT("选择射门方式"));
 // Reconstruct pending/rejected presentation from the same safe snapshot. No gameplay mutation.
 for(auto* PC:{Actor,Defense})
 {
  auto* Screen=PC->GetPlayerMatchScreen();const auto Snapshot=PC->GetOwnerView();
  Screen->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Snapshot,true));
  CheckPrompt(*this,Screen,Snapshot,TEXT("选择射门方式"),true);
  TestEqual(TEXT("Pending never gives central branch CTA"),Screen->GetLongShotResolutionSurface()->GetBranchChoiceWidgets().Num(),0);
  auto Rejected=FFMCodexNetworkMatchPresentationAdapter::Read(Snapshot,false);
  Rejected.Resolution.bRejected=true;Screen->RefreshFromPresentation(Rejected);
  CheckPrompt(*this,Screen,Snapshot,TEXT("选择射门方式"));
  TestEqual(TEXT("Rejection returns ownership from production modal"),Screen->GetLongShotResolutionSurface()->GetVisibility(),ESlateVisibility::Collapsed);
  PC->RefreshPlayerFacingUI();
 }
 CheckBothPrompts(*this,F,TEXT("选择射门方式"));
 TestEqual(TEXT("Original branch surface offers both legal choices"),S->GetLongShotResolutionSurface()->GetBranchChoiceWidgets().Num(),2);
 TestEqual(TEXT("Waiting branch viewer has no buttons"),D->GetLongShotResolutionSurface()->GetBranchChoiceWidgets().Num(),0);
 const int32 Draws=F.Entropy->Calls;
 S->RequestSubmitBranchIntent(Dead?EFMCodexUMGBranchIntent::DeadCorner:EFMCodexUMGBranchIntent::DirectShot);
 TestEqual(TEXT("Typed branch callback accepted"),F.Backend(Actor).LastCode,Code::Accepted);
 TestEqual(TEXT("Intent-determined shot branch draws no route die"),F.Entropy->Calls,Draws);
 const auto BeforeRoll=Actor->GetOwnerView();
 CheckBothPrompts(*this,F,Dead?TEXT("进攻方掷两枚骰"):TEXT("进攻方掷点"));
 auto Roll=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 First,int32 Second=1)
 {
  F.Entropy->PendingWords={uint32(First-1),uint32(Second-1)};F.Entropy->Word=First-1;
  const auto H1=S->GetMatchHeader()->GetPresentation(),H2=D->GetMatchHeader()->GetPresentation();
  if(!Continue(*this,F,PC,K))return false;F.Entropy->PendingWords.Reset();
  const int32 Sends=F.Backend(PC).Sends;PC->GetPlayerMatchScreen()->RequestContinueResolution();TestEqual(TEXT("Reel blocks duplicate callback"),F.Backend(PC).Sends,Sends);
  for(auto* Screen:{S,D})
  {
   TestTrue(TEXT("Original Reel owns both viewers"),Screen->IsInlineFormulaRevealInputBlocked());
   const auto* Prompt=Cast<UTextBlock>(Screen->GetLongShotResolutionSurface()->GetWidgetFromName(TEXT("CentralActionPrompt")));
   TestEqual(TEXT("During Reel actor follows disclosed roll; no premature terminal or next action prompt"),
    Prompt->GetText().ToString(),PC->GetOwnerView().ViewerSide==Side::PlayerA?FString(TEXT("当前操作：玩家 A")):FString(TEXT("当前操作：玩家 B")));
   TestEqual(TEXT("Reel retains original hidden dock gate"),Screen->GetInteractionPanel()->GetVisibility(),ESlateVisibility::Collapsed);
   const auto& Before=Screen==S?H1:H2;
   TestEqual(TEXT("No early A score flash"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Before.PlayerAScoreLabel);
   TestEqual(TEXT("No early B score flash"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Before.PlayerBScoreLabel);
  }
  F.Settle();const auto Phase=S->GetInlineFormulaRevealPhase();Actor->RefreshPlayerFacingUI();Defense->RefreshPlayerFacingUI();
  TestEqual(TEXT("Repeated View cannot replay events"),S->GetInlineFormulaRevealPhase(),Phase);return true;
 };
 const int32 Attack=Immediate?(P.Contains(TEXT("Immediate2"))?2:1):Goal?6:Dead?1:3;
 if(!Roll(Actor,Dead?PairKind(Cut):AttackKind(Cut),Attack,Goal?5:1))return false;
 if(!Dead&&!Immediate)
 {
  TestFalse(TEXT("Attack alone cannot resolve Formula"),Actor->GetOwnerView().Contest.bFormulaResolved);
  CheckBothPrompts(*this,F,TEXT("防守方掷点"));if(!Roll(Defense,DefenseKind(Cut),Goal?1:6))return false;
 }
 const auto View=Actor->GetOwnerView();const auto State=Access::Session(*F.Mode).GetStateSnapshot();
 TestTrue(TEXT("Safe terminal can Advance"),View.bCanAdvance);TestEqual(TEXT("Canonical Goal class"),View.Terminal.Outcome==Outcome::Goal,Goal);
 TestEqual(TEXT("Distinct ImmediateMiss"),View.Terminal.Outcome==Outcome::ImmediateMiss,Immediate);
 TestEqual(TEXT("Canonical Formula only in full Direct contest"),View.Contest.bFormulaResolved,!Dead&&!Immediate);
 TestEqual(TEXT("Accepted event count matches actual dice"),View.Presentation.ResolvedRolls.Num(),Immediate?1:2);
 TestEqual(TEXT("No phantom history"),State.GoalHistory.Num(),int32(Goal));
 if(Goal)TestEqual(TEXT("Safe scorer copied from persisted authority"),View.Terminal.Goal.ScorerCardId,State.GoalHistory.Last().ScorerCardId);
 else TestTrue(TEXT("No phantom scorer"),View.Terminal.Goal.ScorerCardId.IsNone());
 CheckBothPrompts(*this,F,TEXT("下一回合"));
 for(auto* Screen:{S,D})
 {
  TestTrue(TEXT("One original player-facing Screen"),Screen->GetClass()==UFMCodexLocalMatchScreenWidget::StaticClass());
  TestEqual(TEXT("Displayed A catches up after reveal"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
  TestEqual(TEXT("Displayed B catches up after reveal"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
  if(Dead)TestFalse(TEXT("Paired outcome has no Formula surface"),Screen->GetLongShotResolutionSurface()->GetFormulaSurface()->GetPresentation().bVisible);
  if(Immediate)TestFalse(TEXT("Canonical immediate gate hides Formula rows but retains Narrative shell"),Screen->GetLongShotResolutionSurface()->GetFormulaSurface()->GetPresentation().bShowFormulaRows);
 }
 if(!Final&&Goal)
 {
  auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());const auto Slate=Late->TakeWidget();
  Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(View,false));TestFalse(TEXT("Late Screen does not replay prior history"),Late->IsInlineFormulaRevealInputBlocked());
  Late->ResetPresentationSession();Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeRoll,false));
  Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(View,false));Late->PauseInlineFormulaRevealTimerForTesting();
  TestTrue(TEXT("Coalesced shot prefix reveals in original queue"),Late->IsInlineFormulaRevealInputBlocked());
  Late->AdvanceInlineFormulaRevealForTesting(30.f);TestFalse(TEXT("Both coalesced dice settle"),Late->IsInlineFormulaRevealInputBlocked());
  CheckPrompt(*this,Late,View,TEXT("下一回合"));
 }
 if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
 TestFalse(TEXT("Advance clears prior shot state"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);
 for(auto* Screen:{S,D}){TestFalse(TEXT("Old shot surface cleared"),Screen->GetPresentation().LongShotResolution.bVisible);TestEqual(TEXT("Shared Full-Time"),Screen->GetPresentation().FullTime.bVisible,Final);}
 if(!Final)
 {
  CheckBothPrompts(*this,F,TEXT("掷战术点"));TestTrue(TEXT("Next attacker is prior defender"),F.Attacker()==Defense);
  TestTrue(TEXT("Canonical non-final Recovery"),Defense->GetOwnerView().Recovery.SourceAttackSequence>0);
  Defense->GetPlayerMatchScreen()->RequestRollTacticalPoints();TestEqual(TEXT("Next Full D12 accepted"),F.Backend(Defense).LastCode,Code::Accepted);
 }
 else TestFalse(TEXT("Final has no next D12"),S->GetPresentation().Interaction.bCanRollTacticalPoints||D->GetPresentation().Interaction.bCanRollTacticalPoints);
 return true;
}
#endif
