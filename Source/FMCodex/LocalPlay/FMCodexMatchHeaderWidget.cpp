#include "FMCodexMatchHeaderWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPlayerUIPresentationText.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

namespace FMCodexMatchHeaderWidget
{
	UTextBlock* MakeText(UWidgetTree& Tree, const FName Name)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetAutoWrapText(false);
		Result->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
		Result->SetClipping(EWidgetClipping::ClipToBounds);
		Result->SetJustification(ETextJustify::Center);
		return Result;
	}

	void AddFill(UHorizontalBox& Parent, UWidget* Child, const float Value)
	{
		UHorizontalBoxSlot* Slot = Parent.AddChildToHorizontalBox(Child);
		FSlateChildSize Size;
		Size.SizeRule = ESlateSizeRule::Fill;
		Size.Value = Value;
		Slot->SetSize(Size);
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Center);
	}

	void RefreshTracker(
		UWidgetTree& Tree,
		UHorizontalBox& Steps,
		const FFMCodexUMGAttackTurnTrackerViewModel& Presentation)
	{
		Steps.ClearChildren();
		const FString WidgetPrefix = Steps.GetName();
		for (int32 StepIndex = 0;
			StepIndex < Presentation.Steps.Num(); ++StepIndex)
		{
			const FFMCodexUMGAttackTurnStepViewModel& Step =
				Presentation.Steps[StepIndex];
			USizeBox* Bounds = Tree.ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("%sBounds%d"), *WidgetPrefix, StepIndex)));
			Bounds->SetWidthOverride(24.0f);
			Bounds->SetHeightOverride(24.0f);
			UBorder* Frame = Tree.ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("%sFrame%d"), *WidgetPrefix, StepIndex)));
			FLinearColor StepColor;
			FLinearColor OutlineColor;
			float OutlineWidth = 1.0f;
			switch (Step.State)
			{
			case EFMCodexUMGAttackTurnStepState::Used:
				StepColor = Presentation.PrimarySideColor * 0.46f;
				StepColor.A = 0.92f;
				OutlineColor = Presentation.PrimarySideColor;
				break;
			case EFMCodexUMGAttackTurnStepState::Current:
				StepColor = Presentation.PrimarySideColor;
				OutlineColor = FLinearColor(0.95f, 0.97f, 1.0f, 0.92f);
				OutlineWidth = 1.5f;
				break;
			case EFMCodexUMGAttackTurnStepState::Remaining:
			default:
				StepColor = FLinearColor(0.09f, 0.12f, 0.15f, 0.78f);
				OutlineColor = FLinearColor(0.68f, 0.73f, 0.78f, 0.48f);
				break;
			}
			Frame->SetBrush(FSlateRoundedBoxBrush(
				StepColor, 12.0f, OutlineColor, OutlineWidth,
				FVector2f(24.0f, 24.0f)));
			Frame->SetBrushColor(FLinearColor::White);
			Frame->SetPadding(FMargin(2.0f));
			UTextBlock* Label = MakeText(Tree, FName(*FString::Printf(
				TEXT("%sLabel%d"), *WidgetPrefix, StepIndex)));
			Label->SetText(FText::AsNumber(Step.AttackIndex));
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*Label, EFMCodexPlayerUITextRole::Body);
			Label->SetRenderOpacity(
				Step.State == EFMCodexUMGAttackTurnStepState::Current
					? 1.0f : Step.State == EFMCodexUMGAttackTurnStepState::Used
						? 0.82f : 0.62f);
			Frame->AddChild(Label);
			Bounds->AddChild(Frame);
			if (UHorizontalBoxSlot* Slot = Steps.AddChildToHorizontalBox(Bounds))
			{
				Slot->SetPadding(FMargin(StepIndex == 0 ? 0.0f : 4.0f,
					0.0f, 0.0f, 0.0f));
				Slot->SetVerticalAlignment(VAlign_Center);
			}
		}
	}
}

UFMCodexMatchHeaderWidget::UFMCodexMatchHeaderWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexMatchHeaderWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexMatchHeaderWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexMatchHeaderWidget::RefreshFromPresentation(
	const FFMCodexUMGMatchHeaderViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

const FFMCodexUMGMatchHeaderViewModel&
UFMCodexMatchHeaderWidget::GetPresentation() const
{
	return Presentation;
}

FString UFMCodexMatchHeaderWidget::GetDisplayedScoreLabel() const
{
	return Presentation.ScoreLabel;
}

FString UFMCodexMatchHeaderWidget::GetDisplayedAttackerLabel() const
{
	return Presentation.AttackerStatusLabel;
}

FString UFMCodexMatchHeaderWidget::GetDisplayedActorLabel() const
{
	return Presentation.ActorStatusLabel;
}

void UFMCodexMatchHeaderWidget::BuildWidgetTree()
{
	using namespace FMCodexMatchHeaderWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("BroadcastMatchHeaderBounds"));
	Bounds->SetHeightOverride(80.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchHeaderFrame"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		Style.GetCompactPadding());
	Bounds->AddChild(Frame);
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("BroadcastScoreboardRow"));
	Frame->AddChild(Row);

	AttackerStatusRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("LeftPlayerBroadcastRegion"));
	Style.ApplyBorder(*AttackerStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerAAccent, Style.GetCompactPadding());
	UVerticalBox* Left = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LeftPlayerBroadcastBody"));
	AttackerStatusRegion->AddChild(Left);
	PlayerAIdentityText = MakeText(*WidgetTree, TEXT("LeftPlayerIdentityLabel"));
	Style.ApplyText(*PlayerAIdentityText, EFMCodexPlayerUITextRole::Identity);
	Left->AddChildToVerticalBox(PlayerAIdentityText);
	UHorizontalBox* LeftTrackerRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("LeftAttackTurnTracker"));
	UTextBlock* LeftTrackerHeading = MakeText(
		*WidgetTree, TEXT("LeftAttackTurnHeading"));
	LeftTrackerHeading->SetText(
		FFMCodexPlayerUIPresentationText::AttackTurnHeading());
	Style.ApplyText(*LeftTrackerHeading, EFMCodexPlayerUITextRole::Kicker);
	LeftTrackerRow->AddChildToHorizontalBox(LeftTrackerHeading);
	LeftAttackTurnSteps = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("LeftAttackTurnSteps"));
	LeftTrackerRow->AddChildToHorizontalBox(LeftAttackTurnSteps);
	if (UVerticalBoxSlot* TrackerSlot =
		Left->AddChildToVerticalBox(LeftTrackerRow))
	{
		TrackerSlot->SetHorizontalAlignment(HAlign_Center);
		TrackerSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));
	}
	AddFill(*Row, AttackerStatusRegion, 1.0f);

	UBorder* Center = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("CentralBroadcastMatchFacts"));
	Style.ApplyBorder(*Center, EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetCompactPadding());
	UVerticalBox* CenterBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CentralBroadcastMatchFactsBody"));
	Center->AddChild(CenterBody);
	FinalResultRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchHeaderFinalResultRegion"));
	FinalResultText = MakeText(*WidgetTree, TEXT("MatchHeaderFinalResultLabel"));
	FinalResultRegion->AddChild(FinalResultText);
	FinalResultRegion->SetVisibility(ESlateVisibility::Collapsed);
	CentralScoreText = MakeText(*WidgetTree, TEXT("CentralBroadcastScoreValue"));
	CurrentAttackProgressText = MakeText(
		*WidgetTree, TEXT("CurrentAttackProgressLabel"));
	CurrentMatchPhaseText = MakeText(
		*WidgetTree, TEXT("CurrentMatchPhaseStatusLabel"));
	Style.ApplyText(*CentralScoreText, EFMCodexPlayerUITextRole::Score);
	Style.ApplyText(*CurrentAttackProgressText, EFMCodexPlayerUITextRole::Status);
	Style.ApplyText(*CurrentMatchPhaseText, EFMCodexPlayerUITextRole::Secondary);
	FSlateFontInfo CentralScoreFont = CentralScoreText->GetFont();
	CentralScoreFont.Size = 30;
	CentralScoreText->SetFont(CentralScoreFont);
	FSlateFontInfo ProgressFont = CurrentAttackProgressText->GetFont();
	ProgressFont.Size = 14;
	CurrentAttackProgressText->SetFont(ProgressFont);
	FSlateFontInfo PhaseFont = CurrentMatchPhaseText->GetFont();
	PhaseFont.Size = 10;
	CurrentMatchPhaseText->SetFont(PhaseFont);
	CenterBody->AddChildToVerticalBox(CentralScoreText);
	if (UVerticalBoxSlot* ProgressSlot =
		CenterBody->AddChildToVerticalBox(CurrentAttackProgressText))
	{
		ProgressSlot->SetPadding(FMargin(0.0f, 1.0f, 0.0f, 0.0f));
	}
	if (UVerticalBoxSlot* PhaseSlot =
		CenterBody->AddChildToVerticalBox(CurrentMatchPhaseText))
	{
		PhaseSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
	}
	CenterBody->AddChildToVerticalBox(FinalResultRegion);
	AddFill(*Row, Center, 1.35f);

	ActorStatusRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("RightPlayerBroadcastRegion"));
	Style.ApplyBorder(*ActorStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerBAccent, Style.GetCompactPadding());
	UVerticalBox* Right = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("RightPlayerBroadcastBody"));
	ActorStatusRegion->AddChild(Right);
	PlayerBIdentityText = MakeText(*WidgetTree, TEXT("RightPlayerIdentityLabel"));
	Style.ApplyText(*PlayerBIdentityText, EFMCodexPlayerUITextRole::Identity);
	Right->AddChildToVerticalBox(PlayerBIdentityText);
	UHorizontalBox* RightTrackerRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("RightAttackTurnTracker"));
	UTextBlock* RightTrackerHeading = MakeText(
		*WidgetTree, TEXT("RightAttackTurnHeading"));
	RightTrackerHeading->SetText(
		FFMCodexPlayerUIPresentationText::AttackTurnHeading());
	Style.ApplyText(*RightTrackerHeading, EFMCodexPlayerUITextRole::Kicker);
	RightAttackTurnSteps = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RightAttackTurnSteps"));
	RightTrackerRow->AddChildToHorizontalBox(RightAttackTurnSteps);
	RightTrackerRow->AddChildToHorizontalBox(RightTrackerHeading);
	if (UVerticalBoxSlot* TrackerSlot =
		Right->AddChildToVerticalBox(RightTrackerRow))
	{
		TrackerSlot->SetHorizontalAlignment(HAlign_Center);
		TrackerSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));
	}
	AddFill(*Row, ActorStatusRegion, 1.0f);
}

void UFMCodexMatchHeaderWidget::RefreshVisuals()
{
	using namespace FMCodexMatchHeaderWidget;
	if (CurrentAttackProgressText == nullptr)
	{
		return;
	}
	CentralScoreText->SetText(FText::FromString(Presentation.ScoreLabel));
	CurrentAttackProgressText->SetText(
		Presentation.bHasCurrentAttacker
			&& Presentation.CurrentAttackerAttackIndex > 0
			&& Presentation.CurrentAttackerMaxAttackTurns > 0
				? FFMCodexPlayerUIPresentationText::CurrentAttackProgress(
					Presentation.CurrentAttackerLabel,
					Presentation.CurrentAttackerAttackIndex,
					Presentation.CurrentAttackerMaxAttackTurns)
				: FText::GetEmpty());
	CurrentMatchPhaseText->SetText(Presentation.bMatchEnded
		? FText::FromString(Presentation.MatchResultLabel)
		: Presentation.bTacticalPointRollReady
			? FFMCodexPlayerUIPresentationText::WaitingForTacticalPointRoll()
			: Presentation.CurrentAttackerTacticalPoints > 0
				? FFMCodexPlayerUIPresentationText::TacticalPoints(
					Presentation.CurrentAttackerTacticalPoints)
				: FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					Presentation.ActorStatusLabel));
	FSlateFontInfo PhaseFont = CurrentMatchPhaseText->GetFont();
	PhaseFont.Size = Presentation.CurrentAttackerTacticalPoints > 0 ? 14 : 10;
	CurrentMatchPhaseText->SetFont(PhaseFont);
	if (Presentation.CurrentAttackerTacticalPoints > 0)
	{
		const FLinearColor AttackerColor = Presentation.bCurrentAttackerOnLeft
			? Presentation.LeftAttackTurnTracker.PrimarySideColor
			: Presentation.RightAttackTurnTracker.PrimarySideColor;
		CurrentMatchPhaseText->SetColorAndOpacity(FSlateColor(
			AttackerColor * 0.62f
				+ FLinearColor(0.95f, 0.97f, 1.0f, 1.0f) * 0.38f));
	}
	else
	{
		CurrentMatchPhaseText->SetColorAndOpacity(FSlateColor(
			FLinearColor(0.62f, 0.68f, 0.73f, 1.0f)));
	}
	PlayerAIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.LeftPlayerLabel));
	PlayerBIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.RightPlayerLabel));
	RefreshTracker(*WidgetTree, *LeftAttackTurnSteps,
		Presentation.LeftAttackTurnTracker);
	RefreshTracker(*WidgetTree, *RightAttackTurnSteps,
		Presentation.RightAttackTurnTracker);
	AttackerStatusRegion->SetBrushColor(
		Presentation.LeftAttackTurnTracker.PrimarySideColor);
	ActorStatusRegion->SetBrushColor(
		Presentation.RightAttackTurnTracker.PrimarySideColor);
	AttackerStatusRegion->SetRenderOpacity(
		Presentation.bHasCurrentAttacker && !Presentation.bCurrentAttackerOnLeft
			? 0.62f : 1.0f);
	ActorStatusRegion->SetRenderOpacity(
		Presentation.bHasCurrentAttacker && Presentation.bCurrentAttackerOnLeft
			? 0.62f : 1.0f);
	FinalResultText->SetText(FText::FromString(Presentation.MatchResultLabel));
	FinalResultRegion->SetVisibility(Presentation.bMatchEnded
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
