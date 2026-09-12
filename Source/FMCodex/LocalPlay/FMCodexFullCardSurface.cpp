#include "FMCodexFullCardSurface.h"

#include "Components/BorderSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
class SFMCodexFullCardSurface : public SBorder
{
public:
    SLATE_BEGIN_ARGS(SFMCodexFullCardSurface) {} SLATE_END_ARGS()
    void Construct(const FArguments&, UFMCodexFullCardSurface* InOwner)
    {
        Owner = InOwner;
        SBorder::Construct(SBorder::FArguments());
    }
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G,
        const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
        const FWidgetStyle& Style, bool bEnabled) const override
    {
        using ESurface = EFMCodexFullCardSurface;
        if (!Owner.IsValid() || Owner->GetSurface() == ESurface::None)
            return SBorder::OnPaint(Args, G, Cull, Out, Layer, Style, bEnabled);
        const float W = G.GetLocalSize().X, H = G.GetLocalSize().Y;
        if (W < 2.f || H <= 0.f)
            return SCompoundWidget::OnPaint(Args, G, Cull, Out, Layer, Style, bEnabled);
        const bool bEnabledHere = ShouldBeEnabled(bEnabled);
        const ESlateDrawEffect Effect = bEnabledHere ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
        FLinearColor Tint = Style.GetColorAndOpacityTint();
        // Custom vertices have no disabled draw-effect parameter. Match the quiet disabled content.
        FLinearColor FillTint = Tint;
        if (!bEnabledHere) FillTint.A *= .5f;
        const auto Kind = Owner->GetSurface();
        const FLinearColor Accent = Owner->GetAccent();
        auto Alpha = [](FLinearColor Color, float A) { Color.A = A; return Color; };
        auto Line = [&](TArray<FVector2f> Points, FLinearColor Color, float Width = .65f)
        {
            FSlateDrawElement::MakeLines(Out, Layer + 1, G.ToPaintGeometry(),
                Points, Effect, Color * Tint, true, Width);
        };
        using namespace FMCodexFullCardGeometry;
        const FLinearColor Structure = StructuralInk();
        auto BottomSeam = [&]()
        {
            // Opaque seam bed prevents kit pixels tinting the neutral divider.
            FSlateDrawElement::MakeBox(Out,Layer+1,
                G.ToPaintGeometry(FVector2f(W,2.f),FSlateLayoutTransform(FVector2f(0,H-2.f))),
                FCoreStyle::Get().GetBrush("WhiteBrush"),Effect,FLinearColor(.002f,.008f,.017f,1)*Tint);
            Line({{0,H-.65f},{W,H-.65f}},Alpha(Structure,.64f),DetailStroke);
        };
        if (Kind == ESurface::RuleLeft || Kind == ESurface::RuleRight || Kind == ESurface::Footer)
        {
            if (Kind == ESurface::Footer)
            {
                Line({{6,H*.5f},{30,H*.5f}},Alpha(Structure,.48f),.7f);
                Line({{W-30,H*.5f},{W-6,H*.5f}},Alpha(Structure,.48f),.7f);
            }
            else
            {
                const bool bLeft = Kind == ESurface::RuleLeft;
                Line({{0,H*.5f},{W,H*.5f}},Alpha(Structure,.64f),.75f);
                Line({{bLeft ? W-12 : 0,H*.5f},{bLeft ? W : 12,H*.5f}},Alpha(Structure,.80f),.75f);
            }
            return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+2,Style,bEnabledHere);
        }
        // Thin rules above are valid at one unit; only filled polygons need height.
        if (H < 2.f)
            return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer,Style,bEnabledHere);
        constexpr float I = .65f;
        const bool bRow = Kind == ESurface::AttributeRow || Kind == ESurface::SkillRow;
        const float Cut = FMath::Min3(bRow ? RowCut : PanelCut, W * .15f, H * .34f);
        TArray<FVector2f> Shape = {{I+Cut,I},{W-I-Cut,I},{W-I,I+Cut},
            {W-I,H-I-Cut},{W-I-Cut,H-I},{I+Cut,H-I},{I,H-I-Cut},{I,I+Cut}};
        FLinearColor Top(.007f,.019f,.033f,1), Bottom(.002f,.008f,.016f,1);
        FLinearColor Edge = Alpha(Structure,.30f);
        float Stroke = .65f;
        if (Kind == ESurface::Frame || Kind == ESurface::Body)
        {
            const float Inset = Kind == ESurface::Frame ? OuterInset : 0.f;
            const float Corner = Kind == ESurface::Frame ? OuterCut : InnerCut;
            Shape = {{Inset+Corner,Inset},{W-Inset-Corner,Inset},{W-Inset,Inset+Corner},
                {W-Inset,H-Inset-Corner},{W-Inset-Corner,H-Inset},{Inset+Corner,H-Inset},
                {Inset,H-Inset-Corner},{Inset,Inset+Corner}};
            Top = FLinearColor(.003f,.009f,.017f,1);
            Bottom = FLinearColor(.002f,.007f,.013f,1);
            Edge.A = 0;
        }
        else if (Kind == ESurface::Biography)
        {
            Shape = {{I,I},{W-I-Cut,I},{W-I,I+Cut},{W-I,H-I},{I,H-I}};
            Top = FLinearColor(.007f,.020f,.035f,.95f);
            Bottom = FLinearColor(.002f,.010f,.021f,.93f);
            Edge = Alpha(Structure,.64f);
        }
        else if (Kind == ESurface::Identity)
        {
            const float Ramp = FMath::Min(20.f,H*.3f);
            Shape = {{I+Ramp,I},{W-I,I},{W-I,H-I-Cut},{W-I-Cut,H-I},{I,H-I},{I,I+Ramp}};
            Top = FLinearColor(.003f,.012f,.024f,.65f);
            Bottom = FLinearColor(.002f,.008f,.017f,.99f);
            Edge = Alpha(Structure,.64f);
        }
        else if (Kind == ESurface::Number)
        {
            const float Ramp = FMath::Min(W*.62f,H-2*I);
            Shape = {{I+Ramp,I},{W-I-Cut,I},{W-I,I+Cut},
                {W-I,H-I},{I,H-I}};
            Top = FLinearColor(.012f,.032f,.052f,.98f);
            Edge = Alpha(Accent,.72f);
            Stroke = .85f;
        }
        else if ((Kind == ESurface::Section || Kind == ESurface::Attributes))
        {
            Top = FLinearColor(.003f,.012f,.024f,1);
            Edge = Alpha(Structure,.64f);
        }
        else if (Kind == ESurface::Value || Kind == ESurface::Range)
        {
            const float Side = FMath::Min(W*.14f,H*.45f);
            Shape = {{I+Side,I},{W-I-Side,I},{W-I,H*.5f},
                {W-I-Side,H-I},{I+Side,H-I},{I,H*.5f}};
            Top = Alpha(Accent * .19f + FLinearColor(.004f,.009f,.016f,0),1);
            Bottom = Alpha(Accent * .065f + FLinearColor(.002f,.004f,.009f,0),1);
            Edge = Alpha(Accent,.78f);
            Stroke = .85f;
        }
        // A bounded convex fan gives real cut silhouettes and a subtle vertical finish.
        TArray<FSlateVertex> Vertices;
        TArray<SlateIndex> Indices;
        for (int32 N = 0; N < Shape.Num(); ++N)
        {
            const FLinearColor Fill = FMath::Lerp(Top, Bottom, Shape[N].Y / H) * FillTint;
            Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
                G.GetAccumulatedRenderTransform(), Shape[N], FVector2f::ZeroVector, Fill.ToFColor(true)));
            if (N >= 2) { Indices.Add(0); Indices.Add(N-1); Indices.Add(N); }
        }
        const auto Resource = FSlateApplication::Get().GetRenderer()->GetResourceHandle(
            *FCoreStyle::Get().GetBrush("WhiteBrush"));
        FSlateDrawElement::MakeCustomVerts(Out, Layer, Resource, Vertices, Indices, nullptr, 0, 0);
        if (Kind == ESurface::Number)
        {
            Line(TArray<FVector2f>{Shape[4],Shape[0],Shape[1],Shape[2],Shape[3]},Edge,Stroke);
            BottomSeam();
            return SCompoundWidget::OnPaint(Args,G,Cull,Out,Layer+2,Style,bEnabledHere);
        }
        const bool bSection = Kind == ESurface::Section || Kind == ESurface::Attributes;
        if (bSection)
        {
            // The preceding section owns the shared top seam; no stacked parallel caps.
            Shape.RemoveAt(0);
            Shape.Add(FVector2f(I+Cut,I));
        }
        else
        {
            const FVector2f FirstPoint = Shape[0];
            Shape.Add(FirstPoint);
        }
        if (Edge.A > 0) Line(Shape, Edge, Stroke);
        if (Kind == ESurface::Identity) BottomSeam();
        else if (Kind == ESurface::Biography)
        {
            Line({{4,H-5},{4,5},{W-Cut-2,5},{W-4,Cut+3},{W-4,H-5}},
                Alpha(Structure,.20f),DetailStroke);
        }
        else if (Kind == ESurface::Attributes && H > 50.f)
            Line({{W*.5f,31},{W*.5f,H-9}},Alpha(Structure,.45f),.65f);
        else if (bRow)
            Line({{I,H-Cut},{I+Cut,H-I}},Alpha(Structure,.40f),.65f);
        // Panel fill must precede the dynamic UMG text, never paint over it.
        return SCompoundWidget::OnPaint(Args, G, Cull, Out, Layer + 2, Style, bEnabledHere);
    }
private:
    TWeakObjectPtr<UFMCodexFullCardSurface> Owner;
};
}

void UFMCodexFullCardSurface::SetSurface(EFMCodexFullCardSurface InSurface, const FLinearColor& InAccent)
{
    if (Surface == InSurface && Accent == InAccent) return;
    Surface = InSurface;
    Accent = InAccent;
    InvalidateLayoutAndVolatility();
}

TSharedRef<SWidget> UFMCodexFullCardSurface::RebuildWidget()
{
    MyBorder = SNew(SFMCodexFullCardSurface, this);
    if (GetChildrenCount() > 0)
        CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
    return MyBorder.ToSharedRef();
}
