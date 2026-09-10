#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexBroadcastPanel.h"

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
		Slot->SetVerticalAlignment(VAlign_Fill);
	}

	UBorder* MakeTacticalPointChip(
		UWidgetTree& Tree,
		const FString& SidePrefix,
		TObjectPtr<UTextBlock>& OutValueText)
	{
		UFMCodexBroadcastPanel* Chip = Tree.ConstructWidget<UFMCodexBroadcastPanel>(
			UFMCodexBroadcastPanel::StaticClass(), FName(*(SidePrefix + TEXT("TacticalPointChip"))));
		Chip->Surface = EFMCodexBroadcastSurface::TacticalResource;
		Chip->SetPadding(FMargin(0));
		Chip->SetVisibility(ESlateVisibility::Collapsed);
		UHorizontalBox* Body = Tree.ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*(SidePrefix + TEXT("TacticalPointChipBody"))));
		Chip->AddChild(Body);

		UTextBlock* Heading = MakeText(Tree,
			FName(*(SidePrefix + TEXT("TacticalPointChipLabel"))));
		Heading->SetText(
			FFMCodexPlayerUIPresentationText::TacticalPointsHeading());
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*Heading, EFMCodexPlayerUITextRole::Kicker);
		FSlateFontInfo HeadingFont = Heading->GetFont();
		HeadingFont.Size = 11;
		Heading->SetFont(HeadingFont);
		Heading->SetRenderOpacity(0.88f);
		UHorizontalBoxSlot* HeadingSlot = Body->AddChildToHorizontalBox(Heading);
		HeadingSlot->SetPadding(FMargin(11.0f, 3.0f, 9.0f, 3.0f));
		HeadingSlot->SetVerticalAlignment(VAlign_Center);

		OutValueText = MakeText(Tree,
			FName(*(SidePrefix + TEXT("TacticalPointChipValue"))));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*OutValueText, EFMCodexPlayerUITextRole::Status);
		FSlateFontInfo ValueFont = OutValueText->GetFont();
		ValueFont.Size = 18;
		OutValueText->SetFont(ValueFont);
		OutValueText->SetShadowOffset(FVector2D(0,1));
		OutValueText->SetShadowColorAndOpacity(FLinearColor(0,0,0,0.55f));
		UFMCodexBroadcastPanel* ValueBay = Tree.ConstructWidget<UFMCodexBroadcastPanel>(
			UFMCodexBroadcastPanel::StaticClass(), FName(*(SidePrefix + TEXT("TacticalPointValueBay"))));
		ValueBay->Surface = EFMCodexBroadcastSurface::TacticalResourceValue;
		ValueBay->SetPadding(FMargin(10.0f, 2.0f));
		ValueBay->SetVerticalAlignment(VAlign_Center);
		ValueBay->AddChild(OutValueText);
		Body->AddChildToHorizontalBox(ValueBay)->SetVerticalAlignment(VAlign_Fill);
		return Chip;
	}

	void RefreshTacticalPointChip(
		UBorder& Chip,
		UTextBlock& ValueText,
		const bool bVisible,
		const int32 Value,
		const FLinearColor& PrimarySideColor)
	{
		// The label and number retain natural desired widths in every locale.
		// Only material tint changes; visibility and value stay presentation-owned.
		Chip.SetBrushColor(PrimarySideColor);
		if (UFMCodexBroadcastPanel* ValueBay = Cast<UFMCodexBroadcastPanel>(ValueText.GetParent()))
			ValueBay->SetBrushColor(PrimarySideColor);
		ValueText.SetText(bVisible ? FText::AsNumber(Value) : FText::GetEmpty());
		ValueText.SetColorAndOpacity(FSlateColor(
			FLinearColor(0.97f, 0.98f, 1.0f, 1.0f)));
		Chip.SetVisibility(bVisible
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
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
			Bounds->SetWidthOverride(32.0f);
			Bounds->SetHeightOverride(32.0f);
			UBorder* Frame = Tree.ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("%sFrame%d"), *WidgetPrefix, StepIndex)));
			FLinearColor StepColor;
			FLinearColor OutlineColor;
			float OutlineWidth = 1.0f;
			switch (Step.State)
			{
			case EFMCodexUMGAttackTurnStepState::Used:
				StepColor = Presentation.PrimarySideColor * 0.82f
					+ FLinearColor(0.95f, 0.97f, 1.0f, 1.0f) * 0.18f;
				StepColor.A = 0.96f;
				OutlineColor = FLinearColor(0.96f, 0.98f, 1.0f, 0.68f);
				break;
			case EFMCodexUMGAttackTurnStepState::Current:
				StepColor = FLinearColor(0.14f, 0.29f, 0.47f, 1.0f);
				OutlineColor = Presentation.PrimarySideColor * 0.52f
					+ FLinearColor(0.97f, 0.98f, 1.0f, 1.0f) * 0.48f;
				OutlineColor = FLinearColor(0.88f, 0.96f, 1.0f, 1.0f);
				OutlineWidth = 2.0f;
				break;
			case EFMCodexUMGAttackTurnStepState::Remaining:
			default:
				StepColor = FLinearColor(0.035f, 0.052f, 0.066f, 0.14f);
				OutlineColor = FLinearColor(0.72f, 0.77f, 0.82f, 0.62f);
				OutlineWidth = 1.0f;
				break;
			}
			Frame->SetBrush(FSlateRoundedBoxBrush(
				StepColor, 16.0f, OutlineColor, OutlineWidth,
				FVector2f(32.0f, 32.0f)));
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
						? 0.94f : 0.78f);
			FSlateFontInfo StepFont = Label->GetFont();
			StepFont.Size = 17;
			Label->SetFont(StepFont);
			Frame->SetVerticalAlignment(VAlign_Center);
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

FText UFMCodexMatchHeaderWidget::GetDisplayedPhaseText() const
{
	return CurrentMatchPhaseText ? CurrentMatchPhaseText->GetText() : FText::GetEmpty();
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
	Bounds->SetHeightOverride(104.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchHeaderFrame"));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		Style.GetCompactPadding());
	Frame->SetBrushColor(FLinearColor::Transparent);
	Frame->SetPadding(FMargin(0));
	Bounds->AddChild(Frame);
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("BroadcastScoreboardRow"));
	Frame->AddChild(Row);

	AttackerStatusRegion = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("LeftPlayerBroadcastRegion"));
	CastChecked<UFMCodexBroadcastPanel>(AttackerStatusRegion)->Surface = EFMCodexBroadcastSurface::HeaderLeft;
	Style.ApplyBorder(*AttackerStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerAAccent, Style.GetCompactPadding());
	AttackerStatusRegion->SetPadding(FMargin(22.0f, 12.0f, 70.0f, 12.0f));
	AttackerStatusRegion->SetVerticalAlignment(VAlign_Center);
	UVerticalBox* Left = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LeftPlayerBroadcastBody"));
	AttackerStatusRegion->AddChild(Left);
	PlayerAIdentityText = MakeText(*WidgetTree, TEXT("LeftPlayerIdentityLabel"));
	Style.ApplyText(*PlayerAIdentityText, EFMCodexPlayerUITextRole::Identity);
	FSlateFontInfo IdentityFont = PlayerAIdentityText->GetFont();
	IdentityFont.Size = 24;
	PlayerAIdentityText->SetFont(IdentityFont);
	UHorizontalBox* LeftIdentityRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("LeftPlayerIdentityGroup"));
	LeftIdentityRow->AddChildToHorizontalBox(PlayerAIdentityText);
	LeftTacticalPointChip = MakeTacticalPointChip(
		*WidgetTree, TEXT("Left"), LeftTacticalPointValueText);
	if (UHorizontalBoxSlot* ChipSlot =
		LeftIdentityRow->AddChildToHorizontalBox(LeftTacticalPointChip))
	{
		ChipSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
		ChipSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* IdentitySlot =
		Left->AddChildToVerticalBox(LeftIdentityRow))
	{
		IdentitySlot->SetHorizontalAlignment(HAlign_Center);
	}
	UHorizontalBox* LeftTrackerRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("LeftAttackTurnTracker"));
	UTextBlock* LeftTrackerHeading = MakeText(
		*WidgetTree, TEXT("LeftAttackTurnHeading"));
	LeftTrackerHeading->SetText(
		FFMCodexPlayerUIPresentationText::AttackTurnHeading());
	Style.ApplyText(*LeftTrackerHeading, EFMCodexPlayerUITextRole::Body);
	FSlateFontInfo LeftTrackerFont = LeftTrackerHeading->GetFont();
	LeftTrackerFont.Size = 13;
	LeftTrackerHeading->SetFont(LeftTrackerFont);
	LeftTrackerRow->AddChildToHorizontalBox(LeftTrackerHeading);
	LeftAttackTurnSteps = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("LeftAttackTurnSteps"));
	if (UHorizontalBoxSlot* StepsSlot =
		LeftTrackerRow->AddChildToHorizontalBox(LeftAttackTurnSteps))
	{
		StepsSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
		StepsSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* TrackerSlot =
		Left->AddChildToVerticalBox(LeftTrackerRow))
	{
		TrackerSlot->SetHorizontalAlignment(HAlign_Center);
		TrackerSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 0.0f));
	}
	AddFill(*Row, AttackerStatusRegion, 1.0f);

	UFMCodexBroadcastPanel* Center = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("CentralBroadcastMatchFacts"));
	Center->Surface = EFMCodexBroadcastSurface::Score;
	Style.ApplyBorder(*Center, EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetCompactPadding());
	Center->SetPadding(FMargin(12.0f, 2.0f));
	Center->SetVerticalAlignment(VAlign_Center);
	UVerticalBox* CenterBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("CentralBroadcastMatchFactsBody"));
	Center->AddChild(CenterBody);
	FinalResultRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchHeaderFinalResultRegion"));
	// This legacy end-only border used the default opaque white brush and
	// overflowed the fixed-height header. FullTime now owns the result.
	FinalResultRegion->SetBrushColor(FLinearColor::Transparent);
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
	CentralScoreFont.Size = 46;
	CentralScoreText->SetShadowOffset(FVector2D(0,2));
	CentralScoreText->SetShadowColorAndOpacity(FLinearColor(0,0,0,0.65f));
	CentralScoreText->SetFont(CentralScoreFont);
	FSlateFontInfo ProgressFont = CurrentAttackProgressText->GetFont();
	ProgressFont.Size = 16;
	CurrentAttackProgressText->SetFont(ProgressFont);
	FSlateFontInfo PhaseFont = CurrentMatchPhaseText->GetFont();
	PhaseFont.Size = 10;
	CurrentMatchPhaseText->SetFont(PhaseFont);
	CenterBody->AddChildToVerticalBox(CentralScoreText);
	UFMCodexBroadcastPanel* ProgressPlate = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("AttackProgressBackplate"));
	ProgressPlate->Surface = EFMCodexBroadcastSurface::Progress;
	ProgressPlate->SetPadding(FMargin(30.0f, 3.0f));
	ProgressPlate->AddChild(CurrentAttackProgressText);
	CenterBody->AddChildToVerticalBox(ProgressPlate)->SetHorizontalAlignment(HAlign_Center);
	// Keep the canonical phase copy here; the shared screen places it in the pitch HUD.
	CenterBody->AddChildToVerticalBox(CurrentMatchPhaseText);
	CurrentMatchPhaseText->SetVisibility(ESlateVisibility::Collapsed);
	CenterBody->AddChildToVerticalBox(FinalResultRegion);
	AddFill(*Row, Center, 1.20f);

	ActorStatusRegion = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("RightPlayerBroadcastRegion"));
	CastChecked<UFMCodexBroadcastPanel>(ActorStatusRegion)->Surface = EFMCodexBroadcastSurface::HeaderRight;
	Style.ApplyBorder(*ActorStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerBAccent, Style.GetCompactPadding());
	ActorStatusRegion->SetPadding(FMargin(70.0f, 12.0f, 22.0f, 12.0f));
	ActorStatusRegion->SetVerticalAlignment(VAlign_Center);
	UVerticalBox* Right = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("RightPlayerBroadcastBody"));
	ActorStatusRegion->AddChild(Right);
	PlayerBIdentityText = MakeText(*WidgetTree, TEXT("RightPlayerIdentityLabel"));
	Style.ApplyText(*PlayerBIdentityText, EFMCodexPlayerUITextRole::Identity);
	PlayerBIdentityText->SetFont(IdentityFont);
	UHorizontalBox* RightIdentityRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("RightPlayerIdentityGroup"));
	RightIdentityRow->AddChildToHorizontalBox(PlayerBIdentityText);
	RightTacticalPointChip = MakeTacticalPointChip(
		*WidgetTree, TEXT("Right"), RightTacticalPointValueText);
	if (UHorizontalBoxSlot* ChipSlot =
		RightIdentityRow->AddChildToHorizontalBox(RightTacticalPointChip))
	{
		ChipSlot->SetPadding(FMargin(8.0f, 0.0f, 0.0f, 0.0f));
		ChipSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* IdentitySlot =
		Right->AddChildToVerticalBox(RightIdentityRow))
	{
		IdentitySlot->SetHorizontalAlignment(HAlign_Center);
	}
	UHorizontalBox* RightTrackerRow =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("RightAttackTurnTracker"));
	UTextBlock* RightTrackerHeading = MakeText(
		*WidgetTree, TEXT("RightAttackTurnHeading"));
	RightTrackerHeading->SetText(
		FFMCodexPlayerUIPresentationText::AttackTurnHeading());
	Style.ApplyText(*RightTrackerHeading, EFMCodexPlayerUITextRole::Body);
	FSlateFontInfo RightTrackerFont = RightTrackerHeading->GetFont();
	RightTrackerFont.Size = 13;
	RightTrackerHeading->SetFont(RightTrackerFont);
	RightAttackTurnSteps = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RightAttackTurnSteps"));
	RightTrackerRow->AddChildToHorizontalBox(RightTrackerHeading);
	if (UHorizontalBoxSlot* StepsSlot =
		RightTrackerRow->AddChildToHorizontalBox(RightAttackTurnSteps))
	{
		StepsSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 0.0f));
		StepsSlot->SetVerticalAlignment(VAlign_Center);
	}
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
		? FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			Presentation.MatchResultLabel)
		: Presentation.bTacticalPointRollReady
			? FFMCodexPlayerUIPresentationText::WaitingForTacticalPointRoll()
			: Presentation.bAttackActive
				? FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					Presentation.CurrentPhaseLabel)
				: FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					Presentation.ActorStatusLabel));
	FSlateFontInfo PhaseFont = CurrentMatchPhaseText->GetFont();
	PhaseFont.Size = 10;
	CurrentMatchPhaseText->SetFont(PhaseFont);
	CurrentMatchPhaseText->SetColorAndOpacity(FSlateColor(
		FLinearColor(0.62f, 0.68f, 0.73f, 1.0f)));
	PlayerAIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.LeftPlayerLabel));
	PlayerBIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.RightPlayerLabel));
	RefreshTracker(*WidgetTree, *LeftAttackTurnSteps,
		Presentation.LeftAttackTurnTracker);
	RefreshTracker(*WidgetTree, *RightAttackTurnSteps,
		Presentation.RightAttackTurnTracker);
	RefreshTacticalPointChip(
		*LeftTacticalPointChip, *LeftTacticalPointValueText,
		Presentation.bShowLeftTacticalPointChip,
		Presentation.LeftTacticalPoints,
		Presentation.LeftAttackTurnTracker.PrimarySideColor);
	RefreshTacticalPointChip(
		*RightTacticalPointChip, *RightTacticalPointValueText,
		Presentation.bShowRightTacticalPointChip,
		Presentation.RightTacticalPoints,
		Presentation.RightAttackTurnTracker.PrimarySideColor);
	AttackerStatusRegion->SetBrushColor(
		Presentation.LeftAttackTurnTracker.PrimarySideColor);
	ActorStatusRegion->SetBrushColor(
		Presentation.RightAttackTurnTracker.PrimarySideColor);
	AttackerStatusRegion->SetRenderOpacity(1.0f);
	ActorStatusRegion->SetRenderOpacity(1.0f);
	FinalResultText->SetText(
		FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			Presentation.MatchResultLabel));
	FinalResultRegion->SetVisibility(ESlateVisibility::Collapsed);
}
