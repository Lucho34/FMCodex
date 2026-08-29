#include "FMCodexLocalMatchUMGPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Misc/AutomationTest.h"

namespace FMCodexTacticalBranchAlignmentPresentationTests
{
	FFMCodexLocalMatchInteractionView MakeCrossView(
		const EFMCodexLocalMatchInteractionCategory Category)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.AttackSequence = 7;
		View.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
		View.PresentedActionType = ESkillRuleType::Cross;
		View.ActionLabel = TEXT("Cross");
		View.InteractionCategory = Category;
		View.bHumanInteraction = true;
		return View;
	}

	FFMCodexUMGMatchScreenViewModel Build(
		const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString());
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCrossCentralBranchChoiceAlignmentTest,
	"FMCodex.LocalPlay.TacticalBranchAlignment.01.CrossCentralChoice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCrossCentralBranchChoiceAlignmentTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexTacticalBranchAlignmentPresentationTests;
	(void)Parameters;
	FFMCodexLocalMatchInteractionView View = MakeCrossView(
		EFMCodexLocalMatchInteractionCategory::SelectBranchIntent);
	View.BranchIntentOptions = {
		EMatchPlayElectiveBranchIntent::CrossHigh,
		EMatchPlayElectiveBranchIntent::CrossLow
	};

	const FFMCodexUMGMatchScreenViewModel Presentation = Build(View);
	const FFMCodexUMGLongShotResolutionViewModel& SurfaceModel =
		Presentation.LongShotResolution;
	TestTrue(TEXT("Cross method choice uses the central production shell"),
		SurfaceModel.bVisible && SurfaceModel.bSuppressLegacyResolution);
	TestEqual(TEXT("Cross family identity remains explicit"),
		SurfaceModel.SkillType, ESkillRuleType::Cross);
	TestEqual(TEXT("Cross title is Chinese-first"),
		SurfaceModel.TitleLabel, FString(TEXT("传中")));
	TestEqual(TEXT("Cross method stage is player-facing"),
		SurfaceModel.StageLabel, FString(TEXT("选择传中方式")));
	TestEqual(TEXT("Both authority-projected methods are central"),
		SurfaceModel.BranchChoices.Num(), 2);
	if (SurfaceModel.BranchChoices.Num() == 2)
	{
		const FFMCodexUMGBranchChoiceViewModel& High =
			SurfaceModel.BranchChoices[0];
		const FFMCodexUMGBranchChoiceViewModel& Low =
			SurfaceModel.BranchChoices[1];
		TestEqual(TEXT("High label follows canonical terminology"),
			High.Label, FString(TEXT("高球传中")));
		TestEqual(TEXT("High helper derives the canonical comparison"),
			High.SecondaryLabel,
			FString(TEXT("（传球 / 力量 vs 抢断 / 力量）")));
		TestEqual(TEXT("Low label follows canonical terminology"),
			Low.Label, FString(TEXT("低球传中")));
		TestEqual(TEXT("Low helper derives the canonical comparison"),
			Low.SecondaryLabel,
			FString(TEXT("（传球 / 射门 vs 抢断 / 盯防）")));
	}
	TestFalse(TEXT("Method selection consumes no roll action"),
		Presentation.Interaction.PrimaryAction.bAvailable);

	UFMCodexLocalMatchScreenWidget* MatchScreen =
		NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
	MatchScreen->TakeWidget();
	MatchScreen->RefreshFromPresentation(Presentation);
	TestTrue(TEXT("Central Cross choice suppresses the lower duplicate"),
		MatchScreen->GetLongShotResolutionSurface() != nullptr
			&& MatchScreen->GetLongShotResolutionSurface()->GetPresentation()
				.bVisible
			&& MatchScreen->GetInteractionPanel()->GetVisibility()
				== ESlateVisibility::Collapsed);

	UFMCodexLongShotResolutionSurfaceWidget* Surface =
		NewObject<UFMCodexLongShotResolutionSurfaceWidget>(
			GetTransientPackage());
	Surface->TakeWidget();
	Surface->RefreshFromPresentation(SurfaceModel);
	const auto& Options = Surface->GetBranchChoiceWidgets();
	TestEqual(TEXT("Cross uses the same two-option geometry as LongShot"),
		Options.Num(), 2);
	if (Options.Num() == 2)
	{
		Options[0]->TakeWidget();
		Options[1]->TakeWidget();
		UButton* HighButton = Cast<UButton>(
			Options[0]->GetWidgetFromName(TEXT("InteractionOptionButton")));
		UButton* LowButton = Cast<UButton>(
			Options[1]->GetWidgetFromName(TEXT("InteractionOptionButton")));
		const UTextBlock* HighHelper = Cast<UTextBlock>(
			Options[0]->GetWidgetFromName(
				TEXT("InteractionOptionSecondaryLabel")));
		const UTextBlock* LowHelper = Cast<UTextBlock>(
			Options[1]->GetWidgetFromName(
				TEXT("InteractionOptionSecondaryLabel")));
		TestTrue(TEXT("Cross helpers are secondary single-line text"),
			HighHelper != nullptr && LowHelper != nullptr
				&& !HighHelper->GetAutoWrapText()
				&& !LowHelper->GetAutoWrapText());
		TestTrue(TEXT("Cross branch options add no hover-detail dispatch"),
			HighButton != nullptr && LowButton != nullptr
				&& !HighButton->OnHovered.IsBound()
				&& !LowButton->OnHovered.IsBound());
		if (HighButton != nullptr && LowButton != nullptr)
		{
			Surface->ResetBranchDispatchForTesting();
			HighButton->OnClicked.Broadcast();
			TestTrue(TEXT("High click dispatches one typed intent"),
				Surface->GetBranchDispatchCountForTesting() == 1
					&& Surface->GetLastBranchDispatchForTesting()
						== EFMCodexUMGBranchIntent::CrossHigh);
			Surface->ResetBranchDispatchForTesting();
			LowButton->OnClicked.Broadcast();
			TestTrue(TEXT("Low click dispatches one typed intent"),
				Surface->GetBranchDispatchCountForTesting() == 1
					&& Surface->GetLastBranchDispatchForTesting()
						== EFMCodexUMGBranchIntent::CrossLow);
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexCrossSelectedMethodReconstructionAlignmentTest,
	"FMCodex.LocalPlay.TacticalBranchAlignment.02.CrossSelectedReconstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexCrossSelectedMethodReconstructionAlignmentTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexTacticalBranchAlignmentPresentationTests;
	(void)Parameters;
	for (const EMatchPlayElectiveBranchIntent Intent : {
		EMatchPlayElectiveBranchIntent::CrossHigh,
		EMatchPlayElectiveBranchIntent::CrossLow })
	{
		FFMCodexLocalMatchInteractionView View = MakeCrossView(
			EFMCodexLocalMatchInteractionCategory::ContinueResolution);
		View.ElectiveBranchIntent = Intent;
		View.ContinueActionLabel = TEXT("判定传中路线");

		const FFMCodexUMGMatchScreenViewModel Presentation = Build(View);
		const FFMCodexUMGMatchScreenViewModel Rebuilt = Build(View);
		TestFalse(TEXT("Selected method never returns to branch choice"),
			Presentation.LongShotResolution.bVisible);
		TestTrue(TEXT("Selected method reconstructs the next central route CTA"),
			Presentation.InlineFormula.bVisible
				&& Presentation.InlineFormula.ContestId == TEXT("Cross.Route")
				&& Presentation.InlineFormula.StatusLabel == TEXT("等待路线掷点")
				&& Presentation.InlineFormula.PrimaryAction.Claims(
					Presentation.Interaction.PrimaryAction)
				&& Presentation.InlineFormula.ContinueActionLabel
					== TEXT("判定传中路线"));
		TestTrue(TEXT("Repeated reconstruction is stable"),
			Rebuilt.InlineFormula.bVisible
				&& Rebuilt.InlineFormula.ContestId
					== Presentation.InlineFormula.ContestId
				&& Rebuilt.InlineFormula.ContinueActionLabel
					== Presentation.InlineFormula.ContinueActionLabel);

		UFMCodexLocalMatchScreenWidget* MatchScreen =
			NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
		MatchScreen->TakeWidget();
		MatchScreen->RefreshFromPresentation(Presentation);
		TestTrue(TEXT("Reconstructed route CTA remains central without lower duplicate"),
			MatchScreen->GetInlineFormulaSurface() != nullptr
				&& MatchScreen->GetInlineFormulaSurface()->GetPresentation()
					.bVisible
				&& MatchScreen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Collapsed);
	}
	return true;
}

#endif
