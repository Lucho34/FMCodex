#include "MatchPlaySubmitAttackResultQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlaySubmitAttackResultQueryTests
{
	FMatchPlayState MakeState(
		const int32 PlayerAScore,
		const int32 PlayerBScore,
		const EInitialTurnOrderPlayer CurrentAttacker)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = 3;
		RuntimeState.PlayerAState.Score = PlayerAScore;
		RuntimeState.PlayerBState.TotalAttackCount = 3;
		RuntimeState.PlayerBState.Score = PlayerBScore;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentAttacker;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ FName(TEXT("CardA1")) };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ FName(TEXT("CardB1")) };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlaySubmitAttackFacadeResult MakeResult(
		const EMatchPlaySubmitAttackFacadeCode Code,
		const bool bSuccess,
		const FMatchPlayState& BeforeState,
		const FMatchPlayState& AfterState)
	{
		FMatchPlaySubmitAttackFacadeResult Result;
		Result.bSuccess = bSuccess;
		Result.Code = Code;
		Result.Reason = bSuccess
			? TEXT("Attack request was submitted and executed.")
			: TEXT("Attack request was not executed.");
		Result.ErrorMessage = bSuccess
			? FString()
			: Result.Reason;
		Result.BeforeState = BeforeState;
		Result.AfterState = AfterState;
		Result.ExecutionSummary.bExecutionSuccess = bSuccess;
		Result.ExecutionSummary.bAttackResolved = bSuccess;
		return Result;
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlaySubmitAttackResultQuery.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQuerySuccessTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.SuccessView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQuerySuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::Submitted,
			true,
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				1,
				0,
				EInitialTurnOrderPlayer::PlayerA),
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				2,
				0,
				EInitialTurnOrderPlayer::PlayerB));

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestTrue(TEXT("View reports successful submission"), View.bSubmittedSuccessfully);
	TestFalse(TEXT("View does not report rejection"), View.bWasRejectedBeforeExecution);
	TestEqual(
		TEXT("View retains submit code"),
		View.SubmitCode,
		EMatchPlaySubmitAttackFacadeCode::Submitted);
	TestEqual(
		TEXT("View retains before attacker"),
		View.BeforeCurrentAttacker,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("View retains after attacker"),
		View.AfterCurrentAttacker,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryRejectedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.GateRejectedView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryRejectedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackResultQueryTests::MakeState(
			1,
			0,
			EInitialTurnOrderPlayer::PlayerA);
	FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::SubmissionRejected,
			false,
			State,
			State);
	Result.SubmissionGateResult.Code =
		EMatchPlaySubmissionGateCode::NotCurrentAttacker;

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestFalse(TEXT("Rejected view is not successful"), View.bSubmittedSuccessfully);
	TestTrue(TEXT("View reports pre-execution rejection"), View.bWasRejectedBeforeExecution);
	TestFalse(TEXT("View does not report step failure"), View.bAttackStepFailed);
	TestEqual(TEXT("View retains rejection reason"), View.ErrorMessage, Result.ErrorMessage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryStepFailedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.AttackStepFailedView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryStepFailedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlaySubmitAttackResultQueryTests::MakeState(
			1,
			0,
			EInitialTurnOrderPlayer::PlayerA);
	FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::AttackStepFailed,
			false,
			State,
			State);
	Result.AttackStepResult.ErrorCode =
		EMatchPlayAttackStepErrorCode::ExecutionFailed;

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestFalse(TEXT("Failed step view is not successful"), View.bSubmittedSuccessfully);
	TestFalse(TEXT("Step failure is not gate rejection"), View.bWasRejectedBeforeExecution);
	TestTrue(TEXT("View reports step failure"), View.bAttackStepFailed);
	TestEqual(
		TEXT("View retains step-failure code"),
		View.SubmitCode,
		EMatchPlaySubmitAttackFacadeCode::AttackStepFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryScoreChangedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.ScoreChanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryScoreChangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::Submitted,
			true,
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				2,
				1,
				EInitialTurnOrderPlayer::PlayerA),
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				3,
				1,
				EInitialTurnOrderPlayer::PlayerB));

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestTrue(TEXT("Score change is reported"), View.bScoreChanged);
	TestEqual(TEXT("Before PlayerA score is retained"), View.BeforeScore.PlayerAScore, 2);
	TestEqual(TEXT("After PlayerA score is retained"), View.AfterScore.PlayerAScore, 3);
	TestEqual(TEXT("PlayerB score is retained"), View.AfterScore.PlayerBScore, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryScoreUnchangedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.ScoreUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryScoreUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::Submitted,
			true,
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				2,
				1,
				EInitialTurnOrderPlayer::PlayerA),
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				2,
				1,
				EInitialTurnOrderPlayer::PlayerB));

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestFalse(TEXT("Unchanged score is reported"), View.bScoreChanged);
	TestEqual(TEXT("PlayerA score is unchanged"), View.AfterScore.PlayerAScore, 2);
	TestEqual(TEXT("PlayerB score is unchanged"), View.AfterScore.PlayerBScore, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryMatchEndedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.MatchEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryMatchEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlaySubmitAttackResultQueryTests::MakeState(
			1,
			1,
			EInitialTurnOrderPlayer::PlayerA);
	FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::Submitted,
			true,
			BeforeState,
			BeforeState);
	Result.ExecutionSummary.bMatchEnded = true;
	Result.ExecutionSummary.MatchResultType = EMatchResultType::Draw;

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestTrue(TEXT("Match end is reported"), View.bMatchEnded);
	TestTrue(
		TEXT("Execution summary is retained"),
		View.ExecutionSummary.bMatchEnded);
	TestEqual(
		TEXT("Match result is retained"),
		View.ExecutionSummary.MatchResultType,
		EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.DoesNotModifyInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlaySubmitAttackFacadeResult Result =
		MatchPlaySubmitAttackResultQueryTests::MakeResult(
			EMatchPlaySubmitAttackFacadeCode::Submitted,
			true,
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				2,
				1,
				EInitialTurnOrderPlayer::PlayerA),
			MatchPlaySubmitAttackResultQueryTests::MakeState(
				3,
				1,
				EInitialTurnOrderPlayer::PlayerB));
	const FMatchPlaySubmitAttackFacadeResult BeforeQuery = Result;

	const FMatchPlaySubmitAttackResultView View =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result);

	TestTrue(TEXT("View was built"), View.bSubmittedSuccessfully);
	TestEqual(TEXT("Result success is unchanged"), Result.bSuccess, BeforeQuery.bSuccess);
	TestEqual(TEXT("Result code is unchanged"), Result.Code, BeforeQuery.Code);
	TestEqual(TEXT("Reason is unchanged"), Result.Reason, BeforeQuery.Reason);
	TestEqual(
		TEXT("Before score is unchanged"),
		Result.BeforeState.RuntimeState.PlayerAState.Score,
		BeforeQuery.BeforeState.RuntimeState.PlayerAState.Score);
	TestEqual(
		TEXT("After score is unchanged"),
		Result.AfterState.RuntimeState.PlayerAState.Score,
		BeforeQuery.AfterState.RuntimeState.PlayerAState.Score);
	TestTrue(
		TEXT("Before cards are unchanged"),
		Result.BeforeState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== BeforeQuery.BeforeState.CardUsageState
				.PlayerACardUsageState.AvailableCardIds);
	TestTrue(
		TEXT("After cards are unchanged"),
		Result.AfterState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds
			== BeforeQuery.AfterState.CardUsageState
				.PlayerBCardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryNoExecutionCallsTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.ProductionCodeHasNoExecutionCalls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryNoExecutionCallsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackResultQueryTests::LoadProductionSource();

	TestFalse(
		TEXT("Does not call Submit"),
		Source.Contains(TEXT("FMatchPlaySubmitAttackFacade::Submit")));
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
	FMatchPlaySubmitAttackResultQueryNoRandomnessTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.ProductionCodeHasNoRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryNoRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackResultQueryTests::LoadProductionSource();

	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlaySubmitAttackResultQueryNoExternalSystemsTest,
	"FMCodex.CoreRules.MatchPlaySubmitAttackResultQuery.ProductionCodeHasNoExternalSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlaySubmitAttackResultQueryNoExternalSystemsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlaySubmitAttackResultQueryTests::LoadProductionSource();

	TestFalse(
		TEXT("No UI dependency"),
		Source.Contains(TEXT("UI"), ESearchCase::CaseSensitive));
	TestFalse(
		TEXT("No Blueprint dependency"),
		Source.Contains(TEXT("Blueprint")));
	TestFalse(TEXT("No networking dependency"), Source.Contains(TEXT("Net")));
	TestFalse(TEXT("No Steam dependency"), Source.Contains(TEXT("Steam")));
	TestFalse(TEXT("No skill dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No card database dependency"),
		Source.Contains(TEXT("Database")));
	TestFalse(TEXT("No AI dependency"), Source.Contains(TEXT("AIModule")));
	return true;
}

#endif
