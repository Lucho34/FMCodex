#include "FMCodexThroughBallResolutionSurfaceWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexRollReelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
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

	bool HasSameOneOnOneChoiceIdentity(
		const FFMCodexUMGThroughBallResolutionViewModel& Left,
		const FFMCodexUMGThroughBallResolutionViewModel& Right)
	{
		if (!Left.bVisible || !Right.bVisible
			|| Left.Stage != EFMCodexUMGThroughBallStage::OneOnOneChoice
			|| Right.Stage != EFMCodexUMGThroughBallStage::OneOnOneChoice
			|| Left.RouteLabel != Right.RouteLabel
			|| Left.RouteResultLabel != Right.RouteResultLabel
			|| Left.OneOnOneChoices.Num() != Right.OneOnOneChoices.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < Left.OneOnOneChoices.Num(); ++Index)
		{
			const FFMCodexUMGOneOnOneChoiceViewModel& LeftChoice =
				Left.OneOnOneChoices[Index];
			const FFMCodexUMGOneOnOneChoiceViewModel& RightChoice =
				Right.OneOnOneChoices[Index];
			if (LeftChoice.Choice != RightChoice.Choice
				|| LeftChoice.Label != RightChoice.Label
				|| LeftChoice.SecondaryLabel != RightChoice.SecondaryLabel)
			{
				return false;
			}
		}
		return true;
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
	const bool bPreserveOneOnOneChoices =
		FMCodexThroughBallResolutionSurfaceWidget
			::HasSameOneOnOneChoiceIdentity(Presentation, InPresentation);
	Presentation = InPresentation;
	RefreshVisuals(bPreserveOneOnOneChoices);
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

const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>&
UFMCodexThroughBallResolutionSurfaceWidget::GetOneOnOneChoiceWidgets() const
{
	return OneOnOneChoiceWidgets;
}

#if WITH_DEV_AUTOMATION_TESTS
void UFMCodexThroughBallResolutionSurfaceWidget
	::ResetOneOnOneDispatchForTesting()
{
	OneOnOneDispatchCountForTesting = 0;
	LastOneOnOneDispatchForTesting = EFMCodexUMGOneOnOneChoice::None;
}

int32 UFMCodexThroughBallResolutionSurfaceWidget
	::GetOneOnOneDispatchCountForTesting() const
{
	return OneOnOneDispatchCountForTesting;
}

EFMCodexUMGOneOnOneChoice UFMCodexThroughBallResolutionSurfaceWidget
	::GetLastOneOnOneDispatchForTesting() const
{
	return LastOneOnOneDispatchForTesting;
}

#endif

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

void UFMCodexThroughBallResolutionSurfaceWidget::HandleOneOnOneClicked(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	if (Presentation.bVisible
		&& Presentation.Stage == EFMCodexUMGThroughBallStage::OneOnOneChoice
		&& Presentation.OneOnOneChoices.ContainsByPredicate(
			[Choice](const FFMCodexUMGOneOnOneChoiceViewModel& Candidate)
			{
				return Candidate.Choice == Choice;
			}))
	{
#if WITH_DEV_AUTOMATION_TESTS
		++OneOnOneDispatchCountForTesting;
		LastOneOnOneDispatchForTesting = Choice;
#endif
		OnOneOnOneRequested.Broadcast(Choice);
	}
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
	Bounds->SetMaxDesiredWidth(840.0f);
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
	Style.ApplyText(*TitleText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(TitleText);

	RouteText = MakeText(*WidgetTree, TEXT("ThroughBallProductionRoute"));
	Style.ApplyText(*RouteText, EFMCodexPlayerUITextRole::SectionHeading);
	if (UVerticalBoxSlot* RouteBoxSlot = Body->AddChildToVerticalBox(RouteText))
	{
		RouteBoxSlot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 0.0f));
	}

	StageText = MakeText(*WidgetTree, TEXT("ThroughBallProductionStage"));
	Style.ApplyText(*StageText, EFMCodexPlayerUITextRole::ActionTitle);
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

	OutcomeHintText = MakeText(
		*WidgetTree, TEXT("ThroughBallOutcomeRollHint"));
	Style.ApplyText(*OutcomeHintText, EFMCodexPlayerUITextRole::Secondary);
	if (UVerticalBoxSlot* HintSlot =
		Body->AddChildToVerticalBox(OutcomeHintText))
	{
		HintSlot->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 0.0f));
	}

	ResultTitleText = MakeText(*WidgetTree, TEXT("ThroughBallOutcomeTitle"));
	Style.ApplyText(*ResultTitleText, EFMCodexPlayerUITextRole::ActionTitle);
	if (UVerticalBoxSlot* ResultTitleSlot =
		Body->AddChildToVerticalBox(ResultTitleText))
	{
		ResultTitleSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}
	NarrativeText = MakeText(*WidgetTree, TEXT("ThroughBallOutcomeNarrative"));
	NarrativeText->SetAutoWrapText(true);
	Style.ApplyText(*NarrativeText, EFMCodexPlayerUITextRole::Body);
	if (UVerticalBoxSlot* NarrativeSlot =
		Body->AddChildToVerticalBox(NarrativeText))
	{
		NarrativeSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));
	}

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

	OneOnOneChoiceRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("ThroughBallOneOnOneChoiceRow"));
	if (UVerticalBoxSlot* ChoiceRowSlot =
		Body->AddChildToVerticalBox(OneOnOneChoiceRow))
	{
		ChoiceRowSlot->SetHorizontalAlignment(HAlign_Center);
		ChoiceRowSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
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

void UFMCodexThroughBallResolutionSurfaceWidget::RefreshVisuals(
	const bool bPreserveOneOnOneChoices)
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
		Presentation.bVisible && (Presentation.bDiceRevealVisible
			|| !Presentation.RouteResultLabel.IsEmpty())
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	SetOptionalText(RouteResultText, Presentation.RouteResultLabel);
	SetOptionalText(OutcomeHintText,
		Presentation.OutcomeRollHint.bVisible
			? Presentation.OutcomeRollHint.DisplayLabel : FString());
	RollReel->RefreshFromPresentation(Presentation.RollReel);
	SetOptionalText(ResultTitleText,
		Presentation.bNarrativeAvailable ? Presentation.ResultTitle : FString());
	SetOptionalText(NarrativeText,
		Presentation.bNarrativeAvailable
			? Presentation.NarrativeHeadline : FString());
	FormulaSurface->RefreshFromPresentation(Presentation.Formula);
	if (!bPreserveOneOnOneChoices)
	{
		RebuildOneOnOneChoices();
	}
	OneOnOneChoiceRow->SetVisibility(OneOnOneChoiceWidgets.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
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

void UFMCodexThroughBallResolutionSurfaceWidget::RebuildOneOnOneChoices()
{
	OneOnOneChoiceRow->ClearChildren();
	OneOnOneChoiceWidgets.Reset();
	if (Presentation.bVisible
		&& Presentation.Stage == EFMCodexUMGThroughBallStage::OneOnOneChoice)
	{
		for (int32 Index = 0; Index < Presentation.OneOnOneChoices.Num(); ++Index)
		{
			const FFMCodexUMGOneOnOneChoiceViewModel& Choice =
				Presentation.OneOnOneChoices[Index];
			if (Choice.Choice == EFMCodexUMGOneOnOneChoice::None
				|| Choice.Label.IsEmpty())
			{
				continue;
			}
			UFMCodexInteractionOptionWidget* Option =
				WidgetTree->ConstructWidget<UFMCodexInteractionOptionWidget>(
					UFMCodexInteractionOptionWidget::StaticClass(),
					FName(*FString::Printf(
						TEXT("ThroughBallOneOnOneChoice%d"), Index)));
			Option->ConfigureOneOnOne(
				Choice.Label, Choice.SecondaryLabel, Choice.Choice);
			Option->OnOneOnOneRequested.AddDynamic(
				this,
				&UFMCodexThroughBallResolutionSurfaceWidget::HandleOneOnOneClicked);
			if (UHorizontalBoxSlot* OptionSlot =
				OneOnOneChoiceRow->AddChildToHorizontalBox(Option))
			{
				OptionSlot->SetPadding(FMargin(Index == 0 ? 0.0f : 10.0f,
					0.0f, 0.0f, 0.0f));
				OptionSlot->SetVerticalAlignment(VAlign_Center);
			}
			OneOnOneChoiceWidgets.Add(Option);
		}
	}
}
