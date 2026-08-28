#include "FMCodexTacticalDetailPanelWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexTacticalDetailPresentation.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"

namespace FMCodexTacticalInformationTests
{
	const FFMCodexUMGTacticalBranchViewModel* FindBranch(
		const FFMCodexUMGTacticalDetailViewModel& Detail,
		const TCHAR* Label)
	{
		return Detail.Branches.FindByPredicate(
			[Label](const FFMCodexUMGTacticalBranchViewModel& Branch)
			{
				return Branch.Label == Label;
			});
	}

	const FFMCodexUMGTacticalBranchViewModel* FindBranchInRoute(
		const FFMCodexUMGTacticalDetailViewModel& Detail,
		const TCHAR* Label,
		const TCHAR* RouteLabel)
	{
		return Detail.Branches.FindByPredicate(
			[Label, RouteLabel](
				const FFMCodexUMGTacticalBranchViewModel& Branch)
			{
				return Branch.Label == Label
					&& Branch.PrimaryRouteLabel == RouteLabel;
			});
	}

	bool HasRoleAttribute(
		const FFMCodexUMGTacticalBranchViewModel* Branch,
		const EMatchPlayResolutionParticipantRole Role,
		const EMatchPlayResolutionFormulaAttribute Attribute,
		const bool bOptional = false)
	{
		return Branch != nullptr && Branch->RoleAttributes.ContainsByPredicate(
			[Role, Attribute, bOptional](
				const FFMCodexUMGTacticalRoleAttributeViewModel& Row)
			{
				return Row.Role == Role && Row.Attributes.Contains(Attribute)
					&& Row.bOptional == bOptional;
			});
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalInformationPresentationTest,
	"FMCodex.LocalPlay.TacticalInformation.01.FiveTacticLocalizationAndStructure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalInformationPresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexTacticalInformationTests;
	using EAttribute = EMatchPlayResolutionFormulaAttribute;
	using ERole = EMatchPlayResolutionParticipantRole;
	struct FExpectation
	{
		ESkillRuleType SkillType;
		const TCHAR* Name;
		int32 BranchCount;
	};
	const TArray<FExpectation> Expectations = {
		{ ESkillRuleType::LongShot, TEXT("远射"), 2 },
		{ ESkillRuleType::CutInsideShot, TEXT("内切"), 2 },
		{ ESkillRuleType::PassControl, TEXT("控球推进"), 3 },
		{ ESkillRuleType::Cross, TEXT("传中"), 2 },
		{ ESkillRuleType::ThroughBall, TEXT("直塞"), 7 }
	};
	for (const FExpectation& Expectation : Expectations)
	{
		const FFMCodexUMGTacticalDetailViewModel Detail =
			FFMCodexTacticalDetailPresentationBuilder::Build(
				Expectation.SkillType);
		TestTrue(TEXT("Description projection succeeds"), Detail.bValid);
		TestEqual(TEXT("Chinese tactical name"), Detail.DisplayName,
			FString(Expectation.Name));
		TestEqual(TEXT("Canonical branch count"), Detail.Branches.Num(),
			Expectation.BranchCount);
		const FString Combined = Detail.DisplayName + Detail.CardHint;
		TestFalse(TEXT("No raw canonical skill path"),
			Combined.Contains(TEXT("Canonical.Skill")));
	}

	const auto LongShot = FFMCodexTacticalDetailPresentationBuilder::Build(
		ESkillRuleType::LongShot);
	const auto* LongDirect = FindBranch(LongShot, TEXT("直接射门"));
	const auto* LongDead = FindBranch(LongShot, TEXT("直射死角"));
	TestTrue(TEXT("Long Shot direct compact attributes and roll-only dead corner"),
		HasRoleAttribute(LongDirect, ERole::Carrier, EAttribute::LongShot)
			&& HasRoleAttribute(LongDirect, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(LongDirect, ERole::Goalkeeper,
				EAttribute::GoalkeeperPositioning)
			&& LongDead != nullptr && LongDead->bRollOnly
			&& LongDead->RoleAttributes.IsEmpty());

	const auto CutInside = FFMCodexTacticalDetailPresentationBuilder::Build(
		ESkillRuleType::CutInsideShot);
	const auto* CutDirect = FindBranch(CutInside, TEXT("直接射门"));
	const auto* CutDead = FindBranch(CutInside, TEXT("直射死角"));
	TestTrue(TEXT("Cut Inside direct compact attributes and roll-only dead corner"),
		HasRoleAttribute(CutDirect, ERole::Carrier, EAttribute::Shooting)
			&& HasRoleAttribute(CutDirect, ERole::Carrier, EAttribute::Dribbling)
			&& HasRoleAttribute(CutDirect, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(CutDirect, ERole::Goalkeeper,
				EAttribute::GoalkeeperHandling)
			&& CutDead != nullptr && CutDead->bRollOnly
			&& CutDead->RoleAttributes.IsEmpty());

	const auto PassControl = FFMCodexTacticalDetailPresentationBuilder::Build(
		ESkillRuleType::PassControl);
	const auto* Pass = FindBranch(PassControl, TEXT("传球推进"));
	const auto* Dribble = FindBranch(PassControl, TEXT("盘带推进"));
	const auto* Run = FindBranch(PassControl, TEXT("跑动推进"));
	TestTrue(TEXT("Pass Control branches retain distinct canonical attributes"),
		HasRoleAttribute(Pass, ERole::Carrier, EAttribute::Passing)
			&& HasRoleAttribute(Pass, ERole::Runner, EAttribute::Passing)
			&& HasRoleAttribute(Pass, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(Pass, ERole::Helper, EAttribute::Marking, true)
			&& HasRoleAttribute(Pass, ERole::Goalkeeper,
				EAttribute::GoalkeeperHandling)
			&& HasRoleAttribute(Dribble, ERole::Carrier, EAttribute::Dribbling)
			&& HasRoleAttribute(Dribble, ERole::Runner, EAttribute::Passing)
			&& HasRoleAttribute(Dribble, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(Run, ERole::Carrier, EAttribute::OffBall)
			&& HasRoleAttribute(Run, ERole::Runner, EAttribute::Dribbling)
			&& HasRoleAttribute(Run, ERole::Marker, EAttribute::Marking));

	const auto Cross = FFMCodexTacticalDetailPresentationBuilder::Build(
		ESkillRuleType::Cross);
	const auto* High = FindBranch(Cross, TEXT("高球传中"));
	const auto* Low = FindBranch(Cross, TEXT("低球传中"));
	TestTrue(TEXT("Cross High compact role attributes are canonical"),
		HasRoleAttribute(High, ERole::Carrier, EAttribute::Passing)
			&& HasRoleAttribute(High, ERole::Runner, EAttribute::Strength)
			&& HasRoleAttribute(High, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(High, ERole::Helper, EAttribute::Strength, true)
			&& HasRoleAttribute(High, ERole::Goalkeeper,
				EAttribute::GoalkeeperAerial));
	TestTrue(TEXT("Cross Low compact role attributes are canonical"),
		HasRoleAttribute(Low, ERole::Carrier, EAttribute::Passing)
			&& HasRoleAttribute(Low, ERole::Runner, EAttribute::Shooting)
			&& HasRoleAttribute(Low, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(Low, ERole::Helper, EAttribute::Marking, true)
			&& HasRoleAttribute(Low, ERole::Goalkeeper,
				EAttribute::GoalkeeperReflex));

	const auto ThroughBall = FFMCodexTacticalDetailPresentationBuilder::Build(
		ESkillRuleType::ThroughBall);
	const auto* Feet = FindBranchInRoute(
		ThroughBall, TEXT("脚下球"), TEXT("脚下球"));
	const auto* Behind = FindBranchInRoute(
		ThroughBall, TEXT("第一阶段"), TEXT("身后球"));
	const auto* Anti = FindBranchInRoute(
		ThroughBall, TEXT("越位判定"), TEXT("反越位"));
	const auto* BehindDirect = FindBranchInRoute(
		ThroughBall, TEXT("直接射门"), TEXT("身后球"));
	const auto* AntiDirect = FindBranchInRoute(
		ThroughBall, TEXT("直接射门"), TEXT("反越位"));
	const auto* BehindChip = FindBranchInRoute(
		ThroughBall, TEXT("挑射"), TEXT("身后球"));
	const auto* AntiChip = FindBranchInRoute(
		ThroughBall, TEXT("挑射"), TEXT("反越位"));
	TestTrue(TEXT("Through Ball attribute branches stay compact and distinct"),
		HasRoleAttribute(Feet, ERole::Carrier, EAttribute::Passing)
			&& HasRoleAttribute(Feet, ERole::Runner, EAttribute::OffBall)
			&& HasRoleAttribute(Feet, ERole::Marker, EAttribute::Tackling)
			&& HasRoleAttribute(Feet, ERole::Helper, EAttribute::Marking, true)
			&& HasRoleAttribute(Feet, ERole::Goalkeeper,
				EAttribute::GoalkeeperOneOnOne)
			&& HasRoleAttribute(Behind, ERole::Runner, EAttribute::Speed)
			&& HasRoleAttribute(Behind, ERole::Marker, EAttribute::Marking)
			&& HasRoleAttribute(Behind, ERole::Helper, EAttribute::Speed, true)
			&& HasRoleAttribute(BehindDirect, ERole::Runner, EAttribute::Shooting)
			&& HasRoleAttribute(BehindDirect, ERole::Goalkeeper,
				EAttribute::GoalkeeperOneOnOne)
			&& HasRoleAttribute(AntiDirect, ERole::Runner, EAttribute::Shooting)
			&& HasRoleAttribute(AntiDirect, ERole::Goalkeeper,
				EAttribute::GoalkeeperOneOnOne));
	TestTrue(TEXT("Through Ball outcome segments are roll-only, not fake formula"),
		Anti != nullptr && Anti->bRollOnly
			&& Anti->RoleAttributes.IsEmpty()
			&& BehindChip != nullptr && BehindChip->bRollOnly
			&& BehindChip->RoleAttributes.IsEmpty()
			&& AntiChip != nullptr && AntiChip->bRollOnly
			&& AntiChip->RoleAttributes.IsEmpty());
	TestTrue(TEXT("Through Ball projects three primary routes with owned steps"),
		ThroughBall.CardHint == TEXT("脚下球 · 身后球 · 反越位")
			&& Feet != nullptr
			&& Feet->PrimaryRouteLabel == TEXT("脚下球")
			&& Feet->RouteStepLabel == TEXT("属性对抗")
			&& Behind != nullptr
			&& Behind->PrimaryRouteLabel == TEXT("身后球")
			&& Behind->RouteStepLabel == TEXT("第一阶段")
			&& Anti != nullptr
			&& Anti->PrimaryRouteLabel == TEXT("反越位")
			&& Anti->RouteStepLabel == TEXT("越位判定")
			&& BehindDirect != nullptr
			&& BehindDirect->RouteStepLabel == TEXT("直接射门")
			&& BehindDirect->RouteStageLabel == TEXT("成功后：单刀")
			&& BehindChip != nullptr
			&& BehindChip->RouteStepLabel == TEXT("挑射")
			&& BehindChip->RouteStageLabel == TEXT("成功后：单刀")
			&& AntiDirect != nullptr
			&& AntiDirect->RouteStepLabel == TEXT("直接射门")
			&& AntiDirect->RouteStageLabel == TEXT("成功后：单刀")
			&& AntiChip != nullptr
			&& AntiChip->RouteStepLabel == TEXT("挑射")
			&& AntiChip->RouteStageLabel == TEXT("成功后：单刀"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalInformationHoverLifecycleTest,
	"FMCodex.LocalPlay.TacticalInformation.02.HoverLifecycleAndClickRegression",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalInformationHoverLifecycleTest::RunTest(
	const FString& Parameters)
{
	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>();
	if (Screen == nullptr)
	{
		AddError(TEXT("Screen allocation failed"));
		return false;
	}
	Screen->TakeWidget();
	FFMCodexUMGMatchScreenViewModel View;
	View.Interaction.Category = EFMCodexUMGInteractionCategory::SelectSkill;
	View.Interaction.TitleLabel = TEXT("Choose Skill");
	FFMCodexUMGSelectionChoiceViewModel CrossChoice;
	CrossChoice.OptionId = TEXT("Canonical.Skill.Cross.4.6");
	CrossChoice.Label = TEXT("传中");
	CrossChoice.SkillType = ESkillRuleType::Cross;
	CrossChoice.SecondaryLabel = TEXT("高球 / 低球");
	View.Interaction.SelectionChoices.Add(CrossChoice);
	FFMCodexUMGSelectionChoiceViewModel CutChoice;
	CutChoice.OptionId = TEXT("Canonical.Skill.CutInsideShot.4.5");
	CutChoice.Label = TEXT("内切");
	CutChoice.SkillType = ESkillRuleType::CutInsideShot;
	CutChoice.SecondaryLabel = TEXT("射门 · 盘带");
	View.Interaction.SelectionChoices.Add(CutChoice);
	Screen->RefreshFromPresentation(View);

	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	UFMCodexTacticalDetailPanelWidget* Detail = Screen->GetTacticalDetailPanel();
	if (Interaction == nullptr || Detail == nullptr
		|| Interaction->GetRenderedOptionWidgets().Num() != 2)
	{
		AddError(TEXT("Tactical UI did not build two choices and one shared detail"));
		return false;
	}
	UFMCodexInteractionOptionWidget* CrossCard =
		Interaction->GetRenderedOptionWidgets()[0];
	UFMCodexInteractionOptionWidget* CutCard =
		Interaction->GetRenderedOptionWidgets()[1];
	TestTrue(TEXT("Compact tactical cards have primary and secondary content"),
		CrossCard->IsTacticalCard() && CutCard->IsTacticalCard()
			&& CrossCard->GetLabel() == TEXT("传中")
			&& CrossCard->GetSecondaryLabel() == TEXT("高球 / 低球")
			&& !CrossCard->GetLabel().Contains(TEXT("Canonical.Skill")));
	TestEqual(TEXT("Shared detail begins hidden"), Detail->GetVisibility(),
		ESlateVisibility::Collapsed);

	CrossCard->OnTacticalDetailRequested.Broadcast(CrossChoice.OptionId);
	const FString CrossPlayerText = Detail->CollectPlayerFacingText();
	TestTrue(TEXT("Hover/focus Cross publishes Cross detail"),
		Detail->GetVisibility() == ESlateVisibility::Visible
			&& Detail->GetPresentation().SkillType == ESkillRuleType::Cross
			&& CrossPlayerText.Contains(TEXT("高球传中"))
			&& CrossPlayerText.Contains(TEXT("持球：传球"))
			&& CrossPlayerText.Contains(TEXT("跑位球员：力量"))
			&& CrossPlayerText.Contains(TEXT("协防：力量"))
			&& !CrossPlayerText.Contains(TEXT("（可选）"))
			&& CrossPlayerText.Contains(TEXT("门将：制空")));
	TestTrue(TEXT("Compact player surface excludes formula/manual density"),
		!CrossPlayerText.Contains(TEXT("进攻"))
			&& !CrossPlayerText.Contains(TEXT("防守"))
			&& !CrossPlayerText.Contains(TEXT("战术球员"))
			&& !CrossPlayerText.Contains(TEXT("×"))
			&& !CrossPlayerText.Contains(TEXT("固定"))
			&& !CrossPlayerText.Contains(TEXT("+2"))
			&& !CrossPlayerText.Contains(TEXT("掷点 1"))
			&& !CrossPlayerText.Contains(TEXT("  +  ")));
	USizeBox* DetailBounds = Cast<USizeBox>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailBounds")));
	UWrapBox* DetailBranches = Cast<UWrapBox>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailBranches")));
	USizeBox* CrossFirstBranchBounds = Cast<USizeBox>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailBranchBounds0")));
	USizeBox* OptionalRoleBounds = Cast<USizeBox>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailRoleBounds0_3")));
	UTextBlock* OptionalRole = Cast<UTextBlock>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailRole0_3")));
	UTextBlock* OptionalAttribute = Cast<UTextBlock>(
		Detail->GetWidgetFromName(TEXT("TacticalDetailAttribute0_3")));
	UHorizontalBoxSlot* OptionalAttributeSlot = OptionalAttribute != nullptr
		? Cast<UHorizontalBoxSlot>(OptionalAttribute->Slot)
		: nullptr;
	TestTrue(TEXT("Panel uses a compact centered two-column footprint"),
		DetailBounds != nullptr
			&& DetailBounds->GetWidthOverride() >= 760.0f
			&& DetailBounds->GetWidthOverride() <= 800.0f
			&& DetailBranches != nullptr
			&& DetailBranches->GetChildrenCount() == 2
			&& DetailBranches->UseExplicitWrapSize()
			&& DetailBranches->GetHorizontalAlignment() == HAlign_Center
			&& DetailBranches->GetInnerSlotPadding().X <= 5.0f
			&& CrossFirstBranchBounds != nullptr
			&& CrossFirstBranchBounds->GetWidthOverride() >= 350.0f
			&& CrossFirstBranchBounds->GetWidthOverride() <= 380.0f
			&& OptionalRoleBounds != nullptr
			&& OptionalRoleBounds->GetWidthOverride() >= 116.0f
			&& OptionalRole != nullptr && !OptionalRole->GetAutoWrapText()
			&& OptionalRole->GetText().ToString() == TEXT("协防")
			&& OptionalAttribute != nullptr
			&& !OptionalAttribute->GetAutoWrapText()
			&& OptionalAttributeSlot != nullptr
			&& OptionalAttributeSlot->GetHorizontalAlignment() == HAlign_Left
			&& OptionalAttribute->GetText().ToString() == TEXT("力量")
			&& Detail->GetWidgetFromName(TEXT("TacticalDetailBranchScroll"))
				== nullptr);

	UFMCodexTacticalDetailPanelWidget* ThreeBranchPanel =
		NewObject<UFMCodexTacticalDetailPanelWidget>();
	ThreeBranchPanel->TakeWidget();
	ThreeBranchPanel->RefreshFromPresentation(
		FFMCodexTacticalDetailPresentationBuilder::Build(
			ESkillRuleType::PassControl));
	USizeBox* WideFinalBranch = Cast<USizeBox>(
		ThreeBranchPanel->GetWidgetFromName(
			TEXT("TacticalDetailBranchBounds2")));
	TestTrue(TEXT("Three-branch layout uses 2+1 without a narrow orphan card"),
		WideFinalBranch != nullptr
			&& CrossFirstBranchBounds != nullptr
			&& FMath::IsNearlyEqual(WideFinalBranch->GetWidthOverride(),
				(CrossFirstBranchBounds->GetWidthOverride() * 2.0f)
					+ DetailBranches->GetInnerSlotPadding().X)
			&& ThreeBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailBranchScroll")) == nullptr);

	UFMCodexTacticalDetailPanelWidget* SixBranchPanel =
		NewObject<UFMCodexTacticalDetailPanelWidget>();
	SixBranchPanel->TakeWidget();
	SixBranchPanel->RefreshFromPresentation(
		FFMCodexTacticalDetailPresentationBuilder::Build(
			ESkillRuleType::ThroughBall));
	UHorizontalBox* ThroughBallRoutes = Cast<UHorizontalBox>(
		SixBranchPanel->GetWidgetFromName(TEXT("TacticalDetailRoutes")));
	UWrapBox* SixBranchBody = Cast<UWrapBox>(
		SixBranchPanel->GetWidgetFromName(TEXT("TacticalDetailBranches")));
	UTextBlock* FeetRouteTitle = Cast<UTextBlock>(
		SixBranchPanel->GetWidgetFromName(TEXT("TacticalDetailRouteTitle0")));
	UTextBlock* BehindRouteTitle = Cast<UTextBlock>(
		SixBranchPanel->GetWidgetFromName(TEXT("TacticalDetailRouteTitle1")));
	UTextBlock* AntiRouteTitle = Cast<UTextBlock>(
		SixBranchPanel->GetWidgetFromName(TEXT("TacticalDetailRouteTitle2")));
	TestTrue(TEXT("Through Ball renders three peer route groups with nested steps"),
		ThroughBallRoutes != nullptr
			&& ThroughBallRoutes->GetVisibility() == ESlateVisibility::Visible
			&& ThroughBallRoutes->GetChildrenCount() == 3
			&& SixBranchBody != nullptr
			&& SixBranchBody->GetVisibility() == ESlateVisibility::Collapsed
			&& FeetRouteTitle != nullptr
			&& FeetRouteTitle->GetText().ToString() == TEXT("脚下球")
			&& BehindRouteTitle != nullptr
			&& BehindRouteTitle->GetText().ToString() == TEXT("身后球")
			&& AntiRouteTitle != nullptr
			&& AntiRouteTitle->GetText().ToString() == TEXT("反越位")
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailRouteStepTitle0_0")) != nullptr
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailRouteStepTitle1_2")) != nullptr
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailRouteStepTitle2_2")) != nullptr
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailRouteStageTitle1_1")) != nullptr
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailRouteStageTitle2_1")) != nullptr
			&& SixBranchPanel->GetWidgetFromName(
				TEXT("TacticalDetailBranchScroll")) == nullptr);
	CutCard->OnTacticalDetailRequested.Broadcast(CutChoice.OptionId);
	TestTrue(TEXT("Moving to another card replaces rather than stacks detail"),
		Detail->GetVisibility() == ESlateVisibility::Visible
			&& Detail->GetPresentation().SkillType
				== ESkillRuleType::CutInsideShot
			&& !Detail->CollectPlayerFacingText().Contains(TEXT("高球传中")));
	TestTrue(TEXT("Hover projection does not mutate authoritative presentation"),
		Screen->GetMatchController() == nullptr
			&& Screen->GetPresentation().Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectSkill
			&& Screen->GetPresentation().Interaction.SelectionChoices.Num() == 2);

	// The same existing Card intent remains the click path. With no controller,
	// the request is inert, but the shared detail still closes immediately.
	CutCard->OnCardRequested.Broadcast(CutChoice.OptionId);
	TestEqual(TEXT("Click exits detail without a confirmation surface"),
		Detail->GetVisibility(), ESlateVisibility::Collapsed);

	CrossCard->OnTacticalDetailRequested.Broadcast(CrossChoice.OptionId);
	FFMCodexUMGMatchScreenViewModel ExitView = View;
	ExitView.Interaction.Category =
		EFMCodexUMGInteractionCategory::ContinueResolution;
	ExitView.Interaction.SelectionChoices.Reset();
	Screen->RefreshFromPresentation(ExitView);
	TestEqual(TEXT("Leaving tactical selection clears stale detail"),
		Detail->GetVisibility(), ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexTacticalInformationAuthorityBoundaryTest,
	"FMCodex.LocalPlay.TacticalInformation.03.AuthorityAndSourceBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexTacticalInformationAuthorityBoundaryTest::RunTest(
	const FString& Parameters)
{
	FString CatalogSource;
	FString PresentationSource;
	FString OptionSource;
	FString PanelSource;
	FString MatchScreenSource;
	const FString Root = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	TestTrue(TEXT("Catalog source loads"), FFileHelper::LoadFileToString(
		CatalogSource, *(Root + TEXT("Source/FMCodex/CoreRules/TacticalRuleDescription.cpp"))));
	TestTrue(TEXT("Presentation source loads"), FFileHelper::LoadFileToString(
		PresentationSource,
		*(Root + TEXT("Source/FMCodex/LocalPlay/FMCodexTacticalDetailPresentation.cpp"))));
	TestTrue(TEXT("Option source loads"), FFileHelper::LoadFileToString(
		OptionSource,
		*(Root + TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.cpp"))));
	TestTrue(TEXT("Panel source loads"), FFileHelper::LoadFileToString(
		PanelSource,
		*(Root + TEXT("Source/FMCodex/LocalPlay/FMCodexTacticalDetailPanelWidget.cpp"))));
	TestTrue(TEXT("Match Screen source loads"), FFileHelper::LoadFileToString(
		MatchScreenSource,
		*(Root + TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"))));
	const FString Combined = CatalogSource + PresentationSource;
	const TArray<FString> Forbidden = {
		TEXT("RollD6("), TEXT("RandRange("), TEXT("FRandomStream"),
		TEXT("FormulaFacts"), TEXT("FMatchPlayState"), TEXT("SubmitSkill("),
		TEXT("SkillSelectionAvailability")
	};
	for (const FString& Token : Forbidden)
	{
		TestFalse(FString::Printf(TEXT("Read-only detail excludes %s"), *Token),
			Combined.Contains(Token));
	}
	TestTrue(TEXT("Focus and hover share the same presentation intent"),
		OptionSource.Contains(TEXT("NativeOnAddedToFocusPath"))
			&& OptionSource.Contains(TEXT("HandleTacticalHovered"))
			&& OptionSource.Contains(TEXT("OnTacticalDetailRequested.Broadcast")));
	TestTrue(TEXT("Compact DTO owns filtering and Widget has no tactical cases"),
		PresentationSource.Contains(TEXT("AddRoleAttribute"))
			&& PresentationSource.Contains(TEXT("bRollOnly"))
			&& !PanelSource.Contains(TEXT("ESkillRuleType::"))
			&& !PanelSource.Contains(TEXT("ArithmeticContest"))
			&& !PanelSource.Contains(TEXT("OutcomeDecision")));
	TestTrue(TEXT("Deployment reference reuses shared builder and detail panel"),
		MatchScreenSource.Contains(
			TEXT("FFMCodexTacticalDetailPresentationBuilder::Build(SkillType)"))
			&& MatchScreenSource.Contains(
				TEXT("TacticalDetailPanel->RefreshFromPresentation(Detail)"))
			&& !MatchScreenSource.Contains(
				TEXT("DeploymentTacticalDescriptionMap"))
			&& !MatchScreenSource.Contains(
				TEXT("EMatchPlayResolutionFormulaAttribute::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexDeploymentTacticalReferenceEntryTest,
	"FMCodex.LocalPlay.TacticalInformation.04.DeploymentReferenceEntry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexDeploymentTacticalReferenceEntryTest::RunTest(
	const FString& Parameters)
{
	UFMCodexLocalMatchScreenWidget* Screen =
		NewObject<UFMCodexLocalMatchScreenWidget>();
	if (Screen == nullptr)
	{
		AddError(TEXT("Deployment reference screen allocation failed"));
		return false;
	}
	Screen->TakeWidget();
	FFMCodexUMGMatchScreenViewModel DeploymentView;
	DeploymentView.Interaction.Category =
		EFMCodexUMGInteractionCategory::Deploy;
	DeploymentView.Interaction.bCanFinishDeployment = true;
	Screen->RefreshFromPresentation(DeploymentView);

	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	UFMCodexTacticalDetailPanelWidget* Detail =
		Screen->GetTacticalDetailPanel();
	UButton* EntryButton = Interaction != nullptr
		? Cast<UButton>(Interaction->GetWidgetFromName(
			TEXT("DeploymentTacticalReferenceEntryButton")))
		: nullptr;
	UBorder* Controls = Cast<UBorder>(Screen->GetWidgetFromName(
		TEXT("DeploymentTacticalReferenceControls")));
	UTextBlock* EntryLabel = EntryButton != nullptr
		? Cast<UTextBlock>(EntryButton->GetChildAt(0)) : nullptr;
	TestTrue(TEXT("Deployment exposes a local tactical reference entry"),
		Interaction != nullptr && EntryButton != nullptr
			&& EntryButton->GetVisibility() == ESlateVisibility::Visible
			&& EntryLabel != nullptr
			&& EntryLabel->GetText().ToString() == TEXT("战术说明")
			&& Controls != nullptr
			&& Controls->GetVisibility() == ESlateVisibility::Collapsed
			&& Detail != nullptr
			&& Detail->GetVisibility() == ESlateVisibility::Collapsed);

	EntryButton->OnClicked.Broadcast();
	TestTrue(TEXT("Entry opens the shared detail without gameplay state"),
		Screen->IsDeploymentTacticalReferenceOpen()
			&& Screen->GetDeploymentTacticalReferenceSkillType()
				== ESkillRuleType::LongShot
			&& Controls->GetVisibility() == ESlateVisibility::Visible
			&& Detail->GetVisibility() == ESlateVisibility::Visible
			&& Detail->GetPresentation().SkillType
				== ESkillRuleType::LongShot
			&& Screen->GetMatchController() == nullptr
			&& Screen->GetPresentation().Interaction.Category
				== EFMCodexUMGInteractionCategory::Deploy);

	struct FReferenceExpectation
	{
		const TCHAR* ButtonName;
		ESkillRuleType SkillType;
		const TCHAR* ChineseName;
	};
	const TArray<FReferenceExpectation> Expectations = {
		{ TEXT("DeploymentReferenceLongShotButton"),
			ESkillRuleType::LongShot, TEXT("远射") },
		{ TEXT("DeploymentReferenceCutInsideButton"),
			ESkillRuleType::CutInsideShot, TEXT("内切") },
		{ TEXT("DeploymentReferencePassControlButton"),
			ESkillRuleType::PassControl, TEXT("控球推进") },
		{ TEXT("DeploymentReferenceCrossButton"),
			ESkillRuleType::Cross, TEXT("传中") },
		{ TEXT("DeploymentReferenceThroughBallButton"),
			ESkillRuleType::ThroughBall, TEXT("直塞") }
	};
	UHorizontalBox* Selector = Cast<UHorizontalBox>(Screen->GetWidgetFromName(
		TEXT("DeploymentTacticalReferenceSelector")));
	TestTrue(TEXT("Reference selector owns five stable canonical choices"),
		Selector != nullptr && Selector->GetChildrenCount() == 8
			&& Selector->GetChildAt(6)->GetName()
				== TEXT("DeploymentReferenceCloseSpacer"));
	for (int32 Index = 0; Index < Expectations.Num(); ++Index)
	{
		const FReferenceExpectation& Expectation = Expectations[Index];
		UButton* Button = Cast<UButton>(
			Screen->GetWidgetFromName(Expectation.ButtonName));
		UTextBlock* Label = Button != nullptr
			? Cast<UTextBlock>(Button->GetChildAt(0)) : nullptr;
		TestTrue(FString::Printf(TEXT("Reference selector %s is Chinese"),
			Expectation.ChineseName),
			Button != nullptr && Label != nullptr
				&& Label->GetText().ToString() == Expectation.ChineseName
				&& !Label->GetAutoWrapText());
		TestEqual(TEXT("Reference selector order remains canonical"),
			Selector->GetChildAt(Index + 1)->GetName(),
			FString(Expectation.ButtonName) + TEXT("Bounds"));
		Screen->SelectDeploymentTacticalReference(Expectation.SkillType);
		TestTrue(FString::Printf(TEXT("Reference selects %s read-only detail"),
			Expectation.ChineseName),
			Detail->GetPresentation().SkillType == Expectation.SkillType
				&& Detail->GetPresentation().DisplayName
					== Expectation.ChineseName);
	}
	USizeBox* PassControlBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("DeploymentReferencePassControlButtonBounds")));
	TestTrue(TEXT("Pass Control selector owns a stable single-line width"),
		PassControlBounds != nullptr
			&& PassControlBounds->GetWidthOverride() >= 108.0f
			&& PassControlBounds->GetHeightOverride() >= 38.0f);
	const FString PlayerText = Detail->CollectPlayerFacingText();
	TestTrue(TEXT("Deployment reference retains compact player contract"),
		!PlayerText.Contains(TEXT("进攻"))
			&& !PlayerText.Contains(TEXT("防守"))
			&& !PlayerText.Contains(TEXT("战术球员"))
			&& !PlayerText.Contains(TEXT("×"))
			&& !PlayerText.Contains(TEXT("+2"))
			&& !PlayerText.Contains(TEXT("Canonical.Skill")));

	UButton* CloseButton = Cast<UButton>(Screen->GetWidgetFromName(
		TEXT("DeploymentReferenceCloseButton")));
	UTextBlock* CloseLabel = CloseButton != nullptr
		? Cast<UTextBlock>(CloseButton->GetChildAt(0)) : nullptr;
	TestTrue(TEXT("Reference has an explicit Chinese close action"),
		CloseButton != nullptr && CloseLabel != nullptr
			&& CloseLabel->GetText().ToString() == TEXT("关闭战术说明"));
	if (CloseButton != nullptr)
	{
		CloseButton->OnClicked.Broadcast();
	}
	TestTrue(TEXT("Close returns to unchanged deployment presentation"),
		!Screen->IsDeploymentTacticalReferenceOpen()
			&& Controls->GetVisibility() == ESlateVisibility::Collapsed
			&& Detail->GetVisibility() == ESlateVisibility::Collapsed
			&& Screen->GetPresentation().Interaction.Category
				== EFMCodexUMGInteractionCategory::Deploy
			&& Screen->GetPresentation().Interaction.bCanFinishDeployment
			&& !Interaction->IsInteractionBlocked());

	FFMCodexUMGMatchScreenViewModel SelectionView;
	SelectionView.Interaction.Category =
		EFMCodexUMGInteractionCategory::SelectCarrier;
	Screen->RefreshFromPresentation(SelectionView);
	TestTrue(TEXT("Reference entry is not a global non-deployment control"),
		EntryButton->GetVisibility() == ESlateVisibility::Collapsed
			&& !Screen->IsDeploymentTacticalReferenceOpen()
			&& Controls->GetVisibility() == ESlateVisibility::Collapsed);
	Interaction->RequestDeploymentTacticalReference();
	TestFalse(TEXT("Non-deployment presentation cannot open reference"),
		Screen->IsDeploymentTacticalReferenceOpen());
	return true;
}

#endif
