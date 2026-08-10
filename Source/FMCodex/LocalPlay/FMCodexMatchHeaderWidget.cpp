#include "FMCodexMatchHeaderWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

namespace FMCodexMatchHeaderWidget
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

	void SetTextSize(UTextBlock& Text, const int32 Size)
	{
		FSlateFontInfo Font = Text.GetFont();
		Font.Size = Size;
		Text.SetFont(Font);
	}

	void AddFillChild(UHorizontalBox& Parent, UWidget* Child)
	{
		UHorizontalBoxSlot* Slot = Parent.AddChildToHorizontalBox(Child);
		Slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Slot->SetHorizontalAlignment(HAlign_Fill);
		Slot->SetVerticalAlignment(VAlign_Center);
	}

	UBorder* MakeRegion(
		UWidgetTree& Tree,
		const FName Name,
		const FLinearColor& Color,
		const FMargin Padding = FMargin(8.0f))
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		Result->SetPadding(Padding);
		Result->SetBrushColor(Color);
		return Result;
	}

	UVerticalBox* MakeIdentity(
		UWidgetTree& Tree,
		const FName Prefix,
		UTextBlock*& IdentityText)
	{
		UVerticalBox* Body = Tree.ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("IdentityHierarchy"))));
		UBorder* AssetHook = MakeRegion(
			Tree, FName(*(Prefix.ToString() + TEXT("CrestAssetHook"))),
			FLinearColor(0.08f, 0.12f, 0.16f, 1.0f), FMargin(3.0f));
		UTextBlock* HookText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("CrestPlaceholder"))),
			TEXT("TEAM"));
		HookText->SetJustification(ETextJustify::Center);
		SetTextSize(*HookText, 10);
		AssetHook->AddChild(HookText);
		Body->AddChildToVerticalBox(AssetHook);
		IdentityText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("IdentityLabel"))));
		IdentityText->SetJustification(ETextJustify::Center);
		SetTextSize(*IdentityText, 16);
		Body->AddChildToVerticalBox(IdentityText);
		return Body;
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
		USizeBox::StaticClass(), TEXT("MatchHeaderBounds"));
	Bounds->SetMinDesiredWidth(840.0f);
	Bounds->SetMaxDesiredWidth(1120.0f);
	Bounds->SetMaxDesiredHeight(230.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderFrame"),
		FLinearColor(0.02f, 0.045f, 0.07f, 0.98f), FMargin(12.0f));
	Bounds->AddChild(Frame);
	UVerticalBox* RootBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("MatchHeaderHierarchy"));
	Frame->AddChild(RootBody);

	MatchStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderStatusLabel"), TEXT("READY TO PLAY"));
	MatchStatusText->SetJustification(ETextJustify::Center);
	SetTextSize(*MatchStatusText, 13);
	RootBody->AddChildToVerticalBox(MatchStatusText);

	UBorder* ScoreboardRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderScoreboardRegion"),
		FLinearColor(0.045f, 0.105f, 0.15f, 1.0f));
	UHorizontalBox* Scoreboard = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("MatchHeaderScoreboardHierarchy"));
	UTextBlock* PlayerARaw = nullptr;
	AddFillChild(*Scoreboard, MakeIdentity(
		*WidgetTree, TEXT("PlayerA"), PlayerARaw));
	PlayerAIdentityText = PlayerARaw;
	UBorder* ScoreRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderCentralScoreRegion"),
		FLinearColor(0.07f, 0.17f, 0.23f, 1.0f), FMargin(14.0f));
	UHorizontalBox* ScoreBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("MatchHeaderCentralScoreHierarchy"));
	PlayerAScoreText = MakeText(*WidgetTree, TEXT("PlayerAScoreValue"));
	PlayerAScoreText->SetJustification(ETextJustify::Center);
	SetTextSize(*PlayerAScoreText, 34);
	UTextBlock* Separator = MakeText(
		*WidgetTree, TEXT("MatchHeaderScoreSeparator"), TEXT(" - "));
	Separator->SetJustification(ETextJustify::Center);
	SetTextSize(*Separator, 24);
	PlayerBScoreText = MakeText(*WidgetTree, TEXT("PlayerBScoreValue"));
	PlayerBScoreText->SetJustification(ETextJustify::Center);
	SetTextSize(*PlayerBScoreText, 34);
	ScoreBody->AddChildToHorizontalBox(PlayerAScoreText);
	ScoreBody->AddChildToHorizontalBox(Separator);
	ScoreBody->AddChildToHorizontalBox(PlayerBScoreText);
	ScoreRegion->AddChild(ScoreBody);
	AddFillChild(*Scoreboard, ScoreRegion);
	UTextBlock* PlayerBRaw = nullptr;
	AddFillChild(*Scoreboard, MakeIdentity(
		*WidgetTree, TEXT("PlayerB"), PlayerBRaw));
	PlayerBIdentityText = PlayerBRaw;
	ScoreboardRegion->AddChild(Scoreboard);
	RootBody->AddChildToVerticalBox(ScoreboardRegion);

	UHorizontalBox* StatusBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("MatchHeaderStatusHierarchy"));
	AttackerStatusRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderAttackerStatusRegion"),
		FLinearColor(0.11f, 0.20f, 0.12f, 1.0f));
	AttackerStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderAttackerStatusLabel"));
	AttackerStatusText->SetJustification(ETextJustify::Center);
	SetTextSize(*AttackerStatusText, 16);
	AttackerStatusRegion->AddChild(AttackerStatusText);
	AddFillChild(*StatusBody, AttackerStatusRegion);
	ActorStatusRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderActorStatusRegion"),
		FLinearColor(0.10f, 0.13f, 0.22f, 1.0f));
	ActorStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderActorStatusLabel"));
	ActorStatusText->SetJustification(ETextJustify::Center);
	SetTextSize(*ActorStatusText, 17);
	ActorStatusRegion->AddChild(ActorStatusText);
	AddFillChild(*StatusBody, ActorStatusRegion);
	RootBody->AddChildToVerticalBox(StatusBody);

	FinalResultRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderFinalResultRegion"),
		FLinearColor(0.20f, 0.15f, 0.055f, 1.0f), FMargin(12.0f));
	FinalResultText = MakeText(
		*WidgetTree, TEXT("MatchHeaderFinalResultLabel"));
	FinalResultText->SetJustification(ETextJustify::Center);
	SetTextSize(*FinalResultText, 22);
	FinalResultRegion->AddChild(FinalResultText);
	RootBody->AddChildToVerticalBox(FinalResultRegion);
}

void UFMCodexMatchHeaderWidget::RefreshVisuals()
{
	if (MatchStatusText == nullptr)
	{
		return;
	}
	MatchStatusText->SetText(FText::FromString(
		Presentation.MatchStatusLabel.IsEmpty()
			? TEXT("LOCAL MATCH") : Presentation.MatchStatusLabel));
	PlayerAIdentityText->SetText(FText::FromString(Presentation.PlayerALabel));
	PlayerBIdentityText->SetText(FText::FromString(Presentation.PlayerBLabel));
	PlayerAScoreText->SetText(FText::FromString(Presentation.PlayerAScoreLabel));
	PlayerBScoreText->SetText(FText::FromString(Presentation.PlayerBScoreLabel));

	const bool bShowActiveStatus = Presentation.bMatchActive
		&& !Presentation.bMatchEnded;
	AttackerStatusText->SetText(FText::FromString(
		Presentation.AttackerStatusLabel));
	AttackerStatusRegion->SetVisibility(
		bShowActiveStatus && !Presentation.AttackerStatusLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ActorStatusText->SetText(FText::FromString(Presentation.ActorStatusLabel));
	ActorStatusRegion->SetVisibility(
		!Presentation.bMatchEnded && !Presentation.ActorStatusLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	FinalResultText->SetText(FText::FromString(Presentation.MatchResultLabel));
	FinalResultRegion->SetVisibility(Presentation.bMatchEnded
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
