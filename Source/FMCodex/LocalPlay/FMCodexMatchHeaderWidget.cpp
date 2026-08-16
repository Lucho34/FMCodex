#include "FMCodexMatchHeaderWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPlayerUIPresentationText.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

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
	LeftAttackerPointerText = MakeText(
		*WidgetTree, TEXT("LeftCurrentAttackerPointer"));
	LeftAttackerPointerText->SetText(FText::FromString(TEXT("\u25BC")));
	Style.ApplyText(*LeftAttackerPointerText, EFMCodexPlayerUITextRole::Kicker);
	Style.ApplyText(*PlayerAIdentityText, EFMCodexPlayerUITextRole::Identity);
	Left->AddChildToVerticalBox(LeftAttackerPointerText);
	Left->AddChildToVerticalBox(PlayerAIdentityText);
	AddFill(*Row, AttackerStatusRegion, 1.0f);

	UBorder* Center = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("CentralBroadcastMatchFacts"));
	Style.ApplyBorder(*Center, EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetCompactPadding());
	UHorizontalBox* CenterBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("CentralBroadcastMatchFactsBody"));
	Center->AddChild(CenterBody);
	UVerticalBox* StateColumn = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HeaderMatchStateColumn"));
	MatchStatusText = MakeText(*WidgetTree, TEXT("MatchHeaderStatusLabel"));
	FinalResultRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("MatchHeaderFinalResultRegion"));
	FinalResultText = MakeText(*WidgetTree, TEXT("MatchHeaderFinalResultLabel"));
	FinalResultRegion->AddChild(FinalResultText);
	FinalResultRegion->SetVisibility(ESlateVisibility::Collapsed);
	StateColumn->AddChildToVerticalBox(MatchStatusText);
	StateColumn->AddChildToVerticalBox(FinalResultRegion);
	AddFill(*CenterBody, StateColumn, 1.0f);

	USizeBox* ScoreBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("HeaderCentralScoreBounds"));
	ScoreBounds->SetMinDesiredWidth(180.0f);
	CentralScoreText = MakeText(*WidgetTree, TEXT("CentralBroadcastScoreValue"));
	ScoreBounds->AddChild(CentralScoreText);
	CenterBody->AddChildToHorizontalBox(ScoreBounds);

	UVerticalBox* PhaseColumn = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("HeaderPhaseFactsColumn"));
	TurnText = MakeText(*WidgetTree, TEXT("CurrentTurnLabel"));
	TacticalPointsText = MakeText(*WidgetTree, TEXT("CurrentAttackerTacticalPoints"));
	AttackerStatusText = MakeText(*WidgetTree, TEXT("MatchHeaderAttackerStatusLabel"));
	ActorStatusText = MakeText(*WidgetTree, TEXT("MatchHeaderActorStatusLabel"));
	Style.ApplyText(*MatchStatusText, EFMCodexPlayerUITextRole::Kicker);
	Style.ApplyText(*CentralScoreText, EFMCodexPlayerUITextRole::Score);
	Style.ApplyText(*TurnText, EFMCodexPlayerUITextRole::Status);
	Style.ApplyText(*TacticalPointsText, EFMCodexPlayerUITextRole::Secondary);
	FSlateFontInfo MatchStatusFont = MatchStatusText->GetFont();
	MatchStatusFont.Size = 10;
	MatchStatusText->SetFont(MatchStatusFont);
	FSlateFontInfo CentralScoreFont = CentralScoreText->GetFont();
	CentralScoreFont.Size = 32;
	CentralScoreText->SetFont(CentralScoreFont);
	FSlateFontInfo TurnFont = TurnText->GetFont();
	TurnFont.Size = 14;
	TurnText->SetFont(TurnFont);
	FSlateFontInfo TacticalPointsFont = TacticalPointsText->GetFont();
	TacticalPointsFont.Size = 10;
	TacticalPointsText->SetFont(TacticalPointsFont);
	PhaseColumn->AddChildToVerticalBox(TurnText);
	PhaseColumn->AddChildToVerticalBox(TacticalPointsText);
	AddFill(*CenterBody, PhaseColumn, 1.0f);
	AddFill(*Row, Center, 1.35f);

	ActorStatusRegion = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("RightPlayerBroadcastRegion"));
	Style.ApplyBorder(*ActorStatusRegion,
		EFMCodexPlayerUIColorRole::PlayerBAccent, Style.GetCompactPadding());
	UVerticalBox* Right = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("RightPlayerBroadcastBody"));
	ActorStatusRegion->AddChild(Right);
	PlayerBIdentityText = MakeText(*WidgetTree, TEXT("RightPlayerIdentityLabel"));
	RightAttackerPointerText = MakeText(
		*WidgetTree, TEXT("RightCurrentAttackerPointer"));
	RightAttackerPointerText->SetText(FText::FromString(TEXT("\u25BC")));
	Style.ApplyText(*RightAttackerPointerText, EFMCodexPlayerUITextRole::Kicker);
	Style.ApplyText(*PlayerBIdentityText, EFMCodexPlayerUITextRole::Identity);
	Right->AddChildToVerticalBox(RightAttackerPointerText);
	Right->AddChildToVerticalBox(PlayerBIdentityText);
	AddFill(*Row, ActorStatusRegion, 1.0f);
}

void UFMCodexMatchHeaderWidget::RefreshVisuals()
{
	if (MatchStatusText == nullptr)
	{
		return;
	}
	MatchStatusText->SetText(FFMCodexPlayerUIPresentationText::BroadcastStatus(
		Presentation.bMatchEnded, Presentation.bAttackActive,
		Presentation.MatchResultLabel));
	CentralScoreText->SetText(FText::FromString(Presentation.ScoreLabel));
	TurnText->SetText(FFMCodexPlayerUIPresentationText::Turn(
		Presentation.AttackSequence));
	TacticalPointsText->SetText(
		!Presentation.bHasCurrentAttacker
			? FText::GetEmpty()
			: FFMCodexPlayerUIPresentationText::TacticalPoints(
				Presentation.CurrentAttackerTacticalPoints));
	PlayerAIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.LeftPlayerLabel));
	PlayerBIdentityText->SetText(FFMCodexPlayerUIPresentationText::MatchScreenLabel(
		Presentation.RightPlayerLabel));
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	AttackerStatusRegion->SetBrushColor(Style.GetPlayerAccentColor(
		Presentation.LeftPlayerLabel));
	ActorStatusRegion->SetBrushColor(Style.GetPlayerAccentColor(
		Presentation.RightPlayerLabel));
	AttackerStatusRegion->SetRenderOpacity(
		Presentation.bAttackActive && !Presentation.bCurrentAttackerOnLeft
			? 0.62f : 1.0f);
	ActorStatusRegion->SetRenderOpacity(
		Presentation.bAttackActive && Presentation.bCurrentAttackerOnLeft
			? 0.62f : 1.0f);
	LeftAttackerPointerText->SetVisibility(
		Presentation.bAttackActive && Presentation.bCurrentAttackerOnLeft
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	RightAttackerPointerText->SetVisibility(
		Presentation.bAttackActive && !Presentation.bCurrentAttackerOnLeft
			? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	FinalResultText->SetText(FText::FromString(Presentation.MatchResultLabel));
	FinalResultRegion->SetVisibility(Presentation.bMatchEnded
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
