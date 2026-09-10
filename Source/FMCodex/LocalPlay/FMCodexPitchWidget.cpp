#include "FMCodexPitchWidget.h"
#include "FMCodexBroadcastPanel.h"
#include "Components/ScaleBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

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
			Slot->SetAnchors(FAnchors(CenterFraction, 0.0f));
			Slot->SetAlignment(FVector2D(0.5f, 0.0f));
			Slot->SetOffsets(FMargin(0.0f, 5.0f, 160.0f, 28.0f));
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
		UFMCodexBroadcastPanel* Arc = Tree.ConstructWidget<UFMCodexBroadcastPanel>(
			UFMCodexBroadcastPanel::StaticClass(), FName(*(Prefix + TEXT("Arc"))));
		Arc->Surface = bLocalFacing ? EFMCodexBroadcastSurface::MidfieldLeft
			: EFMCodexBroadcastSurface::MidfieldRight;
		Arc->SetBrushColor(FLinearColor(0.72f, 0.80f, 0.70f, 0.64f));
		Arc->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (UCanvasPanelSlot* Slot = Canvas.AddChildToCanvas(Arc))
		{
			Slot->SetAnchors(bLocalFacing ? FAnchors(0.04f, 0.37f, 0.16f, 0.63f)
				: FAnchors(0.84f, 0.37f, 0.96f, 0.63f));
			Slot->SetOffsets(FMargin(0));
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
	static ConstructorHelpers::FObjectFinder<UTexture2D> Turf(
		TEXT("/Game/UI/MatchShell/T_MatchShell_Turf.T_MatchShell_Turf"));
	TurfTexture = Turf.Object;
	static ConstructorHelpers::FObjectFinder<UTexture2D> Stadium(
		TEXT("/Game/UI/MatchShell/T_MatchShell_Stadium.T_MatchShell_Stadium"));
	StadiumTexture = Stadium.Object;
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

void UFMCodexPitchWidget::SetPhaseLabel(const FText& InPhaseLabel)
{
	if (PhaseText) PhaseText->SetText(InPhaseLabel);
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

void UFMCodexPitchWidget::HandleCardDetailHoverRequested(
	UFMCodexPlayerCardWidget* SourceCard)
{
	OnCardDetailHoverRequested.Broadcast(SourceCard);
}

void UFMCodexPitchWidget::HandleCardDetailHoverDismissed(
	UFMCodexPlayerCardWidget* SourceCard)
{
	OnCardDetailHoverDismissed.Broadcast(SourceCard);
}

void UFMCodexPitchWidget::HandleOnPitchSelectionRequested(
	const EFMCodexUMGOnPitchSelectionIntent Intent,
	const FName OptionId)
{
	OnOnPitchSelectionRequested.Broadcast(Intent, OptionId);
}

void UFMCodexPitchWidget::HandleSelectionFeedbackRequested(
	const FName CardId)
{
	OnSelectionFeedbackRequested.Broadcast(CardId);
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
	UOverlay* StadiumLayers = WidgetTree->ConstructWidget<UOverlay>();
	FieldSize->AddChild(StadiumLayers);
	UFMCodexBroadcastPanel* FieldBorder = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("FootballFieldBackground"));
	FieldBorder->Surface = EFMCodexBroadcastSurface::PitchSurround;
	FieldBorder->SetPadding(FMargin(38.0f, 58.0f, 38.0f, 36.0f));
	FieldBorder->SetBrushFromTexture(StadiumTexture);
	UOverlaySlot* SurroundSlot = StadiumLayers->AddChildToOverlay(FieldBorder);
	SurroundSlot->SetHorizontalAlignment(HAlign_Fill);
	SurroundSlot->SetVerticalAlignment(VAlign_Fill);
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("PitchBackgroundAssetHook"));
	Style.ApplyBorder(*Background, EFMCodexPlayerUIColorRole::PitchBackground,
		Style.GetCompactPadding());
	Background->SetBrushFromTexture(TurfTexture);
	Background->SetBrushColor(FLinearColor(0.325f, 0.52f, 0.455f, 1.0f));
	Background->SetPadding(FMargin(0));
	FieldBorder->AddChild(Background);
	FieldCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
		UCanvasPanel::StaticClass(), TEXT("TwoLanePitchCanvas"));
	FieldCanvas->SetClipping(EWidgetClipping::ClipToBounds);
	UFMCodexBroadcastPanel* TurfLight = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("TurfEdgeLighting"));
	TurfLight->Surface = EFMCodexBroadcastSurface::TurfLighting;
	TurfLight->SetPadding(FMargin(0));
	TurfLight->AddChild(FieldCanvas);
	Background->AddChild(TurfLight);
	USizeBox* HudBounds = WidgetTree->ConstructWidget<USizeBox>();
	HudBounds->SetHeightOverride(36.0f);
	UFMCodexBroadcastPanel* Hud = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("PitchSemanticHUD"));
	Hud->Surface = EFMCodexBroadcastSurface::PitchHUD;
	Hud->SetPadding(FMargin(0));
	Hud->SetVisibility(ESlateVisibility::HitTestInvisible);
	HudCanvas = WidgetTree->ConstructWidget<UCanvasPanel>();
	Hud->AddChild(HudCanvas);
	HudBounds->AddChild(Hud);
	UOverlaySlot* HudSlot = StadiumLayers->AddChildToOverlay(HudBounds);
	HudSlot->SetHorizontalAlignment(HAlign_Fill);
	HudSlot->SetVerticalAlignment(VAlign_Top);
	PhaseText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("PitchPhaseStatusLabel"));
	PhaseText->SetJustification(ETextJustify::Center);
	PhaseText->SetAutoWrapText(false);
	PhaseText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	Style.ApplyText(*PhaseText, EFMCodexPlayerUITextRole::Secondary);
	if (UCanvasPanelSlot* PhaseSlot = HudCanvas->AddChildToCanvas(PhaseText))
	{
		PhaseSlot->SetAnchors(FAnchors(0.5f,0));
		PhaseSlot->SetAlignment(FVector2D(0.5f,0));
		PhaseSlot->SetOffsets(FMargin(0,8,210,24));
	}
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
	// Rebuild only semantic side labels; the phase carrier keeps its safe displayed text.
	for (UWidget* Child : HudCanvas->GetAllChildren())
		if (Child != PhaseText) HudCanvas->RemoveChild(Child);
	AddPitchOutline(*WidgetTree, *FieldCanvas, TEXT("PitchTouchline"),
		0.04f, 0.04f, 0.96f, 0.96f, 0.62f);
	AddPitchLine(*WidgetTree, *FieldCanvas, TEXT("PhysicalHalfVisualSeparator"),
		FAnchors(0.5f, 0.04f, 0.5f, 0.96f),
		FMargin(-1.0f, 0.0f, 2.0f, 0.0f), 0.28f);

	for (int32 RegionIndex = 0; RegionIndex < Presentation.Num(); ++RegionIndex)
	{
		const FFMCodexUMGPitchRegionViewModel& Region = Presentation[RegionIndex];
		const float CenterFraction = Region.bLocalFacingLane ? 0.33f : 0.67f;
		AddSemanticLabel(*WidgetTree, *HudCanvas, RegionIndex,
			Region.bLocalFacingLane ? 0.25f : 0.75f, Region.VisualRoleLabel);
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
			SlotWidget->OnCardDetailHoverRequested.AddUObject(
				this, &UFMCodexPitchWidget::HandleCardDetailHoverRequested);
			SlotWidget->OnCardDetailHoverDismissed.AddUObject(
				this, &UFMCodexPitchWidget::HandleCardDetailHoverDismissed);
			SlotWidget->OnOnPitchSelectionRequested.AddUObject(
				this, &UFMCodexPitchWidget::HandleOnPitchSelectionRequested);
			SlotWidget->OnSelectionFeedbackRequested.AddUObject(
				this, &UFMCodexPitchWidget::HandleSelectionFeedbackRequested);
			UUniformGridSlot* GridSlot = SlotGrid->AddChildToUniformGrid(
				SlotWidget, SlotIndex, 0);
			GridSlot->SetHorizontalAlignment(HAlign_Center);
			GridSlot->SetVerticalAlignment(VAlign_Center);
			RenderedSlotWidgets.Add(SlotWidget);
		}
		Lane->AddChildToVerticalBox(SlotGrid);
		UScaleBox* LaneFit = WidgetTree->ConstructWidget<UScaleBox>();
		LaneFit->SetStretch(EStretch::ScaleToFit);
		LaneFit->SetStretchDirection(EStretchDirection::DownOnly);
		LaneFit->AddChild(Lane);
		if (UCanvasPanelSlot* LaneSlot = FieldCanvas->AddChildToCanvas(LaneFit))
		{
			LaneSlot->SetAnchors(FAnchors(CenterFraction, 0.04f, CenterFraction, 0.96f));
			LaneSlot->SetAlignment(FVector2D(0.5f, 0.0f));
			LaneSlot->SetOffsets(FMargin(0.0f, 0.0f, 160.0f, 0.0f));
		}
	}
}
