#include "MatchPlayAttackStep.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayAttackStepTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardB2(TEXT("CardB2"));
	const FName SharedCard(TEXT("SharedCard"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = PlayerAScore;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = PlayerBScore;
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
			{ CardA1, CardA2, SharedCard };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1, CardB2, SharedCard };
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
		const bool bGoal = true,
		const bool bHasFormulaInput = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = bHasFormulaInput;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = bGoal ? 6.0f : 4.0f;
		Request.FormulaInput.Defender.BaseValue = bGoal ? 4.0f : 6.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	bool IsUpdatedStateEmpty(const FMatchPlayState& State)
	{
		return !State.RuntimeState.bIsInitialized
			&& State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState.UsedCardIds.IsEmpty()
			&& State.DeploymentSlotCatalog.Slots.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepSuccessTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ValidRequestSucceeds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepSuccessTest::RunTest(const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Step succeeds for valid request"), Result.bSuccess);
	TestEqual(TEXT("Step has no error"), Result.ErrorCode, EMatchPlayAttackStepErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepExecutionResultTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ReturnsExecutionResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepExecutionResultTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Execution result succeeds"), Result.ExecutionResult.bSuccess);
	TestTrue(TEXT("Execution result resolves attack"), Result.ExecutionResult.bAttackResolved);
	TestEqual(TEXT("Execution result keeps request CardId"), Result.ExecutionResult.AttackRequest.CardId, MatchPlayAttackStepTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepSummaryTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ReturnsExecutionSummary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepSummaryTest::RunTest(const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Summary reports success"), Result.ExecutionSummary.bExecutionSuccess);
	TestEqual(TEXT("Summary keeps request player"), Result.ExecutionSummary.RequestPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Summary keeps CardId"), Result.ExecutionSummary.CardId, MatchPlayAttackStepTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepAfterStatusTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.SuccessSummaryHasAfterStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepAfterStatusTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Success summary has After status"), Result.ExecutionSummary.bHasAfterStatus);
	TestTrue(TEXT("After runtime remains initialized"), Result.ExecutionSummary.AfterStatus.bIsRuntimeInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepCardDeltaTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.SuccessShowsConsumedCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepCardDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestFalse(
		TEXT("Played card leaves available cards"),
		Result.ExecutionResult.UpdatedMatchPlayState.CardUsageState
			.PlayerACardUsageState.AvailableCardIds.Contains(
				MatchPlayAttackStepTests::CardA1));
	TestTrue(
		TEXT("Played card enters used cards"),
		Result.ExecutionResult.UpdatedMatchPlayState.CardUsageState
			.PlayerACardUsageState.UsedCardIds.Contains(
				MatchPlayAttackStepTests::CardA1));
	TestEqual(TEXT("Summary shows one fewer available card"), Result.ExecutionSummary.AfterStatus.PlayerAAvailableCardCount, 2);
	TestEqual(TEXT("Summary shows one used card"), Result.ExecutionSummary.AfterStatus.PlayerAUsedCardCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepOpportunityDeltaTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.SuccessShowsConsumedAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepOpportunityDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestEqual(TEXT("Before PlayerA has three remaining attacks"), Result.ExecutionSummary.BeforeStatus.PlayerARemainingAttackCount, 3);
	TestEqual(TEXT("After PlayerA has two remaining attacks"), Result.ExecutionSummary.AfterStatus.PlayerARemainingAttackCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepGoalDeltaTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.GoalSummaryShowsScoreChange",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepGoalDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				2,
				1),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Goal is reported"), Result.ExecutionSummary.bGoalScored);
	TestEqual(TEXT("Before PlayerA score is two"), Result.ExecutionSummary.BeforeStatus.PlayerAScore, 2);
	TestEqual(TEXT("After PlayerA score is three"), Result.ExecutionSummary.AfterStatus.PlayerAScore, 3);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.ExecutionSummary.AfterStatus.PlayerBScore, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepValidationFailureNoAfterTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ValidationFailureSummaryHasNoAfterStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepValidationFailureNoAfterTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid request fails"), Result.bSuccess);
	TestFalse(TEXT("Validation failure has no After status"), Result.ExecutionSummary.bHasAfterStatus);
	TestTrue(TEXT("Validation failure is marked"), Result.ExecutionSummary.bValidationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepValidationFailureAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ValidationFailureDoesNotChangeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepValidationFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayAttackStepTests::MakeState();
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			State,
			MatchPlayAttackStepTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackStepTests::MissingCard));

	TestFalse(TEXT("Missing card request fails"), Result.bSuccess);
	TestTrue(TEXT("Failure has empty updated state"), MatchPlayAttackStepTests::IsUpdatedStateEmpty(Result.ExecutionResult.UpdatedMatchPlayState));
	TestTrue(TEXT("Input card remains available"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(MatchPlayAttackStepTests::CardA1));
	TestEqual(TEXT("Input attack opportunity is not consumed"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("Input score is unchanged"), State.RuntimeState.PlayerAState.Score, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepFlowFailureNoAfterTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.AttackFlowFailureSummaryHasNoAfterStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepFlowFailureNoAfterTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayAttackRequest Request =
		MatchPlayAttackStepTests::MakeRequest();
	Request.FormulaInput.FormulaType = EFormulaType::Transition;

	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			Request);

	TestFalse(TEXT("AttackFlow failure fails step"), Result.bSuccess);
	TestTrue(TEXT("AttackFlow failure is marked"), Result.ExecutionSummary.bAttackFlowFailed);
	TestFalse(TEXT("AttackFlow failure has no After status"), Result.ExecutionSummary.bHasAfterStatus);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepInputStateTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.DoesNotModifyInputMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepInputStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackStepTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			State,
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Step succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Input PlayerA used attack count remains unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(TEXT("Input available cards remain unchanged"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds == PlayerAAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepInputRequestTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.DoesNotModifyInputAttackRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepInputRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackStepTests::MakeRequest();
	const FName OriginalCardId = Request.CardId;
	const float OriginalAttackerValue =
		Request.FormulaInput.Attacker.BaseValue;

	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			Request);

	TestTrue(TEXT("Step succeeds"), Result.bSuccess);
	TestEqual(TEXT("Request CardId remains unchanged"), Request.CardId, OriginalCardId);
	TestEqual(TEXT("Request formula input remains unchanged"), Request.FormulaInput.Attacker.BaseValue, OriginalAttackerValue);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepPlayerAIsolationTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.PlayerARequestOnlyUpdatesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepPlayerAIsolationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest());

	TestTrue(TEXT("Step succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA used card count changes"), Result.ExecutionSummary.AfterStatus.PlayerAUsedCardCount, 1);
	TestEqual(TEXT("PlayerB available count remains unchanged"), Result.ExecutionSummary.AfterStatus.PlayerBAvailableCardCount, 3);
	TestEqual(TEXT("PlayerB used count remains zero"), Result.ExecutionSummary.AfterStatus.PlayerBUsedCardCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepPlayerBIsolationTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.PlayerBRequestOnlyUpdatesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepPlayerBIsolationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB),
			MatchPlayAttackStepTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackStepTests::CardB1));

	TestTrue(TEXT("Step succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerB used card count changes"), Result.ExecutionSummary.AfterStatus.PlayerBUsedCardCount, 1);
	TestEqual(TEXT("PlayerA available count remains unchanged"), Result.ExecutionSummary.AfterStatus.PlayerAAvailableCardCount, 3);
	TestEqual(TEXT("PlayerA used count remains zero"), Result.ExecutionSummary.AfterStatus.PlayerAUsedCardCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepSharedCardIsolationTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.SameCardIdAcrossPlayersIsIsolated",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepSharedCardIsolationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackStepResult Result =
		FMatchPlayAttackStep::ExecuteAttackStep(
			MatchPlayAttackStepTests::MakeState(),
			MatchPlayAttackStepTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackStepTests::SharedCard));

	TestTrue(TEXT("Shared CardId request succeeds for PlayerA"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerB still has its same-name card available"),
		Result.ExecutionResult.UpdatedMatchPlayState.CardUsageState
			.PlayerBCardUsageState.AvailableCardIds.Contains(
				MatchPlayAttackStepTests::SharedCard));
	TestFalse(
		TEXT("PlayerA used its same-name card"),
		Result.ExecutionResult.UpdatedMatchPlayState.CardUsageState
			.PlayerACardUsageState.AvailableCardIds.Contains(
				MatchPlayAttackStepTests::SharedCard));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepRepeatedInvalidRequestTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.RepeatedInvalidRequestIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepRepeatedInvalidRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayAttackStepTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackStepTests::MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAttackStepTests::MissingCard);

	const FMatchPlayAttackStepResult First =
		FMatchPlayAttackStep::ExecuteAttackStep(State, Request);
	const FMatchPlayAttackStepResult Second =
		FMatchPlayAttackStep::ExecuteAttackStep(State, Request);

	TestFalse(TEXT("First invalid request fails"), First.bSuccess);
	TestFalse(TEXT("Second invalid request fails"), Second.bSuccess);
	TestEqual(TEXT("Error code is consistent"), First.ErrorCode, Second.ErrorCode);
	TestEqual(TEXT("Underlying validation error is consistent"), First.ExecutionResult.UnderlyingRequestValidationErrorCode, Second.ExecutionResult.UnderlyingRequestValidationErrorCode);
	TestEqual(TEXT("Input state remains unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepProductionNoDirectAttackFlowTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ProductionCodeDoesNotDirectlyCallMatchPlayAttackFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepProductionNoDirectAttackFlowTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	const bool bLoaded = FFileHelper::LoadFileToString(
		Source,
		*(FPaths::ProjectDir()
			/ TEXT("Source/FMCodex/CoreRules/MatchPlayAttackStep.cpp")));

	TestTrue(TEXT("Production source can be loaded"), bLoaded);
	TestFalse(TEXT("No direct MatchPlayAttackFlow call"), Source.Contains(TEXT("FMatchPlayAttackFlow::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackStepProductionNoFormulaFlowTest,
	"FMCodex.CoreRules.MatchPlayAttackStep.ProductionCodeDoesNotDirectlyCallFormulaFlows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackStepProductionNoFormulaFlowTest::RunTest(
	const FString& Parameters)
{
	FString Source;
	const bool bLoaded = FFileHelper::LoadFileToString(
		Source,
		*(FPaths::ProjectDir()
			/ TEXT("Source/FMCodex/CoreRules/MatchPlayAttackStep.cpp")));

	TestTrue(TEXT("Production source can be loaded"), bLoaded);
	TestFalse(TEXT("No direct FormulaAttackFlow call"), Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(TEXT("No direct FormulaResolver call"), Source.Contains(TEXT("UFormulaResolver::")));
	return true;
}

#endif
