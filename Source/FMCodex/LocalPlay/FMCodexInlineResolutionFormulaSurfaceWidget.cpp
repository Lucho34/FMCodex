#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"

#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexMatchFlowPanel.h"
#include "FMCodexRollReelWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
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
		UWidgetTree& Tree,
		const FName Prefix,
		UTextBlock*& OutSideText,
		UWrapBox*& OutParticipantBody,
		UTextBlock*& OutKnownSubtotalText,
		UWrapBox*& OutFormulaBody,
		UTextBlock*& OutFinalValueText)
	{
		const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
		UVerticalBox* Body = Tree.ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("Hierarchy"))));
		UHorizontalBox* IdentityLine = Tree.ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("IdentityLine"))));
		OutSideText = MakeText(Tree,
			FName(*(Prefix.ToString() + TEXT("Side"))));
		Style.ApplyText(*OutSideText, EFMCodexPlayerUITextRole::SectionHeading);
		if (UHorizontalBoxSlot* SideSlot =
			IdentityLine->AddChildToHorizontalBox(OutSideText))
		{
			SideSlot->SetPadding(FMargin(0.0f, 0.0f, 14.0f, 0.0f));
			SideSlot->SetVerticalAlignment(VAlign_Center);
		}
		OutParticipantBody = Tree.ConstructWidget<UWrapBox>(
			UWrapBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("Participants"))));
		OutParticipantBody->SetInnerSlotPadding(FVector2D(6.0f, 3.0f));
		if (UHorizontalBoxSlot* ParticipantSlot =
			IdentityLine->AddChildToHorizontalBox(OutParticipantBody))
		{
			ParticipantSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			ParticipantSlot->SetVerticalAlignment(VAlign_Center);
		}
		Body->AddChildToVerticalBox(IdentityLine);

		OutKnownSubtotalText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("KnownSubtotal"))));
		Style.ApplyText(
			*OutKnownSubtotalText, EFMCodexPlayerUITextRole::Status);
		if (UVerticalBoxSlot* SubtotalSlot =
			Body->AddChildToVerticalBox(OutKnownSubtotalText))
		{
			SubtotalSlot->SetPadding(FMargin(0.0f, 5.0f, 0.0f, 0.0f));
		}

		UHorizontalBox* FormulaLine = Tree.ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("FormulaLine"))));
		OutFormulaBody = Tree.ConstructWidget<UWrapBox>(
			UWrapBox::StaticClass(),
			FName(*(Prefix.ToString() + TEXT("Terms"))));
		OutFormulaBody->SetInnerSlotPadding(FVector2D(5.0f, 4.0f));
		if (UHorizontalBoxSlot* TermsSlot =
			FormulaLine->AddChildToHorizontalBox(OutFormulaBody))
		{
			TermsSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			TermsSlot->SetVerticalAlignment(VAlign_Center);
		}
		UTextBlock* Equals = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("Equals"))), TEXT("="));
		Style.ApplyText(*Equals, EFMCodexPlayerUITextRole::Body);
		if (UHorizontalBoxSlot* EqualsSlot =
			FormulaLine->AddChildToHorizontalBox(Equals))
		{
			EqualsSlot->SetPadding(FMargin(7.0f, 0.0f));
			EqualsSlot->SetVerticalAlignment(VAlign_Center);
		}
		UBorder* FinalValueRegion = MakeBorder(
			Tree, FName(*(Prefix.ToString() + TEXT("FinalValueRegion"))),
			EFMCodexPlayerUIColorRole::PanelInset, FMargin(8.0f, 3.0f));
		OutFinalValueText = MakeText(
			Tree, FName(*(Prefix.ToString() + TEXT("FinalValue"))), TEXT("?"));
		FinalValueRegion->AddChild(OutFinalValueText);
		FormulaLine->AddChildToHorizontalBox(FinalValueRegion);
		if (UVerticalBoxSlot* FormulaSlot =
			Body->AddChildToVerticalBox(FormulaLine))
		{
			FormulaSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
		}
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

	ContestText = MakeText(
		*WidgetTree, TEXT("InlineFormulaContestHeading"));
	ContestText->SetJustification(ETextJustify::Center);
	Style.ApplyText(*ContestText, EFMCodexPlayerUITextRole::ActionTitle);
	RootBody->AddChildToVerticalBox(ContestText);

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
	RootBody->AddChildToVerticalBox(RollHelperText);

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

	DiceRevealRegion = MakeBorder(
		*WidgetTree, TEXT("InlineFormulaDiceRevealRegion"),
		EFMCodexPlayerUIColorRole::PanelInset, FMargin(10.0f, 7.0f));
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
	if (UVerticalBoxSlot* DiceSlot =
		RootBody->AddChildToVerticalBox(DiceRevealRegion))
	{
		DiceSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	AttackRegion = MakeBorder(
		*WidgetTree, TEXT("InlineFormulaAttackRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised, FMargin(12.0f, 9.0f));
	UTextBlock* BuiltAttackSideText = nullptr;
	UWrapBox* BuiltAttackParticipantBody = nullptr;
	UTextBlock* BuiltAttackKnownSubtotalText = nullptr;
	UWrapBox* BuiltAttackFormulaBody = nullptr;
	UTextBlock* BuiltAttackFinalValueText = nullptr;
	AttackRegion->AddChild(BuildRow(
		*WidgetTree, TEXT("InlineFormulaAttack"), BuiltAttackSideText,
		BuiltAttackParticipantBody, BuiltAttackKnownSubtotalText,
		BuiltAttackFormulaBody,
		BuiltAttackFinalValueText));
	AttackSideText = BuiltAttackSideText;
	AttackParticipantBody = BuiltAttackParticipantBody;
	AttackKnownSubtotalText = BuiltAttackKnownSubtotalText;
	AttackFormulaBody = BuiltAttackFormulaBody;
	AttackFinalValueText = BuiltAttackFinalValueText;
	RootBody->AddChildToVerticalBox(AttackRegion);

	DefenseRegion = MakeBorder(
		*WidgetTree, TEXT("InlineFormulaDefenseRegion"),
		EFMCodexPlayerUIColorRole::PanelRaised, FMargin(12.0f, 9.0f));
	UTextBlock* BuiltDefenseSideText = nullptr;
	UWrapBox* BuiltDefenseParticipantBody = nullptr;
	UTextBlock* BuiltDefenseKnownSubtotalText = nullptr;
	UWrapBox* BuiltDefenseFormulaBody = nullptr;
	UTextBlock* BuiltDefenseFinalValueText = nullptr;
	DefenseRegion->AddChild(BuildRow(
		*WidgetTree, TEXT("InlineFormulaDefense"), BuiltDefenseSideText,
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
	// Only the pre-roll information mode opts in. All formula and reel layouts
	// retain their original styling, including after this pooled widget is reused.
	const bool bTypeInformation = Presentation.bVisible
		&& Presentation.ContestId == FName(TEXT("SetPiece.Type"))
		&& !Presentation.bDiceRevealVisible
		&& !Presentation.TacticalPlayerSummaryLabel.IsEmpty();
	TypeInformationBody->SetVisibility(bTypeInformation
		? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (bTypeInformation) TacticalPlayerText->SetVisibility(ESlateVisibility::Collapsed);
	auto* Frame = CastChecked<UFMCodexMatchFlowPanel>(GetWidgetFromName(TEXT("InlineFormulaSurfaceFrame")));
	Frame->SetFlowStyleEnabled(bTypeInformation);
	CastChecked<UFMCodexMatchFlowButton>(ContinueButton)->SetFlowStyleEnabled(bTypeInformation);
	CastChecked<USizeBox>(GetWidgetFromName(TEXT("InlineFormulaSurfaceBounds")))->SetMinDesiredWidth(bTypeInformation ? 760.f : 660.f);
	Frame->SetPadding(bTypeInformation ? FMargin(22.f, 22.f) : FMargin(18.f,14.f));
	Style.ApplyText(*ContestText, EFMCodexPlayerUITextRole::ActionTitle);
	Style.ApplyText(*StatusText, EFMCodexPlayerUITextRole::Secondary);
	Style.ApplyButton(*ContinueButton, EFMCodexPlayerUIActionRole::Primary);
	auto* ContinueBounds = CastChecked<USizeBox>(ContinueButton->GetParent());
	ContinueBounds->SetWidthOverride(bTypeInformation ? 224.f : 156.f);
	ContinueBounds->SetHeightOverride(bTypeInformation ? 48.f : 42.f);
	auto* ContinueLabel = CastChecked<UTextBlock>(ContinueButton->GetChildAt(0));
	Style.ApplyText(*ContinueLabel, EFMCodexPlayerUITextRole::Body);
	if (bTypeInformation)
	{
		Style.ApplyFlowText(*ContestText, 24);
		Style.ApplyFlowText(*StatusText, 14, true);
		Style.ApplyFlowText(*ContinueLabel, 18);
		ContinueButton->SetStyle(Style.MakeFlowButtonStyle());
	}
	Style.ApplyBorder(*AttackRegion,
		Presentation.bAttackRowActive
			? EFMCodexPlayerUIColorRole::Warning
			: EFMCodexPlayerUIColorRole::PanelRaised,
		FMargin(12.0f, 9.0f));
	AttackRegion->SetVisibility(Presentation.bShowFormulaRows
		&& Presentation.bShowAttackRow
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	Style.ApplyBorder(*DefenseRegion,
		Presentation.bDefenseRowActive
			? EFMCodexPlayerUIColorRole::Warning
			: EFMCodexPlayerUIColorRole::PanelRaised,
		FMargin(12.0f, 9.0f));
	DefenseRegion->SetVisibility(Presentation.bShowFormulaRows
		&& Presentation.bShowDefenseRow
		? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (Presentation.bAttackRowActive)
	{
		FLinearColor ActiveColor = Style.GetColor(
			EFMCodexPlayerUIColorRole::Warning);
		ActiveColor.A = 0.32f;
		AttackRegion->SetBrushColor(ActiveColor);
	}
	if (Presentation.bDefenseRowActive)
	{
		FLinearColor ActiveColor = Style.GetColor(
			EFMCodexPlayerUIColorRole::Warning);
		ActiveColor.A = 0.32f;
		DefenseRegion->SetBrushColor(ActiveColor);
	}

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
			FMargin(6.0f, 2.0f));
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
			RoleSlot->SetPadding(FMargin(0.0f, 0.0f, 4.0f, 0.0f));
		}
		UTextBlock* Name = MakeText(
			*WidgetTree,
			FName(*FString::Printf(TEXT("%sParticipantName%d"),
				*WidgetNamePrefix, Index)));
		Style.ApplyText(*Name, EFMCodexPlayerUITextRole::Body);
		Identity->AddChildToHorizontalBox(Name);
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
		const auto& Participant = Row.Participants[Index];
		if (UHorizontalBox* Identity = Cast<UHorizontalBox>(Chip->GetChildAt(0)))
		{
			if (UTextBlock* Role = Cast<UTextBlock>(Identity->GetChildAt(0)))
			{
				Role->SetText(FText::FromString(Participant.RoleLabel));
			}
			if (UTextBlock* Name = Cast<UTextBlock>(Identity->GetChildAt(1)))
			{
				Name->SetText(FText::FromString(Participant.PlayerName));
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
		Style.ApplyBorder(*Operand, bPending
			? EFMCodexPlayerUIColorRole::Warning
			: EFMCodexPlayerUIColorRole::PanelInset, FMargin(7.0f, 3.0f));
		if (bPending)
		{
			FLinearColor PendingColor = Style.GetColor(
				EFMCodexPlayerUIColorRole::Warning);
			PendingColor.A = 0.48f;
			Operand->SetBrushColor(PendingColor);
			++RenderedPendingTermCount;
		}
		const FString OperandLabel = Term.ContributorDisplayName.IsEmpty()
			? Term.DisplayLabel
			: FString::Printf(TEXT("%s %s"),
				*Term.ContributorDisplayName, *Term.DisplayLabel);
		OperandText->SetText(FText::FromString(OperandLabel));
		Style.ApplyText(*OperandText, bPending
			? EFMCodexPlayerUITextRole::Status
			: EFMCodexPlayerUITextRole::Body);
	}
	FinalValueText->SetText(FText::FromString(Row.DisplayedResultLabel));
	Style.ApplyText(*FinalValueText, Row.bDisplayedResultResolved
		? EFMCodexPlayerUITextRole::DiceValue
		: EFMCodexPlayerUITextRole::Status);
}

void UFMCodexInlineResolutionFormulaSurfaceWidget::HandleContinueClicked()
{
	RequestContinue();
}
