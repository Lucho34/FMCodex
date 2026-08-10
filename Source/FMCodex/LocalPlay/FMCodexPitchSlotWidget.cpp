#include "FMCodexPitchSlotWidget.h"

#include "FMCodexPlayerCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UFMCodexPitchSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexPitchSlotWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexPitchSlotWidget::RefreshFromPitchSlotPresentation(
	const FFMCodexUMGPitchSlotViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGPitchSlotViewModel&
UFMCodexPitchSlotWidget::GetPresentation() const
{
	return Presentation;
}

UFMCodexPlayerCardWidget* UFMCodexPitchSlotWidget::GetCardWidget() const
{
	return CardWidget;
}

bool UFMCodexPitchSlotWidget::IsShowingOccupiedCard() const
{
	return Presentation.bOccupied && CardWidget != nullptr;
}

void UFMCodexPitchSlotWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* SlotSize = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("CanonicalPitchSlotSize"));
	SlotSize->SetMinDesiredWidth(160.0f);
	SlotSize->SetMinDesiredHeight(138.0f);
	WidgetTree->RootWidget = SlotSize;

	SlotBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("CanonicalPitchSlotBorder"));
	SlotBorder->SetPadding(FMargin(6.0f));
	SlotSize->AddChild(SlotBorder);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CanonicalPitchSlotBody"));
	SlotBorder->AddChild(Body);

	SlotLabelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CanonicalSlotLabel"));
	SlotLabelText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(SlotLabelText);

	ContextText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("RelativeZoneContext"));
	ContextText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(ContextText);

	ContentBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SlotCardPresentationHost"));
	Body->AddChildToVerticalBox(ContentBody);
}

void UFMCodexPitchSlotWidget::RefreshVisuals()
{
	if (SlotBorder == nullptr)
	{
		return;
	}
	SlotLabelText->SetText(FText::FromString(
		Presentation.SlotLabel.IsEmpty()
			? TEXT("Canonical slot") : Presentation.SlotLabel));
	ContextText->SetText(FText::FromString(FString::Printf(
		TEXT("%s | A: %s | B: %s"),
		*Presentation.PhysicalHalfLabel,
		*Presentation.PlayerARelativeZoneLabel,
		*Presentation.PlayerBRelativeZoneLabel)));
	ContentBody->ClearChildren();
	CardWidget = nullptr;
	EmptyStateText = nullptr;
	if (Presentation.bOccupied)
	{
		SlotBorder->SetBrushColor(FLinearColor(0.03f, 0.12f, 0.16f, 0.94f));
		CardWidget = WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
			UFMCodexPlayerCardWidget::StaticClass(), TEXT("OccupiedPitchCard"));
		CardWidget->RefreshFromPresentation(Presentation.Card);
		ContentBody->AddChildToVerticalBox(CardWidget);
	}
	else
	{
		SlotBorder->SetBrushColor(FLinearColor(0.08f, 0.20f, 0.11f, 0.78f));
		EmptyStateText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("EmptySpatialLocation"));
		EmptyStateText->SetText(FText::FromString(
			TEXT("VACANT POSITION\nPitch display only")));
		EmptyStateText->SetAutoWrapText(true);
		ContentBody->AddChildToVerticalBox(EmptyStateText);
	}
}
