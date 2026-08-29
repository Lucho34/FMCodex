#include "FMCodexInteractionPanelWidget.h"

#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

namespace FMCodexInteractionPanelWidget
{
	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Text = FString())
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(Text));
		Result->SetAutoWrapText(true);
		Result->SetClipping(EWidgetClipping::ClipToBounds);
		return Result;
	}

	UButton* MakeButton(
		UWidgetTree& Tree,
		const FName Name,
		UTextBlock*& LabelText,
		const EFMCodexPlayerUIActionRole Role)
	{
		UButton* Result = Tree.ConstructWidget<UButton>(
			UButton::StaticClass(), Name);
		LabelText = MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Label"))));
		LabelText->SetAutoWrapText(false);
		LabelText->SetTextOverflowPolicy(ETextOverflowPolicy::Clip);
		LabelText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*LabelText, EFMCodexPlayerUITextRole::Body);
		Result->AddChild(LabelText);
		FFMCodexPlayerUIStyle::Get().ApplyButton(*Result, Role);
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
}

UFMCodexInteractionPanelWidget::UFMCodexInteractionPanelWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCardWidgetClass = UFMCodexPlayerCardWidget::StaticClass();
	OptionWidgetClass = UFMCodexInteractionOptionWidget::StaticClass();
}

void UFMCodexInteractionPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexInteractionPanelWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexInteractionPanelWidget::RefreshFromPresentation(
	const FFMCodexUMGInteractionViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

void UFMCodexInteractionPanelWidget::SetInteractionBlocked(const bool bBlocked)
{
	bInteractionBlocked = bBlocked;
	SetIsEnabled(!bInteractionBlocked);
}

const FFMCodexUMGInteractionViewModel&
UFMCodexInteractionPanelWidget::GetPresentation() const
{
	return Presentation;
}

bool UFMCodexInteractionPanelWidget::IsInteractionBlocked() const
{
	return bInteractionBlocked;
}

const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
UFMCodexInteractionPanelWidget::GetRenderedCandidateCardWidgets() const
{
	return RenderedCandidateCardWidgets;
}

const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>&
UFMCodexInteractionPanelWidget::GetRenderedOptionWidgets() const
{
	return RenderedOptionWidgets;
}

void UFMCodexInteractionPanelWidget::RequestStartMatch()
{
	if (!bInteractionBlocked)
	{
		OnStartMatchRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestTacticalPointRoll()
{
	if (!bInteractionBlocked)
	{
		OnTacticalPointRollRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestDeployment(
	const FName CardId,
	const FName SlotId,
	const bool bGoalkeeper)
{
	if (bInteractionBlocked)
	{
		return;
	}
	if (bGoalkeeper)
	{
		OnDeployGoalkeeperRequested.Broadcast(SlotId);
	}
	else
	{
		OnDeployOrdinaryRequested.Broadcast(CardId, SlotId);
	}
}

void UFMCodexInteractionPanelWidget::RequestFinishDeployment()
{
	if (!bInteractionBlocked)
	{
		OnFinishDeploymentRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestDeploymentTacticalReference()
{
	if (!bInteractionBlocked
		&& Presentation.Category == EFMCodexUMGInteractionCategory::Deploy)
	{
		OnDeploymentTacticalReferenceRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestCarrier(const FName CardId)
{
	if (!bInteractionBlocked)
	{
		OnCarrierRequested.Broadcast(CardId);
	}
}

void UFMCodexInteractionPanelWidget::RequestMarker(const FName CardId)
{
	if (!bInteractionBlocked)
	{
		OnMarkerRequested.Broadcast(CardId);
	}
}

void UFMCodexInteractionPanelWidget::RequestSkill(const FName SkillId)
{
	if (!bInteractionBlocked)
	{
		OnSkillRequested.Broadcast(SkillId);
	}
}

void UFMCodexInteractionPanelWidget::RequestRunner(const FName CardId)
{
	if (!bInteractionBlocked)
	{
		OnRunnerRequested.Broadcast(CardId);
	}
}

void UFMCodexInteractionPanelWidget::RequestHelper(const FName CardId)
{
	if (!bInteractionBlocked)
	{
		OnHelperRequested.Broadcast(CardId);
	}
}

void UFMCodexInteractionPanelWidget::RequestDecline()
{
	if (!bInteractionBlocked)
	{
		OnDeclineRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestNoLegal()
{
	if (!bInteractionBlocked)
	{
		OnNoLegalRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::RequestBranch(
	const EFMCodexUMGBranchIntent Intent)
{
	if (!bInteractionBlocked)
	{
		OnBranchRequested.Broadcast(Intent);
	}
}

void UFMCodexInteractionPanelWidget::RequestOneOnOne(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	if (!bInteractionBlocked)
	{
		OnOneOnOneRequested.Broadcast(Choice);
	}
}

void UFMCodexInteractionPanelWidget::RequestContinue()
{
	if (!bInteractionBlocked)
	{
		OnContinueRequested.Broadcast();
	}
}

void UFMCodexInteractionPanelWidget::BuildWidgetTree()
{
	using namespace FMCodexInteractionPanelWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InteractionPanelBounds"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Bounds->SetMinDesiredWidth(Style.GetPanelMinWidth());
	Bounds->SetMaxDesiredWidth(Style.GetPanelMaxWidth());
	WidgetTree->RootWidget = Bounds;

	UBorder* Frame = MakeRegion(
		*WidgetTree, TEXT("InteractionPanelFrame"),
		EFMCodexPlayerUIColorRole::PanelBackground,
		Style.GetPanelPadding());
	Bounds->AddChild(Frame);
	UHorizontalBox* Body = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InteractionPanelHierarchy"));
	Frame->AddChild(Body);

	ActionHeaderRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionActionHeader"),
		EFMCodexPlayerUIColorRole::NeutralAccent,
		FMargin(10.0f, 6.0f));
	ActionHeaderRegion->SetVerticalAlignment(VAlign_Center);
	UVerticalBox* HeaderBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionActionHeaderBody"));
	KickerText = MakeText(*WidgetTree, TEXT("InteractionActionKicker"));
	ActorText = MakeText(*WidgetTree, TEXT("InteractionExpectedActor"));
	TitleText = MakeText(*WidgetTree, TEXT("InteractionActionTitle"));
	ContextText = MakeText(*WidgetTree, TEXT("InteractionActionContext"));
	KickerText->SetAutoWrapText(false);
	ActorText->SetAutoWrapText(false);
	TitleText->SetAutoWrapText(false);
	ContextText->SetAutoWrapText(false);
	KickerText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	ActorText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	TitleText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	ContextText->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
	Style.ApplyText(*KickerText, EFMCodexPlayerUITextRole::Kicker);
	Style.ApplyText(*ActorText, EFMCodexPlayerUITextRole::Status);
	Style.ApplyText(*TitleText, EFMCodexPlayerUITextRole::ActionTitle);
	Style.ApplyText(*ContextText, EFMCodexPlayerUITextRole::Secondary);
	// Match-level state belongs in the Broadcast Header. The Dock begins with
	// the acting-player hint and operation context only.
	HeaderBody->AddChildToVerticalBox(ActorText);
	HeaderBody->AddChildToVerticalBox(TitleText);
	HeaderBody->AddChildToVerticalBox(ContextText);
	ActionHeaderRegion->AddChild(HeaderBody);
	Body->AddChildToHorizontalBox(ActionHeaderRegion);

	CandidateRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionCandidateRegion"),
		EFMCodexPlayerUIColorRole::PanelInset,
		Style.GetSectionPadding());
	UVerticalBox* CandidateBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionCandidateBody"));
	DeploymentHandInstructionText = MakeText(
		*WidgetTree, TEXT("DeploymentHandInstruction"));
	DeploymentHandInstructionText->SetAutoWrapText(false);
	DeploymentHandInstructionText->SetTextOverflowPolicy(
		ETextOverflowPolicy::Ellipsis);
	Style.ApplyText(*DeploymentHandInstructionText,
		EFMCodexPlayerUITextRole::SectionHeading);
	CandidateBody->AddChildToVerticalBox(DeploymentHandInstructionText);
	UScrollBox* CandidateScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("InteractionCandidateScroll"));
	CandidateScroll->SetOrientation(Orient_Horizontal);
	CandidateCardsBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InteractionCandidateCards"));
	CandidateScroll->AddChild(CandidateCardsBody);
	CandidateBody->AddChildToVerticalBox(CandidateScroll);
	CandidateRegion->AddChild(CandidateBody);
	Body->AddChildToHorizontalBox(CandidateRegion);

	ChoiceRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionChoiceRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	UVerticalBox* ChoiceBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionChoiceBody"));
	ChoiceSectionText = MakeText(
		*WidgetTree, TEXT("InteractionChoiceSectionTitle"));
	Style.ApplyText(
		*ChoiceSectionText, EFMCodexPlayerUITextRole::SectionHeading);
	// The dock is intentionally a fixed-height strip. Route choices must stay
	// on one row; a WrapBox can move the second choice below the clipped dock.
	ChoiceOptionsBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InteractionChoiceOptions"));
	ChoiceBody->AddChildToVerticalBox(ChoiceSectionText);
	ChoiceBody->AddChildToVerticalBox(ChoiceOptionsBody);
	ChoiceRegion->AddChild(ChoiceBody);
	Body->AddChildToHorizontalBox(ChoiceRegion);

	UHorizontalBox* SecondaryActions =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("InteractionSecondaryActions"));
	UTextBlock* DeclineLabel = nullptr;
	DeclineButton = MakeButton(
		*WidgetTree, TEXT("InteractionDeclineButton"), DeclineLabel,
		EFMCodexPlayerUIActionRole::Decline);
	DeclineButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleDeclineClicked);
	SecondaryActions->AddChildToHorizontalBox(DeclineButton);
	UTextBlock* NoLegalLabel = nullptr;
	NoLegalButton = MakeButton(
		*WidgetTree, TEXT("InteractionNoLegalButton"), NoLegalLabel,
		EFMCodexPlayerUIActionRole::Secondary);
	NoLegalButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleNoLegalClicked);
	SecondaryActions->AddChildToHorizontalBox(NoLegalButton);
	UTextBlock* TacticalReferenceLabel = nullptr;
	DeploymentTacticalReferenceButton = MakeButton(
		*WidgetTree, TEXT("DeploymentTacticalReferenceEntryButton"),
		TacticalReferenceLabel, EFMCodexPlayerUIActionRole::Secondary);
	DeploymentTacticalReferenceButton->OnClicked.AddDynamic(
		this,
		&UFMCodexInteractionPanelWidget::HandleDeploymentTacticalReferenceClicked);
	SecondaryActions->AddChildToHorizontalBox(
		DeploymentTacticalReferenceButton);
	Body->AddChildToHorizontalBox(SecondaryActions);

	UHorizontalBox* PrimaryActions =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("InteractionPrimaryActions"));
	UTextBlock* StartLabel = nullptr;
	StartButton = MakeButton(
		*WidgetTree, TEXT("InteractionStartMatchButton"), StartLabel,
		EFMCodexPlayerUIActionRole::Primary);
	StartButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleStartClicked);
	PrimaryActions->AddChildToHorizontalBox(StartButton);
	UTextBlock* RollLabel = nullptr;
	TacticalPointRollButton = MakeButton(
		*WidgetTree, TEXT("InteractionTacticalPointRollButton"), RollLabel,
		EFMCodexPlayerUIActionRole::Primary);
	Style.ApplyText(*RollLabel, EFMCodexPlayerUITextRole::Kicker);
	TacticalPointRollButton->OnClicked.AddDynamic(
		this,
		&UFMCodexInteractionPanelWidget::HandleTacticalPointRollClicked);
	USizeBox* TacticalPointRollBounds =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), TEXT("TacticalPointPrimaryActionBounds"));
	TacticalPointRollBounds->SetWidthOverride(156.0f);
	TacticalPointRollBounds->SetHeightOverride(48.0f);
	TacticalPointRollBounds->AddChild(TacticalPointRollButton);
	if (UHorizontalBoxSlot* RollSlot =
		PrimaryActions->AddChildToHorizontalBox(TacticalPointRollBounds))
	{
		RollSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
		RollSlot->SetVerticalAlignment(VAlign_Center);
	}
	UTextBlock* FinishLabel = nullptr;
	FinishButton = MakeButton(
		*WidgetTree, TEXT("InteractionFinishDeploymentButton"), FinishLabel,
		EFMCodexPlayerUIActionRole::Primary);
	FinishButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleFinishClicked);
	PrimaryActions->AddChildToHorizontalBox(FinishButton);
	UTextBlock* ContinueLabel = nullptr;
	ContinueButton = MakeButton(
		*WidgetTree, TEXT("InteractionContinueButton"), ContinueLabel,
		EFMCodexPlayerUIActionRole::Primary);
	ContinueButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleContinueClicked);
	PrimaryActions->AddChildToHorizontalBox(ContinueButton);
	Body->AddChildToHorizontalBox(PrimaryActions);

	EmptyStateText = MakeText(*WidgetTree, TEXT("InteractionBoundedFallback"));
	Style.ApplyText(*EmptyStateText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToHorizontalBox(EmptyStateText);
}

void UFMCodexInteractionPanelWidget::RefreshVisuals()
{
	if (KickerText == nullptr)
	{
		return;
	}
	KickerText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.KickerLabel.IsEmpty()
			? TEXT("LOCAL MATCH") : Presentation.KickerLabel));
	KickerText->SetVisibility(ESlateVisibility::Collapsed);
	ActorText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.ExpectedActorLabel));
	TitleText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.TitleLabel.IsEmpty()
			? TEXT("Interaction unavailable") : Presentation.TitleLabel));
	TArray<FString> ContextLines;
	if (!Presentation.bUseOnPitchPlayerSelection)
	{
		ContextLines.Add(Presentation.CategoryLabel);
	}
	ContextLines.Add(Presentation.OnPitchSelectionHintLabel);
	ContextLines.RemoveAll(
		[](const FString& Line) { return Line.IsEmpty(); });
	TArray<FString> LocalizedContextLines;
	for (const FString& Line : ContextLines)
	{
		LocalizedContextLines.Add(
			FFMCodexPlayerUIPresentationText::MatchScreenLabel(Line).ToString());
	}
	ContextText->SetText(FText::FromString(
		FString::Join(LocalizedContextLines, TEXT(" | "))));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	const bool bCompactTacticalPointAction =
		Presentation.bCanRollTacticalPoints;
	Style.ApplyText(*ActorText, bCompactTacticalPointAction
		? EFMCodexPlayerUITextRole::Kicker
		: EFMCodexPlayerUITextRole::Status);
	TitleText->SetVisibility(bCompactTacticalPointAction
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	ContextText->SetVisibility(bCompactTacticalPointAction
		? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	ActionHeaderRegion->SetBrushColor(
		Presentation.KickerLabel.Contains(TEXT("SYSTEM"))
			? Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)
			: Style.GetPlayerAccentColor(Presentation.ExpectedActorLabel));
	ChoiceSectionText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.BranchSectionLabel.IsEmpty()
			? TEXT("LEGAL OPTIONS") : Presentation.BranchSectionLabel));
	DeploymentHandInstructionText->SetText(
		FFMCodexPlayerUIPresentationText::DeploymentHandInstruction());
	DeploymentHandInstructionText->SetVisibility(
		Presentation.Category == EFMCodexUMGInteractionCategory::Deploy
			? ESlateVisibility::HitTestInvisible
			: ESlateVisibility::Collapsed);

	RefreshCandidateChoices();
	CandidateRegion->SetVisibility(
		Presentation.Category == EFMCodexUMGInteractionCategory::Deploy
			|| (!Presentation.bUseOnPitchPlayerSelection
				&& !Presentation.SelectionChoices.IsEmpty())
				? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ChoiceRegion->SetVisibility(
		!Presentation.BranchChoices.IsEmpty()
			|| !Presentation.OneOnOneChoices.IsEmpty()
				? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	auto SetButton = [](UButton* Button, const bool bVisible,
		const FString& Label)
	{
		Button->SetVisibility(bVisible
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (UTextBlock* LabelText = Cast<UTextBlock>(Button->GetChildAt(0)))
		{
			LabelText->SetText(
				FFMCodexPlayerUIPresentationText::MatchScreenLabel(Label));
		}
	};
	SetButton(StartButton, Presentation.bCanStartNewMatch,
		Presentation.PrimaryAction.Label);
	SetButton(TacticalPointRollButton, Presentation.bCanRollTacticalPoints,
		Presentation.PrimaryAction.Label);
	if (UWidget* RollBounds = TacticalPointRollButton->GetParent())
	{
		RollBounds->SetVisibility(Presentation.bCanRollTacticalPoints
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (Presentation.bCanRollTacticalPoints
		&& Presentation.bHasActingSidePrimaryColor)
	{
		TacticalPointRollButton->SetStyle(
			Style.MakeAccentButtonStyle(
				Presentation.ActingSidePrimaryColor));
		const FLinearColor PanelBase =
			Style.GetColor(EFMCodexPlayerUIColorRole::PanelRaised);
		ActionHeaderRegion->SetBrushColor(FMath::Lerp(
			PanelBase, Presentation.ActingSidePrimaryColor, 0.30f));
	}
	SetButton(FinishButton, Presentation.bCanFinishDeployment,
		Presentation.PrimaryAction.Label);
	const bool bUseContinueButton = Presentation.PrimaryAction.bAvailable
		&& !Presentation.bCanStartNewMatch
		&& !Presentation.bCanRollTacticalPoints
		&& !Presentation.bCanFinishDeployment;
	SetButton(ContinueButton, bUseContinueButton,
		Presentation.PrimaryAction.Label);
	SetButton(DeclineButton, Presentation.bCanDecline,
		Presentation.DeclineActionLabel);
	SetButton(NoLegalButton, Presentation.bCanResolveNoLegal,
		Presentation.NoLegalActionLabel);
	SetButton(DeploymentTacticalReferenceButton,
		Presentation.Category == EFMCodexUMGInteractionCategory::Deploy,
		TEXT("TACTICAL REFERENCE"));

	const bool bHasDynamicChoices = !RenderedOptionWidgets.IsEmpty();
	const bool bHasPrimaryAction = Presentation.PrimaryAction.bAvailable
		|| Presentation.bCanDecline
		|| Presentation.bCanResolveNoLegal;
	const FString Fallback = !Presentation.EmptyStateLabel.IsEmpty()
		? Presentation.EmptyStateLabel
		: Presentation.bUseOnPitchPlayerSelection
			? FString()
		: !bHasDynamicChoices && !bHasPrimaryAction
			? FString(TEXT("No player action is available.")) : FString();
	EmptyStateText->SetText(
		FFMCodexPlayerUIPresentationText::MatchScreenLabel(Fallback));
	EmptyStateText->SetVisibility(Fallback.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	SetIsEnabled(!bInteractionBlocked);
}

void UFMCodexInteractionPanelWidget::RefreshCandidateChoices()
{
	using namespace FMCodexInteractionPanelWidget;
	CandidateCardsBody->ClearChildren();
	ChoiceOptionsBody->ClearChildren();
	RenderedCandidateCardWidgets.Reset();
	RenderedOptionWidgets.Reset();

	UClass* CardClass = PlayerCardWidgetClass != nullptr
		? PlayerCardWidgetClass.Get() : UFMCodexPlayerCardWidget::StaticClass();
	// Deployment cards live in the persistent local rack. The dock only
	// presents contextual actions, so it cannot resize the pitch.

	if (Presentation.bUseOnPitchPlayerSelection)
	{
		return;
	}

	for (int32 ChoiceIndex = 0;
		ChoiceIndex < Presentation.SelectionChoices.Num(); ++ChoiceIndex)
	{
		const FFMCodexUMGSelectionChoiceViewModel& Choice =
			Presentation.SelectionChoices[ChoiceIndex];
		UVerticalBox* Group = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*FString::Printf(
				TEXT("SelectionChoiceGroup%d"), ChoiceIndex)));
		if (Choice.bHasCard)
		{
			// The permanent rack already preserves card identity. Selection
			// actions stay compact in the stable bottom dock.
		}
		UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
			FName(*FString::Printf(TEXT("SelectionOption%d"), ChoiceIndex)));
		if (Presentation.Category == EFMCodexUMGInteractionCategory::SelectSkill)
		{
			Option->ConfigureTacticalCard(
				Choice.Label, Choice.SecondaryLabel, Choice.OptionId);
			Option->OnTacticalDetailRequested.AddDynamic(
				this,
				&UFMCodexInteractionPanelWidget::HandleTacticalDetailRequested);
			Option->OnTacticalDetailDismissed.AddDynamic(
				this,
				&UFMCodexInteractionPanelWidget::HandleTacticalDetailDismissed);
		}
		else
		{
			Option->ConfigureCard(Choice.Label, Choice.OptionId);
		}
		switch (Presentation.Category)
		{
		case EFMCodexUMGInteractionCategory::SelectCarrier:
			Option->OnCardRequested.AddDynamic(
				this, &UFMCodexInteractionPanelWidget::HandleCarrierOption);
			break;
		case EFMCodexUMGInteractionCategory::SelectMarker:
			Option->OnCardRequested.AddDynamic(
				this, &UFMCodexInteractionPanelWidget::HandleMarkerOption);
			break;
		case EFMCodexUMGInteractionCategory::SelectSkill:
			Option->OnCardRequested.AddDynamic(
				this, &UFMCodexInteractionPanelWidget::HandleSkillOption);
			break;
		case EFMCodexUMGInteractionCategory::SelectRunner:
			Option->OnCardRequested.AddDynamic(
				this, &UFMCodexInteractionPanelWidget::HandleRunnerOption);
			break;
		case EFMCodexUMGInteractionCategory::SelectHelper:
			Option->OnCardRequested.AddDynamic(
				this, &UFMCodexInteractionPanelWidget::HandleHelperOption);
			break;
		default:
			break;
		}
		Group->AddChildToVerticalBox(Option);
		CandidateCardsBody->AddChildToHorizontalBox(Group);
	}

	for (int32 Index = 0; Index < Presentation.BranchChoices.Num(); ++Index)
	{
		const FFMCodexUMGBranchChoiceViewModel& Choice =
			Presentation.BranchChoices[Index];
		UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
			FName(*FString::Printf(TEXT("BranchIntentOption%d"), Index)));
		Option->ConfigureBranch(Choice.Label, Choice.Intent);
		Option->OnBranchRequested.AddDynamic(
			this, &UFMCodexInteractionPanelWidget::HandleBranchOption);
		if (UHorizontalBoxSlot* ChoiceSlot =
			ChoiceOptionsBody->AddChildToHorizontalBox(Option))
		{
			const FVector2D Gap =
				FFMCodexPlayerUIStyle::Get().GetControlGap();
			ChoiceSlot->SetPadding(FMargin(
				Index == 0 ? 0.0f : Gap.X, 0.0f, 0.0f, 0.0f));
			ChoiceSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
	for (int32 Index = 0; Index < Presentation.OneOnOneChoices.Num(); ++Index)
	{
		const FFMCodexUMGOneOnOneChoiceViewModel& Choice =
			Presentation.OneOnOneChoices[Index];
		UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
			FName(*FString::Printf(TEXT("OneOnOneOption%d"), Index)));
		Option->ConfigureOneOnOne(
			Choice.Label, Choice.SecondaryLabel, Choice.Choice);
		Option->OnOneOnOneRequested.AddDynamic(
			this, &UFMCodexInteractionPanelWidget::HandleOneOnOneOption);
		if (UHorizontalBoxSlot* ChoiceSlot =
			ChoiceOptionsBody->AddChildToHorizontalBox(Option))
		{
			const FVector2D Gap =
				FFMCodexPlayerUIStyle::Get().GetControlGap();
			ChoiceSlot->SetPadding(FMargin(
				Index == 0 ? 0.0f : Gap.X, 0.0f, 0.0f, 0.0f));
			ChoiceSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

UFMCodexInteractionOptionWidget*
UFMCodexInteractionPanelWidget::MakeOptionWidget(const FName Name)
{
	UClass* ResolvedClass = OptionWidgetClass != nullptr
		? OptionWidgetClass.Get() : UFMCodexInteractionOptionWidget::StaticClass();
	UFMCodexInteractionOptionWidget* Result =
		WidgetTree->ConstructWidget<UFMCodexInteractionOptionWidget>(
			ResolvedClass, Name);
	RenderedOptionWidgets.Add(Result);
	return Result;
}

void UFMCodexInteractionPanelWidget::HandleDeploymentCardDragStarted(
	const FName CardId,
	const bool bGoalkeeper)
{
	if (!bInteractionBlocked)
	{
		OnDeploymentDragStarted.Broadcast(CardId, bGoalkeeper);
	}
}

void UFMCodexInteractionPanelWidget::HandleDeploymentCardDragFinished()
{
	OnDeploymentDragFinished.Broadcast();
}

void UFMCodexInteractionPanelWidget::HandleStartClicked()
{
	RequestStartMatch();
}

void UFMCodexInteractionPanelWidget::HandleTacticalPointRollClicked()
{
	RequestTacticalPointRoll();
}

void UFMCodexInteractionPanelWidget::HandleFinishClicked()
{
	RequestFinishDeployment();
}

void UFMCodexInteractionPanelWidget::
HandleDeploymentTacticalReferenceClicked()
{
	RequestDeploymentTacticalReference();
}

void UFMCodexInteractionPanelWidget::HandleDeclineClicked()
{
	RequestDecline();
}

void UFMCodexInteractionPanelWidget::HandleNoLegalClicked()
{
	RequestNoLegal();
}

void UFMCodexInteractionPanelWidget::HandleContinueClicked()
{
	RequestContinue();
}

void UFMCodexInteractionPanelWidget::HandleDeploymentOption(
	const FName CardId,
	const FName SlotId,
	const bool bGoalkeeper)
{
	RequestDeployment(CardId, SlotId, bGoalkeeper);
}

void UFMCodexInteractionPanelWidget::HandleCarrierOption(const FName CardId)
{
	RequestCarrier(CardId);
}

void UFMCodexInteractionPanelWidget::HandleMarkerOption(const FName CardId)
{
	RequestMarker(CardId);
}

void UFMCodexInteractionPanelWidget::HandleSkillOption(const FName SkillId)
{
	RequestSkill(SkillId);
}

void UFMCodexInteractionPanelWidget::HandleTacticalDetailRequested(
	const FName SkillId)
{
	if (!bInteractionBlocked
		&& Presentation.Category == EFMCodexUMGInteractionCategory::SelectSkill)
	{
		OnTacticalDetailRequested.Broadcast(SkillId);
	}
}

void UFMCodexInteractionPanelWidget::HandleTacticalDetailDismissed(
	const FName SkillId)
{
	OnTacticalDetailDismissed.Broadcast(SkillId);
}

void UFMCodexInteractionPanelWidget::HandleRunnerOption(const FName CardId)
{
	RequestRunner(CardId);
}

void UFMCodexInteractionPanelWidget::HandleHelperOption(const FName CardId)
{
	RequestHelper(CardId);
}

void UFMCodexInteractionPanelWidget::HandleBranchOption(
	const EFMCodexUMGBranchIntent Intent)
{
	RequestBranch(Intent);
}

void UFMCodexInteractionPanelWidget::HandleOneOnOneOption(
	const EFMCodexUMGOneOnOneChoice Choice)
{
	RequestOneOnOne(Choice);
}
