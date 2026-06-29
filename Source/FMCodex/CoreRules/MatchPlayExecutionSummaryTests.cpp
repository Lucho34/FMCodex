#include "MatchPlayExecutionSummary.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayExecutionSummaryTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));

	FMatchPlayState MakeState(
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
		RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
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
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlayAttackRequest MakeRequest(const bool bGoal = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = EInitialTurnOrderPlayer::PlayerA;
		Request.CardId = CardA1;
		Request.bHasExternalFormulaInput = true;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = bGoal ? 6.0f : 4.0f;
		Request.FormulaInput.Defender.BaseValue = bGoal ? 4.0f : 6.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	FMatchPlayAttackExecutionResult ExecuteSuccess(
		const FMatchPlayState& State,
		const bool bGoal = true)
	{
		return FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MakeRequest(bGoal));
	}

	FMatchPlayAttackExecutionResult ExecuteValidationFailure(
		const FMatchPlayState& State)
	{
		FMatchPlayAttackRequest Request = MakeRequest();
		Request.PlayerSide = EInitialTurnOrderPlayer::None;
		return FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			Request);
	}

	FMatchPlayAttackExecutionResult ExecuteFlowFailure(
		const FMatchPlayState& State)
	{
		FMatchPlayAttackRequest Request = MakeRequest();
		Request.FormulaInput.FormulaType = EFormulaType::Transition;
		return FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			Request);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryBeforeTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryIncludesBeforeStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryBeforeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState(2, 1);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("Before PlayerA score is retained"), Summary.BeforeStatus.PlayerAScore, 2);
	TestEqual(TEXT("Before PlayerB score is retained"), Summary.BeforeStatus.PlayerBScore, 1);
	TestEqual(TEXT("Before PlayerA available cards are retained"), Summary.BeforeStatus.PlayerAAvailableCardCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryAfterTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SuccessfulExecutionIncludesAfterStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryAfterTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestTrue(TEXT("Successful execution has After status"), Summary.bHasAfterStatus);
	TestTrue(TEXT("After runtime is initialized"), Summary.AfterStatus.bIsRuntimeInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryFailureNoAfterTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.FailedExecutionDoesNotUseDefaultUpdatedStateAsAfterStatus",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryFailureNoAfterTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteValidationFailure(
				Before));

	TestFalse(TEXT("Failed execution has no After status"), Summary.bHasAfterStatus);
	TestFalse(TEXT("Default After status is not marked initialized"), Summary.AfterStatus.bIsRuntimeInitialized);
	TestEqual(TEXT("Default After PlayerA score remains zero"), Summary.AfterStatus.PlayerAScore, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummarySuccessFlagTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsExecutionSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummarySuccessFlagTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Success =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));
	const FMatchPlayExecutionSummary Failure =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteValidationFailure(
				Before));

	TestTrue(TEXT("Success is reported"), Success.bExecutionSuccess);
	TestFalse(TEXT("Failure is reported"), Failure.bExecutionSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryGoalTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsGoalScored",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryGoalTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Goal =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before, true));
	const FMatchPlayExecutionSummary NoGoal =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before, false));

	TestTrue(TEXT("Goal is reported"), Goal.bGoalScored);
	TestFalse(TEXT("No goal is reported"), NoGoal.bGoalScored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryResolvedTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsAttackResolved",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryResolvedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestTrue(TEXT("Resolved attack is reported"), Summary.bAttackResolved);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryEndedTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsMatchEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState(
			0,
			0,
			1,
			0,
			1,
			1);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestTrue(TEXT("Match end is reported"), Summary.bMatchEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryResultTypeTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsMatchResultType",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryResultTypeTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState(
			0,
			0,
			1,
			0,
			1,
			1);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("HomeWin is reported"), Summary.MatchResultType, EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryRequestTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsRequestPlayerAndCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("Request player is reported"), Summary.RequestPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Request CardId is reported"), Summary.CardId, MatchPlayExecutionSummaryTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryValidationFailureTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsValidationFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryValidationFailureTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteValidationFailure(
				Before));

	TestTrue(TEXT("Validation failure is marked"), Summary.bValidationFailed);
	TestFalse(TEXT("AttackFlow failure is not marked"), Summary.bAttackFlowFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryFlowFailureTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryReportsAttackFlowFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryFlowFailureTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteFlowFailure(Before));

	TestFalse(TEXT("Validation failure is not marked"), Summary.bValidationFailed);
	TestTrue(TEXT("AttackFlow failure is marked"), Summary.bAttackFlowFailed);
	TestFalse(TEXT("Failed AttackFlow has no After status"), Summary.bHasAfterStatus);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryErrorTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryPreservesErrorCodeAndMessage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryErrorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayAttackExecutionResult Execution =
		MatchPlayExecutionSummaryTests::ExecuteValidationFailure(Before);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			Execution);

	TestEqual(TEXT("Execution error code is retained"), Summary.ErrorCode, Execution.ErrorCode);
	TestEqual(TEXT("Execution error message is retained"), Summary.ErrorMessage, Execution.ErrorMessage);
	TestTrue(TEXT("Error remains readable"), !Summary.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryScoreDeltaTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryShowsScoreDeltaAfterGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryScoreDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState(2, 1);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("Before PlayerA score is two"), Summary.BeforeStatus.PlayerAScore, 2);
	TestEqual(TEXT("After PlayerA score is three"), Summary.AfterStatus.PlayerAScore, 3);
	TestEqual(TEXT("PlayerB score remains one"), Summary.AfterStatus.PlayerBScore, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryAttackDeltaTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryShowsAttackCountDeltaAfterSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryAttackDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("Before PlayerA remaining attacks are three"), Summary.BeforeStatus.PlayerARemainingAttackCount, 3);
	TestEqual(TEXT("After PlayerA remaining attacks are two"), Summary.AfterStatus.PlayerARemainingAttackCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryCardDeltaTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryShowsCardCountDeltaAfterSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryCardDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(
			Before,
			MatchPlayExecutionSummaryTests::ExecuteSuccess(Before));

	TestEqual(TEXT("Before available count is two"), Summary.BeforeStatus.PlayerAAvailableCardCount, 2);
	TestEqual(TEXT("After available count is one"), Summary.AfterStatus.PlayerAAvailableCardCount, 1);
	TestEqual(TEXT("Before used count is zero"), Summary.BeforeStatus.PlayerAUsedCardCount, 0);
	TestEqual(TEXT("After used count is one"), Summary.AfterStatus.PlayerAUsedCardCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryBeforeInputTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryDoesNotModifyBeforeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryBeforeInputTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState(2, 1);
	const TArray<FName> Available =
		Before.CardUsageState.PlayerACardUsageState.AvailableCardIds;
	const FMatchPlayAttackExecutionResult Execution =
		MatchPlayExecutionSummaryTests::ExecuteSuccess(Before);
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);

	TestTrue(TEXT("Summary is successful"), Summary.bExecutionSuccess);
	TestEqual(TEXT("Before input score is unchanged"), Before.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Before input used attacks are unchanged"), Before.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(TEXT("Before input cards are unchanged"), Before.CardUsageState.PlayerACardUsageState.AvailableCardIds == Available);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryExecutionInputTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryDoesNotModifyExecutionResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryExecutionInputTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayAttackExecutionResult Execution =
		MatchPlayExecutionSummaryTests::ExecuteSuccess(Before);
	const FName PlayedCardId = Execution.AttackRequest.CardId;
	const int32 UpdatedScore =
		Execution.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score;
	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);

	TestTrue(TEXT("Summary is successful"), Summary.bExecutionSuccess);
	TestEqual(TEXT("Execution request CardId is unchanged"), Execution.AttackRequest.CardId, PlayedCardId);
	TestEqual(TEXT("Execution updated score is unchanged"), Execution.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score, UpdatedScore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryNoExecutorTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryDoesNotCallExecutor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryNoExecutorTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	FMatchPlayAttackExecutionResult Execution;
	Execution.AttackRequest =
		MatchPlayExecutionSummaryTests::MakeRequest();
	Execution.ErrorCode =
		EMatchPlayAttackExecutionErrorCode::RequestValidationFailed;
	Execution.ErrorMessage = TEXT("Synthetic validation failure.");

	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);

	TestFalse(TEXT("Synthetic failure remains failed"), Summary.bExecutionSuccess);
	TestFalse(TEXT("Synthetic failure has no After status"), Summary.bHasAfterStatus);
	TestEqual(TEXT("Before state is still queried"), Summary.BeforeStatus.PlayerAAvailableCardCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryNoFlowTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.SummaryDoesNotCallAttackFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryNoFlowTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	FMatchPlayAttackExecutionResult Execution;
	Execution.AttackRequest =
		MatchPlayExecutionSummaryTests::MakeRequest();
	Execution.ErrorCode =
		EMatchPlayAttackExecutionErrorCode::AttackFlowFailed;
	Execution.ErrorMessage = TEXT("Synthetic attack flow failure.");

	const FMatchPlayExecutionSummary Summary =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);

	TestTrue(TEXT("Synthetic AttackFlow failure is marked"), Summary.bAttackFlowFailed);
	TestFalse(TEXT("Synthetic AttackFlow failure has no After status"), Summary.bHasAfterStatus);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExecutionSummaryRepeatedTest,
	"FMCodex.CoreRules.MatchPlayExecutionSummary.RepeatedSummaryIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExecutionSummaryRepeatedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState Before =
		MatchPlayExecutionSummaryTests::MakeState();
	const FMatchPlayAttackExecutionResult Execution =
		MatchPlayExecutionSummaryTests::ExecuteSuccess(Before);
	const FMatchPlayExecutionSummary First =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);
	const FMatchPlayExecutionSummary Second =
		FMatchPlayExecutionSummaryBuilder::BuildSummary(Before, Execution);

	TestEqual(TEXT("Repeated success is stable"), First.bExecutionSuccess, Second.bExecutionSuccess);
	TestEqual(TEXT("Repeated Before score is stable"), First.BeforeStatus.PlayerAScore, Second.BeforeStatus.PlayerAScore);
	TestEqual(TEXT("Repeated After score is stable"), First.AfterStatus.PlayerAScore, Second.AfterStatus.PlayerAScore);
	TestEqual(TEXT("Repeated card count is stable"), First.AfterStatus.PlayerAUsedCardCount, Second.AfterStatus.PlayerAUsedCardCount);
	return true;
}

#endif
