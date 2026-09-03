#include "FMCodexLongShotResolutionSurfaceWidget.h"

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexPlayerUIStyle.h"
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

namespace FMCodexLongShotResolutionSurfaceWidget
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
		Text->SetText(FText::FromString(Value));
		Text->SetVisibility(Value.IsEmpty()
			? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	}

	bool SameChoices(
		const FFMCodexUMGLongShotResolutionViewModel& A,
		const FFMCodexUMGLongShotResolutionViewModel& B)
	{
		if (A.Stage != EFMCodexUMGLongShotStage::BranchChoice
			|| B.Stage != EFMCodexUMGLongShotStage::BranchChoice
			|| A.BranchChoices.Num() != B.BranchChoices.Num())
		{
			return false;
		}
		for (int32 Index = 0; Index < A.BranchChoices.Num(); ++Index)
		{
			if (A.BranchChoices[Index].Intent != B.BranchChoices[Index].Intent
				|| A.BranchChoices[Index].Label != B.BranchChoices[Index].Label
				|| A.BranchChoices[Index].SecondaryLabel
					!= B.BranchChoices[Index].SecondaryLabel)
			{
				return false;
			}
		}
		return true;
	}
}

UFMCodexLongShotResolutionSurfaceWidget
	::UFMCodexLongShotResolutionSurfaceWidget(
		const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexLongShotResolutionSurfaceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexLongShotResolutionSurfaceWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexLongShotResolutionSurfaceWidget::RefreshFromPresentation(
	const FFMCodexUMGLongShotResolutionViewModel& InPresentation)
{
	const bool bPreserve = FMCodexLongShotResolutionSurfaceWidget::SameChoices(
		Presentation, InPresentation);
	Presentation = InPresentation;
	RefreshVisuals(bPreserve);
}

const FFMCodexUMGLongShotResolutionViewModel&
UFMCodexLongShotResolutionSurfaceWidget::GetPresentation() const
{
	return Presentation;
}

UFMCodexRollReelWidget*
UFMCodexLongShotResolutionSurfaceWidget::GetRollReelWidget() const
{
	return RollReel;
}

UFMCodexInlineResolutionFormulaSurfaceWidget*
UFMCodexLongShotResolutionSurfaceWidget::GetFormulaSurface() const
{
	return FormulaSurface;
}

const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>&
UFMCodexLongShotResolutionSurfaceWidget::GetBranchChoiceWidgets() const
{
	return BranchChoiceWidgets;
}

#if WITH_DEV_AUTOMATION_TESTS
void UFMCodexLongShotResolutionSurfaceWidget::ResetBranchDispatchForTesting()
{
	BranchDispatchCountForTesting = 0;
	LastBranchDispatchForTesting = EFMCodexUMGBranchIntent::None;
}

int32 UFMCodexLongShotResolutionSurfaceWidget
	::GetBranchDispatchCountForTesting() const
{
	return BranchDispatchCountForTesting;
}

EFMCodexUMGBranchIntent UFMCodexLongShotResolutionSurfaceWidget
	::GetLastBranchDispatchForTesting() const
{
	return LastBranchDispatchForTesting;
}
#endif

void UFMCodexLongShotResolutionSurfaceWidget::HandleContinueClicked()
{
	const bool bOwnsOuterAction = Presentation.PrimaryAction.bVisible
		&& Presentation.PrimaryAction.Action.bAvailable;
	const bool bOwnsFormulaAction = Presentation.Formula.bVisible
		&& Presentation.Formula.PrimaryAction.bVisible
		&& Presentation.Formula.PrimaryAction.Action.bAvailable;
	if (Presentation.bVisible && (bOwnsOuterAction || bOwnsFormulaAction))
	{
		OnContinueRequested.Broadcast();
	}
}

void UFMCodexLongShotResolutionSurfaceWidget::HandleBranchClicked(
	const EFMCodexUMGBranchIntent Intent)
{
	if (Presentation.bVisible
		&& Presentation.Stage == EFMCodexUMGLongShotStage::BranchChoice
		&& Presentation.BranchChoices.ContainsByPredicate(
			[Intent](const FFMCodexUMGBranchChoiceViewModel& Choice)
			{
				return Choice.Intent == Intent;
			}))
	{
#if WITH_DEV_AUTOMATION_TESTS
		++BranchDispatchCountForTesting;
		LastBranchDispatchForTesting = Intent;
#endif
		OnBranchRequested.Broadcast(Intent);
	}
}

void UFMCodexLongShotResolutionSurfaceWidget::BuildWidgetTree()
{
	using namespace FMCodexLongShotResolutionSurfaceWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("LongShotProductionSurfaceBounds"));
	Bounds->SetMinDesiredWidth(520.0f);
	Bounds->SetMaxDesiredWidth(840.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("LongShotProductionSurfaceFrame"));
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		FMargin(22.0f, 16.0f));
	FLinearColor FrameColor = Style.GetColor(
		EFMCodexPlayerUIColorRole::PanelBackground);
	FrameColor.A = 0.94f;
	Frame->SetBrushColor(FrameColor);
	Bounds->AddChild(Frame);
	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LongShotProductionHierarchy"));
	Frame->AddChild(Body);

	TitleText = MakeText(*WidgetTree, TEXT("LongShotProductionTitle"));
	Style.ApplyText(*TitleText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(TitleText);
	BranchText = MakeText(*WidgetTree, TEXT("LongShotProductionBranch"));
	Style.ApplyText(*BranchText, EFMCodexPlayerUITextRole::SectionHeading);
	Body->AddChildToVerticalBox(BranchText);
	StageText = MakeText(*WidgetTree, TEXT("LongShotProductionStage"));
	Style.ApplyText(*StageText, EFMCodexPlayerUITextRole::ActionTitle);
	Body->AddChildToVerticalBox(StageText);

	BranchChoiceRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("LongShotBranchChoiceRow"));
	if (UVerticalBoxSlot* ChoiceRowSlot = Body->AddChildToVerticalBox(BranchChoiceRow))
	{
		ChoiceRowSlot->SetHorizontalAlignment(HAlign_Center);
		ChoiceRowSlot->SetPadding(FMargin(0.0f, 10.0f));
	}
	HintText = MakeText(*WidgetTree, TEXT("LongShotOutcomeHint"));
	Style.ApplyText(*HintText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(HintText);

	DiceRevealRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("LongShotDiceRevealRegion"));
	Style.ApplyBorder(*DiceRevealRegion, EFMCodexPlayerUIColorRole::PanelInset,
		FMargin(14.0f, 10.0f));
	UVerticalBox* DiceBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LongShotDiceRevealBody"));
	RollReel = WidgetTree->ConstructWidget<UFMCodexRollReelWidget>(
		UFMCodexRollReelWidget::StaticClass(), TEXT("LongShotSharedRollReel"));
	DiceBody->AddChildToVerticalBox(RollReel);
	PairedRollText = MakeText(*WidgetTree, TEXT("LongShotPairedRollResults"));
	Style.ApplyText(*PairedRollText, EFMCodexPlayerUITextRole::SectionHeading);
	DiceBody->AddChildToVerticalBox(PairedRollText);
	DiceRevealRegion->AddChild(DiceBody);
	Body->AddChildToVerticalBox(DiceRevealRegion);

	FormulaSurface = WidgetTree->ConstructWidget<
		UFMCodexInlineResolutionFormulaSurfaceWidget>(
		UFMCodexInlineResolutionFormulaSurfaceWidget::StaticClass(),
		TEXT("LongShotDirectSharedFormulaSurface"));
	FormulaSurface->OnContinueRequested.AddDynamic(
		this, &UFMCodexLongShotResolutionSurfaceWidget::HandleContinueClicked);
	Body->AddChildToVerticalBox(FormulaSurface);
	ResultTitleText = MakeText(*WidgetTree, TEXT("LongShotResultTitle"));
	Style.ApplyText(*ResultTitleText, EFMCodexPlayerUITextRole::ActionTitle);
	Body->AddChildToVerticalBox(ResultTitleText);
	NarrativeText = MakeText(*WidgetTree, TEXT("LongShotNarrative"));
	NarrativeText->SetAutoWrapText(true);
	Style.ApplyText(*NarrativeText, EFMCodexPlayerUITextRole::Body);
	Body->AddChildToVerticalBox(NarrativeText);
	StatusText = MakeText(*WidgetTree, TEXT("LongShotStatus"));
	Style.ApplyText(*StatusText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(StatusText);

	ContinueButton = WidgetTree->ConstructWidget<UButton>(
		UButton::StaticClass(), TEXT("LongShotPrimaryActionButton"));
	Style.ApplyButton(*ContinueButton, EFMCodexPlayerUIActionRole::Primary);
	UTextBlock* ContinueText = MakeText(
		*WidgetTree, TEXT("LongShotPrimaryActionButtonLabel"));
	Style.ApplyText(*ContinueText, EFMCodexPlayerUITextRole::Body);
	ContinueButton->AddChild(ContinueText);
	ContinueButton->OnClicked.AddDynamic(
		this, &UFMCodexLongShotResolutionSurfaceWidget::HandleContinueClicked);
	USizeBox* ContinueBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("LongShotPrimaryActionBounds"));
	ContinueBounds->SetWidthOverride(190.0f);
	ContinueBounds->SetHeightOverride(42.0f);
	ContinueBounds->AddChild(ContinueButton);
	if (UVerticalBoxSlot* ContinueSlot = Body->AddChildToVerticalBox(ContinueBounds))
	{
		ContinueSlot->SetHorizontalAlignment(HAlign_Center);
		ContinueSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}
}

void UFMCodexLongShotResolutionSurfaceWidget::RebuildBranchChoices()
{
	BranchChoiceRow->ClearChildren();
	BranchChoiceWidgets.Reset();
	for (const FFMCodexUMGBranchChoiceViewModel& Choice
		: Presentation.BranchChoices)
	{
		UFMCodexInteractionOptionWidget* Option =
			WidgetTree->ConstructWidget<UFMCodexInteractionOptionWidget>(
				UFMCodexInteractionOptionWidget::StaticClass());
		Option->ConfigureBranch(
			Choice.Label, Choice.SecondaryLabel, Choice.Intent);
		Option->OnBranchRequested.AddDynamic(
			this, &UFMCodexLongShotResolutionSurfaceWidget::HandleBranchClicked);
		if (UHorizontalBoxSlot* ChoiceSlot =
			BranchChoiceRow->AddChildToHorizontalBox(Option))
		{
			ChoiceSlot->SetPadding(FMargin(5.0f, 0.0f));
			ChoiceSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			ChoiceSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		BranchChoiceWidgets.Add(Option);
	}
}

void UFMCodexLongShotResolutionSurfaceWidget::RefreshVisuals(
	const bool bPreserveChoices)
{
	using namespace FMCodexLongShotResolutionSurfaceWidget;
	if (TitleText == nullptr)
	{
		return;
	}
	SetVisibility(Presentation.bVisible
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	TitleText->SetText(FText::FromString(Presentation.TitleLabel));
	// Branch and stage are currently the same semantic heading for resolved shot
	// branches. Keep the stronger stage heading and suppress the duplicate line.
	SetOptionalText(BranchText,
		Presentation.BranchLabel == Presentation.StageLabel
			? FString() : Presentation.BranchLabel);
	SetOptionalText(StageText, Presentation.StageLabel);
	SetOptionalText(StatusText, Presentation.StatusLabel);
	SetOptionalText(HintText, Presentation.OutcomeHintLabel);
	if (!bPreserveChoices)
	{
		RebuildBranchChoices();
	}
	BranchChoiceRow->SetVisibility(BranchChoiceWidgets.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	DiceRevealRegion->SetVisibility(Presentation.bDiceRevealVisible
		|| Presentation.bDeadCornerAVisible || Presentation.bDeadCornerBVisible
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	RollReel->RefreshFromPresentation(Presentation.RollReel);
	SetOptionalText(PairedRollText, Presentation.PairedRollResultLabel);
	FormulaSurface->RefreshFromPresentation(Presentation.Formula);
	SetOptionalText(ResultTitleText,
		Presentation.bNarrativeAvailable ? Presentation.ResultTitle : FString());
	SetOptionalText(NarrativeText, Presentation.bNarrativeAvailable
		? Presentation.NarrativeHeadline : FString());
	if (UTextBlock* Label = Cast<UTextBlock>(ContinueButton->GetChildAt(0)))
	{
		Label->SetText(FText::FromString(Presentation.ContinueActionLabel));
	}
	ContinueButton->GetParent()->SetVisibility(
		Presentation.PrimaryAction.bVisible
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
