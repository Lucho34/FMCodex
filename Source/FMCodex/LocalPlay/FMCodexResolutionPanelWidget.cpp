#include "FMCodexResolutionPanelWidget.h"

#include "FMCodexDiceResultWidget.h"
#include "FMCodexPlayerUIStyle.h"

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
		const EFMCodexPlayerUIColorRole ColorRole,
		const FMargin& Padding)
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		FFMCodexPlayerUIStyle::Get().ApplyBorder(
			*Result, ColorRole, Padding);
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
		UTextBlock* Heading = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Heading"))), Label);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*Heading, EFMCodexPlayerUITextRole::SectionHeading);
		Result->AddChildToVerticalBox(Heading);
		ValueText = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Value"))));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*ValueText, EFMCodexPlayerUITextRole::Body);
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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Bounds->SetMinDesiredWidth(Style.GetPanelMinWidth());
	Bounds->SetMaxDesiredWidth(Style.GetPanelMaxWidth());
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = MakeRegion(
		*WidgetTree, TEXT("ResolutionPanelFrame"),
		EFMCodexPlayerUIColorRole::PanelBackground,
		Style.GetPanelPadding());
	Bounds->AddChild(Frame);
	UVerticalBox* RootBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionPanelHierarchy"));
	Frame->AddChild(RootBody);

	EmptyStateText = MakeText(
		*WidgetTree, TEXT("ResolutionEmptyState"),
		TEXT("Waiting for an authoritative result."));
	Style.ApplyText(*EmptyStateText, EFMCodexPlayerUITextRole::Secondary);
	RootBody->AddChildToVerticalBox(EmptyStateText);

	RejectionRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionRejectedRegion"),
		EFMCodexPlayerUIColorRole::Danger,
		Style.GetPanelPadding());
	RejectionBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionRejectedHierarchy"));
	UTextBlock* RejectionHeading = MakeText(
		*WidgetTree, TEXT("ResolutionRejectedHeading"),
		TEXT("COMMAND REJECTED"));
	Style.ApplyText(
		*RejectionHeading, EFMCodexPlayerUITextRole::ActionTitle);
	RejectionBody->AddChildToVerticalBox(RejectionHeading);
	RejectionReasonText = MakeText(
		*WidgetTree, TEXT("ResolutionRejectedReason"));
	RejectionMessageText = MakeText(
		*WidgetTree, TEXT("ResolutionRejectedMessage"));
	Style.ApplyText(
		*RejectionReasonText, EFMCodexPlayerUITextRole::Status);
	Style.ApplyText(
		*RejectionMessageText, EFMCodexPlayerUITextRole::Body);
	RejectionBody->AddChildToVerticalBox(RejectionReasonText);
	RejectionBody->AddChildToVerticalBox(RejectionMessageText);
	RejectionRegion->AddChild(RejectionBody);
	RootBody->AddChildToVerticalBox(RejectionRegion);

	AcceptedResultBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionAcceptedHierarchy"));
	RootBody->AddChildToVerticalBox(AcceptedResultBody);

	UBorder* StepRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionStepRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	UVerticalBox* StepBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionStepHierarchy"));
	UTextBlock* StepHeading = MakeText(
		*WidgetTree, TEXT("ResolutionStepHeading"), TEXT("STEP"));
	Style.ApplyText(*StepHeading, EFMCodexPlayerUITextRole::SectionHeading);
	StepBody->AddChildToVerticalBox(StepHeading);
	StepTitleText = MakeText(*WidgetTree, TEXT("ResolutionStepTitle"));
	StepSummaryText = MakeText(*WidgetTree, TEXT("ResolutionStepSummary"));
	RouteText = MakeText(*WidgetTree, TEXT("ResolutionRouteSummary"));
	Style.ApplyText(*StepTitleText, EFMCodexPlayerUITextRole::ActionTitle);
	Style.ApplyText(*StepSummaryText, EFMCodexPlayerUITextRole::Body);
	Style.ApplyText(*RouteText, EFMCodexPlayerUITextRole::Secondary);
	StepBody->AddChildToVerticalBox(StepTitleText);
	StepBody->AddChildToVerticalBox(StepSummaryText);
	StepBody->AddChildToVerticalBox(RouteText);
	StepRegion->AddChild(StepBody);
	AcceptedResultBody->AddChildToVerticalBox(StepRegion);

	UBorder* DiceRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionDiceRegion"),
		EFMCodexPlayerUIColorRole::PanelInset,
		Style.GetSectionPadding());
	DiceSection = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionDiceSection"));
	UTextBlock* DiceHeading = MakeText(
		*WidgetTree, TEXT("ResolutionDiceHeading"), TEXT("DICE"));
	Style.ApplyText(*DiceHeading, EFMCodexPlayerUITextRole::SectionHeading);
	DiceSection->AddChildToVerticalBox(DiceHeading);
	UScrollBox* DiceScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("ResolutionDiceScroll"));
	DiceScroll->SetOrientation(Orient_Horizontal);
	DiceBody = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("ResolutionDiceBody"));
	DiceBody->SetInnerSlotPadding(Style.GetControlGap());
	DiceScroll->AddChild(DiceBody);
	DiceSection->AddChildToVerticalBox(DiceScroll);
	DiceRegion->AddChild(DiceSection);
	AcceptedResultBody->AddChildToVerticalBox(DiceRegion);

	UBorder* ComparisonRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionComparisonRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	ComparisonSection = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("ResolutionComparisonSection"));
	UTextBlock* ComparisonHeading = MakeText(
		*WidgetTree, TEXT("ResolutionComparisonHeading"), TEXT("FORMULA COMPARISON"));
	Style.ApplyText(
		*ComparisonHeading, EFMCodexPlayerUITextRole::SectionHeading);
	ComparisonSection->AddChildToVerticalBox(ComparisonHeading);
	ComparisonBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("ResolutionComparisonBody"));
	ComparisonSection->AddChildToVerticalBox(ComparisonBody);
	ComparisonRegion->AddChild(ComparisonSection);
	AcceptedResultBody->AddChildToVerticalBox(ComparisonRegion);

	UBorder* DecisionRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionDecisionRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	UTextBlock* DecisionValue = nullptr;
	DecisionSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionDecisionSection"),
		TEXT("DECISION"), DecisionValue);
	DecisionText = DecisionValue;
	DecisionRegion->AddChild(DecisionSection);
	AcceptedResultBody->AddChildToVerticalBox(DecisionRegion);

	UBorder* ContinuationRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionContinuationRegion"),
		EFMCodexPlayerUIColorRole::PanelInset,
		Style.GetSectionPadding());
	UTextBlock* ContinuationValue = nullptr;
	ContinuationSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionContinuationSection"),
		TEXT("CONTINUES / COMPLETION"), ContinuationValue);
	ContinuationText = ContinuationValue;
	ContinuationRegion->AddChild(ContinuationSection);
	AcceptedResultBody->AddChildToVerticalBox(ContinuationRegion);

	TerminalRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionTerminalRegion"),
		EFMCodexPlayerUIColorRole::TerminalNeutral,
		Style.GetPanelPadding());
	UBorder* ResultIconHook = MakeRegion(
		*WidgetTree, TEXT("ResultIconAssetHook"),
		EFMCodexPlayerUIColorRole::NeutralAccent,
		Style.GetCompactPadding());
	UTextBlock* ResultIconPlaceholder = MakeText(
		*WidgetTree, TEXT("ResultIconPlaceholder"), TEXT("RESULT"));
	Style.ApplyText(
		*ResultIconPlaceholder, EFMCodexPlayerUITextRole::Kicker);
	ResultIconHook->AddChild(ResultIconPlaceholder);
	UTextBlock* TerminalValue = nullptr;
	TerminalSection = MakeLabeledSection(
		*WidgetTree, TEXT("ResolutionTerminalSection"),
		TEXT("RESULT"), TerminalValue);
	TerminalText = TerminalValue;
	Style.ApplyText(*TerminalText, EFMCodexPlayerUITextRole::TerminalResult);
	TerminalSection->AddChildToVerticalBox(ResultIconHook);
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
	TerminalRegion->SetBrushColor(
		FFMCodexPlayerUIStyle::Get().GetTerminalColor(
			Presentation.TerminalLabel));
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
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*Versus, EFMCodexPlayerUITextRole::Identity);
			ComparisonBody->AddChildToHorizontalBox(Versus);
		}
		const FFMCodexUMGComparisonEvidenceViewModel& Evidence =
			Presentation.ComparisonEvidence[Index];
		UBorder* Card = MakeRegion(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceCard%d"), Index)),
			Evidence.HeadingLabel.Contains(TEXT("ATTACK"))
				? EFMCodexPlayerUIColorRole::ActionPrimary
				: EFMCodexPlayerUIColorRole::ActionSecondary,
			FFMCodexPlayerUIStyle::Get().GetSectionPadding());
		UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*FString::Printf(TEXT("ComparisonEvidenceBody%d"), Index)));
		UTextBlock* EvidenceHeading = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceHeading%d"), Index)),
			Evidence.HeadingLabel);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*EvidenceHeading, EFMCodexPlayerUITextRole::SectionHeading);
		Body->AddChildToVerticalBox(EvidenceHeading);
		UTextBlock* EvidenceValue = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("ComparisonEvidenceValue%d"), Index)),
			Evidence.EvidenceLabel);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*EvidenceValue, EFMCodexPlayerUITextRole::ActionTitle);
		Body->AddChildToVerticalBox(EvidenceValue);
		Card->AddChild(Body);
		ComparisonBody->AddChildToHorizontalBox(Card);
	}
	ComparisonSection->GetParent()->SetVisibility(
		Presentation.bVisible && !Presentation.bRejected
			&& !Presentation.ComparisonEvidence.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
