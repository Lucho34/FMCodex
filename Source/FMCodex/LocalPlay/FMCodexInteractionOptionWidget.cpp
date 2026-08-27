#include "FMCodexInteractionOptionWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UFMCodexInteractionOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexInteractionOptionWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexInteractionOptionWidget::ConfigureSimple(
	const FString& InLabel)
{
	Label = InLabel;
	BindingMode = EBindingMode::Simple;
	RefreshVisuals();
}

void UFMCodexInteractionOptionWidget::ConfigureCard(
	const FString& InLabel,
	const FName InCardId)
{
	Label = InLabel;
	CardId = InCardId;
	BindingMode = EBindingMode::Card;
	RefreshVisuals();
}

void UFMCodexInteractionOptionWidget::ConfigureTacticalCard(
	const FString& InLabel,
	const FString& InSecondaryLabel,
	const FName InSkillId)
{
	Label = InLabel;
	SecondaryLabel = InSecondaryLabel;
	CardId = InSkillId;
	BindingMode = EBindingMode::TacticalCard;
	RefreshVisuals();
}

void UFMCodexInteractionOptionWidget::ConfigureDeployment(
	const FString& InLabel,
	const FName InCardId,
	const FName InSlotId,
	const bool bInGoalkeeper)
{
	Label = InLabel;
	CardId = InCardId;
	SlotId = InSlotId;
	bGoalkeeper = bInGoalkeeper;
	BindingMode = EBindingMode::Deployment;
	RefreshVisuals();
}

void UFMCodexInteractionOptionWidget::ConfigureBranch(
	const FString& InLabel,
	const EFMCodexUMGBranchIntent InIntent)
{
	Label = InLabel;
	BranchIntent = InIntent;
	BindingMode = EBindingMode::Branch;
	RefreshVisuals();
}

void UFMCodexInteractionOptionWidget::ConfigureOneOnOne(
	const FString& InLabel,
	const EFMCodexUMGOneOnOneChoice InChoice)
{
	Label = InLabel;
	OneOnOneChoice = InChoice;
	BindingMode = EBindingMode::OneOnOne;
	RefreshVisuals();
}

const FString& UFMCodexInteractionOptionWidget::GetLabel() const
{
	return Label;
}

const FString& UFMCodexInteractionOptionWidget::GetSecondaryLabel() const
{
	return SecondaryLabel;
}

bool UFMCodexInteractionOptionWidget::IsTacticalCard() const
{
	return BindingMode == EBindingMode::TacticalCard;
}

void UFMCodexInteractionOptionWidget::NativeOnAddedToFocusPath(
	const FFocusEvent& InFocusEvent)
{
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	HandleTacticalHovered();
}

void UFMCodexInteractionOptionWidget::NativeOnRemovedFromFocusPath(
	const FFocusEvent& InFocusEvent)
{
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	HandleTacticalUnhovered();
}

void UFMCodexInteractionOptionWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* OptionFrame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("InteractionOptionFrame"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*OptionFrame,
		EFMCodexPlayerUIColorRole::PanelRaised, Style.GetCompactPadding());
	WidgetTree->RootWidget = OptionFrame;

	OptionButton = WidgetTree->ConstructWidget<UButton>(
		UButton::StaticClass(), TEXT("InteractionOptionButton"));
	Style.ApplyButton(
		*OptionButton, EFMCodexPlayerUIActionRole::Secondary);
	OptionBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InteractionOptionBounds"));
	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionOptionContent"));
	OptionLabelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("InteractionOptionLabel"));
	OptionLabelText->SetAutoWrapText(true);
	OptionLabelText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*OptionLabelText, EFMCodexPlayerUITextRole::Body);
	Content->AddChildToVerticalBox(OptionLabelText);
	OptionSecondaryText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("InteractionOptionSecondaryLabel"));
	OptionSecondaryText->SetAutoWrapText(false);
	OptionSecondaryText->SetJustification(ETextJustify::Center);
	OptionSecondaryText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	Style.ApplyText(*OptionSecondaryText, EFMCodexPlayerUITextRole::Secondary);
	Content->AddChildToVerticalBox(OptionSecondaryText);
	OptionButton->AddChild(Content);
	OptionBounds->AddChild(OptionButton);
	OptionFrame->AddChild(OptionBounds);
}

void UFMCodexInteractionOptionWidget::RefreshVisuals()
{
	if (OptionLabelText == nullptr)
	{
		return;
	}
	OptionLabelText->SetText(FText::FromString(
		Label.IsEmpty() ? TEXT("OPTION UNAVAILABLE") : Label));
	const bool bTacticalCard = BindingMode == EBindingMode::TacticalCard;
	const bool bOneOnOneChoice = BindingMode == EBindingMode::OneOnOne;
	OptionSecondaryText->SetText(FText::FromString(SecondaryLabel));
	OptionSecondaryText->SetVisibility(
		bTacticalCard && !SecondaryLabel.IsEmpty()
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	OptionLabelText->SetAutoWrapText(!bTacticalCard && !bOneOnOneChoice);
	OptionLabelText->SetTextOverflowPolicy(
		bTacticalCard ? ETextOverflowPolicy::Ellipsis : ETextOverflowPolicy::Clip);
	FFMCodexPlayerUIStyle::Get().ApplyText(*OptionLabelText,
		bTacticalCard ? EFMCodexPlayerUITextRole::Identity
			: EFMCodexPlayerUITextRole::Body);
	if (bTacticalCard)
	{
		OptionBounds->ClearMinDesiredWidth();
		OptionBounds->ClearMinDesiredHeight();
		OptionBounds->SetWidthOverride(156.0f);
		OptionBounds->SetHeightOverride(58.0f);
	}
	else if (bOneOnOneChoice)
	{
		OptionBounds->ClearWidthOverride();
		OptionBounds->ClearHeightOverride();
		OptionBounds->SetMinDesiredWidth(120.0f);
		OptionBounds->SetMinDesiredHeight(42.0f);
	}
	else
	{
		OptionBounds->ClearWidthOverride();
		OptionBounds->ClearHeightOverride();
		OptionBounds->ClearMinDesiredWidth();
		OptionBounds->ClearMinDesiredHeight();
	}
	const bool bConfigured = BindingMode != EBindingMode::None
		&& !Label.IsEmpty();
	OptionButton->SetIsEnabled(bConfigured);
	FFMCodexPlayerUIStyle::Get().ApplyButton(*OptionButton,
		bConfigured ? EFMCodexPlayerUIActionRole::Secondary
			: EFMCodexPlayerUIActionRole::Disabled);
	BindConfiguredHandler();
}

void UFMCodexInteractionOptionWidget::BindConfiguredHandler()
{
	if (OptionButton == nullptr)
	{
		return;
	}
	OptionButton->OnClicked.Clear();
	OptionButton->OnHovered.Clear();
	OptionButton->OnUnhovered.Clear();
	switch (BindingMode)
	{
	case EBindingMode::Simple:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleSimpleClicked);
		break;
	case EBindingMode::Card:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleCardClicked);
		break;
	case EBindingMode::TacticalCard:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleCardClicked);
		OptionButton->OnHovered.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleTacticalHovered);
		OptionButton->OnUnhovered.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleTacticalUnhovered);
		break;
	case EBindingMode::Deployment:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleDeploymentClicked);
		break;
	case EBindingMode::Branch:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleBranchClicked);
		break;
	case EBindingMode::OneOnOne:
		OptionButton->OnClicked.AddDynamic(
			this, &UFMCodexInteractionOptionWidget::HandleOneOnOneClicked);
		break;
	default:
		break;
	}
}

void UFMCodexInteractionOptionWidget::HandleSimpleClicked()
{
	OnSimpleRequested.Broadcast();
}

void UFMCodexInteractionOptionWidget::HandleCardClicked()
{
	OnCardRequested.Broadcast(CardId);
}

void UFMCodexInteractionOptionWidget::HandleTacticalHovered()
{
	if (BindingMode == EBindingMode::TacticalCard && !CardId.IsNone())
	{
		OnTacticalDetailRequested.Broadcast(CardId);
	}
}

void UFMCodexInteractionOptionWidget::HandleTacticalUnhovered()
{
	if (BindingMode == EBindingMode::TacticalCard && !CardId.IsNone())
	{
		OnTacticalDetailDismissed.Broadcast(CardId);
	}
}

void UFMCodexInteractionOptionWidget::HandleDeploymentClicked()
{
	OnDeploymentRequested.Broadcast(CardId, SlotId, bGoalkeeper);
}

void UFMCodexInteractionOptionWidget::HandleBranchClicked()
{
	OnBranchRequested.Broadcast(BranchIntent);
}

void UFMCodexInteractionOptionWidget::HandleOneOnOneClicked()
{
	OnOneOnOneRequested.Broadcast(OneOnOneChoice);
}
