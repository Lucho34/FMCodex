#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexBroadcastPanel.h"
#include "FMCodexMatchShellStyle.h"

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
#include "Styling/CoreStyle.h"

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
		FSlateFontInfo HeadingFont = FMCodexMatchShellStyle::Font(13);
		Heading->SetFont(HeadingFont);
		Heading->SetRenderOpacity(0.88f);
		UHorizontalBoxSlot* HeadingSlot = Body->AddChildToHorizontalBox(Heading);
		HeadingSlot->SetPadding(FMargin(10.0f, 3.0f, 4.0f, 3.0f));
		HeadingSlot->SetVerticalAlignment(VAlign_Center);

		OutValueText = MakeText(Tree,
			FName(*(SidePrefix + TEXT("TacticalPointChipValue"))));
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*OutValueText, EFMCodexPlayerUITextRole::Status);
		// Match the other numeric facts without the taller CJK line metrics.
		FSlateFontInfo ValueFont = FCoreStyle::GetDefaultFontStyle("Bold", 28);
		OutValueText->SetFont(ValueFont);
		OutValueText->SetShadowOffset(FVector2D::ZeroVector);
		UFMCodexBroadcastPanel* ValueBay = Tree.ConstructWidget<UFMCodexBroadcastPanel>(
			UFMCodexBroadcastPanel::StaticClass(), FName(*(SidePrefix + TEXT("TacticalPointValueBay"))));
		ValueBay->Surface = EFMCodexBroadcastSurface::TacticalResourceValue;
		ValueBay->SetPadding(FMargin(8.0f, 2.0f));
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
			Bounds->SetWidthOverride(36.0f);
			Bounds->SetHeightOverride(36.0f);
			UBorder* Frame = Tree.ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("%sFrame%d"), *WidgetPrefix, StepIndex)));
			FLinearColor StepColor;
			FLinearColor OutlineColor;
			float OutlineWidth = 1.0f;
			switch (Step.State)
			{
			case EFMCodexUMGAttackTurnStepState::Used:
				StepColor = FMCodexMatchShellStyle::DisplayAccent(FMCodexMatchShellStyle::HeaderAccent(Presentation.PrimarySideColor)) * FLinearColor(.24f,.24f,.24f,1);
				StepColor.A = 0.96f;
				OutlineColor = FLinearColor(0.96f, 0.98f, 1.0f, 0.68f);
				break;
			case EFMCodexUMGAttackTurnStepState::Current:
				StepColor = FMCodexMatchShellStyle::CurrentTurnFill(Presentation.PrimarySideColor);
				OutlineColor = FLinearColor(0.88f, 0.96f, 1.0f, 1.0f);
				OutlineWidth = 1.8f;
				break;
			case EFMCodexUMGAttackTurnStepState::Remaining:
			default:
				StepColor = FMCodexMatchShellStyle::Color(13,38,62,64);
				OutlineColor = FMCodexMatchShellStyle::Color(78,111,140);
				OutlineWidth = 1.2f;
				break;
			}
			Frame->SetBrush(FSlateRoundedBoxBrush(
				StepColor, 18.0f, OutlineColor, OutlineWidth,
				FVector2f(36.0f, 36.0f)));
			Frame->SetBrushColor(FLinearColor::White);
			Frame->SetPadding(FMargin(0.0f));
			UTextBlock* Label = MakeText(Tree, FName(*FString::Printf(
				TEXT("%sLabel%d"), *WidgetPrefix, StepIndex)));
			Label->SetText(FText::AsNumber(Step.AttackIndex));
			FFMCodexPlayerUIStyle::Get().ApplyText(
				*Label, EFMCodexPlayerUITextRole::Body);
			Label->SetRenderOpacity(Step.State == EFMCodexUMGAttackTurnStepState::Remaining ? .95f : 1.f);
			Label->SetColorAndOpacity(Step.State == EFMCodexUMGAttackTurnStepState::Current
				? FMCodexMatchShellStyle::Text() : FMCodexMatchShellStyle::Color(163,180,195));
			// Use the numeric face's own metrics; CJK line height pushes digits down.
			FSlateFontInfo StepFont = FCoreStyle::GetDefaultFontStyle("Bold", 19);
			Label->SetFont(StepFont);
			Frame->SetVerticalAlignment(VAlign_Center);
			Frame->AddChild(Label);
			Bounds->AddChild(Frame);
			if (UHorizontalBoxSlot* Slot = Steps.AddChildToHorizontalBox(Bounds))
			{
				Slot->SetPadding(FMargin(StepIndex == 0 ? 0.0f : 9.0f,
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
	AttackerStatusRegion->SetPadding(FMargin(18.0f, 4.0f, 52.0f, 4.0f));
	AttackerStatusRegion->SetVerticalAlignment(VAlign_Center);
	UVerticalBox* Left = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("LeftPlayerBroadcastBody"));
	AttackerStatusRegion->AddChild(Left);
	PlayerAIdentityText = MakeText(*WidgetTree, TEXT("LeftPlayerIdentityLabel"));
	Style.ApplyText(*PlayerAIdentityText, EFMCodexPlayerUITextRole::Identity);
	FSlateFontInfo IdentityFont = FMCodexMatchShellStyle::Font(24, true);
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
		ChipSlot->SetPadding(FMargin(22.0f, 0.0f, 0.0f, 0.0f));
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
	FSlateFontInfo LeftTrackerFont = FMCodexMatchShellStyle::Font(13);
	LeftTrackerHeading->SetFont(LeftTrackerFont);
	LeftTrackerRow->AddChildToHorizontalBox(LeftTrackerHeading)->SetVerticalAlignment(VAlign_Center);
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
		TrackerSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}
	AddFill(*Row, AttackerStatusRegion, 1.0f);

	UFMCodexBroadcastPanel* Center = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("CentralBroadcastMatchFacts"));
	Center->Surface = EFMCodexBroadcastSurface::Score;
	Style.ApplyBorder(*Center, EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetCompactPadding());
	Center->SetPadding(FMargin(12.0f, 0.0f));
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
	FSlateFontInfo CentralScoreFont = FCoreStyle::GetDefaultFontStyle("Bold", 44);
	CentralScoreText->SetShadowOffset(FVector2D::ZeroVector);
	CentralScoreText->SetFont(CentralScoreFont);
	FSlateFontInfo ProgressFont = FMCodexMatchShellStyle::Font(14, true);
	CurrentAttackProgressText->SetFont(ProgressFont);
	FSlateFontInfo PhaseFont = CurrentMatchPhaseText->GetFont();
	PhaseFont.Size = 10;
	CurrentMatchPhaseText->SetFont(PhaseFont);
	CenterBody->AddChildToVerticalBox(CentralScoreText);
	UFMCodexBroadcastPanel* ProgressPlate = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("AttackProgressBackplate"));
	ProgressPlate->Surface = EFMCodexBroadcastSurface::Progress;
	ProgressPlate->SetPadding(FMargin(10.0f, 0.0f));
	ProgressPlate->AddChild(CurrentAttackProgressText);
	CenterBody->AddChildToVerticalBox(ProgressPlate)->SetHorizontalAlignment(HAlign_Center);
	// Keep the canonical phase copy here; the shared screen places it in the pitch HUD.
	CenterBody->AddChildToVerticalBox(CurrentMatchPhaseText);
	CurrentMatchPhaseText->SetVisibility(ESlateVisibility::Collapsed);
	CenterBody->AddChildToVerticalBox(FinalResultRegion);
	AddFill(*Row, Center, 1.20f);
	CastChecked<UHorizontalBoxSlot>(Center->Slot)->SetPadding(FMargin(22,0));

	ActorStatusRegion = WidgetTree->ConstructWidget<UFMCodexBroadcastPanel>(
		UFMCodexBroadcastPanel::StaticClass(), TEXT("RightPlayerBroadcastRegion"));
	CastChecked<UFMCodexBroadcastPanel>(ActorStatusRegion)->Surface = EFMCodexBroadcastSurface::HeaderRight;
	Style.ApplyBorder(*ActorStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerBAccent, Style.GetCompactPadding());
	ActorStatusRegion->SetPadding(FMargin(52.0f, 4.0f, 18.0f, 4.0f));
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
		ChipSlot->SetPadding(FMargin(22.0f, 0.0f, 0.0f, 0.0f));
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
	FSlateFontInfo RightTrackerFont = FMCodexMatchShellStyle::Font(13);
	RightTrackerHeading->SetFont(RightTrackerFont);
	RightAttackTurnSteps = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("RightAttackTurnSteps"));
	RightTrackerRow->AddChildToHorizontalBox(RightTrackerHeading)->SetVerticalAlignment(VAlign_Center);
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
		TrackerSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
	}
	AddFill(*Row, ActorStatusRegion, 1.0f);
}

void UFMCodexMatchHeaderWidget::SetPlayerAccentColors(const FFMCodexUMGSidePrimaryColors& Colors)
{
	PlayerAccentOverride=Colors;
	RefreshVisuals();
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
	// Consume the existing projected palette by default; optional settings supply A/B once.
	FFMCodexUMGAttackTurnTrackerViewModel LeftTracker=Presentation.LeftAttackTurnTracker;
	FFMCodexUMGAttackTurnTrackerViewModel RightTracker=Presentation.RightAttackTurnTracker;
	if(PlayerAccentOverride.IsSet())
	{
		const bool LeftIsA=Presentation.LeftPlayerSide==EInitialTurnOrderPlayer::PlayerA;
		LeftTracker.PrimarySideColor=LeftIsA?PlayerAccentOverride->PlayerAPrimaryColor:PlayerAccentOverride->PlayerBPrimaryColor;
		RightTracker.PrimarySideColor=LeftIsA?PlayerAccentOverride->PlayerBPrimaryColor:PlayerAccentOverride->PlayerAPrimaryColor;
	}
	RefreshTracker(*WidgetTree, *LeftAttackTurnSteps,
		LeftTracker);
	RefreshTracker(*WidgetTree, *RightAttackTurnSteps,
		RightTracker);
	RefreshTacticalPointChip(
		*LeftTacticalPointChip, *LeftTacticalPointValueText,
		Presentation.bShowLeftTacticalPointChip,
		Presentation.LeftTacticalPoints,
		LeftTracker.PrimarySideColor);
	RefreshTacticalPointChip(
		*RightTacticalPointChip, *RightTacticalPointValueText,
		Presentation.bShowRightTacticalPointChip,
		Presentation.RightTacticalPoints,
		RightTracker.PrimarySideColor);
	AttackerStatusRegion->SetBrushColor(
		LeftTracker.PrimarySideColor);
	ActorStatusRegion->SetBrushColor(
		RightTracker.PrimarySideColor);
	AttackerStatusRegion->SetRenderOpacity(1.0f);
	ActorStatusRegion->SetRenderOpacity(1.0f);
	FinalResultText->SetText(
		FFMCodexPlayerUIPresentationText::MatchScreenLabel(
			Presentation.MatchResultLabel));
	FinalResultRegion->SetVisibility(ESlateVisibility::Collapsed);
}
