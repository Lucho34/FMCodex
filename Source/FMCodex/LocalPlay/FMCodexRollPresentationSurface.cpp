#include "FMCodexRollPresentationSurface.h"

#include "Components/BorderSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
class SFMCodexRollPresentationSurface final : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SFMCodexRollPresentationSurface) {} SLATE_END_ARGS()
	void Construct(const FArguments&, UFMCodexRollPresentationSurface* InOwner)
	{
		Owner = InOwner;
		SBorder::Construct(SBorder::FArguments());
	}
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G,
		const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& Style, bool bEnabled) const override
	{
		if (!Owner.IsValid()) return Layer;
		const bool Chamber = Owner->bNumberChamber;
		const FVector2f Size(G.GetLocalSize());
		const float W = Size.X, H = Size.Y;
		if (W < 4 || H < 4) return Layer;
		const float Cut = FMath::Min(Chamber ? 6.0f : 17.0f, FMath::Min(W, H) * .16f);
		const FLinearColor Tint = Style.GetColorAndOpacityTint();
		const FLinearColor Cyan(.02f, .50f, .85f, 1);
		const FLinearColor Steel(.22f, .37f, .53f, 1);
		const FLinearColor Gold(.92f, .67f, .29f, 1);
		const float Activation = Owner->GetActivationProgress();
		const float StartupLight = FMath::Square(FMath::Sin(PI * Activation));
		int32 FillLayer = Layer;
		const auto Resource = FSlateApplication::Get().GetRenderer()->GetResourceHandle(
			*FCoreStyle::Get().GetBrush("WhiteBrush"));
		auto Polygon = [&](const TArray<FVector2f>& Points, FLinearColor Top, FLinearColor Bottom)
		{
			TArray<FSlateVertex> Vertices;
			TArray<SlateIndex> Indices;
			float TopY = H, BottomY = 0;
			for (const auto& Point : Points) { TopY = FMath::Min(TopY, Point.Y); BottomY = FMath::Max(BottomY, Point.Y); }
			for (int32 N = 0; N < Points.Num(); ++N)
			{
				Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
					G.GetAccumulatedRenderTransform(), Points[N], FVector2f::ZeroVector,
					(FMath::Lerp(Top, Bottom, (Points[N].Y-TopY)/FMath::Max(1.0f, BottomY-TopY)) * Tint).ToFColor(true)));
				if (N >= 2) { Indices.Add(0); Indices.Add(N-1); Indices.Add(N); }
			}
			FSlateDrawElement::MakeCustomVerts(Out, FillLayer, Resource, Vertices, Indices, nullptr, 0, 0);
		};
		auto SoftLight = [&](FVector2f Center, FVector2f Radius, FLinearColor Color)
		{
			// A small vertex gradient, not a blur pass or texture-backed effect.
			TArray<FSlateVertex> Vertices;
			TArray<SlateIndex> Indices;
			Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
				G.GetAccumulatedRenderTransform(), Center, FVector2f::ZeroVector, (Color*Tint).ToFColor(true)));
			for (int32 N = 0; N <= 20; ++N)
			{
				const float Angle = N * 2.0f * PI / 20.0f;
				const FVector2f Point = Center + FVector2f(FMath::Cos(Angle)*Radius.X, FMath::Sin(Angle)*Radius.Y);
				Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
					G.GetAccumulatedRenderTransform(), Point, FVector2f::ZeroVector,
					(Color.CopyWithNewOpacity(0)*Tint).ToFColor(true)));
				if (N > 0) { Indices.Add(0); Indices.Add(N); Indices.Add(N+1); }
			}
			FSlateDrawElement::MakeCustomVerts(Out, Layer, Resource, Vertices, Indices, nullptr, 0, 0);
		};
		auto Outline = [&](float Inset)
		{
			return TArray<FVector2f>{{Cut, Inset}, {W-Cut, Inset}, {W-Inset, Cut},
				{W-Inset, H-Cut}, {W-Cut, H-Inset}, {Cut, H-Inset}, {Inset, H-Cut}, {Inset, Cut}};
		};
		auto Line = [&](TArray<FVector2f> Points, FLinearColor Color, float Width = 1.0f)
		{
			FSlateDrawElement::MakeLines(Out, Layer+1, G.ToPaintGeometry(), Points,
				ESlateDrawEffect::None, Color * Tint, true, Width);
		};
		auto Ring = [&](float Inset, FLinearColor Color, float Width)
		{
			auto Points = Outline(Inset);
			const FVector2f ClosingPoint = Points[0];
			Points.Add(ClosingPoint);
			Line(Points, Color, Width);
		};
		// A broad machined bevel, dark gasket and blue inset form separate shells.
		Polygon(Outline(1), FLinearColor(.20f, .30f, .40f, 1), FLinearColor(.025f, .053f, .085f, 1));
		Polygon(Outline(Chamber ? 2 : 4), FLinearColor(.008f, .015f, .03f, 1), FLinearColor(.001f, .003f, .008f, 1));
		Polygon(Outline(Chamber ? 3 : 9), FLinearColor(.018f, .073f, .14f, 1), FLinearColor(.004f, .023f, .05f, 1));
		Polygon(Outline(Chamber ? 5 : 12), FLinearColor(.002f, .012f, .032f, 1), FLinearColor(.001f, .004f, .012f, 1));
		Ring(1, Steel.CopyWithNewOpacity(.95f), Chamber ? .7f : 1.2f);
		Ring(Chamber ? 3 : 10, Cyan.CopyWithNewOpacity(.68f), Chamber ? .7f : 1.2f);
		if (Chamber)
		{
			// The glass-like well has shaded sidewalls and a soft blue center.
			SoftLight({W*.50f, H*.50f}, {W*.43f, H*.48f}, FLinearColor(.02f, .095f, .17f, .60f));
			Polygon({{4, Cut}, {10, Cut+3}, {10, H-Cut-3}, {4, H-Cut}},
				FLinearColor(.020f, .050f, .083f, 1), FLinearColor(.002f, .006f, .014f, 1));
			Polygon({{W-4, Cut}, {W-10, Cut+3}, {W-10, H-Cut-3}, {W-4, H-Cut}},
				FLinearColor(.008f, .022f, .041f, 1), FLinearColor(.001f, .003f, .008f, 1));
			// Embedded light guides stay fixed while only the gold lock line activates.
			Line({{5, Cut}, {5, H-Cut}}, Cyan.CopyWithNewOpacity(.42f), .65f);
			Line({{W-5, Cut}, {W-5, H-Cut}}, Cyan.CopyWithNewOpacity(.42f), .65f);
			Line({{10, Cut+3}, {10, H-Cut-3}}, FLinearColor(.001f, .003f, .008f, .9f), 1.1f);
			Line({{W-10, Cut+3}, {W-10, H-Cut-3}}, Steel.CopyWithNewOpacity(.25f), .6f);
			SoftLight({W*.5f, 5}, {W*.30f, 8}, Cyan.CopyWithNewOpacity(.20f));
			SoftLight({W*.5f, H-5}, {W*.30f, 8}, Cyan.CopyWithNewOpacity(.16f));
			Line({{W*.30f, 3}, {W*.70f, 3}}, Cyan.CopyWithNewOpacity(.60f), .8f);
			Line({{W*.28f, H-3}, {W*.72f, H-3}}, Cyan.CopyWithNewOpacity(.55f), .8f);
			const float Y = H*.5f;
			SoftLight({6, Y}, {6, 12}, Cyan.CopyWithNewOpacity(.40f));
			SoftLight({W-6, Y}, {6, 12}, Cyan.CopyWithNewOpacity(.40f));
			Line({{2, Y-7}, {2, Y+7}}, Steel.CopyWithNewOpacity(.7f), 1.2f);
			Line({{W-2, Y-7}, {W-2, Y+7}}, Steel.CopyWithNewOpacity(.7f), 1.2f);
			Polygon({{3, Y-4}, {8, Y}, {3, Y+4}}, Cyan, FLinearColor(.07f, .65f, 1, 1));
			Polygon({{W-3, Y-4}, {W-8, Y}, {W-3, Y+4}}, Cyan, FLinearColor(.07f, .65f, 1, 1));
			const float Lock = Owner->GetLockEmphasis();
			if (Lock > 0)
			{
				const float HalfWidth = W*.23f * Lock;
				Line({{W*.5f-HalfWidth, Y+24}, {W*.5f+HalfWidth, Y+24}},
					Gold.CopyWithNewOpacity(.92f * Lock), 1.65f);
			}
		}
		else
		{
			constexpr float HeaderBottom = 66;
			const float FooterTop = H-88;
			Polygon({{Cut+5, 12}, {W-Cut-5, 12}, {W-12, Cut+5}, {W-12, HeaderBottom-5},
				{W-21, HeaderBottom}, {21, HeaderBottom}, {12, HeaderBottom-5}, {12, Cut+5}},
				FLinearColor(.015f, .047f, .10f, 1), FLinearColor(.002f, .009f, .022f, 1));
			Polygon({{21, FooterTop}, {W-21, FooterTop}, {W-12, FooterTop+5}, {W-12, H-Cut-5},
				{W-Cut-5, H-12}, {Cut+5, H-12}, {12, H-Cut-5}, {12, FooterTop+5}},
				FLinearColor(.011f, .034f, .072f, 1), FLinearColor(.002f, .008f, .020f, 1));
			SoftLight({W*.5f, 26}, {W*.38f, 22}, FLinearColor(.014f, .09f, .20f, .35f + .35f*StartupLight));
			SoftLight({W*.5f, FooterTop+29}, {W*.34f, 25}, FLinearColor(.01f, .05f, .11f, .35f));
			// Quiet diagonal treatment confined to the unused sides of the chamber.
			for (float Y = HeaderBottom+22; Y < FooterTop-48; Y += 16)
			{
				Line({{18, Y+40}, {65, Y}}, Steel.CopyWithNewOpacity(.12f), .65f);
				Line({{W-65, Y+40}, {W-18, Y}}, Steel.CopyWithNewOpacity(.12f), .65f);
			}
			Line({{12, HeaderBottom-5}, {21, HeaderBottom}, {W-21, HeaderBottom}, {W-12, HeaderBottom-5}}, Steel.CopyWithNewOpacity(.55f));
			Line({{12, FooterTop+5}, {21, FooterTop}, {W-21, FooterTop}, {W-12, FooterTop+5}}, Steel.CopyWithNewOpacity(.65f));
			const float GuideWidth = W * (.08f + .04f*Activation);
			Line({{W*.5f-GuideWidth, HeaderBottom-6}, {W*.5f+GuideWidth, HeaderBottom-6}}, Cyan.CopyWithNewOpacity(.16f), 6);
			Line({{W*.5f-GuideWidth, HeaderBottom-6}, {W*.5f+GuideWidth, HeaderBottom-6}}, Cyan, 2);
			// Fixed brackets seat the chamber into the case; their short rails energize at entry.
			for (float X : {W*.5f-108, W*.5f+108})
			{
				Line({{X, HeaderBottom+24}, {X, FooterTop-24}}, Steel.CopyWithNewOpacity(.18f), 2);
				Line({{X, H*.5f-14}, {X, H*.5f+14}}, Cyan.CopyWithNewOpacity(.20f+.45f*StartupLight), 2);
			}
			// Short illuminated rails, bounded to the corners of the existing modal.
			for (float X : {5.0f, W-5.0f})
			{
				for (float Y : {Cut+12, H-Cut-32})
				{
					Line({{X, Y-3}, {X, Y+23}}, FLinearColor(.001f, .004f, .012f, 1), 6);
					Line({{X, Y}, {X, Y+20}}, Cyan.CopyWithNewOpacity(.22f+.18f*StartupLight), 8);
					Line({{X, Y+2}, {X, Y+18}}, Cyan.CopyWithNewOpacity(.60f+.40f*Activation), 2.8f);
					Line({{X, Y+4}, {X, Y+16}}, FLinearColor(.24f, .78f, 1, .75f), .8f);
				}
			}
			Line({{Cut, 2}, {W-Cut, 2}}, FLinearColor(.43f, .56f, .69f, .9f), 1.5f);
			Line({{Cut+10, 10}, {W-Cut-10, 10}}, Cyan.CopyWithNewOpacity(.50f));
			Line({{Cut+12, H-3}, {W-Cut-12, H-3}}, Steel.CopyWithNewOpacity(.70f), 1.5f);
			Line({{2, Cut+4}, {2, Cut}, {Cut, 2}, {Cut+14, 2}}, Steel, 1.8f);
			Line({{W-2, H-Cut-4}, {W-2, H-Cut}, {W-Cut, H-2}, {W-Cut-14, H-2}}, Steel, 1.8f);
			for (float X : {28.0f, W-48.0f})
			{
				for (int32 N = 0; N < 3; ++N)
				{
					Line({{X+N*6, 39}, {X+N*6+6, 32}}, Steel.CopyWithNewOpacity(.65f), 1.6f);
					Line({{X+N*6, FooterTop+36}, {X+N*6+6, FooterTop+29}}, Steel.CopyWithNewOpacity(.50f), 1.6f);
				}
			}
		}

		const int32 ChildLayer = SCompoundWidget::OnPaint(Args, G, Cull, Out, Layer+2, Style, bEnabled);
		if (Chamber)
		{
			// Stationary slot shadows occlude passing edge fragments without a blur
			// or another digit widget. The centered final value remains unobscured.
			FillLayer = ChildLayer+1;
			const float Shade = H*.16f;
			const FLinearColor Shadow(.001f, .005f, .014f, .85f);
			Polygon({{10, 5}, {W-10, 5}, {W-10, Shade}, {10, Shade}}, Shadow, Shadow.CopyWithNewOpacity(0));
			Polygon({{10, H-Shade}, {W-10, H-Shade}, {W-10, H-5}, {10, H-5}}, Shadow.CopyWithNewOpacity(0), Shadow);
			return FillLayer;
		}
		return ChildLayer;
	}
private:
	TWeakObjectPtr<UFMCodexRollPresentationSurface> Owner;
};
}

TSharedRef<SWidget> UFMCodexRollPresentationSurface::RebuildWidget()
{
	MyBorder = SNew(SFMCodexRollPresentationSurface, this);
	if (GetChildrenCount() > 0)
		CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
	return MyBorder.ToSharedRef();
}

void UFMCodexRollPresentationSurface::SetLockEmphasis(const float InEmphasis)
{
	const float Value = FMath::Clamp(InEmphasis, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(Value, LockEmphasis))
	{
		LockEmphasis = Value;
		if (MyBorder.IsValid()) MyBorder->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void UFMCodexRollPresentationSurface::SetActivationProgress(const float InProgress)
{
	const float Value = FMath::Clamp(InProgress, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(Value, ActivationProgress))
	{
		ActivationProgress = Value;
		if (MyBorder.IsValid()) MyBorder->Invalidate(EInvalidateWidgetReason::Paint);
	}
}
