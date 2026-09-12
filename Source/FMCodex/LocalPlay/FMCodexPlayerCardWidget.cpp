#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerCardSurface.h"
#include "FMCodexFullCardSurface.h"
#include "Rendering/DrawElements.h"

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
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Rendering/SlateRenderer.h"
#include "Brushes/SlateRoundedBoxBrush.h"

DEFINE_LOG_CATEGORY_STATIC(LogFMCodexPlayerCardArt, Log, All);

namespace FMCodexPlayerCardWidget
{
	constexpr float FullCardWidth = 360.0f;
	constexpr float FullCardHeight = 540.0f;
	constexpr float FullCardHeroHeight = 320.0f;
	constexpr float FullCardBiographyWidth = 100.0f;
	constexpr float FullCardPortraitLeft = 0.0f;
	constexpr float FullCardPortraitTop = 0.045f;
	constexpr float FullCardPortraitRight = 1.0f;
	constexpr float FullCardPortraitBottom = 0.658f;
	constexpr float HandMicroDragProxyScale = 1.10f;
	constexpr float PitchMiniInteriorWidth = 130.0f;
	constexpr float PitchMiniInteriorHeight = 134.0f;
	constexpr float PitchMiniPortraitHeight = 112.0f;
	constexpr float PitchMiniHeroZoom = 1.08f;
	constexpr float PitchMiniHeroFocalX = 0.50f;
	constexpr float PitchMiniHeroFocalY = 0.278f;
	constexpr float PitchMiniHeroFocalFrameY = 0.42f;
	constexpr float PitchMiniIdentityHeight = 22.0f;
	constexpr float PitchMiniTacticalMatchInset = 2.0f;
	constexpr float PitchMiniTacticalMatchGlowThickness = 3.0f;
	constexpr float PitchMiniTacticalMatchStrokeThickness = 1.5f;
	constexpr float PitchMiniTacticalMatchPipDiameter = 4.0f;
	constexpr float PitchMiniTacticalMatchPipGap = 3.0f;
	constexpr float PitchMiniTacticalMatchPipLeftInset = 9.0f;
	constexpr float PitchMiniTacticalMatchPipTopInset = 8.0f;
	constexpr int32 PitchMiniNameMaximumFontSize = 15;
	constexpr int32 PitchMiniNameMinimumFontSize = 12;
	constexpr int32 PitchMiniRoleFontSize = 10;
	static_assert(PitchMiniPortraitHeight + PitchMiniIdentityHeight
		== PitchMiniInteriorHeight,
		"Pitch Mini fixed interior regions must total 134 px.");

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
		UBorder* Result = Name == TEXT("HandMicroIdentitySurface")
			? Tree.ConstructWidget<UFMCodexPlayerCardSurface>(UFMCodexPlayerCardSurface::StaticClass(), Name)
			: Tree.ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		Result->SetPadding(Padding);
		Result->SetBrushColor(Color);
		return Result;
	}

    UBorder* MakeFullRegion(UWidgetTree& Tree, const FName Name,
        const FLinearColor& Color, const FMargin Padding)
    {
        auto* Result = Tree.ConstructWidget<UFMCodexFullCardSurface>(
            UFMCodexFullCardSurface::StaticClass(), Name);
        Result->SetPadding(Padding);
        Result->SetBrushColor(Color);
        return Result;
    }

    void SetFullSurface(UBorder* Region, bool bPilot, EFMCodexFullCardSurface Kind,
        const FLinearColor& Accent)
    {
        CastChecked<UFMCodexFullCardSurface>(Region)->SetSurface(
            bPilot ? Kind : EFMCodexFullCardSurface::None, Accent);
    }

	UHorizontalBox* MakeFullCardSectionHeading(
		UWidgetTree& Tree,
		const FName Name,
		const FText& Heading)
	{
		const FString BaseName = Name.ToString();
		UHorizontalBox* Row = Tree.ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), Name);
		auto AddRule = [&Tree, Row, &BaseName](const TCHAR* Suffix)
		{
			USizeBox* RuleBounds = Tree.ConstructWidget<USizeBox>(
				USizeBox::StaticClass(),
				FName(*(BaseName + Suffix + TEXT("Bounds"))));
			RuleBounds->SetHeightOverride(1.0f);
			UBorder* Rule = MakeFullRegion(Tree,
				FName(*(BaseName + Suffix)),
				FLinearColor(0.31f, 0.40f, 0.45f, 0.42f),
				FMargin(0.0f));
			RuleBounds->AddChild(Rule);
			if (UHorizontalBoxSlot* RuleSlot =
				Row->AddChildToHorizontalBox(RuleBounds))
			{
				RuleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				RuleSlot->SetVerticalAlignment(VAlign_Center);
			}
		};
		AddRule(TEXT("RuleLeft"));
		UTextBlock* Title = MakeText(Tree,
			FName(*(BaseName + TEXT("Title"))));
		Title->SetText(Heading);
		Title->SetAutoWrapText(false);
		Title->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*Title, EFMCodexPlayerUITextRole::SectionHeading);
		FSlateFontInfo TitleFont = Title->GetFont();
		TitleFont.Size = 14;
		TitleFont.TypefaceFontName = TEXT("Medium");
		Title->SetFont(TitleFont);
		Title->SetColorAndOpacity(FSlateColor(
			FLinearColor::FromSRGBColor(FColor(0xE4, 0xE8, 0xE7))));
		if (UHorizontalBoxSlot* TitleSlot =
			Row->AddChildToHorizontalBox(Title))
		{
			TitleSlot->SetPadding(FMargin(9.0f, 0.0f));
			TitleSlot->SetVerticalAlignment(VAlign_Center);
		}
		AddRule(TEXT("RuleRight"));
		return Row;
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

	int32 GetMeasuredSingleLineFontSize(
		const FText& Name,
		const FSlateFontInfo& BaseFont,
		const float SafeWidth,
		const int32 MaximumFontSize,
		const int32 MinimumFontSize)
	{
		for (int32 FontSize = MaximumFontSize;
			FontSize >= MinimumFontSize; --FontSize)
		{
			FSlateFontInfo Candidate = BaseFont;
			Candidate.Size = FontSize;
			float MeasuredWidth = 0.0f;
			if (TryMeasureHandMicroName(Name, Candidate, MeasuredWidth)
				&& MeasuredWidth <= SafeWidth)
			{
				return FontSize;
			}
		}
		return MinimumFontSize;
	}

	FLinearColor FullCardSurfaceColor()
	{
		return FLinearColor::FromSRGBColor(FColor(0x07, 0x15, 0x21));
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
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& RequestOnPitchSelection())
	{
		return FReply::Handled();
	}
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton
		&& RequestSelectionFeedback())
	{
		return FReply::Handled();
	}
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

void UFMCodexPlayerCardWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	RequestFullCardDetailHover();
}

void UFMCodexPlayerCardWidget::NativeOnMouseLeave(
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnDetailHoverDismissed.Broadcast(this);
}

void UFMCodexPlayerCardWidget::RefreshFromPresentation(
	const FFMCodexUMGCardViewModel& InPresentation,
	const EFMCodexPlayerCardPresentationMode InMode)
{
	Presentation = InPresentation;
	PresentationMode = InMode;
	if (PresentationMode != EFMCodexPlayerCardPresentationMode::PitchMini
		&& PresentationMode != EFMCodexPlayerCardPresentationMode::HandMicro)
	{
		OnPitchSelectionOptionId = NAME_None;
		bSelectableForCurrentPrompt = false;
		SelectionFeedbackReason = EFMCodexUMGSelectionFeedbackReason::None;
		SetCursor(EMouseCursor::Default);
	}
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
	bDragSourcePresentationActive = false;
	SetRenderOpacity(1.0f);
}

bool UFMCodexPlayerCardWidget::IsDeploymentDragEnabled() const
{
	return bDeploymentDragEnabled;
}

bool UFMCodexPlayerCardWidget::IsSelectedRoleTagVisible() const
{
	return PitchMiniSelectedRoleTag != nullptr
		&& PitchMiniSelectedRoleTag->GetVisibility()
			== ESlateVisibility::HitTestInvisible;
}

FText UFMCodexPlayerCardWidget::GetSelectedRoleTagText() const
{
	return PitchMiniSelectedRoleText != nullptr
		? PitchMiniSelectedRoleText->GetText() : FText::GetEmpty();
}

FName UFMCodexPlayerCardWidget::GetDeploymentDragCardId() const
{
	return DeploymentDragCardId;
}

bool UFMCodexPlayerCardWidget::IsDeploymentDragGoalkeeper() const
{
	return bDeploymentDragGoalkeeper;
}

bool UFMCodexPlayerCardWidget::CanExposeFullCardDetail() const
{
	return !Presentation.CardId.IsNone()
		&& !bDragSourcePresentationActive
		&& (PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
			|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini);
}

bool UFMCodexPlayerCardWidget::IsDragSourcePresentationActive() const
{
	return bDragSourcePresentationActive;
}

EFMCodexUMGCardInteractionState
UFMCodexPlayerCardWidget::GetInteractionState() const
{
	if (bDragSourcePresentationActive)
	{
		return EFMCodexUMGCardInteractionState::DragSource;
	}
	if (IsHovered() && CanExposeFullCardDetail())
	{
		return EFMCodexUMGCardInteractionState::Hover;
	}
	if (bSelectableForCurrentPrompt)
	{
		return EFMCodexUMGCardInteractionState::OnPitchSelectable;
	}
	return PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
		? EFMCodexUMGCardInteractionState::Deployed
		: EFMCodexUMGCardInteractionState::Default;
}

void UFMCodexPlayerCardWidget::ConfigureOnPitchSelection(
	const FName OptionId,
	const bool bSelectable)
{
	bSelectableForCurrentPrompt = bSelectable
		&& (PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
			|| PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro)
		&& !OptionId.IsNone();
	OnPitchSelectionOptionId = bSelectableForCurrentPrompt
		? OptionId : NAME_None;
	SetCursor(bSelectableForCurrentPrompt
		|| SelectionFeedbackReason != EFMCodexUMGSelectionFeedbackReason::None
		? EMouseCursor::Hand : EMouseCursor::Default);
}

void UFMCodexPlayerCardWidget::ClearOnPitchSelection()
{
	ConfigureOnPitchSelection(NAME_None, false);
}

bool UFMCodexPlayerCardWidget::IsSelectableForCurrentPrompt() const
{
	return bSelectableForCurrentPrompt;
}

FName UFMCodexPlayerCardWidget::GetOnPitchSelectionOptionId() const
{
	return OnPitchSelectionOptionId;
}

bool UFMCodexPlayerCardWidget::RequestOnPitchSelection()
{
	if (!bSelectableForCurrentPrompt || OnPitchSelectionOptionId.IsNone())
	{
		return false;
	}
	OnOnPitchSelectionRequested.Broadcast(OnPitchSelectionOptionId);
	return true;
}

void UFMCodexPlayerCardWidget::ConfigureSelectionFeedback(
	const EFMCodexUMGSelectionFeedbackReason InReason)
{
	SelectionFeedbackReason =
		PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
			? InReason : EFMCodexUMGSelectionFeedbackReason::None;
	SetCursor(bSelectableForCurrentPrompt
		|| SelectionFeedbackReason != EFMCodexUMGSelectionFeedbackReason::None
		? EMouseCursor::Hand : EMouseCursor::Default);
}

EFMCodexUMGSelectionFeedbackReason
UFMCodexPlayerCardWidget::GetSelectionFeedbackReason() const
{
	return SelectionFeedbackReason;
}

bool UFMCodexPlayerCardWidget::RequestSelectionFeedback()
{
	if (SelectionFeedbackReason == EFMCodexUMGSelectionFeedbackReason::None
		|| Presentation.CardId.IsNone())
	{
		return false;
	}
	OnSelectionFeedbackRequested.Broadcast(Presentation.CardId);
	return true;
}

bool UFMCodexPlayerCardWidget::RequestFullCardDetailHover()
{
	if (!CanExposeFullCardDetail())
	{
		return false;
	}
	OnDetailHoverRequested.Broadcast(this);
	return true;
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
	Operation->Pivot = EDragPivot::CenterLeft;
	Operation->Offset = FVector2D(0.06f, -0.10f);
	UFMCodexPlayerCardWidget* DragVisual = nullptr;
	if (APlayerController* OwningPlayer = GetOwningPlayer())
	{
		DragVisual = CreateWidget<UFMCodexPlayerCardWidget>(
			OwningPlayer, GetClass());
	}
	else if (UWorld* World = GetWorld())
	{
		DragVisual = CreateWidget<UFMCodexPlayerCardWidget>(World, GetClass());
	}
	if (DragVisual != nullptr)
	{
		DragVisual->RefreshFromPresentation(
			Presentation, EFMCodexPlayerCardPresentationMode::HandMicro);
		DragVisual->SetVisibility(ESlateVisibility::HitTestInvisible);
		DragVisual->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
		DragVisual->SetRenderScale(FVector2D(
			FMCodexPlayerCardWidget::HandMicroDragProxyScale,
			FMCodexPlayerCardWidget::HandMicroDragProxyScale));
		DragVisual->SetRenderOpacity(0.98f);
		Operation->DefaultDragVisual = DragVisual;
	}
	Operation->OnDrop.AddDynamic(
		this, &UFMCodexPlayerCardWidget::HandleDeploymentDragFinished);
	Operation->OnDragCancelled.AddDynamic(
		this, &UFMCodexPlayerCardWidget::HandleDeploymentDragFinished);
	bDragSourcePresentationActive = true;
	SetRenderOpacity(0.28f);
	OnDeploymentDragStarted.Broadcast(
		DeploymentDragCardId, bDeploymentDragGoalkeeper);
	return Operation;
}

void UFMCodexPlayerCardWidget::HandleDeploymentDragFinished(
	UDragDropOperation* Operation)
{
	bDragSourcePresentationActive = false;
	SetRenderOpacity(1.0f);
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

FText UFMCodexPlayerCardWidget::GetRenderedPositionText() const
{
	return RoleText == nullptr ? FText::GetEmpty() : RoleText->GetText();
}

FText UFMCodexPlayerCardWidget::GetRenderedRarityText() const
{
	return RarityText == nullptr ? FText::GetEmpty() : RarityText->GetText();
}

FText UFMCodexPlayerCardWidget::GetRenderedTeamText() const
{
	return TeamText == nullptr ? FText::GetEmpty() : TeamText->GetText();
}

int32 UFMCodexPlayerCardWidget::GetRenderedBiographyRowCount() const
{
	return RenderedBiographyRowCount;
}

int32 UFMCodexPlayerCardWidget::GetFullCardNameFontSize() const
{
	return IdentityText == nullptr ? 0 : IdentityText->GetFont().Size;
}

bool UFMCodexPlayerCardWidget::IsOverallVisible() const
{
	return OverallNumberText != nullptr
		&& OverallNumberText->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UFMCodexPlayerCardWidget::IsPlayerFacingSerialVisible() const
{
	return !Presentation.PlayerFacingSerialLabel.IsEmpty()
        && ((PlayerFacingSerialText != nullptr
            && PlayerFacingSerialText->GetVisibility() != ESlateVisibility::Collapsed)
            || (FullCardCollectionText != nullptr
                && FullCardCollectionText->GetVisibility() != ESlateVisibility::Collapsed));
}

bool UFMCodexPlayerCardWidget::IsDeveloperReferenceVisible() const
{
	return DeveloperReferenceText != nullptr
		&& DeveloperReferenceText->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UFMCodexPlayerCardWidget::IsOwnerVisible() const
{
	return OwnerText != nullptr
		&& OwnerText->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UFMCodexPlayerCardWidget::IsTeamVisible() const
{
	return TeamText != nullptr
		&& TeamText->GetVisibility() != ESlateVisibility::Collapsed;
}

bool UFMCodexPlayerCardWidget::IsRoleIconVisible() const
{
	return RoleIconImage != nullptr
		&& RoleIconImage->GetVisibility() != ESlateVisibility::Collapsed;
}

FLinearColor UFMCodexPlayerCardWidget::GetFullCardBaseSurfaceColor() const
{
	return FullCardBaseSurface == nullptr
		? FLinearColor::Transparent : FullCardBaseSurface->GetBrushColor();
}

const TArray<FLinearColor>&
UFMCodexPlayerCardWidget::GetRenderedAttributeTierColors() const
{
	return RenderedAttributeTierColors;
}

FLinearColor UFMCodexPlayerCardWidget::GetAttributeTierColor(const int32 Value)
{
	if (Value <= 2)
	{
		return FLinearColor::FromSRGBColor(FColor(0x1E, 0xFF, 0x00));
	}
	if (Value <= 4)
	{
		return FLinearColor::FromSRGBColor(FColor(0x00, 0x70, 0xDD));
	}
	if (Value == 5)
	{
		return FLinearColor::FromSRGBColor(FColor(0xA3, 0x35, 0xEE));
	}
	return FLinearColor::FromSRGBColor(FColor(0xD6, 0xA8, 0x42));
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
		return FVector2D(
			FMCodexPlayerCardWidget::FullCardWidth,
			FMCodexPlayerCardWidget::FullCardHeight);
	}
}

FBox2f UFMCodexPlayerCardWidget::CalculatePitchMiniHeroCrop(
	const FIntPoint SourceSize)
{
	using namespace FMCodexPlayerCardWidget;
	if (SourceSize.X <= 0 || SourceSize.Y <= 0)
	{
		return FBox2f(ForceInit);
	}

	const float SourceAspect =
		static_cast<float>(SourceSize.X) / static_cast<float>(SourceSize.Y);
	const float TargetAspect = PitchMiniInteriorWidth / PitchMiniPortraitHeight;
	FVector2f AspectFillSize(1.0f, 1.0f);
	if (SourceAspect > TargetAspect)
	{
		AspectFillSize.X = TargetAspect / SourceAspect;
	}
	else if (SourceAspect < TargetAspect)
	{
		AspectFillSize.Y = SourceAspect / TargetAspect;
	}

	// Start from the distortion-free aspect-fill window, then apply one global
	// tactical-card hero zoom. The focal anchor retains a small amount of hair
	// breathing room while carrying the neck/upper-shirt edge to the lower crop.
	const FVector2f HeroSize = AspectFillSize / PitchMiniHeroZoom;
	const float HeroLeft = FMath::Clamp(
		PitchMiniHeroFocalX - HeroSize.X * 0.5f,
		0.0f, 1.0f - HeroSize.X);
	const float HeroTop = FMath::Clamp(
		PitchMiniHeroFocalY - HeroSize.Y * PitchMiniHeroFocalFrameY,
		0.0f, 1.0f - HeroSize.Y);
	return FBox2f(FVector2f(HeroLeft, HeroTop),
		FVector2f(HeroLeft + HeroSize.X, HeroTop + HeroSize.Y));
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

	CardFrame = MakeFullRegion(*WidgetTree, TEXT("PlayerCardFrame"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::CardFrame),
		FFMCodexPlayerUIStyle::Get().GetSectionPadding());
	CardBounds->AddChild(CardFrame);
	UOverlay* FrameAssetHook = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("CardFrameAssetHook"));
	FrameAssetHook->SetClipping(EWidgetClipping::ClipToBounds);
	if (UBorderSlot* FrameAssetSlot = Cast<UBorderSlot>(
		CardFrame->AddChild(FrameAssetHook)))
	{
		FrameAssetSlot->SetHorizontalAlignment(HAlign_Fill);
		FrameAssetSlot->SetVerticalAlignment(VAlign_Fill);
	}
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
	HandMicroPortraitFallback = MakeFullRegion(*WidgetTree,
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
	AssignedNumberText = MakeText(*WidgetTree, TEXT("AssignedPlayerNumber"));
	ConfigureBoundedSingleLine(*AssignedNumberText);
	AssignedNumberText->SetJustification(ETextJustify::Right);
	AssignedNumberText->SetVisibility(ESlateVisibility::Collapsed);
	FFMCodexPlayerUIStyle::Get().ApplyText(*AssignedNumberText, EFMCodexPlayerUITextRole::Kicker);

	PitchMiniContent = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PitchMiniContent"));
	PitchMiniContent->SetClipping(EWidgetClipping::ClipToBounds);
	USizeBox* PitchPortraitBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PitchMiniPortraitBounds"));
	PitchPortraitBounds->SetWidthOverride(PitchMiniInteriorWidth);
	PitchPortraitBounds->SetHeightOverride(PitchMiniPortraitHeight);
	PitchPortraitBounds->SetClipping(EWidgetClipping::ClipToBounds);
	UOverlay* PitchPortraitLayer = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("PitchMiniPortraitLayer"));
	PitchPortraitLayer->SetClipping(EWidgetClipping::ClipToBounds);
	PitchMiniPortraitFallback = MakeFullRegion(*WidgetTree,
		TEXT("PitchMiniPortraitFallback"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::NeutralAccent), FMargin(0.0f));
	if (UOverlaySlot* FallbackSlot =
		PitchPortraitLayer->AddChildToOverlay(PitchMiniPortraitFallback))
	{
		FallbackSlot->SetHorizontalAlignment(HAlign_Fill);
		FallbackSlot->SetVerticalAlignment(VAlign_Fill);
	}
	USizeBox* PitchFallbackAtmosphereBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniPortraitFallbackAtmosphereBounds"));
	PitchFallbackAtmosphereBounds->SetHeightOverride(28.0f);
	PitchMiniPortraitFallbackAtmosphere = MakeRegion(*WidgetTree,
		TEXT("PitchMiniPortraitFallbackAtmosphere"),
		FLinearColor(0.18f, 0.31f, 0.36f, 0.20f), FMargin(0.0f));
	PitchFallbackAtmosphereBounds->AddChild(
		PitchMiniPortraitFallbackAtmosphere);
	if (UOverlaySlot* AtmosphereSlot = PitchPortraitLayer->AddChildToOverlay(
		PitchFallbackAtmosphereBounds))
	{
		AtmosphereSlot->SetHorizontalAlignment(HAlign_Fill);
		AtmosphereSlot->SetVerticalAlignment(VAlign_Top);
	}
	USizeBox* PitchFallbackHorizonBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniPortraitFallbackHorizonBounds"));
	PitchFallbackHorizonBounds->SetHeightOverride(1.0f);
	PitchFallbackHorizonBounds->AddChild(MakeRegion(*WidgetTree,
		TEXT("PitchMiniPortraitFallbackHorizon"),
		FLinearColor(0.29f, 0.45f, 0.50f, 0.20f), FMargin(0.0f)));
	if (UOverlaySlot* HorizonSlot = PitchPortraitLayer->AddChildToOverlay(
		PitchFallbackHorizonBounds))
	{
		HorizonSlot->SetPadding(FMargin(8.0f, 0.0f, 8.0f, 12.0f));
		HorizonSlot->SetHorizontalAlignment(HAlign_Fill);
		HorizonSlot->SetVerticalAlignment(VAlign_Bottom);
	}
	PitchMiniPortraitImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("PitchMiniPortraitImage"));
	PitchMiniPortraitImage->SetClipping(EWidgetClipping::ClipToBounds);
	if (UOverlaySlot* PortraitSlot =
		PitchPortraitLayer->AddChildToOverlay(PitchMiniPortraitImage))
	{
		PortraitSlot->SetHorizontalAlignment(HAlign_Fill);
		PortraitSlot->SetVerticalAlignment(VAlign_Fill);
	}
	PitchMiniPortraitTonalWash = MakeRegion(*WidgetTree,
		TEXT("PitchMiniPortraitTonalWash"),
		FLinearColor(0.03f, 0.11f, 0.15f, 0.20f), FMargin(0.0f));
	if (UOverlaySlot* WashSlot =
		PitchPortraitLayer->AddChildToOverlay(PitchMiniPortraitTonalWash))
	{
		WashSlot->SetHorizontalAlignment(HAlign_Fill);
		WashSlot->SetVerticalAlignment(VAlign_Fill);
	}
	PitchPortraitBounds->AddChild(PitchPortraitLayer);
	PitchMiniContent->AddChildToVerticalBox(PitchPortraitBounds);

	USizeBox* PitchIdentityBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PitchMiniIdentityBounds"));
	PitchIdentityBounds->SetWidthOverride(PitchMiniInteriorWidth);
	PitchIdentityBounds->SetHeightOverride(PitchMiniIdentityHeight);
	UBorder* PitchIdentitySurface = MakeRegion(*WidgetTree,
		TEXT("PitchMiniIdentitySurface"),
		FLinearColor::FromSRGBColor(FColor(0x0B, 0x20, 0x2E, 0xF2)),
		FMargin(3.0f, 0.0f));
	UHorizontalBox* PitchIdentityRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("PitchMiniIdentityRow"));
	PitchMiniIdentityText = MakeText(
		*WidgetTree, TEXT("PitchMiniPlayerName"));
	PitchMiniIdentitySeparatorText = MakeText(
		*WidgetTree, TEXT("PitchMiniIdentitySeparator"), TEXT("|"));
	PitchMiniRoleText = MakeText(*WidgetTree, TEXT("PitchMiniPosition"));
	ConfigureBoundedSingleLine(*PitchMiniIdentityText);
	ConfigureBoundedSingleLine(*PitchMiniIdentitySeparatorText);
	ConfigureBoundedSingleLine(*PitchMiniRoleText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniIdentityText, EFMCodexPlayerUITextRole::Kicker);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniIdentitySeparatorText, EFMCodexPlayerUITextRole::Secondary);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniRoleText, EFMCodexPlayerUITextRole::Secondary);
	if (UHorizontalBoxSlot* NameSlot =
		PitchIdentityRow->AddChildToHorizontalBox(PitchMiniIdentityText))
	{
		NameSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NameSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UHorizontalBoxSlot* SeparatorSlot =
		PitchIdentityRow->AddChildToHorizontalBox(
			PitchMiniIdentitySeparatorText))
	{
		SeparatorSlot->SetPadding(FMargin(2.0f, 0.0f));
		SeparatorSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UHorizontalBoxSlot* RoleSlot =
		PitchIdentityRow->AddChildToHorizontalBox(PitchMiniRoleText))
	{
		RoleSlot->SetVerticalAlignment(VAlign_Center);
	}
	PitchIdentitySurface->AddChild(PitchIdentityRow);
	PitchIdentityBounds->AddChild(PitchIdentitySurface);
	PitchMiniContent->AddChildToVerticalBox(PitchIdentityBounds);
	FrameAssetHook->AddChildToOverlay(PitchMiniContent);

	USizeBox* PitchSelectedRoleBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniSelectedRoleBounds"));
	PitchSelectedRoleBounds->SetWidthOverride(38.0f);
	PitchSelectedRoleBounds->SetHeightOverride(20.0f);
	PitchMiniSelectedRoleTag = MakeRegion(*WidgetTree,
		TEXT("PitchMiniSelectedRoleTag"),
		FLinearColor::FromSRGBColor(FColor(0x08, 0x1C, 0x2A, 0xE6)),
		FMargin(5.0f, 1.0f));
	PitchMiniSelectedRoleTag->SetBrush(FSlateRoundedBoxBrush(
		FLinearColor::FromSRGBColor(FColor(0x08, 0x1C, 0x2A, 0xE6)),
		3.0f,
		FLinearColor::FromSRGBColor(FColor(0x8D, 0xA9, 0xB8, 0x88)),
		1.0f));
	PitchMiniSelectedRoleTag->SetBrushColor(FLinearColor::White);
	PitchMiniSelectedRoleTag->SetVisibility(ESlateVisibility::Collapsed);
	PitchMiniSelectedRoleText = MakeText(*WidgetTree,
		TEXT("PitchMiniSelectedRoleText"));
	ConfigureBoundedSingleLine(*PitchMiniSelectedRoleText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PitchMiniSelectedRoleText, EFMCodexPlayerUITextRole::Secondary);
	FSlateFontInfo SelectedRoleFont = PitchMiniSelectedRoleText->GetFont();
	SelectedRoleFont.Size = 11;
	SelectedRoleFont.TypefaceFontName = TEXT("Medium");
	PitchMiniSelectedRoleText->SetFont(SelectedRoleFont);
	PitchMiniSelectedRoleText->SetColorAndOpacity(
		FSlateColor(FLinearColor::FromSRGBColor(
			FColor(0xE7, 0xF0, 0xF3))));
	PitchMiniSelectedRoleText->SetJustification(ETextJustify::Center);
	PitchMiniSelectedRoleTag->AddChild(PitchMiniSelectedRoleText);
	PitchSelectedRoleBounds->AddChild(PitchMiniSelectedRoleTag);
	if (UOverlaySlot* SelectedRoleSlot =
		FrameAssetHook->AddChildToOverlay(PitchSelectedRoleBounds))
	{
		SelectedRoleSlot->SetPadding(FMargin(0.0f, 7.0f, 7.0f, 0.0f));
		SelectedRoleSlot->SetHorizontalAlignment(HAlign_Right);
		SelectedRoleSlot->SetVerticalAlignment(VAlign_Top);
	}

	auto AddPitchMiniTacticalMatchSegment =
		[this, FrameAssetHook](const TCHAR* Name,
			const float Width, const float Height,
			const EHorizontalAlignment HorizontalAlignment,
			const EVerticalAlignment VerticalAlignment,
			const FMargin& SegmentPadding,
			TArray<TObjectPtr<UBorder>>& SegmentCollection)
		{
			const FString SegmentName(Name);
			USizeBox* SegmentBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(),
				FName(*(SegmentName + TEXT("Bounds"))));
			if (Width > 0.0f)
			{
				SegmentBounds->SetWidthOverride(Width);
			}
			if (Height > 0.0f)
			{
				SegmentBounds->SetHeightOverride(Height);
			}
			UBorder* Segment = MakeRegion(*WidgetTree, FName(*SegmentName),
				FLinearColor::Transparent, FMargin(0.0f));
			Segment->SetVisibility(ESlateVisibility::Collapsed);
			SegmentBounds->AddChild(Segment);
			if (UOverlaySlot* SegmentSlot =
				FrameAssetHook->AddChildToOverlay(SegmentBounds))
			{
				SegmentSlot->SetPadding(SegmentPadding);
				SegmentSlot->SetHorizontalAlignment(HorizontalAlignment);
				SegmentSlot->SetVerticalAlignment(VerticalAlignment);
			}
			SegmentCollection.Add(Segment);
		};

	const auto AddPitchMiniTacticalMatchPerimeter =
		[&AddPitchMiniTacticalMatchSegment](const TCHAR* Prefix,
			const float Thickness,
			TArray<TObjectPtr<UBorder>>& SegmentCollection)
		{
			AddPitchMiniTacticalMatchSegment(
				*FString::Printf(TEXT("%sTop"), Prefix),
				0.0f, Thickness, HAlign_Fill, VAlign_Top,
				FMargin(PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset, 0.0f), SegmentCollection);
			AddPitchMiniTacticalMatchSegment(
				*FString::Printf(TEXT("%sBottom"), Prefix),
				0.0f, Thickness, HAlign_Fill, VAlign_Bottom,
				FMargin(PitchMiniTacticalMatchInset, 0.0f,
					PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset), SegmentCollection);
			AddPitchMiniTacticalMatchSegment(
				*FString::Printf(TEXT("%sLeft"), Prefix),
				Thickness, 0.0f, HAlign_Left, VAlign_Fill,
				FMargin(PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset, 0.0f,
					PitchMiniTacticalMatchInset), SegmentCollection);
			AddPitchMiniTacticalMatchSegment(
				*FString::Printf(TEXT("%sRight"), Prefix),
				Thickness, 0.0f, HAlign_Right, VAlign_Fill,
				FMargin(0.0f, PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset,
					PitchMiniTacticalMatchInset), SegmentCollection);
		};
	AddPitchMiniTacticalMatchPerimeter(TEXT("PitchMiniTacticalMatchGlow"),
		PitchMiniTacticalMatchGlowThickness,
		PitchMiniTacticalMatchGlowSegments);
	AddPitchMiniTacticalMatchPerimeter(TEXT("PitchMiniTacticalMatchStroke"),
		PitchMiniTacticalMatchStrokeThickness,
		PitchMiniTacticalMatchStrokeSegments);

	TacticalMatchPipGroupBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniTacticalMatchPipGroupBounds"));
	TacticalMatchPipGroupBounds->SetWidthOverride(
		PitchMiniTacticalMatchPipDiameter);
	TacticalMatchPipGroupBounds->SetHeightOverride(
		PitchMiniTacticalMatchPipDiameter * 2.0f
			+ PitchMiniTacticalMatchPipGap);
	TacticalMatchPipGroupBounds->SetVisibility(ESlateVisibility::Collapsed);
	UOverlay* PitchTacticalPipGroup =
		WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(),
			TEXT("PitchMiniTacticalMatchPipGroup"));
	TacticalMatchPipGroupBounds->AddChild(PitchTacticalPipGroup);
	const auto AddPitchMiniTacticalMatchPip =
		[this, PitchTacticalPipGroup](const TCHAR* PipName,
			const TCHAR* BoundsName,
			const EVerticalAlignment VerticalAlignment,
			TObjectPtr<UBorder>& OutPip)
		{
			USizeBox* PipBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(BoundsName));
			PipBounds->SetWidthOverride(PitchMiniTacticalMatchPipDiameter);
			PipBounds->SetHeightOverride(PitchMiniTacticalMatchPipDiameter);
			OutPip = MakeRegion(*WidgetTree, FName(PipName),
				FLinearColor::Transparent, FMargin(0.0f));
			OutPip->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White,
				PitchMiniTacticalMatchPipDiameter * 0.5f,
				FVector2f(PitchMiniTacticalMatchPipDiameter,
					PitchMiniTacticalMatchPipDiameter)));
			OutPip->SetBrushColor(FLinearColor::Transparent);
			OutPip->SetVisibility(ESlateVisibility::Collapsed);
			PipBounds->AddChild(OutPip);
			if (UOverlaySlot* PipSlot =
				PitchTacticalPipGroup->AddChildToOverlay(PipBounds))
			{
				PipSlot->SetHorizontalAlignment(HAlign_Center);
				PipSlot->SetVerticalAlignment(VerticalAlignment);
			}
		};
	AddPitchMiniTacticalMatchPip(TEXT("PitchMiniTacticalMatchPipTop"),
		TEXT("PitchMiniTacticalMatchPipTopBounds"), VAlign_Top,
		PitchMiniTacticalMatchPipTop);
	AddPitchMiniTacticalMatchPip(TEXT("PitchMiniTacticalMatchPipBottom"),
		TEXT("PitchMiniTacticalMatchPipBottomBounds"), VAlign_Bottom,
		PitchMiniTacticalMatchPipBottom);
	if (UOverlaySlot* PipGroupSlot =
		FrameAssetHook->AddChildToOverlay(TacticalMatchPipGroupBounds))
	{
		PipGroupSlot->SetPadding(
			FMargin(PitchMiniTacticalMatchPipLeftInset,
				PitchMiniTacticalMatchPipTopInset, 0.0f, 0.0f));
		PipGroupSlot->SetHorizontalAlignment(HAlign_Left);
		PipGroupSlot->SetVerticalAlignment(VAlign_Top);
	}

	USizeBox* PitchOwnershipRailLeftBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniOwnershipRailLeftBounds"));
	PitchOwnershipRailLeftBounds->SetWidthOverride(3.0f);
	PitchMiniOwnershipRailLeft = MakeRegion(*WidgetTree,
		TEXT("PitchMiniOwnershipRailLeft"), FLinearColor::Transparent,
		FMargin(0.0f));
	PitchOwnershipRailLeftBounds->AddChild(PitchMiniOwnershipRailLeft);
	if (UOverlaySlot* LeftRailSlot = FrameAssetHook->AddChildToOverlay(
		PitchOwnershipRailLeftBounds))
	{
		LeftRailSlot->SetHorizontalAlignment(HAlign_Left);
		LeftRailSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* PitchOwnershipRailRightBounds =
		WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),
			TEXT("PitchMiniOwnershipRailRightBounds"));
	PitchOwnershipRailRightBounds->SetWidthOverride(3.0f);
	PitchMiniOwnershipRailRight = MakeRegion(*WidgetTree,
		TEXT("PitchMiniOwnershipRailRight"), FLinearColor::Transparent,
		FMargin(0.0f));
	PitchOwnershipRailRightBounds->AddChild(PitchMiniOwnershipRailRight);
	if (UOverlaySlot* RightRailSlot = FrameAssetHook->AddChildToOverlay(
		PitchOwnershipRailRightBounds))
	{
		RightRailSlot->SetHorizontalAlignment(HAlign_Right);
		RightRailSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InMatchFullCardHierarchy"));
	FullCardInnerFrame = MakeRegion(*WidgetTree,
		TEXT("InMatchFullCardInnerFrame"),
		FLinearColor(0.30f, 0.38f, 0.43f, 0.42f), FMargin(1.0f));
	FullCardBaseSurface = MakeFullRegion(*WidgetTree,
		TEXT("InMatchFullCardBaseSurface"), FullCardSurfaceColor(),
		FMargin(5.0f));
	FullCardInnerFrame->AddChild(FullCardBaseSurface);
	DetailedContentLayer = FullCardInnerFrame;
	DetailedContentLayer->SetClipping(EWidgetClipping::ClipToBounds);
	if (UBorderSlot* FullCardBodySlot = Cast<UBorderSlot>(
		FullCardBaseSurface->AddChild(Body)))
	{
		FullCardBodySlot->SetHorizontalAlignment(HAlign_Fill);
		FullCardBodySlot->SetVerticalAlignment(VAlign_Fill);
	}
	if (UOverlaySlot* FullCardLayerSlot =
		FrameAssetHook->AddChildToOverlay(DetailedContentLayer))
	{
		FullCardLayerSlot->SetHorizontalAlignment(HAlign_Fill);
		FullCardLayerSlot->SetVerticalAlignment(VAlign_Fill);
	}

	USizeBox* RarityRailBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InMatchFullCardRarityRailBounds"));
	RarityRailBounds->SetHeightOverride(2.0f);
	FullCardRarityRail = MakeRegion(*WidgetTree,
		TEXT("InMatchFullCardRarityRail"), FLinearColor::White,
		FMargin(0.0f));
	RarityRailBounds->AddChild(FullCardRarityRail);
	Body->AddChildToVerticalBox(RarityRailBounds);

	PortraitBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("PortraitAssetBounds"));
	PortraitBounds->SetHeightOverride(FullCardHeroHeight);
	PortraitBounds->SetClipping(EWidgetClipping::ClipToBounds);
	PortraitPresentationRegion = MakeRegion(*WidgetTree,
		TEXT("PortraitPresentationRegion"),
		FLinearColor::FromSRGBColor(FColor(0x09, 0x1B, 0x29)),
		FMargin(0.0f));
	UOverlay* PortraitLayer = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("InMatchFullCardHeroLayer"));
	PortraitLayer->SetClipping(EWidgetClipping::ClipToBounds);
    auto* MissingFull = CastChecked<UFMCodexFullCardSurface>(MakeFullRegion(*WidgetTree,
        TEXT("FullPortraitFallback"), FLinearColor::White, FMargin(0)));
    MissingFull->SetSurface(EFMCodexFullCardSurface::MissingPortrait,FLinearColor::White);
    if (auto* MissingSlot = PortraitLayer->AddChildToOverlay(MissingFull))
    {
        MissingSlot->SetHorizontalAlignment(HAlign_Fill);
        MissingSlot->SetVerticalAlignment(VAlign_Fill);
    }
	PortraitImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("PortraitAssetImage"));
	PortraitImage->SetVisibility(ESlateVisibility::Collapsed);
	PortraitImage->SetClipping(EWidgetClipping::ClipToBounds);
	if (UOverlaySlot* PortraitSlot =
		PortraitLayer->AddChildToOverlay(PortraitImage))
	{
		PortraitSlot->SetHorizontalAlignment(HAlign_Fill);
		PortraitSlot->SetVerticalAlignment(VAlign_Fill);
	}
	PortraitPlaceholderText = MakeText(
		*WidgetTree, TEXT("PortraitPlaceholderText"));
	PortraitPlaceholderText->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PortraitPlaceholderText, EFMCodexPlayerUITextRole::Secondary);
	PortraitLayer->AddChildToOverlay(PortraitPlaceholderText);

	HeaderRegion = MakeRegion(*WidgetTree,
		TEXT("RoleRarityHeaderRegion"), FLinearColor::Transparent,
		FMargin(9.0f, 7.0f));
	UHorizontalBox* Header = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InMatchFullCardTopMetaRow"));
	HeaderRegion->AddChild(Header);
	RoleIconHook = MakeRegion(*WidgetTree, TEXT("RoleIconHook"),
		FLinearColor(0.015f, 0.035f, 0.052f, 0.58f),
		FMargin(5.0f, 2.0f));
	RoleText = MakeText(*WidgetTree, TEXT("CardRole"));
	RoleText->SetAutoWrapText(false);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*RoleText, EFMCodexPlayerUITextRole::SectionHeading);
	FSlateFontInfo RoleFont = RoleText->GetFont();
	RoleFont.Size = 16;
	RoleFont.TypefaceFontName = TEXT("Medium");
	RoleText->SetFont(RoleFont);
	UHorizontalBox* RoleIdentityRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("RoleIdentityRow"));
	USizeBox* RoleIconBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("RoleIconAssetBounds"));
	RoleIconBounds->SetWidthOverride(20.0f);
	RoleIconBounds->SetHeightOverride(20.0f);
	RoleIconImage = WidgetTree->ConstructWidget<UImage>(
		UImage::StaticClass(), TEXT("RoleIconAssetImage"));
	RoleIconImage->SetVisibility(ESlateVisibility::Collapsed);
	RoleIconBounds->AddChild(RoleIconImage);
	RoleIdentityRow->AddChildToHorizontalBox(RoleIconBounds);
	if (UHorizontalBoxSlot* RoleTextSlot =
		RoleIdentityRow->AddChildToHorizontalBox(RoleText))
	{
		RoleTextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		RoleTextSlot->SetVerticalAlignment(VAlign_Center);
	}
	RoleIconHook->AddChild(RoleIdentityRow);
	if (UHorizontalBoxSlot* RoleSlot =
		Header->AddChildToHorizontalBox(RoleIconHook))
	{
		RoleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	RarityText = MakeText(*WidgetTree, TEXT("CardRarity"));
	RarityText->SetAutoWrapText(false);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*RarityText, EFMCodexPlayerUITextRole::Kicker);
	FSlateFontInfo RarityFont = RarityText->GetFont();
	RarityFont.Size = 13;
	RarityFont.TypefaceFontName = TEXT("Medium");
	RarityText->SetFont(RarityFont);
	Header->AddChildToHorizontalBox(RarityText);
	if (UOverlaySlot* HeaderSlot = PortraitLayer->AddChildToOverlay(HeaderRegion))
	{
		HeaderSlot->SetHorizontalAlignment(HAlign_Fill);
		HeaderSlot->SetVerticalAlignment(VAlign_Top);
	}

	UVerticalBox* OverallGroup = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InMatchFullCardOverallGroup"));
	OverallNumberText = MakeText(*WidgetTree, TEXT("OverallNumber"));
	OverallNumberText->SetAutoWrapText(false);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*OverallNumberText, EFMCodexPlayerUITextRole::Identity);
	FSlateFontInfo OverallFont = OverallNumberText->GetFont();
	OverallFont.Size = 44;
	OverallFont.TypefaceFontName = TEXT("Bold");
	OverallNumberText->SetFont(OverallFont);
	OverallLabelText = MakeText(*WidgetTree, TEXT("OverallLabel"));
	OverallLabelText->SetText(
		FFMCodexPlayerUIPresentationText::OverallHeading());
	OverallLabelText->SetAutoWrapText(false);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*OverallLabelText, EFMCodexPlayerUITextRole::Kicker);
	OverallGroup->AddChildToVerticalBox(OverallNumberText);
	OverallGroup->AddChildToVerticalBox(OverallLabelText);
	if (UOverlaySlot* OverallSlot =
		PortraitLayer->AddChildToOverlay(OverallGroup))
	{
		OverallSlot->SetHorizontalAlignment(HAlign_Left);
		OverallSlot->SetVerticalAlignment(VAlign_Top);
		OverallSlot->SetPadding(FMargin(12.0f, 18.0f, 0.0f, 0.0f));
	}

	USizeBox* BiographyBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InMatchFullCardBiographyBounds"));
	BiographyBounds->SetWidthOverride(FullCardBiographyWidth);
	BiographyRegion = MakeFullRegion(*WidgetTree,
		TEXT("InMatchFullCardBiographyRegion"),
		FLinearColor(0.012f, 0.029f, 0.045f, 0.58f),
		FMargin(7.0f, 6.0f));
	BiographyList = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InMatchFullCardBiographyList"));
	BiographyRegion->AddChild(BiographyList);
	BiographyBounds->AddChild(BiographyRegion);
	if (UOverlaySlot* BiographySlot =
		PortraitLayer->AddChildToOverlay(BiographyBounds))
	{
		BiographySlot->SetHorizontalAlignment(HAlign_Right);
		BiographySlot->SetVerticalAlignment(VAlign_Top);
		BiographySlot->SetPadding(FMargin(0.0f, 10.0f, 5.0f, 0.0f));
	}

	IdentityRegion = MakeFullRegion(*WidgetTree,
		TEXT("CardIdentityRegion"),
		FLinearColor::Transparent,
		FMargin(0.0f));
	UOverlay* IdentityBody = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("CardIdentityBody"));
	UVerticalBox* IdentityReadabilityScrim =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("FullCardIdentityReadabilityScrim"));
	auto AddIdentityScrimBand = [this, IdentityReadabilityScrim](
		const FName BoundsName, const FName BandName,
		const float Height, const float Alpha)
	{
		USizeBox* BandBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), BoundsName);
		BandBounds->SetHeightOverride(Height);
		BandBounds->AddChild(MakeRegion(*WidgetTree, BandName,
			FLinearColor(0.004f, 0.012f, 0.021f, Alpha), FMargin(0.0f)));
		IdentityReadabilityScrim->AddChildToVerticalBox(BandBounds);
	};
	AddIdentityScrimBand(TEXT("FullCardIdentityFadeTopBounds"),
		TEXT("FullCardIdentityFadeTop"), 6.0f, 0.12f);
	AddIdentityScrimBand(TEXT("FullCardIdentityFadeMiddleBounds"),
		TEXT("FullCardIdentityFadeMiddle"), 8.0f, 0.34f);
	UBorder* IdentityReadabilityBase = MakeRegion(*WidgetTree,
		TEXT("FullCardIdentityReadabilityBase"),
		FLinearColor(0.004f, 0.012f, 0.021f, 0.62f), FMargin(0.0f));
	if (UVerticalBoxSlot* BaseSlot =
		IdentityReadabilityScrim->AddChildToVerticalBox(IdentityReadabilityBase))
	{
		BaseSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
	if (UOverlaySlot* ScrimSlot =
		IdentityBody->AddChildToOverlay(IdentityReadabilityScrim))
	{
		ScrimSlot->SetHorizontalAlignment(HAlign_Fill);
		ScrimSlot->SetVerticalAlignment(VAlign_Fill);
	}
	UVerticalBox* IdentityTextStack = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CardIdentityTextStack"));
	USizeBox* IdentityAccentBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("FullCardIdentityAccentBounds"));
	IdentityAccentBounds->SetHeightOverride(1.0f);
	FullCardIdentityAccent = MakeRegion(*WidgetTree,
		TEXT("FullCardIdentityAccent"), FLinearColor::White, FMargin(0.0f));
	IdentityAccentBounds->AddChild(FullCardIdentityAccent);
	IdentityText = MakeText(*WidgetTree, TEXT("CardIdentity"));
	IdentityText->SetAutoWrapText(false);
	IdentityText->SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
	IdentityText->SetClipping(EWidgetClipping::ClipToBounds);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*IdentityText, EFMCodexPlayerUITextRole::Identity);
	EnglishIdentityText = MakeText(
		*WidgetTree, TEXT("CardEnglishIdentity"));
	EnglishIdentityText->SetAutoWrapText(false);
	EnglishIdentityText->SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
	EnglishIdentityText->SetClipping(EWidgetClipping::ClipToBounds);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*EnglishIdentityText, EFMCodexPlayerUITextRole::Kicker);
	FullCardIdentitySupplementText = MakeText(
		*WidgetTree, TEXT("FullCardIdentitySupplement"));
	ConfigureBoundedSingleLine(*FullCardIdentitySupplementText);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*FullCardIdentitySupplementText, EFMCodexPlayerUITextRole::Kicker);
	FSlateFontInfo SupplementFont = FullCardIdentitySupplementText->GetFont();
	SupplementFont.Size = 10;
	SupplementFont.TypefaceFontName = TEXT("Medium");
	FullCardIdentitySupplementText->SetFont(SupplementFont);
	FullCardIdentitySupplementText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xB8, 0xC4, 0xC8))));
	if (UVerticalBoxSlot* NameSlot =
		IdentityTextStack->AddChildToVerticalBox(IdentityText))
	{
		NameSlot->SetPadding(FMargin(10.0f, 4.0f, 34.0f, 1.0f));
	}
	if (UVerticalBoxSlot* EnglishSlot =
		IdentityTextStack->AddChildToVerticalBox(EnglishIdentityText))
	{
		EnglishSlot->SetPadding(FMargin(10.0f, 0.0f, 34.0f, 5.0f));
	}
	if (UVerticalBoxSlot* SupplementSlot =
		IdentityTextStack->AddChildToVerticalBox(
			FullCardIdentitySupplementText))
	{
		SupplementSlot->SetPadding(FMargin(10.0f, 0.0f, 38.0f, 5.0f));
	}
	IdentityTextStack->AddChildToVerticalBox(IdentityAccentBounds);
	if (UOverlaySlot* IdentityTextSlot =
		IdentityBody->AddChildToOverlay(IdentityTextStack))
	{
		IdentityTextSlot->SetHorizontalAlignment(HAlign_Fill);
		IdentityTextSlot->SetVerticalAlignment(VAlign_Fill);
	}
	PlayerFacingSerialText = MakeText(
		*WidgetTree, TEXT("PlayerFacingCardSerial"));
	PlayerFacingSerialText->SetAutoWrapText(false);
	PlayerFacingSerialText->SetJustification(ETextJustify::Right);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PlayerFacingSerialText, EFMCodexPlayerUITextRole::Kicker);
	FSlateFontInfo SerialFont = PlayerFacingSerialText->GetFont();
	SerialFont.Size = 11;
	SerialFont.TypefaceFontName = TEXT("Medium");
	PlayerFacingSerialText->SetFont(SerialFont);
	if (UOverlaySlot* SerialSlot =
		IdentityBody->AddChildToOverlay(PlayerFacingSerialText))
	{
		SerialSlot->SetHorizontalAlignment(HAlign_Right);
		SerialSlot->SetVerticalAlignment(VAlign_Bottom);
		SerialSlot->SetPadding(FMargin(0.0f, 0.0f, 7.0f, 5.0f));
	}
    FullCardAssignedNumberText = MakeText(*WidgetTree, TEXT("FullCardAssignedNumber"));
    ConfigureBoundedSingleLine(*FullCardAssignedNumberText);
    FFMCodexPlayerUIStyle::Get().ApplyText(*FullCardAssignedNumberText, EFMCodexPlayerUITextRole::Identity);
    USizeBox* FullNumberBounds = WidgetTree->ConstructWidget<USizeBox>(
        USizeBox::StaticClass(), TEXT("FullCardNumberPlateBounds"));
    FullNumberBounds->SetWidthOverride(84.f);
    UBorder* FullNumberPlate = MakeFullRegion(*WidgetTree, TEXT("FullCardNumberPlate"),
        FLinearColor::Transparent, FMargin(32,4,4,0));
    FullNumberPlate->SetHorizontalAlignment(HAlign_Fill);
    FullCardAssignedNumberText->SetJustification(ETextJustify::Center);
    FullNumberPlate->SetVerticalAlignment(VAlign_Center);
    FullNumberPlate->AddChild(FullCardAssignedNumberText);
    FullNumberBounds->AddChild(FullNumberPlate);
    if (UOverlaySlot* NumberSlot = IdentityBody->AddChildToOverlay(FullNumberBounds))
    {
        NumberSlot->SetHorizontalAlignment(HAlign_Right);
        NumberSlot->SetVerticalAlignment(VAlign_Fill);
        NumberSlot->SetPadding(FMargin(0, 12, 0, 0));
    }
	IdentityRegion->AddChild(IdentityBody);
	if (UOverlaySlot* IdentitySlot =
		PortraitLayer->AddChildToOverlay(IdentityRegion))
	{
		IdentitySlot->SetHorizontalAlignment(HAlign_Fill);
		IdentitySlot->SetVerticalAlignment(VAlign_Bottom);
		IdentitySlot->SetPadding(FMargin(0.0f));
	}
	TeamText = MakeText(*WidgetTree, TEXT("CardTeam"));
	OwnerText = MakeText(*WidgetTree, TEXT("CardOwner"));
	DeveloperReferenceText = MakeText(
		*WidgetTree, TEXT("CardDeveloperReference"));
	TeamText->SetVisibility(ESlateVisibility::Collapsed);
	OwnerText->SetVisibility(ESlateVisibility::Collapsed);
	DeveloperReferenceText->SetVisibility(ESlateVisibility::Collapsed);
	// Retain the established hidden widget contract for automation and derived
	// widget compatibility. These nodes never participate in player-facing
	// layout, but attaching them keeps GetWidgetFromName safe and deterministic.
	UBorder* HiddenLegacyContent = MakeRegion(*WidgetTree,
		TEXT("CardContentReadabilityLayer"), FLinearColor::Transparent,
		FMargin(0.0f));
	UVerticalBox* HiddenLegacyText = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HiddenLegacyFullCardText"));
	HiddenLegacyText->AddChildToVerticalBox(TeamText);
	HiddenLegacyText->AddChildToVerticalBox(OwnerText);
	HiddenLegacyText->AddChildToVerticalBox(DeveloperReferenceText);
	HiddenLegacyContent->AddChild(HiddenLegacyText);
	HiddenLegacyContent->SetVisibility(ESlateVisibility::Collapsed);
	Body->AddChildToVerticalBox(HiddenLegacyContent);
	PortraitPresentationRegion->AddChild(PortraitLayer);
	PortraitBounds->AddChild(PortraitPresentationRegion);
	Body->AddChildToVerticalBox(PortraitBounds);

	AttributeRegion = MakeFullRegion(*WidgetTree,
		TEXT("AttributePresentationRegion"), FullCardSurfaceColor(),
		FMargin(7.0f, 5.0f));
	UVerticalBox* AttributeBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("AttributePresentationBody"));
    USizeBox* AttributeHeadingBounds = WidgetTree->ConstructWidget<USizeBox>(
        USizeBox::StaticClass(), TEXT("AttributeHeadingBounds"));
	AttributeHeadingBounds->AddChild(MakeFullCardSectionHeading(
		*WidgetTree, TEXT("AttributeSection"),
		FFMCodexPlayerUIPresentationText::FullCardAttributesHeading()));
    AttributeBody->AddChildToVerticalBox(AttributeHeadingBounds);
	AttributeGrid = WidgetTree->ConstructWidget<UUniformGridPanel>(
		UUniformGridPanel::StaticClass(), TEXT("StructuredAttributeGrid"));
	AttributeGrid->SetSlotPadding(FMargin(1.0f));
	AttributeBody->AddChildToVerticalBox(AttributeGrid);
	AttributeRegion->AddChild(AttributeBody);
	Body->AddChildToVerticalBox(AttributeRegion);

	SkillRegion = MakeFullRegion(*WidgetTree,
		TEXT("SkillPresentationRegion"),
		FullCardSurfaceColor(), FMargin(7.0f, 4.0f));
	UVerticalBox* SkillBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SkillPresentationBody"));
    USizeBox* SkillHeadingBounds = WidgetTree->ConstructWidget<USizeBox>(
        USizeBox::StaticClass(), TEXT("SkillHeadingBounds"));
	SkillHeadingBounds->AddChild(MakeFullCardSectionHeading(
		*WidgetTree, TEXT("SkillSection"),
		FFMCodexPlayerUIPresentationText::SkillsHeading()));
    SkillBody->AddChildToVerticalBox(SkillHeadingBounds);
	SkillList = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SkillIdentityList"));
	SkillBody->AddChildToVerticalBox(SkillList);
	SkillRegion->AddChild(SkillBody);
	Body->AddChildToVerticalBox(SkillRegion);

	USpacer* SerialAnchorSpacer = WidgetTree->ConstructWidget<USpacer>(
		USpacer::StaticClass(), TEXT("FullCardSerialAnchorSpacer"));
	if (UVerticalBoxSlot* SpacerSlot =
		Body->AddChildToVerticalBox(SerialAnchorSpacer))
	{
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}

    FullCardCollectionText = MakeText(*WidgetTree, TEXT("FullCardCollectionLine"));
    ConfigureBoundedSingleLine(*FullCardCollectionText);
    FFMCodexPlayerUIStyle::Get().ApplyText(*FullCardCollectionText, EFMCodexPlayerUITextRole::Kicker);
    FullCardCollectionText->SetJustification(ETextJustify::Center);
    FSlateFontInfo CollectionFont = FullCardCollectionText->GetFont();
    CollectionFont.Size = 7;
    CollectionFont.LetterSpacing = 130;
    FullCardCollectionText->SetFont(CollectionFont);
    UBorder* CollectionSurface = MakeFullRegion(*WidgetTree, TEXT("FullCardCollectionSurface"),
        FLinearColor::Transparent, FMargin(0));
    CollectionSurface->AddChild(FullCardCollectionText);
    if (UVerticalBoxSlot* CollectionSlot = Body->AddChildToVerticalBox(CollectionSurface))
        CollectionSlot->SetPadding(FMargin(2, 4, 2, 3));

	StatusRegion = MakeRegion(*WidgetTree,
		TEXT("StatusBadgePresentationRegion"), FLinearColor::Transparent,
		FMargin(0.0f));
	StatusBadgeBox = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("StatusBadgeList"));
	StatusRegion->AddChild(StatusBadgeBox);
	StatusRegion->SetVisibility(ESlateVisibility::Collapsed);
	Body->AddChildToVerticalBox(StatusRegion);
	USizeBox* AssignedBounds=WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("AssignedNumberBounds"));
	AssignedBounds->SetWidthOverride(32.f);
	AssignedBounds->SetClipping(EWidgetClipping::ClipToBounds);
	AssignedBounds->AddChild(AssignedNumberText);
	FrameAssetHook->AddChildToOverlay(AssignedBounds);
}

void UFMCodexPlayerCardWidget::RefreshVisuals()
{
	if (IdentityText == nullptr)
	{
		return;
	}

	bPitchMiniPilot = PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
		&& !FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Presentation.CardId).PitchMiniPortrait.IsNull();
	bCanonicalCardFamily = PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro
		&& FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Presentation.CardId).bCanonicalPlayerArt;
	const bool bHandMicro = PresentationMode
		== EFMCodexPlayerCardPresentationMode::HandMicro;
	const bool bPitchMini = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchMini;
	const bool bLegacyCompact = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchCompact;
	const bool bDetailed = PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice;
    bFullCardPilot = bDetailed && FFMCodexPlayerUIAssetReferences::Get()
        .ResolveCardArt(Presentation.CardId).bCanonicalPlayerArt;
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
	CardFrame->SetBrush(FSlateBrush());
	CardFrame->SetPadding(bHandMicro ? FMargin(0.0f)
		: bPitchMini ? FMargin(3.0f) : bDetailed ? FMargin(2.0f)
			: Style.GetSectionPadding());
	HandMicroVisualSystem->SetVisibility(bHandMicro
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PitchMiniContent->SetVisibility(bPitchMini
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	DetailedContentLayer->SetVisibility(bHandMicro || bPitchMini
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	HeaderRegion->SetVisibility(bDetailed
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	PortraitBounds->SetHeightOverride(
		bHandMicro ? 18.0f : bPitchMini ? 52.0f
			: bLegacyCompact ? 32.0f
				: FMCodexPlayerCardWidget::FullCardHeroHeight);
	SkillRegion->SetVisibility(bDetailed || bLegacyCompact
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	AttributeRegion->SetVisibility(bDetailed || bLegacyCompact
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	StatusRegion->SetVisibility(bLegacyCompact
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
			? BaseFrame
			: bDetailed ? RarityAccent : BaseFrame);
	const bool bHasPitchMiniOwnershipAccent = bPitchMini
		&& Presentation.bHasPitchMiniOwnershipAccent
		&& Presentation.PitchMiniOwnershipAccentEdge
			!= EFMCodexUMGPitchMiniOwnershipEdge::None;
	// All Pitch cards share one native ownership outline, including legacy art.
	PitchMiniOwnershipRailLeft->SetVisibility(ESlateVisibility::Collapsed);
	PitchMiniOwnershipRailRight->SetVisibility(ESlateVisibility::Collapsed);
	const bool bShowSelectedRole = bPitchMini
		&& Presentation.SelectedRole != EFMCodexUMGSelectedRole::None
		&& !Presentation.SelectedRoleLabel.IsEmpty();
	PitchMiniSelectedRoleText->SetText(
		FText::FromString(Presentation.SelectedRoleLabel));
	PitchMiniSelectedRoleTag->SetVisibility(bShowSelectedRole
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const bool bPitchMiniTacticalMatchCountValid = ensureAlwaysMsgf(
		Presentation.PitchMiniTacticalMatchCount >= 0
			&& Presentation.PitchMiniTacticalMatchCount <= 2,
		TEXT("Pitch Mini tactical-match presentation count must be 0..2, got %d"),
		Presentation.PitchMiniTacticalMatchCount);
	const bool bPitchMiniTacticalMatchStateConsistent = ensureAlwaysMsgf(
		!bPitchMiniTacticalMatchCountValid
			|| Presentation.bHasPitchMiniTacticalMatch
				== (Presentation.PitchMiniTacticalMatchCount > 0),
		TEXT("Pitch Mini tactical-match count and highlight state disagree"));
	const int32 PitchMiniTacticalMatchCount = bPitchMini
		&& bPitchMiniTacticalMatchCountValid
		&& bPitchMiniTacticalMatchStateConsistent
		? Presentation.PitchMiniTacticalMatchCount : 0;
	const bool bHandMicroTacticalMatchCountValid = ensureAlwaysMsgf(
		Presentation.HandMicroTacticalMatchCount >= 0
			&& Presentation.HandMicroTacticalMatchCount <= 2,
		TEXT("Hand Micro tactical-match presentation count must be 0..2, got %d"),
		Presentation.HandMicroTacticalMatchCount);
	const bool bHandMicroTacticalMatchStateConsistent = ensureAlwaysMsgf(
		!bHandMicroTacticalMatchCountValid
			|| Presentation.bHasHandMicroTacticalMatch
				== (Presentation.HandMicroTacticalMatchCount > 0),
		TEXT("Hand Micro tactical-match count and visibility state disagree"));
	const int32 HandMicroTacticalMatchCount = bHandMicro
		&& bHandMicroTacticalMatchCountValid
		&& bHandMicroTacticalMatchStateConsistent
		? Presentation.HandMicroTacticalMatchCount : 0;
	const int32 VisibleTacticalMatchPipCount = bPitchMini
		? PitchMiniTacticalMatchCount : HandMicroTacticalMatchCount;
	FLinearColor TacticalMatchAccent =
		FLinearColor::FromSRGBColor(FColor(0x8F, 0xE6, 0xC2));
	TacticalMatchAccent.A = 0.88f;
	FLinearColor TacticalMatchGlow = TacticalMatchAccent;
	TacticalMatchGlow.A = 0.09f;
	for (UBorder* Segment : PitchMiniTacticalMatchGlowSegments)
	{
		if (Segment != nullptr)
		{
			Segment->SetBrushColor(TacticalMatchGlow);
			Segment->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	for (UBorder* Segment : PitchMiniTacticalMatchStrokeSegments)
	{
		if (Segment != nullptr)
		{
			Segment->SetBrushColor(TacticalMatchAccent);
			Segment->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	FLinearColor TacticalMatchPipAccent = TacticalMatchAccent;
	TacticalMatchPipAccent.A = 0.96f;
	PitchMiniTacticalMatchPipTop->SetBrushColor(
		TacticalMatchPipAccent);
	PitchMiniTacticalMatchPipBottom->SetBrushColor(
		TacticalMatchPipAccent);
	TacticalMatchPipGroupBounds->SetVisibility(
		VisibleTacticalMatchPipCount > 0
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);
	PitchMiniTacticalMatchPipTop->SetVisibility(
		VisibleTacticalMatchPipCount >= 1
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PitchMiniTacticalMatchPipBottom->SetVisibility(
		VisibleTacticalMatchPipCount == 2
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	const FLinearColor PitchMiniPortraitBase =
		FLinearColor::FromSRGBColor(FColor(0x14, 0x2B, 0x36));
	const FLinearColor PitchMiniWashBase =
		FLinearColor::FromSRGBColor(FColor(0x09, 0x22, 0x2E));
	FLinearColor PitchMiniFallbackColor = PitchMiniPortraitBase;
	FLinearColor PitchMiniWashColor = PitchMiniWashBase;
	FLinearColor PitchMiniFallbackAtmosphereColor =
		FLinearColor::FromSRGBColor(FColor(0x2A, 0x4A, 0x58));
	if (bHasPitchMiniOwnershipAccent)
	{
		PitchMiniFallbackColor = FLinearColor::LerpUsingHSV(
			PitchMiniPortraitBase,
			Presentation.PitchMiniOwnershipAccentColor, 0.12f);
		PitchMiniWashColor = FLinearColor::LerpUsingHSV(PitchMiniWashBase,
			Presentation.PitchMiniOwnershipAccentColor, 0.08f);
		PitchMiniFallbackAtmosphereColor = FLinearColor::LerpUsingHSV(
			PitchMiniFallbackAtmosphereColor,
			Presentation.PitchMiniOwnershipAccentColor, 0.18f);
	}
	PitchMiniFallbackColor.A = 1.0f;
	PitchMiniWashColor.A = 0.12f;
	PitchMiniFallbackAtmosphereColor.A = 0.24f;
	PitchMiniPortraitFallback->SetBrushColor(PitchMiniFallbackColor);
	PitchMiniPortraitFallbackAtmosphere->SetBrushColor(
		PitchMiniFallbackAtmosphereColor);
	PitchMiniPortraitTonalWash->SetBrushColor(PitchMiniWashColor);
	if (FullCardBaseSurface != nullptr)
	{
		FullCardBaseSurface->SetBrushColor(
			FMCodexPlayerCardWidget::FullCardSurfaceColor());
	}
	if (FullCardInnerFrame != nullptr)
	{
		FLinearColor InnerEdge = FLinearColor::LerpUsingHSV(
			FLinearColor(0.30f, 0.38f, 0.43f, 1.0f), RarityAccent, 0.12f);
		InnerEdge.A = 0.48f;
		FullCardInnerFrame->SetBrushColor(InnerEdge);
	}
	if (FullCardRarityRail != nullptr)
	{
		FullCardRarityRail->SetBrushColor(RarityAccent);
	}
	if (FullCardIdentityAccent != nullptr)
	{
		FLinearColor IdentityAccent = RarityAccent;
		IdentityAccent.A = 0.30f;
		FullCardIdentityAccent->SetBrushColor(IdentityAccent);
	}
	RoleIconHook->SetBrushColor(
		FLinearColor(0.015f, 0.035f, 0.052f, 0.58f));

	const FText FullPlayerName = bDetailed
		? FFMCodexPlayerUIPresentationText::InMatchShortPlayerName(
			Presentation.CardId, Presentation.IdentityLabel)
		: FFMCodexPlayerUIPresentationText::PlayerName(
		Presentation.CardId, Presentation.IdentityLabel);
	const bool bHasFullPlayerName = !FullPlayerName.IsEmpty();
	IdentityText->SetText(FullPlayerName);
	IdentityText->SetVisibility(!bDetailed || bHasFullPlayerName
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	FSlateFontInfo FullCardNameFont = IdentityText->GetFont();
	FullCardNameFont.Size = FMCodexPlayerCardWidget::GetMeasuredSingleLineFontSize(
		FullPlayerName, FullCardNameFont, 278.0f, 24, 18);
	FullCardNameFont.TypefaceFontName = TEXT("Medium");
	IdentityText->SetFont(FullCardNameFont);
	IdentityText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xF2, 0xF3, 0xF1))));
	EnglishIdentityText->SetText(FText::FromString(
		Presentation.EnglishIdentityLabel));
	FSlateFontInfo EnglishNameFont = EnglishIdentityText->GetFont();
	EnglishNameFont.Size =
		FMCodexPlayerCardWidget::GetMeasuredSingleLineFontSize(
			EnglishIdentityText->GetText(), EnglishNameFont, 222.0f, 13, 10);
	EnglishNameFont.TypefaceFontName = TEXT("Medium");
	EnglishIdentityText->SetFont(EnglishNameFont);
	EnglishIdentityText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xC1, 0xCB, 0xCF))));
	EnglishIdentityText->SetVisibility(ESlateVisibility::Collapsed);
	FullCardIdentitySupplementText->SetText(
		FFMCodexPlayerUIPresentationText::FullCardIdentitySupplement(
			Presentation.NationalityLabel, Presentation.ClubLabel));
	FullCardIdentitySupplementText->SetVisibility(
		bDetailed && !FullCardIdentitySupplementText->GetText().IsEmpty()
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	IdentityRegion->SetVisibility(!bDetailed
		|| bHasFullPlayerName
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
	TeamText->SetText(FFMCodexPlayerUIPresentationText::TeamName(
		Presentation.CardId));
	TeamText->SetVisibility(ESlateVisibility::Collapsed);
	OwnerText->SetText(FFMCodexPlayerUIPresentationText::Owner(
		Presentation.OwnerLabel));
	OwnerText->SetVisibility(ESlateVisibility::Collapsed);
	RoleText->SetText(bDetailed
		? FFMCodexPlayerUIPresentationText::InMatchCompactRole(
			Presentation.RoleLabel)
		: FFMCodexPlayerUIPresentationText::Role(Presentation.RoleLabel));
	RoleText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xEE, 0xF1, 0xF0))));
	HandMicroRoleText->SetText(
		FFMCodexPlayerUIPresentationText::HandMicroCompactRole(
			Presentation.RoleLabel));
	const FText PitchMiniRole =
		FFMCodexPlayerUIPresentationText::PitchMiniCompactRole(
			Presentation.RoleLabel);
	PitchMiniRoleText->SetText(PitchMiniRole);
	PitchMiniIdentitySeparatorText->SetText(FText::FromString(TEXT("|")));
	FSlateFontInfo PitchMiniRoleFont = PitchMiniRoleText->GetFont();
	PitchMiniRoleFont.Size =
		FMCodexPlayerCardWidget::PitchMiniRoleFontSize;
	PitchMiniRoleFont.TypefaceFontName = TEXT("Medium");
	PitchMiniRoleText->SetFont(PitchMiniRoleFont);
	FSlateFontInfo PitchMiniSeparatorFont =
		PitchMiniIdentitySeparatorText->GetFont();
	PitchMiniSeparatorFont.Size =
		FMCodexPlayerCardWidget::PitchMiniRoleFontSize;
	PitchMiniSeparatorFont.TypefaceFontName = TEXT("Medium");
	PitchMiniIdentitySeparatorText->SetFont(PitchMiniSeparatorFont);
	float RoleWidth = 24.0f;
	float SeparatorWidth = 4.0f;
	FMCodexPlayerCardWidget::TryMeasureHandMicroName(
		PitchMiniRole, PitchMiniRoleFont, RoleWidth);
	FMCodexPlayerCardWidget::TryMeasureHandMicroName(
		PitchMiniIdentitySeparatorText->GetText(),
		PitchMiniSeparatorFont, SeparatorWidth);
	const float PitchMiniNameSafeWidth = FMath::Max(1.0f,
		FMCodexPlayerCardWidget::PitchMiniInteriorWidth - 6.0f
			- RoleWidth - SeparatorWidth - 4.0f - 1.0f);
	FSlateFontInfo PitchMiniNameFont = PitchMiniIdentityText->GetFont();
	// Measure the final typeface, including its localized fallback glyph metrics.
	PitchMiniNameFont.TypefaceFontName = TEXT("Medium");
	PitchMiniNameFont.Size =
		FMCodexPlayerCardWidget::GetMeasuredSingleLineFontSize(
		PlayerName, PitchMiniNameFont, PitchMiniNameSafeWidth,
		FMCodexPlayerCardWidget::PitchMiniNameMaximumFontSize,
		FMCodexPlayerCardWidget::PitchMiniNameMinimumFontSize);
	PitchMiniNameFont.TypefaceFontName = TEXT("Medium");
	PitchMiniIdentityText->SetText(PlayerName);
	PitchMiniIdentityText->SetFont(PitchMiniNameFont);
	PitchMiniIdentityText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xF2, 0xF3, 0xF1))));
	PitchMiniRoleText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xC8, 0xD4, 0xD8))));
	PitchMiniIdentitySeparatorText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0x62, 0x75, 0x7D))));
	RarityText->SetText(
		bDetailed ? FText::GetEmpty()
			: FFMCodexPlayerUIPresentationText::Rarity(
				Presentation.RarityLabel));
	RarityText->SetVisibility(bDetailed
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	RarityText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xA8, 0xB4, 0xB9))));
	const bool bShowOverall = bDetailed && Presentation.bHasOverallRating
		&& Presentation.OverallRating > 0;
	OverallNumberText->SetText(FText::AsNumber(Presentation.OverallRating));
	OverallNumberText->SetColorAndOpacity(FSlateColor(RarityAccent));
	OverallNumberText->SetVisibility(bShowOverall
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	OverallLabelText->SetVisibility(bShowOverall
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	OverallLabelText->SetColorAndOpacity(FSlateColor(
		FLinearColor::FromSRGBColor(FColor(0xD4, 0xD9, 0xD8))));
	HandMicroRarityAccent->SetBrushColor(HandRarityColor);
	PortraitPlaceholderText->SetText(
		FFMCodexPlayerUIPresentationText::PortraitPlaceholder());
	RefreshPresentationArt();
	RefreshBiography();
	DeveloperReferenceText->SetText(FText::FromString(
		Presentation.DeveloperReferenceLabel));
	DeveloperReferenceText->SetVisibility(ESlateVisibility::Collapsed);
	PlayerFacingSerialText->SetText(FText::FromString(
		Presentation.PlayerFacingSerialLabel));
	PlayerFacingSerialText->SetColorAndOpacity(FSlateColor(RarityAccent));
	PlayerFacingSerialText->SetVisibility(
		bDetailed && !Presentation.PlayerFacingSerialLabel.IsEmpty()
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);

	RefreshSkills();
	RefreshAttributes();
	RefreshStatusBadges();
	RefreshPilotSurfaces();
    RefreshFullCardPilot();
    // Shared information-row/chrome refinement also supports unmigrated portraits.
    if (UBorder* Identity = Cast<UBorder>(GetWidgetFromName(TEXT("PitchMiniIdentitySurface"))))
        Identity->SetBrushColor(FLinearColor::FromSRGBColor(FColor(7, 21, 34)));
    if (bPitchMini)
    {
        CardFrame->SetBrush(FSlateRoundedBoxBrush(FLinearColor::White, 5.f));
        CardFrame->SetBrushColor(FLinearColor::FromSRGBColor(FColor(5, 14, 23)));
        PitchMiniPortraitTonalWash->SetBrushColor(FLinearColor::Transparent);
    }
}

void UFMCodexPlayerCardWidget::RefreshPresentationArt()
{
	const FFMCodexPlayerUICardArtReferences Art =
		FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(
			Presentation.CardId);
	ResolvedArtIdentity = Art.ArtIdentity;
	const bool bCanonicalHand = Art.bCanonicalPlayerArt
		&& PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro;
	const bool bCanonicalPitch = PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
		&& !Art.PitchMiniPortrait.IsNull();
	const bool bPitchMiniPurpose = PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini;
	const bool bCanonicalFull = Art.bCanonicalPlayerArt
        && PresentationMode == EFMCodexPlayerCardPresentationMode::InteractionChoice;
    const bool bPurposeIsolated = bCanonicalHand || bPitchMiniPurpose || bCanonicalFull;
	ResolvedCardFrameTexture = bPurposeIsolated || Art.CardFrame.IsNull()
		? nullptr : Art.CardFrame.LoadSynchronous();
	const bool bPrototypePlayer = Presentation.CardId.ToString().StartsWith(
		TEXT("Prototype."));
	TSoftObjectPtr<UTexture2D> ActivePortrait = Art.Portrait;
	if (bCanonicalHand || bCanonicalPitch) ActivePortrait.Reset();
	if (bCanonicalPitch) ActivePortrait = Art.PitchMiniPortrait;
	if (PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice)
	{
		ActivePortrait = !Art.FullCardPortrait.IsNull()
			? Art.FullCardPortrait
			: (bPrototypePlayer ? TSoftObjectPtr<UTexture2D>() : Art.Portrait);
	}
	ResolvedPortraitTexture = ActivePortrait.IsNull()
		? nullptr : ActivePortrait.LoadSynchronous();
	ResolvedHandMicroPortraitTexture = (bPitchMiniPurpose || bCanonicalFull) ? nullptr : Art.HandMicroPortrait.IsNull()
		? (bPrototypePlayer ? nullptr : ResolvedPortraitTexture.Get())
		: Art.HandMicroPortrait.LoadSynchronous();
	ResolvedRoleIconTexture = bPurposeIsolated || Art.RoleIcon.IsNull()
		? nullptr : Art.RoleIcon.LoadSynchronous();
	ResolvedLongShotSkillIconTexture = bPurposeIsolated || Art.LongShotSkillIcon.IsNull()
		? nullptr : Art.LongShotSkillIcon.LoadSynchronous();

	if (!bPurposeIsolated && !Art.CardFrame.IsNull() && ResolvedCardFrameTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional card-frame asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.CardFrame.ToSoftObjectPath().ToString());
	}
	if (!ActivePortrait.IsNull() && ResolvedPortraitTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional portrait asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*ActivePortrait.ToSoftObjectPath().ToString());
	}
	if (!bPitchMiniPurpose && !bCanonicalFull && !Art.HandMicroPortrait.IsNull()
		&& ResolvedHandMicroPortraitTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional Hand Micro portrait asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.HandMicroPortrait.ToSoftObjectPath().ToString());
		ResolvedHandMicroPortraitTexture = ResolvedPortraitTexture;
	}
	if (!bPurposeIsolated && !Art.RoleIcon.IsNull() && ResolvedRoleIconTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional role-icon asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.RoleIcon.ToSoftObjectPath().ToString());
	}
	if (!bPurposeIsolated && !Art.LongShotSkillIcon.IsNull()
		&& ResolvedLongShotSkillIconTexture == nullptr)
	{
		UE_LOG(LogFMCodexPlayerCardArt, Warning,
			TEXT("Optional skill-icon asset failed to load for %s: %s"),
			*Presentation.CardId.ToString(),
			*Art.LongShotSkillIcon.ToSoftObjectPath().ToString());
	}

	if (bPurposeIsolated)
	{
		CardFrameImage->SetBrushFromTexture(nullptr);
		PortraitImage->SetBrushFromTexture(nullptr);
		PitchMiniPortraitImage->SetBrushFromTexture(nullptr);
		HandMicroPortraitImage->SetBrushFromTexture(nullptr);
		RoleIconImage->SetBrushFromTexture(nullptr);
	}
	const bool bHasFrame = ResolvedCardFrameTexture != nullptr;
	const bool bUseFrameTexture = bHasFrame
		&& PresentationMode != EFMCodexPlayerCardPresentationMode::PitchMini
		&& PresentationMode != EFMCodexPlayerCardPresentationMode::HandMicro
		&& PresentationMode
			!= EFMCodexPlayerCardPresentationMode::InteractionChoice;
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
		PortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, false);
		FSlateBrush FullCardPortraitBrush = PortraitImage->GetBrush();
		FullCardPortraitBrush.DrawAs = ESlateBrushDrawType::Image;
		// Full Card artwork is a vertical 2:3 source. The full-width, lower bust
		// window is ratio-matched to the 320 px hero: the face remains strong while
		// more shoulder, neckline and upper-chest art continues beneath the identity
		// scrim. Hand Micro retains its separate Runtime192 source and contract.
		FullCardPortraitBrush.SetUVRegion(FBox2f(
			FVector2f(FMCodexPlayerCardWidget::FullCardPortraitLeft,
				FMCodexPlayerCardWidget::FullCardPortraitTop),
			FVector2f(FMCodexPlayerCardWidget::FullCardPortraitRight,
				FMCodexPlayerCardWidget::FullCardPortraitBottom)));
		PortraitImage->SetBrush(FullCardPortraitBrush);
		PitchMiniPortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, false);
		FSlateBrush PitchMiniPortraitBrush = PitchMiniPortraitImage->GetBrush();
		PitchMiniPortraitBrush.DrawAs = ESlateBrushDrawType::Image;
		const FBox2f PitchMiniHeroCrop = CalculatePitchMiniHeroCrop(
			ResolvedPortraitTexture->GetImportedSize());
		if (PitchMiniHeroCrop.bIsValid)
		{
			PitchMiniPortraitBrush.SetUVRegion(PitchMiniHeroCrop);
		}
		PitchMiniPortraitImage->SetBrush(PitchMiniPortraitBrush);
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
    CastChecked<UFMCodexFullCardSurface>(HandMicroPortraitFallback)->SetSurface(
        EFMCodexFullCardSurface::MissingPortrait,FLinearColor::White);
    CastChecked<UFMCodexFullCardSurface>(PitchMiniPortraitFallback)->SetSurface(
        EFMCodexFullCardSurface::MissingPortrait,FLinearColor::White);
    GetWidgetFromName(TEXT("FullPortraitFallback"))->SetVisibility(!bHasPortrait
        && PresentationMode == EFMCodexPlayerCardPresentationMode::InteractionChoice
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PortraitPlaceholderText->SetVisibility(ESlateVisibility::Collapsed);

	if (bPurposeIsolated)
	{
		if (PresentationMode != EFMCodexPlayerCardPresentationMode::InteractionChoice)
			PortraitImage->SetBrushFromTexture(nullptr);
		if (PresentationMode != EFMCodexPlayerCardPresentationMode::PitchMini
			&& PresentationMode != EFMCodexPlayerCardPresentationMode::PitchCompact)
			PitchMiniPortraitImage->SetBrushFromTexture(nullptr);
	}
	if (bCanonicalPitch)
    {
        // No hidden Hand/Full brush keeps another purpose's portrait alive.
        PortraitImage->SetBrushFromTexture(nullptr);
        HandMicroPortraitImage->SetBrushFromTexture(nullptr);
    }
	const bool bHasRoleIcon = ResolvedRoleIconTexture != nullptr
		&& PresentationMode
			!= EFMCodexPlayerCardPresentationMode::InteractionChoice;
	RoleIconImage->SetVisibility(bHasRoleIcon
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (USizeBox* RoleIconBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("RoleIconAssetBounds"))))
	{
		RoleIconBounds->SetVisibility(bHasRoleIcon
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (bHasRoleIcon)
	{
		RoleIconImage->SetBrushFromTexture(ResolvedRoleIconTexture, true);
	}
}

void UFMCodexPlayerCardWidget::RefreshBiography()
{
	using namespace FMCodexPlayerCardWidget;
	if (BiographyList == nullptr || BiographyRegion == nullptr)
	{
		return;
	}
	BiographyList->ClearChildren();
	RenderedBiographyRowCount = 0;
	const bool bDetailed = PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice;
	auto AddBiographyRow = [this](const FName RowName,
		const FText& Label, const FText& Value, const bool bPrimary)
	{
		USizeBox* RowBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(*(RowName.ToString() + TEXT("Bounds"))));
		RowBounds->SetHeightOverride(bFullCardPilot && Presentation.Skills.Num() >= 3 ? 32.f : 34.f);
		UBorder* RowSurface = FMCodexPlayerCardWidget::MakeRegion(
			*WidgetTree, RowName, FLinearColor::Transparent,
			bFullCardPilot ? FMargin(0,0) : FMargin(2,2));
		UVerticalBox* Copy = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*(RowName.ToString() + TEXT("Copy"))));
		UTextBlock* LabelText = FMCodexPlayerCardWidget::MakeText(
			*WidgetTree, FName(*(RowName.ToString() + TEXT("Label"))));
		LabelText->SetText(Label);
		LabelText->SetAutoWrapText(false);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LabelText, EFMCodexPlayerUITextRole::Kicker);
		FSlateFontInfo LabelFont = LabelText->GetFont();
		LabelFont.Size = 9;
		LabelFont.TypefaceFontName = TEXT("Medium");
		LabelText->SetFont(LabelFont);
		LabelText->SetColorAndOpacity(FSlateColor(
			FLinearColor::FromSRGBColor(FColor(0x99, 0xA8, 0xAE))));
		UTextBlock* ValueText = FMCodexPlayerCardWidget::MakeText(
			*WidgetTree, FName(*(RowName.ToString() + TEXT("Value"))));
		ValueText->SetText(Value);
		ValueText->SetAutoWrapText(false);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*ValueText, EFMCodexPlayerUITextRole::Body);
		FSlateFontInfo ValueFont = ValueText->GetFont();
		// A one-point date adjustment preserves six-unit padding in the narrow Full bio.
		ValueFont.Size = bPrimary && !bFullCardPilot ? 13 : 12;
		ValueFont.TypefaceFontName = TEXT("Medium");
		ValueText->SetFont(ValueFont);
		ValueText->SetColorAndOpacity(FSlateColor(
			FLinearColor::FromSRGBColor(FColor(0xE0, 0xE6, 0xE7))));
		Copy->AddChildToVerticalBox(LabelText);
		Copy->AddChildToVerticalBox(ValueText);
		RowSurface->AddChild(Copy);
		RowBounds->AddChild(RowSurface);
		if (UVerticalBoxSlot* RowSlot =
			BiographyList->AddChildToVerticalBox(RowBounds))
		{
			RowSlot->SetPadding(FMargin(0.0f));
		}
		++RenderedBiographyRowCount;
	};
	auto AddBiographyDivider = [this](const FName DividerName)
	{
		USizeBox* DividerBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(*(DividerName.ToString() + TEXT("Bounds"))));
		DividerBounds->SetHeightOverride(1.0f);
		DividerBounds->AddChild(FMCodexPlayerCardWidget::MakeRegion(
			*WidgetTree, DividerName,
			FLinearColor(0.38f, 0.46f, 0.50f, 0.24f), FMargin(0.0f)));
		if (UVerticalBoxSlot* DividerSlot =
			BiographyList->AddChildToVerticalBox(DividerBounds))
		{
			DividerSlot->SetPadding(bFullCardPilot
                ? (Presentation.Skills.Num() >= 3 ? FMargin(2,1,2,1) : FMargin(2,3,2,3))
                : FMargin(3,3,3,2));
		}
	};
    if (bDetailed)
    {
        const FText Missing = NSLOCTEXT("FMCodexPlayerUI", "MissingBioValue", "—");
        FString DisplayDate = Presentation.BirthDate;
        DisplayDate.ReplaceInline(TEXT("-"), TEXT("."));
        AddBiographyRow(TEXT("BiographyBirthDate"), FFMCodexPlayerUIPresentationText::BirthDateHeading(),
            DisplayDate.IsEmpty() ? Missing : FText::FromString(DisplayDate), true);
        AddBiographyDivider(TEXT("BiographyHeightDivider"));
        AddBiographyRow(TEXT("BiographyHeight"), FFMCodexPlayerUIPresentationText::HeightHeading(),
            Presentation.HeightCm > 0 ? FText::Format(NSLOCTEXT("FMCodexPlayerUI", "BioCentimetres", "{0} cm"),
                FText::AsNumber(Presentation.HeightCm)) : Missing, false);
        AddBiographyDivider(TEXT("BiographyWeightDivider"));
        AddBiographyRow(TEXT("BiographyWeight"), FFMCodexPlayerUIPresentationText::WeightHeading(),
            Presentation.WeightKg > 0 ? FText::Format(NSLOCTEXT("FMCodexPlayerUI", "BioKilograms", "{0} kg"),
                FText::AsNumber(Presentation.WeightKg)) : Missing, false);
        AddBiographyDivider(TEXT("BiographyPositionDivider"));
        const FText Position = FFMCodexPlayerUIPresentationText::InMatchCompactRole(Presentation.RoleLabel);
        AddBiographyRow(TEXT("BiographyPosition"), FFMCodexPlayerUIPresentationText::FullCardPositionTypeHeading(),
            Position.IsEmpty() ? Missing : Position, false);
    }
	const ESlateVisibility BiographyVisibility =
		RenderedBiographyRowCount > 0
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	BiographyRegion->SetVisibility(BiographyVisibility);
	if (USizeBox* BiographyBounds = Cast<USizeBox>(
		GetWidgetFromName(TEXT("InMatchFullCardBiographyBounds"))))
	{
		BiographyBounds->SetVisibility(BiographyVisibility);
	}
}

void UFMCodexPlayerCardWidget::RefreshSkills()
{
	using namespace FMCodexPlayerCardWidget;
	SkillList->ClearChildren();
	RenderedSkillTexts.Reset();
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro)
	{
		return;
	}
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini)
	{
		return;
	}
	const bool bDetailed = PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice;
	if (bDetailed)
	{
		TArray<FFMCodexUMGSkillViewModel> Skills = Presentation.Skills;
		if (Skills.IsEmpty())
		{
			for (const FString& LegacySkill : Presentation.SkillLabels)
			{
				if (!LegacySkill.IsEmpty() && LegacySkill != TEXT("NO SKILL"))
				{
					FFMCodexUMGSkillViewModel& Skill =
						Skills.AddDefaulted_GetRef();
					Skill.CanonicalLabel = LegacySkill;
				}
			}
		}
		SkillRegion->SetVisibility(Skills.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		for (int32 Index = 0; Index < Skills.Num(); ++Index)
		{
			const FFMCodexUMGSkillViewModel& Skill = Skills[Index];
			USizeBox* RowBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("FullCardSkillRowBounds%d"), Index)));
			RowBounds->SetHeightOverride(bFullCardPilot ? 24.0f : 28.0f);
			UBorder* RowSurface = MakeFullRegion(*WidgetTree,
				FName(*FString::Printf(TEXT("FullCardSkillRow%d"), Index)),
				FLinearColor::FromSRGBColor(FColor(0x08, 0x1A, 0x26)),
				FMargin(5.0f, 3.0f));
            if (bFullCardPilot)
            {
                RowSurface->SetPadding(FMargin(4, 0));
                SetFullSurface(RowSurface, true, EFMCodexFullCardSurface::SkillRow,
                    GetHandMicroRarityBaseColor(Presentation.RarityLabel));
            }
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
				UHorizontalBox::StaticClass(), FName(*FString::Printf(
					TEXT("FullCardSkillContent%d"), Index)));
			USizeBox* AccentBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("FullCardSkillAccentBounds%d"), Index)));
			AccentBounds->SetWidthOverride(2.0f);
            AccentBounds->SetVisibility(bFullCardPilot ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
			AccentBounds->SetHeightOverride(14.0f);
			AccentBounds->AddChild(MakeRegion(*WidgetTree,
				FName(*FString::Printf(TEXT("FullCardSkillAccent%d"), Index)),
				FLinearColor(0.34f, 0.58f, 0.68f, 0.62f), FMargin(0.0f)));
			if (UHorizontalBoxSlot* AccentSlot =
				Row->AddChildToHorizontalBox(AccentBounds))
			{
				AccentSlot->SetVerticalAlignment(VAlign_Center);
			}
			const bool bHasRange = Skill.MinTriggerActionPoint > 0
				&& Skill.MaxTriggerActionPoint >= Skill.MinTriggerActionPoint;
			if (bHasRange)
			{
				USizeBox* RangeBounds = WidgetTree->ConstructWidget<USizeBox>(
					USizeBox::StaticClass(), FName(*FString::Printf(
						TEXT("FullCardSkillRangeBounds%d"), Index)));
				RangeBounds->SetWidthOverride(bFullCardPilot ? 54.f : 48.f);
                if (bFullCardPilot) RangeBounds->SetHeightOverride(20.f);
				UBorder* RangeBadge = MakeFullRegion(*WidgetTree,
					FName(*FString::Printf(TEXT("FullCardSkillRange%d"), Index)),
					FLinearColor(0.04f, 0.16f, 0.23f, 0.94f),
					FMargin(4.0f, 1.0f));
                if (bFullCardPilot)
                {
                    RangeBadge->SetPadding(FMargin(3, 0));
                    RangeBadge->SetVerticalAlignment(VAlign_Center);
                    SetFullSurface(RangeBadge, true, EFMCodexFullCardSurface::Range,
                        FLinearColor(.12f,.49f,.63f,1));
                }
				UTextBlock* RangeText = MakeText(*WidgetTree,
					FName(*FString::Printf(
						TEXT("FullCardSkillRangeText%d"), Index)));
				RangeText->SetText(FText::FromString(FString::Printf(
					TEXT("%d–%d"), Skill.MinTriggerActionPoint,
					Skill.MaxTriggerActionPoint)));
				RangeText->SetAutoWrapText(false);
				FFMCodexPlayerUIStyle::Get().ApplyText(
					*RangeText, EFMCodexPlayerUITextRole::Kicker);
				FSlateFontInfo RangeFont = RangeText->GetFont();
				RangeFont.Size = bFullCardPilot ? 12 : 13;
				RangeFont.TypefaceFontName = TEXT("Medium");
				RangeText->SetFont(RangeFont);
				RangeText->SetJustification(ETextJustify::Center);
                if (bFullCardPilot) RangeText->SetRenderTranslation(FVector2D(0,.5f));
				RangeBadge->AddChild(RangeText);
				RangeBounds->AddChild(RangeBadge);
				if (UHorizontalBoxSlot* RangeSlot =
					Row->AddChildToHorizontalBox(RangeBounds))
				{
					RangeSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
					RangeSlot->SetVerticalAlignment(VAlign_Center);
				}
			}
			UTextBlock* SkillText = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("SkillIdentity%d"), Index)));
			SkillText->SetText(FFMCodexPlayerUIPresentationText::Skill(
				Skill.CanonicalLabel));
			SkillText->SetAutoWrapText(false);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*SkillText, EFMCodexPlayerUITextRole::Body);
			FSlateFontInfo SkillFont = SkillText->GetFont();
			SkillFont.Size = bFullCardPilot ? 12 : 13;
			SkillFont.TypefaceFontName = TEXT("Medium");
			SkillText->SetFont(SkillFont);
			if (UHorizontalBoxSlot* SkillSlot =
				Row->AddChildToHorizontalBox(SkillText))
			{
				SkillSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				SkillSlot->SetPadding(FMargin(bFullCardPilot ? 8.f : 6.f, 0, 0, 0));
                if (bFullCardPilot) SkillSlot->SetVerticalAlignment(VAlign_Center);
			}
			RowSurface->AddChild(Row);
			RowBounds->AddChild(RowSurface);
			if (UVerticalBoxSlot* RowSlot =
				SkillList->AddChildToVerticalBox(RowBounds))
			{
				RowSlot->SetPadding(FMargin(0.0f, bFullCardPilot
                    ? 2.f : (Index == 0 ? 1.f : 2.f), 0.0f, 0.0f));
			}
			RenderedSkillTexts.Add(SkillText);
		}
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
	RenderedAttributeValueTexts.Reset();
	RenderedAttributeTierColors.Reset();
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
	const bool bDetailed = PresentationMode
		== EFMCodexPlayerCardPresentationMode::InteractionChoice;
	if (bDetailed)
	{
		TArray<FFMCodexUMGAttributeViewModel> Attributes =
			Presentation.AttributeValues;
		if (Attributes.IsEmpty())
		{
			const bool bHasLegacyAttributeData =
				!RenderedAttributeSummary.IsEmpty()
				&& !RenderedAttributeSummary.Equals(
					TEXT("Attributes unavailable"), ESearchCase::IgnoreCase);
			for (const FString& Entry : bHasLegacyAttributeData
				? SplitStatSummary(RenderedAttributeSummary) : TArray<FString>())
			{
				FString Token;
				FString Value;
				if (Entry.Split(TEXT(" "), &Token, &Value,
					ESearchCase::CaseSensitive, ESearchDir::FromStart))
				{
					FFMCodexUMGAttributeViewModel& Attribute =
						Attributes.AddDefaulted_GetRef();
					Attribute.CanonicalLabel = Token;
					Attribute.Value = FCString::Atoi(*Value);
				}
			}
		}
		const TArray<FString> CanonicalOrder = Presentation.bGoalkeeper
			? TArray<FString>({ TEXT("HAN"), TEXT("POS"), TEXT("REF"),
				TEXT("AER"), TEXT("ANT"), TEXT("1V1") })
			: TArray<FString>({ TEXT("SHO"), TEXT("DRI"), TEXT("PAS"),
				TEXT("OFF"), TEXT("MRK"), TEXT("TKL"), TEXT("SPD"),
				TEXT("STR"), TEXT("STA"), TEXT("LS") });
		TArray<FFMCodexUMGAttributeViewModel> OrderedAttributes;
		OrderedAttributes.Reserve(CanonicalOrder.Num());
		for (const FString& CanonicalToken : CanonicalOrder)
		{
			const FFMCodexUMGAttributeViewModel* Match = Attributes.FindByPredicate(
				[&CanonicalToken](
					const FFMCodexUMGAttributeViewModel& Candidate)
				{
					return Candidate.CanonicalLabel.Equals(
						CanonicalToken, ESearchCase::IgnoreCase);
				});
			if (Match != nullptr)
			{
				OrderedAttributes.Add(*Match);
			}
		}
		Attributes = MoveTemp(OrderedAttributes);
		AttributeRegion->SetVisibility(Attributes.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
		AttributeGrid->SetSlotPadding(bFullCardPilot ? FMargin(6.f, 1.f) : FMargin(1.0f));
		for (int32 Index = 0; Index < Attributes.Num(); ++Index)
		{
			const FFMCodexUMGAttributeViewModel& Attribute = Attributes[Index];
			const FLinearColor TierColor = GetAttributeTierColor(Attribute.Value);
			RenderedAttributeTierColors.Add(TierColor);
			USizeBox* StatCellBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeCellBounds%d"), Index)));
			StatCellBounds->SetHeightOverride(bFullCardPilot
                ? (Presentation.bGoalkeeper && RenderedSkillTexts.IsEmpty() ? 32.f : 21.f) : 30.f);
			UBorder* StatCell = MakeFullRegion(*WidgetTree,
				FName(*FString::Printf(TEXT("AttributeCell%d"), Index)),
				FLinearColor::FromSRGBColor(FColor(0x08, 0x1A, 0x26)),
				FMargin(2.0f, 3.0f));
            if (bFullCardPilot)
            {
                StatCell->SetPadding(FMargin(7, 0, 1, 0));
                SetFullSurface(StatCell, true, EFMCodexFullCardSurface::AttributeRow,
                    GetHandMicroRarityBaseColor(Presentation.RarityLabel));
            }
			UHorizontalBox* StatRow = WidgetTree->ConstructWidget<UHorizontalBox>(
				UHorizontalBox::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeRow%d"), Index)));
			USizeBox* TierTickBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeTierTickBounds%d"), Index)));
			TierTickBounds->SetWidthOverride(2.0f);
            TierTickBounds->SetVisibility(bFullCardPilot ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
			TierTickBounds->SetHeightOverride(14.0f);
			FLinearColor TierTickColor = TierColor;
			TierTickColor.A = 0.62f;
			TierTickBounds->AddChild(MakeRegion(*WidgetTree,
				FName(*FString::Printf(TEXT("AttributeTierTick%d"), Index)),
				TierTickColor, FMargin(0.0f)));
			if (UHorizontalBoxSlot* TickSlot =
				StatRow->AddChildToHorizontalBox(TierTickBounds))
			{
				TickSlot->SetVerticalAlignment(VAlign_Center);
			}
			USizeBox* LabelBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeLabelBounds%d"), Index)));
			LabelBounds->SetWidthOverride(
				bFullCardPilot ? 80.0f : Presentation.bGoalkeeper ? 58.0f : 29.0f);
			UTextBlock* LabelText = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("AttributeLabel%d"), Index)));
			LabelText->SetText(
				FFMCodexPlayerUIPresentationText::AttributeLabel(
					Attribute.CanonicalLabel));
			LabelText->SetAutoWrapText(false);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*LabelText, EFMCodexPlayerUITextRole::Body);
			FSlateFontInfo AttributeLabelFont = LabelText->GetFont();
			AttributeLabelFont.Size = bFullCardPilot ? 12 : Presentation.bGoalkeeper ? 13 : 11;
			AttributeLabelFont.TypefaceFontName = TEXT("Medium");
			LabelText->SetFont(AttributeLabelFont);
			LabelText->SetJustification(ETextJustify::Left);
			LabelBounds->AddChild(LabelText);
			if (UHorizontalBoxSlot* LabelSlot =
				StatRow->AddChildToHorizontalBox(LabelBounds))
			{
				LabelSlot->SetPadding(FMargin(
					Presentation.bGoalkeeper ? 5.0f : 3.0f,
					0.0f, 0.0f, 0.0f));
				LabelSlot->SetVerticalAlignment(VAlign_Center);
			}
			USpacer* ValueAnchorSpacer = WidgetTree->ConstructWidget<USpacer>(
				USpacer::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeValueAnchorSpacer%d"), Index)));
			if (UHorizontalBoxSlot* AnchorSlot =
				StatRow->AddChildToHorizontalBox(ValueAnchorSpacer))
			{
				AnchorSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
			FLinearColor BadgeColor = TierColor;
			BadgeColor.A = 0.22f;
			USizeBox* ValueBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("AttributeValueBounds%d"), Index)));
			ValueBounds->SetWidthOverride(
				bFullCardPilot ? 40.0f : Presentation.bGoalkeeper ? 26.0f : 20.0f);
			UBorder* ValueBadge = MakeFullRegion(*WidgetTree,
				FName(*FString::Printf(TEXT("AttributeTierBadge%d"), Index)),
				BadgeColor, FMargin(2.0f, 1.0f));
            if (bFullCardPilot)
            {
                SetFullSurface(ValueBadge, true, EFMCodexFullCardSurface::Value, TierColor);
                ValueBounds->SetHeightOverride(21.f);
                ValueBadge->SetPadding(FMargin(2, 0));
                ValueBadge->SetVerticalAlignment(VAlign_Center);
            }
			UTextBlock* ValueText = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("AttributeValue%d"), Index)));
			ValueText->SetText(FText::AsNumber(Attribute.Value));
			ValueText->SetAutoWrapText(false);
			ValueText->SetJustification(ETextJustify::Center);
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*ValueText, EFMCodexPlayerUITextRole::Body);
			FSlateFontInfo AttributeValueFont = ValueText->GetFont();
			AttributeValueFont.Size = bFullCardPilot ? 13 : Presentation.bGoalkeeper ? 14 : 12;
			AttributeValueFont.TypefaceFontName = TEXT("Bold");
			ValueText->SetFont(AttributeValueFont);
            // Shared font-ink adjustment, including narrow digits; never value-specific.
            if (bFullCardPilot) ValueText->SetRenderTranslation(FVector2D(.5f,.5f));
			ValueText->SetColorAndOpacity(FSlateColor(bFullCardPilot ? FLinearColor(.87f,.91f,.95f,1) : TierColor));
			ValueBadge->AddChild(ValueText);
			ValueBounds->AddChild(ValueBadge);
			if (UHorizontalBoxSlot* ValueSlot =
				StatRow->AddChildToHorizontalBox(ValueBounds))
			{
				ValueSlot->SetVerticalAlignment(VAlign_Center);
			}
			StatCell->AddChild(StatRow);
			StatCellBounds->AddChild(StatCell);
			const int32 ColumnCount = Presentation.bGoalkeeper ? 3 : 5;
            UUniformGridSlot* StatSlot = AttributeGrid->AddChildToUniformGrid(
                StatCellBounds, bFullCardPilot ? Index % ColumnCount : Index / ColumnCount,
                bFullCardPilot ? Index / ColumnCount : Index % ColumnCount);
            if (bFullCardPilot) StatSlot->SetHorizontalAlignment(HAlign_Fill);
			RenderedAttributeTexts.Add(LabelText);
			RenderedAttributeValueTexts.Add(ValueText);
		}
		return;
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
		|| PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini
		|| PresentationMode
			== EFMCodexPlayerCardPresentationMode::InteractionChoice)
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

FText UFMCodexPlayerCardWidget::GetRenderedAssignedNumber() const
{
	return AssignedNumberText ? AssignedNumberText->GetText() : FText::GetEmpty();
}

void UFMCodexPlayerCardWidget::RefreshPilotSurfaces()
{
	if (UFMCodexPlayerCardSurface* Surface = Cast<UFMCodexPlayerCardSurface>(
		GetWidgetFromName(TEXT("HandMicroIdentitySurface"))))
	{
		Surface->bPremium = bCanonicalCardFamily;
		Surface->InvalidateLayoutAndVolatility();
	}
	if (UWidget* OldChrome=GetWidgetFromName(TEXT("HandMicroSkinChrome")))
		OldChrome->SetVisibility(bCanonicalCardFamily ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (bCanonicalCardFamily)
	{
		CardFrame->SetBrushColor(FLinearColor(.003f,.009f,.014f,1));
		// Hand owns the exact sRGB rarity palette in HandMicro Visual Spec section 15.
        // Pitch/Full continue using their existing general UI accents.
        FLinearColor HandAccent=FMCodexPlayerCardWidget::GetHandMicroRarityBaseColor(Presentation.RarityLabel);
        HandAccent.A=.90f;
        HandMicroRarityAccent->SetBrushColor(HandAccent);
		HandMicroIdentityText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	}
	AssignedNumberText->SetText(FText::FromString(Presentation.AssignedPlayerNumber));
	AssignedNumberText->SetVisibility(PresentationMode == EFMCodexPlayerCardPresentationMode::HandMicro && !Presentation.AssignedPlayerNumber.IsEmpty()
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	const bool Hand=PresentationMode==EFMCodexPlayerCardPresentationMode::HandMicro;
	if (Hand)
	{
		// Keep the frozen content widths, but let the four-sided frame own rarity.
		HandMicroRarityAccent->SetVisibility(bCanonicalCardFamily
			? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
		if (UWidget* TechLine=GetWidgetFromName(TEXT("HandMicroIdentityTechLine")))
			TechLine->SetVisibility(bCanonicalCardFamily
				? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
	FSlateFontInfo Font=AssignedNumberText->GetFont(); Font.Size=16;
	AssignedNumberText->SetFont(Font);
    AssignedNumberText->SetTextOverflowPolicy(Hand ? ETextOverflowPolicy::Ellipsis : ETextOverflowPolicy::Clip);
    if (UHorizontalBoxSlot* RoleSlot=Cast<UHorizontalBoxSlot>(HandMicroRoleText->Slot))
    {
        RoleSlot->SetSize(FSlateChildSize(bCanonicalCardFamily ? ESlateSizeRule::Fill : ESlateSizeRule::Automatic));
        RoleSlot->SetPadding(FMargin(0,0,Hand && !Presentation.AssignedPlayerNumber.IsEmpty() ? 36.f : 0.f,0));
    }
    if (UWidget* Meta=GetWidgetFromName(TEXT("HandMicroPositionLine")))
        if (UVerticalBoxSlot* MetaSlot=Cast<UVerticalBoxSlot>(Meta->Slot))
            MetaSlot->SetPadding(FMargin(0,bCanonicalCardFamily ? 4.f : 0.f,0,0));
    if (UBorder* Skin=Cast<UBorder>(GetWidgetFromName(TEXT("HandMicroSkinBackground"))))
        Skin->SetBrushColor(FLinearColor::FromSRGBColor(bCanonicalCardFamily ? FColor(6,18,30) : FColor(0x0C,0x23,0x30)));

	AssignedNumberText->SetColorAndOpacity(FSlateColor(FLinearColor::FromSRGBColor(FColor(0x92,0xAC,0xBF))));
	if (UOverlaySlot* NumberSlot=Cast<UOverlaySlot>(GetWidgetFromName(TEXT("AssignedNumberBounds"))->Slot))
	{
		NumberSlot->SetHorizontalAlignment(HAlign_Right);
		NumberSlot->SetVerticalAlignment(VAlign_Bottom);
		NumberSlot->SetPadding(FMargin(0,0,8.f,8.f));
	}
}

void UFMCodexPlayerCardWidget::RefreshFullCardPilot()
{
    using namespace FMCodexPlayerCardWidget;
    const FLinearColor Accent = GetHandMicroRarityBaseColor(Presentation.RarityLabel);
    FullCardAssignedNumberText->SetText(FText::FromString(Presentation.AssignedPlayerNumber));
    const bool bHasNumber = PresentationMode == EFMCodexPlayerCardPresentationMode::InteractionChoice && !Presentation.AssignedPlayerNumber.IsEmpty();
    FullCardAssignedNumberText->SetVisibility(bHasNumber
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    GetWidgetFromName(TEXT("FullCardNumberPlateBounds"))->SetVisibility(bHasNumber
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    SetFullSurface(CastChecked<UBorder>(GetWidgetFromName(TEXT("FullCardNumberPlate"))),
        bHasNumber, EFMCodexFullCardSurface::Number, Accent);
    SetFullSurface(BiographyRegion, bFullCardPilot, EFMCodexFullCardSurface::Biography, Accent);
    SetFullSurface(IdentityRegion, bFullCardPilot, EFMCodexFullCardSurface::Identity, Accent);
    SetFullSurface(CardFrame, bFullCardPilot, EFMCodexFullCardSurface::Frame, Accent);
    SetFullSurface(FullCardBaseSurface, bFullCardPilot, EFMCodexFullCardSurface::Body, Accent);
    SetFullSurface(AttributeRegion, bFullCardPilot, EFMCodexFullCardSurface::Attributes, Accent);
    auto* Footer = CastChecked<UBorder>(GetWidgetFromName(TEXT("FullCardCollectionSurface")));
    SetFullSurface(Footer, bFullCardPilot, EFMCodexFullCardSurface::Footer, Accent);
    const bool bDetailed = PresentationMode == EFMCodexPlayerCardPresentationMode::InteractionChoice;
    Footer->SetVisibility(bDetailed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    for (const TCHAR* Node : {TEXT("AttributeHeadingBounds"),TEXT("SkillHeadingBounds")})
    {
        auto* HeadingBounds = CastChecked<USizeBox>(GetWidgetFromName(Node));
        if (bFullCardPilot) HeadingBounds->SetHeightOverride(26.f);
        else HeadingBounds->ClearHeightOverride();
    }
    const bool bDenseBiography = bFullCardPilot && Presentation.Skills.Num() >= 3;
    auto* BioBounds = CastChecked<USizeBox>(GetWidgetFromName(TEXT("InMatchFullCardBiographyBounds")));
    // Narrow bio plus the user-approved two-unit right shift; retain the top anchor.
    BioBounds->SetWidthOverride(bFullCardPilot ? 96.f : FullCardBiographyWidth);
    if (auto* BioSlot = Cast<UOverlaySlot>(BioBounds->Slot))
        BioSlot->SetPadding(bFullCardPilot ? FMargin(0,bDenseBiography ? 12 : 18,10,0) : FMargin(0,10,5,0));
    BiographyRegion->SetPadding(bFullCardPilot
        ? (bDenseBiography ? FMargin(6,7) : FMargin(6,9,6,10)) : FMargin(7,6));
    SetFullSurface(SkillRegion, bFullCardPilot, EFMCodexFullCardSurface::Section, Accent);
    GetWidgetFromName(TEXT("FullCardIdentityReadabilityScrim"))->SetVisibility(bFullCardPilot
        ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    GetWidgetFromName(TEXT("FullCardIdentityAccentBounds"))->SetVisibility(bFullCardPilot
        ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    // Reset presentation-only typography when this widget is rebound to a legacy card.
    const FLinearColor HeadingInk = bFullCardPilot
        ? FLinearColor(.72f,.81f,.85f,1)
        : FLinearColor::FromSRGBColor(FColor(0xE4,0xE8,0xE7));
    for (const TCHAR* Prefix : {TEXT("AttributeSection"), TEXT("SkillSection")})
    {
        const FString Base(Prefix);
        auto* Title = CastChecked<UTextBlock>(GetWidgetFromName(FName(*(Base + TEXT("Title")))));
        FSlateFontInfo Font = Title->GetFont();
        Font.TypefaceFontName = bFullCardPilot ? TEXT("Bold") : TEXT("Medium");
        Font.LetterSpacing = bFullCardPilot ? 120 : 0;
        Title->SetFont(Font);
        Title->SetColorAndOpacity(FSlateColor(HeadingInk));
        if (auto* TitleSlot = Cast<UHorizontalBoxSlot>(Title->Slot))
            TitleSlot->SetPadding(FMargin(bFullCardPilot ? 14.f : 9.f,0));
        for (const TCHAR* Suffix : {TEXT("RuleLeft"), TEXT("RuleRight")})
        {
            auto* Rule = CastChecked<UBorder>(GetWidgetFromName(FName(*(Base + Suffix))));
            FLinearColor RuleColor = bFullCardPilot ? FMCodexFullCardGeometry::StructuralInk() : FLinearColor(.31f,.40f,.45f,1);
            RuleColor.A = bFullCardPilot ? .28f : .42f;
            Rule->SetBrushColor(RuleColor);
            SetFullSurface(Rule, bFullCardPilot, FString(Suffix).EndsWith(TEXT("Left"))
                ? EFMCodexFullCardSurface::RuleLeft : EFMCodexFullCardSurface::RuleRight, Accent);
        }
    }
    FSlateFontInfo RatingFont = OverallNumberText->GetFont();
    RatingFont.Size = bFullCardPilot ? 48 : 44;
    RatingFont.OutlineSettings.OutlineSize = bFullCardPilot ? 1 : 0;
    RatingFont.OutlineSettings.OutlineColor = Accent * .40f;
    OverallNumberText->SetFont(RatingFont);
    // Stable left module: normal two-digit ratings and the legal three-digit DEV case.
    OverallNumberText->SetMinDesiredWidth(bFullCardPilot ? 124.f : 0.f);
    OverallNumberText->SetJustification(ETextJustify::Left);
    OverallLabelText->SetJustification(ETextJustify::Left);
    // Protect only these glyphs over bright lamps; no portrait/rating-region scrim.
    FSlateFontInfo CaptionFont = OverallLabelText->GetFont();
    CaptionFont.OutlineSettings.OutlineSize = bFullCardPilot ? 1 : 0;
    CaptionFont.OutlineSettings.OutlineColor = FLinearColor(.002f,.007f,.014f,.95f);
    OverallLabelText->SetFont(CaptionFont);
    if (bFullCardPilot)
        OverallLabelText->SetColorAndOpacity(FSlateColor(
            FLinearColor::FromSRGBColor(FColor(0xEE,0xF3,0xF4))));
    if (auto* RatingSlot = Cast<UOverlaySlot>(GetWidgetFromName(TEXT("InMatchFullCardOverallGroup"))->Slot))
        RatingSlot->SetPadding(bFullCardPilot ? FMargin(14,18,0,0) : FMargin(12,18,0,0));
    if (auto* CaptionSlot = Cast<UVerticalBoxSlot>(OverallLabelText->Slot))
        CaptionSlot->SetPadding(bFullCardPilot ? FMargin(2,-5,0,0) : FMargin(0));
    FullCardAssignedNumberText->SetRenderTranslation(bFullCardPilot ? FVector2D(.5f,1.f) : FVector2D::ZeroVector);
    FSlateFontInfo IdentityFont = IdentityText->GetFont();
    IdentityFont.LetterSpacing = bFullCardPilot ? 65 : 0;
    IdentityFont.OutlineSettings.OutlineSize = bFullCardPilot ? 1 : 0;
    IdentityFont.OutlineSettings.OutlineColor = FLinearColor(.4f,.45f,.47f,1);
    IdentityFont.TypefaceFontName = bFullCardPilot ? TEXT("Bold") : TEXT("Medium");
    IdentityText->SetFont(IdentityFont);
    FullCardCollectionText->SetVisibility(bDetailed
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    // Serial is a catalog presentation identifier, never an assigned shirt number.
    // There is no authoritative collection denominator; do not fabricate "/290".
    FullCardCollectionText->SetText(FText::Format(
        NSLOCTEXT("FMCodexPlayerUI", "FullCardCollectionLine", "FOOTBALL TACTICAL CARD GAME  {0}"),
        FText::FromString(Presentation.PlayerFacingSerialLabel)));
    FullCardCollectionText->SetColorAndOpacity(FSlateColor(FLinearColor(.32f,.42f,.49f,1)));
    if (UWidget* Rail = GetWidgetFromName(TEXT("InMatchFullCardRarityRailBounds")))
        Rail->SetVisibility(bFullCardPilot ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
    AttributeRegion->SetPadding(bFullCardPilot ? FMargin(7,2,7,4) : FMargin(7,5));
    SkillRegion->SetPadding(bFullCardPilot ? FMargin(13,0,13,3) : FMargin(7,4));
    BiographyRegion->SetBrush(FSlateBrush());
    BiographyRegion->SetBrushColor(FLinearColor(.012f,.029f,.045f,.58f));
    if (UBorder* Scrim = Cast<UBorder>(GetWidgetFromName(TEXT("FullCardIdentityReadabilityBase"))))
        Scrim->SetBrushColor(FLinearColor(.004f,.012f,.021f,bFullCardPilot ? .90f : .62f));
    if (UVerticalBoxSlot* NameSlot = Cast<UVerticalBoxSlot>(IdentityText->Slot))
        NameSlot->SetPadding(bFullCardPilot ? FMargin(16,6,bHasNumber ? 90 : 16,2) : FMargin(10,4,bHasNumber ? 90 : 34,1));
    if (UVerticalBoxSlot* SupplementSlot = Cast<UVerticalBoxSlot>(FullCardIdentitySupplementText->Slot))
        SupplementSlot->SetPadding(bFullCardPilot ? FMargin(16,1,bHasNumber ? 90 : 16,6) : FMargin(10,0,bHasNumber ? 90 : 38,5));
    if (bDetailed) PlayerFacingSerialText->SetVisibility(ESlateVisibility::Collapsed);
    if (!bFullCardPilot)
    {
        FSlateFontInfo NumberFont = FullCardAssignedNumberText->GetFont();
        NumberFont.TypefaceFontName = TEXT("Bold"); NumberFont.Size = 26;
        FullCardAssignedNumberText->SetFont(NumberFont);
        FullCardAssignedNumberText->SetColorAndOpacity(FSlateColor(Accent));
        return;
    }

    CardFrame->SetBrushColor(FullCardSurfaceColor());
    FullCardInnerFrame->SetBrushColor(FLinearColor::Transparent);
    CardFrameFallbackSurface->SetVisibility(ESlateVisibility::Collapsed);
    PlayerFacingSerialText->SetVisibility(ESlateVisibility::Collapsed);
    FLinearColor IdentityAccent = Accent; IdentityAccent.A = .38f;
    FullCardIdentityAccent->SetBrushColor(IdentityAccent);
    // Lift accent midtones without adding the preceding pass's peach-colored white.
    const FLinearColor HeroInk = FMath::Lerp(FLinearColor(FMath::Sqrt(Accent.R),
        FMath::Sqrt(Accent.G),FMath::Sqrt(Accent.B),1),FLinearColor::White,.06f);
    IdentityText->SetColorAndOpacity(FSlateColor(FMath::Lerp(HeroInk,FLinearColor::White,.78f)));
    OverallNumberText->SetColorAndOpacity(FSlateColor(HeroInk));
    FSlateFontInfo NameFont = IdentityText->GetFont();
    NameFont.Size = GetMeasuredSingleLineFontSize(IdentityText->GetText(), NameFont,
        bHasNumber ? 236.f : 312.f, 24, 18);
    IdentityText->SetFont(NameFont);
    FSlateFontInfo NumberFont = FullCardAssignedNumberText->GetFont();
    NumberFont.TypefaceFontName = TEXT("Bold");
    NumberFont.Size = GetMeasuredSingleLineFontSize(FText::FromString(Presentation.AssignedPlayerNumber), NumberFont, 42.f, 26, 12);
    FullCardAssignedNumberText->SetFont(NumberFont);
    FullCardAssignedNumberText->SetColorAndOpacity(FSlateColor(HeroInk));
    const int32 AttributeRows = Presentation.bGoalkeeper ? 3 : 5;
    const int32 SkillRows = RenderedSkillTexts.Num();
    // UI space allocation only: no attribute/skill facts are invented for short cards.
    // Reserve actual breathing room for the three-skill variant inside the same footprint.
    const float HeroHeight = FMath::Clamp(280.f + (2-SkillRows)*26.f
        + (5-AttributeRows)*24.f + (SkillRows == 0 ? 20.f : 0.f)
        - (SkillRows >= 3 ? 12.f : 0.f), 242.f, 360.f);
    PortraitBounds->SetHeightOverride(HeroHeight);
    if (ResolvedPortraitTexture != nullptr)
    {
        // All four Full derivatives share one ratio-preserving viewport. No player-key offsets.
        constexpr float InteriorWidth = FullCardWidth - 16.f;
        const float UVHeight = HeroHeight / (InteriorWidth * 1.5f);
        FSlateBrush PortraitBrush = PortraitImage->GetBrush();
        PortraitBrush.SetUVRegion(FBox2f(FVector2f(0,FullCardPortraitTop),
            FVector2f(1,FullCardPortraitTop+UVHeight)));
        PortraitImage->SetBrush(PortraitBrush);
    }
}

int32 UFMCodexPlayerCardWidget::NativePaint(const FPaintArgs& Args, const FGeometry& G,
	const FSlateRect& Cull, FSlateWindowElementList& Out, int32 Layer,
	const FWidgetStyle& Style, bool bEnabled) const
{
	const int32 Top=Super::NativePaint(Args,G,Cull,Out,Layer,Style,bEnabled);
	if (PresentationMode == EFMCodexPlayerCardPresentationMode::PitchMini)
	{
		const FVector2f Size(G.GetLocalSize());
		const FLinearColor Tint = Style.GetColorAndOpacityTint();
		const bool Enabled = bEnabled && GetIsEnabled();
		const ESlateDrawEffect Effect = Enabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		// Hover replaces the entire ownership stroke; pip and selected-role signals
		// retain their own surfaces and cannot override its four-sided highlight.
		const FLinearColor Owner = ResolvePitchMiniRenderedOwnerColor(Presentation);
		const FLinearColor Outer = IsHovered() && Enabled
			? FLinearColor::FromSRGBColor(FColor(205, 215, 224)) : Owner;
		const FSlateRoundedBoxBrush OwnerFrame(FLinearColor::Transparent, 5.f, Outer * Tint, 3.f);
		const FSlateRoundedBoxBrush InnerFrame(FLinearColor::Transparent, 2.f,
			FLinearColor(.15f, .21f, .26f, .14f) * Tint, .6f);
		// NativePaint runs after child content. An unfilled contour adds contact
		// separation without darkening the portrait/bar or needing a blur/material.
		// The 2-unit sides and 3-unit bottom stay inside the existing 4-unit slot inset.
		const FSlateRoundedBoxBrush ContactEdge(FLinearColor::Transparent, 7.f,
			FLinearColor(.001f, .003f, .004f, .42f) * Tint, 2.f);
		FSlateDrawElement::MakeBox(Out, Top+1,
			G.ToPaintGeometry(Size+FVector2f(4,4), FSlateLayoutTransform(FVector2f(-2,-1))),
			&ContactEdge, Effect, FLinearColor::Transparent);
		FSlateDrawElement::MakeBox(Out, Top+2, G.ToPaintGeometry(), &OwnerFrame, Effect, FLinearColor::Transparent);
		FSlateDrawElement::MakeBox(Out, Top+2,
			G.ToPaintGeometry(Size-FVector2f(6,6), FSlateLayoutTransform(FVector2f(3,3))),
			&InnerFrame, Effect, FLinearColor::Transparent);
		return Top+2;
	}
    if (bFullCardPilot)
    {
        const FVector2f Size(G.GetLocalSize());
        const float W = Size.X, H = Size.Y;
        const FLinearColor Tint = Style.GetColorAndOpacityTint();
        const ESlateDrawEffect Effect = bEnabled && GetIsEnabled()
            ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
        FLinearColor Accent = FMCodexPlayerCardWidget::GetHandMicroRarityBaseColor(Presentation.RarityLabel);
        Accent.A = .82f;
        // Two structural contours and four bounded corner bevels; no bloom or metal texture.
        auto Contour = [&](const float Inset, const float Cut, const FLinearColor Color, const float Width)
        {
            FSlateDrawElement::MakeLines(Out,Top+1,G.ToPaintGeometry(),
                TArray<FVector2f>{{Inset+Cut,Inset},{W-Inset-Cut,Inset},{W-Inset,Inset+Cut},
                {W-Inset,H-Inset-Cut},{W-Inset-Cut,H-Inset},{Inset+Cut,H-Inset},
                {Inset,H-Inset-Cut},{Inset,Inset+Cut},{Inset+Cut,Inset}},Effect,Color*Tint,true,Width);
        };
        using namespace FMCodexFullCardGeometry;
        Contour(OuterInset,OuterCut,Accent,MainStroke);
        FLinearColor Channel = StructuralInk(); Channel.A = .20f;
        Contour(ChannelInset,ChannelCut,Channel,DetailStroke);
        FLinearColor Inner = StructuralInk();
        Inner.A = .48f;
        Contour(InnerInset,InnerCut,Inner,.75f);
        FLinearColor Corner = Accent; Corner.A = .76f;
        for (int32 XSide = 0; XSide < 2; ++XSide)
        for (int32 YSide = 0; YSide < 2; ++YSide)
        {
            TArray<FVector2f> Points = {{4,38},{4,18},{18,4},{38,4}};
            for (FVector2f& P : Points)
            {
                if (XSide) P.X = W-P.X;
                if (YSide) P.Y = H-P.Y;
            }
            FSlateDrawElement::MakeLines(Out,Top+2,G.ToPaintGeometry(),
                Points,Effect,Corner*Tint,true,2.f);
        }
        return Top+2;
    }
	if (!bCanonicalCardFamily) return Top;
	const FVector2f Size(G.GetLocalSize()); const float W=Size.X,H=Size.Y;
	const bool Hand=PresentationMode==EFMCodexPlayerCardPresentationMode::HandMicro;
	const FLinearColor Tint=Style.GetColorAndOpacityTint();
	const ESlateDrawEffect Effect=bEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
	FLinearColor Accent;
    if (Hand)
    {
        // One rounded outline, equal on every edge; no extra side rail or top shard.
        Accent=FMCodexPlayerCardWidget::GetHandMicroRarityBaseColor(Presentation.RarityLabel);
        Accent.A=.85f;
        const bool bHandHover=IsHovered() && bEnabled && !bDragSourcePresentationActive;
        const FLinearColor Outer=bHandHover ? FLinearColor(.72f,.79f,.85f,1.f) : Accent;
        FLinearColor Inset=bHandHover ? Accent : FLinearColor(.18f,.29f,.38f,.22f);
        if (bHandHover) Inset.A=.50f;
        const FSlateRoundedBoxBrush InnerFrame(FLinearColor::Transparent,1.5f,Inset*Tint,.7f);
        const FSlateRoundedBoxBrush OuterFrame(FLinearColor::Transparent,2.5f,Outer*Tint,bHandHover ? 2.f : 1.5f);
        FSlateDrawElement::MakeBox(Out,Top+1,
            G.ToPaintGeometry(FVector2f(W-6,H-6),FSlateLayoutTransform(FVector2f(3,3))),
            &InnerFrame,Effect,FLinearColor::Transparent);
        FSlateDrawElement::MakeBox(Out,Top+1,
            G.ToPaintGeometry(FVector2f(W-2,H-2),FSlateLayoutTransform(FVector2f(1,1))),
            &OuterFrame,Effect,FLinearColor::Transparent);
        return Top+1;
    }
	return Top;
}

FLinearColor UFMCodexPlayerCardWidget::ResolvePitchMiniOwnerColor(const FFMCodexUMGCardViewModel& Card)
{
    const FLinearColor C = Card.PitchMiniOwnershipAccentColor;
    if (!Card.bHasPitchMiniOwnershipAccent || !FMath::IsFinite(C.R) || !FMath::IsFinite(C.G)
        || !FMath::IsFinite(C.B) || !FMath::IsFinite(C.A) || C.A <= 0.f)
        return FLinearColor::FromSRGBColor(FColor(109, 137, 157));
    // The parameter is already the selected side color, never rarity or inferred A/B color.
    return FLinearColor(FMath::Clamp(C.R,0.f,1.f), FMath::Clamp(C.G,0.f,1.f), FMath::Clamp(C.B,0.f,1.f),1.f);
}

FLinearColor UFMCodexPlayerCardWidget::ResolvePitchMiniRenderedOwnerColor(const FFMCodexUMGCardViewModel& Card)
{
	FLinearColor Accent = ResolvePitchMiniOwnerColor(Card);
	constexpr float MinimumLuminance = .22f;
	const float SourceLuminance = .2126f*Accent.R + .7152f*Accent.G + .0722f*Accent.B;
	// Already-readable choices need no correction; this also makes the mapping
	// stable if an accent is accidentally fed through presentation twice.
	if (SourceLuminance >= MinimumLuminance - UE_SMALL_NUMBER)
	{
		return Accent;
	}
	const float Peak = FMath::Max3(Accent.R, Accent.G, Accent.B);
	if (Peak > UE_SMALL_NUMBER && Peak < .72f)
	{
		// Cap exposure by both peak channel and luminance so dark greens /
		// neutral colors do not become unnecessarily bright.
		const float Gain = FMath::Min(.72f / Peak, .42f / FMath::Max(SourceLuminance, UE_SMALL_NUMBER));
		Accent.R *= Gain; Accent.G *= Gain; Accent.B *= Gain;
	}
	// Rec.709 luminance in linear light catches saturated blue/purple, which
	// peak-channel gain alone misses. Minimal neutral mixing preserves RGB hue
	// ordering, limits desaturation, and maps valid black to readable charcoal.
	const float Luminance = .2126f*Accent.R + .7152f*Accent.G + .0722f*Accent.B;
	if (Luminance < MinimumLuminance)
	{
		const float Mix = (MinimumLuminance - Luminance) / (1.f - Luminance);
		Accent = FMath::Lerp(Accent, FLinearColor::White, Mix);
	}
	// Presentation only: no write to the selected/stored player color.
	return Accent;
}
