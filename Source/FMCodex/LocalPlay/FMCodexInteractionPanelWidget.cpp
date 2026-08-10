#include "FMCodexInteractionPanelWidget.h"

#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"

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

void UFMCodexInteractionPanelWidget::RequestBeginAttack()
{
	if (!bInteractionBlocked)
	{
		OnBeginAttackRequested.Broadcast();
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
	UVerticalBox* Body = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionPanelHierarchy"));
	Frame->AddChild(Body);

	ActionHeaderRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionActionHeader"),
		EFMCodexPlayerUIColorRole::NeutralAccent,
		Style.GetSectionPadding());
	UVerticalBox* HeaderBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionActionHeaderBody"));
	KickerText = MakeText(*WidgetTree, TEXT("InteractionActionKicker"));
	ActorText = MakeText(*WidgetTree, TEXT("InteractionExpectedActor"));
	TitleText = MakeText(*WidgetTree, TEXT("InteractionActionTitle"));
	ContextText = MakeText(*WidgetTree, TEXT("InteractionActionContext"));
	Style.ApplyText(*KickerText, EFMCodexPlayerUITextRole::Kicker);
	Style.ApplyText(*ActorText, EFMCodexPlayerUITextRole::Status);
	Style.ApplyText(*TitleText, EFMCodexPlayerUITextRole::ActionTitle);
	Style.ApplyText(*ContextText, EFMCodexPlayerUITextRole::Secondary);
	HeaderBody->AddChildToVerticalBox(KickerText);
	HeaderBody->AddChildToVerticalBox(ActorText);
	HeaderBody->AddChildToVerticalBox(TitleText);
	HeaderBody->AddChildToVerticalBox(ContextText);
	ActionHeaderRegion->AddChild(HeaderBody);
	Body->AddChildToVerticalBox(ActionHeaderRegion);

	UBorder* CandidateRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionCandidateRegion"),
		EFMCodexPlayerUIColorRole::PanelInset,
		Style.GetSectionPadding());
	UScrollBox* CandidateScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("InteractionCandidateScroll"));
	CandidateScroll->SetOrientation(Orient_Horizontal);
	CandidateCardsBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InteractionCandidateCards"));
	CandidateScroll->AddChild(CandidateCardsBody);
	CandidateRegion->AddChild(CandidateScroll);
	Body->AddChildToVerticalBox(CandidateRegion);

	UBorder* ChoiceRegion = MakeRegion(
		*WidgetTree, TEXT("InteractionChoiceRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	UVerticalBox* ChoiceBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InteractionChoiceBody"));
	ChoiceSectionText = MakeText(
		*WidgetTree, TEXT("InteractionChoiceSectionTitle"));
	Style.ApplyText(
		*ChoiceSectionText, EFMCodexPlayerUITextRole::SectionHeading);
	ChoiceOptionsBody = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("InteractionChoiceOptions"));
	ChoiceOptionsBody->SetInnerSlotPadding(Style.GetControlGap());
	ChoiceBody->AddChildToVerticalBox(ChoiceSectionText);
	ChoiceBody->AddChildToVerticalBox(ChoiceOptionsBody);
	ChoiceRegion->AddChild(ChoiceBody);
	Body->AddChildToVerticalBox(ChoiceRegion);

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
	Body->AddChildToVerticalBox(SecondaryActions);

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
	UTextBlock* BeginLabel = nullptr;
	BeginButton = MakeButton(
		*WidgetTree, TEXT("InteractionBeginAttackButton"), BeginLabel,
		EFMCodexPlayerUIActionRole::Primary);
	BeginButton->OnClicked.AddDynamic(
		this, &UFMCodexInteractionPanelWidget::HandleBeginClicked);
	PrimaryActions->AddChildToHorizontalBox(BeginButton);
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
	Body->AddChildToVerticalBox(PrimaryActions);

	EmptyStateText = MakeText(*WidgetTree, TEXT("InteractionBoundedFallback"));
	Style.ApplyText(*EmptyStateText, EFMCodexPlayerUITextRole::Secondary);
	Body->AddChildToVerticalBox(EmptyStateText);
}

void UFMCodexInteractionPanelWidget::RefreshVisuals()
{
	if (KickerText == nullptr)
	{
		return;
	}
	KickerText->SetText(FText::FromString(Presentation.KickerLabel.IsEmpty()
		? TEXT("LOCAL MATCH") : Presentation.KickerLabel));
	ActorText->SetText(FText::FromString(Presentation.ExpectedActorLabel));
	TitleText->SetText(FText::FromString(Presentation.TitleLabel.IsEmpty()
		? TEXT("Interaction unavailable") : Presentation.TitleLabel));
	TArray<FString> ContextLines = {
		Presentation.CategoryLabel,
		Presentation.ActionPointLabel
	};
	ContextLines.RemoveAll(
		[](const FString& Line) { return Line.IsEmpty(); });
	ContextText->SetText(FText::FromString(
		FString::Join(ContextLines, TEXT(" | "))));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	ActionHeaderRegion->SetBrushColor(
		Presentation.KickerLabel.Contains(TEXT("SYSTEM"))
			? Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)
			: Style.GetPlayerAccentColor(Presentation.ExpectedActorLabel));
	ChoiceSectionText->SetText(FText::FromString(
		Presentation.BranchSectionLabel.IsEmpty()
			? TEXT("LEGAL OPTIONS") : Presentation.BranchSectionLabel));

	RefreshCandidateChoices();

	auto SetButton = [](UButton* Button, const bool bVisible,
		const FString& Label)
	{
		Button->SetVisibility(bVisible
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (UTextBlock* LabelText = Cast<UTextBlock>(Button->GetChildAt(0)))
		{
			LabelText->SetText(FText::FromString(Label));
		}
	};
	SetButton(StartButton, Presentation.bCanStartNewMatch,
		Presentation.PrimaryActionLabel);
	SetButton(BeginButton, Presentation.bCanBeginOrdinaryAttack,
		Presentation.PrimaryActionLabel);
	SetButton(FinishButton, Presentation.bCanFinishDeployment,
		Presentation.PrimaryActionLabel);
	SetButton(ContinueButton, Presentation.bCanContinue,
		Presentation.PrimaryActionLabel);
	SetButton(DeclineButton, Presentation.bCanDecline,
		Presentation.DeclineActionLabel);
	SetButton(NoLegalButton, Presentation.bCanResolveNoLegal,
		Presentation.NoLegalActionLabel);

	const bool bHasDynamicChoices = !RenderedOptionWidgets.IsEmpty();
	const bool bHasPrimaryAction = Presentation.bCanStartNewMatch
		|| Presentation.bCanBeginOrdinaryAttack
		|| Presentation.bCanFinishDeployment
		|| Presentation.bCanContinue
		|| Presentation.bCanDecline
		|| Presentation.bCanResolveNoLegal;
	const FString Fallback = !Presentation.EmptyStateLabel.IsEmpty()
		? Presentation.EmptyStateLabel
		: !bHasDynamicChoices && !bHasPrimaryAction
			? FString(TEXT("No player action is available.")) : FString();
	EmptyStateText->SetText(FText::FromString(Fallback));
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
	for (int32 ChoiceIndex = 0;
		ChoiceIndex < Presentation.DeploymentChoices.Num(); ++ChoiceIndex)
	{
		const FFMCodexUMGDeploymentChoiceViewModel& Choice =
			Presentation.DeploymentChoices[ChoiceIndex];
		UVerticalBox* Group = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*FString::Printf(
				TEXT("DeploymentChoiceGroup%d"), ChoiceIndex)));
		UFMCodexPlayerCardWidget* Card =
			WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
				CardClass, FName(*FString::Printf(
					TEXT("DeploymentCandidateCard%d"), ChoiceIndex)));
		Card->RefreshFromPresentation(
			Choice.Card,
			EFMCodexPlayerCardPresentationMode::InteractionChoice);
		Group->AddChildToVerticalBox(Card);
		RenderedCandidateCardWidgets.Add(Card);
		UWrapBox* Destinations = WidgetTree->ConstructWidget<UWrapBox>(
			UWrapBox::StaticClass(), FName(*FString::Printf(
				TEXT("DeploymentDestinations%d"), ChoiceIndex)));
		for (int32 DestinationIndex = 0;
			DestinationIndex < Choice.Destinations.Num(); ++DestinationIndex)
		{
			const FFMCodexUMGDeploymentDestinationViewModel& Destination =
				Choice.Destinations[DestinationIndex];
			UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
				FName(*FString::Printf(TEXT("DeploymentOption%d_%d"),
					ChoiceIndex, DestinationIndex)));
			Option->ConfigureDeployment(
				Destination.Label, Choice.CardId,
				Destination.SlotId, Choice.bGoalkeeper);
			Option->OnDeploymentRequested.AddDynamic(
				this,
				&UFMCodexInteractionPanelWidget::HandleDeploymentOption);
			Destinations->AddChildToWrapBox(Option);
		}
		Group->AddChildToVerticalBox(Destinations);
		CandidateCardsBody->AddChildToHorizontalBox(Group);
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
			UFMCodexPlayerCardWidget* Card =
				WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
					CardClass, FName(*FString::Printf(
						TEXT("SelectionCandidateCard%d"), ChoiceIndex)));
			Card->RefreshFromPresentation(
				Choice.Card,
				EFMCodexPlayerCardPresentationMode::InteractionChoice);
			Group->AddChildToVerticalBox(Card);
			RenderedCandidateCardWidgets.Add(Card);
		}
		UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
			FName(*FString::Printf(TEXT("SelectionOption%d"), ChoiceIndex)));
		Option->ConfigureCard(Choice.Label, Choice.OptionId);
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
		ChoiceOptionsBody->AddChildToWrapBox(Option);
	}
	for (int32 Index = 0; Index < Presentation.OneOnOneChoices.Num(); ++Index)
	{
		const FFMCodexUMGOneOnOneChoiceViewModel& Choice =
			Presentation.OneOnOneChoices[Index];
		UFMCodexInteractionOptionWidget* Option = MakeOptionWidget(
			FName(*FString::Printf(TEXT("OneOnOneOption%d"), Index)));
		Option->ConfigureOneOnOne(Choice.Label, Choice.Choice);
		Option->OnOneOnOneRequested.AddDynamic(
			this, &UFMCodexInteractionPanelWidget::HandleOneOnOneOption);
		ChoiceOptionsBody->AddChildToWrapBox(Option);
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

void UFMCodexInteractionPanelWidget::HandleStartClicked()
{
	RequestStartMatch();
}

void UFMCodexInteractionPanelWidget::HandleBeginClicked()
{
	RequestBeginAttack();
}

void UFMCodexInteractionPanelWidget::HandleFinishClicked()
{
	RequestFinishDeployment();
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
