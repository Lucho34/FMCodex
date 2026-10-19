#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexResolutionPanelWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"

struct FFMCodexNextRoundTestAccess
{
	static void Tick(UFMCodexLocalMatchScreenWidget& S, float Delta) { S.AdvanceNextRoundCountdown(Delta); }
	static auto& Clock(UFMCodexLocalMatchScreenWidget& S) { return S.NextRoundCountdown; }
	static auto& Scene(UFMCodexLocalMatchScreenWidget& S) { return S.TacticalScene; }
	static bool Ready(UFMCodexLocalMatchScreenWidget& S) { return S.IsNextRoundReady(); }
	static void Destruct(UFMCodexLocalMatchScreenWidget& S) { S.NativeDestruct(); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNextRoundCountdownTest,
	"FMCodex.LocalPlay.NextRoundCountdown.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNextRoundCountdownTest::RunTest(const FString&)
{
	using A = FFMCodexNextRoundTestAccess;
	using C = EFMCodexUMGInteractionCategory;
	struct FBackend : IFMCodexMatchScreenBackend
	{
		int32 Sends = 0; bool Pending = false; bool Reject = false;
		FFMCodexMatchScreenRequest Last;
		EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest& R) override
		{ ++Sends; Last = R; Pending = !Reject; return Reject ? EFMCodexMatchScreenSubmission::Rejected : EFMCodexMatchScreenSubmission::Queued; }
		bool IsScreenIntentPending() const override { return Pending; }
	} Backend;
	auto* S = NewObject<UFMCodexLocalMatchScreenWidget>();
	auto Root = S->TakeWidget(); S->SetMatchBackend(&Backend);
	FFMCodexUMGMatchScreenViewModel V;
	V.Header.AttackSequence = 1; V.Header.CurrentPhaseLabel = TEXT("结果");
	V.Interaction.Category = C::AdvanceAfterTerminal;
	V.Interaction.ExpectedActorLabel = TEXT("玩家 A");
	V.Interaction.PrimaryAction.bAvailable = true;
	V.Interaction.PrimaryAction.Category = C::AdvanceAfterTerminal;
	V.Interaction.PrimaryAction.Label = FText::FromString(TEXT("下一回合"));
	V.Interaction.bCanContinue = true;
	auto Show = [&]() { S->RefreshFromPresentation(V); };
	auto Tick = [&](float D) { A::Tick(*S, D); };
	auto NewContext = [&]() { Backend.Pending = false; ++V.Header.AttackSequence; Show(); };
	auto Text = [&]() { return CastChecked<UTextBlock>(S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionContinueButtonLabel")))->GetText().ToString(); };
	Show();
	if (!TestTrue(TEXT("Existing enabled typed Next Round is the eligibility source"), A::Ready(*S))) return false;
	// This isolated safe-VM fixture has no tactical scene facts. Start at the
	// celebration gate, without re-projecting an unrelated synthetic scene.
	A::Clock(*S).Cancel(); A::Scene(*S).Celebration.Start(true, 2.f); Tick(20.f);
	TestEqual(TEXT("Celebration consumes no reading/countdown time"), A::Clock(*S).Sequence, int64(0));
	TestEqual(TEXT("Celebration cannot submit"), Backend.Sends, 0);
	A::Scene(*S).Celebration.Tick(2.f); Tick(0.f); Tick(2.5f);
	TestFalse(TEXT("Former 2.5-second boundary remains in reading"), A::Clock(*S).bCounting);
	Tick(.49f);
	TestEqual(TEXT("Complete result reading has no countdown"), Text(), FString(TEXT("下一回合")));
	Tick(.02f); TestEqual(TEXT("After three-second reading show 5"), Text(), FString(TEXT("下一回合（5）")));
	TestEqual(TEXT("Reading never consumes countdown time"), A::Clock(*S).CountdownElapsed, 0.f);
	Show(); Tick(.5f); Show(); Tick(.5f);
	TestEqual(TEXT("Repeated refresh does not reset elapsed time"), Text(), FString(TEXT("下一回合（4）")));
	Tick(1.f); TestEqual(TEXT("Next visible second is 3"), Text(), FString(TEXT("下一回合（3）")));
	Tick(1.f); TestEqual(TEXT("Next visible second is 2"), Text(), FString(TEXT("下一回合（2）")));
	Tick(1.f); TestEqual(TEXT("Next visible second is 1"), Text(), FString(TEXT("下一回合（1）")));
	TestEqual(TEXT("Not yet submitted"), Backend.Sends, 0);
	Tick(1.f); S->RequestContinueResolution(); Tick(10.f);
	TestEqual(TEXT("Expiry plus click and pending refresh submits exactly once"), Backend.Sends, 1);
	TestTrue(TEXT("Same existing typed Continue path"), Backend.Last.Kind == EFMCodexMatchScreenIntent::Continue && Backend.Last.Category == C::AdvanceAfterTerminal);
	// The terminal headline and Recovery reuse the same region. Check the full
	// parent chain, not just the visibility flag on the newly constructed rows.
	const auto Terminal = V;
	Backend.Pending = false;
	++V.Header.AttackSequence;
	V.Interaction.Category = V.Interaction.PrimaryAction.Category = C::None;
	V.Interaction.PrimaryAction.bAvailable = false;
	V.Resolution.bVisible = V.Resolution.bNonBlockingNotification = true;
	V.Resolution.StepLabel = FFMCodexPlayerUIPresentationText::RecoveryNotificationTitle().ToString();
	V.Resolution.StepSummaryLabel = TEXT("玩家 A · 萨卡\n玩家 B · 罗德里");
	Show();
	auto* Rows = CastChecked<UPanelWidget>(S->GetResolutionPanel()->GetWidgetFromName(TEXT("RecoverySummaryRows")));
	TestEqual(TEXT("Recovery retains both projected player rows"), Rows->GetChildrenCount(), 2);
	for (const UWidget* W = Rows; W; W = W->GetParent())
		TestTrue(TEXT("Recovery rows and every containing region are visible after countdown"), W->IsVisible());
	TestTrue(TEXT("Recovery notification overlay is shown"), S->IsLegacyResolutionOverlayVisible());
	V = Terminal;
	Backend.Pending = false;
	auto* Rebuilt = NewObject<UFMCodexLocalMatchScreenWidget>(); auto RebuiltRoot = Rebuilt->TakeWidget();
	Rebuilt->SetMatchBackend(&Backend); Rebuilt->RefreshFromPresentation(V);
	A::Tick(*Rebuilt, 10.f); Rebuilt->RequestContinueResolution();
	TestEqual(TEXT("Widget reconstruction and delayed old View cannot submit again"), Backend.Sends, 1);
	Rebuilt->ClearMatchController();
	NewContext(); Tick(3.f); Tick(1.f); S->RequestContinueResolution(); Tick(10.f);
	TestEqual(TEXT("Early manual click cancels former expiry"), Backend.Sends, 2);
	NewContext(); Tick(3.f); V.Interaction.ExpectedActorLabel = TEXT("玩家 B"); Show();
	TestFalse(TEXT("Actor change cancels countdown"), A::Clock(*S).bCounting);
	Tick(3.f); V.Header.CurrentPhaseLabel = TEXT("不同阶段"); Show();
	TestFalse(TEXT("Phase change cancels countdown"), A::Clock(*S).bCounting);
	Tick(3.f); V.Interaction.PrimaryAction.bAvailable = false; Show(); Tick(10.f);
	TestEqual(TEXT("Capability withdrawal cancels without command"), Backend.Sends, 2);
	V.Interaction.PrimaryAction.bAvailable = true;
	for (int32 Gate = 0; Gate < 4; ++Gate)
	{
		NewContext(); Tick(3.f);
		V.bActionWaitPromptReadOnly = Gate == 0;
		V.Header.bMatchEnded = Gate == 1;
		V.Resolution.bVisible = V.Resolution.bNonBlockingNotification = Gate == 2;
		Backend.Pending = Gate == 3;
		Show(); Tick(20.f);
		TestEqual(TEXT("Waiting viewer / match end / recovery notice / pending cancel"), Backend.Sends, 2);
		TestEqual(TEXT("Cancelled clock has no context"), A::Clock(*S).Sequence, int64(0));
		V.bActionWaitPromptReadOnly = V.Header.bMatchEnded = V.Resolution.bVisible = V.Resolution.bNonBlockingNotification = false;
		Backend.Pending = false;
	}
	NewContext(); Backend.Reject = true; Tick(3.f); Tick(5.f); Tick(30.f); Show(); Tick(30.f);
	TestEqual(TEXT("Rejected automatic attempt never loops"), Backend.Sends, 3);
	Backend.Reject = false; S->RequestContinueResolution();
	TestEqual(TEXT("Explicit manual retry remains possible"), Backend.Sends, 4);
	NewContext(); Tick(3.f); A::Destruct(*S);
	TestEqual(TEXT("Destruction cancels elapsed clock"), A::Clock(*S).Sequence, int64(0));
	S->ResetPresentationSession();
	TestEqual(TEXT("Reset also clears adapter dedupe for a new match"), Backend.NextRoundDispatch.Sequence, int64(0));
	V.Header.AttackSequence = 1; Show(); Tick(3.f);
	V.Interaction.Category = V.Interaction.PrimaryAction.Category = C::RollCrossAttack;
	Show(); Tick(50.f); TestEqual(TEXT("Final formula dice remain manual"), Backend.Sends, 4);
	S->ClearMatchController();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexNextRoundCrossTheaterReadingTest,
 "FMCodex.LocalPlay.NextRoundCountdown.CrossTheaterReading",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexNextRoundCrossTheaterReadingTest::RunTest(const FString&)
{
 using A=FFMCodexNextRoundTestAccess;
 using C=EFMCodexUMGInteractionCategory;
 // Already-disclosed terminal projections: this checks shared display timing,
 // not tactical formulas or the authority that produced these facts.
 for(auto Skill:{ESkillRuleType::Cross,ESkillRuleType::CutInsideShot,ESkillRuleType::LongShot})
 {
  struct FBackend:IFMCodexMatchScreenBackend
  {
   int Sends=0;
   bool IsScreenIntentPending() const override{return false;}
   EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest&) override
   {++Sends;return EFMCodexMatchScreenSubmission::Queued;}
  } Backend;
  auto* S=NewObject<UFMCodexLocalMatchScreenWidget>();auto Root=S->TakeWidget();S->SetMatchBackend(&Backend);
  FFMCodexUMGMatchScreenViewModel V;
  V.Header.AttackSequence=1;V.Header.CurrentPhaseLabel=TEXT("结果");
  V.Interaction.Category=C::AdvanceAfterTerminal;V.Interaction.ExpectedActorLabel=TEXT("玩家 A");
  V.Interaction.PrimaryAction={};V.Interaction.PrimaryAction.bAvailable=true;
  V.Interaction.PrimaryAction.Category=C::AdvanceAfterTerminal;V.Interaction.PrimaryAction.Label = FText::FromString(TEXT("下一回合"));
  V.Interaction.bCanContinue=true;
  auto& F=V.InlineFormula;
  F.bVisible=F.bNarrativeAvailable=F.bCanContinue=true;
  F.ContestId=Skill==ESkillRuleType::Cross?TEXT("Cross.High"):Skill==ESkillRuleType::LongShot?TEXT("LongShot.DirectShot"):TEXT("CutInsideShot.DirectShot");
  F.NarrativeHeadline=TEXT("本次进攻未能破门。");F.SpatialOutcome=EMatchPlayResolutionDecisionOutcome::Miss;
  F.PrimaryAction.bClaimsAction=F.PrimaryAction.bVisible=true;F.PrimaryAction.Action=V.Interaction.PrimaryAction;
  if(Skill!=ESkillRuleType::Cross)
  {
   auto& Shot=V.LongShotResolution;Shot.bVisible=true;Shot.SkillType=Skill;
   Shot.Stage=EFMCodexUMGLongShotStage::DirectShot;Shot.Formula=F;
  }
  S->RefreshFromPresentation(V);S->ForceLayoutPrepass();
  if(!TestTrue(TEXT("Each theater exposes the original terminal CTA"),A::Ready(*S)))return false;
  auto* Banner=S->GetWidgetFromName(TEXT("CrossResultPlate"));
  auto* Label=CastChecked<UTextBlock>(S->GetWidgetFromName(TEXT("TheaterContinueLabel")));
  A::Clock(*S).Cancel();A::Tick(*S,0.f);A::Tick(*S,2.5f);
  TestTrue(TEXT("Banner remains readable at the old boundary"),Banner->IsVisible());
  TestFalse(TEXT("No countdown overlap during reading"),A::Clock(*S).bCounting);
  S->RefreshFromPresentation(V);A::Tick(*S,.49f);
  TestTrue(TEXT("Repeated View retains banner until three seconds"),Banner->IsVisible());
  A::Tick(*S,.02f);
  TestEqual(TEXT("Reading hands off to five-second countdown"),Label->GetText().ToString(),FString(TEXT("下一回合（5）")));
  TestEqual(TEXT("Only the narrative is retired at handoff"),Banner->GetVisibility(),ESlateVisibility::Hidden);
  TestEqual(TEXT("The first countdown frame has consumed zero seconds"),A::Clock(*S).CountdownElapsed,0.f);
  A::Tick(*S,4.99f);TestEqual(TEXT("No early auto continuation"),Backend.Sends,0);
  A::Tick(*S,.02f);TestEqual(TEXT("Expiry submits once through existing Continue"),Backend.Sends,1);
  S->ClearMatchController();
 }
 return true;
}
#endif
