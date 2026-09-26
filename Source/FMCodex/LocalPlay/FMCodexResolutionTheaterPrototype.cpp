#include "FMCodexResolutionTheaterPrototype.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexOutcomePresentation.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexRollReelWidget.h"
#include "FMCodexPitchWidget.h"
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
	TEXT("Development fallback override; default ON. 0 restores FormulaV2/legacy. Shipping always uses the Cross theater."), ECVF_Default);
TAutoConsoleVariable<int32> LowCrossMode(TEXT("fm.UI.ResolutionStageV2.LowCross"), 1,
	TEXT("Development Low Cross fallback; default ON. 0 restores its legacy presentation. Shipping always uses Low Cross Theater."), ECVF_Default);
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
	// A stable 68x76 allocation hosts ?, Theater Roll v2, then the disclosed digit.
	auto* Unknown=Tree.ConstructWidget<UOverlay>(UOverlay::StaticClass(),Named(Prefix,TEXT("UnknownSlot")));
	for (const auto Suffix:{TEXT("Pending"),TEXT("RollValue")})
	{
		auto* Digit=Text(Tree,Named(Prefix,Suffix),40,Quiet); Digit->SetJustification(ETextJustify::Center);
		if (FString(Suffix)==TEXT("Pending")) Digit->SetText(FText::FromString(TEXT("?")));
		Unknown->AddChildToOverlay(Digit); AlignEquationText(*Digit,EquationBaseline-10);
	}
	auto* Reel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),Named(Prefix,TEXT("Reel")));
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::TheaterInline);
	auto* ReelHost=Tree.ConstructWidget<UScaleBox>(UScaleBox::StaticClass(),Named(Prefix,TEXT("ReelHost")));
	ReelHost->SetStretch(EStretch::ScaleToFit); ReelHost->AddChild(Reel);
	auto* ReelSlot=Unknown->AddChildToOverlay(ReelHost); ReelSlot->SetHorizontalAlignment(HAlign_Fill); ReelSlot->SetVerticalAlignment(VAlign_Fill);
	ReelSlot->SetPadding(FMargin(0));
	Expression->AddChildToHorizontalBox(Bounds(Tree,Unknown,68,76))->SetVerticalAlignment(VAlign_Bottom);
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
	auto* PanelBounds=Bounds(Tree,Panel,600.f); PanelBounds->Rename(*Named(Prefix,TEXT("PanelBounds")).ToString(),&Tree);
	Middle.AddChildToHorizontalBox(PanelBounds)->SetVerticalAlignment(VAlign_Center);
}
void RefreshSide(UWidgetTree& Tree, const FString& Prefix, const FFMCodexUMGInlineFormulaRowViewModel& Row,
	bool bFormula, bool bActive, bool bReveal, int32 Sequence, bool bWinner)
{
	SetText(Tree,Named(Prefix,TEXT("Side")),Row.SideLabel);
	auto* PlayerBounds=Find<USizeBox>(Tree,Named(Prefix,TEXT("SilhouetteBounds")));
	PlayerBounds->SetHeightOverride(bFormula?178.f:128.f); PlayerBounds->SetWidthOverride(138.f);
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
	Show(*Tree.FindWidget(Named(Prefix,TEXT("Value"))),bFormula);
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
	const bool bReel=bFormula && bReveal && Roll && Roll->RollSequenceIndex==Sequence;
	const bool bResolved=bFormula && Roll && Roll->bResolved;
	Show(*Tree.FindWidget(Named(Prefix,TEXT("ReelHost"))),bReel);
	Show(*Tree.FindWidget(Named(Prefix,TEXT("Pending"))),bFormula && !bReel && !bResolved);
	Show(*Tree.FindWidget(Named(Prefix,TEXT("RollValue"))),bResolved && !bReel);
	SetText(Tree,Named(Prefix,TEXT("RollValue")),bResolved ? FString::FromInt(Roll->RawD6) : FString());
	auto* RollValue=Find<UTextBlock>(Tree,Named(Prefix,TEXT("RollValue")));
	RollValue->SetColorAndOpacity(Gold);
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
		if (Term.Kind==K::FixedModifier)
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
bool IsFormulaContest(FName ContestId)
{
	return ContestId==TEXT("Cross.High") || (ContestId==TEXT("Cross.Low") && IsLowCrossEnabled());
}
bool WantsTheater(const FFMCodexUMGMatchScreenViewModel& Screen, const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed)
{
	if (!IsEnabled() || Screen.FullTime.bVisible || Screen.Resolution.bRejected) return false;
	// Only the explicit Low fallback exits at visible route disclosure.
	// Test the established visible disclosure, never the hidden future route alone.
	if (Displayed.ContestId==TEXT("Cross.Route") && !Displayed.RouteResultLabel.IsEmpty()
		&& Screen.InlineFormula.ContestId==TEXT("Cross.Low") && !IsLowCrossEnabled()) return false;
	return (Screen.Interaction.Category==C::SelectBranchIntent && Screen.InlineFormula.ContestId==TEXT("Cross.Setup"))
		|| (Displayed.bVisible && (Displayed.ContestId==TEXT("Cross.Route") || IsFormulaContest(Displayed.ContestId)));
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
	auto* Pad=Tree.ConstructWidget<UBorder>(); Pad->SetBrushColor(FLinearColor::Transparent); Pad->SetPadding(FMargin(0,32,0,76)); Pad->AddChild(Body);
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
	auto* CompositionSlot=Center->AddChildToOverlay(Composition); CompositionSlot->SetVerticalAlignment(VAlign_Center); CompositionSlot->SetHorizontalAlignment(HAlign_Center);
	auto* Middle=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterDuel"));
	BuildSide(Tree,*Middle,TEXT("TheaterAttack"),Athletes);
	auto* VS=Tree.ConstructWidget<UBorder>(UBorder::StaticClass(),TEXT("TheaterVS"));
	VS->SetBrush(FSlateRoundedBoxBrush(Alpha(Color(9,28,41),.9f),22.f,Alpha(Quiet,.48f),1.f)); VS->SetBrushColor(FLinearColor::White); VS->SetPadding(FMargin(8,8));
	auto* VSText=Text(Tree,TEXT("TheaterVSLabel"),28,Quiet); VSText->SetText(FText::FromString(TEXT("VS"))); VSText->SetJustification(ETextJustify::Center); VS->AddChild(VSText);
	auto* VSBounds=Bounds(Tree,VS,64.f); auto* VSSlot=Middle->AddChildToHorizontalBox(VSBounds); VSSlot->SetVerticalAlignment(VAlign_Center); VSSlot->SetPadding(FMargin(10,0));
	BuildSide(Tree,*Middle,TEXT("TheaterDefense"),Athletes);
	Composition->AddChildToVerticalBox(Middle)->SetHorizontalAlignment(HAlign_Center);
	auto* Bottom=Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),TEXT("TheaterBottom"));
	auto* BottomSlot=Composition->AddChildToVerticalBox(Bounds(Tree,Bottom,1040)); BottomSlot->SetHorizontalAlignment(HAlign_Center); BottomSlot->SetPadding(FMargin(0,16,0,0));
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
	Bottom->AddChildToVerticalBox(Status)->SetPadding(FMargin(0,4,0,0));
	// Reel and action share an allocation, so the sides never jump as the roll resolves.
	auto* ActionLane=Tree.ConstructWidget<UOverlay>();
	Bottom->AddChildToVerticalBox(Bounds(Tree,ActionLane,0,72))->SetPadding(FMargin(0,4,0,0));
	Primary=Button(Tree,TEXT("TheaterContinue"),TEXT("TheaterContinueLabel"),FText::GetEmpty());
	auto* PrimaryBounds=Bounds(Tree,Primary,320,62); PrimaryBounds->Rename(TEXT("TheaterPrimaryBounds"),&Tree);
	auto* PrimarySlot=ActionLane->AddChildToOverlay(PrimaryBounds); PrimarySlot->SetHorizontalAlignment(HAlign_Center); PrimarySlot->SetVerticalAlignment(VAlign_Center);
	auto* Choices=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterChoices"));
	High=Button(Tree,TEXT("TheaterHigh"),TEXT("TheaterHighLabel"),LOCTEXT("High","高球传中"));
	Low=Button(Tree,TEXT("TheaterLow"),TEXT("TheaterLowLabel"),LOCTEXT("Low","低球传中"));
	Choices->AddChildToHorizontalBox(Bounds(Tree,High,240,60))->SetPadding(FMargin(0,0,18,0));
	Choices->AddChildToHorizontalBox(Bounds(Tree,Low,240,60));
	auto* ChoiceSlot=ActionLane->AddChildToOverlay(Choices); ChoiceSlot->SetHorizontalAlignment(HAlign_Center); ChoiceSlot->SetVerticalAlignment(VAlign_Center);
	auto* ReelLine=Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),TEXT("TheaterRoll"));
	auto* Reel=Tree.ConstructWidget<UFMCodexRollReelWidget>(UFMCodexRollReelWidget::StaticClass(),TEXT("TheaterReel"));
	Reel->SetVisualVariant(EFMCodexRollVisualVariant::CompactBox);
	ReelLine->AddChildToHorizontalBox(Bounds(Tree,Reel,84,72));
	auto* ReelSlot=ActionLane->AddChildToOverlay(ReelLine); ReelSlot->SetHorizontalAlignment(HAlign_Center); ReelSlot->SetVerticalAlignment(VAlign_Center);
	return Root;
}
void Refresh(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& P, const FFMCodexUMGMatchHeaderViewModel& H, bool bRequestPending)
{
	const bool bFormula=P.bVisible && IsFormulaContest(P.ContestId) && P.bShowFormulaRows;
	// Future safe facts may already exist while the visible route is still gated.
	const FName DisclosedContest=bFormula ? P.ContestId
		: (P.ContestId==TEXT("Cross.Route") && !P.RouteResultLabel.IsEmpty() ? Screen.InlineFormula.ContestId : NAME_None);
	const bool bFinal=bFormula && FMCodexOutcomePresentation::IsFinalReady(P.bNarrativeAvailable,P.bDiceRevealVisible);
	// Setup deliberately has no visible Formula; its public participant rows
	// live on the safe Screen projection, not the empty displayed Formula.
	const auto& ParticipantSurface=Screen.InlineFormula.ContestId==TEXT("Cross.Setup")
		? Screen.InlineFormula : P;
	const auto& Attack=ParticipantSurface.AttackRow;
	const auto& Defense=ParticipantSurface.DefenseRow;
	// Narrative success is projected from authoritative winner facts. Do not
	// compare totals: rapid suppression can legitimately defeat the larger total.
	RefreshSide(Tree,TEXT("TheaterAttack"),Attack,bFormula,P.bAttackRowActive,P.bDiceRevealVisible && P.RollReel.bVisible,P.ActiveRollSequenceIndex,bFinal && P.bNarrativeAttackSuccess);
	RefreshSide(Tree,TEXT("TheaterDefense"),Defense,bFormula,P.bDefenseRowActive,P.bDiceRevealVisible && P.RollReel.bVisible,P.ActiveRollSequenceIndex,bFinal && !P.bNarrativeAttackSuccess);
	Find<UTextBlock>(Tree,TEXT("TheaterTitle"))->SetText(DisclosedContest==TEXT("Cross.High") ? LOCTEXT("HighCross","高球传中")
		: DisclosedContest==TEXT("Cross.Low") ? LOCTEXT("LowCross","低球传中") : LOCTEXT("Cross","传中"));
	auto* Title=Find<UTextBlock>(Tree,TEXT("TheaterTitle"));
	auto TitleFont=Title->GetFont(); TitleFont.Size=bFinal?26:52; Title->SetFont(TitleFont);
	Show(*Title,true);
	auto* Outcome=Find<URichTextBlock>(Tree,TEXT("TheaterOutcome"));
	Outcome->SetText(bFinal ? FText::FromString(FMCodexOutcomePresentation::PrimaryMarkup(P.ContestLabel,P.OutcomeText)) : FText::GetEmpty()); Show(*Outcome,bFinal);
	Show(*Tree.FindWidget(TEXT("TheaterOutcomeDivider")),bFinal);
	Find<UTextBlock>(Tree,TEXT("TheaterSubtitle"))->SetText(bFinal ? FText::GetEmpty()
		: bFormula ? LOCTEXT("Contest","进球判定") : Screen.Interaction.Category==C::SelectBranchIntent
		? LOCTEXT("Setup","选择传中方式") : LOCTEXT("Route","路线判定"));
	// H is BuildDisplayedHeader output, never the un-gated authoritative score.
	const bool bLeftA=H.LeftPlayerSide==EInitialTurnOrderPlayer::PlayerA;
	SetText(Tree,TEXT("TheaterContext"),FString::Printf(TEXT("%s  %s  –  %s  %s"),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.LeftPlayerLabel).ToString(),
		*(bLeftA ? H.PlayerAScoreLabel : H.PlayerBScoreLabel),*(bLeftA ? H.PlayerBScoreLabel : H.PlayerAScoreLabel),
		*FFMCodexPlayerUIPresentationText::MatchScreenLabel(H.RightPlayerLabel).ToString()));
	const bool bAction=P.bVisible && P.PrimaryAction.bVisible && P.PrimaryAction.Action.bAvailable && !bRequestPending;
	Show(*Tree.FindWidget(TEXT("TheaterPrimaryBounds")),bAction);
	Find<UButton>(Tree,TEXT("TheaterContinue"))->SetIsEnabled(bAction);
	const C Category=P.PrimaryAction.Action.Category;
	const bool bRoll=Category==C::RollCrossRoute || Category==C::RollCrossAttack || Category==C::RollCrossDefense;
	Find<UTextBlock>(Tree,TEXT("TheaterContinueLabel"))->SetText(Category==C::RollCrossAttack ? LOCTEXT("AttackRoll","进攻方掷点")
		: Category==C::RollCrossDefense ? LOCTEXT("DefenseRoll","防守方掷点")
		: Category==C::RollCrossRoute ? LOCTEXT("RouteRoll","判定路线")
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
		Show(*Choice,bChoice); Choice->SetIsEnabled(bChoice); bChoices|=bChoice;
	}
	Show(*Tree.FindWidget(TEXT("TheaterChoices")),bChoices);
	// Compact information/reason bar remains separate from the independent CTA.
	FText Detail;
	if (bFinal) Detail=FText::FromString(P.ResolutionReasonLabel);
	else if (Screen.Interaction.Category==C::SelectBranchIntent) Detail=LOCTEXT("ChooseCross","请选择传中方式");
	else if (bFormula)
	{
		const bool bAttackTurn=P.bAttackRowActive;
		if (P.bDiceRevealVisible) Detail=bAttackTurn ? LOCTEXT("AttackRolling","进攻方掷点中") : LOCTEXT("DefenseRolling","防守方掷点中");
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
	FString MainReason=Detail.ToString(), SecondaryReason;
	if (bFinal) Detail.ToString().Split(TEXT("\n"),&MainReason,&SecondaryReason);
	Find<UTextBlock>(Tree,TEXT("TheaterDetail"))->SetText(FText::FromString(MainReason));
	SetText(Tree,TEXT("TheaterReasonSecondary"),SecondaryReason);
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
		CastChecked<UVerticalBoxSlot>(Tree.FindWidget(Named(Prefix,TEXT("Value")))->Slot)->SetPadding(FMargin(0,bFinal?4:14,0,0));
	}
	// The CTA owns the operation; retain only actor/wait ownership outside it.
	// Never consume StatusLabel here: it can alias an Outcome during ResultHold.
	FString Status=P.bDiceRevealVisible ? P.DiceOwnerLabel : FString();
	if (Screen.bMirrorActionWaitPrompt && !P.bDiceRevealVisible)
		Status=Screen.ActionWaitActorText.ToString()+TEXT("  ")+Screen.ActionWaitActionText.ToString();
	else if (bAction || bChoices)
		Status=FFMCodexPlayerUIPresentationText::MatchScreenLabel(Screen.Interaction.ExpectedActorLabel).ToString();
	if (bRequestPending) Status=LOCTEXT("PendingRequest","操作已提交，等待确认").ToString();
	SetText(Tree,TEXT("TheaterStatus"),Status);
	Show(*Tree.FindWidget(TEXT("TheaterRoll")),P.bDiceRevealVisible && !bFormula);
	RefreshReel(Tree,P.RollReel);
}
void RefreshReel(UWidgetTree& Tree, const FFMCodexUMGRollReelViewModel& Reel)
{
	for (const auto Prefix:{TEXT("TheaterAttack"),TEXT("TheaterDefense"),TEXT("Theater")})
	{
		auto* W=Cast<UFMCodexRollReelWidget>(Tree.FindWidget(Named(Prefix,TEXT("Reel"))));
		auto* Host=Tree.FindWidget(Named(Prefix,FString(Prefix)==TEXT("Theater") ? TEXT("Roll") : TEXT("ReelHost")));
		const bool bActiveHost=Host && Host->GetVisibility()!=ESlateVisibility::Collapsed;
		if (W && Host) W->RefreshFromPresentation(bActiveHost ? Reel : FFMCodexUMGRollReelViewModel());
		if (FString(Prefix)!=TEXT("Theater"))
		{
			// Fade only already-disclosed content, using the existing shared clock.
			// No value calculation, text substitution, layout transform or added hold.
			const float Progress=bActiveHost && Reel.bStaticResult ? Reel.FormulaFinalRevealProgress : -1.f;
			const float Opacity=Progress>=0.f ? FMath::Lerp(.84f,1.f,FMath::SmoothStep(0.f,1.f,Progress)) : 1.f;
			Tree.FindWidget(Named(Prefix,TEXT("ResultColumn")))->SetRenderOpacity(Opacity);
		}
	}
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
