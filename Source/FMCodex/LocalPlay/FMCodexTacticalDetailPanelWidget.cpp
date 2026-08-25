#include "FMCodexTacticalDetailPanelWidget.h"

#include "FMCodexPlayerUIStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WrapBox.h"
#include "Components/WrapBoxSlot.h"

namespace FMCodexTacticalDetailPanelWidget
{
	constexpr float PanelWidth = 780.0f;
	constexpr float PanelMaxHeight = 430.0f;
	constexpr float PanelHorizontalPadding = 10.0f;
	constexpr float PanelVerticalPadding = 8.0f;
	constexpr float BranchGap = 5.0f;
	constexpr float BranchWidth = 365.0f;
	constexpr float WideBranchWidth = (BranchWidth * 2.0f) + BranchGap;
	constexpr float RoleWidth = 116.0f;
	constexpr float RouteGap = 6.0f;
	constexpr float RouteWidth = 242.0f;
	constexpr float RouteRoleWidth = 82.0f;

	UTextBlock* MakeText(
		UWidgetTree& Tree,
		const FName Name,
		const EFMCodexPlayerUITextRole TextRole)
	{
		UTextBlock* Result = Tree.ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(), Name);
		Result->SetAutoWrapText(true);
		Result->SetClipping(EWidgetClipping::ClipToBounds);
		FFMCodexPlayerUIStyle::Get().ApplyText(*Result, TextRole);
		return Result;
	}

	void AddSpaced(UVerticalBox& Parent, UWidget* Child, const float Top = 4.0f)
	{
		if (UVerticalBoxSlot* Slot = Parent.AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(FMargin(0.0f, Top, 0.0f, 0.0f));
		}
	}

}

void UFMCodexTacticalDetailPanelWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildWidgetTree();
	RefreshVisuals();
}

TSharedRef<SWidget> UFMCodexTacticalDetailPanelWidget::RebuildWidget()
{
	if (WidgetTree == nullptr)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}
	BuildWidgetTree();
	RefreshVisuals();
	return Super::RebuildWidget();
}

void UFMCodexTacticalDetailPanelWidget::NativeOnMouseEnter(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	OnDetailPointerEntered.Broadcast();
}

void UFMCodexTacticalDetailPanelWidget::NativeOnMouseLeave(
	const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	OnDetailPointerLeft.Broadcast();
}

void UFMCodexTacticalDetailPanelWidget::RefreshFromPresentation(
	const FFMCodexUMGTacticalDetailViewModel& InPresentation)
{
	Presentation = InPresentation;
	RefreshVisuals();
}

void UFMCodexTacticalDetailPanelWidget::ClearPresentation()
{
	Presentation = FFMCodexUMGTacticalDetailViewModel();
	RefreshVisuals();
}

const FFMCodexUMGTacticalDetailViewModel&
UFMCodexTacticalDetailPanelWidget::GetPresentation() const
{
	return Presentation;
}

FString UFMCodexTacticalDetailPanelWidget::CollectPlayerFacingText() const
{
	TArray<FString> Lines = { Presentation.DisplayName, Presentation.CardHint };
	for (const FFMCodexUMGTacticalBranchViewModel& Branch : Presentation.Branches)
	{
		Lines.AddUnique(Branch.PrimaryRouteLabel);
		Lines.AddUnique(Branch.RouteStageLabel);
		Lines.AddUnique(Branch.RouteStepLabel);
		Lines.AddUnique(Branch.Label);
		if (Branch.bRollOnly)
		{
			Lines.Add(TEXT("只看掷点，不看属性"));
		}
		for (const FFMCodexUMGTacticalRoleAttributeViewModel& Row
			: Branch.RoleAttributes)
		{
			Lines.Add(FString::Printf(TEXT("%s：%s"), *Row.RoleLabel,
				*Row.AttributeLabel));
		}
	}
	Lines.RemoveAll([](const FString& Value) { return Value.IsEmpty(); });
	return FString::Join(Lines, TEXT("\n"));
}

void UFMCodexTacticalDetailPanelWidget::BuildWidgetTree()
{
	using namespace FMCodexTacticalDetailPanelWidget;
	if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr)
	{
		return;
	}

	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(
		USizeBox::StaticClass(), TEXT("TacticalDetailBounds"));
	Bounds->SetWidthOverride(PanelWidth);
	Bounds->SetMaxDesiredHeight(PanelMaxHeight);
	WidgetTree->RootWidget = Bounds;

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(
		UBorder::StaticClass(), TEXT("TacticalDetailFrame"));
	Style.ApplyBorder(*Frame, EFMCodexPlayerUIColorRole::PanelBackground,
		FMargin(PanelHorizontalPadding, PanelVerticalPadding));
	Bounds->AddChild(Frame);

	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(
		UVerticalBox::StaticClass(), TEXT("TacticalDetailHierarchy"));
	Frame->AddChild(Root);
	TitleText = MakeText(*WidgetTree, TEXT("TacticalDetailTitle"),
		EFMCodexPlayerUITextRole::ActionTitle);
	Root->AddChildToVerticalBox(TitleText);
	HintText = MakeText(*WidgetTree, TEXT("TacticalDetailHint"),
		EFMCodexPlayerUITextRole::Secondary);
	AddSpaced(*Root, HintText, 0.0f);

	BranchBody = WidgetTree->ConstructWidget<UWrapBox>(
		UWrapBox::StaticClass(), TEXT("TacticalDetailBranches"));
	BranchBody->SetInnerSlotPadding(FVector2D(BranchGap, BranchGap));
	BranchBody->SetExplicitWrapSize(true);
	BranchBody->SetWrapSize(PanelWidth - (PanelHorizontalPadding * 2.0f));
	BranchBody->SetHorizontalAlignment(HAlign_Center);
	AddSpaced(*Root, BranchBody, 3.0f);

	RouteBody = WidgetTree->ConstructWidget<UHorizontalBox>(
		UHorizontalBox::StaticClass(), TEXT("TacticalDetailRoutes"));
	RouteBody->SetVisibility(ESlateVisibility::Collapsed);
	AddSpaced(*Root, RouteBody, 6.0f);
}

void UFMCodexTacticalDetailPanelWidget::RefreshVisuals()
{
	using namespace FMCodexTacticalDetailPanelWidget;
	if (TitleText == nullptr)
	{
		return;
	}
	if (!Presentation.bValid)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	TitleText->SetText(FText::FromString(Presentation.DisplayName));
	HintText->SetText(FText::FromString(Presentation.CardHint));

	BranchBody->ClearChildren();
	RouteBody->ClearChildren();
	const bool bHasPrimaryRoutes = Presentation.Branches.ContainsByPredicate(
		[](const FFMCodexUMGTacticalBranchViewModel& Branch)
		{
			return !Branch.PrimaryRouteLabel.IsEmpty();
		});
	BranchBody->SetVisibility(bHasPrimaryRoutes
		? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	RouteBody->SetVisibility(bHasPrimaryRoutes
		? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bHasPrimaryRoutes)
	{
		TArray<FString> RouteLabels;
		for (const FFMCodexUMGTacticalBranchViewModel& Branch
			: Presentation.Branches)
		{
			RouteLabels.AddUnique(Branch.PrimaryRouteLabel);
		}
		for (int32 RouteIndex = 0; RouteIndex < RouteLabels.Num(); ++RouteIndex)
		{
			const FString& RouteLabel = RouteLabels[RouteIndex];
			USizeBox* RouteBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("TacticalDetailRouteBounds%d"), RouteIndex)));
			RouteBounds->SetWidthOverride(RouteWidth);
			UBorder* RouteFrame = WidgetTree->ConstructWidget<UBorder>(
				UBorder::StaticClass(), FName(*FString::Printf(
					TEXT("TacticalDetailRouteFrame%d"), RouteIndex)));
			FFMCodexPlayerUIStyle::Get().ApplyBorder(*RouteFrame,
				EFMCodexPlayerUIColorRole::PanelRaised, FMargin(6.0f, 5.0f));
			UVerticalBox* RouteContent =
				WidgetTree->ConstructWidget<UVerticalBox>(
					UVerticalBox::StaticClass(), FName(*FString::Printf(
						TEXT("TacticalDetailRouteContent%d"), RouteIndex)));
			UTextBlock* RouteHeading = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("TacticalDetailRouteTitle%d"),
					RouteIndex)), EFMCodexPlayerUITextRole::Identity);
			RouteHeading->SetAutoWrapText(false);
			RouteHeading->SetText(FText::FromString(RouteLabel));
			RouteContent->AddChildToVerticalBox(RouteHeading);

			int32 StepIndex = 0;
			FString LastStageLabel;
			for (int32 BranchIndex = 0;
				BranchIndex < Presentation.Branches.Num(); ++BranchIndex)
			{
				const FFMCodexUMGTacticalBranchViewModel& Branch =
					Presentation.Branches[BranchIndex];
				if (Branch.PrimaryRouteLabel != RouteLabel)
				{
					continue;
				}
				if (!Branch.RouteStageLabel.IsEmpty()
					&& Branch.RouteStageLabel != LastStageLabel)
				{
					UTextBlock* StageHeading = MakeText(*WidgetTree,
						FName(*FString::Printf(
							TEXT("TacticalDetailRouteStageTitle%d_%d"),
							RouteIndex, StepIndex)),
						EFMCodexPlayerUITextRole::Secondary);
					StageHeading->SetAutoWrapText(false);
					StageHeading->SetTextOverflowPolicy(
						ETextOverflowPolicy::Ellipsis);
					StageHeading->SetText(
						FText::FromString(Branch.RouteStageLabel));
					AddSpaced(*RouteContent, StageHeading, 5.0f);
					LastStageLabel = Branch.RouteStageLabel;
				}
				UBorder* StepFrame = WidgetTree->ConstructWidget<UBorder>(
					UBorder::StaticClass(), FName(*FString::Printf(
						TEXT("TacticalDetailRouteStepFrame%d_%d"),
						RouteIndex, StepIndex)));
				FFMCodexPlayerUIStyle::Get().ApplyBorder(*StepFrame,
					EFMCodexPlayerUIColorRole::PanelInset, FMargin(6.0f, 4.0f));
				UVerticalBox* StepContent =
					WidgetTree->ConstructWidget<UVerticalBox>(
						UVerticalBox::StaticClass(), FName(*FString::Printf(
							TEXT("TacticalDetailRouteStepContent%d_%d"),
							RouteIndex, StepIndex)));
				UTextBlock* StepHeading = MakeText(*WidgetTree,
					FName(*FString::Printf(
						TEXT("TacticalDetailRouteStepTitle%d_%d"),
						RouteIndex, StepIndex)),
					EFMCodexPlayerUITextRole::SectionHeading);
				StepHeading->SetText(FText::FromString(
					Branch.RouteStepLabel.IsEmpty()
						? Branch.Label : Branch.RouteStepLabel));
				StepContent->AddChildToVerticalBox(StepHeading);
				if (Branch.bRollOnly)
				{
					UTextBlock* RollOnly = MakeText(*WidgetTree,
						FName(*FString::Printf(
							TEXT("TacticalDetailRouteRollOnly%d_%d"),
							RouteIndex, StepIndex)),
						EFMCodexPlayerUITextRole::Body);
					RollOnly->SetText(FText::FromString(
						TEXT("只看掷点，不看属性")));
					AddSpaced(*StepContent, RollOnly, 2.0f);
				}
				for (int32 RowIndex = 0;
					RowIndex < Branch.RoleAttributes.Num(); ++RowIndex)
				{
					const FFMCodexUMGTacticalRoleAttributeViewModel& RoleAttribute =
						Branch.RoleAttributes[RowIndex];
					UHorizontalBox* Row =
						WidgetTree->ConstructWidget<UHorizontalBox>(
							UHorizontalBox::StaticClass(), FName(*FString::Printf(
								TEXT("TacticalDetailRouteRoleAttribute%d_%d_%d"),
								RouteIndex, StepIndex, RowIndex)));
					UTextBlock* Role = MakeText(*WidgetTree,
						FName(*FString::Printf(
							TEXT("TacticalDetailRouteRole%d_%d_%d"),
							RouteIndex, StepIndex, RowIndex)),
						EFMCodexPlayerUITextRole::Secondary);
					Role->SetAutoWrapText(false);
					Role->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
					Role->SetText(FText::FromString(RoleAttribute.RoleLabel));
					USizeBox* RoleBounds = WidgetTree->ConstructWidget<USizeBox>(
						USizeBox::StaticClass(), FName(*FString::Printf(
							TEXT("TacticalDetailRouteRoleBounds%d_%d_%d"),
							RouteIndex, StepIndex, RowIndex)));
					RoleBounds->SetWidthOverride(RouteRoleWidth);
					RoleBounds->AddChild(Role);
					Row->AddChildToHorizontalBox(RoleBounds);
					UTextBlock* Attribute = MakeText(*WidgetTree,
						FName(*FString::Printf(
							TEXT("TacticalDetailRouteAttribute%d_%d_%d"),
							RouteIndex, StepIndex, RowIndex)),
						EFMCodexPlayerUITextRole::SectionHeading);
					Attribute->SetAutoWrapText(false);
					Attribute->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
					Attribute->SetText(FText::FromString(
						RoleAttribute.AttributeLabel));
					if (UHorizontalBoxSlot* AttributeSlot =
						Row->AddChildToHorizontalBox(Attribute))
					{
						AttributeSlot->SetSize(
							FSlateChildSize(ESlateSizeRule::Fill));
						AttributeSlot->SetVerticalAlignment(VAlign_Center);
					}
					AddSpaced(*StepContent, Row, 1.0f);
				}
				StepFrame->AddChild(StepContent);
				AddSpaced(*RouteContent, StepFrame, 4.0f);
				++StepIndex;
			}
			RouteFrame->AddChild(RouteContent);
			RouteBounds->AddChild(RouteFrame);
			if (UHorizontalBoxSlot* RouteSlot =
				RouteBody->AddChildToHorizontalBox(RouteBounds))
			{
				RouteSlot->SetPadding(FMargin(
					RouteIndex == 0 ? 0.0f : RouteGap, 0.0f, 0.0f, 0.0f));
				RouteSlot->SetVerticalAlignment(VAlign_Top);
			}
		}
		SetVisibility(ESlateVisibility::Visible);
		return;
	}

	for (int32 Index = 0; Index < Presentation.Branches.Num(); ++Index)
	{
		const FFMCodexUMGTacticalBranchViewModel& Branch =
			Presentation.Branches[Index];
		UBorder* BranchFrame = WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(), FName(*FString::Printf(
				TEXT("TacticalDetailBranchFrame%d"), Index)));
		FFMCodexPlayerUIStyle::Get().ApplyBorder(*BranchFrame,
			EFMCodexPlayerUIColorRole::PanelInset, FMargin(8.0f, 5.0f));
		USizeBox* BranchBounds = WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(), FName(*FString::Printf(
				TEXT("TacticalDetailBranchBounds%d"), Index)));
		const bool bWideFinalBranch = Presentation.Branches.Num() == 3
			&& Index == 2;
		BranchBounds->SetWidthOverride(
			bWideFinalBranch ? WideBranchWidth : BranchWidth);
		UVerticalBox* BranchContent = WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(), FName(*FString::Printf(
				TEXT("TacticalDetailBranchContent%d"), Index)));
		UTextBlock* Heading = MakeText(*WidgetTree,
			FName(*FString::Printf(TEXT("TacticalDetailBranchTitle%d"), Index)),
			EFMCodexPlayerUITextRole::SectionHeading);
		Heading->SetText(FText::FromString(Branch.Label));
		BranchContent->AddChildToVerticalBox(Heading);
		if (Branch.bRollOnly)
		{
			UTextBlock* Row = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("TacticalDetailRollOnly%d"), Index)),
				EFMCodexPlayerUITextRole::Body);
			Row->SetText(FText::FromString(TEXT("只看掷点，不看属性")));
			AddSpaced(*BranchContent, Row, 3.0f);
		}
		for (int32 RowIndex = 0; RowIndex < Branch.RoleAttributes.Num(); ++RowIndex)
		{
			const FFMCodexUMGTacticalRoleAttributeViewModel& RoleAttribute =
				Branch.RoleAttributes[RowIndex];
			UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(
				UHorizontalBox::StaticClass(), FName(*FString::Printf(
					TEXT("TacticalDetailRoleAttribute%d_%d"), Index, RowIndex)));
			UTextBlock* Role = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("TacticalDetailRole%d_%d"),
					Index, RowIndex)), EFMCodexPlayerUITextRole::Secondary);
			Role->SetAutoWrapText(false);
			Role->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
			Role->SetText(FText::FromString(RoleAttribute.RoleLabel));
			USizeBox* RoleBounds = WidgetTree->ConstructWidget<USizeBox>(
				USizeBox::StaticClass(), FName(*FString::Printf(
					TEXT("TacticalDetailRoleBounds%d_%d"), Index, RowIndex)));
			RoleBounds->SetWidthOverride(RoleWidth);
			RoleBounds->AddChild(Role);
			if (UHorizontalBoxSlot* RoleSlot =
				Row->AddChildToHorizontalBox(RoleBounds))
			{
				RoleSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				RoleSlot->SetVerticalAlignment(VAlign_Center);
			}
			UTextBlock* Attribute = MakeText(*WidgetTree,
				FName(*FString::Printf(TEXT("TacticalDetailAttribute%d_%d"),
					Index, RowIndex)), EFMCodexPlayerUITextRole::SectionHeading);
			Attribute->SetAutoWrapText(false);
			Attribute->SetTextOverflowPolicy(ETextOverflowPolicy::Ellipsis);
			Attribute->SetJustification(ETextJustify::Left);
			Attribute->SetText(FText::FromString(RoleAttribute.AttributeLabel));
			if (UHorizontalBoxSlot* AttributeSlot =
				Row->AddChildToHorizontalBox(Attribute))
			{
				AttributeSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				AttributeSlot->SetHorizontalAlignment(HAlign_Left);
				AttributeSlot->SetVerticalAlignment(VAlign_Center);
			}
			AddSpaced(*BranchContent, Row, 1.0f);
		}
		BranchFrame->AddChild(BranchContent);
		BranchBounds->AddChild(BranchFrame);
		if (UWrapBoxSlot* BranchSlot =
			BranchBody->AddChildToWrapBox(BranchBounds))
		{
			BranchSlot->SetVerticalAlignment(VAlign_Top);
		}
	}
	SetVisibility(ESlateVisibility::Visible);
}
