#include "FMCodexPitchSlotWidget.h"

#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
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

bool UFMCodexPitchSlotWidget::CanAcceptDeploymentOperation(
	const UFMCodexDeploymentDragDropOperation* Operation) const
{
	return Operation != nullptr
		&& !Operation->CardId.IsNone()
		&& !Presentation.SlotId.IsNone()
		&& !Presentation.bOccupied
		&& Presentation.DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Valid
		&& Presentation.DeploymentTargetCardId == Operation->CardId;
}

bool UFMCodexPitchSlotWidget::TryHandleDeploymentDrop(
	UFMCodexDeploymentDragDropOperation* Operation)
{
	if (!CanAcceptDeploymentOperation(Operation))
	{
		return false;
	}
	OnDeploymentDropped.Broadcast(
		Operation->CardId, Presentation.SlotId, Operation->bGoalkeeper);
	return true;
}

void UFMCodexPitchSlotWidget::NativeOnDragEnter(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	bDragHovered = Cast<UFMCodexDeploymentDragDropOperation>(InOperation)
		!= nullptr;
	RefreshVisuals();
}

void UFMCodexPitchSlotWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	bDragHovered = false;
	RefreshVisuals();
}

bool UFMCodexPitchSlotWidget::NativeOnDragOver(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return CanAcceptDeploymentOperation(
		Cast<UFMCodexDeploymentDragDropOperation>(InOperation));
}

bool UFMCodexPitchSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	bDragHovered = false;
	return TryHandleDeploymentDrop(
		Cast<UFMCodexDeploymentDragDropOperation>(InOperation));
}

void UFMCodexPitchSlotWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* SlotSize = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("CanonicalPitchSlotSize"));
	SlotSize->SetWidthOverride(148.0f);
	SlotSize->SetHeightOverride(148.0f);
	WidgetTree->RootWidget = SlotSize;

	SlotBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("CanonicalPitchSlotBorder"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*SlotBorder,
		EFMCodexPlayerUIColorRole::EmptyPitchSlot,
		FMargin(4.0f));
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
	SlotLabelText->SetVisibility(ESlateVisibility::Collapsed);
	ContextText->SetVisibility(ESlateVisibility::Collapsed);

	TargetStateText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("DeploymentTargetState"));
	TargetStateText->SetAutoWrapText(true);
	Style.ApplyText(*TargetStateText, EFMCodexPlayerUITextRole::Status);
	Body->AddChildToVerticalBox(TargetStateText);

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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	const EFMCodexUMGDeploymentTargetState VisibleTargetState =
		Presentation.bOccupied
			? EFMCodexUMGDeploymentTargetState::Occupied
			: Presentation.DeploymentTargetState;
	FText TargetLabel = FFMCodexPlayerUIPresentationText::EmptyPitchSlot();
	EFMCodexPlayerUIColorRole TargetColorRole =
		EFMCodexPlayerUIColorRole::EmptyPitchSlot;
	switch (VisibleTargetState)
	{
	case EFMCodexUMGDeploymentTargetState::Valid:
		TargetLabel = FFMCodexPlayerUIPresentationText::ValidDeploymentTarget();
		TargetColorRole = bDragHovered
			? EFMCodexPlayerUIColorRole::Warning
			: EFMCodexPlayerUIColorRole::Success;
		break;
	case EFMCodexUMGDeploymentTargetState::Invalid:
		TargetLabel = FFMCodexPlayerUIPresentationText::InvalidDeploymentTarget();
		TargetColorRole = EFMCodexPlayerUIColorRole::Danger;
		break;
	case EFMCodexUMGDeploymentTargetState::Occupied:
		TargetLabel = FText::GetEmpty();
		TargetColorRole = EFMCodexPlayerUIColorRole::OccupiedPitchSlot;
		break;
	case EFMCodexUMGDeploymentTargetState::Unavailable:
		TargetLabel = FFMCodexPlayerUIPresentationText::UnavailableDeploymentTarget();
		TargetColorRole = EFMCodexPlayerUIColorRole::ActionDisabled;
		break;
	default:
		break;
	}
	TargetStateText->SetText(TargetLabel);
	TargetStateText->SetVisibility(
		VisibleTargetState == EFMCodexUMGDeploymentTargetState::Neutral
			|| VisibleTargetState == EFMCodexUMGDeploymentTargetState::Occupied
			? ESlateVisibility::Collapsed
			: ESlateVisibility::HitTestInvisible);
	SlotBorder->SetBrushColor(Style.GetColor(TargetColorRole));

	if (Presentation.bOccupied)
	{
		UClass* ResolvedCardClass = PlayerCardWidgetClass != nullptr
			? PlayerCardWidgetClass.Get()
			: UFMCodexPlayerCardWidget::StaticClass();
		CardWidget = WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
			ResolvedCardClass, TEXT("OccupiedPitchCard"));
		CardWidget->RefreshFromPresentation(
			Presentation.Card,
			EFMCodexPlayerCardPresentationMode::PitchMini);
		ContentBody->AddChildToVerticalBox(CardWidget);
	}
	else
	{
		EmptyStateText = WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), TEXT("EmptySpatialLocation"));
		EmptyStateText->SetText(
			FFMCodexPlayerUIPresentationText::EmptyPitchSlot());
		EmptyStateText->SetAutoWrapText(true);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*EmptyStateText, EFMCodexPlayerUITextRole::Secondary);
		EmptyStateText->SetVisibility(ESlateVisibility::Collapsed);
		ContentBody->AddChildToVerticalBox(EmptyStateText);
	}
}
