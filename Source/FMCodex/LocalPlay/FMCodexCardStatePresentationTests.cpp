#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexHandMicroDiagnostics.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Misc/AutomationTest.h"
#include "Input/Events.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

namespace FMCodexCardStateTests
{
FFMCodexUMGCardRackViewModel Fixture()
{
	FFMCodexUMGCardRackViewModel P;
	P.bLocalRack = true; P.ColumnCount = 1; P.SideLabel = TEXT("Player A");
	for (int32 I = 0; I < 3; ++I)
	{
		auto& C = P.Cells.AddDefaulted_GetRef(); C.StableIndex = I;
		C.Card.CardId = I == 0 ? TEXT("Prototype.Arsenal.ChristianNorgaard")
			: I == 1 ? TEXT("Prototype.Arsenal.MylesLewisSkelly") : TEXT("Prototype.Arsenal.EberechiEze");
		C.Card.IdentityLabel = I == 0 ? TEXT("诺尔高") : I == 1 ? TEXT("刘易斯-斯凯利") : TEXT("埃泽");
		C.Card.RoleLabel = TEXT("MD"); C.Card.RarityLabel = TEXT("White");
		C.Card.AssignedPlayerNumber = FString::FromInt(15 + I);
		C.bSetPieceSelectable = C.bSetPieceSelected = true;
	}
	return P;
}
UWidget* State(UFMCodexCardRackWidget& Rack, int32 Index, const TCHAR* Role)
{
	return Rack.GetWidgetFromName(FName(*FString::Printf(TEXT("CardDraft%s%d"),Role,Index)));
}
// Inspect the current hierarchy, not detached named Widgets retained by UWidgetTree.
bool HasLiveLabel(UWidget* W)
{
	if (!W) return false;
	if (W->GetName().StartsWith(TEXT("CardDraftLabel"))) return true;
	if (auto* Panel = Cast<UPanelWidget>(W))
		for (auto* Child : Panel->GetAllChildren()) if (HasLiveLabel(Child)) return true;
	return false;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCardDraftStateTest,
	"FMCodex.LocalPlay.CardState.DraftOrderAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCardDraftStateTest::RunTest(const FString&)
{
	using namespace FMCodexCardStateTests;
	auto* Rack = NewObject<UFMCodexCardRackWidget>(); Rack->TakeWidget();
	auto P = Fixture(); Rack->RefreshFromPresentation(P);
	auto* Label = CastChecked<UTextBlock>(State(*Rack,0,TEXT("Label")));
	TestEqual(TEXT("Selection denotes draft, not confirmation"),Label->GetText().ToString(),FString(TEXT("已选")));
	TestTrue(TEXT("State is hit-test invisible and cannot focus"),
		State(*Rack,0,TEXT("StateBounds"))->GetVisibility() == ESlateVisibility::HitTestInvisible
		&& !State(*Rack,0,TEXT("StateBounds"))->TakeWidget()->SupportsKeyboardFocus());
	FName Requested;
	auto Handle = Rack->OnCardSelectionRequested.AddLambda([&Requested](FName Id){ Requested = Id; });
	TestTrue(TEXT("Original card selection forwards its original identity"),
		Rack->GetRenderedCardWidgets()[0]->RequestOnPitchSelection() && Requested == P.Cells[0].Card.CardId);
	Rack->OnCardSelectionRequested.Remove(Handle);
	for (int32 I = 0; I < 3; ++I) P.Cells[I].SetPieceSelectionOrder = I + 1;
	Rack->RefreshFromPresentation(P);
	for (int32 I = 0; I < 3; ++I)
	{
		auto* L = CastChecked<UTextBlock>(State(*Rack,I,TEXT("Label")));
		TestEqual(TEXT("Projected order retained with explicit order prefix"), L->GetText().ToString(),FString::Printf(TEXT("#%d"),I+1));
		TestTrue(TEXT("Order retains independent draft selection rail"),State(*Rack,I,TEXT("SelectedRail"))->GetVisibility() == ESlateVisibility::HitTestInvisible);
		TestTrue(TEXT("One badge family for every order"), L->GetFont() == CastChecked<UTextBlock>(State(*Rack,0,TEXT("Label")))->GetFont()
			&& CastChecked<USizeBox>(State(*Rack,I,TEXT("StateBounds")))->GetWidthOverride() == CastChecked<USizeBox>(State(*Rack,0,TEXT("StateBounds")))->GetWidthOverride());
		Rack->GetRenderedCardWidgets()[I]->TakeWidget(); // Realize the newly rebuilt card before querying rendered text.
		TestEqual(TEXT("Shirt identity stays independent"),Rack->GetRenderedCardWidgets()[I]->GetRenderedAssignedNumber().ToString(),P.Cells[I].Card.AssignedPlayerNumber);
	}
	P.Cells[0].bSetPieceSelected = false; P.Cells[0].SetPieceSelectionOrder = 0;
	P.Cells[1].SetPieceSelectionOrder = 1; P.Cells[2].SetPieceSelectionOrder = 2;
	Rack->RefreshFromPresentation(P);
	TestFalse(TEXT("Deselected card has no live selected or order label"),HasLiveLabel(Rack->GetRenderedCardWidgets()[0]->GetParent()));
	TestEqual(TEXT("Renumbered draft has no stale order"),CastChecked<UTextBlock>(State(*Rack,1,TEXT("Label")))->GetText().ToString(),FString(TEXT("#1")));
	for (auto& C : P.Cells) { C.bSetPieceSelectable = C.bSetPieceSelected = false; C.SetPieceSelectionOrder = 0; }
	P.Cells[0].bPlayed = true;
	Rack->RefreshFromPresentation(P);
	TestFalse(TEXT("Ghost and unavailable cells do not retain draft state"),HasLiveLabel(Rack->GetWidgetFromName(TEXT("StableTwoByTenCardRackGrid")))) ;
	TestEqual(TEXT("Ghost retains rack slot"),Rack->GetRenderedCellCount(),3);
	TestEqual(TEXT("Ghost has no interactive card"),Rack->GetRenderedCardWidgets().Num(),2);
	TestFalse(TEXT("Unavailable card did not acquire selection authority"),Rack->GetRenderedCardWidgets()[0]->IsSelectableForCurrentPrompt());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCardStateHoverTest,
	"FMCodex.LocalPlay.CardState.HoverAndSafeArea",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCardStateHoverTest::RunTest(const FString&)
{
	using namespace FMCodexCardStateTests;
	auto* Rack = NewObject<UFMCodexCardRackWidget>(); auto Slate = Rack->TakeWidget();
	auto P = Fixture(); P.Cells.SetNum(1); Rack->RefreshFromPresentation(P);
	auto* Card = Rack->GetRenderedCardWidgets()[0].Get();
	auto* Renderer = new FWidgetRenderer(true);
	auto* Target = Renderer->DrawWidget(Slate,FVector2D(260,140));
	Renderer->DrawWidget(Target,Slate,FVector2D(260,140),0.f);
	auto* Badge = State(*Rack,0,TEXT("StateBounds"));
	const auto& BG = Badge->GetCachedGeometry(); const auto& CG = Card->GetCachedGeometry();
	const FVector2D Local = CG.AbsoluteToLocal(BG.GetAbsolutePosition());
	TestTrue(TEXT("State remains inset within portrait lower margin, clear of identity and rarity"),
		Local.X > 0 && Local.Y > CG.GetLocalSize().Y*.5f
		&& Local.X + BG.GetLocalSize().X < FMCodexHandMicroDiagnostics::PortraitWidth
		&& Local.Y + BG.GetLocalSize().Y < CG.GetLocalSize().Y);
	TestTrue(TEXT("Card retains canonical dimensions and art route"),Card->IsCanonicalCardFamily()
		&& Card->GetConfiguredDimensions().Equals(FVector2D(FMCodexHandMicroDiagnostics::CardWidth,FMCodexHandMicroDiagnostics::CardHeight)));
	const auto Before = Card->GetPresentation(); const auto* Portrait = Card->GetResolvedHandMicroPortraitTexture();
	Card->TakeWidget()->OnMouseEnter(CG,FPointerEvent());
	TestTrue(TEXT("Existing hover receives the pointer"),Card->IsHovered());
	TestTrue(TEXT("Hover retains state, rarity, identity and portrait"),Badge->GetVisibility() == ESlateVisibility::HitTestInvisible
		&& Card->GetPresentation().RarityLabel == Before.RarityLabel && Card->GetRenderedAssignedNumber().ToString() == Before.AssignedPlayerNumber
		&& Card->GetResolvedHandMicroPortraitTexture() == Portrait);
	Card->TakeWidget()->OnMouseLeave(FPointerEvent());
	TestFalse(TEXT("Original hover exit still works"),Card->IsHovered());
	BeginCleanup(Renderer);
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
class FStartCardStatePIE final : public IAutomationLatentCommand
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


class FCardStatePIE final : public IAutomationLatentCommand
{
public:
	explicit FCardStatePIE(FAutomationTestBase* T) : Test(T) {}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds()-Started > 80) { Test->AddError(TEXT("Card-state PIE timed out")); return true; }
		if (!GEditor || !GEditor->PlayWorld || FPlatformTime::Seconds()-Changed < .8) return false;
		auto* C = Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController());
		auto* S = C ? C->GetPlayerMatchScreen() : nullptr; if (!S) return false;
		if (Step == 0) { S->RequestStartNewMatch(); Advance(); return false; }
		if (Step == 1)
		{
			if (!Override(*C,EFMCodexLocalDevRollTarget::FullD12,9)) return true;
			S->RequestRollTacticalPoints(); Advance(); return false;
		}
		if (S->IsInlineFormulaRevealInputBlocked()) return false;
		if (Step == 2)
		{
			if (!Test->TestTrue(TEXT("Natural D12 reaches set-piece type"),C->GetInteractionView().InteractionCategory == EFMCodexLocalMatchInteractionCategory::RollSetPieceType)) return true;
			if (!Override(*C,EFMCodexLocalDevRollTarget::SetPieceType,1)) return true;
			S->RequestContinueResolution(); Advance(); return false;
		}
		if (Step == 3)
		{
			if (!Test->TestTrue(TEXT("Natural Corner reaches candidate draft"),C->GetInteractionView().InteractionCategory == EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker)) return true;
			const auto Legal = C->GetInteractionView().LegalSetPieceCardIds;
			if (!Test->TestTrue(TEXT("Three legal nominees available"),Legal.Num() >= 3)) return true;
			for (int32 I = 0; I < 3; ++I)
			{
				if (!Click(*S,Legal[I])) return true;
				Nominees.Add(Legal[I]);
			}
			Advance(); return false;
		}
		auto* Rack = FindRack(*S,Nominees[0]);
		if (!Rack) { Test->AddError(TEXT("No acting rack")); return true; }
		if (Step == 4)
		{
			Test->TestTrue(TEXT("Original handlers preserve draft click order without submission"),C->GetInteractionView().DraftCornerNomineeCardIds == Nominees
				&& C->GetInteractionView().InteractionCategory == EFMCodexLocalMatchInteractionCategory::DraftCornerAttacker);
			for (int32 I=0; I<3; ++I)
			{
				const auto* Cell = Rack->GetPresentation().Cells.FindByPredicate([&](const auto& X){return X.Card.CardId == Nominees[I];});
				if (!Cell) {Test->AddError(TEXT("Missing nominated cell"));return true;}
				Test->TestEqual(TEXT("Real rack shows projected candidate order"),
					CastChecked<UTextBlock>(FMCodexCardStateTests::State(*Rack,Cell->StableIndex,TEXT("Label")))->GetText().ToString(),FString::Printf(TEXT("#%d"),I+1));
			}
			// Capture context before any temporary framing, preserving the game's DPI.
			CaptureContext(TEXT("Card_State_Context.png"));
			Capture(Rack->GetWidgetFromName(TEXT("PersistentCardRackFrame")),TEXT("Corner_Order_Combined.png"));
			// Secondary static fixture: same real card facts, only the draft label mode
			// differs. This is not another gameplay path or a claimed taker state.
			auto P = Rack->GetPresentation();
			P.Cells.RemoveAll([&](const auto& X){return X.Card.CardId != Nominees[0];});
			P.ColumnCount = 1; P.Cells[0].SetPieceSelectionOrder = 0;
			auto* Fixture = NewObject<UFMCodexCardRackWidget>(); Fixture->TakeWidget(); Fixture->RefreshFromPresentation(P);
			auto* Card = Fixture->GetRenderedCardWidgets()[0].Get();
			Capture(Card->GetParent(),TEXT("Card_Selected_Normal.png"));
			Card->TakeWidget()->OnMouseEnter(Card->GetCachedGeometry(),FPointerEvent());
			Test->TestTrue(TEXT("Static selected fixture exercises native hover"),Card->IsHovered());
			Capture(Card->GetParent(),TEXT("Card_Selected_Hover.png"));
			Card->TakeWidget()->OnMouseLeave(FPointerEvent());
			if (!Click(*S,Nominees[1])) return true;
			Advance();return false;
		}
		Test->TestTrue(TEXT("Real deselection retains remaining click order"),C->GetInteractionView().DraftCornerNomineeCardIds == TArray<FName>({Nominees[0],Nominees[2]}));
		const auto* Cell = Rack->GetPresentation().Cells.FindByPredicate([&](const auto& X){return X.Card.CardId == Nominees[2];});
		Test->TestTrue(TEXT("Real rack renumbers remaining nominee"),Cell && Cell->SetPieceSelectionOrder == 2);
		return true;
	}
private:
	UFMCodexCardRackWidget* FindRack(UFMCodexLocalMatchScreenWidget& S,FName Id)
	{
		for (auto* Rack : {S.GetLocalRackWidget(),S.GetOpponentRackWidget()})
			if (Rack && Rack->GetPresentation().Cells.ContainsByPredicate([Id](const auto& X){return X.Card.CardId == Id;})) return Rack;
		return nullptr;
	}
	bool Click(UFMCodexLocalMatchScreenWidget& S,FName Id)
	{
		auto* Rack=FindRack(S,Id); if (!Rack) return Test->TestTrue(TEXT("Legal hand card present"),false);
		for (const auto& Card : Rack->GetRenderedCardWidgets())
			if (Card->GetPresentation().CardId == Id) return Test->TestTrue(TEXT("Original rack selection handler succeeds"),Card->RequestOnPitchSelection());
		return Test->TestTrue(TEXT("Legal interactive hand card present"),false);
	}
	void Advance() { ++Step; Changed=FPlatformTime::Seconds(); }
	bool Override(AFMCodexLocalMatchPlayerController& C,EFMCodexLocalDevRollTarget Target,int32 Value)
	{
		FFMCodexLocalDevRollOverrideRequest R; R.Target=Target;R.Value=Value;
		return Test->TestTrue(TEXT("Existing DEV provider accepts roll override"),C.SetLocalDevRollOverride(R).bSuccess);
	}
	void Save(const TArray<FColor>& Pixels,const FIntVector& Size,const TCHAR* File)
	{
		const FString Dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("Stage8_7D"));
		IFileManager::Get().MakeDirectory(*Dir,true);
		TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);
		Test->TestTrue(TEXT("PIE evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Dir/File)));
		Test->AddInfo(FString::Printf(TEXT("CARD_STATE_CAPTURE %s %dx%d game=%.3f"),File,Size.X,Size.Y,GEditor->PlayWorld->GetTimeSeconds()));
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
		const FVector2D NativeSize(FMath::CeilToInt(Slate->GetDesiredSize().X),FMath::CeilToInt(Slate->GetDesiredSize().Y));
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
	double Started=FPlatformTime::Seconds(),Changed=0;
	int32 Step=0;
	TArray<FName> Nominees;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexCardStatePIETest,
	"FMCodex.PIE.CardState.CornerDraft",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFMCodexCardStatePIETest::RunTest(const FString&)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartCardStatePIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FCardStatePIE(this)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
