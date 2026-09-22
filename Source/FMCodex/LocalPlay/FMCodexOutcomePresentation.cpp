#include "FMCodexOutcomePresentation.h"

#include "FMCodexMatchFlowPanel.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexPlayerUIStyle.h"
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace FMCodexOutcomePresentation
{
	bool OwnsInlineSurface(const FFMCodexUMGInlineFormulaSurfaceViewModel& P)
	{
		// Ownership begins with the reveal, not after the reel has disappeared.
		// Opposed no-row exceptional terminals and type selection remain deferred.
		return P.bVisible && !P.bShowFormulaRows && (P.bNarrativeAvailable || P.bDiceRevealVisible)
			&& P.ContestId != FName(TEXT("SetPiece.Opposed"))
			&& P.ContestId != FName(TEXT("SetPiece.Type"))
			&& P.ContestId != FName(TEXT("Cross.Route"))
			&& P.ContestId != FName(TEXT("Corner.Route"));
	}

	FLinearColor AccentColor(EFMCodexOutcomeAccent Accent)
	{
		// Convert the display-space candidate tokens to UE linear colors.
		if (Accent == EFMCodexOutcomeAccent::Goal) return FLinearColor::FromSRGBColor(FColor(102, 217, 183));
		if (Accent == EFMCodexOutcomeAccent::NoGoal) return FLinearColor::FromSRGBColor(FColor(232, 163, 109));
		return FLinearColor::FromSRGBColor(FColor(240, 245, 250));
	}

	FString PrimaryMarkup(const FString& CanonicalText, const FFMCodexOutcomeText& Segments)
	{
		auto Escape = [](FString Text)
		{
			Text.ReplaceInline(TEXT("&"), TEXT("&amp;"));
			Text.ReplaceInline(TEXT("<"), TEXT("&lt;"));
			Text.ReplaceInline(TEXT(">"), TEXT("&gt;"));
			Text.ReplaceInline(TEXT("\""), TEXT("&quot;"));
			return Text;
		};
		// Equality is an integrity guard, never a classification or range search.
		// Absent/mismatched segments retain the complete neutral canonical text.
		if (Segments.Keyword.IsEmpty() || Segments.Accent == EFMCodexOutcomeAccent::Neutral
			|| Segments.ToText().ToString() != CanonicalText) return Escape(CanonicalText);
		return Escape(Segments.Prefix.ToString())
			+ (Segments.Accent == EFMCodexOutcomeAccent::Goal ? TEXT("<Goal>") : TEXT("<NoGoal>"))
			+ Escape(Segments.Keyword.ToString()) + TEXT("</>") + Escape(Segments.Suffix.ToString());
	}

	FString JoinContext(const TArray<FString>& Labels)
	{
		TArray<FString> Present;
		for (const auto& Label : Labels) if (!Label.IsEmpty()) Present.Add(Label);
		return FString::Join(Present, TEXT(" · "));
	}

	void ApplyFrameStyle(UFMCodexMatchFlowPanel& Frame, USizeBox& Bounds, bool bEmbedded)
	{
		Frame.SetFlowStyleEnabled(!bEmbedded);
		Frame.SetPadding(bEmbedded ? FMargin(0) : FMargin(24, 22));
		if (bEmbedded) Frame.SetBrushColor(FLinearColor::Transparent);
		Bounds.SetMinDesiredWidth(760.f);
	}

	bool IsFinalReady(bool bNarrativeAvailable, bool bDiceRevealVisible)
	{
		// A local composition gate only. The existing reel/disclosure clock is untouched.
		return bNarrativeAvailable && !bDiceRevealVisible;
	}

	URichTextBlock* BuildPrimary(UWidgetTree& Tree, FName Name)
	{
		const auto& Style = FFMCodexPlayerUIStyle::Get();
		auto* Primary = Tree.ConstructWidget<URichTextBlock>(URichTextBlock::StaticClass(), Name);
		auto* FontSource = NewObject<UTextBlock>(&Tree);
		Style.ApplyFlowText(*FontSource, 28);
		FTextBlockStyle PrimaryStyle;
		PrimaryStyle.SetFont(FontSource->GetFont()).SetColorAndOpacity(FontSource->GetColorAndOpacity());
		auto* Styles = NewObject<UDataTable>(Primary);
		Styles->RowStruct = FRichTextStyleRow::StaticStruct();
		for (const auto Accent : {EFMCodexOutcomeAccent::Goal, EFMCodexOutcomeAccent::NoGoal})
		{
			FRichTextStyleRow Row; Row.TextStyle = PrimaryStyle;
			Row.TextStyle.SetColorAndOpacity(AccentColor(Accent));
			Styles->AddRow(Accent == EFMCodexOutcomeAccent::Goal ? FName(TEXT("Goal")) : FName(TEXT("NoGoal")), Row);
		}
		Primary->SetTextStyleSet(Styles);
		Primary->SetDefaultTextStyle(PrimaryStyle);
		Primary->SetJustification(ETextJustify::Center);
		Primary->SetAutoWrapText(true);
		return Primary;
	}

	void RefreshIntermediate(UWidgetTree& Tree, bool bVisible, const FString& Context, const FString& Detail)
	{
		Tree.FindWidget(TEXT("OutcomeIntermediate"))->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		auto* Title = CastChecked<UTextBlock>(Tree.FindWidget(TEXT("OutcomeIntermediateTitle")));
		Title->SetText(bVisible ? (Context.IsEmpty() ? NSLOCTEXT("FMCodexOutcome", "ResolutionContext", "战术结算") : FText::FromString(Context)) : FText::GetEmpty());
		auto* DetailText = CastChecked<UTextBlock>(Tree.FindWidget(TEXT("OutcomeIntermediateDetail")));
		DetailText->SetText(bVisible ? FText::FromString(Detail) : FText::GetEmpty());
		DetailText->SetVisibility(bVisible && !Detail.IsEmpty() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	void Build(UWidgetTree& Tree, UVerticalBox& Parent)
	{
		const auto& Style = FFMCodexPlayerUIStyle::Get();
		auto* Intermediate = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OutcomeIntermediate"));
		Intermediate->SetVisibility(ESlateVisibility::Collapsed);
		Parent.AddChildToVerticalBox(Intermediate)->SetHorizontalAlignment(HAlign_Fill);
		for (const auto Name : {TEXT("OutcomeIntermediateTitle"), TEXT("OutcomeIntermediateState"), TEXT("OutcomeIntermediateDetail")})
		{
			auto* Item = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
			Style.ApplyFlowText(*Item, FName(Name) == TEXT("OutcomeIntermediateTitle") ? 24 : 14, FName(Name) != TEXT("OutcomeIntermediateTitle"));
			Item->SetJustification(ETextJustify::Center);
			Item->SetAutoWrapText(true);
			Intermediate->AddChildToVerticalBox(Item)->SetPadding(FMargin(8, 4));
		}
		CastChecked<UTextBlock>(Tree.FindWidget(TEXT("OutcomeIntermediateState")))->SetText(NSLOCTEXT("FMCodexOutcome", "Resolving", "正在结算 · 等待掷点完成"));
		auto* Body = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OutcomeHierarchy"));
		Body->SetVisibility(ESlateVisibility::Collapsed);
		Parent.AddChildToVerticalBox(Body);
		auto Text = [&](const TCHAR* Name, int32 Size, bool bSecondary, FMargin Padding)
		{
			auto* Item = Tree.ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
			Style.ApplyFlowText(*Item, Size, bSecondary);
			Item->SetJustification(ETextJustify::Center);
			Item->SetAutoWrapText(true);
			Body->AddChildToVerticalBox(Item)->SetPadding(Padding);
			return Item;
		};
		auto* Reading = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("OutcomeReadingColumn"));
		auto* ReadingSlot = Body->AddChildToVerticalBox(Reading);
		ReadingSlot->SetHorizontalAlignment(HAlign_Fill);
		ReadingSlot->SetPadding(FMargin(8, 0));
		auto* Primary = BuildPrimary(Tree, TEXT("OutcomePrimary"));
		auto* PrimarySlot = Reading->AddChildToVerticalBox(Primary);
		PrimarySlot->SetHorizontalAlignment(HAlign_Fill);
		PrimarySlot->SetPadding(FMargin(0, 10, 0, 8));
		auto* Context = Text(TEXT("OutcomeContext"), 15, true, FMargin(0));
		Body->RemoveChild(Context);
		auto* ContextSlot = Reading->AddChildToVerticalBox(Context);
		ContextSlot->SetHorizontalAlignment(HAlign_Fill);
		ContextSlot->SetPadding(FMargin(0, 0, 0, 10));
		auto* Detail = Text(TEXT("OutcomeDetail"), 13, true, FMargin(0));
		Body->RemoveChild(Detail);
		auto* Chip = Tree.ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OutcomeDetailRegion"));
		// A read-only strip, with natural text supplied by the structured projection.
		Chip->SetBrush(FSlateRoundedBoxBrush(FLinearColor(.006f, .025f, .041f, 1), 3.f,
			FLinearColor(.035f, .08f, .11f, 1), 1.f));
		Chip->SetPadding(FMargin(16, 6));
		Chip->SetVisibility(ESlateVisibility::HitTestInvisible);
		Chip->AddChild(Detail);
		auto* DetailSlot = Body->AddChildToVerticalBox(Chip);
		// Allocate a real reading width before auto-wrap measures the detail. A
		// content-sized centered chip can retain a narrow first-frame wrap width.
		DetailSlot->SetHorizontalAlignment(HAlign_Fill);
		DetailSlot->SetPadding(FMargin(72, 4, 72, 8));
		Text(TEXT("OutcomeSupporting"), 14, true, FMargin(8, 2, 8, 8));
		Body->AddChildToVerticalBox(Style.MakeFlowSeparator(Tree, TEXT("OutcomeFooterRule")))
			->SetPadding(FMargin(0, 12, 0, 2));
	}

	void Refresh(UWidgetTree& Tree, bool bVisible, const FString& Primary,
		const FString& Context, const FString& Detail, const FString& Supporting,
		bool bShowFooter, const FFMCodexOutcomeText& Segments)
	{
		Tree.FindWidget(TEXT("OutcomeHierarchy"))->SetVisibility(bVisible
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		auto SetText = [&](const TCHAR* Name, const FString& Value)
		{
			auto* Item = CastChecked<UTextBlock>(Tree.FindWidget(Name));
			Item->SetText(bVisible ? FText::FromString(Value) : FText::GetEmpty());
			Item->SetVisibility(bVisible && !Value.IsEmpty()
				? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		};
		auto* PrimaryText = CastChecked<URichTextBlock>(Tree.FindWidget(TEXT("OutcomePrimary")));
		PrimaryText->SetText(bVisible ? FText::FromString(PrimaryMarkup(Primary, Segments)) : FText::GetEmpty());
		PrimaryText->SetVisibility(bVisible && !Primary.IsEmpty() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		SetText(TEXT("OutcomeContext"), Context);
		SetText(TEXT("OutcomeDetail"), Detail);
		SetText(TEXT("OutcomeSupporting"), Supporting);
		Tree.FindWidget(TEXT("OutcomeDetailRegion"))->SetVisibility(bVisible && !Detail.IsEmpty()
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Tree.FindWidget(TEXT("OutcomeFooterRule"))->SetVisibility(bVisible && bShowFooter
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	void ApplyActionStyle(UButton& Button)
	{
		const auto& Style = FFMCodexPlayerUIStyle::Get();
		CastChecked<UFMCodexMatchFlowButton>(&Button)->SetFlowStyleEnabled(true);
		Button.SetStyle(Style.MakeFlowButtonStyle());
		auto* Label = CastChecked<UTextBlock>(Button.GetChildAt(0));
		Style.ApplyFlowText(*Label, 20);
		// Short action labels remain one semantic unit across viewport/native DPI.
		Label->SetAutoWrapText(false);
		auto* Bounds = CastChecked<USizeBox>(Button.GetParent());
		Bounds->SetWidthOverride(244.f);
		Bounds->SetMinDesiredHeight(48.f);
		Bounds->ClearHeightOverride();
	}
}
