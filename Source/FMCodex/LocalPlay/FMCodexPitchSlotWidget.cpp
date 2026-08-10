#include "FMCodexPitchSlotWidget.h"

#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

UFMCodexPitchSlotWidget::UFMCodexPitchSlotWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCardWidgetClass = UFMCodexPlayerCardWidget::StaticClass();
}

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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*SlotBorder,
		EFMCodexPlayerUIColorRole::EmptyPitchSlot,
		Style.GetSectionPadding());
	SlotSize->AddChild(SlotBorder);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CanonicalPitchSlotBody"));
	SlotBorder->AddChild(Body);

	SlotLabelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CanonicalSlotLabel"));
	SlotLabelText->SetAutoWrapText(true);
	Style.ApplyText(*SlotLabelText, EFMCodexPlayerUITextRole::SectionHeading);
	Body->AddChildToVerticalBox(SlotLabelText);

	ContextText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("RelativeZoneContext"));
	ContextText->SetAutoWrapText(true);
	Style.ApplyText(*ContextText, EFMCodexPlayerUITextRole::Secondary);
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
		SlotBorder->SetBrushColor(FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::OccupiedPitchSlot));
		UClass* ResolvedCardClass = PlayerCardWidgetClass != nullptr
			? PlayerCardWidgetClass.Get()
			: UFMCodexPlayerCardWidget::StaticClass();
		CardWidget = WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
			ResolvedCardClass, TEXT("OccupiedPitchCard"));
		CardWidget->RefreshFromPresentation(
			Presentation.Card,
			EFMCodexPlayerCardPresentationMode::PitchCompact);
		ContentBody->AddChildToVerticalBox(CardWidget);
	}
	else
	{
		SlotBorder->SetBrushColor(FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::EmptyPitchSlot));
		EmptyStateText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("EmptySpatialLocation"));
		EmptyStateText->SetText(FText::FromString(
			TEXT("VACANT POSITION\nPitch display only")));
		EmptyStateText->SetAutoWrapText(true);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*EmptyStateText, EFMCodexPlayerUITextRole::Secondary);
		ContentBody->AddChildToVerticalBox(EmptyStateText);
	}
}
