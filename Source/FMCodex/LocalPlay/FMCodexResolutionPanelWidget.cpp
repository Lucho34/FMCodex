#include "FMCodexResolutionPanelWidget.h"

#include "FMCodexDiceResultWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"

namespace FMCodexResolutionPanelWidget
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
		const FMargin Padding = FMargin(9.0f))
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		Result->SetPadding(Padding);
		Result->SetBrushColor(Color);
		return Result;
	}

	UVerticalBox* MakeLabeledSection(
		UWidgetTree& Tree,
		const FName Name,
		const TCHAR* Label,
		UTextBlock*& ValueText)
	{
		UVerticalBox* Result = Tree.ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), Name);
		Result->AddChildToVerticalBox(MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Heading"))), Label));
		ValueText = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Value"))));
		Result->AddChildToVerticalBox(ValueText);
		return Result;
	}
}

UFMCodexResolutionPanelWidget::UFMCodexResolutionPanelWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DiceResultWidgetClass = UFMCodexDiceResultWidget::StaticClass();
}

void UFMCodexResolutionPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexResolutionPanelWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexResolutionPanelWidget::RefreshFromPresentation(
	const FFMCodexUMGResolutionViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGResolutionViewModel&
UFMCodexResolutionPanelWidget::GetPresentation() const
{
	return Presentation;
}

const TArray<TObjectPtr<UFMCodexDiceResultWidget>>&
UFMCodexResolutionPanelWidget::GetRenderedDiceWidgets() const
{
	return RenderedDiceWidgets;
}

int32 UFMCodexResolutionPanelWidget::GetRenderedComparisonCount() const
{
	return RenderedComparisonCount;
}

void UFMCodexResolutionPanelWidget::BuildWidgetTree()
{
	using namespace FMCodexResolutionPanelWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("ResolutionPanelBounds"));
	Bounds->SetMinDesiredWidth(840.0f);
	Bounds->SetMaxDesiredWidth(1120.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = MakeRegion(
		*WidgetTree, TEXT("ResolutionPanelFrame"),
		FLinearColor(0.025f, 0.045f, 0.07f, 0.98f), FMargin(12.0f));
	Bounds->AddChild(Frame);
	UVerticalBox* RootBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionPanelHierarchy"));
	Frame->AddChild(RootBody);

	EmptyStateText = MakeText(
		*WidgetTree, TEXT("ResolutionEmptyState"),
		TEXT("Waiting for an authoritative result."));
	RootBody->AddChildToVerticalBox(EmptyStateText);

	UBorder* RejectionRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionRejectedRegion"),
		FLinearColor(0.28f, 0.045f, 0.045f, 1.0f));
	RejectionBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionRejectedHierarchy"));
	RejectionBody->AddChildToVerticalBox(MakeText(
		*WidgetTree, TEXT("ResolutionRejectedHeading"),
		TEXT("COMMAND REJECTED")));
	RejectionReasonText = MakeText(
		*WidgetTree, TEXT("ResolutionRejectedReason"));
	RejectionMessageText = MakeText(
		*WidgetTree, TEXT("ResolutionRejectedMessage"));
	RejectionBody->AddChildToVerticalBox(RejectionReasonText);
	RejectionBody->AddChildToVerticalBox(RejectionMessageText);
	RejectionRegion->AddChild(RejectionBody);
	RootBody->AddChildToVerticalBox(RejectionRegion);

	AcceptedResultBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionAcceptedHierarchy"));
	RootBody->AddChildToVerticalBox(AcceptedResultBody);

	UBorder* StepRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionStepRegion"),
		FLinearColor(0.06f, 0.13f, 0.19f, 1.0f));
	UVerticalBox* StepBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionStepHierarchy"));
	StepBody->AddChildToVerticalBox(MakeText(
		*WidgetTree, TEXT("ResolutionStepHeading"), TEXT("STEP")));
	StepTitleText = MakeText(*WidgetTree, TEXT("ResolutionStepTitle"));
	StepSummaryText = MakeText(*WidgetTree, TEXT("ResolutionStepSummary"));
	RouteText = MakeText(*WidgetTree, TEXT("ResolutionRouteSummary"));
	StepBody->AddChildToVerticalBox(StepTitleText);
	StepBody->AddChildToVerticalBox(StepSummaryText);
	StepBody->AddChildToVerticalBox(RouteText);
	StepRegion->AddChild(StepBody);
	AcceptedResultBody->AddChildToVerticalBox(StepRegion);

	UBorder* DiceRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionDiceRegion"),
		FLinearColor(0.035f, 0.09f, 0.13f, 1.0f));
	DiceSection = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionDiceSection"));
	DiceSection->AddChildToVerticalBox(MakeText(
		*WidgetTree, TEXT("ResolutionDiceHeading"), TEXT("DICE")));
	UScrollBox* DiceScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("ResolutionDiceScroll"));
	DiceScroll->SetOrientation(Orient_Horizontal);
	DiceBody = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("ResolutionDiceBody"));
	DiceBody->SetInnerSlotPadding(FVector2D(7.0f, 7.0f));
	DiceScroll->AddChild(DiceBody);
	DiceSection->AddChildToVerticalBox(DiceScroll);
	DiceRegion->AddChild(DiceSection);
	AcceptedResultBody->AddChildToVerticalBox(DiceRegion);

	UBorder* ComparisonRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionComparisonRegion"),
		FLinearColor(0.04f, 0.105f, 0.145f, 1.0f));
	ComparisonSection = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionComparisonSection"));
	ComparisonSection->AddChildToVerticalBox(MakeText(
		*WidgetTree, TEXT("ResolutionComparisonHeading"), TEXT("COMPARISON")));
	ComparisonBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("ResolutionComparisonBody"));
	ComparisonSection->AddChildToVerticalBox(ComparisonBody);
	ComparisonRegion->AddChild(ComparisonSection);
	AcceptedResultBody->AddChildToVerticalBox(ComparisonRegion);

	UBorder* DecisionRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionDecisionRegion"),
		FLinearColor(0.07f, 0.13f, 0.18f, 1.0f));
	UTextBlock* DecisionValue = nullptr;
	DecisionSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionDecisionSection"),
		TEXT("DECISION"), DecisionValue);
	DecisionText = DecisionValue;
	DecisionRegion->AddChild(DecisionSection);
	AcceptedResultBody->AddChildToVerticalBox(DecisionRegion);

	UBorder* ContinuationRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionContinuationRegion"),
		FLinearColor(0.09f, 0.12f, 0.16f, 1.0f));
	UTextBlock* ContinuationValue = nullptr;
	ContinuationSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionContinuationSection"),
		TEXT("CONTINUES / COMPLETION"), ContinuationValue);
	ContinuationText = ContinuationValue;
	ContinuationRegion->AddChild(ContinuationSection);
	AcceptedResultBody->AddChildToVerticalBox(ContinuationRegion);

	UBorder* TerminalRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionTerminalRegion"),
		FLinearColor(0.11f, 0.28f, 0.16f, 1.0f), FMargin(16.0f));
	UTextBlock* TerminalValue = nullptr;
	TerminalSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionTerminalSection"),
		TEXT("RESULT"), TerminalValue);
	TerminalText = TerminalValue;
	TerminalRegion->AddChild(TerminalSection);
	AcceptedResultBody->AddChildToVerticalBox(TerminalRegion);
}

void UFMCodexResolutionPanelWidget::RefreshVisuals()
{
	if (EmptyStateText == nullptr)
	{
		return;
	}
	const bool bShowEmpty = !Presentation.bVisible;
	const bool bShowRejected = Presentation.bVisible && Presentation.bRejected;
	const bool bShowAccepted = Presentation.bVisible && !Presentation.bRejected;
	EmptyStateText->SetVisibility(bShowEmpty
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	EmptyStateText->SetText(FText::FromString(
		Presentation.EmptyStateLabel.IsEmpty()
			? TEXT("Waiting for an authoritative result.")
			: Presentation.EmptyStateLabel));
	RejectionBody->GetParent()->SetVisibility(bShowRejected
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	AcceptedResultBody->SetVisibility(bShowAccepted
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	RejectionReasonText->SetText(FText::FromString(
		Presentation.DecisionLabel.IsEmpty()
			? TEXT("No resolution result was accepted.")
			: Presentation.DecisionLabel));
	RejectionMessageText->SetText(FText::FromString(
		Presentation.ErrorLabel.IsEmpty()
			? TEXT("The command could not be completed.")
			: Presentation.ErrorLabel));
	StepTitleText->SetText(FText::FromString(Presentation.StepLabel));
	StepSummaryText->SetText(FText::FromString(Presentation.StepSummaryLabel));
	StepSummaryText->SetVisibility(Presentation.StepSummaryLabel.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	RouteText->SetText(FText::FromString(Presentation.RouteLabel));
	RouteText->SetVisibility(Presentation.RouteLabel.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	RefreshDiceResults();
	RefreshComparisonEvidence();
	DecisionText->SetText(FText::FromString(Presentation.DecisionLabel));
	DecisionSection->GetParent()->SetVisibility(
		bShowAccepted && !Presentation.DecisionLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ContinuationText->SetText(FText::FromString(Presentation.ContinuationLabel));
	ContinuationSection->GetParent()->SetVisibility(
		bShowAccepted && !Presentation.ContinuationLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	TerminalText->SetText(FText::FromString(Presentation.TerminalLabel));
	TerminalSection->GetParent()->SetVisibility(
		bShowAccepted && !Presentation.TerminalLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFMCodexResolutionPanelWidget::RefreshDiceResults()
{
	RenderedDiceWidgets.Reset();
	DiceBody->ClearChildren();
	if (!Presentation.bVisible || Presentation.bRejected)
	{
		DiceSection->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	UClass* ResolvedClass = DiceResultWidgetClass != nullptr
		? DiceResultWidgetClass.Get() : UFMCodexDiceResultWidget::StaticClass();
	for (int32 Index = 0; Index < Presentation.DiceResults.Num(); ++Index)
	{
		UFMCodexDiceResultWidget* Die =
			WidgetTree->ConstructWidget<UFMCodexDiceResultWidget>(
				ResolvedClass,
				FName(*FString::Printf(TEXT("AcceptedDiceResult%d"), Index)));
		Die->RefreshFromPresentation(Presentation.DiceResults[Index]);
		DiceBody->AddChildToWrapBox(Die);
		RenderedDiceWidgets.Add(Die);
	}
	DiceSection->GetParent()->SetVisibility(
		Presentation.bVisible && !Presentation.bRejected
			&& !Presentation.DiceResults.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFMCodexResolutionPanelWidget::RefreshComparisonEvidence()
{
	using namespace FMCodexResolutionPanelWidget;
	ComparisonBody->ClearChildren();
	RenderedComparisonCount = 0;
	if (!Presentation.bVisible || Presentation.bRejected)
	{
		ComparisonSection->GetParent()->SetVisibility(
			ESlateVisibility::Collapsed);
		return;
	}
	RenderedComparisonCount = Presentation.ComparisonEvidence.Num();
	for (int32 Index = 0; Index < Presentation.ComparisonEvidence.Num(); ++Index)
	{
		if (Index == 1)
		{
			UTextBlock* Versus = MakeText(
				*WidgetTree, TEXT("ResolutionVersusLabel"), TEXT("VS"));
			Versus->SetJustification(ETextJustify::Center);
			ComparisonBody->AddChildToHorizontalBox(Versus);
		}
		const FFMCodexUMGComparisonEvidenceViewModel& Evidence =
			Presentation.ComparisonEvidence[Index];
		UBorder* Card = MakeRegion(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceCard%d"), Index)),
			FLinearColor(0.055f, 0.14f, 0.19f, 1.0f));
		UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*FString::Printf(TEXT("ComparisonEvidenceBody%d"), Index)));
		Body->AddChildToVerticalBox(MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceHeading%d"), Index)),
			Evidence.HeadingLabel));
		Body->AddChildToVerticalBox(MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceValue%d"), Index)),
			Evidence.EvidenceLabel));
		Card->AddChild(Body);
		ComparisonBody->AddChildToHorizontalBox(Card);
	}
	ComparisonSection->GetParent()->SetVisibility(
		Presentation.bVisible && !Presentation.bRejected
			&& !Presentation.ComparisonEvidence.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
