#include "FMCodexThroughBallResolutionSurfaceWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexRollReelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace FMCodexThroughBallResolutionSurfaceWidget
{
	UTextBlock* MakeText(UWidgetTree& Tree, const FName Name)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetAutoWrapText(false);
		Result->SetJustification(ETextJustify::Center);
		return Result;
	}

	void SetOptionalText(UTextBlock* Text, const FString& Value)
	{
		if (Text == nullptr)
		{
			return;
		}
		Text->SetText(FText::FromString(Value));
		Text->SetVisibility(Value.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	}
}

UFMCodexThroughBallResolutionSurfaceWidget
	::UFMCodexThroughBallResolutionSurfaceWidget(
		const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexThroughBallResolutionSurfaceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget>
UFMCodexThroughBallResolutionSurfaceWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexThroughBallResolutionSurfaceWidget::RefreshFromPresentation(
	const FFMCodexUMGThroughBallResolutionViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGThroughBallResolutionViewModel&
UFMCodexThroughBallResolutionSurfaceWidget::GetPresentation() const
{
	return Presentation;
}

UFMCodexRollReelWidget*
UFMCodexThroughBallResolutionSurfaceWidget::GetRollReelWidget() const
{
	return RollReel;
}

UFMCodexInlineResolutionFormulaSurfaceWidget*
UFMCodexThroughBallResolutionSurfaceWidget::GetFormulaSurface() const
{
	return FormulaSurface;
}

void UFMCodexThroughBallResolutionSurfaceWidget::RequestContinue()
{
	if (Presentation.bVisible
		&& ((Presentation.PrimaryAction.bVisible
				&& Presentation.PrimaryAction.Action.bAvailable)
			|| (Presentation.Formula.PrimaryAction.bVisible
				&& Presentation.Formula.PrimaryAction.Action.bAvailable)))
	{
		OnContinueRequested.Broadcast();
	}
}

void UFMCodexThroughBallResolutionSurfaceWidget::HandleContinueClicked()
{
	RequestContinue();
}

void UFMCodexThroughBallResolutionSurfaceWidget::BuildWidgetTree()
{
	using namespace FMCodexThroughBallResolutionSurfaceWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ThroughBallProductionSurfaceBounds"));
	Bounds->SetMinDesiredWidth(520.0f);
	Bounds->SetMaxDesiredWidth(720.0f);
	WidgetTree->RootWidget = Bounds;

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("ThroughBallProductionSurfaceFrame"));
	Style.ApplyBorder(
		*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		FMargin(22.0f, 16.0f));
	FLinearColor FrameColor = Style.GetColor(
		EFMCodexPlayerUIColorRole::PanelBackground);
	FrameColor.A = 0.94f;
	Frame->SetBrushColor(FrameColor);
	Bounds->AddChild(Frame);

	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ThroughBallProductionHierarchy"));
	Frame->AddChild(Body);

	TitleText = MakeText(*WidgetTree, TEXT("ThroughBallProductionTitle"));
	Style.ApplyText(*TitleText, EFMCodexPlayerUITextRole::ActionTitle);
	Body->AddChildToVerticalBox(TitleText);

	RouteText = MakeText(*WidgetTree, TEXT("ThroughBallProductionRoute"));
	Style.ApplyText(*RouteText, EFMCodexPlayerUITextRole::SectionHeading);
	if (UVerticalBoxSlot* RouteBoxSlot = Body->AddChildToVerticalBox(RouteText))
	{
		RouteBoxSlot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 0.0f));
	}

	StageText = MakeText(*WidgetTree, TEXT("ThroughBallProductionStage"));
	Style.ApplyText(*StageText, EFMCodexPlayerUITextRole::Status);
	if (UVerticalBoxSlot* StageBoxSlot = Body->AddChildToVerticalBox(StageText))
	{
		StageBoxSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 8.0f));
	}

	DiceRevealRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("ThroughBallInitialRouteRevealRegion"));
	Style.ApplyBorder(
		*DiceRevealRegion, EFMCodexPlayerUIColorRole::PanelInset,
		FMargin(14.0f, 10.0f));
	UVerticalBox* RevealBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ThroughBallInitialRouteRevealBody"));
	RollReel = WidgetTree->ConstructWidget<UFMCodexRollReelWidget>(
		UFMCodexRollReelWidget::StaticClass(),
		TEXT("ThroughBallSharedRollReel"));
	if (UVerticalBoxSlot* ReelBoxSlot =
		RevealBody->AddChildToVerticalBox(RollReel))
	{
		ReelBoxSlot->SetHorizontalAlignment(HAlign_Center);
	}
	RouteResultText = MakeText(
		*WidgetTree, TEXT("ThroughBallInitialRouteResult"));
	Style.ApplyText(*RouteResultText, EFMCodexPlayerUITextRole::SectionHeading);
	if (UVerticalBoxSlot* ResultBoxSlot =
		RevealBody->AddChildToVerticalBox(RouteResultText))
	{
		ResultBoxSlot->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 0.0f));
	}
	DiceRevealRegion->AddChild(RevealBody);
	Body->AddChildToVerticalBox(DiceRevealRegion);

	FormulaSurface = WidgetTree->ConstructWidget<
		UFMCodexInlineResolutionFormulaSurfaceWidget>(
		UFMCodexInlineResolutionFormulaSurfaceWidget::StaticClass(),
		TEXT("ThroughBallFeetSharedFormulaSurface"));
	FormulaSurface->OnContinueRequested.AddDynamic(
		this,
		&UFMCodexThroughBallResolutionSurfaceWidget::HandleContinueClicked);
	if (UVerticalBoxSlot* FormulaSlot =
		Body->AddChildToVerticalBox(FormulaSurface))
	{
		FormulaSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}

	StatusText = MakeText(*WidgetTree, TEXT("ThroughBallProductionStatus"));
	Style.ApplyText(*StatusText, EFMCodexPlayerUITextRole::Secondary);
	if (UVerticalBoxSlot* StatusBoxSlot =
		Body->AddChildToVerticalBox(StatusText))
	{
		StatusBoxSlot->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 0.0f));
	}

	ActionPromptText = MakeText(
		*WidgetTree, TEXT("ThroughBallProductionActionPrompt"));
	Style.ApplyText(*ActionPromptText, EFMCodexPlayerUITextRole::Body);
	if (UVerticalBoxSlot* PromptBoxSlot =
		Body->AddChildToVerticalBox(ActionPromptText))
	{
		PromptBoxSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	ContinueButton = WidgetTree->ConstructWidget<UButton>(
		UButton::StaticClass(), TEXT("ThroughBallPrimaryActionButton"));
	Style.ApplyButton(*ContinueButton, EFMCodexPlayerUIActionRole::Primary);
	UTextBlock* ContinueText = MakeText(
		*WidgetTree, TEXT("ThroughBallPrimaryActionButtonLabel"));
	Style.ApplyText(*ContinueText, EFMCodexPlayerUITextRole::Body);
	ContinueButton->AddChild(ContinueText);
	ContinueButton->OnClicked.AddDynamic(
		this,
		&UFMCodexThroughBallResolutionSurfaceWidget::HandleContinueClicked);
	USizeBox* ContinueBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ThroughBallPrimaryActionBounds"));
	ContinueBounds->SetWidthOverride(180.0f);
	ContinueBounds->SetHeightOverride(42.0f);
	ContinueBounds->AddChild(ContinueButton);
	if (UVerticalBoxSlot* ContinueBoxSlot =
		Body->AddChildToVerticalBox(ContinueBounds))
	{
		ContinueBoxSlot->SetHorizontalAlignment(HAlign_Center);
		ContinueBoxSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}
}

void UFMCodexThroughBallResolutionSurfaceWidget::RefreshVisuals()
{
	using namespace FMCodexThroughBallResolutionSurfaceWidget;
	if (TitleText == nullptr)
	{
		return;
	}

	SetVisibility(Presentation.bVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	TitleText->SetText(FText::FromString(Presentation.TitleLabel));
	SetOptionalText(RouteText, Presentation.RouteLabel);
	SetOptionalText(StageText, Presentation.StageLabel);
	SetOptionalText(StatusText, Presentation.StatusLabel);
	SetOptionalText(ActionPromptText, Presentation.ActionPromptLabel);
	DiceRevealRegion->SetVisibility(
		Presentation.bVisible && Presentation.bDiceRevealVisible
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	SetOptionalText(RouteResultText, Presentation.RouteResultLabel);
	RollReel->RefreshFromPresentation(Presentation.RollReel);
	FormulaSurface->RefreshFromPresentation(Presentation.Formula);
	ContinueButton->GetParent()->SetVisibility(
		Presentation.bVisible && Presentation.PrimaryAction.bVisible
			&& Presentation.PrimaryAction.Action.bAvailable
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ContinueButton->SetIsEnabled(
		Presentation.PrimaryAction.Action.bAvailable);
	if (UTextBlock* Label = Cast<UTextBlock>(ContinueButton->GetChildAt(0)))
	{
		Label->SetText(FText::FromString(
			Presentation.PrimaryAction.Action.Label));
	}
}
