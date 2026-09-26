#include "FMCodexRollReelWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexRollPresentationSurface.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// Focused access to cosmetic projection only; real PIE below uses typed commands.
struct FFMCodexRollCosmeticTestAccess
{
	static void Begin(UFMCodexLocalMatchScreenWidget& Screen, int32 Maximum, int32 Final, int64 Event = 17)
	{
		Screen.CancelInlineFormulaReveal();
		FFMCodexCrossRollRevealIdentity Identity;
		Identity.Kind = Maximum == 12 ? EFMCodexUMGCrossRollRevealKind::TacticalPoint
			: EFMCodexUMGCrossRollRevealKind::InitialRoute;
		Identity.AttackSequence = Event;
		Identity.OwnerSide = EInitialTurnOrderPlayer::PlayerA;
		Identity.ContestId = TEXT("RollCosmetic.Focus"); Identity.RollSequenceIndex = 0;
		Screen.BeginInlineFormulaReveal(Identity, false);
		Screen.PauseInlineFormulaRevealTimerForTesting();
		Screen.RollRevealDomainMinimum = 1; Screen.RollRevealDomainMaximum = Maximum;
		Screen.RollRevealAuthoritativeRawValue = Final;
		Screen.bInlineFormulaAuthorityResultAvailable = true;
	}
	static void BeginTheater(UFMCodexLocalMatchScreenWidget& Screen,int32 Final,int64 Event,bool bDefense=false,bool bRoute=false)
	{
		Begin(Screen,6,Final,Event);
		Screen.ActiveCrossRollReveal.ContestId=bRoute ? TEXT("Cross.Route") : TEXT("Cross.High");
		Screen.ActiveCrossRollReveal.Kind=bRoute ? EFMCodexUMGCrossRollRevealKind::InitialRoute : bDefense ? EFMCodexUMGCrossRollRevealKind::Defense : EFMCodexUMGCrossRollRevealKind::Attack;
		Screen.ActiveCrossRollReveal.RollSequenceIndex=bDefense ? 1 : 0;
		Screen.RollRevealCosmeticSeed=GetTypeHash(Screen.ActiveCrossRollReveal.StableKey());
		Screen.TheaterMotion.bActive=true;
	}
	static FFMCodexUMGRollReelViewModel At(UFMCodexLocalMatchScreenWidget& Screen, float Time)
	{
		Screen.InlineFormulaRevealPhaseElapsed = Time;
		return Screen.BuildActiveRollReelPresentation();
	}
	static void Hero(UFMCodexLocalMatchScreenWidget& Screen)
	{ Screen.ActiveCrossRollReveal.ContestId=TEXT("Match.TacticalPoint"); Screen.TheaterMotion.bActive=false; }
	static void Capture(UFMCodexLocalMatchScreenWidget& Screen) { Screen.BeginInlineFormulaFinalCapture(); }
	static FFMCodexUMGRollReelViewModel Current(UFMCodexLocalMatchScreenWidget& Screen)
	{ return Screen.BuildActiveRollReelPresentation(); }
	static void TickMotion(UFMCodexLocalMatchScreenWidget& Screen,float Delta)
	{ Screen.AdvanceInlineFormulaReveal(Delta,false); }
	static void SetAvailable(UFMCodexLocalMatchScreenWidget& Screen, bool bAvailable)
	{ Screen.bInlineFormulaAuthorityResultAvailable = bAvailable; }
	static void Refresh(UFMCodexLocalMatchScreenWidget& Screen, float Time)
	{ Screen.InlineFormulaRevealPhaseElapsed = Time; Screen.RefreshActiveRollReelVisuals(); }
	static void Hide(UFMCodexLocalMatchScreenWidget& Screen)
	{ Screen.CancelInlineFormulaReveal(); Screen.RefreshVisuals(); }
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollActivationContinuityTest,
	"FMCodex.LocalPlay.RollPresentation.ActivationContinuity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollActivationContinuityTest::RunTest(const FString&)
{
	using Access = FFMCodexRollCosmeticTestAccess;
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	Access::Begin(*Screen, 12, 9);
	auto* Frame = CastChecked<UFMCodexRollPresentationSurface>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface")));
	auto* Title = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealTitle"));
	auto* State = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealState"));
	auto* Detail = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult"));
	auto* Reel = Screen->GetTacticalPointRollReel();
	Access::Refresh(*Screen, 0);
	TestTrue(TEXT("Command feedback begins with the shell, before chamber and footer content"),
		Frame->GetRenderOpacity() > 0 && Frame->GetRenderOpacity() < 1
		&& Title->GetRenderOpacity() == 0 && Reel->GetRenderOpacity() == 0 && State->GetRenderOpacity() == 0);
	Access::Refresh(*Screen, .06f);
	TestTrue(TEXT("Title, chamber and status assemble in order without a second clock"),
		Title->GetRenderOpacity() > Reel->GetRenderOpacity() && Reel->GetRenderOpacity() > State->GetRenderOpacity()
		&& Reel->GetRenderOpacity() > 0 && State->GetRenderTransform().Translation.Y > 0);
	const float BeforeRefresh = Reel->GetRenderOpacity();
	Access::Refresh(*Screen, .06f);
	TestEqual(TEXT("Duplicate refresh cannot restart activation"), Reel->GetRenderOpacity(), BeforeRefresh);
	TestEqual(TEXT("Activation never changes the reveal phase"), Screen->GetInlineFormulaRevealPhase(), EFMCodexUMGInlineFormulaRevealPhase::Cycling);
	for (float Time : {.18f, .80f, 4.83f})
	{
		Access::Refresh(*Screen, Time);
		TestTrue(TEXT("Activation ends inside the existing fast segment and cannot replay during a late result"),
			Frame->GetActivationProgress() == 1 && Frame->GetRenderOpacity() == 1
			&& Reel->GetRenderOpacity() == 1 && State->GetRenderOpacity() == 1 && Detail->GetRenderOpacity() == 1
			&& Reel->GetRenderTransform().Scale.Equals(FVector2D(1))
			&& Reel->GetRenderTransform().Translation.IsNearlyZero() && State->GetRenderTransform().Translation.IsNearlyZero());
	}
	Access::Refresh(*Screen, .06f); Access::Hide(*Screen);
	TestTrue(TEXT("Interrupted entrance clears all transforms and activation before reuse"),
		Frame->GetVisibility() == ESlateVisibility::Collapsed && Frame->GetActivationProgress() == 1
		&& Frame->GetRenderTransform().Scale.Equals(FVector2D(1))
		&& Reel->GetRenderOpacity() == 1 && Reel->GetRenderTransform().Scale.Equals(FVector2D(1))
		&& Title->GetRenderTransform().Translation.IsNearlyZero() && Detail->GetRenderTransform().Translation.IsNearlyZero());
	Access::Begin(*Screen, 12, 6, 18); Access::Refresh(*Screen, 0);
	TestTrue(TEXT("A legitimate new event can assemble again without a prior number"),
		Frame->GetActivationProgress() == 0 && Reel->GetRenderOpacity() == 0 && !Reel->GetPresentation().bAuthoritativeValue);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollResultIndependentCyclingTest,
	"FMCodex.LocalPlay.RollPresentation.ResultIndependentCycling",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollResultIndependentCyclingTest::RunTest(const FString&)
{
	using Access = FFMCodexRollCosmeticTestAccess;
	auto* Screen = NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	auto SameStrip = [](const FFMCodexUMGRollReelViewModel& A, const FFMCodexUMGRollReelViewModel& B)
	{
		return A.PreviousValue == B.PreviousValue && A.CenterValue == B.CenterValue && A.NextValue == B.NextValue
			&& FMath::IsNearlyEqual(A.ContinuousPositionCells, B.ContinuousPositionCells)
			&& FMath::IsNearlyEqual(A.ScrollAlpha, B.ScrollAlpha);
	};
	for (int32 Maximum : {6, 12})
	{
		TArray<FFMCodexUMGRollReelViewModel> Baseline;
		for (int32 Final = 1; Final <= Maximum; ++Final)
		{
			Access::Begin(*Screen, Maximum, Final);
			int32 ConsecutiveSteps = 0, OrderedSteps = 0;
			FFMCodexUMGRollReelViewModel Previous;
			for (int32 Frame = 0; Frame < 260; ++Frame)
			{
				const auto Reel = Access::At(*Screen, Frame*.005f);
				if (Final == 1) Baseline.Add(Reel);
				else TestTrue(TEXT("Every pre-settling label/position is independent of the final result"), SameStrip(Baseline[Frame], Reel));
				TestTrue(TEXT("Shuffled labels stay within the consumer domain"),
					Reel.PreviousValue >= 1 && Reel.PreviousValue <= Maximum
					&& Reel.CenterValue >= 1 && Reel.CenterValue <= Maximum
					&& Reel.NextValue >= 1 && Reel.NextValue <= Maximum && !Reel.bAuthoritativeValue);
				if (Frame > 0 && FMath::FloorToInt(Reel.ContinuousPositionCells) != FMath::FloorToInt(Previous.ContinuousPositionCells))
				{
					TestEqual(TEXT("Cell crossings preserve the incoming digit instead of flashing unrelated labels"), Reel.CenterValue, Previous.NextValue);
					++ConsecutiveSteps;
					OrderedSteps += Reel.CenterValue == Previous.CenterValue % Maximum + 1;
				}
				Previous = Reel;
			}
			TestTrue(TEXT("Visible progression is not the old ordered count-up"), ConsecutiveSteps >= 4 && OrderedSteps < ConsecutiveSteps);
			const auto BeforeCapture = Access::At(*Screen, 1.30f);
			Access::Capture(*Screen);
			TestTrue(TEXT("Capture does not relabel any currently visible slot"), SameStrip(BeforeCapture, Access::At(*Screen, 0)));
			const auto Captured = Access::At(*Screen, .12f);
			TestTrue(TEXT("Every D6/D12 result lands exactly in the unchanged capture window"),
				Captured.CenterValue == Final && FMath::IsNearlyZero(Captured.ScrollAlpha));
			Screen->AdvanceInlineFormulaRevealForTesting(.04f);
			const auto Held = Access::At(*Screen, 0);
			TestTrue(TEXT("ResultHold exposes only the accepted final value"), Held.bStaticResult && Held.bAuthoritativeValue && Held.CenterValue == Final);
		}
		Access::Begin(*Screen, Maximum, 1, 18);
		bool bDifferentEvent = false;
		for (int32 Frame = 0; Frame < Baseline.Num(); ++Frame)
			bDifferentEvent |= !SameStrip(Baseline[Frame], Access::At(*Screen, Frame*.005f));
		TestTrue(TEXT("A legitimate new event receives a different deterministic cosmetic order"), bDifferentEvent);
		Access::Begin(*Screen, Maximum, Maximum);
		Access::SetAvailable(*Screen, false);
		const auto Waiting = Access::At(*Screen, 4.83f);
		Access::Capture(*Screen);
		TestEqual(TEXT("Missing authority never starts capture"), Screen->GetInlineFormulaRevealPhase(), EFMCodexUMGInlineFormulaRevealPhase::Cycling);
		Access::SetAvailable(*Screen, true); Access::Capture(*Screen);
		TestTrue(TEXT("Late authority keeps the current moving strip at capture entry"), SameStrip(Waiting, Access::At(*Screen, 0)));
		TestEqual(TEXT("Late capture lands on authority without a whole-domain chase"), Access::At(*Screen, .12f).CenterValue, Maximum);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTheaterContinuousLandingTest,
	"FMCodex.LocalPlay.RollPresentation.TheaterContinuousLanding",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTheaterContinuousLandingTest::RunTest(const FString&)
{
	using Access=FFMCodexRollCosmeticTestAccess;
	auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	for (const auto Variant : {EFMCodexRollVisualVariant::TheaterInline, EFMCodexRollVisualVariant::CompactBox, EFMCodexRollVisualVariant::HeroRoll})
	{
	const bool bHero=Variant==EFMCodexRollVisualVariant::HeroRoll;
	const bool bRoute=Variant==EFMCodexRollVisualVariant::CompactBox;
	const int32 Maximum=bHero ? 12 : 6;
	auto BeginTheater=[&](int32 Final,int64 Event,bool bDefense=false)
	{
		if (bHero) { Access::Begin(*Screen,12,Final,Event); Access::Hero(*Screen); }
		else Access::BeginTheater(*Screen,Final,Event,bDefense,bRoute);
	};
	auto* Widget=NewObject<UFMCodexRollReelWidget>(); Widget->TakeWidget(); Widget->SetVisualVariant(Variant);
	TSet<FString> EventPrefixes;
	for (int64 Event=1;Event<=32;++Event)
	{
		BeginTheater(1,Event,Event%2==0);
		TArray<int32> Digits;
		// Observe over two full pattern periods, including a delayed authority wait.
		for (int32 Cell=7;Cell<57;++Cell)
		{
			const auto R=Access::At(*Screen,.92f+(Cell-6.5f)/5.470588f+.0001f);
			Digits.Add(R.CenterValue);
			TestTrue(TEXT("Theater decorative domain remains D6"),R.CenterValue>=1 && R.CenterValue<=Maximum);
			const int32 N=Digits.Num();
			if (N>1) TestTrue(TEXT("No immediate repeated cycling digit"),Digits[N-1]!=Digits[N-2]);
			if (N>2)
			{
				TestTrue(TEXT("No simple ABA alternation"),Digits[N-1]!=Digits[N-3]);
				const int32 A=(Digits[N-2]-Digits[N-3]+6)%6,B=(Digits[N-1]-Digits[N-2]+6)%6;
				if (!bHero) TestFalse(TEXT("No ascending/descending D6 run including wrap"),A==B && (A==1 || A==5));
			}
		}
		FString Key; for(int32 I=0;I<8;++I) Key+=FString::FromInt(Digits[I]); EventPrefixes.Add(Key);
	}
	TestTrue(TEXT("Different roll identities do not replay one fixed prefix"),EventPrefixes.Num()>8);
	for (int32 Final=1;Final<=Maximum;++Final)
	{
		BeginTheater(Final,71);
		TArray<FFMCodexUMGRollReelViewModel> Prefix;
		float PreviousPosition=0.f,PreviousVelocity=9.f;
		for (int32 Frame=0;Frame<=184;++Frame)
		{
			const auto R=Access::At(*Screen,Frame*.005f); Prefix.Add(R);
			if(Frame>1)
			{
				const float V=(R.ContinuousPositionCells-PreviousPosition)/.005f;
				TestTrue(TEXT("Cycling progressively slows without reverse travel"),V>0 && V<=PreviousVelocity+.002f);
				PreviousVelocity=V;
			}
			PreviousPosition=R.ContinuousPositionCells;
		}
		const auto Before=Access::At(*Screen,.92f); Access::Capture(*Screen);
		const auto Entry=Access::At(*Screen,0.f);
		TestTrue(TEXT("Capture preserves visible cells without relabeling"),Before.CenterValue==Entry.CenterValue && Before.NextValue==Entry.NextValue && FMath::IsNearlyEqual(Before.ContinuousPositionCells,Entry.ContinuousPositionCells));
		PreviousPosition=Entry.ContinuousPositionCells;
		for (int32 Frame=1;Frame<=108;++Frame)
		{
			const auto R=Access::At(*Screen,Frame*.005f);
			const float V=(R.ContinuousPositionCells-PreviousPosition)/.005f;
			TestTrue(TEXT("Landing decelerates continuously without late acceleration"),V>=0.f && V<=PreviousVelocity+.003f);
			PreviousVelocity=V; PreviousPosition=R.ContinuousPositionCells;
			Widget->RefreshFromPresentation(R);
			TestEqual(TEXT("No landing scale pump"),Widget->GetCenterRenderScale(),1.f);
			if (Frame==100)
			{
				TestEqual(TEXT("Authority target visibly approaches as incoming cell"),R.NextValue,Final);
				const auto* Incoming=CastChecked<UTextBlock>(Widget->GetWidgetFromName(TEXT("RollReelNextDigit")));
				TestTrue(TEXT("Incoming target stays bright during ghost fade"),Incoming->GetRenderOpacity()>.97f);
				TestTrue(TEXT("Incoming target is already within one pixel of its final baseline"),Incoming->GetRenderTransform().Translation.Y<1.f);
			}
		}
		TestEqual(TEXT("Every authoritative D6 ends in exact center"),Widget->GetPresentation().CenterValue,Final);
		TestEqual(TEXT("Landing ends at zero offset"),Widget->GetCenterVerticalOffset(),0.f);
		BeginTheater(Final%Maximum+1,71);
		for(int32 Frame=0;Frame<Prefix.Num();++Frame)
		{
			const auto R=Access::At(*Screen,Frame*.005f);
			TestTrue(TEXT("Pre-capture pattern/position independent of final value"),R.CenterValue==Prefix[Frame].CenterValue && R.NextValue==Prefix[Frame].NextValue && R.ContinuousPositionCells==Prefix[Frame].ContinuousPositionCells);
		}
	}
	BeginTheater(Maximum,73); Access::SetAvailable(*Screen,false);
	auto Waiting=Access::At(*Screen,4.83f); Access::Capture(*Screen);
	TestEqual(TEXT("Late authority cannot start Theater landing early"),Screen->GetInlineFormulaRevealPhase(),EFMCodexUMGInlineFormulaRevealPhase::Cycling);
	for(int32 Frame=0;Frame<20;++Frame)
	{
		// Use the production narrow tick: this cosmetic fixture has no full match View.
		Access::TickMotion(*Screen,.016f);
		const auto Next=Access::Current(*Screen);
		TestTrue(TEXT("Repeated authority-wait ticks preserve forward motion"),Next.ContinuousPositionCells>Waiting.ContinuousPositionCells);
		Waiting=Next;
	}
	Access::SetAvailable(*Screen,true); Access::Capture(*Screen);
	TestEqual(TEXT("Late authority keeps exact moving position"),Access::At(*Screen,0.f).ContinuousPositionCells,Waiting.ContinuousPositionCells);
	TestEqual(TEXT("Late authority lands through same bounded capture"),Access::At(*Screen,.54f).CenterValue,Maximum);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFMCodexRollPresentationReuseTest,
	"FMCodex.LocalPlay.RollPresentation.ChamberReuse",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexRollPresentationReuseTest::RunTest(const FString&)
{
	auto* Reel = NewObject<UFMCodexRollReelWidget>();
	Reel->TakeWidget();
	const UTextBlock* Center = Reel->GetCenterDigitWidget();
	auto* Chamber = Cast<UFMCodexRollPresentationSurface>(Reel->GetWidgetFromName(TEXT("RollReelClippedWindow")));
	if (!TestNotNull(TEXT("Shared chamber uses roll-only procedural frame"), Chamber)) return false;
	auto* Bounds = CastChecked<USizeBox>(Reel->GetWidgetFromName(TEXT("RollReelBounds")));
	TestTrue(TEXT("Compact consumers retain their 68x72 layout"), Bounds->GetWidthOverride() == 68 && Bounds->GetHeightOverride() == 72);
	for (int32 Maximum : {6, 12})
	{
		Reel->SetExpandedChamber(Maximum == 12);
		TestEqual(TEXT("Only the main modal expands its internal chamber"), Bounds->GetWidthOverride(), Maximum == 12 ? 96.0f : 68.0f);
		FFMCodexUMGRollReelViewModel Model;
		Model.bVisible = true; Model.bMoving = true; Model.bShowNeighborDigits = true;
		Model.DomainMinimum = 1; Model.DomainMaximum = Maximum;
		Model.PreviousValue = Maximum-1; Model.CenterValue = Maximum; Model.NextValue = 1;
		Model.ScrollAlpha = .35f;
		Reel->RefreshFromPresentation(Model);
		TestTrue(TEXT("Reused D6/D12 chamber is clipped and moving"),
			Reel->HasClippedWindow() && Reel->GetRenderedChildCount() == 3
			&& Reel->GetVisibleNeighborDigitCount() == 2 && Reel->GetCenterVerticalOffset() < 0);
		TestEqual(TEXT("No stale gold lock survives a new roll"), Chamber->GetLockEmphasis(), 0.0f);
		Model.bMoving = false; Model.bShowNeighborDigits = false;
		Model.bStaticResult = true; Model.bAuthoritativeValue = true; Model.bResultHold = true;
		Reel->RefreshFromPresentation(Model);
		TestTrue(TEXT("Final projected value retains the same stable center widget"),
			Reel->GetCenterDigitWidget() == Center && Reel->IsStaticResultTileVisible()
			&& Center->GetText().ToString() == FString::FromInt(Maximum)
			&& Reel->GetCenterRenderOpacity() == 1 && Reel->GetCenterRenderScale() == 1
			&& Reel->GetCenterVerticalOffset() == 0);
		TestEqual(TEXT("Result activates the gold lock line"), Chamber->GetLockEmphasis(), 1.0f);
		Reel->RefreshFromPresentation({});
		TestEqual(TEXT("Hide clears lock decoration before reuse"), Chamber->GetLockEmphasis(), 0.0f);
		TestTrue(TEXT("Hidden resets text, neighbors, opacity and transform before reuse"),
			Reel->GetVisibility() == ESlateVisibility::Collapsed && Center->GetText().IsEmpty()
			&& Reel->GetVisibleNeighborDigitCount() == 0 && Reel->GetCenterRenderOpacity() == 0
			&& Reel->GetCenterVerticalOffset() == 0 && Reel->GetCenterRenderScale() == 1);
	}
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCompactBoxSkinTest,
	"FMCodex.LocalPlay.RollPresentation.CompactBoxSkin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCompactBoxSkinTest::RunTest(const FString&)
{
	auto* Reel=NewObject<UFMCodexRollReelWidget>(); Reel->TakeWidget();
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::CompactBox);
	auto* Bounds=CastChecked<USizeBox>(Reel->GetWidgetFromName(TEXT("RollReelBounds")));
	const auto* Center=Reel->GetCenterDigitWidget();
	Reel->SetExpandedChamber(true);
	TestTrue(TEXT("CompactBox geometry is independent of Legacy expanded chamber"),Bounds->GetWidthOverride()==84.f && Bounds->GetHeightOverride()==72.f);
	TestEqual(TEXT("CompactBox shares production Medium type size"),Center->GetFont().Size,40.f);
	TestNull(TEXT("Route result has no hover explanation"),Reel->GetToolTip());
	FFMCodexUMGRollReelViewModel P;
	P.bVisible=true; P.bMoving=true; P.bShowNeighborDigits=true;
	P.PreviousValue=2; P.CenterValue=4; P.NextValue=6; P.ScrollAlpha=.4f;
	P.LandingOffsetY=-2; P.LandingScale=1.1f;
	Reel->RefreshFromPresentation(P);
	TestEqual(TEXT("CompactBox never inherits Legacy scale pulse"),Reel->GetCenterRenderScale(),1.f);
	TestTrue(TEXT("Fixed clipped cell contains the shared three digits"),Reel->HasClippedWindow() && Reel->GetRenderedChildCount()==3 && Reel->GetVisibleNeighborDigitCount()==2);
	TestTrue(TEXT("Route rolling accent is aqua"),Center->GetColorAndOpacity().GetSpecifiedColor().G>Center->GetColorAndOpacity().GetSpecifiedColor().R);
	P.bMoving=false; P.bShowNeighborDigits=false; P.bStaticResult=true; P.bAuthoritativeValue=true; P.NeighborFadeAlpha=1;
	Reel->RefreshFromPresentation(P);
	TestTrue(TEXT("Locked value uses the same child without neighbors"),Reel->IsStaticResultTileVisible() && Reel->GetCenterDigitWidget()==Center && Reel->GetCenterVerticalOffset()==0);
	TestEqual(TEXT("Settled route is neutral white"),Center->GetColorAndOpacity().GetSpecifiedColor(),FLinearColor::FromSRGBColor(FColor(233,245,248)));
	Reel->RefreshFromPresentation({});
	TestTrue(TEXT("Unresolved hidden state cannot retain old result"),Center->GetText().IsEmpty() && Reel->GetVisibility()==ESlateVisibility::Collapsed);
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::Legacy); Reel->SetExpandedChamber(false);
	TestTrue(TEXT("Legacy remains the original independent footprint"),Bounds->GetWidthOverride()==68.f && Bounds->GetHeightOverride()==72.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHeroRollSkinTest,
	"FMCodex.LocalPlay.RollPresentation.HeroRollSkinAndScope",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHeroRollSkinTest::RunTest(const FString&)
{
	auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(); Screen->TakeWidget();
	auto* Reel=Screen->GetTacticalPointRollReel();
	TestEqual(TEXT("Only main-board D12 host explicitly selects Hero"),Reel->GetVisualVariant(),EFMCodexRollVisualVariant::HeroRoll);
	TestEqual(TEXT("Generic Formula host stays Legacy"),Screen->GetInlineFormulaSurface()->GetRollReelWidget()->GetVisualVariant(),EFMCodexRollVisualVariant::Legacy);
	auto* Shell=CastChecked<USizeBox>(Screen->GetWidgetFromName(TEXT("TacticalRollModalBounds")));
	auto* Chamber=CastChecked<USizeBox>(Reel->GetWidgetFromName(TEXT("RollReelBounds")));
	TestTrue(TEXT("Large shell retains safe board-centered dimensions"),Shell->GetWidthOverride()==380.f && Chamber->GetWidthOverride()==190.f && Chamber->GetHeightOverride()==184.f);
	const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	for (int32 Value=1;Value<=12;++Value)
		TestTrue(TEXT("All D12 numbers fit without shrinking or clipping"),Measure->Measure(FString::FromInt(Value),Reel->GetCenterDigitWidget()->GetFont()).X<160.f);
	FFMCodexUMGMatchScreenViewModel P;
	P.Interaction.Category=EFMCodexUMGInteractionCategory::TacticalPointRoll;
	P.Interaction.ExpectedActorLabel=TEXT("玩家 A");
	for (bool bActor:{true,false})
	{
		P.Interaction.bCanRollTacticalPoints=bActor; Screen->RefreshFromPresentation(P);
		TestEqual(TEXT("Both viewers keep a clean pre-roll board"),Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"))->GetVisibility(),ESlateVisibility::Collapsed);
		TestTrue(TEXT("Waiting never invents a result"),Reel->GetCenterDigitWidget()->GetText().IsEmpty() && Reel->GetVisibility()==ESlateVisibility::Collapsed);
		const auto Copy=CastChecked<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult")))->GetText().ToString();
		TestTrue(TEXT("No redundant pre-roll copy panel"),Copy.IsEmpty());
		TestEqual(TEXT("Pre-roll roster has normal emphasis"),Screen->GetLocalRackWidget()->GetColorAndOpacity(),FLinearColor::White);
	}
	using Access=FFMCodexRollCosmeticTestAccess;
	Access::Begin(*Screen,12,9); Access::Hero(*Screen); Access::Refresh(*Screen,.18f);
	TestTrue(TEXT("Hero focus subdues both rosters more than pitch and score"),
		Screen->GetLocalRackWidget()->GetColorAndOpacity().R < Screen->GetPitchWidget()->GetColorAndOpacity().R
		&& Screen->GetPitchWidget()->GetColorAndOpacity().R < Screen->GetMatchHeader()->GetColorAndOpacity().R
		&& Screen->GetOpponentRackWidget()->GetColorAndOpacity()==Screen->GetLocalRackWidget()->GetColorAndOpacity());
	Access::Hide(*Screen);
	TestTrue(TEXT("Cancellation restores all focus tints"),
		Screen->GetLocalRackWidget()->GetColorAndOpacity()==FLinearColor::White
		&& Screen->GetOpponentRackWidget()->GetColorAndOpacity()==FLinearColor::White
		&& Screen->GetPitchWidget()->GetColorAndOpacity()==FLinearColor::White
		&& Screen->GetMatchHeader()->GetColorAndOpacity()==FLinearColor::White);
	P.Interaction.Category=EFMCodexUMGInteractionCategory::Deploy; Screen->RefreshFromPresentation(P);
	TestEqual(TEXT("Hero does not take over ordinary deployment"),Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"))->GetVisibility(),ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTheaterRollSkinTest,
	"FMCodex.LocalPlay.RollPresentation.TheaterInlineSkin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTheaterRollSkinTest::RunTest(const FString&)
{
	auto* Legacy = NewObject<UFMCodexRollReelWidget>(); Legacy->TakeWidget();
	auto* Theater = NewObject<UFMCodexRollReelWidget>(); Theater->TakeWidget();
	Theater->SetVisualVariant(EFMCodexRollVisualVariant::TheaterInline);
	TestFalse(TEXT("Legacy consumers stay opt-out"),Legacy->UsesTheaterInlineSkin());
	TestTrue(TEXT("Theater skin opts in without a second state machine"),Theater->UsesTheaterInlineSkin());
	auto* Bounds=CastChecked<USizeBox>(Theater->GetWidgetFromName(TEXT("RollReelBounds")));
	const auto* Center=Theater->GetCenterDigitWidget();
	const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	for (const TCHAR* Value:{TEXT("?"),TEXT("1"),TEXT("2"),TEXT("3"),TEXT("4"),TEXT("5"),TEXT("6")})
		TestTrue(TEXT("Every High Cross D6/pending glyph fits fixed slot"),Measure->Measure(Value,Center->GetFont()).X<68.f);
	TestNull(TEXT("Roll operand has no hover explanation"),Theater->GetToolTip());
	for (int32 Value=1; Value<=6; ++Value)
	{
		FFMCodexUMGRollReelViewModel P;
		P.bVisible=true; P.bMoving=true; P.bShowNeighborDigits=true;
		P.CenterValue=Value; P.PreviousValue=Value==1?6:Value-1; P.NextValue=Value==6?1:Value+1;
		P.ScrollAlpha=.4f; P.ContinuousPositionCells=20.4f;
		P.LandingOffsetY=-1.5f; P.LandingScale=1.025f;
		Legacy->RefreshFromPresentation(P); Theater->RefreshFromPresentation(P);
		TestEqual(TEXT("Same source position reaches both views"),Theater->GetPresentation().ContinuousPositionCells,Legacy->GetPresentation().ContinuousPositionCells);
		TestEqual(TEXT("No result generation or conversion in skin"),Center->GetText().ToString(),FString::FromInt(Value));
		TestEqual(TEXT("Theater removes spring/scale while preserving time"),Theater->GetCenterRenderScale(),1.f);
		TestTrue(TEXT("Theater clips ghosted neighboring digits"),Theater->HasClippedWindow() && Theater->GetVisibleNeighborDigitCount()==2);
		TestEqual(TEXT("Stable equation slot width"),Bounds->GetWidthOverride(),68.f);
		TestEqual(TEXT("Stable equation slot height"),Bounds->GetHeightOverride(),76.f);
		P.bMoving=false; P.bShowNeighborDigits=false; P.bStaticResult=true; P.bAuthoritativeValue=true; P.bResultHold=true;
		Theater->RefreshFromPresentation(P);
		TestTrue(TEXT("Same digit holds the authoritative value without replacement"),Theater->IsStaticResultTileVisible() && Theater->GetCenterDigitWidget()==Center);
		TestEqual(TEXT("Exact zero landing offset"),Theater->GetCenterVerticalOffset(),0.f);
		TestEqual(TEXT("Revealed digit retains Theater type size"),Center->GetFont().Size,40.f);
		TestEqual(TEXT("Roll is neutral aqua, not gold winner treatment"),Center->GetColorAndOpacity().GetSpecifiedColor(),FLinearColor::FromSRGBColor(FColor(68,226,216)));
		Theater->RefreshFromPresentation({});
		TestTrue(TEXT("Reuse clears any prior number"),Center->GetText().IsEmpty());
	}
	Theater->SetVisualVariant(EFMCodexRollVisualVariant::Legacy);
	TestEqual(TEXT("Same widget can return to Legacy without rebuilding roll state"),Theater->GetVisualVariant(),EFMCodexRollVisualVariant::Legacy);
	TestEqual(TEXT("Legacy compact geometry restored"),Bounds->GetHeightOverride(),72.f);
	TestFalse(TEXT("Old frame remains isolated"),CastChecked<UFMCodexRollPresentationSurface>(Legacy->GetWidgetFromName(TEXT("RollReelClippedWindow")))->VisualVariant == EFMCodexRollVisualVariant::TheaterInline);
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

namespace
{
TSharedPtr<SWindow> HeroPIEWindow;
class FStartRollPresentationPIE final : public IAutomationLatentCommand
{
public:
	virtual bool Update() override
	{
		// Per-session copy: never persist or replace the user's editor settings.
		auto* Settings = DuplicateObject<ULevelEditorPlaySettings>(
			GetDefault<ULevelEditorPlaySettings>(), GetTransientPackage());
		Settings->NewWindowWidth = 1920; Settings->NewWindowHeight = 1080;
		Settings->SetPlayNetMode(EPlayNetMode::PIE_Standalone);
		Settings->SetPlayNumberOfClients(1);
		HeroPIEWindow = SNew(SWindow).Title(FText::FromString(TEXT("Hero Roll PIE")))
			.ClientSize(FVector2D(1920,1080)).ScreenPosition(FVector2D(0,0)).AutoCenter(EAutoCenter::None)
			.SaneWindowPlacement(false).AdjustInitialSizeAndPositionForDPIScale(false).SizingRule(ESizingRule::UserSized);
		FSlateApplication::Get().AddWindow(HeroPIEWindow.ToSharedRef());
		FRequestPlaySessionParams Params;
		Params.EditorPlaySettings = Settings;
		Params.CustomPIEWindow = HeroPIEWindow;
		GEditor->RequestPlaySession(Params);
		return true;
	}
};

class FRollPresentationPIEPath final : public IAutomationLatentCommand
{
public:
	FRollPresentationPIEPath(FAutomationTestBase* InTest,int32 InD12Value) : Test(InTest),D12Value(InD12Value) {}
	virtual bool Update() override
	{
		if (FPlatformTime::Seconds()-Started > 60)
		{
			Test->AddError(TEXT("Roll presentation PIE path timed out")); return true;
		}
		auto* Controller = GEditor && GEditor->PlayWorld
			? Cast<AFMCodexLocalMatchPlayerController>(GEditor->PlayWorld->GetFirstPlayerController()) : nullptr;
		auto* Screen = Controller ? Controller->GetPlayerMatchScreen() : nullptr;
		if (!Screen) return false;
		if (!bStartedMatch)
		{
			Screen->RequestStartNewMatch();
			bStartedMatch = true; MatchStarted = FPlatformTime::Seconds();
			return false;
		}
		if (!bTypeRoll) CaptureMotion();
		// Let the actual newly constructed match layout render before the click.
		if (FPlatformTime::Seconds()-MatchStarted < 1.0) return false;
		if (!bStartedRoll)
		{
			FFMCodexLocalDevRollOverrideRequest Override;
			Override.Target = EFMCodexLocalDevRollTarget::FullD12;
			Override.Value = D12Value;
			if (!Controller->SetLocalDevRollOverride(Override).bSuccess)
			{ Test->AddError(TEXT("Existing DEV D12 provider seam rejected override")); return true; }
			Test->TestFalse(TEXT("Restart clears prior reveal"), Screen->IsInlineFormulaRevealInputBlocked());
			Capture(TEXT("01_PreRoll_CleanBoard"));
			Test->TestTrue(TEXT("Pre-roll board is clean, undimmed and actionable"),
				Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"))->GetVisibility()==ESlateVisibility::Collapsed
				&& Screen->GetLocalRackWidget()->GetColorAndOpacity()==FLinearColor::White
				&& Screen->GetInteractionPanel()->GetPresentation().bCanRollTacticalPoints);
			Screen->RequestRollTacticalPoints();
			if (!Controller->GetLastDiagnostic().bHostSuccess)
			{ Test->AddError(TEXT("PIE typed Full D12 request failed")); return true; }
			auto* EntryFrame = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"));
			Test->TestTrue(TEXT("Real Enter uses the current Cycling clock"), EntryFrame && EntryFrame->GetRenderOpacity() < 1);
			bStartedRoll = true; RollStarted = FPlatformTime::Seconds();
			RollGameStarted = GEditor->PlayWorld->GetTimeSeconds();
			return false;
		}
		auto* Reel = bTypeRoll ? Screen->GetInlineFormulaSurface()->GetRollReelWidget() : Screen->GetTacticalPointRollReel();
		auto* Frame = Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealSurface"));
		if (!Reel || !Frame) { Test->AddError(TEXT("PIE modal missing")); return true; }
		const auto Phase = Screen->GetInlineFormulaRevealPhase();
		const int32 PhaseIndex = static_cast<int32>(Phase);
		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling)
		{
			const int32 Cell = FMath::FloorToInt(Reel->GetPresentation().ContinuousPositionCells);
			if (Cell != LastCell)
			{
				Test->AddInfo(FString::Printf(TEXT("ROLL_VISIBLE domain=%s cell=%d previous=%d center=%d next=%d"),
					bTypeRoll ? TEXT("D6") : TEXT("D12"), Cell, Reel->GetPresentation().PreviousValue,
					Reel->GetPresentation().CenterValue, Reel->GetPresentation().NextValue));
				LastCell = Cell;
			}
		}
		if (PhaseIndex != LastPhase)
		{
			Test->AddInfo(FString::Printf(TEXT("ROLL_PIE domain=%s phase=%d wall=%.3f game=%.3f center=%d scale=%.3f opacity=%.3f"),
				bTypeRoll ? TEXT("D6") : TEXT("D12"), PhaseIndex, FPlatformTime::Seconds()-RollStarted, GEditor->PlayWorld->GetTimeSeconds()-RollGameStarted,
				Reel->GetPresentation().CenterValue, Reel->GetCenterRenderScale(), Frame->GetRenderOpacity()));
			LastPhase = PhaseIndex;
		}
		if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Cycling)
		{
			bCycling = true;
			if (!bCycleShot && GEditor->PlayWorld->GetTimeSeconds()-RollGameStarted > .20f)
			{
				Capture(bTypeRoll ? TEXT("D6_Cycling") : TEXT("02_HeroRoll_Rolling"));
				bCycleShot = true;
				if (!bTypeRoll) Test->TestTrue(TEXT("Rolling focuses the board without a duplicate action"),
					Screen->GetLocalRackWidget()->GetColorAndOpacity().R < .5f
					&& Screen->IsInlineFormulaRevealInputBlocked());
			}
		}
		else if (Phase == EFMCodexUMGInlineFormulaRevealPhase::Settling)
		{
			bSettling = true;
			Test->TestTrue(TEXT("Modal remains visible through real settling"), bTypeRoll || Frame->GetVisibility() != ESlateVisibility::Collapsed);
		}
		else if (Phase == EFMCodexUMGInlineFormulaRevealPhase::ResultHold)
		{
			if (!bTypeRoll && bResultShot && !bContextShot && GEditor->PlayWorld->GetTimeSeconds()-HoldStarted>.8f)
			{
				Capture(TEXT("06_BoardContext_HeroFocus")); bContextShot=true;
				const auto& G=Frame->GetCachedGeometry();
				const auto& Pitch=Screen->GetWidgetFromName(TEXT("BoardResolutionOverlays"))->GetCachedGeometry();
				const auto P=Pitch.AbsoluteToLocal(G.GetAbsolutePosition());
				const auto End=Pitch.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
				Test->TestTrue(TEXT("Hero remains wholly inside pitch, clear of header and player racks"),
					P.X>=0 && P.Y>=0 && End.X<=Pitch.GetLocalSize().X && End.Y<=Pitch.GetLocalSize().Y);
			}
			if (!bHeld)
			{
				Test->TestTrue(TEXT("PIE result is the provider-accepted value, with no moving neighbors"),
					Reel->IsStaticResultTileVisible() && Reel->GetPresentation().bAuthoritativeValue
					&& Reel->GetPresentation().CenterValue == (bTypeRoll ? 5 : D12Value));
				if (!bTypeRoll) Capture(TEXT("03_HeroRoll_Landed"));
				HoldStarted = GEditor->PlayWorld->GetTimeSeconds();
				bHeld = true;
			}
			if (!bResultShot && GEditor->PlayWorld->GetTimeSeconds()-HoldStarted > .25f)
			{
				Capture(bTypeRoll ? TEXT("D6_Result") : TEXT("04_HeroRoll_Result")); bResultShot = true;
				if (!bTypeRoll)
				{
					auto* Footer = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealResult")));
					auto* State = Cast<UTextBlock>(Screen->GetWidgetFromName(TEXT("TacticalPointRollRevealState")));
					Test->TestTrue(TEXT("Runtime result has one concise authoritative semantic line"),
						Footer && Footer->GetFont().Size == 20
						&& Footer->GetText().ToString() == (D12Value==4 ? TEXT("本回合战术点：4") : TEXT("触发定位球"))
						&& State && State->GetVisibility()==ESlateVisibility::Collapsed && State->GetText().IsEmpty());
				}
			}
		}
		else if (!Screen->IsInlineFormulaRevealInputBlocked())
		{
			Test->TestTrue(TEXT("Natural PIE clock visited Cycling, Settling and ResultHold"), bCycling && bSettling && bHeld);
			if (bTypeRoll)
			{
				Test->TestTrue(TEXT("Legitimate successor D6 exits with no prior digit or lock state"),
					Reel->GetCenterDigitWidget()->GetText().IsEmpty()
					&& Reel->GetCenterRenderOpacity() == 0 && Reel->GetCenterRenderScale() == 1);
				SaveEvidence();
				return true;
			}
			Test->TestTrue(TEXT("Exit clears the modal and prior number"),
				Frame->GetVisibility() == ESlateVisibility::Collapsed && Reel->GetCenterDigitWidget()->GetText().IsEmpty()
				&& Frame->GetRenderOpacity() == 1 && Frame->GetRenderTransform().Scale.Equals(FVector2D(1)));
			Test->TestTrue(TEXT("Hero evidence includes rolling, landed and board context"),
				bCycleShot && bResultShot && bContextShot);
			Test->TestTrue(TEXT("Hero exit restores board tint before the next typed event"),
				Screen->GetLocalRackWidget()->GetColorAndOpacity()==FLinearColor::White
				&& Screen->GetPitchWidget()->GetColorAndOpacity()==FLinearColor::White
				&& Screen->GetMatchHeader()->GetColorAndOpacity()==FLinearColor::White);
			if (RestoredAt<0) { RestoredAt=GEditor->PlayWorld->GetTimeSeconds(); return false; }
			if (GEditor->PlayWorld->GetTimeSeconds()-RestoredAt<.30) return false;
			Capture(TEXT("05_Board_Restored"));
			if (D12Value==4)
			{
				Test->TestEqual(TEXT("Ordinary tactical points return to deployment after the hold"),
					Screen->GetInteractionPanel()->GetPresentation().Category,EFMCodexUMGInteractionCategory::Deploy);
				SaveEvidence();
				return true;
			}
			// A new typed event in the same match, not the known same-sequence restart.
			FFMCodexLocalDevRollOverrideRequest Override;
			Override.Target = EFMCodexLocalDevRollTarget::SetPieceType; Override.Value = 5;
			if (!Controller->SetLocalDevRollOverride(Override).bSuccess)
			{ Test->AddError(TEXT("DEV type D6 seam rejected override")); return true; }
			Screen->RequestContinueResolution();
			if (!Controller->GetLastDiagnostic().bHostSuccess)
			{ Test->AddError(TEXT("Legitimate typed Set Piece successor failed")); return true; }
			bTypeRoll = true; bCycling = bSettling = bHeld = bCycleShot = bResultShot = false; LastPhase = LastCell = -1;
			RollStarted = FPlatformTime::Seconds(); RollGameStarted = GEditor->PlayWorld->GetTimeSeconds();
		}
		return false;
	}
private:
	struct FProofFrame { FString Name; double Time; FIntVector Size; TArray<FColor> Pixels; };
	TArray<FProofFrame> ProofFrames;
	double LastMotion=-1, RestoredAt=-1;
	int32 MotionIndex=0;
	void CaptureMotion()
	{
		const double Now=GEditor->PlayWorld->GetTimeSeconds();
		if (Now-LastMotion < .066) return;
		LastMotion=Now;
		Capture(*FString::Printf(TEXT("Motion_%04d"),MotionIndex++),true);
	}
	void Capture(const TCHAR* State, bool bMotion=false)
	{
		// Read real current Slate output. No clock pause, result forcing or
		// synthetic frames. Defer compression until after the gameplay path.
		auto Window=GEditor->PlayWorld->GetGameViewport()->GetWindow();
		FProofFrame Frame; Frame.Name=State; Frame.Time=GEditor->PlayWorld->GetTimeSeconds();
		if (!Window.IsValid() || !FSlateApplication::Get().TakeScreenshot(Window.ToSharedRef(),Frame.Pixels,Frame.Size))
		{ Test->AddError(TEXT("Current-phase Slate capture failed")); return; }
		if (bMotion)
		{
			TArray<FColor> Small;
			const int32 Height=FMath::RoundToInt(960.f*Frame.Size.Y/Frame.Size.X);
			FImageUtils::ImageResize(Frame.Size.X,Frame.Size.Y,Frame.Pixels,960,Height,Small,false);
			Frame.Pixels=MoveTemp(Small); Frame.Size=FIntVector(960,Height,0);
		}
		else Test->AddInfo(FString::Printf(TEXT("ROLL_CAPTURE %s %dx%d game=%.3f"),State,Frame.Size.X,Frame.Size.Y,Frame.Time-RollGameStarted));
		ProofFrames.Add(MoveTemp(Frame));
	}
	void SaveEvidence()
	{
		const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()
			/TEXT("Stage8_10B_2/PIE")/(D12Value==4 ? TEXT("Ordinary") : TEXT("SetPiece")));
		IFileManager::Get().MakeDirectory(*Directory,true);
		FString Times=TEXT("file,game_seconds\n");
		for (const auto& Frame:ProofFrames)
		{
			TArray64<uint8> PNG;
			FImageUtils::PNGCompressImageArray(Frame.Size.X,Frame.Size.Y,Frame.Pixels,PNG);
			Test->TestTrue(TEXT("Real PIE frame saved"),FFileHelper::SaveArrayToFile(PNG,*(Directory/(Frame.Name+TEXT(".png")))));
			Times+=FString::Printf(TEXT("%s.png,%.6f\n"),*Frame.Name,Frame.Time);
		}
		FFileHelper::SaveStringToFile(Times,*(Directory/TEXT("frame-times.csv")));
		Test->AddInfo(FString::Printf(TEXT("Real motion samples: %d; elapsed game-time timestamps saved"),MotionIndex));
	}
	FAutomationTestBase* Test;
	const int32 D12Value;
	double Started = FPlatformTime::Seconds(), RollStarted = 0, MatchStarted = 0;
	double RollGameStarted = 0, HoldStarted = 0;
	int32 LastPhase = -1, LastCell = -1;
	bool bTypeRoll = false, bResultShot = false, bContextShot = false;
	bool bStartedMatch = false;
	bool bStartedRoll = false, bCycling = false, bSettling = false, bHeld = false, bCycleShot = false;
};
}

IMPLEMENT_COMPLEX_AUTOMATION_TEST(FFMCodexRollPresentationPIETest,
	"FMCodex.PIE.RollPresentation.HeroRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
void FFMCodexRollPresentationPIETest::GetTests(TArray<FString>& Names,TArray<FString>& Commands) const
{
	Names.Add(TEXT("Ordinary")); Commands.Add(TEXT("Ordinary"));
	Names.Add(TEXT("SetPiece")); Commands.Add(TEXT("SetPiece"));
}
bool FFMCodexRollPresentationPIETest::RunTest(const FString& Parameters)
{
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FStartRollPresentationPIE()));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FRollPresentationPIEPath(this,Parameters==TEXT("Ordinary") ? 4 : 9)));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
