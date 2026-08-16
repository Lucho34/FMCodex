#include "FMCodexPitchWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPitchSlotWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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
		Result->SetAutoWrapText(false);
		return Result;
	}

	void AddSemanticLabel(
		UWidgetTree& Tree,
		UCanvasPanel& Canvas,
		const int32 RegionIndex,
		const float CenterFraction,
		const FText& Label)
	{
		UTextBlock* Result = MakeText(Tree, FName(*FString::Printf(
			TEXT("PitchSemanticLabel%d"), RegionIndex)), FString());
		Result->SetText(Label);
		Result->SetJustification(ETextJustify::Center);
		Result->SetClipping(EWidgetClipping::ClipToBounds);
		Result->SetColorAndOpacity(FSlateColor(FLinearColor(
			0.88f, 0.93f, 0.86f, 0.88f)));
		FSlateFontInfo Font = Result->GetFont();
		Font.Size = 14;
		Result->SetFont(Font);
		Result->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* Slot = Canvas.AddChildToCanvas(Result))
		{
			Slot->SetAnchors(FAnchors(CenterFraction, 0.01f));
			Slot->SetAlignment(FVector2D(0.5f, 0.0f));
			Slot->SetOffsets(FMargin(-42.0f, 0.0f, 84.0f, 24.0f));
		}
	}

	UBorder* AddPitchLine(
		UWidgetTree& Tree,
		UCanvasPanel& Canvas,
		const FName Name,
		const FAnchors Anchors,
		const FMargin Offsets,
		const float Opacity = 0.50f)
	{
		UBorder* Line = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		FFMCodexPlayerUIStyle::Get().ApplyBorder(*Line,
			EFMCodexPlayerUIColorRole::PitchCenterLine, FMargin(0.0f));
		Line->SetRenderOpacity(Opacity);
		Line->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* Slot = Canvas.AddChildToCanvas(Line))
		{
			Slot->SetAnchors(Anchors);
			Slot->SetOffsets(Offsets);
		}
		return Line;
	}

	void AddPitchOutline(
		UWidgetTree& Tree,
		UCanvasPanel& Canvas,
		const FString& Prefix,
		const float Left,
		const float Top,
		const float Right,
		const float Bottom,
		const float Opacity)
	{
		AddPitchLine(Tree, Canvas, FName(*(Prefix + TEXT("Top"))),
			FAnchors(Left, Top, Right, Top), FMargin(0.0f, -1.0f, 0.0f, 2.0f), Opacity);
		AddPitchLine(Tree, Canvas, FName(*(Prefix + TEXT("Bottom"))),
			FAnchors(Left, Bottom, Right, Bottom), FMargin(0.0f, -1.0f, 0.0f, 2.0f), Opacity);
		AddPitchLine(Tree, Canvas, FName(*(Prefix + TEXT("Left"))),
			FAnchors(Left, Top, Left, Bottom), FMargin(-1.0f, 0.0f, 2.0f, 0.0f), Opacity);
		AddPitchLine(Tree, Canvas, FName(*(Prefix + TEXT("Right"))),
			FAnchors(Right, Top, Right, Bottom), FMargin(-1.0f, 0.0f, 2.0f, 0.0f), Opacity);
	}

	void AddMidfieldReference(
		UWidgetTree& Tree,
		UCanvasPanel& Canvas,
		const FString& Prefix,
		const bool bLocalFacing)
	{
		const float LandmarkX = bLocalFacing ? 0.08f : 0.92f;
		AddPitchLine(Tree, Canvas, FName(*(Prefix + TEXT("Line"))),
			FAnchors(LandmarkX, 0.08f, LandmarkX, 0.92f),
			FMargin(-1.0f, 0.0f, 2.0f, 0.0f), 0.34f);
		UTextBlock* Arc = MakeText(
			Tree, FName(*(Prefix + TEXT("Arc"))), TEXT("\u25CB"));
		Arc->SetJustification(ETextJustify::Center);
		FSlateFontInfo Font = Arc->GetFont();
		Font.Size = 150;
		Arc->SetFont(Font);
		Arc->SetColorAndOpacity(FSlateColor(FLinearColor(
			0.72f, 0.80f, 0.70f, 0.34f)));
		Arc->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* Slot = Canvas.AddChildToCanvas(Arc))
		{
			Slot->SetAnchors(FAnchors(LandmarkX, 0.5f));
			Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			Slot->SetOffsets(FMargin(-80.0f, -80.0f, 160.0f, 160.0f));
		}
	}

	void AddGoalThirdReference(
		UWidgetTree& Tree,
		UCanvasPanel& Canvas,
		const FString& Prefix,
		const bool bLocalGoal)
	{
		if (bLocalGoal)
		{
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("PenaltyArea"),
				0.04f, 0.27f, 0.18f, 0.73f, 0.42f);
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("GoalArea"),
				0.04f, 0.39f, 0.10f, 0.61f, 0.38f);
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("GoalVisual"),
				0.018f, 0.43f, 0.04f, 0.57f, 0.58f);
		}
		else
		{
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("PenaltyArea"),
				0.82f, 0.27f, 0.96f, 0.73f, 0.42f);
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("GoalArea"),
				0.90f, 0.39f, 0.96f, 0.61f, 0.38f);
			AddPitchOutline(Tree, Canvas, Prefix + TEXT("GoalVisual"),
				0.96f, 0.43f, 0.982f, 0.57f, 0.58f);
		}
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
	ActiveDeploymentCardId = NAME_None;
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

void UFMCodexPitchWidget::BeginDeploymentDrag(
	const FName CardId,
	const TArray<FFMCodexUMGDeploymentChoiceViewModel>& Choices)
{
	ActiveDeploymentCardId = CardId;
	for (FFMCodexUMGPitchRegionViewModel& Region : Presentation)
	{
		for (FFMCodexUMGPitchSlotViewModel& PitchSlotView : Region.Slots)
		{
			FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
				PitchSlotView, ActiveDeploymentCardId, Choices);
		}
	}
	RefreshVisuals();
}

void UFMCodexPitchWidget::EndDeploymentDrag()
{
	ActiveDeploymentCardId = NAME_None;
	for (FFMCodexUMGPitchRegionViewModel& Region : Presentation)
	{
		for (FFMCodexUMGPitchSlotViewModel& PitchSlotView : Region.Slots)
		{
			PitchSlotView.DeploymentTargetCardId = NAME_None;
			PitchSlotView.DeploymentTargetState = PitchSlotView.bOccupied
				? EFMCodexUMGDeploymentTargetState::Occupied
				: EFMCodexUMGDeploymentTargetState::Neutral;
		}
	}
	RefreshVisuals();
}

FName UFMCodexPitchWidget::GetActiveDeploymentCardId() const
{
	return ActiveDeploymentCardId;
}

void UFMCodexPitchWidget::HandleSlotDeploymentDropped(
	const FName CardId,
	const FName SlotId,
	const bool bGoalkeeper)
{
	OnDeploymentDropped.Broadcast(CardId, SlotId, bGoalkeeper);
}

void UFMCodexPitchWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* FieldSize = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("FootballFieldAspectShell"));
	WidgetTree->RootWidget = FieldSize;
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	UBorder* FieldBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("FootballFieldBackground"));
	Style.ApplyBorder(*FieldBorder, EFMCodexPlayerUIColorRole::PanelBackground,
		FMargin(0.0f));
	FieldSize->AddChild(FieldBorder);
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("PitchBackgroundAssetHook"));
	Style.ApplyBorder(*Background, EFMCodexPlayerUIColorRole::PitchBackground,
		Style.GetCompactPadding());
	FieldBorder->AddChild(Background);
	FieldCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("TwoLanePitchCanvas"));
	FieldCanvas->SetClipping(EWidgetClipping::ClipToBounds);
	Background->AddChild(FieldCanvas);
}

void UFMCodexPitchWidget::RefreshVisuals()
{
	using namespace FMCodexPitchWidget;
	if (FieldCanvas == nullptr)
	{
		return;
	}
	FieldCanvas->ClearChildren();
	RenderedSlotWidgets.Reset();
	AddPitchOutline(*WidgetTree, *FieldCanvas, TEXT("PitchTouchline"),
		0.04f, 0.04f, 0.96f, 0.96f, 0.62f);
	AddPitchLine(*WidgetTree, *FieldCanvas, TEXT("PhysicalHalfVisualSeparator"),
		FAnchors(0.5f, 0.04f, 0.5f, 0.96f),
		FMargin(-1.0f, 0.0f, 2.0f, 0.0f), 0.28f);

	for (int32 RegionIndex = 0; RegionIndex < Presentation.Num(); ++RegionIndex)
	{
		const FFMCodexUMGPitchRegionViewModel& Region = Presentation[RegionIndex];
		const float CenterFraction = Region.bLocalFacingLane ? 0.33f : 0.67f;
		AddSemanticLabel(*WidgetTree, *FieldCanvas, RegionIndex,
			CenterFraction, Region.VisualRoleLabel);
		switch (Region.VisualRole)
		{
		case EFMCodexUMGPitchVisualRole::Forward:
			AddGoalThirdReference(*WidgetTree, *FieldCanvas,
				TEXT("OpponentForward"), false);
			break;
		case EFMCodexUMGPitchVisualRole::Backfield:
			AddGoalThirdReference(*WidgetTree, *FieldCanvas,
				TEXT("LocalBackfield"), true);
			break;
		case EFMCodexUMGPitchVisualRole::Midfield:
		default:
			AddMidfieldReference(*WidgetTree, *FieldCanvas,
				Region.bLocalFacingLane ? TEXT("LocalMidfield")
					: TEXT("OpponentMidfield"), Region.bLocalFacingLane);
			break;
		}
		UVerticalBox* Lane = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), Region.PhysicalHalfLabel.Contains(TEXT("B"))
				? TEXT("PlayerBPhysicalHalf") : TEXT("PlayerAPhysicalHalf"));
		Lane->SetClipping(EWidgetClipping::ClipToBounds);

		UUniformGridPanel* SlotGrid =
			WidgetTree->ConstructWidget<UUniformGridPanel>(
				UUniformGridPanel::StaticClass(), FName(*FString::Printf(
					TEXT("VerticalPitchLaneGrid%d"), RegionIndex)));
		SlotGrid->SetSlotPadding(FMargin(2.0f, 6.0f));
		for (int32 SlotIndex = 0; SlotIndex < Region.Slots.Num(); ++SlotIndex)
		{
			UClass* SlotClass = PitchSlotWidgetClass != nullptr
				? PitchSlotWidgetClass.Get()
				: UFMCodexPitchSlotWidget::StaticClass();
			UFMCodexPitchSlotWidget* SlotWidget =
				WidgetTree->ConstructWidget<UFMCodexPitchSlotWidget>(
					SlotClass, FName(*FString::Printf(
						TEXT("CanonicalPitchSlot%d_%d"), RegionIndex, SlotIndex)));
			SlotWidget->RefreshFromPitchSlotPresentation(Region.Slots[SlotIndex]);
			SlotWidget->OnDeploymentDropped.AddUObject(
				this, &UFMCodexPitchWidget::HandleSlotDeploymentDropped);
			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(
				SlotWidget, SlotIndex, 0);
			GridSlot->SetHorizontalAlignment(HAlign_Center);
			GridSlot->SetVerticalAlignment(VAlign_Center);
			RenderedSlotWidgets.Add(SlotWidget);
		}
		Lane->AddChildToVerticalBox(SlotGrid);
		if (UCanvasPanelSlot* LaneSlot = FieldCanvas->AddChildToCanvas(Lane))
		{
			LaneSlot->SetAnchors(FAnchors(CenterFraction, 0.5f));
			LaneSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			LaneSlot->SetAutoSize(true);
		}
	}
}
