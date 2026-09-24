#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexPitchWidget.h"
#include "Blueprint/WidgetTree.h"
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "HAL/IConsoleManager.h"

namespace
{
struct FTheaterModes
{
	IConsoleVariable* Theater=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2"));
	IConsoleVariable* Formula=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.FormulaV2"));
	int32 OldTheater=Theater->GetInt(), OldFormula=Formula->GetInt();
	~FTheaterModes() { Theater->Set(OldTheater,ECVF_SetByCode); Formula->Set(OldFormula,ECVF_SetByCode); }
};
bool Visible(UFMCodexLocalMatchScreenWidget* S, const TCHAR* Name)
{
	auto* W=S->GetWidgetFromName(Name);
	return W && W->GetVisibility()!=ESlateVisibility::Collapsed && W->GetVisibility()!=ESlateVisibility::Hidden;
}
FString Label(UFMCodexLocalMatchScreenWidget* S,const TCHAR* Name)
{
	return CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetText().ToString();
}
void Refresh(UFMCodexLocalMatchScreenWidget* S) { auto P=S->GetPresentation(); S->RefreshFromPresentation(P); }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterScopeTest,
	"FMCodex.LocalPlay.ResolutionTheater.DefaultOnAndScope",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterScopeTest::RunTest(const FString&)
{
	FTheaterModes Modes;
	TestEqual(TEXT("Fresh process defaults to ON"),Modes.Theater->GetInt(),1);
	Modes.Theater->Set(0,ECVF_SetByCode);
	FFMCodexUMGMatchScreenViewModel P;
	P.InlineFormula.bVisible=true; P.InlineFormula.ContestId=TEXT("Cross.High");
	TestFalse(TEXT("OFF does not claim High Cross"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	Modes.Theater->Set(1,ECVF_SetByCode);
	TestTrue(TEXT("ON claims High Cross"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	for (const auto Id:{TEXT("Cross.Low"),TEXT("Corner.High"),TEXT("SetPiece.Short.Direct"),TEXT("ThroughBall.Feet"),TEXT("LongShot.DirectShot")})
	{
		P.InlineFormula.ContestId=Id;
		TestFalse(FString::Printf(TEXT("Excludes %s"),Id),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	}
	P.InlineFormula.ContestId=TEXT("Cross.Setup"); P.InlineFormula.bVisible=false;
	P.Interaction.Category=EFMCodexUMGInteractionCategory::SelectBranchIntent;
	TestTrue(TEXT("Neutral setup visible without actor-only choices"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	P.FullTime.bVisible=true;
	TestFalse(TEXT("Full Time releases theater"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	P.FullTime.bVisible=false; P.Resolution.bRejected=true;
	TestFalse(TEXT("Rejected request preserves recovery surface"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	P.Resolution.bRejected=false; P.InlineFormula.ContestId=TEXT("Cross.Low");
	auto Displayed=P.InlineFormula; Displayed.bVisible=true; Displayed.ContestId=TEXT("Cross.Route");
	TestTrue(TEXT("Hidden Low route must not affect stage"),FMCodexResolutionTheaterPrototype::WantsTheater(P,Displayed));
	Displayed.RouteResultLabel=TEXT("already disclosed route");
	TestFalse(TEXT("Disclosed Low returns to fallback"),FMCodexResolutionTheaterPrototype::WantsTheater(P,Displayed));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FResolutionTheaterViewTest,
	"FMCodex.LocalPlay.ResolutionTheater.SharedViewerLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FResolutionTheaterViewTest::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	N.Add(TEXT("A.High")); C.Add(TEXT("High")); N.Add(TEXT("B.LowFallback")); C.Add(TEXT("Low"));
}
bool FResolutionTheaterViewTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPlayerFacingOrdinaryUITests;
	const bool High=Parameters==TEXT("High"); FTheaterModes Modes;
	Modes.Theater->Set(0,ECVF_SetByCode); Modes.Formula->Set(0,ECVF_SetByCode);
	FUIFixture F(!High,false);
	if (!TestTrue(TEXT("Canonical safe-view fixture reaches Skill"),F.SkillFixture(false))) return false;
	auto* Actor=F.Attacker(); auto* Defender=F.Defender();
	auto* S=Actor->GetPlayerMatchScreen(); auto* D=Defender->GetPlayerMatchScreen();
	TestNull(TEXT("Explicit Development OFF allocates no theater"),S->GetWidgetFromName(TEXT("ResolutionTheater")));
	const auto* Cross=S->GetPresentation().Interaction.SelectionChoices.FindByPredicate([](const auto& V) { return V.SkillType==ESkillRuleType::Cross; });
	if (!TestNotNull(TEXT("Safe legal Cross choice"),Cross)) return false;
	Modes.Theater->Set(1,ECVF_SetByCode); S->RequestSubmitSkill(Cross->OptionId);
	for (auto* W:{S,D})
	{
		TestTrue(TEXT("Both viewers enter immediately after accepted Skill"),Visible(W,TEXT("ResolutionTheater")));
		TestEqual(TEXT("Field stays painted but board input is suppressed"),W->GetWidgetFromName(TEXT("MatchShellViewportFit"))->GetVisibility(),ESlateVisibility::HitTestInvisible);
		TestEqual(TEXT("Entry does not predict route"),Label(W,TEXT("TheaterTitle")),FString(TEXT("传中")));
		TestFalse(TEXT("No invented pre-route formula"),Visible(W,TEXT("TheaterAttackValue")));
		TestEqual(TEXT("Safe carrier role"),Label(W,TEXT("TheaterAttackRole0")),FString(TEXT("持球")));
		TestEqual(TEXT("Safe runner role"),Label(W,TEXT("TheaterAttackRole1")),FString(TEXT("跑位")));
		const auto& Header=W->GetMatchHeader()->GetPresentation();
		TestTrue(TEXT("Score context preserves each viewer's header orientation"),Label(W,TEXT("TheaterContext")).StartsWith(
			FFMCodexPlayerUIPresentationText::MatchScreenLabel(Header.LeftPlayerLabel).ToString()));
	}
	TestTrue(TEXT("Actor owns branch choice"),Visible(S,TEXT("TheaterChoices")));
	TestFalse(TEXT("Waiting viewer has no branch CTA"),Visible(D,TEXT("TheaterChoices")));
	TestTrue(TEXT("Waiting viewer receives explicit prompt"),Label(D,TEXT("TheaterStatus")).Contains(TEXT("等待玩家")));
	if (High)
	{
		FMCodexResolutionTheaterPrototype::FMotion Motion;
		const int32 Calls=F.Entropy->Calls, Sends=F.Backend(Actor).Sends;
		auto& Tree=*S->WidgetTree;
		FMCodexResolutionTheaterPrototype::SetActive(Tree,Motion,true);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.05f);
		TestEqual(TEXT("Entry discards pre-activation frame delta"),Motion.Elapsed,0.f);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.32f);
		TestTrue(TEXT("Attack enters before Defense"),Tree.FindWidget(TEXT("TheaterAttackPanel"))->GetRenderOpacity()>0.f
			&& Tree.FindWidget(TEXT("TheaterDefensePanel"))->GetRenderOpacity()==0.f);
		TestEqual(TEXT("Clutter has faded without hiding the field"),Tree.FindWidget(TEXT("LocalPlayerCardRackRegion"))->GetRenderOpacity(),0.f);
		FMCodexResolutionTheaterPrototype::SetActive(Tree,Motion,true);
		TestEqual(TEXT("Repeated View does not restart entry"),Motion.Elapsed,.32f);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.22f);
		TestTrue(TEXT("Defense enters before VS"),Tree.FindWidget(TEXT("TheaterDefensePanel"))->GetRenderOpacity()>0.f
			&& Tree.FindWidget(TEXT("TheaterVS"))->GetRenderOpacity()==0.f);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.17f);
		TestTrue(TEXT("VS establishes before action lane"),Tree.FindWidget(TEXT("TheaterVS"))->GetRenderOpacity()>0.f
			&& Tree.FindWidget(TEXT("TheaterBottom"))->GetRenderOpacity()==0.f);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.20f);
		TestEqual(TEXT("Action lane accepts input after ordered entry"),Tree.FindWidget(TEXT("TheaterBottom"))->GetVisibility(),ESlateVisibility::SelfHitTestInvisible);
		FMCodexResolutionTheaterPrototype::SetActive(Tree,Motion,false);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.10f);
		const float Interrupted=Motion.FieldProgress;
		FMCodexResolutionTheaterPrototype::SetActive(Tree,Motion,true);
		TestEqual(TEXT("Interrupted exit reverses continuously"),Motion.FieldProgress,Interrupted);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,.01f);
		FMCodexResolutionTheaterPrototype::Tick(Tree,Motion,1.f);
		TestEqual(TEXT("Motion consumes no RNG"),F.Entropy->Calls,Calls);
		TestEqual(TEXT("Motion submits no action"),F.Backend(Actor).Sends,Sends);
	}
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterHigh")))->OnClicked.Broadcast();
	TestEqual(TEXT("Theater choice submits existing typed branch intent"),F.Backend(Actor).Last.IntentKind,Kind::SubmitBranchIntent);
	F.Entropy->Word=High?1:4;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Theater route uses original command"),F.Backend(Actor).Last.IntentKind,Kind::CrossInitialRouteRoll);
	const int32 CallsAfterRoute=F.Entropy->Calls;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Repeated click during reveal cannot resolve twice"),F.Entropy->Calls,CallsAfterRoute);
	F.Settle();
	if (!High)
	{
		for (auto* W:{S,D})
		{
			TestFalse(TEXT("Low exits theater content immediately"),Visible(W,TEXT("TheaterContent")));
			TestTrue(TEXT("Low restores entire board"),Visible(W,TEXT("MatchShellViewportFit")));
			TestEqual(TEXT("Low keeps original formula"),W->GetInlineFormulaSurface()->GetPresentation().ContestId,FName(TEXT("Cross.Low")));
		}
		return true;
	}
	for (auto* W:{S,D})
	{
		TestTrue(TEXT("High retains theater"),Visible(W,TEXT("ResolutionTheater")));
		TestTrue(TEXT("Pending roll is explicit"),Visible(W,TEXT("TheaterAttackPending")));
		TestEqual(TEXT("Displayed value is verbatim safe projection"),Label(W,TEXT("TheaterAttackNumber")),W->GetInlineFormulaSurface()->GetPresentation().AttackRow.DisplayedResultLabel);
	}
	TestEqual(TEXT("Actor roll information uses copy family"),Label(S,TEXT("TheaterDetail")),FString(TEXT("轮到进攻方掷点")));
	TestEqual(TEXT("Waiting roll information uses copy family"),Label(D,TEXT("TheaterDetail")),FString(TEXT("等待进攻方掷点")));
	for (const auto Prefix:{FString(TEXT("TheaterAttack")),FString(TEXT("TheaterDefense"))})
	{
		TestEqual(TEXT("Unresolved RHS is current value"),Label(S,*(Prefix+TEXT("ValueLabel"))),FString(TEXT("当前值")));
		TestTrue(TEXT("Unresolved equals and RHS stay visible"),Visible(S,*(Prefix+TEXT("Equal"))) && Visible(S,*(Prefix+TEXT("FinalNumber"))));
		TestNull(TEXT("Permanent formula breakdown removed"),S->GetWidgetFromName(FName(*(Prefix+TEXT("Equation")))));
		auto* Hover=CastChecked<UBorder>(S->GetWidgetFromName(FName(*(Prefix+TEXT("BaseHover")))));
		const FString Explanation=CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Hover->GetToolTip())->GetContent())->GetContent())->GetText().ToString();
		TestTrue(TEXT("Tooltip describes weighted authoritative attributes"),Explanation.Contains(TEXT("×")) && Explanation.Contains(TEXT("当前基础值")));
		TestFalse(TEXT("Tooltip contains no unresolved die"),Explanation.Contains(TEXT("掷点")));
		if (Prefix.EndsWith(TEXT("Defense"))) TestTrue(TEXT("Fixed +2 is defense bonus, not tactical points"),Explanation.Contains(TEXT("防守加成 +2")) && !Explanation.Contains(TEXT("战术点数")));
	}
	for (const auto Name:{TEXT("TheaterTitle"),TEXT("TheaterAttackName0"),TEXT("TheaterContinueLabel"),TEXT("TheaterAttackFinalNumber")})
		TestEqual(TEXT("Theater text has no faux stroke weight"),CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetFont().OutlineSettings.OutlineSize,0);
	const auto Identity=S->GetPresentation().Header.AttackSequence;
	const auto BeforeFormula=S->GetInlineFormulaSurface()->GetPresentation();
	{
		auto Awaiting=BeforeFormula;
		Awaiting.bDiceRevealVisible=true; Awaiting.RollReel={}; Awaiting.ActiveRollSequenceIndex=0;
		FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,S->GetPresentation(),Awaiting,S->GetMatchHeader()->GetPresentation(),true);
		TestTrue(TEXT("Pending ACK without a reel frame retains the unknown operand"),Visible(S,TEXT("TheaterAttackPending")) && !Visible(S,TEXT("TheaterAttackReelHost")));
		Refresh(S);
	}
	for (int32 Compare=0;Compare<3;++Compare)
	{
		Modes.Theater->Set(Compare==2?1:0,ECVF_SetByCode); Modes.Formula->Set(Compare==1?1:0,ECVF_SetByCode); Refresh(S);
		TestEqual(TEXT("Compare preserves attack identity"),S->GetPresentation().Header.AttackSequence,Identity);
		TestEqual(TEXT("Compare preserves safe subtotal"),S->GetInlineFormulaSurface()->GetPresentation().AttackRow.DisplayedResultLabel,BeforeFormula.AttackRow.DisplayedResultLabel);
		TestEqual(TEXT("Compare never consumes RNG"),F.Entropy->Calls,CallsAfterRoute);
		TestEqual(TEXT("Theater mode wins only when enabled"),Visible(S,TEXT("TheaterContent")),Compare==2);
		if (Compare<2) TestEqual(TEXT("Fallback obeys FormulaV2"),S->GetInlineFormulaSurface()->IsBroadcastPrototypeVisible(),Compare==1);
	}
	F.Entropy->Word=5;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Attack uses original typed command"),F.Backend(Actor).Last.IntentKind,Kind::CrossHighAttackRoll);
	S->PauseInlineFormulaRevealTimerForTesting(); S->AdvanceInlineFormulaRevealForTesting(.4f);
	TestTrue(TEXT("Attack roll occupies the question slot"),Visible(S,TEXT("TheaterAttackReelHost")) && !Visible(S,TEXT("TheaterAttackPending")));
	TestTrue(TEXT("Defense remains unresolved during Attack"),Visible(S,TEXT("TheaterDefensePending")) && !Visible(S,TEXT("TheaterDefenseReelHost")));
	TestFalse(TEXT("Formula roll has no disconnected bottom reel"),Visible(S,TEXT("TheaterRoll")));
	auto* InlineReel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterAttackReel")));
	const auto& SourceReel=S->GetInlineFormulaSurface()->GetRollReelWidget()->GetPresentation();
	TestEqual(TEXT("Inline host reuses the existing projected center"),InlineReel->GetPresentation().CenterValue,SourceReel.CenterValue);
	TestEqual(TEXT("Inline host reuses the existing continuous clock"),InlineReel->GetPresentation().ContinuousPositionCells,SourceReel.ContinuousPositionCells);
	TestTrue(TEXT("Cycling retains the current RHS"),Visible(S,TEXT("TheaterAttackFinalNumber")));
	TestEqual(TEXT("Cycling RHS remains prior subtotal"),Label(S,TEXT("TheaterAttackFinalNumber")),BeforeFormula.AttackRow.DisplayedResultLabel);
	TestEqual(TEXT("Cycling label remains current"),Label(S,TEXT("TheaterAttackValueLabel")),FString(TEXT("当前值")));
	F.Settle();
	TestEqual(TEXT("Revealed operand is the authoritative attack D6"),Label(S,TEXT("TheaterAttackRollValue")),FString(TEXT("6")));
	TestEqual(TEXT("Resolved total is projected without arithmetic"),Label(S,TEXT("TheaterAttackFinalNumber")),S->GetInlineFormulaSurface()->GetPresentation().AttackRow.DisplayedResultLabel);
	TestEqual(TEXT("Disclosed Attack label is final"),Label(S,TEXT("TheaterAttackValueLabel")),FString(TEXT("最终值")));
	TestEqual(TEXT("Unresolved Defense label is still current"),Label(S,TEXT("TheaterDefenseValueLabel")),FString(TEXT("当前值")));
	TestFalse(TEXT("Actor waits for defense without CTA"),Visible(S,TEXT("TheaterPrimaryBounds")));
	TestTrue(TEXT("Defender owns defense CTA"),Visible(D,TEXT("TheaterPrimaryBounds")));
	const FString ScoreBefore=Label(S,TEXT("TheaterContext")); F.Entropy->Word=0;
	CastChecked<UButton>(D->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Defense uses original typed command"),F.Backend(Defender).Last.IntentKind,Kind::CrossHighDefenseRoll);
	TestEqual(TEXT("Theater score obeys original reveal gate"),Label(S,TEXT("TheaterContext")),ScoreBefore);
	TestFalse(TEXT("Outcome hidden during reel"),Visible(S,TEXT("TheaterOutcome")));
	TestFalse(TEXT("Reason cannot disclose suppression during reel"),Label(S,TEXT("TheaterDetail")).Contains(TEXT("快速压制")));
	TestTrue(TEXT("Rich reason is empty before legal visible reveal"),
		CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().IsEmpty());
	TestFalse(TEXT("Winner is not announced while defense is revealing"),
		Label(S,TEXT("TheaterAttackActive"))==TEXT("获胜") || Label(S,TEXT("TheaterDefenseActive"))==TEXT("获胜"));
	F.Settle();
	for (auto* W:{S,D})
	{
		TestTrue(TEXT("Outcome remains inside theater"),Visible(W,TEXT("TheaterOutcome")));
		TestTrue(TEXT("Reason uses actual suppression fact"),Label(W,TEXT("TheaterDetail")).Contains(TEXT("快速压制")) && Label(W,TEXT("TheaterReasonSecondary")).Contains(TEXT("不比较最终总值")));
		const auto Reason = CastChecked<URichTextBlock>(W->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().ToString();
		TestTrue(TEXT("Suppression emphasizes each disclosed die, not the sentence"),
			Reason.StartsWith(TEXT("触发快速压制：<Value>")) && Reason.Contains(TEXT("</> 点压制 <Value>")));
		TestFalse(TEXT("Special rule never gets ordinary comparison explanation"),
			Label(W,TEXT("TheaterReasonSecondary")).Contains(TEXT("本次公式按照总值大小比较")));
		TestEqual(TEXT("Result CTA says next attack"),Label(W,TEXT("TheaterContinueLabel")),FString(TEXT("下一回合")));
		TestFalse(TEXT("Resolved value has no pending question mark"),Visible(W,TEXT("TheaterDefensePending")));
		TestEqual(TEXT("Final value not recomputed by theater"),Label(W,TEXT("TheaterDefenseFinalNumber")),W->GetInlineFormulaSurface()->GetPresentation().DefenseRow.DisplayedResultLabel);
		TestEqual(TEXT("Winner badge follows disclosed narrative"),Label(W,TEXT("TheaterAttackActive")),FString(TEXT("获胜")));
		TestTrue(TEXT("Winner badge is visible"),Visible(W,TEXT("TheaterAttackBadge")));
		TestFalse(TEXT("Losing side does not retain a current badge"),Visible(W,TEXT("TheaterDefenseBadge")));
		TestTrue(TEXT("Outcome retains tactical title within header family"),Visible(W,TEXT("TheaterTitle")));
		TestTrue(TEXT("Continue has distinct arrow icon"),Visible(W,TEXT("TheaterNextIcon")) && !Visible(W,TEXT("TheaterDiceIcon")));
	}
	// Presentation-only fixture: the winner fact is deliberately opposite to the
	// displayed numeric ordering. No alternate gameplay result is submitted.
	{
		auto Display=S->GetInlineFormulaSurface()->GetPresentation();
		Display.bNarrativeAttackSuccess=false;
		FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,S->GetPresentation(),Display,S->GetMatchHeader()->GetPresentation(),false);
		TestTrue(TEXT("Defense winner comes from the fact, never larger total"),Visible(S,TEXT("TheaterDefenseBadge")) && !Visible(S,TEXT("TheaterAttackBadge")));
		TestEqual(TEXT("Defense badge says winner"),Label(S,TEXT("TheaterDefenseActive")),FString(TEXT("获胜")));
		Refresh(S);
	}
	const int32 CallsAtEnd=F.Entropy->Calls; const auto Phase=S->GetInlineFormulaRevealPhase();
	Actor->RefreshPlayerFacingUI(); Defender->RefreshPlayerFacingUI();
	TestEqual(TEXT("Duplicate View preserves settled phase"),S->GetInlineFormulaRevealPhase(),Phase);
	TestEqual(TEXT("Duplicate View consumes no RNG"),F.Entropy->Calls,CallsAtEnd);
	auto* Next=Visible(S,TEXT("TheaterPrimaryBounds"))?S:D;
	CastChecked<UButton>(Next->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	for (auto* W:{S,D})
	{
		TestFalse(TEXT("Terminal handoff releases theater content"),Visible(W,TEXT("TheaterContent")));
		TestTrue(TEXT("Terminal handoff restores board"),Visible(W,TEXT("MatchShellViewportFit")));
		W->ResetPresentationSession(); TestFalse(TEXT("Session reset leaves no stale theater"),Visible(W,TEXT("ResolutionTheater")));
	}
	return true;
}
#endif
