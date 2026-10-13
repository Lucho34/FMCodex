#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "Components/TextBlock.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "TextureCompiler.h"
#include "Slate/WidgetRenderer.h"
#include "Input/Events.h"
#include "ImageUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexHandTacticalMatchStateTest,
	"FMCodex.LocalPlay.UI.HandMicro.TacticalMatchStateGeometryAndReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexHandTacticalMatchStateTest::RunTest(const FString&)
{
	const auto Demo = FFMCodexLocalMatchDemoConfigurationFactory::Create();
	FMatchPlayAuthoritativeSession Session;
	if (!TestTrue(TEXT("Canonical TP 4 fixture starts"),
		Session.InitializeMatch(Demo.OpeningInput).OpeningResult.bSuccess
		&& Session.BeginOrdinaryAttack(4).BeginResult.bSuccess)) return false;
	// DEV presentation fixture: inspect Arsenal's actual TP 4 projection.
	auto State = Session.GetStateSnapshot();
	State.RuntimeState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
	const auto View = FFMCodexLocalMatchInteractionViewBuilder::Build(State, Demo.SkillRuleSet);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Widget world"), World)) return false;
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	auto* Card = CreateWidget<UFMCodexPlayerCardWidget>(World);
	if (!Card)
	{
		GEngine->DestroyWorldContext(World); World->DestroyWorld(false); return false;
	}
	auto Slate = Card->TakeWidget();
	auto* Renderer = new FWidgetRenderer(false);
	auto* Target = Renderer->DrawWidget(Slate, FVector2D(220, 68));
	auto Node = [&](const TCHAR* Name) { return Card->GetWidgetFromName(Name); };
	auto Rect = [](const UWidget* Widget)
	{
		const auto& G = Widget->GetCachedGeometry();
		const auto P = G.GetAbsolutePosition(), S = G.GetAbsoluteSize();
		return FSlateRect(P.X, P.Y, P.X + S.X, P.Y + S.Y);
	};
	auto Contains = [](const FSlateRect& Outer, const FSlateRect& Inner)
	{
		return Inner.Left >= Outer.Left - .1f && Inner.Top >= Outer.Top - .1f
			&& Inner.Right <= Outer.Right + .1f && Inner.Bottom <= Outer.Bottom + .1f;
	};
	auto Draw = [&]() { Renderer->DrawWidget(Target, Slate, FVector2D(220, 68), 0.f); };
	TSet<FString> Roles;
	auto CheckHand = [&](const FFMCodexUMGCardViewModel& Model)
	{
		Card->RefreshFromPresentation(Model, EFMCodexPlayerCardPresentationMode::HandMicro);
		Draw(); Draw();
		const FString Context = Model.CardId.ToString() + TEXT(" ");
		const auto* Area = CastChecked<UTextBlock>(Node(TEXT("HandMicroMatchingSkills")));
		const auto* First = Node(TEXT("PitchMiniTacticalMatchPipTop"));
		const auto* Second = Node(TEXT("PitchMiniTacticalMatchPipBottom"));
		const int32 Count = Model.HandMicroTacticalMatchCount;
		TestTrue(Context + TEXT("resolved 0/1/2 state, including rebind clearing"),
			(Area->GetVisibility() == ESlateVisibility::HitTestInvisible) == (Count > 0)
			&& (First->GetVisibility() == ESlateVisibility::HitTestInvisible) == (Count > 0)
			&& (Second->GetVisibility() == ESlateVisibility::HitTestInvisible) == (Count == 2));
		TArray<FString> Names;
		for (const auto& Skill : Model.HandMicroVisibleTacticalSkills)
			Names.Add(FFMCodexPlayerUIPresentationText::Skill(Skill.CanonicalLabel).ToString());
		TestEqual(Context + TEXT("complete ordered Chinese labels from projection"), Area->GetText().ToString(), FString::Join(Names, TEXT("·")));
		TestNull(TEXT("Superseded capsule removed"), Node(TEXT("HandMicroTacticalMatchBackplate")));
		TestEqual(Context + TEXT("card footprint retained"), Card->GetConfiguredDimensions(), FVector2D(220, 68));
		const auto* Role = CastChecked<UTextBlock>(Node(TEXT("HandMicroPosition")));
		const auto* Name = CastChecked<UTextBlock>(Node(TEXT("HandMicroPlayerName")));
		Roles.Add(Role->GetText().ToString());
		TestTrue(Context + TEXT("complete position label fits"),
			Role->GetDesiredSize().X > 0 && Role->GetDesiredSize().X <= Role->GetCachedGeometry().GetLocalSize().X + .1f);
		TestTrue(Context + TEXT("name and position have separate rows"), Rect(Name).Bottom <= Rect(Role).Top + .1f);
		if (Count > 0)
		{
			const auto A = Rect(Area), R = Rect(Role), N = Rect(Node(TEXT("AssignedNumberBounds")));
			const float NumberInkLeft = N.Right - Node(TEXT("AssignedPlayerNumber"))->GetDesiredSize().X;
			TestTrue(Context + TEXT("label stays inside identity"), Contains(Rect(Node(TEXT("HandMicroIdentityBounds"))), A));
			TestTrue(Context + FString::Printf(TEXT("label clears position: %.1f >= %.1f + 2"), A.Left, R.Right), A.Left >= R.Right + 1.9f);
			TestTrue(Context + FString::Printf(TEXT("label clears number: %.1f <= %.1f - 2"), A.Right, NumberInkLeft), A.Right <= NumberInkLeft - 1.9f);
			TestTrue(Context + TEXT("label clears name row"), Rect(Name).Bottom < A.Top);
			TestTrue(Context + TEXT("full text fits without truncation or tiny type"),
				Area->GetDesiredSize().X <= A.Right - A.Left + .1f && Area->GetFont().Size >= 10);
			TestTrue(Context + TEXT("original dots retain portrait-side geometry"),
				Rect(First).Left < Rect(Node(TEXT("HandMicroIdentityBounds"))).Left
				&& FMath::IsNearlyEqual(Rect(First).Right - Rect(First).Left, 4.f));
			Card->SetHandSkillRevealProgress(.25f); Draw();
			TestTrue(Context + TEXT("animation cannot move geometry"), Rect(Area) == A);
			Card->SetHandSkillRevealProgress(1.f);
		}
	};
	for (const auto* Roster : { &View.PlayerACardRoster, &View.PlayerBCardRoster })
		for (const auto& Source : *Roster) CheckHand(FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Source));
	for (const TCHAR* Role : { TEXT("GK"), TEXT("D"), TEXT("M"), TEXT("A"), TEXT("M/D"), TEXT("A/M") })
		TestTrue(FString(TEXT("Production role covered: ")) + Role, Roles.Contains(Role));
	const TCHAR* Keys[] = { TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.MikelMerino"), TEXT("Prototype.Arsenal.DavidRaya") };
	const int32 Counts[] = { 2, 1, 0, 0 };
	const TCHAR* ExpectedLabels[] = { TEXT("传中·内切"), TEXT("远射"), TEXT(""), TEXT("") };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Keys); ++Index)
	{
		const auto* Source = View.PlayerACardRoster.FindByPredicate(
			[&](const auto& C) { return C.CardId == FName(Keys[Index]); });
		if (!TestNotNull(TEXT("Representative production identity exists"), Source)) continue;
		const auto Model = FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*Source);
		TestEqual(FString(Keys[Index]) + TEXT(" actual TP 4 count"), Model.HandMicroTacticalMatchCount, Counts[Index]);
		CheckHand(Model);
		TestEqual(TEXT("Verified production TP4 names"), CastChecked<UTextBlock>(Node(TEXT("HandMicroMatchingSkills")))->GetText().ToString(), FString(ExpectedLabels[Index]));
		const auto Before = Rect(Node(TEXT("HandMicroPosition")));
		Slate->OnMouseEnter(Card->GetCachedGeometry(), FPointerEvent()); Draw();
		TestTrue(TEXT("Hover retains position lane and resolved state"), Card->IsHovered()
			&& Rect(Node(TEXT("HandMicroPosition"))) == Before
			&& Card->GetPresentation().HandMicroTacticalMatchCount == Counts[Index]);
		Slate->OnMouseLeave(FPointerEvent());
		for (const auto Mode : { EFMCodexPlayerCardPresentationMode::PitchMini, EFMCodexPlayerCardPresentationMode::InteractionChoice })
		{
			Card->RefreshFromPresentation(Model, Mode);
			TestTrue(TEXT("Hand state is cleared outside Hand even with a matching source"),
				Node(TEXT("HandMicroMatchingSkills"))->GetVisibility() == ESlateVisibility::Collapsed);
		}
		CheckHand(Model);
	}
	// One production-rack render for engineering style review; not USER PIE evidence.
	auto* Rack = CreateWidget<UFMCodexCardRackWidget>(World);
	Rack->RefreshFromPresentation(FFMCodexLocalMatchUMGPresentationBuilder::Build(
		View, {}, TEXT(""), EInitialTurnOrderPlayer::PlayerA).LocalRack);
	auto RackSlate = Rack->TakeWidget();
	const auto RackModel = Rack->GetPresentation();
	auto LabelFor = [&](FName Id)
	{
		for (const auto& HandCard : Rack->GetRenderedCardWidgets())
			if (HandCard->GetPresentation().CardId == Id)
			{
				HandCard->TakeWidget();
				return CastChecked<UTextBlock>(HandCard->GetWidgetFromName(TEXT("HandMicroMatchingSkills")));
			}
		return static_cast<UTextBlock*>(nullptr);
	};
	const FName Saka(Keys[0]);
	Rack->RefreshFromPresentation(RackModel, 1, 4, false);
	Rack->AdvanceHandSkillRevealForTesting(3.f);
	TestEqual(TEXT("Visible TP window keeps Skill text fully transparent"), LabelFor(Saka)->GetRenderOpacity(), 0.f);
	Rack->RefreshFromPresentation(RackModel, 1, 4, false);
	TestEqual(TEXT("Rebind while window is open retains fully transparent pending reveal"), LabelFor(Saka)->GetRenderOpacity(), 0.f);
	for (const auto& HandCard : Rack->GetRenderedCardWidgets())
		if (HandCard->GetPresentation().CardId == Saka)
		{
			const auto HandSlate = HandCard->TakeWidget();
			HandSlate->OnMouseEnter(HandCard->GetCachedGeometry(), FPointerEvent());
			TestEqual(TEXT("Hover cannot expose pending Skill text"), LabelFor(Saka)->GetRenderOpacity(), 0.f);
			HandSlate->OnMouseLeave(FPointerEvent());
		}
	Rack->RefreshFromPresentation(RackModel, 1, 4, true);
	TestEqual(TEXT("Dismissal starts from invisible, not a pre-revealed baseline"), LabelFor(Saka)->GetRenderOpacity(), 0.f);
	TestTrue(TEXT("New TP starts a restrained fade"), LabelFor(Saka)->GetRenderOpacity() < 1.f);
	Rack->AdvanceHandSkillRevealForTesting(.25f);
	TestEqual(TEXT("First unobscured frame discards the interval preceding dismissal"), LabelFor(Saka)->GetRenderOpacity(), 0.f);
	Rack->AdvanceHandSkillRevealForTesting(.08f);
	const float MidOpacity = LabelFor(Saka)->GetRenderOpacity();
	TestTrue(TEXT("Actual elapsed time advances the fade"), MidOpacity > 0.f && MidOpacity < 1.f);
	Rack->RefreshFromPresentation(RackModel, 1, 4);
	TestEqual(TEXT("Rack reconstruction retains in-flight reveal progress"), LabelFor(Saka)->GetRenderOpacity(), MidOpacity);
	Rack->AdvanceHandSkillRevealForTesting(.46f);
	const FLinearColor Mint = FLinearColor::FromSRGBColor(FColor(0x8F, 0xE6, 0xC2));
	TestFalse(TEXT("Brightness emphasis still runs at 0.54 seconds"), LabelFor(Saka)->GetColorAndOpacity().GetSpecifiedColor().Equals(Mint, .0001f));
	Rack->AdvanceHandSkillRevealForTesting(.011f);
	TestTrue(TEXT("Entire reveal settles at 0.55 seconds"), LabelFor(Saka)->GetColorAndOpacity().GetSpecifiedColor().Equals(Mint, .0001f));
	Rack->RefreshFromPresentation(RackModel, 1, 4);
	TestEqual(TEXT("Completed reveal does not replay on passive rebind"), LabelFor(Saka)->GetRenderOpacity(), 1.f);
	Rack->RefreshFromPresentation(RackModel, 2, 4, false);
	TestTrue(TEXT("New attack with the same TP gets its own reveal"), LabelFor(Saka)->GetRenderOpacity() < 1.f);
	auto Hidden = RackModel;
	for (auto& Cell : Hidden.Cells)
	{
		Cell.Card.HandMicroVisibleTacticalSkills.Reset();
		Cell.Card.HandMicroTacticalMatchCount = 0;
		Cell.Card.bHasHandMicroTacticalMatch = false;
	}
	Rack->RefreshFromPresentation(Hidden, 2, 4);
	Rack->AdvanceHandSkillRevealForTesting(1.f);
	TestTrue(TEXT("Phase cancellation cannot leave obsolete labels"), LabelFor(Saka)->GetText().IsEmpty()
		&& LabelFor(Saka)->GetVisibility() == ESlateVisibility::Collapsed);
	Rack->RefreshFromPresentation(RackModel, 2, 4);
	TestEqual(TEXT("Returning to an already revealed same-TP label stays static"), LabelFor(Saka)->GetRenderOpacity(), 1.f);
	Rack->RefreshFromPresentation(RackModel, 3, 4, false);
	Rack->RefreshFromPresentation(Hidden, 4, 0, true);
	Rack->AdvanceHandSkillRevealForTesting(1.f);
	TestTrue(TEXT("New attack cancels old pending reveal"), LabelFor(Saka)->GetText().IsEmpty());
	Rack->RefreshFromPresentation(RackModel, 5, 4); // Valid restored TP, no result window.
	Rack->AdvanceHandSkillRevealForTesting(0.f);
	Rack->AdvanceHandSkillRevealForTesting(.6f);
	TestEqual(TEXT("Already-dismissed entry cannot strand labels"), LabelFor(Saka)->GetRenderOpacity(), 1.f);
	Rack->RefreshFromPresentation(RackModel, 6, 4, false, TEXT("PhaseA"));
	Rack->RefreshFromPresentation(RackModel, 6, 4, true, TEXT("PhaseB"));
	TestEqual(TEXT("Same-TP phase change cancels pending emphasis without hiding labels"), LabelFor(Saka)->GetRenderOpacity(), 1.f);
	Rack->RefreshFromPresentation(RackModel, 7, 4, true, TEXT("PhaseA"));
	Rack->AdvanceHandSkillRevealForTesting(0.f);
	Rack->AdvanceHandSkillRevealForTesting(.08f);
	Rack->RefreshFromPresentation(RackModel, 7, 4, true, TEXT("PhaseB"));
	TestEqual(TEXT("Same-TP phase change also cancels in-flight emphasis"), LabelFor(Saka)->GetRenderOpacity(), 1.f);
	const int32 ChangingTPs[] = { 2, 3, 5, 7 };
	const int32 ExpectedCounts[] = { 0, 1, 2, 0 };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(ChangingTPs); ++Index)
	{
		FMatchPlayAuthoritativeSession NextSession;
		NextSession.InitializeMatch(Demo.OpeningInput);
		NextSession.BeginOrdinaryAttack(ChangingTPs[Index]);
		auto NextState = NextSession.GetStateSnapshot();
		NextState.RuntimeState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		const auto NextView = FFMCodexLocalMatchInteractionViewBuilder::Build(NextState, Demo.SkillRuleSet);
		const auto NextModel = FFMCodexLocalMatchUMGPresentationBuilder::Build(NextView, {}, TEXT(""), EInitialTurnOrderPlayer::PlayerA).LocalRack;
		Rack->RefreshFromPresentation(NextModel, 3, ChangingTPs[Index]);
		for (const auto& Cell : NextModel.Cells) CheckHand(Cell.Card);
		const auto* Ode = NextModel.Cells.FindByPredicate([&](const auto& C) { return C.Card.CardId == FName(Keys[1]); });
		TestEqual(TEXT("Production transition covers 0 to 1 to 2 to 0"), Ode->Card.HandMicroTacticalMatchCount, ExpectedCounts[Index]);
		if (ExpectedCounts[Index] > 0) TestTrue(TEXT("Changed TP re-reveals current names once"), LabelFor(FName(Keys[1]))->GetRenderOpacity() < 1.f);
	}
	Rack->RefreshFromPresentation(RackModel); // Static engineering preview.
#if WITH_EDITOR
	// A standalone run must not depend on earlier tests warming portrait resources.
	TArray<UTexture*> Portraits;
	for (const auto& HandCard : Rack->GetRenderedCardWidgets())
		if (auto* Portrait = HandCard->GetResolvedHandMicroPortraitTexture()) Portraits.Add(Portrait);
	FTextureCompilingManager::Get().FinishCompilation(Portraits);
#endif
	const FVector2D RackSize(476, 800);
	// Render linear Slate colors at float precision, then encode to sRGB once.
	auto* RackTarget = NewObject<UTextureRenderTarget2D>();
	RackTarget->ClearColor = FLinearColor::Transparent;
	RackTarget->InitCustomFormat(476, 800, PF_FloatRGBA, true);
	RackTarget->UpdateResourceImmediate(true);
	Renderer->DrawWidget(RackTarget, RackSlate, RackSize, 0.f);
	Renderer->DrawWidget(RackTarget, RackSlate, RackSize, 0.f);
	TArray<FColor> Pixels;
	FReadSurfaceDataFlags ReadFlags(RCM_UNorm);
	ReadFlags.SetLinearToGamma(true);
	if (TestTrue(TEXT("Production rack pixels readable"), RackTarget->GameThread_GetRenderTargetResource()->ReadPixels(Pixels, ReadFlags)))
	{
		const FString Dir = FPaths::ProjectSavedDir() / TEXT("Stage8_29A");
		IFileManager::Get().MakeDirectory(*Dir, true);
		TArray64<uint8> PNG;
		FImageUtils::PNGCompressImageArray(RackTarget->SizeX, RackTarget->SizeY, Pixels, PNG);
		TestTrue(TEXT("Engineering preview saved"), FFileHelper::SaveArrayToFile(PNG, *(Dir / TEXT("HandTP4.png"))));
	}
	BeginCleanup(Renderer);
	GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
	return true;
}

#endif
