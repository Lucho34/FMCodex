#include "FMCodexSelectionFeedbackToastWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UFMCodexSelectionFeedbackToastWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	DismissFeedback();
}

TSharedRef<SWidget> UFMCodexSelectionFeedbackToastWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	return Super::RebuildWidget();
}

void UFMCodexSelectionFeedbackToastWidget::NativeDestruct()
{
	ClearDismissTimer();
	Super::NativeDestruct();
}

void UFMCodexSelectionFeedbackToastWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("SelectionFeedbackToastBounds"));
	Bounds->SetMinDesiredWidth(420.0f);
	Bounds->SetHeightOverride(42.0f);
	WidgetTree->RootWidget = Bounds;

	ToastFrame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("SelectionFeedbackToastFrame"));
	ToastFrame->SetBrush(FSlateRoundedBoxBrush(
		FLinearColor(0.025f, 0.075f, 0.11f, 0.94f), 5.0f,
		FLinearColor(0.88f, 0.64f, 0.27f, 0.78f), 1.0f,
		FVector2f(420.0f, 42.0f)));
	ToastFrame->SetBrushColor(FLinearColor::White);
	ToastFrame->SetPadding(FMargin(18.0f, 7.0f));
	Bounds->AddChild(ToastFrame);

	ToastText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("SelectionFeedbackToastText"));
	ToastText->SetJustification(ETextJustify::Center);
	ToastText->SetAutoWrapText(false);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*ToastText, EFMCodexPlayerUITextRole::Body);
	ToastText->SetColorAndOpacity(FSlateColor(
		FLinearColor(0.96f, 0.96f, 0.92f, 1.0f)));
	ToastFrame->AddChild(ToastText);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UFMCodexSelectionFeedbackToastWidget::ShowFeedback(
	const EFMCodexUMGSelectionFeedbackReason InReason,
	const FString& InLabel)
{
	if (InReason == EFMCodexUMGSelectionFeedbackReason::None
		|| InLabel.IsEmpty())
	{
		DismissFeedback();
		return;
	}

	BuildWidgetTree();
	CurrentReason = InReason;
	++TriggerSerial;
	if (ToastText != nullptr)
	{
		ToastText->SetText(FText::FromString(InLabel));
	}
	SetVisibility(ESlateVisibility::HitTestInvisible);
	ClearDismissTimer();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DismissTimerHandle, this,
			&UFMCodexSelectionFeedbackToastWidget::DismissFeedback,
			DisplayDurationSeconds, false);
	}
}

void UFMCodexSelectionFeedbackToastWidget::DismissFeedback()
{
	ClearDismissTimer();
	CurrentReason = EFMCodexUMGSelectionFeedbackReason::None;
	if (ToastText != nullptr)
	{
		ToastText->SetText(FText::GetEmpty());
	}
	SetVisibility(ESlateVisibility::Collapsed);
}

void UFMCodexSelectionFeedbackToastWidget::ClearDismissTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DismissTimerHandle);
	}
}

EFMCodexUMGSelectionFeedbackReason
UFMCodexSelectionFeedbackToastWidget::GetCurrentReason() const
{
	return CurrentReason;
}

FText UFMCodexSelectionFeedbackToastWidget::GetDisplayedText() const
{
	return ToastText != nullptr ? ToastText->GetText() : FText::GetEmpty();
}

float UFMCodexSelectionFeedbackToastWidget::GetDisplayDurationSeconds() const
{
	return DisplayDurationSeconds;
}

int32 UFMCodexSelectionFeedbackToastWidget::GetTriggerSerial() const
{
	return TriggerSerial;
}

bool UFMCodexSelectionFeedbackToastWidget::IsFeedbackVisible() const
{
	return GetVisibility() == ESlateVisibility::HitTestInvisible;
}
