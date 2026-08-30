#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLongShotResolutionSurfaceWidget.h"

#include "Editor.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "UnrealClient.h"

namespace FMCodexInlineResolutionFormulaPIETests
{
	const FString CaptureDirectory = FPaths::ConvertRelativePathToFull(
		FPaths::ProjectSavedDir() / TEXT("PIE/Stage6131418C"));

	AFMCodexLocalMatchPlayerController* GetPIEController()
	{
		return GEditor == nullptr || GEditor->PlayWorld == nullptr
			? nullptr
			: Cast<AFMCodexLocalMatchPlayerController>(
				GEditor->PlayWorld->GetFirstPlayerController());
	}

	int32 FindPIESeed()
	{
		for (int32 Seed = 0; Seed < 4000000; ++Seed)
		{
			FRandomStream Stream(Seed);
			if (Stream.RandRange(2, 8) == 6
				&& Stream.RandRange(1, 6) == 2
				&& Stream.RandRange(1, 6) == 4
				&& Stream.RandRange(1, 6) == 3)
			{
				return Seed;
			}
		}
		return INDEX_NONE;
	}

	bool DeployNextOrdinary(
		AFMCodexLocalMatchPlayerController& Controller,
		const FString& SlotFragment)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		const bool bDeployingAttacker =
			View.CurrentLegalDeploymentSide == View.CurrentAttackingPlayer;
		FName PreferredCardId = NAME_None;
		if (bDeployingAttacker)
		{
			const FName RequiredCarrierId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
					: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			const FName RequiredRunnerId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
					: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
			const TArray<FFMCodexLocalMatchCardView>& Roster =
				View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
			const bool bCarrierDeployed = Roster.ContainsByPredicate(
				[RequiredCarrierId](const FFMCodexLocalMatchCardView& Card)
				{
					return Card.bDeployed && Card.CardId == RequiredCarrierId;
				});
			PreferredCardId = bCarrierDeployed
				? RequiredRunnerId : RequiredCarrierId;
		}

		const FFMCodexLocalMatchDeploymentOption* Option =
			View.DeploymentOptions.FindByPredicate(
				[PreferredCardId, &SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment)
						&& (PreferredCardId.IsNone()
							|| Candidate.CardId == PreferredCardId);
				});
		if (Option == nullptr && !PreferredCardId.IsNone())
		{
			Option = View.DeploymentOptions.FindByPredicate(
				[&SlotFragment](
					const FFMCodexLocalMatchDeploymentOption& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.SlotId.ToString().Contains(SlotFragment);
				});
		}
		if (Option == nullptr)
		{
			return false;
		}
		Controller.DeployOrdinary(Option->CardId, Option->SlotId);
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	bool SubmitFirst(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		Controller.RefreshPresentation();
		const FFMCodexLocalMatchInteractionView& View =
			Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			Controller.SubmitMarker(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			Controller.SubmitRunner(Id);
			break;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			Controller.SubmitHelper(Id);
			break;
		default:
			return false;
		}
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	bool HasAllSelectedRoles(
		const FFMCodexUMGMatchScreenViewModel& Presentation)
	{
		bool bCarrier = false;
		bool bRunner = false;
		bool bMarker = false;
		for (const FFMCodexUMGPitchRegionViewModel& Region
			: Presentation.PitchRegions)
		{
			for (const FFMCodexUMGPitchSlotViewModel& Slot : Region.Slots)
			{
				bCarrier |= Slot.Card.SelectedRole
					== EFMCodexUMGSelectedRole::Carrier;
				bRunner |= Slot.Card.SelectedRole
					== EFMCodexUMGSelectedRole::Runner;
				bMarker |= Slot.Card.SelectedRole
					== EFMCodexUMGSelectedRole::Marker;
			}
		}
		return bCarrier && bRunner && bMarker;
	}

	const FFMCodexUMGInlineFormulaTermViewModel* FindRawRoll(
		const FFMCodexUMGInlineFormulaRowViewModel& Row)
	{
		return Row.Terms.FindByPredicate(
			[](const FFMCodexUMGInlineFormulaTermViewModel& Term)
			{
				return Term.Kind == EFMCodexUMGInlineFormulaTermKind::RawRoll;
			});
	}

	bool ValidateSurfaceState(
		FAutomationTestBase& Test,
		const int32 StateIndex)
	{
		AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
		if (Controller != nullptr)
		{
			// Exercise the production rebuild path repeatedly in every authority phase.
			Controller->RefreshPresentation();
			Controller->RefreshPresentation();
		}
		UFMCodexLocalMatchScreenWidget* Screen = Controller == nullptr
			? nullptr : Controller->GetPlayerMatchScreen();
		UFMCodexInlineResolutionFormulaSurfaceWidget* Surface =
			Screen == nullptr ? nullptr : Screen->GetInlineFormulaSurface();
		if (Controller == nullptr || Screen == nullptr || Surface == nullptr)
		{
			Test.AddError(FString::Printf(
				TEXT("PIE state %d lacks Controller/Screen/Formula Surface."),
				StateIndex));
			return false;
		}
		const auto& Presentation = Screen->GetPresentation();
		const auto& Formula = Surface->GetPresentation();
		const bool bSharedContext = Formula.bVisible
			&& Formula.bSuppressLegacyResolution
			&& Formula.ContestId == TEXT("Cross.High")
			&& (StateIndex < 3
				? Formula.ContestLabel == TEXT("高球传中")
				: Formula.bNarrativeAvailable
					&& Formula.ContestLabel == Formula.NarrativeHeadline)
			&& Formula.RouteResultLabel
				== TEXT("路线掷点 2 → 判定为高球传中")
			&& Formula.TacticalPlayerSummaryLabel.IsEmpty()
			&& !Screen->IsLegacyResolutionOverlayVisible()
			&& Screen->GetMatchHeader() != nullptr
			&& Screen->GetPitchWidget() != nullptr
			&& Screen->GetInteractionPanel() != nullptr
			&& HasAllSelectedRoles(Presentation)
			&& Formula.AttackRow.bKnownNonRollSubtotalResolved
			&& Formula.DefenseRow.bKnownNonRollSubtotalResolved
			&& Formula.AttackRow.KnownNonRollSubtotalLabel.StartsWith(TEXT("基础值 "))
			&& Formula.DefenseRow.KnownNonRollSubtotalLabel.StartsWith(TEXT("基础值 "))
			&& Formula.RevealPhase == EFMCodexUMGInlineFormulaRevealPhase::None
			&& !Formula.bDiceRevealVisible
			&& !Screen->IsInlineFormulaRevealInputBlocked();
		const auto* AttackRoll = FindRawRoll(Formula.AttackRow);
		const auto* DefenseRoll = FindRawRoll(Formula.DefenseRow);
		bool bStateTruth = false;
		switch (StateIndex)
		{
		case 1:
			bStateTruth = !Formula.AttackRow.bFinalValueResolved
				&& !Formula.DefenseRow.bFinalValueResolved
				&& Formula.AttackRow.bDisplayedResultResolved
				&& Formula.DefenseRow.bDisplayedResultResolved
				&& !Formula.AttackRow.bDisplayedResultIsFinalValue
				&& !Formula.DefenseRow.bDisplayedResultIsFinalValue
				&& FMath::IsNearlyEqual(
					Formula.AttackRow.DisplayedResult,
					Formula.AttackRow.KnownNonRollSubtotal)
				&& FMath::IsNearlyEqual(
					Formula.DefenseRow.DisplayedResult,
					Formula.DefenseRow.KnownNonRollSubtotal)
				&& Formula.AttackRow.DisplayedResultLabel != TEXT("?")
				&& Formula.DefenseRow.DisplayedResultLabel != TEXT("?")
				&& Surface->GetRenderedPendingTermCount() == 1
				&& Formula.bCanContinue
				&& Formula.StatusLabel == TEXT("等待进攻方掷点")
				&& Formula.ContinueActionLabel == TEXT("进攻方掷点")
				&& AttackRoll != nullptr && !AttackRoll->bResolved
				&& AttackRoll->DisplayLabel == TEXT("掷点 ?")
				&& DefenseRoll != nullptr && !DefenseRoll->bResolved
				&& Controller->GetInteractionView().InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::RollCrossAttack
				&& Controller->GetInteractionView().ExpectedActingPlayer
					== Controller->GetInteractionView().CurrentAttackingPlayer;
			break;
		case 2:
			bStateTruth = Formula.AttackRow.bFinalValueResolved
				&& FMath::IsNearlyEqual(Formula.AttackRow.FinalValue, 9.5f)
				&& Formula.AttackRow.bDisplayedResultIsFinalValue
				&& FMath::IsNearlyEqual(
					Formula.AttackRow.DisplayedResult,
					Formula.AttackRow.FinalValue)
				&& !Formula.DefenseRow.bFinalValueResolved
				&& !Formula.DefenseRow.bDisplayedResultIsFinalValue
				&& FMath::IsNearlyEqual(
					Formula.DefenseRow.DisplayedResult,
					Formula.DefenseRow.KnownNonRollSubtotal)
				&& AttackRoll != nullptr && AttackRoll->bResolved
				&& AttackRoll->RawD6 == 4
				&& AttackRoll->DisplayLabel == TEXT("掷点 4")
				&& DefenseRoll != nullptr && !DefenseRoll->bResolved
				&& DefenseRoll->DisplayLabel == TEXT("掷点 ?")
				&& Surface->GetRenderedPendingTermCount() == 1
				&& Formula.bCanContinue
				&& Formula.StatusLabel == TEXT("等待防守方掷点")
				&& Formula.ContinueActionLabel == TEXT("防守方掷点")
				&& Controller->GetInteractionView().InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::RollCrossDefense
				&& Controller->GetInteractionView().ExpectedActingPlayer
					!= Controller->GetInteractionView().CurrentAttackingPlayer;
			break;
		case 3:
			bStateTruth = Formula.AttackRow.bFinalValueResolved
				&& Formula.DefenseRow.bFinalValueResolved
				&& Formula.AttackRow.bDisplayedResultIsFinalValue
				&& Formula.DefenseRow.bDisplayedResultIsFinalValue
				&& FMath::IsNearlyEqual(
					Formula.AttackRow.DisplayedResult,
					Formula.AttackRow.FinalValue)
				&& FMath::IsNearlyEqual(
					Formula.DefenseRow.DisplayedResult,
					Formula.DefenseRow.FinalValue)
				&& FMath::IsNearlyEqual(Formula.AttackRow.FinalValue, 9.5f)
				&& FMath::IsNearlyEqual(Formula.DefenseRow.FinalValue, 9.5f)
				&& AttackRoll != nullptr && AttackRoll->RawD6 == 4
				&& DefenseRoll != nullptr && DefenseRoll->RawD6 == 3
				&& Surface->GetRenderedPendingTermCount() == 0
				&& Formula.bCanContinue
				&& Formula.bNarrativeAvailable
				&& !Formula.NarrativeHeadline.IsEmpty()
				&& Formula.ResultTitle
					== (Formula.bNarrativeAttackSuccess
						? FString(TEXT("进球"))
						: FString(TEXT("防守成功")))
				&& Formula.StatusLabel
					== (Formula.bNarrativeAttackSuccess
						? FString(TEXT("高球传中 · 进球"))
						: FString(TEXT("高球传中 · 防守成功")))
				&& Formula.ContinueActionLabel == TEXT("下一回合")
				&& Controller->GetInteractionView().InteractionCategory
					== EFMCodexLocalMatchInteractionCategory
						::AdvanceAfterTerminal
				&& Controller->GetInteractionView().bCrossFormulaComplete
				&& !Controller->GetInteractionView()
					.bCrossTerminalActionAvailable
				&& Controller->GetInteractionView().bTerminalPendingAdvance
				&& Presentation.InlineFormula.PrimaryAction.Claims(
					Presentation.Interaction.PrimaryAction)
				&& Presentation.Interaction.bCanContinue
				&& Screen->GetInteractionPanel()->GetVisibility()
					== ESlateVisibility::Collapsed
				&& !Screen->GetInteractionPanel()->IsInteractionBlocked()
				&& Controller->GetResolutionFeedback().bTerminal;
			break;
		default:
			break;
		}
		if (!bSharedContext || !bStateTruth)
		{
			Test.AddError(FString::Printf(
				TEXT("PIE state %d failed formula/context truth (shared=%d state=%d)."),
				StateIndex, bSharedContext, bStateTruth));
			return false;
		}
		return true;
	}

	FString CapturePath(const int32 StateIndex)
	{
		static const TCHAR* Names[] = {
			TEXT("Invalid"), TEXT("RouteResult_HighPreRoll"),
			TEXT("AttackSettled_DefensePending"), TEXT("Completed")
		};
		return CaptureDirectory / FString::Printf(TEXT("CrossFlow_%02d_%s.png"),
			StateIndex + 2,
			Names[FMath::Clamp(StateIndex, 0, 3)]);
	}

	FString FlowCapturePath(const int32 StateIndex)
	{
		return CaptureDirectory / (StateIndex == 1
			? TEXT("CrossFlow_01_AfterMarker_Runner.png")
			: TEXT("CrossFlow_02_SingleRouteAction.png"));
	}
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FPrepareCrossHighPIECommand,
	FAutomationTestBase*, Test);

bool FPrepareCrossHighPIECommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
	if (Controller == nullptr || Controller->GetPlayerMatchScreen() == nullptr)
	{
		return false;
	}
	const int32 Seed = FindPIESeed();
	if (Seed == INDEX_NONE)
	{
		Test->AddError(TEXT("PIE Cross High deterministic seed was unavailable."));
		return true;
	}
	Controller->SetNextDemoMatchSeedForTesting(Seed);
	Controller->StartNewDemoMatch();
	Controller->RollDemoTacticalPoints();
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		Test->AddError(TEXT("PIE tactical-point roll failed."));
		return true;
	}
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (!DeployNextOrdinary(*Controller, PhysicalForward))
		{
			Test->AddError(TEXT("PIE Cross High deployment failed."));
			return true;
		}
	}
	Controller->FinishDeployment();
	Controller->FinishDeployment();
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		Test->AddError(TEXT("PIE Cross High FinishDeployment failed."));
		return true;
	}
	Controller->SubmitCarrier(Attacker == EInitialTurnOrderPlayer::PlayerA
		? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
		: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri")));
	if (!Controller->GetLastDiagnostic().bHostSuccess
		|| !SubmitFirst(*Controller,
			EFMCodexLocalMatchInteractionCategory::SelectMarker))
	{
		Test->AddError(TEXT("PIE Cross High Carrier/Marker selection failed."));
		return true;
	}
	Controller->RefreshPresentation();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	// Fixture setup drives authority directly and therefore compresses the
	// player-facing Tactical Point hold. Complete that prior reveal explicitly
	// before beginning the Cross-specific visual gate.
	if (Screen != nullptr && Screen->IsInlineFormulaRevealInputBlocked())
	{
		Screen->PauseInlineFormulaRevealTimerForTesting();
		Screen->AdvanceInlineFormulaRevealForTesting(5.0f);
	}
	UFMCodexInteractionPanelWidget* Panel =
		Screen == nullptr ? nullptr : Screen->GetInteractionPanel();
	if (Panel != nullptr)
	{
		Panel->TakeWidget();
	}
	const UTextBlock* RunnerTitle = Panel == nullptr
		? nullptr
		: Cast<UTextBlock>(
			Panel->GetWidgetFromName(TEXT("InteractionActionTitle")));
	const FFMCodexLocalMatchInteractionView& RunnerView =
		Controller->GetInteractionView();
	if (Screen == nullptr
		|| RunnerView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectRunner
		|| RunnerView.SelectedCarrierCardId.IsNone()
		|| RunnerView.SelectedMarkerCardId.IsNone()
		|| !RunnerView.SelectedRunnerCardId.IsNone()
		|| Screen->GetPresentation().Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectRunner
		|| RunnerTitle == nullptr
		|| RunnerTitle->GetText().ToString() != TEXT("选择跑位球员"))
	{
		Test->AddError(FString::Printf(TEXT(
			"PIE flow did not advance Marker -> Runner before tactical selection "
			"(category=%d stage=%d title='%s' carrier=%s marker=%s runner=%s)."),
			static_cast<int32>(RunnerView.InteractionCategory),
			static_cast<int32>(RunnerView.SelectionStage),
			Screen == nullptr
				? TEXT("<no-screen>")
				: *Screen->GetPresentation().Interaction.TitleLabel,
			*RunnerView.SelectedCarrierCardId.ToString(),
			*RunnerView.SelectedMarkerCardId.ToString(),
			*RunnerView.SelectedRunnerCardId.ToString()));
		return true;
	}
	const FString Path = FlowCapturePath(1);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FScreenshotRequest::RequestScreenshot(Path, true, false);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FAdvanceCrossToRouteEntryPIECommand,
	FAutomationTestBase*, Test);

bool FAdvanceCrossToRouteEntryPIECommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
	if (Controller == nullptr)
	{
		Test->AddError(TEXT("PIE Controller disappeared before route entry."));
		return true;
	}
	if (!SubmitFirst(*Controller,
		EFMCodexLocalMatchInteractionCategory::SelectRunner))
	{
		Test->AddError(TEXT("PIE Cross High Runner selection failed."));
		return true;
	}
	Controller->RefreshPresentation();
	const auto& HelperView = Controller->GetInteractionView();
	if (!HelperView.SelectionOptions.IsEmpty())
	{
		if (!SubmitFirst(*Controller,
			EFMCodexLocalMatchInteractionCategory::SelectHelper))
		{
			Test->AddError(TEXT("PIE Cross High Helper selection failed."));
			return true;
		}
	}
	else if (HelperView.bCanResolveNoLegalChoice)
	{
		Controller->ResolveNoLegalCurrentSelection();
	}
	else
	{
		Controller->DeclineCurrentSelection();
	}
	Controller->SubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		Test->AddError(TEXT("PIE Cross High deferred Skill selection failed."));
		return true;
	}
	Controller->RefreshPresentation();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	UFMCodexInteractionPanelWidget* Panel =
		Screen == nullptr ? nullptr : Screen->GetInteractionPanel();
	UFMCodexLongShotResolutionSurfaceWidget* BranchSurface =
		Screen == nullptr ? nullptr : Screen->GetLongShotResolutionSurface();
	const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>* Options =
		BranchSurface == nullptr
			? nullptr : &BranchSurface->GetBranchChoiceWidgets();
	if (Controller->GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
		|| Screen == nullptr
		|| Screen->GetPresentation().Interaction.Category
			!= EFMCodexUMGInteractionCategory::SelectBranchIntent
		|| Screen->GetPresentation().Interaction.BranchChoices.Num() != 2
		|| Screen->GetPresentation().Interaction.BranchChoices[0].Intent
			!= EFMCodexUMGBranchIntent::CrossHigh
		|| Screen->GetPresentation().Interaction.BranchChoices[1].Intent
			!= EFMCodexUMGBranchIntent::CrossLow
		|| !Screen->GetPresentation().LongShotResolution.bVisible
		|| Screen->GetPresentation().LongShotResolution.SkillType
			!= ESkillRuleType::Cross
		|| Panel == nullptr
		|| Panel->GetVisibility() != ESlateVisibility::Collapsed
		|| Options == nullptr || Options->Num() != 2
		|| (*Options)[0] == nullptr || (*Options)[1] == nullptr
		|| (*Options)[0]->GetVisibility() == ESlateVisibility::Collapsed
		|| (*Options)[0]->GetVisibility() == ESlateVisibility::Hidden
		|| (*Options)[1]->GetVisibility() == ESlateVisibility::Collapsed
		|| (*Options)[1]->GetVisibility() == ESlateVisibility::Hidden
		|| (*Options)[0]->GetLabel() != TEXT("高球传中")
		|| (*Options)[0]->GetSecondaryLabel()
			!= TEXT("（传球 / 力量 vs 抢断 / 力量）")
		|| (*Options)[1]->GetLabel() != TEXT("低球传中")
		|| (*Options)[1]->GetSecondaryLabel()
			!= TEXT("（传球 / 射门 vs 抢断 / 盯防）"))
	{
		Test->AddError(TEXT(
			"PIE Cross route choice did not render centrally with canonical helpers."));
		return true;
	}
	Controller->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
	Controller->RefreshPresentation();
	const FFMCodexLocalMatchInteractionView& RouteEntryView =
		Controller->GetInteractionView();
	if (!Controller->GetLastDiagnostic().bHostSuccess
		|| RouteEntryView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::RollCrossRoute
		|| RouteEntryView.ContinueActionLabel != TEXT("判定传中路线")
		|| !RouteEntryView.AcceptedRolls.IsEmpty())
	{
		Test->AddError(TEXT(
			"PIE Cross route entry was not one clear pre-roll action."));
		return true;
	}
	const FString Path = FlowCapturePath(2);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FScreenshotRequest::RequestScreenshot(Path, true, false);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FResolveMergedCrossRoutePIECommand,
	FAutomationTestBase*, Test);

bool FResolveMergedCrossRoutePIECommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
	if (Controller == nullptr)
	{
		Test->AddError(TEXT("PIE Controller disappeared before merged route action."));
		return true;
	}
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		Test->AddError(TEXT("PIE Screen disappeared before merged route action."));
		return true;
	}
	// The latent command drives the Controller directly, so mirror the
	// player-facing Screen's pre-command RequestInFlight transition first.
	Screen->BeginPendingCrossRollRevealForTesting();
	Controller->RollCrossRoute();
	const bool bInitialRouteIsTwo = Controller->GetInteractionView()
		.ResolutionFacts.Rolls.ContainsByPredicate(
			[](const FMatchPlayResolutionRollFact& Roll)
			{
				return Roll.Semantics
					== EMatchPlayResolutionRollSemantics::BranchSelection
					&& Roll.bResolved && Roll.RawD6 == 2;
			});
	if (!Controller->GetLastDiagnostic().bHostSuccess
		|| Controller->GetLastDiagnostic().CommandName
			!= TEXT("ResolveCrossInitialRouteRoll")
		|| Controller->GetInteractionView().AcceptedRolls.Num() != 1
		|| !bInitialRouteIsTwo)
	{
		Test->AddError(TEXT(
			"PIE merged route action did not produce exactly one route D6 and High pre-roll state."));
	}
	const auto* Surface = Screen == nullptr
		? nullptr : Screen->GetInlineFormulaSurface();
	if (Screen == nullptr || Surface == nullptr
		|| !Screen->IsInlineFormulaRevealInputBlocked()
		|| Surface->GetPresentation().ContestId != TEXT("Cross.Route")
		|| !Surface->GetPresentation().RouteResultLabel.IsEmpty())
	{
		Test->AddError(FString::Printf(TEXT(
			"PIE route cycling did not gate the resolved branch presentation "
			"(phase=%d blocked=%d kind=%d contest=%s route=%s)."),
			Screen == nullptr ? -1
				: static_cast<int32>(Screen->GetInlineFormulaRevealPhase()),
			Screen != nullptr && Screen->IsInlineFormulaRevealInputBlocked(),
			Screen == nullptr ? -1 : static_cast<int32>(
				Screen->GetPresentation().Interaction.CrossRollRevealKind),
			Surface == nullptr ? TEXT("<null>")
				: *Surface->GetPresentation().ContestId.ToString(),
			Surface == nullptr ? TEXT("<null>")
				: *Surface->GetPresentation().RouteResultLabel));
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FAdvanceCrossHighPIECommand,
	FAutomationTestBase*, Test,
	int32, StateIndex,
	bool, bAdvance);

class FWaitForDiceRevealSettlePIECommand final
	: public IAutomationLatentCommand
{
public:
	FWaitForDiceRevealSettlePIECommand(
		FAutomationTestBase* InTest,
		const TCHAR* InContext)
		: Test(InTest)
		, Context(InContext)
	{
	}

	virtual bool Update() override
	{
		using namespace FMCodexInlineResolutionFormulaPIETests;
		if (StartSeconds <= 0.0)
		{
			StartSeconds = FPlatformTime::Seconds();
		}
		AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
		UFMCodexLocalMatchScreenWidget* Screen = Controller == nullptr
			? nullptr : Controller->GetPlayerMatchScreen();
		// Commandlet PIE does not guarantee that game-world timers advance while
		// offscreen screenshot work stalls the editor thread. Advance the exact
		// production state machine deterministically; fresh interactive PIE still
		// validates the real timer and visual feel.
		if (Screen != nullptr && Screen->IsInlineFormulaRevealInputBlocked())
		{
			Screen->AdvanceInlineFormulaRevealForTesting(0.50f);
		}
		if (Screen != nullptr && !Screen->IsInlineFormulaRevealInputBlocked())
		{
			return true;
		}
		// Commandlet screenshot capture can stall the editor thread for several
		// seconds, so this watchdog is deliberately much larger than the 1.1 s
		// production reveal. It guards a stuck reveal without asserting wall-clock
		// timing in an environment where the game world is not ticking.
		if (FPlatformTime::Seconds() - StartSeconds >= 15.0)
		{
			Test->AddError(FString::Printf(
				TEXT("PIE %s dice reveal did not settle within 15 seconds."),
				*Context));
			return true;
		}
		return false;
	}

private:
	FAutomationTestBase* Test = nullptr;
	FString Context;
	double StartSeconds = 0.0;
};

bool FAdvanceCrossHighPIECommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	AFMCodexLocalMatchPlayerController* Controller = GetPIEController();
	if (Controller == nullptr)
	{
		Test->AddError(TEXT("PIE Controller disappeared during formula capture."));
		return true;
	}
	if (bAdvance)
	{
		UFMCodexLocalMatchScreenWidget* Screen =
			Controller->GetPlayerMatchScreen();
		UFMCodexInlineResolutionFormulaSurfaceWidget* Surface =
			Screen == nullptr ? nullptr : Screen->GetInlineFormulaSurface();
		if (Screen == nullptr || Surface == nullptr)
		{
			Test->AddError(TEXT("PIE formula surface disappeared before manual roll."));
			return true;
		}
		Surface->RequestContinue();
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			Test->AddError(FString::Printf(
				TEXT("PIE continuation before state %d failed."), StateIndex));
			return true;
		}
		const bool bExpectedAcceptedTransition = StateIndex == 2
			? Controller->GetLastDiagnostic().CommandName
				== TEXT("ResolveCrossHighAttackRoll")
				&& !Controller->GetResolutionFeedback().bTerminal
			: Controller->GetLastDiagnostic().CommandName
				== TEXT("ApplyCrossTerminalResolution")
				&& Controller->GetResolutionFeedback().bTerminal
				&& Controller->GetInteractionView().bTerminalPendingAdvance;
		if (!bExpectedAcceptedTransition)
		{
			Test->AddError(FString::Printf(
				TEXT("PIE state %d did not use the expected typed roll/terminal transition."),
				StateIndex));
			return true;
		}
		if (!Screen->IsInlineFormulaRevealInputBlocked()
			|| !Surface->GetPresentation().bDiceRevealVisible
			|| Surface->GetPresentation().bCanContinue)
		{
			Test->AddError(FString::Printf(
				TEXT("PIE state %d did not enter gated dice cycling."),
				StateIndex));
		}
		return true;
	}
	ValidateSurfaceState(*Test, StateIndex);
	const FString Path = CapturePath(StateIndex);
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FScreenshotRequest::RequestScreenshot(Path, true, false);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FVerifyPIECaptureCommand,
	FAutomationTestBase*, Test,
	int32, StateIndex);

bool FVerifyPIECaptureCommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	const FString Path = CapturePath(StateIndex);
	if (!FPaths::FileExists(Path))
	{
		Test->AddError(FString::Printf(
			TEXT("PIE state %d screenshot was not written: %s"),
			StateIndex, *Path));
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FVerifyPIEFlowCaptureCommand,
	FAutomationTestBase*, Test,
	int32, StateIndex);

bool FVerifyPIEFlowCaptureCommand::Update()
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	const FString Path = FlowCapturePath(StateIndex);
	if (!FPaths::FileExists(Path))
	{
		Test->AddError(FString::Printf(
			TEXT("PIE flow state %d screenshot was not written: %s"),
			StateIndex, *Path));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInlineResolutionFormulaPIEVisualGateTest,
	"FMCodex.PIE.InlineFormula.CrossHighManualRollVisualGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInlineResolutionFormulaPIEVisualGateTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	IFileManager::Get().MakeDirectory(*CaptureDirectory, true);
	for (int32 StateIndex = 1; StateIndex <= 2; ++StateIndex)
	{
		IFileManager::Get().Delete(*FlowCapturePath(StateIndex), false, true);
	}
	for (int32 StateIndex = 1; StateIndex <= 3; ++StateIndex)
	{
		IFileManager::Get().Delete(*CapturePath(StateIndex), false, true);
	}
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FPrepareCrossHighPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIEFlowCaptureCommand(this, 1));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossToRouteEntryPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIEFlowCaptureCommand(this, 2));
	ADD_LATENT_AUTOMATION_COMMAND(FResolveMergedCrossRoutePIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(
		FWaitForDiceRevealSettlePIECommand(this, TEXT("route")));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 1, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 1));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 2, true));
	ADD_LATENT_AUTOMATION_COMMAND(
		FWaitForDiceRevealSettlePIECommand(this, TEXT("attack")));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 2, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 2));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 3, true));
	ADD_LATENT_AUTOMATION_COMMAND(
		FWaitForDiceRevealSettlePIECommand(this, TEXT("defense")));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 3, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 3));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	return true;
}

#endif
