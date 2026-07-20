#include "MatchPlaySubmitAttackFacade.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlaySubmitAttackFacadeTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = 2;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = 1;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = PlayerATotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = PlayerBTotal;

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
		const FName CardId = CardA1,
		const bool bGoal = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = true;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = bGoal ? 6.0f : 4.0f;
		Request.FormulaInput.Defender.BaseValue = bGoal ? 4.0f : 6.0f;
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
			&& Left.RuntimeState.FirstPlayer
				== Right.RuntimeState.FirstPlayer
			&& Left.RuntimeState.SecondPlayer
				== Right.RuntimeState.SecondPlayer
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
					"Source/FMCodex/CoreRules/MatchPlaySubmitAttackFacade.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeGateRejectTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.GateRejectsWithoutExecutingStep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeGateRejectTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestFalse(TEXT("Submission fails"), Result.bSuccess);
	TestEqual(
		TEXT("Gate rejection code is returned"),
		Result.Code,
		EMatchPlaySubmitAttackFacadeCode::SubmissionRejected);
	TestFalse(
		TEXT("Gate rejects request"),
		Result.SubmissionGateResult.bCanSubmit);
	TestFalse(
		TEXT("Attack step remains unexecuted"),
		Result.AttackStepResult.ExecutionResult.bAttackResolved);
	TestTrue(
		TEXT("After state equals Before state"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeGateRejectAtomicTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.GateRejectionIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeGateRejectAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlaySubmitAttackFacadeTests::CardB1));

	TestTrue(
		TEXT("Card remains available"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(
				MatchPlaySubmitAttackFacadeTests::CardA1));
	TestTrue(
		TEXT("Used cards remain empty"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.IsEmpty());
	TestEqual(
		TEXT("Attack opportunity is unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Score is unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeSuccessTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.ValidRequestExecutesOneStep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			MatchPlaySubmitAttackFacadeTests::MakeState(),
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestTrue(TEXT("Submission succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Submitted code is returned"),
		Result.Code,
		EMatchPlaySubmitAttackFacadeCode::Submitted);
	TestTrue(
		TEXT("Gate allows submission"),
		Result.SubmissionGateResult.bCanSubmit);
	TestTrue(TEXT("Attack step succeeds"), Result.AttackStepResult.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeSummaryTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.SuccessReturnsExecutionSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeSummaryTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			MatchPlaySubmitAttackFacadeTests::MakeState(),
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestTrue(
		TEXT("Summary reports success"),
		Result.ExecutionSummary.bExecutionSuccess);
	TestTrue(
		TEXT("Summary reports resolved attack"),
		Result.ExecutionSummary.bAttackResolved);
	TestEqual(
		TEXT("Summary retains CardId"),
		Result.ExecutionSummary.CardId,
		MatchPlaySubmitAttackFacadeTests::CardA1);
	TestTrue(
		TEXT("Facade summary matches step summary"),
		Result.ExecutionSummary.bGoalScored
			== Result.AttackStepResult.ExecutionSummary.bGoalScored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeAfterStateTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.SuccessReturnsAfterState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeAfterStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestTrue(
		TEXT("After state uses step updated state"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			Result.AttackStepResult.ExecutionResult
				.UpdatedMatchPlayState));
	TestFalse(
		TEXT("Played card leaves available cards"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(
				MatchPlaySubmitAttackFacadeTests::CardA1));
	TestTrue(
		TEXT("Played card enters used cards"),
		Result.AfterState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(
				MatchPlaySubmitAttackFacadeTests::CardA1));
	TestTrue(
		TEXT("Input state remains unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlaySubmitAttackFacadeTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeNotCurrentTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.NonCurrentAttackerFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlaySubmitAttackFacadeTests::CardB1));

	TestFalse(TEXT("Non-current attacker fails"), Result.bSuccess);
	TestEqual(
		TEXT("Underlying reason is retained"),
		Result.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::NotCurrentAttacker);
	TestTrue(
		TEXT("State remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeEmptyCardTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.EmptyCardIdFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeEmptyCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				NAME_None));

	TestFalse(TEXT("Empty CardId fails"), Result.bSuccess);
	TestEqual(
		TEXT("Underlying reason is retained"),
		Result.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::EmptyCardId);
	TestTrue(
		TEXT("State remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeUnavailableOrUsedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.UnavailableOrUsedCardFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeUnavailableOrUsedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState AvailableState =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	const FMatchPlaySubmitAttackFacadeResult UnavailableResult =
		FMatchPlaySubmitAttackFacade::Submit(
			AvailableState,
			MatchPlaySubmitAttackFacadeTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlaySubmitAttackFacadeTests::MissingCard));

	FMatchPlayState UsedState =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	UsedState.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlaySubmitAttackFacadeTests::CardA1);
	UsedState.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlaySubmitAttackFacadeTests::CardA1);
	const FMatchPlaySubmitAttackFacadeResult UsedResult =
		FMatchPlaySubmitAttackFacade::Submit(
			UsedState,
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestEqual(
		TEXT("Unavailable card reason is retained"),
		UnavailableResult.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::CardNotAvailable);
	TestTrue(
		TEXT("Unavailable card state remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			UnavailableResult.AfterState,
			AvailableState));
	TestEqual(
		TEXT("Used card reason is retained"),
		UsedResult.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::CardAlreadyUsed);
	TestTrue(
		TEXT("Used card state remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			UsedResult.AfterState,
			UsedState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeFinishedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.FinishedMatchFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			1);
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestFalse(TEXT("Finished match fails"), Result.bSuccess);
	TestEqual(
		TEXT("Finished-match reason is retained"),
		Result.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::MatchAlreadyFinished);
	TestTrue(
		TEXT("State remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeNoOpportunityTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.NoOpportunityFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeNoOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			2,
			0);
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			State,
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestFalse(TEXT("No opportunity fails"), Result.bSuccess);
	TestEqual(
		TEXT("No-opportunity reason is retained"),
		Result.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::NoRemainingAttackOpportunity);
	TestTrue(
		TEXT("State remains unchanged"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeSingleAttackTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.DoesNotExecuteSecondAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeSingleAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(
			MatchPlaySubmitAttackFacadeTests::MakeState(),
			MatchPlaySubmitAttackFacadeTests::MakeRequest());

	TestEqual(
		TEXT("Exactly one PlayerA opportunity is consumed"),
		Result.AfterState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	TestEqual(
		TEXT("PlayerB opportunity is not consumed"),
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
	FMatchPlaySubmitAttackFacadeStepFailureTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.StepFailureIsRetainedAndAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeStepFailureTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackFacadeTests::MakeState();
	FMatchPlayAttackRequest Request =
		MatchPlaySubmitAttackFacadeTests::MakeRequest();
	Request.FormulaInput.FormulaType = EFormulaType::Transition;

	const FMatchPlaySubmitAttackFacadeResult Result =
		FMatchPlaySubmitAttackFacade::Submit(State, Request);

	TestTrue(
		TEXT("Gate permits structurally valid request"),
		Result.SubmissionGateResult.bCanSubmit);
	TestFalse(TEXT("Step failure fails submission"), Result.bSuccess);
	TestEqual(
		TEXT("Step-failure code is returned"),
		Result.Code,
		EMatchPlaySubmitAttackFacadeCode::AttackStepFailed);
	TestFalse(TEXT("Step failure is retained"), Result.AttackStepResult.bSuccess);
	TestTrue(
		TEXT("After state remains atomic"),
		MatchPlaySubmitAttackFacadeTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeNoLoopTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.ProductionCodeHasNoMatchLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeNoLoopTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackFacadeTests::LoadProductionSource();

	TestEqual(
		TEXT("AttackStep is called from one source location"),
		Source.Replace(
			TEXT("FMatchPlayAttackStep::ExecuteAttackStep"),
			TEXT("")).Len(),
		Source.Len()
			- FCString::Strlen(
				TEXT("FMatchPlayAttackStep::ExecuteAttackStep")));
	TestFalse(TEXT("No while loop"), Source.Contains(TEXT("while (")));
	TestFalse(TEXT("No for loop"), Source.Contains(TEXT("for (")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeNoAutoSelectionOrAITest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.ProductionCodeHasNoAutoSelectionOrAI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeNoAutoSelectionOrAITest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackFacadeTests::LoadProductionSource();

	TestFalse(TEXT("No automatic CardId selection"), Source.Contains(TEXT("SelectCard")));
	TestFalse(TEXT("No AI module dependency"), Source.Contains(TEXT("AIModule")));
	TestFalse(
		TEXT("No AI controller dependency"),
		Source.Contains(TEXT("AIController")));
	TestTrue(
		TEXT("Uses caller-provided request"),
		Source.Contains(TEXT("BeforeState, Request")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.ProductionCodeHasNoRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackFacadeTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackFacadeDependencyBoundaryTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackFacade.ProductionCodeUsesRequiredBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackFacadeDependencyBoundaryTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackFacadeTests::LoadProductionSource();

	TestTrue(
		TEXT("Submission gate is called"),
		Source.Contains(TEXT("FMatchPlaySubmissionGate::CanSubmit")));
	TestTrue(
		TEXT("Attack step is called"),
		Source.Contains(TEXT("FMatchPlayAttackStep::ExecuteAttackStep")));
	TestFalse(
		TEXT("No direct FormulaAttackFlow call"),
		Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(
		TEXT("No direct FormulaResolver call"),
		Source.Contains(TEXT("UFormulaResolver::")));
	TestFalse(TEXT("No skill dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No card database dependency"),
		Source.Contains(TEXT("Database")));
	TestFalse(TEXT("No UI dependency"), Source.Contains(TEXT("UI")));
	TestFalse(
		TEXT("No Blueprint dependency"),
		Source.Contains(TEXT("Blueprint")));
	return true;
}

#endif
