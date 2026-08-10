#include "FMCodexLocalMatchScreenWidget.h"

#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPitchWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

namespace FMCodexLocalMatchScreenWidget
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

	UBorder* MakeRegion(UWidgetTree& Tree, const FName Name)
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		Result->SetPadding(FMargin(12.0f));
		Result->SetBrushColor(FLinearColor(0.025f, 0.045f, 0.07f, 0.96f));
		return Result;
	}

	UButton* MakeButton(
		UWidgetTree& Tree,
		const FName Name,
		const FString& Label)
	{
		UButton* Result = Tree.ConstructWidget<UButton>(
			UButton::StaticClass(), Name);
		Result->AddChild(MakeText(
			Tree, FName(*(Name.ToString() + TEXT("Label"))), Label));
		return Result;
	}
}

UFMCodexLocalMatchScreenWidget::UFMCodexLocalMatchScreenWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PitchWidgetClass = UFMCodexPitchWidget::StaticClass();
}

void UFMCodexLocalMatchScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexLocalMatchScreenWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexLocalMatchScreenWidget::SetMatchController(
	AFMCodexLocalMatchPlayerController* InController)
{
	MatchController = InController;
}

void UFMCodexLocalMatchScreenWidget::ClearMatchController()
{
	MatchController = nullptr;
}

void UFMCodexLocalMatchScreenWidget::RefreshFromPresentation(
	const FFMCodexUMGMatchScreenViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGMatchScreenViewModel&
UFMCodexLocalMatchScreenWidget::GetPresentation() const
{
	return Presentation;
}

AFMCodexLocalMatchPlayerController*
UFMCodexLocalMatchScreenWidget::GetMatchController() const
{
	return MatchController;
}

UFMCodexPitchWidget* UFMCodexLocalMatchScreenWidget::GetPitchWidget() const
{
	return PitchWidget;
}

void UFMCodexLocalMatchScreenWidget::RequestStartNewMatch()
{
	if (MatchController != nullptr)
	{
		MatchController->StartNewDemoMatch();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestBeginOrdinaryAttack()
{
	if (MatchController != nullptr)
	{
		MatchController->BeginDemoOrdinaryAttack();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestDeployOrdinary(
	const FName CardId,
	const FName SlotId)
{
	if (MatchController != nullptr)
	{
		MatchController->DeployOrdinary(CardId, SlotId);
	}
}

void UFMCodexLocalMatchScreenWidget::RequestFinishDeployment()
{
	if (MatchController != nullptr)
	{
		MatchController->FinishDeployment();
	}
}

void UFMCodexLocalMatchScreenWidget::RequestReady()
{
	if (MatchController != nullptr)
	{
		MatchController->AcknowledgeHotSeatHandoff();
	}
}

void UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked()
{
	RequestStartNewMatch();
}

void UFMCodexLocalMatchScreenWidget::HandleBeginOrdinaryAttackClicked()
{
	RequestBeginOrdinaryAttack();
}

void UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked()
{
	RequestFinishDeployment();
}

void UFMCodexLocalMatchScreenWidget::HandleReadyClicked()
{
	RequestReady();
}

void UFMCodexLocalMatchScreenWidget::BuildWidgetTree()
{
	using namespace FMCodexLocalMatchScreenWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(
		UOverlay::StaticClass(), TEXT("MatchScreenRoot"));
	WidgetTree->RootWidget = Root;

	UScrollBox* MainScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("PlayerMatchScroll"));
	Root->AddChildToOverlay(MainScroll);
	MainScreen = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("FourRegionMatchScreen"));
	MainScroll->AddChild(MainScreen);

	UBorder* HeaderRegion = MakeRegion(*WidgetTree, TEXT("MatchHeaderRegion"));
	HeaderText = MakeText(*WidgetTree, TEXT("MatchHeaderText"));
	HeaderRegion->AddChild(HeaderText);
	MainScreen->AddChildToVerticalBox(HeaderRegion);

	UBorder* PitchRegion = MakeRegion(*WidgetTree, TEXT("FootballCardFieldRegion"));
	UClass* ResolvedPitchClass = PitchWidgetClass != nullptr
		? PitchWidgetClass.Get() : UFMCodexPitchWidget::StaticClass();
	PitchWidget = WidgetTree->ConstructWidget<UFMCodexPitchWidget>(
		ResolvedPitchClass, TEXT("DedicatedFootballPitchWidget"));
	PitchRegion->AddChild(PitchWidget);
	MainScreen->AddChildToVerticalBox(PitchRegion);

	UBorder* InteractionRegion = MakeRegion(
		*WidgetTree, TEXT("CurrentInteractionRegion"));
	UVerticalBox* InteractionBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CurrentInteractionBody"));
	InteractionRegion->AddChild(InteractionBody);
	InteractionTitleText = MakeText(
		*WidgetTree, TEXT("CurrentInteractionTitle"));
	InteractionBody->AddChildToVerticalBox(InteractionTitleText);
	InteractionActionsText = MakeText(
		*WidgetTree, TEXT("CurrentInteractionActions"));
	InteractionBody->AddChildToVerticalBox(InteractionActionsText);
	CandidateCardsBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("InteractionCandidateCards"));
	UScrollBox* CandidateScroll = WidgetTree->ConstructWidget<UScrollBox>(
		UScrollBox::StaticClass(), TEXT("InteractionCandidateScroll"));
	CandidateScroll->SetOrientation(Orient_Horizontal);
	CandidateScroll->AddChild(CandidateCardsBody);
	InteractionBody->AddChildToVerticalBox(CandidateScroll);

	UHorizontalBox* CommandRow = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("TypedIntentButtons"));
	StartNewMatchButton = MakeButton(
		*WidgetTree, TEXT("StartNewMatchButton"), TEXT("Start New Match"));
	StartNewMatchButton->OnClicked.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleStartNewMatchClicked);
	CommandRow->AddChildToHorizontalBox(StartNewMatchButton);
	BeginOrdinaryAttackButton = MakeButton(
		*WidgetTree, TEXT("BeginOrdinaryAttackButton"),
		TEXT("Begin Ordinary Attack"));
	BeginOrdinaryAttackButton->OnClicked.AddDynamic(
		this,
		&UFMCodexLocalMatchScreenWidget::HandleBeginOrdinaryAttackClicked);
	CommandRow->AddChildToHorizontalBox(BeginOrdinaryAttackButton);
	FinishDeploymentButton = MakeButton(
		*WidgetTree, TEXT("FinishDeploymentButton"),
		TEXT("Finish Deployment"));
	FinishDeploymentButton->OnClicked.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleFinishDeploymentClicked);
	CommandRow->AddChildToHorizontalBox(FinishDeploymentButton);
	InteractionBody->AddChildToVerticalBox(CommandRow);
	MainScreen->AddChildToVerticalBox(InteractionRegion);

	UBorder* ResolutionRegion = MakeRegion(
		*WidgetTree, TEXT("ResolutionResultRegion"));
	ResolutionText = MakeText(*WidgetTree, TEXT("ResolutionResultText"));
	ResolutionRegion->AddChild(ResolutionText);
	MainScreen->AddChildToVerticalBox(ResolutionRegion);

	HandoffOverlay = MakeRegion(*WidgetTree, TEXT("HotSeatHandoffOverlay"));
	HandoffOverlay->SetBrushColor(FLinearColor(0.01f, 0.01f, 0.015f, 0.995f));
	UVerticalBox* HandoffBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HotSeatHandoffBody"));
	HandoffOverlay->AddChild(HandoffBody);
	HandoffText = MakeText(*WidgetTree, TEXT("HotSeatHandoffText"));
	HandoffBody->AddChildToVerticalBox(HandoffText);
	UButton* ReadyButton = MakeButton(
		*WidgetTree, TEXT("HotSeatReadyButton"), TEXT("Ready"));
	ReadyButton->OnClicked.AddDynamic(
		this, &UFMCodexLocalMatchScreenWidget::HandleReadyClicked);
	HandoffBody->AddChildToVerticalBox(ReadyButton);
	if (UOverlaySlot* HandoffSlot = Root->AddChildToOverlay(HandoffOverlay))
	{
		HandoffSlot->SetHorizontalAlignment(HAlign_Fill);
		HandoffSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UFMCodexLocalMatchScreenWidget::RefreshVisuals()
{
	if (HeaderText == nullptr)
	{
		return;
	}
	HeaderText->SetText(FText::FromString(FString::Printf(
		TEXT("MATCH HEADER\n%s  %s  %s\n%s\n%s\n%s"),
		*Presentation.Header.PlayerALabel,
		*Presentation.Header.ScoreLabel,
		*Presentation.Header.PlayerBLabel,
		*Presentation.Header.CurrentAttackerLabel,
		*Presentation.Header.ExpectedActorLabel,
		*Presentation.Header.MatchStatusLabel)));
	PitchWidget->RefreshFromPitchPresentation(Presentation.PitchRegions);
	RefreshInteraction();

	TArray<FString> ResultLines;
	ResultLines.Add(TEXT("RESOLUTION RESULT"));
	if (Presentation.Resolution.bVisible)
	{
		ResultLines.Add(Presentation.Resolution.StepLabel);
		ResultLines.Append(Presentation.Resolution.DiceLabels);
		ResultLines.Add(Presentation.Resolution.DecisionLabel);
		ResultLines.Add(Presentation.Resolution.TerminalLabel);
		ResultLines.Add(Presentation.Resolution.ErrorLabel);
	}
	else
	{
		ResultLines.Add(TEXT("Waiting for an authoritative result."));
	}
	ResultLines.RemoveAll([](const FString& Line) { return Line.IsEmpty(); });
	ResolutionText->SetText(FText::FromString(FString::Join(
		ResultLines, TEXT("\n"))));

	HandoffOverlay->SetVisibility(Presentation.Handoff.bVisible
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	MainScreen->SetIsEnabled(!Presentation.Handoff.bVisible);
	HandoffText->SetText(FText::FromString(FString::Printf(
		TEXT("%s\n%s\n%s"), *Presentation.Handoff.TitleLabel,
		*Presentation.Handoff.NextPlayerLabel,
		*Presentation.Handoff.ReadyLabel)));
}

void UFMCodexLocalMatchScreenWidget::RefreshInteraction()
{
	using namespace FMCodexLocalMatchScreenWidget;
	InteractionTitleText->SetText(FText::FromString(FString::Printf(
		TEXT("CURRENT INTERACTION\n%s | %s\n%s\n%s"),
		*Presentation.Interaction.KickerLabel,
		*Presentation.Interaction.ClassificationLabel,
		*Presentation.Interaction.TitleLabel,
		*Presentation.Interaction.CategoryLabel)));
	InteractionActionsText->SetText(FText::FromString(
		Presentation.Interaction.LegalActionLabels.IsEmpty()
			? TEXT("No player action required.")
			: FString::Join(Presentation.Interaction.LegalActionLabels,
				TEXT(" | "))));
	StartNewMatchButton->SetVisibility(
		Presentation.Interaction.bCanStartNewMatch
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	BeginOrdinaryAttackButton->SetVisibility(
		Presentation.Interaction.bCanBeginOrdinaryAttack
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	FinishDeploymentButton->SetVisibility(
		Presentation.Interaction.bCanFinishDeployment
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	CandidateCardsBody->ClearChildren();
	for (int32 Index = 0;
		Index < Presentation.Interaction.CandidateCards.Num(); ++Index)
	{
		USizeBox* CardSize = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(*FString::Printf(TEXT("InteractionCardSize%d"), Index)));
		CardSize->SetWidthOverride(260.0f);
		UFMCodexPlayerCardWidget* CardWidget =
			WidgetTree->ConstructWidget<UFMCodexPlayerCardWidget>(
				UFMCodexPlayerCardWidget::StaticClass(),
				FName(*FString::Printf(TEXT("InteractionCard%d"), Index)));
		CardWidget->RefreshFromPresentation(
			Presentation.Interaction.CandidateCards[Index]);
		CardSize->AddChild(CardWidget);
		CandidateCardsBody->AddChildToHorizontalBox(CardSize);
	}
}
