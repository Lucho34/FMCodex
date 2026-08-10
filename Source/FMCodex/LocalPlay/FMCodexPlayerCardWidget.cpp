#include "FMCodexPlayerCardWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"

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
	UBorder* FrameAssetHook = MakeRegion(
		*WidgetTree, TEXT("CardFrameAssetHook"),
		FFMCodexPlayerUIStyle::Get().GetColor(
			EFMCodexPlayerUIColorRole::PanelInset),
		FFMCodexPlayerUIStyle::Get().GetCompactPadding());
	CardFrame->AddChild(FrameAssetHook);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("PlayerCardVisualHierarchy"));
	FrameAssetHook->AddChild(Body);

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
	RoleIconHook->AddChild(RoleText);
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
	PortraitPlaceholderText = MakeText(
		*WidgetTree, TEXT("PortraitPlaceholderText"));
	PortraitPlaceholderText->SetJustification(ETextJustify::Center);
	FFMCodexPlayerUIStyle::Get().ApplyText(
		*PortraitPlaceholderText, EFMCodexPlayerUITextRole::Secondary);
	PortraitAssetHook->AddChild(PortraitPlaceholderText);
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
		*WidgetTree, TEXT("SkillRegionTitle"), TEXT("SKILLS"));
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
		*WidgetTree, TEXT("AttributeRegionTitle"), TEXT("ATTRIBUTES"));
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

	IdentityText->SetText(FText::FromString(
		Presentation.IdentityLabel.IsEmpty()
			? TEXT("UNKNOWN CARD") : Presentation.IdentityLabel));
	OwnerText->SetText(FText::FromString(
		Presentation.OwnerLabel.IsEmpty()
			? TEXT("OWNER N/A") : Presentation.OwnerLabel));
	RoleText->SetText(FText::FromString(
		Presentation.RoleLabel.IsEmpty()
			? TEXT("ROLE N/A") : Presentation.RoleLabel));
	RarityText->SetText(FText::FromString(TEXT("  ")
		+ (Presentation.RarityLabel.IsEmpty()
			? FString(TEXT("RARITY N/A")) : Presentation.RarityLabel)));
	PortraitPlaceholderText->SetText(PortraitPlaceholderLabel);
	DeveloperReferenceText->SetText(FText::FromString(
		Presentation.DeveloperReferenceLabel));
	DeveloperReferenceText->SetVisibility(bCompact
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	RefreshSkills();
	RefreshAttributes();
	RefreshStatusBadges();
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
		UTextBlock* IconText = MakeText(*WidgetTree, FName(*FString::Printf(
			TEXT("SkillIconPlaceholder%d"), Index)), TEXT("SK"));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*IconText, EFMCodexPlayerUITextRole::Secondary);
		IconHook->AddChild(IconText);
		Row->AddChildToHorizontalBox(IconHook);
		UTextBlock* SkillText = MakeText(*WidgetTree, FName(*FString::Printf(
			TEXT("SkillIdentity%d"), Index)), TEXT("  ") + Skills[Index]);
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
			TEXT("AttributeValue%d"), Index)), Attributes[Index]);
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
			TEXT("StatusBadgeLabel%d"), Index)), Statuses[Index]);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*BadgeText, EFMCodexPlayerUITextRole::Kicker);
		Badge->AddChild(BadgeText);
		StatusBadgeBox->AddChildToWrapBox(Badge);
		RenderedStatusTexts.Add(BadgeText);
	}
}
