#include "FMCodexRollReelWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

namespace FMCodexRollReelWidget
{
	constexpr float DigitTravel = 46.0f;

	UTextBlock* MakeDigit(UWidgetTree& Tree, const FName Name)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetJustification(ETextJustify::Center);
		Result->SetAutoWrapText(false);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*Result, EFMCodexPlayerUITextRole::DiceValue);
		return Result;
	}
}

UFMCodexRollReelWidget::UFMCodexRollReelWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexRollReelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexRollReelWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexRollReelWidget::RefreshFromPresentation(
	const FFMCodexUMGRollReelViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGRollReelViewModel&
UFMCodexRollReelWidget::GetPresentation() const
{
	return Presentation;
}

int32 UFMCodexRollReelWidget::GetStripDigitCount() const
{
	return 3;
}

int32 UFMCodexRollReelWidget::GetRenderedChildCount() const
{
	return NumberStrip == nullptr ? 0 : NumberStrip->GetChildrenCount();
}

float UFMCodexRollReelWidget::GetCenterVerticalOffset() const
{
	return LastCenterOffset;
}

float UFMCodexRollReelWidget::GetCenterRenderScale() const
{
	return LastCenterScale;
}

float UFMCodexRollReelWidget::GetCenterRenderOpacity() const
{
	return CenterText == nullptr ? 0.0f : CenterText->GetRenderOpacity();
}

float UFMCodexRollReelWidget::GetMaximumNeighborRenderOpacity() const
{
	return PreviousText == nullptr || NextText == nullptr
		? 0.0f
		: FMath::Max(
			PreviousText->GetRenderOpacity(), NextText->GetRenderOpacity());
}

FLinearColor UFMCodexRollReelWidget::GetFrameBrushColor() const
{
	return ReelFrame == nullptr ? FLinearColor::Transparent
		: ReelFrame->GetBrushColor();
}

const UTextBlock* UFMCodexRollReelWidget::GetCenterDigitWidget() const
{
	return CenterText;
}

bool UFMCodexRollReelWidget::HasClippedWindow() const
{
	return ReelFrame != nullptr
		&& ReelFrame->GetClipping() == EWidgetClipping::ClipToBounds;
}

int32 UFMCodexRollReelWidget::GetVisibleNeighborDigitCount() const
{
	int32 Result = 0;
	for (const UTextBlock* Digit : { PreviousText.Get(), NextText.Get() })
	{
		if (Digit != nullptr
			&& Digit->GetVisibility() != ESlateVisibility::Collapsed
			&& Digit->GetRenderOpacity() > KINDA_SMALL_NUMBER)
		{
			++Result;
		}
	}
	return Result;
}

bool UFMCodexRollReelWidget::IsStaticResultTileVisible() const
{
	return Presentation.bStaticResult
		&& CenterText != nullptr
		&& CenterText->GetVisibility() != ESlateVisibility::Collapsed
		&& GetVisibleNeighborDigitCount() == 0;
}

void UFMCodexRollReelWidget::BuildWidgetTree()
{
	using namespace FMCodexRollReelWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("RollReelBounds"));
	Bounds->SetWidthOverride(68.0f);
	Bounds->SetHeightOverride(72.0f);
	Bounds->SetClipping(EWidgetClipping::ClipToBounds);
	WidgetTree->RootWidget = Bounds;

	ReelFrame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("RollReelClippedWindow"));
	FFMCodexPlayerUIStyle::Get().ApplyBorder(
		*ReelFrame, EFMCodexPlayerUIColorRole::Warning,
		FMargin(3.0f));
	ReelFrame->SetClipping(EWidgetClipping::ClipToBounds);
	Bounds->AddChild(ReelFrame);

	NumberStrip = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("RollReelNumberStrip"));
	NumberStrip->SetClipping(EWidgetClipping::ClipToBounds);
	ReelFrame->AddChild(NumberStrip);

	PreviousText = MakeDigit(*WidgetTree, TEXT("RollReelPreviousDigit"));
	CenterText = MakeDigit(*WidgetTree, TEXT("RollReelCenterDigit"));
	NextText = MakeDigit(*WidgetTree, TEXT("RollReelNextDigit"));
	for (UTextBlock* Digit : { PreviousText, CenterText, NextText })
	{
		if (UOverlaySlot* DigitSlot = NumberStrip->AddChildToOverlay(Digit))
		{
			DigitSlot->SetHorizontalAlignment(HAlign_Fill);
			DigitSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

void UFMCodexRollReelWidget::RefreshVisuals()
{
	using namespace FMCodexRollReelWidget;
	if (ReelFrame == nullptr || PreviousText == nullptr
		|| CenterText == nullptr || NextText == nullptr)
	{
		return;
	}

	SetVisibility(Presentation.bVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	if (LastPreviousValue != Presentation.PreviousValue)
	{
		PreviousText->SetText(FText::AsNumber(Presentation.PreviousValue));
		LastPreviousValue = Presentation.PreviousValue;
	}
	if (LastCenterValue != Presentation.CenterValue)
	{
		CenterText->SetText(FText::AsNumber(Presentation.CenterValue));
		LastCenterValue = Presentation.CenterValue;
	}
	if (LastNextValue != Presentation.NextValue)
	{
		NextText->SetText(FText::AsNumber(Presentation.NextValue));
		LastNextValue = Presentation.NextValue;
	}

	if (!bHasRenderedVisualState
		|| bLastShowNeighborDigits != Presentation.bShowNeighborDigits)
	{
		const ESlateVisibility NeighborVisibility =
			Presentation.bShowNeighborDigits
				? ESlateVisibility::SelfHitTestInvisible
				: ESlateVisibility::Collapsed;
		PreviousText->SetVisibility(NeighborVisibility);
		NextText->SetVisibility(NeighborVisibility);
		CenterText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		bLastShowNeighborDigits = Presentation.bShowNeighborDigits;
	}
	if (Presentation.bStaticResult)
	{
		PreviousText->SetRenderTranslation(FVector2D::ZeroVector);
		CenterText->SetRenderTranslation(FVector2D::ZeroVector);
		NextText->SetRenderTranslation(FVector2D::ZeroVector);
		PreviousText->SetRenderOpacity(0.0f);
		CenterText->SetRenderOpacity(1.0f);
		NextText->SetRenderOpacity(0.0f);
		CenterText->SetRenderScale(FVector2D(1.0f));
		NextText->SetRenderScale(FVector2D(1.0f));
		LastCenterOffset = 0.0f;
		LastCenterScale = 1.0f;
	}
	else
	{
		const float Alpha = FMath::Clamp(
			Presentation.ScrollAlpha, 0.0f, 1.0f);
		const float Travel = Alpha * DigitTravel;
		PreviousText->SetRenderTranslation(FVector2D(
			0.0f, -DigitTravel - Travel + Presentation.LandingOffsetY));
		LastCenterOffset = -Travel + Presentation.LandingOffsetY;
		CenterText->SetRenderTranslation(FVector2D(0.0f, LastCenterOffset));
		NextText->SetRenderTranslation(FVector2D(
			0.0f, DigitTravel - Travel + Presentation.LandingOffsetY));

		PreviousText->SetRenderOpacity(0.20f * (1.0f - Alpha));
		CenterText->SetRenderOpacity(1.0f - 0.55f * Alpha);
		NextText->SetRenderOpacity(0.45f + 0.55f * Alpha);
		const float NeighborOpacityScale = 1.0f - FMath::Clamp(
			Presentation.NeighborFadeAlpha, 0.0f, 1.0f);
		PreviousText->SetRenderOpacity(
			PreviousText->GetRenderOpacity() * NeighborOpacityScale);
		NextText->SetRenderOpacity(
			NextText->GetRenderOpacity() * NeighborOpacityScale);
		const FVector2D LandingScale(Presentation.LandingScale);
		CenterText->SetRenderScale(LandingScale);
		NextText->SetRenderScale(LandingScale);
		LastCenterScale = Presentation.LandingScale;
	}

	bHasRenderedVisualState = true;
}
