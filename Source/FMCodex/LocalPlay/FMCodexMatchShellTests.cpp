#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexMatchShellStyle.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"

namespace
{
bool IsShellWidgetVisible(const UWidget* Widget)
{
    return Widget && Widget->GetVisibility() != ESlateVisibility::Collapsed
        && Widget->GetVisibility() != ESlateVisibility::Hidden;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMatchShellHeaderTest,"FMCodex.LocalPlay.MatchShell.HeaderThemeAndResources",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMatchShellHeaderTest::RunTest(const FString&)
{
    auto* Header=NewObject<UFMCodexMatchHeaderWidget>(); Header->TakeWidget();
    FFMCodexLocalMatchInteractionView View;
    View.bMatchActive=true; View.bTacticalPointRollReady=true;
    View.CurrentAttackingPlayer=EInitialTurnOrderPlayer::PlayerA;
    View.PlayerAMaxAttackTurns=3; View.PlayerBMaxAttackTurns=3;
    View.PlayerACurrentAttackIndex=2; View.PlayerAUsedAttackTurns=1; View.bPlayerACurrentAttackTurn=true;
    View.PlayerAScore=2; View.PlayerBScore=1;
    auto Build=[&](EInitialTurnOrderPlayer Viewer){ return FFMCodexLocalMatchUMGPresentationBuilder::Build(View,{},FString(),Viewer).Header; };
    Header->RefreshFromPresentation(Build(EInitialTurnOrderPlayer::PlayerA));
    TestEqual(TEXT("Left identity remains A"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("LeftPlayerIdentityLabel")))->GetText().ToString(),FString(TEXT("玩家 A")));
    TestEqual(TEXT("Right identity remains B"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("RightPlayerIdentityLabel")))->GetText().ToString(),FString(TEXT("玩家 B")));
    TestEqual(TEXT("Score consumes projected facts"),Header->GetDisplayedScoreLabel(),FString(TEXT("2 - 1")));
    TestFalse(TEXT("No TP before roll"),IsShellWidgetVisible(Header->GetWidgetFromName(TEXT("LeftTacticalPointChip"))));
    auto* Used=CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("LeftAttackTurnStepsFrame0")));
    auto* Current=CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("LeftAttackTurnStepsFrame1")));
    auto* Future=CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("LeftAttackTurnStepsFrame2")));
    TestFalse(TEXT("Completed and current have distinct fills"),Used->Background.TintColor==Current->Background.TintColor);
    TestFalse(TEXT("Current and future have distinct fills"),Current->Background.TintColor==Future->Background.TintColor);
    View.bTacticalPointRollReady=false; View.bCurrentAttackActive=true;
    View.RouteKind=EMatchPlayCurrentAttackRouteKind::Ordinary; View.ActionPoint=3;
    Header->RefreshFromPresentation(Build(EInitialTurnOrderPlayer::PlayerA));
    TestTrue(TEXT("Current attack owns the disclosed TP"),IsShellWidgetVisible(Header->GetWidgetFromName(TEXT("LeftTacticalPointChip"))));
    TestFalse(TEXT("Defender has no duplicate resource"),IsShellWidgetVisible(Header->GetWidgetFromName(TEXT("RightTacticalPointChip"))));
    TestEqual(TEXT("TP value"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("LeftTacticalPointChipValue")))->GetText().ToString(),FString(TEXT("3")));
    const FFMCodexUMGSidePrimaryColors Defaults;
    TestEqual(TEXT("Default A source"),CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("LeftPlayerBroadcastRegion")))->GetBrushColor(),Defaults.PlayerAPrimaryColor);
    TestEqual(TEXT("Default B source"),CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("RightPlayerBroadcastRegion")))->GetBrushColor(),Defaults.PlayerBPrimaryColor);
    const auto NeutralScore=CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("CentralBroadcastScoreValue")))->GetColorAndOpacity();
    FFMCodexUMGSidePrimaryColors Custom; Custom.PlayerAPrimaryColor=FLinearColor(.4,.08,.55,1); Custom.PlayerBPrimaryColor=FLinearColor(.05,.65,.2,1);
    Header->SetPlayerAccentColors(Custom);
    Header->RefreshFromPresentation(Build(EInitialTurnOrderPlayer::PlayerB));
    TestEqual(TEXT("Viewer B has B identity on left"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("LeftPlayerIdentityLabel")))->GetText().ToString(),FString(TEXT("玩家 B")));
    TestEqual(TEXT("Viewer B has A identity on right"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("RightPlayerIdentityLabel")))->GetText().ToString(),FString(TEXT("玩家 A")));
    TestEqual(TEXT("Viewer B maps B accent to left"),CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("LeftPlayerBroadcastRegion")))->GetBrushColor(),Custom.PlayerBPrimaryColor);
    TestEqual(TEXT("Viewer B maps A accent to right"),CastChecked<UBorder>(Header->GetWidgetFromName(TEXT("RightPlayerBroadcastRegion")))->GetBrushColor(),Custom.PlayerAPrimaryColor);
    TestTrue(TEXT("Viewer reorder preserves attacking resource owner"),IsShellWidgetVisible(Header->GetWidgetFromName(TEXT("RightTacticalPointChip"))));
    TestEqual(TEXT("Viewer-relative score unaffected by palette"),Header->GetDisplayedScoreLabel(),FString(TEXT("1 - 2")));
    TestEqual(TEXT("Neutral score text"),CastChecked<UTextBlock>(Header->GetWidgetFromName(TEXT("CentralBroadcastScoreValue")))->GetColorAndOpacity(),NeutralScore);
    TestTrue(TEXT("Black identity still has a visible display edge"),FMCodexMatchShellStyle::DisplayAccent(FLinearColor::Black).R>=.38f);
    TestTrue(TEXT("White edge is constrained"),FMCodexMatchShellStyle::DisplayAccent(FLinearColor::White).R<=.85f);
    const auto A=FFMCodexPlayerUIStyle::Get().MakeDockButtonStyle(EFMCodexPlayerUIActionRole::Primary,&Custom.PlayerAPrimaryColor);
    const auto B=FFMCodexPlayerUIStyle::Get().MakeDockButtonStyle(EFMCodexPlayerUIActionRole::Primary,&Custom.PlayerBPrimaryColor);
    TestEqual(TEXT("CTA remains mint independent of identity"),A.Normal.TintColor,B.Normal.TintColor);
    TestEqual(TEXT("Mint token"),A.Normal.TintColor.GetSpecifiedColor(),FMCodexMatchShellStyle::Mint());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMatchShellActionsTest,"FMCodex.LocalPlay.MatchShell.ActionHierarchyAndWaiting",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMatchShellActionsTest::RunTest(const FString&)
{
    auto* Dock=NewObject<UFMCodexInteractionPanelWidget>(); Dock->TakeWidget();
    FFMCodexUMGInteractionViewModel P;
    P.bCanRollTacticalPoints=true; P.PrimaryAction.bAvailable=true; P.PrimaryAction.Label=TEXT("ROLL TACTICAL POINTS");
    P.ExpectedActorLabel=TEXT("PLAYER A TO ACT");
    P.EmptyStateLabel=TEXT("No player action is available.");
    Dock->RefreshFromPresentation(P);
    auto W=[&](const TCHAR* Name){ return Dock->GetWidgetFromName(Name); };
    TestEqual(TEXT("Production A operation copy"),CastChecked<UTextBlock>(W(TEXT("InteractionExpectedActor")))->GetText().ToString(),FString(TEXT("玩家 A 操作")));
    TestTrue(TEXT("Real TP button retained"),IsShellWidgetVisible(W(TEXT("InteractionTacticalPointRollButton"))));
    TestFalse(TEXT("Roll does not repeat its button instruction"),IsShellWidgetVisible(W(TEXT("DeploymentHandInstruction"))));
    TestFalse(TEXT("Hidden instruction leaves no candidate panel beside roll"),IsShellWidgetVisible(W(TEXT("InteractionCandidateRegion"))));
    TestFalse(TEXT("Empty placeholder cannot compete with action"),IsShellWidgetVisible(W(TEXT("InteractionBoundedFallback"))));
    P.bCanRollTacticalPoints=false; P.bCanFinishDeployment=true; P.Category=EFMCodexUMGInteractionCategory::Deploy;
    P.TitleLabel=TEXT("部署球员");
    Dock->RefreshFromPresentation(P);
    auto* Primary=CastChecked<UButton>(W(TEXT("InteractionFinishDeploymentButton")));
    auto* Secondary=CastChecked<UButton>(W(TEXT("DeploymentTacticalReferenceEntryButton")));
    TestTrue(TEXT("Deployment keeps real primary and information actions"),IsShellWidgetVisible(Primary) && IsShellWidgetVisible(Secondary));
    TestFalse(TEXT("Secondary does not use primary fill"),Primary->GetStyle().Normal.TintColor==Secondary->GetStyle().Normal.TintColor);
    TestEqual(TEXT("Concise finish label"),CastChecked<UTextBlock>(W(TEXT("InteractionFinishDeploymentButtonLabel")))->GetText().ToString(),FString(TEXT("结束部署")));
    TestEqual(TEXT("Deployment title retained"),CastChecked<UTextBlock>(W(TEXT("InteractionActionTitle")))->GetText().ToString(),FString(TEXT("部署球员")));
    TestEqual(TEXT("Tactical information label retained"),CastChecked<UTextBlock>(W(TEXT("DeploymentTacticalReferenceEntryButtonLabel")))->GetText().ToString(),FString(TEXT("战术说明")));
    TestTrue(TEXT("Deployment has no instruction text"),CastChecked<UTextBlock>(W(TEXT("DeploymentHandInstruction")))->GetText().IsEmpty());
    TestFalse(TEXT("Deployment hint collapses"),IsShellWidgetVisible(W(TEXT("DeploymentHandInstruction"))));
    TestFalse(TEXT("Deployment instruction region leaves no gap"),IsShellWidgetVisible(W(TEXT("InteractionCandidateRegion"))));
    P.ExpectedActorLabel=TEXT("PLAYER B TO ACT");
    Dock->RefreshFromPresentation(P);
    TestEqual(TEXT("Production B operation copy"),CastChecked<UTextBlock>(W(TEXT("InteractionExpectedActor")))->GetText().ToString(),FString(TEXT("玩家 B 操作")));
    Dock->SetActionWaitPromptMode(true,false,FText::FromString(TEXT("请玩家 B 操作")),FText::FromString(TEXT("部署球员")));
    Dock->RefreshFromPresentation(P);
    TestEqual(TEXT("Mirrored acting context uses the same concise copy"),CastChecked<UTextBlock>(W(TEXT("InteractionExpectedActor")))->GetText().ToString(),FString(TEXT("玩家 B 操作")));
    Dock->SetActionWaitPromptMode(false,false,FText::GetEmpty(),FText::GetEmpty());
    auto Role=P; Role.Category=EFMCodexUMGInteractionCategory::SelectCarrier; Role.bCanFinishDeployment=false;
    Role.bUseOnPitchPlayerSelection=true; Role.OnPitchSelectionHintLabel=TEXT("Click a player on the pitch");
    Dock->RefreshFromPresentation(Role);
    TestTrue(TEXT("Other states retain optional hint and region"),IsShellWidgetVisible(W(TEXT("DeploymentHandInstruction"))) && IsShellWidgetVisible(W(TEXT("InteractionCandidateRegion"))));
    TestEqual(TEXT("Role hint remains useful"),CastChecked<UTextBlock>(W(TEXT("DeploymentHandInstruction")))->GetText().ToString(),FString(TEXT("点击场上球员选择")));
    Dock->SetActionWaitPromptMode(true,true,FText::FromString(TEXT("等待玩家 B 操作")),FText::FromString(TEXT("部署球员")));
    Dock->RefreshFromPresentation(P);
    TestFalse(TEXT("Waiting viewer has no primary action"),IsShellWidgetVisible(Primary));
    TestFalse(TEXT("Waiting viewer has no secondary action"),IsShellWidgetVisible(Secondary));
    TestEqual(TEXT("Waiting identity survives styling"),CastChecked<UTextBlock>(W(TEXT("InteractionExpectedActor")))->GetText().ToString(),FString(TEXT("等待玩家 B 操作")));
    return true;
}
#endif
