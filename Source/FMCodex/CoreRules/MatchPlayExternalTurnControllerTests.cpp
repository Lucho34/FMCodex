#include "MatchPlayExternalTurnController.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayExternalTurnControllerTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = 3;
		RuntimeState.PlayerAState.Score = 2;
		RuntimeState.PlayerBState.TotalAttackCount = 3;
		RuntimeState.PlayerBState.Score = 1;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = 3;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = 3;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("TestDeploymentSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer PlayerSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = true;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = 6.0f;
		Request.FormulaInput.Defender.BaseValue = 4.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Left.RuntimeState.bIsInitialized
				== Right.RuntimeState.bIsInitialized
			&& Left.RuntimeState.PlayerAState.TotalAttackCount
				== Right.RuntimeState.PlayerAState.TotalAttackCount
			&& Left.RuntimeState.PlayerAState.UsedAttackCount
				== Right.RuntimeState.PlayerAState.UsedAttackCount
			&& Left.RuntimeState.PlayerAState.Score
				== Right.RuntimeState.PlayerAState.Score
			&& Left.RuntimeState.PlayerBState.TotalAttackCount
				== Right.RuntimeState.PlayerBState.TotalAttackCount
			&& Left.RuntimeState.PlayerBState.UsedAttackCount
				== Right.RuntimeState.PlayerBState.UsedAttackCount
			&& Left.RuntimeState.PlayerBState.Score
				== Right.RuntimeState.PlayerBState.Score
			&& Left.RuntimeState.CurrentAttackingPlayer
				== Right.RuntimeState.CurrentAttackingPlayer
			&& Left.CardUsageState.PlayerACardUsageState.AvailableCardIds
				== Right.CardUsageState.PlayerACardUsageState.AvailableCardIds
			&& Left.CardUsageState.PlayerACardUsageState.UsedCardIds
				== Right.CardUsageState.PlayerACardUsageState.UsedCardIds
			&& Left.CardUsageState.PlayerBCardUsageState.AvailableCardIds
				== Right.CardUsageState.PlayerBCardUsageState.AvailableCardIds
			&& Left.CardUsageState.PlayerBCardUsageState.UsedCardIds
				== Right.CardUsageState.PlayerBCardUsageState.UsedCardIds
			&& FMatchPlayDeploymentSlotCatalog::StaticStruct()
				->CompareScriptStruct(
					&Left.DeploymentSlotCatalog,
					&Right.DeploymentSlotCatalog,
					0);
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlayExternalTurnController.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerSuccessTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ValidRequestReturnsSuccessAndView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalTurnControllerTests::MakeState();
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalTurnControllerTests::MakeRequest());

	TestTrue(TEXT("Request is handled"), Result.bHandled);
	TestTrue(TEXT("Submission succeeds"), Result.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Success code is returned"),
		Result.Code,
		EMatchPlayExternalTurnControllerCode::SubmittedSuccessfully);
	TestTrue(
		TEXT("Submit result is retained"),
		Result.SubmitResult.bSuccess);
	TestTrue(
		TEXT("Result view is retained"),
		Result.ResultView.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Input state remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerInvalidRequestTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.InvalidRequestPreservesLowerResults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerInvalidRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			MatchPlayExternalTurnControllerTests::MakeState(),
			MatchPlayExternalTurnControllerTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayExternalTurnControllerTests::CardB1));

	TestTrue(TEXT("Rejected request is handled"), Result.bHandled);
	TestFalse(TEXT("Rejected request does not submit"), Result.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Controller reports rejection"),
		Result.Code,
		EMatchPlayExternalTurnControllerCode::SubmissionRejected);
	TestEqual(
		TEXT("Submit rejection is retained"),
		Result.SubmitResult.Code,
		EMatchPlaySubmitAttackFacadeCode::SubmissionRejected);
	TestTrue(
		TEXT("Result view reports pre-execution rejection"),
		Result.ResultView.bWasRejectedBeforeExecution);
	TestEqual(
		TEXT("Lower error reason is preserved"),
		Result.ErrorMessage,
		Result.SubmitResult.ErrorMessage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerGateAtomicTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.GateRejectionKeepsStateAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerGateAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalTurnControllerTests::MakeState();
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalTurnControllerTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestTrue(
		TEXT("Controller After state equals Before state"),
		MatchPlayExternalTurnControllerTests::AreStatesEqual(
			Result.AfterState,
			State));
	TestTrue(
		TEXT("Submit After state remains atomic"),
		MatchPlayExternalTurnControllerTests::AreStatesEqual(
			Result.AfterState,
			Result.SubmitResult.AfterState));
	TestEqual(
		TEXT("Attack opportunity is not consumed"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Score is unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerAfterStateTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.SuccessUsesFacadeAfterState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerAfterStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			MatchPlayExternalTurnControllerTests::MakeState(),
			MatchPlayExternalTurnControllerTests::MakeRequest());

	TestTrue(
		TEXT("Controller After state equals Submit After state"),
		MatchPlayExternalTurnControllerTests::AreStatesEqual(
			Result.AfterState,
			Result.SubmitResult.AfterState));
	TestEqual(
		TEXT("Exactly one opportunity is consumed"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	TestTrue(
		TEXT("Played card is used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(
				MatchPlayExternalTurnControllerTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerDependencyBoundaryTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ProductionUsesOnlyFacadeAndResultQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerDependencyBoundaryTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalTurnControllerTests::LoadProductionSource();

	TestTrue(
		TEXT("Calls Submit facade"),
		Source.Contains(TEXT("FMatchPlaySubmitAttackFacade::Submit")));
	TestTrue(
		TEXT("Calls result query"),
		Source.Contains(
			TEXT("FMatchPlaySubmitAttackResultQuery::BuildView")));
	TestFalse(
		TEXT("Does not call submission gate"),
		Source.Contains(TEXT("FMatchPlaySubmissionGate::")));
	TestFalse(
		TEXT("Does not call attack step"),
		Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(TEXT("Does not call Flow"), Source.Contains(TEXT("Flow::")));
	TestFalse(
		TEXT("Does not call Resolver"),
		Source.Contains(TEXT("Resolver::")));
	TestFalse(
		TEXT("Does not call Executor"),
		Source.Contains(TEXT("Executor::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerSingleAttackTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.DoesNotExecuteSecondAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerSingleAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			MatchPlayExternalTurnControllerTests::MakeState(),
			MatchPlayExternalTurnControllerTests::MakeRequest());

	TestEqual(
		TEXT("PlayerA consumes one opportunity"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	TestEqual(
		TEXT("PlayerB consumes no opportunity"),
		Result.AfterState.RuntimeState.PlayerBState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Exactly one card is used"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Num(),
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerNoLoopTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ProductionHasNoMatchLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerNoLoopTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalTurnControllerTests::LoadProductionSource();

	TestEqual(
		TEXT("Submit is called from one source location"),
		Source.Replace(
			TEXT("FMatchPlaySubmitAttackFacade::Submit"),
			TEXT("")).Len(),
		Source.Len()
			- FCString::Strlen(
				TEXT("FMatchPlaySubmitAttackFacade::Submit")));
	TestFalse(TEXT("No while loop"), Source.Contains(TEXT("while (")));
	TestFalse(TEXT("No for loop"), Source.Contains(TEXT("for (")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerNoAutoSelectionOrAITest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ProductionHasNoAutoSelectionOrAI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerNoAutoSelectionOrAITest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalTurnControllerTests::LoadProductionSource();

	TestFalse(
		TEXT("No automatic CardId selection"),
		Source.Contains(TEXT("SelectCard")));
	TestFalse(
		TEXT("No AI module dependency"),
		Source.Contains(TEXT("AIModule")));
	TestFalse(
		TEXT("No AI controller dependency"),
		Source.Contains(TEXT("AIController")));
	TestTrue(
		TEXT("Uses caller request"),
		Source.Contains(TEXT("BeforeState, Request")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ProductionHasNoRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalTurnControllerTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalTurnControllerNoExternalSystemsTest,
	"FMCodex.CoreRules.MatchPlayExternalTurnController.ProductionHasNoExternalSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalTurnControllerNoExternalSystemsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalTurnControllerTests::LoadProductionSource();

	TestFalse(
		TEXT("No UI dependency"),
		Source.Contains(TEXT("UI"), ESearchCase::CaseSensitive));
	TestFalse(
		TEXT("No Blueprint dependency"),
		Source.Contains(TEXT("Blueprint")));
	TestFalse(TEXT("No networking dependency"), Source.Contains(TEXT("Network")));
	TestFalse(TEXT("No Steam dependency"), Source.Contains(TEXT("Steam")));
	TestFalse(TEXT("No skill dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No card database dependency"),
		Source.Contains(TEXT("Database")));
	return true;
}

#endif
