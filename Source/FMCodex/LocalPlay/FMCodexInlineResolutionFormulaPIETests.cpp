#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "FMCodexInlineResolutionFormulaSurfaceWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"

#include "Editor.h"
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
		FPaths::ProjectSavedDir() / TEXT("PIE/Stage6131418"));

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
			&& Formula.ContestLabel == TEXT("高球传中")
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
				&& Surface->GetRenderedPendingTermCount() == 1
				&& Formula.bCanContinue
				&& Formula.StatusLabel == TEXT("等待进攻方掷点")
				&& Formula.ContinueActionLabel == TEXT("进攻方掷点")
				&& AttackRoll != nullptr && !AttackRoll->bResolved
				&& AttackRoll->DisplayLabel == TEXT("掷点 ?")
				&& DefenseRoll != nullptr && !DefenseRoll->bResolved
				&& Controller->GetInteractionView().InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::RollCrossHighAttack
				&& Controller->GetInteractionView().ExpectedActingPlayer
					== Controller->GetInteractionView().CurrentAttackingPlayer;
			break;
		case 2:
			bStateTruth = Formula.AttackRow.bFinalValueResolved
				&& FMath::IsNearlyEqual(Formula.AttackRow.FinalValue, 9.5f)
				&& !Formula.DefenseRow.bFinalValueResolved
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
					== EFMCodexLocalMatchInteractionCategory::RollCrossHighDefense
				&& Controller->GetInteractionView().ExpectedActingPlayer
					!= Controller->GetInteractionView().CurrentAttackingPlayer;
			break;
		case 3:
			bStateTruth = Formula.AttackRow.bFinalValueResolved
				&& Formula.DefenseRow.bFinalValueResolved
				&& FMath::IsNearlyEqual(Formula.AttackRow.FinalValue, 9.5f)
				&& FMath::IsNearlyEqual(Formula.DefenseRow.FinalValue, 9.5f)
				&& AttackRoll != nullptr && AttackRoll->RawD6 == 4
				&& DefenseRoll != nullptr && DefenseRoll->RawD6 == 3
				&& Surface->GetRenderedPendingTermCount() == 0
				&& Formula.bCanContinue
				&& Formula.StatusLabel == TEXT("双方掷点已完成")
				&& !Screen->GetInteractionPanel()->IsInteractionBlocked()
				&& !Controller->GetResolutionFeedback().bTerminal;
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
			TEXT("Invalid"), TEXT("PreRoll"), TEXT("AttackSettled_DefensePending"),
			TEXT("Completed")
		};
		return CaptureDirectory / FString::Printf(TEXT("CrossHigh_State%d_%s.png"),
			StateIndex, Names[FMath::Clamp(StateIndex, 0, 3)]);
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
	Controller->SubmitSkill(TEXT("Canonical.Skill.Cross.4.6"));
	if (!Controller->GetLastDiagnostic().bHostSuccess
		|| !SubmitFirst(*Controller,
			EFMCodexLocalMatchInteractionCategory::SelectRunner))
	{
		Test->AddError(TEXT("PIE Cross High Skill/Runner selection failed."));
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
	Controller->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
	Controller->ContinueResolution();
	Controller->ContinueResolution();
	const bool bInitialRouteIsTwo = Controller->GetInteractionView()
		.ResolutionFacts.Rolls.ContainsByPredicate(
			[](const FMatchPlayResolutionRollFact& Roll)
			{
				return Roll.Semantics
					== EMatchPlayResolutionRollSemantics::BranchSelection
					&& Roll.bResolved && Roll.RawD6 == 2;
			});
	if (!Controller->GetLastDiagnostic().bHostSuccess
		|| !bInitialRouteIsTwo
		|| !ValidateSurfaceState(*Test, 1))
	{
		Test->AddError(TEXT("PIE did not reach Cross High pre-roll formula state."));
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
	FAdvanceCrossHighPIECommand,
	FAutomationTestBase*, Test,
	int32, StateIndex,
	bool, bAdvance);

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
		const FString ExpectedCommand = StateIndex == 2
			? TEXT("ResolveCrossHighAttackRoll")
			: TEXT("ResolveCrossHighDefenseRoll");
		if (Controller->GetLastDiagnostic().CommandName != ExpectedCommand
			|| Controller->GetResolutionFeedback().bTerminal)
		{
			Test->AddError(FString::Printf(
				TEXT("PIE state %d did not use the expected typed manual roll command."),
				StateIndex));
			return true;
		}
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInlineResolutionFormulaPIEVisualGateTest,
	"FMCodex.PIE.InlineFormula.CrossHighManualRollVisualGate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInlineResolutionFormulaPIEVisualGateTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexInlineResolutionFormulaPIETests;
	IFileManager::Get().MakeDirectory(*CaptureDirectory, true);
	for (int32 StateIndex = 1; StateIndex <= 3; ++StateIndex)
	{
		IFileManager::Get().Delete(*CapturePath(StateIndex), false, true);
	}
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FPrepareCrossHighPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 1, false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 1));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 2, true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 2));
	ADD_LATENT_AUTOMATION_COMMAND(
		FAdvanceCrossHighPIECommand(this, 3, true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.15f));
	ADD_LATENT_AUTOMATION_COMMAND(FVerifyPIECaptureCommand(this, 3));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	return true;
}

#endif
