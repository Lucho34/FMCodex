#include "FMCodexMatchHeaderWidget.h"

#include "FMCodexPlayerUIStyle.h"

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
		const EFMCodexPlayerUIColorRole ColorRole,
		const FMargin& Padding)
	{
		UBorder* Result = Tree.ConstructWidget<UBorder>(
			UBorder::StaticClass(), Name);
		FFMCodexPlayerUIStyle::Get().ApplyBorder(
			*Result, ColorRole, Padding);
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
			Prefix == TEXT("PlayerA")
				? EFMCodexPlayerUIColorRole::PlayerAAccent
				: EFMCodexPlayerUIColorRole::PlayerBAccent,
			FFMCodexPlayerUIStyle::Get().GetCompactPadding());
		UTextBlock* HookText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("CrestPlaceholder"))),
			TEXT("CREST"));
		HookText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*HookText, EFMCodexPlayerUITextRole::Secondary);
		AssetHook->AddChild(HookText);
		Body->AddChildToVerticalBox(AssetHook);
		IdentityText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("IdentityLabel"))));
		IdentityText->SetJustification(ETextJustify::Center);
		FFMCodexPlayerUIStyle::Get().ApplyText(
			*IdentityText, EFMCodexPlayerUITextRole::Identity);
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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	Bounds->SetMinDesiredWidth(Style.GetPanelMinWidth());
	Bounds->SetMaxDesiredWidth(Style.GetPanelMaxWidth());
	Bounds->SetMaxDesiredHeight(230.0f);
	WidgetTree->RootWidget = Bounds;
	UBorder* Frame = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderFrame"),
		EFMCodexPlayerUIColorRole::PanelBackground,
		Style.GetPanelPadding());
	Bounds->AddChild(Frame);
	UVerticalBox* RootBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("MatchHeaderHierarchy"));
	Frame->AddChild(RootBody);

	MatchStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderStatusLabel"), TEXT("READY TO PLAY"));
	MatchStatusText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*MatchStatusText, EFMCodexPlayerUITextRole::Kicker);
	RootBody->AddChildToVerticalBox(MatchStatusText);

	UBorder* ScoreboardRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderScoreboardRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised,
		Style.GetSectionPadding());
	UHorizontalBox* Scoreboard = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("MatchHeaderScoreboardHierarchy"));
	UTextBlock* PlayerARaw = nullptr;
	AddFillChild(*Scoreboard, MakeIdentity(
		*WidgetTree, TEXT("PlayerA"), PlayerARaw));
	PlayerAIdentityText = PlayerARaw;
	UBorder* ScoreRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderCentralScoreRegion"),
		EFMCodexPlayerUIColorRole::NeutralAccent,
		Style.GetPanelPadding());
	UHorizontalBox* ScoreBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("MatchHeaderCentralScoreHierarchy"));
	PlayerAScoreText = MakeText(*WidgetTree, TEXT("PlayerAScoreValue"));
	PlayerAScoreText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*PlayerAScoreText, EFMCodexPlayerUITextRole::Score);
	UTextBlock* Separator = MakeText(
		*WidgetTree, TEXT("MatchHeaderScoreSeparator"), TEXT(" - "));
	Separator->SetJustification(ETextJustify::Center);
	Style.ApplyText(*Separator, EFMCodexPlayerUITextRole::ActionTitle);
	PlayerBScoreText = MakeText(*WidgetTree, TEXT("PlayerBScoreValue"));
	PlayerBScoreText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*PlayerBScoreText, EFMCodexPlayerUITextRole::Score);
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
		EFMCodexPlayerUIColorRole::NeutralAccent,
		Style.GetSectionPadding());
	AttackerStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderAttackerStatusLabel"));
	AttackerStatusText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*AttackerStatusText, EFMCodexPlayerUITextRole::Status);
	AttackerStatusRegion->AddChild(AttackerStatusText);
	AddFillChild(*StatusBody, AttackerStatusRegion);
	ActorStatusRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderActorStatusRegion"),
		EFMCodexPlayerUIColorRole::SystemStatus,
		Style.GetSectionPadding());
	ActorStatusText = MakeText(
		*WidgetTree, TEXT("MatchHeaderActorStatusLabel"));
	ActorStatusText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*ActorStatusText, EFMCodexPlayerUITextRole::Status);
	ActorStatusRegion->AddChild(ActorStatusText);
	AddFillChild(*StatusBody, ActorStatusRegion);
	RootBody->AddChildToVerticalBox(StatusBody);

	FinalResultRegion = MakeRegion(
		*WidgetTree, TEXT("MatchHeaderFinalResultRegion"),
		EFMCodexPlayerUIColorRole::Warning,
		Style.GetPanelPadding());
	FinalResultText = MakeText(
		*WidgetTree, TEXT("MatchHeaderFinalResultLabel"));
	FinalResultText->SetJustification(ETextJustify::Center);
	Style.ApplyText(
		*FinalResultText, EFMCodexPlayerUITextRole::TerminalResult);
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
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	AttackerStatusRegion->SetBrushColor(
		Style.GetPlayerAccentColor(Presentation.AttackerStatusLabel));
	AttackerStatusRegion->SetVisibility(
		bShowActiveStatus && !Presentation.AttackerStatusLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ActorStatusText->SetText(FText::FromString(Presentation.ActorStatusLabel));
	ActorStatusRegion->SetBrushColor(Presentation.bSystemResolution
		? Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)
		: Style.GetPlayerAccentColor(Presentation.ActorStatusLabel));
	ActorStatusRegion->SetVisibility(
		!Presentation.bMatchEnded && !Presentation.ActorStatusLabel.IsEmpty()
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	FinalResultText->SetText(FText::FromString(Presentation.MatchResultLabel));
	FinalResultRegion->SetBrushColor(Presentation.MatchResultLabel.Contains(
		TEXT("Player A")) || Presentation.MatchResultLabel.Contains(TEXT("Player B"))
		? Style.GetPlayerAccentColor(Presentation.MatchResultLabel)
		: Style.GetColor(EFMCodexPlayerUIColorRole::Warning));
	FinalResultRegion->SetVisibility(Presentation.bMatchEnded
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
