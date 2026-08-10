#include "FMCodexPitchWidget.h"

#include "FMCodexPlayerUIStyle.h"
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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	FieldSize->SetMinDesiredWidth(Style.GetPanelMinWidth());
	FieldSize->SetMaxDesiredWidth(Style.GetPanelMaxWidth());
	WidgetTree->RootWidget = FieldSize;

	UBorder* FieldBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("FootballFieldBackground"));
	Style.ApplyBorder(*FieldBorder,
		EFMCodexPlayerUIColorRole::PanelBackground, Style.GetOuterPadding());
	FieldSize->AddChild(FieldBorder);
	UBorder* BackgroundAssetHook = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("PitchBackgroundAssetHook"));
	Style.ApplyBorder(*BackgroundAssetHook,
		EFMCodexPlayerUIColorRole::PitchBackground, Style.GetPanelPadding());
	FieldBorder->AddChild(BackgroundAssetHook);

	FieldBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("StablePhysicalHalfLayout"));
	BackgroundAssetHook->AddChild(FieldBody);
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
	UTextBlock* FieldHeading = MakeText(
		*WidgetTree, TEXT("FootballFieldHeading"), TEXT("MATCH PITCH"));
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*FieldHeading, EFMCodexPlayerUITextRole::ActionTitle);
	FieldHeading->SetJustification(ETextJustify::Center);
	FieldBody->AddChildToVerticalBox(FieldHeading);

	for (int32 RegionIndex = 0; RegionIndex < Presentation.Num(); ++RegionIndex)
	{
		if (RegionIndex > 0)
		{
			UBorder* CenterField = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), TEXT("CenterFieldVisualSeparator"));
			const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
			Style.ApplyBorder(*CenterField,
				EFMCodexPlayerUIColorRole::PitchCenterLine,
				Style.GetCompactPadding());
			UTextBlock* CenterLabel = MakeText(
				*WidgetTree, TEXT("CenterFieldLabel"),
				TEXT("CENTER LINE  •  MIDFIELD"));
			Style.ApplyText(
				*CenterLabel, EFMCodexPlayerUITextRole::SectionHeading);
			CenterLabel->SetColorAndOpacity(FSlateColor(
				FLinearColor(0.02f, 0.06f, 0.035f, 1.0f)));
			CenterLabel->SetJustification(ETextJustify::Center);
			CenterField->AddChild(CenterLabel);
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
		const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
		Style.ApplyBorder(*HalfBorder,
			Region.bCurrentAttackingSide
				? EFMCodexPlayerUIColorRole::PitchAttackingHalf
				: EFMCodexPlayerUIColorRole::PitchHalf,
			Style.GetOuterPadding());
		UVerticalBox* HalfBody = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*FString::Printf(
				TEXT("PhysicalHalfBody%d"), RegionIndex)));
		HalfBorder->AddChild(HalfBody);
		UTextBlock* HalfHeading = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("PhysicalHalfHeading%d"), RegionIndex)),
			FString::Printf(TEXT("%s%s\n%s"),
				Region.bCurrentAttackingSide ? TEXT("ATTACKING SIDE  |  ") : TEXT(""),
				*Region.RegionLabel,
				*Region.ZoneContextLabel));
		Style.ApplyText(*HalfHeading, Region.bCurrentAttackingSide
			? EFMCodexPlayerUITextRole::Status
			: EFMCodexPlayerUITextRole::SectionHeading);
		HalfBody->AddChildToVerticalBox(HalfHeading);

		UUniformGridPanel* SlotGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(), FName(*FString::Printf(
					TEXT("CanonicalSlotGrid%d"), RegionIndex)));
		SlotGrid->SetSlotPadding(Style.GetCompactPadding());
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
