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
	V.Interaction.PrimaryAction.Label = TEXT("下一回合");
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
	A::Scene(*S).Celebration.Tick(2.f); Tick(0.f); Tick(2.49f);
	TestEqual(TEXT("Complete result reading has no countdown"), Text(), FString(TEXT("下一回合")));
	Tick(.02f); TestEqual(TEXT("After reading show 5"), Text(), FString(TEXT("下一回合（5）")));
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
	NewContext(); Tick(2.5f); Tick(1.f); S->RequestContinueResolution(); Tick(10.f);
	TestEqual(TEXT("Early manual click cancels former expiry"), Backend.Sends, 2);
	NewContext(); Tick(2.5f); V.Interaction.ExpectedActorLabel = TEXT("玩家 B"); Show();
	TestFalse(TEXT("Actor change cancels countdown"), A::Clock(*S).bCounting);
	Tick(2.5f); V.Header.CurrentPhaseLabel = TEXT("不同阶段"); Show();
	TestFalse(TEXT("Phase change cancels countdown"), A::Clock(*S).bCounting);
	Tick(2.5f); V.Interaction.PrimaryAction.bAvailable = false; Show(); Tick(10.f);
	TestEqual(TEXT("Capability withdrawal cancels without command"), Backend.Sends, 2);
	V.Interaction.PrimaryAction.bAvailable = true;
	for (int32 Gate = 0; Gate < 4; ++Gate)
	{
		NewContext(); Tick(2.5f);
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
	NewContext(); Backend.Reject = true; Tick(2.5f); Tick(5.f); Tick(30.f); Show(); Tick(30.f);
	TestEqual(TEXT("Rejected automatic attempt never loops"), Backend.Sends, 3);
	Backend.Reject = false; S->RequestContinueResolution();
	TestEqual(TEXT("Explicit manual retry remains possible"), Backend.Sends, 4);
	NewContext(); Tick(2.5f); A::Destruct(*S);
	TestEqual(TEXT("Destruction cancels elapsed clock"), A::Clock(*S).Sequence, int64(0));
	S->ResetPresentationSession();
	TestEqual(TEXT("Reset also clears adapter dedupe for a new match"), Backend.NextRoundDispatch.Sequence, int64(0));
	V.Header.AttackSequence = 1; Show(); Tick(2.5f);
	V.Interaction.Category = V.Interaction.PrimaryAction.Category = C::RollCrossAttack;
	Show(); Tick(50.f); TestEqual(TEXT("Final formula dice remain manual"), Backend.Sends, 4);
	S->ClearMatchController();
	return true;
}
#endif
