#include "FMCodexDiceResultWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

UFMCodexDiceResultWidget::UFMCodexDiceResultWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexDiceResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexDiceResultWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexDiceResultWidget::RefreshFromPresentation(
	const FFMCodexUMGDiceResultViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGDiceResultViewModel&
UFMCodexDiceResultWidget::GetPresentation() const
{
	return Presentation;
}

int32 UFMCodexDiceResultWidget::GetDisplayedRawD6() const
{
	return Presentation.RawD6;
}

void UFMCodexDiceResultWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("DiceResultBounds"));
	Bounds->SetMinDesiredWidth(112.0f);
	Bounds->SetMaxDesiredWidth(164.0f);
	Bounds->SetMinDesiredHeight(132.0f);
	WidgetTree->RootWidget = Bounds;

	UBorder* DieFrame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("DiceResultFrame"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*DieFrame,
		EFMCodexPlayerUIColorRole::PanelRaised, Style.GetSectionPadding());
	Bounds->AddChild(DieFrame);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("DiceResultHierarchy"));
	DieFrame->AddChild(Body);
	ContextText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("DiceContextLabel"));
	ContextText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*ContextText, EFMCodexPlayerUITextRole::Kicker);
	Body->AddChildToVerticalBox(ContextText);
	UBorder* DiceFace = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("DiceFaceAssetHook"));
	Style.ApplyBorder(*DiceFace,
		EFMCodexPlayerUIColorRole::NeutralAccent, Style.GetCompactPadding());
	RawValueText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("DiceRawD6Value"));
	RawValueText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*RawValueText, EFMCodexPlayerUITextRole::DiceValue);
	DiceFace->AddChild(RawValueText);
	Body->AddChildToVerticalBox(DiceFace);
	PurposeText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("DicePurposeLabel"));
	PurposeText->SetJustification(ETextJustify::Center);
	PurposeText->SetAutoWrapText(true);
	Style.ApplyText(*PurposeText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(PurposeText);
}

void UFMCodexDiceResultWidget::RefreshVisuals()
{
	if (ContextText == nullptr)
	{
		return;
	}
	ContextText->SetText(FText::FromString(
		Presentation.ContextLabel.IsEmpty()
			? TEXT("ACCEPTED DICE") : Presentation.ContextLabel));
	PurposeText->SetText(FText::FromString(
		Presentation.PurposeLabel.IsEmpty()
			? TEXT("Authoritative roll") : Presentation.PurposeLabel));
	RawValueText->SetText(FText::FromString(FString::Printf(
		TEXT("[ %d ]"), Presentation.RawD6)));
}
