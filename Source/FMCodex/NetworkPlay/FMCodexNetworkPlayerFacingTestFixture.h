#pragma once
#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkInitialRouteTestFixture.h"
#include "FMCodexNetworkMatchScreenActions.h"
#include "UObject/GarbageCollection.h"
#include "../LocalPlay/FMCodexLocalMatchScreenWidget.h"
#include "../LocalPlay/FMCodexMatchHeaderWidget.h"
#include "../LocalPlay/FMCodexInteractionPanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "../LocalPlay/FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "../LocalPlay/FMCodexSelectionFeedbackToastWidget.h"
#include "../LocalPlay/FMCodexLocalMatchPlayerController.h"
#include "../LocalPlay/FMCodexLocalMatchResolutionFeedback.h"

namespace FMCodexPlayerFacingOrdinaryUITests
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
			// Long synchronous automation runs otherwise retain retired widget trees
			// until the engine next gets a regular GC tick. The new fixture world is rooted.
			CollectGarbage(RF_NoFlags);
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
		bool SkillFixture(bool Final = false, ESkillRuleType Family = ESkillRuleType::Cross)
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
			if (!Access::Runtime(*Mode).PrepareOrdinaryTerminalMilestone(true, false, true, Family)) return false;
			Access::Publish(*Mode); Settle(); return true;
		}
	};
	inline void CheckPrompt(FAutomationTestBase& T, UFMCodexLocalMatchScreenWidget* S,
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
	inline void CheckBothPrompts(FAutomationTestBase& T,FUIFixture& F,const FString& Action)
	{
		for (auto* PC:{F.A,F.B}) CheckPrompt(T,PC->GetPlayerMatchScreen(),PC->GetOwnerView(),Action);
	}
	inline bool Continue(FAutomationTestBase& T, FUIFixture& F, AFMCodexNetworkMatchPlayerController* PC, Kind Expected)
	{
		const int32 Before = F.Backend(PC).Sends;
		PC->GetPlayerMatchScreen()->RequestContinueResolution();
		return T.TestEqual(TEXT("Shared central callback sends once"), F.Backend(PC).Sends, Before + 1)
			&& T.TestEqual(TEXT("Exact existing typed intent"), F.Backend(PC).Last.IntentKind, Expected)
			&& T.TestEqual(TEXT("Connection-side validation accepts"), F.Backend(PC).LastCode, Code::Accepted);
	}
}

#endif
