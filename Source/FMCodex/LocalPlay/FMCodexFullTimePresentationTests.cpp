#include "FMCodexFullTimePanelWidget.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalDevRollOverrideWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPrototypeTeamContent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#if WITH_EDITOR
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Misc/CommandLine.h"
#include "Misc/Paths.h"
#include "Slate/WidgetRenderer.h"
#endif

namespace FMCodexFullTimeTests
{
	using ESide = EInitialTurnOrderPlayer;
	using ETarget = EFMCodexLocalDevRollTarget;
	struct FWorldFixture
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;
		FWorldFixture()
		{
			if (!World || !GEngine) return;
			GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
			Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			Controller = World->SpawnActor<AFMCodexLocalMatchPlayerController>();
			Controller->RefreshPresentation();
		}
		~FWorldFixture()
		{
			if (World) { if (GEngine) GEngine->DestroyWorldContext(World); World->DestroyWorld(false); }
		}
	};

	TArray<uint8> Bytes(const FMatchPlayState& State)
	{
		TArray<uint8> Data;
		FMemoryWriter Writer(Data);
		auto Copy = State;
		FMatchPlayState::StaticStruct()->SerializeItem(Writer, &Copy, nullptr);
		return Data;
	}

	bool Override(AFMCodexLocalMatchPlayerController& C, ETarget Target, int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest Request;
		Request.Target = Target; Request.Value = Value;
		return C.SetLocalDevRollOverride(Request).bSuccess;
	}

	bool Penalty(FAutomationTestBase& Test, FWorldFixture& F, bool bGoal, FName& Scorer, FName PreferredScorer = NAME_None)
	{
		auto& C = *F.Controller;
		if (!Override(C, ETarget::FullD12, 9)) return false;
		C.RollDemoTacticalPoints();
		if (!Override(C, ETarget::SetPieceType, 6)) return false;
		C.ContinueResolution();
		if (C.GetInteractionView().LegalSetPieceCardIds.IsEmpty()) return false;
		Scorer = C.GetInteractionView().LegalSetPieceCardIds[0];
		if (!PreferredScorer.IsNone())
		{
			if (!Test.TestTrue(TEXT("Visual fixture uses a legal real scorer"),
				C.GetInteractionView().LegalSetPieceCardIds.Contains(PreferredScorer))) return false;
			Scorer = PreferredScorer;
		}
		C.ToggleSetPieceDraftCard(Scorer);
		C.ConfirmSetPieceDraft();
		C.SubmitPenaltyMethod(EMatchPlayPenaltyMethod::Panenka);
		if (!Override(C, ETarget::PenaltyPanenka, bGoal ? 6 : 1)) return false;
		C.ContinueResolution();
		return Test.TestTrue(TEXT("Real penalty reaches terminal, not full time"),
			C.GetLastDiagnostic().bHostSuccess && C.GetInteractionView().bTerminalPendingAdvance
				&& !C.GetInteractionView().FullTime.bVisible);
	}

	FFMCodexUMGMatchScreenViewModel DTO(const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(View, {}, FString());
	}

	UTextBlock* GoalLabel(UScrollBox& List, int32 Index = 0)
	{
		const auto* Bounds = CastChecked<USizeBox>(List.GetChildAt(Index));
		const auto* Row = CastChecked<UHorizontalBox>(Bounds->GetChildAt(0));
		return CastChecked<UTextBlock>(Row->GetChildAt(1));
	}

	bool IsLeftAligned(const UTextBlock& Label)
	{
		// UE 5.3 exposes the reflected property but has no public justification getter.
		const auto* Property = FindFProperty<FByteProperty>(UTextBlock::StaticClass(), TEXT("Justification"));
		return Property && Property->GetPropertyValue_InContainer(&Label) == ETextJustify::Left;
	}

#if WITH_EDITOR
	void CheckRenderedGeometry(FAutomationTestBase& Test, UFMCodexFullTimePanelWidget& Panel, FVector2D Viewport)
	{
		const auto& Bounds = Panel.GetWidgetFromName(TEXT("FullTimeDesignBounds"))->GetCachedGeometry();
		const FVector2D Size = Bounds.GetAbsoluteSize(), Position = Bounds.GetAbsolutePosition();
		Test.TestTrue(TEXT("Centered card envelope has substantial viewport margins"),
			Size.X > Viewport.X * .70 && Size.X <= Viewport.X * .78
				&& Size.Y <= Viewport.Y * .75 && Position.Y >= Viewport.Y * .12
				&& FMath::IsNearlyEqual(Position.X + Size.X / 2, Viewport.X / 2, 1.0));
		Test.AddInfo(FString::Printf(TEXT("Full-time %.0fx%.0f: envelope %.1fx%.1f at %.1f,%.1f; frame height %.1f"),
			Viewport.X, Viewport.Y, Size.X, Size.Y, Position.X, Position.Y, Size.Y * 488 / 540));
		auto* A = CastChecked<UScrollBox>(Panel.GetWidgetFromName(TEXT("FullTimeAGoalList")));
		auto* B = CastChecked<UScrollBox>(Panel.GetWidgetFromName(TEXT("FullTimeBGoalList")));
		const auto& AG = A->GetCachedGeometry(); const auto& BG = B->GetCachedGeometry();
		const auto& AT = GoalLabel(*A)->GetCachedGeometry(); const auto& BT = GoalLabel(*B)->GetCachedGeometry();
		const float AInset = AG.AbsoluteToLocal(AT.GetAbsolutePosition()).X;
		const float BInset = BG.AbsoluteToLocal(BT.GetAbsolutePosition()).X;
		Test.TestTrue(TEXT("Both scorer names start at the same small left inset, never centered"),
			FMath::IsNearlyEqual(AInset, BInset, .1f) && AInset >= 29 && AInset <= 31);
		Test.TestTrue(TEXT("Both scorer rows share the same baseline and column width"),
			FMath::IsNearlyEqual(AT.GetAbsolutePosition().Y, BT.GetAbsolutePosition().Y, 1.f)
				&& FMath::IsNearlyEqual(AG.GetAbsoluteSize().X, BG.GetAbsoluteSize().X, 1.f));
		Test.TestTrue(TEXT("Ordinary goal counts need no scrolling"), A->GetViewFraction() >= .999f && B->GetViewFraction() >= .999f);
		for (const TCHAR* Name : {TEXT("FullTimeTitle"), TEXT("FullTimePlayerAName"), TEXT("FullTimePlayerBName"),
			TEXT("FullTimeTeamAName"), TEXT("FullTimeTeamBName"), TEXT("FullTimeTeamAScore"), TEXT("FullTimeTeamBScore"),
			TEXT("FullTimeScorersHeading"), TEXT("FullTimeConfirmButton")})
		{
			const auto& G = Panel.GetWidgetFromName(Name)->GetCachedGeometry();
			const FVector2D P = G.GetAbsolutePosition(), S = G.GetAbsoluteSize();
			Test.TestTrue(FString::Printf(TEXT("%s remains inside the centered envelope"), Name),
				S.X > 0 && S.Y > 0 && P.X >= Position.X && P.Y >= Position.Y
					&& P.X + S.X <= Position.X + Size.X + 1 && P.Y + S.Y <= Position.Y + Size.Y + 1);
		}
		const auto& Dash = Panel.GetWidgetFromName(TEXT("FullTimeScoreDash"))->GetCachedGeometry();
		Test.TestTrue(TEXT("Score divider remains at viewport center regardless of identities"),
			FMath::IsNearlyEqual(Dash.GetAbsolutePosition().X + Dash.GetAbsoluteSize().X / 2, Viewport.X / 2, 1.0));
	}
#endif
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullTimeLifecycleTest,
	"FMCodex.LocalPlay.FullTime.01.RealLifecycleResultMatrix",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFullTimeLifecycleTest::RunTest(const FString&)
{
	using namespace FMCodexFullTimeTests;
	for (int32 Case = 0; Case < 4; ++Case)
	{
		FWorldFixture F;
		if (!TestNotNull(TEXT("Host"), F.Host) || !TestNotNull(TEXT("Controller"), F.Controller)) return false;
		F.Controller->StartNewDevShortMatch();
		auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>();
		Screen->TakeWidget();
		Screen->SetMatchController(F.Controller);
		int32 GoalsA = 0, GoalsB = 0;
		for (int32 Attack = 0; Attack < 2; ++Attack)
		{
			const auto Side = F.Controller->GetInteractionView().CurrentAttackingPlayer;
			const bool bGoal = (Case & (Side == ESide::PlayerA ? 1 : 2)) != 0;
			FName Scorer;
			const FName PreferredScorer = Case == 3 ? (Side == ESide::PlayerA
				? FName(TEXT("Prototype.Arsenal.ViktorGyokeres")) : FName(TEXT("Prototype.ManchesterCity.JoskoGvardiol"))) : NAME_None;
			if (!Penalty(*this, F, bGoal, Scorer, PreferredScorer)) return false;
			if (bGoal) (Side == ESide::PlayerA ? GoalsA : GoalsB)++;
			const auto Terminal = F.Host->GetMatchSnapshot().Snapshot;
			TestEqual(TEXT("History committed with real score, before advance"), Terminal.GoalHistory.Num(), GoalsA + GoalsB);
			if (bGoal)
			{
				TestTrue(TEXT("History preserves actual side, sequence and penalty scorer"),
					Terminal.GoalHistory.Last().ScoringSide == Side
						&& Terminal.GoalHistory.Last().AttackSequence == Terminal.CurrentAttack.AttackSequence
						&& Terminal.GoalHistory.Last().ScorerCardId == Scorer);
			}
			Screen->RefreshFromPresentation(DTO(F.Controller->GetInteractionView()));
			TestEqual(TEXT("Even final terminal keeps full-time modal closed"),
				Screen->GetFullTimePanel()->GetVisibility(), ESlateVisibility::Collapsed);
			F.Controller->AdvanceAfterTerminal();
			TestTrue(TEXT("Exactly one real advance accepted"), F.Controller->GetLastDiagnostic().bHostSuccess);
			TestEqual(TEXT("Only the second advance ends the match"), F.Controller->GetInteractionView().bMatchEnded, Attack == 1);
			TestEqual(TEXT("Advance does not duplicate goal history"), F.Host->GetMatchSnapshot().Snapshot.GoalHistory.Num(), GoalsA + GoalsB);
		}
		const auto End = F.Host->GetMatchSnapshot().Snapshot;
		TestTrue(TEXT("Both opportunities exhausted; no extra attack or recovery"),
			!End.bHasCurrentAttack && End.RuntimeState.CurrentAttackingPlayer == ESide::None
				&& End.RuntimeState.PlayerAState.UsedAttackCount == 1
				&& End.RuntimeState.PlayerBState.UsedAttackCount == 1);
		const auto View = F.Controller->GetInteractionView();
		TestEqual(TEXT("Real A score"), View.FullTime.PlayerA.Score.ToString(), FString::FromInt(GoalsA));
		TestEqual(TEXT("Real B score"), View.FullTime.PlayerB.Score.ToString(), FString::FromInt(GoalsB));
		TestEqual(TEXT("A roster-backed team name"), View.FullTime.PlayerA.Name.ToString(),
			FFMCodexPrototypeTeamContent::TeamDisplayName(View.PlayerACardRoster[0].CardId).ToString());
		TestEqual(TEXT("B roster-backed team name"), View.FullTime.PlayerB.Name.ToString(),
			FFMCodexPrototypeTeamContent::TeamDisplayName(View.PlayerBCardRoster[0].CardId).ToString());
		TestEqual(TEXT("A real goal rows"), View.FullTime.PlayerA.Goals.Num(), GoalsA);
		TestEqual(TEXT("B real goal rows"), View.FullTime.PlayerB.Goals.Num(), GoalsB);
		for (const auto& Goal : End.GoalHistory)
		{
			const auto& Rows = Goal.ScoringSide == ESide::PlayerA ? View.FullTime.PlayerA.Goals : View.FullTime.PlayerB.Goals;
			TestTrue(TEXT("Preferred scorer name, not ID or fabricated minute"), Rows.ContainsByPredicate([&](const FText& T)
				{ return T.EqualTo(FFMCodexPrototypeTeamContent::PlayerDisplayName(Goal.ScorerCardId)); }));
		}
		Screen->RefreshFromPresentation(DTO(View));
		auto* Panel = Screen->GetFullTimePanel();
		TestEqual(TEXT("Production modal active"), Panel->GetVisibility(), ESlateVisibility::Visible);
		TestEqual(TEXT("Old interaction end block removed"), Screen->GetInteractionPanel()->GetVisibility(), ESlateVisibility::Collapsed);
		TestEqual(TEXT("White header block itself collapsed, not merely covered"),
			Screen->GetMatchHeader()->GetWidgetFromName(TEXT("MatchHeaderFinalResultRegion"))->GetVisibility(), ESlateVisibility::Collapsed);
		TestTrue(TEXT("No stale pitch zones or generic overlay"), DTO(View).PitchRegions.IsEmpty() && !Screen->IsLegacyResolutionOverlayVisible());
		TestTrue(TEXT("Gameplay input locked independently of acknowledgement"), Screen->IsInlineFormulaRevealInputBlocked());
#if WITH_EDITOR
		if ((Case == 0 || Case == 3) && FParse::Param(FCommandLine::Get(), TEXT("FullTimeRenderAudit")))
		{
			// Explicit opt-in real RHI render of the actual widget and real match DTO.
			// No PIE, no viewport gameplay, and no alternative screenshot-only layout.
			auto* Renderer = new FWidgetRenderer(true);
			// Keep Slate alive through geometry inspection; UWidget only holds a weak
			// cached Slate pointer once DrawWidget's temporary virtual window is released.
			const TSharedRef<SWidget> RenderedScreen = Screen->TakeWidget();
			for (const auto Size : {FVector2D(1920, 1080), FVector2D(1280, 720)})
			{
				for (int32 Stress = 0; Stress < (Case == 3 ? 2 : 1); ++Stress)
				{
					auto* Target = UKismetRenderingLibrary::CreateRenderTarget2D(F.World,
						int32(Size.X), int32(Size.Y), RTF_RGBA8);
					auto RenderDTO = DTO(View);
					if (Stress)
					{
						// Explicit presentation-only name stress fixture, not invented match facts.
						RenderDTO.FullTime.PlayerB.PlayerDisplayName = FText::FromString(TEXT("来自曼彻斯特的客队玩家"));
						RenderDTO.FullTime.PlayerB.Name = FText::FromString(TEXT("曼彻斯特城足球俱乐部"));
					}
					Screen->RefreshFromPresentation(RenderDTO);
					RenderedScreen->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
					// First paint updates the retained UWidget tick geometry after a size change;
					// the second is the inspected/exported frame.
					Renderer->DrawWidget(Target, RenderedScreen, Size, 0.0f);
					Renderer->DrawWidget(Target, RenderedScreen, Size, 0.0f);
					CheckRenderedGeometry(*this, *Panel, Size);
					const FString File = FString::Printf(TEXT("FullTime_%d-%d_%dx%d%s.png"), GoalsA, GoalsB,
						int32(Size.X), int32(Size.Y), Stress ? TEXT("_LongIdentity") : TEXT(""));
					const FString Directory = FPaths::ProjectSavedDir() / TEXT("Automation/FullTimeVisual_6_16_12_1");
					UKismetRenderingLibrary::ExportRenderTarget(F.World, Target, Directory, File);
					TestTrue(TEXT("Offscreen production widget exported"), FPaths::FileExists(Directory / File));
				}
			}
			Screen->RefreshFromPresentation(DTO(View));
			BeginCleanup(Renderer);
		}
#endif
		const auto Before = Bytes(End);
		Panel->GetConfirmButton()->OnClicked.Broadcast();
		Panel->Acknowledge();
		Screen->RefreshFromPresentation(DTO(View));
		TestTrue(TEXT("Confirm is idempotent and keeps stable result page"), Panel->IsAcknowledged()
			&& Panel->GetVisibility() == ESlateVisibility::Visible && !Panel->GetConfirmButton()->GetIsEnabled());
		F.Controller->RollDemoTacticalPoints();
		F.Controller->AdvanceAfterTerminal();
		F.Controller->ContinueResolution();
		TestTrue(TEXT("Stale post-match commands and acknowledgement mutate nothing"), Before == Bytes(F.Host->GetMatchSnapshot().Snapshot));
		FMatchPlayState Restored;
		FMemoryReader Reader(Before);
		FMatchPlayState::StaticStruct()->SerializeItem(Reader, &Restored, nullptr);
		TestTrue(TEXT("History survives full snapshot serialization"), Before == Bytes(Restored));
		const auto Rebuilt = FFMCodexLocalMatchInteractionViewBuilder::Build(Restored, F.Host->GetSkillRuleSnapshot().Snapshot);
		TestTrue(TEXT("Fresh projection reconstructs summary without old controller"),
			Rebuilt.FullTime.bVisible && Rebuilt.FullTime.PlayerA.Goals.Num() == GoalsA && Rebuilt.FullTime.PlayerB.Goals.Num() == GoalsB);
		F.Controller->StartNewDemoMatch();
		Screen->RefreshFromPresentation(DTO(F.Controller->GetInteractionView()));
		TestTrue(TEXT("Normal new match resets summary and restores default three"),
			!Panel->IsAcknowledged() && Panel->GetVisibility() == ESlateVisibility::Collapsed
				&& F.Host->GetMatchSnapshot().Snapshot.GoalHistory.IsEmpty()
				&& F.Controller->GetInteractionView().PlayerAMaxAttackTurns == 3
				&& F.Controller->GetInteractionView().PlayerBMaxAttackTurns == 3);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullTimeNormalLengthTest,
	"FMCodex.LocalPlay.FullTime.02.NormalSixAttackLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFullTimeNormalLengthTest::RunTest(const FString&)
{
	using namespace FMCodexFullTimeTests;
	FWorldFixture F;
	F.Controller->StartNewDemoMatch();
	for (int32 Attack = 0; Attack < 6; ++Attack)
	{
		FName Scorer;
		if (!Penalty(*this, F, false, Scorer)) return false;
		F.Controller->AdvanceAfterTerminal();
		TestEqual(TEXT("Default only ends after six successful advances"), F.Controller->GetInteractionView().FullTime.bVisible, Attack == 5);
	}
	TestTrue(TEXT("No fabricated scorers in a scoreless match"), F.Host->GetMatchSnapshot().Snapshot.GoalHistory.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullTimeFallbackTest,
	"FMCodex.LocalPlay.FullTime.03.HistoryFallbackAndLongLists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFullTimeFallbackTest::RunTest(const FString&)
{
	using namespace FMCodexFullTimeTests;
	FWorldFixture F;
	F.Controller->StartNewDevShortMatch();
	for (int32 I = 0; I < 2; ++I)
	{
		FName Scorer;
		if (!Penalty(*this, F, false, Scorer)) return false;
		F.Controller->AdvanceAfterTerminal();
	}
	// Deliberate historical/corrupt-name fixtures, not an alternative gameplay path.
	auto Old = F.Host->GetMatchSnapshot().Snapshot;
	Old.RuntimeState.PlayerAState.Score = 1;
	auto View = FFMCodexLocalMatchInteractionViewBuilder::Build(Old, F.Host->GetSkillRuleSnapshot().Snapshot);
	if (!TestEqual(TEXT("Historical fallback row exists"), View.FullTime.PlayerA.Goals.Num(), 1)) return false;
	TestEqual(TEXT("Older aggregate-only score does not invent a player"), View.FullTime.PlayerA.Goals[0].ToString(), FString(TEXT("部分进球记录不可用")));
	FMatchPlayGoalFact Goal;
	Goal.AttackSequence = 1; Goal.ScoringSide = ESide::PlayerA; Goal.ScorerCardId = TEXT("Missing.Internal.PlayerKey");
	Old.GoalHistory.Add(Goal);
	View = FFMCodexLocalMatchInteractionViewBuilder::Build(Old, F.Host->GetSkillRuleSnapshot().Snapshot);
	TestEqual(TEXT("Missing scorer metadata is generic, never raw ID"), View.FullTime.PlayerA.Goals[0].ToString(), FString(TEXT("进球球员")));
	Old.GoalHistory[0].bSystemAward = true;
	View = FFMCodexLocalMatchInteractionViewBuilder::Build(Old, F.Host->GetSkillRuleSnapshot().Snapshot);
	TestEqual(TEXT("Team award has no invented individual"), View.FullTime.PlayerA.Goals[0].ToString(), FString(TEXT("规则判定进球")));
	auto* Panel = NewObject<UFMCodexFullTimePanelWidget>(); Panel->TakeWidget();
	for (int32 I = 0; I < 16; ++I) View.FullTime.PlayerA.Goals.Add(FText::FromString(TEXT("球员名称展示测试")));
	Panel->RefreshFromPresentation(View.FullTime);
	TestEqual(TEXT("Scrollable goal list retains every row"), Panel->GetPresentation().PlayerA.Goals.Num(), 17);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullTimeIdentityLayoutTest,
	"FMCodex.LocalPlay.FullTime.04.IdentityAndAlignedNameLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFullTimeIdentityLayoutTest::RunTest(const FString&)
{
	using namespace FMCodexFullTimeTests;
	FFMCodexLocalMatchInteractionView View;
	View.FullTime.bVisible = true;
	View.FullTime.PlayerA.Name = FText::FromString(TEXT("阿森纳"));
	View.FullTime.PlayerB.Name = FText::FromString(TEXT("曼彻斯特城"));
	for (const auto Viewer : {ESide::PlayerA, ESide::PlayerB})
	{
		const auto Model = FFMCodexLocalMatchUMGPresentationBuilder::Build(View, {}, FString(), Viewer);
		const auto& A = Model.FullTime.PlayerA; const auto& B = Model.FullTime.PlayerB;
		TestTrue(TEXT("A participant identity agrees with Header, independent of viewer orientation"),
			A.PlayerDisplayName.EqualTo(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
				Viewer == ESide::PlayerA ? Model.Header.LeftPlayerLabel : Model.Header.RightPlayerLabel)));
		TestTrue(TEXT("B participant identity agrees with Header, independent of viewer orientation"),
			B.PlayerDisplayName.EqualTo(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
				Viewer == ESide::PlayerB ? Model.Header.LeftPlayerLabel : Model.Header.RightPlayerLabel)));
		TestTrue(TEXT("Participant and team labels remain distinct"), !A.PlayerDisplayName.EqualTo(A.Name)
			&& !B.PlayerDisplayName.EqualTo(B.Name) && A.Name.EqualTo(View.FullTime.PlayerA.Name) && B.Name.EqualTo(View.FullTime.PlayerB.Name));
	}
	auto* Panel = NewObject<UFMCodexFullTimePanelWidget>(); Panel->TakeWidget();
	auto P = DTO(View).FullTime;
	P.PlayerA.Score = P.PlayerB.Score = FText::FromString(TEXT("1"));
	P.PlayerA.Goals = {FText::FromString(TEXT("哲凯赖什"))};
	P.PlayerB.Goals = {FText::FromString(TEXT("格瓦迪奥尔"))};
	Panel->RefreshFromPresentation(P);
	auto* Player = CastChecked<UTextBlock>(Panel->GetWidgetFromName(TEXT("FullTimePlayerBName")));
	auto* Team = CastChecked<UTextBlock>(Panel->GetWidgetFromName(TEXT("FullTimeTeamBName")));
	auto* Score = CastChecked<UTextBlock>(Panel->GetWidgetFromName(TEXT("FullTimeTeamBScore")));
	TestTrue(TEXT("Primary participant is above and stronger than secondary team, below score emphasis"),
		Player->GetText().EqualTo(P.PlayerB.PlayerDisplayName) && Team->GetText().EqualTo(P.PlayerB.Name)
			&& Player->GetFont().Size > Team->GetFont().Size && Score->GetFont().Size > Player->GetFont().Size
			&& Player->GetColorAndOpacity().GetSpecifiedColor().GetLuminance() > Team->GetColorAndOpacity().GetSpecifiedColor().GetLuminance()
			&& CastChecked<UCanvasPanelSlot>(Player->Slot)->GetPosition().Y < CastChecked<UCanvasPanelSlot>(Team->Slot)->GetPosition().Y);
	const auto* ScoreSlot = CastChecked<UCanvasPanelSlot>(Score->Slot);
	const FVector2D ScorePosition = ScoreSlot->GetPosition(), ScoreSize = ScoreSlot->GetSize();
	const int32 NormalPlayerSize = Player->GetFont().Size;
	P.PlayerB.PlayerDisplayName = FText::FromString(TEXT("来自曼彻斯特的客队玩家"));
	P.PlayerB.Name = FText::FromString(TEXT("曼彻斯特城足球俱乐部"));
	P.PlayerB.Goals.Add(FText::FromString(TEXT("用于验证很长球员展示名保持单行的测试名称")));
	Panel->RefreshFromPresentation(P);
	TestTrue(TEXT("Long participant shrinks before a readable bounded ellipsis"),
		Player->GetFont().Size < NormalPlayerSize && Player->GetFont().Size >= 18 && !Player->GetAutoWrapText()
			&& Player->GetText().ToString().StartsWith(TEXT("来自曼彻斯特"))
			&& Player->GetText().ToString().EndsWith(TEXT("…"))
			&& P.PlayerB.PlayerDisplayName.ToString() == TEXT("来自曼彻斯特的客队玩家"));
	TestTrue(TEXT("Long identities do not move or resize score slot"), ScoreSlot->GetPosition() == ScorePosition && ScoreSlot->GetSize() == ScoreSize);
	for (const TCHAR* Name : {TEXT("FullTimeAGoalList"), TEXT("FullTimeBGoalList")})
	{
		auto* List = CastChecked<UScrollBox>(Panel->GetWidgetFromName(Name));
		for (int32 I = 0; I < List->GetChildrenCount(); ++I)
		{
			auto* Label = GoalLabel(*List, I);
			TestTrue(TEXT("Every scorer row fills its column and has single-line left-aligned text"),
				CastChecked<UScrollBoxSlot>(List->GetChildAt(I)->Slot)->GetHorizontalAlignment() == HAlign_Fill
					&& IsLeftAligned(*Label) && !Label->GetAutoWrapText() && Label->GetFont().Size >= 14);
		}
	}
	P.PlayerA.Goals.Reset(); P.PlayerB.Goals.Reset();
	Panel->RefreshFromPresentation(P);
	for (const TCHAR* Name : {TEXT("FullTimeAGoalList"), TEXT("FullTimeBGoalList")})
	{
		auto* List = CastChecked<UScrollBox>(Panel->GetWidgetFromName(Name));
		TestEqual(TEXT("Empty side has exactly one structured row"), List->GetChildrenCount(), 1);
		TestTrue(TEXT("Empty dash uses the same left-aligned name slot, not a centered standalone label"),
			GoalLabel(*List)->GetText().ToString() == TEXT("—") && IsLeftAligned(*GoalLabel(*List)));
	}
	TestEqual(TEXT("Shared scorer heading"), CastChecked<UTextBlock>(Panel->GetWidgetFromName(TEXT("FullTimeScorersHeading")))->GetText().ToString(), FString(TEXT("进球球员")));
	return true;
}

namespace FMCodexFullTimeTests
{
	TSharedPtr<SWidget> FindSlateWidget(const TSharedRef<SWidget>& Root, const TFunctionRef<bool(SWidget&)>& Predicate)
	{
		if (Predicate(Root.Get())) return Root;
		auto* Children = Root->GetChildren();
		for (int32 I = 0; I < Children->Num(); ++I)
			if (auto Found = FindSlateWidget(Children->GetChildAt(I), Predicate)) return Found;
		return nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFullTimeDevAttentionTest,
	"FMCodex.LocalPlay.FullTime.05.DevCollapseAcknowledgeAndRestart",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFullTimeDevAttentionTest::RunTest(const FString&)
{
	using namespace FMCodexFullTimeTests;
	FWorldFixture F;
	F.Controller->InitializePlayerFacingUI();
	auto* Screen = F.Controller->GetPlayerMatchScreen();
	if (!TestNotNull(TEXT("Real controller-owned screen"), Screen)) return false;
	const TSharedRef<SWidget> LiveScreen = Screen->TakeWidget();
	F.Controller->StartNewDevShortMatch();
	const auto Dev = SNew(SFMCodexLocalDevRollOverrideWidget).Controller(F.Controller);
	const auto Controls = StaticCastSharedPtr<SExpandableArea>(FindSlateWidget(Dev, [](SWidget& W) { return W.GetType() == TEXT("SExpandableArea"); }));
	if (!TestTrue(TEXT("Existing DEV expandable area exists"), Controls.IsValid())) return false;
	Controls->SetExpanded(true);
	TestTrue(TEXT("DEV can expand before full-time"), Controls->IsExpanded() && Controls->IsEnabled());
	for (int32 I = 0; I < 2; ++I)
	{
		FName Scorer;
		if (!Penalty(*this, F, false, Scorer)) return false;
		F.Controller->AdvanceAfterTerminal();
	}
	const auto Before = Bytes(F.Host->GetMatchSnapshot().Snapshot);
	Dev->Tick(FGeometry(), 0, 0);
	Dev->SlatePrepass();
	TestTrue(TEXT("Unacknowledged result compacts and disables DEV expansion"), !Controls->IsExpanded() && !Controls->IsEnabled());
	Screen->GetFullTimePanel()->Acknowledge();
	Dev->Tick(FGeometry(), 0, 0);
	Dev->SlatePrepass();
	TestTrue(TEXT("Acknowledgement restores affordance without expanding over result"), Controls->IsEnabled() && !Controls->IsExpanded());
	TestTrue(TEXT("DEV attention and acknowledgement never mutate authority"), Before == Bytes(F.Host->GetMatchSnapshot().Snapshot));
	Controls->SetExpanded(true);
	const auto Restart = StaticCastSharedPtr<SButton>(FindSlateWidget(Dev, [](SWidget& W)
	{
		if (W.GetType() != TEXT("SButton")) return false;
		return FindSlateWidget(W.AsShared(), [](SWidget& Child)
		{
			return Child.GetType() == TEXT("STextBlock")
				&& static_cast<STextBlock&>(Child).GetText().ToString() == TEXT("重开：双方各 1 次进攻回合");
		}).IsValid();
	}));
	if (!TestTrue(TEXT("Existing DEV short-match button remains reachable"), Restart.IsValid())) return false;
	Restart->SimulateClick();
	Dev->Tick(FGeometry(), 0, 0);
	Dev->SlatePrepass();
	TestTrue(TEXT("Actual DEV button restarts short match and resets acknowledgement"),
		!Screen->GetFullTimePanel()->GetPresentation().bVisible && !Screen->GetFullTimePanel()->IsAcknowledged()
			&& F.Controller->GetInteractionView().PlayerAMaxAttackTurns == 1
			&& F.Controller->GetInteractionView().PlayerBMaxAttackTurns == 1 && Controls->IsEnabled());
	return true;
}

#endif
