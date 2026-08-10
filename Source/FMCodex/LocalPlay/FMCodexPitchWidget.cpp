#include "FMCodexPitchWidget.h"

#include "FMCodexPitchSlotWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"

namespace FMCodexPitchWidget
{
	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Text)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(Text));
		Result->SetAutoWrapText(true);
		return Result;
	}
}

UFMCodexPitchWidget::UFMCodexPitchWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PitchSlotWidgetClass = UFMCodexPitchSlotWidget::StaticClass();
}

void UFMCodexPitchWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexPitchWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexPitchWidget::RefreshFromPitchPresentation(
	const TArray<FFMCodexUMGPitchRegionViewModel>& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const TArray<FFMCodexUMGPitchRegionViewModel>&
UFMCodexPitchWidget::GetPresentation() const
{
	return Presentation;
}

const TArray<TObjectPtr<UFMCodexPitchSlotWidget>>&
UFMCodexPitchWidget::GetRenderedSlotWidgets() const
{
	return RenderedSlotWidgets;
}

void UFMCodexPitchWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* FieldSize = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("FootballFieldAspectShell"));
	FieldSize->SetMinDesiredWidth(840.0f);
	FieldSize->SetMaxDesiredWidth(1120.0f);
	WidgetTree->RootWidget = FieldSize;

	UBorder* FieldBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("FootballFieldBackground"));
	FieldBorder->SetPadding(FMargin(14.0f));
	FieldBorder->SetBrushColor(FLinearColor(0.025f, 0.16f, 0.075f, 0.98f));
	FieldSize->AddChild(FieldBorder);

	FieldBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("StablePhysicalHalfLayout"));
	FieldBorder->AddChild(FieldBody);
}

void UFMCodexPitchWidget::RefreshVisuals()
{
	using namespace FMCodexPitchWidget;
	if (FieldBody == nullptr)
	{
		return;
	}
	FieldBody->ClearChildren();
	RenderedSlotWidgets.Reset();
	FieldBody->AddChildToVerticalBox(MakeText(
		*WidgetTree, TEXT("FootballFieldHeading"), TEXT("FOOTBALL FIELD")));

	for (int32 RegionIndex = 0; RegionIndex < Presentation.Num(); ++RegionIndex)
	{
		if (RegionIndex > 0)
		{
			UBorder* CenterField = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), TEXT("CenterFieldVisualSeparator"));
			CenterField->SetPadding(FMargin(8.0f));
			CenterField->SetBrushColor(
				FLinearColor(0.78f, 0.82f, 0.78f, 0.92f));
			CenterField->AddChild(MakeText(
				*WidgetTree, TEXT("CenterFieldLabel"),
				TEXT("CENTER FIELD  |  VISUAL SEPARATOR")));
			FieldBody->AddChildToVerticalBox(CenterField);
		}

		const FFMCodexUMGPitchRegionViewModel& Region =
			Presentation[RegionIndex];
		const FName HalfName = RegionIndex == 0
			? FName(TEXT("PlayerBPhysicalHalf"))
			: RegionIndex == 1
				? FName(TEXT("PlayerAPhysicalHalf"))
				: FName(*FString::Printf(TEXT("PhysicalHalf%d"), RegionIndex));
		UBorder* HalfBorder = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), HalfName);
		HalfBorder->SetPadding(FMargin(10.0f));
		HalfBorder->SetBrushColor(Region.bCurrentAttackingSide
			? FLinearColor(0.34f, 0.31f, 0.08f, 0.94f)
			: FLinearColor(0.045f, 0.22f, 0.10f, 0.90f));
		UVerticalBox* HalfBody = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*FString::Printf(
				TEXT("PhysicalHalfBody%d"), RegionIndex)));
		HalfBorder->AddChild(HalfBody);
		HalfBody->AddChildToVerticalBox(MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("PhysicalHalfHeading%d"), RegionIndex)),
			FString::Printf(TEXT("%s%s\n%s"),
				Region.bCurrentAttackingSide ? TEXT("ATTACKING SIDE  |  ") : TEXT(""),
				*Region.RegionLabel,
				*Region.ZoneContextLabel)));

		UUniformGridPanel* SlotGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(), FName(*FString::Printf(
					TEXT("CanonicalSlotGrid%d"), RegionIndex)));
		SlotGrid->SetSlotPadding(FMargin(4.0f));
		for (int32 SlotIndex = 0; SlotIndex < Region.Slots.Num(); ++SlotIndex)
		{
			UClass* SlotClass = PitchSlotWidgetClass != nullptr
				? PitchSlotWidgetClass.Get()
				: UFMCodexPitchSlotWidget::StaticClass();
			UFMCodexPitchSlotWidget* SlotWidget =
				WidgetTree->ConstructWidget<UFMCodexPitchSlotWidget>(
					SlotClass, FName(*FString::Printf(
						TEXT("CanonicalPitchSlot%d_%d"),
						RegionIndex, SlotIndex)));
			SlotWidget->RefreshFromPitchSlotPresentation(
				Region.Slots[SlotIndex]);
			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(
				SlotWidget, SlotIndex / 5, SlotIndex % 5);
			GridSlot->SetHorizontalAlignment(HAlign_Fill);
			GridSlot->SetVerticalAlignment(VAlign_Fill);
			RenderedSlotWidgets.Add(SlotWidget);
		}
		HalfBody->AddChildToVerticalBox(SlotGrid);
		FieldBody->AddChildToVerticalBox(HalfBorder);
	}
}
