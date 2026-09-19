#include "FMCodexRollReelWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexRollPresentationSurface.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"

namespace FMCodexRollReelWidget
{
	constexpr float DigitTravel = 38.0f;

	UTextBlock* MakeDigit(UWidgetTree& Tree, const FName Name)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetJustification(ETextJustify::Center);
		Result->SetAutoWrapText(false);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*Result, EFMCodexPlayerUITextRole::DiceValue);
		Result->SetColorAndOpacity(FSlateColor(FLinearColor(.92f, .67f, .29f, 1.0f)));
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

void UFMCodexRollReelWidget::SetExpandedChamber(const bool bExpanded)
{
	bExpandedChamber = bExpanded;
	if (auto* Bounds = WidgetTree ? Cast<USizeBox>(WidgetTree->RootWidget) : nullptr)
	{
		Bounds->SetWidthOverride(bExpanded ? 96.0f : 68.0f);
		Bounds->SetHeightOverride(bExpanded ? 112.0f : 72.0f);
	}
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
	Bounds->SetWidthOverride(bExpandedChamber ? 96.0f : 68.0f);
	Bounds->SetHeightOverride(bExpandedChamber ? 112.0f : 72.0f);
	Bounds->SetClipping(EWidgetClipping::ClipToBounds);
	WidgetTree->RootWidget = Bounds;

	auto* Chamber = WidgetTree->ConstructWidget<UFMCodexRollPresentationSurface>(
		UFMCodexRollPresentationSurface::StaticClass(), TEXT("RollReelClippedWindow"));
	Chamber->bNumberChamber = true;
	ReelFrame = Chamber;
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
		Digit->SetRenderTransformPivot(FVector2D(.5f, .5f));
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

	auto* Chamber = CastChecked<UFMCodexRollPresentationSurface>(ReelFrame);
	Chamber->SetLockEmphasis(!Presentation.bVisible ? 0.0f
		: Presentation.bStaticResult ? 1.0f : Presentation.NeighborFadeAlpha);
	SetVisibility(Presentation.bVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	if (!Presentation.bVisible)
	{
		// A reused reel starts with no hidden prior number or landing transform.
		for (UTextBlock* Digit : {PreviousText.Get(), CenterText.Get(), NextText.Get()})
		{
			Digit->SetText(FText::GetEmpty());
			Digit->SetRenderTranslation(FVector2D::ZeroVector);
			Digit->SetRenderScale(FVector2D(1.0f));
			Digit->SetRenderOpacity(0.0f);
		}
		LastPreviousValue = LastCenterValue = LastNextValue = MIN_int32;
		LastCenterOffset = 0.0f;
		LastCenterScale = 1.0f;
		bHasRenderedVisualState = false;
		return;
	}
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
		PreviousText->SetRenderScale(FVector2D(1.0f));
		CenterText->SetRenderScale(FVector2D(1.0f));
		CenterText->SetColorAndOpacity(FSlateColor(FLinearColor(.92f, .67f, .29f, 1)));
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

		// Focus follows distance from the selector, so the incoming number grows
		// into the same gold center and boundary fragments stay secondary.
		const float NeighborOpacityScale = 1.0f - FMath::Clamp(
			Presentation.NeighborFadeAlpha, 0.0f, 1.0f);
		auto FocusDigit = [&](UTextBlock* Digit, float Distance, bool bNeighbor)
		{
			const float D = FMath::Clamp(Distance, 0.0f, 1.0f);
			const float Focus = 1.0f - D * D * (3.0f - 2.0f * D);
			const float EdgeFade = 1.0f - FMath::Clamp((Distance - 1.0f) / .55f, 0.0f, 1.0f);
			Digit->SetRenderScale(FVector2D((.62f + .38f * Focus) * Presentation.LandingScale));
			Digit->SetRenderOpacity((.20f + .80f * Focus) * EdgeFade * (bNeighbor ? NeighborOpacityScale : 1.0f));
			Digit->SetColorAndOpacity(FSlateColor(FMath::Lerp(
				FLinearColor(.30f, .43f, .55f, 1), FLinearColor(.92f, .67f, .29f, 1), Focus)));
		};
		FocusDigit(PreviousText, 1.0f + Alpha, true);
		FocusDigit(CenterText, Alpha, false);
		FocusDigit(NextText, 1.0f - Alpha, true);
		LastCenterScale = CenterText->GetRenderTransform().Scale.X;
	}

	bHasRenderedVisualState = true;
}
