#include "FMCodexMatchFlowPanel.h"

#include "Components/BorderSlot.h"
#include "Components/ButtonSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SLeafWidget.h"

namespace
{
// Shared paint vocabulary for the two real consumers, A and C. No texture assets.
struct FFlowPaint
{
	const FGeometry& G;
	FSlateWindowElementList& Out;
	int32 Layer;
	FLinearColor Tint;
	void Line(const TArray<FVector2f>& P, FLinearColor Color, float Width = 1.f) const
	{
		FSlateDrawElement::MakeLines(Out, Layer+1, G.ToPaintGeometry(), P,
			ESlateDrawEffect::None, Color*Tint, true, Width);
	}
	TArray<FVector2f> Ring(float Inset, float Cut) const
	{
		const float W=G.GetLocalSize().X, H=G.GetLocalSize().Y;
		return {{Cut,Inset},{W-Cut,Inset},{W-Inset,Cut},{W-Inset,H-Cut},
			{W-Cut,H-Inset},{Cut,H-Inset},{Inset,H-Cut},{Inset,Cut}};
	}
	void Outline(float Inset, float Cut, FLinearColor Color, float Width=1.f) const
	{
		auto P=Ring(Inset,Cut); const auto Start=P[0]; P.Add(Start); Line(P,Color,Width);
	}
	void Fill(float Cut, FLinearColor Top, FLinearColor Bottom) const
	{
		const auto P=Ring(1,Cut);
		TArray<FSlateVertex> V; TArray<SlateIndex> Indices;
		for (int32 I=0; I<P.Num(); ++I)
		{
			const auto Color=FMath::Lerp(Top,Bottom,P[I].Y/G.GetLocalSize().Y)*Tint;
			V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),
				P[I],FVector2f::ZeroVector,Color.ToFColor(true)));
			if (I>=2) {Indices.Add(0); Indices.Add(I-1); Indices.Add(I);}
		}
		const auto Resource=FSlateApplication::Get().GetRenderer()->GetResourceHandle(*FCoreStyle::Get().GetBrush("WhiteBrush"));
		FSlateDrawElement::MakeCustomVerts(Out,Layer,Resource,V,Indices,nullptr,0,0);
	}
	void Hatching(float Extent) const
	{
		const float W=G.GetLocalSize().X, H=G.GetLocalSize().Y;
		for (float Y=22; Y<H-24; Y+=13)
		{
			const float Run=FMath::Min(Extent,Y-12);
			Line({{12,Y},{12+Run,Y-Run}},FLinearColor(.07f,.22f,.33f,.13f),.65f);
			const float RightRun=FMath::Min(Extent,H-12-Y);
			Line({{W-12,Y},{W-12-RightRun,Y+RightRun}},FLinearColor(.07f,.22f,.33f,.13f),.65f);
		}
	}
};

class SFMCodexMatchFlowPanel final : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SFMCodexMatchFlowPanel) {} SLATE_END_ARGS()
	void Construct(const FArguments&, UFMCodexMatchFlowPanel* InOwner)
	{
		Owner = InOwner;
		SBorder::Construct(SBorder::FArguments());
	}
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G,
		const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& Style, bool bEnabled) const override
	{
		if (!Owner.IsValid() || !Owner->IsFlowStyleEnabled())
			return SBorder::OnPaint(Args, G, Cull, Out, Layer, Style, bEnabled);
		const FVector2f Size(G.GetLocalSize());
		const float W = Size.X, H = Size.Y;
		if (W < 32 || H < 32) return Layer;
		const FFlowPaint P{G,Out,Layer,Style.GetColorAndOpacityTint()};
		if (Owner->IsRuleCard())
		{
			P.Fill(6,FLinearColor(.009f,.035f,.065f,1),FLinearColor(.003f,.017f,.032f,1));
			P.Outline(1,6,FLinearColor(.035f,.14f,.23f,.9f));
			P.Line({{9,3},{W-9,3}},FLinearColor(.08f,.23f,.32f,.32f));
			P.Line({{80,13},{80,H-13}},FLinearColor(.08f,.24f,.34f,.65f));
		}
		else
		{
			P.Fill(18,FLinearColor(.003f,.020f,.044f,1),FLinearColor(.0015f,.010f,.021f,1));
			P.Hatching(76);
			P.Outline(1,18,FLinearColor(.12f,.25f,.35f,.85f),1.5f);
			P.Outline(4,20,FLinearColor(.003f,.009f,.018f,1),2.f);
			P.Outline(7,22,FLinearColor(.025f,.20f,.32f,.85f));
			P.Line({{28,4},{W-28,4}},FLinearColor(.14f,.31f,.41f,.55f));
			for (float X : {3.f,W-3.f})
				for (float Y : {38.f,H-56.f})
				{
					P.Line({{X,Y-3},{X,Y+21}},FLinearColor(.01f,.20f,.29f,.24f),5.f);
					P.Line({{X,Y},{X,Y+18}},FLinearColor(.025f,.43f,.62f,.8f),1.8f);
				}
			P.Line({{W*.5f-28,7},{W*.5f+28,7}},FLinearColor(.06f,.32f,.45f,.7f),1.3f);
			for (float X : {W*.22f,W*.75f})
				for (int32 I=0;I<3;++I)
					P.Line({{X+I*7,34},{X+I*7+5,27}},FLinearColor(.08f,.25f,.37f,.65f),1.4f);
		}
		return SCompoundWidget::OnPaint(Args, G, Cull, Out, Layer+2, Style, bEnabled);
	}
private:
	TWeakObjectPtr<UFMCodexMatchFlowPanel> Owner;
};

class SFMCodexMatchFlowButton final : public SButton
{
public:
	void Construct(const SButton::FArguments& Args, UFMCodexMatchFlowButton* InOwner)
	{
		Owner=InOwner; SButton::Construct(Args);
	}
	virtual int32 OnPaint(const FPaintArgs& Args,const FGeometry& G,const FSlateRect& Cull,
		FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& WidgetStyle,bool bEnabled) const override
	{
		if (!Owner.IsValid() || !Owner->IsFlowStyleEnabled())
			return SButton::OnPaint(Args,G,Cull,Out,Layer,WidgetStyle,bEnabled);
		if (G.GetLocalSize().X<16 || G.GetLocalSize().Y<16) return Layer;
		const bool bActive=ShouldBeEnabled(bEnabled);
		const bool bFocus=HasKeyboardFocus();
		const auto& B=Owner->GetStyle();
		const auto& Brush=!bActive?B.Disabled:IsPressed()?B.Pressed:(IsHovered()||bFocus)?B.Hovered:B.Normal;
		const FFlowPaint P{G,Out,Layer,WidgetStyle.GetColorAndOpacityTint()};
		const auto Fill=Brush.TintColor.GetSpecifiedColor();
		const auto Edge=Brush.OutlineSettings.Color.GetSpecifiedColor();
		const float W=G.GetLocalSize().X,H=G.GetLocalSize().Y;
		P.Fill(9,Fill,Fill*FLinearColor(.55f,.6f,.65f,1));
		P.Outline(1,9,Edge,Brush.OutlineSettings.Width);
		P.Outline(4,11,Edge*FLinearColor(1,1,1,.16f));
		P.Line({{11,2},{29,2}},Edge*FLinearColor(1,1,1,.7f),1.7f);
		P.Line({{W-29,H-2},{W-11,H-2}},Edge*FLinearColor(1,1,1,.7f),1.7f);
		if (H>80) // Decision card cue; the short primary action remains centered.
			P.Line({{W-25,H-28},{W-19,H-22},{W-25,H-16}},Edge,1.8f);
		if (bActive && bFocus)
			P.Line({{8,17},{8,H-17}},B.Hovered.OutlineSettings.Color.GetSpecifiedColor(),2.f);
		// SButton still arranges pressed padding and owns every input delegate.
		// Apply native disabled text treatment once; a second tint obscures the reason.
		return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+2,WidgetStyle,bEnabled);
	}
private:
	TWeakObjectPtr<UFMCodexMatchFlowButton> Owner;
};

class SFMCodexMatchFlowDiagram final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SFMCodexMatchFlowDiagram) {} SLATE_END_ARGS()
	void Construct(const FArguments&, EFMCodexFlowDiagram InDiagram) { Diagram=InDiagram; }
	virtual FVector2D ComputeDesiredSize(float) const override {return UFMCodexMatchFlowDiagram::ViewportSize();}
	virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,
		FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle& Style,bool) const override
	{
		const FFlowPaint P{G,Out,Layer,Style.GetColorAndOpacityTint()};
		const FLinearColor Base(.14f,.36f,.49f,.62f), Mark(.25f,.57f,.72f,.85f);
		// Every pictogram uses a 72x52 viewport, 6px safe area and this exact pitch.
		P.Line({{6,8},{66,8},{66,46},{6,46},{6,8}},Base);
		P.Line({{21,8},{21,24},{51,24},{51,8}},Base);
		P.Line({{28,8},{28,15},{44,15},{44,8}},Base);
		P.Line({{30,8},{30,6},{42,6},{42,8}},Base);
		auto Circle=[&](FVector2f C,float R,FLinearColor Color)
		{
			TArray<FVector2f> Points;
			for(int32 I=0;I<=16;++I) {const float A=I*2*PI/16;Points.Add(C+FVector2f(FMath::Cos(A),FMath::Sin(A))*R);}
			P.Line(Points,Color);
		};
		auto Arrow=[&](FVector2f From,FVector2f To)
		{
			const auto D=(To-From).GetSafeNormal(); const FVector2f N(-D.Y,D.X);
			P.Line({From,To},Mark); P.Line({To-D*4+N*2,To,To-D*4-N*2},Mark);
		};
		FVector2f Ball(36,32);
		switch(Diagram)
		{
		case EFMCodexFlowDiagram::Corner: Ball={9,11}; Arrow({13,13},{28,20}); break;
		case EFMCodexFlowDiagram::LongFreeKick: Ball={36,40}; Arrow({36,35},{36,23}); break;
		case EFMCodexFlowDiagram::ShortFreeKick:
			Ball={30,32}; P.Line({{38,23},{48,23}},Mark); Arrow({30,28},{33,17}); break;
		case EFMCodexFlowDiagram::Penalty: Ball={36,20}; Arrow({36,16},{36,10}); break;
		case EFMCodexFlowDiagram::Combination:
			Ball={18,37}; Circle({47,31},2.5f,Mark); Arrow({23,36},{42,32}); Arrow({47,27},{40,14}); break;
		case EFMCodexFlowDiagram::HighCross:
		case EFMCodexFlowDiagram::LowCross: Ball={10,32}; Arrow({14,30},{35,18}); break;
		case EFMCodexFlowDiagram::Power: Ball={36,39}; Arrow({33,33},{33,17}); Arrow({39,33},{39,17}); break;
		case EFMCodexFlowDiagram::Panenka: Ball={36,21}; Arrow({36,17},{36,11}); break;
		default: Ball={22,37}; Arrow({26,32},{39,14}); break;
		}
		Circle(Ball,2.5f,Mark);
		return Layer+1;
	}
private:
	EFMCodexFlowDiagram Diagram;
};
}

TSharedRef<SWidget> UFMCodexMatchFlowPanel::RebuildWidget()
{
	MyBorder = SNew(SFMCodexMatchFlowPanel, this);
	if (GetChildrenCount() > 0)
		CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
	return MyBorder.ToSharedRef();
}

void UFMCodexMatchFlowPanel::SetFlowStyleEnabled(bool bEnabled)
{
	if (bFlowStyleEnabled == bEnabled) return;
	bFlowStyleEnabled = bEnabled;
	if (MyBorder.IsValid()) MyBorder->Invalidate(EInvalidateWidgetReason::Paint);
}

TSharedRef<SWidget> UFMCodexMatchFlowButton::RebuildWidget()
{
	MyButton=SNew(SFMCodexMatchFlowButton,this)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked,SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate,SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate,SlateHandleReleased))
		.OnHovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate,SlateHandleHovered))
		.OnUnhovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate,SlateHandleUnhovered))
		.ButtonStyle(&GetStyle()).ClickMethod(GetClickMethod()).TouchMethod(GetTouchMethod())
		.PressMethod(GetPressMethod()).IsFocusable(GetIsFocusable());
	if (GetChildrenCount()>0) CastChecked<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	return MyButton.ToSharedRef();
}

void UFMCodexMatchFlowButton::SetFlowStyleEnabled(bool bEnabled)
{
	if (bFlowStyleEnabled==bEnabled) return;
	bFlowStyleEnabled=bEnabled;
	if (MyButton.IsValid()) MyButton->Invalidate(EInvalidateWidgetReason::Paint);
}

TSharedRef<SWidget> UFMCodexMatchFlowDiagram::RebuildWidget()
{
	SetVisibility(ESlateVisibility::HitTestInvisible);
	return SNew(SFMCodexMatchFlowDiagram,Diagram);
}
