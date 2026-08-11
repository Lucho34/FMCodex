#include "FMCodexPlayerCardWidget.h"

#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

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
				Presentation, EFMCodexPlayerCardPresentationMode::PitchCompact);
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

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedRoleIconTexture() const
{
	return ResolvedRoleIconTexture;
}

UTexture2D* UFMCodexPlayerCardWidget::GetResolvedLongShotSkillIconTexture() const
{
	return ResolvedLongShotSkillIconTexture;
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
	WidgetTree->RootWidget = CardBounds;

	CardFrame = MakeRegion(*WidgetTree, TEXT("PlayerCardFrame"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::CardFrame),
		FFMCodexPlayerUIStyle::Get().GetSectionPadding());
	CardBounds->AddChild(CardFrame);
	UOverlay* FrameAssetHook = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("CardFrameAssetHook"));
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

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PlayerCardVisualHierarchy"));
	UBorder* ContentReadabilityLayer = MakeRegion(
		*WidgetTree, TEXT("CardContentReadabilityLayer"),
		FLinearColor(0.005f, 0.012f, 0.02f, 0.62f), FMargin(7.0f));
	ContentReadabilityLayer->AddChild(Body);
	FrameAssetHook->AddChildToOverlay(ContentReadabilityLayer);

	UBorder* HeaderRegion = MakeRegion(
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

	UBorder* IdentityRegion = MakeRegion(
		*WidgetTree, TEXT("CardIdentityRegion"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelRaised),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	UVerticalBox* IdentityBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CardIdentityBody"));
	IdentityText = MakeText(*WidgetTree, TEXT("CardIdentity"));
	OwnerText = MakeText(*WidgetTree, TEXT("CardOwner"));
	DeveloperReferenceText = MakeText(
		*WidgetTree, TEXT("CardDeveloperReference"));
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*IdentityText, EFMCodexPlayerUITextRole::Identity);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*OwnerText, EFMCodexPlayerUITextRole::Kicker);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*DeveloperReferenceText, EFMCodexPlayerUITextRole::Secondary);
	IdentityBody->AddChildToVerticalBox(IdentityText);
	IdentityBody->AddChildToVerticalBox(OwnerText);
	IdentityBody->AddChildToVerticalBox(DeveloperReferenceText);
	IdentityRegion->AddChild(IdentityBody);
	Body->AddChildToVerticalBox(IdentityRegion);

	UBorder* SkillRegion = MakeRegion(
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

	UBorder* AttributeRegion = MakeRegion(
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

	UBorder* StatusRegion = MakeRegion(
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

	const bool bCompact = PresentationMode
		== EFMCodexPlayerCardPresentationMode::PitchCompact;
	CardBounds->SetWidthOverride(bCompact ? 148.0f : 240.0f);
	CardBounds->SetHeightOverride(bCompact ? 208.0f : 360.0f);
	PortraitBounds->SetHeightOverride(bCompact ? 32.0f : 82.0f);
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	CardFrame->SetBrushColor(Style.GetColor(Presentation.bGoalkeeper
		? EFMCodexPlayerUIColorRole::GoalkeeperCardFrame
		: EFMCodexPlayerUIColorRole::CardFrame));
	RoleIconHook->SetBrushColor(Presentation.bGoalkeeper
		? Style.GetColor(EFMCodexPlayerUIColorRole::GoalkeeperCardFrame)
		: Style.GetPlayerAccentColor(Presentation.OwnerLabel));

	IdentityText->SetText(Presentation.IdentityLabel.IsEmpty()
		? FFMCodexPlayerUIPresentationText::UnknownCard()
		: FText::FromString(Presentation.IdentityLabel));
	OwnerText->SetText(FFMCodexPlayerUIPresentationText::Owner(
		Presentation.OwnerLabel));
	RoleText->SetText(FFMCodexPlayerUIPresentationText::Role(
		Presentation.RoleLabel));
	RarityText->SetText(FText::Format(FText::FromString(TEXT("  {0}")),
		FFMCodexPlayerUIPresentationText::Rarity(Presentation.RarityLabel)));
	PortraitPlaceholderText->SetText(
		FFMCodexPlayerUIPresentationText::PortraitPlaceholder());
	RefreshPresentationArt();
	DeveloperReferenceText->SetText(FText::FromString(
		Presentation.DeveloperReferenceLabel));
	DeveloperReferenceText->SetVisibility(bCompact
		|| ResolvedArtIdentity
			== FFMCodexPlayerUIAssetReferences::Get().GetGoldenSampleArtIdentity()
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
	CardFrameImage->SetVisibility(bHasFrame
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	CardFrameFallbackSurface->SetVisibility(bHasFrame
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (bHasFrame)
	{
		CardFrameImage->SetBrushFromTexture(
			ResolvedCardFrameTexture, false);
	}

	const bool bHasPortrait = ResolvedPortraitTexture != nullptr;
	PortraitImage->SetVisibility(bHasPortrait
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	PortraitPlaceholderText->SetVisibility(bHasPortrait
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	if (bHasPortrait)
	{
		PortraitImage->SetBrushFromTexture(ResolvedPortraitTexture, true);
	}

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
