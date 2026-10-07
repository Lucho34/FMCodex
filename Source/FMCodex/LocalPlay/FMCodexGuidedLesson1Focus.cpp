#include "FMCodexGuidedLesson1Focus.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "Widgets/Images/SImage.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Layout/WidgetPath.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScaleBox.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FMCodexLesson1Focus"
namespace FMCodexLesson1Focus
{
FGlyphDots MeasureGlyphDots(const FString& Text, const FSlateFontInfo& Font, float Scale)
{
	FGlyphDots Result;
	const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	Result.TextSize=FVector2D(Measure->Measure(Text,Font,Scale));
	Result.Diameter=Result.TextSize.Y*.085f;
	Result.Gap=Result.TextSize.Y*.055f;
	for (int32 I=0;I<Text.Len();++I)
	{
		const TCHAR C=Text[I];
		const bool Han=(C>=0x3400 && C<=0x9fff) || (C>=0xf900 && C<=0xfaff);
		if (!Han && !FChar::IsAlnum(C)) continue; // Spaces/punctuation keep their advance, not a dot.
		// FontMeasure ranges are [Start, End), including for a single character.
		const float End=Measure->Measure(Text,0,I+1,Font,true,Scale).X;
		const float Advance=Measure->Measure(Text,I,I+1,Font,false,Scale).X;
		Result.Centers.Add(End-Advance*.5f);
	}
	return Result;
}

TOptional<FSlateRect> ConvertTargetGeometry(const FGeometry& Target, const FGeometry& Overlay, ETargetShape Shape)
{
	const auto Size=Target.GetLocalSize();
	if (Size.X<=0 || Size.Y<=0 || Overlay.GetLocalSize().X<=0 || Overlay.GetLocalSize().Y<=0) return {};
	FSlateRect R(MAX_flt,MAX_flt,-MAX_flt,-MAX_flt);
	for (const auto Corner : {FVector2D::ZeroVector,FVector2D(Size.X,0),FVector2D(0,Size.Y),FVector2D(Size)})
	{
		const auto P=Overlay.AbsoluteToLocal(Target.LocalToAbsolute(Corner));
		if (!FMath::IsFinite(P.X) || !FMath::IsFinite(P.Y)) return {};
		R.Left=FMath::Min(R.Left,static_cast<float>(P.X)); R.Right=FMath::Max(R.Right,static_cast<float>(P.X));
		R.Top=FMath::Min(R.Top,static_cast<float>(P.Y)); R.Bottom=FMath::Max(R.Bottom,static_cast<float>(P.Y));
	}
	// Even visual breathing room only, never a coordinate correction.
	float Padding=0.f;
	switch (Shape)
	{
	case ETargetShape::Card: case ETargetShape::Button: Padding=3.f; break;
	case ETargetShape::FullCardRow: Padding=1.f; break;
	case ETargetShape::DeploymentSlot: case ETargetShape::FormulaValue: Padding=2.f; break;
	default: break;
	}
	R=R.ExtendBy(FMargin(Padding));
	if (R.Right<=0 || R.Bottom<=0 || R.Left>=Overlay.GetLocalSize().X || R.Top>=Overlay.GetLocalSize().Y) return {};
	return R;
}

TOptional<FSlateRect> TargetRect(UWidget* Target, const FGeometry& OverlayDesktop, ETargetShape Shape)
{
	if (!Target || !Target->IsVisible()) return {};
	if (auto* Card=Cast<UFMCodexPlayerCardWidget>(Target))
		for (const auto Name : {TEXT("HandMicroVisualSystem"),TEXT("PitchMiniContent")})
			if (auto* Surface=Card->GetWidgetFromName(Name); Surface && Surface->IsVisible()) { Target=Surface; break; }
	const auto Slate=Target->GetCachedWidget();
	if (!Slate.IsValid()) return {};
	// Re-arrange the live visible path in desktop space. This handles render transforms,
	// window position and DPI once, and cannot reuse a detached/hidden target's old cache.
	FWidgetPath Path;
	if (!FSlateApplication::Get().GeneratePathToWidgetUnchecked(Slate.ToSharedRef(),Path) || !Path.IsValid()) return {};
	const auto& Geometry=Path.Widgets.Last().Geometry;
	if (Slate->GetDesiredSize().IsNearlyZero()) return {}; // Wait for normal layout/prepass.
	return ConvertTargetGeometry(Geometry,OverlayDesktop,Shape);
}

TArray<UWidget*> Targets(UFMCodexLocalMatchScreenWidget* S, const FFMCodexGuidedLesson1& L)
{
	TArray<UWidget*> Result;
	if (!S) return Result;
	auto Add = [&](UWidget* W) { if (W && W->IsVisible()) Result.AddUnique(W); };
	using Focus = EFMCodexLesson1Focus;
	const auto F = L.FocusTarget();
	if (F == Focus::TacticPoint || F == Focus::FinishDeployment)
	{
		if (auto* P = S->GetInteractionPanel()) Add(P->GetWidgetFromName(F == Focus::TacticPoint
			? TEXT("InteractionTacticalPointRollButton") : TEXT("InteractionFinishDeploymentButton")));
	}
	if (F == Focus::HandCard || F == Focus::Deployment)
	{
		if (auto* Rack = S->GetLocalRackWidget())
			for (UFMCodexPlayerCardWidget* Card : Rack->GetRenderedCardWidgets())
				if (Card->GetPresentation().CardId == L.Attacker() || (F == Focus::Deployment && L.IsComparison() && Card->GetPresentation().CardId == L.Gyokeres())) Add(Card);
	}
	if (F == Focus::Deployment || F == Focus::Carrier)
	{
		if (auto* P = S->GetPitchWidget())
			for (UFMCodexPitchSlotWidget* Slot : P->GetRenderedSlotWidgets())
				if (Slot->GetPresentation().SlotId == L.AttackerSlot())
					Add(F == Focus::Carrier ? static_cast<UWidget*>(Slot->GetCardWidget()) : Slot);
	}
	if (F == Focus::Skill)
	{
		if (auto* P = S->GetInteractionPanel())
			for (UFMCodexInteractionOptionWidget* Option : P->GetRenderedOptionWidgets())
				if (Option->IsTacticalCard() && Option->GetOptionId() == L.Skill()) Add(Option);
	}
	if (L.OpponentTarget()==EFMCodexLesson1OpponentTarget::HandCard)
	{
		if (auto* Rack = S->GetOpponentRackWidget())
			for (UFMCodexPlayerCardWidget* Card : Rack->GetRenderedCardWidgets()) if (Card->GetPresentation().CardId == L.Stones()) Add(Card);
	}
	if (L.OpponentTarget()==EFMCodexLesson1OpponentTarget::FieldCard)
		if (auto* Pitch = S->GetPitchWidget())
			for (UFMCodexPitchSlotWidget* Slot : Pitch->GetRenderedSlotWidgets()) if (Slot->GetPresentation().SlotId == L.DefenderSlot()) Add(Slot->GetCardWidget());
	// Automatic B has no production finish button. Point to its real side status.
	if (L.OpponentTarget()==EFMCodexLesson1OpponentTarget::SideStatus && S->GetMatchHeader())
		Add(S->GetMatchHeader()->GetWidgetFromName(TEXT("RightPlayerBroadcastRegion")));
	if (L.KeepsFullCardOpen() && S->IsDetailOverlayVisible())
	{
	 auto* Card=S->GetDetailOverlayCard();
	 if (L.GetStep()==EFMCodexLesson1Step::ShootingExplanation) Add(Card->FindAttributePresentationWidget(TEXT("SHO")));
	 if (L.GetStep()==EFMCodexLesson1Step::SkillRangeExplanation) Add(Card->FindSkillPresentationWidget(L.Skill()));
	 if (L.GetStep()==EFMCodexLesson1Step::TraitExplanation) Add(Card->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier")));
	}
	if (F == Focus::DirectShot) Add(S->GetWidgetFromName(TEXT("TheaterNearDirect")));
	if (F == Focus::FormulaValue) Add(S->GetWidgetFromName(TEXT("TheaterAttackBaseHover")));
	if (F == Focus::AttackRoll) Add(S->GetWidgetFromName(TEXT("TheaterContinue")));
	return Result;
}

// A small lesson-owned skin. No registration or changes to the production UI theme.
struct FVisuals
{
	static FLinearColor Color(uint8 R,uint8 G,uint8 B,uint8 A=255) { return FLinearColor::FromSRGBColor(FColor(R,G,B,A)); }
	const FLinearColor Amber=Color(221,180,108), Mint=Color(42,232,197), White=Color(239,247,252), Quiet=Color(139,177,200);
	FSlateRoundedBoxBrush Panel{Color(9,27,43),12.f,Color(111,167,192),1.f};
	FSlateRoundedBoxBrush Inner{FLinearColor::Transparent,10.f,Color(46,76,97),1.f};
	FSlateRoundedBoxBrush Target{FLinearColor::Transparent,5.f,Amber,1.5f};
	FSlateRoundedBoxBrush Chip{Color(15,37,56),10.f,Color(77,126,161),1.f};
	FButtonStyle Primary, Utility;
	FTextBlockStyle ActionText, HeadingText, BodyText, SmallText, CategoryText;
	FVisuals()
	{
		Primary.SetNormal(FSlateRoundedBoxBrush(Mint,9.f))
			.SetHovered(FSlateRoundedBoxBrush(Color(105,249,222),9.f))
			.SetPressed(FSlateRoundedBoxBrush(Color(23,192,169),9.f))
			.SetNormalPadding(FMargin(20,13)).SetPressedPadding(FMargin(20,14,20,12));
		Utility.SetNormal(FSlateRoundedBoxBrush(Color(10,26,39,150),6.f))
			.SetHovered(FSlateRoundedBoxBrush(Color(29,53,70),6.f))
			.SetPressed(FSlateRoundedBoxBrush(Color(9,23,35),6.f));
		ActionText.SetFont(FCoreStyle::GetDefaultFontStyle("Regular",17)).SetColorAndOpacity(White);
		HeadingText=FTextBlockStyle(ActionText).SetFont(FCoreStyle::GetDefaultFontStyle("Bold",34));
		BodyText=FTextBlockStyle(ActionText).SetFont(FCoreStyle::GetDefaultFontStyle("Regular",20));
		SmallText=FTextBlockStyle(ActionText).SetFont(FCoreStyle::GetDefaultFontStyle("Regular",14)).SetColorAndOpacity(Quiet);
		CategoryText=FTextBlockStyle(ActionText).SetColorAndOpacity(Quiet);
	}
};
const FVisuals& Skin() { static const FVisuals Value; return Value; }

// Native inline run; the paragraph still owns wrapping/alignment. Each keyword
// stays together, with dots measured from the same font advances as its text.
class SGuideKeywordText final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGuideKeywordText) {} SLATE_END_ARGS()
	void Construct(const FArguments&, const FText& Text, const FTextBlockStyle& TextStyle)
	{
		Dots=MeasureGlyphDots(Text.ToString(),TextStyle.Font);
		SetVisibility(EVisibility::HitTestInvisible);
		ChildSlot.Padding(FMargin(0,0,0,Dots.ExtraHeight()))
			[SNew(STextBlock).Text(Text).Font(TextStyle.Font).ColorAndOpacity(TextStyle.ColorAndOpacity).SimpleTextMode(true)];
	}
	int32 OnPaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Cull,
		FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool bEnabled) const override
	{
		Layer=SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabled);
		const FSlateRoundedBoxBrush Dot(Skin().White,Dots.Diameter*.5f);
		for (const auto X:Dots.Centers)
			FSlateDrawElement::MakeBox(Out,Layer+1,G.ToPaintGeometry(FVector2D(Dots.Diameter,Dots.Diameter),
				FSlateLayoutTransform(FVector2D(X-Dots.Diameter*.5f,Dots.TextSize.Y+Dots.Gap))),&Dot,
				ESlateDrawEffect::None,Skin().White*Style.GetColorAndOpacityTint());
		return Layer+1;
	}
private:
	FGlyphDots Dots;
};

// Display labels only. The existing lesson state machine still owns every transition.
FText Heading(const FFMCodexGuidedLesson1& L)
{
	using Step=EFMCodexLesson1Step;
	switch(L.GetStep())
	{
	case Step::Intro: return LOCTEXT("TitleIntro","进行一次远射");
	case Step::TacticPointExplanation: return LOCTEXT("TitleTP","观察球员技能");
	case Step::ShootingExplanation: return LOCTEXT("TitleShooting","观察射门属性");
	case Step::TraitExplanation: return LOCTEXT("TitleTrait","远射专家 A");
	case Step::SkillRangeExplanation: return LOCTEXT("TitleRange","远射 · 3–5");
	case Step::FinishExplanation: return LOCTEXT("TitleFinish","球员已经就位");
	case Step::CarrierExplanation: return LOCTEXT("TitleCarrier","选择持球队员");
	case Step::SkillExplanation: return LOCTEXT("TitleSkill","选择远射战术");
	case Step::DirectExplanation: return LOCTEXT("TitleDirect","直接射门");
	case Step::FormulaExplanation: return LOCTEXT("TitleFormula","理解公式");
	case Step::Rewind: return LOCTEXT("TitleReplay","换个人，再试一次");
	case Step::Summary: return LOCTEXT("TitleSummary","选对球员，改变结果");
	case Step::Complete: return LOCTEXT("TitleComplete","教学已完成");
	default: return FText();
	}
}
FText Category(const FFMCodexGuidedLesson1& L)
{
	using Step=EFMCodexLesson1Step;
	switch(L.GetStep())
	{
	case Step::TacticPoint: return LOCTEXT("CategoryTP","进攻战术点");
	case Step::InspectOdegaard: case Step::InspectGyokeres: return LOCTEXT("CategoryInspect","查看球员");
	case Step::Deploy: return L.IsComparison()?LOCTEXT("CategoryCompare","比较球员"):LOCTEXT("CategoryDeploy","部署球员");
	case Step::FinishDeployment: return LOCTEXT("CategoryFinish","结束部署");
	case Step::Carrier: return LOCTEXT("CategoryCarrier","持球队员");
	case Step::Skill: return LOCTEXT("CategorySkill","选择战术");
	case Step::DirectShot: return LOCTEXT("CategoryMethod","选择方式");
	case Step::FormulaHover: case Step::FormulaExplanation: return LOCTEXT("CategoryFormula","公式");
	case Step::AttackRoll: return LOCTEXT("CategoryRoll","进攻判定");
	case Step::ResultReveal: return LOCTEXT("CategoryResult","远射结果");
	case Step::RewindTransition: return LOCTEXT("CategoryReplay","教学重演");
	default: return LOCTEXT("CategoryOpponent","对手行动");
	}
}
class SLessonFocus final : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLessonFocus) {} SLATE_END_ARGS()
	void Construct(const FArguments&, AFMCodexLocalMatchPlayerController* InController)
	{
		Controller=InController;
		SetVisibility(EVisibility::SelfHitTestInvisible);
		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
				.Visibility_Lambda([this](){ return Modal() && !Lesson()->IsFormulaTeaching()?EVisibility::Visible:EVisibility::Collapsed; })
				.OnMouseButtonDown_Lambda([](const FGeometry&,const FPointerEvent&){ return FReply::Handled(); })
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			.Padding(TAttribute<FMargin>::CreateLambda([this](){ return ExplanationPadding(); }))
			[
				SNew(SBox).WidthOverride_Lambda([this](){ return FMath::Clamp(GetCachedGeometry().GetLocalSize().X*.46,560.,660.); })
				.Visibility_Lambda([this](){ return Modal()?EVisibility::SelfHitTestInvisible:EVisibility::Collapsed; })
				[ModalPanel()]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(TAttribute<FMargin>::CreateLambda([this](){ return StripPadding(); }))
			[
				SNew(SBox).Tag(TEXT("LessonActionStrip")).WidthOverride_Lambda([this](){ return StripWidth(); })
				.Visibility_Lambda([this](){ return !Yielding() && !Modal()?EVisibility::SelfHitTestInvisible:EVisibility::Collapsed; })
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f)
						[SNew(SBox).Visibility_Lambda([this](){ return !Modal()?EVisibility::SelfHitTestInvisible:EVisibility::Hidden; })[StepStrip()]]
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top).Padding(8,0,0,0)
					[
						ExitButton()
					]
				]
			]
			+ SOverlay::Slot().Expose(ProxySlot).HAlign(HAlign_Left).VAlign(VAlign_Top)
			[
				SAssignNew(ProxyBox,SBox).Tag(TEXT("LessonOpponentProxy"))
				.Visibility(EVisibility::Collapsed)
			]
			+ SOverlay::Slot()
			[
				SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FLinearColor(0,0,0,.62f))
				.Visibility_Lambda([this](){ return Confirming()?EVisibility::Visible:EVisibility::Collapsed; })
				.OnMouseButtonDown_Lambda([](const FGeometry&,const FPointerEvent&){ return FReply::Handled(); })
				.HAlign(HAlign_Center).VAlign(VAlign_Center)[ExitPanel()]
			]

		];
	}
	~SLessonFocus() { ClearProxy(); RestoreStatus(); }
	void Tick(const FGeometry& G,const double Now,const float Delta) override
	{
		SCompoundWidget::Tick(G,Now,Delta);
		VisualSeconds+=Delta;
		auto* L=Lesson();
		auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
		UpdateProxy(G,Delta);
		// Motion changes during Tick, after Slate's bound-attribute update. Apply the
		// proxy layout together now, so card, spotlight and pointer paint the same frame.
		ProxyBox->SetVisibility(ProxyVisible()?EVisibility::HitTestInvisible:EVisibility::Collapsed);
		if(ProxyVisible())
		{
			ProxySlot->SetPadding(FMargin(ProxyRect.Left,ProxyRect.Top,0,0));
			ProxyBox->SetWidthOverride(ProxyRect.Right-ProxyRect.Left);
			ProxyBox->SetHeightOverride(ProxyRect.Bottom-ProxyRect.Top);
			ProxyBox->SlatePrepass(G.GetAccumulatedLayoutTransform().GetScale());
		}
		// Trace the exact old visibility race without delaying either production reveal or Lesson polling.
		if(L && S && L->GetStep()==EFMCodexLesson1Step::AttackRoll
			&& Controller->GetInteractionView().InteractionCategory==EFMCodexLocalMatchInteractionCategory::RollLongShotDirectDefense
			&& !S->IsInlineFormulaRevealInputBlocked() && S->GetTacticalScene().Phase==FMCodexTacticalScene::EPhase::FormulaHold)
		{
			if(!bHandoffLogged) UE_LOG(LogTemp,Display,TEXT("LESSON1_ROLL_HANDOFF stale=AttackRoll current=Defense reveal=settled scene=FormulaHold tutorialYield=%d strip=collapsed"),Yielding());
			bHandoffLogged=true;
		}
		else bHandoffLogged=false;
		UWidget* Desired=nullptr;
		if(L && S && !Modal() && !S->IsInlineFormulaRevealInputBlocked()
			&& (L->FocusTarget()==EFMCodexLesson1Focus::TacticPoint || L->FocusTarget()==EFMCodexLesson1Focus::FinishDeployment))
			if(auto* P=S->GetInteractionPanel()) Desired=P->GetWidgetFromName(TEXT("InteractionBoundedFallback"));
		if(MutedStatus.Get()!=Desired)
		{
			RestoreStatus();
			if(Desired) { MutedStatus=Desired; StatusOpacity=Desired->GetRenderOpacity(); Desired->SetRenderOpacity(0.f); }
		}
		Invalidate(EInvalidateWidgetReason::Paint);
	}
	int32 OnPaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Cull,
		FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool bEnabled) const override
	{
		auto* L=Lesson();
		auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
		if(!L || !S || Yielding() || Confirming()) return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabled);
		const FVector2D Size=G.GetLocalSize();
		TArray<FSlateRect> Holes;
		FGeometry Desktop=G;
		Desktop.AppendTransform(FSlateLayoutTransform(Args.GetWindowToDesktopTransform()));
		auto Shape=[&](UWidget* W)
		{
			if (L->KeepsFullCardOpen()) return ETargetShape::FullCardRow;
			if (Cast<UFMCodexPlayerCardWidget>(W)) return ETargetShape::Card;
			if (Cast<UFMCodexPitchSlotWidget>(W)) return ETargetShape::DeploymentSlot;
			if (L->FocusTarget()==EFMCodexLesson1Focus::FormulaValue) return ETargetShape::FormulaValue;
			return ETargetShape::Button;
		};
		if(!S->IsInlineFormulaRevealInputBlocked())
			for(auto* W:Targets(S,*L))
			{
				const auto R=TargetRect(W,Desktop,Shape(W));
				if(R.IsSet()) Holes.Add(R.GetValue());
			}
		if(ProxyVisible())
		{
			const auto Painted=ConvertTargetGeometry(FindChildGeometry(G,ProxyBox.ToSharedRef()),G,ETargetShape::Content);
			if(Painted.IsSet())
			{
				Holes.Add(Painted->ExtendBy(FMargin(3.f)));
				if(bProxyAtDestination && FMath::IsNearlyEqual(Painted->Left,ProxyTo.Left,1.f)
					&& FMath::IsNearlyEqual(Painted->Top,ProxyTo.Top,1.f)) bProxyArrivalPainted=true;
			}
		}
		const int32 PointerCount=Holes.Num();
		if(L->FocusTarget()==EFMCodexLesson1Focus::DirectShot)
			if(const auto R=TargetRect(S->GetWidgetFromName(TEXT("TheaterNearDirectHint")),Desktop,ETargetShape::Content); R.IsSet()) Holes.Add(R.GetValue());
		const auto Detail=S->IsDetailOverlayVisible()?TargetRect(S->GetDetailOverlayCard(),Desktop,ETargetShape::Content):TOptional<FSlateRect>();
		if(Detail.IsSet()) Holes.Add(Detail.GetValue());
		auto DrawBox=[&](const FSlateRect& R,const FSlateBrush* Brush,FLinearColor Color,int32 Z)
		{
			FSlateBrush Painted=*Brush;
			Painted.OutlineSettings.Color=Brush->OutlineSettings.Color.GetColor(Style)*Color;
			FSlateDrawElement::MakeBox(Out,Z,G.ToPaintGeometry(FVector2D(R.Right-R.Left,R.Bottom-R.Top),
				FSlateLayoutTransform(FVector2D(R.Left,R.Top))),&Painted,ESlateDrawEffect::None,Color*Brush->GetTint(Style));
		};
		const bool bDim=Modal() || (!Holes.IsEmpty() && !S->IsInlineFormulaRevealInputBlocked())
			|| L->GetStep()==EFMCodexLesson1Step::RewindTransition;
		if(bDim)
		{
			// Visual spotlight subtraction only. No coordinate-based input or gameplay gate.
			TArray<float> X{0.f,static_cast<float>(Size.X)},Y{0.f,static_cast<float>(Size.Y)};
			for(const auto& R:Holes) { X.Add(FMath::Clamp(R.Left,0.f,static_cast<float>(Size.X))); X.Add(FMath::Clamp(R.Right,0.f,static_cast<float>(Size.X))); Y.Add(FMath::Clamp(R.Top,0.f,static_cast<float>(Size.Y))); Y.Add(FMath::Clamp(R.Bottom,0.f,static_cast<float>(Size.Y))); }
			X.Sort(); Y.Sort();
			for(int32 I=1;I<X.Num();++I) for(int32 J=1;J<Y.Num();++J)
			{
				const FVector2D Mid((X[I-1]+X[I])*.5f,(Y[J-1]+Y[J])*.5f);
				if(!Holes.ContainsByPredicate([&](const FSlateRect& R){ return R.ContainsPoint(Mid); }))
					DrawBox(FSlateRect(X[I-1],Y[J-1],X[I],Y[J]),FCoreStyle::Get().GetBrush("WhiteBrush"),
						FLinearColor(.002f,.008f,.018f,Modal()?.60f:.43f),Layer);
			}
		}
		++Layer;
		const float Pulse=.5f+.5f*FMath::Sin(VisualSeconds*3.49f);
		for(int32 I=0;I<PointerCount;++I)
		{
			const auto& R=Holes[I];
			if(Detail.IsSet())
			{
				const auto D=Detail.GetValue();
				if(!L->KeepsFullCardOpen() && R.Left<D.Right && R.Right>D.Left && R.Top<D.Bottom && R.Bottom>D.Top) continue;
			}
			DrawBox(R,&Skin().Target,FLinearColor(1,1,1,.72f+.22f*Pulse),Layer+1);
			if(S->IsDetailOverlayVisible()) continue; // Keep real card detail unobstructed.
			const float CenterX=(R.Left+R.Right)*.5f;
			const bool bOpponent=L->IsOpponentPresenting();
			const float ArrowRoom=bOpponent?96.f:48.f;
			const bool PreferSide=L->IsOpponentPresenting() || L->FocusTarget()==EFMCodexLesson1Focus::HandCard
				|| L->FocusTarget()==EFMCodexLesson1Focus::Carrier || L->FocusTarget()==EFMCodexLesson1Focus::Deployment;
			const bool Above=R.Top>ArrowRoom && !PreferSide;
			const float Bob=3.f*Pulse;
			TArray<FVector2D> Arrow;
			if(Above)
			{
				const FVector2D Tip(CenterX,R.Top-12-Bob);
				Arrow={Tip+FVector2D(-15,-15),Tip,Tip+FVector2D(15,-15)};
			}
			else if (Size.X-R.Right>ArrowRoom || R.Left>ArrowRoom)
			{
				const bool FromRight=Size.X-R.Right>ArrowRoom;
				const float Sign=FromRight?1.f:-1.f;
				const FVector2D Tip(FromRight?R.Right+12+Bob:R.Left-12-Bob,(R.Top+R.Bottom)*.5f);
				Arrow={Tip+FVector2D(Sign*15,-15),Tip,Tip+FVector2D(Sign*15,15)};
			}
			else if (Size.Y-R.Bottom>ArrowRoom)
			{
				const FVector2D Tip(CenterX,R.Bottom+12+Bob);
				Arrow={Tip+FVector2D(-15,15),Tip,Tip+FVector2D(15,15)};
			}
			if (Arrow.IsEmpty()) continue;
			FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Arrow,ESlateDrawEffect::None,Skin().Amber,true,4.f);
			if(bOpponent)
			{
				const FVector2D Tip=Arrow[1];
				const bool FromRight=Tip.X>R.Right;
				const float Left=FMath::Clamp(FromRight?Tip.X+22.f:Tip.X-70.f,4.f,static_cast<float>(Size.X)-52.f);
				const float Top=FMath::Clamp(Tip.Y-14.f,4.f,static_cast<float>(Size.Y)-32.f);
				const FSlateRoundedBoxBrush Label(FVisuals::Color(15,37,56),5.f,Skin().Amber,1.f);
				DrawBox(FSlateRect(Left,Top,Left+48,Top+28),&Label,FLinearColor::White,Layer+1);
				FSlateDrawElement::MakeText(Out,Layer+2,G.ToPaintGeometry(FVector2D(48,28),FSlateLayoutTransform(FVector2D(Left+9,Top+5))),
					LOCTEXT("OpponentPointer","对手"),FCoreStyle::GetDefaultFontStyle("Regular",12),ESlateDrawEffect::None,Skin().Quiet);
			}
			if(PointerCount==1 && Above && (L->FocusTarget()==EFMCodexLesson1Focus::TacticPoint || L->FocusTarget()==EFMCodexLesson1Focus::FinishDeployment))
			{
				const FString Copy=L->FocusTarget()==EFMCodexLesson1Focus::TacticPoint
					? LOCTEXT("PointerTP","掷出本回合的战术点").ToString():LOCTEXT("PointerFinish","确认球员已经就位").ToString();
				const float W=300.f,H=52.f;
				const float Left=FMath::Clamp(CenterX-W*.5f,16.f,static_cast<float>(Size.X)-W-16.f);
				const FSlateRect Bubble(Left,R.Top-94,Left+W,R.Top-94+H);
				DrawBox(Bubble,&Skin().Chip,FLinearColor::White,Layer+1);
				FSlateDrawElement::MakeText(Out,Layer+2,G.ToPaintGeometry(FVector2D(W,H),
					FSlateLayoutTransform(FVector2D(Left+24,Bubble.Top+13))),Copy,
					FCoreStyle::GetDefaultFontStyle("Regular",19),ESlateDrawEffect::None,Skin().White);
			}
		}
		return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+3,Style,bEnabled);
	}
private:
	FFMCodexGuidedLesson1* Lesson() const { return Controller.IsValid()?Controller->GetGuidedLesson1():nullptr; }
	bool Yielding() const
	{
	 auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
	 return Lesson() && S && Lesson()->YieldsToProduction(S->GetTacticalScene(),S->IsInlineFormulaRevealInputBlocked(),Controller->GetInteractionView().InteractionCategory);
	}
	bool Modal() const { return !Yielding() && Lesson() && Lesson()->IsExplanationMode(); }
	bool Confirming() const { return Lesson() && Lesson()->IsExitConfirmationOpen(); }
	bool ProxyVisible() const { return ProxyCard.IsValid() && Lesson() && Lesson()->IsOpponentDeploymentMoving() && !Yielding() && !Confirming(); }
	void ClearProxy()
	{
		if(MutedSource.IsValid()) MutedSource->SetRenderOpacity(SourceOpacity);
		MutedSource.Reset();
		if(ProxyBox.IsValid()) { ProxyBox->SetVisibility(EVisibility::Collapsed); ProxyBox->SetContent(SNullWidget::NullWidget); }
		ProxyCard.Reset(); ProxySeconds=0.f; bProxyAtDestination=false; bProxyArrivalPainted=false;
	}
	void UpdateProxy(const FGeometry& G,float Delta)
	{
		auto* L=Lesson(); auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
		if(!L || !S || L->GetStep()!=EFMCodexLesson1Step::OpponentDeploy || L->IsOpponentSettling())
		{
			if(ProxyCard.IsValid()) ClearProxy();
			return;
		}
		if(Confirming() || Yielding()) return;
		if(!ProxyCard.IsValid())
		{
			if(!L->CanStartOpponentDeploymentMove()) return;
			UFMCodexPlayerCardWidget* Source=nullptr; UFMCodexPitchSlotWidget* Destination=nullptr;
			if(auto* Rack=S->GetOpponentRackWidget())
				for(UFMCodexPlayerCardWidget* Card:Rack->GetRenderedCardWidgets()) if(Card->GetPresentation().CardId==L->Stones()) Source=Card;
			if(auto* Pitch=S->GetPitchWidget())
				for(UFMCodexPitchSlotWidget* Slot:Pitch->GetRenderedSlotWidgets()) if(Slot->GetPresentation().SlotId==L->DefenderSlot()) Destination=Slot;
			const auto From=TargetRect(Source,G,ETargetShape::Content), To=TargetRect(Destination,G,ETargetShape::Content);
			if(!From.IsSet() || !To.IsSet()) return; // Wait for real arranged geometry, never teleport.
			ProxyFrom=*From; ProxyTo=*To; ProxyRect=ProxyFrom;
			const float Height=(ProxyTo.Right-ProxyTo.Left)*(ProxyFrom.Bottom-ProxyFrom.Top)/(ProxyFrom.Right-ProxyFrom.Left);
			const float CenterY=(ProxyTo.Top+ProxyTo.Bottom)*.5f;
			ProxyTo.Top=CenterY-Height*.5f; ProxyTo.Bottom=CenterY+Height*.5f;
			ProxyCard.Reset(NewObject<UFMCodexPlayerCardWidget>(S));
			ProxyCard->RefreshFromPresentation(Source->GetPresentation(),EFMCodexPlayerCardPresentationMode::HandMicro);
			ProxyBox->SetContent(SNew(SScaleBox).Stretch(EStretch::ScaleToFit)[ProxyCard->TakeWidget()]);
			MutedSource=Source; SourceOpacity=Source->GetRenderOpacity(); Source->SetRenderOpacity(0.f);
			L->BeginOpponentDeploymentMove();
			UE_LOG(LogTemp,Display,TEXT("LESSON1_PROXY begin comparison=%d duration=%.2f source=(%.1f,%.1f) destination=(%.1f,%.1f)"),L->IsComparison(),L->OpponentDeployMove*L->OpponentPace(),ProxyFrom.Left,ProxyFrom.Top,ProxyTo.Left,ProxyTo.Top);
			return;
		}
		if(bProxyArrivalPainted)
		{
			if(!L->IsOpponentActionDue())
			{
				L->CompleteOpponentDeploymentMove(); // Notification only; controller submits the original typed command once.
				UE_LOG(LogTemp,Display,TEXT("LESSON1_PROXY paintedArrival elapsed=%.3f commandPending=1"),ProxySeconds);
			}
			return;
		}
		ProxySeconds+=FMath::Max(0.f,Delta);
		const float T=FMath::Clamp(ProxySeconds/(L->OpponentDeployMove*L->OpponentPace()),0.f,1.f);
		const float Ease=T*T*(3.f-2.f*T);
		ProxyRect=FSlateRect(FMath::Lerp(ProxyFrom.Left,ProxyTo.Left,Ease),FMath::Lerp(ProxyFrom.Top,ProxyTo.Top,Ease),
			FMath::Lerp(ProxyFrom.Right,ProxyTo.Right,Ease),FMath::Lerp(ProxyFrom.Bottom,ProxyTo.Bottom,Ease));
		bProxyAtDestination=T>=1.f;
	}
	TOptional<FSlateRect> PlacementRect(UWidget* W, const FGeometry& G) const
	{
		// Layout attributes must not recursively arrange their own window. Last layout is
		// sufficient for panel placement; painted focus frames use the fresh visible path.
		return W && W->IsVisible()?ConvertTargetGeometry(W->GetTickSpaceGeometry(),G,ETargetShape::Content):TOptional<FSlateRect>();
	}
	FMargin ExplanationPadding() const
	{
		const auto& G=GetTickSpaceGeometry();
		auto* L=Lesson(); auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
		if (!L || !S) return FMargin(0);
		// Slate centered slots use PreMargin-PostMargin as their offset. Zero total
		// margin moves the center without shrinking the allotted content area.
		auto CenterOffset=[](FVector2D Offset){return FMargin(Offset.X*.5,Offset.Y*.5,-Offset.X*.5,-Offset.Y*.5);};
		if (L->KeepsFullCardOpen())
			if (const auto Card=PlacementRect(S->GetDetailOverlayCard(),G); Card.IsSet())
			{
				if(L->GetStep()==EFMCodexLesson1Step::SkillRangeExplanation && ExplanationPanel.IsValid())
				{
					const auto Pitch=PlacementRect(S->GetPitchWidget(),G), Header=PlacementRect(S->GetMatchHeader(),G);
					const FVector2D Size=G.GetLocalSize(), Panel=ExplanationPanel->GetDesiredSize();
					const float Left=Card->Right+24.f, Right=Size.X-24.f;
					const float PitchCenter=Pitch.IsSet()?(Pitch->Left+Pitch->Right)*.5f:Size.X*.5f;
					const float X=FMath::Clamp(FMath::Lerp((Left+Right)*.5f,PitchCenter,.35f),Left+Panel.X*.5f,FMath::Max(Left+Panel.X*.5f,Right-Panel.X*.5f));
					const float Top=Header.IsSet()?Header->Bottom+16.f:16.f, Bottom=Pitch.IsSet()?Pitch->Bottom-16.f:Size.Y-16.f;
					const float Y=FMath::Clamp(static_cast<float>(Size.Y)*.55f,Top+Panel.Y*.5f,FMath::Max(Top+Panel.Y*.5f,Bottom-Panel.Y*.5f));
					return CenterOffset(FVector2D(X-Size.X*.5f,Y-Size.Y*.5f));
				}
				return CenterOffset(FVector2D(Card->Right*.5,0));
			}
		if (L->GetStep()==EFMCodexLesson1Step::FormulaExplanation)
		{
			const auto Subtitle=PlacementRect(S->GetWidgetFromName(TEXT("TheaterSubtitle")),G);
			const auto Formula=PlacementRect(S->GetWidgetFromName(TEXT("TheaterAttackPanelBounds")),G);
			if (Subtitle.IsSet() && Formula.IsSet() && ExplanationPanel.IsValid())
			{
				const float Height=ExplanationPanel->GetDesiredSize().Y;
				const float Center=FMath::Min((Subtitle->Bottom+Formula->Top)*.5f,Formula->Top-12.f-Height*.5f);
				return CenterOffset(FVector2D(0,Center-G.GetLocalSize().Y*.5));
			}
		}
		return FMargin(0);
	}
	double StripWidth() const { return FMath::Min(800., FMath::Max(0., GetCachedGeometry().GetLocalSize().X-48.)); }
	FMargin StripPadding() const
	{
		const auto& G=GetTickSpaceGeometry();
		float Center=G.GetLocalSize().X*.5f, Top=132.f;
		if (auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr)
		{
			if (const auto H=PlacementRect(S->GetMatchHeader(),G); H.IsSet()) Top=H->Bottom+10.f;
			if (const auto P=PlacementRect(S->GetPitchWidget(),G); P.IsSet()) Center=(P->Left+P->Right)*.5f;
		}
		if (Lesson() && (Lesson()->IsFormulaTeaching() || Lesson()->GetStep()==EFMCodexLesson1Step::AttackRoll))
			if (auto* S=Controller->GetPlayerMatchScreen())
				if (const auto H=PlacementRect(S->GetWidgetFromName(TEXT("TheaterSubtitle")),G); H.IsSet()) Top=H->Bottom+8.f;
		return FMargin(FMath::Clamp(Center-StripWidth()*.5,24.,FMath::Max(24.,G.GetLocalSize().X-StripWidth()-24.)),Top,0,0);
	}
	TSharedRef<SWidget> ExitPanel()
	{
		return SNew(SBox).WidthOverride(470)
		[SNew(SBorder).BorderImage(&Skin().Panel).Padding(28)
			[SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,16)
			[SNew(STextBlock).Text(LOCTEXT("ExitTitle","退出教学？")).Font(FCoreStyle::GetDefaultFontStyle("Bold",26)).ColorAndOpacity(Skin().White)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,24)
			[SNew(STextBlock).Text(LOCTEXT("ExitBody","退出后将结束当前教学流程。")).Font(FCoreStyle::GetDefaultFontStyle("Regular",18)).ColorAndOpacity(Skin().Quiet)]
			+ SVerticalBox::Slot().AutoHeight()
			[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.f).Padding(0,0,12,0)
				[SAssignNew(ContinueTeachingButton,SButton).Tag(TEXT("LessonExitCancel")).ButtonStyle(&Skin().Primary)
					.OnClicked_Lambda([this](){ if(Lesson()) Lesson()->CancelExitConfirmation(); return FReply::Handled(); })
					[SNew(STextBlock).Text(LOCTEXT("Stay","继续教学")).Font(FCoreStyle::GetDefaultFontStyle("Bold",18)).ColorAndOpacity(FVisuals::Color(4,30,39))]]
				+ SHorizontalBox::Slot().AutoWidth()
				[SNew(SButton).Tag(TEXT("LessonExitConfirm")).ButtonStyle(&Skin().Utility).ContentPadding(16)
					.OnClicked_Lambda([this](){ if(Lesson() && Lesson()->ConfirmExit()) Controller->ExitGuidedLesson1(); return FReply::Handled(); })
					[SNew(STextBlock).Text(LOCTEXT("Leave","退出教学")).Font(FCoreStyle::GetDefaultFontStyle("Regular",16)).ColorAndOpacity(Skin().Quiet)]]
			]
		]];
	}
	TSharedRef<SWidget> ExitButton()
	{
		return SNew(SButton).Tag(TEXT("LessonExit")).ButtonStyle(&Skin().Utility).ContentPadding(FMargin(12,8))
			.IsEnabled_Lambda([this](){ return Lesson() && !Confirming(); })
			.OnClicked_Lambda([this](){ if(Lesson()) Lesson()->RequestExitConfirmation(); return FReply::Handled().SetUserFocus(ContinueTeachingButton.ToSharedRef()); })
			[SNew(STextBlock).Text(LOCTEXT("Exit","退出教学")).Font(FCoreStyle::GetDefaultFontStyle("Regular",12)).ColorAndOpacity(Skin().Quiet)];
	}
	TSharedRef<SWidget> TeachingText(TAttribute<FText> Copy, const FTextBlockStyle& TextStyle,
		EFMCodexLesson1CopySurface Surface=EFMCodexLesson1CopySurface::Body, bool bWrap=true)
	{
		// Native rich text only supplies inline placement; our small primitive owns glyph dots.
		auto Decorator=FWidgetDecorator::Create(TEXT("concept"),FWidgetDecorator::FCreateWidget::CreateLambda(
			[TextStyle](const FTextRunInfo& Run, const ISlateStyle*)
			{
				const auto Word=SNew(SGuideKeywordText,Run.Content,TextStyle);
				const auto Dots=MeasureGlyphDots(Run.Content.ToString(),TextStyle.Font);
				const auto Baseline=static_cast<int16>(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->GetBaseline(TextStyle.Font)-FMath::CeilToInt(Dots.ExtraHeight()));
				return FSlateWidgetRun::FWidgetRunInfo(Word,Baseline);
			}));
		return SNew(SRichTextBlock).Text_Lambda([this,Copy,Surface](){ return Lesson()?Lesson()->EmphasizeKeywords(Copy.Get(),Surface):Copy.Get(); })
			.TextStyle(&TextStyle).AutoWrapText(bWrap).LineHeightPercentage(1.12f)
			+ SRichTextBlock::Decorator(Decorator);
	}

	void RestoreStatus()
	{
		if(MutedStatus.IsValid()) MutedStatus->SetRenderOpacity(StatusOpacity);
		MutedStatus.Reset();
	}
	TSharedRef<SWidget> RuleLine()
	{
		return SNew(SBox).HeightOverride(1.f)
			[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FVisuals::Color(48,83,106))];
	}
	TSharedRef<SWidget> ModalPanel()
	{
		return SAssignNew(ExplanationPanel,SBorder).Tag(TEXT("LessonModalPanel")).BorderImage(&Skin().Panel).Padding(2.f)
		[
			SNew(SBorder).BorderImage(&Skin().Inner).Padding(FMargin(32,0,32,26))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0,0,0,28)
				[SNew(SBox).WidthOverride(58).HeightOverride(4)
					[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Skin().Amber)]]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)
				[SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
					[SNew(STextBlock).Text_Lambda([this](){ return Lesson() && Lesson()->IsComparison()
						?LOCTEXT("EyebrowCompare","教学 · 换个人试试"):LOCTEXT("Eyebrow","教学 · 进攻入门"); })
						.Font(FCoreStyle::GetDefaultFontStyle("Regular",16)).ColorAndOpacity(Skin().Quiet)]
					+ SHorizontalBox::Slot().AutoWidth()[ExitButton()]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)
				[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Heading(*Lesson()):FText(); }),Skin().HeadingText,EFMCodexLesson1CopySurface::Heading)]
				+ SVerticalBox::Slot().AutoHeight()
				[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Lesson()->Instruction():FText(); }),Skin().BodyText)]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,14,0,0)
				[SNew(SBox).Visibility_Lambda([this](){ return Lesson() && !Lesson()->Explanation().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed; })
					[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Lesson()->Explanation():FText(); }),Skin().SmallText,EFMCodexLesson1CopySurface::Secondary)]]

				+ SVerticalBox::Slot().AutoHeight().Padding(0,24,0,20)[RuleLine()]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SNew(SButton).Tag(TEXT("LessonExplanationContinue")).ButtonStyle(&Skin().Primary)
					.OnClicked_Lambda([this](){ if(Controller.IsValid()) Controller->GuidedLesson1Primary(); return FReply::Handled(); })
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(22,0,28,0)
						[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Lesson()->PrimaryLabel():FText(); })
							.Font(FCoreStyle::GetDefaultFontStyle("Bold",20)).ColorAndOpacity(FVisuals::Color(4,30,39))]
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[SNew(STextBlock).Text(FText::FromString(TEXT("›"))).Font(FCoreStyle::GetDefaultFontStyle("Regular",27)).ColorAndOpacity(FVisuals::Color(4,30,39))]
					]
				]
			]
		];
	}
	TSharedRef<SWidget> StepStrip()
	{
		return SNew(SBorder).BorderImage(&Skin().Panel).Padding(FMargin(16,10))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,12,0)
				[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Category(*Lesson()):FText(); }),Skin().CategoryText,EFMCodexLesson1CopySurface::Section,false)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,2,12,2)
				[SNew(SBox).WidthOverride(1.f)
					[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Skin().Amber)]]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Lesson()->Instruction():FText(); }),Skin().ActionText)]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,10,0,0)
			[SNew(SBox).Visibility_Lambda([this](){ return Lesson() && !Lesson()->Explanation().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed; })
				[TeachingText(TAttribute<FText>::CreateLambda([this](){ return Lesson()?Lesson()->Explanation():FText(); }),Skin().SmallText,EFMCodexLesson1CopySurface::Secondary)]]
		];
	}
	TSharedPtr<SButton> ContinueTeachingButton;
	TSharedPtr<SBorder> ExplanationPanel;
	TSharedPtr<SBox> ProxyBox;
	SOverlay::FOverlaySlot* ProxySlot=nullptr;
	TStrongObjectPtr<UFMCodexPlayerCardWidget> ProxyCard;
	TWeakObjectPtr<UFMCodexPlayerCardWidget> MutedSource;
	FSlateRect ProxyFrom{0,0,0,0}, ProxyTo{0,0,0,0}, ProxyRect{0,0,0,0};
	float SourceOpacity=1.f, ProxySeconds=0.f;
	bool bProxyAtDestination=false, bHandoffLogged=false;
	mutable bool bProxyArrivalPainted=false;
	TWeakObjectPtr<AFMCodexLocalMatchPlayerController> Controller;
	TWeakObjectPtr<UWidget> MutedStatus;
	float StatusOpacity=1.f;
	float VisualSeconds=0.f;
};
TSharedRef<SWidget> Build(AFMCodexLocalMatchPlayerController* C) { return SNew(SLessonFocus,C); }
}
#undef LOCTEXT_NAMESPACE
#endif
