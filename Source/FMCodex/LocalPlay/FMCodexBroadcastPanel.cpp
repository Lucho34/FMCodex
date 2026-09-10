#include "FMCodexBroadcastPanel.h"

#include "Components/BorderSlot.h"
#include "Components/ButtonSlot.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

namespace
{
// Decorative venue art only. Sponsor rhythm is authored once, never gameplay RNG.
struct FVenueSponsor
{
	const TCHAR* Name;
	FLinearColor Face;
	FLinearColor Ink;
	FLinearColor Accent;
};

struct FVenuePanel
{
	int32 Sponsor; // INDEX_NONE leaves a quiet, unlit section of the ribbon.
	float Weight;
	bool bMarkOnly = false;
};

void PaintVenueAdvertising(const FGeometry& Geometry, FSlateWindowElementList& Out,
	int32 Layer, const FLinearColor& Tint)
{
	const FVector2f Size(Geometry.GetLocalSize());
	const float W=Size.X, H=Size.Y;
	static const FVenueSponsor Sponsors[] = {
		{TEXT("GOALGRID"),   {0.010f,0.035f,0.047f,1}, {0.53f,0.65f,0.68f,0.88f}, {0.15f,0.42f,0.46f,0.80f}},
		{TEXT("FOOTLAB"),    {0.380f,0.460f,0.425f,1}, {0.001f,0.004f,0.003f,1}, {0.002f,0.008f,0.006f,1}},
		{TEXT("ARENA+"),     {0.030f,0.023f,0.012f,1}, {0.68f,0.67f,0.60f,0.86f}, {0.47f,0.35f,0.17f,0.74f}},
		{TEXT("MATCHPRO"),   {0.014f,0.033f,0.065f,1}, {0.51f,0.57f,0.65f,0.84f}, {0.20f,0.34f,0.49f,0.76f}},
		{TEXT("NOVA XI"),    {0.012f,0.018f,0.022f,1}, {0.52f,0.59f,0.62f,0.78f}, {0.31f,0.40f,0.44f,0.70f}},
		{TEXT("STRIKE ONE"), {0.031f,0.019f,0.021f,1}, {0.64f,0.61f,0.58f,0.87f}, {0.42f,0.23f,0.21f,0.68f}},
		{TEXT("PITCHLAB"),   {0.010f,0.024f,0.022f,1}, {0.49f,0.59f,0.56f,0.79f}, {0.24f,0.39f,0.33f,0.70f}}
	};
	static const FVenuePanel Top[] = {
		{0,1.50f},{INDEX_NONE,0.28f},{4,1.10f},{2,0.48f,true},
		{1,1.30f},{5,1.55f},{INDEX_NONE,0.22f},{6,1.20f},{3,1.50f}
	};
	static const FVenuePanel Bottom[] = {
		{5,1.45f},{1,1.05f},{INDEX_NONE,0.30f},{2,1.45f},{6,0.62f,true},
		{4,1.20f},{0,1.60f},{INDEX_NONE,0.38f},{3,1.40f}
	};
	static const FVenuePanel Left[] = {
		{0,0.95f,true},{INDEX_NONE,0.82f},{6,1.26f,true},{4,0.55f,true},
		{INDEX_NONE,1.08f},{2,1.40f,true},{3,0.70f,true}
	};
	static const FVenuePanel Right[] = {
		{INDEX_NONE,0.58f},{5,1.20f,true},{2,0.88f,true},{INDEX_NONE,1.22f},
		{1,1.42f,true},{0,0.68f,true},{INDEX_NONE,0.78f}
	};
	const FSlateBrush* White=FCoreStyle::Get().GetBrush("WhiteBrush");
	auto Line = [&](const FGeometry& G, TArray<FVector2f> Points, FLinearColor Color, float Thickness=1.0f)
	{
		FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),Points,
			ESlateDrawEffect::None,Color*Tint,true,Thickness);
	};
	auto Box = [&](const FGeometry& G, FVector2f Position, FVector2f Extent, FLinearColor Color)
	{
		FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(Extent,FSlateLayoutTransform(Position)),
			White,ESlateDrawEffect::None,Color*Tint);
	};
	auto Wash = [&](const FGeometry& G, FVector2f Extent, FLinearColor TopColor, FLinearColor BottomColor)
	{
		FSlateDrawElement::MakeGradient(Out,Layer,G.ToPaintGeometry(),
			{FSlateGradientStop(FVector2f(0,0),TopColor*Tint),
			 FSlateGradientStop(FVector2f(0,Extent.Y),BottomColor*Tint)},Orient_Horizontal);
	};
	auto Mark = [&](const FGeometry& G, FVector2f Center, float Radius, int32 Identity, bool bOnDarkFace=false)
	{
		const FLinearColor C= Identity==1 && bOnDarkFace
			? FLinearColor(0.24f,0.42f,0.34f,0.76f) : Sponsors[Identity].Accent;
		auto Stroke = [&](TArray<FVector2f> Points)
		{
			for (FVector2f& P : Points) P=Center+P*Radius;
			FLinearColor Halo=C; Halo.A*=0.10f;
			Line(G,Points,Halo,3.0f);
			Line(G,Points,C,1.1f);
		};
		switch (Identity)
		{
		case 0: // Open grid: no club crest.
			Stroke({{-1,-1},{-1,1},{1,1},{1,0},{0,0}});
			Stroke({{0,-1},{1,-1},{1,-0.5f}}); break;
		case 1: // Two connected test nodes.
			Stroke({{-1,0.6f},{-0.4f,-0.6f},{0.4f,0.6f},{1,-0.6f}});
			Stroke({{-0.7f,1},{0.7f,1}}); break;
		case 2: // Plus inside an open arena bracket.
			Stroke({{-1,-0.6f},{-1,0.6f}});
			Stroke({{1,-0.6f},{1,0.6f}});
			Stroke({{-0.5f,0},{0.5f,0}});
			Stroke({{0,-0.5f},{0,0.5f}}); break;
		case 3: // Offset M construction.
			Stroke({{-1,0.8f},{-1,-0.8f},{0,0.3f},{1,-0.8f},{1,0.8f}}); break;
		case 4: // Quiet hexagonal technical mark.
			Stroke({{0,-1},{0.87f,-0.5f},{0.87f,0.5f},{0,1},{-0.87f,0.5f},{-0.87f,-0.5f},{0,-1}});
			Stroke({{-0.3f,0.3f},{0.3f,-0.3f}}); break;
		case 5: // A single broken strike path.
			Stroke({{-1,-0.65f},{0.35f,-0.65f},{-0.35f,0.65f},{1,0.65f}}); break;
		default: // Abstract field-study grid, independent of tactical-board facts.
			Stroke({{-1,-0.65f},{1,-0.65f},{1,0.65f},{-1,0.65f},{-1,-0.65f}});
			Stroke({{0,-0.65f},{0,0.65f}}); break;
		}
	};
	const auto FontMeasure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	auto Wordmark = [&](const FGeometry& G, float Length, float Height, int32 Identity)
	{
		const auto& Sponsor=Sponsors[Identity];
		const bool bLarge=Identity==2 || Identity==3;
		const FSlateFontInfo Font=FCoreStyle::GetDefaultFontStyle(Identity==4 ? "Regular" : "Bold",bLarge ? 11 : 9);
		const FString Name(Sponsor.Name);
		const FVector2f TextSize(FontMeasure->Measure(Name,Font));
		const float Scale=FMath::Min(1.0f,FMath::Min((Length-32)/TextSize.X,(Height-3)/TextSize.Y));
		const float GroupWidth=TextSize.X*Scale+(bLarge ? 0.0f : 17.0f);
		const float Start=(Length-GroupWidth)*0.5f;
		if (!bLarge) Mark(G,{Start+5,Height*0.5f},4,Identity);
		const FVector2f Position(Start+(bLarge ? 0.0f : 17.0f),(Height-TextSize.Y*Scale)*0.5f);
		FSlateDrawElement::MakeText(Out,Layer+1,
			G.ToPaintGeometry(TextSize,FSlateLayoutTransform(Scale,Position)),
			Name,Font,ESlateDrawEffect::None,Sponsor.Ink*Tint);
		if (Identity==0 || Identity==3)
			Line(G,{{Length-14,Height*0.5f},{Length-6,Height*0.5f}},Sponsor.Accent);
		if (Identity==5)
			Line(G,{{Start,Height-2},{Start+GroupWidth*0.36f,Height-2}},Sponsor.Accent);
	};
	auto Ribbon = [&](FVector2f Position, float Length, float Height,
		TConstArrayView<FVenuePanel> Panels, bool bSide)
	{
		const FVector2f Extent=bSide ? FVector2f(Height,Length) : FVector2f(Length,Height);
		const FGeometry G=Geometry.MakeChild(Extent,FSlateLayoutTransform(Position));
		// Continuous housing: no individual button-like outlines. Mechanical seams
		// have their own spacing and are independent of sponsor-panel boundaries.
		Box(G,{-2,-2},Extent+FVector2f(4,5),FLinearColor(0.001f,0.003f,0.005f,0.96f));
		Wash(G,Extent,FLinearColor(0.008f,0.016f,0.021f,1),FLinearColor(0.002f,0.005f,0.008f,1));
		float Weight=0;
		for (const auto& P : Panels) Weight+=P.Weight;
		float Offset=0;
		for (const auto& P : Panels)
		{
			const float Span=Length*P.Weight/Weight;
			const FVector2f SectionExtent=bSide ? FVector2f(Height,Span) : FVector2f(Span,Height);
			const FGeometry Section=G.MakeChild(SectionExtent,FSlateLayoutTransform(
				bSide ? FVector2f(0,Offset) : FVector2f(Offset,0)));
			if (P.Sponsor!=INDEX_NONE)
			{
				const auto& Sponsor=Sponsors[P.Sponsor];
				// Inverse mint signage is confined to horizontal runs. Narrow side
				// housings retain the quiet dark substrate even for the same identity.
				const FLinearColor Face=bSide ? Sponsor.Face*FLinearColor(0.22f,0.22f,0.22f,1) : Sponsor.Face;
				Wash(Section,SectionExtent,Face,Face*FLinearColor(0.60f,0.60f,0.60f,1));
				if (bSide)
				{
					// Upright, sparse marks; never repeated rotated sponsor words.
					Mark(Section,{Height*0.5f,Span*0.46f},FMath::Min(5.5f,Height*0.32f),P.Sponsor,true);
					FLinearColor Quiet=Sponsor.Accent; Quiet.A*=0.28f;
					Line(Section,{{Height*0.5f,Span*0.70f},{Height*0.5f,Span*0.80f}},Quiet);
				}
				else if (P.bMarkOnly)
					Mark(Section,{Span*0.5f,Height*0.5f},5.0f,P.Sponsor);
				else
					Wordmark(Section,Span,Height,P.Sponsor);
			}
			Offset+=Span;
		}
		// Fine LED substrate overlays the lit content, faint enough to avoid moire.
		const float ScanExtent=bSide ? Extent.X : Extent.Y;
		for (float P=2; P<ScanExtent-1; P+=3)
			Line(G,bSide ? TArray<FVector2f>{{P,1},{P,Extent.Y-1}}
				: TArray<FVector2f>{{1,P},{Extent.X-1,P}},FLinearColor(0,0,0,0.10f),0.5f);
		if (bSide)
		{
			const bool bLeft=Position.X<W*0.5f;
			const float BackX=bLeft ? -3.0f : Height+1.0f;
			Box(G,{BackX,0},{2,Length},FLinearColor(0.040f,0.059f,0.065f,0.88f));
			Line(G,{{0,0},{0,Length}},FLinearColor(0.12f,0.17f,0.19f,0.60f));
			Line(G,{{Height,0},{Height,Length}},FLinearColor(0,0,0,0.88f),2);
			for (float Y=37; Y<Length; Y+=83)
			{
				Line(G,{{0,Y},{Height,Y}},FLinearColor(0,0,0,0.75f));
				const float FaceX=bLeft ? 0.0f : Height;
				Line(G,{{BackX,Y+4},{FaceX,Y},{FaceX,Y-2}},
					FLinearColor(0.08f,0.11f,0.12f,0.65f),1.4f);
			}
		}
		else
		{
			Line(G,{{0,0},{Length,0}},FLinearColor(0.13f,0.18f,0.20f,0.56f));
			Line(G,{{0,Height},{Length,Height}},FLinearColor(0,0,0,0.88f),2);
			for (float X=28; X<Length; X+=69)
			{
				Line(G,{{X,1},{X,Height-1}},FLinearColor(0,0,0,0.52f),0.75f);
				Box(G,{X,Height+1},{2,3},FLinearColor(0.045f,0.065f,0.075f,0.82f));
			}
		}
	};
	Ribbon({40,41},W-80,15,Top,false);
	Ribbon({40,H-34},W-80,18,Bottom,false);
	Ribbon({17,66},H-114,19,Left,true);
	Ribbon({W-36,66},H-114,19,Right,true);
	// Quiet corner returns and a dark service gap behind the LED housing.
	for (const float X : {24.0f,W-24.0f})
	{
		Line(Geometry,{{X,48},{X,60},{X+(X<W*0.5f ? 10 : -10),64}},
			FLinearColor(0.08f,0.12f,0.14f,0.70f));
		Line(Geometry,{{X,H-47},{X,H-25},{X+(X<W*0.5f ? 12 : -12),H-25}},
			FLinearColor(0.08f,0.12f,0.14f,0.65f));
	}
}

class SFMCodexBroadcastPanel : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SFMCodexBroadcastPanel) {}
	SLATE_END_ARGS()
	void Construct(const FArguments&, UFMCodexBroadcastPanel* InOwner)
	{
		Owner = InOwner;
		SBorder::Construct(SBorder::FArguments());
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
		const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& Style, bool bEnabled) const override
	{
		const auto* Panel = Owner.Get();
		if (!Panel) return Layer;
		const FVector2f Size(Geometry.GetLocalSize());
		const float W = Size.X, H = Size.Y;
		const auto Kind = Panel->Surface;
		const FLinearColor Tint = Style.GetColorAndOpacityTint();
		FLinearColor Accent = Panel->GetBrushColor();
		const bool bHeader = Kind == EFMCodexBroadcastSurface::HeaderLeft
			|| Kind == EFMCodexBroadcastSurface::HeaderRight;
		if (bHeader)
		{
			const float Neutral = FMath::Min3(Accent.R, Accent.G, Accent.B) * 0.85f;
			Accent = FLinearColor(Accent.R - Neutral, Accent.G - Neutral, Accent.B - Neutral, Accent.A);
			if (Accent.B > Accent.R) Accent.G *= 0.60f;
		}
		auto Lines = [&](TArray<FVector2f> Points, FLinearColor Color, float Width = 1.0f)
		{
			FSlateDrawElement::MakeLines(Out, Layer + 1, Geometry.ToPaintGeometry(),
				Points, ESlateDrawEffect::None, Color * Tint, true, Width);
		};
		if (Kind == EFMCodexBroadcastSurface::MidfieldLeft
			|| Kind == EFMCodexBroadcastSurface::MidfieldRight)
		{
			// True half-circle on the canonical halfway boundary; no font glyph approximation.
			const bool bLeft = Kind == EFMCodexBroadcastSurface::MidfieldLeft;
			TArray<FVector2f> Arc;
			for (int32 I = 0; I <= 48; ++I)
			{
				const float A = -HALF_PI + PI * I / 48.0f;
				Arc.Add(FVector2f(bLeft ? FMath::Cos(A) * W : W - FMath::Cos(A) * W,
					H * 0.5f + FMath::Sin(A) * H * 0.5f));
			}
			Lines(Arc, Accent, 1.6f);
			return Layer + 1;
		}
		auto Quad = [&](const TArray<FVector2f>& Points, FLinearColor Top, FLinearColor Bottom)
		{
			TArray<FSlateVertex> V;
			for (int32 I = 0; I < 4; ++I)
			{
				V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(
					Geometry.GetAccumulatedRenderTransform(), Points[I], FVector2f::ZeroVector,
					((I < 2 ? Top : Bottom) * Tint).ToFColor(true)));
			}
			const auto Resource = FSlateApplication::Get().GetRenderer()->GetResourceHandle(
				*FCoreStyle::Get().GetBrush("WhiteBrush"));
			FSlateDrawElement::MakeCustomVerts(Out, Layer, Resource, V,
				TArray<SlateIndex>{0, 1, 2, 0, 2, 3}, nullptr, 0, 0);
		};
		auto Rect = [&](float X, float Y, float Width, float Height, FLinearColor Top, FLinearColor Bottom)
		{
			Quad({{X,Y},{X+Width,Y},{X+Width,Y+Height},{X,Y+Height}}, Top, Bottom);
		};
		const FLinearColor Edge(0.13f,0.25f,0.36f,0.65f);
		const FLinearColor Glint(0.36f,0.57f,0.73f,0.30f);
		if (bHeader)
		{
			const bool bLeft = Kind == EFMCodexBroadcastSurface::HeaderLeft;
			const float Cut = FMath::Min(H * 0.66f, W * 0.18f);
			TArray<FVector2f> Shape = bLeft
				? TArray<FVector2f>{{0,0},{W-Cut,0},{W,H},{0,H}}
				: TArray<FVector2f>{{Cut,0},{W,0},{W,H},{0,H}};
			FLinearColor Top = FMath::Lerp(FLinearColor(0.003f,0.010f,0.024f,1), Accent, 0.58f);
			FLinearColor Bottom = FMath::Lerp(FLinearColor(0.002f,0.006f,0.014f,1), Accent, 0.12f);
			Top.A = 0.94f; Bottom.A = 0.90f;
			Quad(Shape, Top, Bottom);
			const TArray<FVector2f> Rail = bLeft
				? TArray<FVector2f>{{W-Cut-14,0},{W-Cut,0},{W,H},{W-14,H}}
				: TArray<FVector2f>{{Cut,0},{Cut+14,0},{14,H},{0,H}};
			Quad(Rail, Accent * FLinearColor(1.35f,1.35f,1.35f,1), Accent * FLinearColor(0.35f,0.35f,0.35f,1));
			const FVector2f FirstPoint = Shape[0];
			Shape.Add(FirstPoint);
			Lines(Shape, Glint);
			Lines({{bLeft ? 0.0f : Cut+18,4},{bLeft ? W-Cut-18 : W,4}}, Accent, 2);
			Lines({{0,H-4},{W,H-4}}, FLinearColor(0.25f,0.48f,0.68f,0.27f));
			const float X = bLeft ? 28.0f : W-64.0f;
			// Small broadcast corner ticks, deliberately no invented crest or sponsor.
			for (int32 I=0; I<3; ++I)
				Lines({{X+I*12,H-11},{X+I*12+5,H-18}}, Accent * FLinearColor(1,1,1,0.45f), 2);
		}
		else if (Kind == EFMCodexBroadcastSurface::Score)
		{
			Rect(0,0,W,H,FLinearColor(0.003f,0.010f,0.020f,0.66f),FLinearColor(0.001f,0.004f,0.009f,0.88f));
			const float L = W*0.20f, R = W*0.80f;
			Quad({{L+18,1},{R-18,1},{R,H-18},{L,H-18}},
				FLinearColor(0.016f,0.037f,0.061f,0.83f),FLinearColor(0.002f,0.007f,0.016f,0.92f));
			Lines({{0,4},{L+18,4},{L+26,0},{R-26,0},{R-18,4},{W,4}},Glint);
			Lines({{0,H-4},{W,H-4}},Edge);
			Lines({{L-30,H*0.46f},{L-10,H*0.46f}},Glint);
			Lines({{R+10,H*0.46f},{R+30,H*0.46f}},Glint);
		}
		else if (Kind == EFMCodexBroadcastSurface::TacticalResource
			|| Kind == EFMCodexBroadcastSurface::TacticalResourceValue)
		{
			const bool bValue = Kind == EFMCodexBroadcastSurface::TacticalResourceValue;
			const float Cut=5.0f;
			const FLinearColor ResourceEdge=FMath::Lerp(Edge,Accent,0.36f);
			if (bValue)
			{
				Quad({{0,1},{W-1,1},{W-Cut,H-1},{0,H-1}},
					FMath::Lerp(FLinearColor(0.025f,0.055f,0.090f,1),Accent,0.38f),
					FLinearColor(0.010f,0.027f,0.050f,1));
				Lines({{0,5},{0,H-5}},ResourceEdge);
			}
			else
			{
				Quad({{0,0},{W,0},{W-Cut,H},{0,H}},FLinearColor(0.008f,0.022f,0.042f,0.96f),
					FLinearColor(0.003f,0.010f,0.022f,0.98f));
				Lines({{1,1},{W-1,1},{W-Cut,H-1},{1,H-1},{1,1}},ResourceEdge);
				Lines({{6,3},{W-6,3}},FLinearColor(0.38f,0.59f,0.76f,0.18f));
				Lines({{6,H-1},{W-Cut-3,H-1}},Accent*FLinearColor(1,1,1,0.68f));
			}
		}
		else if (Kind == EFMCodexBroadcastSurface::Progress)
		{
			Quad({{8,0},{W-8,0},{W,H},{0,H}},
				FLinearColor(0.003f,0.016f,0.034f,1),FLinearColor(0.001f,0.007f,0.015f,0.98f));
			Lines({{12,0},{W-12,0}},Glint);
			Lines({{8,H-1},{W-8,H-1}},FLinearColor(0.04f,0.21f,0.37f,0.65f));
		}
		else if (Kind == EFMCodexBroadcastSurface::PitchSurround)
		{
			// The supplied venue remains scenery: cropped stands, separate physical
			// sideline boards, then the live tactical turf and its independent geometry.
			Rect(0,32,W,H-32,FLinearColor(0.001f,0.007f,0.015f,0.65f),FLinearColor(0.001f,0.004f,0.009f,0.90f));
			auto Stands = [&](FVector2f Position, FVector2f Extent, FVector2f UVMin, FVector2f UVMax)
			{
				FSlateBrush Venue = Panel->Background;
				Venue.SetUVRegion(FBox2f(UVMin,UVMax));
				FSlateDrawElement::MakeBox(Out,Layer,
					Geometry.ToPaintGeometry(Extent,FSlateLayoutTransform(Position)),
					&Venue,ESlateDrawEffect::None,FLinearColor(0.72f,0.78f,0.84f,0.94f)*Tint);
			};
			Stands({0,36},{W,18},{0.12f,0.20f},{0.88f,0.31f});
			Stands({0,50},{32,H-74},{0.01f,0.18f},{0.12f,0.48f});
			Stands({W-32,50},{32,H-74},{0.88f,0.18f},{0.99f,0.48f});
			Stands({0,H-30},{W,30},{0.06f,0.16f},{0.94f,0.29f});
			Quad({{25,51},{38,58},{38,H-36},{23,H-24}},
				FLinearColor(0.027f,0.044f,0.052f,0.96f),FLinearColor(0.005f,0.014f,0.020f,0.98f));
			Quad({{W-38,58},{W-25,51},{W-23,H-24},{W-38,H-36}},
				FLinearColor(0.027f,0.044f,0.052f,0.96f),FLinearColor(0.005f,0.014f,0.020f,0.98f));
			Rect(30,49,W-60,9,FLinearColor(0.027f,0.044f,0.052f,0.95f),FLinearColor(0.002f,0.006f,0.010f,1));
			Rect(24,H-37,W-48,13,FLinearColor(0.010f,0.019f,0.023f,1),FLinearColor(0.002f,0.005f,0.009f,1));
			PaintVenueAdvertising(Geometry,Out,Layer,Tint);
			Lines({{38,58},{38,H-36},{W-38,H-36},{W-38,58}},FLinearColor(0.09f,0.19f,0.15f,0.70f),1.5f);
			Lines({{23,51},{21,H-23},{W-21,H-23},{W-23,51}},FLinearColor(0.11f,0.18f,0.23f,0.70f));
		}
		else if (Kind == EFMCodexBroadcastSurface::TurfLighting)
		{
			// A quiet turf wash and soft perimeter shadows seat the live field within
			// the venue. Markings and drop targets remain separate interactive children.
			Rect(0,0,W,H,FLinearColor(0.012f,0.065f,0.023f,0.14f),FLinearColor(0.003f,0.025f,0.012f,0.22f));
			auto Shade = [&](FVector2f Position, FVector2f Extent, EOrientation Orientation,
				FLinearColor StartColor, FLinearColor EndColor)
			{
				FSlateDrawElement::MakeGradient(Out,Layer+1,
					Geometry.ToPaintGeometry(Extent,FSlateLayoutTransform(Position)),
					{FSlateGradientStop(FVector2f(0,0),StartColor*Tint),
					 FSlateGradientStop(Extent,EndColor*Tint)},Orientation);
			};
			const FLinearColor Dark(0.001f,0.006f,0.005f,0.48f), Clear(0.001f,0.006f,0.005f,0);
			// Slate names the stop-line orientation: vertical stops interpolate along X.
			Shade({0,0},{70,H},Orient_Vertical,Dark,Clear);
			Shade({W-70,0},{70,H},Orient_Vertical,Clear,Dark);
			Shade({0,0},{W,48},Orient_Horizontal,Dark,Clear);
			Shade({0,H-80},{W,80},Orient_Horizontal,Clear,Dark);
		}
		else if (Kind == EFMCodexBroadcastSurface::PitchHUD)
		{
			Rect(0,0,W,H,FLinearColor(0.003f,0.020f,0.021f,0.94f),FLinearColor(0.001f,0.008f,0.014f,0.94f));
			Quad({{W*.38f,1},{W*.62f,1},{W*.64f,H-1},{W*.36f,H-1}},
				FLinearColor(0.020f,0.040f,0.047f,0.95f),FLinearColor(0.004f,0.013f,0.021f,0.95f));
			Lines({{18,0},{W-18,0}},Glint);
			Lines({{8,H-1},{W-8,H-1}},FLinearColor(0.04f,0.17f,0.15f,0.75f));
			for (const float X : {W*.13f,W*.36f,W*.64f,W*.87f})
			{
				const float D=X<W*.5f ? -1 : 1;
				Lines({{X-D*4,H*.36f},{X+D*2,H*.5f},{X-D*4,H*.64f}},FLinearColor(0.10f,0.37f,0.34f,0.48f),1.4f);
				Lines({{X-D*11,H*.36f},{X-D*5,H*.5f},{X-D*11,H*.64f}},FLinearColor(0.10f,0.37f,0.34f,0.28f),1.4f);
			}
		}
		else if (Kind == EFMCodexBroadcastSurface::Prompt)
		{
			// Open status rail: deliberately no enclosed button-shaped frame.
			Rect(0,0,W,H,FLinearColor(0.003f,0.016f,0.030f,0.35f),FLinearColor(0.001f,0.006f,0.015f,0.12f));
			Lines({{1,10},{1,H-10}},Accent,3);
			Lines({{W-1,10},{W-1,H-10}},Edge);
			Lines({{13,H*.5f-4},{17,H*.5f},{13,H*.5f+4}},FLinearColor(0.32f,0.66f,0.92f,0.95f),2);
		}
		else if (Kind == EFMCodexBroadcastSurface::Instruction)
		{
			Rect(0,0,W,H,FLinearColor(0.017f,0.031f,0.043f,0.80f),FLinearColor(0.003f,0.010f,0.017f,0.92f));
			Lines({{6,0},{W-6,0},{W,6},{W,H-6},{W-6,H},{6,H},{0,H-6},{0,6},{6,0}},Edge);
			Lines({{7,2},{W-7,2}},Glint);
			const float X=22,Y=H*.5f;
			Lines({{X-8,Y},{X+8,Y}},Glint,1.5f);
			Lines({{X,Y-8},{X,Y+8}},Glint,1.5f);
			Lines({{X-5,Y-3},{X-8,Y},{X-5,Y+3}},Glint,1.5f);
			Lines({{X+5,Y-3},{X+8,Y},{X+5,Y+3}},Glint,1.5f);
			Lines({{X-3,Y-5},{X,Y-8},{X+3,Y-5}},Glint,1.5f);
			Lines({{X-3,Y+5},{X,Y+8},{X+3,Y+5}},Glint,1.5f);
		}
		else if (Kind == EFMCodexBroadcastSurface::Brand)
		{
			Lines({{8,H*.52f},{W-368,H*.52f},{W-355,H*.52f-13}},Edge);
			for (int32 I=0;I<3;++I)
				Lines({{W-350+I*10,H*.52f},{W-338+I*10,H*.52f-14}},FLinearColor(0.13f,0.25f,0.35f,0.60f),3);
			TArray<FVector2f> Ball;
			for (int32 I=0;I<=32;++I)
				Ball.Add({W-19+13*FMath::Cos(2*PI*I/32),H*.5f+13*FMath::Sin(2*PI*I/32)});
			Lines(Ball,Edge,1.5f);
			TArray<FVector2f> CenterPatch;
			for (int32 I=0;I<=5;++I)
			{
				const float A=-HALF_PI+2*PI*I/5;
				const FVector2f Point(W-19+5*FMath::Cos(A),H*.5f+5*FMath::Sin(A));
				CenterPatch.Add(Point);
				Lines({Point,{W-19+13*FMath::Cos(A),H*.5f+13*FMath::Sin(A)}},Edge);
			}
			Lines(CenterPatch,Edge);
		}
		else
		{
			const bool bDock=Kind==EFMCodexBroadcastSurface::Dock;
			const bool bHeading=Kind==EFMCodexBroadcastSurface::RosterHeading;
			Rect(0,0,W,H,bHeading ? FMath::Lerp(FLinearColor(0.006f,0.019f,0.035f,1),Accent,0.10f)
				: FLinearColor(0.010f,0.024f,0.038f,0.94f),
				FLinearColor(0.001f,0.005f,0.012f,bDock ? 0.82f : 0.93f));
			Lines({{6,0},{W-6,0},{W,6},{W,H-6},{W-6,H},{6,H},{0,H-6},{0,6},{6,0}},Edge);
			Lines({{10,2},{W-10,2}},Glint);
			if (bHeading)
			{
				Lines({{2,5},{2,H-5}},Accent,4);
				Lines({{10,H-1},{W-10,H-1}},Accent*FLinearColor(1,1,1,0.45f));
				Lines({{W-32,8},{W-8,8},{W-8,16}},Glint);
			}
			else if (bDock)
			{
				Lines({{18,0},{W*.42f,0},{W*.42f+10,4},{W-18,4}},Glint);
				Lines({{20,H-2},{W-20,H-2}},FLinearColor(0.04f,0.13f,0.20f,0.48f));
			}
			else
			{
				Lines({{2,30},{2,8},{8,2},{32,2}},Glint);
				Lines({{W-32,H-2},{W-8,H-2},{W-2,H-8},{W-2,H-30}},Glint);
			}
		}
		return SCompoundWidget::OnPaint(Args, Geometry, Cull, Out, Layer + 2,
			Style, ShouldBeEnabled(bEnabled));
	}
private:
	TWeakObjectPtr<UFMCodexBroadcastPanel> Owner;
};
}

TSharedRef<SWidget> UFMCodexBroadcastPanel::RebuildWidget()
{
	MyBorder = SNew(SFMCodexBroadcastPanel, this);
	if (GetChildrenCount() > 0)
		CastChecked<UBorderSlot>(GetContentSlot())->BuildSlot(MyBorder.ToSharedRef());
	return MyBorder.ToSharedRef();
}

namespace
{
class SFMCodexBroadcastButton : public SButton
{
public:
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
		const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
		const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const bool bEnabled = ShouldBeEnabled(bParentEnabled);
		const ESlateDrawEffect Effect = bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		const FSlateBrush* Brush = GetBorderImage();
		const FLinearColor Tint = InWidgetStyle.GetColorAndOpacityTint();
		FSlateDrawElement::MakeBox(Out, Layer, Geometry.ToPaintGeometry(), Brush, Effect,
			Brush->GetTint(InWidgetStyle) * Tint * GetBorderBackgroundColor().GetColor(InWidgetStyle));
		const FVector2f Size(Geometry.GetLocalSize());
		const float W=Size.X,H=Size.Y;
		const FPaintGeometry Inset = Geometry.ToPaintGeometry(
			FVector2f(FMath::Max(0.0f,W-6),FMath::Max(0.0f,H-6)),
			FSlateLayoutTransform(FVector2f(3,3)));
		const float Light = !bEnabled ? 0.015f : IsPressed() ? 0.025f : IsHovered() ? 0.22f : 0.13f;
		FSlateDrawElement::MakeGradient(Out, Layer+1, Inset,
			{FSlateGradientStop(FVector2f(0,0), FLinearColor(0.60f,0.78f,1,Light)*Tint),
			 FSlateGradientStop(FVector2f(0,H*.48f), FLinearColor(0.08f,0.16f,0.28f,0.02f)*Tint),
			 FSlateGradientStop(FVector2f(0,H-6), FLinearColor(0,0,0,IsPressed() ? 0.42f : 0.30f)*Tint)},
			Orient_Horizontal, Effect);
		const FLinearColor Rim=FLinearColor(0.55f,0.77f,1,bEnabled ? 0.25f : 0.05f)*Tint;
		FSlateDrawElement::MakeLines(Out,Layer+2,Geometry.ToPaintGeometry(),
			TArray<FVector2f>{{8,3},{W-8,3}},Effect,Rim,true,1);
		FSlateDrawElement::MakeLines(Out,Layer+2,Geometry.ToPaintGeometry(),
			TArray<FVector2f>{{7,H-3},{W-7,H-3}},Effect,FLinearColor(0,0,0,0.50f)*Tint,true,2);
		FSlateDrawElement::MakeLines(Out,Layer+2,Geometry.ToPaintGeometry(),
			TArray<FVector2f>{{14,H*.5f-4},{18,H*.5f},{14,H*.5f+4}},Effect,Rim,true,1.5f);
		return SCompoundWidget::OnPaint(Args, Geometry, Cull, Out, Layer+3, InWidgetStyle, bEnabled);
	}
};
}

TSharedRef<SWidget> UFMCodexBroadcastButton::RebuildWidget()
{
	MyButton = SNew(SFMCodexBroadcastButton)
		.OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleClicked))
		.OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandlePressed))
		.OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleReleased))
		.OnHovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleHovered))
		.OnUnhovered(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleUnhovered))
		.ButtonStyle(&GetStyle())
		.ClickMethod(GetClickMethod())
		.TouchMethod(GetTouchMethod())
		.PressMethod(GetPressMethod())
		.IsFocusable(GetIsFocusable());
	if (GetChildrenCount() > 0)
		CastChecked<UButtonSlot>(GetContentSlot())->BuildSlot(MyButton.ToSharedRef());
	return MyButton.ToSharedRef();
}
