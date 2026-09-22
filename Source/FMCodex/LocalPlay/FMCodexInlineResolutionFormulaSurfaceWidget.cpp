#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexOutcomePresentation.h"
#include "FMCodexRollReelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/RichTextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

namespace FMCodexInlineResolutionFormulaSurfaceWidget
{
	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const FString& InitialText = FString())
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetText(FText::FromString(InitialText));
		Result->SetAutoWrapText(false);
		return Result;
	}

	UBorder* MakeBorder(
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

	UVerticalBox* BuildRow(
		UWidgetTree& Tree, const FName Prefix, const bool bAttack,
		UTextBlock*& OutSideText, UWrapBox*& OutParticipantBody,
		UTextBlock*& OutKnownSubtotalText, UWrapBox*& OutFormulaBody,
		UTextBlock*& OutFinalValueText)
	{
		const auto& Style = FFMCodexPlayerUIStyle::Get();
		auto Named = [&](const TCHAR* Suffix) { return FName(*(Prefix.ToString()+Suffix)); };
		auto* Body = Tree.ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),Named(TEXT("Hierarchy")));
		auto* Header = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),Named(TEXT("IdentityLine")));
		auto* Section = Tree.ConstructWidget<UFMCodexMatchFlowPanel>(UFMCodexMatchFlowPanel::StaticClass(),Named(TEXT("SectionHeader")));
		Section->SetFormulaRole(EFMCodexFormulaPanelRole::Section);
		Section->SetPadding(FMargin(14,6));
		OutSideText = MakeText(Tree,Named(TEXT("Side")));
		Section->AddChild(OutSideText);
		auto* SectionSlot = Header->AddChildToHorizontalBox(Section);
		SectionSlot->SetVerticalAlignment(VAlign_Center);
		SectionSlot->SetPadding(FMargin(0,0,12,0));
		OutParticipantBody = Tree.ConstructWidget<UWrapBox>(UWrapBox::StaticClass(),Named(TEXT("Participants")));
		OutParticipantBody->SetInnerSlotPadding(FVector2D(6,4));
		auto* IdentitySlot = Header->AddChildToHorizontalBox(OutParticipantBody);
		IdentitySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		IdentitySlot->SetVerticalAlignment(VAlign_Center);
		Body->AddChildToVerticalBox(Header);
		Body->AddChildToVerticalBox(Style.MakeFlowSeparator(Tree,Named(TEXT("ModuleRule"))))->SetPadding(FMargin(0,9,0,10));

		auto* Columns = Tree.ConstructWidget<UHorizontalBox>();
		auto* Motif = Tree.ConstructWidget<UFMCodexMatchFlowDiagram>(UFMCodexMatchFlowDiagram::StaticClass(),Named(TEXT("Motif")));
		Motif->SetDiagram(bAttack ? EFMCodexFlowDiagram::FormulaAttack : EFMCodexFlowDiagram::FormulaDefense);
		Motif->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* MotifSlot = Columns->AddChildToHorizontalBox(Motif);
		MotifSlot->SetVerticalAlignment(VAlign_Center);
		MotifSlot->SetPadding(FMargin(0,0,12,0));
		auto* Composition = Tree.ConstructWidget<UVerticalBox>();
		auto* CompositionSlot = Columns->AddChildToHorizontalBox(Composition);
		CompositionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		CompositionSlot->SetVerticalAlignment(VAlign_Center);
		CompositionSlot->SetPadding(FMargin(0,0,16,0));
		OutKnownSubtotalText = MakeText(Tree,Named(TEXT("KnownSubtotal")));
		Composition->AddChildToVerticalBox(OutKnownSubtotalText)->SetPadding(FMargin(0,0,0,12));
		auto* FormulaLine = Tree.ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(),Named(TEXT("FormulaLine")));
		OutFormulaBody = Tree.ConstructWidget<UWrapBox>(UWrapBox::StaticClass(),Named(TEXT("Terms")));
		OutFormulaBody->SetInnerSlotPadding(FVector2D(7,7));
		FormulaLine->AddChildToHorizontalBox(OutFormulaBody)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		Composition->AddChildToVerticalBox(FormulaLine);

		auto* ValueRegion = Tree.ConstructWidget<UFMCodexMatchFlowPanel>(UFMCodexMatchFlowPanel::StaticClass(),Named(TEXT("FinalValueRegion")));
		ValueRegion->SetFormulaRole(EFMCodexFormulaPanelRole::Value);
		ValueRegion->SetPadding(FMargin(12,9,12,12));
		auto* ValueBody = Tree.ConstructWidget<UVerticalBox>();
		auto* Label = MakeText(Tree,Named(TEXT("ValueState")));
		Style.ApplyFlowText(*Label,13,true);
		Label->SetJustification(ETextJustify::Center);
		ValueBody->AddChildToVerticalBox(Label);
		OutFinalValueText = MakeText(Tree,Named(TEXT("FinalValue")),TEXT("?"));
		OutFinalValueText->SetJustification(ETextJustify::Center);
		auto* NumberSlot = ValueBody->AddChildToVerticalBox(OutFinalValueText);
		NumberSlot->SetPadding(FMargin(0,9,0,0));
		NumberSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		NumberSlot->SetVerticalAlignment(VAlign_Center);
		ValueRegion->AddChild(ValueBody);
		auto* ValueBounds = Tree.ConstructWidget<USizeBox>();
		ValueBounds->SetWidthOverride(128.f);
		ValueBounds->SetHeightOverride(112.f);
		ValueBounds->AddChild(ValueRegion);
		Columns->AddChildToHorizontalBox(ValueBounds)->SetVerticalAlignment(VAlign_Center);
		Body->AddChildToVerticalBox(Columns);
		return Body;
	}
}

UFMCodexInlineResolutionFormulaSurfaceWidget
	::UFMCodexInlineResolutionFormulaSurfaceWidget(
		const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget>
UFMCodexInlineResolutionFormulaSurfaceWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::RefreshFromPresentation(
	const FFMCodexUMGInlineFormulaSurfaceViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::SetEmbeddedFormulaLayout(bool bEmbedded)
{
	bEmbeddedFormulaLayout = bEmbedded;
	RefreshVisuals();
}

const FFMCodexUMGInlineFormulaSurfaceViewModel&
UFMCodexInlineResolutionFormulaSurfaceWidget::GetPresentation() const
{
	return Presentation;
}

int32 UFMCodexInlineResolutionFormulaSurfaceWidget
	::GetRenderedAttackTermCount() const
{
	return RenderedAttackTermCount;
}

int32 UFMCodexInlineResolutionFormulaSurfaceWidget
	::GetRenderedDefenseTermCount() const
{
	return RenderedDefenseTermCount;
}

int32 UFMCodexInlineResolutionFormulaSurfaceWidget
	::GetRenderedPendingTermCount() const
{
	return RenderedPendingTermCount;
}

UFMCodexRollReelWidget*
UFMCodexInlineResolutionFormulaSurfaceWidget::GetRollReelWidget() const
{
	return RollReel;
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::RequestContinue()
{
	if (Presentation.bVisible
		&& Presentation.PrimaryAction.bVisible
		&& Presentation.PrimaryAction.Action.bAvailable)
	{
		OnContinueRequested.Broadcast();
	}
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::BuildWidgetTree()
{
	using namespace FMCodexInlineResolutionFormulaSurfaceWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InlineFormulaSurfaceBounds"));
	Bounds->SetMinDesiredWidth(660.0f);
	Bounds->SetMaxDesiredWidth(820.0f);
	WidgetTree->RootWidget = Bounds;

	auto* Frame = WidgetTree->ConstructWidget<UFMCodexMatchFlowPanel>(
		UFMCodexMatchFlowPanel::StaticClass(), TEXT("InlineFormulaSurfaceFrame"));
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground, FMargin(18.0f, 14.0f));
	FLinearColor FrameColor = Style.GetColor(
		EFMCodexPlayerUIColorRole::PanelBackground);
	FrameColor.A = 0.94f;
	Frame->SetBrushColor(FrameColor);
	Bounds->AddChild(Frame);

	UVerticalBox* RootBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("InlineFormulaSurfaceHierarchy"));
	Frame->AddChild(RootBody);
	FMCodexOutcomePresentation::Build(*WidgetTree, *RootBody);

	auto* ResultBadge = MakeText(*WidgetTree,TEXT("InlineFormulaResultBadge"));
	ResultBadge->SetText(NSLOCTEXT("FMCodexFormula","ResultBadge","结算结果"));
	Style.ApplyFlowText(*ResultBadge,12,true);
	ResultBadge->SetColorAndOpacity(FLinearColor(.30f,.61f,.75f,1));
	ResultBadge->SetJustification(ETextJustify::Center);
	ResultBadge->SetVisibility(ESlateVisibility::Collapsed);
	RootBody->AddChildToVerticalBox(ResultBadge)->SetPadding(FMargin(0,0,0,7));
	ContestText = MakeText(
		*WidgetTree, TEXT("InlineFormulaContestHeading"));
	ContestText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*ContestText, EFMCodexPlayerUITextRole::ActionTitle);
	RootBody->AddChildToVerticalBox(ContestText);
	RootBody->AddChildToVerticalBox(FMCodexOutcomePresentation::BuildPrimary(*WidgetTree, TEXT("InlineFormulaOutcomeHeading")));

	StatusText = MakeText(*WidgetTree, TEXT("InlineFormulaStatus"));
	StatusText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*StatusText, EFMCodexPlayerUITextRole::Secondary);
	if (UVerticalBoxSlot* StatusSlot =
		RootBody->AddChildToVerticalBox(StatusText))
	{
		StatusSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 7.0f));
	}

	RollHelperText = MakeText(*WidgetTree, TEXT("InlineFormulaRollHelper"));
	RollHelperText->SetJustification(ETextJustify::Center);
	RollHelperText->SetAutoWrapText(true);
	Style.ApplyText(*RollHelperText, EFMCodexPlayerUITextRole::Secondary);
	auto* RuleHint = MakeBorder(*WidgetTree,TEXT("InlineFormulaRuleHint"),EFMCodexPlayerUIColorRole::PanelInset,FMargin(0));
	RuleHint->AddChild(RollHelperText);
	RootBody->AddChildToVerticalBox(RuleHint)->SetHorizontalAlignment(HAlign_Center);

	RouteResultText = MakeText(*WidgetTree, TEXT("InlineFormulaRouteResult"));
	RouteResultText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*RouteResultText, EFMCodexPlayerUITextRole::SectionHeading);
	if (UVerticalBoxSlot* RouteSlot =
		RootBody->AddChildToVerticalBox(RouteResultText))
	{
		RouteSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 7.0f));
	}

	TacticalPlayerText = MakeText(
		*WidgetTree, TEXT("InlineFormulaTacticalPlayers"));
	TacticalPlayerText->SetJustification(ETextJustify::Center);
	TacticalPlayerText->SetAutoWrapText(true);
	Style.ApplyText(*TacticalPlayerText, EFMCodexPlayerUITextRole::Secondary);
	if (UVerticalBoxSlot* TacticalSlot =
		RootBody->AddChildToVerticalBox(TacticalPlayerText))
	{
		TacticalSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	// A static display descriptor, never a set of selectable result buttons.
	TypeInformationBody = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("SetPieceTypeInformationBody"));
	TypeInformationBody->SetVisibility(ESlateVisibility::Collapsed);
	TypeInformationBody->AddChildToVerticalBox(Style.MakeFlowSeparator(*WidgetTree, TEXT("SetPieceTypeHeaderRule")));
	auto* Rules = WidgetTree->ConstructWidget<UUniformGridPanel>(
		UUniformGridPanel::StaticClass(), TEXT("SetPieceTypeRules"));
	Rules->SetSlotPadding(FMargin(8.f, 6.f));
	const auto RuleLabels = FFMCodexPlayerUIPresentationText::SetPieceTypeRuleLabels();
	for (int32 I = 0; I < RuleLabels.Num(); ++I)
	{
		const FString Name = FString::Printf(TEXT("SetPieceTypeRule%d"), I);
		auto* Entry = WidgetTree->ConstructWidget<UFMCodexMatchFlowPanel>(UFMCodexMatchFlowPanel::StaticClass(), FName(*Name));
		Entry->SetRuleCardStyle();
		Entry->SetPadding(FMargin(14.f,12.f));
		Entry->SetVisibility(ESlateVisibility::HitTestInvisible);
		auto* Row = WidgetTree->ConstructWidget<UHorizontalBox>();
		auto* RangeBounds = WidgetTree->ConstructWidget<USizeBox>();
		RangeBounds->SetWidthOverride(76.f);
		auto* Range = MakeText(*WidgetTree, FName(*(Name+TEXT("Range"))));
		Range->SetText(RuleLabels[I].Range);
		Style.ApplyFlowText(*Range, 24);
		Range->SetColorAndOpacity(FLinearColor(.40f,.73f,.88f,1));
		RangeBounds->AddChild(Range);
		Row->AddChildToHorizontalBox(RangeBounds)->SetVerticalAlignment(VAlign_Center);
		auto* Type = MakeText(*WidgetTree, FName(*(Name+TEXT("Type"))));
		Type->SetText(RuleLabels[I].TypeName);
		Style.ApplyFlowText(*Type, 16);
		Type->SetAutoWrapText(false);
		auto* TypeSlot = Row->AddChildToHorizontalBox(Type);
		TypeSlot->SetVerticalAlignment(VAlign_Center);
		TypeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TypeSlot->SetPadding(FMargin(4.f,0.f,8.f,0.f));
		auto* Diagram = WidgetTree->ConstructWidget<UFMCodexMatchFlowDiagram>(
			UFMCodexMatchFlowDiagram::StaticClass(), FName(*(Name+TEXT("Diagram"))));
		switch (RuleLabels[I].Type)
		{
		case ESetPieceSelectedType::LongFreeKick: Diagram->SetDiagram(EFMCodexFlowDiagram::LongFreeKick); break;
		case ESetPieceSelectedType::ShortFreeKick: Diagram->SetDiagram(EFMCodexFlowDiagram::ShortFreeKick); break;
		case ESetPieceSelectedType::Penalty: Diagram->SetDiagram(EFMCodexFlowDiagram::Penalty); break;
		default: Diagram->SetDiagram(EFMCodexFlowDiagram::Corner); break;
		}
		Row->AddChildToHorizontalBox(Diagram)->SetVerticalAlignment(VAlign_Center);
		Entry->AddChild(Row);
		auto* RuleSlot = Rules->AddChildToUniformGrid(Entry, I/2, I%2);
		RuleSlot->SetHorizontalAlignment(HAlign_Fill);
		RuleSlot->SetVerticalAlignment(VAlign_Fill);
	}
	TypeInformationBody->AddChildToVerticalBox(Rules)->SetPadding(FMargin(0.f, 8.f, 0.f, 10.f));
	TypeInformationBody->AddChildToVerticalBox(Style.MakeFlowSeparator(*WidgetTree, TEXT("SetPieceTypeFooterRule")));
	RootBody->AddChildToVerticalBox(TypeInformationBody);

	DiceRevealRegion = WidgetTree->ConstructWidget<UFMCodexMatchFlowPanel>(
		UFMCodexMatchFlowPanel::StaticClass(),TEXT("InlineFormulaDiceRevealRegion"));
	Style.ApplyBorder(*DiceRevealRegion,EFMCodexPlayerUIColorRole::PanelInset,FMargin(10,7));
	UHorizontalBox* DiceRevealLine =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), TEXT("InlineFormulaDiceRevealLine"));
	DiceOwnerText = MakeText(*WidgetTree, TEXT("InlineFormulaDiceOwner"));
	Style.ApplyText(*DiceOwnerText, EFMCodexPlayerUITextRole::SectionHeading);
	if (UHorizontalBoxSlot* OwnerSlot =
		DiceRevealLine->AddChildToHorizontalBox(DiceOwnerText))
	{
		OwnerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		OwnerSlot->SetVerticalAlignment(VAlign_Center);
		OwnerSlot->SetPadding(FMargin(2.0f, 0.0f, 12.0f, 0.0f));
	}
	USizeBox* DiceBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InlineFormulaDiceBounds"));
	DiceBounds->SetWidthOverride(68.0f);
	DiceBounds->SetHeightOverride(72.0f);
	RollReel = WidgetTree->ConstructWidget<UFMCodexRollReelWidget>(
		UFMCodexRollReelWidget::StaticClass(), TEXT("InlineFormulaRollReel"));
	DiceBounds->AddChild(RollReel);
	DiceRevealLine->AddChildToHorizontalBox(DiceBounds);
	DiceRevealRegion->AddChild(DiceRevealLine);
	auto* RollHostBounds = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),TEXT("InlineFormulaRollHostBounds"));
	RollHostBounds->AddChild(DiceRevealRegion);
	if (UVerticalBoxSlot* DiceSlot = RootBody->AddChildToVerticalBox(RollHostBounds))
	{
		DiceSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	AttackRegion = WidgetTree->ConstructWidget<UFMCodexMatchFlowPanel>(
		UFMCodexMatchFlowPanel::StaticClass(), TEXT("InlineFormulaAttackRegion"));
	AttackRegion->SetPadding(FMargin(18.f,14.f));
	UTextBlock* BuiltAttackSideText = nullptr;
	UWrapBox* BuiltAttackParticipantBody = nullptr;
	UTextBlock* BuiltAttackKnownSubtotalText = nullptr;
	UWrapBox* BuiltAttackFormulaBody = nullptr;
	UTextBlock* BuiltAttackFinalValueText = nullptr;
	AttackRegion->AddChild(BuildRow(
		*WidgetTree, TEXT("InlineFormulaAttack"), true, BuiltAttackSideText,
		BuiltAttackParticipantBody, BuiltAttackKnownSubtotalText,
		BuiltAttackFormulaBody,
		BuiltAttackFinalValueText));
	AttackSideText = BuiltAttackSideText;
	AttackParticipantBody = BuiltAttackParticipantBody;
	AttackKnownSubtotalText = BuiltAttackKnownSubtotalText;
	AttackFormulaBody = BuiltAttackFormulaBody;
	AttackFinalValueText = BuiltAttackFinalValueText;
	auto* HeaderRule = Style.MakeFlowSeparator(*WidgetTree, TEXT("InlineFormulaHeaderRule"));
	RootBody->AddChildToVerticalBox(HeaderRule)->SetPadding(FMargin(0,5,0,12));
	RootBody->AddChildToVerticalBox(AttackRegion);

	DefenseRegion = WidgetTree->ConstructWidget<UFMCodexMatchFlowPanel>(
		UFMCodexMatchFlowPanel::StaticClass(), TEXT("InlineFormulaDefenseRegion"));
	DefenseRegion->SetPadding(FMargin(18.f,14.f));
	UTextBlock* BuiltDefenseSideText = nullptr;
	UWrapBox* BuiltDefenseParticipantBody = nullptr;
	UTextBlock* BuiltDefenseKnownSubtotalText = nullptr;
	UWrapBox* BuiltDefenseFormulaBody = nullptr;
	UTextBlock* BuiltDefenseFinalValueText = nullptr;
	DefenseRegion->AddChild(BuildRow(
		*WidgetTree, TEXT("InlineFormulaDefense"), false, BuiltDefenseSideText,
		BuiltDefenseParticipantBody, BuiltDefenseKnownSubtotalText,
		BuiltDefenseFormulaBody,
		BuiltDefenseFinalValueText));
	DefenseSideText = BuiltDefenseSideText;
	DefenseParticipantBody = BuiltDefenseParticipantBody;
	DefenseKnownSubtotalText = BuiltDefenseKnownSubtotalText;
	DefenseFormulaBody = BuiltDefenseFormulaBody;
	DefenseFinalValueText = BuiltDefenseFinalValueText;
	if (UVerticalBoxSlot* DefenseSlot =
		RootBody->AddChildToVerticalBox(DefenseRegion))
	{
		DefenseSlot->SetPadding(FMargin(0.0f, 7.0f, 0.0f, 0.0f));
	}

	auto* FooterRule = Style.MakeFlowSeparator(*WidgetTree, TEXT("InlineFormulaFooterRule"));
	RootBody->AddChildToVerticalBox(FooterRule)->SetPadding(FMargin(0,14,0,0));
	ContinueButton = WidgetTree->ConstructWidget<UFMCodexMatchFlowButton>(
		UFMCodexMatchFlowButton::StaticClass(), TEXT("InlineFormulaContinueButton"));
	Style.ApplyButton(*ContinueButton, EFMCodexPlayerUIActionRole::Primary);
	UTextBlock* ContinueText = MakeText(
		*WidgetTree, TEXT("InlineFormulaContinueButtonLabel"));
	ContinueText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*ContinueText, EFMCodexPlayerUITextRole::Body);
	ContinueButton->AddChild(ContinueText);
	ContinueButton->OnClicked.AddDynamic(
		this,
		&UFMCodexInlineResolutionFormulaSurfaceWidget::HandleContinueClicked);
	USizeBox* ContinueBounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("InlineFormulaContinueBounds"));
	ContinueBounds->SetWidthOverride(156.0f);
	ContinueBounds->SetHeightOverride(42.0f);
	ContinueBounds->AddChild(ContinueButton);
	if (UVerticalBoxSlot* ContinueSlot =
		RootBody->AddChildToVerticalBox(ContinueBounds))
	{
		ContinueSlot->SetHorizontalAlignment(HAlign_Center);
		ContinueSlot->SetPadding(FMargin(0.0f, 10.0f, 0.0f, 0.0f));
	}
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::RefreshVisuals()
{
	if (ContestText == nullptr)
	{
		return;
	}
	SetVisibility(Presentation.bVisible
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);
	ContestText->SetText(FText::FromString(Presentation.ContestLabel));
	ContestText->SetVisibility(
		!Presentation.ContestLabel.IsEmpty()
			&& (!Presentation.bParentOwnsContestHeading
				|| Presentation.bNarrativeAvailable)
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	StatusText->SetText(FText::FromString(Presentation.StatusLabel));
	RollHelperText->SetText(FText::FromString(Presentation.RollHelperLabel));
	RollHelperText->SetVisibility(Presentation.RollHelperLabel.IsEmpty()
		? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	RouteResultText->SetText(FText::FromString(Presentation.RouteResultLabel));
	RouteResultText->SetVisibility(Presentation.RouteResultLabel.IsEmpty()
			|| Presentation.bParentOwnsRouteContext
		? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	TacticalPlayerText->SetText(
		FText::FromString(Presentation.TacticalPlayerSummaryLabel));
	TacticalPlayerText->SetVisibility(
		Presentation.TacticalPlayerSummaryLabel.IsEmpty()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::SelfHitTestInvisible);
	DiceRevealRegion->SetVisibility(
		Presentation.bVisible && Presentation.bDiceRevealVisible
			? ESlateVisibility::SelfHitTestInvisible
			: ESlateVisibility::Collapsed);
	DiceOwnerText->SetText(FText::FromString(Presentation.DiceOwnerLabel));
	RollReel->RefreshFromPresentation(Presentation.RollReel);
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	// Arithmetic mode is semantic, not inferred from a title or a tactic name.
	// Type information and arithmetic retain their accepted parameters.
	const bool bFormula = Presentation.bVisible && Presentation.bShowFormulaRows;
	const bool bOutcomeFamily = FMCodexOutcomePresentation::OwnsInlineSurface(Presentation);
	const bool bEmbeddedFormula = bFormula && bEmbeddedFormulaLayout;
	const bool bTypeInformation = Presentation.bVisible
		&& Presentation.ContestId == FName(TEXT("SetPiece.Type"))
		&& !Presentation.bDiceRevealVisible
		&& !Presentation.TacticalPlayerSummaryLabel.IsEmpty();
	TypeInformationBody->SetVisibility(bTypeInformation
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bTypeInformation) TacticalPlayerText->SetVisibility(ESlateVisibility::Collapsed);
	auto* Frame = CastChecked<UFMCodexMatchFlowPanel>(GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")));
	Frame->SetFlowStyleEnabled(bTypeInformation || ((bFormula || bOutcomeFamily) && !bEmbeddedFormulaLayout));
	FLinearColor LegacyFrameColor = Style.GetColor(EFMCodexPlayerUIColorRole::PanelBackground);
	LegacyFrameColor.A = .94f;
	Frame->SetBrushColor(bEmbeddedFormula ? FLinearColor::Transparent : LegacyFrameColor);
	GetWidgetFromName(TEXT("InlineFormulaHeaderRule"))->SetVisibility(bFormula
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	GetWidgetFromName(TEXT("InlineFormulaFooterRule"))->SetVisibility(bFormula
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	CastChecked<UFMCodexMatchFlowButton>(ContinueButton)->SetFlowStyleEnabled(bTypeInformation || bFormula);
	CastChecked<USizeBox>(GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")))->SetMinDesiredWidth(bTypeInformation || bFormula ? 760.f : 660.f);
	Frame->SetPadding(bEmbeddedFormula ? FMargin(0) : bTypeInformation || bFormula
		? FMargin(22.f,22.f) : FMargin(18.f,14.f));
	Style.ApplyText(*ContestText, EFMCodexPlayerUITextRole::ActionTitle);
	Style.ApplyText(*StatusText, EFMCodexPlayerUITextRole::Secondary);
	Style.ApplyButton(*ContinueButton, EFMCodexPlayerUIActionRole::Primary);
	auto* ContinueBounds = CastChecked<USizeBox>(ContinueButton->GetParent());
	ContinueBounds->ClearMinDesiredHeight();
	ContinueBounds->SetWidthOverride(bFormula ? 288.f : bTypeInformation ? 224.f : 156.f);
	ContinueBounds->SetHeightOverride(bFormula ? 52.f : bTypeInformation ? 48.f : 42.f);
	auto* ContinueLabel = CastChecked<UTextBlock>(ContinueButton->GetChildAt(0));
	ContinueLabel->SetAutoWrapText(false);
	Style.ApplyText(*ContinueLabel, EFMCodexPlayerUITextRole::Body);
	if (bTypeInformation)
	{
		Style.ApplyFlowText(*ContestText, 24);
		Style.ApplyFlowText(*StatusText, 14, true);
		Style.ApplyFlowText(*ContinueLabel, 18);
		ContinueButton->SetStyle(Style.MakeFlowButtonStyle());
	}
	if (bFormula)
	{
		Style.ApplyFlowText(*ContestText, Presentation.bNarrativeAvailable ? 28 : 24);
		ContestText->SetAutoWrapText(true);
		Style.ApplyFlowText(*StatusText, Presentation.bNarrativeAvailable ? 13 : 15, true);
		StatusText->SetAutoWrapText(true);
		Style.ApplyFlowText(*RollHelperText, 12, true);
		Style.ApplyFlowText(*RouteResultText, 14, true);
		Style.ApplyFlowText(*ContinueLabel, 20);
		auto ActionStyle = Style.MakeFlowButtonStyle();
		ActionStyle.Normal.TintColor = FLinearColor(.009f,.052f,.105f,1);
		ActionStyle.Normal.OutlineSettings.Color = FLinearColor(.10f,.43f,.60f,1);
		ContinueButton->SetStyle(ActionStyle);
	}
	else
	{
		ContestText->SetAutoWrapText(false);
		StatusText->SetAutoWrapText(false);
		Style.ApplyText(*RollHelperText, EFMCodexPlayerUITextRole::Secondary);
		Style.ApplyText(*RouteResultText, EFMCodexPlayerUITextRole::SectionHeading);
	}
	const bool bFormulaRoll = bFormula && Presentation.bDiceRevealVisible;
	GetWidgetFromName(TEXT("InlineFormulaResultBadge"))->SetVisibility(bFormula && Presentation.bNarrativeAvailable
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	// One owner/phase line inside the compact reveal host replaces duplicated
	// rolling status. No modification to the projected phase, clock or reel data.
	StatusText->SetVisibility(bFormulaRoll ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	auto* RuleHint = CastChecked<UBorder>(GetWidgetFromName(TEXT("InlineFormulaRuleHint")));
	RuleHint->SetVisibility(Presentation.RollHelperLabel.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	RuleHint->SetPadding(bFormula ? FMargin(12,4) : FMargin(0));
	CastChecked<UVerticalBoxSlot>(RuleHint->Slot)->SetHorizontalAlignment(bFormula ? HAlign_Center : HAlign_Fill);
	RuleHint->SetBrushColor(bFormula ? FLinearColor(.003f,.015f,.030f,1) : FLinearColor::Transparent);
	auto* RollHostBounds = CastChecked<USizeBox>(GetWidgetFromName(TEXT("InlineFormulaRollHostBounds")));
	RollHostBounds->SetVisibility(Presentation.bVisible && Presentation.bDiceRevealVisible
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	auto* RollHostSlot = CastChecked<UVerticalBoxSlot>(RollHostBounds->Slot);
	RollHostSlot->SetHorizontalAlignment(bFormula ? HAlign_Center : HAlign_Fill);
	if (bFormula) RollHostBounds->SetWidthOverride(360.f); else RollHostBounds->ClearWidthOverride();
	auto* RollPanel = CastChecked<UFMCodexMatchFlowPanel>(DiceRevealRegion);
	RollPanel->SetFormulaRole(bFormula || bOutcomeFamily ? EFMCodexFormulaPanelRole::RollHost : EFMCodexFormulaPanelRole::None);
	Style.ApplyBorder(*DiceRevealRegion,EFMCodexPlayerUIColorRole::PanelInset,bFormula ? FMargin(20,10) : FMargin(10,7));
	auto* DiceBounds = CastChecked<USizeBox>(GetWidgetFromName(TEXT("InlineFormulaDiceBounds")));
	DiceBounds->SetWidthOverride(bFormula ? 96.f : 68.f);
	DiceBounds->SetHeightOverride(bFormula ? 112.f : 72.f);
	RollReel->SetExpandedChamber(bFormula);
	Style.ApplyText(*DiceOwnerText,EFMCodexPlayerUITextRole::SectionHeading);
	if (bFormulaRoll)
	{
		Style.ApplyFlowText(*DiceOwnerText,20);
		DiceOwnerText->SetText(Presentation.bDiceRolling
			? FText::Format(NSLOCTEXT("FMCodexFormula","RollingOwner","{0}中"),FText::FromString(Presentation.DiceOwnerLabel))
			: FText::FromString(Presentation.DiceOwnerLabel));
	}
	CastChecked<UFMCodexMatchFlowPanel>(AttackRegion)->SetContestRowStyle(Presentation.bAttackRowActive);
	CastChecked<UFMCodexMatchFlowPanel>(DefenseRegion)->SetContestRowStyle(Presentation.bDefenseRowActive);
	AttackRegion->SetVisibility(Presentation.bShowFormulaRows && Presentation.bShowAttackRow
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	DefenseRegion->SetVisibility(Presentation.bShowFormulaRows && Presentation.bShowDefenseRow
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	RenderedPendingTermCount = 0;
	RefreshRow(Presentation.AttackRow, TEXT("InlineFormulaAttack"),
		AttackSideText, AttackParticipantBody, AttackKnownSubtotalText,
		AttackFormulaBody,
		AttackFinalValueText, AttackParticipantItems, AttackTermItems);
	RenderedAttackTermCount = Presentation.AttackRow.Terms.Num();
	RefreshRow(Presentation.DefenseRow, TEXT("InlineFormulaDefense"),
		DefenseSideText, DefenseParticipantBody, DefenseKnownSubtotalText,
		DefenseFormulaBody,
		DefenseFinalValueText, DefenseParticipantItems, DefenseTermItems);
	RenderedDefenseTermCount = Presentation.DefenseRow.Terms.Num();

	ContinueButton->GetParent()->SetVisibility(
		Presentation.bVisible && Presentation.PrimaryAction.bVisible
			&& Presentation.PrimaryAction.Action.bAvailable
			? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ContinueButton->SetIsEnabled(
		Presentation.PrimaryAction.Action.bAvailable);
	if (UTextBlock* Label = Cast<UTextBlock>(ContinueButton->GetChildAt(0)))
	{
		Label->SetText(FText::FromString(
			Presentation.PrimaryAction.Action.Label));
	}
	// Keep the family frame during ResultHold; full final content belongs to
	// the composition only after the existing reel lifetime ends.
	const bool bOutcome = bOutcomeFamily && FMCodexOutcomePresentation::IsFinalReady(Presentation.bNarrativeAvailable, Presentation.bDiceRevealVisible);
	FMCodexOutcomePresentation::RefreshIntermediate(*WidgetTree, bOutcomeFamily && Presentation.bDiceRevealVisible,
		Presentation.ResolutionContextLabel, Presentation.OutcomeRollDetail.IsEmpty() ? Presentation.RouteResultLabel : Presentation.OutcomeRollDetail);
	FMCodexOutcomePresentation::Refresh(*WidgetTree, bOutcome,
		Presentation.ContestLabel, Presentation.StatusLabel,
		Presentation.bParentOwnsRouteContext ? FString() : !Presentation.OutcomeRollDetail.IsEmpty()
			? Presentation.OutcomeRollDetail : Presentation.RouteResultLabel,
		Presentation.RollHelperLabel, ContinueButton->GetParent()->GetVisibility() != ESlateVisibility::Collapsed,
		Presentation.OutcomeText);
	if (bOutcomeFamily)
	{
		FMCodexOutcomePresentation::ApplyFrameStyle(*Frame,
			*CastChecked<USizeBox>(GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds"))), bEmbeddedFormulaLayout);
	}
	if (bOutcomeFamily)
	{
		UWidget* Hidden[] = {ContestText, StatusText, RouteResultText, TacticalPlayerText};
		for (auto* Item : Hidden) Item->SetVisibility(ESlateVisibility::Collapsed);
		RuleHint->SetVisibility(ESlateVisibility::Collapsed);
		if (Presentation.bNarrativeAvailable) RollHelperText->SetVisibility(ESlateVisibility::Collapsed);
		if (bOutcome) FMCodexOutcomePresentation::ApplyActionStyle(*ContinueButton);
		else ContinueButton->GetParent()->SetVisibility(ESlateVisibility::Collapsed);
	}
	// Formula-linked results share only the semantic sentence renderer. All rows,
	// values, chips, margins and embedded roll geometry retain their accepted layout.
	const bool bFormulaFinal = bFormula && FMCodexOutcomePresentation::IsFinalReady(Presentation.bNarrativeAvailable, Presentation.bDiceRevealVisible);
	auto* FormulaHeadline = CastChecked<URichTextBlock>(GetWidgetFromName(TEXT("InlineFormulaOutcomeHeading")));
	FormulaHeadline->SetText(bFormulaFinal ? FText::FromString(FMCodexOutcomePresentation::PrimaryMarkup(Presentation.ContestLabel, Presentation.OutcomeText)) : FText::GetEmpty());
	FormulaHeadline->SetVisibility(bFormulaFinal ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bFormulaFinal) ContestText->SetVisibility(ESlateVisibility::Collapsed);
	if (bFormula && Presentation.bDiceRevealVisible && Presentation.bNarrativeAvailable)
	{
		ContestText->SetText(Presentation.ResolutionContextLabel.IsEmpty()
			? FFMCodexPlayerUIPresentationText::ResolutionContest(Presentation.ContestId) : FText::FromString(Presentation.ResolutionContextLabel));
		Style.ApplyFlowText(*ContestText, 24);
		StatusText->SetText(NSLOCTEXT("FMCodexOutcome", "FormulaResolving", "正在结算"));
		GetWidgetFromName(TEXT("InlineFormulaResultBadge"))->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::RefreshRow(
	const FFMCodexUMGInlineFormulaRowViewModel& Row,
	const FString& WidgetNamePrefix,
	UTextBlock* SideText,
	UWrapBox* ParticipantBody,
	UTextBlock* KnownSubtotalText,
	UWrapBox* FormulaBody,
	UTextBlock* FinalValueText,
	TArray<TObjectPtr<UWidget>>& ParticipantItems,
	TArray<TObjectPtr<UWidget>>& TermItems)
{
	using namespace FMCodexInlineResolutionFormulaSurfaceWidget;
	if (SideText == nullptr || ParticipantBody == nullptr
		|| KnownSubtotalText == nullptr
		|| FormulaBody == nullptr || FinalValueText == nullptr
		|| WidgetTree == nullptr)
	{
		return;
	}
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	SideText->SetText(FText::FromString(Row.SideLabel));
	Style.ApplyFlowText(*SideText, 20);
	SideText->SetColorAndOpacity(FLinearColor(.57f,.79f,.90f,1));
	Style.ApplyFlowText(*KnownSubtotalText, 17);
	KnownSubtotalText->SetAutoWrapText(true);
	KnownSubtotalText->SetText(
		FText::FromString(Row.KnownNonRollSubtotalLabel));

	while (ParticipantItems.Num() < Row.Participants.Num())
	{
		const int32 Index = ParticipantItems.Num();
		UBorder* Chip = MakeBorder(
			*WidgetTree,
			FName(*FString::Printf(TEXT("%sParticipant%d"),
				*WidgetNamePrefix, Index)),
			EFMCodexPlayerUIColorRole::PanelInset,
			FMargin(10.0f, 6.0f));
		UHorizontalBox* Identity = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*FString::Printf(TEXT("%sParticipantIdentity%d"),
				*WidgetNamePrefix, Index)));
		UTextBlock* Role = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("%sParticipantRole%d"),
				*WidgetNamePrefix, Index)));
		Style.ApplyText(*Role, EFMCodexPlayerUITextRole::Secondary);
		if (UHorizontalBoxSlot* RoleSlot = Identity->AddChildToHorizontalBox(Role))
		{
			RoleSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
			RoleSlot->SetVerticalAlignment(VAlign_Center);
		}
		UTextBlock* Name = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("%sParticipantName%d"),
				*WidgetNamePrefix, Index)));
		Style.ApplyText(*Name, EFMCodexPlayerUITextRole::Body);
		Identity->AddChildToHorizontalBox(Name)->SetVerticalAlignment(VAlign_Center);
		Chip->AddChild(Identity);
		ParticipantBody->AddChildToWrapBox(Chip);
		ParticipantItems.Add(Chip);
	}
	for (int32 Index = 0; Index < ParticipantItems.Num(); ++Index)
	{
		UBorder* Chip = Cast<UBorder>(ParticipantItems[Index]);
		const bool bUsed = Row.Participants.IsValidIndex(Index);
		if (Chip == nullptr)
		{
			continue;
		}
		Chip->SetVisibility(bUsed
			? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (!bUsed)
		{
			continue;
		}
		Chip->SetBrush(FSlateRoundedBoxBrush(FLinearColor(.007f,.023f,.039f,1),
			5.f,FLinearColor(.08f,.25f,.36f,.8f),1.f));
		Chip->SetBrushColor(FLinearColor::White);
		const auto& Participant = Row.Participants[Index];
		if (UHorizontalBox* Identity = Cast<UHorizontalBox>(Chip->GetChildAt(0)))
		{
			if (UTextBlock* Role = Cast<UTextBlock>(Identity->GetChildAt(0)))
			{
				Role->SetText(FText::FromString(Participant.RoleLabel));
				Style.ApplyFlowText(*Role, 14, true);
			}
			if (UTextBlock* Name = Cast<UTextBlock>(Identity->GetChildAt(1)))
			{
				Name->SetText(FText::FromString(Participant.PlayerName));
				Style.ApplyFlowText(*Name, 17);
			}
		}
	}

	while (TermItems.Num() < Row.Terms.Num())
	{
		const int32 Index = TermItems.Num();
		UHorizontalBox* TermItem = WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(), FName(*FString::Printf(
				TEXT("%sTermItem%d"), *WidgetNamePrefix, Index)));
		UTextBlock* Plus = MakeText(
			*WidgetTree, FName(*FString::Printf(TEXT("%sPlus%d"),
				*WidgetNamePrefix, Index)), TEXT("+"));
		Style.ApplyText(*Plus, EFMCodexPlayerUITextRole::Body);
		if (UHorizontalBoxSlot* PlusSlot = TermItem->AddChildToHorizontalBox(Plus))
		{
			PlusSlot->SetPadding(FMargin(0.0f, 0.0f, 5.0f, 0.0f));
			PlusSlot->SetVerticalAlignment(VAlign_Center);
		}
		UBorder* Operand = MakeBorder(
			*WidgetTree,
			FName(*FString::Printf(TEXT("%sOperand%d"),
				*WidgetNamePrefix, Index)),
			EFMCodexPlayerUIColorRole::PanelInset,
			FMargin(7.0f, 3.0f));
		UTextBlock* OperandText = MakeText(
			*WidgetTree, FName(*FString::Printf(TEXT("%sOperandText%d"),
				*WidgetNamePrefix, Index)));
		Operand->AddChild(OperandText);
		TermItem->AddChildToHorizontalBox(Operand);
		FormulaBody->AddChildToWrapBox(TermItem);
		TermItems.Add(TermItem);
	}
	for (int32 Index = 0; Index < TermItems.Num(); ++Index)
	{
		UHorizontalBox* TermItem = Cast<UHorizontalBox>(TermItems[Index]);
		const bool bUsed = Row.Terms.IsValidIndex(Index);
		if (TermItem == nullptr)
		{
			continue;
		}
		TermItem->SetVisibility(bUsed
			? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (!bUsed)
		{
			continue;
		}
		const FFMCodexUMGInlineFormulaTermViewModel& Term = Row.Terms[Index];
		UTextBlock* Plus = Cast<UTextBlock>(TermItem->GetChildAt(0));
		UBorder* Operand = Cast<UBorder>(TermItem->GetChildAt(1));
		UTextBlock* OperandText = Operand == nullptr
			? nullptr : Cast<UTextBlock>(Operand->GetChildAt(0));
		if (Plus != nullptr)
		{
			Plus->SetVisibility(Index > 0
				&& Term.Kind != EFMCodexUMGInlineFormulaTermKind::FixedModifier
					? ESlateVisibility::HitTestInvisible
					: ESlateVisibility::Collapsed);
		}
		if (Operand == nullptr || OperandText == nullptr)
		{
			continue;
		}
		const bool bPending = Term.bNextPendingRoll;
		Operand->SetBrush(FSlateRoundedBoxBrush(
			bPending ? FLinearColor(.012f,.062f,.091f,1) : FLinearColor(.006f,.023f,.038f,1),
			3.f, bPending ? FLinearColor(.13f,.46f,.60f,.9f) : FLinearColor(.045f,.15f,.23f,.8f),1.f));
		Operand->SetBrushColor(FLinearColor::White);
		Operand->SetPadding(FMargin(10.f,8.f));
		if (bPending) ++RenderedPendingTermCount;
		const FString OperandLabel = Term.ContributorDisplayName.IsEmpty()
			? Term.DisplayLabel
			: FString::Printf(TEXT("%s %s"),
				*Term.ContributorDisplayName, *Term.DisplayLabel);
		OperandText->SetText(FText::FromString(OperandLabel));
		Style.ApplyFlowText(*OperandText, 17);
		if (Presentation.bNarrativeAvailable && Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll && Term.bResolved)
			OperandText->SetColorAndOpacity(FLinearColor(.82f,.64f,.34f,1));
		if (Plus) Style.ApplyFlowText(*Plus, 16);
	}
	FinalValueText->SetText(FText::FromString(Row.DisplayedResultLabel));
	// Consume only the already-gated displayed-value distinction. Never inspect a
	// hidden FinalValue or infer a winner from the amount or the other row.
	const bool bFinal = Row.bDisplayedResultResolved && Row.bDisplayedResultIsFinalValue;
	auto* ValueState = CastChecked<UTextBlock>(GetWidgetFromName(FName(*(WidgetNamePrefix + TEXT("ValueState")))));
	ValueState->SetText(bFinal
		? NSLOCTEXT("FMCodexFormula", "FinalValue", "最终值")
		: NSLOCTEXT("FMCodexFormula", "CurrentValue", "当前值"));
	const FLinearColor Gold(.82f,.64f,.34f,1);
	Style.ApplyFlowText(*FinalValueText, bFinal ? 42 : 30);
	FinalValueText->SetColorAndOpacity(bFinal ? Gold : FLinearColor(.57f,.73f,.81f,1));
	ValueState->SetColorAndOpacity(bFinal ? Gold : FLinearColor(.40f,.56f,.66f,1));
	auto* ValueRegion = CastChecked<UFMCodexMatchFlowPanel>(GetWidgetFromName(FName(*(WidgetNamePrefix + TEXT("FinalValueRegion")))));
	ValueRegion->SetFormulaRole(EFMCodexFormulaPanelRole::Value,bFinal);
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::HandleContinueClicked()
{
	RequestContinue();
}
