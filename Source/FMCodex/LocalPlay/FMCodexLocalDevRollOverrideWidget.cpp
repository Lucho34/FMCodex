#include "FMCodexLocalDevRollOverrideWidget.h"

#if !UE_BUILD_SHIPPING

#include "FMCodexLocalDevRollOverride.h"
#include "FMCodexLocalMatchPlayerController.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace FMCodexLocalDevRollOverrideWidget
{
	using ETarget = EFMCodexLocalDevRollTarget;

	const TArray<ETarget>& Targets()
	{
		static const TArray<ETarget> Values = {
			ETarget::TacticalPoint,
			ETarget::ThroughBallRoute,
			ETarget::ThroughBallBehindDefenseP1,
			ETarget::ThroughBallAntiOffside,
			ETarget::ThroughBallFeetAttack,
			ETarget::ThroughBallFeetDefense,
			ETarget::CrossRoute,
			ETarget::CrossHighAttack,
			ETarget::CrossHighDefense,
			ETarget::CrossLowAttack,
			ETarget::CrossLowDefense,
			ETarget::OneOnOneChipShotAttack,
			ETarget::OneOnOneDirectShotAttack,
			ETarget::OneOnOneDirectShotDefense
		};
		return Values;
	}

	FString Label(const ETarget Target)
	{
		switch (Target)
		{
		case ETarget::TacticalPoint: return TEXT("战术点");
		case ETarget::ThroughBallRoute: return TEXT("直塞路线");
		case ETarget::ThroughBallBehindDefenseP1: return TEXT("身后球 P1");
		case ETarget::ThroughBallAntiOffside: return TEXT("反越位");
		case ETarget::ThroughBallFeetAttack: return TEXT("脚下球·进攻");
		case ETarget::ThroughBallFeetDefense: return TEXT("脚下球·防守");
		case ETarget::CrossRoute: return TEXT("传中路线");
		case ETarget::CrossHighAttack: return TEXT("高球传中·进攻");
		case ETarget::CrossHighDefense: return TEXT("高球传中·防守");
		case ETarget::CrossLowAttack: return TEXT("低球传中·进攻");
		case ETarget::CrossLowDefense: return TEXT("低球传中·防守");
		case ETarget::OneOnOneChipShotAttack: return TEXT("单刀·挑射");
		case ETarget::OneOnOneDirectShotAttack: return TEXT("单刀·直接射门进攻");
		case ETarget::OneOnOneDirectShotDefense: return TEXT("单刀·直接射门防守");
		default: return TEXT("未选择");
		}
	}

	TSharedRef<SWidget> Button(
		const FText& Text,
		const FOnClicked& OnClicked)
	{
		return SNew(SButton)
			.ContentPadding(FMargin(7.0f, 3.0f))
			.OnClicked(OnClicked)
			[
				SNew(STextBlock).Text(Text)
			];
	}
}

void SFMCodexLocalDevRollOverrideWidget::Construct(
	const FArguments& InArgs)
{
	Controller = InArgs._Controller;
	using namespace FMCodexLocalDevRollOverrideWidget;
	ChildSlot
	[
		SNew(SBorder)
		.Padding(6.0f)
		.BorderBackgroundColor(FLinearColor(0.04f, 0.025f, 0.02f, 0.94f))
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("DEV 掷点")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FLinearColor(1.0f, 0.64f, 0.22f))
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
				[
					SNew(STextBlock)
					.Text(this, &SFMCodexLocalDevRollOverrideWidget::PendingOverridesText)
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.78f))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()[
						Button(FText::FromString(TEXT("<")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::SelectPreviousTarget))]
					+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(5.0f, 0.0f)[
						SNew(STextBlock).Text(this,
							&SFMCodexLocalDevRollOverrideWidget::SelectedTargetText)]
					+ SHorizontalBox::Slot().AutoWidth()[
						Button(FText::FromString(TEXT(">")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::SelectNextTarget))]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth()[
						Button(FText::FromString(TEXT("-")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::SelectPreviousValue))]
					+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center)[
						SNew(STextBlock).Text(this,
							&SFMCodexLocalDevRollOverrideWidget::SelectedValueText)]
					+ SHorizontalBox::Slot().AutoWidth()[
						Button(FText::FromString(TEXT("+")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::SelectNextValue))]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(1.0f)[
						Button(FText::FromString(TEXT("设置")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::SetSelectedOverride))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(1.0f)[
						Button(FText::FromString(TEXT("清除此项")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::ClearSelectedOverride))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(1.0f)[
						Button(FText::FromString(TEXT("全部清除")), FOnClicked::CreateSP(
							this, &SFMCodexLocalDevRollOverrideWidget::ClearAllOverrides))]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(2.0f)
				[
					SNew(STextBlock)
					.Text(this, &SFMCodexLocalDevRollOverrideWidget::LastCommandText)
					.ColorAndOpacity(FLinearColor(0.65f, 0.75f, 0.82f))
				]
			]
		]
	];
}

FReply SFMCodexLocalDevRollOverrideWidget::SelectPreviousTarget()
{
	const int32 Count = FMCodexLocalDevRollOverrideWidget::Targets().Num();
	SelectedTargetIndex = (SelectedTargetIndex + Count - 1) % Count;
	ClampSelectedValue();
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::SelectNextTarget()
{
	SelectedTargetIndex = (SelectedTargetIndex + 1)
		% FMCodexLocalDevRollOverrideWidget::Targets().Num();
	ClampSelectedValue();
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::SelectPreviousValue()
{
	SelectedValue = SelectedValue <= SelectedMinimum()
		? SelectedMaximum() : SelectedValue - 1;
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::SelectNextValue()
{
	SelectedValue = SelectedValue >= SelectedMaximum()
		? SelectedMinimum() : SelectedValue + 1;
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::SetSelectedOverride()
{
	if (!Controller.IsValid())
	{
		LastCommand = TEXT("Controller unavailable");
		return FReply::Handled();
	}
	FFMCodexLocalDevRollOverrideRequest Request;
	Request.Target = FMCodexLocalDevRollOverrideWidget::Targets()[SelectedTargetIndex];
	Request.Value = SelectedValue;
	const FFMCodexLocalDevRollOverrideCommandResult Result =
		Controller->SetLocalDevRollOverride(Request);
	LastCommand = Result.bSuccess ? TEXT("已设置；等待 Authority 消费")
		: Result.ErrorMessage;
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::ClearSelectedOverride()
{
	if (Controller.IsValid())
	{
		Controller->ClearLocalDevRollOverride(
			FMCodexLocalDevRollOverrideWidget::Targets()[SelectedTargetIndex]);
		LastCommand = TEXT("已清除此项");
	}
	return FReply::Handled();
}

FReply SFMCodexLocalDevRollOverrideWidget::ClearAllOverrides()
{
	if (Controller.IsValid())
	{
		Controller->ClearAllLocalDevRollOverrides();
		LastCommand = TEXT("已全部清除");
	}
	return FReply::Handled();
}

FText SFMCodexLocalDevRollOverrideWidget::SelectedTargetText() const
{
	return FText::FromString(FMCodexLocalDevRollOverrideWidget::Label(
		FMCodexLocalDevRollOverrideWidget::Targets()[SelectedTargetIndex]));
}

FText SFMCodexLocalDevRollOverrideWidget::SelectedValueText() const
{
	return FText::AsNumber(SelectedValue);
}

FText SFMCodexLocalDevRollOverrideWidget::PendingOverridesText() const
{
	if (!Controller.IsValid())
	{
		return FText::FromString(TEXT("待消费：Controller unavailable"));
	}
	const TArray<FFMCodexLocalDevPendingRollOverride> Pending =
		Controller->GetLocalDevPendingRollOverrides();
	if (Pending.IsEmpty())
	{
		return FText::FromString(TEXT("待消费：无"));
	}
	TArray<FString> Lines;
	for (const FFMCodexLocalDevPendingRollOverride& Item : Pending)
	{
		Lines.Add(FString::Printf(TEXT("%s → %d"),
			*FMCodexLocalDevRollOverrideWidget::Label(Item.Target), Item.Value));
	}
	return FText::FromString(TEXT("待消费：\n") + FString::Join(Lines, TEXT("\n")));
}

FText SFMCodexLocalDevRollOverrideWidget::LastCommandText() const
{
	return FText::FromString(LastCommand);
}

int32 SFMCodexLocalDevRollOverrideWidget::SelectedMinimum() const
{
	return FMCodexLocalDevRollOverrideWidget::Targets()[SelectedTargetIndex]
		== EFMCodexLocalDevRollTarget::TacticalPoint ? 2 : 1;
}

int32 SFMCodexLocalDevRollOverrideWidget::SelectedMaximum() const
{
	return FMCodexLocalDevRollOverrideWidget::Targets()[SelectedTargetIndex]
		== EFMCodexLocalDevRollTarget::TacticalPoint ? 8 : 6;
}

void SFMCodexLocalDevRollOverrideWidget::ClampSelectedValue()
{
	SelectedValue = FMath::Clamp(
		SelectedValue, SelectedMinimum(), SelectedMaximum());
}

#endif
