#include "FMCodexResolutionTheaterPrototype.h"
#include "FMCodexTacticalDetailPresentation.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexOutcomePresentation.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexRollPresentationStyle.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexPitchSlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/NativeWidgetHost.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/RichTextBlock.h"
#include "Components/ScaleBox.h"
#include "Components/ScaleBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "HAL/IConsoleManager.h"
#include "Engine/DataTable.h"
#include "Components/Image.h"
#include "Fonts/FontMeasure.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SLeafWidget.h"

#define LOCTEXT_NAMESPACE "FMCodexResolutionTheater"
namespace FMCodexResolutionTheaterPrototype
{
namespace
{
#if !UE_BUILD_SHIPPING
TAutoConsoleVariable<int32> Mode(TEXT("fm.UI.ResolutionStageV2"), 1,
	TEXT("Development fallback override; default ON. 0 restores FormulaV2/legacy. Shipping always uses the adopted Cross and Free Kick theaters."), ECVF_Default);
TAutoConsoleVariable<int32> LowCrossMode(TEXT("fm.UI.ResolutionStageV2.LowCross"), 1,
	TEXT("Development Low Cross fallback; default ON. 0 restores its legacy presentation. Shipping always uses Low Cross Theater."), ECVF_Default);
TAutoConsoleVariable<int32> NearMode(TEXT("fm.UI.ResolutionStageV2.NearFreeKick"), 1,
	TEXT("Development Near Free Kick fallback; default ON. 0 restores the legacy Near flow. Shipping always uses Near Free Kick Theater."), ECVF_Default);
TAutoConsoleVariable<int32> LongMode(TEXT("fm.UI.ResolutionStageV2.LongFreeKick"), 1,
	TEXT("Development Long Free Kick fallback; default ON. 0 restores the legacy Long flow. Shipping always uses Long Free Kick Theater."), ECVF_Default);
TAutoConsoleVariable<int32> PenaltyMode(TEXT("fm.UI.ResolutionStageV2.Penalty"), 1,
	TEXT("Development Penalty fallback; default ON. 0 restores the legacy Penalty flow. Shipping always uses Penalty Theater."), ECVF_Default);
TAutoConsoleVariable<int32> CornerMode(TEXT("fm.UI.ResolutionStageV2.CornerSelection"), 1,
	TEXT("Development Corner planning fallback; default ON. 0 restores legacy selection. Shipping always uses Corner Theater."), ECVF_Default);
TAutoConsoleVariable<int32> CornerResolutionMode(TEXT("fm.UI.ResolutionStageV2.CornerResolution"), 1,
	TEXT("Development Corner resolution fallback; default ON. 0 restores legacy resolution. Shipping always uses Corner Theater."), ECVF_Default);
#endif
using K = EFMCodexUMGInlineFormulaTermKind;
using C = EFMCodexUMGInteractionCategory;
FLinearColor Color(uint8 R, uint8 G, uint8 B) { return FLinearColor::FromSRGBColor(FColor(R,G,B)); }
const FLinearColor Ink = Color(5,16,26), White = Color(235,246,250), Quiet = Color(154,180,197);
const FLinearColor Aqua = Color(68,226,216), Gold = Color(235,214,164);
const FLinearColor Mint = Color(57,231,186);
FLinearColor Alpha(FLinearColor V, float A) { V.A=A; return V; }
FName Named(const FString& Prefix, const TCHAR* Suffix) { return FName(*(Prefix+Suffix)); }
template<typename T> T* Find(UWidgetTree& Tree, FName Name) { return CastChecked<T>(Tree.FindWidget(Name)); }
void Show(UWidget& W, bool bShow) { W.SetVisibility(bShow ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed); }
void SetText(UWidgetTree& Tree, FName Name, const FString& Value) { Find<UTextBlock>(Tree,Name)->SetText(FText::FromString(Value)); }
UTextBlock* Text(UWidgetTree& Tree, FName Name, int32 Size, FLinearColor Tint=White)
{
	auto* W=Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),Name);
	FFMCodexPlayerUIStyle::Get().ApplyFlowText(*W,Size);
	W->SetColorAndOpacity(Tint); W->SetAutoWrapText(false);
	W->SetShadowOffset(FVector2D::ZeroVector);
	// Existing Roboto faces have native weight; the CJK fallback is Regular.
	// Never expand Chinese strokes with a same-color outline to simulate bold.
	auto Font=W->GetFont(); Font.TypefaceFontName=Size>=24 ? FName(TEXT("Medium")) : FName(TEXT("Regular"));
	Font.OutlineSettings.OutlineSize=0; W->SetFont(Font);
	return W;
}
USizeBox* Bounds(UWidgetTree& Tree, UWidget* Child, float Width, float Height=0)
{
	auto* W=Tree.ConstructWidget<USizeBox>();
	if (Width>0) W->SetWidthOverride(Width);
	if (Height>0) W->SetHeightOverride(Height);
	W->AddChild(Child); return W;
}
UScaleBox* Fit(UWidgetTree& Tree, UWidget* Child, EHorizontalAlignment Align=HAlign_Left)
{
	auto* W=Tree.ConstructWidget<UScaleBox>(); W->SetStretch(EStretch::ScaleToFit);
	W->SetStretchDirection(EStretchDirection::DownOnly);
	CastChecked<UScaleBoxSlot>(W->AddChild(Child))->SetHorizontalAlignment(Align); return W;
}
USizeBox* Rule(UWidgetTree& Tree, float Width, FLinearColor Tint=Quiet)
{
	auto* W=Tree.ConstructWidget<UBorder>(); W->SetPadding(FMargin(0)); W->SetBrushColor(Alpha(Tint,.30f));
	return Bounds(Tree,W,Width,1.f);
}

// Small vector marks share one stroke family; they encode no match facts.
enum class EMark { Ball, Shield, Dice, Arrow };
class STheaterMark final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STheaterMark) : _Mark(EMark::Ball), _Tint(FLinearColor::White) {}
		SLATE_ARGUMENT(EMark, Mark)
		SLATE_ARGUMENT(FLinearColor, Tint)
	SLATE_END_ARGS()
	void Construct(const FArguments& A) { Mark=A._Mark; MarkTint=A._Tint; }
	FVector2D ComputeDesiredSize(float) const override { return FVector2D(40); }
	int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
		int32 Layer, const FWidgetStyle& Style, bool) const override
	{
		const FVector2D Size=G.GetLocalSize(); const FLinearColor Tint=MarkTint*Style.GetColorAndOpacityTint();
		auto Line=[&](std::initializer_list<FVector2D> Points)
		{
			TArray<FVector2D> P; for (const auto& V:Points) P.Add(V*Size/48.f);
			FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),P,ESlateDrawEffect::None,Tint,true,1.8f);
		};
		if (Mark==EMark::Dice)
		{
			Line({{24,3},{43,14},{43,35},{24,46},{5,35},{5,14},{24,3}});
			Line({{5,14},{24,25},{43,14}}); Line({{24,25},{24,46}});
			const FSlateRoundedBoxBrush Dot(Tint,2.f);
			for (const auto& V:{FVector2D(24,13),{13,25},{16,34},{33,26},{35,34}})
				FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(Size/12.f,FSlateLayoutTransform((V-FVector2D(2))*Size/48.f)),&Dot,ESlateDrawEffect::None,Tint);
		}
		else if (Mark==EMark::Shield)
		{
			Line({{24,3},{41,10},{39,28},{34,36},{24,44},{14,36},{9,28},{7,10},{24,3}});
			Line({{24,10},{34,15},{32,28},{24,36},{16,28},{14,15},{24,10}});
		}
		else if (Mark==EMark::Arrow) Line({{18,9},{32,24},{18,39}});
		else
		{
			TArray<FVector2D> Circle; for (int32 I=0;I<=32;++I) { const float A=I*2.f*PI/32; Circle.Add((FVector2D(24)+FVector2D(FMath::Cos(A),FMath::Sin(A))*20)*Size/48.f); }
			FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),Circle,ESlateDrawEffect::None,Tint,true,1.6f);
			Line({{24,14},{34,21},{30,33},{18,33},{14,21},{24,14}});
			Line({{24,14},{24,4}}); Line({{34,21},{43,16}}); Line({{30,33},{36,40}});
			Line({{18,33},{12,40}}); Line({{14,21},{5,16}});
		}
		return Layer;
	}
private:
	EMark Mark=EMark::Ball; FLinearColor MarkTint;
};
UWidget* Mark(UWidgetTree& Tree, FName Name, EMark Kind, float Size, FLinearColor Tint=White)
{
	auto* W=Tree.ConstructWidget<UNativeWidgetHost>(UNativeWidgetHost::StaticClass(),Name);
	W->SetContent(SNew(STheaterMark).Mark(Kind).Tint(Tint)); W->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Bounds(Tree,W,Size,Size);
}

class STheaterGlass final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STheaterGlass) {} SLATE_END_ARGS()
	void Construct(const FArguments&) {}
	FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
		int32 Layer, const FWidgetStyle& Style, bool) const override
	{
		const FVector2D Size=G.GetLocalSize(); const auto Tint=Style.GetColorAndOpacityTint();
		const FSlateRoundedBoxBrush Base(Alpha(Color(5,18,30),.91f),9.f,Alpha(Color(130,180,204),.35f),1.f);
		FSlateDrawElement::MakeBox(Out,Layer,G.ToPaintGeometry(),&Base,ESlateDrawEffect::None,Base.GetTint(Style)*Tint);
		TArray<FSlateGradientStop> Stops;
		Stops.Emplace(FVector2D(0,0),Alpha(Color(70,123,150),.22f)*Tint);
		Stops.Emplace(FVector2D(0,Size.Y*.40f),Alpha(Color(23,53,73),.08f)*Tint);
		Stops.Emplace(FVector2D(0,Size.Y-12),Alpha(Color(5,18,30),.15f)*Tint);
		FSlateDrawElement::MakeGradient(Out,Layer+1,G.ToPaintGeometry(Size-FVector2D(12),FSlateLayoutTransform(FVector2D(6))),Stops,Orient_Horizontal);
		TArray<FVector2D> Edge{{12,1},{Size.X-12,1}};
		FSlateDrawElement::MakeLines(Out,Layer+2,G.ToPaintGeometry(),Edge,ESlateDrawEffect::None,Alpha(White,.23f)*Tint,true,1.f);
		return Layer+2;
	}
};
UOverlay* Glass(UWidgetTree& Tree, UBorder& Frame, FName SurfaceName=NAME_None)
{
	Frame.SetPadding(FMargin(0)); Frame.SetBrushColor(FLinearColor::Transparent);
	auto* Layers=Tree.ConstructWidget<UOverlay>(); Frame.AddChild(Layers);
	auto* Surface=Tree.ConstructWidget<UNativeWidgetHost>(UNativeWidgetHost::StaticClass(),SurfaceName); Surface->SetContent(SNew(STheaterGlass));
	auto* Slot=Layers->AddChildToOverlay(Surface); Slot->SetHorizontalAlignment(HAlign_Fill); Slot->SetVerticalAlignment(VAlign_Fill);
	return Layers;
}

// Generated original athletes, imported as a UI atlas. Each UV island retains
// its native aspect ratio; the figures are decorative and have no player identity.
UWidget* Athlete(UWidgetTree& Tree, FName Name, UTexture2D* Atlas, bool bAttack)
{
	auto* W=Tree.ConstructWidget<UImage>(UImage::StaticClass(),Name);
	FSlateBrush Brush; Brush.SetResourceObject(Atlas);
	const FVector2D Min=bAttack?FVector2D(0,.09):FVector2D(.46,.17);
	const FVector2D Max=bAttack?FVector2D(.45,.87):FVector2D(1,.87);
	Brush.SetUVRegion(FBox2f(FVector2f(Min),FVector2f(Max)));
	Brush.ImageSize=(Max-Min)*FVector2D(1448,1086);
	W->SetBrush(Brush); W->SetColorAndOpacity(Alpha(Color(133,179,204),.24f));
	W->SetVisibility(ESlateVisibility::HitTestInvisible);
	return W;
}

// Apply emphasis to numeric tokens in the already-disclosed reason sentence.
// This formatter does not classify reasons, compare values or calculate facts.
FString ReasonMarkup(const FString& Plain)
{
	FString Result;
	for (int32 I=0;I<Plain.Len();)
	{
		if (FChar::IsDigit(Plain[I]))
		{
			Result+=TEXT("<Value>");
			do { Result.AppendChar(Plain[I++]); }
			while (I<Plain.Len() && (FChar::IsDigit(Plain[I]) || Plain[I]==TEXT('.')));
			Result+=TEXT("</>");
		}
		else
		{
			const TCHAR Ch=Plain[I++];
			if (Ch==TEXT('&')) Result+=TEXT("&amp;");
			else if (Ch==TEXT('<')) Result+=TEXT("&lt;");
			else if (Ch==TEXT('>')) Result+=TEXT("&gt;");
			else Result.AppendChar(Ch);
		}
	}
	return Result;
}

// Reuse the live Match Board stadium asset. Vertex alpha dissolves its upper
// lights/stands into the real turf below, without a second pitch or render target.
class STheaterBackdrop final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STheaterBackdrop) {} SLATE_ARGUMENT(FSlateBrush, Stadium) SLATE_END_ARGS()
	void Construct(const FArguments& A) { Stadium=A._Stadium; }
	FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
	int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&, FSlateWindowElementList& Out,
		int32 Layer, const FWidgetStyle& Style, bool) const override
	{
		const FVector2D Size=G.GetLocalSize(); const auto Tint=Style.GetColorAndOpacityTint();
		if (Stadium.GetResourceObject())
		{
			TArray<FSlateVertex> Vertices; TArray<SlateIndex> Indices;
			const float Ys[]{0,.18f,.34f,.48f,.64f};
			const float Opacities[]{.98f,.95f,.80f,.30f,0};
			for (int32 Y=0;Y<5;++Y) for (int32 X=0;X<3;++X)
			{
				const float U=X*.5f;
				const auto Light=Alpha(Color(190,216,241),Opacities[Y]*(X==1?.92f:1.f))*Tint;
				Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),
					FVector2f(Size*FVector2D(U,Ys[Y])),FVector2f(U,Ys[Y]),Light.ToFColor(true)));
			}
			for (int32 Y=0;Y<4;++Y) for (int32 X=0;X<2;++X)
			{
				const SlateIndex I=Y*3+X; Indices.Append({I,static_cast<SlateIndex>(I+1),static_cast<SlateIndex>(I+3),
					static_cast<SlateIndex>(I+1),static_cast<SlateIndex>(I+4),static_cast<SlateIndex>(I+3)});
			}
			FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Stadium),Vertices,Indices,nullptr,0,0);
		}
		TArray<FSlateGradientStop> Stops;
		Stops.Emplace(FVector2D(0,0),Alpha(Color(3,12,22),.44f)*Tint);
		Stops.Emplace(FVector2D(0,Size.Y*.42f),Alpha(Color(8,30,47),.61f)*Tint);
		Stops.Emplace(FVector2D(0,Size.Y),Alpha(Color(5,21,29),.43f)*Tint);
		// Slate names the orientation of the stop lines, not the gradient direction.
		FSlateDrawElement::MakeGradient(Out,Layer+1,G.ToPaintGeometry(),Stops,Orient_Horizontal);
		// Diffused arena lights in the upper corners. Nested rounded washes have
		// no rectangular cutoff behind the title or reading columns.
		for (int32 I=0; I<64; ++I)
		{
			const float T=1.f-I/72.f;
			const FVector2D LightSize(Size.X*.42f*T,Size.Y*.40f*T);
			const FSlateRoundedBoxBrush Wash(FLinearColor::White,LightSize.Y*.5f);
			for (const float X:{.01f,.99f})
				FSlateDrawElement::MakeBox(Out,Layer+2,G.ToPaintGeometry(LightSize,
					FSlateLayoutTransform(FVector2D(Size.X*X,Size.Y*.17f)-LightSize*.5f)),&Wash,
					ESlateDrawEffect::None,Alpha(Color(116,179,215),.006f)*Tint);
		}
		Stops.Reset();
		Stops.Emplace(FVector2D(0,0),Alpha(Ink,.48f)*Tint);
		Stops.Emplace(FVector2D(Size.X*.24f,0),Alpha(Ink,0)*Tint);
		Stops.Emplace(FVector2D(Size.X*.76f,0),Alpha(Ink,0)*Tint);
		Stops.Emplace(FVector2D(Size.X,0),Alpha(Ink,.48f)*Tint);
		FSlateDrawElement::MakeGradient(Out,Layer+3,G.ToPaintGeometry(),Stops,Orient_Vertical);
		return Layer+3;
	}
private: FSlateBrush Stadium;
};

class STheaterDivider final : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(STheaterDivider) {} SLATE_END_ARGS()
	void Construct(const FArguments&) {}
	FVector2D ComputeDesiredSize(float) const override { return FVector2D(640,10); }
	int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& Out,
		int32 Layer,const FWidgetStyle& Style,bool) const override
	{
		const auto Size=G.GetLocalSize(); const auto Tint=Style.GetColorAndOpacityTint();
		for (int32 I=0;I<4;++I)
		{
			const float Height=1.f+I*2.f, Opacity=I==0?.66f:.055f;
			TArray<FSlateGradientStop> Stops;
			Stops.Emplace(FVector2D(0,0),Alpha(Aqua,0));
			Stops.Emplace(FVector2D(Size.X*.40f,0),Alpha(Aqua,Opacity*.24f)*Tint);
			Stops.Emplace(FVector2D(Size.X*.5f,0),Alpha(I==0?White:Mint,Opacity)*Tint);
			Stops.Emplace(FVector2D(Size.X*.60f,0),Alpha(Aqua,Opacity*.24f)*Tint);
			Stops.Emplace(FVector2D(Size.X,0),Alpha(Aqua,0));
			FSlateDrawElement::MakeGradient(Out,Layer,G.ToPaintGeometry(FVector2D(Size.X,Height),
				FSlateLayoutTransform(FVector2D(0,(Size.Y-Height)*.5f))),Stops,Orient_Vertical);
		}
		return Layer;
	}
};

UButton* Button(UWidgetTree& Tree, FName Name, FName LabelName, const FText& Label)
{
	auto* W=Tree.ConstructWidget<UButton>(UButton::StaticClass(),Name);
	FButtonStyle Style;
	Style.SetNormal(FSlateRoundedBoxBrush(Mint,9.f,Alpha(White,.55f),1.f));
	Style.SetHovered(FSlateRoundedBoxBrush(Color(116,249,212),9.f,White,2.f));
	Style.SetPressed(FSlateRoundedBoxBrush(Color(38,184,158),9.f));
	Style.SetDisabled(FSlateRoundedBoxBrush(Color(39,62,72),5.f));
	Style.SetNormalPadding(FMargin(18,6)); Style.SetPressedPadding(FMargin(18,8,18,4)); W->SetStyle(Style);
	auto* Content=Tree.ConstructWidget<UHorizontalBox>();
	if (Name==TEXT("TheaterContinue"))
	{
		auto* RollLeading=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterRollLeading"));
		RollLeading->AddChildToHorizontalBox(Mark(Tree,TEXT("TheaterDiceIcon"),EMark::Dice,36,Ink))->SetVerticalAlignment(VAlign_Center);
		Content->AddChildToHorizontalBox(RollLeading)->SetVerticalAlignment(VAlign_Center);
		auto* Divider=Tree.ConstructWidget<UBorder>(); Divider->SetBrushColor(Alpha(Ink,.38f)); Divider->SetPadding(FMargin(0));
		auto* DividerSlot=RollLeading->AddChildToHorizontalBox(Bounds(Tree,Divider,1,34)); DividerSlot->SetPadding(FMargin(18,0)); DividerSlot->SetVerticalAlignment(VAlign_Center);
	}
	auto* T=Text(Tree,LabelName,26,Ink); T->SetText(Label); T->SetJustification(ETextJustify::Center);
	if (Name==TEXT("TheaterNearDirect") || Name==TEXT("TheaterNearCombination"))
	{
		// Peer methods share the native Regular face, including the CJK fallback.
		auto MethodFont=T->GetFont(); MethodFont.TypefaceFontName=TEXT("Regular"); T->SetFont(MethodFont);
	}
	Content->AddChildToHorizontalBox(T)->SetVerticalAlignment(VAlign_Center);
	if (Name==TEXT("TheaterContinue"))
	{
		auto* Next=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterNextTrailing"));
		auto* NextSlot=Next->AddChildToHorizontalBox(Mark(Tree,TEXT("TheaterNextIcon"),EMark::Arrow,26,Ink));
		NextSlot->SetPadding(FMargin(20,0,0,0)); NextSlot->SetVerticalAlignment(VAlign_Center);
		Content->AddChildToHorizontalBox(Next)->SetVerticalAlignment(VAlign_Center);
	}
	auto* Slot=CastChecked<UButtonSlot>(W->AddChild(Content)); Slot->SetHorizontalAlignment(HAlign_Center); Slot->SetVerticalAlignment(VAlign_Center);
	return W;
}
// One typographic baseline for the expression and the larger result. Font metrics
// keep integers, decimals and ? aligned without offsets tied to a particular value.
constexpr float EquationBaseline = 68.f;
void AlignEquationText(UTextBlock& TextBlock, float Baseline = EquationBaseline)
{
	auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const auto& Font = TextBlock.GetFont();
	const float Ascent = Measure->GetMaxCharacterHeight(Font) + Measure->GetBaseline(Font);
	auto* Slot = CastChecked<UOverlaySlot>(TextBlock.Slot);
	Slot->SetHorizontalAlignment(HAlign_Center);
	Slot->SetVerticalAlignment(VAlign_Top);
	Slot->SetPadding(FMargin(0, Baseline - Ascent, 0, 0));
}
USizeBox* EquationCell(UWidgetTree& Tree, UTextBlock& TextBlock, float Width)
{
	auto* Layer = Tree.ConstructWidget<UOverlay>();
	Layer->AddChildToOverlay(&TextBlock);
	AlignEquationText(TextBlock);
	return Bounds(Tree, Layer, Width, 86);
}
USizeBox* RollOperand(UWidgetTree& Tree, const FString& Prefix)
{
	// A stable 68x76 allocation hosts ?, Theater Roll v2, then the disclosed digit.
	auto* Unknown=Tree.ConstructWidget<UOverlay>(UOverlay::StaticClass(),Named(Prefix,TEXT("UnknownSlot")));
	for (const auto Suffix:{TEXT("Pending"),TEXT("RollValue")})
	{
		auto* Digit=Text(Tree,Named(Prefix,Suffix),40,FString(Suffix)==TEXT("Pending")?Quiet:FMCodexRollPresentationStyle::AuthoritativeLandedValue()); Digit->SetJustification(ETextJustify::Center);
		if (FString(Suffix)==TEXT("Pending")) Digit->SetText(FText::FromString(TEXT("?")));
		Unknown->AddChildToOverlay(Digit); AlignEquationText(*Digit,EquationBaseline-10);
	}
	auto* Reel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),Named(Prefix,TEXT("Reel")));
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::TheaterInline);
	auto* ReelHost=Tree.ConstructWidget<UScaleBox>(UScaleBox::StaticClass(),Named(Prefix,TEXT("ReelHost")));
	ReelHost->SetStretch(EStretch::ScaleToFit); ReelHost->AddChild(Reel);
	auto* ReelSlot=Unknown->AddChildToOverlay(ReelHost); ReelSlot->SetHorizontalAlignment(HAlign_Fill); ReelSlot->SetVerticalAlignment(VAlign_Fill);
	ReelSlot->SetPadding(FMargin(0));
	if (Prefix==TEXT("TheaterAttack"))
	{
		auto* AttackRollCaption=Text(Tree,TEXT("TheaterAttackRollCaption"),14,Quiet);
		AttackRollCaption->SetText(LOCTEXT("AttackRollCaption","进攻掷点"));
		auto* CaptionSlot=Unknown->AddChildToOverlay(AttackRollCaption);
		CaptionSlot->SetHorizontalAlignment(HAlign_Center); CaptionSlot->SetVerticalAlignment(VAlign_Top);
		CaptionSlot->SetPadding(FMargin(0,-10,0,0)); Show(*AttackRollCaption,false);
	}
	return Bounds(Tree,Unknown,68,76);
}
void BuildSide(UWidgetTree& Tree, UHorizontalBox& Middle, const FString& Prefix, UTexture2D* Atlas)
{
	auto* Panel=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Panel")));
	auto* Layers=Glass(Tree,*Panel);
	const bool bAttack=Prefix==TEXT("TheaterAttack");
	auto* Player=Athlete(Tree,Named(Prefix,TEXT("Silhouette")),Atlas,bAttack);
	auto* PlayerBounds=Bounds(Tree,Fit(Tree,Player,HAlign_Center),138,178);
	PlayerBounds->Rename(*Named(Prefix,TEXT("SilhouetteBounds")).ToString(),&Tree);
	auto* PlayerSlot=Layers->AddChildToOverlay(PlayerBounds);
	PlayerSlot->SetHorizontalAlignment(bAttack?HAlign_Left:HAlign_Right); PlayerSlot->SetVerticalAlignment(VAlign_Bottom);
	PlayerSlot->SetPadding(FMargin(10,0,10,14));
	auto* Accent=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Accent")));
	Accent->SetPadding(FMargin(0)); Accent->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White,FVector4(5.f,0.f,0.f,5.f)));
	auto* AccentSlot=Layers->AddChildToOverlay(Bounds(Tree,Accent,5)); AccentSlot->SetHorizontalAlignment(HAlign_Left); AccentSlot->SetVerticalAlignment(VAlign_Fill); AccentSlot->SetPadding(FMargin(0));
	// Separate columns guarantee every text/equation bounds stays clear of the athlete.
	auto* Body=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),Named(Prefix,TEXT("SafeContent")));
	auto* BodySlot=Layers->AddChildToOverlay(Body); BodySlot->SetHorizontalAlignment(HAlign_Fill); BodySlot->SetVerticalAlignment(VAlign_Fill);
	BodySlot->SetPadding(bAttack?FMargin(160,16,22,16):FMargin(22,16,160,16));
	auto* Head=Tree.ConstructWidget<UHorizontalBox>();
	auto* IconSlot=Head->AddChildToHorizontalBox(Mark(Tree,NAME_None,Prefix==TEXT("TheaterAttack")?EMark::Ball:EMark::Shield,30)); IconSlot->SetPadding(FMargin(0,0,14,0)); IconSlot->SetVerticalAlignment(VAlign_Center);
	Head->AddChildToHorizontalBox(Text(Tree,Named(Prefix,TEXT("Side")),32))->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	auto* Badge=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("Badge")));
	Badge->SetPadding(FMargin(13,3)); Badge->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White,4.f));
	auto* Active=Text(Tree,Named(Prefix,TEXT("Active")),17,Ink); Badge->AddChild(Active);
	Head->AddChildToHorizontalBox(Badge)->SetVerticalAlignment(VAlign_Center);
	Body->AddChildToVerticalBox(Head);
	Body->AddChildToVerticalBox(Rule(Tree,0,Quiet))->SetPadding(FMargin(0,8,0,10));
	auto* People=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),Named(Prefix,TEXT("People")));
	Body->AddChildToVerticalBox(People);
	auto* Value=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),Named(Prefix,TEXT("Value")));
	auto* Numbers=Tree.ConstructWidget<UHorizontalBox>();
	auto* Expression=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),Named(Prefix,TEXT("Expression")));
	auto* BaseHover=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("BaseHover")));
	BaseHover->SetBrushColor(FLinearColor::Transparent); BaseHover->SetPadding(FMargin(0));
	BaseHover->SetVisibility(ESlateVisibility::Visible);
	auto* BaseContent=Tree.ConstructWidget<UOverlay>(); BaseHover->AddChild(Bounds(Tree,BaseContent,84,86));
	auto* Number=Text(Tree,Named(Prefix,TEXT("Number")),34,Quiet); Number->SetJustification(ETextJustify::Center);
	BaseContent->AddChildToOverlay(Number); AlignEquationText(*Number);
	auto* Underline=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(Prefix,TEXT("BaseUnderline")));
	Underline->SetPadding(FMargin(0)); Underline->SetBrushColor(Alpha(Quiet,.65f));
	auto* LineBounds=Bounds(Tree,Underline,54,2);
	LineBounds->Rename(*Named(Prefix,TEXT("UnderlineBounds")).ToString(),&Tree);
	auto* LineSlot=BaseContent->AddChildToOverlay(LineBounds);
	LineSlot->SetHorizontalAlignment(HAlign_Center); LineSlot->SetVerticalAlignment(VAlign_Top);
	LineSlot->SetPadding(FMargin(0,EquationBaseline+5,0,0));
	// A native Slate tooltip, owned by this hover target, never a gameplay surface.
	auto* Tip=Tree.ConstructWidget<UBorder>(); Tip->SetPadding(FMargin(18,14));
	Tip->SetBrush(FSlateRoundedBoxBrush(Color(8,26,40),7.f,Alpha(Quiet,.65f),1.f)); Tip->SetBrushColor(FLinearColor::White);
	auto* TipText=Text(Tree,Named(Prefix,TEXT("BaseExplanation")),17); TipText->SetAutoWrapText(true); Tip->AddChild(Bounds(Tree,TipText,410.f));
	BaseHover->SetToolTip(Tip);
	Expression->AddChildToHorizontalBox(BaseHover)->SetVerticalAlignment(VAlign_Bottom);
	auto* Plus=Text(Tree,Named(Prefix,TEXT("Plus")),24,Alpha(Quiet,.8f)); Plus->SetText(FText::FromString(TEXT("+"))); Plus->SetJustification(ETextJustify::Center);
	Expression->AddChildToHorizontalBox(EquationCell(Tree,*Plus,24))->SetVerticalAlignment(VAlign_Bottom);
	Expression->AddChildToHorizontalBox(RollOperand(Tree,Prefix))->SetVerticalAlignment(VAlign_Bottom);
	auto* Equal=Text(Tree,Named(Prefix,TEXT("Equal")),24,Alpha(Quiet,.8f)); Equal->SetText(FText::FromString(TEXT("="))); Equal->SetJustification(ETextJustify::Center);
	Expression->AddChildToHorizontalBox(EquationCell(Tree,*Equal,24))->SetVerticalAlignment(VAlign_Bottom);
	Numbers->AddChildToHorizontalBox(Expression)->SetVerticalAlignment(VAlign_Bottom);
	auto* Result=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),Named(Prefix,TEXT("ResultColumn")));
	auto* Caption=Text(Tree,Named(Prefix,TEXT("ValueLabel")),15,Quiet); Caption->SetJustification(ETextJustify::Center);
	Result->AddChildToVerticalBox(Caption);
	auto* Final=Text(Tree,Named(Prefix,TEXT("FinalNumber")),66);
	auto FinalFont=Final->GetFont(); FinalFont.TypefaceFontName=TEXT("Bold"); Final->SetFont(FinalFont);
	Final->SetJustification(ETextJustify::Center);
	// Preserve the approved 154x86 result fit and native type size. Align its
	// scaled font baseline to the expression; do not clip a larger line box.
	auto* FinalFit=Fit(Tree,Final,HAlign_Center);
	FinalFit->Rename(*Named(Prefix,TEXT("FinalFit")).ToString(),&Tree);
	Result->AddChildToVerticalBox(Bounds(Tree,FinalFit,154,86));
	auto* ResultSlot=Numbers->AddChildToHorizontalBox(Result);
	ResultSlot->SetVerticalAlignment(VAlign_Bottom); ResultSlot->SetPadding(FMargin(12,0,0,0));
	Value->AddChildToVerticalBox(Fit(Tree,Numbers,HAlign_Center));
	Body->AddChildToVerticalBox(Value)->SetPadding(FMargin(0,14,0,0));
	if (bAttack)
	{
		auto* Pair=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterPair"));
		Pair->AddChildToHorizontalBox(RollOperand(Tree,TEXT("TheaterPairA")))->SetVerticalAlignment(VAlign_Bottom);
		auto* PlusPair=Text(Tree,NAME_None,24,Quiet); PlusPair->SetText(FText::FromString(TEXT("+")));
		Pair->AddChildToHorizontalBox(EquationCell(Tree,*PlusPair,24))->SetVerticalAlignment(VAlign_Bottom);
		Pair->AddChildToHorizontalBox(RollOperand(Tree,TEXT("TheaterPairB")))->SetVerticalAlignment(VAlign_Bottom);
		auto* EqualPair=Text(Tree,NAME_None,24,Quiet); EqualPair->SetText(FText::FromString(TEXT("=")));
		Pair->AddChildToHorizontalBox(EquationCell(Tree,*EqualPair,24))->SetVerticalAlignment(VAlign_Bottom);
		auto* Total=Text(Tree,TEXT("TheaterPairTotal"),54,Gold);
		Pair->AddChildToHorizontalBox(EquationCell(Tree,*Total,116))->SetVerticalAlignment(VAlign_Bottom);
		Body->AddChildToVerticalBox(Pair)->SetPadding(FMargin(0,14,0,0));
	}

	auto* PanelBounds=Bounds(Tree,Panel,600.f); PanelBounds->Rename(*Named(Prefix,TEXT("PanelBounds")).ToString(),&Tree);
	Middle.AddChildToHorizontalBox(PanelBounds)->SetVerticalAlignment(VAlign_Center);
}
// The candidate pool and eligibility are safe facts; this function only selects a
// presentation subject. It never derives eligibility from the Full Card attributes.
void RefreshTakerHelper(UWidgetTree& Tree, FTakerInspection& Inspection)
{
 if (!Inspection.bActive) return;
 const auto& Cells=Find<UFMCodexCardRackWidget>(Tree,TEXT("TheaterTakers"))->GetPresentation().Cells;
 const auto* Selected=Cells.FindByPredicate([](const auto& Cell){return Cell.bSetPieceSelected;});
 if (Inspection.bOrderedMultiSelect)
  for (const auto& Cell:Cells)
   if (Cell.bSetPieceSelected && (!Selected || Cell.SetPieceSelectionOrder>Selected->SetPieceSelectionOrder)) Selected=&Cell;
 const auto* Hovered=Cells.FindByPredicate([&](const auto& Cell){return Cell.Card.CardId==Inspection.HoveredId;});
 const auto* Subject=Hovered?Hovered:Selected;
 auto* Full=Find<UFMCodexPlayerCardWidget>(Tree,TEXT("TheaterTakerFullCard"));
 Full->RefreshFromPresentation(Subject?Subject->Card:FFMCodexUMGCardViewModel(),EFMCodexPlayerCardPresentationMode::InteractionChoice);
 Full->SetVisibility(Subject?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 Show(*Tree.FindWidget(TEXT("TheaterTakerPlaceholder")),!Subject);
 // Ordered multi-select reuses inspection, but its stage owns its own rule copy.
 if (Inspection.bOrderedMultiSelect) return;
 const FText DirectRule=Inspection.bPenalty?LOCTEXT("PenaltyTakerDirectRule","常规点球：取射门 / 传球较高值，对抗门将预判（门将预判 -3）") : Inspection.bLongFreeKick?LOCTEXT("LongTakerDirectRule","直接射门：远射对抗门将站位 + 2；进攻掷点 1–2 直接射偏") : LOCTEXT("TakerDirectRule","直接射门：取射门 / 传球较高值，与对方门将手控球进行判定");
 const FText CombinationRule=Inspection.bPenalty?LOCTEXT("PenaltyTakerPanenkaRule","勺子点球：掷一枚骰子；1 射失，2–6 进球") : Inspection.bLongFreeKick?LOCTEXT("LongTakerPowerRule","重炮轰门：两枚骰子总和 ≥ 11 进球，无属性门槛") : LOCTEXT("TakerCombinationRule","战术配合：需射门 + 传球 ≥ 8；两枚骰子总和 ≥ 9 进球");
 const auto PlayerName=[](const FFMCodexUMGCardViewModel& Card)
 { return Card.IdentityLabel.IsEmpty()?LOCTEXT("TakerFallback","球员"):FText::FromString(Card.IdentityLabel); };
 // Selection owns the subtitle; hovering only changes the inspected candidate and rule suffix.
 Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(Selected
  ?FText::Format(LOCTEXT("TakerSelectedSubtitle","已选主罚球员：{0}"),PlayerName(Selected->Card))
  :LOCTEXT("TakerSelectionSubtitle","选择主罚球员"));
 FText Secondary=CombinationRule;
 if (Subject && !Inspection.bLongFreeKick && !Inspection.bPenalty)
 {
  const bool* Eligibility=Inspection.CombinationEligibility.Find(Subject->Card.CardId);
  const FText Status=Eligibility?(*Eligibility?LOCTEXT("TakerEligible","可用"):LOCTEXT("TakerIneligible","不可用")):LOCTEXT("TakerUnknown","资格暂不可用");
  Secondary=FText::Format(LOCTEXT("TakerCombinationCandidate","{0}，{1}{2}"),CombinationRule,PlayerName(Subject->Card),Status);
 }
 Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(DirectRule);
 Find<UTextBlock>(Tree,TEXT("TheaterReasonSecondary"))->SetText(Secondary);
 Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),true);
}
void RefreshSide(UWidgetTree& Tree, const FString& Prefix, const FFMCodexUMGInlineFormulaRowViewModel& Row,
	bool bFormula, bool bActive, bool bReveal, bool bWinner, bool bRollOnly = false)
{
	SetText(Tree,Named(Prefix,TEXT("Side")),Row.SideLabel);
	auto* PlayerBounds=Find<USizeBox>(Tree,Named(Prefix,TEXT("SilhouetteBounds")));
	PlayerBounds->SetHeightOverride(bFormula || bRollOnly?178.f:128.f); PlayerBounds->SetWidthOverride(138.f);
	Find<UTextBlock>(Tree,Named(Prefix,TEXT("Active")))->SetText(bWinner ? LOCTEXT("Winner","获胜") : LOCTEXT("Active","当前"));
	Find<UBorder>(Tree,Named(Prefix,TEXT("Badge")))->SetVisibility(bActive || bWinner ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	Find<UBorder>(Tree,Named(Prefix,TEXT("Badge")))->SetBrushColor(bWinner?Mint:Aqua);
	Find<UBorder>(Tree,Named(Prefix,TEXT("Accent")))->SetBrushColor(bWinner?Mint:bActive?Aqua:Alpha(Quiet,.32f));
	auto* People=Find<UHorizontalBox>(Tree,Named(Prefix,TEXT("People")));
	while (People->GetChildrenCount()<Row.Participants.Num())
	{
		const int32 I=People->GetChildrenCount(); auto* Person=Tree.ConstructWidget<UVerticalBox>();
		Person->AddChildToVerticalBox(Text(Tree,FName(*(Prefix+FString::Printf(TEXT("Role%d"),I))),15,Quiet));
		auto* Name=Text(Tree,FName(*(Prefix+FString::Printf(TEXT("Name%d"),I))),23);
		Person->AddChildToVerticalBox(Fit(Tree,Name))->SetPadding(FMargin(0,4,0,0));
		auto* Slot=People->AddChildToHorizontalBox(Person); Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); Slot->SetPadding(FMargin(0,0,12,0));
	}
	for (int32 I=0;I<People->GetChildrenCount();++I)
	{
		const bool bExists=Row.Participants.IsValidIndex(I); Show(*People->GetChildAt(I),bExists);
		SetText(Tree,FName(*(Prefix+FString::Printf(TEXT("Role%d"),I))),bExists ? Row.Participants[I].RoleLabel : FString());
		SetText(Tree,FName(*(Prefix+FString::Printf(TEXT("Name%d"),I))),bExists ? Row.Participants[I].PlayerName : FString());
	}
	const bool bValueLane = bFormula || bRollOnly;
	Show(*Tree.FindWidget(Named(Prefix,TEXT("Value"))),bValueLane);
	// A skipped DirectShot keeps the same real attack operand and motion. Its
	// candidate Base/Final are not an executed Formula, even when already known.
	// Keep the ordinary attack allocation through ImmediateMiss. Hidden removes
	// comparison content and hit targets without shrinking the card or silhouette.
	Find<UBorder>(Tree,Named(Prefix,TEXT("BaseHover")))->SetVisibility(
		bRollOnly ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	for (auto* Part:{Tree.FindWidget(Named(Prefix,TEXT("Plus")))->GetParent()->GetParent(),
		Tree.FindWidget(Named(Prefix,TEXT("Equal")))->GetParent()->GetParent(),
		Cast<UPanelWidget>(Tree.FindWidget(Named(Prefix,TEXT("ResultColumn"))))})
		Part->SetVisibility(bRollOnly?ESlateVisibility::Hidden:ESlateVisibility::HitTestInvisible);
	const bool bFinal=Row.bDisplayedResultResolved && Row.bDisplayedResultIsFinalValue;
	// KnownNonRollSubtotalLabel includes a supporting-copy prefix. Format the
	// provided scalar for the numeric anchor; never sum operands or subtract dice.
	SetText(Tree,Named(Prefix,TEXT("Number")),bFormula && Row.bKnownNonRollSubtotalResolved
		? FText::AsNumber(Row.KnownNonRollSubtotal).ToString() : FString(TEXT("?")));
	auto* Number=Find<UTextBlock>(Tree,Named(Prefix,TEXT("Number")));
	Number->SetColorAndOpacity(Quiet);
	auto NumberFont=Number->GetFont(); NumberFont.OutlineSettings.OutlineSize=0; Number->SetFont(NumberFont);
	const float NumberWidth=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Number->GetText(),NumberFont).X;
	Find<USizeBox>(Tree,Named(Prefix,TEXT("UnderlineBounds")))->SetWidthOverride(FMath::Clamp(NumberWidth*.84f,22.f,78.f));
	Find<UTextBlock>(Tree,Named(Prefix,TEXT("ValueLabel")))->SetText(bFinal ? LOCTEXT("Final","最终值") : LOCTEXT("Current","当前值"));
	const auto* Roll=Row.Terms.FindByPredicate([](const auto& Term) { return Term.Kind==K::RawRoll; });
	// Each opposed row has one die. Follow its displayed reveal owner: Near's
	// separate Attack/Defense streams both use index 0, unlike Formula term indices.
	// The opponent keeps its settled operand; reveal identity/dedupe remains in Screen.
	const bool bReel=bValueLane && bActive && bReveal && Roll;
	const bool bResolved=bValueLane && Roll && Roll->bResolved;
	Show(*Tree.FindWidget(Named(Prefix,TEXT("ReelHost"))),bReel);
	Show(*Tree.FindWidget(Named(Prefix,TEXT("Pending"))),bValueLane && !bReel && !bResolved);
	Show(*Tree.FindWidget(Named(Prefix,TEXT("RollValue"))),bResolved && !bReel);
	SetText(Tree,Named(Prefix,TEXT("RollValue")),bResolved ? FString::FromInt(Roll->RawD6) : FString());
	auto* RollValue=Find<UTextBlock>(Tree,Named(Prefix,TEXT("RollValue")));
	RollValue->SetColorAndOpacity(FMCodexRollPresentationStyle::AuthoritativeLandedValue());
	auto RollFont=RollValue->GetFont(); RollFont.OutlineSettings.OutlineSize=0; RollValue->SetFont(RollFont);
	SetText(Tree,Named(Prefix,TEXT("FinalNumber")),Row.bDisplayedResultResolved ? Row.DisplayedResultLabel : FString(TEXT("?")));
	auto* Final=Find<UTextBlock>(Tree,Named(Prefix,TEXT("FinalNumber")));
	Final->SetColorAndOpacity(bFinal ? Gold : White);
	auto FinalFont=Final->GetFont(); FinalFont.Size=bFinal?66:58; Final->SetFont(FinalFont);
	auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
	const FVector2D TextSize=Measure->Measure(Final->GetText(),FinalFont);
	const float FitScale=FMath::Min(1.f,FMath::Min(154.f/FMath::Max(1.f,float(TextSize.X)),86.f/FMath::Max(1.f,float(TextSize.Y))));
	const float Baseline=(TextSize.Y+Measure->GetBaseline(FinalFont))*FitScale;
	Find<UScaleBox>(Tree,Named(Prefix,TEXT("FinalFit")))->SetRenderTranslation(FVector2D(0,EquationBaseline-Baseline));
	// Explain only the provided non-roll facts. No local arithmetic or hidden dice.
	FString Explanation=LOCTEXT("BaseTitle","基础值").ToString();
	for (const auto& Term:Row.Terms)
	{
		if (Term.Kind==K::RawRoll || !Term.bResolved) continue;
		FText Line;
		if (Term.AttributeLabel.IsEmpty() && Term.ModifierSourceLabel.IsEmpty())
			Line=FText::FromString(Term.DisplayLabel);
		else if (Term.Kind==K::FixedModifier)
			Line=FText::Format(LOCTEXT("ModifierLine","{0} +{1}"),FText::FromString(Term.ModifierSourceLabel),FText::AsNumber(Term.Contribution));
		else
			Line=FText::Format(LOCTEXT("AttributeLine","{0} · {1} {2} × {3}"),FText::FromString(Term.ContributorDisplayName),
				FText::FromString(Term.AttributeLabel),FText::AsNumber(Term.SourceValue),FText::AsNumber(Term.Multiplier));
		Explanation+=TEXT("\n")+Line.ToString();
	}
	if (Row.bKnownNonRollSubtotalResolved)
		Explanation+=TEXT("\n")+FText::Format(LOCTEXT("BaseSum","当前基础值：{0}"),FText::AsNumber(Row.KnownNonRollSubtotal)).ToString();
	auto* Hover=Find<UBorder>(Tree,Named(Prefix,TEXT("BaseHover")));
	CastChecked<UTextBlock>(CastChecked<USizeBox>(CastChecked<UBorder>(Hover->GetToolTip())->GetContent())->GetContent())->SetText(FText::FromString(Explanation));

}

}

bool IsEnabled() {
#if UE_BUILD_SHIPPING
	return true;
#else
	return Mode.GetValueOnGameThread()!=0;
#endif
}
bool IsLowCrossEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return LowCrossMode.GetValueOnGameThread()!=0;
#endif
}
bool IsNearFreeKickEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return NearMode.GetValueOnGameThread()!=0;
#endif
}
bool IsLongFreeKickEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return LongMode.GetValueOnGameThread()!=0;
#endif
}
bool IsPenaltyEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return PenaltyMode.GetValueOnGameThread()!=0;
#endif
}
bool IsFormulaContest(FName ContestId)
{
	return ContestId==TEXT("Cross.High") || (ContestId==TEXT("Cross.Low") && IsLowCrossEnabled());
}
bool IsThroughBallFormulaContest(FName ContestId)
{
	return ContestId==TEXT("ThroughBall.Feet") || ContestId==TEXT("ThroughBall.BehindDefense.P1")
		|| ContestId==TEXT("ThroughBall.OneOnOne.DirectShot");
}
bool IsDirectShotContest(FName ContestId)
{
	return ContestId==TEXT("LongShot.DirectShot") || ContestId==TEXT("CutInsideShot.DirectShot");
}
bool IsDeadCornerContest(FName ContestId)
{
	return ContestId==TEXT("LongShot.DeadCorner") || ContestId==TEXT("CutInsideShot.DeadCorner");
}
bool IsOrdinaryShotConsumer(const FFMCodexUMGLongShotResolutionViewModel& Shot)
{
	return Shot.bVisible && (Shot.SkillType==ESkillRuleType::LongShot || Shot.SkillType==ESkillRuleType::CutInsideShot)
		&& (Shot.Stage==EFMCodexUMGLongShotStage::BranchChoice || Shot.Stage==EFMCodexUMGLongShotStage::DirectShot
			|| Shot.Stage==EFMCodexUMGLongShotStage::DeadCorner);
}
bool IsCornerSelectionEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return CornerMode.GetValueOnGameThread()!=0;
#endif
}
bool IsCornerResolutionEnabled()
{
#if UE_BUILD_SHIPPING
	return true;
#else
	return IsEnabled() && CornerResolutionMode.GetValueOnGameThread()!=0;
#endif
}
void ApplyCornerDisplayCopy(const FFMCodexUMGMatchScreenViewModel& Screen, FFMCodexUMGInlineFormulaSurfaceViewModel& P)
{
	if (!IsCornerResolutionEnabled() || Screen.SetPiece.Type!=ESetPieceSelectedType::Corner) return;
	// The wire retains its existing authored text. Normalize only this local displayed copy.
	for (FString* Label:{&P.ContestLabel,&P.ResolutionContextLabel,&P.RollHelperLabel,&P.RouteResultLabel,&P.StatusLabel,&P.ResolutionReasonLabel})
		Label->ReplaceInline(TEXT("低平球"),TEXT("低球"));
	if (!FMCodexOutcomePresentation::IsFinalReady(P.bNarrativeAvailable,P.bDiceRevealVisible)
		|| !P.bShowFormulaRows || P.AttackRow.Participants.Num()!=1) return;
	const auto Route=Screen.SetPiece.CornerActualRoute;
	if (Route!=EMatchPlayCornerRouteIntent::High && Route!=EMatchPlayCornerRouteIntent::Low) return;
	const auto Accent=P.OutcomeText.Accent;
	if (Accent!=EFMCodexOutcomeAccent::Goal && Accent!=EFMCodexOutcomeAccent::NoGoal) return;
	const FText Name=FText::FromString(P.AttackRow.Participants[0].PlayerName);
	// Preserve the authority-authored scorer. If its name differs from the disclosed
	// attacker, keep the original sentence; never select a scorer from roster/card data.
	if (Accent==EFMCodexOutcomeAccent::Goal && P.OutcomeText.Prefix.ToString()!=FText::Format(
		NSLOCTEXT("FMCodexCorner","GoalPrefix","{0}角球"),Name).ToString()) return;
	P.OutcomeText.Prefix=FText::Format(Route==EMatchPlayCornerRouteIntent::High
		?LOCTEXT("CornerHighFinish","{0}接角球高球攻门"):LOCTEXT("CornerLowFinish","{0}接角球低球攻门"),Name);
	P.OutcomeText.Keyword=Accent==EFMCodexOutcomeAccent::Goal
		?LOCTEXT("CornerScored","得分"):LOCTEXT("CornerNotScored","未能得分");
	P.ContestLabel=P.OutcomeText.ToText().ToString();
}
bool IsThroughBallEventConsumer(const FFMCodexUMGThroughBallResolutionViewModel& Event)
{
	return Event.bVisible && (Event.Stage==EFMCodexUMGThroughBallStage::InitialRoute
		|| Event.Stage==EFMCodexUMGThroughBallStage::AntiOffsideCheck
		|| Event.Stage==EFMCodexUMGThroughBallStage::FeetContest
		|| Event.Stage==EFMCodexUMGThroughBallStage::BehindDefenseFirstStage
		|| Event.Stage==EFMCodexUMGThroughBallStage::OneOnOneChoice
		|| Event.Stage==EFMCodexUMGThroughBallStage::OneOnOneResolution);
}
bool WantsTheater(const FFMCodexUMGMatchScreenViewModel& Screen, const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed)
{
	if (!IsEnabled() || Screen.FullTime.bVisible || Screen.Resolution.bRejected) return false;
	if (IsThroughBallEventConsumer(Screen.ThroughBallResolution)) return true;
	if (IsOrdinaryShotConsumer(Screen.LongShotResolution)
		&& (Screen.LongShotResolution.Stage==EFMCodexUMGLongShotStage::BranchChoice
			|| Screen.LongShotResolution.Stage==EFMCodexUMGLongShotStage::DeadCorner)) return true;
	if (Displayed.bVisible && IsDirectShotContest(Displayed.ContestId))
	{
		const auto& Shot = Screen.LongShotResolution;
		return Shot.bVisible && Shot.Stage==EFMCodexUMGLongShotStage::DirectShot
			&& ((Shot.SkillType==ESkillRuleType::LongShot && Displayed.ContestId==TEXT("LongShot.DirectShot"))
				|| (Shot.SkillType==ESkillRuleType::CutInsideShot && Displayed.ContestId==TEXT("CutInsideShot.DirectShot")));
	}
	if (Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::Corner)
		return (Screen.SetPiece.bCornerDraft ? IsCornerSelectionEnabled() : IsCornerResolutionEnabled())
			&& !(Displayed.bVisible && Displayed.ContestId==TEXT("SetPiece.Type"))
			&& Screen.Interaction.CrossRollRevealKind!=EFMCodexUMGCrossRollRevealKind::TacticalPoint;
	if (Screen.SetPiece.bVisible && (Screen.SetPiece.Type==ESetPieceSelectedType::ShortFreeKick || Screen.SetPiece.Type==ESetPieceSelectedType::LongFreeKick || Screen.SetPiece.Type==ESetPieceSelectedType::Penalty))
	{
		// The Type reel includes its existing ResultHold. Future safe method/terminal facts
		// cannot claim the stage while that displayed surface still owns the reveal.
		return (Screen.SetPiece.Type==ESetPieceSelectedType::ShortFreeKick?IsNearFreeKickEnabled():Screen.SetPiece.Type==ESetPieceSelectedType::LongFreeKick?IsLongFreeKickEnabled():IsPenaltyEnabled()) && !(Displayed.bVisible && Displayed.ContestId==TEXT("SetPiece.Type"))
			&& !(Screen.Interaction.CrossRollRevealKind==EFMCodexUMGCrossRollRevealKind::TacticalPoint);
	}

	// Only the explicit Low fallback exits at visible route disclosure.
	// Test the established visible disclosure, never the hidden future route alone.
	if (Displayed.ContestId==TEXT("Cross.Route") && !Displayed.RouteResultLabel.IsEmpty()
		&& Screen.InlineFormula.ContestId==TEXT("Cross.Low") && !IsLowCrossEnabled()) return false;
	return (Screen.Interaction.Category==C::SelectBranchIntent && Screen.InlineFormula.ContestId==TEXT("Cross.Setup"))
		|| (Displayed.bVisible && (Displayed.ContestId==TEXT("Cross.Route") || IsFormulaContest(Displayed.ContestId)));
}
void BuildParticipantDraw(UWidgetTree& Tree, UVerticalBox& Composition)
{
	auto* Draw=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterParticipantDraw"));
	for (int32 Side=0; Side<2; ++Side)
	{
		if (Side==1)
		{
			auto* Center=Tree.ConstructWidget<UVerticalBox>();
			auto* Label=Text(Tree,NAME_None,18,Quiet); Label->SetText(LOCTEXT("SharedD6","共同 D6"));
			Center->AddChildToVerticalBox(Label)->SetHorizontalAlignment(HAlign_Center);
			auto* Host=Tree.ConstructWidget<UOverlay>();
			auto* Pending=Text(Tree,TEXT("TheaterDrawPending"),42,Quiet); Pending->SetText(FText::FromString(TEXT("?")));
			auto* PendingSlot=Host->AddChildToOverlay(Pending); PendingSlot->SetHorizontalAlignment(HAlign_Center); PendingSlot->SetVerticalAlignment(VAlign_Center);
			auto* Reel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),TEXT("TheaterDrawReel"));
			Reel->SetVisualVariant(EFMCodexRollVisualVariant::CompactBox);
			auto* ReelBounds=Bounds(Tree,Reel,84,72); ReelBounds->Rename(TEXT("TheaterDrawReelHost"),&Tree);
			auto* ReelSlot=Host->AddChildToOverlay(ReelBounds); ReelSlot->SetHorizontalAlignment(HAlign_Center); ReelSlot->SetVerticalAlignment(VAlign_Center);
			Center->AddChildToVerticalBox(Bounds(Tree,Host,140,92));
			auto* Arrows=Text(Tree,NAME_None,22,Quiet); Arrows->SetText(FText::FromString(TEXT("←  →")));
			Center->AddChildToVerticalBox(Arrows)->SetHorizontalAlignment(HAlign_Center);
			auto* CenterSlot=Draw->AddChildToHorizontalBox(Center); CenterSlot->SetVerticalAlignment(VAlign_Center); CenterSlot->SetPadding(FMargin(18,0));
		}
		const FString Prefix=Side==0?TEXT("TheaterDrawAttack"):TEXT("TheaterDrawDefense");
		auto* Panel=Tree.ConstructWidget<UBorder>(); auto* Layers=Glass(Tree,*Panel,Named(Prefix,TEXT("Glass")));
		auto* Content=Tree.ConstructWidget<UVerticalBox>();
		auto* Heading=Text(Tree,NAME_None,26); Heading->SetText(Side==0?LOCTEXT("AttackCandidates","进攻候选"):LOCTEXT("DefenseCandidates","防守候选"));
		Content->AddChildToVerticalBox(Heading)->SetPadding(FMargin(0,0,0,16));
		for(int32 I=0;I<3;++I)
		{
			const FString RowPrefix=Prefix+FString::FromInt(I);
			auto* Row=Tree.ConstructWidget<UHorizontalBox>();
			auto* Range=Text(Tree,Named(RowPrefix,TEXT("Range")),16,Quiet);
			Row->AddChildToHorizontalBox(Bounds(Tree,Fit(Tree,Range),104))->SetVerticalAlignment(VAlign_Center);
			auto* Card=Tree.ConstructWidget<UFMCodexPlayerCardWidget>(UFMCodexPlayerCardWidget::StaticClass(),Named(RowPrefix,TEXT("Card")));
			Card->SetVisibility(ESlateVisibility::HitTestInvisible);
			Row->AddChildToHorizontalBox(Bounds(Tree,Card,280,76));
			auto* Frame=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),Named(RowPrefix,TEXT("Frame"))); Frame->SetPadding(FMargin(4)); Frame->AddChild(Row);
			Content->AddChildToVerticalBox(Frame)->SetPadding(FMargin(0,4));
		}
		auto* ContentSlot=Layers->AddChildToOverlay(Content); ContentSlot->SetPadding(FMargin(24,22));
		Draw->AddChildToHorizontalBox(Bounds(Tree,Panel,440,356));
	}
	Composition.AddChildToVerticalBox(Draw)->SetHorizontalAlignment(HAlign_Center);
}
void RefreshParticipantDraw(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& P, bool bVisible)
{
	Show(*Tree.FindWidget(TEXT("TheaterParticipantDraw")),bVisible);
	Show(*Tree.FindWidget(TEXT("TheaterDrawReelHost")),bVisible && P.bDiceRevealVisible);
	Show(*Tree.FindWidget(TEXT("TheaterDrawPending")),bVisible && !P.bDiceRevealVisible);
	if(!bVisible) return;
	const bool bDisclosed=!P.RouteResultLabel.IsEmpty();
	for(int32 Side=0;Side<2;++Side)
	{
		const auto& Ids=Side==0?Screen.SetPiece.CornerAttackers:Screen.SetPiece.CornerDefenders;
		const auto& Labels=Side==0?Screen.SetPiece.CornerAttackerRollLabels:Screen.SetPiece.CornerDefenderRollLabels;
		const FName Selected=Side==0?Screen.SetPiece.CornerRunner:Screen.SetPiece.CornerHelper;
		for(int32 I=0;I<3;++I)
		{
			const FString Prefix=(Side==0?FString(TEXT("TheaterDrawAttack")):FString(TEXT("TheaterDrawDefense")))+FString::FromInt(I);
			auto* Frame=Find<UBorder>(Tree,Named(Prefix,TEXT("Frame"))); Show(*Frame,Ids.IsValidIndex(I));
			if(!Ids.IsValidIndex(I)) continue;
			const bool bSelected=bDisclosed && Ids[I]==Selected;
			Frame->SetBrush(FSlateRoundedBoxBrush(FLinearColor::Transparent,4.f,bSelected?Aqua:FLinearColor::Transparent,2.f)); Frame->SetBrushColor(FLinearColor::White);
			Frame->SetRenderOpacity(bDisclosed && !bSelected?.35f:1.f);
			Find<UTextBlock>(Tree,Named(Prefix,TEXT("Range")))->SetText(FText::Format(LOCTEXT("CornerPositionRange","{0}号位\n掷点 {1}"),
				FText::AsNumber(I+1),FText::FromString(Labels.IsValidIndex(I)?Labels[I]:FString())));
			for(const auto* Rack:{&Screen.LocalRack,&Screen.OpponentRack})
				if(const auto* Cell=Rack->Cells.FindByPredicate([&](const auto& V){return V.Card.CardId==Ids[I];}))
				{ Find<UFMCodexPlayerCardWidget>(Tree,Named(Prefix,TEXT("Card")))->RefreshFromPresentation(Cell->Card,EFMCodexPlayerCardPresentationMode::HandMicro); break; }
		}
	}
}
UOverlay* Build(UWidgetTree& Tree, UButton*& Primary, UButton*& High, UButton*& Low, UTexture2D* Athletes)
{
	auto* Root=Tree.ConstructWidget<UOverlay>(UOverlay::StaticClass(),TEXT("ResolutionTheater"));
	Root->SetClipping(EWidgetClipping::ClipToBounds);
	auto* Backdrop=Tree.ConstructWidget<UNativeWidgetHost>(UNativeWidgetHost::StaticClass(),TEXT("TheaterLighting"));
	const auto* MatchBackground=Find<UBorder>(Tree,TEXT("MatchScreenStyleBackground"));
	Backdrop->SetContent(SNew(STheaterBackdrop).Stadium(MatchBackground->Background));
	Backdrop->SetVisibility(ESlateVisibility::HitTestInvisible);
	auto* BackSlot=Root->AddChildToOverlay(Backdrop); BackSlot->SetHorizontalAlignment(HAlign_Fill); BackSlot->SetVerticalAlignment(VAlign_Fill);
	auto* Body=Tree.ConstructWidget<UVerticalBox>();
	auto* Pad=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterBodyPadding")); Pad->SetBrushColor(FLinearColor::Transparent); Pad->SetPadding(FMargin(0,32,0,76)); Pad->AddChild(Body);
	auto* Design=Bounds(Tree,Pad,1600,900);
	auto* FitRoot=Tree.ConstructWidget<UScaleBox>(UScaleBox::StaticClass(),TEXT("TheaterContent")); FitRoot->SetStretch(EStretch::ScaleToFit); FitRoot->AddChild(Design);
	auto* FitSlot=Root->AddChildToOverlay(FitRoot); FitSlot->SetHorizontalAlignment(HAlign_Fill); FitSlot->SetVerticalAlignment(VAlign_Fill);
	auto* Context=Text(Tree,TEXT("TheaterContext"),13,Alpha(Quiet,.85f)); Context->SetJustification(ETextJustify::Center);
	Body->AddChildToVerticalBox(Context);
	auto* Top=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("TheaterTop"));
	auto* Crest=Tree.ConstructWidget<UHorizontalBox>();
	Crest->AddChildToHorizontalBox(Rule(Tree,72,Quiet))->SetVerticalAlignment(VAlign_Center);
	Crest->AddChildToHorizontalBox(Mark(Tree,NAME_None,EMark::Ball,26))->SetPadding(FMargin(16,0));
	Crest->AddChildToHorizontalBox(Rule(Tree,72,Quiet))->SetVerticalAlignment(VAlign_Center);
	Top->AddChildToVerticalBox(Crest)->SetHorizontalAlignment(HAlign_Center);
	auto* Title=Text(Tree,TEXT("TheaterTitle"),52); Title->SetJustification(ETextJustify::Center); Top->AddChildToVerticalBox(Title)->SetPadding(FMargin(0,4,0,0));
	auto* Outcome=FMCodexOutcomePresentation::BuildPrimary(Tree,TEXT("TheaterOutcome")); Outcome->SetDefaultFont(Title->GetFont());
	// BuildPrimary creates a per-widget style table. Keep semantic spans at the
	// same local broadcast size, without changing the shared Outcome family.
	for (const auto RowName:{FName(TEXT("Goal")),FName(TEXT("NoGoal"))})
		if (auto* Row=Outcome->GetTextStyleSet()->FindRow<FRichTextStyleRow>(RowName,TEXT("Theater")))
		{
			auto SemanticFont=Title->GetFont(); SemanticFont.OutlineSettings.OutlineSize=0;
			Row->TextStyle.SetFont(SemanticFont);
		}
	auto* OutcomeStyles=Outcome->GetTextStyleSet();
	Outcome->SetTextStyleSet(nullptr); Outcome->SetTextStyleSet(OutcomeStyles);
	Top->AddChildToVerticalBox(Bounds(Tree,Outcome,1200))->SetHorizontalAlignment(HAlign_Center);
	auto* Divider=Tree.ConstructWidget<UNativeWidgetHost>(UNativeWidgetHost::StaticClass(),TEXT("TheaterOutcomeDivider"));
	Divider->SetContent(SNew(STheaterDivider)); Divider->SetVisibility(ESlateVisibility::HitTestInvisible);
	Top->AddChildToVerticalBox(Divider)->SetHorizontalAlignment(HAlign_Center);
	auto* Subtitle=Text(Tree,TEXT("TheaterSubtitle"),19,Quiet); Subtitle->SetJustification(ETextJustify::Center); Top->AddChildToVerticalBox(Subtitle)->SetPadding(FMargin(0,8));
	Body->AddChildToVerticalBox(Bounds(Tree,Top,0,178))->SetPadding(FMargin(0,16,0,0));
	auto* Center=Tree.ConstructWidget<UOverlay>();
	Body->AddChildToVerticalBox(Center)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	auto* Composition=Tree.ConstructWidget<UVerticalBox>();
	auto* CompositionFit=Tree.ConstructWidget<UScaleBox>(UScaleBox::StaticClass(),TEXT("TheaterCompositionFit"));
	CompositionFit->SetStretchDirection(EStretchDirection::DownOnly); CompositionFit->AddChild(Composition);
	auto* CompositionSlot=Center->AddChildToOverlay(CompositionFit); CompositionSlot->SetVerticalAlignment(VAlign_Fill); CompositionSlot->SetHorizontalAlignment(HAlign_Fill);
	BuildParticipantDraw(Tree,*Composition);
	auto* Middle=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterDuel"));
	BuildSide(Tree,*Middle,TEXT("TheaterAttack"),Athletes);
	auto* VS=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterVS"));
	VS->SetBrush(FSlateRoundedBoxBrush(Alpha(Color(9,28,41),.9f),22.f,Alpha(Quiet,.48f),1.f)); VS->SetBrushColor(FLinearColor::White); VS->SetPadding(FMargin(8,8));
	auto* VSText=Text(Tree,TEXT("TheaterVSLabel"),28,Quiet); VSText->SetText(FText::FromString(TEXT("VS"))); VSText->SetJustification(ETextJustify::Center); VS->AddChild(VSText);
	auto* VSBounds=Bounds(Tree,VS,64.f); auto* VSSlot=Middle->AddChildToHorizontalBox(VSBounds); VSSlot->SetVerticalAlignment(VAlign_Center); VSSlot->SetPadding(FMargin(10,0));
	BuildSide(Tree,*Middle,TEXT("TheaterDefense"),Athletes);
	Composition->AddChildToVerticalBox(Middle)->SetHorizontalAlignment(HAlign_Center);
	auto* Candidates=Tree.ConstructWidget<UFMCodexCardRackWidget>(UFMCodexCardRackWidget::StaticClass(),TEXT("TheaterTakers"));
	Candidates->SetProminentDraftSelection(true);
	auto* InspectionRow=Tree.ConstructWidget<UHorizontalBox>();
 auto* RackSlot=InspectionRow->AddChildToHorizontalBox(Bounds(Tree,Candidates,1000)); RackSlot->SetVerticalAlignment(VAlign_Center);
 auto* Inspector=Tree.ConstructWidget<UOverlay>(UOverlay::StaticClass(),TEXT("TheaterTakerInspector"));
 Inspector->SetVisibility(ESlateVisibility::HitTestInvisible);
 auto* Full=Tree.ConstructWidget<UFMCodexPlayerCardWidget>(UFMCodexPlayerCardWidget::StaticClass(),TEXT("TheaterTakerFullCard"));
 auto* FullFit=Tree.ConstructWidget<UScaleBox>(); FullFit->SetStretch(EStretch::ScaleToFit); FullFit->AddChild(Full);
 auto* FullSlot=Inspector->AddChildToOverlay(FullFit); FullSlot->SetHorizontalAlignment(HAlign_Fill); FullSlot->SetVerticalAlignment(VAlign_Fill);
 auto* Placeholder=Text(Tree,TEXT("TheaterTakerPlaceholder"),18,Quiet);
 Placeholder->SetText(LOCTEXT("TakerInspectPlaceholder","悬停球员查看详细属性"));
 auto* PlaceholderSlot=Inspector->AddChildToOverlay(Placeholder); PlaceholderSlot->SetHorizontalAlignment(HAlign_Center); PlaceholderSlot->SetVerticalAlignment(VAlign_Center);
 auto* InspectorSlot=InspectionRow->AddChildToHorizontalBox(Bounds(Tree,Inspector,300,450)); InspectorSlot->SetPadding(FMargin(24,0,0,0));
 auto* CandidateBounds=Bounds(Tree,InspectionRow,1324,450); CandidateBounds->Rename(TEXT("TheaterTakerBounds"),&Tree);
	Composition->AddChildToVerticalBox(CandidateBounds)->SetHorizontalAlignment(HAlign_Center);
	auto* SelectionMessage=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterSelectionMessage"));
	auto* SelectionLayers=Glass(Tree,*SelectionMessage,TEXT("TheaterSelectionGlass"));
	auto* SelectionText=Text(Tree,TEXT("TheaterSelectionMessageText"),24);
	SelectionText->SetJustification(ETextJustify::Center);
	auto* SelectionTextSlot=SelectionLayers->AddChildToOverlay(SelectionText);
	SelectionTextSlot->SetHorizontalAlignment(HAlign_Center); SelectionTextSlot->SetVerticalAlignment(VAlign_Center);
	auto* SelectionMessageBounds=Bounds(Tree,SelectionMessage,1000,240); SelectionMessageBounds->Rename(TEXT("TheaterSelectionMessageBounds"),&Tree);
	Composition->AddChildToVerticalBox(SelectionMessageBounds)->SetHorizontalAlignment(HAlign_Center);
	auto* SelectionHint=Tree.ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(),TEXT("TheaterSelectionHint"));
	auto SelectionFont=Placeholder->GetFont(); SelectionFont.Size=16;
	SelectionHint->SetDefaultFont(SelectionFont); SelectionHint->SetDefaultColorAndOpacity(Quiet);
	SelectionHint->SetAutoWrapText(false);
	SelectionHint->SetJustification(ETextJustify::Center);
	auto* SelectionStyles=NewObject<UDataTable>(SelectionHint); SelectionStyles->RowStruct=FRichTextStyleRow::StaticStruct();
	FRichTextStyleRow DangerStyle;
	DangerStyle.TextStyle.SetFont(SelectionFont).SetColorAndOpacity(FFMCodexPlayerUIStyle::Get().GetColor(EFMCodexPlayerUIColorRole::Danger));
	SelectionStyles->AddRow(TEXT("Danger"),DangerStyle); SelectionHint->SetTextStyleSet(SelectionStyles);
	// A fixed one-line allocation keeps the roster, inspector and CTAs stable for
	// empty, underfilled and full drafts; only the warning phrase has a red span.
	auto* SelectionHintBounds=Bounds(Tree,Fit(Tree,SelectionHint,HAlign_Center),1324,28);
	SelectionHintBounds->Rename(TEXT("TheaterSelectionHintBounds"),&Tree);
	Composition->AddChildToVerticalBox(SelectionHintBounds)->SetPadding(FMargin(0,8,0,0));
	auto* Methods=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterNearMethods"));
	for (bool bDirect:{true,false})
	{
		auto* ChoiceBody=Tree.ConstructWidget<UVerticalBox>();
		auto* Choice=Button(Tree,bDirect?TEXT("TheaterNearDirect"):TEXT("TheaterNearCombination"),
			bDirect?TEXT("TheaterNearDirectLabel"):TEXT("TheaterNearCombinationLabel"),
			bDirect?LOCTEXT("NearDirect","直接射门"):LOCTEXT("NearCombination","战术配合"));
		ChoiceBody->AddChildToVerticalBox(Bounds(Tree,Choice,410,62));
		auto* Explanation=Tree.ConstructWidget<UHorizontalBox>();
		auto* Diagram=Tree.ConstructWidget<UFMCodexMatchFlowDiagram>(UFMCodexMatchFlowDiagram::StaticClass(),
			bDirect?TEXT("TheaterNearDirectDiagram"):TEXT("TheaterNearCombinationDiagram"));
		Diagram->SetDiagram(bDirect?EFMCodexFlowDiagram::Direct:EFMCodexFlowDiagram::Combination);
		Diagram->SetVisibility(ESlateVisibility::HitTestInvisible);
		Explanation->AddChildToHorizontalBox(Diagram)->SetVerticalAlignment(VAlign_Center);
		auto* Hint=Text(Tree,bDirect?TEXT("TheaterNearDirectHint"):TEXT("TheaterNearCombinationHint"),16,Quiet);
		Hint->SetText(bDirect?LOCTEXT("NearDirectHint","取射门 / 传球较高值\n对抗门将手控球 + 防守加成")
			:LOCTEXT("NearCombinationHint","需射门 + 传球 ≥ 8\n两枚骰子总和 ≥ 9 进球"));
		auto* HintSlot=Explanation->AddChildToHorizontalBox(Hint);
		HintSlot->SetPadding(FMargin(14,0,0,0)); HintSlot->SetVerticalAlignment(VAlign_Center);
		auto* ExplanationBounds=Bounds(Tree,Explanation,410,60);
		ExplanationBounds->Rename(bDirect?TEXT("TheaterDirectExplanationBounds"):TEXT("TheaterAlternativeExplanationBounds"),&Tree);
		ChoiceBody->AddChildToVerticalBox(ExplanationBounds)->SetPadding(FMargin(0,10,0,0));
		Methods->AddChildToHorizontalBox(ChoiceBody)->SetPadding(FMargin(20,18));
	}
	Composition->AddChildToVerticalBox(Methods)->SetHorizontalAlignment(HAlign_Center);


	auto* Bottom=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("TheaterBottom"));
	auto* BottomBounds=Bounds(Tree,Bottom,1040); BottomBounds->Rename(TEXT("TheaterBottomBounds"),&Tree);
	auto* BottomSlot=Composition->AddChildToVerticalBox(BottomBounds); BottomSlot->SetHorizontalAlignment(HAlign_Center); BottomSlot->SetPadding(FMargin(0,16,0,0));
	auto* Lane=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterInfoBar"));
	auto* LaneLayers=Glass(Tree,*Lane,TEXT("TheaterLaneGlass"));
	auto* InfoRow=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterReasonLayout"));
	auto* ReasonMark=Mark(Tree,TEXT("TheaterReasonMark"),EMark::Dice,44,White);
	auto* MarkSlot=InfoRow->AddChildToHorizontalBox(ReasonMark); MarkSlot->SetVerticalAlignment(VAlign_Center);
	auto* Separator=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterReasonSeparator"));
	Separator->SetPadding(FMargin(0)); Separator->SetBrushColor(Alpha(Quiet,.65f));
	auto* SeparatorSlot=InfoRow->AddChildToHorizontalBox(Bounds(Tree,Separator,1,46));
	SeparatorSlot->SetPadding(FMargin(24,0)); SeparatorSlot->SetVerticalAlignment(VAlign_Center);
	auto* Info=Tree.ConstructWidget<UVerticalBox>();
	InfoRow->AddChildToHorizontalBox(Info)->SetVerticalAlignment(VAlign_Center);
	auto* LaneContent=LaneLayers->AddChildToOverlay(Fit(Tree,InfoRow,HAlign_Center));
	LaneContent->SetHorizontalAlignment(HAlign_Fill); LaneContent->SetVerticalAlignment(VAlign_Center); LaneContent->SetPadding(FMargin(36,12));
	Bottom->AddChildToVerticalBox(Bounds(Tree,Lane,0,78));
	auto* Detail=Text(Tree,TEXT("TheaterDetail"),18,White); Detail->SetJustification(ETextJustify::Left);
	Info->AddChildToVerticalBox(Detail);
	auto* Reason=Tree.ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(),TEXT("TheaterReasonPrimary"));
	auto ReasonFont=Detail->GetFont(); ReasonFont.Size=20; ReasonFont.TypefaceFontName=TEXT("Medium");
	Reason->SetDefaultFont(ReasonFont); Reason->SetDefaultColorAndOpacity(White);
	Reason->SetJustification(ETextJustify::Left); Reason->SetAutoWrapText(false);
	auto* Styles=NewObject<UDataTable>(Reason); Styles->RowStruct=FRichTextStyleRow::StaticStruct();
	FRichTextStyleRow ValueStyle; ReasonFont.TypefaceFontName=TEXT("Bold"); ReasonFont.Size=23;
	ValueStyle.TextStyle.SetFont(ReasonFont).SetColorAndOpacity(Mint);
	Styles->AddRow(TEXT("Value"),ValueStyle); Reason->SetTextStyleSet(Styles);
	Info->AddChildToVerticalBox(Reason);
	auto* Secondary=Text(Tree,TEXT("TheaterReasonSecondary"),14,Quiet); Secondary->SetJustification(ETextJustify::Left);
	Info->AddChildToVerticalBox(Secondary)->SetPadding(FMargin(0,3,0,0));
	auto* Status=Text(Tree,TEXT("TheaterStatus"),14,Quiet); Status->SetJustification(ETextJustify::Center);
	auto* StatusSlot=Bottom->AddChildToVerticalBox(Status); StatusSlot->SetHorizontalAlignment(HAlign_Fill); StatusSlot->SetPadding(FMargin(0,4,0,0));
	// Reel and action share an allocation, so the sides never jump as the roll resolves.
	auto* ActionLane=Tree.ConstructWidget<UOverlay>(UOverlay::StaticClass(),TEXT("TheaterActionLane"));
	auto* ActionBounds=Bounds(Tree,ActionLane,0,72); ActionBounds->Rename(TEXT("TheaterActionBounds"),&Tree);
	Bottom->AddChildToVerticalBox(ActionBounds)->SetPadding(FMargin(0,4,0,0));
	Primary=Button(Tree,TEXT("TheaterContinue"),TEXT("TheaterContinueLabel"),FText::GetEmpty());
	auto* PrimaryBounds=Bounds(Tree,Primary,320,62); PrimaryBounds->Rename(TEXT("TheaterPrimaryBounds"),&Tree);
	auto* PrimaryActions=Tree.ConstructWidget<UHorizontalBox>();
	PrimaryActions->AddChildToHorizontalBox(PrimaryBounds);
	auto* SelectionReturn=Button(Tree,TEXT("TheaterSelectionReturn"),TEXT("TheaterSelectionReturnLabel"),LOCTEXT("SelectionReturn","返回补充"));
	auto* ReturnBounds=Bounds(Tree,SelectionReturn,240,62); ReturnBounds->Rename(TEXT("TheaterSelectionReturnBounds"),&Tree);
	PrimaryActions->AddChildToHorizontalBox(ReturnBounds)->SetPadding(FMargin(18,0,0,0));
	auto* PrimarySlot=ActionLane->AddChildToOverlay(PrimaryActions); PrimarySlot->SetHorizontalAlignment(HAlign_Center); PrimarySlot->SetVerticalAlignment(VAlign_Center);
	auto* Choices=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterChoices"));
	High=Button(Tree,TEXT("TheaterHigh"),TEXT("TheaterHighLabel"),LOCTEXT("High","高球传中"));
	Low=Button(Tree,TEXT("TheaterLow"),TEXT("TheaterLowLabel"),LOCTEXT("Low","低球传中"));
	for (bool bHigh:{true,false})
	{
		auto* ChoiceBody=Tree.ConstructWidget<UVerticalBox>();
		ChoiceBody->AddChildToVerticalBox(Bounds(Tree,bHigh?High:Low,240,60));
		auto* Hint=Text(Tree,bHigh?TEXT("TheaterHighHint"):TEXT("TheaterLowHint"),14,Quiet);
		Hint->SetText(bHigh?LOCTEXT("CornerHighChoiceHint","进攻：力量\n防守：力量、门将制空")
			:LOCTEXT("CornerLowChoiceHint","进攻：射门\n防守：盯防、门将反应"));
		Hint->SetJustification(ETextJustify::Center);
		auto* HintBounds=Bounds(Tree,Hint,240,50); HintBounds->Rename(bHigh?TEXT("TheaterHighHintBounds"):TEXT("TheaterLowHintBounds"),&Tree);
		ChoiceBody->AddChildToVerticalBox(HintBounds)->SetPadding(FMargin(0,10,0,0));
		Choices->AddChildToHorizontalBox(ChoiceBody)->SetPadding(bHigh?FMargin(0,0,18,0):FMargin(0));
	}
	auto* ChoiceSlot=ActionLane->AddChildToOverlay(Choices); ChoiceSlot->SetHorizontalAlignment(HAlign_Center); ChoiceSlot->SetVerticalAlignment(VAlign_Center);
	auto* ReelLine=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterRoll"));
	auto* Reel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),TEXT("TheaterReel"));
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::CompactBox);
	ReelLine->AddChildToHorizontalBox(Bounds(Tree,Reel,84,72));
	auto* ReelSlot=ActionLane->AddChildToOverlay(ReelLine); ReelSlot->SetHorizontalAlignment(HAlign_Center); ReelSlot->SetVerticalAlignment(VAlign_Center);
	// ThroughBall independent events use the same dedicated action/roll lane as
	// Cross route. The context card owns identity only; no die or placeholder.
	auto* EventReel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),TEXT("TheaterEventReel"));
	EventReel->SetVisualVariant(EFMCodexRollVisualVariant::CompactBox);
	auto* EventReelHost=Bounds(Tree,EventReel,84,72); EventReelHost->Rename(TEXT("TheaterEventReelHost"),&Tree);
	auto* EventCell=Bounds(Tree,EventReelHost,84,72); EventCell->Rename(TEXT("TheaterEventCell"),&Tree);
	auto* EventBounds=Bounds(Tree,EventCell,84,72); EventBounds->Rename(TEXT("TheaterTacticalEvent"),&Tree);
	auto* EventSlot=ActionLane->AddChildToOverlay(EventBounds);
	EventSlot->SetHorizontalAlignment(HAlign_Center); EventSlot->SetVerticalAlignment(VAlign_Center);
	Show(*EventBounds,false);
	return Root;
}
FFMCodexUMGInlineFormulaRowViewModel ThroughBallContext(const FFMCodexUMGMatchScreenViewModel& Screen,
	bool bCarrier)
{
	FFMCodexUMGInlineFormulaRowViewModel Row;
	Row.SideLabel=LOCTEXT("ThroughBallAttack","进攻").ToString();
	// SelectedRole is already projected from selected stable IDs. Slot position,
	// roster membership and Narrative text do not determine these subjects.
	const auto Add=[&](EFMCodexUMGSelectedRole Role)
	{
		for (const auto& Region:Screen.PitchRegions)
			for (const auto& Slot:Region.Slots)
				if (Slot.Card.SelectedRole==Role)
				{
					Row.Participants.Add({Role==EFMCodexUMGSelectedRole::Carrier
						?LOCTEXT("ThroughBallPasser","传球").ToString():Slot.Card.SelectedRoleLabel,
						Slot.Card.IdentityLabel.IsEmpty()?LOCTEXT("PlayerFallback","球员").ToString():Slot.Card.IdentityLabel});
					return;
				}
	};
	if (bCarrier) Add(EFMCodexUMGSelectedRole::Carrier);
	Add(EFMCodexUMGSelectedRole::Runner);
	return Row;
}
void RefreshThroughBallEvent(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGMatchHeaderViewModel& H, bool bRequestPending)
{
	const auto& P=Screen.ThroughBallResolution;
	const bool bRoute=P.Stage==EFMCodexUMGThroughBallStage::InitialRoute;
	const bool bChip=P.Stage==EFMCodexUMGThroughBallStage::OneOnOneResolution;
	const bool bRolling=P.bDiceRevealVisible;
	for (const auto Name:{TEXT("TheaterDuel"),TEXT("TheaterParticipantDraw"),TEXT("TheaterTakerBounds"),
		TEXT("TheaterSelectionMessageBounds"),TEXT("TheaterSelectionHintBounds"),TEXT("TheaterNearMethods"),
		TEXT("TheaterSelectionReturnBounds"),TEXT("TheaterChoices"),TEXT("TheaterRoll"),
		TEXT("TheaterOutcome"),TEXT("TheaterOutcomeDivider"),TEXT("TheaterReasonPrimary"),TEXT("TheaterReasonSecondary")})
		Show(*Tree.FindWidget(Name),false);
	Show(*Tree.FindWidget(TEXT("TheaterTacticalEvent")),true);
	// Reuse the existing compact participant/art panel, with no Formula or
	// invented opponent. Shared context always keeps passer before runner.
	const auto Context=ThroughBallContext(Screen,!bChip);
	Show(*Tree.FindWidget(TEXT("TheaterDuel")),!Context.Participants.IsEmpty());
	Show(*Tree.FindWidget(TEXT("TheaterDefensePanelBounds")),false);
	Show(*Tree.FindWidget(TEXT("TheaterVS"))->GetParent(),false);
	Show(*Tree.FindWidget(TEXT("TheaterPair")),false);
	Show(*Tree.FindWidget(TEXT("TheaterAttackRollCaption")),false);
	RefreshSide(Tree,TEXT("TheaterAttack"),Context,false,false,false,false);
	Find<UScaleBox>(Tree,TEXT("TheaterCompositionFit"))->SetStretch(EStretch::None);
	CastChecked<UOverlaySlot>(Tree.FindWidget(TEXT("TheaterCompositionFit"))->Slot)->SetVerticalAlignment(VAlign_Center);
	Find<UBorder>(Tree,TEXT("TheaterBodyPadding"))->SetPadding(FMargin(0,32,0,76));
	Find<USizeBox>(Tree,TEXT("TheaterBottomBounds"))->SetWidthOverride(1040.f);
	Find<USizeBox>(Tree,TEXT("TheaterActionBounds"))->SetHeightOverride(72.f);
	Find<USizeBox>(Tree,TEXT("TheaterPrimaryBounds"))->SetWidthOverride(320.f);
	Show(*Tree.FindWidget(TEXT("TheaterInfoBar"))->GetParent(),true);
	Show(*Tree.FindWidget(TEXT("TheaterReasonMark"))->GetParent(),false);
	Show(*Tree.FindWidget(TEXT("TheaterReasonSeparator"))->GetParent(),false);

	auto* Title=Find<UTextBlock>(Tree,TEXT("TheaterTitle"));
	auto Font=Title->GetFont(); Font.Size=52; Title->SetFont(Font);
	Title->SetText(LOCTEXT("ThroughBall","直塞")); Show(*Title,true);
	Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bRoute
		?LOCTEXT("ThroughBallRoute","路线判定"):bChip?LOCTEXT("ThroughBallChip","单刀 · 挑射"):LOCTEXT("ThroughBallAnti","反越位判定"));
	const bool bLeftA=H.LeftPlayerSide==EInitialTurnOrderPlayer::PlayerA;
	SetText(Tree,TEXT("TheaterContext"),FString::Printf(TEXT("%s  %s  –  %s  %s"),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.LeftPlayerLabel).ToString(),
		*(bLeftA?H.PlayerAScoreLabel:H.PlayerBScoreLabel),*(bLeftA?H.PlayerBScoreLabel:H.PlayerAScoreLabel),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.RightPlayerLabel).ToString()));

	FText Rule=FFMCodexTacticalDetailPresentationBuilder::BuildThroughBallRouteHint();
	if (!bRoute)
	{
		TArray<FString> Ranges;
		for (const auto& Entry:P.OutcomeRollHint.Entries)
			Ranges.Add(Entry.OutcomeId==TEXT("OneOnOne")
				?FFMCodexPlayerUIPresentationText::TacticalOutcomeRange(Entry.Minimum,Entry.Maximum,
					NSLOCTEXT("FMCodexThroughBall","AntiFormsOneOnOne","形成单刀")).ToString():Entry.DisplayLabel);
		Rule=FText::FromString(FString::Join(Ranges,TEXT("　｜　")));
	}
	// Hidden keeps the 84x72 lane geometry during the pre-roll CTA. The same
	// allocation then paints motion, landed value and hold without moving the card.
	Tree.FindWidget(TEXT("TheaterEventReelHost"))->SetVisibility(
		bRolling?ESlateVisibility::SelfHitTestInvisible:ESlateVisibility::Hidden);

	// These are display-gated semantic facts. Never derive an outcome from CenterValue.
	FText Detail=Rule;
	if (bRoute && !P.RouteResultLabel.IsEmpty()) Detail=FText::FromString(P.RouteResultLabel);
	if (!bRoute && P.bNarrativeAvailable && !P.ResultTitle.IsEmpty()) Detail=FText::FromString(P.ResultTitle);
	Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(Detail); Show(*Tree.FindWidget(TEXT("TheaterDetail")),true);

	const bool bAction=!bRolling && !bRequestPending && P.PrimaryAction.bVisible && P.PrimaryAction.Action.bAvailable;
	Show(*Tree.FindWidget(TEXT("TheaterPrimaryBounds")),bAction);
	Find<UButton>(Tree,TEXT("TheaterContinue"))->SetIsEnabled(bAction);
	Find<UButton>(Tree,TEXT("TheaterContinue"))->SetBackgroundColor(FLinearColor::White);
	Find<UTextBlock>(Tree,TEXT("TheaterContinueLabel"))->SetText(FText::FromString(P.PrimaryAction.Action.Label));
	for (const auto Name:{TEXT("TheaterDiceIcon"),TEXT("TheaterRollLeading")}) Show(*Tree.FindWidget(Name),true);
	for (const auto Name:{TEXT("TheaterNextIcon"),TEXT("TheaterNextTrailing")}) Show(*Tree.FindWidget(Name),false);
	FText Status=FFMCodexPlayerUIPresentationText::MatchScreenLabel(Screen.Interaction.ExpectedActorLabel);
	if (Screen.bMirrorActionWaitPrompt && !bRolling)
		Status=FText::Format(LOCTEXT("EventWait","{0}  {1}"),Screen.ActionWaitActorText,Screen.ActionWaitActionText);
	if (bRequestPending) Status=LOCTEXT("PendingRequest","操作已提交，等待确认");
	Find<UTextBlock>(Tree,TEXT("TheaterStatus"))->SetText(Status);
	Tree.FindWidget(TEXT("TheaterStatus"))->SetVisibility(bRolling?ESlateVisibility::Hidden:ESlateVisibility::SelfHitTestInvisible);
	RefreshReel(Tree,P.RollReel);
}
void Refresh(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed, const FFMCodexUMGMatchHeaderViewModel& H, bool bRequestPending, FTakerInspection& Inspection,
	bool bSelectionConfirmationPending)
{
	const auto& Through=Screen.ThroughBallResolution;
	const bool bThrough=IsThroughBallEventConsumer(Through);
	const bool bThroughChoice=bThrough && Through.Stage==EFMCodexUMGThroughBallStage::OneOnOneChoice
		&& !Through.Formula.bDiceRevealVisible;
	const bool bThroughEvent=bThrough && (Through.Stage==EFMCodexUMGThroughBallStage::InitialRoute
		|| Through.Stage==EFMCodexUMGThroughBallStage::AntiOffsideCheck
		|| (Through.Stage==EFMCodexUMGThroughBallStage::OneOnOneResolution && !Through.Formula.bVisible));
	if (bThroughEvent && !FMCodexOutcomePresentation::IsFinalReady(Through.bNarrativeAvailable,Through.bDiceRevealVisible))
	{
		ClearTakerInspection(Tree,Inspection);
		RefreshThroughBallEvent(Tree,Screen,H,bRequestPending);
		return;
	}
	Show(*Tree.FindWidget(TEXT("TheaterTacticalEvent")),false);
	Show(*Tree.FindWidget(TEXT("TheaterEventReelHost")),false);
	Show(*Tree.FindWidget(TEXT("TheaterAttackRollCaption")),false);
	for (const auto Name:{TEXT("TheaterNearDirectDiagram"),TEXT("TheaterNearCombinationDiagram")})
		Show(*Tree.FindWidget(Name),true);
	const auto& Shot=Screen.LongShotResolution;
	const bool bShotConsumer=IsOrdinaryShotConsumer(Shot);
	const bool bShotChoice=bShotConsumer && Shot.Stage==EFMCodexUMGLongShotStage::BranchChoice;
	const bool bDeadCorner=bShotConsumer && Shot.Stage==EFMCodexUMGLongShotStage::DeadCorner;
	// Adapt existing display fields only; DeadCorner never acquires Formula rows.
	auto ShotContext=Displayed;
	if (bThrough) ShotContext=Through.Formula;
	if (bThroughChoice || bThroughEvent)
	{
		ShotContext={}; ShotContext.bVisible=true;
		ShotContext.bShowFormulaRows=ShotContext.bShowAttackRow=ShotContext.bShowDefenseRow=false;
		ShotContext.bNarrativeAvailable=!bThroughChoice && Through.bNarrativeAvailable;
		ShotContext.ContestLabel=Through.NarrativeHeadline;
		ShotContext.OutcomeText=Through.OutcomeText;
		ShotContext.PrimaryAction=Through.PrimaryAction;
		ShotContext.ResolutionReasonLabel=Through.OutcomeRollDetail;
		ShotContext.AttackRow=ThroughBallContext(Screen,!bThroughChoice
			&& Through.Stage==EFMCodexUMGThroughBallStage::AntiOffsideCheck);
	}
	if (bShotChoice || bDeadCorner)
	{
		ShotContext={}; ShotContext.bVisible=true; ShotContext.bShowFormulaRows=false;
		ShotContext.bShowAttackRow=ShotContext.bShowDefenseRow=false;
		ShotContext.bDiceRevealVisible=Shot.bDiceRevealVisible;
		ShotContext.ActiveRollSequenceIndex=Displayed.ActiveRollSequenceIndex;
		ShotContext.RollReel=Shot.RollReel; ShotContext.PrimaryAction=Shot.PrimaryAction;
		ShotContext.bNarrativeAvailable=Shot.bNarrativeAvailable;
		ShotContext.NarrativeHeadline=Shot.NarrativeHeadline; ShotContext.OutcomeText=Shot.OutcomeText;
		ShotContext.ContestLabel=Shot.NarrativeHeadline;
		ShotContext.ResolutionReasonLabel=Shot.OutcomeRollDetail;
		ShotContext.RollHelperLabel=Shot.OutcomeHintLabel;
		ShotContext.DiceOwnerLabel=Shot.StatusLabel;
		ShotContext.AttackRow=Shot.Formula.AttackRow;
	}
	const auto& P=ShotContext;
	const bool bNear=Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::ShortFreeKick && IsNearFreeKickEnabled();
	const bool bLong=Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::LongFreeKick && IsLongFreeKickEnabled();
	const bool bPenalty=Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::Penalty && IsPenaltyEnabled();
	const bool bCorner=Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::Corner && IsCornerSelectionEnabled() && Screen.SetPiece.bCornerDraft;
	// Fit the whole planning composition, preserving the Full Card's native design
	// and a common scale for rack, inspector, rules and confirmation controls.
	Find<UScaleBox>(Tree,TEXT("TheaterCompositionFit"))->SetStretch(bCorner?EStretch::ScaleToFit:EStretch::None);
	auto* CompositionSlot=CastChecked<UOverlaySlot>(Tree.FindWidget(TEXT("TheaterCompositionFit"))->Slot);
	CompositionSlot->SetHorizontalAlignment(bCorner?HAlign_Fill:HAlign_Center);
	CompositionSlot->SetVerticalAlignment(bCorner?VAlign_Fill:VAlign_Center);
	const bool bCornerResolution=Screen.SetPiece.bVisible && Screen.SetPiece.Type==ESetPieceSelectedType::Corner && IsCornerResolutionEnabled() && !Screen.SetPiece.bCornerDraft;
	const bool bShared=bCornerResolution && P.ContestId==TEXT("Corner.Participants");
	const bool bSetPieceTheater=bNear || bLong || bPenalty || bCorner || bCornerResolution;
	const bool bDirectShot=IsDirectShotContest(P.ContestId);
	const bool bThroughFormula=bThrough && IsThroughBallFormulaContest(P.ContestId);
	const bool bShotRollOnly=(bDirectShot || (bThroughFormula && P.ContestId==TEXT("ThroughBall.BehindDefense.P1"))) && !P.bShowFormulaRows;
	const bool bSelection=bSetPieceTheater && (Screen.SetPiece.bTakerWait || bCorner);
	const auto& Options=bCorner?Screen.SetPiece.CornerOptions:Screen.SetPiece.TakerOptions;
 const bool bInspect=bSelection && !Options.IsEmpty() && !bRequestPending;
 if (!bInspect) ClearTakerInspection(Tree,Inspection);
 else
 {
  Inspection.bActive=true; Inspection.bLongFreeKick=bLong; Inspection.bPenalty=bPenalty; Inspection.bOrderedMultiSelect=bCorner; Inspection.CombinationEligibility.Reset();
  Tree.FindWidget(TEXT("TheaterTakerInspector"))->SetVisibility(ESlateVisibility::HitTestInvisible);
  for (const auto& Fact:Screen.SetPiece.NearTakerEligibility)
   Inspection.CombinationEligibility.Add(Fact.CardId,Fact.bCanUseTacticalCombination);
 }
 Find<UBorder>(Tree,TEXT("TheaterBodyPadding"))->SetPadding(bSelection?FMargin(0,20,0,20):FMargin(0,32,0,76));
	const bool bMethod=bSetPieceTheater && (Screen.SetPiece.bMethodWait || (bCornerResolution && P.ContestId==TEXT("Corner.Setup") && !P.bDiceRevealVisible));
	const bool bCornerChoice=bCornerResolution && bMethod;
	Show(*Tree.FindWidget(TEXT("TheaterInfoBar"))->GetParent(),!bCornerChoice);
	Find<USizeBox>(Tree,TEXT("TheaterActionBounds"))->SetHeightOverride(bCornerChoice?122.f:72.f);
	for (const auto Name:{TEXT("TheaterHighHintBounds"),TEXT("TheaterLowHintBounds")})
	{
		Show(*Tree.FindWidget(Name),bCornerChoice);
		CastChecked<UVerticalBoxSlot>(Tree.FindWidget(Name)->Slot)->SetPadding(bCornerChoice?FMargin(0,10,0,0):FMargin(0));
	}
	// Match the candidate + inspector row, method button edges, or full duel width.
	// Keep the duel allocation for single-side outcomes so the footer never jumps at reveal.
	Find<USizeBox>(Tree,TEXT("TheaterBottomBounds"))->SetWidthOverride(bShotChoice?860.f:bDeadCorner?1284.f:bSetPieceTheater?(bSelection?1324.f:bMethod?860.f:1284.f):1040.f);
	const bool bPair=(bNear && Screen.SetPiece.NearMethod==EMatchPlayShortFreeKickMethod::Angled)
		|| (bLong && Screen.SetPiece.LongMethod==EMatchPlayLongFreeKickMethod::Power);
	const bool bPanenka=bPenalty && Screen.SetPiece.PenaltyMethod==EMatchPlayPenaltyMethod::Panenka;
	const bool bCompactRoll=bPair || bPanenka || bDeadCorner;
	const bool bFormula=P.bVisible && (IsFormulaContest(P.ContestId) || bSetPieceTheater || bDirectShot || bThroughFormula) && P.bShowFormulaRows;
	Show(*Tree.FindWidget(TEXT("TheaterDuel")),!bSelection && !bShared && !Screen.SetPiece.bNoTakerNoGoal && (!bCornerResolution || !P.AttackRow.Participants.IsEmpty()));
	for (const auto Name:{TEXT("TheaterDirectExplanationBounds"),TEXT("TheaterAlternativeExplanationBounds")})
		Find<USizeBox>(Tree,Name)->SetHeightOverride(bThroughChoice?26.f:bShotChoice?112.f:60.f);
	for (bool bDirect:{true,false})
	{
		auto* Hint=Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectHint"):TEXT("TheaterNearCombinationHint"));
		auto* Slot=CastChecked<UHorizontalBoxSlot>(Hint->Slot);
		Slot->SetSize(FSlateChildSize(bThroughChoice?ESlateSizeRule::Fill:ESlateSizeRule::Automatic));
		Slot->SetPadding(bThroughChoice?FMargin(0):FMargin(14,0,0,0));
		Hint->SetJustification(bThroughChoice?ETextJustify::Center:ETextJustify::Left);
		auto* Bounds=Tree.FindWidget(bDirect?TEXT("TheaterDirectExplanationBounds"):TEXT("TheaterAlternativeExplanationBounds"));
		CastChecked<UVerticalBoxSlot>(Bounds->Slot)->SetPadding(FMargin(0,bThroughChoice?6:10,0,0));
	}
	for (const auto Name:{TEXT("TheaterNearDirectHint"),TEXT("TheaterNearCombinationHint")})
		Find<UTextBlock>(Tree,Name)->SetColorAndOpacity(bShotChoice?White:Quiet);
	Show(*Tree.FindWidget(TEXT("TheaterTakerBounds")),bSelection && !Options.IsEmpty());
	Show(*Tree.FindWidget(TEXT("TheaterSelectionMessageBounds")),bCorner && Options.IsEmpty());
	Show(*Tree.FindWidget(TEXT("TheaterSelectionHintBounds")),bCorner);
	Show(*Tree.FindWidget(TEXT("TheaterSelectionReturnBounds")),bCorner && bSelectionConfirmationPending && !bRequestPending);
	Show(*Tree.FindWidget(TEXT("TheaterNearMethods")),bThroughChoice || bShotChoice || (bMethod && !bCornerChoice));
	const bool bDefenseVisible=!bThroughChoice && !bThroughEvent && !bShotChoice && !bDeadCorner && !bShotRollOnly && (!bSetPieceTheater || (bCornerResolution && !P.DefenseRow.Participants.IsEmpty()) || (bFormula && (P.bShowDefenseRow || (bLong && P.bDiceRevealVisible && !P.RollReel.bStaticResult))));
	Show(*Tree.FindWidget(TEXT("TheaterDefensePanelBounds")),bDefenseVisible);
	Show(*Tree.FindWidget(TEXT("TheaterVS"))->GetParent(),bDefenseVisible);
	Show(*Tree.FindWidget(TEXT("TheaterPair")),bCompactRoll);
	// Panenka consumes the same inline operand with no invented second die or arithmetic.
	auto* CompactOperands=Find<UHorizontalBox>(Tree,TEXT("TheaterPair"));
	for (int32 I=1;I<CompactOperands->GetChildrenCount();++I) Show(*CompactOperands->GetChildAt(I),!bPanenka);
	if (bSetPieceTheater)
	{
		FFMCodexUMGCardRackViewModel Candidates; Candidates.bLocalRack=true; Candidates.ColumnCount=4;
		Candidates.SideLabel=Screen.Interaction.ExpectedActorLabel;
		int32 SelectedCount=0;
		for (const auto* Rack:{&Screen.LocalRack,&Screen.OpponentRack})
			for (const auto& Cell:Rack->Cells) if (Options.Contains(Cell.Card.CardId) && Cell.bSetPieceSelected) ++SelectedCount;
		for (FName Id:Options)
			for (const auto* Rack:{&Screen.LocalRack,&Screen.OpponentRack})
				if (const auto* Cell=Rack->Cells.FindByPredicate([Id](const auto& V){return V.Card.CardId==Id;}))
				{
					auto Copy=*Cell; Copy.StableIndex=Candidates.Cells.Num(); Copy.bDeploymentDraggable=false;
					Copy.bSetPieceSelectable=!bRequestPending && (!bCorner || (!bSelectionConfirmationPending && (Copy.bSetPieceSelected || SelectedCount<3)));
					Candidates.Cells.Add(Copy); break;
				}
		auto* Takers=Find<UFMCodexCardRackWidget>(Tree,TEXT("TheaterTakers"));
		Takers->RefreshFromPresentation(Candidates);
		const uint32 Generation=++Inspection.Generation;
		for (UFMCodexPlayerCardWidget* Card:Takers->GetRenderedCardWidgets())
		{
			const auto* Cell=Candidates.Cells.FindByPredicate([Card](const auto& V){return V.Card.CardId==Card->GetPresentation().CardId;});
			Card->SetRenderOpacity(bCorner && Cell && !Cell->bSetPieceSelectable && !Cell->bSetPieceSelected?.45f:1.f);
			Card->OnDetailHoverRequested.AddWeakLambda(&Tree,[&Tree,&Inspection,Generation](UFMCodexPlayerCardWidget* Hovered)
    { if (Inspection.bActive && Inspection.Generation==Generation) { Inspection.HoveredId=Hovered->GetPresentation().CardId; RefreshTakerHelper(Tree,Inspection); } });
			Card->OnDetailHoverDismissed.AddWeakLambda(&Tree,[&Tree,&Inspection,Generation](UFMCodexPlayerCardWidget* Hovered)
    { if (Inspection.bActive && Inspection.Generation==Generation && Inspection.HoveredId==Hovered->GetPresentation().CardId) { Inspection.HoveredId=NAME_None; RefreshTakerHelper(Tree,Inspection); } });
		}
		for (bool bDirect:{true,false})
		{
			const bool bEnabled=bMethod && !bRequestPending
				&& (bPenalty?Screen.SetPiece.PenaltyMethods.Contains(bDirect?EMatchPlayPenaltyMethod::Direct:EMatchPlayPenaltyMethod::Panenka)
				:bLong?Screen.SetPiece.LongMethods.Contains(bDirect?EMatchPlayLongFreeKickMethod::Direct:EMatchPlayLongFreeKickMethod::Power)
				:Screen.SetPiece.NearMethods.Contains(bDirect?EMatchPlayShortFreeKickMethod::Direct:EMatchPlayShortFreeKickMethod::Angled));
			Find<UButton>(Tree,bDirect?TEXT("TheaterNearDirect"):TEXT("TheaterNearCombination"))->SetIsEnabled(bEnabled);
			Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectLabel"):TEXT("TheaterNearCombinationLabel"))->SetColorAndOpacity(bEnabled?Ink:Quiet);
		}
		Find<UTextBlock>(Tree,TEXT("TheaterNearDirectLabel"))->SetText(bPenalty?LOCTEXT("PenaltyDirect","常规点球"):LOCTEXT("NearDirect","直接射门"));
		Find<UFMCodexMatchFlowDiagram>(Tree,TEXT("TheaterNearDirectDiagram"))->SetDiagram(bPenalty?EFMCodexFlowDiagram::PenaltyDirect:EFMCodexFlowDiagram::Direct);
		Find<UTextBlock>(Tree,TEXT("TheaterNearCombinationLabel"))->SetText(bPenalty?LOCTEXT("PenaltyPanenka","勺子点球"):bLong?FFMCodexPlayerUIPresentationText::LongFreeKickPowerStage():LOCTEXT("NearCombination","战术配合"));
		Find<UFMCodexMatchFlowDiagram>(Tree,TEXT("TheaterNearCombinationDiagram"))->SetDiagram(bPenalty?EFMCodexFlowDiagram::PenaltyChip:bLong?EFMCodexFlowDiagram::Power:EFMCodexFlowDiagram::Combination);
		Find<UTextBlock>(Tree,TEXT("TheaterNearDirectHint"))->SetText(bPenalty?LOCTEXT("PenaltyDirectHint","取射门 / 传球较高值\n对抗门将预判（门将预判 -3）"):bLong?LOCTEXT("LongDirectHint","远射对抗门将站位 + 2\n进攻掷点 1–2 直接射偏"):LOCTEXT("NearDirectHint","取射门 / 传球较高值\n对抗门将手控球 + 防守加成"));
		Find<UTextBlock>(Tree,TEXT("TheaterNearCombinationHint"))->SetText(bPenalty?LOCTEXT("PenaltyPanenkaHint","掷一枚骰子\n1 射失，2–6 进球"):bLong?LOCTEXT("LongPowerHint","无属性门槛\n两枚骰子总和 ≥ 11 进球"):LOCTEXT("NearCombinationHint","需射门 + 传球 ≥ 8\n两枚骰子总和 ≥ 9 进球"));
		for (int32 I=0;I<2;++I)
		{
			const FString Prefix=I==0?TEXT("TheaterPairA"):TEXT("TheaterPairB");
			const auto* Operand=P.AttackRow.Terms.FindByPredicate([I](const auto& Term){return Term.Kind==K::RawRoll && Term.RollSequenceIndex==I;});
			const bool bReel=bCompactRoll && P.bDiceRevealVisible && P.ActiveRollSequenceIndex==I;
			const bool bResolved=bCompactRoll && Operand && Operand->bResolved;
			Show(*Tree.FindWidget(Named(Prefix,TEXT("ReelHost"))),bReel);
			Show(*Tree.FindWidget(Named(Prefix,TEXT("Pending"))),!bReel && !bResolved);
			Show(*Tree.FindWidget(Named(Prefix,TEXT("RollValue"))),!bReel && bResolved);
			SetText(Tree,Named(Prefix,TEXT("RollValue")),bResolved?FString::FromInt(Operand->RawD6):FString());
		}
		SetText(Tree,TEXT("TheaterPairTotal"),bPair && P.AttackRow.bFinalValueResolved?P.AttackRow.FinalValueLabel:FString(TEXT("?")));
		Find<UTextBlock>(Tree,TEXT("TheaterPairTotal"))->SetColorAndOpacity(P.AttackRow.bFinalValueResolved?Gold:Quiet);
	}
	// Future safe facts may already exist while the visible route is still gated.
	const FName DisclosedContest=bFormula ? P.ContestId
		: (P.ContestId==TEXT("Cross.Route") && !P.RouteResultLabel.IsEmpty() ? Screen.InlineFormula.ContestId : NAME_None);
	const bool bTransitionResult=bThroughFormula && P.ContestId==TEXT("ThroughBall.BehindDefense.P1")
		&& P.bNarrativeAvailable && P.bNarrativeAttackSuccess;
	const bool bFinal=bTransitionResult || ((bThrough || bFormula || bSetPieceTheater || bDirectShot || bDeadCorner)
		&& FMCodexOutcomePresentation::IsFinalReady(P.bNarrativeAvailable,P.bDiceRevealVisible));
	// Setup deliberately has no visible Formula; its public participant rows
	// live on the safe Screen projection, not the empty displayed Formula.
	const auto& ParticipantSurface=Screen.InlineFormula.ContestId==TEXT("Cross.Setup")
		? Screen.InlineFormula : P;
	auto Attack=ParticipantSurface.AttackRow;
	if (bSetPieceTheater && !bCornerResolution && !bFormula && !bCompactRoll)
	{
		Attack.SideLabel=TEXT("进攻"); Attack.Participants.Reset();
		if (!Screen.SetPiece.TakerCardId.IsNone()) Attack.Participants.Add({TEXT("主罚球员"),Screen.SetPiece.TakerLabel.ToString()});
	}
	const auto& Defense=ParticipantSurface.DefenseRow;
	// Narrative success is projected from authoritative winner facts. Do not
	// compare totals: rapid suppression can legitimately defeat the larger total.
	RefreshSide(Tree,TEXT("TheaterAttack"),Attack,bFormula,P.bAttackRowActive || (bShotRollOnly && P.bDiceRevealVisible),P.bDiceRevealVisible && P.RollReel.bVisible,bFinal && !bShotRollOnly && P.bNarrativeAttackSuccess,bShotRollOnly);
	RefreshSide(Tree,TEXT("TheaterDefense"),Defense,bFormula,P.bDefenseRowActive,P.bDiceRevealVisible && P.RollReel.bVisible,bFinal && !bShotRollOnly && !P.bNarrativeAttackSuccess);
	Find<UTextBlock>(Tree,TEXT("TheaterTitle"))->SetText(bDirectShot ? FText::FromString(P.ResolutionContextLabel) : bSetPieceTheater ? (bFinal && (bCornerResolution || Screen.SetPiece.NearMethod!=EMatchPlayShortFreeKickMethod::None || Screen.SetPiece.LongMethod!=EMatchPlayLongFreeKickMethod::None || Screen.SetPiece.PenaltyMethod!=EMatchPlayPenaltyMethod::None)?FText::FromString(P.ResolutionContextLabel):FFMCodexPlayerUIPresentationText::SetPieceName(Screen.SetPiece.Type)) : DisclosedContest==TEXT("Cross.High") ? LOCTEXT("HighCross","高球传中")
		: DisclosedContest==TEXT("Cross.Low") ? LOCTEXT("LowCross","低球传中") : LOCTEXT("Cross","传中"));
	auto* Title=Find<UTextBlock>(Tree,TEXT("TheaterTitle"));
	auto TitleFont=Title->GetFont(); TitleFont.Size=bFinal?26:52; Title->SetFont(TitleFont);
	Show(*Title,true);
	auto* Outcome=Find<URichTextBlock>(Tree,TEXT("TheaterOutcome"));
	Outcome->SetText(bFinal ? FText::FromString(FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel,P.OutcomeText)) : FText::GetEmpty()); Show(*Outcome,bFinal);
	Show(*Tree.FindWidget(TEXT("TheaterOutcomeDivider")),bFinal);
	Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bFinal ? FText::GetEmpty()
		: bSetPieceTheater ? (bSelection?LOCTEXT("NearSelect","选择主罚球员"):bMethod?LOCTEXT("NearMethod","选择结算方式")
			:bPenalty?(bPanenka?LOCTEXT("PenaltyPanenka","勺子点球"):LOCTEXT("PenaltyDirectStage","常规点球 · 进球判定")):bPair?(bLong?FFMCodexPlayerUIPresentationText::LongFreeKickPowerStage():LOCTEXT("NearPairStage","战术配合")):LOCTEXT("NearDirectStage","直接射门 · 进球判定"))
		: bFormula || bDirectShot ? LOCTEXT("Contest","进球判定") : Screen.Interaction.Category==C::SelectBranchIntent
		? LOCTEXT("Setup","选择传中方式") : LOCTEXT("Route","路线判定"));
	// H is BuildDisplayedHeader output, never the un-gated authoritative score.
	const bool bLeftA=H.LeftPlayerSide==EInitialTurnOrderPlayer::PlayerA;
	SetText(Tree,TEXT("TheaterContext"),FString::Printf(TEXT("%s  %s  –  %s  %s"),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.LeftPlayerLabel).ToString(),
		*(bLeftA ? H.PlayerAScoreLabel : H.PlayerBScoreLabel),*(bLeftA ? H.PlayerBScoreLabel : H.PlayerAScoreLabel),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.RightPlayerLabel).ToString()));
	const bool bConfirm=bSelection && (Screen.LocalRack.Cells.ContainsByPredicate([](const auto& V){return V.bSetPieceSelected;})
		|| Screen.OpponentRack.Cells.ContainsByPredicate([](const auto& V){return V.bSetPieceSelected;}));
	const bool bAction=(bConfirm || (P.bVisible && P.PrimaryAction.bVisible && P.PrimaryAction.Action.bAvailable)) && !bRequestPending;
	Show(*Tree.FindWidget(TEXT("TheaterPrimaryBounds")),bAction);
	Find<UButton>(Tree,TEXT("TheaterContinue"))->SetIsEnabled(bAction);
	const C Category=P.PrimaryAction.Action.Category;
	const bool bRoll=Category==C::RollLongShotDeadCorner || Category==C::RollCutInsideShotDeadCorner || Category==C::RollLongShotDirectAttack || Category==C::RollLongShotDirectDefense || Category==C::RollCutInsideShotDirectAttack || Category==C::RollCutInsideShotDirectDefense || Category==C::RollCrossRoute || Category==C::RollCrossAttack || Category==C::RollCrossDefense || (Category==C::RollShortFreeKickDirectAttack || Category==C::RollLongFreeKickDirectAttack)
		|| (Category==C::RollShortFreeKickDirectDefense || Category==C::RollLongFreeKickDirectDefense) || (Category==C::RollShortFreeKickAngled || Category==C::RollLongFreeKickPower) || Category==C::RollPenaltyDirectAttack || Category==C::RollPenaltyDirectDefense || Category==C::RollPenaltyPanenka || Category==C::RollCornerParticipantSelection || Category==C::RollCornerRoute || Category==C::RollCornerAttack || Category==C::RollCornerDefense
		|| (bThroughFormula && Category!=C::AdvanceAfterTerminal);
	Find<UTextBlock>(Tree,TEXT("TheaterContinueLabel"))->SetText(bConfirm?LOCTEXT("NearConfirm","确认主罚球员"):Category==C::RollCrossAttack ? LOCTEXT("AttackRoll","进攻方掷点")
		: Category==C::RollCrossDefense ? LOCTEXT("DefenseRoll","防守方掷点")
		: Category==C::RollCrossRoute ? LOCTEXT("RouteRoll","判定路线")
		: (Category==C::RollShortFreeKickAngled || Category==C::RollLongFreeKickPower) ? LOCTEXT("NearPairRoll","掷两枚骰子")
		: Category==C::RollPenaltyPanenka ? LOCTEXT("PenaltySingleRoll","掷一枚骰子")
		: bFinal ? LOCTEXT("NextAttack","下一回合") : FText::FromString(P.PrimaryAction.Action.Label));
	Find<UButton>(Tree,TEXT("TheaterContinue"))->SetBackgroundColor(FLinearColor::White);
	Show(*Tree.FindWidget(TEXT("TheaterDiceIcon")),bRoll);
	Show(*Tree.FindWidget(TEXT("TheaterNextIcon")),!bRoll);
	Show(*Tree.FindWidget(TEXT("TheaterRollLeading")),bRoll);
	Show(*Tree.FindWidget(TEXT("TheaterNextTrailing")),!bRoll);
	bool bChoices=false;
	for (const auto Intent : {EFMCodexUMGBranchIntent::CrossHigh,EFMCodexUMGBranchIntent::CrossLow})
	{
		const bool bHigh=Intent==EFMCodexUMGBranchIntent::CrossHigh;
		const bool bChoice=Screen.Interaction.Category==C::SelectBranchIntent && !bRequestPending
			&& Screen.Interaction.BranchChoices.ContainsByPredicate([Intent](const auto& V) { return V.Intent==Intent; });
		auto* Choice=Find<UButton>(Tree,bHigh ? TEXT("TheaterHigh") : TEXT("TheaterLow"));
		Show(*Choice,bChoice || bCornerChoice);
		if (bCornerChoice) Choice->SetVisibility(ESlateVisibility::Visible);
		Choice->SetIsEnabled(bCornerChoice?(!bRequestPending && Screen.SetPiece.bCornerIntentWait && !Screen.bActionWaitPromptReadOnly):bChoice);
		Find<UTextBlock>(Tree,bHigh?TEXT("TheaterHighLabel"):TEXT("TheaterLowLabel"))->SetText(bCornerChoice
			?(bHigh?LOCTEXT("CornerHigh","高球"):LOCTEXT("CornerLow","低球"))
			:(bHigh?LOCTEXT("High","高球传中"):LOCTEXT("Low","低球传中")));
		bChoices|=bChoice || bCornerChoice;
	}
	Show(*Tree.FindWidget(TEXT("TheaterChoices")),bChoices);
	// Compact information/reason bar remains separate from the independent CTA.
	FText Detail;
	bool bPrimaryOwnsRollStatus=false;
	bool bActionOwnerOnly=false;
	if (bFinal) Detail=FText::FromString(P.ResolutionReasonLabel);
	else if (bSelection) Detail=bConfirm?LOCTEXT("NearDraft","已选主罚球员，确认后继续")
		:Screen.SetPiece.TakerOptions.IsEmpty()?LOCTEXT("NearWaitTaker","等待进攻方选择主罚球员"):LOCTEXT("NearChooseTaker","请选择主罚球员");
	else if (bMethod) Detail=(bPenalty?Screen.SetPiece.PenaltyMethods.IsEmpty():bLong?Screen.SetPiece.LongMethods.IsEmpty():Screen.SetPiece.NearMethods.IsEmpty())?LOCTEXT("NearWaitMethod","等待进攻方选择结算方式")
		:bNear && !Screen.SetPiece.NearMethods.Contains(EMatchPlayShortFreeKickMethod::Angled)?LOCTEXT("NearIneligible","战术配合不可用：需射门 + 传球 ≥ 8")
		:bPenalty?LOCTEXT("PenaltyChooseMethod","当前主罚球员可选择常规点球或勺子点球"):bLong?LOCTEXT("LongChooseMethod","当前主罚球员可选择直接射门或重炮轰门"):LOCTEXT("NearChooseMethod","当前主罚球员可选择直接射门或战术配合");
	else if (bPanenka || bShotRollOnly)
	{
		Detail=P.bDiceRevealVisible?LOCTEXT("AttackRolling","进攻方掷点中"):bAction?LOCTEXT("AttackTurn","轮到进攻方掷点"):LOCTEXT("WaitAttack","等待进攻方掷点");
		bPrimaryOwnsRollStatus=P.bDiceRevealVisible;
		bActionOwnerOnly=!P.bDiceRevealVisible;
	}
	else if (bPair) Detail=FText::FromString(P.RollHelperLabel.IsEmpty()
		? FFMCodexPlayerUIPresentationText::SetPieceCompactOutcomeHint(Screen.SetPiece.Type).ToString():P.RollHelperLabel);
	else if (Screen.Interaction.Category==C::SelectBranchIntent) Detail=LOCTEXT("ChooseCross","请选择传中方式");
	else if (bFormula)
	{
		bActionOwnerOnly=!P.bDiceRevealVisible;
		const bool bAttackTurn=P.bAttackRowActive;
		if (P.bDiceRevealVisible)
		{
			Detail=bAttackTurn ? LOCTEXT("AttackRolling","进攻方掷点中") : LOCTEXT("DefenseRolling","防守方掷点中");
			bPrimaryOwnsRollStatus=true;
		}
		else if (bAction) Detail=bAttackTurn ? LOCTEXT("AttackTurn","轮到进攻方掷点") : LOCTEXT("DefenseTurn","轮到防守方掷点");
		else Detail=bAttackTurn ? LOCTEXT("WaitAttack","等待进攻方掷点") : LOCTEXT("WaitDefense","等待防守方掷点");
	}
	else if (P.ContestId==TEXT("Cross.Route") && P.bDiceRevealVisible)
	{
		const bool bLanded=P.RollReel.bStaticResult && P.RollReel.bAuthoritativeValue
			&& !P.RouteResultLabel.IsEmpty();
		if (bLanded && (DisclosedContest==TEXT("Cross.High") || DisclosedContest==TEXT("Cross.Low")))
			Detail=FText::Format(DisclosedContest==TEXT("Cross.High")
				? LOCTEXT("RouteHighLanded","掷点结果为 {0}，判定为高球传中")
				: LOCTEXT("RouteLowLanded","掷点结果为 {0}，判定为低球传中"), FText::AsNumber(P.RollReel.CenterValue));
		else Detail=LOCTEXT("RouteRolling","正在判定传中路线");
	}
	else Detail=FText::FromString(P.RollHelperLabel);
	// Reveal remains active during ResultHold. Describe the visible landed die,
	// not the reveal lifetime, without reading the still-gated terminal result.
	if (bDirectShot && !bFinal && P.bDiceRevealVisible
		&& P.RollReel.bStaticResult && P.RollReel.bAuthoritativeValue)
		Detail=P.bDefenseRowActive?LOCTEXT("ShotDefenseLanded","防守方掷点已落定")
			:LOCTEXT("ShotAttackLanded","进攻方掷点已落定");
	FString MainReason=Detail.ToString(), SecondaryReason;
	if (bFinal) Detail.ToString().Split(TEXT("\n"),&MainReason,&SecondaryReason);
	if (bDeadCorner && bFinal)
	{
		// Explain the already-disclosed outcome; never decide it from the sum.
		if (P.OutcomeText.Accent==EFMCodexOutcomeAccent::NoGoal)
			SecondaryReason=LOCTEXT("DeadCornerMissReason","总和未达到 11–12，未进球").ToString();
		else if (P.OutcomeText.Accent==EFMCodexOutcomeAccent::Goal)
			SecondaryReason=LOCTEXT("DeadCornerGoalReason","总和达到 11–12，进球").ToString();
	}
	Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(FText::FromString(MainReason));
	SetText(Tree,TEXT("TheaterReasonSecondary"),SecondaryReason);
	if (bPanenka && !bFinal)
	{
		SecondaryReason=FFMCodexPlayerUIPresentationText::SetPieceCompactOutcomeHint(Screen.SetPiece.Type).ToString();
		SetText(Tree,TEXT("TheaterReasonSecondary"),SecondaryReason);
	}
	if ((bLong || bDirectShot) && bFormula && !bFinal && P.bAttackRowActive)
	{
		SecondaryReason=LOCTEXT("LongAttackMissHint","进攻掷点 1–2：直接射偏").ToString();
		SetText(Tree,TEXT("TheaterReasonSecondary"),SecondaryReason);
	}
	if (bThroughFormula && P.ContestId==TEXT("ThroughBall.BehindDefense.P1") && bFormula && !bFinal && P.bAttackRowActive)
	{
		SecondaryReason=FFMCodexPlayerUIPresentationText::ThroughBallBehindDefenseOutcomeHint().ToString();
		SetText(Tree,TEXT("TheaterReasonSecondary"),SecondaryReason);
	}
	Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),!SecondaryReason.IsEmpty());
	Show(*Tree.FindWidget(TEXT("TheaterReasonMark"))->GetParent(),bFinal);
	Show(*Tree.FindWidget(TEXT("TheaterReasonSeparator"))->GetParent(),bFinal);
	auto* Reason=Find<URichTextBlock>(Tree,TEXT("TheaterReasonPrimary"));
	Reason->SetText(bFinal?FText::FromString(ReasonMarkup(MainReason)):FText::GetEmpty());
	Show(*Reason,bFinal && !MainReason.IsEmpty());
	Show(*Tree.FindWidget(TEXT("TheaterDetail")),!bFinal && !Detail.IsEmpty());
	// The resolved composition is denser; the reel's stable allocation is untouched.
	for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
	{
		CastChecked<UVerticalBoxSlot>(Tree.FindWidget(Named(Prefix,TEXT("Value")))->Slot)->SetPadding(FMargin(0,bFinal && !bShotRollOnly?4:14,0,0));
	}
	// The CTA owns the operation; retain only actor/wait ownership outside it.
	// Never consume StatusLabel here: it can alias an Outcome during ResultHold.
	FString Status=P.bDiceRevealVisible ? P.DiceOwnerLabel : FString();
	bool bHelperOnlyNamesRollOwner=P.bDiceRevealVisible;
	if (Screen.bMirrorActionWaitPrompt && !P.bDiceRevealVisible)
	{
		Status=Screen.ActionWaitActorText.ToString()+TEXT("  ")+Screen.ActionWaitActionText.ToString();
		bHelperOnlyNamesRollOwner=false;
	}
	else if (bAction || bChoices || bMethod || bSelection)
	{
		Status=FFMCodexPlayerUIPresentationText::MatchScreenLabel(Screen.Interaction.ExpectedActorLabel).ToString();
		bHelperOnlyNamesRollOwner=false;
	}
	if (bRequestPending)
	{
		Status=LOCTEXT("PendingRequest","操作已提交，等待确认").ToString();
		bHelperOnlyNamesRollOwner=false;
	}
	if (bCornerResolution) Status.ReplaceInline(TEXT("低平球"),TEXT("低球"));
	SetText(Tree,TEXT("TheaterStatus"),Status);
	// De-duplicate by presentation meaning, never by translated string matching.
	// Actor/wait/pending and pair-die sequence helpers add information; keep them.
	// Hidden preserves the same allocation as visible text throughout the reveal.
	// Only the action-only branches above move authoritative ownership into the
	// main bar. Rule, success-condition and result/reason branches retain theirs.
	if (bActionOwnerOnly && !Status.IsEmpty())
	{
		Detail=FText::FromString(Status);
		Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(Detail);
	}
	const bool bRedundantHelper=(bActionOwnerOnly && !Status.IsEmpty())
		|| (bPrimaryOwnsRollStatus && bHelperOnlyNamesRollOwner);
	Tree.FindWidget(TEXT("TheaterStatus"))->SetVisibility(bRedundantHelper?ESlateVisibility::Hidden:ESlateVisibility::SelfHitTestInvisible);
	// Taker rules are peers; result/roll explanations retain their supporting hierarchy.
	auto* RuleLine=Find<UTextBlock>(Tree,TEXT("TheaterReasonSecondary"));
	auto RuleFont=Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->GetFont();
	if (!bInspect && !bCorner) RuleFont.Size=14;
	RuleLine->SetFont(RuleFont); RuleLine->SetColorAndOpacity(bInspect || bCorner?White:Quiet);
	Show(*Tree.FindWidget(TEXT("TheaterRoll")),P.bDiceRevealVisible && !bFormula && !bCompactRoll && !bShared && !bShotConsumer && !bThrough);
	RefreshParticipantDraw(Tree,Screen,P,bShared);
	RefreshReel(Tree,P.RollReel);
	if (bInspect) RefreshTakerHelper(Tree,Inspection);
	if (bCornerResolution)
	{
		if (bFormula) Find<UTextBlock>(Tree,TEXT("TheaterTitle"))->SetText(FText::FromString(P.ResolutionContextLabel));
		if (!bFinal)
			Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bShared?LOCTEXT("CornerDraw","一枚共同骰子，确定双方实际球员")
				:bMethod?LOCTEXT("CornerChooseRoute","选择角球路线"):P.ContestId==TEXT("Corner.Route")?LOCTEXT("CornerRoute","路线判定")
				:LOCTEXT("Contest","进球判定"));
		if (bMethod) Detail=FText::GetEmpty();
		else if (bShared)
			Detail=P.RouteResultLabel.IsEmpty()?(P.bDiceRevealVisible?LOCTEXT("CornerSelecting","正在确定双方实际球员"):LOCTEXT("CornerDrawHint","同一掷点确定双方实际球员；号位表示锁定顺序"))
				:LOCTEXT("CornerSelected","实际进攻与防守球员已选定；其余候选不消耗");
		else if (!bFinal && !bFormula && P.ContestId==TEXT("Corner.Route"))
			Detail=!P.RouteResultLabel.IsEmpty()?FText::FromString(P.RouteResultLabel)
				:P.bDiceRevealVisible?LOCTEXT("CornerRouteRolling","正在判定角球路线"):FText::FromString(P.RollHelperLabel);
		if (!bFinal) Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(Detail);
		if (!bFinal) Show(*Tree.FindWidget(TEXT("TheaterDetail")),!Detail.IsEmpty());
		if (P.ContestId==TEXT("Corner.Route") && P.bDiceRevealVisible && P.RouteResultLabel.IsEmpty())
		{
			SetText(Tree,TEXT("TheaterReasonSecondary"),P.RollHelperLabel);
			Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),!P.RollHelperLabel.IsEmpty());
		}
		else if (!bFinal && !bShared && Screen.SetPiece.CornerBonus>0)
		{
			Find<UTextBlock>(Tree,TEXT("TheaterReasonSecondary"))->SetText(FText::Format(LOCTEXT("CornerAppliedCount","{0}候选人数优势 +{1}（公式基础值含此加成）"),
				Screen.SetPiece.CornerBonusSide==Screen.SetPiece.AttackingSide?LOCTEXT("AttackSide","进攻方"):LOCTEXT("DefenseSide","防守方"),FText::AsNumber(Screen.SetPiece.CornerBonus)));
			Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),true);
		}
	}
	if (bShotChoice || bDeadCorner)
	{
		Find<UTextBlock>(Tree,TEXT("TheaterTitle"))->SetText(FText::FromString(Shot.TitleLabel));
		Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bFinal?FText::GetEmpty():FText::FromString(Shot.StageLabel));
		// Branch availability and ownership are already projected for this viewer.
		if (bShotChoice)
		{
			FString Unavailable;
			for (bool bDirect:{true,false})
			{
				const auto Intent=bDirect?EFMCodexUMGBranchIntent::DirectShot:EFMCodexUMGBranchIntent::DeadCorner;
				const auto* Choice=Shot.BranchChoices.FindByPredicate([Intent](const auto& C){return C.Intent==Intent;});
				auto* Button=Find<UButton>(Tree,bDirect?TEXT("TheaterNearDirect"):TEXT("TheaterNearCombination"));
				const bool bEnabled=Choice && !bRequestPending && !Screen.bActionWaitPromptReadOnly;
				Button->SetIsEnabled(bEnabled);
				Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectLabel"):TEXT("TheaterNearCombinationLabel"))->SetText(
					Choice?FText::FromString(Choice->Label):(bDirect?LOCTEXT("NearDirect","直接射门"):LOCTEXT("ShotDeadCorner","射向死角")));
				Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectLabel"):TEXT("TheaterNearCombinationLabel"))->SetColorAndOpacity(bEnabled?Ink:Quiet);
				Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectHint"):TEXT("TheaterNearCombinationHint"))->SetText(
					FFMCodexTacticalDetailPresentationBuilder::BuildShotBranchExplanation(Shot.SkillType,bDirect));
				Find<UFMCodexMatchFlowDiagram>(Tree,bDirect?TEXT("TheaterNearDirectDiagram"):TEXT("TheaterNearCombinationDiagram"))->SetDiagram(bDirect?EFMCodexFlowDiagram::Direct:EFMCodexFlowDiagram::Power);
				if (!Choice && !Screen.bActionWaitPromptReadOnly)
					Unavailable=FText::Format(LOCTEXT("ShotUnavailable","当前不可选择{0}"),
						bDirect?LOCTEXT("NearDirect","直接射门"):LOCTEXT("ShotDeadCorner","射向死角")).ToString();
			}
			SetText(Tree,TEXT("TheaterDetail"),bRequestPending?LOCTEXT("PendingRequest","操作已提交，等待确认").ToString()
				:Screen.bActionWaitPromptReadOnly?LOCTEXT("ShotWaiting","等待进攻方选择射门方式").ToString()
				:!Unavailable.IsEmpty()?Unavailable:LOCTEXT("ShotChoose","选择本次射门方式").ToString());
			Show(*Tree.FindWidget(TEXT("TheaterDetail")),true);
			Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),false);
			SetText(Tree,TEXT("TheaterStatus"),Screen.bMirrorActionWaitPrompt
				?Screen.ActionWaitActorText.ToString()+TEXT("  ")+Screen.ActionWaitActionText.ToString()
				:FFMCodexPlayerUIPresentationText::MatchScreenLabel(Screen.Interaction.ExpectedActorLabel).ToString());
		}
		if (bDeadCorner)
		{
			for (int32 I=0; I<2; ++I)
			{
				const FString Prefix=I==0?TEXT("TheaterPairA"):TEXT("TheaterPairB");
				const bool bActive=P.bDiceRevealVisible && P.ActiveRollSequenceIndex==I;
				const bool bLanded=I==0?Shot.bDeadCornerAVisible:Shot.bDeadCornerBVisible;
				Show(*Tree.FindWidget(Named(Prefix,TEXT("ReelHost"))),bActive);
				Find<UFMCodexRollReelWidget>(Tree,Named(Prefix,TEXT("Reel")))->RefreshFromPresentation(bActive?P.RollReel:FFMCodexUMGRollReelViewModel());
				Show(*Tree.FindWidget(Named(Prefix,TEXT("Pending"))),!bActive && !bLanded);
				Show(*Tree.FindWidget(Named(Prefix,TEXT("RollValue"))),!bActive && bLanded);
				SetText(Tree,Named(Prefix,TEXT("RollValue")),bLanded?FString::FromInt(I==0?Shot.DeadCornerA:Shot.DeadCornerB):FString());
			}
			// The existing display formatter supplies the sum of accepted dice.
			// No Formula, threshold comparison or outcome is calculated here.
			SetText(Tree,TEXT("TheaterPairTotal"),P.AttackRow.bFinalValueResolved?P.AttackRow.FinalValueLabel:FString(TEXT("?")));
			Find<UTextBlock>(Tree,TEXT("TheaterPairTotal"))->SetColorAndOpacity(P.AttackRow.bFinalValueResolved?Gold:Quiet);
			if (!bFinal)
			{
				const FText GoalHint=LOCTEXT("DeadCornerRollingGoalHint","两枚掷点总和达到 11–12：进球");
				SetText(Tree,TEXT("TheaterDetail"),P.bDiceRevealVisible?Shot.StatusLabel:GoalHint.ToString());
				Show(*Tree.FindWidget(TEXT("TheaterDetail")),true);
				Find<UTextBlock>(Tree,TEXT("TheaterReasonSecondary"))->SetText(GoalHint);
				Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),P.bDiceRevealVisible);
				if (P.bDiceRevealVisible) Tree.FindWidget(TEXT("TheaterStatus"))->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	if (bThrough)
	{
		for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
			Find<UTextBlock>(Tree,Named(Prefix,TEXT("FinalNumber")))->SetColorAndOpacity(White);
		Find<UTextBlock>(Tree,TEXT("TheaterTitle"))->SetText(LOCTEXT("ThroughBall","直塞"));
		const FText Stage=bThroughChoice?LOCTEXT("OneOnOneMethod","单刀 · 选择射门方式")
			:P.ContestId==TEXT("ThroughBall.Feet")?LOCTEXT("ThroughBallFeet","脚下球")
			:P.ContestId==TEXT("ThroughBall.BehindDefense.P1")?LOCTEXT("ThroughBallBehind","身后球")
			:P.ContestId==TEXT("ThroughBall.OneOnOne.DirectShot")?LOCTEXT("ThroughBallDirect","单刀 · 直接射门")
			:FText::FromString(Through.StageLabel);
		Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bFinal?FText::GetEmpty():Stage);
		Show(*Tree.FindWidget(TEXT("TheaterDuel")),!bThroughEvent || !P.AttackRow.Participants.IsEmpty());
		Show(*Tree.FindWidget(TEXT("TheaterAttackRollCaption")),bShotRollOnly);
		if (bThroughChoice)
		{
			for (bool bDirect:{true,false})
			{
				const auto Method=bDirect?EFMCodexUMGOneOnOneChoice::DirectShot:EFMCodexUMGOneOnOneChoice::ChipShot;
				const auto* Choice=Through.OneOnOneChoices.FindByPredicate([Method](const auto& V){return V.Choice==Method;});
				const bool bEnabled=Choice && !bRequestPending && !Screen.bActionWaitPromptReadOnly;
				Find<UButton>(Tree,bDirect?TEXT("TheaterNearDirect"):TEXT("TheaterNearCombination"))->SetIsEnabled(bEnabled);
				auto* Label=Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectLabel"):TEXT("TheaterNearCombinationLabel"));
				Label->SetText(Choice?FText::FromString(Choice->Label):bDirect?LOCTEXT("NearDirect","直接射门"):LOCTEXT("ChipMethod","挑射"));
				Label->SetColorAndOpacity(bEnabled?Ink:Quiet);
				Find<UTextBlock>(Tree,bDirect?TEXT("TheaterNearDirectHint"):TEXT("TheaterNearCombinationHint"))->SetText(bDirect
					?LOCTEXT("ThroughBallDirectHint","比较射门与门将单刀"):LOCTEXT("ThroughBallChipHint","仅按掷点判定"));
				// A method comparison does not introduce a spatial tactical scene.
				Show(*Tree.FindWidget(bDirect?TEXT("TheaterNearDirectDiagram"):TEXT("TheaterNearCombinationDiagram")),false);
			}
			SetText(Tree,TEXT("TheaterDetail"),LOCTEXT("OneOnOneChoose","已形成单刀，选择射门方式").ToString());
			Show(*Tree.FindWidget(TEXT("TheaterDetail")),true);
			SetText(Tree,TEXT("TheaterStatus"),bRequestPending?LOCTEXT("PendingRequest","操作已提交，等待确认").ToString():Screen.bMirrorActionWaitPrompt
				?Screen.ActionWaitActorText.ToString()+TEXT("  ")+Screen.ActionWaitActionText.ToString()
				:FFMCodexPlayerUIPresentationText::MatchScreenLabel(Screen.Interaction.ExpectedActorLabel).ToString());
		}
		if (bThroughEvent && bFinal && P.ResolutionReasonLabel.IsEmpty())
			Show(*Tree.FindWidget(TEXT("TheaterInfoBar"))->GetParent(),false);
	}
	if (bShotConsumer || bThrough)
	{
		// Preserve the projected wording. Allocate its measured width including
		// icon, separator and padding instead of allowing long CJK labels to overflow.
		auto* Label=Find<UTextBlock>(Tree,TEXT("TheaterContinueLabel"));
		auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		Find<USizeBox>(Tree,TEXT("TheaterPrimaryBounds"))->SetWidthOverride(
			FMath::Max(320.f, float(Measure->Measure(Label->GetText(),Label->GetFont()).X)+120.f));
	}
	else Find<USizeBox>(Tree,TEXT("TheaterPrimaryBounds"))->SetWidthOverride(320.f);
	if (bCorner)
	{
		// Only existing viewer-safe nomination facts and transient draft styling.
		// Post-lock execution has a separate Development gate; planning stays unchanged.
		const bool bAttack=Screen.SetPiece.CornerStage==EMatchPlaySetPieceCornerRouteStage::AwaitingAttackerNominations;
		const bool bActor=Screen.Interaction.PrimaryAction.bAvailable && !Screen.bActionWaitPromptReadOnly;
		const auto& Cells=Find<UFMCodexCardRackWidget>(Tree,TEXT("TheaterTakers"))->GetPresentation().Cells;
		int32 Count=0;
		for (const auto& Cell:Cells) if (Cell.bSetPieceSelected) ++Count;
		Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bActor
			? FText::Format(LOCTEXT("CornerPlanningCount","{0} · 已选 {1} / 3"),bAttack?LOCTEXT("CornerAttackSelection","选择进攻候选"):LOCTEXT("CornerDefenseSelection","选择防守候选"),FText::AsNumber(Count))
			: bAttack?LOCTEXT("CornerWaitAttack","等待进攻方选择候选球员"):LOCTEXT("CornerWaitDefense","等待防守方选择候选球员"));
		const FText Zero=bAttack?LOCTEXT("CornerZeroAttack","可不派进攻候选；<Danger>若进攻方锁定 0 人，本次角球直接不进球</>")
			:LOCTEXT("CornerZeroDefense","可不派防守候选；<Danger>若防守方锁定 0 人且进攻方有候选，对方直接进球</>");
		Find<URichTextBlock>(Tree,TEXT("TheaterSelectionHint"))->SetText(!bActor
			?LOCTEXT("CornerSealedWait","双方锁定后公开候选名单，再确定实际对位球员")
			:bSelectionConfirmationPending && Count<3?(Count==0
				?FText::Format(LOCTEXT("CornerConfirmZeroDraft","{0}；确认继续锁定，或返回补充"),Zero)
				:FText::Format(LOCTEXT("CornerConfirmUnderfilledDraft","已选 {0}/3，<Danger>未选满 3 人</>；确认继续锁定，或返回补充"),FText::AsNumber(Count)))
			:Count==0?Zero:Count==3?LOCTEXT("CornerFullDraft","已选满 3 人；可再次点击已选球员撤销，编号表示选人顺序")
			:LOCTEXT("CornerOrderedDraft","按顺序选择 0–3 名候选；再次点击已选球员撤销，编号表示选人顺序"));
		FString Message=bActor?LOCTEXT("CornerNoCandidates","当前没有可用候选球员\n可确认不派候选").ToString()
			:LOCTEXT("CornerSealedMessage","候选名单封存中\n等待当前玩家锁定").ToString();
		if (!bActor && Screen.SetPiece.bCornerAttackerLocked && !Screen.SetPiece.bHideCornerAttackerDetails)
		{
			Message=LOCTEXT("CornerOwnLocked","进攻候选已锁定").ToString();
			for (int32 I=0;I<Screen.SetPiece.CornerAttackers.Num();++I)
				for (const auto* Rack:{&Screen.LocalRack,&Screen.OpponentRack})
					if (const auto* Cell=Rack->Cells.FindByPredicate([&](const auto& V){return V.Card.CardId==Screen.SetPiece.CornerAttackers[I];}))
					{ Message+=FString::Printf(TEXT("\n%d. %s"),I+1,*Cell->Card.IdentityLabel); break; }
			if (Screen.SetPiece.CornerAttackers.IsEmpty()) Message+=TEXT("\n")+LOCTEXT("CornerNoneLocked","不派进攻候选").ToString();
		}
		SetText(Tree,TEXT("TheaterSelectionMessageText"),Message);
		Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(bAttack
			?LOCTEXT("CornerAttackAttributes","高球看力量，低球看射门")
			:LOCTEXT("CornerDefenseAttributes","高球看力量，低球看盯防"));
		Find<UTextBlock>(Tree,TEXT("TheaterReasonSecondary"))->SetText(LOCTEXT("CornerCountRule","候选人数多 1 人最终点数 +2，多 2 人 +3；未选中参与角球球员不消耗"));
		Show(*Tree.FindWidget(TEXT("TheaterDetail")),true); Show(*Tree.FindWidget(TEXT("TheaterReasonSecondary")),true);
		Show(*Tree.FindWidget(TEXT("TheaterPrimaryBounds")),bActor && !bRequestPending);
		Find<UButton>(Tree,TEXT("TheaterContinue"))->SetIsEnabled(bActor && !bRequestPending);
		Find<UTextBlock>(Tree,TEXT("TheaterContinueLabel"))->SetText(bSelectionConfirmationPending
			?LOCTEXT("CornerContinueLock","继续锁定"):FFMCodexPlayerUIPresentationText::CornerLockCandidates(bAttack));
		Show(*Tree.FindWidget(TEXT("TheaterSelectionReturnBounds")),bActor && bSelectionConfirmationPending && !bRequestPending);
		Find<UButton>(Tree,TEXT("TheaterSelectionReturn"))->SetIsEnabled(bActor && !bRequestPending);
	}
}
void RefreshReel(UWidgetTree& Tree, const FFMCodexUMGRollReelViewModel& Reel)
{
	for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense"),TEXT("Theater"),TEXT("TheaterPairA"),TEXT("TheaterPairB"),TEXT("TheaterDraw"),TEXT("TheaterEvent")})
	{
		auto* W=Cast<UFMCodexRollReelWidget>(Tree.FindWidget(Named(Prefix,TEXT("Reel"))));
		auto* Host=Tree.FindWidget(Named(Prefix,FString(Prefix)==TEXT("Theater") ? TEXT("Roll") : TEXT("ReelHost")));
		const bool bActiveHost=Host && Host->GetVisibility()!=ESlateVisibility::Collapsed;
		if (W && Host) W->RefreshFromPresentation(bActiveHost ? Reel : FFMCodexUMGRollReelViewModel());
		if (FString(Prefix)==TEXT("TheaterAttack") || FString(Prefix)==TEXT("TheaterDefense"))
		{
			// Fade only already-disclosed content, using the existing shared clock.
			// No value calculation, text substitution, layout transform or added hold.
			const float Progress=bActiveHost && Reel.bStaticResult ? Reel.FormulaFinalRevealProgress : -1.f;
			const float Opacity=Progress>=0.f ? FMath::Lerp(.84f,1.f,FMath::SmoothStep(0.f,1.f,Progress)) : 1.f;
			Tree.FindWidget(Named(Prefix,TEXT("ResultColumn")))->SetRenderOpacity(Opacity);
		}
	}
}
void ClearTakerInspection(UWidgetTree& Tree, FTakerInspection& Inspection)
{
 Inspection.bActive=false; Inspection.HoveredId=NAME_None; Inspection.CombinationEligibility.Reset(); ++Inspection.Generation;
 auto* Full=Find<UFMCodexPlayerCardWidget>(Tree,TEXT("TheaterTakerFullCard"));
 Full->RefreshFromPresentation({}); Full->SetVisibility(ESlateVisibility::Collapsed);
 Show(*Tree.FindWidget(TEXT("TheaterTakerInspector")),false);
}
void SetActive(UWidgetTree& Tree, FMotion& Motion, bool bActive)
{
	if (Motion.bActive!=bActive)
	{
		Motion.bActive=bActive; Motion.Elapsed=0.f; Motion.FieldTransitionFrom=Motion.FieldProgress;
		Motion.bAwaitingFirstFrame=bActive;
	}
	// Only presentation input is suppressed. Typed action eligibility and all
	// authority/reveal clocks remain with the original Screen/adapter.
	Tree.FindWidget(TEXT("MatchShellViewportFit"))->SetVisibility(bActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::SelfHitTestInvisible);
	if (!bActive) RefreshReel(Tree,{});
	Tick(Tree,Motion,0.f);
}
void RefreshHover(UWidgetTree& Tree)
{
	// Pointer affordance remains live after the entry animation has stopped ticking.
	// It has no clock, match state or reveal dependency.
	for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense")})
	{
		const auto* Hover=Find<UBorder>(Tree,Named(Prefix,TEXT("BaseHover")));
		const bool bHover=Hover->IsHovered() || Hover->GetCachedGeometry().IsUnderLocation(FSlateApplication::Get().GetCursorPos());
		Find<UBorder>(Tree,Named(Prefix,TEXT("BaseUnderline")))->SetBrushColor(Alpha(bHover?White:Quiet,bHover?.95f:.65f));
	}
}
void Tick(UWidgetTree& Tree, FMotion& Motion, float DeltaSeconds)
{
	// A tactical click can occur late in the Slate frame. Its preceding frame
	// delta is not elapsed theater time and must not skip the opening transition.
	if (DeltaSeconds>0.f && Motion.bAwaitingFirstFrame) { Motion.bAwaitingFirstFrame=false; DeltaSeconds=0.f; }
	Motion.Elapsed=FMath::Min(1.f,Motion.Elapsed+FMath::Max(0.f,DeltaSeconds));
	auto Ease=[&](float Delay,float Duration) { const float T=FMath::Clamp((Motion.Elapsed-Delay)/Duration,0.f,1.f); return 1.f-FMath::Pow(1.f-T,3.f); };
	Motion.FieldProgress=FMath::Lerp(Motion.FieldTransitionFrom,Motion.bActive ? 1.f : 0.f,Ease(0,Motion.bActive?.40f:.30f));
	const float Clutter=Motion.bActive ? 1-Ease(0,.18f) : Ease(.08f,.22f);
	for (const auto Name:{TEXT("BroadcastMatchHeaderRegion"),TEXT("LocalPlayerCardRackRegion"),TEXT("OpponentCardRackRegion"),TEXT("ContextActionDockRegion")})
		if (auto* W=Tree.FindWidget(Name))
		{
			W->SetRenderOpacity(Clutter);
			W->SetVisibility(Clutter>0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
		}
	// Keep the real turf; fade the board-space markings that conflict with the
	// forward stadium atmosphere. Restore the same canvas on exit. No duplicate pitch asset,
	// render target, gameplay view or hidden-card snapshot is created.
	if (auto* Pitch=Cast<UFMCodexPitchWidget>(Tree.FindWidget(TEXT("DedicatedFootballPitchWidget"))))
	{
		if (Pitch->WidgetTree)
		{
			auto* Turf=Pitch->WidgetTree->FindWidget(TEXT("PitchBackgroundAssetHook"));
			if (auto* Field=Pitch->WidgetTree->FindWidget(TEXT("TwoLanePitchCanvas")))
				Field->SetRenderOpacity(1.f-Motion.FieldProgress);
			if (Motion.bActive && !Motion.bHasFieldGeometry && Turf)
			{
				const auto& PG=Pitch->GetCachedGeometry(); const auto& TG=Turf->GetCachedGeometry(); const auto& RG=Tree.RootWidget->GetCachedGeometry();
				const FVector2D Origin=PG.AbsoluteToLocal(TG.GetAbsolutePosition());
				const FVector2D Size=PG.AbsoluteToLocal(TG.LocalToAbsolute(TG.GetLocalSize()))-Origin;
				if (Size.X>1 && Size.Y>1 && RG.GetLocalSize().X>1)
				{
					const FVector2D ViewOrigin=PG.AbsoluteToLocal(RG.GetAbsolutePosition());
					const FVector2D ViewSize=PG.AbsoluteToLocal(RG.LocalToAbsolute(RG.GetLocalSize()))-ViewOrigin;
					Motion.FieldScale=ViewSize/Size;
					const FVector2D Pivot=PG.GetLocalSize()*Pitch->GetRenderTransformPivot();
					Motion.FieldTranslation=ViewOrigin-Origin*Motion.FieldScale-Pivot*(FVector2D(1)-Motion.FieldScale);
					Motion.bHasFieldGeometry=true;
				}
			}
			if (auto* Hud=Pitch->WidgetTree->FindWidget(TEXT("PitchSemanticHUD")))
			{
				Hud->SetRenderOpacity(Clutter); Hud->SetVisibility(Clutter>0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
			}
			for (const auto& Slot:Pitch->GetRenderedSlotWidgets()) if (Slot)
			{
				Slot->SetRenderOpacity(Clutter); Slot->SetVisibility(Clutter>0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
			}
		}
		Pitch->SetRenderScale(FMath::Lerp(FVector2D(1),Motion.FieldScale,Motion.FieldProgress));
		Pitch->SetRenderTranslation(Motion.FieldTranslation*Motion.FieldProgress);
	}
	// Sibling resolution surfaces remain refreshed by their normal owners; only
	// their paint is suppressed here, so OFF never has to reconstruct their state.
	if (auto* Layers=Tree.FindWidget(TEXT("BoardResolutionOverlays")))
		Layers->SetVisibility(Motion.bActive ? ESlateVisibility::Hidden : ESlateVisibility::SelfHitTestInvisible);
	if (!Motion.bActive && Motion.Elapsed>=.30f) Motion.bHasFieldGeometry=false;
	if (auto* Root=Tree.FindWidget(TEXT("ResolutionTheater")))
	{
		Show(*Root,Motion.bActive || Motion.FieldProgress>0.f);
		Tree.FindWidget(TEXT("TheaterLighting"))->SetRenderOpacity(Motion.FieldProgress);
		Show(*Tree.FindWidget(TEXT("TheaterContent")),Motion.bActive);
		if (!Motion.bActive) return;
		Tree.FindWidget(TEXT("TheaterTop"))->SetRenderOpacity(Ease(.08f,.18f));
		Tree.FindWidget(TEXT("TheaterContext"))->SetRenderOpacity(Ease(.08f,.18f));
		int32 Index=0;
		for (const auto& Pair : {TPair<const TCHAR*,float>(TEXT("TheaterAttackPanel"),-38.f),{TEXT("TheaterDefensePanel"),38.f}})
		{
			const float A=Ease(Index++==0?.20f:.42f,.18f);
			auto* W=Tree.FindWidget(Pair.Key); W->SetRenderTranslation(FVector2D(Pair.Value*(1-A),0)); W->SetRenderOpacity(A);
			W->SetVisibility(A>0.f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
		}
		const float VS=Ease(.64f,.10f);
		Tree.FindWidget(TEXT("TheaterVS"))->SetRenderOpacity(VS);
		Tree.FindWidget(TEXT("TheaterVS"))->SetVisibility(VS>0.f ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		Tree.FindWidget(TEXT("TheaterVS"))->SetRenderScale(FVector2D(.9f+.1f*VS));
		auto* Bottom=Tree.FindWidget(TEXT("TheaterBottom"));
		Bottom->SetRenderOpacity(Ease(.76f,.10f));
		Bottom->SetVisibility(Motion.Elapsed>=.76f ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Hidden);
	}
}
}
#undef LOCTEXT_NAMESPACE
