#include "FMCodexPlayerCardWidget.h"

#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexHandMicroDiagnostics.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Rendering/SlateRenderer.h"

DEFINE_LOG_CATEGORY_STATIC(LogFMCodexPlayerCardArt, Log, All);

namespace FMCodexPlayerCardWidget
{
	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& InitialText = FString())
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(InitialText));
		Result->SetAutoWrapText(true);
		return Result;
	}

	UBorder* MakeRegion(
		UWidgetTree& Tree,
		const FName Name,
		const FLinearColor& Color,
		const FMargin Padding = FMargin(5.0f))
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		Result->SetPadding(Padding);
		Result->SetBrushColor(Color);
		return Result;
	}

	void ConfigureBoundedSingleLine(UTextBlock& Text)
	{
		Text.SetAutoWrapText(false);
		Text.SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		Text.SetClipping(EWidgetClipping::ClipToBounds);
	}

	void ConfigureHandMicroName(UTextBlock& Text)
	{
		Text.SetAutoWrapText(false);
		Text.SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
		Text.SetClipping(EWidgetClipping::Inherit);
	}

	bool TryMeasureHandMicroName(
		const FText& Name,
		const FSlateFontInfo& Font,
		float& OutWidth)
	{
		if (!FSlateApplication::IsInitialized()
			|| FSlateApplication::Get().GetRenderer() == nullptr)
		{
			return false;
		}
		const TSharedRef<FSlateFontMeasure> FontMeasure =
			FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		OutWidth = FontMeasure->Measure(Name, Font).X;
		return FMath::IsFinite(OutWidth);
	}

	int32 GetHandMicroNameFontSize(
		const FText& Name,
		const FSlateFontInfo& BaseFont,
		const float HandMicroNameSafeWidth,
		const int32 MaximumFontSize)
	{
		for (int32 FontSize = MaximumFontSize;
			FontSize >= FMCodexHandMicroDiagnostics::MinimumNameFontSize; --FontSize)
		{
			FSlateFontInfo CandidateFont = BaseFont;
			CandidateFont.Size = FontSize;
			float MeasuredWidth = 0.0f;
			if (TryMeasureHandMicroName(Name, CandidateFont, MeasuredWidth)
				&& MeasuredWidth <= HandMicroNameSafeWidth)
			{
				return FontSize;
			}
		}
		return FMCodexHandMicroDiagnostics::MinimumNameFontSize;
	}

	bool DoesHandMicroNameRequireEllipsis(
		const FText& Name,
		const FSlateFontInfo& BaseFont,
		const float HandMicroNameSafeWidth)
	{
		FSlateFontInfo MinimumFont = BaseFont;
		MinimumFont.Size = FMCodexHandMicroDiagnostics::MinimumNameFontSize;
		float MeasuredWidth = 0.0f;
		return !TryMeasureHandMicroName(Name, MinimumFont, MeasuredWidth)
			|| MeasuredWidth > HandMicroNameSafeWidth;
	}

	FLinearColor GetHandMicroRarityBaseColor(const FString& CanonicalLabel)
	{
		FColor SRGBColor = FColor::White;
		if (CanonicalLabel == TEXT("Regional") || CanonicalLabel == TEXT("Club"))
		{
			SRGBColor = FColor(0x1E, 0xFF, 0x00);
		}
		else if (CanonicalLabel == TEXT("National"))
		{
			SRGBColor = FColor(0x00, 0x70, 0xDD);
		}
		else if (CanonicalLabel == TEXT("Continental"))
		{
			SRGBColor = FColor(0xA3, 0x35, 0xEE);
		}
		else if (CanonicalLabel == TEXT("World Class")
			|| CanonicalLabel == TEXT("Pilot"))
		{
			SRGBColor = FColor(0xFF, 0x80, 0x00);
		}
		return FLinearColor::FromSRGBColor(SRGBColor);
	}

	TArray<FString> SplitStatSummary(const FString& Summary)
	{
		TArray<FString> Result;
		Summary.ParseIntoArray(Result, TEXT("|"), true);
		for (FString& Entry : Result)
		{
			Entry.TrimStartAndEndInline();
		}
		if (Result.IsEmpty())
		{
			Result.Add(TEXT("Attributes unavailable"));
		}
		return Result;
	}
}

void UFMCodexPlayerCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexPlayerCardWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

FReply UFMCodexPlayerCardWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bDeploymentDragEnabled)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UFMCodexPlayerCardWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	OutOperation = BeginDeploymentDrag();
	if (OutOperation == nullptr)
	{
		Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	}
}

void UFMCodexPlayerCardWidget::RefreshFromPresentation(
	const FFMCodexUMGCardViewModel& InPresentation,
	const EFMCodexPlayerCardPresentationMode InMode)
{
	Presentation = InPresentation;
	PresentationMode = InMode;
	RefreshVisuals();
}

void UFMCodexPlayerCardWidget::SetPresentationMode(
	const EFMCodexPlayerCardPresentationMode InMode)
{
	PresentationMode = InMode;
	RefreshVisuals();
}

void UFMCodexPlayerCardWidget::ConfigureDeploymentDrag(
	const FName CardId,
	const bool bGoalkeeper)
{
	DeploymentDragCardId = CardId;
	bDeploymentDragGoalkeeper = bGoalkeeper;
	bDeploymentDragEnabled = !CardId.IsNone();
}

void UFMCodexPlayerCardWidget::ClearDeploymentDrag()
{
	DeploymentDragCardId = NAME_None;
	bDeploymentDragGoalkeeper = false;
	bDeploymentDragEnabled = false;
}

bool UFMCodexPlayerCardWidget::IsDeploymentDragEnabled() const
{
	return bDeploymentDragEnabled;
}

FName UFMCodexPlayerCardWidget::GetDeploymentDragCardId() const
{
	return DeploymentDragCardId;
}

bool UFMCodexPlayerCardWidget::IsDeploymentDragGoalkeeper() const
{
	return bDeploymentDragGoalkeeper;
}

UFMCodexDeploymentDragDropOperation*
UFMCodexPlayerCardWidget::BeginDeploymentDrag()
{
	if (!bDeploymentDragEnabled || DeploymentDragCardId.IsNone())
	{
		return nullptr;
	}

	UFMCodexDeploymentDragDropOperation* Operation =
		NewObject<UFMCodexDeploymentDragDropOperation>(this);
	Operation->CardId = DeploymentDragCardId;
	Operation->bGoalkeeper = bDeploymentDragGoalkeeper;
	Operation->CardPresentation = Presentation;
	Operation->Pivot = EDragPivot::CenterCenter;
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		UFMCodexPlayerCardWidget* DragVisual =
			CreateWidget<UFMCodexPlayerCardWidget>(OwningPlayer, GetClass());
		if (DragVisual != nullptr)
		{
			DragVisual->RefreshFromPresentation(
				Presentation, EFMCodexPlayerCardPresentationMode::PitchMini);
			Operation->DefaultDragVisual = DragVisual;
		}
	}
	Operation->OnDrop.AddDynamic(
		this, &UFMCodexPlayerCardWidget::HandleDeploymentDragFinished);
	Operation->OnDragCancelled.AddDynamic(
		this, &UFMCodexPlayerCardWidget::HandleDeploymentDragFinished);
	OnDeploymentDragStarted.Broadcast(
		DeploymentDragCardId, bDeploymentDragGoalkeeper);
	return Operation;
}

void UFMCodexPlayerCardWidget::HandleDeploymentDragFinished(
	UDragDropOperation* Operation)
{
	OnDeploymentDragFinished.Broadcast();
}

const FFMCodexUMGCardViewModel&
UFMCodexPlayerCardWidget::GetPresentation() const
{
	return Presentation;
}

EFMCodexPlayerCardPresentationMode
UFMCodexPlayerCardWidget::GetPresentationMode() const
{
	return PresentationMode;
}

const FString& UFMCodexPlayerCardWidget::GetRenderedAttributeSummary() const
{
	return RenderedAttributeSummary;
}

int32 UFMCodexPlayerCardWidget::GetRenderedSkillCount() const
{
	return RenderedSkillTexts.Num();
}

int32 UFMCodexPlayerCardWidget::GetRenderedAttributeCount() const
{
	return RenderedAttributeTexts.Num();
}

int32 UFMCodexPlayerCardWidget::GetRenderedStatusBadgeCount() const
{
	return RenderedStatusTexts.Num();
}

FText UFMCodexPlayerCardWidget::GetRenderedIdentityText() const
{
	return IdentityText == nullptr ? FText::GetEmpty() : IdentityText->GetText();
}

FText UFMCodexPlayerCardWidget::GetRenderedTeamText() const
{
	return TeamText == nullptr ? FText::GetEmpty() : TeamText->GetText();
}

bool UFMCodexPlayerCardWidget::IsGoalkeeperVisualVariant() const
{
	return Presentation.bGoalkeeper;
}

FName UFMCodexPlayerCardWidget::GetResolvedArtIdentity() const
{
	return ResolvedArtIdentity;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedCardFrameTexture() const
{
	return ResolvedCardFrameTexture;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedPortraitTexture() const
{
	return ResolvedPortraitTexture;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedHandMicroPortraitTexture() const
{
	return ResolvedHandMicroPortraitTexture;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedRoleIconTexture() const
{
	return ResolvedRoleIconTexture;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedLongShotSkillIconTexture() const
{
	return ResolvedLongShotSkillIconTexture;
}

FVector2D UFMCodexPlayerCardWidget::GetConfiguredDimensions() const
{
	switch (PresentationMode)
	{
	case EFMCodexPlayerCardPresentationMode::HandMicro:
		return FVector2D(
			FMCodexHandMicroDiagnostics::CardWidth,
			FMCodexHandMicroDiagnostics::CardHeight);
	case EFMCodexPlayerCardPresentationMode::PitchMini:
		return FVector2D(136.0f, 140.0f);
	case EFMCodexPlayerCardPresentationMode::PitchCompact:
		return FVector2D(148.0f, 208.0f);
	default:
		return FVector2D(240.0f, 360.0f);
	}
}

void UFMCodexPlayerCardWidget::BuildWidgetTree()
{
	using namespace FMCodexPlayerCardWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	CardBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PlayerCardBounds"));
	CardBounds->SetClipping(EWidgetClipping::ClipToBounds);
	WidgetTree->RootWidget = CardBounds;

	CardFrame = MakeRegion(*WidgetTree, TEXT("PlayerCardFrame"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::CardFrame),
		FFMCodexPlayerUIStyle::Get().GetSectionPadding());
	CardBounds->AddChild(CardFrame);
	UOverlay* FrameAssetHook = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("CardFrameAssetHook"));
	FrameAssetHook->SetClipping(EWidgetClipping::ClipToBounds);
	CardFrame->AddChild(FrameAssetHook);
	CardFrameImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("CardFrameAssetImage"));
	CardFrameImage->SetVisibility(ESlateVisibility::Collapsed);
	CardFrameImage->SetClipping(EWidgetClipping::ClipToBounds);
	FrameAssetHook->AddChildToOverlay(CardFrameImage);
	CardFrameFallbackSurface = MakeRegion(
		*WidgetTree, TEXT("CardFrameFallbackSurface"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelInset),
		FMargin(0.0f));
	FrameAssetHook->AddChildToOverlay(CardFrameFallbackSurface);

	HandMicroVisualSystem = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroVisualSystem"));
	HandMicroVisualSystem->SetClipping(EWidgetClipping::ClipToBounds);
	UBorder* HandSkinBase = MakeRegion(*WidgetTree,
		TEXT("HandMicroSkinBackground"),
		FLinearColor::FromSRGBColor(FColor(0x0C, 0x23, 0x30)),
		FMargin(0.0f));
	if (UOverlaySlot* SkinBaseSlot =
		HandMicroVisualSystem->AddChildToOverlay(HandSkinBase))
	{
		SkinBaseSlot->SetHorizontalAlignment(HAlign_Fill);
		SkinBaseSlot->SetVerticalAlignment(VAlign_Fill);
	}

	HandMicroContent = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("HandMicroContent"));
	HandMicroContent->SetClipping(EWidgetClipping::ClipToBounds);
	USizeBox* HandPortraitCellBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroPortraitCellBounds"));
	HandPortraitCellBounds->SetWidthOverride(
		FMCodexHandMicroDiagnostics::PortraitWidth);
	HandPortraitCellBounds->SetHeightOverride(
		FMCodexHandMicroDiagnostics::CardHeight);
	HandPortraitCellBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UOverlay* HandPortraitCell = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroPortraitCell"));
	USizeBox* HandPortraitBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroPortraitBounds"));
	HandPortraitBounds->SetWidthOverride(
		FMCodexHandMicroDiagnostics::PortraitWidth);
	HandPortraitBounds->SetHeightOverride(
		FMCodexHandMicroDiagnostics::PortraitImageHeight);
	HandPortraitBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UOverlay* HandPortraitLayer = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroPortraitLayer"));
	HandMicroPortraitFallback = MakeRegion(*WidgetTree,
		TEXT("HandMicroPortraitFallback"),
		FLinearColor::FromSRGBColor(FColor(0x0C, 0x23, 0x30)),
		FMargin(0.0f));
	if (UOverlaySlot* FallbackSlot =
		HandPortraitLayer->AddChildToOverlay(HandMicroPortraitFallback))
	{
		FallbackSlot->SetHorizontalAlignment(HAlign_Fill);
		FallbackSlot->SetVerticalAlignment(VAlign_Fill);
	}
	HandMicroPortraitImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("HandMicroFaceSafePortrait"));
	HandMicroPortraitImage->SetClipping(EWidgetClipping::ClipToBounds);
	if (UOverlaySlot* PortraitSlot =
		HandPortraitLayer->AddChildToOverlay(HandMicroPortraitImage))
	{
		PortraitSlot->SetHorizontalAlignment(HAlign_Fill);
		PortraitSlot->SetVerticalAlignment(VAlign_Fill);
	}
	UBorder* HandPortraitAtmosphere = MakeRegion(*WidgetTree,
		TEXT("HandMicroPortraitAtmosphere"),
		FLinearColor::Transparent, FMargin(0.0f));
	if (UOverlaySlot* AtmosphereSlot =
		HandPortraitLayer->AddChildToOverlay(HandPortraitAtmosphere))
	{
		AtmosphereSlot->SetHorizontalAlignment(HAlign_Fill);
		AtmosphereSlot->SetVerticalAlignment(VAlign_Fill);
	}
	HandPortraitBounds->AddChild(HandPortraitLayer);
	if (UOverlaySlot* PortraitImageSlot =
		HandPortraitCell->AddChildToOverlay(HandPortraitBounds))
	{
		PortraitImageSlot->SetHorizontalAlignment(HAlign_Center);
		PortraitImageSlot->SetVerticalAlignment(VAlign_Center);
	}
	HandPortraitCellBounds->AddChild(HandPortraitCell);
	HandMicroContent->AddChildToHorizontalBox(HandPortraitCellBounds);
	HandMicroTextHierarchy = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HandMicroTextHierarchy"));
	HandMicroIdentityText = MakeText(
		*WidgetTree, TEXT("HandMicroPlayerName"));
	ConfigureHandMicroName(*HandMicroIdentityText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*HandMicroIdentityText, EFMCodexPlayerUITextRole::Identity);
	FSlateFontInfo HandNameFont = HandMicroIdentityText->GetFont();
	HandNameFont.Size = 22;
	HandNameFont.TypefaceFontName = TEXT("Medium");
	HandNameFont.LetterSpacing = 0;
	HandMicroIdentityText->SetFont(HandNameFont);
	HandMicroIdentityText->SetColorAndOpacity(
		FSlateColor(FLinearColor::FromSRGBColor(
			FColor(0xF2, 0xF6, 0xF8))));
	HandMicroIdentityText->SetJustification(ETextJustify::Left);
	HandMicroTextHierarchy->AddChildToVerticalBox(HandMicroIdentityText);
	UHorizontalBox* HandMeta = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("HandMicroPositionLine"));
	HandMicroRoleText = MakeText(*WidgetTree, TEXT("HandMicroPosition"));
	ConfigureBoundedSingleLine(*HandMicroRoleText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*HandMicroRoleText, EFMCodexPlayerUITextRole::Secondary);
	FSlateFontInfo HandRoleFont = HandMicroRoleText->GetFont();
	HandRoleFont.Size = 14;
	HandRoleFont.TypefaceFontName = TEXT("Medium");
	HandRoleFont.LetterSpacing = 0;
	HandMicroRoleText->SetFont(HandRoleFont);
	HandMicroRoleText->SetColorAndOpacity(
		FSlateColor(FLinearColor::FromSRGBColor(
			FColor(0xC6, 0xD3, 0xDA))));
	HandMicroRoleText->SetJustification(ETextJustify::Left);
	HandMeta->AddChildToHorizontalBox(HandMicroRoleText);
	if (UVerticalBoxSlot* HandMetaSlot =
		HandMicroTextHierarchy->AddChildToVerticalBox(HandMeta))
	{
		HandMetaSlot->SetPadding(FMargin(0.0f));
	}
	UBorder* HandIdentitySurface = MakeRegion(*WidgetTree,
		TEXT("HandMicroIdentitySurface"),
		FLinearColor::FromSRGBColor(FColor(0x1C, 0x35, 0x42)),
		FMargin(0.0f));
	UOverlay* HandIdentityLayers = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroIdentityMaterialLayers"));
	HandIdentitySurface->AddChild(HandIdentityLayers);
	USizeBox* HandIdentityTopLineBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroIdentityTechLineBounds"));
	HandIdentityTopLineBounds->SetHeightOverride(1.0f);
	UBorder* HandIdentityTopLine = MakeRegion(*WidgetTree,
		TEXT("HandMicroIdentityTechLine"),
		FLinearColor(0.22f, 0.42f, 0.50f, 0.16f), FMargin(0.0f));
	HandIdentityTopLineBounds->AddChild(HandIdentityTopLine);
	if (UOverlaySlot* IdentityLineSlot =
		HandIdentityLayers->AddChildToOverlay(HandIdentityTopLineBounds))
	{
		IdentityLineSlot->SetHorizontalAlignment(HAlign_Fill);
		IdentityLineSlot->SetVerticalAlignment(VAlign_Top);
		IdentityLineSlot->SetPadding(FMargin(7.0f, 4.0f, 7.0f, 0.0f));
	}
	if (UOverlaySlot* IdentityTextSlot =
		HandIdentityLayers->AddChildToOverlay(HandMicroTextHierarchy))
	{
		IdentityTextSlot->SetHorizontalAlignment(HAlign_Fill);
		IdentityTextSlot->SetVerticalAlignment(VAlign_Center);
		IdentityTextSlot->SetPadding(FMargin(10.0f, 7.0f, 6.0f, 7.0f));
	}
	HandMicroIdentityBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroIdentityBounds"));
	HandMicroIdentityBounds->SetWidthOverride(88.0f);
	HandMicroIdentityBounds->SetHeightOverride(64.0f);
	HandMicroIdentityBounds->AddChild(HandIdentitySurface);
	if (UHorizontalBoxSlot* HandTextSlot =
		HandMicroContent->AddChildToHorizontalBox(HandMicroIdentityBounds))
	{
		HandTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		HandTextSlot->SetHorizontalAlignment(HAlign_Fill);
		HandTextSlot->SetVerticalAlignment(VAlign_Fill);
	}
	USizeBox* HandDividerBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroIdentityDividerBounds"));
	HandDividerBounds->SetWidthOverride(1.0f);
	HandDividerBounds->SetHeightOverride(64.0f);
	UBorder* HandDivider = MakeRegion(*WidgetTree,
		TEXT("HandMicroIdentityDivider"),
		FLinearColor(0.35f, 0.43f, 0.50f, 0.26f), FMargin(0.0f));
	HandDividerBounds->AddChild(HandDivider);
	if (UOverlaySlot* DividerSlot =
		HandIdentityLayers->AddChildToOverlay(HandDividerBounds))
	{
		DividerSlot->SetHorizontalAlignment(HAlign_Left);
		DividerSlot->SetVerticalAlignment(VAlign_Fill);
	}
	USizeBox* HandRarityBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HandMicroRarityAccentBounds"));
	HandRarityBounds->SetWidthOverride(4.0f);
	HandRarityBounds->SetHeightOverride(64.0f);
	HandMicroRarityAccent = MakeRegion(*WidgetTree,
		TEXT("HandMicroRarityAccent"), FLinearColor::White, FMargin(0.0f));
	HandRarityBounds->AddChild(HandMicroRarityAccent);
	if (UHorizontalBoxSlot* RarityBoundsSlot =
		HandMicroContent->AddChildToHorizontalBox(HandRarityBounds))
	{
		RarityBoundsSlot->SetHorizontalAlignment(HAlign_Fill);
		RarityBoundsSlot->SetVerticalAlignment(VAlign_Center);
	}
	HandMicroVisualSystem->AddChildToOverlay(HandMicroContent);

	UOverlay* HandSkinChrome = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("HandMicroSkinChrome"));
	auto AddSkinRail = [this, HandSkinChrome](
		const FName BoundsName, const FName RailName,
		const float Width, const float Height,
		const EHorizontalAlignment HorizontalAlignment,
		const EVerticalAlignment VerticalAlignment,
		const FMargin& SlotPadding, const FLinearColor& Color)
	{
		USizeBox* RailBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), BoundsName);
		if (Width > 0.0f)
		{
			RailBounds->SetWidthOverride(Width);
		}
		if (Height > 0.0f)
		{
			RailBounds->SetHeightOverride(Height);
		}
		UBorder* Rail = MakeRegion(
			*WidgetTree, RailName, Color, FMargin(0.0f));
		RailBounds->AddChild(Rail);
		if (UOverlaySlot* RailSlot =
			HandSkinChrome->AddChildToOverlay(RailBounds))
		{
			RailSlot->SetHorizontalAlignment(HorizontalAlignment);
			RailSlot->SetVerticalAlignment(VerticalAlignment);
			RailSlot->SetPadding(SlotPadding);
		}
	};
	const FLinearColor ChromeColor(0.38f, 0.48f, 0.54f, 0.34f);
	AddSkinRail(TEXT("HandMicroSkinTopRailBounds"),
		TEXT("HandMicroSkinTopRail"), 0.0f, 1.0f,
		HAlign_Fill, VAlign_Top, FMargin(0.0f), ChromeColor);
	AddSkinRail(TEXT("HandMicroSkinBottomRailBounds"),
		TEXT("HandMicroSkinBottomRail"), 0.0f, 1.0f,
		HAlign_Fill, VAlign_Bottom, FMargin(0.0f), ChromeColor);
	AddSkinRail(TEXT("HandMicroSkinLeftRailBounds"),
		TEXT("HandMicroSkinLeftRail"), 1.0f, 0.0f,
		HAlign_Left, VAlign_Fill, FMargin(0.0f), ChromeColor);
	AddSkinRail(TEXT("HandMicroSkinRightRailBounds"),
		TEXT("HandMicroSkinRightRail"), 1.0f, 0.0f,
		HAlign_Right, VAlign_Fill, FMargin(0.0f), ChromeColor);
	AddSkinRail(TEXT("HandMicroSkinTopAccentBounds"),
		TEXT("HandMicroSkinTopAccent"), 18.0f, 1.0f,
		HAlign_Right, VAlign_Top, FMargin(0.0f, 3.0f, 8.0f, 0.0f),
		FLinearColor(0.30f, 0.55f, 0.62f, 0.18f));
	if (UOverlaySlot* ChromeSlot =
		HandMicroVisualSystem->AddChildToOverlay(HandSkinChrome))
	{
		ChromeSlot->SetHorizontalAlignment(HAlign_Fill);
		ChromeSlot->SetVerticalAlignment(VAlign_Fill);
	}
	FrameAssetHook->AddChildToOverlay(HandMicroVisualSystem);

	PitchMiniContent = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PitchMiniContent"));
	PitchMiniContent->SetClipping(EWidgetClipping::ClipToBounds);
	USizeBox* PitchPortraitBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PitchMiniPortraitBounds"));
	PitchPortraitBounds->SetHeightOverride(94.0f);
	PitchPortraitBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UOverlay* PitchPortraitLayer = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("PitchMiniPortraitLayer"));
	PitchMiniPortraitFallback = MakeRegion(*WidgetTree,
		TEXT("PitchMiniPortraitFallback"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::NeutralAccent), FMargin(0.0f));
	PitchPortraitLayer->AddChildToOverlay(PitchMiniPortraitFallback);
	PitchMiniPortraitImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("PitchMiniPortraitImage"));
	PitchMiniPortraitImage->SetClipping(EWidgetClipping::ClipToBounds);
	PitchPortraitLayer->AddChildToOverlay(PitchMiniPortraitImage);
	PitchPortraitBounds->AddChild(PitchPortraitLayer);
	PitchMiniContent->AddChildToVerticalBox(PitchPortraitBounds);
	PitchMiniIdentityText = MakeText(
		*WidgetTree, TEXT("PitchMiniPlayerName"));
	PitchMiniRoleText = MakeText(*WidgetTree, TEXT("PitchMiniPosition"));
	ConfigureBoundedSingleLine(*PitchMiniIdentityText);
	ConfigureBoundedSingleLine(*PitchMiniRoleText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniIdentityText, EFMCodexPlayerUITextRole::Kicker);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniRoleText, EFMCodexPlayerUITextRole::Secondary);
	PitchMiniContent->AddChildToVerticalBox(PitchMiniIdentityText);
	UHorizontalBox* PitchMeta = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("PitchMiniPositionLine"));
	PitchMeta->AddChildToHorizontalBox(PitchMiniRoleText);
	PitchMiniContent->AddChildToVerticalBox(PitchMeta);
	USizeBox* PitchRarityBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PitchMiniRarityAccentBounds"));
	PitchRarityBounds->SetHeightOverride(5.0f);
	PitchMiniRarityAccent = MakeRegion(*WidgetTree,
		TEXT("PitchMiniRarityAccent"), FLinearColor::White, FMargin(0.0f));
	PitchRarityBounds->AddChild(PitchMiniRarityAccent);
	PitchMiniContent->AddChildToVerticalBox(PitchRarityBounds);
	FrameAssetHook->AddChildToOverlay(PitchMiniContent);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PlayerCardVisualHierarchy"));
	DetailedContentLayer = MakeRegion(
		*WidgetTree, TEXT("CardContentReadabilityLayer"),
		FLinearColor(0.005f, 0.012f, 0.02f, 0.62f), FMargin(7.0f));
	DetailedContentLayer->SetClipping(EWidgetClipping::ClipToBounds);
	DetailedContentLayer->AddChild(Body);
	FrameAssetHook->AddChildToOverlay(DetailedContentLayer);

	HeaderRegion = MakeRegion(
		*WidgetTree, TEXT("RoleRarityHeaderRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelRaised),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RoleRarityHeader"));
	HeaderRegion->AddChild(Header);
	RoleIconHook = MakeRegion(*WidgetTree, TEXT("RoleIconHook"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::NeutralAccent),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	RoleText = MakeText(*WidgetTree, TEXT("CardRole"));
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*RoleText, EFMCodexPlayerUITextRole::SectionHeading);
	UHorizontalBox* RoleContent = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RoleIconAndText"));
	USizeBox* RoleIconBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("RoleIconAssetBounds"));
	RoleIconBounds->SetWidthOverride(22.0f);
	RoleIconBounds->SetHeightOverride(22.0f);
	RoleIconImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("RoleIconAssetImage"));
	RoleIconImage->SetVisibility(ESlateVisibility::Collapsed);
	RoleIconBounds->AddChild(RoleIconImage);
	RoleContent->AddChildToHorizontalBox(RoleIconBounds);
	RoleContent->AddChildToHorizontalBox(RoleText);
	RoleIconHook->AddChild(RoleContent);
	Header->AddChildToHorizontalBox(RoleIconHook);
	RarityText = MakeText(*WidgetTree, TEXT("CardRarity"));
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*RarityText, EFMCodexPlayerUITextRole::Secondary);
	Header->AddChildToHorizontalBox(RarityText);
	Body->AddChildToVerticalBox(HeaderRegion);

	PortraitBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PortraitAssetBounds"));
	PortraitPresentationRegion = MakeRegion(
		*WidgetTree, TEXT("PortraitPresentationRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelRaised),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UBorder* PortraitAssetHook = MakeRegion(
		*WidgetTree, TEXT("PortraitAssetHook"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelInset),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UOverlay* PortraitLayer = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("PortraitAssetLayer"));
	UScaleBox* PortraitScale = WidgetTree->ConstructWidget<UScaleBox>(
		UScaleBox::StaticClass(), TEXT("PortraitAspectScale"));
	PortraitScale->SetStretch(EStretch::ScaleToFill);
	PortraitScale->SetClipping(EWidgetClipping::ClipToBounds);
	PortraitImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("PortraitAssetImage"));
	PortraitImage->SetVisibility(ESlateVisibility::Collapsed);
	PortraitScale->AddChild(PortraitImage);
	PortraitLayer->AddChildToOverlay(PortraitScale);
	PortraitPlaceholderText = MakeText(
		*WidgetTree, TEXT("PortraitPlaceholderText"));
	PortraitPlaceholderText->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PortraitPlaceholderText, EFMCodexPlayerUITextRole::Secondary);
	PortraitLayer->AddChildToOverlay(PortraitPlaceholderText);
	PortraitAssetHook->AddChild(PortraitLayer);
	PortraitPresentationRegion->AddChild(PortraitAssetHook);
	PortraitBounds->AddChild(PortraitPresentationRegion);
	Body->AddChildToVerticalBox(PortraitBounds);

	IdentityRegion = MakeRegion(
		*WidgetTree, TEXT("CardIdentityRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelRaised),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UVerticalBox* IdentityBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CardIdentityBody"));
	IdentityText = MakeText(*WidgetTree, TEXT("CardIdentity"));
	IdentityText->SetAutoWrapText(false);
	IdentityText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	TeamText = MakeText(*WidgetTree, TEXT("CardTeam"));
	OwnerText = MakeText(*WidgetTree, TEXT("CardOwner"));
	DeveloperReferenceText = MakeText(
		*WidgetTree, TEXT("CardDeveloperReference"));
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*IdentityText, EFMCodexPlayerUITextRole::Identity);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*TeamText, EFMCodexPlayerUITextRole::Secondary);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*OwnerText, EFMCodexPlayerUITextRole::Kicker);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*DeveloperReferenceText, EFMCodexPlayerUITextRole::Secondary);
	IdentityBody->AddChildToVerticalBox(IdentityText);
	IdentityBody->AddChildToVerticalBox(TeamText);
	IdentityBody->AddChildToVerticalBox(OwnerText);
	IdentityBody->AddChildToVerticalBox(DeveloperReferenceText);
	IdentityRegion->AddChild(IdentityBody);
	Body->AddChildToVerticalBox(IdentityRegion);

	SkillRegion = MakeRegion(
		*WidgetTree, TEXT("SkillPresentationRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelInset),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UVerticalBox* SkillBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SkillPresentationBody"));
	UTextBlock* SkillTitle = MakeText(
		*WidgetTree, TEXT("SkillRegionTitle"));
	SkillTitle->SetText(FFMCodexPlayerUIPresentationText::SkillsHeading());
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*SkillTitle, EFMCodexPlayerUITextRole::SectionHeading);
	SkillBody->AddChildToVerticalBox(SkillTitle);
	SkillList = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SkillIdentityList"));
	SkillBody->AddChildToVerticalBox(SkillList);
	SkillRegion->AddChild(SkillBody);
	Body->AddChildToVerticalBox(SkillRegion);

	AttributeRegion = MakeRegion(
		*WidgetTree, TEXT("AttributePresentationRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelInset),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UVerticalBox* AttributeBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("AttributePresentationBody"));
	UTextBlock* AttributeTitle = MakeText(
		*WidgetTree, TEXT("AttributeRegionTitle"));
	AttributeTitle->SetText(
		FFMCodexPlayerUIPresentationText::AttributesHeading());
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*AttributeTitle, EFMCodexPlayerUITextRole::SectionHeading);
	AttributeBody->AddChildToVerticalBox(AttributeTitle);
	AttributeGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(
		UUniformGridPanel::StaticClass(), TEXT("StructuredAttributeGrid"));
	AttributeBody->AddChildToVerticalBox(AttributeGrid);
	AttributeRegion->AddChild(AttributeBody);
	Body->AddChildToVerticalBox(AttributeRegion);

	StatusRegion = MakeRegion(
		*WidgetTree, TEXT("StatusBadgePresentationRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelRaised),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	StatusBadgeBox = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("StatusBadgeList"));
	StatusRegion->AddChild(StatusBadgeBox);
	Body->AddChildToVerticalBox(StatusRegion);
}

void UFMCodexPlayerCardWidget::RefreshVisuals()
{
	if (IdentityText == nullptr)
	{
		return;
	}

	const bool bHandMicro = PresentationMode
		== EFMCodexPlayerCardPresentationMode::HandMicro;
	const bool bPitchMini = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchMini;
	const bool bLegacyCompact = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchCompact;
	const bool bDetailed = PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice;
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	const FVector2D Dimensions = GetConfiguredDimensions();
	const float HandMicroCardHeight = bHandMicro
		? Dimensions.Y : FMCodexHandMicroDiagnostics::CardHeight;
	const float HandMicroNameSafeWidth =
		FMCodexHandMicroDiagnostics::NameSafeWidth;
	CardBounds->SetWidthOverride(Dimensions.X);
	CardBounds->SetHeightOverride(Dimensions.Y);
	HandMicroIdentityBounds->SetWidthOverride(
		FMCodexHandMicroDiagnostics::IdentityWidth);
	HandMicroIdentityBounds->SetHeightOverride(HandMicroCardHeight);
	if (USizeBox* PortraitCellBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("HandMicroPortraitCellBounds"))))
	{
		PortraitCellBounds->SetHeightOverride(HandMicroCardHeight);
	}
	if (USizeBox* PortraitImageBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("HandMicroPortraitBounds"))))
	{
		PortraitImageBounds->SetWidthOverride(
			FMCodexHandMicroDiagnostics::PortraitWidth);
		PortraitImageBounds->SetHeightOverride(
			FMCodexHandMicroDiagnostics::PortraitImageHeight);
	}
	if (USizeBox* DividerBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("HandMicroIdentityDividerBounds"))))
	{
		DividerBounds->SetHeightOverride(HandMicroCardHeight);
	}
	if (USizeBox* RarityBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("HandMicroRarityAccentBounds"))))
	{
		RarityBounds->SetHeightOverride(HandMicroCardHeight);
	}
	if (UOverlaySlot* IdentityTextSlot = HandMicroTextHierarchy != nullptr
		? Cast<UOverlaySlot>(HandMicroTextHierarchy->Slot) : nullptr)
	{
		IdentityTextSlot->SetPadding(FMargin(
			FMCodexHandMicroDiagnostics::NamePaddingLeft,
			7.0f,
			FMCodexHandMicroDiagnostics::NamePaddingRight,
			7.0f));
	}
	CardFrame->SetPadding(bHandMicro ? FMargin(0.0f)
		: bPitchMini ? FMargin(3.0f) : Style.GetSectionPadding());
	HandMicroVisualSystem->SetVisibility(bHandMicro
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PitchMiniContent->SetVisibility(bPitchMini
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	DetailedContentLayer->SetVisibility(bHandMicro || bPitchMini
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PortraitBounds->SetHeightOverride(
		bHandMicro ? 18.0f : bPitchMini ? 52.0f : bLegacyCompact ? 32.0f : 82.0f);
	SkillRegion->SetVisibility(bDetailed || bLegacyCompact
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	AttributeRegion->SetVisibility(bDetailed || bLegacyCompact
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	StatusRegion->SetVisibility(bDetailed || bLegacyCompact
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	const FLinearColor RarityAccent = Style.GetRarityAccentColor(
		Presentation.RarityLabel);
	FLinearColor HandRarityColor =
		FMCodexPlayerCardWidget::GetHandMicroRarityBaseColor(
		Presentation.RarityLabel);
	HandRarityColor.A = 0.45f;
	const FLinearColor BaseFrame = Style.GetColor(Presentation.bGoalkeeper
		? EFMCodexPlayerUIColorRole::GoalkeeperCardFrame
		: EFMCodexPlayerUIColorRole::CardFrame);
	CardFrame->SetBrushColor(bHandMicro
		? Style.GetColor(EFMCodexPlayerUIColorRole::CardFrame)
		: bPitchMini
			? FLinearColor::LerpUsingHSV(BaseFrame, RarityAccent, 0.10f)
			: BaseFrame);
	RoleIconHook->SetBrushColor(Presentation.bGoalkeeper
		? Style.GetColor(EFMCodexPlayerUIColorRole::GoalkeeperCardFrame)
		: Style.GetPlayerAccentColor(Presentation.OwnerLabel));

	IdentityText->SetText(FFMCodexPlayerUIPresentationText::PlayerName(
		Presentation.CardId, Presentation.IdentityLabel));
	const FText PlayerName = FFMCodexPlayerUIPresentationText::CompactPlayerName(
		Presentation.CardId, Presentation.IdentityLabel);
	const FText PrimaryHandMicroPlayerName =
		FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
			Presentation.CardId, Presentation.IdentityLabel);
	FSlateFontInfo HandMicroNameFont = HandMicroIdentityText->GetFont();
	HandMicroNameFont.Size = FMCodexPlayerCardWidget::GetHandMicroNameFontSize(
		PrimaryHandMicroPlayerName, HandMicroNameFont, HandMicroNameSafeWidth,
		FMCodexHandMicroDiagnostics::StandardNameFontSize);
	HandMicroIdentityText->SetText(PrimaryHandMicroPlayerName);
	HandMicroIdentityText->SetFont(HandMicroNameFont);
	HandMicroIdentityText->SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
	PitchMiniIdentityText->SetText(PlayerName);
	TeamText->SetText(FFMCodexPlayerUIPresentationText::TeamName(
		Presentation.CardId));
	TeamText->SetVisibility(TeamText->GetText().IsEmpty()
		|| bHandMicro || bPitchMini
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	OwnerText->SetText(FFMCodexPlayerUIPresentationText::Owner(
		Presentation.OwnerLabel));
	OwnerText->SetVisibility(bHandMicro || bPitchMini
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	RoleText->SetText(FFMCodexPlayerUIPresentationText::Role(
		Presentation.RoleLabel));
	const FText CompactRole = FFMCodexPlayerUIPresentationText::CompactRole(
		Presentation.RoleLabel);
	HandMicroRoleText->SetText(
		FFMCodexPlayerUIPresentationText::HandMicroCompactRole(
			Presentation.RoleLabel));
	PitchMiniRoleText->SetText(CompactRole);
	RarityText->SetText(FText::Format(FText::FromString(TEXT("  {0}")),
		FFMCodexPlayerUIPresentationText::Rarity(Presentation.RarityLabel)));
	HandMicroRarityAccent->SetBrushColor(HandRarityColor);
	PitchMiniRarityAccent->SetBrushColor(RarityAccent);
	PortraitPlaceholderText->SetText(
		FFMCodexPlayerUIPresentationText::PortraitPlaceholder());
	RefreshPresentationArt();
	DeveloperReferenceText->SetText(FText::FromString(
		Presentation.DeveloperReferenceLabel));
	DeveloperReferenceText->SetVisibility(!bDetailed
		|| ResolvedArtIdentity
			== FFMCodexPlayerUIAssetReferences::Get().GetGoldenSampleArtIdentity()
		|| ResolvedArtIdentity.ToString().StartsWith(TEXT("PrototypeTeam."))
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	RefreshSkills();
	RefreshAttributes();
	RefreshStatusBadges();
}

void UFMCodexPlayerCardWidget::RefreshPresentationArt()
{
	const FFMCodexPlayerUICardArtReferences Art =
		FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(
			Presentation.CardId);
	ResolvedArtIdentity = Art.ArtIdentity;
	ResolvedCardFrameTexture = Art.CardFrame.IsNull()
		? nullptr : Art.CardFrame.LoadSynchronous();
	ResolvedPortraitTexture = Art.Portrait.IsNull()
		? nullptr : Art.Portrait.LoadSynchronous();
	ResolvedHandMicroPortraitTexture = Art.HandMicroPortrait.IsNull()
		? ResolvedPortraitTexture.Get() : Art.HandMicroPortrait.LoadSynchronous();
	ResolvedRoleIconTexture = Art.RoleIcon.IsNull()
		? nullptr : Art.RoleIcon.LoadSynchronous();
	ResolvedLongShotSkillIconTexture = Art.LongShotSkillIcon.IsNull()
		? nullptr : Art.LongShotSkillIcon.LoadSynchronous();

	if (!Art.CardFrame.IsNull() && ResolvedCardFrameTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional card-frame asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.CardFrame.ToSoftObjectPath().ToString());
	}
	if (!Art.Portrait.IsNull() && ResolvedPortraitTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional portrait asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.Portrait.ToSoftObjectPath().ToString());
	}
	if (!Art.HandMicroPortrait.IsNull()
		&& ResolvedHandMicroPortraitTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional Hand Micro portrait asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.HandMicroPortrait.ToSoftObjectPath().ToString());
		ResolvedHandMicroPortraitTexture = ResolvedPortraitTexture;
	}
	if (!Art.RoleIcon.IsNull() && ResolvedRoleIconTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional role-icon asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.RoleIcon.ToSoftObjectPath().ToString());
	}
	if (!Art.LongShotSkillIcon.IsNull()
		&& ResolvedLongShotSkillIconTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional skill-icon asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.LongShotSkillIcon.ToSoftObjectPath().ToString());
	}

	const bool bHasFrame = ResolvedCardFrameTexture != nullptr;
	const bool bUseFrameTexture = bHasFrame
		&& PresentationMode != EFMCodexPlayerCardPresentationMode::HandMicro;
	CardFrameImage->SetVisibility(bUseFrameTexture
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	CardFrameFallbackSurface->SetVisibility(bUseFrameTexture
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (bUseFrameTexture)
	{
		CardFrameImage->SetBrushFromTexture(
			ResolvedCardFrameTexture, false);
	}

	const bool bHasPortrait = ResolvedPortraitTexture != nullptr;
	const bool bHasHandMicroPortrait =
		ResolvedHandMicroPortraitTexture != nullptr;
	PortraitImage->SetVisibility(bHasPortrait
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PortraitPlaceholderText->SetVisibility(bHasPortrait
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (bHasPortrait)
	{
		PortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, true);
		PitchMiniPortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, true);
	}
	if (bHasHandMicroPortrait)
	{
		HandMicroPortraitImage->SetBrushFromTexture(
			ResolvedHandMicroPortraitTexture, false);
		FSlateBrush MicroFaceSafeBrush = HandMicroPortraitImage->GetBrush();
		MicroFaceSafeBrush.DrawAs = ESlateBrushDrawType::Image;
		const float MicroPortraitUVHeight = FMath::Clamp(
			Art.HandMicroPortraitUVHeight, 0.01f, 1.0f);
		const float MicroPortraitTop = FMath::Clamp(
			Art.HandMicroPortraitTop, 0.0f, 1.0f - MicroPortraitUVHeight);
		MicroFaceSafeBrush.SetUVRegion(FBox2f(
			FVector2f(0.0f, MicroPortraitTop),
			FVector2f(1.0f, MicroPortraitTop + MicroPortraitUVHeight)));
		HandMicroPortraitImage->SetBrush(MicroFaceSafeBrush);
	}
	HandMicroPortraitImage->SetVisibility(bHasHandMicroPortrait
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PitchMiniPortraitImage->SetVisibility(bHasPortrait
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	HandMicroPortraitFallback->SetVisibility(bHasHandMicroPortrait
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PitchMiniPortraitFallback->SetVisibility(bHasPortrait
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PortraitPlaceholderText->SetVisibility(bHasPortrait
		? ESlateVisibility::Collapsed
		: PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
			|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
				? ESlateVisibility::Collapsed
				: ESlateVisibility::HitTestInvisible);

	const bool bHasRoleIcon = ResolvedRoleIconTexture != nullptr;
	RoleIconImage->SetVisibility(bHasRoleIcon
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bHasRoleIcon)
	{
		RoleIconImage->SetBrushFromTexture(ResolvedRoleIconTexture, true);
	}
}

void UFMCodexPlayerCardWidget::RefreshSkills()
{
	using namespace FMCodexPlayerCardWidget;
	SkillList->ClearChildren();
	RenderedSkillTexts.Reset();
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
		|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini)
	{
		return;
	}
	const TArray<FString> Skills = Presentation.SkillLabels.IsEmpty()
		? TArray<FString>({ Presentation.SkillSummaryLabel.IsEmpty()
			? FString(TEXT("NO SKILL")) : Presentation.SkillSummaryLabel })
		: Presentation.SkillLabels;
	for (int32 Index = 0; Index < Skills.Num(); ++Index)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), FName(*FString::Printf(
				TEXT("SkillIdentityRow%d"), Index)));
		UBorder* IconHook = MakeRegion(*WidgetTree, FName(*FString::Printf(
			TEXT("SkillIconHook%d"), Index)),
			FFMCodexPlayerUIStyle::Get().GetColor(
				EFMCodexPlayerUIColorRole::SkillBadge), FMargin(3.0f));
		const bool bHasSkillIcon = Skills[Index] == TEXT("Long Shot")
			&& ResolvedLongShotSkillIconTexture != nullptr;
		USizeBox* IconBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), FName(*FString::Printf(
				TEXT("SkillIconAssetBounds%d"), Index)));
		IconBounds->SetWidthOverride(22.0f);
		IconBounds->SetHeightOverride(22.0f);
		UImage* IconImage = WidgetTree->ConstructWidget<UImage>(
			UImage::StaticClass(), FName(*FString::Printf(
				TEXT("SkillIconAssetImage%d"), Index)));
		IconImage->SetVisibility(bHasSkillIcon
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bHasSkillIcon)
		{
			IconImage->SetBrushFromTexture(ResolvedLongShotSkillIconTexture, true);
		}
		IconBounds->AddChild(IconImage);
		IconHook->AddChild(IconBounds);
		IconHook->SetVisibility(bHasSkillIcon
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Row->AddChildToHorizontalBox(IconHook);
		UTextBlock* SkillText = MakeText(*WidgetTree, FName(*FString::Printf(
			TEXT("SkillIdentity%d"), Index)));
		SkillText->SetText(FText::Format(FText::FromString(TEXT("  {0}")),
			FFMCodexPlayerUIPresentationText::Skill(Skills[Index])));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*SkillText, EFMCodexPlayerUITextRole::Body);
		Row->AddChildToHorizontalBox(SkillText);
		SkillList->AddChildToVerticalBox(Row);
		RenderedSkillTexts.Add(SkillText);
	}
}

void UFMCodexPlayerCardWidget::RefreshAttributes()
{
	using namespace FMCodexPlayerCardWidget;
	AttributeGrid->ClearChildren();
	RenderedAttributeTexts.Reset();
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
		|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini)
	{
		RenderedAttributeSummary.Reset();
		return;
	}
	RenderedAttributeSummary = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchCompact
			? Presentation.CompactAttributeSummary
			: Presentation.FullAttributeSummary;
	if (RenderedAttributeSummary.IsEmpty())
	{
		RenderedAttributeSummary = TEXT("Attributes unavailable");
	}
	const TArray<FString> Attributes = SplitStatSummary(
		RenderedAttributeSummary);
	for (int32 Index = 0; Index < Attributes.Num(); ++Index)
	{
		UBorder* StatCell = MakeRegion(*WidgetTree, FName(*FString::Printf(
			TEXT("AttributeCell%d"), Index)),
			FFMCodexPlayerUIStyle::Get().GetColor(
				EFMCodexPlayerUIColorRole::AttributeCell), FMargin(3.0f));
		UTextBlock* StatText = MakeText(*WidgetTree, FName(*FString::Printf(
			TEXT("AttributeValue%d"), Index)));
		StatText->SetText(
			FFMCodexPlayerUIPresentationText::Attribute(Attributes[Index]));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*StatText, EFMCodexPlayerUITextRole::Body);
		StatCell->AddChild(StatText);
		AttributeGrid->AddChildToUniformGrid(
			StatCell, Index / 2, Index % 2);
		RenderedAttributeTexts.Add(StatText);
	}
}

void UFMCodexPlayerCardWidget::RefreshStatusBadges()
{
	using namespace FMCodexPlayerCardWidget;
	StatusBadgeBox->ClearChildren();
	RenderedStatusTexts.Reset();
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
		|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini)
	{
		return;
	}
	const TArray<FString> Statuses = Presentation.StatusLabels.IsEmpty()
		? TArray<FString>({ Presentation.StatusSummaryLabel.IsEmpty()
			? FString(TEXT("UNAVAILABLE")) : Presentation.StatusSummaryLabel })
		: Presentation.StatusLabels;
	for (int32 Index = 0; Index < Statuses.Num(); ++Index)
	{
		UBorder* Badge = MakeRegion(*WidgetTree, FName(*FString::Printf(
			TEXT("StatusBadge%d"), Index)),
			FFMCodexPlayerUIStyle::Get().GetStatusBadgeColor(Statuses[Index]),
			FMargin(5.0f, 3.0f));
		UTextBlock* BadgeText = MakeText(*WidgetTree, FName(*FString::Printf(
			TEXT("StatusBadgeLabel%d"), Index)));
		BadgeText->SetText(
			FFMCodexPlayerUIPresentationText::Status(Statuses[Index]));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*BadgeText, EFMCodexPlayerUITextRole::Kicker);
		Badge->AddChild(BadgeText);
		StatusBadgeBox->AddChildToWrapBox(Badge);
		RenderedStatusTexts.Add(BadgeText);
	}
}
