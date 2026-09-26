#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexPitchWidget.h"
#include "Blueprint/WidgetTree.h"
#include "../NetworkPlay/FMCodexNetworkPlayerFacingTestFixture.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
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
	IConsoleVariable* Low=IConsoleManager::Get().FindConsoleVariable(TEXT("fm.UI.ResolutionStageV2.LowCross"));
	int32 OldTheater=Theater->GetInt(), OldFormula=Formula->GetInt();
	int32 OldLow=Low->GetInt();
	~FTheaterModes() { Theater->Set(OldTheater,ECVF_SetByCode); Formula->Set(OldFormula,ECVF_SetByCode); Low->Set(OldLow,ECVF_SetByCode); }
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
	TestEqual(TEXT("Low review defaults to ON without console command"),Modes.Low->GetInt(),1);
	Modes.Theater->Set(0,ECVF_SetByCode);
	FFMCodexUMGMatchScreenViewModel P;
	P.InlineFormula.bVisible=true; P.InlineFormula.ContestId=TEXT("Cross.High");
	TestFalse(TEXT("OFF does not claim High Cross"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	Modes.Theater->Set(1,ECVF_SetByCode);
	TestTrue(TEXT("ON claims High Cross"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	P.InlineFormula.ContestId=TEXT("Cross.Low");
	TestTrue(TEXT("Development includes Low Formula"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	Modes.Low->Set(0,ECVF_SetByCode);
	TestFalse(TEXT("Low fallback only excludes Low"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	P.InlineFormula.ContestId=TEXT("Cross.High");
	TestTrue(TEXT("Low fallback preserves High"),FMCodexResolutionTheaterPrototype::WantsTheater(P,P.InlineFormula));
	Modes.Low->Set(1,ECVF_SetByCode);
	for (const auto Id:{TEXT("Corner.High"),TEXT("SetPiece.Short.Direct"),TEXT("ThroughBall.Feet"),TEXT("LongShot.DirectShot")})
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
	TestTrue(TEXT("Disclosed Low stays inside Theater"),FMCodexResolutionTheaterPrototype::WantsTheater(P,Displayed));
	Modes.Low->Set(0,ECVF_SetByCode);
	TestFalse(TEXT("Disclosed Low returns to fallback"),FMCodexResolutionTheaterPrototype::WantsTheater(P,Displayed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FResolutionTheaterParticipantSafetyTest,
	"FMCodex.LocalPlay.ResolutionTheater.ParticipantDisclosure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FResolutionTheaterParticipantSafetyTest::RunTest(const FString&)
{
	using namespace FMCodexPlayerFacingOrdinaryUITests;
	FUIFixture F;
	if (!TestTrue(TEXT("Real deployment activates goalkeeper"),F.ReachSkill(TEXT("Prototype.Arsenal.BukayoSaka"),TEXT("Prototype.Arsenal.KaiHavertz"),true))) return false;
	SkillPayload Skill; Skill.SkillId=TEXT("Canonical.Skill.Cross.4.6");
	if (!TestTrue(TEXT("Canonical Cross selected"),F.Send(F.Attacker(),Kind::SubmitSkill,{},{},{},{},{},Skill))) return false;
	for (auto Side:{Side::PlayerA,Side::PlayerB})
	{
		const auto VisibleView=Access::Safe(*F.Mode,Side,true);
		const auto HiddenView=Access::Safe(*F.Mode,Side,false);
		const auto Visible=FFMCodexLocalMatchUMGPresentationBuilder::Build(VisibleView,{},FString(),Side);
		const auto Hidden=FFMCodexLocalMatchUMGPresentationBuilder::Build(HiddenView,{},FString(),Side);
		TestEqual(TEXT("Both permitted viewers receive real keeper"),Visible.InlineFormula.DefenseRow.Participants.Num(),3);
		TestTrue(TEXT("Withheld initial disclosure cannot publish Cross participants"),Hidden.InlineFormula.DefenseRow.Participants.IsEmpty());
		TestFalse(TEXT("Withheld context cannot activate Theater"),FMCodexResolutionTheaterPrototype::WantsTheater(Hidden,Hidden.InlineFormula));
	}
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FResolutionTheaterViewTest,
	"FMCodex.LocalPlay.ResolutionTheater.SharedViewerLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FResolutionTheaterViewTest::GetTests(TArray<FString>& N,TArray<FString>& C) const
{
	N.Add(TEXT("A.High")); C.Add(TEXT("High"));
	N.Add(TEXT("A.HighGoalkeeper")); C.Add(TEXT("HighGK"));
	N.Add(TEXT("B.LowGoalkeeper")); C.Add(TEXT("LowBGK"));
	N.Add(TEXT("A.Low")); C.Add(TEXT("LowA")); N.Add(TEXT("B.Low")); C.Add(TEXT("LowB"));
	N.Add(TEXT("B.LowFallback")); C.Add(TEXT("LowFallback"));
}
bool FResolutionTheaterViewTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexPlayerFacingOrdinaryUITests;
	const bool High=Parameters.StartsWith(TEXT("High")); const bool WithGK=Parameters.EndsWith(TEXT("GK")); FTheaterModes Modes;
	const bool Fallback=Parameters==TEXT("LowFallback");
	Modes.Low->Set(Fallback?0:1,ECVF_SetByCode);
	Modes.Theater->Set(0,ECVF_SetByCode); Modes.Formula->Set(0,ECVF_SetByCode);
	FUIFixture F(Parameters.StartsWith(TEXT("LowB")) || Fallback,false);
	const bool BFirst=Parameters.StartsWith(TEXT("LowB"));
	const bool Ready=WithGK ? F.ReachSkill(
		FName(BFirst?TEXT("Prototype.ManchesterCity.JeremyDoku"):TEXT("Prototype.Arsenal.BukayoSaka")),
		FName(BFirst?TEXT("Prototype.ManchesterCity.ErlingHaaland"):TEXT("Prototype.Arsenal.KaiHavertz")),true) : F.SkillFixture(false);
	if (!TestTrue(TEXT("Canonical safe-view fixture reaches Skill"),Ready)) return false;
	if (WithGK) F.Settle(); // ReachSkill advances authority; finish the entry reveal before UI intent.
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
		if (!TestNotNull(FString::Printf(TEXT("Setup carrier; safe contest=%s participants=%d"), *W->GetPresentation().InlineFormula.ContestId.ToString(), W->GetPresentation().InlineFormula.AttackRow.Participants.Num()), W->GetWidgetFromName(TEXT("TheaterAttackRole0")))) return false;
		TestEqual(TEXT("Safe carrier role"),Label(W,TEXT("TheaterAttackRole0")),FString(TEXT("持球")));
		TestEqual(TEXT("Safe runner role"),Label(W,TEXT("TheaterAttackRole1")),FString(TEXT("跑位")));
		const auto& Header=W->GetMatchHeader()->GetPresentation();
		TestTrue(TEXT("Score context preserves each viewer's header orientation"),Label(W,TEXT("TheaterContext")).StartsWith(
			FFMCodexPlayerUIPresentationText::MatchScreenLabel(Header.LeftPlayerLabel).ToString()));
	}
	auto CheckKeeper=[&](UFMCodexLocalMatchScreenWidget* W)
	{
		const auto* Role=Cast<UTextBlock>(W->GetWidgetFromName(TEXT("TheaterDefenseRole2")));
		TestEqual(TEXT("Keeper identity follows real activation, not roster membership"),
			Role && Role->GetText().ToString()==TEXT("门将"),WithGK);
		if (WithGK)
		{
			const auto& Facts=W->GetPresentation().InlineFormula.DefenseRow.Participants;
			TestEqual(TEXT("Shared row retains three actual defense participants"),Facts.Num(),3);
			if (Facts.Num()==3) TestEqual(TEXT("Rendered keeper matches safe participant"),Label(W,TEXT("TheaterDefenseName2")),Facts[2].PlayerName);
		}
	};
	for (auto* W:{S,D}) CheckKeeper(W);
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
	for (auto* W:{S,D})
	{
		CheckKeeper(W);
		TestEqual(TEXT("Pre-roll keeps canonical route hint"),Label(W,TEXT("TheaterDetail")),W->GetInlineFormulaSurface()->GetPresentation().RollHelperLabel);
		TestFalse(TEXT("Pre-roll info is not blank"),Label(W,TEXT("TheaterDetail")).IsEmpty());
	}
	F.Entropy->Word=High?1:4;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Theater route uses original command"),F.Backend(Actor).Last.IntentKind,Kind::CrossInitialRouteRoll);
	const int32 CallsAfterRoute=F.Entropy->Calls;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Repeated click during reveal cannot resolve twice"),F.Entropy->Calls,CallsAfterRoute);
	for (auto* W:{S,D})
	{
		W->PauseInlineFormulaRevealTimerForTesting();
		W->AdvanceInlineFormulaRevealForTesting(.4f);
		CheckKeeper(W);
		TestEqual(TEXT("Rolling route has concise non-result copy"),Label(W,TEXT("TheaterDetail")),FString(TEXT("正在判定传中路线")));
		auto* RouteReel=CastChecked<UFMCodexRollReelWidget>(W->GetWidgetFromName(TEXT("TheaterReel")));
		TestEqual(TEXT("Cross route explicitly selects CompactBox"),RouteReel->GetVisualVariant(),EFMCodexRollVisualVariant::CompactBox);
		TestTrue(TEXT("Both viewers use moving safe route projection"),RouteReel->GetPresentation().bMoving && !RouteReel->GetPresentation().bAuthoritativeValue);
		TestEqual(TEXT("Future High/Low is not in title"),Label(W,TEXT("TheaterTitle")),FString(TEXT("传中")));
		TestTrue(TEXT("Route result remains gated"),W->GetInlineFormulaSurface()->GetPresentation().RouteResultLabel.IsEmpty());
		TestFalse(TEXT("Neither viewer may act during route roll"),Visible(W,TEXT("TheaterPrimaryBounds")));
		W->AdvanceInlineFormulaRevealForTesting(1.062f);
		if (!Fallback)
		{
			CheckKeeper(W);
			TestEqual(TEXT("Landed copy uses disclosed actual D6 and branch"),Label(W,TEXT("TheaterDetail")),FString(High?TEXT("掷点结果为 2，判定为高球传中"):TEXT("掷点结果为 5，判定为低球传中")));
			TestTrue(TEXT("Route lands after the unchanged 1.46 second budget"),RouteReel->IsStaticResultTileVisible());
			TestEqual(TEXT("Landed route is the authoritative D6"),RouteReel->GetPresentation().CenterValue,High?2:5);
			TestEqual(TEXT("Only landed route permits tactical title"),Label(W,TEXT("TheaterTitle")),FString(High?TEXT("高球传中"):TEXT("低球传中")));
		}
	}
	F.Settle();
	if (!Fallback) for (auto* W:{S,D}) CheckKeeper(W);
	if (Fallback)
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
		TestTrue(TEXT("Actual Cross route retains theater"),Visible(W,TEXT("ResolutionTheater")));
		TestEqual(TEXT("Only disclosed route supplies title"),Label(W,TEXT("TheaterTitle")),FString(High?TEXT("高球传中"):TEXT("低球传中")));
		TestEqual(TEXT("Board does not reappear after route"),W->GetWidgetFromName(TEXT("MatchShellViewportFit"))->GetVisibility(),ESlateVisibility::HitTestInvisible);
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
		if (!High) TestTrue(TEXT("Low tooltip uses the actual route attributes"),Explanation.Contains(Prefix.EndsWith(TEXT("Attack"))?TEXT("射门"):TEXT("盯防")));
		if (Prefix.EndsWith(TEXT("Defense"))) TestTrue(TEXT("Fixed +2 is defense bonus, not tactical points"),Explanation.Contains(TEXT("防守加成 +2")) && !Explanation.Contains(TEXT("战术点数")));
	}
	for (const auto Name:{TEXT("TheaterTitle"),TEXT("TheaterAttackName0"),TEXT("TheaterContinueLabel"),TEXT("TheaterAttackFinalNumber")})
		TestEqual(TEXT("Theater text has no faux stroke weight"),CastChecked<UTextBlock>(S->GetWidgetFromName(Name))->GetFont().OutlineSettings.OutlineSize,0);
	const auto Identity=S->GetPresentation().Header.AttackSequence;
	const auto BeforeFormula=S->GetInlineFormulaSurface()->GetPresentation();
	{
		auto Awaiting=BeforeFormula;
		Awaiting.bDiceRevealVisible=true; Awaiting.RollReel={}; Awaiting.ActiveRollSequenceIndex=0;
		FMCodexResolutionTheaterPrototype::FTakerInspection Inspection;
		FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,S->GetPresentation(),Awaiting,S->GetMatchHeader()->GetPresentation(),true,Inspection);
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
		if (Compare<2) TestEqual(TEXT("Fallback obeys existing FormulaV2 scope"),S->GetInlineFormulaSurface()->IsBroadcastPrototypeVisible(),High && Compare==1);
	}
	F.Entropy->Word=5;
	CastChecked<UButton>(S->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Attack uses original typed command"),F.Backend(Actor).Last.IntentKind,High?Kind::CrossHighAttackRoll:Kind::CrossLowAttackRoll);
	S->PauseInlineFormulaRevealTimerForTesting(); S->AdvanceInlineFormulaRevealForTesting(.4f);
	TestTrue(TEXT("Attack roll occupies the question slot"),Visible(S,TEXT("TheaterAttackReelHost")) && !Visible(S,TEXT("TheaterAttackPending")));
	TestTrue(TEXT("Defense remains unresolved during Attack"),Visible(S,TEXT("TheaterDefensePending")) && !Visible(S,TEXT("TheaterDefenseReelHost")));
	TestFalse(TEXT("Formula roll has no disconnected bottom reel"),Visible(S,TEXT("TheaterRoll")));
	auto* InlineReel=CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterAttackReel")));
	TestTrue(TEXT("Migrated Cross formula slot opts into v2"),InlineReel->UsesTheaterInlineSkin());
	TestFalse(TEXT("Shared legacy Formula source keeps its old skin"),S->GetInlineFormulaSurface()->GetRollReelWidget()->UsesTheaterInlineSkin());
	TestEqual(TEXT("Route has a distinct bounded variant"),CastChecked<UFMCodexRollReelWidget>(S->GetWidgetFromName(TEXT("TheaterReel")))->GetVisualVariant(),EFMCodexRollVisualVariant::CompactBox);
	TestFalse(TEXT("Rolling action is not actionable"),Visible(S,TEXT("TheaterPrimaryBounds")));
	const auto& SourceReel=S->GetInlineFormulaSurface()->GetRollReelWidget()->GetPresentation();
	TestEqual(TEXT("Inline host reuses the existing projected center"),InlineReel->GetPresentation().CenterValue,SourceReel.CenterValue);
	TestEqual(TEXT("Inline host reuses the existing continuous clock"),InlineReel->GetPresentation().ContinuousPositionCells,SourceReel.ContinuousPositionCells);
	TestTrue(TEXT("Cycling retains the current RHS"),Visible(S,TEXT("TheaterAttackFinalNumber")));
	TestEqual(TEXT("Cycling RHS remains prior subtotal"),Label(S,TEXT("TheaterAttackFinalNumber")),BeforeFormula.AttackRow.DisplayedResultLabel);
	TestEqual(TEXT("Cycling label remains current"),Label(S,TEXT("TheaterAttackValueLabel")),FString(TEXT("当前值")));
	// Both sides preserve the 1.46s motion budget + .18s disclosure gap. The new
	// cosmetic fade cannot grant disclosure, unblock input or dim the other side.
	auto CheckFinalFade=[&](UFMCodexLocalMatchScreenWidget* W,const FString& Prefix)
	{
		auto* R=CastChecked<UFMCodexRollReelWidget>(W->GetWidgetFromName(FName(*(Prefix+TEXT("Reel")))));
		auto* RHS=W->GetWidgetFromName(FName(*(Prefix+TEXT("ResultColumn"))));
		W->AdvanceInlineFormulaRevealForTesting(1.07f); // .01 into original hold
		TestTrue(TEXT("Authoritative die settles before total disclosure"),R->GetPresentation().bStaticResult);
		TestEqual(TEXT("High Cross domain is D6"),R->GetPresentation().DomainMaximum,6);
		TestEqual(TEXT("Pre-disclosure progress is absent"),R->GetPresentation().FormulaFinalRevealProgress,-1.f);
		TestEqual(TEXT("Current label stays until original gate"),Label(W,*FString(Prefix+TEXT("ValueLabel"))),FString(TEXT("当前值")));
		W->AdvanceInlineFormulaRevealForTesting(.16f);
		TestEqual(TEXT("No premature total fade at .17s"),RHS->GetRenderOpacity(),1.f);
		W->AdvanceInlineFormulaRevealForTesting(.02f);
		TestEqual(TEXT("Label changes at existing disclosure gate"),Label(W,*FString(Prefix+TEXT("ValueLabel"))),FString(TEXT("最终值")));
		TestTrue(TEXT("Disclosed RHS and label begin restrained fade together"),RHS->GetRenderOpacity()>=.84f && RHS->GetRenderOpacity()<.9f);
		TestFalse(TEXT("Visual fade cannot unblock next action"),Visible(W,TEXT("TheaterPrimaryBounds")));
		W->AdvanceInlineFormulaRevealForTesting(.13f);
		TestEqual(TEXT("Final RHS returns to full opacity without new hold"),RHS->GetRenderOpacity(),1.f);
	};
	CheckFinalFade(S,TEXT("TheaterAttack"));
	F.Settle();
	TestEqual(TEXT("Revealed operand is the authoritative attack D6"),Label(S,TEXT("TheaterAttackRollValue")),FString(TEXT("6")));
	TestEqual(TEXT("Resolved total is projected without arithmetic"),Label(S,TEXT("TheaterAttackFinalNumber")),S->GetInlineFormulaSurface()->GetPresentation().AttackRow.DisplayedResultLabel);
	TestEqual(TEXT("Disclosed Attack label is final"),Label(S,TEXT("TheaterAttackValueLabel")),FString(TEXT("最终值")));
	TestEqual(TEXT("Unresolved Defense label is still current"),Label(S,TEXT("TheaterDefenseValueLabel")),FString(TEXT("当前值")));
	TestFalse(TEXT("Actor waits for defense without CTA"),Visible(S,TEXT("TheaterPrimaryBounds")));
	TestTrue(TEXT("Defender owns defense CTA"),Visible(D,TEXT("TheaterPrimaryBounds")));
	const FString AttackFinalBeforeDefense=Label(S,TEXT("TheaterAttackFinalNumber"));
	const FString ScoreBefore=Label(S,TEXT("TheaterContext")); F.Entropy->Word=0;
	CastChecked<UButton>(D->GetWidgetFromName(TEXT("TheaterContinue")))->OnClicked.Broadcast();
	TestEqual(TEXT("Defense uses original typed command"),F.Backend(Defender).Last.IntentKind,High?Kind::CrossHighDefenseRoll:Kind::CrossLowDefenseRoll);
	D->PauseInlineFormulaRevealTimerForTesting(); D->AdvanceInlineFormulaRevealForTesting(.4f);
	TestTrue(TEXT("Defense alone owns active v2 slot"),Visible(D,TEXT("TheaterDefenseReelHost")) && !Visible(D,TEXT("TheaterAttackReelHost")));
	TestEqual(TEXT("Attack remains resolved while Defense rolls"),Label(D,TEXT("TheaterAttackFinalNumber")),AttackFinalBeforeDefense);
	TestFalse(TEXT("Defense roll has no duplicate CTA"),Visible(D,TEXT("TheaterPrimaryBounds")));

	TestEqual(TEXT("Theater score obeys original reveal gate"),Label(S,TEXT("TheaterContext")),ScoreBefore);
	TestFalse(TEXT("Outcome hidden during reel"),Visible(S,TEXT("TheaterOutcome")));
	TestFalse(TEXT("Reason cannot disclose suppression during reel"),Label(S,TEXT("TheaterDetail")).Contains(TEXT("快速压制")));
	TestTrue(TEXT("Rich reason is empty before legal visible reveal"),
		CastChecked<URichTextBlock>(S->GetWidgetFromName(TEXT("TheaterReasonPrimary")))->GetText().IsEmpty());
	TestFalse(TEXT("Winner is not announced while defense is revealing"),
		Label(S,TEXT("TheaterAttackActive"))==TEXT("获胜") || Label(S,TEXT("TheaterDefenseActive"))==TEXT("获胜"));
	CheckFinalFade(D,TEXT("TheaterDefense"));
	TestEqual(TEXT("Resolved Attack stays fully readable during Defense fade"),D->GetWidgetFromName(TEXT("TheaterAttackResultColumn"))->GetRenderOpacity(),1.f);
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
		FMCodexResolutionTheaterPrototype::FTakerInspection Inspection;
		FMCodexResolutionTheaterPrototype::Refresh(*S->WidgetTree,S->GetPresentation(),Display,S->GetMatchHeader()->GetPresentation(),false,Inspection);
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
// Presentation fixtures deliberately exercise meaning, not translated substring matching.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTheaterHelperMeaningTest,
 "FMCodex.LocalPlay.ResolutionTheater.HelperMeaning",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTheaterHelperMeaningTest::RunTest(const FString&)
{
 auto* Tree=NewObject<UWidgetTree>(); UButton *Primary=nullptr,*High=nullptr,*Low=nullptr;
 // Build reads the existing Match Shell background brush.
 Tree->RootWidget=Tree->ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("MatchScreenStyleBackground"));
 Tree->RootWidget=FMCodexResolutionTheaterPrototype::Build(*Tree,Primary,High,Low,nullptr);
 FMCodexResolutionTheaterPrototype::FTakerInspection Inspection;
 FFMCodexUMGMatchScreenViewModel Screen; FFMCodexUMGMatchHeaderViewModel Header;
 FFMCodexUMGInlineFormulaSurfaceViewModel P;
 auto Apply=[&](bool Pending=false){FMCodexResolutionTheaterPrototype::Refresh(*Tree,Screen,P,Header,Pending,Inspection);};
 auto VisibleHelper=[&](){return Tree->FindWidget(TEXT("TheaterStatus"))->GetVisibility()==ESlateVisibility::SelfHitTestInvisible;};
 auto Text=[&](const TCHAR* Name){return CastChecked<UTextBlock>(Tree->FindWidget(Name))->GetText().ToString();};
 for (const auto Type:{ESetPieceSelectedType::ShortFreeKick,ESetPieceSelectedType::LongFreeKick})
 {
  Screen={}; P={}; Screen.SetPiece.bVisible=true; Screen.SetPiece.Type=Type;
  Screen.SetPiece.bTakerWait=true; Screen.SetPiece.TakerOptions.Add(TEXT("PresentationCandidate"));
  Screen.Interaction.ExpectedActorLabel=TEXT("请玩家 A 操作");
  Apply();
  const auto* A=CastChecked<UTextBlock>(Tree->FindWidget(TEXT("TheaterDetail")));
  const auto* B=CastChecked<UTextBlock>(Tree->FindWidget(TEXT("TheaterReasonSecondary")));
  TestTrue(TEXT("Near/Long taker rules have equal fonts"),A->GetFont()==B->GetFont());
  TestTrue(TEXT("Near/Long taker rules have equal primary brightness"),A->GetColorAndOpacity()==B->GetColorAndOpacity());
  TestTrue(TEXT("Selection retains operator information"),VisibleHelper() && !Text(TEXT("TheaterStatus")).IsEmpty());
  Screen.SetPiece.bTakerWait=false; Screen.SetPiece.bMethodWait=true; Apply();
  TestTrue(TEXT("Method selection retains operator information"),VisibleHelper());
  TestEqual(TEXT("Leaving selection restores supporting explanation size"),B->GetFont().Size,14.f);
  Screen.SetPiece.bMethodWait=false;
  Screen.SetPiece.NearMethod=EMatchPlayShortFreeKickMethod::Angled;
  Screen.SetPiece.LongMethod=EMatchPlayLongFreeKickMethod::Power;
  P.bVisible=true; P.bDiceRevealVisible=true; P.RollHelperLabel=TEXT("两枚骰子总和达到门槛进球");
  for (const auto Owner:{TEXT("第一枚掷点"),TEXT("第二枚掷点")})
  {
   P.DiceOwnerLabel=Owner; Apply();
   TestTrue(TEXT("Threshold roll retains additional die-sequence context"),VisibleHelper());
   TestEqual(TEXT("Die-sequence copy stays intact"),Text(TEXT("TheaterStatus")),FString(Owner));
   TestEqual(TEXT("Threshold explanation stays intact"),Text(TEXT("TheaterDetail")),P.RollHelperLabel);
  }
 }
 for (const auto Contest:{TEXT("Cross.High"),TEXT("Cross.Low"),TEXT("SetPiece.Short.Direct"),TEXT("SetPiece.Long.Direct")})
 {
  Screen={}; P={}; P.ContestId=Contest; P.bVisible=true; P.bShowFormulaRows=true;
  if (FString(Contest).StartsWith(TEXT("SetPiece.")))
  {Screen.SetPiece.bVisible=true;Screen.SetPiece.Type=FString(Contest).Contains(TEXT("Short"))?ESetPieceSelectedType::ShortFreeKick:ESetPieceSelectedType::LongFreeKick;}
  for (bool Attack:{true,false})
  {
   P.bAttackRowActive=Attack; P.bDefenseRowActive=!Attack; P.bDiceRevealVisible=true;
   P.DiceOwnerLabel=TEXT("localized roll-owner label"); Apply();
   TestEqual(TEXT("Role-only duplicate is Hidden, never Collapsed"),Tree->FindWidget(TEXT("TheaterStatus"))->GetVisibility(),ESlateVisibility::Hidden);
   TestEqual(TEXT("Primary rolling status remains"),Text(TEXT("TheaterDetail")),FString(Attack?TEXT("进攻方掷点中"):TEXT("防守方掷点中")));
   Apply(true); TestTrue(TEXT("Pending ACK adds information and stays visible"),VisibleHelper());
   P.bDiceRevealVisible=false; Screen.bMirrorActionWaitPrompt=true;
   Screen.ActionWaitActorText=FText::FromString(TEXT("等待玩家 B 操作")); Screen.ActionWaitActionText=FText::FromString(TEXT("掷点")); Apply();
   TestTrue(TEXT("Waiting viewer retains identity and expected action"),VisibleHelper() && Text(TEXT("TheaterStatus")).Contains(TEXT("等待玩家 B")));
   Screen.bMirrorActionWaitPrompt=false; P.PrimaryAction.bVisible=true; P.PrimaryAction.Action.bAvailable=true;
   Screen.Interaction.ExpectedActorLabel=TEXT("请玩家 B 操作"); Apply();
   TestTrue(TEXT("Legal operator remains visible before roll"),VisibleHelper() && !Text(TEXT("TheaterStatus")).IsEmpty());
   P.bNarrativeAvailable=true; P.ResolutionReasonLabel=TEXT("已揭示结果\n原因说明"); Apply();
   TestTrue(TEXT("Outcome retains operator"),VisibleHelper());
   TestEqual(TEXT("Outcome retains reveal-safe reason"),Text(TEXT("TheaterReasonSecondary")),FString(TEXT("原因说明")));
   P.bNarrativeAvailable=false; P.PrimaryAction={};
  }
 }
 Screen={};P={};P.bVisible=true;P.ContestId=TEXT("Cross.Route");P.bDiceRevealVisible=true;P.DiceOwnerLabel=TEXT("路线掷点");Apply();
 TestTrue(TEXT("Independent route context is retained"),VisibleHelper());
 return true;
}

#endif
