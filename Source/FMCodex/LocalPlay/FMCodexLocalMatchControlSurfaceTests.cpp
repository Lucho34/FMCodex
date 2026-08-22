#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexDeploymentDragDropOperation.h"
#include "FMCodexLocalMatchHostGameMode.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchPlayerController.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexHandMicroDiagnostics.h"
#include "FMCodexInteractionOptionWidget.h"
#include "FMCodexDiceResultWidget.h"
#include "FMCodexMatchHeaderWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPlayerUIPresentationText.h"
#include "FMCodexPlayerUIStyle.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexResolutionPanelWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/MemoryWriter.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "Rendering/SlateRenderer.h"

namespace FMCodexLocalMatchControlSurfaceTests
{
	void AcknowledgeIfPending(
		AFMCodexLocalMatchPlayerController& Controller)
	{
		Controller.RefreshPresentation();
	}

	class FScopedPlayableWorld final
	{
	public:
		FScopedPlayableWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			if (World == nullptr || GEngine == nullptr)
			{
				return;
			}

			FWorldContext& Context =
				GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			Host = World->SpawnActor<AFMCodexLocalMatchHostGameMode>();
			Controller = World->SpawnActor<AFMCodexLocalMatchPlayerController>();
			if (Controller != nullptr)
			{
				for (int32 Seed = 0; Seed < 1000; ++Seed)
				{
					FRandomStream Stream(Seed);
					if (Stream.RandRange(2, 8) == 6)
					{
						Controller->SetNextDemoMatchSeedForTesting(Seed);
						break;
					}
				}
				Controller->RefreshPresentation();
			}
		}

		~FScopedPlayableWorld()
		{
			if (World != nullptr)
			{
				if (GEngine != nullptr)
				{
					GEngine->DestroyWorldContext(World);
				}
				World->DestroyWorld(false);
			}
		}

		AFMCodexLocalMatchHostGameMode* GetHost() const { return Host; }
		AFMCodexLocalMatchPlayerController* GetController() const
		{
			return Controller;
		}

	private:
		UWorld* World = nullptr;
		AFMCodexLocalMatchHostGameMode* Host = nullptr;
		AFMCodexLocalMatchPlayerController* Controller = nullptr;
	};

	bool LoadProductionSource(const TCHAR* RelativePath, FString& OutSource)
	{
		return FFileHelper::LoadFileToString(
			OutSource,
			*FPaths::ConvertRelativePathToFull(
				FPaths::ProjectDir() / RelativePath));
	}

	TArray<uint8> SerializeState(const FMatchPlayState& State)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes);
		FMatchPlayState Copy = State;
		FMatchPlayState::StaticStruct()->SerializeItem(Writer, &Copy, nullptr);
		return Bytes;
	}

	bool DeployNextOrdinary(
		AFMCodexLocalMatchPlayerController& Controller,
		const FString& SlotFragment)
	{
		AcknowledgeIfPending(Controller);
		const auto& View = Controller.GetInteractionView();
		FName PreferredCardId = NAME_None;
		const bool bDeployingAttacker =
			View.CurrentLegalDeploymentSide == View.CurrentAttackingPlayer;
		bool bCrossCarrierAlreadyDeployed = false;
		FName RequiredCrossCarrierId = NAME_None;
		FName RequiredRunnerId = NAME_None;
		if (bDeployingAttacker)
		{
			RequiredCrossCarrierId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
					: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri"));
			RequiredRunnerId = View.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
					: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
			const TArray<FFMCodexLocalMatchCardView>& AttackerRoster =
				View.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
					? View.PlayerACardRoster : View.PlayerBCardRoster;
			bCrossCarrierAlreadyDeployed = AttackerRoster.ContainsByPredicate(
				[RequiredCrossCarrierId](const FFMCodexLocalMatchCardView& Card)
				{
					return Card.bDeployed && Card.CardId == RequiredCrossCarrierId;
				});
		}
		for (const FFMCodexLocalMatchDeploymentGroup& Group
			: View.DeploymentGroups)
		{
			const bool bHasMatchingSlot = Group.LegalSlots.ContainsByPredicate(
				[&SlotFragment](const FFMCodexLocalMatchSlotView& Slot)
				{
					return Slot.SlotId.ToString().Contains(SlotFragment);
				});
			const bool bHasUsableCross = Group.Card.Skills.ContainsByPredicate(
				[&View](const FFMCodexLocalMatchCardView::FSkill& Skill)
				{
					return Skill.CanonicalLabel == TEXT("Cross")
						&& View.ActionPoint >= Skill.MinTriggerActionPoint
						&& View.ActionPoint <= Skill.MaxTriggerActionPoint;
				});
			if (!Group.bGoalkeeper && bHasMatchingSlot
				&& ((!bCrossCarrierAlreadyDeployed
						&& Group.CardId == RequiredCrossCarrierId
						&& bHasUsableCross)
					|| (bCrossCarrierAlreadyDeployed
						&& Group.CardId == RequiredRunnerId)))
			{
				PreferredCardId = Group.CardId;
				break;
			}
		}
		for (const FFMCodexLocalMatchDeploymentOption& Option
			: View.DeploymentOptions)
		{
			if (!Option.bGoalkeeper
				&& Option.SlotId.ToString().Contains(SlotFragment)
				&& (PreferredCardId.IsNone()
					|| Option.CardId == PreferredCardId))
			{
				Controller.DeployOrdinary(Option.CardId, Option.SlotId);
				return Controller.GetLastDiagnostic().bHostSuccess;
			}
		}
		return false;
	}

	bool SubmitFirstSelection(
		AFMCodexLocalMatchPlayerController& Controller,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		AcknowledgeIfPending(Controller);
		const auto& View = Controller.GetInteractionView();
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
			Controller.SubmitCarrier(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
			Controller.SubmitMarker(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
			Controller.SubmitSkill(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
			Controller.SubmitRunner(Id); break;
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
			Controller.SubmitHelper(Id); break;
		default:
			return false;
		}
		return Controller.GetLastDiagnostic().bHostSuccess;
	}

	int32 FindSeedForRolls(const TArray<int32>& Rolls)
	{
		for (int32 Seed = 0; Seed < 4000000; ++Seed)
		{
			FRandomStream Stream(Seed);
			bool bMatches = true;
			for (const int32 Expected : Rolls)
			{
				if (Stream.RandRange(1, 6) != Expected)
				{
					bMatches = false;
					break;
				}
			}
			if (bMatches)
			{
				return Seed;
			}
		}
		return INDEX_NONE;
	}

	int32 FindSeedForTacticalPointAndRolls(
		const int32 TacticalPoint,
		const TArray<int32>& Rolls)
	{
		for (int32 Seed = 0; Seed < 4000000; ++Seed)
		{
			FRandomStream Stream(Seed);
			if (Stream.RandRange(2, 8) != TacticalPoint)
			{
				continue;
			}
			bool bMatches = true;
			for (const int32 Expected : Rolls)
			{
				if (Stream.RandRange(1, 6) != Expected)
				{
					bMatches = false;
					break;
				}
			}
			if (bMatches)
			{
				return Seed;
			}
		}
		return INDEX_NONE;
	}

	FFMCodexLocalMatchInteractionView ViewFor(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules)
	{
		return FFMCodexLocalMatchInteractionViewBuilder::Build(
			Host.GetMatchSnapshot().Snapshot, Rules);
	}

	bool DeployNextOrdinary(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules,
		const FString& SlotFragment)
	{
		const FFMCodexLocalMatchInteractionView View = ViewFor(Host, Rules);
		for (const FFMCodexLocalMatchDeploymentOption& Option
			: View.DeploymentOptions)
		{
			if (Option.bGoalkeeper
				|| !Option.SlotId.ToString().Contains(SlotFragment))
			{
				continue;
			}
			FMatchPlayAuthoritativeDeployOrdinaryRequest Request;
			Request.RequestingSide = Option.Side;
			Request.CardId = Option.CardId;
			Request.SlotId = Option.SlotId;
			return Host.DeployOrdinary(Request).bSuccess;
		}
		return false;
	}

	bool SubmitFirstHostSelection(
		AFMCodexLocalMatchHostGameMode& Host,
		const FSkillRuleSnapshotSet& Rules,
		const EFMCodexLocalMatchInteractionCategory Expected)
	{
		const FFMCodexLocalMatchInteractionView View = ViewFor(Host, Rules);
		if (View.InteractionCategory != Expected
			|| View.SelectionOptions.IsEmpty())
		{
			return false;
		}
		const FName Id = View.SelectionOptions[0].Id;
		switch (Expected)
		{
		case EFMCodexLocalMatchInteractionCategory::SelectCarrier:
		{
			FMatchPlayAuthoritativeSubmitCarrierRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.CarrierCardId = Id;
			return Host.SubmitCarrier(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectMarker:
		{
			FMatchPlayAuthoritativeSubmitMarkerRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.MarkerCardId = Id;
			return Host.SubmitMarker(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectSkill:
		{
			FMatchPlayAuthoritativeSubmitSkillRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.SkillId = Id;
			return Host.SubmitSkill(Rules, Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectRunner:
		{
			FMatchPlayAuthoritativeSubmitRunnerRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.RunnerCardId = Id;
			return Host.SubmitRunner(Request).bSuccess;
		}
		case EFMCodexLocalMatchInteractionCategory::SelectHelper:
		{
			FMatchPlayAuthoritativeSubmitHelperRequest Request;
			Request.RequestingSide = View.ExpectedActingPlayer;
			Request.HelperCardId = Id;
			return Host.SubmitHelper(Request).bSuccess;
		}
		default:
			return false;
		}
	}

	bool CompleteCrossAttack(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const TCHAR* Label)
	{
		AcknowledgeIfPending(Controller);
		Controller.RollDemoTacticalPoints();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: BeginOrdinaryAttack failed: %s"),
				Label,
				*Controller.GetLastDiagnostic().Message));
			return false;
		}
		const EInitialTurnOrderPlayer Attacker =
			Controller.GetInteractionView().CurrentAttackingPlayer;
		const FString PhysicalForward =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("NearB")
				: TEXT("NearA");
		for (int32 Index = 0; Index < 4; ++Index)
		{
			if (!DeployNextOrdinary(Controller, PhysicalForward))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: ordinary deployment %d failed."),
					Label, Index + 1));
				return false;
			}
		}

		AcknowledgeIfPending(Controller);
		Controller.FinishDeployment();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: first FinishDeployment failed."), Label));
			return false;
		}
		AcknowledgeIfPending(Controller);
		Controller.FinishDeployment();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: second FinishDeployment failed."), Label));
			return false;
		}

		for (const EFMCodexLocalMatchInteractionCategory Category : {
			EFMCodexLocalMatchInteractionCategory::SelectCarrier,
			EFMCodexLocalMatchInteractionCategory::SelectMarker,
			EFMCodexLocalMatchInteractionCategory::SelectSkill,
			EFMCodexLocalMatchInteractionCategory::SelectRunner,
			EFMCodexLocalMatchInteractionCategory::SelectHelper })
		{
			bool bSubmitted = false;
			if (Category == EFMCodexLocalMatchInteractionCategory::SelectCarrier)
			{
				AcknowledgeIfPending(Controller);
				Controller.SubmitCarrier(Attacker
					== EInitialTurnOrderPlayer::PlayerA
						? FName(TEXT("Prototype.Arsenal.BukayoSaka"))
						: FName(TEXT("Prototype.ManchesterCity.RayanAitNouri")));
				bSubmitted = Controller.GetLastDiagnostic().bHostSuccess;
			}
			else if (Category
				== EFMCodexLocalMatchInteractionCategory::SelectSkill)
			{
				AcknowledgeIfPending(Controller);
				Controller.SubmitSkill(
					FName(TEXT("Canonical.Skill.Cross.4.6")));
				bSubmitted = Controller.GetLastDiagnostic().bHostSuccess;
			}
			else
			{
				bSubmitted = SubmitFirstSelection(Controller, Category);
			}
			if (!bSubmitted)
			{
				Test.AddError(FString::Printf(
					TEXT("%s: selection category %s failed (actual %s, diagnostic %s)."),
					Label,
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(Category),
					*FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Controller.GetInteractionView().InteractionCategory),
					*Controller.GetLastDiagnostic().Message));
				return false;
			}
		}

		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: Cross did not request BranchIntent."), Label));
			return false;
		}
		AcknowledgeIfPending(Controller);
		Controller.SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: BranchIntent failed."), Label));
			return false;
		}

		for (int32 Step = 0; Step < 3; ++Step)
		{
			if (Controller.GetInteractionView().InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
			{
				Test.AddError(FString::Printf(
					TEXT("%s: system step %d was not Continue."),
					Label, Step + 1));
				return false;
			}
			Controller.ContinueResolution();
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				Test.AddError(FString::Printf(
					TEXT("%s: system step %d failed: %s"),
					Label,
					Step + 1,
					*Controller.GetLastDiagnostic().Message));
				return false;
			}
			const auto& Feedback = Controller.GetResolutionFeedback();
			if (Step == 1
				&& (!Feedback.StepTitle.Contains(TEXT("Route"))
					|| !Feedback.RouteSummary.Contains(TEXT("Cross"))
					|| Feedback.DiceEntries.IsEmpty()))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: route feedback did not expose authoritative Cross branch/D6 evidence."),
					Label));
				return false;
			}
			if (Step == 2
				&& (!Feedback.DecisionSummary.Contains(TEXT("Formula"))
					|| Feedback.DiceEntries.Num() != 3))
			{
				Test.AddError(FString::Printf(
					TEXT("%s: Cross plan feedback did not retain its three authoritative D6 records."),
					Label));
				return false;
			}
		}
		if (Controller.GetInteractionView().AcceptedRolls.Num() != 3)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: Cross did not expose its route and two post-route D6 records."),
				Label));
			return false;
		}

		Controller.ContinueResolution();
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			Test.AddError(FString::Printf(
				TEXT("%s: terminal application failed: %s"),
				Label,
				*Controller.GetLastDiagnostic().Message));
			return false;
		}
		const auto& TerminalFeedback = Controller.GetResolutionFeedback();
		if (!TerminalFeedback.bTerminal
			|| !TerminalFeedback.TerminalSummary.StartsWith(TEXT("RESULT: "))
			|| TerminalFeedback.ComparisonEntries.Num() < 2
			|| !TerminalFeedback.DecisionSummary.Contains(TEXT("Winner:"))
			|| !TerminalFeedback.ContinuationSummary.Contains(TEXT("Next attacker:")))
		{
			Test.AddError(FString::Printf(
				TEXT("%s: terminal feedback did not expose authoritative Formula/Completion evidence."),
				Label));
			return false;
		}
		return !Controller.GetInteractionView().bCurrentAttackActive
			&& Controller.GetInteractionView().bTacticalPointRollReady;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPresentationBoundaryTest,
	"FMCodex.LocalPlay.ControlSurface.11.PresentationBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPresentationBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	const auto Empty =
		FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
	TestFalse(TEXT("NoActiveMatch is not active"), Empty.bMatchActive);
	TestEqual(TEXT("NoActiveMatch asks StartMatch"),
		Empty.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::StartMatch);

	AFMCodexLocalMatchHostGameMode* HostClassDefaults =
		GetMutableDefault<AFMCodexLocalMatchHostGameMode>();
	TestNotNull(TEXT("Host class default exists"), HostClassDefaults);
	if (HostClassDefaults != nullptr)
	{
		TestTrue(TEXT("Host owns the normal presentation class"),
			HostClassDefaults->PlayerControllerClass.Get()
				== AFMCodexLocalMatchPlayerController::StaticClass());
	}

	FString ControllerSource;
	FString ViewHeader;
	TestTrue(TEXT("Controller production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Interaction view header loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.h"),
		ViewHeader));
	TestFalse(TEXT("UI has no Session owner/ref"),
		ControllerSource.Contains(TEXT("FMatchPlayAuthoritativeSession")));
	TestFalse(TEXT("UI has no provider owner/ref"),
		ControllerSource.Contains(TEXT("D6Provider")));
	TestFalse(TEXT("UI has no generic reflection dispatch"),
		ControllerSource.Contains(TEXT("ProcessEvent"))
			|| ControllerSource.Contains(TEXT("FindFunction")));
	TestTrue(TEXT("Interaction view is presentation-only value data"),
		ViewHeader.Contains(TEXT("FFMCodexLocalMatchInteractionView"))
			&& !ViewHeader.Contains(TEXT("FMatchPlayState Snapshot")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchEntryAndTacticalPointRollTest,
	"FMCodex.LocalPlay.MatchStartFlow.01.EntryAndAuthoritativeRoll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchEntryAndTacticalPointRollTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	TestNotNull(TEXT("Match-start Host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TestTrue(TEXT("Match initialization succeeds"),
		Host->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, 0x613141).bSuccess);
	const FMatchPlayState Entry = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Match entry already has a valid current attacker"),
		Entry.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
		|| Entry.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Player A prototype maximum is three"),
		Entry.RuntimeState.PlayerAState.TotalAttackCount, 3);
	TestEqual(TEXT("Player B prototype maximum is three"),
		Entry.RuntimeState.PlayerBState.TotalAttackCount, 3);
	TestEqual(TEXT("Player A begins with zero used attacks"),
		Entry.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("Player B begins with zero used attacks"),
		Entry.RuntimeState.PlayerBState.UsedAttackCount, 0);
	TestFalse(TEXT("Entry does not roll or create CurrentAttack automatically"),
		Entry.bHasCurrentAttack);

	const FFMCodexLocalMatchInteractionView EntryView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(
			Entry, Demo.SkillRuleSet);
	TestEqual(TEXT("Entry projects Tactical Point Roll directly"),
		EntryView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::TacticalPointRoll);
	TestTrue(TEXT("Entry explicitly projects roll readiness"),
		EntryView.bTacticalPointRollReady);
	TestEqual(TEXT("Current attacker starts on its first opportunity"),
		Entry.RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
				? EntryView.PlayerACurrentAttackIndex
				: EntryView.PlayerBCurrentAttackIndex,
		1);
	TestEqual(TEXT("No Tactical Point exists before manual intent"),
		EntryView.ActionPoint, 0);

	const EInitialTurnOrderPlayer Attacker =
		Entry.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	const TArray<uint8> EntryBytes = SerializeState(Entry);
	const auto DefenderRoll = Host->RollTacticalPoints(Defender);
	TestFalse(TEXT("Defending side cannot roll"), DefenderRoll.bSuccess);
	TestEqual(TEXT("Defending side receives exact ownership error"),
		DefenderRoll.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::RequestingSideNotCurrentAttacker);
	TestTrue(TEXT("Rejected defender intent leaves state byte-identical"),
		EntryBytes == SerializeState(Host->GetMatchSnapshot().Snapshot));

	const auto AcceptedRoll = Host->RollTacticalPoints(Attacker);
	TestTrue(TEXT("Current attacker may roll exactly once"),
		AcceptedRoll.bSuccess);
	TestTrue(TEXT("Rolled value obeys current ordinary 2..8 contract"),
		AcceptedRoll.TacticalPoints >= 2
			&& AcceptedRoll.TacticalPoints <= 8);
	const FMatchPlayState AfterRoll = Host->GetMatchSnapshot().Snapshot;
	TestTrue(TEXT("Successful roll creates authoritative CurrentAttack"),
		AfterRoll.bHasCurrentAttack);
	TestEqual(TEXT("CurrentAttack stores the authoritative Tactical Point"),
		AfterRoll.CurrentAttack.ActionPoint, AcceptedRoll.TacticalPoints);
	const TArray<uint8> AfterRollBytes = SerializeState(AfterRoll);
	const auto DuplicateRoll = Host->RollTacticalPoints(Attacker);
	TestFalse(TEXT("Duplicate roll is rejected"), DuplicateRoll.bSuccess);
	TestEqual(TEXT("Duplicate roll receives exact readiness error"),
		DuplicateRoll.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::TacticalPointRollNotReady);
	TestTrue(TEXT("Duplicate roll leaves state byte-identical"),
		AfterRollBytes == SerializeState(Host->GetMatchSnapshot().Snapshot));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchAttackTurnTrackerProjectionTest,
	"FMCodex.LocalPlay.MatchStartFlow.02.AttackTurnTrackerProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchAttackTurnTrackerProjectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	if (Host == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	if (!Host->StartNewLocalMatch(
		Demo.OpeningInput, Demo.SkillRuleSet, 0x613142).bSuccess)
	{
		return false;
	}
	const FMatchPlayState Entry = Host->GetMatchSnapshot().Snapshot;
	const EInitialTurnOrderPlayer First =
		Entry.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Second =
		First == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	const FFMCodexLocalMatchInteractionView EntryView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(
			Entry, Demo.SkillRuleSet);
	const FFMCodexUMGMatchScreenViewModel EntryPresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			EntryView, FFMCodexLocalMatchResolutionFeedback(), FString(),
			First);
	const FFMCodexUMGAttackTurnTrackerViewModel& CurrentTracker =
		EntryPresentation.Header.LeftAttackTurnTracker;
	const FFMCodexUMGAttackTurnTrackerViewModel& OtherTracker =
		EntryPresentation.Header.RightAttackTurnTracker;
	TestEqual(TEXT("Current-side tracker has three projected steps"),
		CurrentTracker.Steps.Num(), 3);
	TestEqual(TEXT("Other-side tracker has three projected steps"),
		OtherTracker.Steps.Num(), 3);
	TestEqual(TEXT("First opportunity is current"),
		CurrentTracker.Steps[0].State,
		EFMCodexUMGAttackTurnStepState::Current);
	TestEqual(TEXT("Second opportunity remains"),
		CurrentTracker.Steps[1].State,
		EFMCodexUMGAttackTurnStepState::Remaining);
	TestEqual(TEXT("Other side has no used/current opportunity yet"),
		OtherTracker.Steps[0].State,
		EFMCodexUMGAttackTurnStepState::Remaining);
	TestTrue(TEXT("Pre-roll Header projects no Tactical Point owner or fake zero"),
		!EntryPresentation.Header.bShowLeftTacticalPointChip
			&& !EntryPresentation.Header.bShowRightTacticalPointChip
			&& EntryPresentation.Header.LeftTacticalPoints == 0
			&& EntryPresentation.Header.RightTacticalPoints == 0
			&& EntryPresentation.Header.CurrentAttackerTacticalPointsLabel.IsEmpty());

	FMatchPlayState Switched = Entry;
	FPlayerRuntimeState& FirstState =
		First == EInitialTurnOrderPlayer::PlayerA
			? Switched.RuntimeState.PlayerAState
			: Switched.RuntimeState.PlayerBState;
	FirstState.UsedAttackCount = 1;
	Switched.RuntimeState.CurrentAttackingPlayer = Second;
	Switched.bHasCurrentAttack = false;
	const FFMCodexLocalMatchInteractionView SwitchedView =
		FFMCodexLocalMatchInteractionViewBuilder::Build(
			Switched, Demo.SkillRuleSet);
	const FFMCodexUMGMatchScreenViewModel SwitchedPresentation =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			SwitchedView, FFMCodexLocalMatchResolutionFeedback(), FString(),
			First);
	TestEqual(TEXT("Completed side projects one used opportunity"),
		SwitchedPresentation.Header.LeftAttackTurnTracker.Steps[0].State,
		EFMCodexUMGAttackTurnStepState::Used);
	TestEqual(TEXT("New attacker projects its first opportunity current"),
		SwitchedPresentation.Header.RightAttackTurnTracker.Steps[0].State,
		EFMCodexUMGAttackTurnStepState::Current);
	TestTrue(TEXT("Switch projects manual roll readiness for new attacker"),
		SwitchedPresentation.Header.bTacticalPointRollReady);
	TestTrue(TEXT("Switch clears stale Tactical Point ownership before the new roll"),
		!SwitchedPresentation.Header.bShowLeftTacticalPointChip
			&& !SwitchedPresentation.Header.bShowRightTacticalPointChip
			&& SwitchedPresentation.Header.LeftTacticalPoints == 0
			&& SwitchedPresentation.Header.RightTacticalPoints == 0);

	FString HeaderWidgetSource;
	TestTrue(TEXT("Header Widget source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"),
		HeaderWidgetSource));
	TestTrue(TEXT("Widget renders projected step states without runtime inference"),
		HeaderWidgetSource.Contains(TEXT("Step.State"))
			&& !HeaderWidgetSource.Contains(TEXT("UsedAttackCount"))
			&& !HeaderWidgetSource.Contains(TEXT("TotalAttackCount"))
			&& !HeaderWidgetSource.Contains(TEXT("CurrentAttackingPlayer")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchControlSurfaceFlowTest,
	"FMCodex.LocalPlay.ControlSurface.12.CrossEndToEnd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchControlSurfaceFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Real World has authoritative GameMode Host"), Host);
	TestNotNull(TEXT("Real World has local hot-seat Controller"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Initial controller view asks StartMatch"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::StartMatch);
	Controller->StartNewDemoMatch();
	TestTrue(TEXT("StartNewMatch succeeds through Controller"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Snapshot refresh asks TacticalPointRoll"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::TacticalPointRoll);
	TestTrue(TEXT("Initial Tactical Point roll is immediately ready"),
		Controller->GetInteractionView().bTacticalPointRollReady);

	Controller->FinishDeployment();
	TestFalse(TEXT("Rejected out-of-stage command is displayed"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Failure refresh preserves authoritative category"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::TacticalPointRoll);

	const EInitialTurnOrderPlayer FirstAttacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	TestTrue(TEXT("Representative Cross attack completes"),
		CompleteCrossAttack(*this, *Controller, TEXT("AttackOne")));
	TestTrue(TEXT("Completion exposes next-player roll readiness"),
		Controller->GetInteractionView().bTacticalPointRollReady);
	TestTrue(TEXT("Terminal completion switches the current attacker"),
		Controller->GetInteractionView().CurrentAttackingPlayer
			!= FirstAttacker);
	const FMatchPlayState AfterFirstCompletion =
		Host->GetMatchSnapshot().Snapshot;
	const FPlayerRuntimeState& CompletedAttackerState =
		FirstAttacker == EInitialTurnOrderPlayer::PlayerA
			? AfterFirstCompletion.RuntimeState.PlayerAState
			: AfterFirstCompletion.RuntimeState.PlayerBState;
	TestEqual(TEXT("Used count advances only at canonical completion"),
		CompletedAttackerState.UsedAttackCount, 1);
	const TArray<uint8> HandoffStateBytes = SerializeState(AfterFirstCompletion);
	const auto OldAttackerRoll = Host->RollTacticalPoints(FirstAttacker);
	TestFalse(TEXT("Completed old attacker cannot roll for the new attack"),
		OldAttackerRoll.bSuccess);
	TestEqual(TEXT("Old attacker receives exact ownership error after handoff"),
		OldAttackerRoll.ErrorCode,
		EFMCodexLocalMatchHostErrorCode::RequestingSideNotCurrentAttacker);
	TestTrue(TEXT("Rejected old-attacker roll leaves handoff state byte-identical"),
		HandoffStateBytes
			== SerializeState(Host->GetMatchSnapshot().Snapshot));
	const FFMCodexLocalMatchInteractionView& AfterCompletionView =
		Controller->GetInteractionView();
	TestEqual(TEXT("Completed side projects no current attack index"),
		FirstAttacker == EInitialTurnOrderPlayer::PlayerA
			? AfterCompletionView.PlayerACurrentAttackIndex
			: AfterCompletionView.PlayerBCurrentAttackIndex,
		0);
	TestEqual(TEXT("New attacker projects its first attack index"),
		FirstAttacker == EInitialTurnOrderPlayer::PlayerA
			? AfterCompletionView.PlayerBCurrentAttackIndex
			: AfterCompletionView.PlayerACurrentAttackIndex,
		1);
	TestEqual(TEXT("Rendered score A matches snapshot"),
		Controller->GetInteractionView().PlayerAScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Rendered score B matches snapshot"),
		Controller->GetInteractionView().PlayerBScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerBState.Score);
	TestTrue(TEXT("Terminal command exposes safe readable result summary"),
		Controller->GetLastDiagnostic().PresentationSummary == TEXT("GOAL")
			|| Controller->GetLastDiagnostic().PresentationSummary
				== TEXT("Attack complete - no goal"));

	FMatchPlayState EndedSnapshot = Host->GetMatchSnapshot().Snapshot;
	EndedSnapshot.RuntimeState.PlayerAState.UsedAttackCount =
		EndedSnapshot.RuntimeState.PlayerAState.TotalAttackCount;
	EndedSnapshot.RuntimeState.PlayerBState.UsedAttackCount =
		EndedSnapshot.RuntimeState.PlayerBState.TotalAttackCount;
	EndedSnapshot.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::None;
	EndedSnapshot.bHasCurrentAttack = false;
	const auto Rules = Host->GetSkillRuleSnapshot();
	const auto EndedView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		EndedSnapshot, Rules.Snapshot);
	TestTrue(TEXT("Canonical ended snapshot derives match ended"),
		EndedView.bMatchEnded);
	TestEqual(TEXT("Canonical ended snapshot derives MatchEnded category"),
		EndedView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::MatchEnded);
	TestFalse(TEXT("Canonical three-per-side boundary exposes no fourth roll"),
		EndedView.bTacticalPointRollReady);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchOneOnOnePresentationTest,
	"FMCodex.LocalPlay.ControlSurface.13.OneOnOneCategory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchOneOnOnePresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	TestNotNull(TEXT("OneOnOne Host exists"), Host);
	if (Host == nullptr)
	{
		return false;
	}

	FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FName ThroughBallSkillId(TEXT("Demo.Skill.ThroughBall"));
	for (TArray<FPlayerCardData>* Deck : {
		&Demo.OpeningInput.OpeningInput.PlayerADeck,
		&Demo.OpeningInput.OpeningInput.PlayerBDeck })
	{
		for (FPlayerCardData& Card : *Deck)
		{
			if (!Card.bIsGoalkeeper)
			{
				Card.AttackSkillIds = { ThroughBallSkillId };
			}
		}
	}
	FSkillRuleSnapshot Rule;
	Rule.SkillId = ThroughBallSkillId;
	Rule.SkillType = ESkillRuleType::ThroughBall;
	Rule.MinTriggerActionPoint = 2;
	Rule.MaxTriggerActionPoint = 8;
	Demo.SkillRuleSet.SkillRules = { Rule };

	const int32 Seed = FindSeedForRolls({ 3, 6, 1, 2 });
	TestTrue(TEXT("Deterministic OneOnOne seed exists"), Seed != INDEX_NONE);
	if (Seed == INDEX_NONE
		|| !Host->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, Seed).bSuccess
		|| !Host->BeginOrdinaryAttack(6).bSuccess)
	{
		return false;
	}

	const EInitialTurnOrderPlayer Attacker =
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB")
			: TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("OneOnOne fixture deploys a legal ordinary card"),
			DeployNextOrdinary(*Host, Demo.SkillRuleSet, PhysicalForward));
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const auto View = ViewFor(*Host, Demo.SkillRuleSet);
		TestTrue(TEXT("OneOnOne fixture finishes deployment"),
			Host->FinishDeployment(
				View.AttackSequence, View.ExpectedActingPlayer).bSuccess);
	}
	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectCarrier,
		EFMCodexLocalMatchInteractionCategory::SelectMarker,
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		TestTrue(TEXT("OneOnOne fixture submits canonical selection"),
			SubmitFirstHostSelection(*Host, Demo.SkillRuleSet, Category));
	}

	TestTrue(TEXT("OneOnOne fixture begins resolution"),
		Host->BeginResolutionSession().bSuccess);
	TestTrue(TEXT("OneOnOne fixture resolves BehindDefense route"),
		Host->ResolveInitialRoute().bSuccess);
	TestEqual(TEXT("Deterministic route is BehindDefense"),
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.ResolutionSession
			.ActualBranch.ThroughBall,
		EMatchPlayThroughBallActualBranch::BehindDefense);
	TestTrue(TEXT("OneOnOne fixture resolves P1 plan"),
		Host->ResolveThroughBallBehindDefenseP1DecisionOrPlan().bSuccess);
	TestTrue(TEXT("OneOnOne fixture resolves P1 Formula"),
		Host->ResolveThroughBallBehindDefenseP1Formula().bSuccess);
	const auto BeforeP2View = ViewFor(*Host, Demo.SkillRuleSet);
	const auto P2 = Host->ResolveThroughBallBehindDefenseP2Decision();
	TestTrue(TEXT("OneOnOne fixture resolves P2"), P2.bSuccess);
	TestEqual(TEXT("P2 authority requires OneOnOne"),
		P2.AuthoritativeResult.OrchestrationResult.QueryResult.Decision,
		EThroughBallBehindDefenseP2OutcomeDecision::OneOnOneRequired);

	const auto View = ViewFor(*Host, Demo.SkillRuleSet);
	TestEqual(TEXT("Snapshot-derived view asks OneOnOne choice"),
		View.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("OneOnOne is visibly a human interaction"),
		View.bHumanInteraction);
	TestEqual(TEXT("OneOnOne expected side is attacker"),
		View.ExpectedActingPlayer, Attacker);
	TestEqual(TEXT("OneOnOne exposes exactly ChipShot and DirectShot"),
		View.OneOnOneOptions.Num(), 2);
	const auto P2Feedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallBehindDefenseP2Decision"),
			P2,
			BeforeP2View,
			View);
	TestTrue(TEXT("P2 feedback exposes authoritative continuation"),
		P2Feedback.StepTitle.Contains(TEXT("P2"))
			&& P2Feedback.DecisionSummary.Contains(TEXT("One-on-One")));
	TestTrue(TEXT("P2 feedback exposes regenerated P1 Formula evidence"),
		P2Feedback.ComparisonEntries.Num() >= 2);

	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest Choice;
	Choice.RequestingSide = Attacker;
	Choice.Choice = EMatchPlayThroughBallOneOnOneShotChoice::DirectShot;
	TestTrue(TEXT("DirectShot choice succeeds"),
		Host->SubmitThroughBallOneOnOneShotChoice(Choice).bSuccess);
	const auto BeforePlanView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto DirectPlan =
		Host->ResolveThroughBallOneOnOneDirectShotPostRoutePlan();
	TestTrue(TEXT("DirectShot authoritative plan succeeds"),
		DirectPlan.bSuccess);
	const auto AfterPlanView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto PlanFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneDirectShotPostRoutePlan"),
			DirectPlan,
			BeforePlanView,
			AfterPlanView);
	TestTrue(TEXT("DirectShot plan shows shooter and goalkeeper evidence"),
		PlanFeedback.ComparisonEntries.Num() == 2
			&& PlanFeedback.ComparisonEntries[0].Contains(TEXT("Shooting"))
			&& PlanFeedback.ComparisonEntries[1].Contains(TEXT("OneOnOne"))
			&& PlanFeedback.ComparisonEntries[1].Contains(TEXT("activation")));
	TestTrue(TEXT("DirectShot plan exposes both OneOnOne D6 records"),
		PlanFeedback.DiceEntries.Num() >= 2);

	const auto DirectFormula =
		Host->ResolveThroughBallOneOnOneDirectShotFormula();
	TestTrue(TEXT("DirectShot authoritative Formula succeeds"),
		DirectFormula.bSuccess);
	const auto FormulaFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneDirectShotFormula"),
			DirectFormula,
			AfterPlanView,
			AfterPlanView);
	TestTrue(TEXT("DirectShot Formula feedback exposes final values"),
		FormulaFeedback.ComparisonEntries.Num() == 2
			&& FormulaFeedback.ComparisonEntries[0].Contains(TEXT("final"))
			&& FormulaFeedback.ComparisonEntries[1].Contains(
				TEXT("authoritative modifier")));
	TestTrue(TEXT("DirectShot Formula feedback exposes winner/reason"),
		FormulaFeedback.DecisionSummary.Contains(TEXT("Winner:"))
			&& FormulaFeedback.DecisionSummary.Contains(TEXT("Reason:")));

	const auto BeforeTerminalView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto Terminal = Host->ApplyThroughBallTerminalResolution();
	TestTrue(TEXT("DirectShot terminal application succeeds"),
		Terminal.bSuccess);
	const auto AfterTerminalView = ViewFor(*Host, Demo.SkillRuleSet);
	const auto TerminalFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ApplyThroughBallTerminalResolution"),
			Terminal,
			BeforeTerminalView,
			AfterTerminalView);
	TestTrue(TEXT("DirectShot terminal shows Goal or Miss"),
		TerminalFeedback.TerminalSummary == TEXT("RESULT: GOAL")
			|| TerminalFeedback.TerminalSummary == TEXT("RESULT: MISS"));
	TestTrue(TEXT("DirectShot terminal shows score and next attacker"),
		TerminalFeedback.ContinuationSummary.Contains(TEXT("Score:"))
			&& TerminalFeedback.ContinuationSummary.Contains(
				TEXT("Next attacker:")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchAutomaticHandoffPresentationContractTest,
	"FMCodex.LocalPlay.ControlSurface.14.AutomaticHandoffPresentationContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchAutomaticHandoffPresentationContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FString ControllerHeader;
	FString ControllerSource;
	FString ScreenHeader;
	FString ScreenSource;
	FString PresentationHeader;
	TestTrue(TEXT("Automatic handoff production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.h"),
			ControllerHeader)
		&& LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
			ControllerSource)
		&& LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
			ScreenHeader)
		&& LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
			ScreenSource)
		&& LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h"),
			PresentationHeader));
	const FString ProductionReadySurface = ControllerSource + ScreenSource
		+ PresentationHeader;
	TestTrue(TEXT("Production has no manual Ready or handoff presentation path"),
		!ProductionReadySurface.Contains(TEXT("AcknowledgeHotSeatHandoff"))
			&& !ProductionReadySurface.Contains(TEXT("IsAwaitingHotSeatHandoff"))
			&& !ProductionReadySurface.Contains(TEXT("RequestReady"))
			&& !ProductionReadySurface.Contains(TEXT("HotSeatHandoffOverlay"))
			&& !ProductionReadySurface.Contains(TEXT("PASS CONTROL"))
			&& !ProductionReadySurface.Contains(TEXT("Next Player:")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHotSeatAuthorityAndReadabilityTest,
	"FMCodex.LocalPlay.ControlSurface.15.HotSeatAuthorityAndReadability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHotSeatAuthorityAndReadabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Local Host exists"), Host);
	TestNotNull(TEXT("Local Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	TestTrue(TEXT("Start succeeds"), Controller->GetLastDiagnostic().bHostSuccess);
	Controller->FinishDeployment();
	TestFalse(TEXT("Out-of-stage command fails authoritatively"),
		Controller->GetLastDiagnostic().bHostSuccess);
	Controller->RollDemoTacticalPoints();
	TestTrue(TEXT("Tactical Point roll is immediately reachable"),
		Controller->GetLastDiagnostic().bHostSuccess);

	const auto& DeploymentView = Controller->GetInteractionView();
	int32 GroupedSlotCount = 0;
	bool bHasOrdinaryGroup = false;
	bool bHasGoalkeeperGroup = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: DeploymentView.DeploymentGroups)
	{
		GroupedSlotCount += Group.LegalSlotIds.Num();
		bHasGoalkeeperGroup |= Group.bGoalkeeper;
		bHasOrdinaryGroup |= !Group.bGoalkeeper;
	}
	TestTrue(TEXT("Deployment has readable ordinary groups"), bHasOrdinaryGroup);
	TestEqual(TEXT("Grouping preserves every legal deployment option"),
		GroupedSlotCount, DeploymentView.DeploymentOptions.Num());
	TestTrue(TEXT("Large legal option set remains complete"),
		DeploymentView.DeploymentOptions.Num() > 10);

	const FFMCodexLocalMatchDeploymentOption FirstOption =
		DeploymentView.DeploymentOptions[0];
	Controller->DeployOrdinary(FirstOption.CardId, FirstOption.SlotId);
	TestTrue(TEXT("First deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	bHasGoalkeeperGroup = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: Controller->GetInteractionView().DeploymentGroups)
	{
		bHasGoalkeeperGroup |= Group.bGoalkeeper;
	}
	TestTrue(TEXT("Defender deployment has readable goalkeeper group"),
		bHasGoalkeeperGroup);
	const auto BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FFMCodexLocalMatchDeploymentOption SecondOption =
		Controller->GetInteractionView().DeploymentOptions[0];
	Controller->DeployOrdinary(SecondOption.CardId, NAME_None);
	TestFalse(TEXT("Rejected deployment does not auto-handoff"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Rejected deployment leaves authority unchanged"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	const FFMCodexLocalMatchDeploymentGroup* GoalkeeperGroup =
		Controller->GetInteractionView().DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Group)
			{
				return Group.bGoalkeeper && !Group.LegalSlotIds.IsEmpty();
			});
	TestNotNull(TEXT("Defender goalkeeper remains available"), GoalkeeperGroup);
	if (GoalkeeperGroup == nullptr)
	{
		return false;
	}
	Controller->DeployGoalkeeper(GoalkeeperGroup->LegalSlotIds[0]);
	TestTrue(TEXT("Goalkeeper deployment remains directly reachable"),
		Controller->GetLastDiagnostic().bHostSuccess);

	FString ControllerSource;
	TestTrue(TEXT("Controller source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Full control surface remains scrollable"),
		ControllerSource.Contains(TEXT("SNew(SScrollBox)"))
			&& ControllerSource.Contains(TEXT("ScrollBarAlwaysVisible(true)")));
	TestTrue(TEXT("Deployment uses wrapped grouped slot controls"),
		ControllerSource.Contains(TEXT("DeploymentGroups"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)")));
	TestTrue(TEXT("Normal controls have no obsolete Ready gate"),
		!ControllerSource.Contains(TEXT("PASS CONTROL"))
			&& !ControllerSource.Contains(TEXT("AllowGameplayCommand")));
	TestTrue(TEXT("Choice controls carry explicit presentation headings"),
		ControllerSource.Contains(TEXT("Branch / Shot Type"))
			&& ControllerSource.Contains(TEXT("One-on-One Shot Type")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchHotSeatTwoSideFlowTest,
	"FMCodex.LocalPlay.ControlSurface.16.HotSeatTwoSideFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchHotSeatTwoSideFlowTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Controller = PlayableWorld.GetController();
	if (Controller == nullptr)
	{
		return false;
	}
	Controller->StartNewDemoMatch();
	Controller->RollDemoTacticalPoints();
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("Two-side flow deploys through handoffs"),
			DeployNextOrdinary(*Controller, PhysicalForward));
	}
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	TestEqual(TEXT("Attacker is prompted for Carrier"),
		Controller->GetInteractionView().InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectCarrier);
	const FName CarrierId =
		Controller->GetInteractionView().SelectionOptions[0].Id;
	TestEqual(TEXT("Carrier option shows attacker side"),
		Controller->GetInteractionView().SelectionOptions[0].Side,
		Attacker);
	Controller->SubmitCarrier(CarrierId);
	TestTrue(TEXT("Carrier selection succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Marker is assigned to the other player"),
		Controller->GetInteractionView().ExpectedActingPlayer,
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA);
	const FName MarkerId =
		Controller->GetInteractionView().SelectionOptions[0].Id;
	TestEqual(TEXT("Marker option shows defender side"),
		Controller->GetInteractionView().SelectionOptions[0].Side,
		Controller->GetInteractionView().ExpectedActingPlayer);
	Controller->SubmitMarker(MarkerId);
	TestTrue(TEXT("Marker selection succeeds for projected defender"),
		Controller->GetLastDiagnostic().bHostSuccess);

	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		AcknowledgeIfPending(*Controller);
		if (Category
			== EFMCodexLocalMatchInteractionCategory::SelectSkill)
		{
			TestTrue(TEXT("Skill choice includes readable family text"),
				Controller->GetInteractionView().SelectionOptions[0]
					.Label.Contains(TEXT("Cross")));
		}
		TestTrue(TEXT("Remaining selection stays reachable"),
			SubmitFirstSelection(*Controller, Category));
	}
	AcknowledgeIfPending(*Controller);
	Controller->SubmitBranchIntent(EMatchPlayElectiveBranchIntent::CrossHigh);
	TestTrue(TEXT("BranchIntent remains reachable"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Continue label identifies Begin Resolution"),
		Controller->GetInteractionView().ContinueActionLabel,
		FString(TEXT("Continue - Begin Resolution")));
	Controller->ContinueResolution();
	TestEqual(TEXT("Continue label identifies route resolution"),
		Controller->GetInteractionView().ContinueActionLabel,
		FString(TEXT("Continue - Resolve Route")));
	Controller->ContinueResolution();
	TestTrue(TEXT("Authoritative route D6 remains visible"),
		Controller->GetInteractionView().AcceptedRolls.Num() > 0);
	TestEqual(TEXT("Initial roll is grouped separately"),
		Controller->GetInteractionView().AcceptedRolls[0].Group,
		EFMCodexLocalMatchRollGroup::InitialRoute);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCardPitchPresentationTest,
	"FMCodex.LocalPlay.ControlSurface.17.CardPitchPresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCardPitchPresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	Controller->RollDemoTacticalPoints();
	TestTrue(TEXT("Presentation fixture begins attack"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const auto& InitialView = Controller->GetInteractionView();
	TestEqual(TEXT("Pitch has both canonical physical halves"),
		InitialView.PitchRegions.Num(), 2);
	int32 PresentedSlotCount = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: InitialView.PitchRegions)
	{
		PresentedSlotCount += Region.Slots.Num();
		TestFalse(TEXT("Pitch region has a player-readable label"),
			Region.Label.IsEmpty());
	}
	TestEqual(TEXT("Pitch regions preserve every canonical slot"),
		PresentedSlotCount,
		Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots.Num());

	int32 PresentedLegalSlots = 0;
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup = nullptr;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: InitialView.DeploymentGroups)
	{
		PresentedLegalSlots += Group.LegalSlots.Num();
		TestEqual(TEXT("Readable locations preserve raw legal slot count"),
			Group.LegalSlots.Num(), Group.LegalSlotIds.Num());
		if (!Group.bGoalkeeper && OrdinaryGroup == nullptr
			&& Group.Card.CardId.ToString().StartsWith(TEXT("Prototype."))
			&& Group.Card.SkillLabels.Contains(TEXT("Cross")))
		{
			OrdinaryGroup = &Group;
		}
	}
	TestEqual(TEXT("Card grouping preserves all legal options"),
		PresentedLegalSlots, InitialView.DeploymentOptions.Num());
	TestNotNull(TEXT("An ordinary readable card is available"), OrdinaryGroup);
	if (OrdinaryGroup == nullptr || OrdinaryGroup->LegalSlots.IsEmpty())
	{
		return false;
	}
	TestEqual(TEXT("Card view preserves canonical CardId"),
		OrdinaryGroup->Card.CardId, OrdinaryGroup->CardId);
	TestFalse(TEXT("Canonical card has a player-facing display name"),
		OrdinaryGroup->Card.DisplayLabel.IsEmpty());
	TestFalse(TEXT("Card shows canonical role"),
		OrdinaryGroup->Card.PositionLabel.IsEmpty());
	TestTrue(TEXT("Card shows compact authoritative attributes"),
		OrdinaryGroup->Card.AttributeSummary.Contains(TEXT("SHO"))
			&& OrdinaryGroup->Card.AttributeSummary.Contains(TEXT("PAS")));
	TestTrue(TEXT("Demo ordinary card shows readable Cross skill"),
		OrdinaryGroup->Card.SkillLabels.Contains(TEXT("Cross")));

	const FName DeployedCardId = OrdinaryGroup->CardId;
	const FName DeployedSlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const EMatchPlayNeutralSlotSide ExpectedHalf =
		OrdinaryGroup->LegalSlots[0].NeutralSide;
	const EMatchPlayRelativeDeploymentZone ExpectedZone =
		OrdinaryGroup->LegalSlots[0].RelativeZone;
	Controller->DeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Readable deployment command succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);

	int32 PresentedPlacements = 0;
	bool bFoundCanonicalCard = false;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			if (!Slot.bOccupied)
			{
				continue;
			}
			++PresentedPlacements;
			const FFMCodexLocalMatchCardView& Card = Slot.Card;
			if (Card.CardId == DeployedCardId)
			{
				bFoundCanonicalCard = Card.bDeployed
					&& Slot.SlotId == DeployedSlotId
					&& Card.SlotId == DeployedSlotId
					&& Card.NeutralSide == ExpectedHalf
					&& Card.RelativeZone == ExpectedZone;
			}
		}
	}
	TestEqual(TEXT("Exactly one deployed card appears on pitch"),
		PresentedPlacements, 1);
	TestTrue(TEXT("Deployed card maps to canonical half, zone and slot"),
		bFoundCanonicalCard);

	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const int32 ScoreABefore = Controller->GetInteractionView().PlayerAScore;
	const int32 ScoreBBefore = Controller->GetInteractionView().PlayerBScore;
	const auto CategoryBefore =
		Controller->GetInteractionView().InteractionCategory;
	TestFalse(TEXT("Next side still has canonical deployment options"),
		Controller->GetInteractionView().DeploymentOptions.IsEmpty());
	if (Controller->GetInteractionView().DeploymentOptions.IsEmpty())
	{
		return false;
	}
	const FFMCodexLocalMatchDeploymentOption NextOption =
		Controller->GetInteractionView().DeploymentOptions[0];
	Controller->DeployOrdinary(NextOption.CardId, NAME_None);
	TestFalse(TEXT("Invalid next-side deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestEqual(TEXT("Rejected command has readable result summary"),
		Controller->GetLastDiagnostic().PresentationSummary,
		FString(TEXT("Command rejected")));
	TestTrue(TEXT("Rejected command leaves authority byte-identical"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Rejected command leaves score A unchanged"),
		Controller->GetInteractionView().PlayerAScore, ScoreABefore);
	TestEqual(TEXT("Rejected command leaves score B unchanged"),
		Controller->GetInteractionView().PlayerBScore, ScoreBBefore);
	TestEqual(TEXT("Rejected command leaves interaction unchanged"),
		Controller->GetInteractionView().InteractionCategory, CategoryBefore);

	int32 PlacementsAfterFailure = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			PlacementsAfterFailure += Slot.bOccupied ? 1 : 0;
		}
	}
	TestEqual(TEXT("Rejected command causes no optimistic card movement"),
		PlacementsAfterFailure, PresentedPlacements);

	bool bHasReadableGoalkeeper = false;
	for (const FFMCodexLocalMatchDeploymentGroup& Group
		: Controller->GetInteractionView().DeploymentGroups)
	{
		if (Group.bGoalkeeper)
		{
			bHasReadableGoalkeeper = Group.Card.bGoalkeeper
				&& Group.Card.PositionLabel.Contains(TEXT("Goalkeeper"))
				&& Group.Card.GoalkeeperAttributeSummary.Contains(TEXT("HAN"))
				&& !Group.Card.bGoalkeeperUsedThisMatch;
			break;
		}
	}
	TestTrue(TEXT("Defender goalkeeper is visibly distinct and authoritative"),
		bHasReadableGoalkeeper);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCandidatePresentationTest,
	"FMCodex.LocalPlay.ControlSurface.18.CandidatePresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCandidatePresentationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->StartNewDemoMatch();
	Controller->RollDemoTacticalPoints();
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FString PhysicalForward =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestTrue(TEXT("Candidate fixture deploys through readable options"),
			DeployNextOrdinary(*Controller, PhysicalForward));
	}
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);
	Controller->FinishDeployment();
	AcknowledgeIfPending(*Controller);

	const auto& CarrierView = Controller->GetInteractionView();
	TestEqual(TEXT("Fixture reaches Carrier"), CarrierView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectCarrier);
	TestFalse(TEXT("Carrier candidates exist"),
		CarrierView.SelectionOptions.IsEmpty());
	if (CarrierView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const FFMCodexLocalMatchSelectionOption Carrier =
		CarrierView.SelectionOptions[0];
	TestTrue(TEXT("Carrier candidate includes its card view"),
		Carrier.bHasCard);
	TestEqual(TEXT("Carrier button and card share canonical identity"),
		Carrier.RelatedCardId, Carrier.Id);
	TestEqual(TEXT("Carrier card keeps expected acting side"),
		Carrier.Card.Side, CarrierView.ExpectedActingPlayer);
	TestTrue(TEXT("Carrier card is visibly deployed"),
		Carrier.Card.bDeployed);
	TestEqual(TEXT("Header attacker derives from authority"),
		CarrierView.CurrentAttackingPlayer,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer);
	TestEqual(TEXT("Header score A derives from authority"),
		CarrierView.PlayerAScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerAState.Score);
	TestEqual(TEXT("Header score B derives from authority"),
		CarrierView.PlayerBScore,
		Host->GetMatchSnapshot().Snapshot.RuntimeState.PlayerBState.Score);

	Controller->SubmitCarrier(Carrier.Id);
	AcknowledgeIfPending(*Controller);
	const auto& MarkerView = Controller->GetInteractionView();
	TestFalse(TEXT("Marker candidates exist"),
		MarkerView.SelectionOptions.IsEmpty());
	if (MarkerView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const auto Marker = MarkerView.SelectionOptions[0];
	TestTrue(TEXT("Marker candidate includes matching deployed card"),
		Marker.bHasCard && Marker.RelatedCardId == Marker.Id
			&& Marker.Card.CardId == Marker.Id && Marker.Card.bDeployed);
	Controller->SubmitMarker(Marker.Id);
	AcknowledgeIfPending(*Controller);

	const auto& SkillView = Controller->GetInteractionView();
	TestEqual(TEXT("Fixture reaches Skill"), SkillView.InteractionCategory,
		EFMCodexLocalMatchInteractionCategory::SelectSkill);
	TestFalse(TEXT("Skill choices exist"), SkillView.SelectionOptions.IsEmpty());
	if (SkillView.SelectionOptions.IsEmpty())
	{
		return false;
	}
	const auto Skill = SkillView.SelectionOptions[0];
	TestTrue(TEXT("Skill label maps canonical enum to readable text"),
		Skill.Label.Contains(TEXT("Cross")));
	TestTrue(TEXT("Skill choice shows the carrier using it"),
		Skill.bHasCard && Skill.RelatedCardId == Carrier.Id
			&& Skill.Card.CardId == Carrier.Id);
	TestEqual(TEXT("Expected actor remains snapshot-derived"),
		SkillView.ExpectedActingPlayer, Attacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPresentationShellContractTest,
	"FMCodex.LocalPlay.ControlSurface.19.PresentationShellContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPresentationShellContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	if (Host == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	TestTrue(TEXT("Ended-view fixture starts"),
		Host->StartNewLocalMatch(Demo.OpeningInput, Demo.SkillRuleSet).bSuccess);
	FMatchPlayState EndedState = Host->GetMatchSnapshot().Snapshot;
	EndedState.RuntimeState.PlayerAState.Score = 2;
	EndedState.RuntimeState.PlayerBState.Score = 1;
	EndedState.RuntimeState.PlayerAState.UsedAttackCount =
		EndedState.RuntimeState.PlayerAState.TotalAttackCount;
	EndedState.RuntimeState.PlayerBState.UsedAttackCount =
		EndedState.RuntimeState.PlayerBState.TotalAttackCount;
	EndedState.bHasCurrentAttack = false;
	const auto EndedView = FFMCodexLocalMatchInteractionViewBuilder::Build(
		EndedState, Demo.SkillRuleSet);
	TestTrue(TEXT("Ended shell is snapshot-derived"), EndedView.bMatchEnded);
	TestEqual(TEXT("Ended shell retains final score A"),
		EndedView.PlayerAScore, 2);
	TestEqual(TEXT("Ended shell retains final score B"),
		EndedView.PlayerBScore, 1);
	TestEqual(TEXT("Ended shell exposes canonical winner"),
		EndedView.MatchResult, EMatchResultType::HomeWin);
	TestEqual(TEXT("Ended shell exposes no human actor"),
		EndedView.ExpectedActingPlayer, EInitialTurnOrderPlayer::None);

	TestEqual(TEXT("LongShot readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::LongShot), FString(TEXT("Long Shot")));
	TestEqual(TEXT("CutInside readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::CutInsideShot), FString(TEXT("Cut Inside")));
	TestEqual(TEXT("PassControl readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::PassControl), FString(TEXT("Pass Control")));
	TestEqual(TEXT("ThroughBall readable mapping"),
		FFMCodexLocalMatchInteractionViewBuilder::ToString(
			ESkillRuleType::ThroughBall), FString(TEXT("Through Ball")));

	FString ControllerSource;
	TestTrue(TEXT("Presentation source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Player-readable header and field shell exist"),
		ControllerSource.Contains(TEXT("FMCODEX LOCAL MATCH"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION")));
	TestTrue(TEXT("Cards and legal locations use derived view types"),
		ControllerSource.Contains(TEXT("MakeCardPanel"))
			&& ControllerSource.Contains(TEXT("Group.LegalSlots")));
	TestTrue(TEXT("Dice groups include canonical OneOnOne presentation"),
		ControllerSource.Contains(TEXT("Accepted Dice - One-on-One")));
	TestTrue(TEXT("Match-ended shell offers explicit final result/restart"),
		ControllerSource.Contains(TEXT("FINAL RESULT"))
			&& ControllerSource.Contains(TEXT("Start New Local Match")));
	TestTrue(TEXT("Interaction remains scrollable for large option sets"),
		ControllerSource.Contains(TEXT("SNew(SScrollBox)"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)")));
	TestFalse(TEXT("Presentation source has no direct authoritative State write"),
		ControllerSource.Contains(TEXT("CurrentAttack.DeploymentPlacements.Add"))
			|| ControllerSource.Contains(TEXT("RuntimeState.PlayerAState.Score ="))
			|| ControllerSource.Contains(TEXT("RuntimeState.PlayerBState.Score =")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchFeedbackSemanticShapesTest,
	"FMCodex.LocalPlay.ControlSurface.20.FeedbackSemanticShapes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchFeedbackSemanticShapesTest::RunTest(
	const FString& Parameters)
{
	FFMCodexLocalMatchInteractionView View;
	View.ActionLabel = TEXT("Through Ball");
	View.ActualBranchLabel = TEXT("Anti-Offside");

	FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult AntiOffside;
	AntiOffside.AuthoritativeResult.OrchestrationResult.OutcomeResult.Decision =
		EThroughBallAntiOffsideOutcomeDecision::Offside;
	const auto AntiOffsideFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallAntiOffsideDecision"),
			AntiOffside, View, View);
	TestTrue(TEXT("AntiOffside uses authoritative decision label"),
		AntiOffsideFeedback.DecisionSummary.Contains(TEXT("Offside")));
	TestTrue(TEXT("AntiOffside retains authoritative route context"),
		AntiOffsideFeedback.RouteSummary.Contains(TEXT("Anti-Offside")));

	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult Chip;
	Chip.AuthoritativeResult.OrchestrationResult.QueryResult.Decision =
		EThroughBallOneOnOneChipShotOutcomeDecision::Goal;
	const auto ChipFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ResolveThroughBallOneOnOneChipShotDecision"),
			Chip, View, View);
	TestEqual(TEXT("ChipShot exposes authoritative Goal"),
		ChipFeedback.DecisionSummary, FString(TEXT("Goal")));
	TestTrue(TEXT("ChipShot never fabricates goalkeeper Formula evidence"),
		ChipFeedback.ComparisonEntries.IsEmpty());
	TestTrue(TEXT("ChipShot explicitly identifies non-Formula shape"),
		ChipFeedback.StepSummary.Contains(TEXT("without Formula")));

	FFMCodexLocalMatchApplyShotTerminalResolutionResult ImmediateMiss;
	auto& ShotTerminal =
		ImmediateMiss.AuthoritativeResult.OrchestrationResult;
	ShotTerminal.TerminalSource =
		EMatchPlayShotTerminalSource::LongShotDirectShotImmediateMiss;
	ShotTerminal.CompletionResult.bSuccess = true;
	ShotTerminal.CompletionResult.AfterState.RuntimeState.PlayerAState.Score = 0;
	ShotTerminal.CompletionResult.AfterState.RuntimeState.PlayerBState.Score = 0;
	ShotTerminal.CompletionResult.NextAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;
	ShotTerminal.CompletionResult.OpportunityResolveResult.bSuccess = true;
	const auto ImmediateMissFeedback =
		FFMCodexLocalMatchResolutionFeedbackBuilder::Build(
			TEXT("ApplyShotTerminalResolution"),
			ImmediateMiss, View, FFMCodexLocalMatchInteractionView());
	TestEqual(TEXT("ImmediateMiss terminal semantic is direct"),
		ImmediateMissFeedback.TerminalSummary,
		FString(TEXT("RESULT: IMMEDIATE MISS")));
	TestTrue(TEXT("Non-Formula terminal has no fake comparison"),
		ImmediateMissFeedback.ComparisonEntries.IsEmpty());
	TestTrue(TEXT("Completion feedback exposes score/opportunity/next actor"),
		ImmediateMissFeedback.ContinuationSummary.Contains(TEXT("Score:"))
			&& ImmediateMissFeedback.ContinuationSummary.Contains(
				TEXT("Opportunity consumed: yes"))
			&& ImmediateMissFeedback.ContinuationSummary.Contains(
				TEXT("Next attacker: Player B")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchRejectedFeedbackAuthorityTest,
	"FMCodex.LocalPlay.ControlSurface.21.RejectedFeedbackAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchRejectedFeedbackAuthorityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->StartNewDemoMatch();
	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const auto CategoryBefore =
		Controller->GetInteractionView().InteractionCategory;
	Controller->FinishDeployment();
	const auto& Rejected = Controller->GetResolutionFeedback();
	TestTrue(TEXT("Rejected feedback is explicit"),
		Rejected.bVisible && Rejected.bRejected
			&& Rejected.StepTitle == TEXT("Command Rejected"));
	TestEqual(TEXT("Rejected feedback preserves typed command"),
		Rejected.CommandName, FString(TEXT("FinishDeployment")));
	TestTrue(TEXT("Rejected feedback preserves diagnostic reason"),
		!Rejected.ErrorMessage.IsEmpty());
	TestTrue(TEXT("Rejected feedback has no false accepted evidence"),
		Rejected.DiceEntries.IsEmpty()
			&& Rejected.ComparisonEntries.IsEmpty()
			&& Rejected.TerminalSummary.IsEmpty());
	TestTrue(TEXT("Rejected feedback leaves State byte-identical"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestEqual(TEXT("Feedback does not drive InteractionCategory"),
		Controller->GetInteractionView().InteractionCategory, CategoryBefore);

	Controller->RefreshPresentation();
	TestTrue(TEXT("Presentation refresh remains authority-neutral"),
		BeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchFeedbackAuthorityBoundaryTest,
	"FMCodex.LocalPlay.ControlSurface.22.FeedbackAuthorityBoundary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchFeedbackAuthorityBoundaryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FString FeedbackSource;
	FString ControllerSource;
	TestTrue(TEXT("Feedback production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchResolutionFeedback.cpp"),
		FeedbackSource));
	TestTrue(TEXT("Controller production source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	const FString PresentationSource = FeedbackSource + ControllerSource;
	TestFalse(TEXT("Presentation never calls FormulaResolver"),
		PresentationSource.Contains(TEXT("ResolveFormula(")));
	TestFalse(TEXT("Presentation never calculates goalkeeper half"),
		PresentationSource.Contains(TEXT("CalculateGoalkeeperHalf(")));
	TestFalse(TEXT("Presentation contains no route threshold table"),
		PresentationSource.Contains(TEXT("Roll <="))
			|| PresentationSource.Contains(TEXT("RawD6 <="))
			|| PresentationSource.Contains(TEXT("Roll >=")));
	TestFalse(TEXT("Feedback contains no RNG or authoritative history cache"),
		FeedbackSource.Contains(TEXT("RandRange"))
			|| FeedbackSource.Contains(TEXT("LastGameplayD6"))
			|| FeedbackSource.Contains(TEXT("CurrentAttackRolls"))
			|| FeedbackSource.Contains(TEXT("FormulaHistoryAuthority")));
	TestTrue(TEXT("Feedback panel is visibly separate"),
		ControllerSource.Contains(TEXT("MakeFeedbackPanel"))
			&& ControllerSource.Contains(TEXT("RESOLUTION"))
			&& ControllerSource.Contains(TEXT("COMMAND REJECTED")));

	const int32 ContinueStart = ControllerSource.Find(
		TEXT("void AFMCodexLocalMatchPlayerController::ContinueResolution"));
	const int32 ContinueEnd = ControllerSource.Find(
		TEXT("void AFMCodexLocalMatchPlayerController::RebuildControlSurface"));
	const FString ContinueBody = ContinueStart != INDEX_NONE
		&& ContinueEnd > ContinueStart
			? ControllerSource.Mid(
				ContinueStart, ContinueEnd - ContinueStart)
			: FString();
	TestTrue(TEXT("Feedback never selects the next gameplay command"),
		!ContinueBody.IsEmpty()
			&& !ContinueBody.Contains(TEXT("ResolutionFeedback")));
	TestTrue(TEXT("Stage 5.7 card/pitch shell remains present"),
		ControllerSource.Contains(TEXT("FMCODEX LOCAL MATCH"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("MakeCardPanel")));
	TestTrue(TEXT("Obsolete handoff blocker is absent"),
		!ControllerSource.Contains(TEXT("PASS CONTROL"))
			&& !ControllerSource.Contains(TEXT("AllowGameplayCommand")));
	return true;
}

namespace FMCodexLocalMatchFullFamilyTests
{
	struct FFamilyExpectation
	{
		ESkillRuleType SkillType = ESkillRuleType::None;
		const TCHAR* SkillId = TEXT("");
		const TCHAR* ReadableLabel = TEXT("");
		const TCHAR* PlayerACardId = TEXT("");
		const TCHAR* PlayerBCardId = TEXT("");
		int32 ActionPoint = 0;
	};

	TArray<FFamilyExpectation> FamilyExpectations()
	{
		return {
			{ ESkillRuleType::Cross, TEXT("Canonical.Skill.Cross.4.6"),
				TEXT("Cross"), TEXT("Prototype.Arsenal.BukayoSaka"),
				TEXT("Prototype.ManchesterCity.RayanAitNouri"), 6 },
			{ ESkillRuleType::LongShot, TEXT("Canonical.Skill.LongShot.3.5"),
				TEXT("Long Shot"), TEXT("Prototype.Arsenal.ViktorGyokeres"),
				TEXT("Prototype.ManchesterCity.PhilFoden"), 4 },
			{ ESkillRuleType::CutInsideShot,
				TEXT("Canonical.Skill.CutInsideShot.4.5"), TEXT("Cut Inside"),
				TEXT("Prototype.Arsenal.GabrielMartinelli"),
				TEXT("Prototype.ManchesterCity.OmarMarmoush"), 4 },
			{ ESkillRuleType::PassControl, TEXT("Canonical.Skill.PassControl.6.8"),
				TEXT("Pass Control"), TEXT("Prototype.Arsenal.MartinOdegaard"),
				TEXT("Prototype.ManchesterCity.BernardoSilva"), 6 },
			{ ESkillRuleType::ThroughBall, TEXT("Canonical.Skill.ThroughBall.5.6"),
				TEXT("Through Ball"), TEXT("Prototype.Arsenal.MartinOdegaard"),
				TEXT("Prototype.ManchesterCity.Rodri"), 5 }
		};
	}

	FName FamilyCardId(
		const FFamilyExpectation& Family,
		const EInitialTurnOrderPlayer Side)
	{
		return FName(Side == EInitialTurnOrderPlayer::PlayerA
			? Family.PlayerACardId : Family.PlayerBCardId);
	}

	EInitialTurnOrderPlayer OtherSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	bool Fail(
		FAutomationTestBase& Test,
		const FString& Family,
		const FString& Message)
	{
		Test.AddError(Family + TEXT(": ") + Message);
		return false;
	}

	bool DeployParticipants(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFamilyExpectation& Family,
		const EInitialTurnOrderPlayer Attacker)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
		const FName CarrierCardId = FamilyCardId(Family, Attacker);
		const FString PhysicalForward =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("NearB") : TEXT("NearA");
		bool bCarrierDeployed = false;
		bool bCrossRunnerDeployed = false;
		bool bDefenderGoalkeeperDeployed = false;
		const bool bRequiresGoalkeeper =
			Family.SkillType == ESkillRuleType::ThroughBall;
		const int32 RequiredDeployments = 5;

		for (int32 Step = 0; Step < RequiredDeployments; ++Step)
		{
			AcknowledgeIfPending(Controller);
			const auto& View = Controller.GetInteractionView();
			if (View.InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::Deploy)
			{
				return Fail(Test, FamilyLabel,
					TEXT("deployment interaction was not available"));
			}

			FFMCodexLocalMatchDeploymentOption SelectedOption;
			bool bFoundOption = false;
			if (bRequiresGoalkeeper
				&& View.CurrentLegalDeploymentSide == Defender
				&& !bDefenderGoalkeeperDeployed)
			{
				for (const auto& Option : View.DeploymentOptions)
				{
					if (Option.bGoalkeeper)
					{
						SelectedOption = Option;
						bFoundOption = true;
						break;
					}
				}
			}

			if (!bFoundOption)
			{
				const FName RequiredCrossRunnerId =
					Attacker == EInitialTurnOrderPlayer::PlayerA
						? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
						: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
				const bool bRequireCrossRunner =
					Family.SkillType == ESkillRuleType::Cross
					&& View.CurrentLegalDeploymentSide == Attacker
					&& bCarrierDeployed && !bCrossRunnerDeployed;
				FName PreferredFamilyCardId = NAME_None;
				for (const FFMCodexLocalMatchDeploymentGroup& Group
					: View.DeploymentGroups)
				{
					const bool bHasMatchingSlot = Group.LegalSlots.ContainsByPredicate(
						[&PhysicalForward](const FFMCodexLocalMatchSlotView& Slot)
						{
							return Slot.SlotId.ToString().Contains(PhysicalForward);
						});
					const bool bHasUsableFamily = Group.Card.Skills.ContainsByPredicate(
						[&Family, &View](const FFMCodexLocalMatchCardView::FSkill& Skill)
						{
							return Skill.CanonicalLabel == Family.ReadableLabel
								&& View.ActionPoint >= Skill.MinTriggerActionPoint
								&& View.ActionPoint <= Skill.MaxTriggerActionPoint;
						});
					if (!Group.bGoalkeeper && bHasMatchingSlot && bHasUsableFamily)
					{
						PreferredFamilyCardId = Group.CardId;
						break;
					}
				}
				for (const auto& Option : View.DeploymentOptions)
				{
					if (Option.bGoalkeeper
						|| !Option.SlotId.ToString().Contains(PhysicalForward))
					{
						continue;
					}
					if (View.CurrentLegalDeploymentSide == Attacker
						&& !bCarrierDeployed
						&& Option.CardId != CarrierCardId)
					{
						continue;
					}
					if (bRequireCrossRunner
						&& Option.CardId != RequiredCrossRunnerId)
					{
						continue;
					}
					if (!bRequireCrossRunner
						&& (View.CurrentLegalDeploymentSide != Attacker
							|| bCarrierDeployed)
						&& !PreferredFamilyCardId.IsNone()
						&& Option.CardId != PreferredFamilyCardId)
					{
						continue;
					}
					SelectedOption = Option;
					bFoundOption = true;
					break;
				}
			}

			if (!bFoundOption)
			{
				return Fail(Test, FamilyLabel,
					TEXT("no required normal-demo deployment option was available"));
			}

			if (SelectedOption.bGoalkeeper)
			{
				Controller.DeployGoalkeeper(SelectedOption.SlotId);
				bDefenderGoalkeeperDeployed = true;
			}
			else
			{
				Controller.DeployOrdinary(
					SelectedOption.CardId, SelectedOption.SlotId);
				bCarrierDeployed = bCarrierDeployed
					|| SelectedOption.CardId == CarrierCardId;
				bCrossRunnerDeployed = bCrossRunnerDeployed
					|| (Family.SkillType == ESkillRuleType::Cross
						&& SelectedOption.CardId
							== (Attacker == EInitialTurnOrderPlayer::PlayerA
								? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
								: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"))));
			}

			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("normal-demo deployment command was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}

		if (!bCarrierDeployed
			|| (bRequiresGoalkeeper && !bDefenderGoalkeeperDeployed))
		{
			return Fail(Test, FamilyLabel,
				TEXT("required carrier/goalkeeper deployment was incomplete"));
		}

		for (int32 SideIndex = 0; SideIndex < 2; ++SideIndex)
		{
			AcknowledgeIfPending(Controller);
			Controller.FinishDeployment();
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("Finish Deployment was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}
		return true;
	}

	bool SubmitRequiredSelections(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFamilyExpectation& Family,
		const EInitialTurnOrderPlayer Attacker)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		const FName CarrierCardId = FamilyCardId(Family, Attacker);
		const FName SkillId(Family.SkillId);

		AcknowledgeIfPending(Controller);
		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectCarrier)
		{
			return Fail(Test, FamilyLabel, TEXT("Carrier was not requested"));
		}
		bool bCarrierVisible = false;
		for (const auto& Option
			: Controller.GetInteractionView().SelectionOptions)
		{
			bCarrierVisible = bCarrierVisible || Option.Id == CarrierCardId;
		}
		if (!bCarrierVisible)
		{
			return Fail(Test, FamilyLabel,
				TEXT("the family carrier was not an authoritative legal option"));
		}
		Controller.SubmitCarrier(CarrierCardId);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Carrier submission failed"));
		}

		AcknowledgeIfPending(Controller);
		if (!SubmitFirstSelection(
			Controller, EFMCodexLocalMatchInteractionCategory::SelectMarker))
		{
			return Fail(Test, FamilyLabel, TEXT("Marker submission failed"));
		}

		AcknowledgeIfPending(Controller);
		const auto& SkillView = Controller.GetInteractionView();
		if (SkillView.InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::SelectSkill)
		{
			return Fail(Test, FamilyLabel, TEXT("Skill was not requested"));
		}
		bool bSkillVisible = false;
		bool bReadableLabelVisible = false;
		for (const auto& Option : SkillView.SelectionOptions)
		{
			if (Option.Id == SkillId)
			{
				bSkillVisible = true;
				bReadableLabelVisible = Option.Label.Contains(Family.ReadableLabel);
			}
		}
		if (!bSkillVisible || !bReadableLabelVisible)
		{
			return Fail(Test, FamilyLabel,
				TEXT("SkillId or readable family label was not presented"));
		}
		Controller.SubmitSkill(SkillId);
		if (!Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Skill submission failed"));
		}

		FString SelectionTrace = FString::Printf(TEXT("after-skill=%d"),
			static_cast<int32>(Controller.GetInteractionView().InteractionCategory));
		for (int32 Guard = 0; Guard < 4; ++Guard)
		{
			AcknowledgeIfPending(Controller);
			const auto& View = Controller.GetInteractionView();
			if (View.InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::SelectRunner
				&& View.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectHelper)
			{
				break;
			}
			if (!View.SelectionOptions.IsEmpty())
			{
				SelectionTrace += FString::Printf(TEXT(", choose=%s"),
					*View.SelectionOptions[0].Id.ToString());
				if (!SubmitFirstSelection(Controller, View.InteractionCategory))
				{
					return Fail(Test, FamilyLabel,
						TEXT("Runner/Helper submission failed"));
				}
			}
			else if (View.bCanResolveNoLegalChoice)
			{
				Controller.ResolveNoLegalCurrentSelection();
			}
			else if (View.bCanDecline)
			{
				Controller.DeclineCurrentSelection();
			}
			else
			{
				return Fail(Test, FamilyLabel,
					TEXT("Runner/Helper stage had no actionable operation"));
			}
			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("Runner/Helper operation was rejected"));
			}
			SelectionTrace += FString::Printf(TEXT(", after-step=%d"),
				static_cast<int32>(
					Controller.GetInteractionView().InteractionCategory));
		}

		AcknowledgeIfPending(Controller);
		const auto& ChoiceView = Controller.GetInteractionView();
		if (Family.SkillType == ESkillRuleType::LongShot
			|| Family.SkillType == ESkillRuleType::CutInsideShot)
		{
			if (ChoiceView.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::DirectShot)
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::DeadCorner))
			{
				return Fail(Test, FamilyLabel,
					TEXT("DirectShot/DeadCorner choices were incomplete"));
			}
			Controller.SubmitBranchIntent(
				EMatchPlayElectiveBranchIntent::DirectShot);
		}
		else if (Family.SkillType == ESkillRuleType::Cross)
		{
			if (ChoiceView.InteractionCategory
					!= EFMCodexLocalMatchInteractionCategory::SelectBranchIntent
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::CrossHigh)
				|| !ChoiceView.BranchIntentOptions.Contains(
					EMatchPlayElectiveBranchIntent::CrossLow))
			{
				return Fail(Test, FamilyLabel, FString::Printf(
					TEXT("Cross High/Low choices were incomplete (category %d, option count %d, %s)"),
					static_cast<int32>(ChoiceView.InteractionCategory),
					ChoiceView.BranchIntentOptions.Num(), *SelectionTrace));
			}
			Controller.SubmitBranchIntent(
				EMatchPlayElectiveBranchIntent::CrossHigh);
		}

		if ((Family.SkillType == ESkillRuleType::LongShot
				|| Family.SkillType == ESkillRuleType::CutInsideShot
				|| Family.SkillType == ESkillRuleType::Cross)
			&& !Controller.GetLastDiagnostic().bHostSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("Branch choice was rejected"));
		}

		if (Controller.GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			return Fail(Test, FamilyLabel,
				TEXT("normal selections did not reach Begin Resolution"));
		}
		return true;
	}

	bool CompleteNormalDemoFamilyAttack(
		FAutomationTestBase& Test,
		AFMCodexLocalMatchHostGameMode& Host,
		AFMCodexLocalMatchPlayerController& Controller,
		const FFMCodexLocalMatchDemoConfiguration& Demo,
		const FFamilyExpectation& Family,
		const int32 Seed)
	{
		using namespace FMCodexLocalMatchControlSurfaceTests;
		const FString FamilyLabel(Family.ReadableLabel);
		if (!Host.StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, Seed).bSuccess)
		{
			return Fail(Test, FamilyLabel,
				TEXT("normal production demo configuration did not start"));
		}
		Controller.RefreshPresentation();
		AcknowledgeIfPending(Controller);
		if (!Host.BeginOrdinaryAttack(Family.ActionPoint).bSuccess)
		{
			return Fail(Test, FamilyLabel, TEXT("attack did not begin"));
		}
		Controller.RefreshPresentation();

		const EInitialTurnOrderPlayer Attacker =
			Controller.GetInteractionView().CurrentAttackingPlayer;
		if (!DeployParticipants(Test, Controller, Family, Attacker)
			|| !SubmitRequiredSelections(Test, Controller, Family, Attacker))
		{
			return false;
		}

		bool bSawResolvedRoute = false;
		bool bSawOneOnOne = false;
		for (int32 Guard = 0;
			Guard < 12 && Controller.GetInteractionView().bCurrentAttackActive;
			++Guard)
		{
			const auto& View = Controller.GetInteractionView();
			bSawResolvedRoute = bSawResolvedRoute
				|| !View.ActualBranchLabel.IsEmpty();
			if (View.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
			{
				if (Family.SkillType != ESkillRuleType::ThroughBall
					|| !View.OneOnOneOptions.Contains(
						EMatchPlayThroughBallOneOnOneShotChoice::ChipShot)
					|| !View.OneOnOneOptions.Contains(
						EMatchPlayThroughBallOneOnOneShotChoice::DirectShot))
				{
					return Fail(Test, FamilyLabel,
						TEXT("OneOnOne choices were missing or unexpected"));
				}
				bSawOneOnOne = true;
				AcknowledgeIfPending(Controller);
				Controller.SubmitOneOnOneShotChoice(
					EMatchPlayThroughBallOneOnOneShotChoice::DirectShot);
			}
			else if (View.InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
			{
				Controller.ContinueResolution();
			}
			else
			{
				return Fail(Test, FamilyLabel,
					TEXT("resolution reached an unexpected interaction category"));
			}

			if (!Controller.GetLastDiagnostic().bHostSuccess)
			{
				return Fail(Test, FamilyLabel,
					TEXT("typed Host resolution command was rejected: ")
					+ Controller.GetLastDiagnostic().Message);
			}
		}

		if (Controller.GetInteractionView().bCurrentAttackActive)
		{
			return Fail(Test, FamilyLabel,
				TEXT("attack did not reach terminal Completion within the guard"));
		}
		const auto& Feedback = Controller.GetResolutionFeedback();
		if (!bSawResolvedRoute
			|| !Feedback.bTerminal
			|| !Feedback.TerminalSummary.StartsWith(TEXT("RESULT: "))
			|| !Feedback.RouteSummary.Contains(Family.ReadableLabel))
		{
			return Fail(Test, FamilyLabel,
				TEXT("route/feedback/terminal evidence was incomplete"));
		}
		if (Family.SkillType == ESkillRuleType::ThroughBall)
		{
			bool bHasActiveGoalkeeperEvidence = false;
			for (const FString& Entry : Feedback.ComparisonEntries)
			{
				bHasActiveGoalkeeperEvidence = bHasActiveGoalkeeperEvidence
					|| (Entry.Contains(TEXT("Goalkeeper"))
						&& Entry.Contains(TEXT("activation active")));
			}
			if (!bSawOneOnOne || !bHasActiveGoalkeeperEvidence)
			{
				return Fail(Test, FamilyLabel,
					TEXT("normal-demo OneOnOne or active GK Formula evidence was absent"));
			}
		}
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchNormalDemoFamilyInventoryTest,
	"FMCodex.LocalPlay.ControlSurface.23.NormalDemoFamilyInventory",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchNormalDemoFamilyInventoryTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchFullFamilyTests;
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const TArray<FFamilyExpectation> Expected = FamilyExpectations();
	TestEqual(TEXT("Canonical catalog exposes thirteen distinct Skill/range rules"),
		Demo.SkillRuleSet.SkillRules.Num(), 13);

	TSet<FName> RuleIds;
	TSet<uint8> RuleTypes;
	for (const FSkillRuleSnapshot& Rule : Demo.SkillRuleSet.SkillRules)
	{
		RuleIds.Add(Rule.SkillId);
		RuleTypes.Add(static_cast<uint8>(Rule.SkillType));
		TestTrue(TEXT("Canonical Skill minimum AP is supported"),
			Rule.MinTriggerActionPoint >= 2
				&& Rule.MinTriggerActionPoint <= 8);
		TestTrue(TEXT("Canonical Skill maximum AP is supported and ordered"),
			Rule.MaxTriggerActionPoint >= Rule.MinTriggerActionPoint
				&& Rule.MaxTriggerActionPoint <= 8);
	}
	TestEqual(TEXT("Thirteen canonical SkillIds are unique"), RuleIds.Num(), 13);
	TestEqual(TEXT("Five canonical Skill types remain represented"),
		RuleTypes.Num(), 5);

	for (const FFamilyExpectation& Family : Expected)
	{
		const FName ExpectedSkillId(Family.SkillId);
		const FSkillRuleSnapshot* MatchingRule =
			Demo.SkillRuleSet.SkillRules.FindByPredicate(
				[&](const FSkillRuleSnapshot& Rule)
				{
					return Rule.SkillId == ExpectedSkillId;
				});
		TestNotNull(FString::Printf(TEXT("%s rule exists"), Family.ReadableLabel),
			MatchingRule);
		if (MatchingRule != nullptr)
		{
			TestEqual(FString::Printf(TEXT("%s rule maps unambiguously"),
				Family.ReadableLabel), MatchingRule->SkillType, Family.SkillType);
		}
		TestEqual(FString::Printf(TEXT("%s readable UI mapping"),
			Family.ReadableLabel),
			FFMCodexLocalMatchInteractionViewBuilder::ToString(Family.SkillType),
			FString(Family.ReadableLabel));
	}

	TSet<FName> AllCardIds;
	int32 TotalSkillAssignments = 0;
	for (const TArray<FPlayerCardData>* Deck : {
		&Demo.OpeningInput.OpeningInput.PlayerADeck,
		&Demo.OpeningInput.OpeningInput.PlayerBDeck })
	{
		TestEqual(TEXT("Each normal demo deck still has twenty cards"),
			Deck->Num(), 20);
		int32 GoalkeeperCount = 0;
		for (const FPlayerCardData& Card : *Deck)
		{
			AllCardIds.Add(Card.CardId);
			if (Card.bIsGoalkeeper)
			{
				++GoalkeeperCount;
				continue;
			}
			TestTrue(TEXT("Workbook Skill cardinality remains zero through three"),
				Card.AttackSkillIds.Num() >= 0
					&& Card.AttackSkillIds.Num() <= 3);
			TotalSkillAssignments += Card.AttackSkillIds.Num();
			for (const FName SkillId : Card.AttackSkillIds)
			{
				TestTrue(TEXT("Every card Skill resolves to a canonical rule"),
					RuleIds.Contains(SkillId));
			}
		}
		TestEqual(TEXT("Each canonical club has one goalkeeper"),
			GoalkeeperCount, 1);
	}
	TestEqual(TEXT("All forty demo CardIds remain unique"),
		AllCardIds.Num(), 40);
	TestEqual(TEXT("Workbook Skill assignment count is exact"),
		TotalSkillAssignments, 36);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchNormalDemoFullFamilyReachabilityTest,
	"FMCodex.LocalPlay.ControlSurface.24.NormalDemoFullFamilyReachability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchNormalDemoFullFamilyReachabilityTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Normal-demo full-family Host exists"), Host);
	TestNotNull(TEXT("Normal-demo full-family Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const int32 ThroughBallSeed = FindSeedForRolls({ 3, 6, 1, 2 });
	TestTrue(TEXT("Deterministic normal-demo OneOnOne seed exists"),
		ThroughBallSeed != INDEX_NONE);
	if (ThroughBallSeed == INDEX_NONE)
	{
		return false;
	}

	for (const FFamilyExpectation& Family : FamilyExpectations())
	{
		const int32 Seed = Family.SkillType == ESkillRuleType::ThroughBall
			? ThroughBallSeed
			: 1000 + static_cast<int32>(Family.SkillType);
		TestTrue(FString::Printf(
			TEXT("Normal production demo reaches %s selection/resolution/terminal"),
			Family.ReadableLabel),
			CompleteNormalDemoFamilyAttack(
				*this, *Host, *Controller, Demo, Family, Seed));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPlayerFacingScreenFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.25.PlayerFacingScreenFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPlayerFacingScreenFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FFMCodexLocalMatchInteractionView View;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::StartMatch;
	auto Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("No-match status is player-readable"),
		Screen.MatchStatusLabel, FString(TEXT("READY TO PLAY")));
	TestEqual(TEXT("No-match interaction invites a local match"),
		Screen.InteractionTitle, FString(TEXT("Start a Local Match")));
	TestFalse(TEXT("No-match waiting state is not system resolution"),
		Screen.bSystemResolution);

	View.bMatchActive = true;
	View.bHumanInteraction = true;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerB;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectMarker;
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Human action is explicit"), Screen.InteractionKicker,
		FString(TEXT("PLAYER ACTION")));
	TestEqual(TEXT("Acting player is prominent"), Screen.ActingStatusLabel,
		FString(TEXT("PLAYER B TO ACT")));
	TestEqual(TEXT("Human task is plain language"), Screen.InteractionTitle,
		FString(TEXT("Select Marker")));

	View.bHumanInteraction = false;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::ContinueResolution;
	View.ContinueActionLabel = TEXT("Roll Route Dice");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestTrue(TEXT("System-owned step is explicit"), Screen.bSystemResolution);
	TestEqual(TEXT("System action uses authoritative continuation label"),
		Screen.InteractionTitle, FString(TEXT("Roll Route Dice")));

	View.bHumanInteraction = true;
	View.ExpectedActingPlayer = EInitialTurnOrderPlayer::PlayerA;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::SelectBranchIntent;
	View.ActionLabel = TEXT("Cross");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Cross branch prompt is family-specific"),
		Screen.InteractionTitle, FString(TEXT("Choose Cross Type")));
	View.ActionLabel = TEXT("Long Shot");
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Shot branch prompt is family-specific"),
		Screen.InteractionTitle, FString(TEXT("Choose Shot Type")));

	View.bMatchEnded = true;
	View.InteractionCategory =
		EFMCodexLocalMatchInteractionCategory::MatchEnded;
	Screen =
		FFMCodexLocalMatchInteractionViewBuilder::BuildScreenPresentation(View);
	TestEqual(TEXT("Ended match has final-result status"),
		Screen.ActingStatusLabel, FString(TEXT("FINAL RESULT")));
	TestEqual(TEXT("Ended match has completion title"),
		Screen.InteractionTitle, FString(TEXT("Match Complete")));

	FString ControllerSource;
	TestTrue(TEXT("Player-facing production source loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
			ControllerSource));
	TestTrue(TEXT("Four primary screen regions are explicit"),
		ControllerSource.Contains(TEXT("MATCH HEADER"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION"))
			&& ControllerSource.Contains(TEXT("RESOLUTION RESULT")));
	TestTrue(TEXT("Developer diagnostics are secondary and collapsible"),
		ControllerSource.Contains(TEXT("SNew(SExpandableArea)"))
			&& ControllerSource.Contains(TEXT("Developer Details"))
			&& ControllerSource.Contains(TEXT("InitiallyCollapsed(true)")));
	TestTrue(TEXT("Field exposes canonical side-relative football zones"),
		ControllerSource.Contains(TEXT("RELATIVE ZONES"))
			&& ControllerSource.Contains(TEXT("Slot.PlayerARelativeZone"))
			&& ControllerSource.Contains(TEXT("Slot.PlayerBRelativeZone")));
	TestTrue(TEXT("Card reference is secondary to readable card data"),
		ControllerSource.Contains(TEXT("DeveloperReferenceLabel"))
			&& ControllerSource.Contains(TEXT("SkillSummaryLabel")));
	TestTrue(TEXT("Resolution evidence has structured dice and comparison labels"),
		ControllerSource.Contains(TEXT("DICE"))
			&& ControllerSource.Contains(TEXT("D6  %d"))
			&& ControllerSource.Contains(TEXT("FORMULA COMPARISON"))
			&& ControllerSource.Contains(TEXT("ATTACK"))
			&& ControllerSource.Contains(TEXT("DEFENSE")));
	TestTrue(TEXT("All established player controls remain reachable"),
		ControllerSource.Contains(TEXT("Finish Deployment"))
			&& ControllerSource.Contains(TEXT("Decline"))
			&& ControllerSource.Contains(TEXT("Resolve No Legal Choice"))
			&& ControllerSource.Contains(TEXT("Branch / Shot Type"))
			&& ControllerSource.Contains(TEXT("One-on-One Shot Type"))
			&& ControllerSource.Contains(TEXT("ContinueResolution()")));

	const int32 SurfaceStart = ControllerSource.Find(
		TEXT("AFMCodexLocalMatchPlayerController::BuildControlSurface"));
	const FString SurfaceBody = SurfaceStart == INDEX_NONE
		? FString() : ControllerSource.Mid(SurfaceStart);
	TestTrue(TEXT("Screen builder remains a presentation-only consumer"),
		!SurfaceBody.IsEmpty()
			&& !SurfaceBody.Contains(TEXT("Host->"))
			&& !SurfaceBody.Contains(TEXT("RandRange"))
			&& !SurfaceBody.Contains(TEXT("ResolveFormula("))
			&& !SurfaceBody.Contains(TEXT("RuntimeState.")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchPitchAndZoneRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.26.PitchAndZoneRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchPitchAndZoneRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Pitch refinement Host exists"), Host);
	TestNotNull(TEXT("Pitch refinement Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Controller->StartNewDemoMatch();
	AcknowledgeIfPending(*Controller);
	Controller->RollDemoTacticalPoints();
	AcknowledgeIfPending(*Controller);
	TestTrue(TEXT("Pitch fixture begins authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess);

	const FFMCodexLocalMatchInteractionView InitialView =
		Controller->GetInteractionView();
	TestEqual(TEXT("Pitch has two physical halves"),
		InitialView.PitchRegions.Num(), 2);
	if (InitialView.PitchRegions.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("Player B half is stably first"),
		InitialView.PitchRegions[0].NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerB);
	TestEqual(TEXT("Player A half is stably second"),
		InitialView.PitchRegions[1].NeutralSide,
		EMatchPlayNeutralSlotSide::NearPlayerA);

	TArray<FName> PresentedSlotOrder;
	int32 EmptySlotCount = 0;
	int32 AttackingSideEmphasisCount = 0;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: InitialView.PitchRegions)
	{
		AttackingSideEmphasisCount += Region.bCurrentAttackingSide ? 1 : 0;
		TestFalse(TEXT("Physical half has side-relative zone context"),
			Region.ZoneContextLabel.IsEmpty());
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			PresentedSlotOrder.Add(Slot.SlotId);
			EmptySlotCount += Slot.bOccupied ? 0 : 1;
			const auto PlayerAZone =
				FMatchPlayRelativeDeploymentZoneResolver::Resolve(
					Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog,
					Slot.SlotId,
					InitialView.CurrentAttackingPlayer,
					EInitialTurnOrderPlayer::PlayerA);
			const auto PlayerBZone =
				FMatchPlayRelativeDeploymentZoneResolver::Resolve(
					Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog,
					Slot.SlotId,
					InitialView.CurrentAttackingPlayer,
					EInitialTurnOrderPlayer::PlayerB);
			TestTrue(TEXT("Canonical resolver accepts displayed slot for A"),
				PlayerAZone.bSuccess);
			TestTrue(TEXT("Canonical resolver accepts displayed slot for B"),
				PlayerBZone.bSuccess);
			if (PlayerAZone.bSuccess && PlayerBZone.bSuccess)
			{
				TestEqual(TEXT("Player A displayed zone equals canonical resolver"),
					Slot.PlayerARelativeZone, PlayerAZone.RelativeZone);
				TestEqual(TEXT("Player B displayed zone equals canonical resolver"),
					Slot.PlayerBRelativeZone, PlayerBZone.RelativeZone);
			}
		}
	}
	TestEqual(TEXT("All canonical slots are represented"),
		PresentedSlotOrder.Num(),
		Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots.Num());
	TestEqual(TEXT("Initial field visibly represents every slot as empty"),
		EmptySlotCount, PresentedSlotOrder.Num());
	TestEqual(TEXT("Exactly one physical side is emphasized as attacking"),
		AttackingSideEmphasisCount, 1);
	TestTrue(TEXT("Direction is derived and player-readable"),
		InitialView.AttackDirectionLabel.Contains(
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				InitialView.CurrentAttackingPlayer))
			&& InitialView.AttackDirectionLabel.Contains(TEXT("Half")));

	TArray<FName> ExpectedStableOrder;
	for (const EMatchPlayNeutralSlotSide Side : {
		EMatchPlayNeutralSlotSide::NearPlayerB,
		EMatchPlayNeutralSlotSide::NearPlayerA })
	{
		for (const FMatchPlayDeploymentSlotDefinition& Slot
			: Host->GetMatchSnapshot().Snapshot.DeploymentSlotCatalog.Slots)
		{
			if (Slot.NeutralSide == Side)
			{
				ExpectedStableOrder.Add(Slot.SlotId);
			}
		}
	}
	TestTrue(TEXT("Slot ordering is physical-half then canonical catalog order"),
		PresentedSlotOrder == ExpectedStableOrder);

	auto FindPitchSlot = [](
		const FFMCodexLocalMatchInteractionView& View,
		const FName SlotId) -> const FFMCodexLocalMatchPitchSlotView*
	{
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			if (const FFMCodexLocalMatchPitchSlotView* Slot =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexLocalMatchPitchSlotView& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Slot;
			}
		}
		return nullptr;
	};

	FName RejectedCardId = NAME_None;
	FName RejectedSlotId = NAME_None;
	bool bDeployedGoalkeeper = false;
	TSet<EInitialTurnOrderPlayer> PopulatedSides;
	for (int32 Step = 0; Step < 4; ++Step)
	{
		AcknowledgeIfPending(*Controller);
		const FFMCodexLocalMatchInteractionView BeforeView =
			Controller->GetInteractionView();
		const EInitialTurnOrderPlayer Side =
			BeforeView.CurrentLegalDeploymentSide;
		const bool bPreferGoalkeeper = !bDeployedGoalkeeper
			&& Side != BeforeView.CurrentAttackingPlayer;
		const FFMCodexLocalMatchDeploymentGroup* Group =
			BeforeView.DeploymentGroups.FindByPredicate(
				[Side, bPreferGoalkeeper](
					const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return Candidate.Side == Side
						&& Candidate.bGoalkeeper == bPreferGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				});
		TestNotNull(TEXT("Dense fixture has a legal authoritative group"), Group);
		if (Group == nullptr)
		{
			return false;
		}
		const FFMCodexLocalMatchSlotView Destination = Group->LegalSlots[0];
		TestTrue(TEXT("Destination label relates action to zone and canonical slot"),
			Destination.Label.Contains(TEXT("Slot "))
				&& Destination.Label.Contains(
					FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Destination.RelativeZone))
				&& Destination.Label.Contains(
					FFMCodexLocalMatchInteractionViewBuilder::ToString(
						Destination.NeutralSide)));
		const FFMCodexLocalMatchCardView ExpectedCard = Group->Card;
		if (Group->bGoalkeeper)
		{
			Controller->DeployGoalkeeper(Destination.SlotId);
			bDeployedGoalkeeper = true;
		}
		else
		{
			Controller->DeployOrdinary(Group->CardId, Destination.SlotId);
			if (RejectedCardId.IsNone())
			{
				RejectedCardId = Group->CardId;
				RejectedSlotId = Destination.SlotId;
			}
		}
		TestTrue(TEXT("Authoritative deployment succeeds"),
			Controller->GetLastDiagnostic().bHostSuccess);
		const FFMCodexLocalMatchInteractionView AfterView =
			Controller->GetInteractionView();
		const FFMCodexLocalMatchPitchSlotView* PresentedSlot =
			FindPitchSlot(AfterView, Destination.SlotId);
		TestNotNull(TEXT("Snapshot rebuild retains exact destination slot"),
			PresentedSlot);
		if (PresentedSlot != nullptr)
		{
			TestTrue(TEXT("Exact slot becomes occupied by authoritative card"),
				PresentedSlot->bOccupied
					&& PresentedSlot->Card.CardId == ExpectedCard.CardId
					&& PresentedSlot->Card.SlotId == Destination.SlotId);
			TestTrue(TEXT("Pitch and candidate identity presentation agree"),
				PresentedSlot->Card.DisplayLabel == ExpectedCard.DisplayLabel
					&& PresentedSlot->Card.Side == ExpectedCard.Side
					&& PresentedSlot->Card.SkillLabels == ExpectedCard.SkillLabels);
			TestEqual(TEXT("Pitch card zone equals destination resolver zone"),
				PresentedSlot->Card.RelativeZone, Destination.RelativeZone);
		}
		PopulatedSides.Add(Side);
	}
	TestTrue(TEXT("Dense fixture includes a goalkeeper"), bDeployedGoalkeeper);
	TestEqual(TEXT("Dense fixture populates both player sides"),
		PopulatedSides.Num(), 2);

	int32 OccupiedSlotCount = 0;
	int32 GoalkeeperCount = 0;
	TArray<FName> OrderAfterDeployments;
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: Controller->GetInteractionView().PitchRegions)
	{
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			OrderAfterDeployments.Add(Slot.SlotId);
			if (Slot.bOccupied)
			{
				++OccupiedSlotCount;
				GoalkeeperCount += Slot.Card.bGoalkeeper ? 1 : 0;
			}
		}
	}
	TestEqual(TEXT("Dense pitch drops no deployed cards"), OccupiedSlotCount, 4);
	TestEqual(TEXT("Goalkeeper is represented exactly once"), GoalkeeperCount, 1);
	TestTrue(TEXT("Deployment refresh never changes stable slot order"),
		OrderAfterDeployments == ExpectedStableOrder);

	AcknowledgeIfPending(*Controller);
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	auto PitchSignature = [](const FFMCodexLocalMatchInteractionView& View)
	{
		TArray<FString> Result;
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
			{
				Result.Add(FString::Printf(
					TEXT("%s|%s|%s"),
					*Slot.SlotId.ToString(),
					Slot.bOccupied ? TEXT("occupied") : TEXT("empty"),
					*Slot.Card.CardId.ToString()));
			}
		}
		return Result;
	};
	const TArray<FString> PitchBeforeRejected =
		PitchSignature(Controller->GetInteractionView());
	Controller->DeployOrdinary(RejectedCardId, RejectedSlotId);
	TestFalse(TEXT("Repeated deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Rejected deployment leaves State byte-identical"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestTrue(TEXT("Rejected deployment causes no optimistic pitch movement"),
		PitchBeforeRejected == PitchSignature(Controller->GetInteractionView()));
	TestTrue(TEXT("Rejected deployment remains visibly diagnosed"),
		Controller->GetResolutionFeedback().bRejected);

	FString ControllerSource;
	FString ViewSource;
	TestTrue(TEXT("Pitch controller source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Pitch view source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
		ViewSource));
	TestTrue(TEXT("Dense slots use wrapping and bounded compact cards"),
		ControllerSource.Contains(TEXT("MakeCompactPitchCard"))
			&& ControllerSource.Contains(TEXT("SNew(SWrapBox)"))
			&& ControllerSource.Contains(TEXT("WidthOverride(218.0f)")));
	TestTrue(TEXT("Empty slot copy explicitly avoids false legality"),
		ControllerSource.Contains(TEXT("EMPTY SLOT"))
			&& ControllerSource.Contains(
				TEXT("Legality is shown in player actions")));
	TestTrue(TEXT("Stage 6.1 architecture remains present"),
		ControllerSource.Contains(TEXT("MATCH HEADER"))
			&& ControllerSource.Contains(TEXT("FOOTBALL FIELD"))
			&& ControllerSource.Contains(TEXT("CURRENT INTERACTION"))
			&& ControllerSource.Contains(TEXT("RESOLUTION RESULT"))
			&& ControllerSource.Contains(TEXT("Developer Details")));

	const int32 PitchSlotStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakePitchSlot"));
	const int32 FeedbackStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeFeedbackPanel"));
	const FString PitchWidgetBody = PitchSlotStart != INDEX_NONE
		&& FeedbackStart > PitchSlotStart
			? ControllerSource.Mid(
				PitchSlotStart, FeedbackStart - PitchSlotStart)
			: FString();
	TestTrue(TEXT("Pitch slots remain non-interactive state presentation"),
		!PitchWidgetBody.IsEmpty()
			&& !PitchWidgetBody.Contains(TEXT("MakeButton"))
			&& !PitchWidgetBody.Contains(TEXT("Host->"))
			&& !PitchWidgetBody.Contains(TEXT("LegalityEvaluator"))
			&& !PitchWidgetBody.Contains(TEXT("AvailabilityQuery")));

	const int32 PitchModelStart = ViewSource.Find(
		TEXT("void BuildPitchRegions"));
	const int32 SelectionStart = ViewSource.Find(
		TEXT("void AddSelectionOption"));
	const FString PitchModelBody = PitchModelStart != INDEX_NONE
		&& SelectionStart > PitchModelStart
			? ViewSource.Mid(PitchModelStart, SelectionStart - PitchModelStart)
			: FString();
	TestTrue(TEXT("Pitch model reuses canonical resolver without legality logic"),
		!PitchModelBody.IsEmpty()
			&& PitchModelBody.Contains(
				TEXT("FMatchPlayRelativeDeploymentZoneResolver::Resolve"))
			&& !PitchModelBody.Contains(TEXT("Legality"))
			&& !PitchModelBody.Contains(TEXT("Availability"))
			&& !PitchModelBody.Contains(TEXT("ResolveFormula("))
			&& !PitchModelBody.Contains(TEXT("RandRange")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchCardVisualHierarchyRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.27.CardVisualHierarchyRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchCardVisualHierarchyRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Card hierarchy Host exists"), Host);
	TestNotNull(TEXT("Card hierarchy Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	Controller->StartNewDemoMatch();
	AcknowledgeIfPending(*Controller);
	Controller->RollDemoTacticalPoints();
	AcknowledgeIfPending(*Controller);
	const FFMCodexLocalMatchInteractionView InitialView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		InitialView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty()
					&& Candidate.Card.CardId.ToString().StartsWith(
						TEXT("Prototype."))
					&& Candidate.Card.SkillLabels.Contains(TEXT("Cross"));
			});
	TestNotNull(TEXT("Ordinary interaction card exists"), OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView OrdinaryCard = OrdinaryGroup->Card;
	TestFalse(TEXT("Canonical display name remains player-facing"),
		OrdinaryCard.DisplayLabel.IsEmpty());
	TestFalse(TEXT("Compact role is derived and readable"),
		OrdinaryCard.CompactRoleLabel.IsEmpty());
	TestTrue(TEXT("Production card exposes a readable Skill"),
		!OrdinaryCard.SkillLabels.IsEmpty()
			&& OrdinaryCard.SkillSummaryLabel.Contains(
				OrdinaryCard.SkillLabels[0]));
	TestTrue(TEXT("Available status is presentation-derived"),
		OrdinaryCard.bAvailable
			&& OrdinaryCard.StatusLabels.Contains(TEXT("AVAILABLE"))
			&& OrdinaryCard.StatusSummaryLabel.Contains(TEXT("AVAILABLE")));
	TestTrue(TEXT("Developer reference retains CardId and rarity"),
		OrdinaryCard.DeveloperReferenceLabel.Contains(
			OrdinaryCard.CardId.ToString())
			&& OrdinaryCard.DeveloperReferenceLabel.Contains(TEXT("Rarity:")));
	for (const TCHAR* FullAttribute : {
		TEXT("SHO"), TEXT("DRI"), TEXT("PAS"), TEXT("OFF"), TEXT("MRK"),
		TEXT("TKL"), TEXT("SPD"), TEXT("STR"), TEXT("STA"), TEXT("LS") })
	{
		TestTrue(FString::Printf(TEXT("Interaction attributes retain %s"),
			FullAttribute), OrdinaryCard.AttributeSummary.Contains(FullAttribute));
	}
	TestTrue(TEXT("Compact outfield subset is stable and bounded"),
		OrdinaryCard.CompactAttributeSummary.Contains(TEXT("SHO"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("PAS"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("DRI"))
			&& OrdinaryCard.CompactAttributeSummary.Contains(TEXT("SPD"))
			&& !OrdinaryCard.CompactAttributeSummary.Contains(TEXT("OFF"))
			&& !OrdinaryCard.CompactAttributeSummary.Contains(TEXT("STR")));

	auto FindGroup = [](const FFMCodexLocalMatchInteractionView& View,
		const FName CardId) -> const FFMCodexLocalMatchDeploymentGroup*
	{
		return View.DeploymentGroups.FindByPredicate(
			[CardId](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	};
	auto FindAuthorityCard = [](FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId) -> FPlayerCardRuleSnapshot*
	{
		FPlayerCardRuleSnapshotSet& Set =
			Side == EInitialTurnOrderPlayer::PlayerA
				? State.CardSnapshotAuthority.PlayerACardSnapshots
				: State.CardSnapshotAuthority.PlayerBCardSnapshots;
		return Set.Cards.FindByPredicate(
			[CardId](const FPlayerCardRuleSnapshot& Candidate)
			{
				return Candidate.CardId == CardId;
			});
	};
	FMatchPlayState MultiSkillState = Host->GetMatchSnapshot().Snapshot;
	FPlayerCardRuleSnapshot* MultiSkillSnapshot = FindAuthorityCard(
		MultiSkillState, OrdinaryCard.Side, OrdinaryCard.CardId);
	TestNotNull(TEXT("Multi-Skill fixture finds authoritative snapshot"),
		MultiSkillSnapshot);
	if (MultiSkillSnapshot != nullptr && Demo.SkillRuleSet.SkillRules.Num() >= 2)
	{
		MultiSkillSnapshot->SkillIds = {
			Demo.SkillRuleSet.SkillRules[0].SkillId,
			Demo.SkillRuleSet.SkillRules[1].SkillId
		};
		const FFMCodexLocalMatchInteractionView MultiSkillView =
			FFMCodexLocalMatchInteractionViewBuilder::Build(
				MultiSkillState, Demo.SkillRuleSet);
		const FFMCodexLocalMatchDeploymentGroup* MultiSkillGroup =
			FindGroup(MultiSkillView, OrdinaryCard.CardId);
		TestNotNull(TEXT("Multi-Skill card remains present"), MultiSkillGroup);
		if (MultiSkillGroup != nullptr)
		{
			TestEqual(TEXT("All authoritative Skills remain in CardView"),
				MultiSkillGroup->Card.SkillLabels.Num(), 2);
			for (const FString& Skill : MultiSkillGroup->Card.SkillLabels)
			{
				TestTrue(TEXT("Compact/detail summary never drops a Skill"),
					MultiSkillGroup->Card.SkillSummaryLabel.Contains(Skill));
			}
		}

		MultiSkillSnapshot->SkillIds.Reset();
		const FFMCodexLocalMatchInteractionView NoSkillView =
			FFMCodexLocalMatchInteractionViewBuilder::Build(
				MultiSkillState, Demo.SkillRuleSet);
		const FFMCodexLocalMatchDeploymentGroup* NoSkillGroup =
			FindGroup(NoSkillView, OrdinaryCard.CardId);
		TestNotNull(TEXT("No-Skill fallback card remains present"), NoSkillGroup);
		if (NoSkillGroup != nullptr)
		{
			TestTrue(TEXT("No-Skill fallback is bounded and explicit"),
				NoSkillGroup->Card.SkillLabels.IsEmpty()
					&& NoSkillGroup->Card.SkillSummaryLabel == TEXT("NO SKILL"));
		}
	}

	auto FindPitchSlot = [](const FFMCodexLocalMatchInteractionView& View,
		const FName SlotId) -> const FFMCodexLocalMatchPitchSlotView*
	{
		for (const FFMCodexLocalMatchPitchRegionView& Region : View.PitchRegions)
		{
			if (const FFMCodexLocalMatchPitchSlotView* Slot =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexLocalMatchPitchSlotView& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Slot;
			}
		}
		return nullptr;
	};
	const FFMCodexLocalMatchSlotView OrdinaryDestination =
		OrdinaryGroup->LegalSlots[0];
	Controller->DeployOrdinary(
		OrdinaryCard.CardId, OrdinaryDestination.SlotId);
	TestTrue(TEXT("Card hierarchy deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const FFMCodexLocalMatchInteractionView AfterOrdinary =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchPitchSlotView* OrdinaryPitchSlot =
		FindPitchSlot(AfterOrdinary, OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Deployed ordinary card appears on pitch"),
		OrdinaryPitchSlot);
	if (OrdinaryPitchSlot != nullptr)
	{
		const FFMCodexLocalMatchCardView& PitchCard = OrdinaryPitchSlot->Card;
		TestTrue(TEXT("Pitch and interaction use identical shared identity"),
			PitchCard.CardId == OrdinaryCard.CardId
				&& PitchCard.DisplayLabel == OrdinaryCard.DisplayLabel
				&& PitchCard.Side == OrdinaryCard.Side
				&& PitchCard.CompactRoleLabel == OrdinaryCard.CompactRoleLabel
				&& PitchCard.SkillLabels == OrdinaryCard.SkillLabels
				&& PitchCard.SkillSummaryLabel == OrdinaryCard.SkillSummaryLabel
				&& PitchCard.DeveloperReferenceLabel
					== OrdinaryCard.DeveloperReferenceLabel);
		TestTrue(TEXT("Deployed status is freshly snapshot-derived"),
			PitchCard.bDeployed
				&& PitchCard.StatusLabels.Contains(TEXT("DEPLOYED")));
	}

	FFMCodexLocalMatchDeploymentGroup GoalkeeperGroup;
	bool bFoundGoalkeeper = false;
	for (int32 Attempt = 0; Attempt < 2 && !bFoundGoalkeeper; ++Attempt)
	{
		AcknowledgeIfPending(*Controller);
		const FFMCodexLocalMatchInteractionView View =
			Controller->GetInteractionView();
		if (const FFMCodexLocalMatchDeploymentGroup* Found =
			View.DeploymentGroups.FindByPredicate(
				[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return Candidate.bGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				}))
		{
			GoalkeeperGroup = *Found;
			bFoundGoalkeeper = true;
			break;
		}
		const FFMCodexLocalMatchDeploymentGroup* NextOrdinary =
			View.DeploymentGroups.FindByPredicate(
				[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& !Candidate.LegalSlots.IsEmpty();
				});
		if (NextOrdinary == nullptr)
		{
			break;
		}
		Controller->DeployOrdinary(
			NextOrdinary->CardId, NextOrdinary->LegalSlots[0].SlotId);
	}
	TestTrue(TEXT("Authoritative deployment exposes goalkeeper card"),
		bFoundGoalkeeper);
	if (bFoundGoalkeeper)
	{
		TestTrue(TEXT("GK hierarchy uses only faithful goalkeeper data"),
			GoalkeeperGroup.Card.bGoalkeeper
				&& GoalkeeperGroup.Card.CompactRoleLabel == TEXT("GK")
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("HAN"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("POS"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("REF"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("AER"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("ANT"))
				&& GoalkeeperGroup.Card.GoalkeeperAttributeSummary.Contains(TEXT("1v1"))
				&& GoalkeeperGroup.Card.CompactAttributeSummary.Contains(TEXT("AER"))
				&& !GoalkeeperGroup.Card.CompactAttributeSummary.Contains(TEXT("SHO")));
		const FName GoalkeeperSlotId = GoalkeeperGroup.LegalSlots[0].SlotId;
		Controller->DeployGoalkeeper(GoalkeeperSlotId);
		TestTrue(TEXT("Goalkeeper deployment succeeds"),
			Controller->GetLastDiagnostic().bHostSuccess);
		const FFMCodexLocalMatchPitchSlotView* GoalkeeperPitchSlot =
			FindPitchSlot(Controller->GetInteractionView(), GoalkeeperSlotId);
		TestNotNull(TEXT("Goalkeeper appears in exact pitch slot"),
			GoalkeeperPitchSlot);
		if (GoalkeeperPitchSlot != nullptr)
		{
			TestTrue(TEXT("GK pitch/candidate/developer identity agrees"),
				GoalkeeperPitchSlot->Card.CardId == GoalkeeperGroup.Card.CardId
					&& GoalkeeperPitchSlot->Card.DisplayLabel
						== GoalkeeperGroup.Card.DisplayLabel
					&& GoalkeeperPitchSlot->Card.CompactRoleLabel == TEXT("GK")
					&& GoalkeeperPitchSlot->Card.DeveloperReferenceLabel
						== GoalkeeperGroup.Card.DeveloperReferenceLabel
					&& GoalkeeperPitchSlot->Card.StatusLabels.Contains(
						TEXT("DEPLOYED")));
		}
	}

	AcknowledgeIfPending(*Controller);
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FFMCodexLocalMatchPitchSlotView* BeforeRejectedSlot =
		FindPitchSlot(
			Controller->GetInteractionView(), OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Rejected fixture retains ordinary pitch card"),
		BeforeRejectedSlot);
	if (BeforeRejectedSlot == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView CardBeforeRejected =
		BeforeRejectedSlot->Card;
	Controller->DeployOrdinary(
		OrdinaryCard.CardId, OrdinaryDestination.SlotId);
	TestFalse(TEXT("Repeated card deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Rejected card action leaves State byte-identical"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot));
	const FFMCodexLocalMatchPitchSlotView* CardAfterRejected =
		FindPitchSlot(Controller->GetInteractionView(), OrdinaryDestination.SlotId);
	TestNotNull(TEXT("Rejected refresh retains the pitch card"),
		CardAfterRejected);
	if (CardAfterRejected != nullptr)
	{
		TestTrue(TEXT("Rejected action changes no card presentation evidence"),
			CardAfterRejected->Card.CardId == CardBeforeRejected.CardId
				&& CardAfterRejected->Card.DisplayLabel
					== CardBeforeRejected.DisplayLabel
				&& CardAfterRejected->Card.SkillLabels
					== CardBeforeRejected.SkillLabels
				&& CardAfterRejected->Card.AttributeSummary
					== CardBeforeRejected.AttributeSummary
				&& CardAfterRejected->Card.CompactAttributeSummary
					== CardBeforeRejected.CompactAttributeSummary
				&& CardAfterRejected->Card.StatusLabels
					== CardBeforeRejected.StatusLabels);
	}

	FString ControllerSource;
	FString ViewSource;
	TestTrue(TEXT("Card renderer source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
		ControllerSource));
	TestTrue(TEXT("Card presentation source loads"), LoadProductionSource(
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
		ViewSource));
	TestTrue(TEXT("Typography and bounded interaction hierarchy are explicit"),
		ControllerSource.Contains(TEXT("GetDefaultFontStyle(\"Bold\", 16)"))
			&& ControllerSource.Contains(TEXT("MaxDesiredWidth(520.0f)"))
			&& ControllerSource.Contains(TEXT("OUTFIELD ATTRIBUTES"))
			&& ControllerSource.Contains(TEXT("GOALKEEPER ATTRIBUTES"))
			&& ControllerSource.Contains(TEXT("STATUS  |  ")));
	const int32 InteractionCardStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeCardPanel"));
	const int32 CompactCardStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakeCompactPitchCard"));
	const int32 PitchSlotStart = ControllerSource.Find(
		TEXT("TSharedRef<SWidget> MakePitchSlot"));
	const FString InteractionCardBody = InteractionCardStart != INDEX_NONE
		&& CompactCardStart > InteractionCardStart
			? ControllerSource.Mid(
				InteractionCardStart, CompactCardStart - InteractionCardStart)
			: FString();
	const FString CompactCardBody = CompactCardStart != INDEX_NONE
		&& PitchSlotStart > CompactCardStart
			? ControllerSource.Mid(
				CompactCardStart, PitchSlotStart - CompactCardStart)
			: FString();
	TestTrue(TEXT("Both variants consume one shared CardView presentation"),
		!InteractionCardBody.IsEmpty() && !CompactCardBody.IsEmpty()
			&& InteractionCardBody.Contains(TEXT("Card.SkillSummaryLabel"))
			&& CompactCardBody.Contains(TEXT("Card.SkillSummaryLabel"))
			&& InteractionCardBody.Contains(TEXT("Card.StatusSummaryLabel"))
			&& CompactCardBody.Contains(TEXT("Card.StatusSummaryLabel")));
	TestTrue(TEXT("Developer reference stays out of compact pitch hierarchy"),
		InteractionCardBody.Contains(TEXT("Card.DeveloperReferenceLabel"))
			&& !CompactCardBody.Contains(TEXT("Card.DeveloperReferenceLabel")));
	TestTrue(TEXT("Card renderers contain no gameplay legality or command logic"),
		!InteractionCardBody.Contains(TEXT("LegalityEvaluator"))
			&& !CompactCardBody.Contains(TEXT("LegalityEvaluator"))
			&& !InteractionCardBody.Contains(TEXT("AvailabilityQuery"))
			&& !CompactCardBody.Contains(TEXT("AvailabilityQuery"))
			&& !InteractionCardBody.Contains(TEXT("Host->"))
			&& !CompactCardBody.Contains(TEXT("Host->")));
	TestTrue(TEXT("Stage 6.2 pitch shell remains present"),
		ControllerSource.Contains(TEXT("CENTER / PHYSICAL HALF BOUNDARY"))
			&& ControllerSource.Contains(TEXT("RELATIVE ZONES"))
			&& ControllerSource.Contains(TEXT("EMPTY SLOT"))
			&& ControllerSource.Contains(TEXT("CURRENT ATTACKING SIDE")));
	TestTrue(TEXT("Card status derivation contains no legality calculation"),
		ViewSource.Contains(TEXT("FinalizeCardPresentation"))
			&& !ViewSource.Contains(TEXT("StatusSummaryLabel ==")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexLocalMatchUMGPlayerFacingFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.28.UMGPlayerFacingFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexLocalMatchUMGPlayerFacingFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("UMG foundation Host exists"), Host);
	TestNotNull(TEXT("UMG foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Controller creates and owns root UMG screen"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	TestTrue(TEXT("Widget input boundary points only to its Controller"),
		Screen->GetMatchController() == Controller
			&& (Screen->GetOwningPlayer() == Controller
				|| Screen->GetOwningPlayer() == nullptr));
	for (const FName RegionName : {
		FName(TEXT("MatchHeaderRegion")),
		FName(TEXT("FootballCardFieldRegion")),
		FName(TEXT("CurrentInteractionRegion")),
		FName(TEXT("ResolutionResultRegion")) })
	{
		TestNotNull(FString::Printf(TEXT("Root screen exposes %s"),
			*RegionName.ToString()), Screen->GetWidgetFromName(RegionName));
	}
	TestTrue(TEXT("No-match UMG exposes explicit Start intent"),
		Screen->GetPresentation().Interaction.bCanStartNewMatch
			&& Screen->GetPresentation().Interaction.CategoryLabel
				== TEXT("Start Match"));

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("UMG StartNewMatch reaches typed authoritative Host path"),
		Host->HasActiveLocalMatch()
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("StartNewLocalMatch"));
	const FFMCodexLocalMatchInteractionView& StartedView =
		Controller->GetInteractionView();
	const FFMCodexUMGMatchScreenViewModel& StartedUMG =
		Screen->GetPresentation();
	TestTrue(TEXT("UMG header equals authoritative presentation contract"),
		StartedUMG.Header.ScoreLabel == FString::Printf(
			TEXT("%d - %d"), StartedView.PlayerAScore,
			StartedView.PlayerBScore)
			&& StartedUMG.Header.CurrentAttackerLabel.Contains(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					StartedView.CurrentAttackingPlayer))
			&& StartedUMG.Header.ExpectedActorLabel.Contains(
				FFMCodexLocalMatchInteractionViewBuilder::ToString(
					StartedView.ExpectedActingPlayer))
			&& StartedUMG.Header.bMatchEnded == StartedView.bMatchEnded);
	TestTrue(TEXT("Authoritative human transition exposes roll immediately"),
		StartedView.bTacticalPointRollReady
			&& StartedUMG.Interaction.bCanRollTacticalPoints);

	Screen->RequestRollTacticalPoints();
	TestTrue(TEXT("UMG TacticalPointRoll creates authoritative CurrentAttack"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().bCurrentAttackActive
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy
			&& Screen->GetPresentation().Interaction.bCanFinishDeployment);

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexUMGMatchScreenViewModel& DeploymentUMG =
		Screen->GetPresentation();
	int32 PitchSlotCount = 0;
	for (const FFMCodexUMGPitchRegionViewModel& Region
		: DeploymentUMG.PitchRegions)
	{
		PitchSlotCount += Region.Slots.Num();
	}
	TestTrue(TEXT("UMG pitch preserves two physical halves and 10 slots"),
		DeploymentUMG.PitchRegions.Num() == 2 && PitchSlotCount == 10);
	TestTrue(TEXT("UMG interaction preserves bounded candidate cards"),
		!DeploymentUMG.Interaction.CandidateCards.IsEmpty()
			&& DeploymentUMG.Interaction.CandidateCards.Num()
				== DeploymentView.DeploymentGroups.Num());

	TSet<FString> SkillFamilies;
	for (const FFMCodexUMGCardViewModel& Card
		: DeploymentUMG.Interaction.CandidateCards)
	{
		for (const FString& Skill : Card.SkillLabels)
		{
			SkillFamilies.Add(Skill);
		}
	}
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("UMG retains %s family"), Family),
			SkillFamilies.Contains(Family));
	}

	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("UMG deployment proof finds ordinary candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView OrdinaryCardView = OrdinaryGroup->Card;
	const FFMCodexUMGCardViewModel* CandidateCard =
		DeploymentUMG.Interaction.CandidateCards.FindByPredicate(
			[OrdinaryGroup](const FFMCodexUMGCardViewModel& Candidate)
			{
				return Candidate.CardId == OrdinaryGroup->CardId;
			});
	TestNotNull(TEXT("UMG candidate has shared Card identity"), CandidateCard);
	if (CandidateCard != nullptr)
	{
		TestTrue(TEXT("UMG Card DTO preserves Stage 6.3 semantics"),
			CandidateCard->IdentityLabel == OrdinaryGroup->Card.DisplayLabel
				&& CandidateCard->RoleLabel
					== OrdinaryGroup->Card.CompactRoleLabel
				&& CandidateCard->SkillLabels == OrdinaryGroup->Card.SkillLabels
				&& CandidateCard->CompactAttributeSummary
					== OrdinaryGroup->Card.CompactAttributeSummary
				&& CandidateCard->StatusLabels
					== OrdinaryGroup->Card.StatusLabels);
	}

	const FName DeployedCardId = OrdinaryGroup->CardId;
	const FName DeployedSlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const TArray<uint8> BeforeDeployment =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Representative typed UMG deployment succeeds"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeDeployment
				!= SerializeState(Host->GetMatchSnapshot().Snapshot));
	auto FindUMGSlot = [](const FFMCodexUMGMatchScreenViewModel& View,
		const FName SlotId) -> const FFMCodexUMGPitchSlotViewModel*
	{
		for (const FFMCodexUMGPitchRegionViewModel& Region : View.PitchRegions)
		{
			if (const FFMCodexUMGPitchSlotViewModel* Found =
				Region.Slots.FindByPredicate(
					[SlotId](const FFMCodexUMGPitchSlotViewModel& Candidate)
					{
						return Candidate.SlotId == SlotId;
					}))
			{
				return Found;
			}
		}
		return nullptr;
	};
	const FFMCodexUMGPitchSlotViewModel* DeployedSlot =
		FindUMGSlot(Screen->GetPresentation(), DeployedSlotId);
	TestNotNull(TEXT("Successful UMG command refreshes exact pitch slot"),
		DeployedSlot);
	if (DeployedSlot != nullptr)
	{
		TestTrue(TEXT("Candidate-to-pitch UMG identity remains equivalent"),
			DeployedSlot->bOccupied
				&& DeployedSlot->Card.CardId == DeployedCardId
				&& DeployedSlot->Card.IdentityLabel
					== OrdinaryCardView.DisplayLabel
				&& DeployedSlot->Card.SkillLabels
					== OrdinaryCardView.SkillLabels);
	}

	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardId, DeployedSlotId);
	TestTrue(TEXT("Rejected UMG intent leaves State unchanged and refreshes failure"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Screen->GetPresentation().Resolution.bVisible
			&& Screen->GetPresentation().Resolution.bRejected
			&& !Screen->GetPresentation().Resolution.ErrorLabel.IsEmpty());
	const FFMCodexUMGPitchSlotViewModel* AfterRejectedSlot =
		FindUMGSlot(Screen->GetPresentation(), DeployedSlotId);
	TestTrue(TEXT("Rejected UMG intent performs no optimistic card mutation"),
		AfterRejectedSlot != nullptr && AfterRejectedSlot->bOccupied
			&& AfterRejectedSlot->Card.CardId == DeployedCardId
			&& AfterRejectedSlot->Card.SkillLabels
				== OrdinaryCardView.SkillLabels);

	FFMCodexLocalMatchCardView GoalkeeperSource;
	GoalkeeperSource.CardId = TEXT("TestGK");
	GoalkeeperSource.DisplayLabel = TEXT("Card TestGK");
	GoalkeeperSource.CompactRoleLabel = TEXT("GK");
	GoalkeeperSource.SkillLabels = { TEXT("Cross"), TEXT("Through Ball") };
	GoalkeeperSource.SkillSummaryLabel = TEXT("Cross  |  Through Ball");
	GoalkeeperSource.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperSource.GoalkeeperAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 2 | 1v1 2");
	GoalkeeperSource.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	GoalkeeperSource.StatusSummaryLabel = TEXT("GK USED  |  GK ACTIVE");
	GoalkeeperSource.bGoalkeeper = true;
	const FFMCodexUMGCardViewModel GoalkeeperDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(GoalkeeperSource);
	TestTrue(TEXT("UMG DTO retains complete multi-Skill GK presentation"),
		GoalkeeperDTO.bGoalkeeper && GoalkeeperDTO.RoleLabel == TEXT("GK")
			&& GoalkeeperDTO.SkillLabels.Num() == 2
			&& GoalkeeperDTO.FullAttributeSummary
				== GoalkeeperSource.GoalkeeperAttributeSummary
			&& GoalkeeperDTO.StatusLabels == GoalkeeperSource.StatusLabels);
	const FFMCodexUMGCardViewModel MissingCardDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard({});
	TestTrue(TEXT("UMG Card DTO missing-data fallback is bounded"),
		MissingCardDTO.IdentityLabel == TEXT("UNKNOWN CARD")
			&& MissingCardDTO.RoleLabel == TEXT("ROLE N/A")
			&& MissingCardDTO.SkillSummaryLabel == TEXT("NO SKILL")
			&& MissingCardDTO.StatusSummaryLabel == TEXT("UNAVAILABLE"));

	FFMCodexLocalMatchResolutionFeedback FeedbackFixture;
	FeedbackFixture.bVisible = true;
	FeedbackFixture.bTerminal = true;
	FeedbackFixture.StepTitle = TEXT("STEP FIXTURE");
	FeedbackFixture.DiceEntries.Add({
		EFMCodexLocalMatchRollGroup::PostRoute, TEXT("Fixture"), 6 });
	FeedbackFixture.DecisionSummary = TEXT("DECISION FIXTURE");
	FeedbackFixture.TerminalSummary = TEXT("RESULT: GOAL");
	const FFMCodexUMGMatchScreenViewModel FeedbackDTO =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(), FeedbackFixture,
			Controller->GetLastDiagnostic().Message);
	TestTrue(TEXT("UMG resolution DTO preserves Step/Dice/Decision/Terminal"),
		FeedbackDTO.Resolution.bVisible
			&& FeedbackDTO.Resolution.StepLabel == TEXT("STEP FIXTURE")
			&& FeedbackDTO.Resolution.DiceLabels.Num() == 1
			&& FeedbackDTO.Resolution.DiceLabels[0].Contains(TEXT("D6 6"))
			&& FeedbackDTO.Resolution.DecisionLabel
				== TEXT("DECISION FIXTURE")
			&& FeedbackDTO.Resolution.TerminalLabel == TEXT("RESULT: GOAL"));

	FString RootWidgetHeader;
	FString RootWidgetSource;
	FString CardWidgetSource;
	FString PresentationHeader;
	FString PresentationSource;
	FString ControllerSource;
	FString BuildRules;
	TestTrue(TEXT("UMG source boundary files load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
			RootWidgetHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h"),
				PresentationHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.cpp"),
				PresentationSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/FMCodex.Build.cs"),
				BuildRules));
	const FString WidgetSources = RootWidgetHeader + RootWidgetSource
		+ CardWidgetSource;
	TestTrue(TEXT("Widget has no Session/provider/State authority escape"),
		!WidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !WidgetSources.Contains(TEXT("D6Provider"))
			&& !WidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !WidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Widget contains no rule, Formula, route, or legality logic"),
		!WidgetSources.Contains(TEXT("FormulaResolver"))
			&& !WidgetSources.Contains(TEXT("Legality"))
			&& !WidgetSources.Contains(TEXT("RandRange"))
			&& !WidgetSources.Contains(TEXT("RouteThreshold")));
	TestTrue(TEXT("Widget exposes explicit typed intents, not generic dispatch"),
		RootWidgetHeader.Contains(TEXT("RequestStartNewMatch"))
			&& RootWidgetHeader.Contains(TEXT("RequestRollTacticalPoints"))
			&& RootWidgetHeader.Contains(TEXT("RequestDeployOrdinary"))
			&& !WidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !WidgetSources.Contains(TEXT("ProcessEvent"))
			&& !WidgetSources.Contains(TEXT("FindFunction")));
	TestTrue(TEXT("Presentation DTO is bounded and never mirrors MatchPlay State"),
		PresentationHeader.Contains(TEXT("FFMCodexUMGMatchHeaderViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGCardViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGPitchSlotViewModel"))
			&& PresentationHeader.Contains(TEXT("FFMCodexUMGResolutionViewModel"))
			&& !PresentationHeader.Contains(TEXT("FMatchPlayState")));
	TestTrue(TEXT("Controller owns UMG lifecycle and preserves Slate fallback"),
		ControllerSource.Contains(TEXT("InitializePlayerFacingUI"))
			&& ControllerSource.Contains(TEXT("RefreshPlayerMatchScreen"))
			&& ControllerSource.Contains(TEXT("PlayerMatchScreen->RemoveFromParent"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface"))
			&& ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("bEnableDeveloperSlateSurface")));
	TestTrue(TEXT("UMG is the only added module dependency"),
		BuildRules.Contains(TEXT("\"Slate\", \"SlateCore\", \"UMG\""))
			&& !BuildRules.Contains(
				TEXT("PrivateDependencyModuleNames.Add(\"OnlineSubsystemSteam\")")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGPitchWidgetVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.29.UMGPitchWidgetVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGPitchWidgetVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Pitch foundation Host exists"), Host);
	TestNotNull(TEXT("Pitch foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Root UMG screen exists for Pitch composition"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Root composes dedicated Pitch Widget"), Pitch);
	if (Pitch == nullptr)
	{
		return false;
	}
	Pitch->TakeWidget();
	TestNotNull(TEXT("Root keeps dedicated Pitch region"),
		Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")));

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("Pitch stage exposes Tactical Point roll without a modal"),
		Screen->GetPresentation().Interaction.bCanRollTacticalPoints);
	Screen->RequestRollTacticalPoints();
	TestTrue(TEXT("Pitch fixture reaches authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy);

	const TArray<FFMCodexUMGPitchRegionViewModel>& PitchPresentation =
		Pitch->GetPresentation();
	const bool bLocalViewerIsPlayerA =
		Screen->GetPresentation().LocalPlayerLabel == TEXT("Player A");
	TestTrue(TEXT("Local viewer is the left lane without mutating physical identity"),
		PitchPresentation.Num() == 2
			&& PitchPresentation[0].RegionLabel
				== (bLocalViewerIsPlayerA
					? TEXT("Half Near Player A") : TEXT("Half Near Player B"))
			&& PitchPresentation[1].RegionLabel
				== (bLocalViewerIsPlayerA
					? TEXT("Half Near Player B") : TEXT("Half Near Player A"))
			&& PitchPresentation[0].bLocalFacingLane
			&& !PitchPresentation[1].bLocalFacingLane
			&& Pitch->GetWidgetFromName(TEXT("PlayerBPhysicalHalf")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerAPhysicalHalf")) != nullptr);
	TestNotNull(TEXT("Pitch contains visual-only center field separator"),
		Pitch->GetWidgetFromName(TEXT("PhysicalHalfVisualSeparator")));

	const TArray<TObjectPtr<UFMCodexPitchSlotWidget>>& RenderedSlots =
		Pitch->GetRenderedSlotWidgets();
	int32 DTOCount = 0;
	TArray<FName> ExpectedSlotOrder;
	for (const FFMCodexUMGPitchRegionViewModel& Region : PitchPresentation)
	{
		DTOCount += Region.Slots.Num();
		for (const FFMCodexUMGPitchSlotViewModel& PitchSlot : Region.Slots)
		{
			ExpectedSlotOrder.Add(PitchSlot.SlotId);
		}
	}
	TestTrue(TEXT("Dedicated Pitch Widget renders 10/10 canonical slots"),
		DTOCount == 10 && RenderedSlots.Num() == 10);
	bool bDeterministicOrder = RenderedSlots.Num() == ExpectedSlotOrder.Num();
	for (int32 Index = 0;
		bDeterministicOrder && Index < RenderedSlots.Num(); ++Index)
	{
		bDeterministicOrder = RenderedSlots[Index] != nullptr
			&& RenderedSlots[Index]->GetPresentation().SlotId
				== ExpectedSlotOrder[Index];
	}
	TestTrue(TEXT("Pitch slots preserve deterministic catalog ordering"),
		bDeterministicOrder);
	bool bVerticalLanePerHalf = RenderedSlots.Num() == 10;
	for (int32 Index = 0;
		bVerticalLanePerHalf && Index < RenderedSlots.Num(); ++Index)
	{
		const UUniformGridSlot* GridSlot = RenderedSlots[Index] == nullptr
			? nullptr : Cast<UUniformGridSlot>(RenderedSlots[Index]->Slot);
		bVerticalLanePerHalf = GridSlot != nullptr
			&& GridSlot->GetRow() == Index % 5
			&& GridSlot->GetColumn() == 0;
	}
	TestTrue(TEXT("Each physical half renders one vertical lane of five slots"),
		bVerticalLanePerHalf
			&& PitchPresentation[0].Slots.Num() == 5
			&& PitchPresentation[1].Slots.Num() == 5);

	int32 EmptySlots = 0;
	for (UFMCodexPitchSlotWidget* SlotWidget : RenderedSlots)
	{
		if (SlotWidget == nullptr)
		{
			continue;
		}
		SlotWidget->TakeWidget();
		if (!SlotWidget->GetPresentation().bOccupied)
		{
			++EmptySlots;
			TestFalse(TEXT("Empty slot never fabricates a Card binding"),
				SlotWidget->IsShowingOccupiedCard());
			TestNotNull(TEXT("Empty slot exposes non-action spatial state"),
				SlotWidget->GetWidgetFromName(TEXT("EmptySpatialLocation")));
		}
	}
	TestEqual(TEXT("Initial deployment pitch has 10 empty locations"),
		EmptySlots, 10);

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("Pitch correspondence finds legal deployment candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FFMCodexLocalMatchCardView DeployedCardSource = OrdinaryGroup->Card;
	const FFMCodexLocalMatchSlotView Destination = OrdinaryGroup->LegalSlots[0];
	auto FindRenderedSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName SlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == SlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};
	UFMCodexPitchSlotWidget* DestinationBefore =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestNotNull(TEXT("Every legal destination corresponds to a visible slot"),
		DestinationBefore);
	if (DestinationBefore != nullptr)
	{
		const FFMCodexUMGPitchSlotViewModel& DestinationPresentation =
			DestinationBefore->GetPresentation();
		const FString ExpectedZone =
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Destination.RelativeZone);
		const FString& ActingSideZone =
			DeploymentView.ExpectedActingPlayer
				== EInitialTurnOrderPlayer::PlayerA
					? DestinationPresentation.PlayerARelativeZoneLabel
					: DestinationPresentation.PlayerBRelativeZoneLabel;
		TestTrue(TEXT("Destination and Pitch share physical half and relative zone"),
			DestinationPresentation.PhysicalHalfLabel
				== FFMCodexLocalMatchInteractionViewBuilder::ToString(
					Destination.NeutralSide)
				&& ActingSideZone == ExpectedZone);
	}

	int32 AttackingRegionCount = 0;
	for (const FFMCodexUMGPitchRegionViewModel& Region : PitchPresentation)
	{
		AttackingRegionCount += Region.bCurrentAttackingSide ? 1 : 0;
		TestTrue(TEXT("Each Pitch half retains readable relative-zone context"),
			!Region.ZoneContextLabel.IsEmpty()
				&& Region.ZoneContextLabel.Contains(TEXT("Player A:"))
				&& Region.ZoneContextLabel.Contains(TEXT("Player B:")));
	}
	TestEqual(TEXT("Exactly one physical half receives attacker emphasis"),
		AttackingRegionCount, 1);
	const FString ExpectedAttackingHalf =
		DeploymentView.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
				? TEXT("Half Near Player A") : TEXT("Half Near Player B");
	const int32 ExpectedAttackingRegion = PitchPresentation.IndexOfByPredicate(
		[&ExpectedAttackingHalf](const FFMCodexUMGPitchRegionViewModel& Region)
		{
			return Region.PhysicalHalfLabel == ExpectedAttackingHalf;
		});
	TestTrue(TEXT("Attacker emphasis comes from DTO, never Widget calculation"),
		PitchPresentation.IsValidIndex(ExpectedAttackingRegion)
			&& PitchPresentation[ExpectedAttackingRegion].bCurrentAttackingSide);

	TSet<FString> SkillFamilies;
	for (const FFMCodexUMGCardViewModel& Candidate
		: Screen->GetPresentation().Interaction.CandidateCards)
	{
		for (const FString& Skill : Candidate.SkillLabels)
		{
			SkillFamilies.Add(Skill);
		}
	}
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("Pitch stage preserves %s family"), Family),
			SkillFamilies.Contains(Family));
	}

	const TArray<uint8> BeforeDeployment =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardSource.CardId, Destination.SlotId);
	TestTrue(TEXT("Successful deployment refreshes Pitch from authority"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeDeployment
				!= SerializeState(Host->GetMatchSnapshot().Snapshot));
	UFMCodexPitchSlotWidget* DestinationAfter =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestNotNull(TEXT("Refreshed occupied slot remains visible"), DestinationAfter);
	if (DestinationAfter != nullptr)
	{
		DestinationAfter->TakeWidget();
		UFMCodexPlayerCardWidget* PitchCard =
			DestinationAfter->GetCardWidget();
		TestNotNull(TEXT("Occupied Slot binds shared Player Card Widget"), PitchCard);
		TestTrue(TEXT("Pitch Card binding preserves Stage 6.3 Card contract"),
			DestinationAfter->IsShowingOccupiedCard()
				&& PitchCard != nullptr
				&& PitchCard->GetPresentation().CardId
					== DeployedCardSource.CardId
				&& PitchCard->GetPresentation().IdentityLabel
					== DeployedCardSource.DisplayLabel
				&& PitchCard->GetPresentation().RoleLabel
					== DeployedCardSource.CompactRoleLabel
				&& PitchCard->GetPresentation().SkillLabels
					== DeployedCardSource.SkillLabels
				&& PitchCard->GetPresentation().CompactAttributeSummary
					== DeployedCardSource.CompactAttributeSummary
				&& PitchCard->GetPresentation().StatusLabels.Contains(
					TEXT("DEPLOYED")));
	}

	TArray<FName> OccupancyOrderBeforeRejected;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		OccupancyOrderBeforeRejected.Add(
			SlotWidget != nullptr
				? SlotWidget->GetPresentation().SlotId : NAME_None);
	}
	const TArray<uint8> BeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(DeployedCardSource.CardId, Destination.SlotId);
	TArray<FName> OccupancyOrderAfterRejected;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		OccupancyOrderAfterRejected.Add(
			SlotWidget != nullptr
				? SlotWidget->GetPresentation().SlotId : NAME_None);
	}
	UFMCodexPitchSlotWidget* DestinationRejected =
		FindRenderedSlot(*Pitch, Destination.SlotId);
	TestTrue(TEXT("Rejected deployment causes no optimistic Pitch movement"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& BeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& OccupancyOrderBeforeRejected == OccupancyOrderAfterRejected
			&& DestinationRejected != nullptr
			&& DestinationRejected->GetPresentation().bOccupied
			&& DestinationRejected->GetPresentation().Card.CardId
				== DeployedCardSource.CardId);

	FFMCodexUMGPitchSlotViewModel GoalkeeperSlotFixture =
		Pitch->GetPresentation()[0].Slots[0];
	GoalkeeperSlotFixture.bOccupied = true;
	GoalkeeperSlotFixture.Card.CardId = TEXT("PitchGK");
	GoalkeeperSlotFixture.Card.IdentityLabel = TEXT("Card PitchGK");
	GoalkeeperSlotFixture.Card.OwnerLabel = TEXT("Player B");
	GoalkeeperSlotFixture.Card.RoleLabel = TEXT("GK");
	GoalkeeperSlotFixture.Card.SkillLabels = { TEXT("Cross"), TEXT("Through Ball") };
	GoalkeeperSlotFixture.Card.SkillSummaryLabel = TEXT("Cross | Through Ball");
	GoalkeeperSlotFixture.Card.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperSlotFixture.Card.StatusLabels = { TEXT("GK ACTIVE") };
	GoalkeeperSlotFixture.Card.StatusSummaryLabel = TEXT("GK ACTIVE");
	GoalkeeperSlotFixture.Card.bGoalkeeper = true;
	UFMCodexPitchSlotWidget* GoalkeeperSlotWidget =
		CreateWidget<UFMCodexPitchSlotWidget>(
			Screen->GetWorld(), UFMCodexPitchSlotWidget::StaticClass());
	TestNotNull(TEXT("Standalone GK Pitch Slot constructs"), GoalkeeperSlotWidget);
	if (GoalkeeperSlotWidget != nullptr)
	{
		GoalkeeperSlotWidget->RefreshFromPitchSlotPresentation(
			GoalkeeperSlotFixture);
		GoalkeeperSlotWidget->TakeWidget();
		const UFMCodexPlayerCardWidget* GKCard =
			GoalkeeperSlotWidget->GetCardWidget();
		TestTrue(TEXT("GK Slot retains GK role, Skills, attributes and status"),
			GoalkeeperSlotWidget->IsShowingOccupiedCard()
				&& GKCard != nullptr
				&& GKCard->GetPresentation().bGoalkeeper
				&& GKCard->GetPresentation().RoleLabel == TEXT("GK")
				&& GKCard->GetPresentation().SkillLabels.Num() == 2
				&& GKCard->GetPresentation().CompactAttributeSummary
					== GoalkeeperSlotFixture.Card.CompactAttributeSummary
				&& GKCard->GetPresentation().StatusSummaryLabel
					== TEXT("GK ACTIVE"));
	}

	FString PitchHeader;
	FString PitchSource;
	FString SlotHeader;
	FString SlotSource;
	FString RootHeader;
	FString RootSource;
	FString ControllerSource;
	TestTrue(TEXT("Pitch boundary production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.h"), PitchHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"), PitchSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.h"), SlotHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"), SlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString PitchWidgetSources = PitchHeader + PitchSource
		+ SlotHeader + SlotSource;
	TestTrue(TEXT("Pitch/Slot Widgets receive presentation DTOs only"),
		PitchHeader.Contains(TEXT("FFMCodexUMGPitchRegionViewModel"))
			&& SlotHeader.Contains(TEXT("FFMCodexUMGPitchSlotViewModel"))
			&& !PitchWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !PitchWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !PitchWidgetSources.Contains(TEXT("D6Provider"))
			&& !PitchWidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Pitch/Slot Widgets contain zero gameplay authority or input"),
		!PitchWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !PitchWidgetSources.Contains(TEXT("Legality"))
			&& !PitchWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !PitchWidgetSources.Contains(TEXT("RandRange"))
			&& !PitchWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !PitchWidgetSources.Contains(TEXT("UButton"))
			&& !PitchWidgetSources.Contains(TEXT("OnClicked"))
			&& !PitchWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Pitch shell uses dynamic landmarks without gameplay geography"),
		PitchSource.Contains(TEXT("FootballFieldBackground"))
			&& PitchSource.Contains(TEXT("PlayerBPhysicalHalf"))
			&& PitchSource.Contains(TEXT("PhysicalHalfVisualSeparator"))
			&& PitchSource.Contains(TEXT("PlayerAPhysicalHalf"))
			&& PitchSource.Contains(TEXT("PitchSemanticLabel"))
			&& PitchSource.Contains(TEXT("OpponentForward"))
			&& PitchSource.Contains(TEXT("LocalBackfield"))
			&& PitchSource.Contains(TEXT("GoalVisual"))
			&& !PitchSource.Contains(TEXT("PitchCenterCircleVisual"))
			&& !PitchSource.Contains(TEXT("PitchAttackingHalf"))
			&& !PitchSource.Contains(TEXT("Backfield row"))
			&& !PitchSource.Contains(TEXT("Forward row")));
	TestTrue(TEXT("Root delegates Pitch layout to dedicated configurable class"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexPitchWidget>"))
			&& RootSource.Contains(TEXT("DedicatedFootballPitchWidget"))
			&& RootSource.Contains(TEXT("RefreshFromPitchPresentation"))
			&& !RootSource.Contains(TEXT("UWrapBox* SlotRow")));
	TestTrue(TEXT("Controller remains unaware of Pitch geometry"),
		!ControllerSource.Contains(TEXT("UFMCodexPitchWidget"))
			&& !ControllerSource.Contains(TEXT("UniformGrid"))
			&& !ControllerSource.Contains(TEXT("SlotIndex / 5")));
	TestTrue(TEXT("Slate developer surface remains unchanged and available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGPlayerCardVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.30.UMGPlayerCardVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGPlayerCardVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Card foundation Host exists"), Host);
	TestNotNull(TEXT("Card foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Card foundation root UMG screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel MultiSkillFixture;
	MultiSkillFixture.CardId = TEXT("VisualCard");
	MultiSkillFixture.IdentityLabel = TEXT("Card VisualCard");
	MultiSkillFixture.OwnerLabel = TEXT("Player A");
	MultiSkillFixture.RoleLabel = TEXT("FW / MF");
	MultiSkillFixture.RarityLabel = TEXT("National");
	MultiSkillFixture.SkillLabels = {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball")
	};
	MultiSkillFixture.SkillSummaryLabel = FString::Join(
		MultiSkillFixture.SkillLabels, TEXT(" | "));
	MultiSkillFixture.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 2");
	MultiSkillFixture.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 2 | MRK 1 | TKL 1 | SPD 2 | STR 3 | STA 4 | LS 5");
	MultiSkillFixture.StatusLabels = { TEXT("AVAILABLE") };
	MultiSkillFixture.StatusSummaryLabel = TEXT("AVAILABLE");
	MultiSkillFixture.DeveloperReferenceLabel =
		TEXT("Card reference: VisualCard | Rarity: National");

	UFMCodexPlayerCardWidget* CardWidget =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	TestNotNull(TEXT("Reusable Player Card Widget constructs"), CardWidget);
	if (CardWidget == nullptr)
	{
		return false;
	}
	CardWidget->RefreshFromPresentation(
		MultiSkillFixture,
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	CardWidget->TakeWidget();
	for (const TCHAR* RegionName : {
		TEXT("PlayerCardFrame"), TEXT("RoleRarityHeaderRegion"),
		TEXT("PortraitPresentationRegion"), TEXT("CardIdentityRegion"),
		TEXT("SkillPresentationRegion"),
		TEXT("AttributePresentationRegion"),
		TEXT("StatusBadgePresentationRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Card hierarchy contains %s"),
			RegionName), CardWidget->GetWidgetFromName(RegionName));
	}
	TestTrue(TEXT("PitchCompact has bounded card and portrait shells"),
		CardWidget->GetWidgetFromName(TEXT("PlayerCardBounds")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("PortraitAssetBounds"))
				!= nullptr);
	TestTrue(TEXT("Role and rarity have independent discoverable positions"),
		CardWidget->GetWidgetFromName(TEXT("CardRole")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("CardRarity")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("RoleIconHook")) != nullptr);
	TestTrue(TEXT("Portrait and Skill future asset hooks are explicit"),
		CardWidget->GetWidgetFromName(TEXT("PortraitPlaceholderText")) != nullptr
			&& CardWidget->GetWidgetFromName(TEXT("SkillIconHook0")) != nullptr);
	TestTrue(TEXT("PitchCompact preserves every multi-Skill identity"),
		CardWidget->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::PitchCompact
			&& CardWidget->GetRenderedSkillCount()
				== MultiSkillFixture.SkillLabels.Num());
	TestTrue(TEXT("PitchCompact renders stable SHO PAS DRI SPD stat block"),
		CardWidget->GetRenderedAttributeCount() == 4
			&& CardWidget->GetRenderedAttributeSummary()
				== MultiSkillFixture.CompactAttributeSummary
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SHO"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("PAS"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("DRI"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SPD")));
	TestEqual(TEXT("Status labels become structured badge entries"),
		CardWidget->GetRenderedStatusBadgeCount(), 1);
	const UTextBlock* DeveloperReference = Cast<UTextBlock>(
		CardWidget->GetWidgetFromName(TEXT("CardDeveloperReference")));
	TestTrue(TEXT("Developer reference does not dominate PitchCompact"),
		DeveloperReference != nullptr
			&& DeveloperReference->GetVisibility()
				== ESlateVisibility::Collapsed);

	CardWidget->SetPresentationMode(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("Interaction mode preserves all ten outfield attributes"),
		CardWidget->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& CardWidget->GetRenderedAttributeCount() == 10
			&& CardWidget->GetRenderedAttributeSummary()
				== MultiSkillFixture.FullAttributeSummary);
	for (const TCHAR* Attribute : {
		TEXT("SHO"), TEXT("DRI"), TEXT("PAS"), TEXT("OFF"), TEXT("MRK"),
		TEXT("TKL"), TEXT("SPD"), TEXT("STR"), TEXT("STA"), TEXT("LS") })
	{
		TestTrue(FString::Printf(TEXT("Interaction mode retains %s"),
			Attribute),
			CardWidget->GetRenderedAttributeSummary().Contains(Attribute));
	}
	TestTrue(TEXT("Interaction Full Card omits developer references"),
		DeveloperReference != nullptr
			&& DeveloperReference->GetVisibility()
				== ESlateVisibility::Collapsed);

	FFMCodexUMGCardViewModel GoalkeeperFixture = MultiSkillFixture;
	GoalkeeperFixture.CardId = TEXT("VisualGK");
	GoalkeeperFixture.IdentityLabel = TEXT("Card VisualGK");
	GoalkeeperFixture.RoleLabel = TEXT("GK");
	GoalkeeperFixture.CompactAttributeSummary =
		TEXT("HAN 5 | REF 4 | AER 3 | 1v1 2");
	GoalkeeperFixture.FullAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 3 | 1v1 2");
	GoalkeeperFixture.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	GoalkeeperFixture.StatusSummaryLabel = TEXT("GK USED | GK ACTIVE");
	GoalkeeperFixture.bGoalkeeper = true;
	CardWidget->RefreshFromPresentation(
		GoalkeeperFixture,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("GK is a distinct presentation-only card variant"),
		CardWidget->IsGoalkeeperVisualVariant()
			&& CardWidget->GetPresentation().RoleLabel == TEXT("GK")
			&& CardWidget->GetRenderedAttributeCount() == 6
			&& CardWidget->GetRenderedStatusBadgeCount() == 0
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("HAN"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("POS"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("REF"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("AER"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("ANT"))
			&& CardWidget->GetRenderedAttributeSummary().Contains(TEXT("1v1"))
			&& !CardWidget->GetRenderedAttributeSummary().Contains(TEXT("SHO")));

	FFMCodexLocalMatchCardView MissingCardView;
	const FFMCodexUMGCardViewModel MissingFixture =
		FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(MissingCardView);
	CardWidget->RefreshFromPresentation(
		MissingFixture,
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("Missing presentation data has safe bounded fallbacks"),
		MissingFixture.IdentityLabel == TEXT("UNKNOWN CARD")
			&& MissingFixture.RoleLabel == TEXT("ROLE N/A")
			&& MissingFixture.SkillSummaryLabel == TEXT("NO SKILL")
			&& MissingFixture.CompactAttributeSummary
				== TEXT("Attributes unavailable")
			&& MissingFixture.FullAttributeSummary
				== TEXT("Attributes unavailable")
			&& MissingFixture.StatusSummaryLabel == TEXT("UNAVAILABLE")
			&& MissingFixture.RarityLabel == TEXT("RARITY N/A")
			&& CardWidget->GetRenderedSkillCount() == 0
			&& CardWidget->GetRenderedAttributeCount() == 0
			&& CardWidget->GetRenderedStatusBadgeCount() == 0);

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("Card stage has no full-screen handoff blocker"),
		Screen->GetPresentation().Interaction.bCanRollTacticalPoints);
	Screen->RequestRollTacticalPoints();
	TestTrue(TEXT("Card fixture reaches authoritative deployment"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::Deploy);

	const TArray<FFMCodexUMGCardViewModel>& CandidateDTOs =
		Screen->GetPresentation().Interaction.CandidateCards;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>& CandidateWidgets =
		Screen->GetLocalRackWidget()->GetRenderedCardWidgets();
	TestEqual(TEXT("Every candidate DTO receives one Card Widget"),
		CandidateWidgets.Num(), Screen->GetPresentation().LocalRack.Cells.Num());
	bool bCandidateBindingsExact = CandidateWidgets.Num()
		== Screen->GetPresentation().LocalRack.Cells.Num();
	TSet<FString> CandidateSkillFamilies;
	for (int32 Index = 0; Index < CandidateWidgets.Num(); ++Index)
	{
		const UFMCodexPlayerCardWidget* CandidateWidget = CandidateWidgets[Index];
		bCandidateBindingsExact = bCandidateBindingsExact
			&& CandidateWidget != nullptr
			&& CandidateWidget->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::HandMicro;
		for (const FString& Skill : CandidateWidget->GetPresentation().SkillLabels)
		{
			CandidateSkillFamilies.Add(Skill);
		}
	}
	TestTrue(TEXT("Persistent rack cards use shared DTO in HandMicro mode"),
		bCandidateBindingsExact);
	UFMCodexPlayerCardWidget* MicroFixture =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	if (MicroFixture == nullptr)
	{
		return false;
	}
	FFMCodexUMGCardViewModel LongMicro = MultiSkillFixture;
	LongMicro.CardId = TEXT("Demo.A.Outfield.19");
	LongMicro.IdentityLabel =
		TEXT("CardDemo.A.Outfield.19.InternalDeveloperIdentifier");
	LongMicro.DeveloperReferenceLabel =
		TEXT("CardId=Demo.A.Outfield.19 | fallback diagnostic text");
	MicroFixture->RefreshFromPresentation(
		LongMicro, EFMCodexPlayerCardPresentationMode::HandMicro);
	MicroFixture->TakeWidget();
	const UTextBlock* MicroName = Cast<UTextBlock>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroPlayerName")));
	const UTextBlock* MicroPosition = Cast<UTextBlock>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroPosition")));
	const USizeBox* MicroPortraitBounds = Cast<USizeBox>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroPortraitBounds")));
	const UBorder* MicroRarity = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroRarityAccent")));
	const USizeBox* MicroRarityBounds = Cast<USizeBox>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroRarityAccentBounds")));
	const USizeBox* MicroIdentityBounds = Cast<USizeBox>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroIdentityBounds")));
	const UVerticalBox* MicroTextHierarchy = Cast<UVerticalBox>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroTextHierarchy")));
	const UOverlaySlot* MicroIdentityContentSlot = MicroTextHierarchy != nullptr
		? Cast<UOverlaySlot>(MicroTextHierarchy->Slot) : nullptr;
	const UHorizontalBoxSlot* MicroRarityBoundsSlot =
		MicroRarityBounds != nullptr
			? Cast<UHorizontalBoxSlot>(MicroRarityBounds->Slot) : nullptr;
	const UBorder* MicroPortraitAtmosphere = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroPortraitAtmosphere")));
	const UBorder* MicroSkinBackground = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinBackground")));
	const UBorder* MicroIdentitySurface = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroIdentitySurface")));
	const UHorizontalBox* MicroContent = Cast<UHorizontalBox>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroContent")));
	TestTrue(TEXT("Hand Micro implements the approved bounded 220x68 card"),
		MicroFixture->GetConfiguredDimensions().Equals(FVector2D(220.0f, 68.0f))
			&& MicroFixture->GetWidgetFromName(TEXT("PlayerCardBounds"))
				->GetClipping() == EWidgetClipping::ClipToBounds
			&& MicroName != nullptr
			&& MicroName->GetClipping() == EWidgetClipping::Inherit
			&& MicroPosition != nullptr
			&& MicroPosition->GetTextOverflowPolicy() == ETextOverflowPolicy::Ellipsis
			&& MicroPosition->GetText().ToString() == TEXT("A/M")
			&& MicroPortraitBounds != nullptr
			&& MicroPortraitBounds->GetWidthOverride() == 96.0f
			&& MicroPortraitBounds->GetHeightOverride() == 64.0f
			&& MicroIdentityBounds != nullptr
			&& MicroIdentityBounds->GetWidthOverride() == 120.0f
			&& MicroIdentityBounds->GetHeightOverride() == 68.0f
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroPortraitCrop"))
				== nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroPortraitScale"))
				== nullptr
			&& MicroRarity != nullptr && MicroRarityBounds != nullptr
			&& MicroRarityBounds->GetWidthOverride() == 4.0f
			&& MicroRarityBounds->GetHeightOverride() == 68.0f
			&& MicroRarityBoundsSlot != nullptr
			&& MicroRarityBoundsSlot->GetHorizontalAlignment() == HAlign_Fill
			&& MicroRarityBoundsSlot->GetVerticalAlignment() == VAlign_Center
			&& MicroContent != nullptr
			&& MicroContent->GetChildrenCount() == 3
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroVisualSystem"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinBackground"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroIdentitySurface"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(
				TEXT("HandMicroIdentityMaterialLayers")) != nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroIdentityTechLine"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroIdentityDivider"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroRaritySystem"))
				== nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroRarityTrack"))
				== nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroRarityInset"))
				== nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinChrome"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinTopRail"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinBottomRail"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinLeftRail"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroSkinRightRail"))
				!= nullptr
			&& MicroFixture->GetWidgetFromName(TEXT("HandMicroRarityBadge"))
				== nullptr);
	TestTrue(TEXT("Dedicated portrait art is not compensated by a runtime atmosphere overlay"),
		MicroPortraitAtmosphere != nullptr
			&& MicroPortraitAtmosphere->GetBrushColor().A == 0.0f);
	TestTrue(TEXT("Hand Micro draft colors and identity safe area are exact"),
		MicroSkinBackground != nullptr
			&& MicroSkinBackground->GetBrushColor().ToFColorSRGB()
				== FColor(0x0C, 0x23, 0x30)
			&& MicroIdentitySurface != nullptr
			&& MicroIdentitySurface->GetBrushColor().ToFColorSRGB()
				== FColor(0x1C, 0x35, 0x42)
			&& MicroIdentityContentSlot != nullptr
			&& MicroIdentityContentSlot->GetPadding()
				== FMargin(4.0f, 7.0f, 4.0f, 7.0f)
			&& MicroName != nullptr
			&& MicroName->GetColorAndOpacity().GetSpecifiedColor().ToFColorSRGB()
				== FColor(0xF2, 0xF6, 0xF8)
			&& MicroPosition != nullptr
			&& MicroPosition->GetColorAndOpacity().GetSpecifiedColor().ToFColorSRGB()
				== FColor(0xC6, 0xD3, 0xDA));
	constexpr float HandMicroContentWidth = 220.0f;
	TestTrue(TEXT("Hand Micro draft share stays within the two-percent tolerance"),
		FMath::IsNearlyEqual(96.0f / HandMicroContentWidth, 0.4364f, 0.001f)
			&& FMath::IsNearlyEqual(120.0f / HandMicroContentWidth, 0.5455f, 0.001f)
			&& FMath::IsNearlyEqual(4.0f / HandMicroContentWidth, 0.0182f, 0.001f));
	TestTrue(TEXT("Hand Micro identity is a left-aligned name-first hierarchy"),
		MicroName != nullptr && MicroPosition != nullptr
			&& MicroName->GetFont().Size >= 12
			&& MicroName->GetFont().Size <= 16
			&& MicroName->GetFont().TypefaceFontName == TEXT("Medium")
			&& MicroPosition->GetFont().Size == 14
			&& MicroPosition->GetFont().TypefaceFontName == TEXT("Medium")
			&& MicroName->GetColorAndOpacity().GetSpecifiedColor().R
				> MicroPosition->GetColorAndOpacity().GetSpecifiedColor().R
			&& MicroTextHierarchy != nullptr
			&& MicroTextHierarchy->GetChildrenCount() == 2
			&& MicroTextHierarchy->GetChildAt(0) == MicroName
			&& MicroTextHierarchy->GetChildAt(1)
				== MicroFixture->GetWidgetFromName(TEXT("HandMicroPositionLine"))
			&& MicroIdentityContentSlot != nullptr
			&& MicroIdentityContentSlot->GetHorizontalAlignment() == HAlign_Fill
			&& MicroIdentityContentSlot->GetVerticalAlignment() == VAlign_Center);
	TestTrue(TEXT("Hand Micro removes technical and full-detail leakage"),
		MicroName != nullptr
			&& !MicroName->GetText().ToString().Contains(TEXT("Demo."))
			&& !MicroName->GetText().ToString().Contains(TEXT("CardId"))
			&& !MicroName->GetText().ToString().Contains(TEXT("Developer"))
			&& MicroFixture->GetRenderedSkillCount() == 0
			&& MicroFixture->GetRenderedAttributeCount() == 0
			&& MicroFixture->GetRenderedStatusBadgeCount() == 0
			&& MicroFixture->GetWidgetFromName(TEXT("CardDeveloperReference"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& MicroFixture->GetWidgetFromName(TEXT("PortraitPlaceholderText"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& MicroFixture->GetWidgetFromName(TEXT("CardContentReadabilityLayer"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	TestEqual(TEXT("Prototype Hand Micro uses common short name"),
		FFMCodexPlayerUIPresentationText::CompactPlayerName(
			TEXT("Prototype.Arsenal.BukayoSaka"), FString()).ToString(),
		FString(TEXT("\u8428\u5361")));
	TestEqual(TEXT("Validation alias is bounded to Hand Micro presentation"),
		FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
			TEXT("Demo.A.Outfield.01"), FString()).ToString(),
		FString(TEXT("\u9A6C\u4E01\u5185\u5229")));
	TestEqual(TEXT("Pitch Mini keeps the canonical compact demo identity"),
		FFMCodexPlayerUIPresentationText::CompactPlayerName(
			TEXT("Demo.A.Outfield.01"), FString()).ToString(),
		FString(TEXT("A\u961F 01\u53F7")));
	FString HandMicroWidgetSource;
	TestTrue(TEXT("Hand Micro widget production source loads"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			HandMicroWidgetSource));
	const int32 NameFitStart = HandMicroWidgetSource.Find(
		TEXT("int32 GetHandMicroNameFontSize"));
	const int32 NameFitEnd = HandMicroWidgetSource.Find(
		TEXT("bool DoesHandMicroNameRequireEllipsis"),
		ESearchCase::CaseSensitive, ESearchDir::FromStart, NameFitStart);
	const FString NameFitSource = NameFitStart != INDEX_NONE
		&& NameFitEnd > NameFitStart
		? HandMicroWidgetSource.Mid(NameFitStart, NameFitEnd - NameFitStart)
		: FString();
	TestTrue(TEXT("Hand Micro Auto-Fit uses real Slate metrics from 16 to 12"),
		FMCodexHandMicroDiagnostics::StandardNameFontSize == 16
			&& FMCodexHandMicroDiagnostics::MinimumNameFontSize == 12
			&& HandMicroWidgetSource.Contains(TEXT("MaximumFontSize"))
			&& HandMicroWidgetSource.Contains(TEXT("MinimumNameFontSize"))
			&& NameFitSource.Contains(TEXT("TryMeasureHandMicroName"))
			&& NameFitSource.Contains(TEXT("MeasuredWidth <= HandMicroNameSafeWidth"))
			&& !NameFitSource.Contains(TEXT(".Len()")));
	for (const TPair<FString, FString>& PositionCase : {
		TPair<FString, FString>(TEXT("MD"), TEXT("M/D")),
		TPair<FString, FString>(TEXT("AM"), TEXT("A/M")),
		TPair<FString, FString>(TEXT("AMD"), TEXT("A/M/D")),
		TPair<FString, FString>(TEXT("AD"), TEXT("A/D")) })
	{
		TestEqual(FString::Printf(TEXT("Hand Micro role %s uses slash notation"),
			*PositionCase.Key),
			FFMCodexPlayerUIPresentationText::HandMicroCompactRole(
				PositionCase.Key).ToString(), PositionCase.Value);
	}
	struct FHandMicroNameMetricCase
	{
		FName CardId;
		FString PrimaryName;
		FString ExpectedDisplayName;
		bool bExpectedFallback;
	};
	const TArray<FHandMicroNameMetricCase> HandMicroNameMetricCases = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("\u62C9\u4E9A"),
			TEXT("\u62C9\u4E9A"), false },
		{ TEXT("Prototype.Arsenal.DeclanRice"), TEXT("\u8D56\u65AF"),
			TEXT("\u8D56\u65AF"), false },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("\u5384\u5FB7\u9AD8"),
			TEXT("\u5384\u5FB7\u9AD8"), false },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"), TEXT("\u9A6C\u4E01\u5185\u5229"),
			TEXT("\u9A6C\u4E01\u5185\u5229"), false },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"), TEXT("\u52A0\u5E03\u91CC\u57C3\u5C14"),
			TEXT("\u52A0\u5E03\u91CC\u57C3\u5C14"), false },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"), TEXT("\u683C\u74E6\u8FEA\u5965\u5C14"),
			TEXT("\u683C\u74E6\u8FEA\u5965\u5C14"), false },
		{ TEXT("Visual.HandMicro.Kvaratskhelia"),
			TEXT("\u514B\u74E6\u62C9\u8328\u8D6B\u5229\u4E9A"),
			TEXT("\u514B\u74E6\u62C9\u8328\u8D6B\u5229\u4E9A"), false }
	};
	for (const FHandMicroNameMetricCase& NameCase : HandMicroNameMetricCases)
	{
		UFMCodexPlayerCardWidget* NameFixture =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		FFMCodexUMGCardViewModel NameCard = LongMicro;
		NameCard.CardId = NameCase.CardId;
		NameCard.RoleLabel = TEXT("FW / MF / DF");
		NameFixture->RefreshFromPresentation(
			NameCard, EFMCodexPlayerCardPresentationMode::HandMicro);
		NameFixture->TakeWidget();
		NameFixture->ForceLayoutPrepass();
		const UTextBlock* NameText = Cast<UTextBlock>(
			NameFixture->GetWidgetFromName(TEXT("HandMicroPlayerName")));
		const UTextBlock* PositionText = Cast<UTextBlock>(
			NameFixture->GetWidgetFromName(TEXT("HandMicroPosition")));
		float DisplayedWidth = TNumericLimits<float>::Max();
		float PrimaryWidthAtFloor = TNumericLimits<float>::Max();
		if (NameText != nullptr && FSlateApplication::IsInitialized()
			&& FSlateApplication::Get().GetRenderer() != nullptr)
		{
			const TSharedRef<FSlateFontMeasure> FontMeasure =
				FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			DisplayedWidth = FontMeasure->Measure(
				NameText->GetText(), NameText->GetFont()).X;
			FSlateFontInfo FloorFont = NameText->GetFont();
			FloorFont.Size = 12;
			PrimaryWidthAtFloor = FontMeasure->Measure(
				FText::FromString(NameCase.PrimaryName), FloorFont).X;
		}
		if (NameText != nullptr)
		{
			AddInfo(FString::Printf(
				TEXT("HAND_MICRO_NAME_METRIC primary=%s displayed=%s selected_size=%.1f displayed_width=%.2f primary_width_at_12=%.2f fallback=%s"),
				*NameCase.PrimaryName, *NameText->GetText().ToString(),
				NameText->GetFont().Size, DisplayedWidth, PrimaryWidthAtFloor,
				NameCase.bExpectedFallback ? TEXT("yes") : TEXT("no")));
		}
		TestTrue(FString::Printf(TEXT("Hand Micro name %s follows measured fit policy"),
			*NameCase.PrimaryName),
			NameText != nullptr
				&& NameText->GetText().ToString() == NameCase.ExpectedDisplayName
				&& NameText->GetFont().Size >= 12
				&& NameText->GetFont().Size <= 16
				&& DisplayedWidth <= 112.0f
				&& NameText->GetTextOverflowPolicy() == ETextOverflowPolicy::Clip
				&& NameText->GetClipping() == EWidgetClipping::Inherit
				&& PositionText != nullptr
				&& PositionText->GetText().ToString() == TEXT("A/M/D")
				&& (NameCase.bExpectedFallback
					? PrimaryWidthAtFloor > 112.0f
						&& !FFMCodexPlayerUIPresentationText::
							HandMicroFallbackPlayerName(NameCase.CardId).IsEmpty()
					: NameText->GetText().ToString() == NameCase.PrimaryName));
	}
	UFMCodexPlayerCardWidget* OverflowNameFixture =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	FFMCodexUMGCardViewModel OverflowNameCard = LongMicro;
	OverflowNameCard.CardId = TEXT("Visual.HandMicro.Overlong");
	OverflowNameCard.IdentityLabel = TEXT("ExtraordinarilyLongPlayerIdentity");
	OverflowNameFixture->RefreshFromPresentation(
		OverflowNameCard, EFMCodexPlayerCardPresentationMode::HandMicro);
	OverflowNameFixture->TakeWidget();
	const UTextBlock* OverflowNameText = Cast<UTextBlock>(
		OverflowNameFixture->GetWidgetFromName(TEXT("HandMicroPlayerName")));
	TestTrue(TEXT("Hand Micro keeps the readable floor and clips only as a last resort"),
		OverflowNameText != nullptr
			&& OverflowNameText->GetFont().Size == 12
			&& OverflowNameText->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Clip);
	const TArray<FName> RepresentativePortraits = {
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.MikelMerino"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.JeremyDoku")
	};
	constexpr float HandMicroPortraitTop = 0.0f;
	constexpr float HandMicroPortraitUVHeight = 1.0f;
	bool bRepresentativePortraitsShareFocalContract = true;
	for (const FName PortraitCardId : RepresentativePortraits)
	{
		UFMCodexPlayerCardWidget* PortraitMicroFixture =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		FFMCodexUMGCardViewModel PortraitMicro = LongMicro;
		PortraitMicro.CardId = PortraitCardId;
		PortraitMicroFixture->RefreshFromPresentation(
			PortraitMicro, EFMCodexPlayerCardPresentationMode::HandMicro);
		PortraitMicroFixture->TakeWidget();
		const FFMCodexPlayerUICardArtReferences PortraitArt =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(PortraitCardId);
		const FString PortraitAssetPath =
			PortraitArt.HandMicroPortrait.ToSoftObjectPath().ToString();
		bRepresentativePortraitsShareFocalContract =
			bRepresentativePortraitsShareFocalContract
			&& FMath::IsNearlyEqual(
				PortraitArt.HandMicroPortraitTop, HandMicroPortraitTop)
			&& FMath::IsNearlyEqual(
				PortraitArt.HandMicroPortraitUVHeight, HandMicroPortraitUVHeight)
			&& PortraitAssetPath.Contains(TEXT("HandMicroApprovedRollout"))
			&& PortraitAssetPath.Contains(TEXT("ApprovedRuntime192"));
		const UImage* MicroPortrait = Cast<UImage>(
			PortraitMicroFixture->GetWidgetFromName(
				TEXT("HandMicroFaceSafePortrait")));
		const FBox2f MicroPortraitUV = MicroPortrait != nullptr
			? static_cast<FBox2f>(MicroPortrait->GetBrush().GetUVRegion())
			: FBox2f();
		const UOverlaySlot* MicroPortraitSlot = MicroPortrait != nullptr
			? Cast<UOverlaySlot>(MicroPortrait->Slot) : nullptr;
		UTexture2D* ResolvedMicroPortrait =
			PortraitMicroFixture->GetResolvedHandMicroPortraitTexture();
		TestNotNull(FString::Printf(
			TEXT("Portrait %s resolves a dedicated Hand Micro texture"),
			*PortraitCardId.ToString()), ResolvedMicroPortrait);
		TestTrue(FString::Printf(
			TEXT("Portrait %s keeps its Micro asset separate from Full Card art"),
			*PortraitCardId.ToString()),
			ResolvedMicroPortrait != nullptr
				&& ResolvedMicroPortrait
					!= PortraitMicroFixture->GetResolvedPortraitTexture()
				&& !PortraitArt.HandMicroPortrait.IsNull());
		if (ResolvedMicroPortrait != nullptr)
		{
			const FIntPoint ImportedMicroSize =
				ResolvedMicroPortrait->GetImportedSize();
			TestEqual(FString::Printf(
				TEXT("Portrait %s imported Micro width"),
				*PortraitCardId.ToString()),
				ImportedMicroSize.X, 192);
			TestEqual(FString::Printf(
				TEXT("Portrait %s imported Micro height"),
				*PortraitCardId.ToString()),
				ImportedMicroSize.Y, 128);
			TestTrue(FString::Printf(
				TEXT("Portrait %s uses the bounded sharpness import contract"),
				*PortraitCardId.ToString()),
				ResolvedMicroPortrait->LODGroup == TEXTUREGROUP_UI
					&& ResolvedMicroPortrait->CompressionSettings == TC_BC7
					&& ResolvedMicroPortrait->MipGenSettings == TMGS_Sharpen1
					&& ResolvedMicroPortrait->Filter == TF_Trilinear
					&& ResolvedMicroPortrait->NeverStream
					&& ResolvedMicroPortrait->SRGB
					&& ResolvedMicroPortrait->LODBias == 0);
		}
		TestTrue(FString::Printf(
			TEXT("Portrait %s maps the complete 3:2 Micro source without crop"),
			*PortraitCardId.ToString()),
			MicroPortrait != nullptr
				&& MicroPortrait->GetBrush().GetResourceObject()
					== ResolvedMicroPortrait
				&& MicroPortraitSlot != nullptr
				&& MicroPortraitSlot->GetHorizontalAlignment() == HAlign_Fill
				&& MicroPortraitSlot->GetVerticalAlignment() == VAlign_Fill
				&& MicroPortrait->GetBrush().DrawAs
					== ESlateBrushDrawType::Image
				&& MicroPortraitUV.bIsValid
				&& FMath::IsNearlyEqual(MicroPortraitUV.Min.X, 0.0f)
				&& FMath::IsNearlyEqual(MicroPortraitUV.Max.X, 1.0f)
				&& FMath::IsNearlyEqual(MicroPortraitUV.Min.Y, HandMicroPortraitTop)
				&& FMath::IsNearlyEqual(MicroPortraitUV.Max.Y,
					HandMicroPortraitTop + HandMicroPortraitUVHeight)
				&& FMath::IsNearlyEqual(
					PortraitArt.HandMicroPortraitTop, HandMicroPortraitTop)
				&& FMath::IsNearlyEqual(
					PortraitArt.HandMicroPortraitUVHeight, HandMicroPortraitUVHeight)
				&& PortraitArt.HandMicroPortraitTop >= 0.0f
				&& PortraitArt.HandMicroPortraitTop
					+ PortraitArt.HandMicroPortraitUVHeight <= 1.0f
				&& FMath::IsNearlyEqual(
					1.5f / HandMicroPortraitUVHeight, 96.0f / 64.0f,
					0.001f));
	}
	TestTrue(TEXT("Representative Hand Micro portraits share one normalized focal safe area"),
		bRepresentativePortraitsShareFocalContract);
	const FFMCodexPlayerUICardArtReferences FallbackArt =
		FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(
			TEXT("Unknown.Fallback.Card"));
	const UBorder* HandMicroFallback = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("HandMicroPortraitFallback")));
	TestTrue(TEXT("Generic fallback retains a safe dark presentation focal"),
		FallbackArt.Portrait.IsNull()
			&& FallbackArt.HandMicroPortrait.IsNull()
			&& FMath::IsNearlyEqual(FallbackArt.HandMicroPortraitTop, 0.06f)
			&& FMath::IsNearlyEqual(
				FallbackArt.HandMicroPortraitUVHeight, 0.426667f)
			&& HandMicroFallback != nullptr
			&& HandMicroFallback->GetVisibility() != ESlateVisibility::Collapsed
			&& HandMicroFallback->GetBrushColor().R < 0.05f
			&& HandMicroFallback->GetBrushColor().G < 0.05f
			&& HandMicroFallback->GetBrushColor().B < 0.05f);
	const TArray<TPair<FString, FColor>> HandMicroRarityCases = {
		{ TEXT("Common"), FColor(0xFF, 0xFF, 0xFF) },
		{ TEXT("Regional"), FColor(0x1E, 0xFF, 0x00) },
		{ TEXT("National"), FColor(0x00, 0x70, 0xDD) },
		{ TEXT("Continental"), FColor(0xA3, 0x35, 0xEE) },
		{ TEXT("World Class"), FColor(0xFF, 0x80, 0x00) }
	};
	for (const TPair<FString, FColor>& RarityCase : HandMicroRarityCases)
	{
		UFMCodexPlayerCardWidget* RarityFixture =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		FFMCodexUMGCardViewModel RarityCard = LongMicro;
		RarityCard.CardId = TEXT("Prototype.Arsenal.BukayoSaka");
		RarityCard.RarityLabel = RarityCase.Key;
		RarityFixture->RefreshFromPresentation(
			RarityCard, EFMCodexPlayerCardPresentationMode::HandMicro);
		RarityFixture->TakeWidget();
		const UBorder* RarityStrip = Cast<UBorder>(
			RarityFixture->GetWidgetFromName(TEXT("HandMicroRarityAccent")));
		const FLinearColor ActualLinear = RarityStrip != nullptr
			? RarityStrip->GetBrushColor() : FLinearColor::Transparent;
		const FColor ActualSRGB = ActualLinear.ToFColorSRGB();
		TestTrue(FString::Printf(
			TEXT("Hand Micro rarity %s preserves its canonical base hue"),
			*RarityCase.Key),
			RarityStrip != nullptr
				&& ActualSRGB.R == RarityCase.Value.R
				&& ActualSRGB.G == RarityCase.Value.G
				&& ActualSRGB.B == RarityCase.Value.B
				&& FMath::IsNearlyEqual(ActualLinear.A, 0.45f));
	}
	FFMCodexUMGCardViewModel GoalkeeperMicro = LongMicro;
	GoalkeeperMicro.CardId = TEXT("Prototype.Arsenal.DavidRaya");
	GoalkeeperMicro.bGoalkeeper = true;
	MicroFixture->RefreshFromPresentation(
		GoalkeeperMicro, EFMCodexPlayerCardPresentationMode::HandMicro);
	MicroFixture->TakeWidget();
	const UBorder* HandMicroFrame = Cast<UBorder>(
		MicroFixture->GetWidgetFromName(TEXT("PlayerCardFrame")));
	TestTrue(TEXT("Hand Micro uses a restrained collectible frame instead of a rarity field"),
		HandMicroFrame != nullptr
			&& HandMicroFrame->GetBrushColor().Equals(
				FFMCodexPlayerUIStyle::Get().GetColor(
					EFMCodexPlayerUIColorRole::CardFrame))
			&& MicroFixture->GetWidgetFromName(TEXT("CardFrameAssetImage"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& MicroFixture->GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() != ESlateVisibility::Collapsed);
	for (const TCHAR* Family : {
		TEXT("Long Shot"), TEXT("Cut Inside"), TEXT("Pass Control"),
		TEXT("Cross"), TEXT("Through Ball") })
	{
		TestTrue(FString::Printf(TEXT("Card Widget supports %s family"), Family),
			CandidateSkillFamilies.Contains(Family));
	}

	const FFMCodexLocalMatchInteractionView& DeploymentView =
		Controller->GetInteractionView();
	const FFMCodexLocalMatchDeploymentGroup* OrdinaryGroup =
		DeploymentView.DeploymentGroups.FindByPredicate(
			[](const FFMCodexLocalMatchDeploymentGroup& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.LegalSlots.IsEmpty();
			});
	TestNotNull(TEXT("Card refresh fixture has ordinary candidate"),
		OrdinaryGroup);
	if (OrdinaryGroup == nullptr)
	{
		return false;
	}
	const FName CardId = OrdinaryGroup->CardId;
	const FName SlotId = OrdinaryGroup->LegalSlots[0].SlotId;
	const FFMCodexUMGCardRackCellViewModel* CandidateBeforeDeployment =
		Screen->GetPresentation().LocalRack.Cells.FindByPredicate(
			[CardId](const FFMCodexUMGCardRackCellViewModel& Candidate)
			{
				return Candidate.Card.CardId == CardId;
			});
	TestNotNull(TEXT("Selected candidate has shared UMG Card DTO"),
		CandidateBeforeDeployment);
	if (CandidateBeforeDeployment == nullptr)
	{
		return false;
	}
	const FFMCodexUMGCardViewModel CandidateSnapshot =
		CandidateBeforeDeployment->Card;
	Screen->RequestDeployOrdinary(CardId, SlotId);
	TestTrue(TEXT("Successful command refreshes card from authority"),
		Controller->GetLastDiagnostic().bHostSuccess);

	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Card stage retains dedicated Pitch Widget"), Pitch);
	if (Pitch == nullptr)
	{
		return false;
	}
	auto FindRenderedSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName ExpectedSlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == ExpectedSlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};
	UFMCodexPitchSlotWidget* OccupiedSlot = FindRenderedSlot(*Pitch, SlotId);
	TestNotNull(TEXT("Successful refresh keeps deployed card in exact slot"),
		OccupiedSlot);
	if (OccupiedSlot == nullptr)
	{
		return false;
	}
	OccupiedSlot->TakeWidget();
	UFMCodexPlayerCardWidget* PitchCard = OccupiedSlot->GetCardWidget();
	TestTrue(TEXT("PitchSlot reuses Card Widget in PitchMini mode"),
		Pitch->GetRenderedSlotWidgets().Num() == 10
			&& PitchCard != nullptr
			&& PitchCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchMini);
	if (PitchCard == nullptr)
	{
		return false;
	}
	const FFMCodexUMGCardViewModel DeployedSnapshot =
		PitchCard->GetPresentation();
	TestTrue(TEXT("Pitch and candidate retain one presentation identity"),
		DeployedSnapshot.CardId == CandidateSnapshot.CardId
			&& DeployedSnapshot.IdentityLabel == CandidateSnapshot.IdentityLabel
			&& DeployedSnapshot.OwnerLabel == CandidateSnapshot.OwnerLabel
			&& DeployedSnapshot.RoleLabel == CandidateSnapshot.RoleLabel
			&& DeployedSnapshot.RarityLabel == CandidateSnapshot.RarityLabel
			&& DeployedSnapshot.SkillLabels == CandidateSnapshot.SkillLabels
			&& DeployedSnapshot.CompactAttributeSummary
				== CandidateSnapshot.CompactAttributeSummary
			&& DeployedSnapshot.FullAttributeSummary
				== CandidateSnapshot.FullAttributeSummary
			&& DeployedSnapshot.StatusLabels.Contains(TEXT("DEPLOYED")));

	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(CardId, SlotId);
	TestFalse(TEXT("Repeated deployment is rejected"),
		Controller->GetLastDiagnostic().bHostSuccess);
	UFMCodexPitchSlotWidget* RejectedSlot = FindRenderedSlot(*Pitch, SlotId);
	if (RejectedSlot != nullptr)
	{
		RejectedSlot->TakeWidget();
	}
	const UFMCodexPlayerCardWidget* RejectedCard = RejectedSlot != nullptr
		? RejectedSlot->GetCardWidget() : nullptr;
	TestTrue(TEXT("Rejected command has no optimistic card/status mutation"),
		StateBeforeRejected == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& RejectedSlot != nullptr
			&& RejectedCard != nullptr
			&& RejectedSlot->GetPresentation().SlotId == SlotId
			&& RejectedCard->GetPresentation().CardId
				== DeployedSnapshot.CardId
			&& RejectedCard->GetPresentation().IdentityLabel
				== DeployedSnapshot.IdentityLabel
			&& RejectedCard->GetPresentation().RoleLabel
				== DeployedSnapshot.RoleLabel
			&& RejectedCard->GetPresentation().SkillLabels
				== DeployedSnapshot.SkillLabels
			&& RejectedCard->GetPresentation().CompactAttributeSummary
				== DeployedSnapshot.CompactAttributeSummary
			&& RejectedCard->GetPresentation().FullAttributeSummary
				== DeployedSnapshot.FullAttributeSummary
			&& RejectedCard->GetPresentation().StatusLabels
				== DeployedSnapshot.StatusLabels);

	FString CardHeader;
	FString CardSource;
	FString DTOHeader;
	FString RootSource;
	FString InteractionPanelSource;
	FString SlotSource;
	FString ControllerSource;
	TestTrue(TEXT("Card visual boundary production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.h"),
			CardHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchUMGPresentation.h"),
				DTOHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionPanelSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				SlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString CardWidgetSources = CardHeader + CardSource;
	TestTrue(TEXT("Card Widget consumes reflected presentation DTO only"),
		CardHeader.Contains(TEXT("FFMCodexUMGCardViewModel"))
			&& DTOHeader.Contains(TEXT("BlueprintType"))
			&& !CardWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !CardWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !CardWidgetSources.Contains(TEXT("D6Provider"))
			&& !CardWidgetSources.Contains(TEXT("HostGameMode")));
	TestTrue(TEXT("Card Widget contains zero gameplay calculation or input"),
		!CardWidgetSources.Contains(TEXT("Legality"))
			&& !CardWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !CardWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !CardWidgetSources.Contains(TEXT("RandRange"))
			&& !CardWidgetSources.Contains(TEXT("UButton"))
			&& !CardWidgetSources.Contains(TEXT("OnClicked"))
			&& !CardWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Hand Micro identity text is explicitly left aligned"),
		CardSource.Contains(
			TEXT("HandMicroIdentityText->SetJustification(ETextJustify::Left)"))
			&& CardSource.Contains(
				TEXT("HandMicroRoleText->SetJustification(ETextJustify::Left)")));
	TestTrue(TEXT("Card modes affect presentation density only"),
		CardHeader.Contains(TEXT("HandMicro"))
			&& CardHeader.Contains(TEXT("PitchMini"))
			&& CardHeader.Contains(TEXT("InteractionChoice"))
			&& CardSource.Contains(TEXT("SetWidthOverride"))
			&& CardSource.Contains(TEXT("FullAttributeSummary"))
			&& CardSource.Contains(TEXT("CompactAttributeSummary")));
	TestTrue(TEXT("Pitch and interaction select one shared configurable card"),
		SlotSource.Contains(TEXT("PlayerCardWidgetClass"))
			&& SlotSource.Contains(TEXT("PitchMini"))
			&& RootSource.Contains(TEXT("InteractionPanelWidgetClass"))
			&& InteractionPanelSource.Contains(TEXT("PlayerCardWidgetClass"))
			&& InteractionPanelSource.Contains(TEXT("InteractionChoice")));
	TestTrue(TEXT("Controller remains unaware of card visual geometry"),
		!ControllerSource.Contains(TEXT("PlayerCardBounds"))
			&& !ControllerSource.Contains(TEXT("PortraitPresentationRegion"))
			&& !ControllerSource.Contains(TEXT("StructuredAttributeGrid")));
	TestTrue(TEXT("Root and Pitch semantics remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) == nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerBPhysicalHalf")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PhysicalHalfVisualSeparator")) != nullptr
			&& Pitch->GetWidgetFromName(TEXT("PlayerAPhysicalHalf")) != nullptr);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGInteractionPanelVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.31.UMGInteractionPanelVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGInteractionPanelVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Interaction foundation Host exists"), Host);
	TestNotNull(TEXT("Interaction foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Interaction foundation root screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	TestNotNull(TEXT("Root composes dedicated Interaction Panel"), Panel);
	if (Panel == nullptr)
	{
		return false;
	}
	Panel->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("InteractionPanelBounds"), TEXT("InteractionActionHeader"),
		TEXT("InteractionCandidateScroll"), TEXT("InteractionChoiceRegion"),
		TEXT("InteractionSecondaryActions"), TEXT("InteractionPrimaryActions"),
		TEXT("InteractionBoundedFallback") })
	{
		TestNotNull(FString::Printf(TEXT("Panel hierarchy contains %s"), Region),
			Panel->GetWidgetFromName(Region));
	}

	FFMCodexUMGCardViewModel Card;
	Card.CardId = TEXT("Panel.Card.Cross");
	Card.IdentityLabel = TEXT("Panel Candidate");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW");
	Card.SkillLabels = { TEXT("Cross") };
	Card.SkillSummaryLabel = TEXT("Cross");
	Card.CompactAttributeSummary = TEXT("SHO 4 | PAS 5 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 4 | DRI 3 | PAS 5 | OFF 4 | MRK 1 | TKL 1 | SPD 4 | STR 2 | STA 3 | LS 2");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");
	Card.RarityLabel = TEXT("Club");

	auto BasePresentation = [](
		const EFMCodexUMGInteractionCategory Category,
		const TCHAR* Title)
	{
		FFMCodexUMGInteractionViewModel Result;
		Result.Category = Category;
		Result.KickerLabel = TEXT("PLAYER ACTION");
		Result.ExpectedActorLabel = TEXT("Player A");
		Result.TitleLabel = Title;
		Result.CategoryLabel = Title;
		return Result;
	};
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};

	FFMCodexUMGInteractionViewModel Start = BasePresentation(
		EFMCodexUMGInteractionCategory::StartMatch, TEXT("START LOCAL MATCH"));
	Start.bCanStartNewMatch = true;
	Start.PrimaryActionLabel = TEXT("START LOCAL MATCH");
	Panel->RefreshFromPresentation(Start);
	TestTrue(TEXT("StartMatch exposes one readable primary action"),
		IsVisible(Panel->GetWidgetFromName(TEXT("InteractionStartMatchButton")))
			&& Panel->GetPresentation().TitleLabel == TEXT("START LOCAL MATCH"));

	FFMCodexUMGInteractionViewModel Begin = BasePresentation(
		EFMCodexUMGInteractionCategory::TacticalPointRoll, TEXT("ROLL TACTICAL POINTS"));
	Begin.ExpectedActorLabel = TEXT("PLAYER A TO ACT");
	Begin.bCanRollTacticalPoints = true;
	Begin.PrimaryActionLabel = TEXT("ROLL TACTICAL POINTS");
	Begin.ActionPointLabel = TEXT("Action Point: 6");
	Panel->RefreshFromPresentation(Begin);
	const UTextBlock* BeginContext = Cast<UTextBlock>(
		Panel->GetWidgetFromName(TEXT("InteractionActionContext")));
	const UWidget* BeginKicker = Panel->GetWidgetFromName(
		TEXT("InteractionActionKicker"));
	const UTextBlock* BeginActor = Cast<UTextBlock>(
		Panel->GetWidgetFromName(TEXT("InteractionExpectedActor")));
	const UWidget* BeginTitle = Panel->GetWidgetFromName(
		TEXT("InteractionActionTitle"));
	const UButton* BeginButton = Cast<UButton>(
		Panel->GetWidgetFromName(TEXT("InteractionTacticalPointRollButton")));
	const USizeBox* BeginButtonBounds = Cast<USizeBox>(
		Panel->GetWidgetFromName(TEXT("TacticalPointPrimaryActionBounds")));
	const UTextBlock* BeginButtonLabel = BeginButton != nullptr
		? Cast<UTextBlock>(BeginButton->GetChildAt(0)) : nullptr;
	TestTrue(TEXT("TacticalPointRoll Dock ignores persistent AP while preserving action DTO"),
		IsVisible(Panel->GetWidgetFromName(TEXT("InteractionTacticalPointRollButton")))
			&& Panel->GetPresentation().ActionPointLabel.Contains(TEXT("6"))
			&& BeginContext != nullptr
			&& !BeginContext->GetText().ToString().Contains(TEXT("6"))
			&& (BeginKicker == nullptr
				|| BeginKicker->GetVisibility() == ESlateVisibility::Collapsed));
	TestTrue(TEXT("TacticalPointRoll Dock has one compact primary wording hierarchy"),
		BeginActor != nullptr
			&& BeginActor->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					TEXT("PLAYER A TO ACT")))
			&& BeginTitle != nullptr
			&& BeginTitle->GetVisibility() == ESlateVisibility::Collapsed
			&& BeginContext->GetVisibility() == ESlateVisibility::Collapsed
			&& BeginButtonLabel != nullptr
			&& BeginButtonLabel->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					TEXT("ROLL TACTICAL POINTS")))
			&& BeginButtonLabel->GetFont().Size == 12);
	TestTrue(TEXT("TacticalPointRoll CTA uses repaired compact bounds"),
		BeginButtonBounds != nullptr
			&& FMath::IsNearlyEqual(BeginButtonBounds->GetWidthOverride(), 156.0f)
			&& FMath::IsNearlyEqual(BeginButtonBounds->GetHeightOverride(), 48.0f));

	FFMCodexUMGInteractionViewModel Deploy = BasePresentation(
		EFMCodexUMGInteractionCategory::Deploy, TEXT("DEPLOYMENT"));
	Deploy.bCanFinishDeployment = true;
	Deploy.PrimaryActionLabel = TEXT("FINISH DEPLOYMENT");
	FFMCodexUMGDeploymentChoiceViewModel Ordinary;
	Ordinary.CardId = Card.CardId;
	Ordinary.Card = Card;
	Ordinary.Destinations.Add({ TEXT("Slot.A"), TEXT("Slot A") });
	Ordinary.Destinations.Add({ TEXT("Slot.B"), TEXT("Slot B") });
	FFMCodexUMGDeploymentChoiceViewModel Goalkeeper = Ordinary;
	Goalkeeper.CardId = TEXT("Panel.Card.GK");
	Goalkeeper.Card.CardId = Goalkeeper.CardId;
	Goalkeeper.Card.IdentityLabel = TEXT("Panel Goalkeeper");
	Goalkeeper.Card.RoleLabel = TEXT("GK");
	Goalkeeper.Card.bGoalkeeper = true;
	Goalkeeper.bGoalkeeper = true;
	Goalkeeper.Destinations = { { TEXT("Slot.GK"), TEXT("Goalkeeper Slot") } };
	Deploy.DeploymentChoices = { Ordinary, Goalkeeper };
	Panel->RefreshFromPresentation(Deploy);
	TestTrue(TEXT("Deployment dock defers cards to the persistent rack"),
		Panel->GetRenderedCandidateCardWidgets().IsEmpty()
			&& Panel->GetRenderedOptionWidgets().IsEmpty()
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("DeploymentHandInstruction")))
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionFinishDeploymentButton"))));

	const TArray<EFMCodexUMGInteractionCategory> SelectionCategories = {
		EFMCodexUMGInteractionCategory::SelectCarrier,
		EFMCodexUMGInteractionCategory::SelectMarker,
		EFMCodexUMGInteractionCategory::SelectSkill,
		EFMCodexUMGInteractionCategory::SelectRunner,
		EFMCodexUMGInteractionCategory::SelectHelper
	};
	for (const EFMCodexUMGInteractionCategory Category : SelectionCategories)
	{
		FFMCodexUMGInteractionViewModel Selection = BasePresentation(
			Category, TEXT("SELECT LEGAL OPTION"));
		FFMCodexUMGSelectionChoiceViewModel Choice;
		Choice.OptionId = Category == EFMCodexUMGInteractionCategory::SelectSkill
			? FName(TEXT("Demo.Skill.Cross")) : Card.CardId;
		Choice.Label = Category == EFMCodexUMGInteractionCategory::SelectSkill
			? TEXT("Cross") : TEXT("Select Panel Candidate");
		Choice.bHasCard = Category != EFMCodexUMGInteractionCategory::SelectSkill;
		Choice.Card = Card;
		Selection.SelectionChoices = { Choice, Choice };
		Selection.SelectionChoices[1].OptionId = TEXT("Panel.SecondOption");
		Selection.bCanDecline = Category
			!= EFMCodexUMGInteractionCategory::SelectCarrier;
		Selection.bCanResolveNoLegal = true;
		Selection.DeclineActionLabel = TEXT("DECLINE");
		Selection.NoLegalActionLabel = TEXT("RESOLVE NO LEGAL");
		Panel->RefreshFromPresentation(Selection);
		TestTrue(FString::Printf(TEXT("Category %d preserves all supplied options"),
			static_cast<int32>(Category)),
			Panel->GetRenderedOptionWidgets().Num() == 2
				&& Panel->GetRenderedCandidateCardWidgets().IsEmpty()
				&& IsVisible(Panel->GetWidgetFromName(
					TEXT("InteractionNoLegalButton"))));
	}
	FFMCodexUMGInteractionViewModel OnPitchCarrier = BasePresentation(
		EFMCodexUMGInteractionCategory::SelectCarrier, TEXT("Select Carrier"));
	OnPitchCarrier.SelectionChoices = {
		{ Card.CardId, TEXT("Select Panel Candidate"), true, Card } };
	OnPitchCarrier.bUseOnPitchPlayerSelection = true;
	OnPitchCarrier.OnPitchSelectionHintLabel =
		TEXT("Click a player on the pitch");
	Panel->RefreshFromPresentation(OnPitchCarrier);
	const UTextBlock* OnPitchContext = Cast<UTextBlock>(
		Panel->GetWidgetFromName(TEXT("InteractionActionContext")));
	TestTrue(TEXT("On-pitch Carrier keeps instruction and removes bottom PlayerKey buttons"),
		Panel->GetRenderedOptionWidgets().IsEmpty()
			&& Panel->GetWidgetFromName(TEXT("InteractionCandidateRegion"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& Panel->GetWidgetFromName(TEXT("InteractionBoundedFallback"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& OnPitchContext != nullptr
			&& OnPitchContext->GetText().ToString().Contains(
				TEXT("\u573A\u4E0A\u7403\u5458")));
	FFMCodexUMGInteractionViewModel DistinctFallbacks = BasePresentation(
		EFMCodexUMGInteractionCategory::SelectMarker, TEXT("SELECT MARKER"));
	DistinctFallbacks.bCanDecline = true;
	DistinctFallbacks.bCanResolveNoLegal = true;
	DistinctFallbacks.DeclineActionLabel = TEXT("DECLINE MARKER");
	DistinctFallbacks.NoLegalActionLabel = TEXT("NO LEGAL MARKER - RESOLVE");
	Panel->RefreshFromPresentation(DistinctFallbacks);
	const UButton* DeclineButton = Cast<UButton>(
		Panel->GetWidgetFromName(TEXT("InteractionDeclineButton")));
	const UButton* NoLegalButton = Cast<UButton>(
		Panel->GetWidgetFromName(TEXT("InteractionNoLegalButton")));
	TestTrue(TEXT("Decline and NoLegal remain distinct secondary actions"),
		IsVisible(DeclineButton) && IsVisible(NoLegalButton)
			&& Cast<UTextBlock>(DeclineButton->GetChildAt(0))->GetText().ToString()
				!= Cast<UTextBlock>(NoLegalButton->GetChildAt(0))->GetText().ToString());

	auto VerifyBranch = [this, Panel, &BasePresentation](
		const FString& Section,
		const EFMCodexUMGBranchIntent First,
		const EFMCodexUMGBranchIntent Second)
	{
		FFMCodexUMGInteractionViewModel Branch = BasePresentation(
			EFMCodexUMGInteractionCategory::SelectBranchIntent,
			TEXT("CHOOSE ROUTE"));
		Branch.BranchSectionLabel = Section;
		Branch.BranchChoices = {
			{ First, First == EFMCodexUMGBranchIntent::CrossHigh
				? TEXT("HIGH") : TEXT("DIRECT SHOT") },
			{ Second, Second == EFMCodexUMGBranchIntent::CrossLow
				? TEXT("LOW") : TEXT("DEAD CORNER") }
		};
		Panel->RefreshFromPresentation(Branch);
		TestTrue(FString::Printf(TEXT("%s renders two distinct branch intents"),
			*Section), Panel->GetRenderedOptionWidgets().Num() == 2
			&& Panel->GetPresentation().BranchSectionLabel == Section);
	};
	VerifyBranch(TEXT("SHOT TYPE"), EFMCodexUMGBranchIntent::DirectShot,
		EFMCodexUMGBranchIntent::DeadCorner);
	VerifyBranch(TEXT("CROSS TYPE"), EFMCodexUMGBranchIntent::CrossHigh,
		EFMCodexUMGBranchIntent::CrossLow);

	FFMCodexUMGInteractionViewModel OneOnOne = BasePresentation(
		EFMCodexUMGInteractionCategory::SelectOneOnOneShot,
		TEXT("ONE-ON-ONE"));
	OneOnOne.BranchSectionLabel = TEXT("CHOOSE SHOT");
	OneOnOne.OneOnOneChoices = {
		{ EFMCodexUMGOneOnOneChoice::ChipShot, TEXT("CHIP SHOT") },
		{ EFMCodexUMGOneOnOneChoice::DirectShot, TEXT("DIRECT SHOT") }
	};
	Panel->RefreshFromPresentation(OneOnOne);
	TestEqual(TEXT("OneOnOne renders ChipShot and DirectShot only"),
		Panel->GetRenderedOptionWidgets().Num(), 2);

	FFMCodexUMGInteractionViewModel Continue = BasePresentation(
		EFMCodexUMGInteractionCategory::ContinueResolution,
		TEXT("Resolve Route"));
	Continue.KickerLabel = TEXT("SYSTEM RESOLUTION");
	Continue.bSystemResolution = true;
	Continue.bCanContinue = true;
	Continue.PrimaryActionLabel = TEXT("CONTINUE");
	Panel->RefreshFromPresentation(Continue);
	TestTrue(TEXT("System resolution presents advancing rather than choosing"),
		Panel->GetPresentation().bSystemResolution
			&& IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionContinueButton"))));

	FFMCodexUMGInteractionViewModel Ended = BasePresentation(
		EFMCodexUMGInteractionCategory::MatchEnded, TEXT("MATCH COMPLETE"));
	Ended.bMatchEnded = true;
	Ended.EmptyStateLabel = TEXT("Match complete. No action required.");
	Panel->RefreshFromPresentation(Ended);
	TestTrue(TEXT("MatchEnded contains no gameplay progression control"),
		Panel->GetRenderedOptionWidgets().IsEmpty()
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionStartMatchButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionTacticalPointRollButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionFinishDeploymentButton")))
			&& !IsVisible(Panel->GetWidgetFromName(
				TEXT("InteractionContinueButton"))));

	// Real player-facing flow: Panel -> root -> Controller -> authoritative Host.
	Controller->RefreshPresentation();
	Panel = Screen->GetInteractionPanel();
	TestEqual(TEXT("Real panel starts in StartMatch"),
		Panel->GetPresentation().Category,
		EFMCodexUMGInteractionCategory::StartMatch);
	Panel->RequestStartMatch();
	TestTrue(TEXT("Panel StartMatch reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& !Panel->IsInteractionBlocked()
			&& Panel->GetPresentation().bCanRollTacticalPoints);
	Panel->RequestTacticalPointRoll();
	TestTrue(TEXT("Panel TacticalPointRoll reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const EInitialTurnOrderPlayer FlowAttacker =
		Host->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const FName CrossCarrierId =
		FMCodexLocalMatchFullFamilyTests::FamilyCardId(
			FMCodexLocalMatchFullFamilyTests::FamilyExpectations()[0],
			FlowAttacker);
	const FString FlowPhysicalForward =
		FlowAttacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");

	int32 SuccessfulDeployments = 0;
	bool bRejectedIntentVerified = false;
	bool bCrossCarrierDeployed = false;
	bool bCrossRunnerDeployed = false;
	const FName CrossRunnerId = FlowAttacker
		== EInitialTurnOrderPlayer::PlayerA
			? FName(TEXT("Prototype.Arsenal.ViktorGyokeres"))
			: FName(TEXT("Prototype.ManchesterCity.ErlingHaaland"));
	while (SuccessfulDeployments < 5)
	{
		const FFMCodexUMGInteractionViewModel& Deployment =
			Panel->GetPresentation();
		if (Deployment.Category != EFMCodexUMGInteractionCategory::Deploy
			|| Deployment.DeploymentChoices.IsEmpty())
		{
			AddError(TEXT("Real Panel flow lost deployment choices"));
			return false;
		}
		const EInitialTurnOrderPlayer CurrentDeploymentSide =
			Controller->GetInteractionView().CurrentLegalDeploymentSide;
		const bool bNeedDefenderGoalkeeper = false;
		const FFMCodexUMGDeploymentChoiceViewModel* Choice = nullptr;
		if (bNeedDefenderGoalkeeper)
		{
			Choice = Deployment.DeploymentChoices.FindByPredicate(
				[](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
				{
					return Candidate.bGoalkeeper
						&& !Candidate.Destinations.IsEmpty();
				});
		}
		else
		{
			const int32 CurrentActionPoint =
				Controller->GetInteractionView().ActionPoint;
			const bool bPreferCross = Deployment.DeploymentChoices.ContainsByPredicate(
				[CurrentActionPoint, &FlowPhysicalForward](
					const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
				{
					return !Candidate.bGoalkeeper
						&& Candidate.Card.Skills.ContainsByPredicate(
							[CurrentActionPoint](const FFMCodexUMGSkillViewModel& Skill)
							{
								return Skill.CanonicalLabel == TEXT("Cross")
									&& CurrentActionPoint >= Skill.MinTriggerActionPoint
									&& CurrentActionPoint <= Skill.MaxTriggerActionPoint;
							})
						&& Candidate.Destinations.ContainsByPredicate(
							[&FlowPhysicalForward](
								const FFMCodexUMGDeploymentDestinationViewModel& Destination)
							{
								return Destination.SlotId.ToString().Contains(
									FlowPhysicalForward);
							});
				});
			const bool bRequireCrossRunner =
				CurrentDeploymentSide == FlowAttacker
				&& bCrossCarrierDeployed && !bCrossRunnerDeployed;
			Choice = Deployment.DeploymentChoices.FindByPredicate(
				[CurrentDeploymentSide, FlowAttacker, CrossCarrierId,
					CrossRunnerId, bCrossCarrierDeployed, bCrossRunnerDeployed,
					bRequireCrossRunner, bPreferCross, CurrentActionPoint,
					&FlowPhysicalForward](
					const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
				{
					if (Candidate.bGoalkeeper
						|| (CurrentDeploymentSide == FlowAttacker
							&& !bCrossCarrierDeployed
							&& Candidate.CardId != CrossCarrierId))
					{
						return false;
					}
					if (bRequireCrossRunner
						&& Candidate.CardId != CrossRunnerId)
					{
						return false;
					}
					if ((CurrentDeploymentSide != FlowAttacker
							|| (bCrossCarrierDeployed
								&& bCrossRunnerDeployed))
						&& bPreferCross
						&& !Candidate.Card.Skills.ContainsByPredicate(
							[CurrentActionPoint](const FFMCodexUMGSkillViewModel& Skill)
							{
								return Skill.CanonicalLabel == TEXT("Cross")
									&& CurrentActionPoint >= Skill.MinTriggerActionPoint
									&& CurrentActionPoint <= Skill.MaxTriggerActionPoint;
							}))
					{
						return false;
					}
					return Candidate.Destinations.ContainsByPredicate(
						[&FlowPhysicalForward](
							const FFMCodexUMGDeploymentDestinationViewModel& Destination)
						{
							return Destination.SlotId.ToString().Contains(
								FlowPhysicalForward);
						});
				});
		}
		if (Choice == nullptr)
		{
			AddError(TEXT("Real Panel flow found no required deployment"));
			return false;
		}
		const FName DeployedCardId = Choice->CardId;
		const bool bDeployedGoalkeeper = Choice->bGoalkeeper;
		const FFMCodexUMGDeploymentDestinationViewModel* Destination =
			bDeployedGoalkeeper ? &Choice->Destinations[0]
			: Choice->Destinations.FindByPredicate(
				[&FlowPhysicalForward](
					const FFMCodexUMGDeploymentDestinationViewModel& Candidate)
				{
					return Candidate.SlotId.ToString().Contains(
						FlowPhysicalForward);
				});
		if (Destination == nullptr)
		{
			return false;
		}
		const FName DeployedSlotId = Destination->SlotId;
		Panel->RequestDeployment(
			DeployedCardId, DeployedSlotId, bDeployedGoalkeeper);
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			AddError(TEXT("Real Panel ordinary deployment was rejected"));
			return false;
		}
		bCrossCarrierDeployed = bCrossCarrierDeployed
			|| DeployedCardId == CrossCarrierId;
		bCrossRunnerDeployed = bCrossRunnerDeployed
			|| DeployedCardId == CrossRunnerId;
		++SuccessfulDeployments;
		if (!bRejectedIntentVerified)
		{
			const TArray<uint8> BeforeRejected =
				SerializeState(Host->GetMatchSnapshot().Snapshot);
			const EFMCodexUMGInteractionCategory BeforeCategory =
				Panel->GetPresentation().Category;
			const int32 BeforeChoiceCount =
				Panel->GetPresentation().DeploymentChoices.Num();
			Panel->RequestDeployment(
				DeployedCardId, DeployedSlotId, bDeployedGoalkeeper);
			bRejectedIntentVerified =
				!Controller->GetLastDiagnostic().bHostSuccess
				&& BeforeRejected
					== SerializeState(Host->GetMatchSnapshot().Snapshot)
				&& Panel->GetPresentation().Category == BeforeCategory
				&& Panel->GetPresentation().DeploymentChoices.Num()
					== BeforeChoiceCount;
		}
	}
	TestTrue(TEXT("Rejected Panel intent has no optimistic selection mutation"),
		bRejectedIntentVerified);
	for (int32 Side = 0; Side < 2; ++Side)
	{
		Panel->RequestFinishDeployment();
		TestTrue(TEXT("Panel FinishDeployment reaches authoritative Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
	}
	TestTrue(TEXT("Real Panel deployment includes the canonical Cross carrier"),
		bCrossCarrierDeployed);
	const FFMCodexUMGSelectionChoiceViewModel* Carrier =
		Panel->GetPresentation().SelectionChoices.FindByPredicate(
			[CrossCarrierId](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.OptionId == CrossCarrierId;
			});
	TestNotNull(TEXT("Real Panel exposes deployed Cross carrier"), Carrier);
	if (Carrier == nullptr)
	{
		return false;
	}
	Panel->RequestCarrier(Carrier->OptionId);
	TestTrue(TEXT("Panel Carrier reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	if (!Panel->GetPresentation().SelectionChoices.IsEmpty())
	{
		Panel->RequestMarker(
			Panel->GetPresentation().SelectionChoices[0].OptionId);
	}
	else if (Panel->GetPresentation().bCanResolveNoLegal)
	{
		Panel->RequestNoLegal();
	}
	else
	{
		AddError(TEXT("Real Panel exposes neither Marker nor NoLegal Marker"));
		return false;
	}
	TestTrue(TEXT("Panel Marker/NoLegal Marker reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	const FFMCodexUMGSelectionChoiceViewModel* CrossSkill =
		Panel->GetPresentation().SelectionChoices.FindByPredicate(
			[](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.Label.Contains(TEXT("Cross"));
			});
	TestNotNull(TEXT("Real Panel exposes only authoritative legal Skills"),
		CrossSkill);
	if (CrossSkill == nullptr)
	{
		return false;
	}
	Panel->RequestSkill(CrossSkill->OptionId);
	TestTrue(TEXT("Panel Skill reaches authoritative Host"),
		Controller->GetLastDiagnostic().bHostSuccess);
	for (int32 Guard = 0; Guard < 4
		&& (Panel->GetPresentation().Category
			== EFMCodexUMGInteractionCategory::SelectRunner
			|| Panel->GetPresentation().Category
				== EFMCodexUMGInteractionCategory::SelectHelper); ++Guard)
	{
		const EFMCodexUMGInteractionCategory Category =
			Panel->GetPresentation().Category;
		if (!Panel->GetPresentation().SelectionChoices.IsEmpty())
		{
			const FName Id =
				Panel->GetPresentation().SelectionChoices[0].OptionId;
			if (Category == EFMCodexUMGInteractionCategory::SelectRunner)
			{
				Panel->RequestRunner(Id);
			}
			else
			{
				Panel->RequestHelper(Id);
			}
		}
		else if (Panel->GetPresentation().bCanResolveNoLegal)
		{
			Panel->RequestNoLegal();
		}
		else
		{
			Panel->RequestDecline();
		}
		TestTrue(TEXT("Panel Runner/Helper route reaches Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
	}
	TestTrue(TEXT("Representative Panel flow reaches Cross BranchIntent"),
		Panel->GetPresentation().Category
			== EFMCodexUMGInteractionCategory::SelectBranchIntent
			&& Panel->GetPresentation().BranchChoices.Num() == 2);
	if (Panel->GetPresentation().Category
		== EFMCodexUMGInteractionCategory::SelectBranchIntent)
	{
		Panel->RequestBranch(EFMCodexUMGBranchIntent::CrossHigh);
		TestTrue(TEXT("Panel Cross BranchIntent reaches authoritative Host"),
			Controller->GetLastDiagnostic().bHostSuccess);
	}

	// Deterministic normal-demo OneOnOne proof, with the actual choice routed
	// through the dedicated Widget and existing typed Controller method.
	FScopedPlayableWorld OneOnOneWorld;
	AFMCodexLocalMatchHostGameMode* OneOnOneHost = OneOnOneWorld.GetHost();
	AFMCodexLocalMatchPlayerController* OneOnOneController =
		OneOnOneWorld.GetController();
	TestNotNull(TEXT("OneOnOne UMG Host exists"), OneOnOneHost);
	TestNotNull(TEXT("OneOnOne UMG Controller exists"), OneOnOneController);
	if (OneOnOneHost == nullptr || OneOnOneController == nullptr)
	{
		return false;
	}
	FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FName ThroughBallSkillId(TEXT("Demo.Skill.ThroughBall"));
	for (TArray<FPlayerCardData>* Deck : {
		&Demo.OpeningInput.OpeningInput.PlayerADeck,
		&Demo.OpeningInput.OpeningInput.PlayerBDeck })
	{
		for (FPlayerCardData& DemoCard : *Deck)
		{
			if (!DemoCard.bIsGoalkeeper)
			{
				DemoCard.AttackSkillIds = { ThroughBallSkillId };
			}
		}
	}
	FSkillRuleSnapshot ThroughBallRule;
	ThroughBallRule.SkillId = ThroughBallSkillId;
	ThroughBallRule.SkillType = ESkillRuleType::ThroughBall;
	ThroughBallRule.MinTriggerActionPoint = 2;
	ThroughBallRule.MaxTriggerActionPoint = 8;
	Demo.SkillRuleSet.SkillRules = { ThroughBallRule };
	const int32 OneOnOneSeed = FindSeedForRolls({ 3, 6, 1, 2 });
	if (OneOnOneSeed == INDEX_NONE
		|| !OneOnOneHost->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, OneOnOneSeed).bSuccess
		|| !OneOnOneHost->BeginOrdinaryAttack(6).bSuccess)
	{
		AddError(TEXT("Could not initialize deterministic OneOnOne UMG fixture"));
		return false;
	}
	const EInitialTurnOrderPlayer OneOnOneAttacker = OneOnOneHost
		->GetMatchSnapshot().Snapshot.RuntimeState.CurrentAttackingPlayer;
	const FString PhysicalForward =
		OneOnOneAttacker == EInitialTurnOrderPlayer::PlayerA
			? TEXT("NearB") : TEXT("NearA");
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (!DeployNextOrdinary(
			*OneOnOneHost, Demo.SkillRuleSet, PhysicalForward))
		{
			AddError(TEXT("Could not deploy deterministic OneOnOne fixture"));
			return false;
		}
	}
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const FFMCodexLocalMatchInteractionView View =
			ViewFor(*OneOnOneHost, Demo.SkillRuleSet);
		if (!OneOnOneHost->FinishDeployment(
			View.AttackSequence, View.ExpectedActingPlayer).bSuccess)
		{
			return false;
		}
	}
	for (const EFMCodexLocalMatchInteractionCategory Category : {
		EFMCodexLocalMatchInteractionCategory::SelectCarrier,
		EFMCodexLocalMatchInteractionCategory::SelectMarker,
		EFMCodexLocalMatchInteractionCategory::SelectSkill,
		EFMCodexLocalMatchInteractionCategory::SelectRunner,
		EFMCodexLocalMatchInteractionCategory::SelectHelper })
	{
		if (!SubmitFirstHostSelection(
			*OneOnOneHost, Demo.SkillRuleSet, Category))
		{
			AddError(TEXT("Could not select deterministic OneOnOne fixture"));
			return false;
		}
	}
	if (!OneOnOneHost->BeginResolutionSession().bSuccess
		|| !OneOnOneHost->ResolveInitialRoute().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP1DecisionOrPlan().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP1Formula().bSuccess
		|| !OneOnOneHost->ResolveThroughBallBehindDefenseP2Decision().bSuccess)
	{
		AddError(TEXT("Could not reach deterministic OneOnOne decision"));
		return false;
	}
	OneOnOneController->RefreshPresentation();
	OneOnOneController->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* OneOnOneScreen =
		OneOnOneController->GetPlayerMatchScreen();
	if (OneOnOneScreen == nullptr)
	{
		return false;
	}
	OneOnOneScreen->TakeWidget();
	UFMCodexInteractionPanelWidget* OneOnOnePanel =
		OneOnOneScreen->GetInteractionPanel();
	TestTrue(TEXT("Authoritative OneOnOne renders both player choices"),
		OneOnOnePanel != nullptr
			&& OneOnOnePanel->GetPresentation().Category
				== EFMCodexUMGInteractionCategory::SelectOneOnOneShot
			&& OneOnOnePanel->GetPresentation().OneOnOneChoices.Num() == 2
			&& OneOnOnePanel->GetRenderedOptionWidgets().Num() == 2);
	if (OneOnOnePanel == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("OneOnOne choice is immediately unblocked"),
		OneOnOnePanel->IsInteractionBlocked());
	OneOnOnePanel->RequestOneOnOne(EFMCodexUMGOneOnOneChoice::DirectShot);
	TestTrue(TEXT("OneOnOne Widget choice is accepted by Host"),
		OneOnOneController->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Accepted OneOnOne refresh changes interaction category"),
		OneOnOnePanel->GetPresentation().Category
			!= EFMCodexUMGInteractionCategory::SelectOneOnOneShot);
	TestTrue(TEXT("Accepted OneOnOne refresh removes choice controls"),
		OneOnOnePanel->GetPresentation().OneOnOneChoices.IsEmpty());

	FString PanelHeader;
	FString PanelSource;
	FString OptionHeader;
	FString OptionSource;
	FString RootHeader;
	FString RootSource;
	FString ControllerSource;
	TestTrue(TEXT("Interaction authority audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.h"),
			PanelHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				PanelSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.h"),
				OptionHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.cpp"),
				OptionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString InteractionWidgetSources =
		PanelHeader + PanelSource + OptionHeader + OptionSource;
	TestTrue(TEXT("Interaction widgets consume reflected DTOs only"),
		PanelHeader.Contains(TEXT("FFMCodexUMGInteractionViewModel"))
			&& !InteractionWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !InteractionWidgetSources.Contains(TEXT("HostGameMode"))
			&& !InteractionWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !InteractionWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Interaction widgets contain no gameplay authority"),
		!InteractionWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !InteractionWidgetSources.Contains(TEXT("AvailabilityQuery"))
			&& !InteractionWidgetSources.Contains(TEXT("RandRange"))
			&& !InteractionWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !InteractionWidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !InteractionWidgetSources.Contains(TEXT("ProcessEvent")));
	TestTrue(TEXT("Root delegates to panel and Controller owns typed commands"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexInteractionPanelWidget>"))
			&& RootSource.Contains(TEXT("DedicatedInteractionPanelWidget"))
			&& RootSource.Contains(TEXT("RequestSubmitCarrier"))
			&& RootSource.Contains(TEXT("RequestContinueResolution"))
			&& ControllerSource.Contains(TEXT("SubmitCarrier"))
			&& ControllerSource.Contains(TEXT("ContinueResolution")));
	TestTrue(TEXT("Stage 6.4 root regions remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) == nullptr);
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGResolutionVisualFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.32.UMGResolutionVisualFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGResolutionVisualFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Resolution foundation Host exists"), Host);
	TestNotNull(TEXT("Resolution foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Resolution foundation root UMG screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexResolutionPanelWidget* RootResolution =
		Screen->GetResolutionPanel();
	TestNotNull(TEXT("Root composes dedicated Resolution Panel"), RootResolution);
	if (RootResolution == nullptr)
	{
		return false;
	}
	RootResolution->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("ResolutionPanelBounds"), TEXT("ResolutionStepRegion"),
		TEXT("ResolutionDiceRegion"), TEXT("ResolutionComparisonRegion"),
		TEXT("ResolutionDecisionRegion"),
		TEXT("ResolutionContinuationRegion"),
		TEXT("ResolutionContinueActionBounds"),
		TEXT("ResolutionTerminalRegion"), TEXT("ResolutionRejectedRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Resolution hierarchy contains %s"),
			Region), RootResolution->GetWidgetFromName(Region));
	}
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};

	FFMCodexUMGResolutionViewModel Empty;
	RootResolution->RefreshFromPresentation(Empty);
	TestTrue(TEXT("No-feedback state is bounded and non-diagnostic"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionEmptyState")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy"))));

	FFMCodexUMGResolutionViewModel Rich;
	Rich.bVisible = true;
	Rich.bCanContinue = true;
	Rich.ContinueActionLabel = TEXT("Continue - Apply Formula / Result");
	Rich.StepLabel = TEXT("One-on-One - Direct Shot");
	Rich.StepSummaryLabel = TEXT("Direct Shot Formula resolved");
	Rich.RouteLabel = TEXT("Through Ball -> Behind Defense");
	Rich.DiceResults = {
		{ TEXT("ROUTE"), TEXT("Initial Route"), 6 },
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Direct Shot Attack"), 4 },
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Direct Shot Defense"), 2 }
	};
	Rich.ComparisonEvidence = {
		{ TEXT("ATTACK"),
			TEXT("Shooter: Shooting 5 | modifier +1.0 | D6 4 | final 10.0") },
		{ TEXT("DEFENSE"),
			TEXT("Goalkeeper: OneOnOne 5 | activation active | authoritative modifier +2.5 | D6 2 | final 9.5") },
		{ TEXT("EVIDENCE"),
			TEXT("Goalkeeper participated in the authoritative Formula") }
	};
	Rich.DecisionLabel =
		TEXT("Winner: Attacker | Reason: Higher final value");
	Rich.ContinuationLabel = TEXT("Continue: Apply terminal result");
	Rich.TerminalLabel = TEXT("RESULT: GOAL");
	Rich.bTerminal = true;
	RootResolution->RefreshFromPresentation(Rich);
	TestTrue(TEXT("Resolution overlay exposes DTO-routed Continue control"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionContinueButton")))
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionContinueButtonLabel")))->GetText().EqualTo(
					FFMCodexPlayerUIPresentationText::MatchScreenLabel(
						Rich.ContinueActionLabel)));
	TestEqual(TEXT("Every authoritative die receives one Dice Widget"),
		RootResolution->GetRenderedDiceWidgets().Num(), 3);
	bool bExactDiceBinding = true;
	for (int32 Index = 0;
		Index < RootResolution->GetRenderedDiceWidgets().Num(); ++Index)
	{
		const UFMCodexDiceResultWidget* Die =
			RootResolution->GetRenderedDiceWidgets()[Index];
		bExactDiceBinding = bExactDiceBinding && Die != nullptr
			&& Die->GetDisplayedRawD6() == Rich.DiceResults[Index].RawD6
			&& Die->GetPresentation().PurposeLabel
				== Rich.DiceResults[Index].PurposeLabel;
	}
	TestTrue(TEXT("Dice Widgets bind exact accepted RawD6 and Purpose"),
		bExactDiceBinding);
	TestTrue(TEXT("Formula evidence renders ATTACK VS DEFENSE without math"),
		RootResolution->GetRenderedComparisonCount() == 3
			&& RootResolution->GetWidgetFromName(
				TEXT("ResolutionVersusLabel")) != nullptr
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ComparisonEvidenceValue0")))->GetText().ToString()
					== Rich.ComparisonEvidence[0].EvidenceLabel
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ComparisonEvidenceValue1")))->GetText().ToString()
					== Rich.ComparisonEvidence[1].EvidenceLabel);
	TestTrue(TEXT("Winner/reason and GK evidence remain authoritative text"),
		Cast<UTextBlock>(RootResolution->GetWidgetFromName(
			TEXT("ResolutionDecisionSectionValue")))->GetText().ToString()
				== Rich.DecisionLabel
			&& Rich.ComparisonEvidence[1].EvidenceLabel.Contains(
				TEXT("Goalkeeper"))
			&& Rich.ComparisonEvidence[1].EvidenceLabel.Contains(TEXT("final 9.5")));
	TestTrue(TEXT("Terminal result is a distinct prominent section"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionTerminalRegion")))
			&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalSectionValue")))->GetText().ToString()
					== TEXT("RESULT: GOAL"));

	FFMCodexUMGResolutionViewModel Chip;
	Chip.bVisible = true;
	Chip.StepLabel = TEXT("One-on-One - Chip Shot");
	Chip.StepSummaryLabel = TEXT("Chip Shot decision resolved without Formula");
	Chip.DiceResults = {
		{ TEXT("ONE-ON-ONE"), TEXT("One-on-One Chip Shot Attack"), 5 }
	};
	Chip.DecisionLabel = TEXT("Goal");
	RootResolution->RefreshFromPresentation(Chip);
	TestTrue(TEXT("ChipShot is a distinct non-Formula presentation"),
		RootResolution->GetRenderedDiceWidgets().Num() == 1
			&& RootResolution->GetRenderedComparisonCount() == 0
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionComparisonRegion")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion"))));

	const TArray<TPair<FString, FString>> ReadableShapes = {
		{ TEXT("Cross - High Cross"), TEXT("Cross -> High Cross") },
		{ TEXT("Pass Control"), TEXT("Pass Control -> Pass Advance") },
		{ TEXT("Long Shot - Direct Shot"), TEXT("Long Shot -> Direct Shot") },
		{ TEXT("Cut Inside - Dead Corner"), TEXT("Cut Inside -> Dead Corner") },
		{ TEXT("Through Ball - Behind Defense P1"),
			TEXT("Through Ball -> Behind Defense") },
		{ TEXT("Through Ball - Behind Defense P2"),
			TEXT("Through Ball -> Behind Defense") },
		{ TEXT("Through Ball - Anti-Offside"),
			TEXT("Through Ball -> Anti-Offside") }
	};
	for (const TPair<FString, FString>& Shape : ReadableShapes)
	{
		FFMCodexUMGResolutionViewModel ShapeDTO;
		ShapeDTO.bVisible = true;
		ShapeDTO.StepLabel = Shape.Key;
		ShapeDTO.RouteLabel = Shape.Value;
		ShapeDTO.DecisionLabel = TEXT("Authoritative decision");
		RootResolution->RefreshFromPresentation(ShapeDTO);
		TestTrue(FString::Printf(TEXT("%s shape renders without bespoke rules"),
			*Shape.Key),
			Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionStepTitle")))->GetText().ToString() == Shape.Key
				&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
					TEXT("ResolutionRouteSummary")))->GetText().ToString()
						== Shape.Value);
	}

	for (const TCHAR* TerminalSemantic : {
		TEXT("RESULT: GOAL"), TEXT("RESULT: MISS"), TEXT("RESULT: NO GOAL"),
		TEXT("RESULT: IMMEDIATE MISS"), TEXT("RESULT: OFFSIDE"),
		TEXT("RESULT: OUT OF PLAY"),
		TEXT("RESULT: DEFENDER STOPPED ATTACK") })
	{
		FFMCodexUMGResolutionViewModel TerminalDTO;
		TerminalDTO.bVisible = true;
		TerminalDTO.bTerminal = true;
		TerminalDTO.StepLabel = TEXT("Attack Completed");
		TerminalDTO.TerminalLabel = TerminalSemantic;
		TerminalDTO.ContinuationLabel =
			TEXT("Attack complete | Score: Player A 1 - 0 Player B | Next attacker: Player B | Opportunity consumed: yes");
		RootResolution->RefreshFromPresentation(TerminalDTO);
		TestTrue(FString::Printf(TEXT("%s remains readable"), TerminalSemantic),
			Cast<UTextBlock>(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalSectionValue")))->GetText().ToString()
					== TerminalSemantic
				&& Cast<UTextBlock>(RootResolution->GetWidgetFromName(
					TEXT("ResolutionContinuationSectionValue")))
					->GetText().ToString().Contains(TEXT("Next attacker: Player B")));
	}

	FFMCodexUMGResolutionViewModel Rejected = Rich;
	Rejected.bRejected = true;
	Rejected.StepLabel = TEXT("Command Rejected");
	Rejected.DecisionLabel = TEXT("No resolution result was accepted");
	Rejected.ErrorLabel = TEXT("Readable rejection reason");
	RootResolution->RefreshFromPresentation(Rejected);
	TestTrue(TEXT("Rejected command cannot visually mix old accepted evidence"),
		IsVisible(RootResolution->GetWidgetFromName(
			TEXT("ResolutionRejectedRegion")))
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy")))
			&& RootResolution->GetRenderedDiceWidgets().IsEmpty()
			&& RootResolution->GetRenderedComparisonCount() == 0
			&& !IsVisible(RootResolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion"))));

	// Reproduce the reported Cut Inside path through production LocalPlay input.
	// The deterministic seed only stabilizes the normal Host-owned dice sequence.
	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	TestNotNull(TEXT("Cut Inside flow retains dedicated Interaction Panel"), Interaction);
	if (Interaction == nullptr)
	{
		return false;
	}
	const int32 CutInsideSeed =
		FindSeedForTacticalPointAndRolls(4, { 6, 2, 5, 1 });
	if (CutInsideSeed == INDEX_NONE)
	{
		return false;
	}
	Controller->SetNextDemoMatchSeedForTesting(CutInsideSeed);
	Interaction->RequestStartMatch();
	TestTrue(TEXT("Cut Inside UMG flow starts through Interaction Panel"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Screen->GetPresentation().Interaction.bCanRollTacticalPoints
			&& Screen->GetResolutionPanel() == RootResolution);
	Interaction->RequestTacticalPointRoll();
	TestTrue(TEXT("Cut Inside UMG flow begins through Interaction Panel"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetInteractionView().ActionPoint == 4);
	const EInitialTurnOrderPlayer CutInsideAttacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FFamilyExpectation CutInsideFamily = FamilyExpectations()[2];
	if (!DeployParticipants(*this, *Controller, CutInsideFamily, CutInsideAttacker)
		|| !SubmitRequiredSelections(
			*this, *Controller, CutInsideFamily, CutInsideAttacker))
	{
		return false;
	}
	bool bSawCutInsideRoute = false;
	bool bSawCutInsideComparison = false;
	bool bSawResolutionStarted = false;
	bool bAdvancedPastResolutionStarted = false;
	for (int32 Guard = 0;
		Guard < 12 && Controller->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		if (Controller->GetInteractionView().InteractionCategory
			!= EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			AddError(TEXT("Cut Inside resolution left ContinueResolution unexpectedly"));
			return false;
		}
		TestTrue(TEXT("Resolution overlay keeps its DTO-routed Continue reachable"),
			RootResolution->GetPresentation().bCanContinue
				&& IsVisible(RootResolution->GetWidgetFromName(
					TEXT("ResolutionContinueButton"))));
		RootResolution->RequestContinue();
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			AddError(TEXT("Cut Inside UMG Continue was rejected"));
			return false;
		}
		const FFMCodexUMGResolutionViewModel& Resolution =
			RootResolution->GetPresentation();
		if (Controller->GetLastDiagnostic().CommandName
			== TEXT("BeginResolutionSession"))
		{
			bSawResolutionStarted = Resolution.StepLabel
				== TEXT("Resolution Started")
				&& Controller->GetInteractionView().InteractionCategory
					== EFMCodexLocalMatchInteractionCategory::ContinueResolution
				&& Screen->GetWidgetFromName(TEXT("ResolutionPresentationLayer"))
					->GetVisibility() == ESlateVisibility::SelfHitTestInvisible;
		}
		else if (bSawResolutionStarted
			&& (Controller->GetLastDiagnostic().CommandName
					== TEXT("ResolveInitialRoute")
				|| Controller->GetLastDiagnostic().CommandName
					== TEXT("ResolveIntentDeterminedRoute")))
		{
			bAdvancedPastResolutionStarted = true;
		}
		bSawCutInsideRoute = bSawCutInsideRoute
			|| Resolution.RouteLabel.Contains(TEXT("Cut Inside"));
		bSawCutInsideComparison = bSawCutInsideComparison
			|| (Resolution.ComparisonEvidence.Num() >= 2
				&& Resolution.DecisionLabel.Contains(TEXT("Winner:")));
	}
	TestTrue(TEXT("Cut Inside overlay progresses past Resolution Started"),
		bSawResolutionStarted && bAdvancedPastResolutionStarted
			&& bSawCutInsideRoute && bSawCutInsideComparison);
	TestTrue(TEXT("Cut Inside terminal renders result and completion summary"),
		RootResolution->GetPresentation().bTerminal
			&& RootResolution->GetPresentation().TerminalLabel.StartsWith(
				TEXT("RESULT: "))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Attack complete"))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Score:"))
			&& RootResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Next attacker:")));
	TestTrue(TEXT("Completed attack enters next-player roll readiness"),
		Controller->GetInteractionView().bTacticalPointRollReady
			&& Screen->GetPresentation().Interaction.bCanRollTacticalPoints);
	const FFMCodexUMGMatchHeaderViewModel& SwitchedHeader =
		Screen->GetPresentation().Header;
	TestTrue(TEXT("Terminal feedback no longer masks the next attacker"),
		!Screen->GetPresentation().Resolution.bVisible
			&& Screen->GetWidgetFromName(TEXT("ResolutionPresentationLayer"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	TestTrue(TEXT("Completed attack projects Used then next-side Current"),
		SwitchedHeader.LeftAttackTurnTracker.Steps.Num() == 3
			&& SwitchedHeader.RightAttackTurnTracker.Steps.Num() == 3
			&& SwitchedHeader.LeftAttackTurnTracker.Steps[0].State
				== EFMCodexUMGAttackTurnStepState::Current
			&& SwitchedHeader.RightAttackTurnTracker.Steps[0].State
				== EFMCodexUMGAttackTurnStepState::Used
			&& Screen->GetPresentation().Interaction.bCanRollTacticalPoints);

	// Normal-demo deterministic ThroughBall: no test-only rules or deck mutation.
	FScopedPlayableWorld OneOnOneWorld;
	AFMCodexLocalMatchHostGameMode* OneOnOneHost = OneOnOneWorld.GetHost();
	AFMCodexLocalMatchPlayerController* OneOnOneController =
		OneOnOneWorld.GetController();
	if (OneOnOneHost == nullptr || OneOnOneController == nullptr)
	{
		return false;
	}
	OneOnOneController->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* OneOnOneScreen =
		OneOnOneController->GetPlayerMatchScreen();
	if (OneOnOneScreen == nullptr)
	{
		return false;
	}
	OneOnOneScreen->TakeWidget();
	UFMCodexInteractionPanelWidget* OneOnOneInteraction =
		OneOnOneScreen->GetInteractionPanel();
	UFMCodexResolutionPanelWidget* OneOnOneResolution =
		OneOnOneScreen->GetResolutionPanel();
	const FFMCodexLocalMatchDemoConfiguration Demo =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const FFamilyExpectation ThroughBallFamily = FamilyExpectations()[4];
	const int32 ThroughBallSeed =
		FindSeedForTacticalPointAndRolls(5, { 3, 6, 1, 2 });
	if (ThroughBallSeed == INDEX_NONE
		|| !OneOnOneHost->StartNewLocalMatch(
			Demo.OpeningInput, Demo.SkillRuleSet, ThroughBallSeed).bSuccess)
	{
		return false;
	}
	OneOnOneController->RefreshPresentation();
	AcknowledgeIfPending(*OneOnOneController);
	OneOnOneController->RollDemoTacticalPoints();
	const EInitialTurnOrderPlayer ThroughBallAttacker =
		OneOnOneController->GetInteractionView().CurrentAttackingPlayer;
	if (!OneOnOneController->GetLastDiagnostic().bHostSuccess
		|| !DeployParticipants(
			*this, *OneOnOneController, ThroughBallFamily,
			ThroughBallAttacker)
		|| !SubmitRequiredSelections(
			*this, *OneOnOneController, ThroughBallFamily,
			ThroughBallAttacker))
	{
		return false;
	}
	bool bSawOneOnOneHandoff = false;
	bool bSawShooterGoalkeeperPlan = false;
	bool bSawAuthoritativeDirectFormula = false;
	for (int32 Guard = 0;
		Guard < 14 && OneOnOneController->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		const EFMCodexLocalMatchInteractionCategory Category =
			OneOnOneController->GetInteractionView().InteractionCategory;
		if (Category
			== EFMCodexLocalMatchInteractionCategory::SelectOneOnOneShot)
		{
			bSawOneOnOneHandoff = true;
			TestTrue(TEXT("P2 feedback announces authoritative OneOnOne handoff"),
				OneOnOneResolution->GetPresentation().DecisionLabel.Contains(
					TEXT("One-on-One")));
			OneOnOneInteraction->RequestOneOnOne(
				EFMCodexUMGOneOnOneChoice::DirectShot);
		}
		else if (Category
			== EFMCodexLocalMatchInteractionCategory::ContinueResolution)
		{
			OneOnOneResolution->RequestContinue();
		}
		else
		{
			AddError(TEXT("Normal-demo OneOnOne reached unexpected category"));
			return false;
		}
		if (!OneOnOneController->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
		const FFMCodexUMGResolutionViewModel& Resolution =
			OneOnOneResolution->GetPresentation();
		if (Resolution.ComparisonEvidence.Num() >= 2
			&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
				TEXT("Shooter"))
			&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
				TEXT("Goalkeeper")))
		{
			bSawShooterGoalkeeperPlan = bSawShooterGoalkeeperPlan
				|| (Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
					TEXT("Shooter"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("Goalkeeper")));
			bSawAuthoritativeDirectFormula = bSawAuthoritativeDirectFormula
				|| (Resolution.DecisionLabel.Contains(TEXT("Winner:"))
					&& Resolution.DecisionLabel.Contains(TEXT("Reason:"))
					&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
						TEXT("D6"))
					&& Resolution.ComparisonEvidence[0].EvidenceLabel.Contains(
						TEXT("final"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("D6"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("authoritative modifier"))
					&& Resolution.ComparisonEvidence[1].EvidenceLabel.Contains(
						TEXT("final")));
		}
	}
	TestTrue(TEXT("Normal demo reaches OneOnOne and shooter/GK plan"),
		bSawOneOnOneHandoff && bSawShooterGoalkeeperPlan);
	TestTrue(TEXT("DirectShot shows authoritative final values/winner/reason"),
		bSawAuthoritativeDirectFormula);
	TestTrue(TEXT("OneOnOne terminal remains Goal/Miss with completion"),
		OneOnOneResolution->GetPresentation().bTerminal
			&& (OneOnOneResolution->GetPresentation().TerminalLabel
					== TEXT("RESULT: GOAL")
				|| OneOnOneResolution->GetPresentation().TerminalLabel
					== TEXT("RESULT: MISS"))
			&& OneOnOneResolution->GetPresentation().ContinuationLabel.Contains(
				TEXT("Attack complete")));

	FString ResolutionHeader;
	FString ResolutionSource;
	FString DiceHeader;
	FString DiceSource;
	FString RootHeader;
	FString RootSource;
	FString InteractionSource;
	FString ControllerSource;
	TestTrue(TEXT("Resolution authority audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.h"),
			ResolutionHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"),
				ResolutionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.h"),
				DiceHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.cpp"),
				DiceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString ResolutionWidgetSources =
		ResolutionHeader + ResolutionSource + DiceHeader + DiceSource;
	TestTrue(TEXT("Resolution Widgets receive reflected presentation only"),
		ResolutionHeader.Contains(TEXT("FFMCodexUMGResolutionViewModel"))
			&& DiceHeader.Contains(TEXT("FFMCodexUMGDiceResultViewModel"))
			&& !ResolutionWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !ResolutionWidgetSources.Contains(TEXT("HostGameMode"))
			&& !ResolutionWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !ResolutionWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Resolution Widgets contain no Formula/route/RNG authority"),
		!ResolutionWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !ResolutionWidgetSources.Contains(TEXT("CalculateGoalkeeperHalf"))
			&& !ResolutionWidgetSources.Contains(TEXT("EFormulaWinner"))
			&& !ResolutionWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !ResolutionWidgetSources.Contains(TEXT("RandomStream"))
			&& !ResolutionWidgetSources.Contains(TEXT("RandRange"))
			&& !ResolutionWidgetSources.Contains(TEXT("RollD6")));
	TestTrue(TEXT("Resolution Continue is a presentation intent, not authority"),
		ResolutionHeader.Contains(TEXT("FFMCodexResolutionContinueRequested"))
			&& ResolutionSource.Contains(TEXT("RequestContinue"))
			&& RootSource.Contains(TEXT("OnContinueRequested.AddDynamic"))
			&& !ResolutionWidgetSources.Contains(TEXT("ExecuteCommandByName"))
			&& !ResolutionWidgetSources.Contains(TEXT("ProcessEvent"))
			&& !ResolutionWidgetSources.Contains(TEXT("ContinueResolution()"))
			&& InteractionSource.Contains(TEXT("RequestContinue")));
	TestTrue(TEXT("Root delegates result rendering to configurable panel"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexResolutionPanelWidget>"))
			&& RootSource.Contains(TEXT("DedicatedResolutionPanelWidget"))
			&& RootSource.Contains(TEXT("RefreshFromPresentation"))
			&& !RootSource.Contains(TEXT("ResolutionResultText")));
	TestTrue(TEXT("Stage 6.4-6.7 root regions remain intact"),
		Screen->GetWidgetFromName(TEXT("MatchHeaderRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("FootballCardFieldRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("CurrentInteractionRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("ResolutionResultRegion")) != nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffOverlay")) == nullptr);
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGMatchHeaderVisualRefinementTest,
	"FMCodex.LocalPlay.ControlSurface.33.UMGMatchHeaderVisualRefinement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGMatchHeaderVisualRefinementTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Header refinement Host exists"), Host);
	TestNotNull(TEXT("Header refinement Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	UFMCodexMatchHeaderWidget* Header = Screen->GetMatchHeader();
	TestNotNull(TEXT("Root composes dedicated Match Header"), Header);
	if (Header == nullptr)
	{
		return false;
	}
	Header->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("BroadcastMatchHeaderBounds"),
		TEXT("BroadcastScoreboardRow"),
		TEXT("LeftPlayerBroadcastRegion"),
		TEXT("CentralBroadcastMatchFacts"),
		TEXT("RightPlayerBroadcastRegion"),
		TEXT("LeftPlayerIdentityGroup"),
		TEXT("RightPlayerIdentityGroup"),
		TEXT("LeftTacticalPointChip"),
		TEXT("RightTacticalPointChip"),
		TEXT("LeftAttackTurnSteps"),
		TEXT("RightAttackTurnSteps"),
		TEXT("CurrentAttackProgressLabel"),
		TEXT("CurrentMatchPhaseStatusLabel") })
	{
		TestNotNull(FString::Printf(TEXT("Header hierarchy contains %s"), Region),
			Header->GetWidgetFromName(Region));
	}
	auto IsVisible = [](const UWidget* Widget)
	{
		return Widget != nullptr
			&& Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Widget->GetVisibility() != ESlateVisibility::Hidden;
	};
	auto BuildHeader = [](const FFMCodexLocalMatchInteractionView& View)
	{
		return FFMCodexLocalMatchUMGPresentationBuilder::Build(
			View, FFMCodexLocalMatchResolutionFeedback(), FString()).Header;
	};

	const FFMCodexLocalMatchInteractionView PreMatch =
		FFMCodexLocalMatchInteractionViewBuilder::BuildNoActiveMatch();
	const FFMCodexUMGMatchHeaderViewModel PreMatchDTO = BuildHeader(PreMatch);
	Header->RefreshFromPresentation(PreMatchDTO);
	TestTrue(TEXT("Pre-match Header is neutral and bounded"),
		!PreMatchDTO.bMatchActive && !PreMatchDTO.bMatchEnded
			&& PreMatchDTO.MatchStatusLabel == TEXT("READY TO PLAY")
			&& PreMatchDTO.ActorStatusLabel == TEXT("WAITING TO START")
			&& Header->GetDisplayedScoreLabel() == TEXT("0 - 0")
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("LeftPlayerBroadcastRegion")))
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("RightPlayerBroadcastRegion"))));

	auto ActiveView = [](const EInitialTurnOrderPlayer Attacker,
		const EInitialTurnOrderPlayer Actor,
		const bool bSystem)
	{
		FFMCodexLocalMatchInteractionView View;
		View.bMatchActive = true;
		View.bCurrentAttackActive = true;
		View.PlayerAScore = 2;
		View.PlayerBScore = 1;
		View.CurrentAttackingPlayer = Attacker;
		View.ExpectedActingPlayer = bSystem
			? EInitialTurnOrderPlayer::None : Actor;
		View.bHumanInteraction = !bSystem;
		View.InteractionCategory = bSystem
			? EFMCodexLocalMatchInteractionCategory::ContinueResolution
			: EFMCodexLocalMatchInteractionCategory::SelectMarker;
		return View;
	};
	const TArray<FFMCodexLocalMatchInteractionView> ActiveCases = {
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerA, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::PlayerB, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerA,
			EInitialTurnOrderPlayer::None, true),
		ActiveView(EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerA, false),
		ActiveView(EInitialTurnOrderPlayer::PlayerB,
			EInitialTurnOrderPlayer::PlayerB, false)
	};
	const TArray<FString> ExpectedAttackers = {
		TEXT("PLAYER A ATTACKING"), TEXT("PLAYER A ATTACKING"),
		TEXT("PLAYER A ATTACKING"), TEXT("PLAYER B ATTACKING"),
		TEXT("PLAYER B ATTACKING")
	};
	const TArray<FString> ExpectedActors = {
		TEXT("PLAYER A TO ACT"), TEXT("PLAYER B TO ACT"),
		TEXT("SYSTEM RESOLUTION"), TEXT("PLAYER A TO ACT"),
		TEXT("PLAYER B TO ACT")
	};
	for (int32 Index = 0; Index < ActiveCases.Num(); ++Index)
	{
		const FFMCodexUMGMatchHeaderViewModel DTO = BuildHeader(ActiveCases[Index]);
		Header->RefreshFromPresentation(DTO);
		TestTrue(FString::Printf(TEXT("Attacker/actor case %d stays distinct"),
			Index), Header->GetDisplayedAttackerLabel() == ExpectedAttackers[Index]
			&& Header->GetDisplayedActorLabel() == ExpectedActors[Index]
			&& Header->GetDisplayedScoreLabel() == TEXT("2 - 1")
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("LeftPlayerBroadcastRegion")))
			&& IsVisible(Header->GetWidgetFromName(
				TEXT("RightPlayerBroadcastRegion"))));
	}

	FFMCodexUMGMatchHeaderViewModel DirectScore;
	DirectScore.PlayerALabel = TEXT("Player A");
	DirectScore.PlayerBLabel = TEXT("Player B");
	DirectScore.ScoreLabel = TEXT("7 - 3");
	DirectScore.PlayerAScoreLabel = TEXT("7");
	DirectScore.PlayerBScoreLabel = TEXT("3");
	DirectScore.LeftScoreLabel = TEXT("7");
	DirectScore.RightScoreLabel = TEXT("3");
	DirectScore.MatchStatusLabel = TEXT("MATCH IN PROGRESS");
	Header->RefreshFromPresentation(DirectScore);
	const UTextBlock* CentralScore = Cast<UTextBlock>(Header->GetWidgetFromName(
		TEXT("CentralBroadcastScoreValue")));
	TestTrue(TEXT("Scoreboard has one central DTO score without side duplication"),
		Header->GetDisplayedScoreLabel() == TEXT("7 - 3")
			&& CentralScore != nullptr
			&& CentralScore->GetText().ToString() == TEXT("7 - 3")
			&& Header->GetWidgetFromName(TEXT("LeftPlayerScoreValue")) == nullptr
			&& Header->GetWidgetFromName(TEXT("RightPlayerScoreValue")) == nullptr);

	const TArray<TPair<EMatchResultType, FString>> EndedCases = {
		{ EMatchResultType::HomeWin, TEXT("Player A Win") },
		{ EMatchResultType::AwayWin, TEXT("Player B Win") },
		{ EMatchResultType::Draw, TEXT("Draw") }
	};
	for (const TPair<EMatchResultType, FString>& EndedCase : EndedCases)
	{
		FFMCodexLocalMatchInteractionView EndedView = ActiveCases[0];
		EndedView.bMatchEnded = true;
		EndedView.bCurrentAttackActive = false;
		EndedView.MatchResult = EndedCase.Key;
		EndedView.PlayerAScore = EndedCase.Key == EMatchResultType::AwayWin ? 1 : 3;
		EndedView.PlayerBScore = EndedCase.Key == EMatchResultType::HomeWin ? 2 : 3;
		EndedView.ExpectedActingPlayer = EInitialTurnOrderPlayer::None;
		const FFMCodexUMGMatchHeaderViewModel EndedDTO = BuildHeader(EndedView);
		Header->RefreshFromPresentation(EndedDTO);
		TestTrue(FString::Printf(TEXT("Ended result %s is canonical"),
			*EndedCase.Value),
			EndedDTO.MatchStatusLabel == TEXT("MATCH ENDED")
				&& EndedDTO.MatchResultLabel == EndedCase.Value
				&& IsVisible(Header->GetWidgetFromName(
					TEXT("LeftPlayerBroadcastRegion")))
				&& IsVisible(Header->GetWidgetFromName(
					TEXT("RightPlayerBroadcastRegion"))));
	}

	// Real normal-demo Cross flow proves refresh, attacker/defender distinction,
	// system resolution, rejection atomicity and completion/next-attack state.
	UFMCodexInteractionPanelWidget* Interaction = Screen->GetInteractionPanel();
	if (Interaction == nullptr)
	{
		return false;
	}
	Controller->RefreshPresentation();
	Interaction->RequestStartMatch();
	TestTrue(TEXT("Real Header refreshes active match without handoff"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Header->GetPresentation().bMatchActive
			&& Header->GetPresentation().ActorStatusLabel.Contains(TEXT("TO ACT")));
	const bool bFirstAttackerOnLeft =
		Header->GetPresentation().bCurrentAttackerOnLeft;
	const FString FirstAttackerLabel =
		Header->GetPresentation().CurrentAttackerLabel;
	const UTextBlock* PreRollPhase = Cast<UTextBlock>(
		Header->GetWidgetFromName(TEXT("CurrentMatchPhaseStatusLabel")));
	TestTrue(TEXT("Pre-roll Header keeps TP absent and central waiting state"),
		!Header->GetPresentation().bShowLeftTacticalPointChip
			&& !Header->GetPresentation().bShowRightTacticalPointChip
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("LeftTacticalPointChip")))
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("RightTacticalPointChip")))
			&& PreRollPhase != nullptr
			&& PreRollPhase->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::WaitingForTacticalPointRoll()));
	const FString CurrentTrackerPrefix =
		Header->GetPresentation().bCurrentAttackerOnLeft
			? TEXT("LeftAttackTurnSteps") : TEXT("RightAttackTurnSteps");
	const UBorder* CurrentTrackerNode = Cast<UBorder>(
		Header->GetWidgetFromName(FName(*(CurrentTrackerPrefix + TEXT("Frame0")))));
	const UBorder* RemainingTrackerNode = Cast<UBorder>(
		Header->GetWidgetFromName(FName(*(CurrentTrackerPrefix + TEXT("Frame1")))));
	const UVerticalBoxSlot* LeftTrackerSlot = Cast<UVerticalBoxSlot>(
		Header->GetWidgetFromName(TEXT("LeftAttackTurnTracker"))->Slot);
	const UVerticalBoxSlot* RightTrackerSlot = Cast<UVerticalBoxSlot>(
		Header->GetWidgetFromName(TEXT("RightAttackTurnTracker"))->Slot);
	TestTrue(TEXT("Attack-turn steps render as centered circular nodes"),
		CurrentTrackerNode != nullptr && RemainingTrackerNode != nullptr
			&& CurrentTrackerNode->Background.DrawAs
				== ESlateBrushDrawType::RoundedBox
			&& RemainingTrackerNode->Background.DrawAs
				== ESlateBrushDrawType::RoundedBox
			&& CurrentTrackerNode->Background.OutlineSettings.Width
				> RemainingTrackerNode->Background.OutlineSettings.Width
			&& LeftTrackerSlot != nullptr
			&& LeftTrackerSlot->GetHorizontalAlignment() == HAlign_Center
			&& RightTrackerSlot != nullptr
			&& RightTrackerSlot->GetHorizontalAlignment() == HAlign_Center);
	FFMCodexUMGMatchHeaderViewModel TrackerStyleFixture =
		Header->GetPresentation();
	FFMCodexUMGAttackTurnTrackerViewModel& StyledTracker =
		bFirstAttackerOnLeft
			? TrackerStyleFixture.LeftAttackTurnTracker
			: TrackerStyleFixture.RightAttackTurnTracker;
	StyledTracker.Steps[0].State = EFMCodexUMGAttackTurnStepState::Used;
	StyledTracker.Steps[1].State = EFMCodexUMGAttackTurnStepState::Current;
	StyledTracker.Steps[2].State = EFMCodexUMGAttackTurnStepState::Remaining;
	Header->RefreshFromPresentation(TrackerStyleFixture);
	const UBorder* UsedTrackerNode = Cast<UBorder>(Header->GetWidgetFromName(
		FName(*(CurrentTrackerPrefix + TEXT("Frame0")))));
	const UBorder* StyledCurrentTrackerNode = Cast<UBorder>(
		Header->GetWidgetFromName(
			FName(*(CurrentTrackerPrefix + TEXT("Frame1")))));
	const UBorder* StyledRemainingTrackerNode = Cast<UBorder>(
		Header->GetWidgetFromName(
			FName(*(CurrentTrackerPrefix + TEXT("Frame2")))));
	const UTextBlock* UsedTrackerLabel = Cast<UTextBlock>(
		Header->GetWidgetFromName(
			FName(*(CurrentTrackerPrefix + TEXT("Label0")))));
	const UTextBlock* RemainingTrackerLabel = Cast<UTextBlock>(
		Header->GetWidgetFromName(
			FName(*(CurrentTrackerPrefix + TEXT("Label2")))));
	const USizeBox* UsedTrackerBounds = Cast<USizeBox>(
		Header->GetWidgetFromName(
			FName(*(CurrentTrackerPrefix + TEXT("Bounds0")))));
	TestTrue(TEXT("Tracker states use hollow Remaining, filled Used and ringed Current"),
		UsedTrackerNode != nullptr && StyledCurrentTrackerNode != nullptr
			&& StyledRemainingTrackerNode != nullptr
			&& UsedTrackerLabel != nullptr && RemainingTrackerLabel != nullptr
			&& UsedTrackerBounds != nullptr
			&& UsedTrackerNode->Background.DrawAs
				== ESlateBrushDrawType::RoundedBox
			&& StyledCurrentTrackerNode->Background.DrawAs
				== ESlateBrushDrawType::RoundedBox
			&& StyledRemainingTrackerNode->Background.DrawAs
				== ESlateBrushDrawType::RoundedBox
			&& UsedTrackerNode->Background.TintColor.GetSpecifiedColor().A
				> StyledRemainingTrackerNode->Background.TintColor
					.GetSpecifiedColor().A
			&& StyledCurrentTrackerNode->Background.OutlineSettings.Width
				> UsedTrackerNode->Background.OutlineSettings.Width
			&& UsedTrackerLabel->GetRenderOpacity()
				> RemainingTrackerLabel->GetRenderOpacity()
			&& FMath::IsNearlyEqual(UsedTrackerBounds->GetWidthOverride(), 24.0f)
			&& FMath::IsNearlyEqual(UsedTrackerBounds->GetHeightOverride(), 24.0f));
	Controller->RefreshPresentation();
	TestTrue(TEXT("Header actor follows the projected authoritative actor"),
		Header->GetDisplayedActorLabel().Contains(
			FFMCodexLocalMatchInteractionViewBuilder::ToString(
				Controller->GetInteractionView().ExpectedActingPlayer).ToUpper()));
	Interaction->RequestTacticalPointRoll();
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	const UTextBlock* TacticalPointStatus = Cast<UTextBlock>(
		Header->GetWidgetFromName(TEXT("CurrentMatchPhaseStatusLabel")));
	const bool bTacticalPointChipOnLeft =
		Header->GetPresentation().bShowLeftTacticalPointChip;
	const UWidget* CurrentTacticalPointChip = Header->GetWidgetFromName(
		bTacticalPointChipOnLeft
			? TEXT("LeftTacticalPointChip") : TEXT("RightTacticalPointChip"));
	const UWidget* DefendingTacticalPointChip = Header->GetWidgetFromName(
		bTacticalPointChipOnLeft
			? TEXT("RightTacticalPointChip") : TEXT("LeftTacticalPointChip"));
	const UTextBlock* TacticalPointValue = Cast<UTextBlock>(
		Header->GetWidgetFromName(bTacticalPointChipOnLeft
			? TEXT("LeftTacticalPointChipValue")
			: TEXT("RightTacticalPointChipValue")));
	const UTextBlock* TacticalPointHeading = Cast<UTextBlock>(
		Header->GetWidgetFromName(bTacticalPointChipOnLeft
			? TEXT("LeftTacticalPointChipLabel")
			: TEXT("RightTacticalPointChipLabel")));
	TestTrue(TEXT("Post-roll TP belongs only to the current attacker Header"),
		Header->GetPresentation().CurrentAttackerTacticalPoints > 0
			&& Header->GetPresentation().bShowLeftTacticalPointChip
				!= Header->GetPresentation().bShowRightTacticalPointChip
			&& bTacticalPointChipOnLeft == bFirstAttackerOnLeft
			&& IsVisible(CurrentTacticalPointChip)
			&& !IsVisible(DefendingTacticalPointChip)
			&& TacticalPointValue != nullptr
			&& TacticalPointValue->GetText().ToString()
				== FString::FromInt(
					Header->GetPresentation().CurrentAttackerTacticalPoints)
			&& TacticalPointValue->GetFont().Size == 14
			&& TacticalPointHeading != nullptr
			&& TacticalPointHeading->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::TacticalPointsHeading())
			&& TacticalPointHeading->GetFont().Size == 9
			&& TacticalPointStatus != nullptr
			&& TacticalPointStatus->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::MatchScreenLabel(
					Header->GetPresentation().CurrentPhaseLabel))
			&& !TacticalPointStatus->GetText().EqualTo(
				FFMCodexPlayerUIPresentationText::TacticalPoints(
					Header->GetPresentation().CurrentAttackerTacticalPoints))
			&& TacticalPointStatus->GetFont().Size == 10
			&& TacticalPointStatus->GetFont().Size
				< Cast<UTextBlock>(Header->GetWidgetFromName(
					TEXT("CentralBroadcastScoreValue")))->GetFont().Size);
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer Defender = OtherSide(Attacker);
	const FFamilyExpectation CrossFamily = FamilyExpectations()[0];
	if (!DeployParticipants(*this, *Controller, CrossFamily, Attacker))
	{
		return false;
	}
	const FName CarrierId = FamilyCardId(CrossFamily, Attacker);
	AcknowledgeIfPending(*Controller);
	TestTrue(TEXT("Carrier Header shows attacker acting"),
		Header->GetDisplayedAttackerLabel().Contains(
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("PLAYER A") : TEXT("PLAYER B"))
			&& Header->GetDisplayedActorLabel().Contains(
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));
	Interaction->RequestCarrier(CarrierId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	TestTrue(TEXT("Marker Header keeps attacker but shows defender acting"),
		Header->GetDisplayedAttackerLabel().Contains(
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? TEXT("PLAYER A") : TEXT("PLAYER B"))
			&& Header->GetDisplayedActorLabel().Contains(
				Defender == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));
	if (Interaction->GetPresentation().SelectionChoices.IsEmpty())
	{
		return false;
	}
	Interaction->RequestMarker(
		Interaction->GetPresentation().SelectionChoices[0].OptionId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	const FFMCodexUMGSelectionChoiceViewModel* CrossSkill =
		Interaction->GetPresentation().SelectionChoices.FindByPredicate(
			[](const FFMCodexUMGSelectionChoiceViewModel& Candidate)
			{
				return Candidate.Label.Contains(TEXT("Cross"));
			});
	if (CrossSkill == nullptr)
	{
		return false;
	}
	Interaction->RequestSkill(CrossSkill->OptionId);
	if (!Controller->GetLastDiagnostic().bHostSuccess)
	{
		return false;
	}
	for (int32 Guard = 0; Guard < 4; ++Guard)
	{
		const EFMCodexUMGInteractionCategory Category =
			Interaction->GetPresentation().Category;
		if (Category != EFMCodexUMGInteractionCategory::SelectRunner
			&& Category != EFMCodexUMGInteractionCategory::SelectHelper)
		{
			break;
		}
		if (!Interaction->GetPresentation().SelectionChoices.IsEmpty())
		{
			const FName Id =
				Interaction->GetPresentation().SelectionChoices[0].OptionId;
			if (Category == EFMCodexUMGInteractionCategory::SelectRunner)
			{
				Interaction->RequestRunner(Id);
			}
			else
			{
				Interaction->RequestHelper(Id);
			}
		}
		else if (Interaction->GetPresentation().bCanResolveNoLegal)
		{
			Interaction->RequestNoLegal();
		}
		else
		{
			Interaction->RequestDecline();
		}
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
	}
	Interaction->RequestBranch(EFMCodexUMGBranchIntent::CrossHigh);
	TestTrue(TEXT("System resolution is explicit and attacker is retained"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Header->GetPresentation().bSystemResolution
			&& Header->GetDisplayedActorLabel() == TEXT("SYSTEM RESOLUTION")
			&& Header->GetDisplayedAttackerLabel().Contains(
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? TEXT("PLAYER A") : TEXT("PLAYER B")));

	const FFMCodexUMGMatchHeaderViewModel BeforeRejected =
		Header->GetPresentation();
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestRollTacticalPoints();
	TestTrue(TEXT("Rejected command cannot optimistically mutate Header"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& StateBeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Header->GetPresentation().ScoreLabel == BeforeRejected.ScoreLabel
			&& Header->GetPresentation().AttackerStatusLabel
				== BeforeRejected.AttackerStatusLabel
			&& Header->GetPresentation().ActorStatusLabel
				== BeforeRejected.ActorStatusLabel
			&& Header->GetPresentation().MatchStatusLabel
				== BeforeRejected.MatchStatusLabel);

	for (int32 Guard = 0;
		Guard < 12 && Controller->GetInteractionView().bCurrentAttackActive;
		++Guard)
	{
		Interaction->RequestContinue();
		if (!Controller->GetLastDiagnostic().bHostSuccess)
		{
			return false;
		}
	}
	const FMatchPlayState& CompletedState = Host->GetMatchSnapshot().Snapshot;
	const FMatchRuntimeState& CompletedRuntime = CompletedState.RuntimeState;
	TestTrue(TEXT("Completion Header score is refreshed from authority"),
		Header->GetPresentation().PlayerAScoreLabel
			== FString::FromInt(CompletedRuntime.PlayerAState.Score)
			&& Header->GetPresentation().PlayerBScoreLabel
				== FString::FromInt(CompletedRuntime.PlayerBState.Score));
	TestTrue(TEXT("Between attacks identifies next attacker without active claim"),
		!Header->GetPresentation().bAttackActive
			&& Header->GetDisplayedAttackerLabel().StartsWith(
				TEXT("NEXT ATTACKER:"))
			&& !Header->GetDisplayedAttackerLabel().EndsWith(TEXT(" ATTACKING")));
	const FFMCodexUMGMatchHeaderViewModel& SwitchedHeader =
		Header->GetPresentation();
	const bool bCompletedAttackerOnLeft =
		SwitchedHeader.LeftPlayerLabel == FirstAttackerLabel;
	const FFMCodexUMGAttackTurnTrackerViewModel& CompletedAttackerTracker =
		bCompletedAttackerOnLeft
			? SwitchedHeader.LeftAttackTurnTracker
			: SwitchedHeader.RightAttackTurnTracker;
	const FFMCodexUMGAttackTurnTrackerViewModel& NewAttackerTracker =
		bCompletedAttackerOnLeft
			? SwitchedHeader.RightAttackTurnTracker
			: SwitchedHeader.LeftAttackTurnTracker;
	TestTrue(TEXT("Side switch clears stale TP and projects Used then Current"),
		CompletedAttackerTracker.Steps[0].State
			== EFMCodexUMGAttackTurnStepState::Used
			&& NewAttackerTracker.Steps[0].State
				== EFMCodexUMGAttackTurnStepState::Current
			&& !SwitchedHeader.bShowLeftTacticalPointChip
			&& !SwitchedHeader.bShowRightTacticalPointChip
			&& SwitchedHeader.LeftTacticalPoints == 0
			&& SwitchedHeader.RightTacticalPoints == 0
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("LeftTacticalPointChip")))
			&& !IsVisible(Header->GetWidgetFromName(
				TEXT("RightTacticalPointChip"))));
	Interaction->RequestTacticalPointRoll();
	const FFMCodexUMGMatchHeaderViewModel& NewAttackHeader =
		Header->GetPresentation();
	const bool bFirstAttackerNowOnLeft =
		NewAttackHeader.LeftPlayerLabel == FirstAttackerLabel;
	const FString& NewAttackerPanelLabel =
		NewAttackHeader.bShowLeftTacticalPointChip
			? NewAttackHeader.LeftPlayerLabel : NewAttackHeader.RightPlayerLabel;
	TestTrue(TEXT("New attacker receives TP only after its own authoritative roll"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& !Controller->GetResolutionFeedback().bTerminal
			&& NewAttackHeader.CurrentAttackerLabel != FirstAttackerLabel
			&& NewAttackHeader.bShowLeftTacticalPointChip
				!= NewAttackHeader.bShowRightTacticalPointChip
			&& NewAttackHeader.bShowLeftTacticalPointChip
				== NewAttackHeader.bCurrentAttackerOnLeft
			&& NewAttackerPanelLabel == NewAttackHeader.CurrentAttackerLabel
			&& (bFirstAttackerNowOnLeft
				? NewAttackHeader.LeftTacticalPoints == 0
					&& NewAttackHeader.RightTacticalPoints > 0
				: NewAttackHeader.RightTacticalPoints == 0
					&& NewAttackHeader.LeftTacticalPoints > 0));

	FString HeaderHeader;
	FString HeaderSource;
	FString RootHeader;
	FString RootSource;
	FString PitchSource;
	FString CardSource;
	FString InteractionSource;
	FString ResolutionSource;
	FString ControllerSource;
	TestTrue(TEXT("Header boundary audit sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.h"),
			HeaderHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"),
				HeaderSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.h"),
				RootHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				RootSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"),
				PitchSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"),
				InteractionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"),
				ResolutionSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource));
	const FString HeaderWidgetSources = HeaderHeader + HeaderSource;
	TestTrue(TEXT("Match Header receives reflected presentation DTO only"),
		HeaderHeader.Contains(TEXT("FFMCodexUMGMatchHeaderViewModel"))
			&& !HeaderWidgetSources.Contains(TEXT("AuthoritativeSession"))
			&& !HeaderWidgetSources.Contains(TEXT("HostGameMode"))
			&& !HeaderWidgetSources.Contains(TEXT("FMatchPlayState"))
			&& !HeaderWidgetSources.Contains(TEXT("FMatchPlayCurrentAttack"))
			&& !HeaderWidgetSources.Contains(TEXT("D6Provider")));
	TestTrue(TEXT("Match Header contains no score/winner/actor calculation"),
		!HeaderWidgetSources.Contains(TEXT("CurrentAttackingPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("ExpectedActingPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("EInitialTurnOrderPlayer"))
			&& !HeaderWidgetSources.Contains(TEXT("EMatchResultType"))
			&& !HeaderWidgetSources.Contains(TEXT("FMath"))
			&& !HeaderWidgetSources.Contains(TEXT("Score =")));
	TestTrue(TEXT("Match Header contains no gameplay rules, RNG or input"),
		!HeaderWidgetSources.Contains(TEXT("FormulaResolver"))
			&& !HeaderWidgetSources.Contains(TEXT("RouteThreshold"))
			&& !HeaderWidgetSources.Contains(TEXT("Legality"))
			&& !HeaderWidgetSources.Contains(TEXT("RandRange"))
			&& !HeaderWidgetSources.Contains(TEXT("RandomStream"))
			&& !HeaderWidgetSources.Contains(TEXT("UButton"))
			&& !HeaderWidgetSources.Contains(TEXT("OnClicked"))
			&& !HeaderWidgetSources.Contains(TEXT("ExecuteCommandByName")));
	TestTrue(TEXT("Header is family-independent and contains no resolution detail"),
		!HeaderWidgetSources.Contains(TEXT("Cross"))
			&& !HeaderWidgetSources.Contains(TEXT("ThroughBall"))
			&& !HeaderWidgetSources.Contains(TEXT("Dice"))
			&& !HeaderWidgetSources.Contains(TEXT("Formula")));
	TestTrue(TEXT("Root delegates Header to a configurable dedicated Widget"),
		RootHeader.Contains(TEXT("TSubclassOf<UFMCodexMatchHeaderWidget>"))
			&& RootSource.Contains(TEXT("DedicatedMatchHeaderWidget"))
			&& !RootSource.Contains(TEXT("MatchHeaderText")));
	TestNotNull(TEXT("Stage 6.5 Pitch Widget remains present"),
		Screen->GetPitchWidget());
	TestNotNull(TEXT("Stage 6.7 Interaction Panel remains present"),
		Screen->GetInteractionPanel());
	TestNotNull(TEXT("Stage 6.8 Resolution Panel remains present"),
		Screen->GetResolutionPanel());
	TestTrue(TEXT("Stage 6.5-6.8 source structures remain present"),
		PitchSource.Contains(TEXT("CanonicalPitchSlot"))
			&& PitchSource.Contains(TEXT("RenderedSlotWidgets.Add"))
			&& CardSource.Contains(TEXT("PlayerCardFrame"))
			&& InteractionSource.Contains(TEXT("InteractionPanelBounds"))
			&& ResolutionSource.Contains(TEXT("ResolutionPanelBounds")));
	TestTrue(TEXT("Slate developer reference surface remains available"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexUMGVisualStyleFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.34.UMGVisualStyleFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexUMGVisualStyleFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIStyle& Style = FFMCodexPlayerUIStyle::Get();
	TestTrue(TEXT("Shared UI style defaults are valid"),
		Style.HasValidDefaults());
	TestTrue(TEXT("Score and terminal typography dominate body text"),
		Style.GetFontSize(EFMCodexPlayerUITextRole::Score)
			> Style.GetFontSize(EFMCodexPlayerUITextRole::Identity)
			&& Style.GetFontSize(EFMCodexPlayerUITextRole::TerminalResult)
				> Style.GetFontSize(EFMCodexPlayerUITextRole::Body));
	TestTrue(TEXT("Temporary player accents are replaceable and distinct"),
		!Style.GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent)));
	TestTrue(TEXT("Action roles have visibly distinct presentation colors"),
		!Style.GetColor(EFMCodexPlayerUIColorRole::ActionPrimary).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::ActionSecondary))
			&& !Style.GetColor(EFMCodexPlayerUIColorRole::ActionDecline).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::ActionPrimary)));
	TestTrue(TEXT("Presentation-only status and terminal emphasis is defined"),
		Style.GetStatusBadgeColor(TEXT("AVAILABLE")).Equals(
			Style.GetColor(EFMCodexPlayerUIColorRole::StatusAvailable))
			&& Style.GetStatusBadgeColor(TEXT("USED")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::StatusUsed))
			&& Style.GetTerminalColor(TEXT("GOAL")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::Success))
			&& Style.GetTerminalColor(TEXT("NO GOAL")).Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::Danger)));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Style foundation Host exists"), Host);
	TestNotNull(TEXT("Style foundation Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	for (const TCHAR* RootRegion : {
		TEXT("MatchScreenStyleBackground"), TEXT("MatchHeaderRegion"),
		TEXT("FootballCardFieldRegion"), TEXT("CurrentInteractionRegion"),
		TEXT("ResolutionResultRegion") })
	{
		TestNotNull(FString::Printf(TEXT("Styled root contains %s"), RootRegion),
			Screen->GetWidgetFromName(RootRegion));
	}

	UFMCodexMatchHeaderWidget* Header = Screen->GetMatchHeader();
	if (Header == nullptr)
	{
		return false;
	}
	Header->TakeWidget();
	FFMCodexUMGMatchHeaderViewModel HeaderDTO;
	HeaderDTO.bMatchActive = true;
	HeaderDTO.bAttackActive = true;
	HeaderDTO.MatchStatusLabel = TEXT("MATCH IN PROGRESS");
	HeaderDTO.PlayerAScoreLabel = TEXT("2");
	HeaderDTO.PlayerBScoreLabel = TEXT("1");
	HeaderDTO.ScoreLabel = TEXT("2 - 1");
	HeaderDTO.AttackerStatusLabel = TEXT("PLAYER A ATTACKING");
	HeaderDTO.ActorStatusLabel = TEXT("PLAYER B TO ACT");
	HeaderDTO.LeftAttackTurnTracker.PrimarySideColor =
		Style.GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent);
	HeaderDTO.RightAttackTurnTracker.PrimarySideColor =
		Style.GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent);
	HeaderDTO.bHumanAction = true;
	Header->RefreshFromPresentation(HeaderDTO);
	const UBorder* AttackerRegion = Cast<UBorder>(Header->GetWidgetFromName(
		TEXT("LeftPlayerBroadcastRegion")));
	const UBorder* ActorRegion = Cast<UBorder>(Header->GetWidgetFromName(
		TEXT("RightPlayerBroadcastRegion")));
	TestTrue(TEXT("Header styling preserves ATTACKING versus TO ACT"),
		AttackerRegion != nullptr && ActorRegion != nullptr
			&& AttackerRegion->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::PlayerAAccent))
			&& ActorRegion->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::PlayerBAccent))
			&& Header->GetDisplayedAttackerLabel()
				== TEXT("PLAYER A ATTACKING")
			&& Header->GetDisplayedActorLabel() == TEXT("PLAYER B TO ACT"));
	HeaderDTO.bSystemResolution = true;
	HeaderDTO.bHumanAction = false;
	HeaderDTO.ActorStatusLabel = TEXT("SYSTEM RESOLUTION");
	Header->RefreshFromPresentation(HeaderDTO);
	TestTrue(TEXT("Header system resolution preserves the broadcast frame"),
		ActorRegion != nullptr && Header->GetPresentation().bSystemResolution);
	HeaderDTO.bMatchEnded = true;
	HeaderDTO.MatchStatusLabel = TEXT("MATCH ENDED");
	HeaderDTO.MatchResultLabel = TEXT("Player A Win");
	Header->RefreshFromPresentation(HeaderDTO);
	TestTrue(TEXT("Match-ended style preserves canonical result priority"),
		Header->GetPresentation().MatchResultLabel == TEXT("Player A Win")
			&& Header->GetWidgetFromName(TEXT("MatchHeaderFinalResultRegion"))
				->GetVisibility() == ESlateVisibility::Visible
			&& Header->GetWidgetFromName(
				TEXT("RightPlayerBroadcastRegion"))->GetVisibility()
					!= ESlateVisibility::Collapsed);

	FFMCodexUMGCardViewModel Card;
	Card.CardId = TEXT("Style.Card.AllFamilies");
	Card.IdentityLabel = TEXT("Style Foundation Card");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW");
	Card.RarityLabel = TEXT("Club");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary = TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		Region.RegionLabel = RegionIndex == 0
			? TEXT("PLAYER B HALF") : TEXT("PLAYER A HALF");
		Region.ZoneContextLabel = TEXT("Canonical relative zones");
		Region.bCurrentAttackingSide = RegionIndex == 0;
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Style.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.SlotLabel = FString::Printf(TEXT("POSITION %d"), SlotIndex + 1);
			Slot.PhysicalHalfLabel = Region.RegionLabel;
			Slot.PlayerARelativeZoneLabel = TEXT("Relative A");
			Slot.PlayerBRelativeZoneLabel = TEXT("Relative B");
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied)
			{
				Slot.Card = Card;
			}
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	if (Pitch == nullptr)
	{
		return false;
	}
	Pitch->TakeWidget();
	Pitch->RefreshFromPitchPresentation(PitchPresentation);
	TestTrue(TEXT("Styled Pitch preserves two halves and 10/10 slots"),
		Pitch->GetPresentation().Num() == 2
			&& Pitch->GetRenderedSlotWidgets().Num() == 10
			&& Pitch->GetWidgetFromName(TEXT("PitchBackgroundAssetHook"))
				!= nullptr
			&& Pitch->GetWidgetFromName(TEXT("PhysicalHalfVisualSeparator"))
				!= nullptr);
	UFMCodexPitchSlotWidget* OccupiedSlot =
		Pitch->GetRenderedSlotWidgets()[0];
	UFMCodexPitchSlotWidget* EmptySlot =
		Pitch->GetRenderedSlotWidgets()[1];
	OccupiedSlot->TakeWidget();
	EmptySlot->TakeWidget();
	TestTrue(TEXT("Pitch slots distinguish occupied from passive empty state"),
		OccupiedSlot->IsShowingOccupiedCard()
			&& !EmptySlot->IsShowingOccupiedCard()
			&& EmptySlot->GetWidgetFromName(TEXT("EmptySpatialLocation"))
				!= nullptr
			&& Cast<UBorder>(OccupiedSlot->GetWidgetFromName(
				TEXT("CanonicalPitchSlotBorder")))->GetBrushColor().Equals(
					Style.GetColor(
						EFMCodexPlayerUIColorRole::OccupiedPitchSlot))
			&& Cast<UBorder>(EmptySlot->GetWidgetFromName(
				TEXT("CanonicalPitchSlotBorder")))->GetBrushColor().Equals(
					Style.GetColor(
						EFMCodexPlayerUIColorRole::EmptyPitchSlot)));
	UFMCodexPlayerCardWidget* PitchCard = OccupiedSlot->GetCardWidget();
	if (PitchCard == nullptr)
	{
		return false;
	}
	PitchCard->TakeWidget();
	TestTrue(TEXT("PitchMini Card preserves identity and asset-ready hooks"),
		PitchCard->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::PitchMini
			&& PitchCard->GetConfiguredDimensions().Equals(FVector2D(136.0f, 140.0f))
			&& PitchCard->GetRenderedSkillCount() == 0
			&& PitchCard->GetRenderedAttributeCount() == 0
			&& PitchCard->GetRenderedStatusBadgeCount() == 0
			&& PitchCard->GetWidgetFromName(TEXT("CardFrameAssetHook")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniPortraitImage")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniPlayerName")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniIdentityRow")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniSkillBand")) == nullptr
			&& PitchCard->GetWidgetFromName(
				TEXT("PitchMiniTacticalMatchStrokeTop")) != nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniRarityAccent")) == nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniRarityAccentBounds")) == nullptr
			&& PitchCard->GetWidgetFromName(TEXT("PitchMiniRarityBadge")) == nullptr
			&& OccupiedSlot->GetWidgetFromName(TEXT("DeploymentTargetState"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	PitchCard->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("InteractionChoice retains all five Skill labels"),
		PitchCard->GetPresentationMode()
			== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& PitchCard->GetRenderedSkillCount() == Card.SkillLabels.Num()
			&& PitchCard->GetRenderedAttributeCount() == 10);
	Card.bGoalkeeper = true;
	Card.RoleLabel = TEXT("GK");
	Card.StatusLabels = { TEXT("GK USED"), TEXT("GK ACTIVE") };
	Card.CompactAttributeSummary = TEXT("HAN 5 | REF 4 | AER 3 | 1v1 4");
	Card.FullAttributeSummary =
		TEXT("HAN 5 | POS 4 | REF 4 | AER 3 | ANT 3 | 1v1 4");
	PitchCard->RefreshFromPresentation(
		Card, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestTrue(TEXT("GK Full Card keeps canonical data without status/debug chrome"),
		PitchCard->IsGoalkeeperVisualVariant()
			&& PitchCard->GetRenderedStatusBadgeCount() == 0
			&& Cast<UBorder>(PitchCard->GetWidgetFromName(
				TEXT("PlayerCardFrame")))->GetBrushColor().Equals(
					Style.GetRarityAccentColor(Card.RarityLabel)));

	UFMCodexInteractionPanelWidget* Interaction =
		Screen->GetInteractionPanel();
	if (Interaction == nullptr)
	{
		return false;
	}
	Interaction->TakeWidget();
	FFMCodexUMGInteractionViewModel Action;
	Action.Category = EFMCodexUMGInteractionCategory::ContinueResolution;
	Action.KickerLabel = TEXT("PLAYER ACTION");
	Action.ExpectedActorLabel = TEXT("PLAYER A TO ACT");
	Action.TitleLabel = TEXT("CHOOSE NEXT ACTION");
	Action.bCanContinue = true;
	Action.bCanDecline = true;
	Action.bCanResolveNoLegal = true;
	Action.PrimaryActionLabel = TEXT("CONTINUE");
	Action.DeclineActionLabel = TEXT("DECLINE");
	Action.NoLegalActionLabel = TEXT("NO LEGAL CHOICE");
	Interaction->RefreshFromPresentation(Action);
	auto ButtonTint = [](const UButton* Button)
	{
		return Button->GetStyle().Normal.TintColor.GetSpecifiedColor();
	};
	const UButton* PrimaryButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionContinueButton")));
	const UButton* DeclineButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionDeclineButton")));
	const UButton* SecondaryButton = Cast<UButton>(
		Interaction->GetWidgetFromName(TEXT("InteractionNoLegalButton")));
	TestTrue(TEXT("Interaction primary secondary and decline styles are distinct"),
		PrimaryButton != nullptr && DeclineButton != nullptr
			&& SecondaryButton != nullptr
			&& !ButtonTint(PrimaryButton).Equals(ButtonTint(SecondaryButton))
			&& !ButtonTint(PrimaryButton).Equals(ButtonTint(DeclineButton)));
	Action.Category = EFMCodexUMGInteractionCategory::SelectOneOnOneShot;
	Action.bCanContinue = false;
	Action.bCanDecline = false;
	Action.bCanResolveNoLegal = false;
	Action.OneOnOneChoices = {
		{ EFMCodexUMGOneOnOneChoice::ChipShot, TEXT("CHIP SHOT") },
		{ EFMCodexUMGOneOnOneChoice::DirectShot, TEXT("DIRECT SHOT") }
	};
	Interaction->RefreshFromPresentation(Action);
	TestEqual(TEXT("OneOnOne choices retain styled typed options"),
		Interaction->GetRenderedOptionWidgets().Num(), 2);
	Action.KickerLabel = TEXT("SYSTEM RESOLUTION");
	Action.ExpectedActorLabel = TEXT("SYSTEM RESOLUTION");
	Interaction->RefreshFromPresentation(Action);
	TestTrue(TEXT("Interaction system mode has dedicated visual treatment"),
		Cast<UBorder>(Interaction->GetWidgetFromName(
			TEXT("InteractionActionHeader")))->GetBrushColor().Equals(
				Style.GetColor(EFMCodexPlayerUIColorRole::SystemStatus)));

	UFMCodexResolutionPanelWidget* Resolution =
		Screen->GetResolutionPanel();
	if (Resolution == nullptr)
	{
		return false;
	}
	Resolution->TakeWidget();
	FFMCodexUMGResolutionViewModel Result;
	Result.bVisible = true;
	Result.bTerminal = true;
	Result.StepLabel = TEXT("FINISHING FORMULA");
	Result.StepSummaryLabel = TEXT("Authoritative comparison accepted");
	Result.DiceResults = { { TEXT("ATTACK"), TEXT("Finishing roll"), 6 } };
	Result.ComparisonEvidence = {
		{ TEXT("ATTACK"), TEXT("11") },
		{ TEXT("DEFENSE"), TEXT("8") }
	};
	Result.DecisionLabel = TEXT("ATTACK WINS");
	Result.TerminalLabel = TEXT("GOAL");
	Resolution->RefreshFromPresentation(Result);
	TestTrue(TEXT("Resolution styles Dice Comparison Decision and terminal result"),
		Resolution->GetRenderedDiceWidgets().Num() == 1
			&& Resolution->GetRenderedComparisonCount() == 2
			&& Resolution->GetWidgetFromName(TEXT("ResolutionVersusLabel"))
				!= nullptr
			&& Resolution->GetWidgetFromName(TEXT("ResultIconAssetHook"))
				!= nullptr
			&& Cast<UBorder>(Resolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion")))->GetBrushColor().Equals(
					Style.GetColor(EFMCodexPlayerUIColorRole::Success)));
	UFMCodexDiceResultWidget* Die = Resolution->GetRenderedDiceWidgets()[0];
	Die->TakeWidget();
	TestTrue(TEXT("Dice reads as a bounded result object"),
		Die->GetWidgetFromName(TEXT("DiceFaceAssetHook")) != nullptr
			&& Cast<UTextBlock>(Die->GetWidgetFromName(
				TEXT("DiceRawD6Value")))->GetText().ToString() == TEXT("[ 6 ]"));
	for (const TCHAR* TerminalLabel : {
		TEXT("MISS"), TEXT("NO GOAL"), TEXT("IMMEDIATE MISS"),
		TEXT("OFFSIDE"), TEXT("OUT OF PLAY"),
		TEXT("DEFENDER STOPPED ATTACK") })
	{
		Result.TerminalLabel = TerminalLabel;
		Resolution->RefreshFromPresentation(Result);
		TestTrue(FString::Printf(TEXT("%s receives failure emphasis"),
			TerminalLabel),
			Cast<UBorder>(Resolution->GetWidgetFromName(
				TEXT("ResolutionTerminalRegion")))->GetBrushColor().Equals(
					Style.GetColor(EFMCodexPlayerUIColorRole::Danger)));
	}
	Result.bRejected = true;
	Result.DecisionLabel = TEXT("COMMAND REJECTED");
	Result.ErrorLabel = TEXT("Authoritative state unchanged");
	Resolution->RefreshFromPresentation(Result);
	TestTrue(TEXT("Rejected command is distinct from successful resolution"),
		Resolution->GetWidgetFromName(TEXT("ResolutionRejectedRegion"))
			->GetVisibility() == ESlateVisibility::Visible
			&& Resolution->GetWidgetFromName(
				TEXT("ResolutionAcceptedHierarchy"))->GetVisibility()
					== ESlateVisibility::Collapsed);

	const TArray<uint8> StateBeforeStyleRefresh =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	FFMCodexUMGMatchScreenViewModel RefreshedPresentation =
		Screen->GetPresentation();
	Screen->RefreshFromPresentation(RefreshedPresentation);
	TestTrue(TEXT("Style refresh has no obsolete handoff surface"),
		StateBeforeStyleRefresh == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Screen->GetInteractionPanel()->GetIsEnabled()
			&& Screen->GetWidgetFromName(TEXT("HotSeatHandoffText")) == nullptr
			&& Screen->GetWidgetFromName(TEXT("HotSeatReadyButton")) == nullptr);

	FString StyleHeader;
	FString StyleSource;
	FString RootSource;
	FString HeaderSource;
	FString PitchSource;
	FString SlotSource;
	FString CardSource;
	FString InteractionSource;
	FString OptionSource;
	FString ResolutionSource;
	FString DiceSource;
	FString ControllerSource;
	TestTrue(TEXT("Visual style boundary sources load"),
		LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIStyle.h"), StyleHeader)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIStyle.cpp"), StyleSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"), RootSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"), HeaderSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"), PitchSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"), SlotSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"), CardSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"), InteractionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionOptionWidget.cpp"), OptionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexResolutionPanelWidget.cpp"), ResolutionSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexDiceResultWidget.cpp"), DiceSource)
			&& LoadProductionSource(TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"), ControllerSource));
	const FString StyleSources = StyleHeader + StyleSource;
	const FString StyledWidgetSources = RootSource + HeaderSource + PitchSource
		+ SlotSource + CardSource + InteractionSource + OptionSource
		+ ResolutionSource + DiceSource;
	TestTrue(TEXT("Shared style owns no gameplay authority"),
		!StyleSources.Contains(TEXT("FMatchPlayState"))
			&& !StyleSources.Contains(TEXT("AuthoritativeSession"))
			&& !StyleSources.Contains(TEXT("HostGameMode"))
			&& !StyleSources.Contains(TEXT("D6Provider"))
			&& !StyleSources.Contains(TEXT("FormulaResolver"))
			&& !StyleSources.Contains(TEXT("Legality"))
			&& !StyleSources.Contains(TEXT("RouteThreshold"))
			&& !StyleSources.Contains(TEXT("RandRange"))
			&& !StyleSources.Contains(TEXT("RandomStream")));
	TestTrue(TEXT("Every major UMG structure consumes one shared visual style"),
		RootSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& HeaderSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& PitchSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& SlotSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& CardSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& InteractionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& OptionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& ResolutionSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& DiceSource.Contains(TEXT("FMCodexPlayerUIStyle.h")));
	TestTrue(TEXT("Asset integration hooks remain bounded to visual Widgets"),
		StyledWidgetSources.Contains(TEXT("CardFrameAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("PortraitPresentationRegion"))
			&& StyledWidgetSources.Contains(TEXT("SkillIconHook"))
			&& StyledWidgetSources.Contains(TEXT("RoleIconHook"))
			&& StyledWidgetSources.Contains(TEXT("BroadcastRegion"))
			&& StyledWidgetSources.Contains(TEXT("PitchBackgroundAssetHook"))
			&& StyledWidgetSources.Contains(TEXT("ResultIconAssetHook")));
	TestTrue(TEXT("Style introduces no generic dispatch or gameplay calculations"),
		!StyleSources.Contains(TEXT("ExecuteCommandByName"))
			&& !StyleSources.Contains(TEXT("CommandDispatcher"))
			&& !StyleSources.Contains(TEXT("ScoreResolver"))
			&& !StyleSources.Contains(TEXT("CurrentAttackingPlayer"))
			&& !StyleSources.Contains(TEXT("ExpectedActingPlayer")));
	TestTrue(TEXT("Typed Controller and Slate developer surface remain owned"),
		ControllerSource.Contains(TEXT("BuildControlSurface"))
			&& ControllerSource.Contains(TEXT("InitializeDeveloperSlateSurface"))
			&& ControllerSource.Contains(TEXT("SubmitCarrier"))
			&& ControllerSource.Contains(TEXT("ContinueResolution"))
			&& !ControllerSource.Contains(TEXT("ExecuteCommandByName")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexValidatedPlayerCardArtPilotIntegrationTest,
	"FMCodex.LocalPlay.ControlSurface.35.ValidatedPlayerCardArtPilotIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexValidatedPlayerCardArtPilotIntegrationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIAssetReferences& AssetReferences =
		FFMCodexPlayerUIAssetReferences::Get();
	const FFMCodexPlayerUICardArtReferences PilotArt =
		AssetReferences.ResolveCardArt(AssetReferences.GetPilotCardId());
	const FFMCodexPlayerUICardArtReferences MissingArt =
		AssetReferences.ResolveCardArt(TEXT("Demo.Missing.Cosmetic"));
	TestTrue(TEXT("Pilot catalog uses one deterministic presentation mapping"),
		AssetReferences.GetPilotCardId() == FName(TEXT("Demo.A.Outfield.01"))
			&& PilotArt.ArtIdentity == AssetReferences.GetPilotArtIdentity()
			&& !PilotArt.CardFrame.IsNull()
			&& !PilotArt.Portrait.IsNull()
			&& MissingArt.ArtIdentity.IsNone()
			&& MissingArt.CardFrame.IsNull()
			&& MissingArt.Portrait.IsNull());

	UTexture2D* FrameTexture = PilotArt.CardFrame.LoadSynchronous();
	UTexture2D* PortraitTexture = PilotArt.Portrait.LoadSynchronous();
	TestTrue(TEXT("Imported pilot packages load as substantial Texture2D assets"),
		FrameTexture != nullptr && PortraitTexture != nullptr
			&& FrameTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& PortraitTexture->GetImportedSize() == FIntPoint(1024, 1536));
	if (FrameTexture == nullptr || PortraitTexture == nullptr)
	{
		return false;
	}

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Pilot art integration Host exists"), Host);
	TestNotNull(TEXT("Pilot art integration Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Pilot art integration screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel Card;
	Card.CardId = AssetReferences.GetPilotCardId();
	Card.IdentityLabel = TEXT("Pilot Prototype Player");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW / MF");
	Card.RarityLabel = TEXT("Pilot");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	auto MakeCardWidget = [Screen, &Card](
		const EFMCodexPlayerCardPresentationMode Mode)
	{
		UFMCodexPlayerCardWidget* Widget =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		if (Widget != nullptr)
		{
			Widget->RefreshFromPresentation(Card, Mode);
			Widget->TakeWidget();
		}
		return Widget;
	};
	UFMCodexPlayerCardWidget* PitchCompactCard = MakeCardWidget(
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	UFMCodexPlayerCardWidget* InteractionChoiceCard = MakeCardWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestNotNull(TEXT("PitchCompact pilot Card constructs"), PitchCompactCard);
	TestNotNull(TEXT("InteractionChoice pilot Card constructs"),
		InteractionChoiceCard);
	if (PitchCompactCard == nullptr || InteractionChoiceCard == nullptr)
	{
		return false;
	}

	auto HasBoundPilotBrushes = [FrameTexture, PortraitTexture](
		const UFMCodexPlayerCardWidget& Widget)
	{
		const UImage* FrameImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("CardFrameAssetImage")));
		const UImage* PortraitImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("PortraitAssetImage")));
		return FrameImage != nullptr && PortraitImage != nullptr
			&& FrameImage->GetBrush().GetResourceObject() == FrameTexture
			&& PortraitImage->GetBrush().GetResourceObject() == PortraitTexture
			&& FrameImage->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& PortraitImage->GetVisibility()
				== ESlateVisibility::HitTestInvisible;
	};
	auto HasBoundPilotFullCard = [PortraitTexture](
		const UFMCodexPlayerCardWidget& Widget)
	{
		const UImage* FrameImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("CardFrameAssetImage")));
		const UImage* PortraitImage = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("PortraitAssetImage")));
		return FrameImage != nullptr && PortraitImage != nullptr
			&& FrameImage->GetVisibility() == ESlateVisibility::Collapsed
			&& PortraitImage->GetBrush().GetResourceObject() == PortraitTexture
			&& PortraitImage->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& Widget.GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible;
	};
	TestTrue(TEXT("PitchCompact binds real frame and portrait brush resources"),
		HasBoundPilotBrushes(*PitchCompactCard)
			&& PitchCompactCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchCompact);
	TestTrue(TEXT("InteractionChoice uses portrait art with production frame"),
		HasBoundPilotFullCard(*InteractionChoiceCard)
			&& InteractionChoiceCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& PitchCompactCard->GetResolvedArtIdentity()
				== InteractionChoiceCard->GetResolvedArtIdentity()
			&& PitchCompactCard->GetResolvedCardFrameTexture()
				== InteractionChoiceCard->GetResolvedCardFrameTexture()
			&& PitchCompactCard->GetResolvedPortraitTexture()
				== InteractionChoiceCard->GetResolvedPortraitTexture());
	TestTrue(TEXT("Pilot art supplements and preserves card semantics"),
		PitchCompactCard->GetPresentation().CardId == Card.CardId
			&& PitchCompactCard->GetRenderedSkillCount() == 5
			&& PitchCompactCard->GetRenderedAttributeCount() == 4
			&& PitchCompactCard->GetRenderedStatusBadgeCount() == 1
			&& InteractionChoiceCard->GetRenderedSkillCount() == 5
			&& InteractionChoiceCard->GetRenderedAttributeCount() == 10
			&& InteractionChoiceCard->GetRenderedStatusBadgeCount() == 0);

	FFMCodexUMGCardViewModel MissingCard = Card;
	MissingCard.CardId = TEXT("Demo.Missing.Cosmetic");
	UFMCodexPlayerCardWidget* FallbackCard =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	if (FallbackCard == nullptr)
	{
		return false;
	}
	FallbackCard->RefreshFromPresentation(
		MissingCard, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	FallbackCard->TakeWidget();
	const UImage* MissingFrameImage = Cast<UImage>(
		FallbackCard->GetWidgetFromName(TEXT("CardFrameAssetImage")));
	const UImage* MissingPortraitImage = Cast<UImage>(
		FallbackCard->GetWidgetFromName(TEXT("PortraitAssetImage")));
	TestTrue(TEXT("Missing frame safely restores standard style surface"),
		FallbackCard->GetResolvedCardFrameTexture() == nullptr
			&& MissingFrameImage != nullptr
			&& MissingFrameImage->GetVisibility() == ESlateVisibility::Collapsed
			&& FallbackCard->GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("Missing portrait cleanly omits development placeholder"),
		FallbackCard->GetResolvedPortraitTexture() == nullptr
			&& MissingPortraitImage != nullptr
			&& MissingPortraitImage->GetVisibility()
				== ESlateVisibility::Collapsed
			&& FallbackCard->GetWidgetFromName(TEXT("PortraitPlaceholderText"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& FallbackCard->GetPresentation().CardId == MissingCard.CardId);

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		Region.RegionLabel = RegionIndex == 0
			? TEXT("PLAYER B HALF") : TEXT("PLAYER A HALF");
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Pilot.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.SlotLabel = FString::Printf(TEXT("POSITION %d"), SlotIndex + 1);
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied)
			{
				Slot.Card = Card;
			}
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	const TArray<uint8> StateBeforePresentation =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	Pitch->RefreshFromPitchPresentation(PitchPresentation);
	TestTrue(TEXT("Pilot art preserves all 10 canonical Pitch slots"),
		Pitch->GetRenderedSlotWidgets().Num() == 10);
	UFMCodexPitchSlotWidget* PilotSlot =
		Pitch->GetRenderedSlotWidgets().IsEmpty()
			? nullptr : Pitch->GetRenderedSlotWidgets()[0];
	if (PilotSlot != nullptr)
	{
		PilotSlot->TakeWidget();
	}
	UFMCodexPlayerCardWidget* PilotPitchCard = PilotSlot == nullptr
		? nullptr : PilotSlot->GetCardWidget();
	TestTrue(TEXT("Pitch slot uses the centralized pilot art identity"),
		PilotPitchCard != nullptr
			&& PilotPitchCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::PitchMini
			&& PilotPitchCard->GetResolvedArtIdentity()
				== InteractionChoiceCard->GetResolvedArtIdentity()
			&& HasBoundPilotBrushes(*PilotPitchCard));
	TestTrue(TEXT("Cosmetic Pitch enrichment cannot mutate authority"),
		StateBeforePresentation
			== SerializeState(Host->GetMatchSnapshot().Snapshot));

	Screen->RequestStartNewMatch();
	const TArray<uint8> StateBeforeRejected =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	Screen->RequestDeployOrdinary(
		AssetReferences.GetPilotCardId(), NAME_None);
	TestTrue(TEXT("Rejected typed command preserves State and pilot art"),
		!Controller->GetLastDiagnostic().bHostSuccess
			&& StateBeforeRejected
				== SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& HasBoundPilotFullCard(*InteractionChoiceCard)
			&& InteractionChoiceCard->GetPresentation().CardId == Card.CardId);

	FString AssetReferenceHeader;
	FString AssetReferenceSource;
	FString CardSource;
	FString AuthoritySources;
	FString SourcePart;
	TestTrue(TEXT("Pilot presentation boundary sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.h"),
			AssetReferenceHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp"),
				AssetReferenceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource));
	for (const TCHAR* Path : {
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.h"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.h"),
		TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchD6Provider.cpp"),
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.h"),
		TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
		TEXT("Source/FMCodex/CoreRules/MatchPlayState.h") })
	{
		if (LoadProductionSource(Path, SourcePart))
		{
			AuthoritySources += SourcePart;
		}
		else
		{
			AddError(FString::Printf(TEXT("Could not audit %s"), Path));
		}
	}
	TestTrue(TEXT("Soft asset paths are centralized in UI presentation"),
		AssetReferenceHeader.Contains(TEXT("TSoftObjectPtr<UTexture2D>"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Cards/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Portraits/"))
			&& !CardSource.Contains(TEXT("/Game/UI/")));
	TestTrue(TEXT("Authority State Host Session and D6 provider have zero UI refs"),
		!AuthoritySources.Contains(TEXT("/Game/UI/"))
			&& !AuthoritySources.Contains(TEXT("TSoftObjectPtr<UTexture2D>"))
			&& !AuthoritySources.Contains(TEXT("LoadObject<UTexture2D>"))
			&& !AuthoritySources.Contains(TEXT("LoadSynchronous")));
	TestTrue(TEXT("Card asset layer stays presentation-only"),
		!CardSource.Contains(TEXT("FMatchPlayState"))
			&& !CardSource.Contains(TEXT("AuthoritativeSession"))
			&& !CardSource.Contains(TEXT("D6Provider"))
			&& !CardSource.Contains(TEXT("FormulaResolver"))
			&& !CardSource.Contains(TEXT("Legality"))
			&& CardSource.Contains(TEXT("FMCodexPlayerUIStyle.h"))
			&& CardSource.Contains(TEXT("CardFrameAssetHook"))
			&& CardSource.Contains(TEXT("PortraitPresentationRegion")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexGoldenSampleVisualDirectionTest,
	"FMCodex.LocalPlay.ControlSurface.36.GoldenSampleVisualDirection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexGoldenSampleVisualDirectionTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	const FFMCodexPlayerUIAssetReferences& References =
		FFMCodexPlayerUIAssetReferences::Get();
	const FFMCodexPlayerUICardArtReferences Golden =
		References.ResolveCardArt(References.GetGoldenSampleCardId());
	const FFMCodexPlayerUICardArtReferences Missing =
		References.ResolveCardArt(TEXT("Demo.Missing.GoldenSample"));
	TestTrue(TEXT("Golden Sample uses one deterministic presentation identity"),
		References.GetGoldenSampleCardId() == FName(TEXT("Demo.A.Outfield.02"))
			&& References.GetGoldenSampleArtIdentity()
				== FName(TEXT("GoldenSample.PlayerCard.01"))
			&& Golden.ArtIdentity == References.GetGoldenSampleArtIdentity()
			&& !Golden.CardFrame.IsNull()
			&& !Golden.Portrait.IsNull()
			&& !Golden.RoleIcon.IsNull()
			&& !Golden.LongShotSkillIcon.IsNull());
	TestTrue(TEXT("Unknown CardId has all four cosmetic fallbacks"),
		Missing.ArtIdentity.IsNone()
			&& Missing.CardFrame.IsNull()
			&& Missing.Portrait.IsNull()
			&& Missing.RoleIcon.IsNull()
			&& Missing.LongShotSkillIcon.IsNull());

	UTexture2D* FrameTexture = Golden.CardFrame.LoadSynchronous();
	UTexture2D* PortraitTexture = Golden.Portrait.LoadSynchronous();
	UTexture2D* RoleIconTexture = Golden.RoleIcon.LoadSynchronous();
	UTexture2D* SkillIconTexture = Golden.LongShotSkillIcon.LoadSynchronous();
	TestTrue(TEXT("All four Golden Sample packages load with expected dimensions"),
		FrameTexture != nullptr && PortraitTexture != nullptr
			&& RoleIconTexture != nullptr && SkillIconTexture != nullptr
			&& FrameTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& PortraitTexture->GetImportedSize() == FIntPoint(1024, 1536)
			&& RoleIconTexture->GetImportedSize() == FIntPoint(1254, 1254)
			&& SkillIconTexture->GetImportedSize() == FIntPoint(1254, 1254)
			&& !RoleIconTexture->CompressionNoAlpha
			&& !SkillIconTexture->CompressionNoAlpha);
	if (FrameTexture == nullptr || PortraitTexture == nullptr
		|| RoleIconTexture == nullptr || SkillIconTexture == nullptr)
	{
		return false;
	}

	TestEqual(TEXT("Forward role has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Role(TEXT("FW / MF")).ToString(),
		FString(TEXT("前锋 / 中场")));
	TestEqual(TEXT("Long Shot has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Skill(TEXT("Long Shot")).ToString(),
		FString(TEXT("远射")));
	TestEqual(TEXT("Attribute has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Attribute(TEXT("SHO 5")).ToString(),
		FString(TEXT("射门 5")));
	TestEqual(TEXT("Status has centralized zh-CN presentation"),
		FFMCodexPlayerUIPresentationText::Status(TEXT("AVAILABLE")).ToString(),
		FString(TEXT("可用")));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.GetController();
	TestNotNull(TEXT("Golden Sample Host exists"), Host);
	TestNotNull(TEXT("Golden Sample Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen = Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Golden Sample screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();

	FFMCodexUMGCardViewModel Card;
	Card.CardId = References.GetGoldenSampleCardId();
	Card.IdentityLabel = TEXT("Golden Sample Forward");
	Card.OwnerLabel = TEXT("Player A");
	Card.RoleLabel = TEXT("FW / MF");
	Card.RarityLabel = TEXT("Pilot");
	Card.SkillLabels = { TEXT("Long Shot"), TEXT("Cut Inside"),
		TEXT("Pass Control"), TEXT("Cross"), TEXT("Through Ball") };
	Card.SkillSummaryLabel = FString::Join(Card.SkillLabels, TEXT(" | "));
	Card.CompactAttributeSummary =
		TEXT("SHO 5 | PAS 4 | DRI 3 | SPD 4");
	Card.FullAttributeSummary =
		TEXT("SHO 5 | DRI 3 | PAS 4 | OFF 4 | MRK 2 | TKL 1 | SPD 4 | STR 3 | STA 4 | LS 5");
	Card.StatusLabels = { TEXT("AVAILABLE") };
	Card.StatusSummaryLabel = TEXT("AVAILABLE");

	auto MakeWidget = [Screen, &Card](
		const EFMCodexPlayerCardPresentationMode Mode,
		const FFMCodexUMGCardViewModel* Override = nullptr)
	{
		UFMCodexPlayerCardWidget* Widget =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		if (Widget != nullptr)
		{
			Widget->RefreshFromPresentation(
				Override == nullptr ? Card : *Override, Mode);
			Widget->TakeWidget();
		}
		return Widget;
	};
	const TArray<uint8> StateBeforeWidgets =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	UFMCodexPlayerCardWidget* PitchCompact = MakeWidget(
		EFMCodexPlayerCardPresentationMode::PitchCompact);
	UFMCodexPlayerCardWidget* InteractionChoice = MakeWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice);
	TestNotNull(TEXT("Golden Sample PitchCompact constructs"), PitchCompact);
	TestNotNull(TEXT("Golden Sample InteractionChoice constructs"),
		InteractionChoice);
	if (PitchCompact == nullptr || InteractionChoice == nullptr)
	{
		return false;
	}

	auto HasFourBrushes = [FrameTexture, PortraitTexture, RoleIconTexture,
		SkillIconTexture](const UFMCodexPlayerCardWidget& Widget)
	{
		const UImage* Frame = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("CardFrameAssetImage")));
		const UImage* Portrait = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("PortraitAssetImage")));
		const UImage* RoleIcon = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("RoleIconAssetImage")));
		const UImage* SkillIcon = Cast<UImage>(
			Widget.GetWidgetFromName(TEXT("SkillIconAssetImage0")));
		return Frame != nullptr && Portrait != nullptr && RoleIcon != nullptr
			&& SkillIcon != nullptr
			&& Frame->GetBrush().GetResourceObject() == FrameTexture
			&& Portrait->GetBrush().GetResourceObject() == PortraitTexture
			&& RoleIcon->GetBrush().GetResourceObject() == RoleIconTexture
			&& SkillIcon->GetBrush().GetResourceObject() == SkillIconTexture
			&& Frame->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Portrait->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& RoleIcon->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& SkillIcon->GetVisibility() == ESlateVisibility::HitTestInvisible;
	};
	TestTrue(TEXT("PitchCompact binds all four Golden Sample resources"),
		HasFourBrushes(*PitchCompact)
			&& PitchCompact->GetRenderedSkillCount() == 5
			&& PitchCompact->GetRenderedAttributeCount() == 4);
	const UImage* FullCardFrame = Cast<UImage>(
		InteractionChoice->GetWidgetFromName(TEXT("CardFrameAssetImage")));
	const UImage* FullCardPortrait = Cast<UImage>(
		InteractionChoice->GetWidgetFromName(TEXT("PortraitAssetImage")));
	const UImage* FullCardRoleIcon = Cast<UImage>(
		InteractionChoice->GetWidgetFromName(TEXT("RoleIconAssetImage")));
	TestTrue(TEXT("InteractionChoice isolates production Full Card art usage"),
		FullCardFrame != nullptr
			&& FullCardFrame->GetVisibility() == ESlateVisibility::Collapsed
			&& FullCardPortrait != nullptr
			&& FullCardPortrait->GetBrush().GetResourceObject() == PortraitTexture
			&& FullCardPortrait->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& FullCardRoleIcon != nullptr
			&& FullCardRoleIcon->GetVisibility() == ESlateVisibility::Collapsed
			&& InteractionChoice->GetWidgetFromName(
				TEXT("SkillIconAssetImage0")) == nullptr
			&& InteractionChoice->GetRenderedSkillCount() == 5
			&& InteractionChoice->GetRenderedAttributeCount() == 10
			&& InteractionChoice->GetRenderedStatusBadgeCount() == 0);
	TestTrue(TEXT("Golden Sample uses the same art identity in both modes"),
		PitchCompact->GetResolvedArtIdentity()
			== References.GetGoldenSampleArtIdentity()
			&& PitchCompact->GetResolvedArtIdentity()
				== InteractionChoice->GetResolvedArtIdentity()
			&& PitchCompact->GetResolvedRoleIconTexture()
				== RoleIconTexture
			&& InteractionChoice->GetResolvedLongShotSkillIconTexture()
				== SkillIconTexture);

	const UTextBlock* RoleText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("CardRole")));
	const UTextBlock* SkillText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("SkillIdentity0")));
	const UTextBlock* AttributeText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("AttributeValue0")));
	const UTextBlock* AttributeLabel = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("AttributeLabel0")));
	const UTextBlock* StatusText = Cast<UTextBlock>(
		InteractionChoice->GetWidgetFromName(TEXT("StatusBadgeLabel0")));
	TestTrue(TEXT("Golden Sample renders production role Skill and attribute data"),
		RoleText != nullptr && RoleText->GetText().ToString() == TEXT("A/M")
			&& SkillText != nullptr && SkillText->GetText().ToString().Contains(TEXT("远射"))
			&& AttributeLabel != nullptr
			&& AttributeLabel->GetText().ToString() == TEXT("射门")
			&& AttributeText != nullptr
			&& AttributeText->GetText().ToString() == TEXT("5")
			&& StatusText == nullptr);
	TestTrue(TEXT("UMG-only Golden Sample refresh cannot mutate authority"),
		StateBeforeWidgets == SerializeState(Host->GetMatchSnapshot().Snapshot));

	FFMCodexUMGCardViewModel MissingCard = Card;
	MissingCard.CardId = TEXT("Demo.Missing.GoldenSample");
	UFMCodexPlayerCardWidget* Fallback = MakeWidget(
		EFMCodexPlayerCardPresentationMode::InteractionChoice, &MissingCard);
	if (Fallback == nullptr)
	{
		return false;
	}
	const UImage* MissingFrame = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("CardFrameAssetImage")));
	const UImage* MissingPortrait = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("PortraitAssetImage")));
	const UImage* MissingRole = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("RoleIconAssetImage")));
	const UImage* MissingSkill = Cast<UImage>(
		Fallback->GetWidgetFromName(TEXT("SkillIconAssetImage0")));
	const UTextBlock* MissingRoleText = Cast<UTextBlock>(
		Fallback->GetWidgetFromName(TEXT("CardRole")));
	const UTextBlock* MissingSkillText = Cast<UTextBlock>(
		Fallback->GetWidgetFromName(TEXT("SkillIdentity0")));
	TestTrue(TEXT("Frame fallback remains visible and portrait omission stays clean"),
		MissingFrame != nullptr
			&& MissingFrame->GetVisibility() == ESlateVisibility::Collapsed
			&& MissingPortrait != nullptr
			&& MissingPortrait->GetVisibility() == ESlateVisibility::Collapsed
			&& Fallback->GetWidgetFromName(TEXT("CardFrameFallbackSurface"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& Fallback->GetWidgetFromName(TEXT("PortraitPlaceholderText"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	TestTrue(TEXT("Missing icons preserve compact Position and Chinese Skill text"),
		MissingRole != nullptr
			&& MissingRole->GetVisibility() == ESlateVisibility::Collapsed
			&& MissingSkill == nullptr
			&& MissingRoleText != nullptr
			&& MissingRoleText->GetText().ToString() == TEXT("A/M")
			&& MissingSkillText != nullptr
			&& MissingSkillText->GetText().ToString().Contains(TEXT("远射")));

	TArray<FFMCodexUMGPitchRegionViewModel> PitchPresentation;
	for (int32 RegionIndex = 0; RegionIndex < 2; ++RegionIndex)
	{
		FFMCodexUMGPitchRegionViewModel Region;
		for (int32 SlotIndex = 0; SlotIndex < 5; ++SlotIndex)
		{
			FFMCodexUMGPitchSlotViewModel Slot;
			Slot.SlotId = FName(*FString::Printf(
				TEXT("Golden.Slot.%d.%d"), RegionIndex, SlotIndex));
			Slot.bOccupied = RegionIndex == 0 && SlotIndex == 0;
			if (Slot.bOccupied) Slot.Card = Card;
			Region.Slots.Add(Slot);
		}
		PitchPresentation.Add(Region);
	}
	Screen->GetPitchWidget()->RefreshFromPitchPresentation(PitchPresentation);
	TestEqual(TEXT("Golden Sample preserves the 10-slot Pitch hierarchy"),
		Screen->GetPitchWidget()->GetRenderedSlotWidgets().Num(), 10);

	FString ArtDirection;
	FString AssetReferenceSource;
	FString CardSource;
	FString TextSource;
	TestTrue(TEXT("Golden Sample visual contracts are checked in"),
		LoadProductionSource(TEXT("Docs/Visual/Visual_Art_Direction_v1.md"),
			ArtDirection)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp"),
				AssetReferenceSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIPresentationText.cpp"),
				TextSource));
	TestTrue(TEXT("Art direction freezes four language-neutral prompt specs"),
		ArtDirection.Contains(TEXT("Exact prompt — Card Frame"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Player Portrait"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Forward Role Icon"))
			&& ArtDirection.Contains(TEXT("Exact prompt — Long Shot Skill Icon"))
			&& ArtDirection.Contains(TEXT("no readable text")));
	TestTrue(TEXT("Golden asset paths stay centralized and presentation-only"),
		AssetReferenceSource.Contains(TEXT("/Game/UI/Cards/GoldenSample/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Portraits/GoldenSample/"))
			&& AssetReferenceSource.Contains(TEXT("/Game/UI/Icons/GoldenSample/"))
			&& !CardSource.Contains(TEXT("/Game/UI/"))
			&& TextSource.Contains(TEXT("LOCTEXT"))
			&& !TextSource.Contains(TEXT("FMatchPlayState"))
			&& !TextSource.Contains(TEXT("AuthoritativeSession"))
			&& !TextSource.Contains(TEXT("D6Provider")));
	for (const TCHAR* RelativePath : {
		TEXT("ArtSource/UI/GoldenSample/Cards/T_Golden_CardFrame_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Portraits/T_Golden_PlayerPortrait_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Icons/T_Golden_Role_Forward_01.png"),
		TEXT("ArtSource/UI/GoldenSample/Icons/T_Golden_Skill_LongShot_01.png") })
	{
		TestTrue(FString::Printf(TEXT("Golden source exists: %s"), RelativePath),
			FPaths::FileExists(FPaths::Combine(FPaths::ProjectDir(), RelativePath)));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexGoldenLayoutPrototypeTest,
	"FMCodex.LocalPlay.ControlSurface.38.GoldenLayoutPrototype",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexGoldenLayoutPrototypeTest::RunTest(const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	auto* Host = PlayableWorld.GetHost();
	auto* Controller = PlayableWorld.GetController();
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	auto* Screen = Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	for (const TCHAR* Region : {
		TEXT("BroadcastMatchHeaderRegion"), TEXT("LocalPlayerCardRackRegion"),
		TEXT("CentralPitchRegion"), TEXT("OpponentCardRackRegion"),
		TEXT("ContextActionDockRegion"), TEXT("ResolutionPresentationLayer") })
	{
		TestNotNull(FString::Printf(TEXT("Golden macro region %s exists"), Region),
			Screen->GetWidgetFromName(Region));
	}
	auto* HeaderBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("BroadcastMatchHeaderRegion")));
	auto* MainBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("GoldenLayoutMainMatchArea")));
	auto* DockBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("ContextActionDockRegion")));
	auto* LocalBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("LocalPlayerCardRackRegion")));
	auto* PitchBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("CentralPitchRegion")));
	auto* OpponentBounds = Cast<USizeBox>(Screen->GetWidgetFromName(
		TEXT("OpponentCardRackRegion")));
	TestTrue(TEXT("1920x1080 working proportions are explicit and bounded"),
		HeaderBounds != nullptr && MainBounds != nullptr && DockBounds != nullptr
			&& LocalBounds != nullptr && PitchBounds != nullptr
			&& OpponentBounds != nullptr
			&& FMath::IsNearlyEqual(HeaderBounds->GetHeightOverride(), 80.0f)
			&& FMath::IsNearlyEqual(MainBounds->GetHeightOverride(), 880.0f)
			&& FMath::IsNearlyEqual(DockBounds->GetHeightOverride(), 120.0f)
			&& FMath::IsNearlyEqual(LocalBounds->GetWidthOverride(), 476.0f, 8.0f)
			&& FMath::IsNearlyEqual(PitchBounds->GetWidthOverride(), 968.0f, 16.0f)
			&& FMath::IsNearlyEqual(OpponentBounds->GetWidthOverride(), 476.0f, 8.0f)
			&& FMath::IsNearlyEqual(
				1920.0f - LocalBounds->GetWidthOverride()
					- OpponentBounds->GetWidthOverride(), 968.0f, 16.0f));
	TestTrue(TEXT("Idle resolution layer is non-space-reserving"),
		Screen->GetWidgetFromName(TEXT("ResolutionPresentationLayer"))
			->GetVisibility() == ESlateVisibility::Collapsed);

	Screen->RequestStartNewMatch();
	TestTrue(TEXT("Start success keeps the non-resolution layer collapsed"),
		!Screen->GetPresentation().Resolution.bVisible
			&& Screen->GetWidgetFromName(TEXT("ResolutionPresentationLayer"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	Screen->RequestRollTacticalPoints();

	const FFMCodexUMGMatchScreenViewModel PlayerAView =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(),
			FFMCodexLocalMatchResolutionFeedback(), FString(),
			EInitialTurnOrderPlayer::PlayerA);
	const FFMCodexUMGMatchScreenViewModel PlayerBView =
		FFMCodexLocalMatchUMGPresentationBuilder::Build(
			Controller->GetInteractionView(),
			FFMCodexLocalMatchResolutionFeedback(), FString(),
			EInitialTurnOrderPlayer::PlayerB);
	TestTrue(TEXT("Local-left orientation reverses presentation only"),
		PlayerAView.LocalRack.SideLabel == TEXT("Player A")
			&& PlayerAView.OpponentRack.SideLabel == TEXT("Player B")
			&& PlayerAView.PitchRegions.Num() == 2
			&& PlayerAView.PitchRegions[0].PhysicalHalfLabel.Contains(TEXT("A"))
			&& PlayerBView.LocalRack.SideLabel == TEXT("Player B")
			&& PlayerBView.OpponentRack.SideLabel == TEXT("Player A")
			&& PlayerBView.PitchRegions.Num() == 2
			&& PlayerBView.PitchRegions[0].PhysicalHalfLabel.Contains(TEXT("B")));
	TestTrue(TEXT("Both public racks are full stable row-major 2x10 grids"),
		PlayerAView.LocalRack.Cells.Num() == 20
			&& PlayerAView.OpponentRack.Cells.Num() == 20
			&& PlayerAView.LocalRack.ColumnCount == 2
			&& PlayerAView.LocalRack.RowCount == 10
			&& PlayerAView.OpponentRack.ColumnCount == 2
			&& PlayerAView.OpponentRack.RowCount == 10);
	const UUniformGridPanel* LocalGrid = Cast<UUniformGridPanel>(
		Screen->GetLocalRackWidget()->GetWidgetFromName(
			TEXT("StableTwoByTenCardRackGrid")));
	TestTrue(TEXT("Approved rack geometry fits without reflow or macro-layout change"),
		LocalGrid != nullptr
			&& LocalGrid->GetSlotPadding() == FMargin(6.0f, 4.0f)
			&& Screen->GetLocalRackWidget()->GetRenderedCardWidgets().Num() == 20
			&& Screen->GetLocalRackWidget()->GetRenderedCardWidgets()[0]
				->GetConfiguredDimensions().Equals(FVector2D(220.0f, 68.0f))
			&& FMath::IsNearlyEqual(2.0f * 220.0f + 12.0f, 452.0f)
			&& FMath::IsNearlyEqual(10.0f * 68.0f + 9.0f * 8.0f, 752.0f)
			&& 452.0f <= 476.0f - 12.0f
			&& 752.0f <= 880.0f - 8.0f);
	const UCanvasPanel* PitchCanvas = Cast<UCanvasPanel>(
		Screen->GetPitchWidget()->GetWidgetFromName(TEXT("TwoLanePitchCanvas")));
	TArray<float> LaneCenters;
	if (PitchCanvas != nullptr)
	{
		for (int32 Index = 0; Index < PitchCanvas->GetChildrenCount(); ++Index)
		{
			const UWidget* Child = PitchCanvas->GetChildAt(Index);
			if (Child != nullptr
				&& (Child->GetName() == TEXT("PlayerAPhysicalHalf")
					|| Child->GetName() == TEXT("PlayerBPhysicalHalf")))
			{
				const UCanvasPanelSlot* LaneSlot = Cast<UCanvasPanelSlot>(Child->Slot);
				if (LaneSlot != nullptr)
				{
					LaneCenters.Add(LaneSlot->GetAnchors().Minimum.X);
				}
			}
		}
	}
	LaneCenters.Sort();
	TestTrue(TEXT("Reduced pitch keeps bounded symmetric five-card lanes"),
		LaneCenters.Num() == 2
			&& FMath::IsNearlyEqual(LaneCenters[0], 0.33f, 0.01f)
			&& FMath::IsNearlyEqual(LaneCenters[1], 0.67f, 0.01f));
	bool bSorted = true;
	for (int32 Index = 1; Index < PlayerAView.LocalRack.Cells.Num(); ++Index)
	{
		const FName Previous = PlayerAView.LocalRack.Cells[Index - 1].Card.CardId;
		const FName Current = PlayerAView.LocalRack.Cells[Index].Card.CardId;
		const auto& Roster = Controller->GetInteractionView().PlayerACardRoster;
		const auto* PreviousCard = Roster.FindByPredicate(
			[Previous](const FFMCodexLocalMatchCardView& Card)
			{ return Card.CardId == Previous; });
		const auto* CurrentCard = Roster.FindByPredicate(
			[Current](const FFMCodexLocalMatchCardView& Card)
			{ return Card.CardId == Current; });
		bSorted = bSorted && PreviousCard != nullptr && CurrentCard != nullptr
			&& PreviousCard->RackSortGroup <= CurrentCard->RackSortGroup;
	}
	TestTrue(TEXT("Initial rack grouping is GK then D then M then A"), bSorted);

	const FFMCodexUMGMatchScreenViewModel& ActingView =
		Controller->GetInteractionView().ExpectedActingPlayer
			== EInitialTurnOrderPlayer::PlayerB ? PlayerBView : PlayerAView;
	int32 DraggableCount = 0;
	int32 VisibleNonChoiceCount = 0;
	for (const FFMCodexUMGCardRackCellViewModel& Cell : ActingView.LocalRack.Cells)
	{
		DraggableCount += Cell.bDeploymentDraggable ? 1 : 0;
		VisibleNonChoiceCount += !Cell.bDeploymentDraggable ? 1 : 0;
	}
	TestTrue(TEXT("Full local rack exposes legality without inventing it"),
		DraggableCount == ActingView.Interaction.DeploymentChoices.Num()
			&& DraggableCount > 0 && VisibleNonChoiceCount > 0);
	TestTrue(TEXT("Opponent rack cannot emit local deployment drag"),
		!ActingView.OpponentRack.Cells.ContainsByPredicate(
			[](const FFMCodexUMGCardRackCellViewModel& Cell)
			{ return Cell.bDeploymentDraggable; }));
	const auto& LocalCardWidgets =
		Screen->GetLocalRackWidget()->GetRenderedCardWidgets();
	const auto& OpponentCardWidgets =
		Screen->GetOpponentRackWidget()->GetRenderedCardWidgets();
	int32 EnabledLocalDrags = 0;
	int32 DisabledLocalDrags = 0;
	for (const UFMCodexPlayerCardWidget* Card : LocalCardWidgets)
	{
		EnabledLocalDrags += Card != nullptr && Card->IsDeploymentDragEnabled();
		DisabledLocalDrags += Card != nullptr && !Card->IsDeploymentDragEnabled();
	}
	TestTrue(TEXT("Rendered racks preserve legal-only local actionability"),
		EnabledLocalDrags == DraggableCount
			&& EnabledLocalDrags > 0 && DisabledLocalDrags > 0
			&& !OpponentCardWidgets.ContainsByPredicate(
				[](const TObjectPtr<UFMCodexPlayerCardWidget>& Card)
				{ return Card != nullptr && Card->IsDeploymentDragEnabled(); }));

	const auto* Ordinary = Controller->GetInteractionView().DeploymentGroups
		.FindByPredicate([](const FFMCodexLocalMatchDeploymentGroup& Group)
		{
			return !Group.bGoalkeeper && !Group.LegalSlots.IsEmpty();
		});
	if (Ordinary == nullptr)
	{
		return false;
	}
	const FName PlayedCardId = Ordinary->CardId;
	int32 StableIndex = INDEX_NONE;
	for (const FFMCodexUMGCardRackCellViewModel& Cell : ActingView.LocalRack.Cells)
	{
		if (Cell.Card.CardId == PlayedCardId)
		{
			StableIndex = Cell.StableIndex;
			break;
		}
	}
	Screen->RequestDeployOrdinary(PlayedCardId, Ordinary->LegalSlots[0].SlotId);
	const auto& AfterLocalRack = Screen->GetPresentation().LocalRack;
	const auto& AfterOpponentRack = Screen->GetPresentation().OpponentRack;
	const FFMCodexUMGCardRackCellViewModel* PlayedCell =
		AfterLocalRack.Cells.FindByPredicate(
			[PlayedCardId](const FFMCodexUMGCardRackCellViewModel& Cell)
			{ return Cell.Card.CardId == PlayedCardId; });
	UFMCodexCardRackWidget* GhostRack = Screen->GetLocalRackWidget();
	if (PlayedCell == nullptr)
	{
		PlayedCell = AfterOpponentRack.Cells.FindByPredicate(
			[PlayedCardId](const FFMCodexUMGCardRackCellViewModel& Cell)
			{ return Cell.Card.CardId == PlayedCardId; });
		GhostRack = Screen->GetOpponentRackWidget();
	}
	TestTrue(TEXT("Played card leaves an in-place ghost without reflow"),
		PlayedCell != nullptr
			&& PlayedCell->bPlayed
			&& PlayedCell->StableIndex == StableIndex
			&& Screen->GetLocalRackWidget()->GetRenderedCellCount() == 20
			&& Screen->GetOpponentRackWidget()->GetRenderedCellCount() == 20);
	USizeBox* GhostBounds = Cast<USizeBox>(GhostRack->GetWidgetFromName(FName(*FString::Printf(
		TEXT("PlayedCardGhostBounds%d"), StableIndex))));
	UBorder* GhostCell = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(TEXT("PlayedCardGhostCell%d"), StableIndex))));
	USizeBox* GhostPortraitBounds = Cast<USizeBox>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostPortraitBounds%d"), StableIndex))));
	UBorder* GhostPortrait = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostPortraitShape%d"), StableIndex))));
	USizeBox* GhostDividerBounds = Cast<USizeBox>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostDividerBounds%d"), StableIndex))));
	UBorder* GhostDivider = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(TEXT("PlayedCardGhostDivider%d"), StableIndex))));
	UBorder* GhostIdentity = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostIdentityShape%d"), StableIndex))));
	USizeBox* GhostTrailingRailBounds = Cast<USizeBox>(
		GhostRack->GetWidgetFromName(FName(*FString::Printf(
			TEXT("PlayedCardGhostTrailingRailBounds%d"), StableIndex))));
	UBorder* GhostTrailingRail = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostTrailingRail%d"), StableIndex))));
	UBorder* GhostTopFrame = Cast<UBorder>(GhostRack->GetWidgetFromName(
		FName(*FString::Printf(
			TEXT("PlayedCardGhostFrameTop%d"), StableIndex))));
	TestTrue(TEXT("Played cell is a text-free low-contrast Ghost Frame"),
		GhostBounds != nullptr
			&& GhostBounds->GetWidthOverride() == 220.0f
			&& GhostBounds->GetHeightOverride() == 68.0f
			&& GhostCell != nullptr && GhostCell->GetRenderOpacity() == 1.0f
			&& GhostCell->GetBrushColor().ToFColorSRGB()
				== FColor(0x17, 0x30, 0x3B)
			&& GhostRack->GetWidgetFromName(FName(*FString::Printf(
				TEXT("PlayedCardGhostStructure%d"), StableIndex))) != nullptr
			&& GhostPortraitBounds != nullptr
			&& GhostPortraitBounds->GetWidthOverride() == 96.0f
			&& GhostPortraitBounds->GetHeightOverride() == 68.0f
			&& GhostPortrait != nullptr
			&& GhostPortrait->GetRenderOpacity() == 1.0f
			&& GhostDividerBounds != nullptr
			&& GhostDividerBounds->GetWidthOverride() == 1.0f
			&& GhostDivider != nullptr
			&& GhostDivider->GetRenderOpacity() == 0.10f
			&& GhostIdentity != nullptr
			&& GhostIdentity->GetRenderOpacity() == 1.0f
			&& GhostTrailingRailBounds != nullptr
			&& GhostTrailingRailBounds->GetWidthOverride() == 4.0f
			&& GhostTrailingRail != nullptr
			&& GhostTrailingRail->GetRenderOpacity() == 1.0f
			&& GhostTrailingRail->GetBrushColor().ToFColorSRGB()
				== FColor(0x17, 0x30, 0x3B)
			&& GhostTopFrame != nullptr
			&& FMath::IsNearlyEqual(GhostTopFrame->GetBrushColor().A, 0.18f)
			&& GhostRack->GetWidgetFromName(FName(*FString::Printf(
				TEXT("PlayedCardGhostRarity%d"), StableIndex))) == nullptr
			&& GhostRack->GetWidgetFromName(FName(*FString::Printf(
				TEXT("PlayedCardGhostText%d"), StableIndex))) == nullptr);

	TestTrue(TEXT("Pitch remains exactly two vertical five-slot lanes"),
		Screen->GetPitchWidget()->GetPresentation().Num() == 2
			&& Screen->GetPitchWidget()->GetPresentation()[0].Slots.Num() == 5
			&& Screen->GetPitchWidget()->GetPresentation()[1].Slots.Num() == 5
			&& Screen->GetPitchWidget()->GetRenderedSlotWidgets().Num() == 10);
	for (const TCHAR* GeometryHook : {
		TEXT("PitchTouchlineTop"), TEXT("PhysicalHalfVisualSeparator") })
	{
		TestNotNull(FString::Printf(TEXT("Pitch owns visual geometry %s"),
			GeometryHook), Screen->GetPitchWidget()->GetWidgetFromName(GeometryHook));
	}
	TestTrue(TEXT("Pitch exposes no large Half strips or permanent ATTACKING text"),
		Screen->GetPitchWidget()->GetWidgetFromName(TEXT("TacticalRegionHeading0"))
			== nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("TacticalRegionHeading1")) == nullptr);
	const UTextBlock* LocalRackHeading = Cast<UTextBlock>(
		Screen->GetLocalRackWidget()->GetWidgetFromName(TEXT("CardRackHeading")));
	const UTextBlock* OpponentRackHeading = Cast<UTextBlock>(
		Screen->GetOpponentRackWidget()->GetWidgetFromName(TEXT("CardRackHeading")));
	TestTrue(TEXT("Rack orientation uses localized player-facing labels"),
		LocalRackHeading != nullptr && OpponentRackHeading != nullptr
			&& LocalRackHeading->GetText().ToString() == TEXT("\u672C\u65B9")
			&& OpponentRackHeading->GetText().ToString() == TEXT("\u5BF9\u65B9"));
	auto HasCurrentTrackerStep = [](
		const FFMCodexUMGAttackTurnTrackerViewModel& Tracker)
	{
		return Tracker.Steps.ContainsByPredicate(
			[](const FFMCodexUMGAttackTurnStepViewModel& Step)
			{
				return Step.State
					== EFMCodexUMGAttackTurnStepState::Current;
			});
	};
	TestTrue(TEXT("Exactly one Header-side Tracker marks Current"),
		Screen->GetMatchHeader()->GetWidgetFromName(
			TEXT("LeftCurrentAttackerPointer")) == nullptr
			&& Screen->GetMatchHeader()->GetWidgetFromName(
				TEXT("RightCurrentAttackerPointer")) == nullptr
			&& (HasCurrentTrackerStep(PlayerAView.Header.LeftAttackTurnTracker)
				!= HasCurrentTrackerStep(
					PlayerAView.Header.RightAttackTurnTracker)));
	TestTrue(TEXT("Presentation supplies local-facing tactical labels"),
		!PlayerAView.PitchRegions[0].TacticalRegionLabel.IsEmpty()
			&& !PlayerAView.PitchRegions[1].TacticalRegionLabel.IsEmpty()
			&& PlayerAView.PitchRegions[0].TacticalRegionLabel
				!= PlayerAView.PitchRegions[1].TacticalRegionLabel);
	const bool bPlayerAAttacking = Controller->GetInteractionView()
		.CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA;
	TestTrue(TEXT("Attacking and defending local views receive canonical labels"),
		bPlayerAAttacking
			? PlayerAView.PitchRegions[0].TacticalRegionLabel == TEXT("Midfield")
				&& PlayerAView.PitchRegions[1].TacticalRegionLabel == TEXT("Forward")
				&& PlayerBView.PitchRegions[0].TacticalRegionLabel == TEXT("Backfield")
				&& PlayerBView.PitchRegions[1].TacticalRegionLabel == TEXT("Midfield")
			: PlayerBView.PitchRegions[0].TacticalRegionLabel == TEXT("Midfield")
				&& PlayerBView.PitchRegions[1].TacticalRegionLabel == TEXT("Forward")
				&& PlayerAView.PitchRegions[0].TacticalRegionLabel == TEXT("Backfield")
				&& PlayerAView.PitchRegions[1].TacticalRegionLabel == TEXT("Midfield"));
	const FFMCodexUMGMatchScreenViewModel& AttackingLocalView =
		bPlayerAAttacking ? PlayerAView : PlayerBView;
	const FFMCodexUMGMatchScreenViewModel& DefendingLocalView =
		bPlayerAAttacking ? PlayerBView : PlayerAView;
	TestTrue(TEXT("Local-facing DTO carries typed dynamic visual roles and labels"),
		AttackingLocalView.PitchRegions[0].VisualRole
			== EFMCodexUMGPitchVisualRole::Midfield
			&& AttackingLocalView.PitchRegions[0].VisualRoleLabel.ToString()
				== TEXT("\u4E2D\u573A")
			&& AttackingLocalView.PitchRegions[1].VisualRole
				== EFMCodexUMGPitchVisualRole::Forward
			&& AttackingLocalView.PitchRegions[1].VisualRoleLabel.ToString()
				== TEXT("\u524D\u573A")
			&& DefendingLocalView.PitchRegions[0].VisualRole
				== EFMCodexUMGPitchVisualRole::Backfield
			&& DefendingLocalView.PitchRegions[0].VisualRoleLabel.ToString()
				== TEXT("\u540E\u573A")
			&& DefendingLocalView.PitchRegions[1].VisualRole
				== EFMCodexUMGPitchVisualRole::Midfield
			&& DefendingLocalView.PitchRegions[1].VisualRoleLabel.ToString()
				== TEXT("\u4E2D\u573A"));
	Screen->GetPitchWidget()->RefreshFromPitchPresentation(
		AttackingLocalView.PitchRegions);
	TestTrue(TEXT("Local attack renders Midfield then opponent goal-third only"),
		Screen->GetPitchWidget()->GetWidgetFromName(TEXT("LocalMidfieldArc"))
			!= nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("OpponentForwardPenaltyAreaTop")) != nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("OpponentForwardGoalVisualTop")) != nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("LocalBackfieldGoalVisualTop")) == nullptr);
	Screen->GetPitchWidget()->RefreshFromPitchPresentation(
		DefendingLocalView.PitchRegions);
	TestTrue(TEXT("Local defense renders local goal-third then Midfield only"),
		Screen->GetPitchWidget()->GetWidgetFromName(
			TEXT("LocalBackfieldPenaltyAreaTop")) != nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("LocalBackfieldGoalVisualTop")) != nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("OpponentMidfieldArc")) != nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("OpponentForwardGoalVisualTop")) == nullptr
			&& Screen->GetPitchWidget()->GetWidgetFromName(
				TEXT("PitchCenterCircleVisual")) == nullptr);
	const UTextBlock* SemanticLabel0 = Cast<UTextBlock>(
		Screen->GetPitchWidget()->GetWidgetFromName(TEXT("PitchSemanticLabel0")));
	const UTextBlock* SemanticLabel1 = Cast<UTextBlock>(
		Screen->GetPitchWidget()->GetWidgetFromName(TEXT("PitchSemanticLabel1")));
	TestTrue(TEXT("Dynamic Half labels are readable localized bounded text"),
		SemanticLabel0 != nullptr && SemanticLabel1 != nullptr
			&& SemanticLabel0->GetText().ToString() == TEXT("\u540E\u573A")
			&& SemanticLabel1->GetText().ToString() == TEXT("\u4E2D\u573A")
			&& SemanticLabel0->GetClipping() == EWidgetClipping::ClipToBounds
			&& SemanticLabel1->GetClipping() == EWidgetClipping::ClipToBounds);
	TestTrue(TEXT("Broadcast header receives score turn attacker and one TP value"),
		!PlayerAView.Header.ScoreLabel.IsEmpty()
			&& !PlayerAView.Header.TurnLabel.IsEmpty()
			&& !PlayerAView.Header.CurrentAttackerLabel.IsEmpty()
			&& PlayerAView.Header.CurrentAttackerTacticalPointsLabel.Contains(
				TEXT("TACTICAL POINTS")));
	UFMCodexMatchHeaderWidget* GoldenHeader = Screen->GetMatchHeader();
	UFMCodexInteractionPanelWidget* GoldenDock = Screen->GetInteractionPanel();
	UWidget* GoldenDockKicker = GoldenDock != nullptr
		? GoldenDock->GetWidgetFromName(TEXT("InteractionActionKicker")) : nullptr;
	TestTrue(TEXT("Header owns score progress phase and trackers without side scores"),
		GoldenHeader != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("CentralBroadcastScoreValue")) != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("CurrentAttackProgressLabel")) != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("CurrentMatchPhaseStatusLabel")) != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("LeftAttackTurnSteps")) != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("RightAttackTurnSteps")) != nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("LeftPlayerScoreValue")) == nullptr
			&& GoldenHeader->GetWidgetFromName(
				TEXT("RightPlayerScoreValue")) == nullptr);
	TestTrue(TEXT("Dock owns operation context without persistent match facts"),
		GoldenDock != nullptr && PlayerAView.Interaction.ActionPointLabel.IsEmpty()
			&& GoldenDock->GetWidgetFromName(
				TEXT("CentralBroadcastScoreValue")) == nullptr
			&& GoldenDock->GetWidgetFromName(
				TEXT("CurrentAttackProgressLabel")) == nullptr
			&& GoldenDock->GetWidgetFromName(
				TEXT("CurrentMatchPhaseStatusLabel")) == nullptr);
	TestTrue(TEXT("Dock generic match kicker is absent or collapsed"),
		GoldenDockKicker == nullptr
			|| GoldenDockKicker->GetVisibility() == ESlateVisibility::Collapsed);
	FFMCodexUMGMatchHeaderViewModel LongHeader = PlayerAView.Header;
	LongHeader.LeftPlayerLabel = TEXT("\u672C\u65B9\u957F\u7403\u961F\u4E0E\u73A9\u5BB6\u8EAB\u4EFD\u6807\u7B7E\u5B89\u5168\u533A\u68C0\u67E5");
	LongHeader.RightPlayerLabel = TEXT("\u5BF9\u65B9\u957F\u7403\u961F\u4E0E\u73A9\u5BB6\u8EAB\u4EFD\u6807\u7B7E\u5B89\u5168\u533A\u68C0\u67E5");
	if (GoldenHeader == nullptr)
	{
		return false;
	}
	GoldenHeader->RefreshFromPresentation(LongHeader);
	const UTextBlock* LongLeftHeader = Cast<UTextBlock>(
		GoldenHeader->GetWidgetFromName(
			TEXT("LeftPlayerIdentityLabel")));
	const UTextBlock* LongRightHeader = Cast<UTextBlock>(
		GoldenHeader->GetWidgetFromName(
			TEXT("RightPlayerIdentityLabel")));
	TestTrue(TEXT("Representative long Chinese Header identity stays bounded"),
		LongLeftHeader != nullptr && LongRightHeader != nullptr
			&& !LongLeftHeader->GetAutoWrapText()
			&& !LongRightHeader->GetAutoWrapText()
			&& LongLeftHeader->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Ellipsis
			&& LongRightHeader->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Ellipsis
			&& LongLeftHeader->GetClipping() == EWidgetClipping::ClipToBounds
			&& LongRightHeader->GetClipping() == EWidgetClipping::ClipToBounds);
	UFMCodexInteractionPanelWidget* TextSafeDock =
		CreateWidget<UFMCodexInteractionPanelWidget>(
			Screen->GetWorld(), UFMCodexInteractionPanelWidget::StaticClass());
	FFMCodexUMGInteractionViewModel LongAction;
	LongAction.KickerLabel = TEXT("LOCAL MATCH");
	LongAction.ExpectedActorLabel = TEXT("PLAYER A TO ACT");
	LongAction.TitleLabel = TEXT("\u8BF7\u9009\u62E9\u5F53\u524D\u8FDB\u653B\u9636\u6BB5\u4E2D\u7684\u5408\u6CD5\u64CD\u4F5C\u5E76\u786E\u8BA4\u7ED3\u679C");
	LongAction.CategoryLabel = TEXT("\u6B63\u5728\u8FDB\u884C\u957F\u4E0A\u4E0B\u6587\u5B89\u5168\u533A\u68C0\u67E5");
	LongAction.ActionPointLabel = TEXT("TACTICAL POINTS  99");
	LongAction.PrimaryActionLabel = TEXT("START LOCAL MATCH");
	LongAction.bCanStartNewMatch = true;
	TextSafeDock->RefreshFromPresentation(LongAction);
	TextSafeDock->TakeWidget();
	const UTextBlock* LongActionTitle = Cast<UTextBlock>(
		TextSafeDock->GetWidgetFromName(TEXT("InteractionActionTitle")));
	const UTextBlock* LongActionContext = Cast<UTextBlock>(
		TextSafeDock->GetWidgetFromName(TEXT("InteractionActionContext")));
	const UButton* PrimaryButton = Cast<UButton>(
		TextSafeDock->GetWidgetFromName(TEXT("InteractionStartMatchButton")));
	const UTextBlock* PrimaryLabel = PrimaryButton != nullptr
		? Cast<UTextBlock>(PrimaryButton->GetChildAt(0)) : nullptr;
	TestTrue(TEXT("Long Chinese Dock text is bounded and primary label remains full"),
		LongActionTitle != nullptr && LongActionContext != nullptr
			&& !LongActionTitle->GetAutoWrapText()
			&& !LongActionContext->GetAutoWrapText()
			&& LongActionTitle->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Ellipsis
			&& LongActionTitle->GetClipping() == EWidgetClipping::ClipToBounds
			&& LongActionContext->GetClipping() == EWidgetClipping::ClipToBounds
			&& !LongActionContext->GetText().ToString().Contains(TEXT("99"))
			&& PrimaryButton->GetVisibility() == ESlateVisibility::Visible
			&& PrimaryLabel != nullptr && !PrimaryLabel->GetAutoWrapText()
			&& PrimaryLabel->GetTextOverflowPolicy() == ETextOverflowPolicy::Clip
			&& PrimaryLabel->GetClipping() == EWidgetClipping::ClipToBounds
			&& PrimaryLabel->GetText().ToString() == TEXT("\u5F00\u59CB\u672C\u5730\u5BF9\u6218"));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexFiveSlotDragDropDeploymentIntegrationTest,
	"FMCodex.LocalPlay.ControlSurface.37.FiveSlotDragDropDeploymentIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexFiveSlotDragDropDeploymentIntegrationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;

	const FFMCodexLocalMatchDemoConfiguration Configuration =
		FFMCodexLocalMatchDemoConfigurationFactory::Create();
	const TArray<FMatchPlayDeploymentSlotDefinition>& CanonicalSlots =
		Configuration.OpeningInput.DeploymentSlotCatalog.Slots;
	int32 NearPlayerACount = 0;
	int32 NearPlayerBCount = 0;
	TSet<FName> CanonicalSlotIds;
	for (const FMatchPlayDeploymentSlotDefinition& SlotDefinition
		: CanonicalSlots)
	{
		NearPlayerACount += SlotDefinition.NeutralSide
			== EMatchPlayNeutralSlotSide::NearPlayerA ? 1 : 0;
		NearPlayerBCount += SlotDefinition.NeutralSide
			== EMatchPlayNeutralSlotSide::NearPlayerB ? 1 : 0;
		CanonicalSlotIds.Add(SlotDefinition.SlotId);
	}
	TestTrue(TEXT("Canonical demo owns exactly five slots per physical half"),
		CanonicalSlots.Num() == 10
			&& NearPlayerACount == 5
			&& NearPlayerBCount == 5
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.01"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.05"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.01"))
			&& CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.05"))
			&& !CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearA.06"))
			&& !CanonicalSlotIds.Contains(TEXT("Demo.Slot.NearB.06")));

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Drag/drop integration Host exists"), Host);
	TestNotNull(TEXT("Drag/drop integration Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Drag/drop integration Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	Screen->RequestRollTacticalPoints();

	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	UFMCodexCardRackWidget* LocalRack = Screen->GetLocalRackWidget();
	TestNotNull(TEXT("Drag/drop integration Pitch exists"), Pitch);
	TestNotNull(TEXT("Drag/drop integration Panel exists"), Panel);
	TestNotNull(TEXT("Drag/drop integration local rack exists"), LocalRack);
	if (Pitch == nullptr || Panel == nullptr || LocalRack == nullptr)
	{
		return false;
	}
	const FFMCodexUMGMatchScreenViewModel& DeploymentPresentation =
		Screen->GetPresentation();
	TestTrue(TEXT("Deployment hand and pitch expose bounded product DTOs"),
		DeploymentPresentation.Interaction.Category
			== EFMCodexUMGInteractionCategory::Deploy
			&& !DeploymentPresentation.Interaction.DeploymentChoices.IsEmpty()
			&& LocalRack->GetRenderedCellCount() == 20
			&& LocalRack->GetRenderedCardWidgets().Num() == 20
			&& Panel->GetRenderedOptionWidgets().IsEmpty()
			&& Pitch->GetPresentation().Num() == 2
			&& Pitch->GetPresentation()[0].Slots.Num() == 5
			&& Pitch->GetPresentation()[1].Slots.Num() == 5
			&& Pitch->GetRenderedSlotWidgets().Num() == 10);

	bool bCanonicalVerticalLanes = Pitch->GetRenderedSlotWidgets().Num() == 10;
	for (int32 Index = 0;
		bCanonicalVerticalLanes && Index < Pitch->GetRenderedSlotWidgets().Num();
		++Index)
	{
		const UFMCodexPitchSlotWidget* SlotWidget =
			Pitch->GetRenderedSlotWidgets()[Index];
		const UUniformGridSlot* GridSlot = SlotWidget == nullptr
			? nullptr : Cast<UUniformGridSlot>(SlotWidget->Slot);
		bCanonicalVerticalLanes = GridSlot != nullptr
			&& GridSlot->GetRow() == Index % 5
			&& GridSlot->GetColumn() == 0;
	}
	TestTrue(TEXT("Rendered board is one vertical lane per physical half"),
		bCanonicalVerticalLanes);

	auto FindChoice = [](const FFMCodexUMGInteractionViewModel& Interaction,
		const bool bGoalkeeper)
		-> const FFMCodexUMGDeploymentChoiceViewModel*
	{
		return Interaction.DeploymentChoices.FindByPredicate(
			[bGoalkeeper](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
			{
				return Candidate.bGoalkeeper == bGoalkeeper
					&& !Candidate.Destinations.IsEmpty();
			});
	};
	auto FindHandCard = [](UFMCodexCardRackWidget& Rack,
		const FName CardId) -> UFMCodexPlayerCardWidget*
	{
		for (UFMCodexPlayerCardWidget* CardWidget
			: Rack.GetRenderedCardWidgets())
		{
			if (CardWidget != nullptr
				&& CardWidget->GetDeploymentDragCardId() == CardId)
			{
				return CardWidget;
			}
		}
		return nullptr;
	};
	auto FindPitchSlot = [](UFMCodexPitchWidget& PitchWidget,
		const FName SlotId) -> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: PitchWidget.GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == SlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};

	const FFMCodexUMGDeploymentChoiceViewModel* OrdinaryChoiceSource =
		FindChoice(Screen->GetPresentation().Interaction, false);
	TestNotNull(TEXT("Authoritative presentation exposes ordinary hand card"),
		OrdinaryChoiceSource);
	if (OrdinaryChoiceSource == nullptr)
	{
		return false;
	}
	const FFMCodexUMGDeploymentChoiceViewModel OrdinaryChoice =
		*OrdinaryChoiceSource;
	UFMCodexPlayerCardWidget* OrdinaryCard =
		FindHandCard(*LocalRack, OrdinaryChoice.CardId);
	TestNotNull(TEXT("Ordinary presentation card is a real drag source"),
		OrdinaryCard);
	if (OrdinaryCard == nullptr)
	{
		return false;
	}

	const TArray<uint8> StateBeforeCancel =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FString CommandBeforeCancel = Controller->GetLastDiagnostic().CommandName;
	UFMCodexDeploymentDragDropOperation* CancelledOperation =
		OrdinaryCard->BeginDeploymentDrag();
	TestNotNull(TEXT("Mouse drag creates typed deployment operation"),
		CancelledOperation);
	if (CancelledOperation == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Drag payload contains presentation identity only"),
		CancelledOperation->CardId == OrdinaryChoice.CardId
			&& !CancelledOperation->bGoalkeeper
			&& CancelledOperation->CardPresentation.CardId
				== OrdinaryChoice.CardId
			&& Pitch->GetActiveDeploymentCardId() == OrdinaryChoice.CardId);
	CancelledOperation->DragCancelled(FPointerEvent());
	TestTrue(TEXT("Cancelled drag emits zero command and zero State mutation"),
		Pitch->GetActiveDeploymentCardId().IsNone()
			&& Controller->GetLastDiagnostic().CommandName == CommandBeforeCancel
			&& StateBeforeCancel
				== SerializeState(Host->GetMatchSnapshot().Snapshot));

	UFMCodexDeploymentDragDropOperation* OrdinaryOperation =
		OrdinaryCard->BeginDeploymentDrag();
	if (OrdinaryOperation == nullptr)
	{
		return false;
	}
	UFMCodexPitchSlotWidget* OrdinaryTarget = FindPitchSlot(
		*Pitch, OrdinaryChoice.Destinations[0].SlotId);
	TestNotNull(TEXT("Every ordinary destination maps to a rendered drop target"),
		OrdinaryTarget);
	if (OrdinaryTarget == nullptr)
	{
		return false;
	}
	const int32 PlacementsBeforeOrdinary =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.DeploymentPlacements.Num();
	TestTrue(TEXT("Ordinary destination is projected valid without rule queries"),
		OrdinaryTarget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Valid
			&& OrdinaryTarget->CanAcceptDeploymentOperation(OrdinaryOperation));
	const bool bOrdinaryDropAccepted =
		OrdinaryTarget->TryHandleDeploymentDrop(OrdinaryOperation);
	TestTrue(TEXT("Ordinary drop follows typed authoritative command chain"),
		bOrdinaryDropAccepted
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("DeployOrdinary")
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.Num() == PlacementsBeforeOrdinary + 1
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.ContainsByPredicate(
					[&OrdinaryChoice](
						const FMatchPlayDeploymentPlacement& Placement)
					{
						return Placement.CardId == OrdinaryChoice.CardId
							&& Placement.SlotId
								== OrdinaryChoice.Destinations[0].SlotId;
					}));
	OrdinaryOperation->Drop(FPointerEvent());
	const FFMCodexUMGDeploymentDestinationViewModel* StaleDestination =
		OrdinaryChoice.Destinations.FindByPredicate(
			[&OrdinaryChoice](
				const FFMCodexUMGDeploymentDestinationViewModel& Destination)
			{
				return Destination.SlotId
					!= OrdinaryChoice.Destinations[0].SlotId;
			});
	TestNotNull(TEXT("Ordinary fixture retains an empty stale destination"),
		StaleDestination);
	if (StaleDestination == nullptr)
	{
		return false;
	}
	Pitch->BeginDeploymentDrag(
		OrdinaryChoice.CardId, { OrdinaryChoice });
	UFMCodexPitchSlotWidget* StaleTarget =
		FindPitchSlot(*Pitch, StaleDestination->SlotId);
	TestNotNull(TEXT("Stale presentation projects a rendered drop target"),
		StaleTarget);
	if (StaleTarget == nullptr)
	{
		return false;
	}
	const TArray<uint8> StateBeforeStaleDrop =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const bool bStaleDropLocallyAccepted =
		StaleTarget->TryHandleDeploymentDrop(OrdinaryOperation);
	TestTrue(TEXT("Stale rendered target still emits its typed intent"),
		bStaleDropLocallyAccepted);
	TestEqual(TEXT("Stale typed intent remains ordinary deployment"),
		Controller->GetLastDiagnostic().CommandName,
		FString(TEXT("DeployOrdinary")));
	TestFalse(TEXT("Authority rejects the stale ordinary deployment"),
		Controller->GetLastDiagnostic().bHostSuccess);
	TestTrue(TEXT("Stale authoritative rejection preserves State"),
		StateBeforeStaleDrop
			== SerializeState(Host->GetMatchSnapshot().Snapshot));
	TestTrue(TEXT("Stale rejection refreshes failure and clears drag projection"),
		Screen->GetPresentation().Resolution.bRejected
			&& Pitch->GetActiveDeploymentCardId().IsNone());

	const FFMCodexUMGDeploymentChoiceViewModel* GoalkeeperChoiceSource =
		FindChoice(Screen->GetPresentation().Interaction, true);
	TestNotNull(TEXT("Next authoritative hand exposes goalkeeper choice"),
		GoalkeeperChoiceSource);
	if (GoalkeeperChoiceSource == nullptr)
	{
		return false;
	}
	const FFMCodexUMGDeploymentChoiceViewModel GoalkeeperChoice =
		*GoalkeeperChoiceSource;
	UFMCodexPlayerCardWidget* GoalkeeperCard =
		FindHandCard(*LocalRack, GoalkeeperChoice.CardId);
	TestNotNull(TEXT("Goalkeeper presentation card is a real drag source"),
		GoalkeeperCard);
	if (GoalkeeperCard == nullptr)
	{
		return false;
	}
	UFMCodexDeploymentDragDropOperation* GoalkeeperOperation =
		GoalkeeperCard->BeginDeploymentDrag();
	if (GoalkeeperOperation == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Goalkeeper drag retains presentation identity and type"),
		GoalkeeperOperation->CardId == GoalkeeperChoice.CardId
			&& GoalkeeperOperation->bGoalkeeper);

	UFMCodexPitchSlotWidget* InvalidTarget = nullptr;
	UFMCodexPitchSlotWidget* OccupiedTarget = nullptr;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		if (SlotWidget == nullptr)
		{
			continue;
		}
		if (SlotWidget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Invalid)
		{
			InvalidTarget = SlotWidget;
		}
		if (SlotWidget->GetPresentation().DeploymentTargetState
			== EFMCodexUMGDeploymentTargetState::Occupied)
		{
			OccupiedTarget = SlotWidget;
		}
	}
	TestNotNull(TEXT("Goalkeeper projection marks non-destination invalid"),
		InvalidTarget);
	TestNotNull(TEXT("Goalkeeper projection preserves occupied target state"),
		OccupiedTarget);
	if (InvalidTarget == nullptr)
	{
		return false;
	}
	const TArray<uint8> StateBeforeInvalidDrop =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FString CommandBeforeInvalidDrop =
		Controller->GetLastDiagnostic().CommandName;
	TestTrue(TEXT("Invalid drop emits zero command and zero State mutation"),
		!InvalidTarget->TryHandleDeploymentDrop(GoalkeeperOperation)
			&& Controller->GetLastDiagnostic().CommandName
				== CommandBeforeInvalidDrop
			&& StateBeforeInvalidDrop
				== SerializeState(Host->GetMatchSnapshot().Snapshot));

	UFMCodexPitchSlotWidget* GoalkeeperTarget = FindPitchSlot(
		*Pitch, GoalkeeperChoice.Destinations[0].SlotId);
	TestNotNull(TEXT("Goalkeeper destination maps to a rendered drop target"),
		GoalkeeperTarget);
	if (GoalkeeperTarget == nullptr)
	{
		return false;
	}
	const int32 PlacementsBeforeGoalkeeper =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.DeploymentPlacements.Num();
	const bool bGoalkeeperDropAccepted =
		GoalkeeperTarget->TryHandleDeploymentDrop(GoalkeeperOperation);
	TestTrue(TEXT("Goalkeeper drop routes slot-only typed authoritative command"),
		bGoalkeeperDropAccepted
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("DeployGoalkeeper")
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.Num() == PlacementsBeforeGoalkeeper + 1
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.ContainsByPredicate(
					[&GoalkeeperChoice](
						const FMatchPlayDeploymentPlacement& Placement)
					{
						return Placement.CardId == GoalkeeperChoice.CardId
							&& Placement.SlotId
								== GoalkeeperChoice.Destinations[0].SlotId;
					}));
	GoalkeeperOperation->Drop(FPointerEvent());

	FFMCodexUMGPitchSlotViewModel ProjectionSlot;
	ProjectionSlot.SlotId = TEXT("Projection.Valid");
	FFMCodexUMGDeploymentChoiceViewModel ProjectionChoice;
	ProjectionChoice.CardId = TEXT("Projection.Card");
	ProjectionChoice.Destinations.Add(
		{ ProjectionSlot.SlotId, TEXT("Projection destination") });
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bValidProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Valid;
	ProjectionSlot.SlotId = TEXT("Projection.Invalid");
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bInvalidProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Invalid;
	ProjectionSlot.bOccupied = true;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, ProjectionChoice.CardId, { ProjectionChoice });
	const bool bOccupiedProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Occupied;
	ProjectionSlot.bOccupied = false;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, TEXT("Projection.Unknown"), { ProjectionChoice });
	const bool bUnavailableProjection = ProjectionSlot.DeploymentTargetState
		== EFMCodexUMGDeploymentTargetState::Unavailable;
	FFMCodexUMGDeploymentTargetProjector::ProjectSlot(
		ProjectionSlot, NAME_None, { ProjectionChoice });
	TestTrue(TEXT("Presentation projector covers all deployment target states"),
		bValidProjection && bInvalidProjection && bOccupiedProjection
			&& bUnavailableProjection
			&& ProjectionSlot.DeploymentTargetState
				== EFMCodexUMGDeploymentTargetState::Neutral);

	FString DragOperationHeader;
	FString CardWidgetSource;
	FString PitchSlotSource;
	FString PitchWidgetSource;
	FString ScreenSource;
	FString ControllerSource;
	FString HostSource;
	FString SessionSource;
	TestTrue(TEXT("Deployment drag/drop authority boundary sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexDeploymentDragDropOperation.h"),
			DragOperationHeader)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				PitchSlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"),
				PitchWidgetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ScreenSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchPlayerController.cpp"),
				ControllerSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"),
				HostSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
				SessionSource));
	const FString DragDropSources = DragOperationHeader + CardWidgetSource
		+ PitchSlotSource + PitchWidgetSource;
	TestTrue(TEXT("Drag/drop layer contains no gameplay authority or legality"),
		!DragDropSources.Contains(TEXT("FMatchPlayState"))
			&& !DragDropSources.Contains(TEXT("AuthoritativeSession"))
			&& !DragDropSources.Contains(TEXT("LocalMatchHost"))
			&& !DragDropSources.Contains(TEXT("RelativeDeploymentZoneResolver"))
			&& !DragDropSources.Contains(TEXT("Legality"))
			&& !DragDropSources.Contains(TEXT("Formula")));
	TestTrue(TEXT("Screen preserves explicit ordinary and slot-only GK routing"),
		ScreenSource.Contains(TEXT("RequestDeployOrdinary(CardId, SlotId)"))
			&& ScreenSource.Contains(TEXT("RequestDeployGoalkeeper(SlotId)"))
			&& !ScreenSource.Contains(TEXT("RequestDeployGoalkeeper(CardId"))
			&& ControllerSource.Contains(TEXT("Host->DeployOrdinary(Request)"))
			&& ControllerSource.Contains(TEXT("Host->DeployGoalkeeper(Request)"))
			&& HostSource.Contains(
				TEXT("AuthoritativeSession.DeployGoalkeeper(Request)")));
	FString SessionCountSource = SessionSource;
	const int32 SerializedEntrypointCount = SessionCountSource.ReplaceInline(
		TEXT("ExecuteSerialized<"), TEXT(""), ESearchCase::CaseSensitive);
	TestEqual(TEXT("Authoritative Session serialized entrypoint count is unchanged"),
		SerializedEntrypointCount, 42);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexHandMicroProductionContractTest,
	"FMCodex.LocalPlay.ControlSurface.39.HandMicroProductionContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexHandMicroProductionContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	IConsoleVariable* ReviewCVar = IConsoleManager::Get().FindConsoleVariable(
		TEXT("FMCodex.UI.HandMicroReview"));
	IConsoleVariable* PageCVar = IConsoleManager::Get().FindConsoleVariable(
		TEXT("FMCodex.UI.HandMicroReviewPage"));
	if (ReviewCVar == nullptr || PageCVar == nullptr)
	{
		return false;
	}
	struct FScopedConsoleState
	{
		TArray<TPair<IConsoleVariable*, int32>> Values;
		~FScopedConsoleState()
		{
			for (const TPair<IConsoleVariable*, int32>& Value : Values)
			{
				Value.Key->Set(Value.Value, ECVF_SetByConsole);
			}
		}
	} ConsoleState { {
		{ ReviewCVar, ReviewCVar->GetInt() },
		{ PageCVar, PageCVar->GetInt() }
	} };
	TestEqual(TEXT("Production review is opt-in"), ReviewCVar->GetInt(), 0);
	TestTrue(TEXT("Historical experiment CVars are removed"),
		IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroFullNameCandidate")) == nullptr
		&& IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroUnifiedNameSize")) == nullptr
		&& IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroHeight68")) == nullptr
		&& IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroSharpnessDiagnostic")) == nullptr
		&& IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroSharpnessDiagnosticPage")) == nullptr
		&& IConsoleManager::Get().FindConsoleVariable(
			TEXT("FMCodex.UI.HandMicroArtConformanceOverride")) == nullptr);
	ReviewCVar->Set(1, ECVF_SetByConsole);
	PageCVar->Set(0, ECVF_SetByConsole);

	struct FPortraitInventoryEntry
	{
		FName CardId;
		FString DisplayName;
		bool bGolden;
	};
	const TArray<FPortraitInventoryEntry> Inventory = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("拉亚"), true },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("萨利巴"), true },
		{ TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("萨卡"), true },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("厄德高"), true },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"),
			TEXT("多纳鲁马"), true },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), TEXT("哈兰德"), true },
		{ TEXT("Prototype.Arsenal.DeclanRice"), TEXT("赖斯"), false },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"), TEXT("马丁内利"), false },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"), TEXT("加布里埃尔"), false },
		{ TEXT("Prototype.Arsenal.MikelMerino"), TEXT("梅里诺"), false },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"), TEXT("福登"), false },
		{ TEXT("Prototype.ManchesterCity.Rodri"), TEXT("罗德里"), false },
		{ TEXT("Prototype.ManchesterCity.RubenDias"), TEXT("迪亚斯"), false },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"), TEXT("格瓦迪奥尔"), false },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"), TEXT("贝尔纳多"), false },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"), TEXT("多库"), false }
	};
	TestEqual(TEXT("Current actually-used portrait inventory is explicit"),
		Inventory.Num(), 16);
	int32 GoldenCount = 0;
	int32 ExpandedCount = 0;
	bool bAllProductionAssetsValid = true;
	for (const FPortraitInventoryEntry& Entry : Inventory)
	{
		const FFMCodexPlayerUICardArtReferences Art =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Entry.CardId);
		UTexture2D* HandTexture = Art.HandMicroPortrait.IsNull()
			? nullptr : Art.HandMicroPortrait.LoadSynchronous();
		const FString HandPath = Art.HandMicroPortrait.ToSoftObjectPath().ToString();
		bAllProductionAssetsValid = bAllProductionAssetsValid
			&& HandTexture != nullptr
			&& HandTexture->GetImportedSize() == FIntPoint(192, 128)
			&& HandTexture->LODGroup == TEXTUREGROUP_UI
			&& HandTexture->CompressionSettings == TC_BC7
			&& HandTexture->MipGenSettings == TMGS_Sharpen1
			&& HandTexture->Filter == TF_Trilinear
			&& HandTexture->NeverStream && HandTexture->SRGB
			&& HandTexture->LODBias == 0
			&& HandPath.Contains(TEXT("/Game/UI/Portraits/PrototypeTeams/"
				"HandMicroApprovedRollout/"))
			&& HandPath.Contains(TEXT("ApprovedRuntime192"))
			&& !HandPath.Contains(TEXT("/Developers/"))
			&& FMath::IsNearlyEqual(Art.HandMicroPortraitTop, 0.0f)
			&& FMath::IsNearlyEqual(Art.HandMicroPortraitUVHeight, 1.0f)
			&& (Art.Portrait.IsNull()
				|| Art.Portrait.ToSoftObjectPath() != Art.HandMicroPortrait.ToSoftObjectPath());
		Entry.bGolden ? ++GoldenCount : ++ExpandedCount;
	}
	TestTrue(TEXT("Golden six plus ten expanded players use one production Runtime192 contract"),
		bAllProductionAssetsValid && GoldenCount == 6 && ExpandedCount == 10);

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchPlayerController* Controller = PlayableWorld.GetController();
	if (Controller == nullptr)
	{
		return false;
	}
	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen = Controller->GetPlayerMatchScreen();
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	const UWidget* RolloutPage = Screen->GetWidgetFromName(
		TEXT("HandMicroProductionReviewPortraitsPage"));
	const UUniformGridPanel* RolloutGrid = Cast<UUniformGridPanel>(
		Screen->GetWidgetFromName(TEXT("HandMicroProductionReviewPortraitsGrid")));
	const USizeBox* DiagnosticBounds = Cast<USizeBox>(
		Screen->GetWidgetFromName(TEXT("HandMicroProductionReviewBounds")));
	TestTrue(TEXT("Production review page 0 exposes the approved 16 portraits"),
		RolloutPage != nullptr
			&& RolloutPage->GetVisibility() == ESlateVisibility::HitTestInvisible
			&& RolloutGrid != nullptr && RolloutGrid->GetChildrenCount() == 16
			&& DiagnosticBounds != nullptr
			&& DiagnosticBounds->GetWidthOverride() == 560.0f
			&& DiagnosticBounds->GetHeightOverride() == 660.0f);
	PageCVar->Set(1, ECVF_SetByConsole);
	Screen->RefreshFromPresentation(Screen->GetPresentation());
	const UUniformGridPanel* TypographyGrid = Cast<UUniformGridPanel>(
		Screen->GetWidgetFromName(TEXT("HandMicroProductionReviewTypographyGrid")));
	TestTrue(TEXT("Production review page 1 is the bounded five-name stress surface"),
		TypographyGrid != nullptr && TypographyGrid->GetChildrenCount() == 5
			&& Screen->GetWidgetFromName(TEXT("HandMicroProductionReviewTypographyPage"))
				->GetVisibility() == ESlateVisibility::HitTestInvisible);
	PageCVar->Set(2, ECVF_SetByConsole);
	Screen->RefreshFromPresentation(Screen->GetPresentation());
	const UFMCodexCardRackWidget* ReviewRack = Cast<UFMCodexCardRackWidget>(
		Screen->GetWidgetFromName(TEXT("HandMicroProductionReviewRack")));
	TestTrue(TEXT("Production review page 2 uses the real 2x10 rack renderer"),
		ReviewRack != nullptr && ReviewRack->GetRenderedCellCount() == 20
			&& ReviewRack->GetWidgetFromName(TEXT("CardRackScrollBox")) == nullptr
			&& ReviewRack->GetWidgetFromName(TEXT("CardRackPageControl")) == nullptr
			&& ReviewRack->GetWidgetFromName(TEXT("PlayedCardGhostBounds4")) != nullptr);

	bool bAllCardsUseApprovedRenderer = true;
	for (const FPortraitInventoryEntry& Entry : Inventory)
	{
		FFMCodexUMGCardViewModel Model;
		Model.CardId = Entry.CardId;
		Model.IdentityLabel = Entry.DisplayName;
		Model.RoleLabel = TEXT("AMD");
		Model.RarityLabel = TEXT("Continental");
		UFMCodexPlayerCardWidget* Card =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		Card->RefreshFromPresentation(
			Model, EFMCodexPlayerCardPresentationMode::HandMicro);
		Card->TakeWidget();
		const USizeBox* PortraitCell = Cast<USizeBox>(
			Card->GetWidgetFromName(TEXT("HandMicroPortraitCellBounds")));
		const USizeBox* PortraitImage = Cast<USizeBox>(
			Card->GetWidgetFromName(TEXT("HandMicroPortraitBounds")));
		const USizeBox* Identity = Cast<USizeBox>(
			Card->GetWidgetFromName(TEXT("HandMicroIdentityBounds")));
		const USizeBox* Rarity = Cast<USizeBox>(
			Card->GetWidgetFromName(TEXT("HandMicroRarityAccentBounds")));
		const UTextBlock* Name = Cast<UTextBlock>(
			Card->GetWidgetFromName(TEXT("HandMicroPlayerName")));
		const UImage* HandImage = Cast<UImage>(
			Card->GetWidgetFromName(TEXT("HandMicroFaceSafePortrait")));
		const FBox2f UV = HandImage != nullptr
			? static_cast<FBox2f>(HandImage->GetBrush().GetUVRegion()) : FBox2f();
		bAllCardsUseApprovedRenderer = bAllCardsUseApprovedRenderer
			&& Card->GetConfiguredDimensions() == FVector2D(220.0f, 68.0f)
			&& PortraitCell != nullptr
			&& PortraitCell->GetWidthOverride() == 96.0f
			&& PortraitCell->GetHeightOverride() == 68.0f
			&& PortraitImage != nullptr
			&& PortraitImage->GetWidthOverride() == 96.0f
			&& PortraitImage->GetHeightOverride() == 64.0f
			&& Identity != nullptr && Identity->GetWidthOverride() == 120.0f
			&& Identity->GetHeightOverride() == 68.0f
			&& Rarity != nullptr && Rarity->GetWidthOverride() == 4.0f
			&& Rarity->GetHeightOverride() == 68.0f
			&& Name != nullptr && Name->GetText().ToString() == Entry.DisplayName
			&& Name->GetFont().Size <= 16 && Name->GetFont().Size >= 12
			&& Name->GetTextOverflowPolicy() == ETextOverflowPolicy::Clip
			&& UV.bIsValid && UV.Min == FVector2f(0.0f, 0.0f)
			&& UV.Max == FVector2f(1.0f, 1.0f)
			&& HandImage->GetRenderTransform().Scale == FVector2D(1.0f, 1.0f)
			&& Card->GetRenderTransform().Scale == FVector2D(1.0f, 1.0f);
		if (Entry.CardId == TEXT("Prototype.Arsenal.GabrielMagalhaes"))
		{
			bAllCardsUseApprovedRenderer = bAllCardsUseApprovedRenderer
				&& Name->GetText().ToString() == TEXT("加布里埃尔");
		}
	}
	TestTrue(TEXT("All 16 portraits use approved geometry, typography, full UV and no transforms"),
		bAllCardsUseApprovedRenderer);

	FFMCodexUMGCardViewModel IsolationModel;
	IsolationModel.CardId = TEXT("Prototype.Arsenal.WilliamSaliba");
	IsolationModel.IdentityLabel = TEXT("萨利巴");
	IsolationModel.RoleLabel = TEXT("DF");
	UFMCodexPlayerCardWidget* FullCard =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	FullCard->RefreshFromPresentation(
		IsolationModel, EFMCodexPlayerCardPresentationMode::InteractionChoice);
	FullCard->TakeWidget();
	UFMCodexPlayerCardWidget* PitchMini =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	PitchMini->RefreshFromPresentation(
		IsolationModel, EFMCodexPlayerCardPresentationMode::PitchMini);
	PitchMini->TakeWidget();
	const UImage* FullPortraitImage = Cast<UImage>(
		FullCard->GetWidgetFromName(TEXT("PortraitAssetImage")));
	const UImage* PitchPortraitImage = Cast<UImage>(
		PitchMini->GetWidgetFromName(TEXT("PitchMiniPortraitImage")));
	TestTrue(TEXT("Full Card Hero Bust is isolated from Pitch Mini and Hand Micro portraits"),
		FullCard->GetResolvedPortraitTexture() != nullptr
			&& FullCard->GetResolvedPortraitTexture()->GetPathName().Contains(
				TEXT("T_Prototype_Arsenal_WilliamSaliba_FullCardHeroBust_01"))
			&& FullPortraitImage != nullptr
			&& FullPortraitImage->GetBrush().GetResourceObject()
				== FullCard->GetResolvedPortraitTexture()
			&& PitchMini->GetResolvedPortraitTexture() != nullptr
			&& PitchMini->GetResolvedPortraitTexture()->GetPathName().Contains(
				TEXT("T_Prototype_Arsenal_WilliamSaliba_01"))
			&& PitchPortraitImage != nullptr
			&& PitchPortraitImage->GetBrush().GetResourceObject()
				== PitchMini->GetResolvedPortraitTexture()
			&& FullCard->GetResolvedHandMicroPortraitTexture() != nullptr
			&& FullCard->GetResolvedHandMicroPortraitTexture()->GetPathName()
				.Contains(TEXT("T_Prototype_Arsenal_WilliamSaliba_HandMicro_ApprovedRuntime192"))
			&& FullCard->GetResolvedHandMicroPortraitTexture()
				!= FullCard->GetResolvedPortraitTexture()
			&& FullCard->GetResolvedHandMicroPortraitTexture()
				!= PitchMini->GetResolvedPortraitTexture()
			&& PitchMini->GetConfiguredDimensions() == FVector2D(136.0f, 140.0f));

	Screen->RequestStartNewMatch();
	Screen->RefreshFromPresentation(Screen->GetPresentation());
	Screen->ForceLayoutPrepass();
	auto AuditRack = [](const UFMCodexCardRackWidget* Rack) -> bool
	{
		if (Rack == nullptr || Rack->GetRenderedCellCount() != 20
			|| Rack->GetWidgetFromName(TEXT("CardRackScrollBox")) != nullptr
			|| Rack->GetWidgetFromName(TEXT("CardRackPageControl")) != nullptr)
		{
			return false;
		}
		for (const UFMCodexPlayerCardWidget* Card : Rack->GetRenderedCardWidgets())
		{
			if (Card == nullptr
				|| Card->GetConfiguredDimensions() != FVector2D(220.0f, 68.0f))
			{
				return false;
			}
		}
		return Rack->GetDesiredSize().Y <= 880.0f;
	};
	const USizeBox* HeaderBounds = Cast<USizeBox>(
		Screen->GetWidgetFromName(TEXT("BroadcastMatchHeaderRegion")));
	const USizeBox* MainBounds = Cast<USizeBox>(
		Screen->GetWidgetFromName(TEXT("GoldenLayoutMainMatchArea")));
	const USizeBox* DockBounds = Cast<USizeBox>(
		Screen->GetWidgetFromName(TEXT("ContextActionDockRegion")));
	const USizeBox* PitchBounds = Cast<USizeBox>(
		Screen->GetWidgetFromName(TEXT("CentralPitchRegion")));
	TestTrue(TEXT("220x68 preserves 10-row 1080p no-scroll fit and macro layout"),
		AuditRack(Screen->GetLocalRackWidget())
			&& AuditRack(Screen->GetOpponentRackWidget())
			&& 10.0f * 68.0f + 9.0f * 8.0f == 752.0f
			&& HeaderBounds != nullptr && HeaderBounds->GetHeightOverride() == 80.0f
			&& MainBounds != nullptr && MainBounds->GetHeightOverride() == 880.0f
			&& DockBounds != nullptr && DockBounds->GetHeightOverride() == 120.0f
			&& PitchBounds != nullptr
			&& PitchBounds->GetWidthOverride()
				== FMCodexHandMicroDiagnostics::PitchWidth);

	FString GeneratorSource;
	FString AssetSource;
	FString CardSource;
	FString RackSource;
	FString PitchSource;
	FString HeaderSource;
	FString DockSource;
	FString HostSource;
	FString SessionSource;
	TestTrue(TEXT("Rollout provenance and isolation sources load"),
		LoadProductionSource(
			TEXT("Scripts/GenerateHandMicroPortraits.py"),
			GeneratorSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerUIAssetReferences.cpp"),
				AssetSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
				CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexCardRackWidget.cpp"), RackSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchWidget.cpp"), PitchSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexMatchHeaderWidget.cpp"), HeaderSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexInteractionPanelWidget.cpp"), DockSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchHostGameMode.cpp"), HostSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
				SessionSource));
	const int32 ProductionBindingStart = AssetSource.Find(
		TEXT("PrototypeHandMicroPortraits = {"));
	const int32 DiagnosticBindingStart = AssetSource.Find(
		TEXT("HandMicroValidationPortraits = {"));
	FString RuntimeBindingCount = ProductionBindingStart != INDEX_NONE
		&& DiagnosticBindingStart > ProductionBindingStart
		? AssetSource.Mid(ProductionBindingStart,
			DiagnosticBindingStart - ProductionBindingStart)
		: FString();
	const int32 BoundPathTokenCount = RuntimeBindingCount.ReplaceInline(
		TEXT("ApprovedRuntime192"), TEXT(""), ESearchCase::CaseSensitive);
	FString SessionCountSource = SessionSource;
	const int32 SerializedEntrypointCount = SessionCountSource.ReplaceInline(
		TEXT("ExecuteSerialized<"), TEXT(""), ESearchCase::CaseSensitive);
	TestTrue(TEXT("Canonical generator is deterministic and freezes all 16 outputs"),
		GeneratorSource.Contains(TEXT("CANDIDATES ="))
			&& GeneratorSource.Contains(TEXT("EXPECTED_HASHES ="))
			&& GeneratorSource.Contains(TEXT("crop_width * 2 != crop_height * 3"))
			&& GeneratorSource.Contains(TEXT("Image.Resampling.LANCZOS"))
			&& GeneratorSource.Contains(TEXT("sha256(runtime_output) != expected_runtime"))
			&& GeneratorSource.Contains(TEXT("hashes_verified"))
			&& GeneratorSource.Contains(TEXT("sharpen=none"))
			&& GeneratorSource.Contains(TEXT("reconstruction=none"))
			&& GeneratorSource.Contains(TEXT("runtime_transform=none"))
			&& !GeneratorSource.Contains(TEXT("UnsharpMask"))
			&& !GeneratorSource.Contains(TEXT("ImageEnhance")));
	TestTrue(TEXT("Production rollout remains exactly 16 Hand-Micro-only bindings"),
		ProductionBindingStart != INDEX_NONE
			&& DiagnosticBindingStart != INDEX_NONE
			&& BoundPathTokenCount == 32
			&& !PitchSource.Contains(TEXT("HandMicroApprovedRollout"))
			&& !HeaderSource.Contains(TEXT("HandMicroApprovedRollout"))
			&& !DockSource.Contains(TEXT("HandMicroApprovedRollout"))
			&& !HostSource.Contains(TEXT("HandMicroApprovedRollout"))
			&& !RackSource.Contains(TEXT("SetRenderTransform"))
			&& !RackSource.Contains(TEXT("SetRenderScale"))
			&& !CardSource.Contains(TEXT("HandMicroNameFont.Size = 11")));
	TestEqual(TEXT("Authority typed serialized entrypoint contract remains 42/42"),
		SerializedEntrypointCount, 42);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexMatchScreenInteractionUXContractTest,
	"FMCodex.LocalPlay.ControlSurface.40.MatchScreenInteractionUXContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexMatchScreenInteractionUXContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Interaction UX Host exists"), Host);
	TestNotNull(TEXT("Interaction UX Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Interaction UX Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	AcknowledgeIfPending(*Controller);
	Screen->RequestRollTacticalPoints();
	AcknowledgeIfPending(*Controller);

	UFMCodexCardRackWidget* LocalRack = Screen->GetLocalRackWidget();
	UFMCodexCardRackWidget* OpponentRack = Screen->GetOpponentRackWidget();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	TestNotNull(TEXT("Interaction UX local Rack exists"), LocalRack);
	TestNotNull(TEXT("Interaction UX opponent Rack exists"), OpponentRack);
	TestNotNull(TEXT("Interaction UX Pitch exists"), Pitch);
	if (LocalRack == nullptr || OpponentRack == nullptr || Pitch == nullptr
		|| LocalRack->GetRenderedCardWidgets().IsEmpty()
		|| OpponentRack->GetRenderedCardWidgets().IsEmpty())
	{
		return false;
	}

	UFMCodexPlayerCardWidget* LocalHoverCard =
		LocalRack->GetRenderedCardWidgets()[0];
	UFMCodexPlayerCardWidget* OpponentHoverCard =
		OpponentRack->GetRenderedCardWidgets()[0];
	TestTrue(TEXT("Populated local and opponent Hand Micro expose detail"),
		LocalHoverCard != nullptr && OpponentHoverCard != nullptr
			&& LocalHoverCard->CanExposeFullCardDetail()
			&& OpponentHoverCard->CanExposeFullCardDetail());
	if (LocalHoverCard == nullptr || OpponentHoverCard == nullptr)
	{
		return false;
	}

	LocalHoverCard->OnDetailHoverRequested.Broadcast(LocalHoverCard);
	UFMCodexPlayerCardWidget* DetailCard = Screen->GetDetailOverlayCard();
	TestTrue(TEXT("Local Hand hover reuses one hit-test-invisible Full Card"),
		Screen->IsDetailOverlayVisible()
			&& Screen->IsDetailOverlayHitTestInvisible()
			&& Screen->GetInteractionState()
				== EFMCodexUMGCardInteractionState::Hover
			&& DetailCard != nullptr
			&& DetailCard->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::InteractionChoice
			&& DetailCard->GetPresentation().CardId
				== LocalHoverCard->GetPresentation().CardId
			&& DetailCard->GetConfiguredDimensions()
				== FVector2D(360.0f, 540.0f));
	UFMCodexPlayerCardWidget* FirstDetailInstance = DetailCard;
	OpponentHoverCard->OnDetailHoverRequested.Broadcast(OpponentHoverCard);
	TestTrue(TEXT("Opponent hover updates the same Full Card instance"),
		Screen->IsDetailOverlayVisible()
			&& Screen->GetDetailOverlayCard() == FirstDetailInstance
			&& Screen->GetDetailOverlayCard()->GetPresentation().CardId
				== OpponentHoverCard->GetPresentation().CardId);
	LocalHoverCard->OnDetailHoverDismissed.Broadcast(LocalHoverCard);
	TestTrue(TEXT("Stale source leave cannot dismiss the newer hover"),
		Screen->IsDetailOverlayVisible());
	OpponentHoverCard->OnDetailHoverDismissed.Broadcast(OpponentHoverCard);
	TestFalse(TEXT("Active source leave dismisses detail"),
		Screen->IsDetailOverlayVisible());

	FFMCodexUMGCardViewModel EmptyModel;
	UFMCodexPlayerCardWidget* EmptyCard =
		CreateWidget<UFMCodexPlayerCardWidget>(
			Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
	EmptyCard->RefreshFromPresentation(
		EmptyModel, EFMCodexPlayerCardPresentationMode::HandMicro);
	TestFalse(TEXT("Empty card cannot expose Full Card detail"),
		EmptyCard->CanExposeFullCardDetail());

	const FVector2D LeftPosition =
		UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
			FVector2D(20.0f, 4.0f), FVector2D(220.0f, 68.0f),
			FVector2D(1920.0f, 1080.0f), true);
	const FVector2D RightPosition =
		UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
			FVector2D(1680.0f, 1010.0f), FVector2D(220.0f, 68.0f),
			FVector2D(1920.0f, 1080.0f), false);
	const FVector2D LeftBottomPosition =
		UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
			FVector2D(20.0f, 1010.0f), FVector2D(220.0f, 68.0f),
			FVector2D(1920.0f, 1080.0f), true);
	const FVector2D RightTopPosition =
		UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
			FVector2D(1680.0f, 4.0f), FVector2D(220.0f, 68.0f),
			FVector2D(1920.0f, 1080.0f), false);
	const FVector2D PitchPosition =
		UFMCodexLocalMatchScreenWidget::CalculateDetailOverlayPosition(
			FVector2D(860.0f, 360.0f), FVector2D(136.0f, 140.0f),
			FVector2D(1920.0f, 1080.0f), true);
	TestTrue(TEXT("Detail placement opens inward and clamps to viewport"),
		LeftPosition.X == 256.0f && LeftPosition.Y == 12.0f
			&& RightPosition.X < 1680.0f
			&& RightPosition.X >= 12.0f
			&& RightPosition.Y == 528.0f
			&& RightPosition.X + 360.0f <= 1908.0f
			&& LeftBottomPosition == FVector2D(256.0f, 528.0f)
			&& RightTopPosition == FVector2D(1304.0f, 12.0f)
			&& PitchPosition == FVector2D(1012.0f, 160.0f));

	const FFMCodexUMGDeploymentChoiceViewModel* DeploymentChoice =
		Screen->GetPresentation().Interaction.DeploymentChoices.FindByPredicate(
			[](const FFMCodexUMGDeploymentChoiceViewModel& Candidate)
			{
				return !Candidate.bGoalkeeper
					&& !Candidate.Destinations.IsEmpty();
			});
	TestNotNull(TEXT("Authoritative interaction exposes ordinary deployment"),
		DeploymentChoice);
	if (DeploymentChoice == nullptr)
	{
		return false;
	}
	const FFMCodexUMGDeploymentChoiceViewModel SelectedDeployment =
		*DeploymentChoice;
	UFMCodexPlayerCardWidget* DragSource = nullptr;
	for (UFMCodexPlayerCardWidget* Card : LocalRack->GetRenderedCardWidgets())
	{
		if (Card != nullptr
			&& Card->GetDeploymentDragCardId() == SelectedDeployment.CardId)
		{
			DragSource = Card;
			break;
		}
	}
	TestNotNull(TEXT("Eligible local Hand card is the drag source"), DragSource);
	if (DragSource == nullptr)
	{
		return false;
	}
	const int32 StableSourceIndex =
		Screen->GetPresentation().LocalRack.Cells.IndexOfByPredicate(
			[&SelectedDeployment](const FFMCodexUMGCardRackCellViewModel& Cell)
			{
				return Cell.Card.CardId == SelectedDeployment.CardId;
			});
	DragSource->OnDetailHoverRequested.Broadcast(DragSource);
	UFMCodexDeploymentDragDropOperation* CancelledOperation =
		DragSource->BeginDeploymentDrag();
	TestNotNull(TEXT("Drag creates typed operation"), CancelledOperation);
	if (CancelledOperation == nullptr)
	{
		return false;
	}
	UFMCodexPlayerCardWidget* HandMicroDragProxy =
		Cast<UFMCodexPlayerCardWidget>(CancelledOperation->DefaultDragVisual);
	TestTrue(TEXT("Drag suppresses hover and reserves the exact source slot"),
		!Screen->IsDetailOverlayVisible()
			&& Screen->GetInteractionState()
				== EFMCodexUMGCardInteractionState::Dragging
			&& DragSource->IsDragSourcePresentationActive()
			&& DragSource->GetInteractionState()
				== EFMCodexUMGCardInteractionState::DragSource
			&& FMath::IsNearlyEqual(DragSource->GetRenderOpacity(), 0.28f)
			&& LocalRack->GetRenderedCellCount() == 20
			&& LocalRack->GetRenderedCardWidgets().Contains(DragSource));
	TestTrue(TEXT("Drag proxy is a uniform Hand Micro identification surface"),
		HandMicroDragProxy != nullptr
			&& HandMicroDragProxy->GetPresentation().CardId
				== SelectedDeployment.CardId
			&& HandMicroDragProxy->GetPresentationMode()
				== EFMCodexPlayerCardPresentationMode::HandMicro
			&& HandMicroDragProxy->GetConfiguredDimensions()
				== FVector2D(220.0f, 68.0f)
			&& HandMicroDragProxy->GetRenderTransform().Scale.Equals(
				FVector2D(1.10f, 1.10f), 0.001f)
			&& (HandMicroDragProxy->GetConfiguredDimensions()
				* HandMicroDragProxy->GetRenderTransform().Scale)
				.Equals(FVector2D(242.0f, 74.8f), 0.01f)
			&& CancelledOperation->Pivot == EDragPivot::CenterLeft
			&& HandMicroDragProxy->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& FMath::IsNearlyEqual(
				HandMicroDragProxy->GetRenderOpacity(), 0.98f));
	if (HandMicroDragProxy != nullptr)
	{
		TestFalse(TEXT("Drag proxy omits Overall"),
			HandMicroDragProxy->IsOverallVisible());
		TestFalse(TEXT("Drag proxy omits Serial"),
			HandMicroDragProxy->IsPlayerFacingSerialVisible());
		TestEqual(TEXT("Drag proxy omits Biography"),
			HandMicroDragProxy->GetRenderedBiographyRowCount(), 0);
		TestEqual(TEXT("Drag proxy omits Attributes"),
			HandMicroDragProxy->GetRenderedAttributeCount(), 0);
		TestEqual(TEXT("Drag proxy omits Skills"),
			HandMicroDragProxy->GetRenderedSkillCount(), 0);
		TestFalse(TEXT("Drag proxy omits developer reference"),
			HandMicroDragProxy->IsDeveloperReferenceVisible());
		TestFalse(TEXT("Drag proxy omits owner/team diagnostics"),
			HandMicroDragProxy->IsOwnerVisible()
				|| HandMicroDragProxy->IsTeamVisible());
		TestFalse(TEXT("Drag proxy omits Full Card role icon"),
			HandMicroDragProxy->IsRoleIconVisible());
		TestEqual(TEXT("Drag proxy preserves the source Hand Micro portrait binding"),
			HandMicroDragProxy->GetResolvedHandMicroPortraitTexture(),
			DragSource->GetResolvedHandMicroPortraitTexture());
	}

	auto FindPitchSlot = [Pitch](const FName SlotId)
		-> UFMCodexPitchSlotWidget*
	{
		for (UFMCodexPitchSlotWidget* SlotWidget
			: Pitch->GetRenderedSlotWidgets())
		{
			if (SlotWidget != nullptr
				&& SlotWidget->GetPresentation().SlotId == SlotId)
			{
				return SlotWidget;
			}
		}
		return nullptr;
	};
	UFMCodexPitchSlotWidget* LegalTarget = FindPitchSlot(
		SelectedDeployment.Destinations[0].SlotId);
	UFMCodexPitchSlotWidget* InvalidTarget = nullptr;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Pitch->GetRenderedSlotWidgets())
	{
		if (SlotWidget != nullptr
			&& SlotWidget->GetPresentation().DeploymentTargetState
				== EFMCodexUMGDeploymentTargetState::Invalid)
		{
			InvalidTarget = SlotWidget;
			break;
		}
	}
	if (InvalidTarget == nullptr)
	{
		FFMCodexUMGPitchSlotViewModel InvalidPresentation;
		InvalidPresentation.SlotId = TEXT("InteractionUX.InvalidTarget");
		InvalidPresentation.DeploymentTargetCardId = SelectedDeployment.CardId;
		InvalidPresentation.DeploymentTargetState =
			EFMCodexUMGDeploymentTargetState::Invalid;
		InvalidTarget = CreateWidget<UFMCodexPitchSlotWidget>(
			Screen->GetWorld(), UFMCodexPitchSlotWidget::StaticClass());
		InvalidTarget->RefreshFromPitchSlotPresentation(InvalidPresentation);
		InvalidTarget->TakeWidget();
	}
	TestNotNull(TEXT("Legal target exists"), LegalTarget);
	TestNotNull(TEXT("Non-legal target exists"), InvalidTarget);
	if (LegalTarget == nullptr || InvalidTarget == nullptr)
	{
		return false;
	}
	LegalTarget->SetDeploymentDragHovered(CancelledOperation, true);
	InvalidTarget->SetDeploymentDragHovered(CancelledOperation, true);
	TestTrue(TEXT("Only authoritative legal target receives active cue"),
		LegalTarget->IsDeploymentDragHovered()
			&& LegalTarget->GetInteractionState()
				== EFMCodexUMGCardInteractionState::DragOverLegalSlot
			&& !LegalTarget->IsDeploymentTargetLabelVisible()
			&& !InvalidTarget->IsDeploymentDragHovered()
			&& !InvalidTarget->IsDeploymentTargetLabelVisible());

	const TArray<uint8> StateBeforeCancel =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	const FString CommandBeforeCancel = Controller->GetLastDiagnostic().CommandName;
	CancelledOperation->DragCancelled(FPointerEvent());
	TestTrue(TEXT("Cancel restores source and clears all temporary UX"),
		!DragSource->IsDragSourcePresentationActive()
			&& FMath::IsNearlyEqual(DragSource->GetRenderOpacity(), 1.0f)
			&& Screen->GetLastCompletedDragState()
				== EFMCodexUMGCardInteractionState::DropCancelled
			&& Pitch->GetActiveDeploymentCardId().IsNone()
			&& !Screen->IsDetailOverlayVisible()
			&& Controller->GetLastDiagnostic().CommandName == CommandBeforeCancel
			&& SerializeState(Host->GetMatchSnapshot().Snapshot)
				== StateBeforeCancel);

	UFMCodexDeploymentDragDropOperation* SuccessOperation =
		DragSource->BeginDeploymentDrag();
	if (SuccessOperation == nullptr)
	{
		return false;
	}
	LegalTarget = FindPitchSlot(SelectedDeployment.Destinations[0].SlotId);
	TestNotNull(TEXT("Legal target is rebuilt for the second drag"), LegalTarget);
	if (LegalTarget == nullptr)
	{
		return false;
	}
	const int32 PlacementsBefore =
		Host->GetMatchSnapshot().Snapshot.CurrentAttack.DeploymentPlacements.Num();
	TestTrue(TEXT("Success uses existing typed authoritative deployment path"),
		LegalTarget->TryHandleDeploymentDrop(SuccessOperation)
			&& Controller->GetLastDiagnostic().CommandName
				== TEXT("DeployOrdinary")
			&& Controller->GetLastDiagnostic().bHostSuccess
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack
				.DeploymentPlacements.Num() == PlacementsBefore + 1);
	SuccessOperation->Drop(FPointerEvent());
	TestEqual(TEXT("Successful operation records DropSuccess presentation state"),
		Screen->GetLastCompletedDragState(),
		EFMCodexUMGCardInteractionState::DropSuccess);
	TestTrue(TEXT("Successful operation clears active pitch projection"),
		Pitch->GetActiveDeploymentCardId().IsNone());
	TestTrue(TEXT("Source Rack index remains stable"),
		StableSourceIndex != INDEX_NONE);
	auto HasPermanentGhost = [&SelectedDeployment](
		const FFMCodexUMGCardRackViewModel& RackPresentation,
		UFMCodexCardRackWidget* RackWidget)
	{
		if (RackWidget == nullptr)
		{
			return false;
		}
		const FFMCodexUMGCardRackCellViewModel* PlayedCell =
			RackPresentation.Cells.FindByPredicate(
				[&SelectedDeployment](
					const FFMCodexUMGCardRackCellViewModel& Cell)
				{
					return Cell.Card.CardId == SelectedDeployment.CardId
						&& Cell.bPlayed;
				});
		return PlayedCell != nullptr
			&& RackWidget->GetWidgetFromName(FName(*FString::Printf(
				TEXT("PlayedCardGhostBounds%d"), PlayedCell->StableIndex))) != nullptr
			&& RackWidget->GetCellInteractionState(PlayedCell->StableIndex)
				== EFMCodexUMGCardInteractionState::Ghost
			&& RackWidget->GetRenderedCellCount() == 20;
	};
	TestTrue(TEXT("Successful deployment leaves permanent owner-Rack Ghost"),
		HasPermanentGhost(
			Screen->GetPresentation().LocalRack, Screen->GetLocalRackWidget())
			|| HasPermanentGhost(
				Screen->GetPresentation().OpponentRack,
				Screen->GetOpponentRackWidget()));

	UFMCodexPlayerCardWidget* DeployedPitchCard = nullptr;
	for (UFMCodexPitchSlotWidget* SlotWidget
		: Screen->GetPitchWidget()->GetRenderedSlotWidgets())
	{
		if (SlotWidget != nullptr)
		{
			SlotWidget->TakeWidget();
		}
		if (SlotWidget != nullptr && SlotWidget->GetCardWidget() != nullptr
			&& SlotWidget->GetCardWidget()->GetPresentation().CardId
				== SelectedDeployment.CardId)
		{
			DeployedPitchCard = SlotWidget->GetCardWidget();
			break;
		}
	}
	TestNotNull(TEXT("Authoritative refresh renders deployed Pitch card"),
		DeployedPitchCard);
	if (DeployedPitchCard != nullptr)
	{
		DeployedPitchCard->OnDetailHoverRequested.Broadcast(DeployedPitchCard);
		TestTrue(TEXT("Deployed Pitch hover reuses the same transient Full Card"),
			Screen->IsDetailOverlayVisible()
				&& Screen->GetDetailOverlayCard() == FirstDetailInstance
				&& Screen->GetDetailOverlayCard()->GetPresentation().CardId
					== SelectedDeployment.CardId
				&& DeployedPitchCard->GetPresentationMode()
					== EFMCodexPlayerCardPresentationMode::PitchMini
				&& DeployedPitchCard->GetInteractionState()
					== EFMCodexUMGCardInteractionState::Deployed
				&& DeployedPitchCard->GetConfiguredDimensions()
					== FVector2D(136.0f, 140.0f));
	}

	FString CardSource;
	FString PitchSlotSource;
	FString ScreenSource;
	FString SessionSource;
	TestTrue(TEXT("Interaction UX production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexPitchSlotWidget.cpp"),
				PitchSlotSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ScreenSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/MatchPlayRuntime/MatchPlayAuthoritativeSession.cpp"),
				SessionSource));
	const FString UXSource = CardSource + PitchSlotSource + ScreenSource;
	TestTrue(TEXT("UMG consumes DTO legality and contains no rule derivation"),
		ScreenSource.Contains(TEXT("Presentation.Interaction.DeploymentChoices"))
			&& PitchSlotSource.Contains(
				TEXT("Presentation.DeploymentTargetState"))
			&& !UXSource.Contains(TEXT("RelativeDeploymentZoneResolver"))
			&& !UXSource.Contains(TEXT("FMatchPlayState"))
			&& !UXSource.Contains(TEXT("FormulaResolver")));
	FString SessionCountSource = SessionSource;
	const int32 SerializedEntrypointCount = SessionCountSource.ReplaceInline(
		TEXT("ExecuteSerialized<"), TEXT(""), ESearchCase::CaseSensitive);
	TestEqual(TEXT("Authority typed serialized entrypoint remains 42/42"),
		SerializedEntrypointCount, 42);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInMatchFullCardProductionFoundationContractTest,
	"FMCodex.LocalPlay.ControlSurface.41.InMatchFullCardProductionFoundationContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInMatchFullCardProductionFoundationContractTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("Full Card production Host exists"), Host);
	TestNotNull(TEXT("Full Card production Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("Full Card production Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	AcknowledgeIfPending(*Controller);
	Screen->RequestRollTacticalPoints();
	AcknowledgeIfPending(*Controller);

	TArray<FFMCodexUMGCardViewModel> LiveCards;
	auto CollectLiveCards = [&LiveCards](
		const FFMCodexUMGCardRackViewModel& Rack)
	{
		for (const FFMCodexUMGCardRackCellViewModel& Cell : Rack.Cells)
		{
			if (!Cell.Card.CardId.IsNone()
				&& !LiveCards.ContainsByPredicate(
					[&Cell](const FFMCodexUMGCardViewModel& Existing)
					{
						return Existing.CardId == Cell.Card.CardId;
					}))
			{
				LiveCards.Add(Cell.Card);
			}
		}
	};
	CollectLiveCards(Screen->GetPresentation().LocalRack);
	CollectLiveCards(Screen->GetPresentation().OpponentRack);
	const FFMCodexUMGCardViewModel* Outfield = LiveCards.FindByPredicate(
		[](const FFMCodexUMGCardViewModel& Card)
		{
			return Card.CardId == TEXT("Prototype.Arsenal.BukayoSaka");
		});
	const FFMCodexUMGCardViewModel* Goalkeeper = LiveCards.FindByPredicate(
		[](const FFMCodexUMGCardViewModel& Card)
		{
			return Card.bGoalkeeper && Card.AttributeValues.Num() == 6;
		});
	const FFMCodexUMGCardViewModel* IntegratedContentCard =
		LiveCards.FindByPredicate([](const FFMCodexUMGCardViewModel& Card)
		{
			return Card.CardId
				== TEXT("Prototype.Arsenal.GabrielMartinelli");
		});
	const FFMCodexUMGCardViewModel* GabrielNoSkill =
		LiveCards.FindByPredicate([](const FFMCodexUMGCardViewModel& Card)
		{
			return Card.CardId
				== TEXT("Prototype.Arsenal.GabrielMagalhaes");
		});
	const FFMCodexUMGCardViewModel* Donnarumma =
		LiveCards.FindByPredicate([](const FFMCodexUMGCardViewModel& Card)
		{
			return Card.CardId
				== TEXT("Prototype.ManchesterCity.GianluigiDonnarumma");
		});
	TestTrue(TEXT("Live racks expose dynamic real player prototypes"),
		LiveCards.Num() >= 16 && Outfield != nullptr && Goalkeeper != nullptr
			&& IntegratedContentCard != nullptr && GabrielNoSkill != nullptr
			&& Donnarumma != nullptr);
	if (Outfield == nullptr || Goalkeeper == nullptr
		|| IntegratedContentCard == nullptr || GabrielNoSkill == nullptr
		|| Donnarumma == nullptr)
	{
		return false;
	}

	auto CreateFullCard = [Screen](const FFMCodexUMGCardViewModel& Model)
	{
		UFMCodexPlayerCardWidget* Card =
			CreateWidget<UFMCodexPlayerCardWidget>(
				Screen->GetWorld(), UFMCodexPlayerCardWidget::StaticClass());
		if (Card != nullptr)
		{
			Card->RefreshFromPresentation(Model,
				EFMCodexPlayerCardPresentationMode::InteractionChoice);
			Card->TakeWidget();
		}
		return Card;
	};
	UFMCodexPlayerCardWidget* OutfieldCard = CreateFullCard(*Outfield);
	UFMCodexPlayerCardWidget* GoalkeeperCard = CreateFullCard(*Goalkeeper);
	UFMCodexPlayerCardWidget* IntegratedContentFullCard =
		CreateFullCard(*IntegratedContentCard);
	UFMCodexPlayerCardWidget* GabrielNoSkillCard =
		CreateFullCard(*GabrielNoSkill);
	UFMCodexPlayerCardWidget* DonnarummaCard = CreateFullCard(*Donnarumma);
	TestNotNull(TEXT("Outfield Full Card constructs"), OutfieldCard);
	TestNotNull(TEXT("Goalkeeper Full Card constructs"), GoalkeeperCard);
	TestNotNull(TEXT("Integrated-content Full Card constructs"),
		IntegratedContentFullCard);
	TestNotNull(TEXT("Gabriel no-Skill Full Card constructs"),
		GabrielNoSkillCard);
	TestNotNull(TEXT("Donnarumma long-name Full Card constructs"),
		DonnarummaCard);
	if (OutfieldCard == nullptr || GoalkeeperCard == nullptr
		|| IntegratedContentFullCard == nullptr || GabrielNoSkillCard == nullptr
		|| DonnarummaCard == nullptr)
	{
		return false;
	}
	TestTrue(TEXT("Integrated Full Card-only Hero Bust preserves data and Hand Micro isolation"),
		IntegratedContentFullCard->GetRenderedIdentityText().ToString()
				== TEXT("马丁内利")
			&& IntegratedContentFullCard->GetWidgetFromName(
				TEXT("CardIdentityRegion"))->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& IntegratedContentFullCard->GetResolvedPortraitTexture() != nullptr
			&& IntegratedContentFullCard->GetResolvedPortraitTexture()
				->GetPathName().Contains(TEXT(
					"T_Prototype_Arsenal_GabrielMartinelli_FullCardHeroBust_01"))
			&& IntegratedContentFullCard->GetResolvedHandMicroPortraitTexture()
				!= nullptr
			&& IntegratedContentFullCard->GetResolvedHandMicroPortraitTexture()
				!= IntegratedContentFullCard->GetResolvedPortraitTexture()
			&& FFMCodexPlayerUIPresentationText::HandMicroPlayerName(
				IntegratedContentCard->CardId,
				IntegratedContentCard->IdentityLabel).ToString()
				== TEXT("\u9A6C\u4E01\u5185\u5229"));

	TestTrue(TEXT("Full Card keeps canonical 360x540 portrait geometry"),
		OutfieldCard->GetConfiguredDimensions().Equals(
			FVector2D(360.0f, 540.0f), 0.01f)
			&& UFMCodexLocalMatchScreenWidget::GetCanonicalFullCardDimensions()
				.Equals(FVector2D(360.0f, 540.0f), 0.01f)
			&& FMath::IsNearlyEqual(
				OutfieldCard->GetConfiguredDimensions().X
					/ OutfieldCard->GetConfiguredDimensions().Y,
				2.0 / 3.0, 0.0001));
	const USizeBox* PortraitBounds = Cast<USizeBox>(
		OutfieldCard->GetWidgetFromName(TEXT("PortraitAssetBounds")));
	TestTrue(TEXT("Dedicated Full Card portrait owns a large hero region"),
		PortraitBounds != nullptr
			&& FMath::IsNearlyEqual(PortraitBounds->GetHeightOverride(), 320.0f)
			&& OutfieldCard->GetResolvedPortraitTexture() != nullptr
			&& OutfieldCard->GetResolvedHandMicroPortraitTexture() != nullptr
			&& OutfieldCard->GetResolvedPortraitTexture()
				!= OutfieldCard->GetResolvedHandMicroPortraitTexture()
			&& !OutfieldCard->GetResolvedPortraitTexture()->GetPathName().Contains(
				TEXT("Runtime192"))
			&& OutfieldCard->GetResolvedHandMicroPortraitTexture()->GetPathName()
				.Contains(TEXT("Runtime192")));

	const UBorder* Frame = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("PlayerCardFrame")));
	const UBorder* RarityRail = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("InMatchFullCardRarityRail")));
	const UBorder* InnerFrame = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("InMatchFullCardInnerFrame")));
	const UBorder* IdentityAccent = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("FullCardIdentityAccent")));
	const UTextBlock* NameText = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("CardIdentity")));
	const UTextBlock* IdentitySupplement = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("FullCardIdentitySupplement")));
	const UTextBlock* PositionText = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("CardRole")));
	const UTextBlock* RarityText = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("CardRarity")));
	const UTextBlock* OverallNumber = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("OverallNumber")));
	const UTextBlock* OverallLabel = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("OverallLabel")));
	const UTextBlock* SerialText = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("PlayerFacingCardSerial")));
	const FLinearColor ExpectedRarity =
		FFMCodexPlayerUIStyle::Get().GetRarityAccentColor(
			Outfield->RarityLabel);
	TestTrue(TEXT("Full Card uses stable deep navy plus non-text rarity accents"),
		Frame != nullptr && RarityRail != nullptr && InnerFrame != nullptr
			&& IdentityAccent != nullptr && NameText != nullptr
			&& IdentitySupplement != nullptr
			&& PositionText != nullptr && RarityText != nullptr
			&& OverallNumber != nullptr && OverallLabel != nullptr
			&& SerialText != nullptr
			&& OutfieldCard->GetFullCardBaseSurfaceColor().ToFColorSRGB()
				== FColor(0x07, 0x15, 0x21)
			&& Frame->GetBrushColor().ToFColorSRGB()
				== ExpectedRarity.ToFColorSRGB()
			&& RarityRail->GetBrushColor().ToFColorSRGB()
				== ExpectedRarity.ToFColorSRGB()
			&& NameText->GetColorAndOpacity().GetSpecifiedColor().ToFColorSRGB()
				== FColor(0xF2, 0xF3, 0xF1)
			&& PositionText->GetColorAndOpacity().GetSpecifiedColor().ToFColorSRGB()
				== FColor(0xEE, 0xF1, 0xF0)
			&& RarityText->GetVisibility() == ESlateVisibility::Collapsed
			&& OutfieldCard->GetRenderedRarityText().IsEmpty()
			&& OverallNumber->GetColorAndOpacity().GetSpecifiedColor()
				.ToFColorSRGB() == ExpectedRarity.ToFColorSRGB()
			&& OverallLabel->GetColorAndOpacity().GetSpecifiedColor()
				.ToFColorSRGB() == FColor(0xD4, 0xD9, 0xD8)
			&& SerialText->GetColorAndOpacity().GetSpecifiedColor()
				.ToFColorSRGB() == ExpectedRarity.ToFColorSRGB());
	TestTrue(TEXT("Full Card uses a restrained layered manufacturing frame"),
		InnerFrame != nullptr && IdentityAccent != nullptr
			&& InnerFrame->GetPadding() == FMargin(1.0f)
			&& InnerFrame->GetBrushColor().A < 0.60f
			&& IdentityAccent->GetBrushColor().A < 0.60f
			&& OutfieldCard->GetWidgetFromName(
				TEXT("AttributeSectionRuleLeft")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("AttributeSectionRuleRight")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("SkillSectionRuleLeft")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("SkillSectionRuleRight")) != nullptr);

	TestTrue(TEXT("Normal Full Card visibility contains no technical metadata"),
		!OutfieldCard->IsDeveloperReferenceVisible()
			&& !OutfieldCard->IsOwnerVisible()
			&& !OutfieldCard->IsTeamVisible()
			&& !OutfieldCard->IsRoleIconVisible()
			&& !OutfieldCard->GetRenderedIdentityText().ToString().Contains(
				TEXT("Demo."))
			&& !OutfieldCard->GetRenderedIdentityText().ToString().Contains(
				TEXT("Prototype."))
			&& !OutfieldCard->GetRenderedIdentityText().ToString().Contains(
				TEXT("Card "))
			&& OutfieldCard->GetWidgetFromName(TEXT("CardType")) == nullptr
			&& OutfieldCard->GetWidgetFromName(TEXT("PreferredFoot")) == nullptr
			&& OutfieldCard->GetWidgetFromName(TEXT("AttributeIcon0")) == nullptr
			&& OutfieldCard->GetWidgetFromName(TEXT("FullCardSkillIcon0")) == nullptr);

	TestTrue(TEXT("Approved factual and presentation data populate the Full Card DTO"),
		Outfield->BirthDate == TEXT("2001-09-05")
			&& Outfield->HeightCm == 178 && Outfield->WeightKg == 72
			&& Outfield->bHasOverallRating && Outfield->OverallRating == 100
			&& Outfield->EnglishIdentityLabel == TEXT("Bukayo Saka")
			&& Outfield->NationalityLabel == TEXT("英格兰")
			&& Outfield->ClubLabel == TEXT("阿森纳")
			&& Outfield->PlayerFacingSerialLabel == TEXT("015")
			&& OutfieldCard->GetRenderedBiographyRowCount() == 4
			&& OutfieldCard->IsOverallVisible()
			&& OutfieldCard->IsPlayerFacingSerialVisible()
			&& OutfieldCard->GetWidgetFromName(
				TEXT("InMatchFullCardBiographyBounds"))->GetVisibility()
				== ESlateVisibility::HitTestInvisible
			&& OutfieldCard->GetWidgetFromName(TEXT("CardEnglishIdentity"))
				->GetVisibility() == ESlateVisibility::Collapsed
			&& IdentitySupplement != nullptr
			&& IdentitySupplement->GetText().ToString()
				== TEXT("国籍：英格兰  |  俱乐部：阿森纳")
			&& IdentitySupplement->GetVisibility()
				== ESlateVisibility::HitTestInvisible);
	const UTextBlock* BiographyPositionLabel = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("BiographyPositionLabel")));
	const UTextBlock* BiographyPositionValue = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("BiographyPositionValue")));
	const USizeBox* BiographyBounds = Cast<USizeBox>(
		OutfieldCard->GetWidgetFromName(
			TEXT("InMatchFullCardBiographyBounds")));
	const UBorder* BiographyRegion = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(
			TEXT("InMatchFullCardBiographyRegion")));
	const UBorder* BiographyPositionSurface = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("BiographyPosition")));
	const UBorder* BiographyBirthDateSurface = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("BiographyBirthDate")));
	const UVerticalBox* BiographyList = Cast<UVerticalBox>(
		OutfieldCard->GetWidgetFromName(TEXT("InMatchFullCardBiographyList")));
	const UOverlaySlot* BiographyRegionSlot = BiographyBounds != nullptr
		? Cast<UOverlaySlot>(BiographyBounds->Slot) : nullptr;
	const UBorder* FullCardIdentityRegion = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("CardIdentityRegion")));
	const UBorder* IdentityFadeTop = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("FullCardIdentityFadeTop")));
	const UBorder* IdentityFadeMiddle = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(TEXT("FullCardIdentityFadeMiddle")));
	const UBorder* IdentityReadabilityBase = Cast<UBorder>(
		OutfieldCard->GetWidgetFromName(
			TEXT("FullCardIdentityReadabilityBase")));
	const UVerticalBox* IdentityTextStack = Cast<UVerticalBox>(
		OutfieldCard->GetWidgetFromName(TEXT("CardIdentityTextStack")));
	const USizeBox* IdentityAccentBounds = Cast<USizeBox>(
		OutfieldCard->GetWidgetFromName(TEXT("FullCardIdentityAccentBounds")));
	const UOverlaySlot* IdentityRegionSlot = FullCardIdentityRegion != nullptr
		? Cast<UOverlaySlot>(FullCardIdentityRegion->Slot) : nullptr;
	const UTextBlock* FullCardOverall = Cast<UTextBlock>(
		OutfieldCard->GetWidgetFromName(TEXT("OverallNumber")));
	TestTrue(TEXT("PositionType completes the ordered biography metadata family"),
		OutfieldCard->GetWidgetFromName(TEXT("RoleRarityHeaderRegion"))
			->GetVisibility() == ESlateVisibility::Collapsed
			&& BiographyPositionLabel != nullptr
			&& BiographyPositionLabel->GetText().ToString() == TEXT("位置类型")
			&& BiographyPositionValue != nullptr
			&& BiographyPositionValue->GetText().EqualTo(
				OutfieldCard->GetRenderedPositionText())
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyHeightDivider")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyWeightDivider")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyPositionDivider")) != nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyFactsDivider")) == nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyHeightSeparator")) == nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyWeightSeparator")) == nullptr
			&& BiographyList != nullptr
			&& BiographyList->GetChildrenCount() == 7
			&& BiographyList->GetChildAt(0)->GetName()
				== TEXT("BiographyBirthDateBounds")
			&& BiographyList->GetChildAt(2)->GetName()
				== TEXT("BiographyHeightBounds")
			&& BiographyList->GetChildAt(4)->GetName()
				== TEXT("BiographyWeightBounds")
			&& BiographyList->GetChildAt(6)->GetName()
				== TEXT("BiographyPositionBounds"));
	TestTrue(TEXT("Portrait-first metadata treatment is open and tile-free"),
		BiographyBounds != nullptr
			&& FMath::IsNearlyEqual(BiographyBounds->GetWidthOverride(), 100.0f)
			&& BiographyRegion != nullptr
			&& BiographyRegion->GetBrushColor().A < 0.65f
			&& BiographyRegionSlot != nullptr
			&& BiographyRegionSlot->GetVerticalAlignment() == VAlign_Top
			&& BiographyPositionSurface != nullptr
			&& BiographyPositionSurface->GetBrushColor().A == 0.0f
			&& BiographyBirthDateSurface != nullptr
			&& BiographyBirthDateSurface->GetBrushColor().A == 0.0f
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyPositionMarker")) == nullptr
			&& OutfieldCard->GetWidgetFromName(
				TEXT("BiographyBirthDateMarker")) == nullptr
			&& IdentityRegionSlot != nullptr
			&& IdentityRegionSlot->GetPadding() == FMargin(0.0f)
			&& FullCardOverall != nullptr
			&& FullCardOverall->GetFont().Size == 44);
	TestTrue(TEXT("Portrait-backed identity uses a rising scrim, not a solid cut panel"),
		FullCardIdentityRegion != nullptr
			&& FullCardIdentityRegion->GetBrushColor().A == 0.0f
			&& IdentityFadeTop != nullptr
			&& IdentityFadeMiddle != nullptr
			&& IdentityReadabilityBase != nullptr
			&& IdentityFadeTop->GetBrushColor().A
				< IdentityFadeMiddle->GetBrushColor().A
			&& IdentityFadeMiddle->GetBrushColor().A
				< IdentityReadabilityBase->GetBrushColor().A
			&& IdentityReadabilityBase->GetBrushColor().A < 0.70f
			&& IdentityTextStack != nullptr
			&& IdentityAccentBounds != nullptr
			&& IdentityTextStack->GetChildrenCount() == 4
			&& IdentityTextStack->GetChildAt(3) == IdentityAccentBounds
			&& IdentityAccent != nullptr
			&& IdentityAccent->GetBrushColor().A <= 0.30f);

	const TSet<FString> OutfieldAttributeContract = {
		TEXT("SHO"), TEXT("DRI"), TEXT("PAS"), TEXT("OFF"), TEXT("MRK"),
		TEXT("TKL"), TEXT("SPD"), TEXT("STR"), TEXT("STA"), TEXT("LS") };
	const TSet<FString> GoalkeeperAttributeContract = {
		TEXT("HAN"), TEXT("POS"), TEXT("REF"), TEXT("AER"), TEXT("ANT"),
		TEXT("1V1") };
	auto HasExactAttributeContract = [](
		const FFMCodexUMGCardViewModel& Card,
		const TSet<FString>& Contract)
	{
		if (Card.AttributeValues.Num() != Contract.Num())
		{
			return false;
		}
		for (const FFMCodexUMGAttributeViewModel& Attribute
			: Card.AttributeValues)
		{
			if (!Contract.Contains(Attribute.CanonicalLabel)
				|| Attribute.Value < 1 || Attribute.Value > 6
				|| Attribute.CanonicalLabel.Equals(
					TEXT("Creativity"), ESearchCase::IgnoreCase))
			{
				return false;
			}
		}
		return true;
	};
	const UUniformGridPanel* OutfieldAttributeGrid = Cast<UUniformGridPanel>(
		OutfieldCard->GetWidgetFromName(TEXT("StructuredAttributeGrid")));
	const UUniformGridPanel* GoalkeeperAttributeGrid = Cast<UUniformGridPanel>(
		GoalkeeperCard->GetWidgetFromName(TEXT("StructuredAttributeGrid")));
	bool bFiveByTwoGrid = OutfieldAttributeGrid != nullptr
		&& OutfieldAttributeGrid->GetChildrenCount() == 10;
	if (bFiveByTwoGrid)
	{
		for (int32 Index = 0; Index < 10; ++Index)
		{
			const UUniformGridSlot* Slot = Cast<UUniformGridSlot>(
				OutfieldAttributeGrid->GetChildAt(Index)->Slot);
			bFiveByTwoGrid = bFiveByTwoGrid && Slot != nullptr
				&& Slot->GetRow() == Index / 5
				&& Slot->GetColumn() == Index % 5;
		}
	}
	bool bThreeByTwoGoalkeeperGrid = GoalkeeperAttributeGrid != nullptr
		&& GoalkeeperAttributeGrid->GetChildrenCount() == 6;
	if (bThreeByTwoGoalkeeperGrid)
	{
		for (int32 Index = 0; Index < 6; ++Index)
		{
			const UUniformGridSlot* Slot = Cast<UUniformGridSlot>(
				GoalkeeperAttributeGrid->GetChildAt(Index)->Slot);
			bThreeByTwoGoalkeeperGrid = bThreeByTwoGoalkeeperGrid
				&& Slot != nullptr && Slot->GetRow() == Index / 3
				&& Slot->GetColumn() == Index % 3;
		}
	}
	TestTrue(TEXT("Outfield and goalkeeper use legitimate canonical attributes"),
		HasExactAttributeContract(*Outfield, OutfieldAttributeContract)
			&& HasExactAttributeContract(*Goalkeeper,
				GoalkeeperAttributeContract)
			&& OutfieldCard->GetRenderedAttributeCount() == 10
			&& GoalkeeperCard->GetRenderedAttributeCount() == 6
			&& GoalkeeperAttributeGrid != nullptr
			&& GoalkeeperAttributeGrid->GetChildrenCount() == 6
			&& bFiveByTwoGrid && bThreeByTwoGoalkeeperGrid);
	bool bFixedValueGeometry = true;
	for (int32 Index = 0; Index < 10; ++Index)
	{
		const USizeBox* ValueBounds = Cast<USizeBox>(
			OutfieldCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeValueBounds%d"), Index))));
		const USizeBox* LabelBounds = Cast<USizeBox>(
			OutfieldCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeLabelBounds%d"), Index))));
		const USizeBox* CellBounds = Cast<USizeBox>(
			OutfieldCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeCellBounds%d"), Index))));
		const USizeBox* TickBounds = Cast<USizeBox>(
			OutfieldCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeTierTickBounds%d"), Index))));
		bFixedValueGeometry = bFixedValueGeometry && ValueBounds != nullptr
			&& LabelBounds != nullptr && CellBounds != nullptr
			&& TickBounds != nullptr
			&& FMath::IsNearlyEqual(ValueBounds->GetWidthOverride(), 20.0f)
			&& FMath::IsNearlyEqual(LabelBounds->GetWidthOverride(), 29.0f)
			&& FMath::IsNearlyEqual(CellBounds->GetHeightOverride(), 30.0f)
			&& FMath::IsNearlyEqual(TickBounds->GetWidthOverride(), 2.0f);
	}
	for (int32 Index = 0; Index < 6; ++Index)
	{
		const USizeBox* ValueBounds = Cast<USizeBox>(
			GoalkeeperCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeValueBounds%d"), Index))));
		const USizeBox* LabelBounds = Cast<USizeBox>(
			GoalkeeperCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeLabelBounds%d"), Index))));
		const USizeBox* CellBounds = Cast<USizeBox>(
			GoalkeeperCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeCellBounds%d"), Index))));
		const USizeBox* TickBounds = Cast<USizeBox>(
			GoalkeeperCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("AttributeTierTickBounds%d"), Index))));
		bFixedValueGeometry = bFixedValueGeometry && ValueBounds != nullptr
			&& LabelBounds != nullptr && CellBounds != nullptr
			&& TickBounds != nullptr
			&& FMath::IsNearlyEqual(ValueBounds->GetWidthOverride(), 26.0f)
			&& FMath::IsNearlyEqual(LabelBounds->GetWidthOverride(), 58.0f)
			&& FMath::IsNearlyEqual(CellBounds->GetHeightOverride(), 30.0f)
			&& FMath::IsNearlyEqual(TickBounds->GetWidthOverride(), 2.0f);
	}
	TestTrue(TEXT("Attribute labels and values use fixed structural anchors"),
		bFixedValueGeometry);

	TestTrue(TEXT("Attribute tier color contract is exact and restrained"),
		UFMCodexPlayerCardWidget::GetAttributeTierColor(1).ToFColorSRGB()
			== FColor(0x1E, 0xFF, 0x00)
			&& UFMCodexPlayerCardWidget::GetAttributeTierColor(2).ToFColorSRGB()
				== FColor(0x1E, 0xFF, 0x00)
			&& UFMCodexPlayerCardWidget::GetAttributeTierColor(3).ToFColorSRGB()
				== FColor(0x00, 0x70, 0xDD)
			&& UFMCodexPlayerCardWidget::GetAttributeTierColor(4).ToFColorSRGB()
				== FColor(0x00, 0x70, 0xDD)
			&& UFMCodexPlayerCardWidget::GetAttributeTierColor(5).ToFColorSRGB()
				== FColor(0xA3, 0x35, 0xEE)
			&& UFMCodexPlayerCardWidget::GetAttributeTierColor(6).ToFColorSRGB()
				== FColor(0xD6, 0xA8, 0x42)
			&& OutfieldCard->GetRenderedAttributeTierColors().Num() == 10);

	bool bSkillsUseRealRanges = !Outfield->Skills.IsEmpty();
	for (const FFMCodexUMGSkillViewModel& Skill : Outfield->Skills)
	{
		bSkillsUseRealRanges = bSkillsUseRealRanges
			&& !Skill.CanonicalLabel.IsEmpty()
			&& Skill.MinTriggerActionPoint > 0
			&& Skill.MaxTriggerActionPoint >= Skill.MinTriggerActionPoint;
	}
	TestTrue(TEXT("Skills consume actual identities and actual threshold ranges"),
		bSkillsUseRealRanges
			&& OutfieldCard->GetRenderedSkillCount() == Outfield->Skills.Num());
	TestTrue(TEXT("Gabriel's legitimate no-Skill state collapses cleanly"),
		GabrielNoSkill->Skills.IsEmpty()
			&& GabrielNoSkillCard->GetRenderedSkillCount() == 0
			&& GabrielNoSkillCard->GetWidgetFromName(
				TEXT("SkillPresentationRegion"))->GetVisibility()
				== ESlateVisibility::Collapsed);
	FFMCodexUMGCardViewModel OneSkillReview = *Outfield;
	OneSkillReview.DeveloperReferenceLabel = TEXT("TestOnly.OneSkillCapacity");
	OneSkillReview.Skills.SetNum(1);
	FFMCodexUMGCardViewModel TwoSkillReview = *Outfield;
	TwoSkillReview.DeveloperReferenceLabel = TEXT("TestOnly.TwoSkillCapacity");
	FFMCodexUMGCardViewModel ThreeSkillReview = *Outfield;
	ThreeSkillReview.DeveloperReferenceLabel = TEXT("TestOnly.ThreeSkillCapacity");
	for (const FFMCodexUMGCardViewModel& Candidate : LiveCards)
	{
		for (const FFMCodexUMGSkillViewModel& Skill : Candidate.Skills)
		{
			const bool bAlreadyPresent = TwoSkillReview.Skills.ContainsByPredicate(
				[&Skill](const FFMCodexUMGSkillViewModel& Existing)
				{
					return Existing.CanonicalLabel == Skill.CanonicalLabel;
				});
			if (!bAlreadyPresent && TwoSkillReview.Skills.Num() < 2)
			{
				TwoSkillReview.Skills.Add(Skill);
			}
			const bool bThreeAlreadyPresent =
				ThreeSkillReview.Skills.ContainsByPredicate(
					[&Skill](const FFMCodexUMGSkillViewModel& Existing)
					{
						return Existing.CanonicalLabel == Skill.CanonicalLabel;
					});
			if (!bThreeAlreadyPresent && ThreeSkillReview.Skills.Num() < 3)
			{
				ThreeSkillReview.Skills.Add(Skill);
			}
		}
	}
	UFMCodexPlayerCardWidget* OneSkillCard = CreateFullCard(OneSkillReview);
	UFMCodexPlayerCardWidget* TwoSkillCard = CreateFullCard(TwoSkillReview);
	UFMCodexPlayerCardWidget* ThreeSkillCard = CreateFullCard(ThreeSkillReview);
	bool bThreeSkillRowsHaveStableHeight = ThreeSkillCard != nullptr;
	for (int32 Index = 0; Index < 3 && bThreeSkillRowsHaveStableHeight; ++Index)
	{
		const USizeBox* RowBounds = Cast<USizeBox>(
			ThreeSkillCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("FullCardSkillRowBounds%d"), Index))));
		const USizeBox* AccentBounds = Cast<USizeBox>(
			ThreeSkillCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("FullCardSkillAccentBounds%d"), Index))));
		const USizeBox* RangeBounds = Cast<USizeBox>(
			ThreeSkillCard->GetWidgetFromName(FName(*FString::Printf(
				TEXT("FullCardSkillRangeBounds%d"), Index))));
		bThreeSkillRowsHaveStableHeight = RowBounds != nullptr
			&& AccentBounds != nullptr && RangeBounds != nullptr
			&& FMath::IsNearlyEqual(RowBounds->GetHeightOverride(), 28.0f)
			&& FMath::IsNearlyEqual(AccentBounds->GetWidthOverride(), 2.0f)
			&& FMath::IsNearlyEqual(RangeBounds->GetWidthOverride(), 48.0f);
	}
	TestTrue(TEXT("Full Card structurally supports 0, 1, 2 and 3 Skills"),
		GabrielNoSkillCard->GetRenderedSkillCount() == 0
			&& OneSkillCard != nullptr
			&& OneSkillCard->GetRenderedSkillCount() == 1
			&& TwoSkillReview.Skills.Num() == 2 && TwoSkillCard != nullptr
			&& TwoSkillCard->GetRenderedSkillCount() == 2
			&& ThreeSkillReview.Skills.Num() == 3 && ThreeSkillCard != nullptr
			&& ThreeSkillCard->GetRenderedSkillCount() == 3
			&& bThreeSkillRowsHaveStableHeight);

	const FFMCodexUMGCardViewModel* ShortName = nullptr;
	const FFMCodexUMGCardViewModel* LongName = nullptr;
	for (const FFMCodexUMGCardViewModel& Card : LiveCards)
	{
		if (ShortName == nullptr
			|| Card.IdentityLabel.Len() < ShortName->IdentityLabel.Len())
		{
			ShortName = &Card;
		}
		if (LongName == nullptr
			|| Card.IdentityLabel.Len() > LongName->IdentityLabel.Len())
		{
			LongName = &Card;
		}
	}
	UFMCodexPlayerCardWidget* ShortNameCard = ShortName != nullptr
		? CreateFullCard(*ShortName) : nullptr;
	UFMCodexPlayerCardWidget* LongNameCard = LongName != nullptr
		? CreateFullCard(*LongName) : nullptr;
	TestTrue(TEXT("Full Card short names retain a readable bounded text contract"),
		ShortNameCard != nullptr && LongNameCard != nullptr
			&& ShortNameCard->GetFullCardNameFontSize() >= 18
			&& ShortNameCard->GetFullCardNameFontSize() <= 24
			&& LongNameCard->GetFullCardNameFontSize() >= 18
			&& LongNameCard->GetFullCardNameFontSize() <= 24);
	const UTextBlock* MartinelliEnglish = Cast<UTextBlock>(
		IntegratedContentFullCard->GetWidgetFromName(TEXT("CardEnglishIdentity")));
	const UTextBlock* DonnarummaEnglish = Cast<UTextBlock>(
		DonnarummaCard->GetWidgetFromName(TEXT("CardEnglishIdentity")));
	TestTrue(TEXT("In-Match identity is concise while legal metadata is preserved"),
		IntegratedContentFullCard->GetRenderedIdentityText().ToString()
			== TEXT("马丁内利")
			&& MartinelliEnglish != nullptr
			&& MartinelliEnglish->GetText().ToString()
				== TEXT("Gabriel Martinelli")
			&& DonnarummaCard->GetRenderedIdentityText().ToString()
				== TEXT("多纳鲁马")
			&& DonnarummaEnglish != nullptr
			&& DonnarummaEnglish->GetText().ToString()
				== TEXT("Gianluigi Donnarumma")
			&& MartinelliEnglish->GetVisibility() == ESlateVisibility::Collapsed
			&& DonnarummaEnglish->GetVisibility() == ESlateVisibility::Collapsed
			&& MartinelliEnglish->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Clip
			&& DonnarummaEnglish->GetTextOverflowPolicy()
				== ETextOverflowPolicy::Clip);

	IConsoleVariable* ReviewCVar = IConsoleManager::Get().FindConsoleVariable(
		TEXT("FMCodex.UI.FullCardReview"));
	TestNotNull(TEXT("Developer-only Full Card review CVar exists"), ReviewCVar);
	const int32 PreviousReviewValue = ReviewCVar != nullptr
		? ReviewCVar->GetInt() : 0;
	if (ReviewCVar != nullptr)
	{
		const FFMCodexUMGMatchScreenViewModel ReviewPresentation =
			Screen->GetPresentation();
		const TArray<TSet<FName>> ExpectedReviewPages = {
			{ TEXT("Prototype.Arsenal.WilliamSaliba"),
				TEXT("Prototype.Arsenal.MartinOdegaard") },
			{ TEXT("Prototype.Arsenal.DeclanRice"),
				TEXT("Prototype.ManchesterCity.ErlingHaaland") },
			{ TEXT("Prototype.ManchesterCity.PhilFoden"),
				TEXT("Prototype.ManchesterCity.RubenDias") },
			{ TEXT("Prototype.Arsenal.BukayoSaka"),
				TEXT("Prototype.ManchesterCity.Rodri") },
			{ TEXT("Prototype.ManchesterCity.Rodri"),
				TEXT("Prototype.Arsenal.GabrielMagalhaes") }
		};
		TSet<FName> AllReviewCardIds;
		bool bEveryReviewPageIsRepresentative = true;
		for (int32 PageIndex = 0;
			PageIndex < ExpectedReviewPages.Num(); ++PageIndex)
		{
			ReviewCVar->Set(PageIndex + 1, ECVF_SetByCode);
			Screen->RefreshFromPresentation(ReviewPresentation);
			TSet<FName> ReviewCardIds;
			for (UFMCodexPlayerCardWidget* ReviewCard
				: Screen->GetFullCardProductionReviewCards())
			{
				if (ReviewCard == nullptr)
				{
					bEveryReviewPageIsRepresentative = false;
					continue;
				}
				ReviewCardIds.Add(ReviewCard->GetPresentation().CardId);
				AllReviewCardIds.Add(ReviewCard->GetPresentation().CardId);
				bEveryReviewPageIsRepresentative =
					bEveryReviewPageIsRepresentative
					&& ReviewCard->GetConfiguredDimensions().Equals(
						FVector2D(360.0f, 540.0f), 0.01f)
					&& ReviewCard->GetPresentation().CardId.ToString()
						.StartsWith(TEXT("Prototype."))
					&& (ReviewCard->GetPresentation().AttributeValues.Num() == 10
						|| ReviewCard->GetPresentation().AttributeValues.Num() == 6)
					&& !ReviewCard->IsDeveloperReferenceVisible();
				if (PageIndex == 4
					&& ReviewCard->GetPresentation().CardId
						== TEXT("Prototype.ManchesterCity.Rodri"))
				{
					bEveryReviewPageIsRepresentative =
						bEveryReviewPageIsRepresentative
						&& ReviewCard->GetRenderedSkillCount() == 3
						&& ReviewCard->GetPresentation().Skills.Num() == 3;
				}
			}
			for (const FName ExpectedCardId : ExpectedReviewPages[PageIndex])
			{
				bEveryReviewPageIsRepresentative =
					bEveryReviewPageIsRepresentative
					&& ReviewCardIds.Contains(ExpectedCardId);
			}
			bEveryReviewPageIsRepresentative = bEveryReviewPageIsRepresentative
				&& Screen->IsFullCardProductionReviewVisible()
				&& Screen->GetFullCardProductionReviewCardCount() == 2;
		}
		TestTrue(TEXT("Five true-size review pages cover the rollout six, frozen pair and stress cases"),
			bEveryReviewPageIsRepresentative && AllReviewCardIds.Num() == 9);
		ReviewCVar->Set(0, ECVF_SetByCode);
		Screen->RefreshFromPresentation(ReviewPresentation);
		TestFalse(TEXT("Full Card review page is absent from normal PIE"),
			Screen->IsFullCardProductionReviewVisible());
		ReviewCVar->Set(PreviousReviewValue, ECVF_SetByCode);
	}

	FString CardSource;
	FString InteractionViewSource;
	FString ReviewSource;
	FString DiagnosticsSource;
	TestTrue(TEXT("Full Card production sources load"),
		LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			CardSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchInteractionView.cpp"),
				InteractionViewSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ReviewSource)
			&& LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexFullCardDiagnostics.cpp"),
				DiagnosticsSource));
	TestTrue(TEXT("Production path binds approved fields without UMG calculation"),
		InteractionViewSource.Contains(TEXT("Prototype->Card.BirthDate"))
			&& InteractionViewSource.Contains(TEXT("Prototype->Card.HeightCm"))
			&& InteractionViewSource.Contains(TEXT("Prototype->Card.WeightKg"))
			&& InteractionViewSource.Contains(
				TEXT("Prototype->NationalityDisplayName"))
			&& InteractionViewSource.Contains(
				TEXT("Prototype->TeamDisplayName"))
			&& InteractionViewSource.Contains(
				TEXT("Prototype->EnglishDisplayName"))
			&& InteractionViewSource.Contains(
				TEXT("FFMCodexPlayerOverall::Calculate"))
			&& InteractionViewSource.Contains(
				TEXT("Prototype->PlayerFacingSerial"))
			&& CardSource.Contains(TEXT("Presentation.bHasOverallRating"))
			&& CardSource.Contains(TEXT("Presentation.PlayerFacingSerialLabel"))
			&& !CardSource.Contains(TEXT("PreferredFoot"))
			&& !CardSource.Contains(TEXT("OverallRating ="))
			&& !CardSource.Contains(TEXT("BirthDate = TEXT("))
			&& !CardSource.Contains(TEXT("HeightCm ="))
			&& !CardSource.Contains(TEXT("WeightKg =")));
	TestTrue(TEXT("Full Card uses one ratio-matched lower hero-bust crop"),
		CardSource.Contains(TEXT("FullCardHeroHeight = 320.0f"))
			&& CardSource.Contains(TEXT("FullCardPortraitLeft = 0.0f"))
			&& CardSource.Contains(TEXT("FullCardPortraitTop = 0.045f"))
			&& CardSource.Contains(TEXT("FullCardPortraitRight = 1.0f"))
			&& CardSource.Contains(TEXT("FullCardPortraitBottom = 0.658f"))
			&& CardSource.Contains(TEXT("FullCardIdentityReadabilityScrim"))
			&& !CardSource.Contains(TEXT("PerPlayerFullCardCrop")));
	TestTrue(TEXT("Developer review is a cheat-gated non-shipping surface"),
		DiagnosticsSource.Contains(TEXT("ECVF_Cheat"))
			&& DiagnosticsSource.Contains(TEXT("UE_BUILD_SHIPPING"))
			&& DiagnosticsSource.Contains(TEXT("FMCodex.UI.FullCardReview"))
			&& ReviewSource.Contains(TEXT("UE_BUILD_SHIPPING"))
			&& ReviewSource.Contains(
				TEXT("RefreshFullCardProductionReviewSurface")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexInMatchFullCardInformationArchitectureContractTest,
	"FMCodex.LocalPlay.ControlSurface.42.InMatchFullCardInformationArchitectureContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexInMatchFullCardInformationArchitectureContractTest::RunTest(
	const FString& Parameters)
{
	struct FExpectedIdentity
	{
		FName CardId;
		FString ShortName;
		FString LegalChineseName;
	};
	const TArray<FExpectedIdentity> ExpectedIdentities = {
		{ TEXT("Prototype.Arsenal.DavidRaya"), TEXT("拉亚"), TEXT("大卫·拉亚") },
		{ TEXT("Prototype.Arsenal.WilliamSaliba"), TEXT("萨利巴"), TEXT("威廉·萨利巴") },
		{ TEXT("Prototype.Arsenal.BukayoSaka"), TEXT("萨卡"), TEXT("布卡约·萨卡") },
		{ TEXT("Prototype.Arsenal.MartinOdegaard"), TEXT("厄德高"), TEXT("马丁·厄德高") },
		{ TEXT("Prototype.Arsenal.DeclanRice"), TEXT("赖斯"), TEXT("德克兰·赖斯") },
		{ TEXT("Prototype.Arsenal.GabrielMartinelli"), TEXT("马丁内利"), TEXT("加布里埃尔·马丁内利") },
		{ TEXT("Prototype.Arsenal.GabrielMagalhaes"), TEXT("加布里埃尔"), TEXT("加布里埃尔·马加良斯") },
		{ TEXT("Prototype.Arsenal.MikelMerino"), TEXT("梅里诺"), TEXT("米克尔·梅里诺") },
		{ TEXT("Prototype.ManchesterCity.GianluigiDonnarumma"), TEXT("多纳鲁马"), TEXT("吉安路易吉·多纳鲁马") },
		{ TEXT("Prototype.ManchesterCity.ErlingHaaland"), TEXT("哈兰德"), TEXT("埃尔林·哈兰德") },
		{ TEXT("Prototype.ManchesterCity.PhilFoden"), TEXT("福登"), TEXT("菲尔·福登") },
		{ TEXT("Prototype.ManchesterCity.Rodri"), TEXT("罗德里"), TEXT("罗德里") },
		{ TEXT("Prototype.ManchesterCity.RubenDias"), TEXT("迪亚斯"), TEXT("鲁本·迪亚斯") },
		{ TEXT("Prototype.ManchesterCity.JoskoGvardiol"), TEXT("格瓦迪奥尔"), TEXT("约什科·格瓦迪奥尔") },
		{ TEXT("Prototype.ManchesterCity.BernardoSilva"), TEXT("贝尔纳多"), TEXT("贝尔纳多·席尔瓦") },
		{ TEXT("Prototype.ManchesterCity.JeremyDoku"), TEXT("多库"), TEXT("杰里米·多库") }
	};
	const TSet<FName> ExpectedFullCardHeroBustArt = {
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.MikelMerino"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.JeremyDoku")
	};
	const TSet<FName> ExpectedSharedPortraitHeroBustArt = {
		TEXT("Prototype.Arsenal.WilliamSaliba"),
		TEXT("Prototype.Arsenal.GabrielMagalhaes"),
		TEXT("Prototype.Arsenal.MartinOdegaard"),
		TEXT("Prototype.Arsenal.DeclanRice"),
		TEXT("Prototype.Arsenal.MikelMerino"),
		TEXT("Prototype.Arsenal.GabrielMartinelli"),
		TEXT("Prototype.ManchesterCity.ErlingHaaland"),
		TEXT("Prototype.ManchesterCity.PhilFoden"),
		TEXT("Prototype.ManchesterCity.RubenDias"),
		TEXT("Prototype.ManchesterCity.JoskoGvardiol"),
		TEXT("Prototype.ManchesterCity.BernardoSilva"),
		TEXT("Prototype.ManchesterCity.JeremyDoku")
	};
	const TSet<FName> ExpectedFullCardPilotArt = {
		TEXT("Prototype.Arsenal.BukayoSaka"),
		TEXT("Prototype.Arsenal.DavidRaya"),
		TEXT("Prototype.ManchesterCity.Rodri"),
		TEXT("Prototype.ManchesterCity.GianluigiDonnarumma")
	};
	int32 DedicatedFullCardArtCount = 0;
	int32 MissingFullCardArtCount = 0;
	int32 FullCardPilotArtCount = 0;
	int32 FullCardHeroBustArtCount = 0;
	bool bAllShortNamesExplicit = ExpectedIdentities.Num() == 16;
	bool bAllMetadataPreserved = true;
	bool bArtBoundaryIsHonest = true;
	for (const FExpectedIdentity& Expected : ExpectedIdentities)
	{
		const FText ShortName =
			FFMCodexPlayerUIPresentationText::InMatchShortPlayerName(
				Expected.CardId, Expected.LegalChineseName);
		const FFMCodexPrototypePlayerDefinition* Definition =
			FFMCodexPrototypeTeamContent::Find(Expected.CardId);
		bAllShortNamesExplicit = bAllShortNamesExplicit
			&& ShortName.ToString() == Expected.ShortName
			&& !ShortName.ToString().Contains(TEXT("Prototype."))
			&& !ShortName.ToString().Contains(TEXT("CardId"));
		bAllMetadataPreserved = bAllMetadataPreserved
			&& Definition != nullptr
			&& Definition->Card.DisplayName.ToString()
				== Expected.LegalChineseName
			&& Definition->CanonicalChineseDisplayName.ToString()
				== Expected.LegalChineseName
			&& Definition->PreferredDisplayName.ToString()
				== Expected.ShortName
			&& !Definition->EnglishDisplayName.IsEmpty()
			&& !Definition->NationalityDisplayName.IsEmpty()
			&& !Definition->TeamDisplayName.IsEmpty()
			&& FFMCodexPlayerUIPresentationText::PlayerName(
				Expected.CardId, FString()).ToString()
				== Expected.ShortName
			&& FFMCodexPrototypeTeamContent::CanonicalChinesePlayerName(
				Expected.CardId).ToString() == Expected.LegalChineseName;

		const FFMCodexPlayerUICardArtReferences Art =
			FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(
				Expected.CardId);
		const TSoftObjectPtr<UTexture2D>& FullCardPortrait =
			Art.FullCardPortrait.IsNull()
				? Art.Portrait : Art.FullCardPortrait;
		const FString PortraitPath =
			FullCardPortrait.ToSoftObjectPath().ToString();
		const bool bUsesFullCardPilotArtwork =
			PortraitPath.Contains(TEXT("_FullCardPilot_02"));
		const bool bUsesFullCardHeroBustArtwork =
			PortraitPath.Contains(TEXT("_FullCardHeroBust_01"));
		if (bUsesFullCardPilotArtwork)
		{
			++FullCardPilotArtCount;
		}
		if (bUsesFullCardHeroBustArtwork)
		{
			++FullCardHeroBustArtCount;
		}
		if (FullCardPortrait.IsNull())
		{
			++MissingFullCardArtCount;
		}
		else
		{
			++DedicatedFullCardArtCount;
		}
		bArtBoundaryIsHonest = bArtBoundaryIsHonest
			&& !FullCardPortrait.IsNull()
			&& bUsesFullCardPilotArtwork
				== ExpectedFullCardPilotArt.Contains(Expected.CardId)
			&& bUsesFullCardHeroBustArtwork
				== ExpectedFullCardHeroBustArt.Contains(Expected.CardId)
			&& (!bUsesFullCardHeroBustArtwork
				|| (!Art.FullCardPortrait.IsNull()
					&& (ExpectedSharedPortraitHeroBustArt.Contains(Expected.CardId)
						? !Art.Portrait.IsNull() : Art.Portrait.IsNull())
					&& Art.Portrait.ToSoftObjectPath()
						!= Art.FullCardPortrait.ToSoftObjectPath()))
			&& !Art.HandMicroPortrait.IsNull()
			&& !PortraitPath.Contains(TEXT("Runtime192"))
			&& !PortraitPath.Contains(
				TEXT("/Game/UI/Portraits/T_Pilot_PlayerPortrait_01"))
			&& !PortraitPath.Contains(TEXT("GoldenSample"));
	}
	TestTrue(TEXT("All 16 Full Card short names are explicit and deterministic"),
		bAllShortNamesExplicit);
	TestTrue(TEXT("Short-name presentation preserves full Chinese and English metadata"),
		bAllMetadataPreserved);
	TestTrue(TEXT("Full Card art audit is exactly 16 dedicated and 0 missing"),
		bArtBoundaryIsHonest && DedicatedFullCardArtCount == 16
			&& MissingFullCardArtCount == 0
			&& FullCardPilotArtCount == 4
			&& FullCardHeroBustArtCount == 12);
	TestTrue(TEXT("In-Match position uses compact slash notation"),
		FFMCodexPlayerUIPresentationText::InMatchCompactRole(TEXT("GK"))
			.ToString() == TEXT("GK")
			&& FFMCodexPlayerUIPresentationText::InMatchCompactRole(TEXT("DF"))
				.ToString() == TEXT("D")
			&& FFMCodexPlayerUIPresentationText::InMatchCompactRole(TEXT("MF/DF"))
				.ToString() == TEXT("M/D")
			&& FFMCodexPlayerUIPresentationText::InMatchCompactRole(
				TEXT("FW/MF/DF")).ToString() == TEXT("A/M/D"));

	FString CardSource;
	FString ReviewSource;
	TestTrue(TEXT("Information architecture production sources load"),
		FMCodexLocalMatchControlSurfaceTests::LoadProductionSource(
			TEXT("Source/FMCodex/LocalPlay/FMCodexPlayerCardWidget.cpp"),
			CardSource)
			&& FMCodexLocalMatchControlSurfaceTests::LoadProductionSource(
				TEXT("Source/FMCodex/LocalPlay/FMCodexLocalMatchScreenWidget.cpp"),
				ReviewSource));
	TestTrue(TEXT("Review-only stress DTO cannot mutate production content"),
		ReviewSource.Contains(TEXT("FullCardReview.ThreeSkillStress"))
			&& ReviewSource.Contains(TEXT("FFMCodexUMGCardViewModel StressCard"))
			&& !ReviewSource.Contains(TEXT("Card.AttackSkillIds.Add"))
			&& CardSource.Contains(TEXT("Index / ColumnCount"))
			&& CardSource.Contains(TEXT("Presentation.bGoalkeeper ? 3 : 5")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFMCodexOnPitchCarrierSelectionFoundationTest,
	"FMCodex.LocalPlay.ControlSurface.43.OnPitchCarrierSelectionFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFMCodexOnPitchCarrierSelectionFoundationTest::RunTest(
	const FString& Parameters)
{
	using namespace FMCodexLocalMatchControlSurfaceTests;
	using namespace FMCodexLocalMatchFullFamilyTests;

	FScopedPlayableWorld PlayableWorld;
	AFMCodexLocalMatchHostGameMode* Host = PlayableWorld.GetHost();
	AFMCodexLocalMatchPlayerController* Controller =
		PlayableWorld.GetController();
	TestNotNull(TEXT("On-pitch selection Host exists"), Host);
	TestNotNull(TEXT("On-pitch selection Controller exists"), Controller);
	if (Host == nullptr || Controller == nullptr)
	{
		return false;
	}

	Controller->InitializePlayerFacingUI();
	UFMCodexLocalMatchScreenWidget* Screen =
		Controller->GetPlayerMatchScreen();
	TestNotNull(TEXT("On-pitch selection Screen exists"), Screen);
	if (Screen == nullptr)
	{
		return false;
	}
	Screen->TakeWidget();
	Screen->RequestStartNewMatch();
	Screen->RequestRollTacticalPoints();
	const EInitialTurnOrderPlayer Attacker =
		Controller->GetInteractionView().CurrentAttackingPlayer;
	const FFamilyExpectation CrossFamily = FamilyExpectations()[0];
	if (!DeployParticipants(*this, *Controller, CrossFamily, Attacker))
	{
		return false;
	}
	Controller->RefreshPresentation();

	const FFMCodexLocalMatchInteractionView& InteractionView =
		Controller->GetInteractionView();
	UFMCodexPitchWidget* Pitch = Screen->GetPitchWidget();
	UFMCodexInteractionPanelWidget* Panel = Screen->GetInteractionPanel();
	TestNotNull(TEXT("On-pitch selection Pitch exists"), Pitch);
	TestNotNull(TEXT("On-pitch selection Panel exists"), Panel);
	if (Pitch == nullptr || Panel == nullptr)
	{
		return false;
	}

	TSet<FName> LegalOptionIds;
	for (const FFMCodexLocalMatchSelectionOption& Option
		: InteractionView.SelectionOptions)
	{
		LegalOptionIds.Add(Option.Id);
	}
	TSet<FName> StructurallySelectableOwnIds;
	TSet<FName> OpponentDeployedIds;
	TSet<FName> NonDeployedOwnIds;
	const TArray<FFMCodexLocalMatchCardView>& AttackerRoster =
		Attacker == EInitialTurnOrderPlayer::PlayerA
			? InteractionView.PlayerACardRoster
			: InteractionView.PlayerBCardRoster;
	for (const FFMCodexLocalMatchCardView& Card : AttackerRoster)
	{
		if (!Card.bDeployed)
		{
			NonDeployedOwnIds.Add(Card.CardId);
		}
	}
	for (const FFMCodexLocalMatchPitchRegionView& Region
		: InteractionView.PitchRegions)
	{
		for (const FFMCodexLocalMatchPitchSlotView& Slot : Region.Slots)
		{
			if (!Slot.bOccupied)
			{
				continue;
			}
			if (Slot.Card.Side == Attacker && !Slot.Card.bGoalkeeper)
			{
				StructurallySelectableOwnIds.Add(Slot.Card.CardId);
			}
			else if (Slot.Card.Side != Attacker)
			{
				OpponentDeployedIds.Add(Slot.Card.CardId);
			}
		}
	}

	TSet<FName> ProjectedSelectableIds;
	UFMCodexPlayerCardWidget* OwnNoTacticalMatchCard = nullptr;
	UFMCodexPlayerCardWidget* OwnTacticalMatchCard = nullptr;
	UFMCodexPlayerCardWidget* OpponentCard = nullptr;
	UFMCodexPitchSlotWidget* EmptyPitchSlot = nullptr;
	for (UFMCodexPitchSlotWidget* Slot : Pitch->GetRenderedSlotWidgets())
	{
		if (Slot == nullptr)
		{
			continue;
		}
		Slot->TakeWidget();
		const FFMCodexUMGPitchSlotViewModel& SlotView = Slot->GetPresentation();
		if (!SlotView.bOccupied)
		{
			EmptyPitchSlot = EmptyPitchSlot != nullptr ? EmptyPitchSlot : Slot;
			continue;
		}
		UFMCodexPlayerCardWidget* CardWidget = Slot->GetCardWidget();
		if (SlotView.bSelectableForCurrentPrompt)
		{
			ProjectedSelectableIds.Add(SlotView.OnPitchSelectionOptionId);
			if (SlotView.Card.PitchMiniTacticalMatchCount == 0)
			{
				OwnNoTacticalMatchCard = OwnNoTacticalMatchCard != nullptr
					? OwnNoTacticalMatchCard : CardWidget;
			}
			else
			{
				OwnTacticalMatchCard = OwnTacticalMatchCard != nullptr
					? OwnTacticalMatchCard : CardWidget;
			}
		}
		else if (OpponentDeployedIds.Contains(SlotView.Card.CardId))
		{
			OpponentCard = OpponentCard != nullptr ? OpponentCard : CardWidget;
		}
	}

	TestTrue(TEXT("Carrier authority exposes every structurally selectable own deployment"),
		InteractionView.InteractionCategory
			== EFMCodexLocalMatchInteractionCategory::SelectCarrier
			&& !LegalOptionIds.IsEmpty()
			&& LegalOptionIds.Num() == StructurallySelectableOwnIds.Num()
			&& LegalOptionIds.Includes(StructurallySelectableOwnIds)
			&& ProjectedSelectableIds.Num() == LegalOptionIds.Num()
			&& ProjectedSelectableIds.Includes(LegalOptionIds));
	TestTrue(TEXT("Carrier UI is instructional and has no bottom PlayerKey buttons"),
		Screen->GetPresentation().Interaction.bUseOnPitchPlayerSelection
			&& Screen->GetPresentation().Interaction.Category
				== EFMCodexUMGInteractionCategory::SelectCarrier
			&& !Screen->GetPresentation().Interaction.OnPitchSelectionHintLabel.IsEmpty()
			&& Panel->GetRenderedOptionWidgets().IsEmpty()
			&& Panel->GetWidgetFromName(TEXT("InteractionCandidateRegion"))
				->GetVisibility() == ESlateVisibility::Collapsed);
	TestNotNull(TEXT("Own deployed card without Tactical Match remains selectable"),
		OwnNoTacticalMatchCard);
	TestNotNull(TEXT("Tactical Match own deployed card remains independently visible"),
		OwnTacticalMatchCard);
	TestNotNull(TEXT("Opponent deployed card remains non-selectable"), OpponentCard);
	TestNotNull(TEXT("Empty pitch slot remains non-selectable"), EmptyPitchSlot);
	if (OwnNoTacticalMatchCard == nullptr || OwnTacticalMatchCard == nullptr
		|| OpponentCard == nullptr || EmptyPitchSlot == nullptr)
	{
		return false;
	}

	OwnNoTacticalMatchCard->TakeWidget();
	OwnTacticalMatchCard->TakeWidget();
	OpponentCard->TakeWidget();
	TestTrue(TEXT("Structural selectability has no dedicated outline glow or lift"),
		OwnNoTacticalMatchCard->IsSelectableForCurrentPrompt()
			&& OwnNoTacticalMatchCard->GetInteractionState()
				== EFMCodexUMGCardInteractionState::OnPitchSelectable
			&& OwnNoTacticalMatchCard->GetWidgetFromName(
				TEXT("PitchMiniSelectionStrokeTop")) == nullptr
			&& OwnNoTacticalMatchCard->GetWidgetFromName(
				TEXT("PitchMiniSelectionGlowTop")) == nullptr
			&& OwnNoTacticalMatchCard->GetRenderTransform().Scale
				.Equals(FVector2D(1.0f, 1.0f))
			&& !OpponentCard->IsSelectableForCurrentPrompt());
	TestTrue(TEXT("Tactical Match pips remain independent from click selectability"),
		OwnNoTacticalMatchCard->GetPresentation()
			.PitchMiniTacticalMatchCount == 0
			&& OwnTacticalMatchCard->GetPresentation()
				.PitchMiniTacticalMatchCount > 0
			&& OwnTacticalMatchCard->IsSelectableForCurrentPrompt());

	TestTrue(TEXT("SelectCarrier preserves the normal Pitch Mini Full Card hover route"),
		OwnNoTacticalMatchCard->RequestFullCardDetailHover()
			&& Screen->IsDetailOverlayVisible()
			&& Screen->GetDetailOverlayCard() != nullptr
			&& Screen->GetDetailOverlayCard()->GetPresentation().CardId
				== OwnNoTacticalMatchCard->GetPresentation().CardId);

	const TArray<uint8> BeforeIllegalClick =
		SerializeState(Host->GetMatchSnapshot().Snapshot);
	TestFalse(TEXT("Opponent deployed card refuses own-player selection click"),
		OpponentCard->RequestOnPitchSelection());
	TestTrue(TEXT("Opponent click leaves authority state unchanged"),
		BeforeIllegalClick == SerializeState(Host->GetMatchSnapshot().Snapshot)
			&& Controller->GetInteractionView().InteractionCategory
				== EFMCodexLocalMatchInteractionCategory::SelectCarrier);
	TestTrue(TEXT("Empty slot exposes no on-pitch card submission surface"),
		!EmptyPitchSlot->GetPresentation().bSelectableForCurrentPrompt
			&& EmptyPitchSlot->GetCardWidget() == nullptr);

	UFMCodexPlayerCardWidget* NonDeployedRackCard = nullptr;
	for (UFMCodexPlayerCardWidget* RackCard
		: Screen->GetLocalRackWidget()->GetRenderedCardWidgets())
	{
		if (RackCard != nullptr
			&& NonDeployedOwnIds.Contains(
				RackCard->GetPresentation().CardId))
		{
			NonDeployedRackCard = RackCard;
			break;
		}
	}
	TestNotNull(TEXT("A non-deployed rack card exists for path isolation"),
		NonDeployedRackCard);
	if (NonDeployedRackCard == nullptr)
	{
		return false;
	}
	TestFalse(TEXT("Non-deployed rack card cannot submit through on-pitch path"),
		NonDeployedRackCard->RequestOnPitchSelection());

	const FName CommittedCarrierId =
		OwnNoTacticalMatchCard->GetOnPitchSelectionOptionId();
	TestTrue(TEXT("Single no-Tactical-Match own-card click emits Carrier intent"),
		OwnNoTacticalMatchCard->RequestOnPitchSelection());
	TestTrue(TEXT("Poor tactical choice commits immediately and advances"),
		Controller->GetLastDiagnostic().bHostSuccess
			&& Controller->GetLastDiagnostic().CommandName == TEXT("SubmitCarrier")
			&& Controller->GetInteractionView().InteractionCategory
				!= EFMCodexLocalMatchInteractionCategory::SelectCarrier
			&& Host->GetMatchSnapshot().Snapshot.CurrentAttack.ActionPreparation
				.CarrierCardId == CommittedCarrierId);

	return true;
}

#endif
