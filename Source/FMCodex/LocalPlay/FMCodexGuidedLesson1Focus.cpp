#include "FMCodexGuidedLesson1Focus.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexGuidedLesson1.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPitchWidget.h"
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
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FMCodexLesson1Focus"
namespace FMCodexLesson1Focus
{
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
				if (Card->GetPresentation().CardId == L.Attacker() || (L.IsComparison() && Card->GetPresentation().CardId == L.Gyokeres())) Add(Card);
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
	if (F == Focus::DirectShot) Add(S->GetWidgetFromName(TEXT("TheaterNearDirect")));
	if (F == Focus::AttackRoll) Add(S->GetWidgetFromName(TEXT("TheaterContinue")));
	return Result;
}

// A small lesson-owned skin. No registration or changes to the production UI theme.
struct FVisuals
{
	static FLinearColor Color(uint8 R,uint8 G,uint8 B,uint8 A=255) { return FLinearColor::FromSRGBColor(FColor(R,G,B,A)); }
	const FLinearColor Mint=Color(42,232,197), White=Color(239,247,252), Quiet=Color(139,177,200);
	FSlateRoundedBoxBrush Panel{Color(9,27,43),12.f,Color(111,167,192),1.f};
	FSlateRoundedBoxBrush Inner{FLinearColor::Transparent,10.f,Color(46,76,97),1.f};
	FSlateRoundedBoxBrush Target{FLinearColor::Transparent,12.f,Mint,2.5f};
	FSlateRoundedBoxBrush Glow{FLinearColor::Transparent,15.f,Color(42,232,197,45),6.f};
	FSlateRoundedBoxBrush Chip{Color(15,37,56),10.f,Color(77,126,161),1.f};
	FButtonStyle Primary, Utility;
	FTextBlockStyle ActionText;
	FSlateStyleSet Rich{TEXT("FMCodex.Lesson1.LocalSkin")};
	FVisuals()
	{
		Primary.SetNormal(FSlateRoundedBoxBrush(Mint,9.f))
			.SetHovered(FSlateRoundedBoxBrush(Color(105,249,222),9.f))
			.SetPressed(FSlateRoundedBoxBrush(Color(23,192,169),9.f))
			.SetNormalPadding(FMargin(20,13)).SetPressedPadding(FMargin(20,14,20,12));
		Utility.SetNormal(FSlateRoundedBoxBrush(Color(10,26,39,230),6.f,Color(61,86,105),1.f))
			.SetHovered(FSlateRoundedBoxBrush(Color(29,53,70),6.f,Quiet,1.f))
			.SetPressed(FSlateRoundedBoxBrush(Color(9,23,35),6.f));
		ActionText.SetFont(FCoreStyle::GetDefaultFontStyle("Regular",22)).SetColorAndOpacity(White);
		Rich.Set("em",FTextBlockStyle(ActionText).SetFont(FCoreStyle::GetDefaultFontStyle("Bold",22)).SetColorAndOpacity(Mint));
	}
};
const FVisuals& Skin() { static const FVisuals Value; return Value; }

// Display labels only. The existing lesson state machine still owns every transition.
FText Heading(const FFMCodexGuidedLesson1& L)
{
	using Step=EFMCodexLesson1Step;
	switch(L.GetStep())
	{
	case Step::Intro: return LOCTEXT("TitleIntro","进行一次远射");
	case Step::TacticPointExplanation: return LOCTEXT("TitleTP","观察球员技能");
	case Step::SkillRangeExplanation: return LOCTEXT("TitleRange","远射 · 3–5");
	case Step::FinishExplanation: return LOCTEXT("TitleFinish","球员已经就位");
	case Step::CarrierExplanation: return LOCTEXT("TitleCarrier","选择持球队员");
	case Step::SkillExplanation: return LOCTEXT("TitleSkill","选择远射战术");
	case Step::DirectExplanation: return LOCTEXT("TitleDirect","直接射门");
	case Step::Rewind: return LOCTEXT("TitleReplay","换个人，再试一次");
	case Step::Summary: return LOCTEXT("TitleSummary","选对球员，改变结果");
	case Step::Complete: return LOCTEXT("TitleComplete","第一课已完成");
	default: return FText();
	}
}
FText Category(const FFMCodexGuidedLesson1& L)
{
	using Step=EFMCodexLesson1Step;
	switch(L.GetStep())
	{
	case Step::TacticPoint: return LOCTEXT("CategoryTP","战术点");
	case Step::InspectGyokeres: return LOCTEXT("CategoryInspect","查看球员");
	case Step::Deploy: return L.IsComparison()?LOCTEXT("CategoryCompare","比较球员"):LOCTEXT("CategoryDeploy","部署球员");
	case Step::FinishDeployment: return LOCTEXT("CategoryFinish","结束部署");
	case Step::Carrier: return LOCTEXT("CategoryCarrier","持球队员");
	case Step::Skill: return LOCTEXT("CategorySkill","选择战术");
	case Step::DirectShot: return LOCTEXT("CategoryMethod","选择方式");
	case Step::AttackRoll: return LOCTEXT("CategoryRoll","进攻判定");
	case Step::ResultReveal: return LOCTEXT("CategoryResult","远射结果");
	case Step::RewindTransition: return LOCTEXT("CategoryReplay","教学重演");
	default: return LOCTEXT("CategoryOpponent","对手行动");
	}
}
FText ActionCopy(const FFMCodexGuidedLesson1& L)
{
	FString Text=L.Instruction().ToString();
	// Emphasis is typography only; never infer legality or outcomes from these strings.
	const TCHAR* Word=nullptr;
	switch(L.FocusTarget())
	{
	case EFMCodexLesson1Focus::HandCard: Word=TEXT("哲凯赖什"); break;
	case EFMCodexLesson1Focus::Deployment: if(!L.IsComparison()) Word=TEXT("哲凯赖什"); break;
	case EFMCodexLesson1Focus::Carrier: Word=L.IsComparison()?TEXT("厄德高"):TEXT("哲凯赖什"); break;
	case EFMCodexLesson1Focus::Skill: Word=TEXT("远射"); break;
	case EFMCodexLesson1Focus::DirectShot: Word=TEXT("直接射门"); break;
	default: break;
	}
	if(Word) Text.ReplaceInline(Word,*(FString(TEXT("<em>"))+Word+TEXT("</>")));
	return FText::FromString(Text);
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
				.Visibility_Lambda([this](){ return Modal()?EVisibility::Visible:EVisibility::Collapsed; })
				.OnMouseButtonDown_Lambda([](const FGeometry&,const FPointerEvent&){ return FReply::Handled(); })
			]
			+ SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
			.Padding(TAttribute<FMargin>::CreateLambda([this](){
				return Lesson() && Lesson()->GetStep()==EFMCodexLesson1Step::SkillRangeExplanation?FMargin(210,0,0,0):FMargin(0);
			}))
			[
				SNew(SBox).WidthOverride_Lambda([this](){ return FMath::Clamp(GetCachedGeometry().GetLocalSize().X*.46,560.,660.); })
				.Visibility_Lambda([this](){ return Modal()?EVisibility::SelfHitTestInvisible:EVisibility::Collapsed; })
				[ModalPanel()]
			]
			+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Top)
			.Padding(TAttribute<FMargin>::CreateLambda([this](){ return StripPadding(); }))
			[
				SNew(SBox).WidthOverride_Lambda([this](){
					const FMargin P=StripPadding();
					return FMath::Min(960.,FMath::Max(0.,GetCachedGeometry().GetLocalSize().X-P.Left-P.Right-48.));
				})
				.Visibility_Lambda([this](){ return !Modal()?EVisibility::HitTestInvisible:EVisibility::Collapsed; })
				[StepStrip()]
			]
			+ SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(16.f)
			[
				SNew(SButton).ButtonStyle(&Skin().Utility).ContentPadding(FMargin(16,8))
				.OnClicked_Lambda([this](){ if(Controller.IsValid()) Controller->ExitGuidedLesson1(); return FReply::Handled(); })
				[SNew(STextBlock).Text(LOCTEXT("Exit","退出教学")).Font(FCoreStyle::GetDefaultFontStyle("Regular",12)).ColorAndOpacity(Skin().Quiet)]
			]
		];
	}
	~SLessonFocus() { RestoreStatus(); }
	void Tick(const FGeometry& G,const double Now,const float Delta) override
	{
		SCompoundWidget::Tick(G,Now,Delta);
		VisualSeconds+=Delta;
		auto* L=Lesson();
		auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr;
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
		if(!L || !S) return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabled);
		const FVector2D Size=G.GetLocalSize();
		TArray<FSlateRect> Holes;
		auto Rect=[&](UWidget* W)
		{
			const auto& Geo=W->GetCachedGeometry();
			const auto A=G.AbsoluteToLocal(Geo.LocalToAbsolute(FVector2D::ZeroVector));
			const auto B=G.AbsoluteToLocal(Geo.LocalToAbsolute(Geo.GetLocalSize()));
			return FSlateRect(FMath::Clamp(A.X-7.,0.,Size.X),FMath::Clamp(A.Y-7.,0.,Size.Y),
				FMath::Clamp(B.X+7.,0.,Size.X),FMath::Clamp(B.Y+7.,0.,Size.Y));
		};
		if(!S->IsInlineFormulaRevealInputBlocked())
			for(auto* W:Targets(S,*L))
			{
				const auto R=Rect(W);
				if(R.Right-R.Left>10 && R.Bottom-R.Top>10) Holes.Add(R);
			}
		const int32 PointerCount=Holes.Num();
		if(L->FocusTarget()==EFMCodexLesson1Focus::DirectShot)
			if(auto* Hint=S->GetWidgetFromName(TEXT("TheaterNearDirectHint"))) Holes.Add(Rect(Hint));
		if(S->IsDetailOverlayVisible()) Holes.Add(Rect(S->GetDetailOverlayCard()));
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
			for(const auto& R:Holes) { X.Add(R.Left); X.Add(R.Right); Y.Add(R.Top); Y.Add(R.Bottom); }
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
			if(S->IsDetailOverlayVisible())
			{
				const auto Detail=Rect(S->GetDetailOverlayCard());
				if(R.Left<Detail.Right && R.Right>Detail.Left && R.Top<Detail.Bottom && R.Bottom>Detail.Top) continue;
			}
			const FSlateRect Outer(R.Left-3,R.Top-3,R.Right+3,R.Bottom+3);
			DrawBox(Outer,&Skin().Glow,FLinearColor(1,1,1,.40f+.25f*Pulse),Layer);
			DrawBox(R,&Skin().Target,FLinearColor(1,1,1,.72f+.22f*Pulse),Layer+1);
			if(S->IsDetailOverlayVisible()) continue; // Keep real card detail unobstructed.
			const float CenterX=(R.Left+R.Right)*.5f;
			const bool Above=R.Top>220;
			const float Bob=3.f*Pulse;
			TArray<FVector2D> Arrow;
			if(Above)
			{
				const FVector2D Tip(CenterX,R.Top-12-Bob);
				Arrow={Tip+FVector2D(-15,-15),Tip,Tip+FVector2D(15,-15)};
			}
			else
			{
				const bool FromRight=R.Right+64<Size.X;
				const float Sign=FromRight?1.f:-1.f;
				const FVector2D Tip(FromRight?R.Right+12+Bob:R.Left-12-Bob,(R.Top+R.Bottom)*.5f);
				Arrow={Tip+FVector2D(Sign*15,-15),Tip,Tip+FVector2D(Sign*15,15)};
			}
			FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Arrow,ESlateDrawEffect::None,Skin().Mint,true,5.f);
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
		if(L->GetStep()==EFMCodexLesson1Step::SkillRangeExplanation && S->IsDetailOverlayVisible())
			if(auto* Row=S->GetDetailOverlayCard()->GetWidgetFromName(TEXT("FullCardSkillRow0")))
			{
				const auto R=Rect(Row);
				DrawBox(R,&Skin().Target,FLinearColor::White,Layer+1);
			}
		return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+3,Style,bEnabled);
	}
private:
	FFMCodexGuidedLesson1* Lesson() const { return Controller.IsValid()?Controller->GetGuidedLesson1():nullptr; }
	bool Modal() const { return Lesson() && Lesson()->IsExplanationMode(); }
	FMargin StripPadding() const
	{
		// Keep the real hover card readable without moving or restyling production UI.
		if(auto* S=Controller.IsValid()?Controller->GetPlayerMatchScreen():nullptr)
			if(S->IsDetailOverlayVisible())
			{
				const auto& Card=S->GetDetailOverlayCard()->GetCachedGeometry();
				const auto Right=GetCachedGeometry().AbsoluteToLocal(Card.LocalToAbsolute(Card.GetLocalSize()));
				return FMargin(Right.X+24.f,112.f,24.f,0.f);
			}
		const float Width=GetCachedGeometry().GetLocalSize().X;
		return FMargin(FMath::Max(48.f,(Width-960.f)*.5f),112,24,0);
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
		return SNew(SBorder).BorderImage(&Skin().Panel).Padding(2.f)
		[
			SNew(SBorder).BorderImage(&Skin().Inner).Padding(FMargin(32,0,32,26))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left).Padding(0,0,0,28)
				[SNew(SBox).WidthOverride(58).HeightOverride(4)
					[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(Skin().Mint)]]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)
				[SNew(STextBlock).Text_Lambda([this](){ return Lesson() && Lesson()->IsComparison()
					?LOCTEXT("EyebrowCompare","第一课 · 换个人试试"):LOCTEXT("Eyebrow","第一课 · 进攻入门"); })
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",16)).ColorAndOpacity(Skin().Quiet)]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,18)
				[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Heading(*Lesson()):FText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Bold",34)).AutoWrapText(true).ColorAndOpacity(Skin().White)]
				+ SVerticalBox::Slot().AutoHeight()
				[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Lesson()->Instruction():FText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",20)).AutoWrapText(true).LineHeightPercentage(1.15f).ColorAndOpacity(Skin().White)]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,14,0,0)
				[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Lesson()->Explanation():FText(); })
					.Visibility_Lambda([this](){ return Lesson() && !Lesson()->Explanation().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed; })
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",14)).AutoWrapText(true).ColorAndOpacity(Skin().Quiet)]
				+ SVerticalBox::Slot().AutoHeight().Padding(0,24,0,20)[RuleLine()]
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right)
				[
					SNew(SButton).ButtonStyle(&Skin().Primary)
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
		return SNew(SBorder).BorderImage(&Skin().Panel).Padding(FMargin(26,17))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,22,0)
				[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Category(*Lesson()):FText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Regular",17)).ColorAndOpacity(Skin().Quiet)]
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,2,22,2)
				[SNew(SBox).WidthOverride(1.f)
					[SNew(SBorder).Padding(0).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush")).BorderBackgroundColor(FVisuals::Color(90,143,170))]]
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[SNew(SRichTextBlock).Text_Lambda([this](){ return Lesson()?ActionCopy(*Lesson()):FText(); })
					.TextStyle(&Skin().ActionText).DecoratorStyleSet(&Skin().Rich).AutoWrapText(true)]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,10,0,0)
			[SNew(STextBlock).Text_Lambda([this](){ return Lesson()?Lesson()->Explanation():FText(); })
				.Visibility_Lambda([this](){ return Lesson() && !Lesson()->Explanation().IsEmpty()?EVisibility::HitTestInvisible:EVisibility::Collapsed; })
				.Font(FCoreStyle::GetDefaultFontStyle("Regular",14)).AutoWrapText(true).ColorAndOpacity(Skin().Quiet)]
		];
	}
	TWeakObjectPtr<AFMCodexLocalMatchPlayerController> Controller;
	TWeakObjectPtr<UWidget> MutedStatus;
	float StatusOpacity=1.f;
	float VisualSeconds=0.f;
};
TSharedRef<SWidget> Build(AFMCodexLocalMatchPlayerController* C) { return SNew(SLessonFocus,C); }
}
#undef LOCTEXT_NAMESPACE
#endif
