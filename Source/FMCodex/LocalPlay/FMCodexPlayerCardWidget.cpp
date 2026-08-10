#include "FMCodexPlayerCardWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

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
	const FFMCodexUMGCardViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGCardViewModel&
UFMCodexPlayerCardWidget::GetPresentation() const
{
	return Presentation;
}

void UFMCodexPlayerCardWidget::BuildWidgetTree()
{
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UBorder* CardBorder = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("CardBorder"));
	CardBorder->SetPadding(FMargin(8.0f));
	CardBorder->SetBrushColor(FLinearColor(0.04f, 0.10f, 0.16f, 0.96f));
	WidgetTree->RootWidget = CardBorder;

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CardBody"));
	CardBorder->AddChild(Body);

	IdentityText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardIdentity"));
	IdentityText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(IdentityText);

	RoleText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardRole"));
	Body->AddChildToVerticalBox(RoleText);

	SkillText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardSkills"));
	SkillText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(SkillText);

	AttributeText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardAttributes"));
	AttributeText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(AttributeText);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(
		UTextBlock::StaticClass(), TEXT("CardStatus"));
	StatusText->SetAutoWrapText(true);
	Body->AddChildToVerticalBox(StatusText);
}

void UFMCodexPlayerCardWidget::RefreshVisuals()
{
	if (IdentityText == nullptr)
	{
		return;
	}
	IdentityText->SetText(FText::FromString(Presentation.IdentityLabel));
	RoleText->SetText(FText::FromString(FString::Printf(
		TEXT("%s | %s"), *Presentation.OwnerLabel,
		*Presentation.RoleLabel)));
	SkillText->SetText(FText::FromString(
		TEXT("SKILL | ") + Presentation.SkillSummaryLabel));
	AttributeText->SetText(FText::FromString(
		Presentation.CompactAttributeSummary));
	StatusText->SetText(FText::FromString(
		Presentation.StatusSummaryLabel));
}
