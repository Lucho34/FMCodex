#include "FMCodexInteractionOptionWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

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
	OptionLabelText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("InteractionOptionLabel"));
	OptionLabelText->SetAutoWrapText(true);
	OptionLabelText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*OptionLabelText, EFMCodexPlayerUITextRole::Body);
	OptionButton->AddChild(OptionLabelText);
	OptionFrame->AddChild(OptionButton);
}

void UFMCodexInteractionOptionWidget::RefreshVisuals()
{
	if (OptionLabelText == nullptr)
	{
		return;
	}
	OptionLabelText->SetText(FText::FromString(
		Label.IsEmpty() ? TEXT("OPTION UNAVAILABLE") : Label));
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
