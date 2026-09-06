#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkPlayerFacingTestFixture.h"
#include "../LocalPlay/FMCodexLongShotResolutionSurfaceWidget.h"
#include "../LocalPlay/FMCodexThroughBallResolutionSurfaceWidget.h"

namespace FMCodexOrdinaryPlayerUITests
{
using namespace FMCodexPlayerFacingOrdinaryUITests;
const FFMCodexUMGInlineFormulaSurfaceViewModel& Formula(UFMCodexLocalMatchScreenWidget* S,bool Feet)
{
    return Feet?S->GetThroughBallResolutionSurface()->GetFormulaSurface()->GetPresentation()
        :S->GetLongShotResolutionSurface()->GetFormulaSurface()->GetPresentation();
}
}
IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexOrdinaryPlayerUI,"FMCodex.NetworkPlay.PlayerFacingOrdinaryUI.GoldenPaths",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
void FFMCodexOrdinaryPlayerUI::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
    for(const TCHAR* S:{TEXT("A"),TEXT("B")})for(const TCHAR* R:{TEXT("Pass"),TEXT("Dribble"),TEXT("Run"),TEXT("Feet")})
    for(const TCHAR* G:{TEXT("Goal"),TEXT("NoGoal")})for(const TCHAR* End:{TEXT("Next"),TEXT("Final")})
    {auto P=FString::Printf(TEXT("%s.%s.%s.%s"),S,R,G,End);N.Add(P);C.Add(P);}
}
bool FFMCodexOrdinaryPlayerUI::RunTest(const FString& P)
{
    using namespace FMCodexOrdinaryPlayerUITests;
    TArray<FString> Parts;P.ParseIntoArray(Parts,TEXT("."));
    const bool B=Parts[0]==TEXT("B"),Feet=Parts[1]==TEXT("Feet"),Goal=Parts[2]==TEXT("Goal"),Final=Parts[3]==TEXT("Final");
    const auto Family=Feet?ESkillRuleType::ThroughBall:ESkillRuleType::PassControl;
    FUIFixture F(B!=Final,Final);
    if(!TestTrue(TEXT("Canonical fixture ends at genuine Skill wait"),F.SkillFixture(Final,Family)))return false;
    auto* Actor=F.Attacker();auto* Defender=F.Defender();auto* S=Actor->GetPlayerMatchScreen();auto* D=Defender->GetPlayerMatchScreen();
    if(!TestNotNull(TEXT("Shared production screen"),S)||!TestNotNull(TEXT("Shared remote screen"),D))return false;
    TestTrue(TEXT("Both use original shared widget class"),S->GetClass()==UFMCodexLocalMatchScreenWidget::StaticClass()&&D->GetClass()==S->GetClass());
    CheckBothPrompts(*this,F,TEXT("选择战术"));
    const auto* Choice=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([&](const auto& O){return O.SkillType==Family&&O.bEnabled;});
    if(!TestNotNull(TEXT("Supported family has explicit player capability"),Choice))return false;
    S->RequestSubmitSkill(Choice->OptionId);
    TestEqual(TEXT("Existing skill callback accepted"),F.Backend(Actor).LastCode,Code::Accepted);
    CheckBothPrompts(*this,F,Feet?TEXT("掷直塞路线骰"):TEXT("掷传控路线骰"));
    F.Entropy->Word=Parts[1]==TEXT("Dribble")?2:Parts[1]==TEXT("Run")?4:0;
    if(!Continue(*this,F,Actor,Feet?Kind::ThroughBallInitialRouteRoll:Kind::PassControlInitialRouteRoll))return false;
    for(auto* PC:{Actor,Defender})
    {
        TestEqual(TEXT("Only accepted route event"),PC->GetOwnerView().Presentation.ResolvedRolls.Num(),1);
        TestTrue(TEXT("Existing route Reel blocks duplicate input"),PC->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
        const auto Pending=FFMCodexNetworkMatchPresentationAdapter::Read(PC->GetOwnerView(),true);
        TestFalse(TEXT("Pending disables nested Formula CTA"),Pending.LongShotResolution.Formula.PrimaryAction.Action.bAvailable||Pending.ThroughBallResolution.Formula.PrimaryAction.Action.bAvailable);
        const auto& Public=PC->GetOwnerView().Presentation;
        TestTrue(TEXT("Safe unresolved Formula available on both sides"),Feet?Public.ThroughBallSurface.Formula.bVisible:Public.BranchSurface.Formula.bVisible);
    }
    F.Settle();CheckBothPrompts(*this,F,TEXT("进攻方掷点"));
    TestFalse(TEXT("Canonical route label available"),Actor->GetOwnerView().InitialRoute.RouteLabel.IsEmpty());
    const auto RoutePhase=S->GetInlineFormulaRevealPhase();Actor->RefreshPlayerFacingUI();Defender->RefreshPlayerFacingUI();
    TestEqual(TEXT("Repeated revision does not replay route"),S->GetInlineFormulaRevealPhase(),RoutePhase);
    F.Entropy->Word=Goal?5:0;
    if(!Continue(*this,F,Actor,Feet?Kind::ThroughBallFeetAttackRoll:Kind::PassControlAttackRoll))return false;
    for(auto* Screen:{S,D}) TestTrue(TEXT("Attack Reel on both viewers"),Screen->IsInlineFormulaRevealInputBlocked());
    const int32 Sends=F.Backend(Actor).Sends;S->RequestContinueResolution();
    TestEqual(TEXT("Animation duplicate callback suppressed"),F.Backend(Actor).Sends,Sends);
    TestEqual(TEXT("Accepted prefix contains route and Attack"),Actor->GetOwnerView().Presentation.ResolvedRolls.Num(),2);
    F.Settle();CheckBothPrompts(*this,F,TEXT("防守方掷点"));
    const auto BeforeA=S->GetMatchHeader()->GetPresentation(),BeforeB=D->GetMatchHeader()->GetPresentation();
    F.Entropy->Word=Goal?0:5;
    if(!Continue(*this,F,Defender,Feet?Kind::ThroughBallFeetDefenseRoll:Kind::PassControlDefenseRoll))return false;
    TestEqual(TEXT("Three accepted events, no future rolls"),Actor->GetOwnerView().Presentation.ResolvedRolls.Num(),3);
    const auto& State=Access::Session(*F.Mode).GetStateSnapshot();
    TestEqual(TEXT("Public terminal equals authority goal history"),Actor->GetOwnerView().PublicGoalHistory.Num(),State.GoalHistory.Num());
    TestEqual(TEXT("Goal/NoGoal fixture follows canonical formula"),Actor->GetOwnerView().Terminal.Outcome,
        Goal?EFMCodexNetworkTerminalOutcome::Goal:EFMCodexNetworkTerminalOutcome::NoGoal);
    for(auto* Screen:{S,D})
    {
        TestTrue(TEXT("Defense Reel on both viewers"),Screen->IsInlineFormulaRevealInputBlocked());
        TestFalse(TEXT("No early Narrative"),Formula(Screen,Feet).bNarrativeAvailable);
        const auto& Before=Screen==S?BeforeA:BeforeB;
        TestEqual(TEXT("Visible A score held while authoritative/public score arrived"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Before.PlayerAScoreLabel);
        TestEqual(TEXT("Visible B score held while authoritative/public score arrived"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Before.PlayerBScoreLabel);
    }
    F.Settle();CheckBothPrompts(*this,F,TEXT("下一回合"));
    for(auto* Screen:{S,D})
    {
        TestTrue(TEXT("Reused Formula resolved and Narrative disclosed"),Formula(Screen,Feet).bVisible&&Formula(Screen,Feet).bNarrativeAvailable);
        TestEqual(TEXT("Visible A reaches safe score after result"),Screen->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,Screen->GetPresentation().Header.PlayerAScoreLabel);
        TestEqual(TEXT("Visible B reaches safe score after result"),Screen->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,Screen->GetPresentation().Header.PlayerBScoreLabel);
    }
    const auto Settled=S->GetInlineFormulaRevealPhase();Actor->RefreshPlayerFacingUI();Defender->RefreshPlayerFacingUI();
    TestEqual(TEXT("Terminal event deduplicated"),S->GetInlineFormulaRevealPhase(),Settled);
    const int32 Post=Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount();
    if(!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal))return false;
    TestEqual(TEXT("Advance has no extra contest RNG"),Access::Runtime(*F.Mode).GetPostRouteProviderInvocationCount(),Post);
    for(auto* PC:{Actor,Defender})
    {
        const auto& M=PC->GetPlayerMatchScreen()->GetPresentation();
        TestEqual(TEXT("Existing FullTime follows final match"),M.FullTime.bVisible,Final);
        TestFalse(TEXT("Stale Formula removed"),M.ThroughBallResolution.Formula.bVisible||M.LongShotResolution.Formula.bVisible);
        if(Final) TestFalse(TEXT("MatchEnded has no extra D12"),M.Interaction.bCanRollTacticalPoints);
    }
    if(!Final)
    {
        CheckBothPrompts(*this,F,TEXT("掷战术点"));auto* Next=F.Attacker();
        TestTrue(TEXT("Next actor is old defender"),Next==Defender);
        Next->GetPlayerMatchScreen()->RequestRollTacticalPoints();
        TestEqual(TEXT("Next FullD12 uses original screen callback"),F.Backend(Next).Last.IntentKind,Kind::RequestInitialActionPointRoll);
        TestEqual(TEXT("FullD12 accepted"),F.Backend(Next).LastCode,Code::Accepted);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexOrdinaryPlayerCapabilities,"FMCodex.NetworkPlay.PlayerFacingOrdinaryUI.CapabilityBoundary",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFMCodexOrdinaryPlayerCapabilities::RunTest(const FString&)
{
    using namespace FMCodexOrdinaryPlayerUITests;FUIFixture F;
    if(!TestTrue(TEXT("Ordinary PassControl skill fixture"),F.SkillFixture(false,ESkillRuleType::PassControl)))return false;
    const auto V=F.Attacker()->GetOwnerView();
    for(const auto& O:V.Presentation.Interaction.SelectionChoices)
    {
        TestEqual(TEXT("General capability admits complete ordinary tactics"),O.bEnabled,O.SkillType==ESkillRuleType::Cross||O.SkillType==ESkillRuleType::PassControl||O.SkillType==ESkillRuleType::ThroughBall||O.SkillType==ESkillRuleType::LongShot||O.SkillType==ESkillRuleType::CutInsideShot);
        if(O.SkillType==ESkillRuleType::ThroughBall)
        {
            FFMCodexMatchScreenRequest R;R.Kind=ScreenIntent::Skill;R.OptionId=O.OptionId;Envelope E;FFMCodexNetworkIntentClientState Client;
            TestTrue(TEXT("Normal shared screen submits complete ThroughBall"),FFMCodexNetworkMatchScreenActions::Begin(R,V,Client,E));
        }
    }
    return true;
}
#endif
