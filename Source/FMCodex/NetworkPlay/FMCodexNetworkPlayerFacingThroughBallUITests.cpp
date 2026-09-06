#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
#include "../LocalPlay/FMCodexThroughBallResolutionSurfaceWidget.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexThroughBallPlayerUI,"FMCodex.NetworkPlay.PlayerFacingThroughBallUI.GoldenPaths",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexThroughBallPlayerUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
 for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* Path:{TEXT("BehindOut1"),TEXT("BehindOut2"),TEXT("BehindStop"),TEXT("BehindDirectGoal"),TEXT("BehindDirectMiss"),TEXT("BehindChipGoal"),TEXT("BehindChipMiss"),TEXT("AntiOffside1"),TEXT("AntiOffside5"),TEXT("AntiDirectGoal"),TEXT("AntiChipMiss")})for(const TCHAR* End:{TEXT("Next"),TEXT("Final")})
 {auto P=FString::Printf(TEXT("%s.%s.%s"),S,Path,End);N.Add(P);C.Add(P);}
}
bool FFMCodexThroughBallPlayerUI::RunTest(const FString& P)
{
 using namespace FMCodexPlayerFacingOrdinaryUITests;
 const bool B=P.StartsWith(TEXT("B")),Final=P.EndsWith(TEXT("Final")),Anti=P.Contains(TEXT("Anti")),Direct=P.Contains(TEXT("Direct")),Chip=P.Contains(TEXT("Chip")),Goal=P.Contains(TEXT("Goal")),Out=P.Contains(TEXT("Out")),Stop=P.Contains(TEXT("Stop"));
 FUIFixture F(B!=Final,Final);if(!TestTrue(TEXT("Canonical setup stops before player-owned skill"),F.SkillFixture(Final,ESkillRuleType::ThroughBall)))return false;
 auto* Actor=F.Attacker();auto* Defense=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* D=Defense->GetPlayerMatchScreen();
 CheckBothPrompts(*this,F,TEXT("选择战术"));const auto* Skill=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([](const auto& O){return O.SkillType==ESkillRuleType::ThroughBall&&O.bEnabled;});
 if(!TestNotNull(TEXT("General ThroughBall capability complete"),Skill))return false;S->RequestSubmitSkill(Skill->OptionId);
 TestEqual(TEXT("Shared skill callback accepted"),F.Backend(Actor).LastCode,Code::Accepted);CheckBothPrompts(*this,F,TEXT("掷直塞路线骰"));
 const auto BeforeRoute=Actor->GetOwnerView();
 auto Roll=[&](AFMCodexNetworkMatchPlayerController* PC,Kind K,int32 D6)
 {
  F.Entropy->Word=D6-1;const auto H1=S->GetMatchHeader()->GetPresentation(),H2=D->GetMatchHeader()->GetPresentation();
  if(!Continue(*this,F,PC,K))return false;
  const int32 Sends=F.Backend(PC).Sends;PC->GetPlayerMatchScreen()->RequestContinueResolution();TestEqual(TEXT("Reel suppresses duplicate callback"),F.Backend(PC).Sends,Sends);
  for(auto* Screen:{S,D})
  {
   TestTrue(TEXT("Existing shared Reel owns reveal"),Screen->IsInlineFormulaRevealInputBlocked());
   const auto& Before=Screen==S?H1:H2;TestEqual(TEXT("No early A score flash"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Before.PlayerAScoreLabel);
   TestEqual(TEXT("No early B score flash"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Before.PlayerBScoreLabel);
  }
  F.Settle();auto Phase=S->GetInlineFormulaRevealPhase();Actor->RefreshPlayerFacingUI();Defense->RefreshPlayerFacingUI();
  TestEqual(TEXT("Repeated view never replays accepted event"),S->GetInlineFormulaRevealPhase(),Phase);return true;
 };
 if(!Roll(Actor,Kind::ThroughBallInitialRouteRoll,Anti?5:3))return false;
 CheckBothPrompts(*this,F,Anti?TEXT("掷反越位点数"):TEXT("进攻方掷点"));
 const int32 Primary=Out?(P.Contains(TEXT("Out2"))?2:1):Stop?3:Direct||Chip?6:P.Contains(TEXT("Offside5"))?5:1;
 if(!Roll(Actor,Anti?Kind::ThroughBallAntiOffsideAttackRoll:Kind::ThroughBallBehindDefenseP1AttackRoll,Primary))return false;
 if(!Anti&&!Out)
 {
  CheckBothPrompts(*this,F,TEXT("防守方掷点"));if(!Roll(Defense,Kind::ThroughBallBehindDefenseP1DefenseRoll,Stop?6:1))return false;
 }
 if(Direct||Chip)
 {
  CheckBothPrompts(*this,F,TEXT("选择单刀射门方式"));
  TestEqual(TEXT("Original central choice surface owns both choices"),S->GetThroughBallResolutionSurface()->GetOneOnOneChoiceWidgets().Num(),2);
  TestEqual(TEXT("Other viewer has no choice buttons"),D->GetThroughBallResolutionSurface()->GetOneOnOneChoiceWidgets().Num(),0);
  const int32 Before=S->GetPresentation().ResolvedRolls.Num();const int32 Draws=F.Entropy->Calls;
  S->RequestSubmitOneOnOneChoice(Direct?EFMCodexUMGOneOnOneChoice::DirectShot:EFMCodexUMGOneOnOneChoice::ChipShot);
  TestEqual(TEXT("Typed choice uses shared callback"),F.Backend(Actor).Last.IntentKind,Kind::SubmitThroughBallOneOnOneShotChoice);
  TestEqual(TEXT("Choice accepted"),F.Backend(Actor).LastCode,Code::Accepted);TestEqual(TEXT("Choice uses no RNG"),F.Entropy->Calls,Draws);
  TestEqual(TEXT("Choice preserves earlier accepted events"),S->GetPresentation().ResolvedRolls.Num(),Before);
  CheckBothPrompts(*this,F,Direct?TEXT("进攻方掷点"):TEXT("掷挑射点数"));
  if(!Roll(Actor,Direct?Kind::ThroughBallOneOnOneDirectShotAttackRoll:Kind::ThroughBallOneOnOneChipShotAttackRoll,Goal?6:1))return false;
  if(Direct){CheckBothPrompts(*this,F,TEXT("防守方掷点"));if(!Roll(Defense,Kind::ThroughBallOneOnOneDirectShotDefenseRoll,Goal?1:6))return false;}
 }
 const auto& View=Actor->GetOwnerView();const auto& State=Access::Session(*F.Mode).GetStateSnapshot();
 TestTrue(TEXT("Persisted terminal"),View.bCanAdvance);TestEqual(TEXT("Expected canonical terminal goal class"),View.Terminal.Outcome==EFMCodexNetworkTerminalOutcome::Goal,Goal);
 CheckBothPrompts(*this,F,TEXT("下一回合"));
 for(auto* Screen:{S,D})
 {
  TestTrue(TEXT("One original shared Screen"),Screen->GetClass()==UFMCodexLocalMatchScreenWidget::StaticClass());
  TestEqual(TEXT("Displayed A score catches up after reveal"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
  TestEqual(TEXT("Displayed B score catches up after reveal"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
  TestEqual(TEXT("Terminal offers no stale OneOnOne choice"),Screen->GetThroughBallResolutionSurface()->GetOneOnOneChoiceWidgets().Num(),0);
 }
 TestEqual(TEXT("Network history copied from authority"),View.PublicGoalHistory.Num(),State.GoalHistory.Num());
 if(Goal&&!View.PublicGoalHistory.IsEmpty())TestEqual(TEXT("Runner is authoritative scorer"),View.PublicGoalHistory.Last().ScorerCardId,State.CurrentAttack.SelectedAction.RunnerCardId);

 if(P==TEXT("A.BehindDirectGoal.Next")||P==TEXT("B.AntiChipMiss.Next"))
 {
  auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());const auto Slate=Late->TakeWidget();
  Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(View,false));
  TestFalse(TEXT("Late construction does not replay unobserved history"),Late->IsInlineFormulaRevealInputBlocked());
  CheckPrompt(*this,Late,View,TEXT("下一回合"));
  Late->ResetPresentationSession();Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeRoute,false));
  Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(View,false));Late->PauseInlineFormulaRevealTimerForTesting();
  TestTrue(TEXT("Coalesced accepted prefix begins existing reveal"),Late->IsInlineFormulaRevealInputBlocked());
  if(Goal)TestTrue(TEXT("Coalesced terminal holds painted score"),Late->GetMatchHeader()->GetPresentation().PlayerAScoreLabel!=View.Presentation.Header.PlayerAScoreLabel);
  Late->AdvanceInlineFormulaRevealForTesting(30.f);
  TestFalse(TEXT("Entire conditional and OneOnOne prefix settles"),Late->IsInlineFormulaRevealInputBlocked());
  TestEqual(TEXT("Coalesced reveal ends at safe score"),Late->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,View.Presentation.Header.PlayerAScoreLabel);
  CheckPrompt(*this,Late,View,TEXT("下一回合"));
 }
 const int32 Post=Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount();
 if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
 TestEqual(TEXT("Advance consumes no post-route RNG"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post);
 TestFalse(TEXT("Advance cleared current attack"),Access::Session(*F.Mode).GetStateSnapshot().bHasCurrentAttack);
 for(auto* Screen:{S,D}){const auto& M=Screen->GetPresentation();TestFalse(TEXT("Old ThroughBall surface cleared"),M.ThroughBallResolution.bVisible);TestEqual(TEXT("Canonical Full-Time"),M.FullTime.bVisible,Final);}
 if(!Final)
 {
  CheckBothPrompts(*this,F,TEXT("掷战术点"));TestTrue(TEXT("Next actor is prior defender"),F.Attacker()==Defense);
  TestTrue(TEXT("Canonical Recovery after non-final Advance"),Defense->GetOwnerView().Recovery.SourceAttackSequence>0);
  Defense->GetPlayerMatchScreen()->RequestRollTacticalPoints();TestEqual(TEXT("Next Full D12 accepted"),F.Backend(Defense).LastCode,Code::Accepted);
 }
 else TestFalse(TEXT("Final does not expose extra Full D12"),S->GetPresentation().Interaction.bCanRollTacticalPoints||D->GetPresentation().Interaction.bCanRollTacticalPoints);
 return true;
}
#endif
