#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Input/Events.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSetPieceFlowIsolationTest,
	"FMCodex.LocalPlay.SetPieceFlowPresentation.ModeIsolationAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSetPieceFlowIsolationTest::RunTest(const FString&)
{
	auto* Formula = NewObject<UFMCodexInlineResolutionFormulaSurfaceWidget>();
	const auto FormulaSlate = Formula->TakeWidget();
	FFMCodexUMGInlineFormulaSurfaceViewModel A;
	A.bVisible = true; A.ContestId = TEXT("SetPiece.Type");
	A.ContestLabel = TEXT("定位球类型"); A.bShowFormulaRows = false;
	A.TacticalPlayerSummaryLabel = FFMCodexPlayerUIPresentationText::SetPieceTypeRollHint().ToString();
	A.PrimaryAction.bVisible = true; A.PrimaryAction.Action.bAvailable = true;
	A.PrimaryAction.Action.Label = TEXT("掷定位球类型");
	Formula->RefreshFromPresentation(A);
	auto* Frame = CastChecked<UFMCodexMatchFlowPanel>(Formula->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")));
	auto* Rules = CastChecked<UUniformGridPanel>(Formula->GetWidgetFromName(TEXT("SetPieceTypeRules")));
	auto* RuleBody = Formula->GetWidgetFromName(TEXT("SetPieceTypeInformationBody"));
	auto* Action = CastChecked<UButton>(Formula->GetWidgetFromName(TEXT("InlineFormulaContinueButton")));
	TestTrue(TEXT("Stable type mode opts in; rule subtree cannot intercept input"),
		Frame->IsFlowStyleEnabled() && Rules->GetChildrenCount() == 4
		&& RuleBody->GetVisibility() == ESlateVisibility::HitTestInvisible);
	const TCHAR* Ranges[] = {TEXT("1–2"), TEXT("3–4"), TEXT("5"), TEXT("6")};
	const TCHAR* Types[] = {TEXT("角球"), TEXT("远距离任意球"), TEXT("近距离任意球"), TEXT("点球")};
	auto* TypeRenderer = new FWidgetRenderer(true);
	auto* TypeTarget = TypeRenderer->DrawWidget(FormulaSlate,FVector2D(760,430));
	TypeRenderer->DrawWidget(TypeTarget,FormulaSlate,FVector2D(760,430),0.f);
	for (int32 I=0; I<4; ++I)
	{
		const FString Prefix = FString::Printf(TEXT("SetPieceTypeRule%d"), I);
		const auto* Range = CastChecked<UTextBlock>(Formula->GetWidgetFromName(FName(*(Prefix+TEXT("Range")))));
		const auto* Type = CastChecked<UTextBlock>(Formula->GetWidgetFromName(FName(*(Prefix+TEXT("Type")))));
		TestTrue(TEXT("Every static range/name matches the central hint"),
			Range->GetText().ToString() == Ranges[I] && Type->GetText().ToString() == Types[I]
			&& A.TacticalPlayerSummaryLabel.Contains(FString(Ranges[I])+TEXT("：")+Types[I]));
		TestFalse(TEXT("Rule item is not a button or keyboard focus target"),
			Rules->GetChildAt(I)->IsA<UButton>() || Rules->GetChildAt(I)->TakeWidget()->SupportsKeyboardFocus());
		auto* Diagram=CastChecked<UFMCodexMatchFlowDiagram>(Formula->GetWidgetFromName(FName(*(Prefix+TEXT("Diagram")))));
		TestTrue(TEXT("All four static diagrams use one exact viewport and no input/focus"),
			Diagram->GetCachedGeometry().GetLocalSize().Equals(UFMCodexMatchFlowDiagram::ViewportSize(),.1f)
			&& Diagram->GetVisibility()==ESlateVisibility::HitTestInvisible
			&& !Diagram->TakeWidget()->SupportsKeyboardFocus());
	}
	BeginCleanup(TypeRenderer);
	TestTrue(TEXT("Existing sole action and focus behavior retained"), Action->GetIsEnabled() && Action->GetIsFocusable());
	A.PrimaryAction.bVisible = false; A.PrimaryAction.Action.bAvailable = false;
	Formula->RefreshFromPresentation(A);
	TestTrue(TEXT("Read-only type view has no enabled footer action"),
		!Action->GetIsEnabled() && Action->GetParent()->GetVisibility() == ESlateVisibility::Collapsed);
	A.bDiceRevealVisible = true; A.TacticalPlayerSummaryLabel.Empty();
	Formula->RefreshFromPresentation(A);
	TestTrue(TEXT("Type reel keeps its original host and no static result highlight"),
		!Frame->IsFlowStyleEnabled() && RuleBody->GetVisibility() == ESlateVisibility::Collapsed);
	FFMCodexUMGInlineFormulaSurfaceViewModel D;
	D.bVisible=true; D.ContestId=TEXT("Fixture.Formula");
	D.ContestLabel=TEXT("定位球类型"); // Same localized title cannot opt in.
	D.TacticalPlayerSummaryLabel=A.TacticalPlayerSummaryLabel;
	D.bShowFormulaRows=true; D.bShowAttackRow=true;
	D.AttackRow.bKnownNonRollSubtotalResolved=true; D.AttackRow.KnownNonRollSubtotal=5;
	D.AttackRow.bDisplayedResultResolved=true; D.AttackRow.DisplayedResultLabel=TEXT("5");
	Formula->RefreshFromPresentation(D);
	TestTrue(TEXT("Reused formula retains original row data, font and action bounds"),
		!Frame->IsFlowStyleEnabled() && Formula->GetPresentation().AttackRow.KnownNonRollSubtotal == 5
		&& !CastChecked<UFMCodexMatchFlowButton>(Action)->IsFlowStyleEnabled()
		&& !Formula->GetPresentation().AttackRow.bFinalValueResolved
		&& CastChecked<UTextBlock>(Formula->GetWidgetFromName(TEXT("InlineFormulaAttackFinalValue")))->GetText().ToString()==TEXT("5")
		&& CastChecked<USizeBox>(Action->GetParent())->GetWidthOverride()==156.f);
	Formula->RefreshFromPresentation({});
	TestTrue(TEXT("Hidden type state clears decoration"), !Frame->IsFlowStyleEnabled());

	// Presentation fixtures exercise the shared Screen with already-safe values.
	// They do not manufacture an authoritative match or exercise transport.
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>();
	const auto SlateScreen = Screen->TakeWidget();
	FFMCodexUMGMatchScreenViewModel M;
	M.SetPiece.bVisible=true; M.SetPiece.bSelectionSupported=true; M.SetPiece.bMethodWait=true;
	M.SetPiece.AttackingSide=EInitialTurnOrderPlayer::PlayerA;
	M.SetPiece.ActingSide=EInitialTurnOrderPlayer::PlayerA;
	M.SetPiece.TakerCardId=TEXT("Fixture.Player");
	auto& Cell=M.LocalRack.Cells.AddDefaulted_GetRef();
	Cell.Card.CardId=M.SetPiece.TakerCardId;
	Cell.Card.IdentityLabel=TEXT("亚历山大·阿诺德");
	auto* Renderer = new FWidgetRenderer(true);
	const ESetPieceSelectedType TypesToCheck[] = {ESetPieceSelectedType::ShortFreeKick,
		ESetPieceSelectedType::LongFreeKick, ESetPieceSelectedType::Penalty, ESetPieceSelectedType::Corner};
	const TCHAR* Buttons[][2] = {{TEXT("ShortDirectMethod"),TEXT("ShortAngledMethod")},
		{TEXT("LongDirectMethod"),TEXT("LongPowerMethod")}, {TEXT("PenaltyDirectMethod"),TEXT("PenaltyPanenkaMethod")},
		{TEXT("CornerHighIntent"),TEXT("CornerLowIntent")}};
	for (int32 I=0; I<4; ++I)
	{
		M.SetPiece.Type=TypesToCheck[I];
		M.SetPiece.TypeLabel=FFMCodexPlayerUIPresentationText::SetPieceName(TypesToCheck[I]);
		M.SetPiece.bMethodWait=I!=3; M.SetPiece.bCornerIntentWait=I==3;
		Screen->RefreshFromPresentation(M);
		auto* Target = Renderer->DrawWidget(SlateScreen, FVector2D(1920,1080));
		Renderer->DrawWidget(Target, SlateScreen, FVector2D(1920,1080), 0.f);
		auto* Left=CastChecked<UButton>(Screen->GetWidgetFromName(Buttons[I][0]));
		auto* Right=CastChecked<UButton>(Screen->GetWidgetFromName(Buttons[I][1]));
		TestTrue(TEXT("Each method family uses aligned equal-width cards"),
			Left->GetCachedGeometry().GetLocalSize().X > 200
			&& Left->GetCachedGeometry().GetLocalSize().Equals(Right->GetCachedGeometry().GetLocalSize(), .1f)
			&& CastChecked<UHorizontalBoxSlot>(Left->Slot)->GetSize().SizeRule==ESlateSizeRule::Fill);
		for (const TCHAR* Name : Buttons[I])
		{
			auto* Helper=CastChecked<UTextBlock>(Screen->GetWidgetFromName(FName(*(FString(Name)+TEXT("Helper")))));
			TestTrue(TEXT("Authored rule groups fit without tiny text or clipped lines"),
				Helper->GetDesiredSize().X <= Helper->GetCachedGeometry().GetLocalSize().X+.5f
				&& Helper->GetFont().Size >= 14);
			auto* Reason=CastChecked<UTextBlock>(Screen->GetWidgetFromName(FName(*(FString(Name)+TEXT("Reason")))));
			TestTrue(TEXT("Reserved reason region fits its complete explanation"),
				Reason->GetDesiredSize().X<=Reason->GetCachedGeometry().GetLocalSize().X+.5f);
		}
		const auto LeftSlate=Left->TakeWidget();
		const FPointerEvent Pointer(0,FVector2D::ZeroVector,FVector2D::ZeroVector,
			TSet<FKey>{EKeys::LeftMouseButton},EKeys::LeftMouseButton,0,FModifierKeysState());
		LeftSlate->OnMouseEnter(Left->GetCachedGeometry(),Pointer);
		TestTrue(TEXT("Native button hover and keyboard affordance survive the new paint layer"),
			LeftSlate->IsHovered() && LeftSlate->SupportsKeyboardFocus());
		LeftSlate->OnMouseButtonDown(Left->GetCachedGeometry(),Pointer);
		TestTrue(TEXT("Native press retains downward content padding"),Left->IsPressed()
			&& Left->GetStyle().PressedPadding.Top>Left->GetStyle().NormalPadding.Top);
		LeftSlate->OnMouseLeave(Pointer);
		LeftSlate->OnMouseCaptureLost(FCaptureLostEvent(0,0));
		if (I==0)
		{
			TestTrue(TEXT("Projected ineligible choice stays disabled with its explanation"),
				!Right->GetIsEnabled() && Left->GetIsEnabled()
				&& CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("ShortAngledMethodReason")))->GetText().ToString().Contains(TEXT("不满足"))
				&& !CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceChoiceFooter")))->GetText().ToString().Contains(TEXT("不满足")));
			M.SetPiece.NearMethods.Add(EMatchPlayShortFreeKickMethod::Angled);
			Cell.Card.IdentityLabel=TEXT("埃泽");
			Screen->RefreshFromPresentation(M);
			TestTrue(TEXT("Reuse clears prior eligibility reason and player identity"), Right->GetIsEnabled()
				&& CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("ShortAngledMethodReason")))->GetText().IsEmpty()
				&& !CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceChoiceFooter")))->GetText().ToString().Contains(TEXT("不满足"))
				&& CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceProductionStatus")))->GetText().ToString().Contains(TEXT("埃泽")));
		}
	}
	BeginCleanup(Renderer);
	struct FPendingBackend final : IFMCodexMatchScreenBackend
	{
		bool bPending=true;
		virtual bool IsScreenIntentPending() const override {return bPending;}
		virtual EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest&) override
		{return EFMCodexMatchScreenSubmission::Queued;}
	} Backend;
	Screen->SetMatchBackend(&Backend);
	Screen->RefreshFromPresentation(M);
	TestFalse(TEXT("Existing pending seam disables method input"),
		CastChecked<UButton>(Screen->GetWidgetFromName(TEXT("CornerHighIntent")))->GetIsEnabled());
	TestTrue(TEXT("Pending footer explains the temporary input block"),
		CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("SetPieceChoiceFooter")))->GetText().ToString().Contains(TEXT("正在提交")));
	Backend.bPending=false;
	Screen->RefreshFromPresentation(M);
	TestTrue(TEXT("Pending completion restores original projected input"),
		CastChecked<UButton>(Screen->GetWidgetFromName(TEXT("CornerHighIntent")))->GetIsEnabled());
	Screen->SetMatchBackend(nullptr);
	M.bActionWaitPromptReadOnly=true;
	Screen->RefreshFromPresentation(M);
	TestTrue(TEXT("Waiting viewer has no choice actions"),
		Screen->GetWidgetFromName(TEXT("SetPieceMethodChoiceRow"))->GetVisibility()==ESlateVisibility::Collapsed);
	M.bActionWaitPromptReadOnly=false; M.SetPiece.bCornerIntentWait=false;
	M.SetPiece.bTakerWait=true; M.SetPiece.bMethodWait=false;
	Screen->RefreshFromPresentation(M);
	TestFalse(TEXT("Taker selection retains original host decoration"),
		CastChecked<UFMCodexMatchFlowPanel>(Screen->GetWidgetFromName(TEXT("SetPieceProductionResolutionSurface")))->IsFlowStyleEnabled());
	M.SetPiece.bTakerWait=false; M.SetPiece.bCornerDraft=true;
	M.SetPiece.CornerStage=EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations;
	Screen->RefreshFromPresentation(M);
	TestFalse(TEXT("Corner draft never becomes a method-card surface"),
		CastChecked<UFMCodexMatchFlowPanel>(Screen->GetWidgetFromName(TEXT("SetPieceProductionResolutionSurface")))->IsFlowStyleEnabled());
	return true;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexLocalMatchPlayerController.h"
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
class FStartSetPieceFlowPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		auto* Settings=DuplicateObject<ULevelEditorPlaySettings>(GetDefault<ULevelEditorPlaySettings>(),GetTransientPackage());
		Settings->NewWindowWidth=1600; Settings->NewWindowHeight=900;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		FRequestPlaySessionParams P; P.EditorPlaySettings=Settings;
		GEditor->RequestPlaySession(P);
		return true;
	}
};

class FSetPieceFlowPIE final : public IAutomationLatentCommand
{
public:
	explicit FSetPieceFlowPIE(FAutomationTestBase* InTest):Test(InTest){}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds()-Started>75) {Test->AddError(TEXT("Set-piece PIE path timed out"));return true;}
		if (!GEditor || !GEditor->PlayWorld) return false;
		auto* C=Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S=C ? C->GetPlayerMatchScreen() : nullptr;
		if (!S || FPlatformTime::Seconds()-Changed<.8) return false;
		if (Step==0) {S->RequestStartNewMatch(); Advance(); return false;}
		if (Step==1)
		{
			if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,9)) return true;
			S->RequestRollTacticalPoints(); Advance(); return false;
		}
		if (S->IsInlineFormulaRevealInputBlocked()) return false;
		if (Step==2)
		{
			Test->TestTrue(TEXT("Natural D12 leads to type information A"),
				C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollSetPieceType);
			Capture(S->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")),TEXT("TypeInformation_After.png"));
			// One whole-window context capture checks the native-scale render's color.
			auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
			TArray<FColor> Pixels; FIntVector Size=FIntVector::ZeroValue;
			if (Window.IsValid() && FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(),Pixels,Size))
			{
				TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
				Test->TestTrue(TEXT("Unedited PIE window context saved"),FFileHelper::SaveArrayToFile(PNG,
					*(FPaths::ProjectSavedDir()/TEXT("Stage8_7B_1/TypeInformation_Context.png"))));
			}
			if (!Override(*C,EFMCodexLocalDevRollTarget::SetPieceType,5)) return true;
			S->RequestContinueResolution(); Advance(); return false;
		}
		if (Step==3)
		{
			const auto& V=C->GetInteractionView();
			if (V.LegalSetPieceCardIds.IsEmpty()) {Test->AddError(TEXT("No legal taker projected"));return true;}
			C->ToggleSetPieceDraftCard(V.LegalSetPieceCardIds[0]);
			// Selection is still local draft; the original primary handler confirms it.
			auto* Confirm=CastChecked<UButton>(S->GetWidgetFromName(TEXT("SetPieceProductionPrimaryAction")));
			Confirm->OnClicked.Broadcast(); Advance(); return false;
		}
		if (Step==4)
		{
			Test->TestTrue(TEXT("Confirmed taker reaches real method choice C"),
				C->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::SelectSetPieceMethod);
			Capture(S->GetWidgetFromName(TEXT("SetPieceProductionResolutionBounds")),TEXT("TacticalChoice_After.png"));
			auto* Unavailable=CastChecked<UButton>(S->GetWidgetFromName(TEXT("ShortAngledMethod")));
			Test->TestFalse(TEXT("Representative taker exposes the real disabled choice"),Unavailable->GetIsEnabled());
			Capture(Unavailable,TEXT("TacticalChoice_Disabled.png"));
			auto* Direct=CastChecked<UButton>(S->GetWidgetFromName(TEXT("ShortDirectMethod")));
			Test->TestTrue(TEXT("Direct choice is enabled and owns its original click"),Direct->GetIsEnabled());
			Direct->OnClicked.Broadcast(); Advance(); return false;
		}
		Test->TestTrue(TEXT("Original method handler enters unchanged opposed formula"),
			C->GetLastDiagnostic().bHostSuccess && S->GetInlineFormulaSurface()->GetPresentation().bShowFormulaRows
			&& !CastChecked<UFMCodexMatchFlowPanel>(S->GetInlineFormulaSurface()->GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")))->IsFlowStyleEnabled());
		return true;
	}
private:
	void Advance(){++Step; Changed=FPlatformTime::Seconds();}
	bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target=Target; R.Value=Value;
		return Test->TestTrue(TEXT("Existing authoritative DEV provider accepts purpose override"),C.SetLocalDevRollOverride(R).bSuccess);
	}
	void Capture(UWidget* Widget,const TCHAR* File)
	{
		// Use the real PIE window's 2D renderer, avoiding the different 3D render-target
		// gamma pipeline. Temporarily frame the existing widget at native Slate size,
		// then restore the window content. No view, action or gameplay phase changes.
		const auto Slate=Widget->TakeWidget();
		Slate->SlatePrepass(1.f);
		const FVector2D NativeSize(FMath::CeilToInt(Widget->GetCachedGeometry().GetLocalSize().X),
			FMath::CeilToInt(Slate->GetDesiredSize().Y));
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		if (!Window.IsValid()) {Test->AddError(TEXT("No PIE capture window"));return;}
		const auto OriginalContent=Window->GetContent();
		const auto OriginalParent=Slate->GetParentWidget();
		Window->SetContent(SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)
			[SNew(SBox).WidthOverride(NativeSize.X).HeightOverride(NativeSize.Y)[Slate]]);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		TArray<FColor> Pixels; FIntVector Size=FIntVector::ZeroValue;
		const bool bRead=FSlateApplication::Get().TakeScreenshot(Slate,Pixels,Size);
		Window->SetContent(OriginalContent);
		if (OriginalParent.IsValid()) Slate->AssignParentWidget(OriginalParent);
		Window->SlatePrepass(Window->GetDPIScaleFactor());
		if (!bRead) {Test->AddError(TEXT("Native PIE widget screenshot failed"));return;}
		const FString Dir=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_7B_1"));
		IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("Runtime evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
		Test->AddInfo(FString::Printf(TEXT("FLOW_CAPTURE %s %dx%d actual PIE widget, native-size window framing; game=%.3f"),File,Size.X,Size.Y,GEditor->PlayWorld->GetTimeSeconds()));
	}
	FAutomationTestBase* Test;
	double Started=FPlatformTime::Seconds(),Changed=0;
	int32 Step=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexSetPieceFlowPIETest,
	"FMCodex.PIE.SetPieceFlowPresentation.TypeToChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexSetPieceFlowPIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartSetPieceFlowPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FSetPieceFlowPIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
