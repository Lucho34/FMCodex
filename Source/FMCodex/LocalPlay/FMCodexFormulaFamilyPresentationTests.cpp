#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"
#include "FMCodexThroughBallResolutionSurfaceWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexRollReelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Misc/AutomationTest.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

namespace FMCodexFormulaFamilyTests
{
FFMCodexUMGInlineFormulaSurfaceViewModel Pending()
{
	FFMCodexUMGInlineFormulaSurfaceViewModel P;
	P.bVisible = true;
	P.ContestId = TEXT("LongFreeKick.Direct");
	P.ContestLabel = TEXT("远距离任意球 · 直接射门");
	P.StatusLabel = TEXT("等待进攻方掷点");
	P.bAttackRowActive = true;
	P.PrimaryAction.bVisible = P.PrimaryAction.Action.bAvailable = true;
	P.PrimaryAction.Action.Label = TEXT("进攻方掷点");
	P.AttackRow.SideLabel = TEXT("进攻方");
	P.DefenseRow.SideLabel = TEXT("防守方");
	P.AttackRow.KnownNonRollSubtotalLabel = TEXT("主罚球员远射");
	P.DefenseRow.KnownNonRollSubtotalLabel = TEXT("门将站位与防守加成");
	for (auto* Row : {&P.AttackRow, &P.DefenseRow})
	{
		Row->bDisplayedResultResolved = Row->bKnownNonRollSubtotalResolved = true;
		Row->DisplayedResultLabel = Row == &P.AttackRow ? TEXT("3") : TEXT("7");
		Row->FinalValue = 777; // Deliberately unavailable raw value must never render.
		FFMCodexUMGInlineFormulaParticipantViewModel Actor;
		Actor.RoleLabel = Row == &P.AttackRow ? TEXT("主罚球员") : TEXT("门将");
		Actor.PlayerName = Row == &P.AttackRow ? TEXT("苏比门迪") : TEXT("多纳鲁马");
		Row->Participants.Add(Actor);
		FFMCodexUMGInlineFormulaTermViewModel Attribute;
		Attribute.DisplayLabel = Row == &P.AttackRow ? TEXT("远射 3") : TEXT("站位 5");
		Row->Terms.Add(Attribute);
		FFMCodexUMGInlineFormulaTermViewModel Roll;
		Roll.Kind = EFMCodexUMGInlineFormulaTermKind::RawRoll;
		Roll.DisplayLabel = TEXT("掷点 ?");
		Roll.bNextPendingRoll = Row == &P.AttackRow;
		Row->Terms.Add(Roll);
	}
	return P;
}
UTextBlock* Text(UUserWidget& W, const TCHAR* Name)
{
	return CastChecked<UTextBlock>(W.GetWidgetFromName(Name));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFormulaValueReadabilityTest,
	"FMCodex.LocalPlay.FormulaFamily.ReadableStatesAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFormulaValueReadabilityTest::RunTest(const FString&)
{
	using namespace FMCodexFormulaFamilyTests;
	auto* W = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>();
	const auto Slate = W->TakeWidget();
	auto P = Pending();
	W->RefreshFromPresentation(P);
	auto* Value = Text(*W,TEXT("InlineFormulaAttackFinalValue"));
	auto* Label = Text(*W,TEXT("InlineFormulaAttackValueState"));
	const int32 SubtotalSize = Value->GetFont().Size;
	const FLinearColor SubtotalColor = Value->GetColorAndOpacity().GetSpecifiedColor();
	TestEqual(TEXT("Pending uses explicit public subtotal label"),Label->GetText().ToString(),FString(TEXT("当前值")));
	TestEqual(TEXT("Only already-projected value renders"),Value->GetText().ToString(),FString(TEXT("3")));
	TestTrue(TEXT("Subtotal is cool, not result gold"),SubtotalColor.B > SubtotalColor.R);
	TestNull(TEXT("Unknown roll is never equated to a resolved subtotal"),W->GetWidgetFromName(TEXT("InlineFormulaAttackEquals")));
	auto* Operand = CastChecked<UBorder>(W->GetWidgetFromName(TEXT("InlineFormulaAttackOperand1")));
	const FLinearColor PendingColor = Operand->Background.TintColor.GetSpecifiedColor();
	TestTrue(TEXT("Pending roll is cyan, not gold"),PendingColor.B > PendingColor.R);
	auto* Terms = CastChecked<UWrapBox>(W->GetWidgetFromName(TEXT("InlineFormulaAttackTerms")));
	auto* FirstItem = Terms->GetChildAt(0);
	P.AttackRow.bDisplayedResultIsFinalValue = true;
	P.AttackRow.DisplayedResultLabel = TEXT("7");
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Disclosed row is labelled final"),Label->GetText().ToString(),FString(TEXT("最终值")));
	TestTrue(TEXT("Final value has distinctly greater visual weight"),Value->GetFont().Size >= SubtotalSize+10
		&& Value->GetColorAndOpacity().GetSpecifiedColor().R > Value->GetColorAndOpacity().GetSpecifiedColor().B);
	TestEqual(TEXT("Other row stays a subtotal"),Text(*W,TEXT("InlineFormulaDefenseValueState"))->GetText().ToString(),FString(TEXT("当前值")));
	P.DefenseRow.bDisplayedResultIsFinalValue = true;
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Both sides use final styling irrespective of winner"),Text(*W,TEXT("InlineFormulaDefenseValueState"))->GetText().ToString(),FString(TEXT("最终值")));
	P = Pending();
	P.AttackRow.bDisplayedResultResolved = false;
	P.AttackRow.DisplayedResultLabel = TEXT("?");
	P.PrimaryAction.Action.bAvailable = false;
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Unavailable value has neutral pending label"),Label->GetText().ToString(),FString(TEXT("当前值")));
	TestEqual(TEXT("Unknown value is unchanged"),Value->GetText().ToString(),FString(TEXT("?")));
	TestTrue(TEXT("No stale final gold after pooled reuse"),Value->GetColorAndOpacity().GetSpecifiedColor().Equals(SubtotalColor));
	TestTrue(TEXT("No duplicate terms on reuse"),Terms->GetChildAt(0)==FirstItem && Terms->GetChildrenCount()==2);
	TestFalse(TEXT("Unavailable CTA remains disabled"),CastChecked<UButton>(W->GetWidgetFromName(TEXT("InlineFormulaContinueButton")))->GetIsEnabled());
	// Exercise a real shared wrapping boundary, including multiple participants and modifiers.
	P = Pending();
	P.AttackRow.Participants[0].PlayerName = TEXT("刘易斯·斯凯利");
	auto SecondParticipant = P.AttackRow.Participants[0];
	SecondParticipant.RoleLabel = TEXT("跑位");
	SecondParticipant.PlayerName = TEXT("哈弗茨");
	P.AttackRow.Participants.Add(SecondParticipant);
	for (int32 I=0; I<5; ++I)
	{
		auto Term = P.AttackRow.Terms[0];
		Term.DisplayLabel = TEXT("防守加成 +2");
		P.AttackRow.Terms.Add(Term);
	}
	W->RefreshFromPresentation(P);
	auto* Renderer = new FWidgetRenderer(true);
	auto* Target = Renderer->DrawWidget(Slate,FVector2D(760,650));
	Renderer->DrawWidget(Target,Slate,FVector2D(760,650),0.f);
	for (int32 I=0; I<Terms->GetChildrenCount(); ++I)
	{
		auto* Item = Terms->GetChildAt(I);
		TestTrue(TEXT("Formula groups fit the allocated column"),Item->GetCachedGeometry().GetLocalSize().X <= Terms->GetCachedGeometry().GetLocalSize().X+.5f);
	}
	TestTrue(TEXT("Long formula wraps across rows without crowding summary"),Terms->GetDesiredSize().Y > 40.f);
	BeginCleanup(Renderer);
	// Identity typography is one role contract on both sides, not fixed pixel tokens.
	for (const TCHAR* Suffix : {TEXT("ParticipantRole0"),TEXT("ParticipantName0")})
	{
		auto* A = Text(*W,*(FString(TEXT("InlineFormulaAttack"))+Suffix));
		auto* D = Text(*W,*(FString(TEXT("InlineFormulaDefense"))+Suffix));
		TestTrue(TEXT("Both identity rows share one font role"),A->GetFont().Size == D->GetFont().Size
			&& A->GetFont().TypefaceFontName == D->GetFont().TypefaceFontName && A->GetFont().FontObject == D->GetFont().FontObject);
	}
	auto* AP = CastChecked<UBorder>(W->GetWidgetFromName(TEXT("InlineFormulaAttackParticipant0")));
	auto* DP = CastChecked<UBorder>(W->GetWidgetFromName(TEXT("InlineFormulaDefenseParticipant0")));
	TestTrue(TEXT("Identity capsules share padding and border vocabulary"),AP->GetPadding() == DP->GetPadding()
		&& AP->Background.OutlineSettings.Width == DP->Background.OutlineSettings.Width);
	for (const TCHAR* Name : {TEXT("InlineFormulaAttackMotif"),TEXT("InlineFormulaDefenseMotif")})
	{
		auto* Motif = CastChecked<UFMCodexMatchFlowDiagram>(W->GetWidgetFromName(Name));
		TestTrue(TEXT("Role motif is decorative, with the shared viewport"),Motif->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& !Motif->TakeWidget()->SupportsKeyboardFocus() && Motif->GetDesiredSize().Equals(UFMCodexMatchFlowDiagram::ViewportSize()));
	}
	P = Pending();
	P.bDiceRevealVisible = P.bDiceRolling = true;
	P.DiceOwnerLabel = TEXT("进攻方掷点");
	P.RevealPhase = EFMCodexUMGInlineFormulaRevealPhase::Cycling;
	P.RollReel.bVisible = P.RollReel.bMoving = P.RollReel.bShowNeighborDigits = true;
	P.RollReel.PreviousValue = 2; P.RollReel.CenterValue = 3; P.RollReel.NextValue = 4;
	P.RollReel.ScrollAlpha = .4f;
	P.PrimaryAction.bVisible = false;
	W->RefreshFromPresentation(P);
	auto* Reel = W->GetRollReelWidget();
	TestTrue(TEXT("Formula uses the approved clipped reel without changing its data"),Reel->HasClippedWindow()
		&& Reel->GetPresentation().CenterValue == P.RollReel.CenterValue
		&& Reel->GetPresentation().ScrollAlpha == P.RollReel.ScrollAlpha && !Reel->GetPresentation().bAuthoritativeValue);
	TestEqual(TEXT("One natural owner/phase line"),Text(*W,TEXT("InlineFormulaDiceOwner"))->GetText().ToString(),FString(TEXT("进攻方掷点中")));
	TestTrue(TEXT("Compact reveal has a family role, no duplicate status or CTA"),
		CastChecked<UFMCodexMatchFlowPanel>(W->GetWidgetFromName(TEXT("InlineFormulaDiceRevealRegion")))->GetFormulaRole() == EFMCodexFormulaPanelRole::RollHost
		&& Text(*W,TEXT("InlineFormulaStatus"))->GetVisibility() == ESlateVisibility::Collapsed
		&& W->GetWidgetFromName(TEXT("InlineFormulaContinueBounds"))->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("Cycling never upgrades current amount to final"),Label->GetText().ToString(),FString(TEXT("当前值")));
	P = Pending(); P.bNarrativeAvailable = true;
	P.ContestLabel = TEXT("贝尔纳多远距离任意球直接射偏。");
	P.StatusLabel = TEXT("远距离任意球 · 直接射门 · 未进球");
	P.RouteResultLabel = TEXT("进攻方掷点：1");
	P.bShowDefenseRow = false;
	P.AttackRow.Terms.Last().bResolved = true;
	P.AttackRow.Terms.Last().DisplayLabel = TEXT("掷点 1");
	W->RefreshFromPresentation(P);
	TestEqual(TEXT("Complete canonical result title is preserved"),Text(*W,TEXT("InlineFormulaContestHeading"))->GetText().ToString(),P.ContestLabel);
	TestEqual(TEXT("Canonical key detail is preserved without parsing"),Text(*W,TEXT("InlineFormulaRouteResult"))->GetText().ToString(),P.RouteResultLabel);
	TestTrue(TEXT("Result hierarchy gives primary prose more weight than context"),
		Text(*W,TEXT("InlineFormulaContestHeading"))->GetFont().Size > Text(*W,TEXT("InlineFormulaStatus"))->GetFont().Size);
	TestTrue(TEXT("Revealed roll chip receives restrained result emphasis"),
		Text(*W,TEXT("InlineFormulaAttackOperandText1"))->GetColorAndOpacity().GetSpecifiedColor().R
		> Text(*W,TEXT("InlineFormulaAttackOperandText1"))->GetColorAndOpacity().GetSpecifiedColor().B);
	P = Pending(); W->RefreshFromPresentation(P);
	TestTrue(TEXT("Reuse clears prior roll and result furniture"),!Reel->GetPresentation().bVisible
		&& W->GetWidgetFromName(TEXT("InlineFormulaRollHostBounds"))->GetVisibility() == ESlateVisibility::Collapsed
		&& W->GetWidgetFromName(TEXT("InlineFormulaResultBadge"))->GetVisibility() == ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFormulaHostIsolationTest,
	"FMCodex.LocalPlay.FormulaFamily.EmbeddedHostIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFormulaHostIsolationTest::RunTest(const FString&)
{
	using namespace FMCodexFormulaFamilyTests;
	auto* Shot = NewObject<UFMCodexLongShotResolutionSurfaceWidget>(); Shot->TakeWidget();
	auto* Through = NewObject<UFMCodexThroughBallResolutionSurfaceWidget>(); Through->TakeWidget();
	FFMCodexUMGLongShotResolutionViewModel S;
	FFMCodexUMGThroughBallResolutionViewModel T;
	S.bVisible = T.bVisible = true;
	S.Formula = T.Formula = Pending();
	S.Formula.bParentOwnsContestHeading = T.Formula.bParentOwnsContestHeading = true;
	Shot->RefreshFromPresentation(S); Through->RefreshFromPresentation(T);
	auto* SF = CastChecked<UFMCodexMatchFlowPanel>(Shot->GetWidgetFromName(TEXT("LongShotProductionSurfaceFrame")));
	auto* TF = CastChecked<UFMCodexMatchFlowPanel>(Through->GetWidgetFromName(TEXT("ThroughBallProductionSurfaceFrame")));
	TestTrue(TEXT("Both actual formula hosts share Tier 2 decoration"),SF->IsFlowStyleEnabled() && TF->IsFlowStyleEnabled());
	for (auto* Host : {static_cast<UUserWidget*>(Shot), static_cast<UUserWidget*>(Through)})
	{
		const auto Name = Host == Shot ? TEXT("LongShotDirectSharedFormulaSurface") : TEXT("ThroughBallFeetSharedFormulaSurface");
		auto* Inner = CastChecked<UFMCodexInlineResolutionFormulaSurfaceWidget>(Host->GetWidgetFromName(Name));
		auto* Frame = CastChecked<UFMCodexMatchFlowPanel>(Inner->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")));
		TestTrue(TEXT("Nested formula has one outer shell"),!Frame->IsFlowStyleEnabled() && Frame->GetBrushColor().A == 0.f);
	}
	T.Formula.bParentOwnsContestHeading = false;
	Through->RefreshFromPresentation(T);
	auto* Inner = CastChecked<UFMCodexInlineResolutionFormulaSurfaceWidget>(Through->GetWidgetFromName(TEXT("ThroughBallFeetSharedFormulaSurface")));
	TestFalse(TEXT("Embedded geometry is independent of heading ownership"),
		CastChecked<UFMCodexMatchFlowPanel>(Inner->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled());
	S.Formula.bShowFormulaRows = T.Formula.bShowFormulaRows = false;
	Shot->RefreshFromPresentation(S); Through->RefreshFromPresentation(T);
	TestTrue(TEXT("Outcome-only consumers retain legacy appearance on reuse"),!SF->IsFlowStyleEnabled() && !TF->IsFlowStyleEnabled());
	return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "PlayInEditorDataTypes.h"
#include "Settings/LevelEditorPlaySettings.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Widgets/SWindow.h"
#include "Widgets/Layout/SBox.h"

namespace
{
class FStartFormulaFamilyPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
		Settings->NewWindowWidth = 1600; Settings->NewWindowHeight = 900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams P; P.EditorPlaySettings = Settings;
		GEditor->RequestPlaySession(P);
		return true;
	}
};

class FFormulaFamilyPIE final : public IAutomationLatentCommand
{
public:
	explicit FFormulaFamilyPIE(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds()-Started > 90) { Test->AddError(TEXT("Formula PIE path timed out")); return true; }
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C = Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S = C ? C->GetPlayerMatchScreen() : nullptr;
		if (!S || FPlatformTime::Seconds()-Changed < .8) return false;
		if (Step == 0) { S->RequestStartNewMatch(); Advance(); return false; }
		if (Step == 1)
		{
			if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,9)) return true;
			S->RequestRollTacticalPoints(); Advance(); return false;
		}
		if (S->IsInlineFormulaRevealInputBlocked())
		{
			if (Step == 6 || Step == 7)
			{
				const auto& P = S->GetInlineFormulaSurface()->GetPresentation();
				const auto& Row = Step == 6 ? P.AttackRow : P.DefenseRow;
				// At the first blocked frame, the accepted roll has not visibly revealed.
				if (!bCheckedGate)
				{
					Test->TestFalse(TEXT("New roll is not styled final before visible reveal"),Row.bDisplayedResultIsFinalValue);
					bCheckedGate = true;
				}
				if (Step == 6 && !bCapturedRolling && P.RevealPhase == EFMCodexUMGInlineFormulaRevealPhase::Cycling)
				{
					Capture(S->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Formula_Rolling_After.png"));
					bCapturedRolling = true;
				}
				if (Step == 7 && !bCapturedFinal && P.AttackRow.bDisplayedResultIsFinalValue && P.DefenseRow.bDisplayedResultIsFinalValue)
				{
					Capture(S->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Formula_Final_After.png"));
					bCapturedFinal = true;
				}
			}
			return false;
		}
		if (Step == 2)
		{
			if (!Test->TestTrue(TEXT("Natural D12 reaches type information"),C->GetInteractionView().InteractionCategory == EFMCodexLocalMatchInteractionCategory::RollSetPieceType)) return true;
			if (!Override(*C,EFMCodexLocalDevRollTarget::SetPieceType,3)) return true;
			S->RequestContinueResolution(); Advance(); return false;
		}
		if (Step == 3)
		{
			const auto& V = C->GetInteractionView();
			if (V.LegalSetPieceCardIds.IsEmpty()) { Test->AddError(TEXT("No legal taker projected")); return true; }
			C->ToggleSetPieceDraftCard(V.LegalSetPieceCardIds[0]);
			CastChecked<UButton>(S->GetWidgetFromName(TEXT("SetPieceProductionPrimaryAction")))->OnClicked.Broadcast();
			Advance(); return false;
		}
		if (Step == 4)
		{
			if (!Test->TestTrue(TEXT("Original taker confirmation reaches method selection"),C->GetInteractionView().InteractionCategory == EFMCodexLocalMatchInteractionCategory::SelectSetPieceMethod)) return true;
			auto* Direct = CastChecked<UButton>(S->GetWidgetFromName(TEXT("LongDirectMethod")));
			if (!Test->TestTrue(TEXT("Original direct-shot handler is available"),Direct->GetIsEnabled())) return true;
			Direct->OnClicked.Broadcast(); Advance(); return false;
		}
		if (Step == 8)
		{
			Test->TestTrue(TEXT("Original next-action handler completes the terminal handoff"),C->GetLastDiagnostic().bHostSuccess && !C->GetInteractionView().bTerminalPendingAdvance);
			return true;
		}
		auto* Formula = S->GetInlineFormulaSurface();
		const auto& P = Formula->GetPresentation();
		if (!Test->TestTrue(TEXT("Real long free kick formula remains visible"),P.bVisible && P.bShowFormulaRows)) return true;
		if (Step == 5)
		{
			Test->TestFalse(TEXT("Attack begins as public subtotal"),P.AttackRow.bDisplayedResultIsFinalValue);
			Test->TestFalse(TEXT("Defense begins as public subtotal"),P.DefenseRow.bDisplayedResultIsFinalValue);
			Test->TestEqual(TEXT("Public subtotal has explicit UI label"),FMCodexFormulaFamilyTests::Text(*Formula,TEXT("InlineFormulaAttackValueState"))->GetText().ToString(),FString(TEXT("当前值")));
			// Context must precede native framing: resizing the offscreen window
			// needs a game/layout tick before its viewport scale settles again.
			CaptureContext(TEXT("Formula_Family_Context.png"));
			Capture(Formula->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Formula_PreRoll_After.png"));
			if (!Override(*C,EFMCodexLocalDevRollTarget::LongFreeKickDirectAttack,4)) return true;
			Formula->RequestContinue(); Advance(); return false;
		}
		if (Step == 6)
		{
			Test->TestTrue(TEXT("Attack amount updates after the existing reveal gate"),bCheckedGate && P.AttackRow.bDisplayedResultResolved);
			// Set-piece projection finalizes both rows at contest completion, unlike
			// some other formula consumers. A disclosed first roll is still intermediate.
			Test->TestFalse(TEXT("First roll alone does not finalize the set-piece contest"),P.AttackRow.bDisplayedResultIsFinalValue);
			Test->TestEqual(TEXT("Intermediate amount keeps the projected nonfinal label"),FMCodexFormulaFamilyTests::Text(*Formula,TEXT("InlineFormulaAttackValueState"))->GetText().ToString(),FString(TEXT("当前值")));
			Test->TestFalse(TEXT("Unrolled defense stays a public subtotal"),P.DefenseRow.bDisplayedResultIsFinalValue);
			if (!Override(*C,EFMCodexLocalDevRollTarget::LongFreeKickDirectDefense,4)) return true;
			Formula->RequestContinue(); Advance(); return false;
		}
		Test->TestTrue(TEXT("Both final values appear only after their original reveal"),bCheckedGate && P.AttackRow.bDisplayedResultIsFinalValue && P.DefenseRow.bDisplayedResultIsFinalValue);
		Test->TestEqual(TEXT("Final label follows the displayed result flag"),FMCodexFormulaFamilyTests::Text(*Formula,TEXT("InlineFormulaDefenseValueState"))->GetText().ToString(),FString(TEXT("最终值")));
		Test->TestTrue(TEXT("Single representative path captured cycling and final disclosure"),bCapturedRolling && bCapturedFinal);
		Test->TestTrue(TEXT("Result keeps the canonical narrative and original action"),P.bNarrativeAvailable && P.PrimaryAction.bVisible && P.PrimaryAction.Action.bAvailable);
		Capture(Formula->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("Formula_Result_After.png"));
		Formula->RequestContinue(); Advance(); return false;
	}
private:
	void Advance() { ++Step; Changed = FPlatformTime::Seconds(); bCheckedGate = false; }
	bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target = Target; R.Value = Value;
		return Test->TestTrue(TEXT("Existing authoritative DEV provider accepts purpose override"),C.SetLocalDevRollOverride(R).bSuccess);
	}
	void Save(const TArray<FColor>& Pixels,const FIntVector& Size,const TCHAR* File)
	{
		const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_7C_1"));
		IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
		Test->AddInfo(FString::Printf(TEXT("FORMULA_CAPTURE %s %dx%d game=%.3f"),File,Size.X,Size.Y,GEditor->PlayWorld->GetTimeSeconds()));
	}
	void CaptureContext(const TCHAR* File)
	{
		auto Window = GEditor->PlayWorld->GetGameViewport()->GetWindow();
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		if (Window.IsValid() && FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(),Pixels,Size)) Save(Pixels,Size,File);
		else Test->AddError(TEXT("PIE context capture failed"));
	}
	void Capture(UWidget* Widget,const TCHAR* File)
	{
		// Existing live widget in the real PIE 2D window renderer. Temporarily frame
		// at native Slate size for readability; restore content and its parent.
		const auto Slate = Widget->TakeWidget(); Slate->SlatePrepass(1.f);
		const FVector2D NativeSize(FMath::CeilToInt(Widget->GetCachedGeometry().GetLocalSize().X),FMath::CeilToInt(Slate->GetDesiredSize().Y));
		auto Window = GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Window.IsValid()) { Test->AddError(TEXT("No PIE capture window")); return; }
		const auto OriginalContent = Window->GetContent();
		const auto OriginalParent = Slate->GetParentWidget();
		const FVector2D OriginalClientSize = Window->GetClientSizeInScreen();
		// A native formula can exceed the small offscreen PIE client. Give the
		// capture enough space so the footer/CTA is not clipped by that window.
		Window->Resize(FVector2D(FMath::Max(1000.0,NativeSize.X+80),FMath::Max(700.0,NativeSize.Y+80)));
		Window->SetContent(SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[SNew(SBox).WidthOverride(NativeSize.X).HeightOverride(NativeSize.Y)[Slate]]);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		TArray<FColor> Pixels; FIntVector Size = FIntVector::ZeroValue;
		const bool bRead = FSlateApplication::Get().TakeScreenshot(Slate,Pixels,Size);
		Window->SetContent(OriginalContent);
		if (OriginalParent.IsValid()) Slate->AssignParentWidget(OriginalParent);
		Window->Resize(OriginalClientSize);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		Test->TestTrue(TEXT("Native capture includes full formula height"),bRead && Size.Y >= FMath::FloorToInt(NativeSize.Y));
		if (bRead) Save(Pixels,Size,File); else Test->AddError(TEXT("Native PIE widget capture failed"));
	}
	FAutomationTestBase* Test;
	double Started = FPlatformTime::Seconds(), Changed = 0;
	int32 Step = 0;
	bool bCheckedGate = false;
	bool bCapturedRolling = false, bCapturedFinal = false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexFormulaFamilyPIETest,
	"FMCodex.PIE.FormulaFamily.LongFreeKickReadableStates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexFormulaFamilyPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartFormulaFamilyPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FFormulaFamilyPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
