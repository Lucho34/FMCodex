#include "FMCodexCardRackWidget.h"

#include "FMCodexHandMicroDiagnostics.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

UFMCodexCardRackWidget::UFMCodexCardRackWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCardWidgetClass = UFMCodexPlayerCardWidget::StaticClass();
}

void UFMCodexCardRackWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexCardRackWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexCardRackWidget::RefreshFromPresentation(
	const FFMCodexUMGCardRackViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGCardRackViewModel&
UFMCodexCardRackWidget::GetPresentation() const
{
	return Presentation;
}

const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
UFMCodexCardRackWidget::GetRenderedCardWidgets() const
{
	return RenderedCardWidgets;
}

int32 UFMCodexCardRackWidget::GetRenderedCellCount() const
{
	return RenderedCellCount;
}

EFMCodexUMGCardInteractionState
UFMCodexCardRackWidget::GetCellInteractionState(const int32 StableIndex) const
{
	const FFMCodexUMGCardRackCellViewModel* Cell =
		Presentation.Cells.FindByPredicate(
			[StableIndex](const FFMCodexUMGCardRackCellViewModel& Candidate)
			{
				return Candidate.StableIndex == StableIndex;
			});
	if (Cell == nullptr)
	{
		return EFMCodexUMGCardInteractionState::Default;
	}
	if (Cell->bPlayed)
	{
		return EFMCodexUMGCardInteractionState::Ghost;
	}
	const TObjectPtr<UFMCodexPlayerCardWidget>* RenderedCard =
		RenderedCardWidgets.FindByPredicate(
			[Cell](const UFMCodexPlayerCardWidget* Candidate)
			{
				return Candidate != nullptr
					&& Candidate->GetPresentation().CardId == Cell->Card.CardId;
			});
	return RenderedCard == nullptr || RenderedCard->Get() == nullptr
		? EFMCodexUMGCardInteractionState::Default
		: RenderedCard->Get()->GetInteractionState();
}

void UFMCodexCardRackWidget::HandleCardDragStarted(
	const FName CardId,
	const bool bGoalkeeper)
{
	OnCardDragStarted.Broadcast(CardId, bGoalkeeper);
}

void UFMCodexCardRackWidget::HandleCardDragFinished()
{
	OnCardDragFinished.Broadcast();
}

void UFMCodexCardRackWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("PersistentCardRackFrame"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		FMargin(6.0f, 4.0f));
	Frame->SetClipping(EWidgetClipping::ClipToBounds);
	WidgetTree->RootWidget = Frame;

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PersistentCardRackHierarchy"));
	Frame->AddChild(Body);
	UHorizontalBox* HeadingRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("CardRackHeadingRow"));
	RackHeading = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardRackHeading"));
	RackHeading->SetJustification(ETextJustify::Center);
	Style.ApplyText(*RackHeading, EFMCodexPlayerUITextRole::SectionHeading);
	if (UHorizontalBoxSlot* HeadingSlot =
		HeadingRow->AddChildToHorizontalBox(RackHeading))
	{
		HeadingSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		HeadingSlot->SetHorizontalAlignment(HAlign_Right);
		HeadingSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
	}
	TacticalPlayerCountText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("TacticalPlayerCountStatus"));
	TacticalPlayerCountText->SetJustification(ETextJustify::Left);
	Style.ApplyText(
		*TacticalPlayerCountText, EFMCodexPlayerUITextRole::Secondary);
	if (UHorizontalBoxSlot* CountSlot =
		HeadingRow->AddChildToHorizontalBox(TacticalPlayerCountText))
	{
		CountSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CountSlot->SetHorizontalAlignment(HAlign_Left);
		CountSlot->SetVerticalAlignment(VAlign_Center);
	}
	Body->AddChildToVerticalBox(HeadingRow);
	RackGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(
		UUniformGridPanel::StaticClass(), TEXT("StableTwoByTenCardRackGrid"));
	RackGrid->SetSlotPadding(FMargin(6.0f, 4.0f));
	if (UVerticalBoxSlot* GridBodySlot = Body->AddChildToVerticalBox(RackGrid))
	{
		GridBodySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		GridBodySlot->SetVerticalAlignment(VAlign_Top);
	}
}

void UFMCodexCardRackWidget::RefreshVisuals()
{
	if (RackGrid == nullptr)
	{
		return;
	}
	RackHeading->SetText(FFMCodexPlayerUIPresentationText::RackHeading(
		Presentation.SideLabel, Presentation.bLocalRack));
	if (TacticalPlayerCountText != nullptr)
	{
		TacticalPlayerCountText->SetText(FText::FromString(
			Presentation.TacticalPlayerCountLabel));
		TacticalPlayerCountText->SetVisibility(
			Presentation.bHasTacticalPlayerCount
				? ESlateVisibility::HitTestInvisible
				: ESlateVisibility::Collapsed);
	}
	RackGrid->ClearChildren();
	RenderedCardWidgets.Reset();
	RenderedCellCount = 0;

	const int32 ColumnCount = FMath::Max(1, Presentation.ColumnCount);
	const float CardWidth = FMCodexHandMicroDiagnostics::CardWidth;
	const float CardHeight = FMCodexHandMicroDiagnostics::CardHeight;
	for (int32 Index = 0; Index < Presentation.Cells.Num(); ++Index)
	{
		const FFMCodexUMGCardRackCellViewModel& Cell = Presentation.Cells[Index];
		UWidget* CellWidget = nullptr;
		if (Cell.bPlayed)
		{
			USizeBox* GhostBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostBounds%d"), Cell.StableIndex)));
			GhostBounds->SetWidthOverride(CardWidth);
			GhostBounds->SetHeightOverride(CardHeight);
			UBorder* Ghost = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostCell%d"), Cell.StableIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*Ghost,
				EFMCodexPlayerUIColorRole::CardFrame, FMargin(0.0f));
			Ghost->SetBrushColor(FLinearColor::FromSRGBColor(
				FColor(0x17, 0x30, 0x3B)));
			Ghost->SetRenderOpacity(1.0f);
			UHorizontalBox* GhostStructure =
				WidgetTree->ConstructWidget<UHorizontalBox>(
					UHorizontalBox::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostStructure%d"), Cell.StableIndex)));
			USizeBox* GhostPortraitBounds =
				WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostPortraitBounds%d"), Cell.StableIndex)));
			GhostPortraitBounds->SetWidthOverride(96.0f);
			GhostPortraitBounds->SetHeightOverride(CardHeight);
			UBorder* GhostPortrait = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostPortraitShape%d"), Cell.StableIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*GhostPortrait,
				EFMCodexPlayerUIColorRole::PanelInset, FMargin(0.0f));
			GhostPortrait->SetBrushColor(FLinearColor::FromSRGBColor(
				FColor(0x17, 0x30, 0x3B)));
			GhostPortrait->SetRenderOpacity(1.0f);
			GhostPortraitBounds->AddChild(GhostPortrait);
			if (UHorizontalBoxSlot* PortraitSlot =
				GhostStructure->AddChildToHorizontalBox(GhostPortraitBounds))
			{
				PortraitSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
			}
			USizeBox* GhostDividerBounds =
				WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostDividerBounds%d"), Cell.StableIndex)));
			GhostDividerBounds->SetWidthOverride(1.0f);
			UBorder* GhostDivider = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostDivider%d"), Cell.StableIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*GhostDivider,
				EFMCodexPlayerUIColorRole::NeutralAccent, FMargin(0.0f));
			GhostDivider->SetRenderOpacity(0.10f);
			GhostDividerBounds->AddChild(GhostDivider);
			GhostStructure->AddChildToHorizontalBox(GhostDividerBounds);
			UBorder* GhostIdentity = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostIdentityShape%d"), Cell.StableIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*GhostIdentity,
				EFMCodexPlayerUIColorRole::PanelBackground, FMargin(0.0f));
			GhostIdentity->SetBrushColor(FLinearColor::FromSRGBColor(
				FColor(0x17, 0x30, 0x3B)));
			GhostIdentity->SetRenderOpacity(1.0f);
			if (UHorizontalBoxSlot* IdentitySlot =
				GhostStructure->AddChildToHorizontalBox(GhostIdentity))
			{
				IdentitySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
			USizeBox* GhostTrailingRailBounds =
				WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostTrailingRailBounds%d"), Cell.StableIndex)));
			GhostTrailingRailBounds->SetWidthOverride(4.0f);
			UBorder* GhostTrailingRail = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostTrailingRail%d"), Cell.StableIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*GhostTrailingRail,
				EFMCodexPlayerUIColorRole::PanelInset, FMargin(0.0f));
			GhostTrailingRail->SetBrushColor(FLinearColor::FromSRGBColor(
				FColor(0x17, 0x30, 0x3B)));
			GhostTrailingRail->SetRenderOpacity(1.0f);
			GhostTrailingRailBounds->AddChild(GhostTrailingRail);
			GhostStructure->AddChildToHorizontalBox(GhostTrailingRailBounds);
			UOverlay* GhostLayers = WidgetTree->ConstructWidget<UOverlay>(
				UOverlay::StaticClass(), FName(*FString::Printf(
					TEXT("PlayedCardGhostLayers%d"), Cell.StableIndex)));
			GhostLayers->AddChildToOverlay(GhostStructure);
			auto AddGhostFrameRail = [this, GhostLayers, &Cell](
				const TCHAR* EdgeName,
				const float Width,
				const float Height,
				const EHorizontalAlignment HorizontalAlignment,
				const EVerticalAlignment VerticalAlignment)
			{
				USizeBox* RailBounds = WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostFrame%sBounds%d"),
						EdgeName, Cell.StableIndex)));
				if (Width > 0.0f)
				{
					RailBounds->SetWidthOverride(Width);
				}
				if (Height > 0.0f)
				{
					RailBounds->SetHeightOverride(Height);
				}
				UBorder* Rail = WidgetTree->ConstructWidget<UBorder>(
					UBorder::StaticClass(), FName(*FString::Printf(
						TEXT("PlayedCardGhostFrame%s%d"),
						EdgeName, Cell.StableIndex)));
				Rail->SetBrushColor(FLinearColor(0.48f, 0.57f, 0.62f, 0.18f));
				RailBounds->AddChild(Rail);
				if (UOverlaySlot* RailSlot =
					GhostLayers->AddChildToOverlay(RailBounds))
				{
					RailSlot->SetHorizontalAlignment(HorizontalAlignment);
					RailSlot->SetVerticalAlignment(VerticalAlignment);
				}
			};
			AddGhostFrameRail(TEXT("Top"), 0.0f, 1.0f,
				HAlign_Fill, VAlign_Top);
			AddGhostFrameRail(TEXT("Bottom"), 0.0f, 1.0f,
				HAlign_Fill, VAlign_Bottom);
			AddGhostFrameRail(TEXT("Left"), 1.0f, 0.0f,
				HAlign_Left, VAlign_Fill);
			AddGhostFrameRail(TEXT("Right"), 1.0f, 0.0f,
				HAlign_Right, VAlign_Fill);
			Ghost->AddChild(GhostLayers);
			GhostBounds->AddChild(Ghost);
			CellWidget = GhostBounds;
		}
		else
		{
			UClass* CardClass = PlayerCardWidgetClass != nullptr
				? PlayerCardWidgetClass.Get()
				: UFMCodexPlayerCardWidget::StaticClass();
			UFMCodexPlayerCardWidget* Card =
				WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
					CardClass, FName(*FString::Printf(
						TEXT("StableRackCard%d"), Cell.StableIndex)));
			Card->RefreshFromPresentation(
				Cell.Card, EFMCodexPlayerCardPresentationMode::HandMicro);
			if (Presentation.bLocalRack && Cell.bDeploymentDraggable)
			{
				Card->ConfigureDeploymentDrag(Cell.Card.CardId, Cell.bGoalkeeper);
				Card->OnDeploymentDragStarted.AddUObject(
					this, &UFMCodexCardRackWidget::HandleCardDragStarted);
				Card->OnDeploymentDragFinished.AddUObject(
					this, &UFMCodexCardRackWidget::HandleCardDragFinished);
			}
			else
			{
				Card->ClearDeploymentDrag();
			}
			RenderedCardWidgets.Add(Card);
			CellWidget = Card;
		}

		UUniformGridSlot* GridSlot = RackGrid->AddChildToUniformGrid(
			CellWidget, Index / ColumnCount, Index % ColumnCount);
		GridSlot->SetHorizontalAlignment(HAlign_Center);
		GridSlot->SetVerticalAlignment(VAlign_Center);
		++RenderedCellCount;
	}
}
