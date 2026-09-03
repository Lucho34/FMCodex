#include "FMCodexFullTimePanelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"

#define LOCTEXT_NAMESPACE "FMCodexFullTime"

namespace FMCodexFullTimePanel
{
	const FLinearColor Ink(0.88f, 0.92f, 0.96f);
	const FLinearColor Silver(0.34f, 0.43f, 0.52f);

	int32 PaintFootball(const FGeometry& G, FSlateWindowElementList& Out, int32 Layer,
		const FVector2D Center, float Radius)
	{
		const FSlateRoundedBoxBrush Circle(FLinearColor::White);
		const auto Disc = [&](float R, FLinearColor Color)
		{
			FSlateDrawElement::MakeBox(Out, Layer++, G.ToPaintGeometry(FVector2D(R * 2),
				FSlateLayoutTransform(Center - FVector2D(R))), &Circle, ESlateDrawEffect::None, Color);
		};
		const FLinearColor Dark(0.015f, 0.027f, 0.041f);
		Disc(Radius, Silver);
		Disc(Radius * 0.93f, Dark);
		Disc(Radius * 0.76f, FLinearColor(0.53f, 0.60f, 0.64f));
		TArray<FVector2D> Points;
		for (int32 I = 0; I <= 5; ++I)
		{
			const float Angle = -PI / 2 + (I % 5) * 2 * PI / 5;
			Points.Add(Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius * 0.27f);
		}
		FSlateDrawElement::MakeLines(Out, Layer++, G.ToPaintGeometry(), Points,
			ESlateDrawEffect::None, Dark, true, Radius * 0.35f);
		for (int32 I = 0; I < 5; ++I)
		{
			const FVector2D Direction = (Points[I] - Center).GetSafeNormal();
			const FVector2D Edge = Center + Direction * Radius * 0.66f;
			FSlateDrawElement::MakeLines(Out, Layer, G.ToPaintGeometry(), {Points[I], Edge},
				ESlateDrawEffect::None, Dark, true, Radius * 0.055f);
			FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(FVector2D(Radius * 0.27f),
				FSlateLayoutTransform(Edge - FVector2D(Radius * 0.135f))), &Circle, ESlateDrawEffect::None, Dark);
		}
		return Layer;
	}

	class SArtwork final : public SLeafWidget
	{
	public:
		SLATE_BEGIN_ARGS(SArtwork) {} SLATE_ARGUMENT(bool, FootballOnly) SLATE_END_ARGS()
		bool bFootballOnly = false;
		void Construct(const FArguments& Args) { bFootballOnly = Args._FootballOnly; SetCanTick(false); }
		virtual FVector2D ComputeDesiredSize(float) const override { return bFootballOnly ? FVector2D(20) : FVector2D(1000, 540); }
		virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&,
			FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
		{
			if (bFootballOnly)
			{
				return PaintFootball(G, Out, Layer, G.GetLocalSize() * 0.5f,
					FMath::Min(G.GetLocalSize().X, G.GetLocalSize().Y) * 0.49f);
			}
			const auto Box = [&](FVector2D P, FVector2D Size, const FSlateBrush& Brush, FLinearColor Color, int32 Z)
			{
				FSlateDrawElement::MakeBox(Out, Z, G.ToPaintGeometry(Size, FSlateLayoutTransform(P)), &Brush,
					ESlateDrawEffect::None, Color);
			};
			const auto Line = [&](TArray<FVector2D> Points, FLinearColor Color, float Width = 1.0f)
			{
				FSlateDrawElement::MakeLines(Out, Layer + 3, G.ToPaintGeometry(), Points,
					ESlateDrawEffect::None, Color, true, Width);
			};
			const FSlateRoundedBoxBrush Frame(FLinearColor::White, 18.0f, Silver, 1.4f);
			const FSlateRoundedBoxBrush Shadow(FLinearColor::White, 22.0f);
			Box({5, 9}, {990, 490}, Shadow, FLinearColor(0, 0, 0, 0.35f), Layer);
			Box({1, 1}, {998, 488}, Frame, FLinearColor(0.012f, 0.022f, 0.034f), Layer + 1);
			TArray<FSlateGradientStop> Stops;
			Stops.Emplace(FVector2D(0, 0), FLinearColor(0.029f, 0.049f, 0.082f, 0.8f));
			Stops.Emplace(FVector2D(0, 120), FLinearColor(0.015f, 0.027f, 0.040f, 0.2f));
			Stops.Emplace(FVector2D(0, 320), FLinearColor(0.009f, 0.019f, 0.026f, 0.3f));
			Stops.Emplace(FVector2D(0, 460), FLinearColor(0.023f, 0.040f, 0.072f, 0.8f));
			FSlateDrawElement::MakeGradient(Out, Layer + 2,
				G.ToPaintGeometry(FVector2D(968, 460), FSlateLayoutTransform(FVector2D(16, 14))),
				Stops, Orient_Horizontal, ESlateDrawEffect::None);
			// Fine brushed-metal grain, decorative only; no RNG and no texture asset.
			for (int32 I = 0; I < 95; ++I)
			{
				const float X = 26.0f + I * 10.0f;
				Line({{X, 18}, {FMath::Max(20.0f, X - 95.0f), 112}}, FLinearColor(0.25f, 0.34f, 0.45f, 0.018f));
			}
			Line({{70, 130}, {930, 130}}, FLinearColor(0.35f, 0.44f, 0.54f, 0.6f));
			Line({{55, 280}, {435, 280}}, FLinearColor(0.35f, 0.44f, 0.54f, 0.4f));
			Line({{565, 280}, {945, 280}}, FLinearColor(0.35f, 0.44f, 0.54f, 0.4f));
			Line({{394, 150}, {394, 250}}, FLinearColor(0.3f, 0.4f, 0.5f, 0.16f));
			Line({{606, 150}, {606, 250}}, FLinearColor(0.3f, 0.4f, 0.5f, 0.16f));
			Line({{500, 310}, {500, 416}}, FLinearColor(0.3f, 0.4f, 0.5f, 0.18f));
			Line({{205, 81}, {350, 81}}, Silver);
			Line({{650, 81}, {795, 81}}, Silver);
			const FSlateRoundedBoxBrush Circle(FLinearColor::White);
			Box({359, 78}, {6, 6}, Circle, Ink, Layer + 4);
			Box({635, 78}, {6, 6}, Circle, Ink, Layer + 4);
			Line({{2, 408}, {500, 466}, {998, 408}}, FLinearColor(0.5f, 0.59f, 0.68f), 1.2f);
			Line({{2, 412}, {500, 470}, {998, 412}}, FLinearColor(0.2f, 0.3f, 0.42f), 1.0f);
			return PaintFootball(G, Out, Layer + 4, {500, 468}, 25);
		}
	};

	void Place(UCanvasPanel& Canvas, UWidget* Widget, const FVector2D Position, const FVector2D Size)
	{
		auto* Slot = Canvas.AddChildToCanvas(Widget);
		Slot->SetPosition(Position);
		Slot->SetSize(Size);
	}

	UTextBlock* Text(UWidgetTree& Tree, const FName Name, const FText& Label, int32 Size, bool bBold = false)
	{
		auto* T = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		T->SetText(Label);
		T->SetFont(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
		T->SetColorAndOpacity(Ink);
		T->SetJustification(ETextJustify::Center);
		T->SetAutoWrapText(false);
		T->SetShadowOffset(FVector2D(0, 2));
		T->SetShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.5f));
		T->SetVisibility(ESlateVisibility::HitTestInvisible);
		return T;
	}

	// Bounded local name fitting: single line, shrink before ellipsis, never enlarge.
	// Fixed canvas/row widths keep the score and the other column stationary.
	void FitName(UTextBlock& Label, const FText& Value, float Width, int32 BaseSize, int32 MinSize)
	{
		auto Font = Label.GetFont();
		Font.Size = BaseSize;
		if (FSlateApplication::IsInitialized())
		{
			const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			while (Font.Size > MinSize && Measure->Measure(Value, Font).X > Width) --Font.Size;
			if (Measure->Measure(Value, Font).X > Width)
			{
				FString Fitted = Value.ToString();
				while (Fitted.Len() > 1
					&& Measure->Measure(FText::FromString(Fitted + TEXT("…")), Font).X > Width)
				{
					Fitted.LeftChopInline(1);
				}
				Label.SetText(FText::FromString(Fitted + TEXT("…")));
			}
			else Label.SetText(Value);
		}
		else Label.SetText(Value);
		Label.SetFont(Font);
		Label.SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		Label.SetClipping(EWidgetClipping::ClipToBounds);
	}
}

TSharedRef<SWidget> UFMCodexFullTimeArtworkWidget::RebuildWidget()
{
	return SNew(FMCodexFullTimePanel::SArtwork).FootballOnly(bFootballOnly);
}

TSharedRef<SWidget> UFMCodexFullTimePanelWidget::RebuildWidget()
{
	if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexFullTimePanelWidget::BuildWidgetTree()
{
	using namespace FMCodexFullTimePanel;
	if (WidgetTree->RootWidget) return;
	SetIsFocusable(true);
	auto* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("FullTimeModalScrim"));
	Dim->SetBrushColor(FLinearColor(0.002f, 0.008f, 0.012f, 0.74f));
	Dim->SetPadding(FMargin(0));
	WidgetTree->RootWidget = Dim;
	auto* SafeArea = WidgetTree->ConstructWidget<UCanvasPanel>();
	Dim->AddChild(SafeArea);
	auto* Scale = WidgetTree->ConstructWidget<UScaleBox>(UScaleBox::StaticClass(), TEXT("FullTimeCardFit"));
	Scale->SetStretch(EStretch::ScaleToFit);
	Scale->SetStretchDirection(EStretchDirection::Both);
	auto* SafeSlot = SafeArea->AddChildToCanvas(Scale);
	SafeSlot->SetAnchors(FAnchors(0.12f, 0.13f, 0.88f, 0.87f));
	SafeSlot->SetOffsets(FMargin(0));
	auto* Bounds = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("FullTimeDesignBounds"));
	Bounds->SetWidthOverride(1000);
	Bounds->SetHeightOverride(540);
	Scale->AddChild(Bounds);
	auto* Layers = WidgetTree->ConstructWidget<UOverlay>();
	Bounds->AddChild(Layers);
	auto* Art = WidgetTree->ConstructWidget<UFMCodexFullTimeArtworkWidget>();
	Art->SetVisibility(ESlateVisibility::HitTestInvisible);
	Layers->AddChildToOverlay(Art);
	auto* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	auto* ContentSlot = Layers->AddChildToOverlay(Canvas);
	ContentSlot->SetHorizontalAlignment(HAlign_Fill);
	ContentSlot->SetVerticalAlignment(VAlign_Fill);
	Place(*Canvas, Text(*WidgetTree, TEXT("FullTimeEnglish"), LOCTEXT("English", "F U L L   T I M E"), 13), {300, 24}, {400, 26});
	Place(*Canvas, Text(*WidgetTree, TEXT("FullTimeTitle"), LOCTEXT("Title", "全场结束"), 35), {375, 53}, {250, 60});
	PlayerAName = Text(*WidgetTree, TEXT("FullTimePlayerAName"), FText(), 26);
	PlayerBName = Text(*WidgetTree, TEXT("FullTimePlayerBName"), FText(), 26);
	Place(*Canvas, PlayerAName, {150, 167}, {232, 44});
	Place(*Canvas, PlayerBName, {618, 167}, {232, 44});
	TeamAName = Text(*WidgetTree, TEXT("FullTimeTeamAName"), FText(), 17);
	TeamBName = Text(*WidgetTree, TEXT("FullTimeTeamBName"), FText(), 17);
	TeamAName->SetColorAndOpacity(FLinearColor(0.48f, 0.57f, 0.65f));
	TeamBName->SetColorAndOpacity(FLinearColor(0.48f, 0.57f, 0.65f));
	Place(*Canvas, TeamAName, {150, 216}, {232, 30});
	Place(*Canvas, TeamBName, {618, 216}, {232, 30});
	TeamAScore = Text(*WidgetTree, TEXT("FullTimeTeamAScore"), FText(), 64, true);
	TeamBScore = Text(*WidgetTree, TEXT("FullTimeTeamBScore"), FText(), 64, true);
	Place(*Canvas, TeamAScore, {396, 149}, {90, 100});
	Place(*Canvas, Text(*WidgetTree, TEXT("FullTimeScoreDash"), FText::FromString(TEXT("-")), 38), {482, 167}, {36, 74});
	Place(*Canvas, TeamBScore, {514, 149}, {90, 100});
	const auto Badge = [&](FName Name, float X, UTextBlock*& Mark)
	{
		auto* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		Frame->SetPadding(FMargin(3));
		Frame->SetHorizontalAlignment(HAlign_Center);
		Frame->SetVerticalAlignment(VAlign_Center);
		Mark = Text(*WidgetTree, NAME_None, FText(), 24, true);
		Frame->AddChild(Mark);
		Place(*Canvas, Frame, {X, 159}, {78, 88});
		return Frame;
	};
	UTextBlock* MarkA; UTextBlock* MarkB;
	TeamABadgeBorder = Badge(TEXT("FullTimeTeamABadge"), 48, MarkA);
	TeamBBadgeBorder = Badge(TEXT("FullTimeTeamBBadge"), 874, MarkB);
	TeamABadge = MarkA; TeamBBadge = MarkB;
	TeamAGoals = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("FullTimeAGoalList"));
	TeamBGoals = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("FullTimeBGoalList"));
	auto* Heading = Text(*WidgetTree, TEXT("FullTimeScorersHeading"), LOCTEXT("Scorers", "进球球员"), 15);
	Heading->SetColorAndOpacity(FLinearColor(0.54f, 0.63f, 0.71f));
	Place(*Canvas, Heading, {440, 267}, {120, 28});
	Place(*Canvas, TeamAGoals, {88, 316}, {368, 100});
	Place(*Canvas, TeamBGoals, {544, 316}, {368, 100});
	ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("FullTimeConfirmButton"));
	FButtonStyle ButtonStyle;
	ButtonStyle.SetNormal(FSlateRoundedBoxBrush(FLinearColor(0.016f, 0.035f, 0.038f, 0.45f), 6.0f));
	ButtonStyle.SetHovered(FSlateRoundedBoxBrush(FLinearColor(0.052f, 0.10f, 0.12f), 8.0f, Silver, 1.0f));
	ButtonStyle.SetPressed(FSlateRoundedBoxBrush(FLinearColor(0.012f, 0.028f, 0.033f), 8.0f));
	ButtonStyle.SetDisabled(ButtonStyle.Normal);
	ConfirmButton->SetStyle(ButtonStyle);
	ConfirmText = Text(*WidgetTree, TEXT("FullTimeConfirmLabel"), FText(), 16);
	ConfirmButton->AddChild(ConfirmText);
	ConfirmButton->OnClicked.AddDynamic(this, &UFMCodexFullTimePanelWidget::Acknowledge);
	Place(*Canvas, ConfirmButton, {350, 507}, {300, 32});
}

void UFMCodexFullTimePanelWidget::RefreshFromPresentation(const FFMCodexFullTimePresentation& InPresentation)
{
	const bool bOpening = InPresentation.bVisible && !Presentation.bVisible;
	if (!InPresentation.bVisible || bOpening) bAcknowledged = false;
	Presentation = InPresentation;
	RefreshVisuals();
	if (bOpening && GetOwningPlayer()) SetKeyboardFocus();
}

void UFMCodexFullTimePanelWidget::RefreshVisuals()
{
	using namespace FMCodexFullTimePanel;
	SetVisibility(Presentation.bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (!TeamAName) return;
	const auto Team = [&](const FFMCodexFullTimeTeamPresentation& P, UTextBlock& Player, UTextBlock& Name,
		UTextBlock& Score, UTextBlock& Mark, UBorder& Badge, UScrollBox& Goals)
	{
		FitName(Player, P.PlayerDisplayName.IsEmpty() ? LOCTEXT("UnnamedPlayer", "玩家") : P.PlayerDisplayName, 232, 26, 18);
		FitName(Name, P.Name, 232, 17, 13);
		FitName(Score, P.Score, 90, 64, 40);
		Mark.SetText(P.BadgeMark);
		Badge.SetBrush(FSlateRoundedBoxBrush(P.Color, FVector4(24, 24, 48, 48), Silver, 2.0f));
		Goals.ClearChildren();
		// Empty rows share the name start edge, row height and fill alignment with goals.
		const int32 Count = FMath::Max(1, P.Goals.Num());
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const bool bEmpty = P.Goals.IsEmpty();
			auto* RowBounds = WidgetTree->ConstructWidget<USizeBox>();
			RowBounds->SetHeightOverride(32);
			auto* Line = WidgetTree->ConstructWidget<UHorizontalBox>();
			RowBounds->AddChild(Line);
			auto* BallBounds = WidgetTree->ConstructWidget<USizeBox>();
			BallBounds->SetWidthOverride(20); BallBounds->SetHeightOverride(20);
			auto* Ball = WidgetTree->ConstructWidget<UFMCodexFullTimeArtworkWidget>();
			Ball->bFootballOnly = true;
			Ball->SetVisibility(bEmpty ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
			BallBounds->AddChild(Ball);
			auto* BallSlot = Line->AddChildToHorizontalBox(BallBounds);
			BallSlot->SetVerticalAlignment(VAlign_Center);
			BallSlot->SetPadding(FMargin(0, 0, 10, 0));
			auto* Row = Text(*WidgetTree, NAME_None, FText(), 19);
			Row->SetJustification(ETextJustify::Left);
			FitName(*Row, bEmpty ? LOCTEXT("EmptyGoals", "—") : P.Goals[Index], 328, 19, 14);
			auto* TextSlot = Line->AddChildToHorizontalBox(Row);
			TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TextSlot->SetVerticalAlignment(VAlign_Center);
			TextSlot->SetHorizontalAlignment(HAlign_Fill);
			auto* GoalSlot = CastChecked<UScrollBoxSlot>(Goals.AddChild(RowBounds));
			GoalSlot->SetHorizontalAlignment(HAlign_Fill);
			GoalSlot->SetVerticalAlignment(VAlign_Top);
		}
	};
	Team(Presentation.PlayerA, *PlayerAName, *TeamAName, *TeamAScore, *TeamABadge, *TeamABadgeBorder, *TeamAGoals);
	Team(Presentation.PlayerB, *PlayerBName, *TeamBName, *TeamBScore, *TeamBBadge, *TeamBBadgeBorder, *TeamBGoals);
	ConfirmText->SetText(bAcknowledged ? LOCTEXT("Acknowledged", "已确认 · 比赛结束")
		: LOCTEXT("Confirm", "»   确认比赛结果   «"));
	ConfirmButton->SetIsEnabled(!bAcknowledged);
}

void UFMCodexFullTimePanelWidget::Acknowledge()
{
	if (!Presentation.bVisible || bAcknowledged) return;
	bAcknowledged = true;
	RefreshVisuals(); // Stable post-match summary; no menu destination exists yet.
}

FReply UFMCodexFullTimePanelWidget::NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Presentation.bVisible)
	{
		if (Event.GetKey() == EKeys::Enter || Event.GetKey() == EKeys::SpaceBar
			|| Event.GetKey() == EKeys::Gamepad_FaceButton_Bottom) Acknowledge();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(Geometry, Event);
}

#undef LOCTEXT_NAMESPACE
