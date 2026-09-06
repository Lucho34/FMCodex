#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"
#include "FMCodexNetworkMatchScreenActions.h"
#include "../LocalPlay/FMCodexLocalMatchScreenWidget.h"
#include "../LocalPlay/FMCodexMatchHeaderWidget.h"
#include "../LocalPlay/FMCodexInteractionPanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "../LocalPlay/FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "../LocalPlay/FMCodexSelectionFeedbackToastWidget.h"
#include "../LocalPlay/FMCodexLocalMatchPlayerController.h"
#include "../LocalPlay/FMCodexLocalMatchResolutionFeedback.h"

namespace FMCodexPlayerFacingCrossUITests
{
	using namespace FMCodexNetworkInitialRouteTests;
	using ScreenIntent = EFMCodexMatchScreenIntent;
	using Category = EFMCodexUMGInteractionCategory;
	using Submission = EFMCodexMatchScreenSubmission;

	// Test transport harness. It exercises the exact production screen/action adapter;
	// generated RPC callspace and natural OnRep are independently tested in two real processes.
	struct FBackend : IFMCodexMatchScreenBackend
	{
		FFixture* F = nullptr;
		AFMCodexNetworkMatchPlayerController* PC = nullptr;
		int32 Sends = 0;
		Envelope Last;
		Code LastCode = Code::None;
		virtual bool IsScreenIntentPending() const override { return F->Client(PC).IsPending(); }
		virtual Submission SubmitScreenIntent(const FFMCodexMatchScreenRequest& Request) override
		{
			if (!FFMCodexNetworkMatchScreenActions::Begin(Request, PC->GetOwnerView(), F->Client(PC), Last)) return Submission::Rejected;
			++Sends;
			const auto Ack = F->Mode->SubmitConnectionPlayerIntent(PC, Last);
			LastCode = Ack.Code;
			F->Client(PC).ObserveView(PC->GetOwnerView()); F->Client(PC).ObserveAck(Ack);
			PC->RefreshPlayerFacingUI();
			return Submission::Queued;
		}
	};
	struct FUIFixture : FFixture
	{
		FBackend BA, BB;
		TSharedPtr<SWidget> SlateA, SlateB;
		FUIFixture(bool BFirst = false, bool Final = false) : FFixture(BFirst, 6, Final)
		{
			Access::Runtime(*Mode).EnablePlayerFacingPresentation();
			Access::Publish(*Mode);
			BA.F = BB.F = this; BA.PC = A; BB.PC = B;
			if (A->GetPlayerMatchScreen()) SlateA = A->GetPlayerMatchScreen()->TakeWidget();
			if (B->GetPlayerMatchScreen()) SlateB = B->GetPlayerMatchScreen()->TakeWidget();
			A->RefreshPlayerFacingUI(); B->RefreshPlayerFacingUI();
			if (A->GetPlayerMatchScreen()) A->GetPlayerMatchScreen()->SetMatchBackend(&BA);
			if (B->GetPlayerMatchScreen()) B->GetPlayerMatchScreen()->SetMatchBackend(&BB);
		}
		~FUIFixture()
		{
			if (A->GetPlayerMatchScreen()) A->GetPlayerMatchScreen()->SetMatchBackend(nullptr);
			if (B->GetPlayerMatchScreen()) B->GetPlayerMatchScreen()->SetMatchBackend(nullptr);
		}
		FBackend& Backend(AFMCodexNetworkMatchPlayerController* PC) { return PC == A ? BA : BB; }
		void Settle()
		{
			for (auto* PC : {A, B}) if (auto* S = PC->GetPlayerMatchScreen())
			{
				S->PauseInlineFormulaRevealTimerForTesting();
				S->AdvanceInlineFormulaRevealForTesting(20.f);
			}
		}
		bool SkillFixture(bool Final = false)
		{
			if (Final)
			{
				Entropy->Word = 0;
				if (!Send(Attacker(), Kind::RequestInitialActionPointRoll)) return false;
				FMatchPlayAuthoritativeAdvanceAfterTerminalRequest R;
				R.AttackSequence = Attacker()->GetOwnerView().AttackSequence; R.RequestingSide = Attacker()->GetOwnerView().ViewerSide;
				if (!Access::Runtime(*Mode).SubmitPlayerIntent(FMatchPlayPlayerIntent::Create(
					EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal, R)).bSuccess) return false;
				Access::Publish(*Mode);
			}
			Entropy->Word = 5;
			if (!Access::Runtime(*Mode).PrepareInitialRouteMilestone(ESkillRuleType::Cross)) return false;
			Access::Publish(*Mode); Settle(); return true;
		}
	};
	void CheckPrompt(FAutomationTestBase& T, UFMCodexLocalMatchScreenWidget* S,
		const FFMCodexNetworkClientViewSnapshot& V, const FString& Action, bool bPending = false)
	{
		auto* Panel = S->GetInteractionPanel();
		const bool Acting = V.ExpectedActingSide == V.ViewerSide;
		T.TestEqual(TEXT("Original actor identity retained for existing player accent style"),S->GetPresentation().Interaction.ExpectedActorLabel,V.Presentation.Interaction.ExpectedActorLabel);
		const FString Owner = V.ExpectedActingSide == Side::PlayerA ? TEXT("玩家 A") : TEXT("玩家 B");
		const FString ExpectedActor = Acting ? bPending ? TEXT("正在提交，请稍候") : TEXT("轮到你操作")
			: FString(TEXT("等待")) + Owner + TEXT(" 操作");
		const FString ExpectedAction = Acting ? Action : Action == TEXT("下一回合")
			? FString(TEXT("等待下一回合推进")) : FString(TEXT("等待")) + Action;
		T.TestTrue(TEXT("Mirror prompt uses existing shared dock"), S->GetPresentation().bMirrorActionWaitPrompt);
		T.TestEqual(TEXT("Dock visible after the presentation handoff"),Panel->GetVisibility(),ESlateVisibility::Visible);
		const auto* ActorText = Cast<UTextBlock>(Panel->GetWidgetFromName(TEXT("InteractionExpectedActor")));
		const auto* ActionText = Cast<UTextBlock>(Panel->GetWidgetFromName(TEXT("InteractionActionTitle")));
		T.TestEqual(TEXT("Rendered actor is viewer-relative to safe actor"),ActorText->GetText().ToString(),ExpectedActor);
		T.TestEqual(TEXT("Rendered action matches current safe step"),ActionText->GetText().ToString(),ExpectedAction);
		T.TestTrue(TEXT("Action explanation visible even for active D12"),ActionText->GetVisibility()!=ESlateVisibility::Collapsed);
		T.TestEqual(TEXT("No generic no-player-action fallback"),Panel->GetWidgetFromName(TEXT("InteractionBoundedFallback"))->GetVisibility(),ESlateVisibility::Collapsed);
		const bool Central = Action == TEXT("进攻方掷点") || Action == TEXT("防守方掷点")
			|| Action == TEXT("掷传中路线骰") || Action == TEXT("下一回合") || Action == TEXT("选择传中方式");
		if (!Acting || Central)
		{
			for (const TCHAR* Name : {TEXT("InteractionContinueButton"),TEXT("InteractionTacticalPointRollButton"),TEXT("InteractionFinishDeploymentButton")})
				if (const auto* Button=Panel->GetWidgetFromName(Name))
					T.TestEqual(TEXT("Read-only status has no duplicate lower CTA"),Button->GetVisibility(),ESlateVisibility::Collapsed);
		}
		if (!Acting)
		{
			const auto& M=S->GetPresentation();
			T.TestFalse(TEXT("Waiting viewer has no primary action capability"),M.Interaction.PrimaryAction.bAvailable);
			T.TestFalse(TEXT("Waiting viewer has no central Formula CTA"),M.InlineFormula.PrimaryAction.bVisible);
			T.TestTrue(TEXT("Waiting viewer has no actionable selection"),M.Interaction.SelectionChoices.IsEmpty() && M.Interaction.BranchChoices.IsEmpty());
		}
	}
	void CheckBothPrompts(FAutomationTestBase& T,FUIFixture& F,const FString& Action)
	{
		for (auto* PC:{F.A,F.B}) CheckPrompt(T,PC->GetPlayerMatchScreen(),PC->GetOwnerView(),Action);
	}
	bool Continue(FAutomationTestBase& T, FUIFixture& F, AFMCodexNetworkMatchPlayerController* PC, Kind Expected)
	{
		const int32 Before = F.Backend(PC).Sends;
		PC->GetPlayerMatchScreen()->RequestContinueResolution();
		return T.TestEqual(TEXT("Shared central callback sends once"), F.Backend(PC).Sends, Before + 1)
			&& T.TestEqual(TEXT("Exact existing typed intent"), F.Backend(PC).Last.IntentKind, Expected)
			&& T.TestEqual(TEXT("Connection-side validation accepts"), F.Backend(PC).LastCode, Code::Accepted);
	}
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPlayerFacingCrossGolden,
	"FMCodex.NetworkPlay.PlayerFacingCrossUI.01.GoldenPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexPlayerFacingCrossGolden::GetTests(TArray<FString>& N, TArray<FString>& C) const
{
	for (const TCHAR* A : {TEXT("A"),TEXT("B")}) for (const TCHAR* R : {TEXT("High"),TEXT("Low")})
		for (const TCHAR* G : {TEXT("Goal"),TEXT("NoGoal")}) for (const TCHAR* F : {TEXT("Next"),TEXT("Final")})
		{ auto S = FString::Printf(TEXT("%s.%s.%s.%s"), A,R,G,F); N.Add(S); C.Add(S); }
}
bool FFMCodexPlayerFacingCrossGolden::RunTest(const FString& P)
{
	using namespace FMCodexPlayerFacingCrossUITests;
	TArray<FString> Parts; P.ParseIntoArray(Parts,TEXT("."));
	const bool BFirst=Parts[0]==TEXT("B"), High=Parts[1]==TEXT("High"), Goal=Parts[2]==TEXT("Goal"), Final=Parts[3]==TEXT("Final");
	FUIFixture F(BFirst != Final, Final);
	if (!TestTrue(TEXT("Canonical fixture stops at Skill"), F.SkillFixture(Final))) return false;
	auto* Actor=F.Attacker(); auto* Defender=F.Defender();
	auto* S=Actor->GetPlayerMatchScreen(); auto* D=Defender->GetPlayerMatchScreen();
	if (!TestNotNull(TEXT("Attacker uses shared screen"),S) || !TestNotNull(TEXT("Defender uses shared screen"),D)) return false;
	TestTrue(TEXT("Both reuse exact Local screen class"), S->GetClass()==UFMCodexLocalMatchScreenWidget::StaticClass() && D->GetClass()==S->GetClass());
	for (auto* PC : {F.A,F.B})
	{
		const auto& V=PC->GetOwnerView(); const auto& M=PC->GetPlayerMatchScreen()->GetPresentation();
		TestTrue(TEXT("Same revision carries player-facing safe presentation"),V.Presentation.bAvailable);
		TestFalse(TEXT("Network has no Local Start"),M.Interaction.bCanStartNewMatch);
		TestEqual(TEXT("Fixed viewer orientation"),M.Header.LeftPlayerSide,V.ViewerSide);
		TestEqual(TEXT("Public full local hand"),M.LocalRack.Cells.Num(),20);
		TestEqual(TEXT("Public full opponent hand"),M.OpponentRack.Cells.Num(),20);
		TestTrue(TEXT("Pitch projection present"),!M.PitchRegions.IsEmpty());
		TestFalse(TEXT("Declines have no false network capability"),M.Interaction.bCanDecline);
		for (const auto& Cell:M.LocalRack.Cells) if (!Cell.Card.CardId.IsNone())
		{
			TestFalse(TEXT("Chinese player name hydrated"),Cell.Card.IdentityLabel.IsEmpty());
			TestFalse(TEXT("No developer ID text on card"),!Cell.Card.DeveloperReferenceLabel.IsEmpty());
		}
	}
	CheckBothPrompts(*this,F,TEXT("选择战术"));
	const auto& Choices=S->GetPresentation().Interaction.SelectionChoices;
	const auto* Cross=Choices.FindByPredicate([](const auto& C){return C.SkillType==ESkillRuleType::Cross;});
	if (!TestNotNull(TEXT("Safe Cross choice exists"),Cross)) return false;
	const FName SkillId=Cross->OptionId;
	S->RequestSubmitSkill(SkillId);
	TestEqual(TEXT("Skill callback maps existing intent"),F.Backend(Actor).Last.IntentKind,Kind::SubmitSkill);
	TestEqual(TEXT("Skill accepted"),F.Backend(Actor).LastCode,Code::Accepted);
	TestEqual(TEXT("Branch remains player intent"),S->GetPresentation().Interaction.Category,Category::SelectBranchIntent);
	CheckBothPrompts(*this,F,TEXT("选择传中方式"));
	S->RequestSubmitBranchIntent(High ? EFMCodexUMGBranchIntent::CrossLow : EFMCodexUMGBranchIntent::CrossHigh);
	TestEqual(TEXT("Branch callback maps existing intent"),F.Backend(Actor).Last.IntentKind,Kind::SubmitBranchIntent);
	CheckBothPrompts(*this,F,TEXT("掷传中路线骰"));
	const auto BeforeRoute = Actor->GetOwnerView().Presentation;
	F.Entropy->Word=4;
	if (!Continue(*this,F,Actor,Kind::CrossInitialRouteRoll)) return false;
	const auto AfterRoute = Actor->GetOwnerView().Presentation;
	TestTrue(TEXT("Pending formula rows remain available on both sides"),Actor->GetOwnerView().Presentation.InlineFormula.bVisible && Defender->GetOwnerView().Presentation.InlineFormula.bVisible);
	TestEqual(TEXT("Only accepted route event before attack"),Actor->GetOwnerView().Presentation.ResolvedRolls.Num(),1);
	TestTrue(TEXT("Actor route Reel active"),S->IsInlineFormulaRevealInputBlocked());
	TestTrue(TEXT("Nonacting viewer observes same route Reel"),D->IsInlineFormulaRevealInputBlocked());
	F.Settle();
	CheckBothPrompts(*this,F,TEXT("进攻方掷点"));
	const auto RoutePhase=S->GetInlineFormulaRevealPhase();
	Actor->RefreshPlayerFacingUI(); Defender->RefreshPlayerFacingUI();
	TestEqual(TEXT("Repeated revision does not replay route"),S->GetInlineFormulaRevealPhase(),RoutePhase);
	F.Entropy->Word=Goal?5:0;
	if (!Continue(*this,F,Actor,High?Kind::CrossHighAttackRoll:Kind::CrossLowAttackRoll)) return false;
	TestEqual(TEXT("Accepted route and Attack only; no future defense"),Actor->GetOwnerView().Presentation.ResolvedRolls.Num(),2);
	TestTrue(TEXT("Attack Reel active on both viewers"),S->IsInlineFormulaRevealInputBlocked() && D->IsInlineFormulaRevealInputBlocked());
	for(auto* Screen:{S,D}) TestEqual(TEXT("Future prompt waits for Reel and Narrative timeline"),Screen->GetInteractionPanel()->GetVisibility(),ESlateVisibility::Collapsed);
	F.Settle();
	CheckBothPrompts(*this,F,TEXT("防守方掷点"));
	const auto BeforeScore=S->GetMatchHeader()->GetPresentation();
	F.Entropy->Word=Goal?0:5;
	if (!Continue(*this,F,Defender,High?Kind::CrossHighDefenseRoll:Kind::CrossLowDefenseRoll)) return false;
	TestTrue(TEXT("Defense Reel active on both viewers"),S->IsInlineFormulaRevealInputBlocked() && D->IsInlineFormulaRevealInputBlocked());
	for(auto* Screen:{S,D}) TestEqual(TEXT("Future prompt waits for Reel and Narrative timeline"),Screen->GetInteractionPanel()->GetVisibility(),ESlateVisibility::Collapsed);
	TestEqual(TEXT("Persisted safe terminal uses canonical result"),Actor->GetOwnerView().CrossTerminal.Outcome,
		Goal?EFMCodexNetworkTerminalOutcome::Goal:EFMCodexNetworkTerminalOutcome::NoGoal);
	TestEqual(TEXT("Header A holds already-painted score during defense Reel"),
		S->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,BeforeScore.PlayerAScoreLabel);
	TestEqual(TEXT("Header B holds already-painted score during defense Reel"),
		S->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,BeforeScore.PlayerBScoreLabel);
	TestFalse(TEXT("Narrative waits for reveal"),S->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
	F.Settle();
	CheckBothPrompts(*this,F,TEXT("下一回合"));
	const auto& Formula=S->GetInlineFormulaSurface()->GetPresentation();
	TestTrue(TEXT("Existing Formula receives safe facts"),Formula.bVisible);
	TestTrue(TEXT("Existing Narrative disclosed once"),Formula.bNarrativeAvailable);
	TestEqual(TEXT("Visible A score reaches safe score after reveal"),S->GetMatchHeader()->GetPresentation().PlayerAScoreLabel,S->GetPresentation().Header.PlayerAScoreLabel);
	TestEqual(TEXT("Visible B score reaches safe score after reveal"),S->GetMatchHeader()->GetPresentation().PlayerBScoreLabel,S->GetPresentation().Header.PlayerBScoreLabel);
	const auto Settled=S->GetInlineFormulaRevealPhase();
	Actor->RefreshPlayerFacingUI(); Defender->RefreshPlayerFacingUI();
	TestEqual(TEXT("Repeated terminal does not replay"),S->GetInlineFormulaRevealPhase(),Settled);
	CheckBothPrompts(*this,F,TEXT("下一回合"));
	TestEqual(TEXT("Exactly persisted public history"),Actor->GetOwnerView().PublicGoalHistory.Num(),int32(Goal));
	if (Goal) TestFalse(TEXT("Chinese canonical scorer label"),Actor->GetOwnerView().CrossTerminal.Goal.ScorerLabel.IsEmpty());
	else TestTrue(TEXT("NoGoal invents no scorer"),Actor->GetOwnerView().CrossTerminal.Goal.ScorerCardId.IsNone());
	// Late UI construction reads latest safe terminal without replaying an unobserved historic roll.
	auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());
	const auto LateSlate = Late->TakeWidget();
	Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Actor->GetOwnerView(),false));
	TestFalse(TEXT("Late screen has no historical replay"),Late->IsInlineFormulaRevealInputBlocked());
	TestTrue(TEXT("Late screen displays latest terminal"),Late->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
	CheckPrompt(*this,Late,Actor->GetOwnerView(),TEXT("下一回合"));
	Late->ResetPresentationSession();
	Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeRoute,false));
	Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(Actor->GetOwnerView().Presentation,false));
	Late->PauseInlineFormulaRevealTimerForTesting();
	Late->AdvanceInlineFormulaRevealForTesting(3.2f);
	TestEqual(TEXT("Coalesced prefix continues with Attack Reel"),Late->GetInlineFormulaSurface()->GetPresentation().DiceOwnerLabel,FString(TEXT("进攻方掷点")));
	TestFalse(TEXT("Coalesced Attack cannot reveal terminal narrative"),Late->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
	Late->AdvanceInlineFormulaRevealForTesting(4.2f);
	TestEqual(TEXT("Coalesced prefix continues with Defense Reel"),Late->GetInlineFormulaSurface()->GetPresentation().DiceOwnerLabel,FString(TEXT("防守方掷点")));
	Late->AdvanceInlineFormulaRevealForTesting(5.f);
	TestTrue(TEXT("Coalesced terminal finally discloses"),Late->GetInlineFormulaSurface()->GetPresentation().bNarrativeAvailable);
	const auto OldSequence=Actor->GetOwnerView().AttackSequence;
	if (!Continue(*this,F,Actor,Kind::AdvanceAfterTerminal)) return false;
	for (auto* PC : {F.A,F.B})
	{
		const auto& M=PC->GetPlayerMatchScreen()->GetPresentation();
		TestEqual(TEXT("MatchEnded reaches reused Full-Time"),M.FullTime.bVisible,Final);
		if(Final) { TestFalse(TEXT("MatchEnded clears mirror status"),M.bMirrorActionWaitPrompt); TestEqual(TEXT("No stale Advance dock"),PC->GetPlayerMatchScreen()->GetInteractionPanel()->GetVisibility(),ESlateVisibility::Collapsed); }
		TestFalse(TEXT("Advance clears old Formula"),M.InlineFormula.bVisible);
		TestTrue(TEXT("Advance clears stale Reel"),Final
			? PC->GetPlayerMatchScreen()->GetInlineFormulaRevealPhase()==EFMCodexUMGInlineFormulaRevealPhase::None
			: !PC->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
	}
	if (!Final)
	{
		CheckBothPrompts(*this,F,TEXT("掷战术点"));
		auto* Next=F.Attacker();
		TestTrue(TEXT("Control moves to next attacker"),Next==Defender);
		TestTrue(TEXT("Next Full D12 belongs to new attacker"),Next->GetPlayerMatchScreen()->GetPresentation().Interaction.bCanRollTacticalPoints);
		Next->GetPlayerMatchScreen()->RequestRollTacticalPoints();
		TestEqual(TEXT("Full D12 callback maps typed entry intent"),F.Backend(Next).Last.IntentKind,Kind::RequestInitialActionPointRoll);
		TestTrue(TEXT("New attack sequence advances"),Next->GetOwnerView().AttackSequence>OldSequence);
		TestTrue(TEXT("Next attack D12 animates"),Next->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
		F.Settle();
		// Same event data is a presentation-only dedupe fixture, scoped to the actual new attack sequence.
		auto PendingAgain=BeforeRoute, ResolvedAgain=AfterRoute;
		const auto NewSequence=Next->GetOwnerView().AttackSequence;
		PendingAgain.Header.AttackSequence=ResolvedAgain.Header.AttackSequence=NewSequence;
		for(auto& E:ResolvedAgain.ResolvedRolls) E.AttackSequence=NewSequence;
		Late->ResetPresentationSession();
		Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(BeforeRoute,false));
		Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(AfterRoute,false));
		Late->AdvanceInlineFormulaRevealForTesting(20.f);
		Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(PendingAgain,false));
		Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(ResolvedAgain,false));
		TestTrue(TEXT("Same route purpose can animate in the next attack"),Late->IsInlineFormulaRevealInputBlocked());
	}
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPlayerFacingCrossPrelude,
	"FMCodex.NetworkPlay.PlayerFacingCrossUI.02.DeploymentAndRoles",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexPlayerFacingCrossPrelude::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ N={TEXT("A"),TEXT("B")}; C=N; }
bool FFMCodexPlayerFacingCrossPrelude::RunTest(const FString& P)
{
	using namespace FMCodexPlayerFacingCrossUITests;
	FUIFixture F(P==TEXT("B")); auto* A=F.Attacker(); auto* D=F.Defender();
	if (!TestNotNull(TEXT("Shared entry screen"),A->GetPlayerMatchScreen())) return false;
	CheckBothPrompts(*this,F,TEXT("掷战术点"));
	A->GetPlayerMatchScreen()->RequestRollTacticalPoints(); F.Settle();
	CheckBothPrompts(*this,F,TEXT("部署球员并完成部署"));
	TestEqual(TEXT("D12 uses entry intent"),F.Backend(A).Last.IntentKind,Kind::RequestInitialActionPointRoll);
	const bool IsA=A==F.A;
	const FString Half=IsA?TEXT("NearA"):TEXT("NearB"),Other=IsA?TEXT("NearB"):TEXT("NearA");
	const FName Carrier=IsA?FName(TEXT("Prototype.Arsenal.BukayoSaka")):FName(TEXT("Prototype.ManchesterCity.JeremyDoku"));
	const FName Runner=IsA?FName(TEXT("Prototype.Arsenal.KaiHavertz")):FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
	auto Deploy=[&](auto* PC,const FString& H,FName Card=NAME_None)
	{
		const auto Choice=F.Choice(PC,H,Card);
		PC->GetPlayerMatchScreen()->RequestDeployOrdinary(Choice.CardId,Choice.SlotId);
		return TestEqual(TEXT("Deploy gesture maps exact pair"),F.Backend(PC).Last.Deployment.CardId,Choice.CardId)
			&& TestEqual(TEXT("Deployment accepted"),F.Backend(PC).LastCode,Code::Accepted);
	};
	if (!Deploy(A,Half,Carrier)||!Deploy(D,Half)||!Deploy(A,Other,Runner)||!Deploy(D,Other)) return false;
	A->GetPlayerMatchScreen()->RequestFinishDeployment();
	TestEqual(TEXT("Finish maps existing intent"),F.Backend(A).Last.IntentKind,Kind::FinishDeployment);
	if (!Deploy(D,Other)) return false;
	const FName GK=D->GetOwnerView().GoalkeeperOption.Choice.SlotId;
	D->GetPlayerMatchScreen()->RequestDeployGoalkeeper(GK);
	TestEqual(TEXT("GK sends slot only"),F.Backend(D).Last.Goalkeeper.SlotId,GK);
	TestEqual(TEXT("GK accepted"),F.Backend(D).LastCode,Code::Accepted);
	D->GetPlayerMatchScreen()->RequestFinishDeployment();
	CheckBothPrompts(*this,F,TEXT("选择持球球员"));
	A->GetPlayerMatchScreen()->RequestSubmitCarrier(Carrier);
	TestEqual(TEXT("Carrier typed mapping"),F.Backend(A).Last.IntentKind,Kind::SubmitCarrier);
	if (!TestFalse(TEXT("Server offers Marker"),D->GetOwnerView().MarkerOptions.IsEmpty())) return false;
	CheckBothPrompts(*this,F,TEXT("选择盯人球员"));
	D->GetPlayerMatchScreen()->RequestSubmitMarker(D->GetOwnerView().MarkerOptions[0].Choice.MarkerCardId);
	TestEqual(TEXT("Marker typed mapping"),F.Backend(D).Last.IntentKind,Kind::SubmitMarker);
	CheckBothPrompts(*this,F,TEXT("选择跑位球员"));
	A->GetPlayerMatchScreen()->RequestSubmitRunner(Runner);
	TestEqual(TEXT("Runner typed mapping"),F.Backend(A).Last.IntentKind,Kind::SubmitRunner);
	if (!TestFalse(TEXT("Server offers Helper"),D->GetOwnerView().HelperOptions.IsEmpty())) return false;
	CheckBothPrompts(*this,F,TEXT("选择协防球员"));
	D->GetPlayerMatchScreen()->RequestSubmitHelper(D->GetOwnerView().HelperOptions[0].Choice.HelperCardId);
	TestEqual(TEXT("Helper typed mapping"),F.Backend(D).Last.IntentKind,Kind::SubmitHelper);
	TestEqual(TEXT("All callbacks reach genuine Skill wait"),A->GetOwnerView().EntryWait,EFMCodexNetworkEntryWait::SkillSelection);
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPlayerFacingCrossAsync,
	"FMCodex.NetworkPlay.PlayerFacingCrossUI.03.Async",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexPlayerFacingCrossAsync::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ N={TEXT("AckFirst"),TEXT("ViewFirst"),TEXT("Reject"),TEXT("NewMatch")}; C=N; }
bool FFMCodexPlayerFacingCrossAsync::RunTest(const FString& P)
{
	using namespace FMCodexPlayerFacingCrossUITests;
	FUIFixture F;
	if (!TestTrue(TEXT("Canonical Skill fixture"),F.SkillFixture())) return false;
	auto* PC=F.Attacker(); auto V=PC->GetOwnerView();
	FFMCodexNetworkIntentClientState Client; Envelope E,Duplicate;
	FFMCodexMatchScreenRequest R; R.Kind=ScreenIntent::Skill; R.OptionId=TEXT("Canonical.Skill.Cross.4.6");
	if (!TestTrue(TEXT("Shared action queues once"),FFMCodexNetworkMatchScreenActions::Begin(R,V,Client,E))) return false;
	TestTrue(TEXT("Pending is transport state"),Client.IsPending());
	TestFalse(TEXT("Duplicate gesture cannot send"),FFMCodexNetworkMatchScreenActions::Begin(R,V,Client,Duplicate));
	const auto Pending=FFMCodexNetworkMatchPresentationAdapter::Read(V,true);
	TestEqual(TEXT("Pending actor prompt explains submission"),Pending.ActionWaitActorText.ToString(),FString(TEXT("正在提交，请稍候")));
	TestTrue(TEXT("Pending retains cards and pitch"),!Pending.LocalRack.Cells.IsEmpty()&&!Pending.PitchRegions.IsEmpty());
	TestTrue(TEXT("Pending removes actionable selection"),Pending.Interaction.SelectionChoices.IsEmpty());
	const auto State=Access::Session(*F.Mode).GetStateSnapshot();
	if (P==TEXT("Reject"))
	{
		E.Skill.SkillId=TEXT("Invalid.Skill");
		const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
		TestTrue(TEXT("Server rejects fabricated choice"),Ack.Code!=Code::Accepted);
		Client.ObserveAck(Ack);
		TestFalse(TEXT("Rejection releases pending without view advance"),Client.IsPending());
		TestTrue(TEXT("Rejected state unchanged"),SameState(State,Access::Session(*F.Mode).GetStateSnapshot()));
		PC->GetPlayerMatchScreen()->NotifyScreenRequestRejected();
		TestFalse(TEXT("Reject cancels optimistic wait"),PC->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
		CheckBothPrompts(*this,F,TEXT("选择战术"));
		TestFalse(TEXT("Latest options recover"),FFMCodexNetworkMatchPresentationAdapter::Read(V,Client.IsPending()).Interaction.SelectionChoices.IsEmpty());
	}
	else if (P==TEXT("NewMatch"))
	{
		V.MatchInstanceId=FGuid::NewGuid(); Client.ObserveView(V);
		TestFalse(TEXT("New match clears pending epoch"),Client.IsPending());
		PC->GetPlayerMatchScreen()->ResetPresentationSession();
		PC->GetPlayerMatchScreen()->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(V,false));
		TestFalse(TEXT("New match clears reveal identity cache"),PC->GetPlayerMatchScreen()->IsInlineFormulaRevealInputBlocked());
	}
	else
	{
		const auto Ack=F.Mode->SubmitConnectionPlayerIntent(PC,E);
		TestEqual(TEXT("Typed request accepted"),Ack.Code,Code::Accepted);
		if (P==TEXT("AckFirst")) Client.ObserveAck(Ack); else Client.ObserveView(PC->GetOwnerView());
		TestTrue(TEXT("First half cannot claim synchronous success"),Client.IsPending());
		if (P==TEXT("AckFirst")) Client.ObserveView(PC->GetOwnerView()); else Client.ObserveAck(Ack);
		TestFalse(TEXT("Correlated ACK plus view releases pending"),Client.IsPending());
		CheckBothPrompts(*this,F,TEXT("选择传中方式"));
		Client.ObserveAck(Ack);Client.ObserveView(V);
		TestFalse(TEXT("Repeated ACK and old view do not re-pend"),Client.IsPending());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexPlayerFacingCrossBoundary,
	"FMCodex.NetworkPlay.PlayerFacingCrossUI.04.SafeContractAndCapabilities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexPlayerFacingCrossBoundary::RunTest(const FString&)
{
	using namespace FMCodexPlayerFacingCrossUITests;
	FUIFixture F;
	if (!TestTrue(TEXT("Skill fixture"),F.SkillFixture())) return false;
	auto* PC=F.Attacker(); const auto& V=PC->GetOwnerView();
	// Storage roundtrip checks the exact value contract (the real smoke covers UE network replication).
	TArray<uint8> Bytes; FMemoryWriter W(Bytes); FObjectAndNameAsStringProxyArchive Save(W,false);
	auto Copy=V.Presentation;
	FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Save,&Copy,nullptr);
	FFMCodexNetworkMatchPresentation Read;
	FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,false);
	FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Load,&Read,nullptr);
	TestTrue(TEXT("Safe value-only presentation roundtrips"),FFMCodexNetworkMatchPresentation::StaticStruct()->CompareScriptStruct(&Copy,&Read,0));
	AddInfo(FString::Printf(TEXT("Presentation storage bytes=%d; unique card identities=%d"),Bytes.Num(),Copy.CardCatalog.Num()));
	TestTrue(TEXT("Public catalog bounded"),Copy.CardCatalog.Num()<=40);
	TSet<FName> Ids;
	for (const auto& C:Copy.CardCatalog) { TestFalse(TEXT("Card static data appears once"),Ids.Contains(C.CardId));Ids.Add(C.CardId); }
	TestNull(TEXT("No raw State"),FFMCodexNetworkMatchPresentation::StaticStruct()->FindPropertyByName(TEXT("MatchPlayState")));
	TestNull(TEXT("No raw InteractionView"),FFMCodexNetworkMatchPresentation::StaticStruct()->FindPropertyByName(TEXT("InteractionView")));
	TestNull(TEXT("No RNG"),FFMCodexNetworkMatchPresentation::StaticStruct()->FindPropertyByName(TEXT("RandomStream")));
	for (ScreenIntent Unsupported:{ScreenIntent::StartMatch,ScreenIntent::Decline,ScreenIntent::NoLegal,ScreenIntent::OneOnOne})
	{
		FFMCodexMatchScreenRequest R;R.Kind=Unsupported;Envelope E;FFMCodexNetworkIntentClientState C;
		TestFalse(TEXT("Unsupported shared gesture has no network capability"),FFMCodexNetworkMatchScreenActions::Begin(R,V,C,E));
	}
	for (const auto& O:V.Presentation.Interaction.SelectionChoices)
	{
		TestEqual(TEXT("Only Cross capability enabled; legality retained"),O.bEnabled,O.SkillType==ESkillRuleType::Cross);
		if (!O.bEnabled) TestFalse(TEXT("Disabled option explains scope"),O.SecondaryLabel.IsEmpty());
	}
	// Withhold an already accepted Attack: no event or derived Formula may encode it.
	F.Entropy->Word=4;
	F.A->GetPlayerMatchScreen()->RequestSubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
	F.A->GetPlayerMatchScreen()->RequestSubmitBranchIntent(EFMCodexUMGBranchIntent::CrossLow);
	F.A->GetPlayerMatchScreen()->RequestContinueResolution(); F.Settle();
	F.Entropy->Word=5; F.A->GetPlayerMatchScreen()->RequestContinueResolution(); F.Settle();
	FFMCodexLocalMatchViewerDisclosure Hidden;
	Hidden.bRevealInitialActionPointRoll=true;Hidden.bRevealRouteRoll=true;
	Hidden.bPreservePendingCrossFormula=true;Hidden.RevealedContestD6Count=0;
	const auto HiddenSafe=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(
		Access::Session(*F.Mode).GetStateSnapshot(),Access::CallerRules(*F.Mode),Side::PlayerB,Hidden);
	const auto HiddenProjection=FFMCodexNetworkMatchPresentationAdapter::Project(HiddenSafe,Side::PlayerB);
	TestEqual(TEXT("Withheld accepted Attack never becomes a resolved UI event"),HiddenProjection.ResolvedRolls.Num(),1);
	TestFalse(TEXT("Derived Formula removed when an accepted value is withheld"),HiddenProjection.InlineFormula.bVisible);
	TestTrue(TEXT("Future unresolved descriptors carry no raw D6"),!HiddenSafe.ResolutionFacts.Rolls.ContainsByPredicate(
		[](const auto& R){return !R.bResolved && R.RawD6!=0;}));
	FString Screen,Controller,Local,Launcher;
	auto ReadSource=[&](const TCHAR* Path,FString& Out) {return FFileHelper::LoadFileToString(Out,*(FPaths::ProjectDir()/Path));};
	TestTrue(TEXT("Architecture sources readable"),
		ReadSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),Screen)
		&&ReadSource(TEXT("Source/FMCodex/NetworkPlay/FMCodexNetworkMatchPlayerController.cpp"),Controller)
		&&ReadSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),Local)
		&&ReadSource(TEXT("Scripts/NetworkPlay/LaunchNetworkPlayDev.ps1"),Launcher));
	const int32 Start=Controller.Find(TEXT("EFMCodexMatchScreenSubmission AFMCodexNetworkMatchPlayerController::SubmitScreenIntent("));
	const int32 End=Controller.Find(TEXT("void AFMCodexNetworkMatchPlayerController::DevPlayerFacingAction"),ESearchCase::CaseSensitive,ESearchDir::FromStart,Start);
	const FString Submit=Controller.Mid(Start,End-Start);
	TestTrue(TEXT("Both owner paths call generated RPC"),Submit.Contains(TEXT("ServerSubmitPlayerIntent(Envelope)")));
	TestFalse(TEXT("No Host authority shortcut"),Submit.Contains(TEXT("HasAuthority"))||Submit.Contains(TEXT("GetAuthGameMode"))||Submit.Contains(TEXT("_Implementation")));
	TestFalse(TEXT("No raw read from Network controller"),Controller.Contains(TEXT("GetStateSnapshot"))||Controller.Contains(TEXT("GetInteractionView")));
	TestFalse(TEXT("Shared UI does not generate gameplay RNG"),Screen.Contains(TEXT("FRandomStream"))||Screen.Contains(TEXT("FMath::Rand")));
	TestTrue(TEXT("Local remains direct adapter with existing Start"),Local.Contains(TEXT("AFMCodexLocalMatchPlayerController::SubmitScreenIntent"))&&Local.Contains(TEXT("StartNewDemoMatch();")));
	TestFalse(TEXT("Local backend has no RPC dependency"),Local.Contains(TEXT("ServerSubmitPlayerIntent")));
	TestTrue(TEXT("DEV diagnostics optional and hidden by default"),Controller.Contains(TEXT("IsPlayerFacingMode() && !FParse::Param"))&&Controller.Contains(TEXT("FMCodexNetworkDiagnostics")));
	TestTrue(TEXT("Milestone launch retains normal diagnostic mode"),Launcher.Contains(TEXT("$PlayerFacingCrossMilestone"))&&Launcher.Contains(TEXT("$CrossTerminalMilestone")));
	return true;
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexPlayerFacingPromptMirror,
	"FMCodex.NetworkPlay.PlayerFacingCrossUI.05.PromptMirror",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexPlayerFacingPromptMirror::GetTests(TArray<FString>& N,TArray<FString>& C) const
{ N={TEXT("A.LateDefense"),TEXT("B.LateDefense"),TEXT("A.RejectAfterHandoff"),TEXT("B.RejectAfterHandoff")}; C=N; }
bool FFMCodexPlayerFacingPromptMirror::RunTest(const FString& P)
{
	using namespace FMCodexPlayerFacingCrossUITests;
	FUIFixture F(P.StartsWith(TEXT("B")));
	if (!TestTrue(TEXT("Canonical prelude"),F.SkillFixture())) return false;
	auto* Actor=F.Attacker(); auto* S=Actor->GetPlayerMatchScreen();
	S->RequestSubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
	S->RequestSubmitBranchIntent(EFMCodexUMGBranchIntent::CrossLow);
	F.Entropy->Word=4; S->RequestContinueResolution(); F.Settle();
	F.Entropy->Word=5; S->RequestContinueResolution(); F.Settle();
	CheckBothPrompts(*this,F,TEXT("防守方掷点"));
	if(P.EndsWith(TEXT("LateDefense")))
	{
		for(auto* PC:{F.A,F.B})
		{
			auto* Late=CreateWidget<UFMCodexLocalMatchScreenWidget>(F.World,UFMCodexLocalMatchScreenWidget::StaticClass());
			const auto Slate=Late->TakeWidget();
			Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(PC->GetOwnerView(),false));
			CheckPrompt(*this,Late,PC->GetOwnerView(),TEXT("防守方掷点"));
			for(int32 I=0;I<3;++I)
			{
				Late->RefreshFromPresentation(FFMCodexNetworkMatchPresentationAdapter::Read(PC->GetOwnerView(),false));
				CheckPrompt(*this,Late,PC->GetOwnerView(),TEXT("防守方掷点"));
				TestFalse(TEXT("Repeated late View cannot replay handoff"),Late->IsInlineFormulaRevealInputBlocked());
			}
		}
	}
	else
	{
		// A fresh-ID forged repeat of the former actor's roll must not change the handoff.
		auto Forged=F.Backend(Actor).Last; ++Forged.RequestId;
		const auto Before=Access::Session(*F.Mode).GetStateSnapshot();
		const auto Ack=F.Mode->SubmitConnectionPlayerIntent(Actor,Forged);
		TestTrue(TEXT("Waiting former actor cannot reroll by forging a request"),Ack.Code!=Code::Accepted);
		TestTrue(TEXT("Rejected request preserves complete authority"),SameState(Before,Access::Session(*F.Mode).GetStateSnapshot()));
		S->NotifyScreenRequestRejected(); Actor->RefreshPlayerFacingUI();
		CheckBothPrompts(*this,F,TEXT("防守方掷点"));
	}
	return true;
}
#endif
