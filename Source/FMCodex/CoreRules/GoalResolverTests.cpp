#include "GoalResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace GoalResolverTests
{
	FMatchRuntimeState MakeRuntimeState(
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 5,
		const int32 PlayerAUsed = 2,
		const int32 PlayerBTotal = 6,
		const int32 PlayerBUsed = 2)
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = PlayerATotal;
		State.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.PlayerAState.Score = PlayerAScore;
		State.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.PlayerBState.Score = PlayerBScore;
		State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.OpeningResultSnapshot.bSuccess = true;
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount =
			PlayerATotal;
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount =
			PlayerBTotal;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverPlayerAGoalTest,
	"FMCodex.CoreRules.GoalResolver.PlayerAGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverPlayerAGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(2, 1);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("PlayerA goal succeeds"), Result.bSuccess);
	TestEqual(TEXT("Scoring side is PlayerA"), Result.ScoringSide, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA score before is reported"), Result.PlayerAScoreBefore, 2);
	TestEqual(TEXT("PlayerB score before is reported"), Result.PlayerBScoreBefore, 1);
	TestEqual(TEXT("PlayerA score increases"), Result.PlayerAScoreAfter, 3);
	TestEqual(TEXT("PlayerB score does not increase"), Result.PlayerBScoreAfter, 1);
	TestEqual(TEXT("Updated PlayerA score increases"), Result.UpdatedRuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("Updated PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("Successful result has no error"), Result.ErrorCode, EGoalResolveErrorCode::None);
	TestTrue(TEXT("Successful result has no error message"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverPlayerBGoalTest,
	"FMCodex.CoreRules.GoalResolver.PlayerBGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverPlayerBGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(1, 3);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("PlayerB goal succeeds"), Result.bSuccess);
	TestEqual(TEXT("Scoring side is PlayerB"), Result.ScoringSide, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerA score does not increase"), Result.PlayerAScoreAfter, 1);
	TestEqual(TEXT("PlayerB score increases"), Result.PlayerBScoreAfter, 4);
	TestEqual(TEXT("Updated PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Updated PlayerB score increases"), Result.UpdatedRuntimeState.PlayerBState.Score, 4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverInputUnchangedTest,
	"FMCodex.CoreRules.GoalResolver.InputStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverInputUnchangedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(2, 2);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("Goal succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input PlayerA score remains unchanged"), State.PlayerAState.Score, 2);
	TestEqual(TEXT("Input PlayerB score remains unchanged"), State.PlayerBState.Score, 2);
	TestEqual(TEXT("Updated copy records the goal"), Result.UpdatedRuntimeState.PlayerAState.Score, 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverPreservesUnrelatedStateTest,
	"FMCodex.CoreRules.GoalResolver.PreservesUnrelatedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverPreservesUnrelatedStateTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(0, 0);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("Goal succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA total attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.TotalAttackCount,
		State.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerA used attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount,
		State.PlayerAState.UsedAttackCount);
	TestEqual(
		TEXT("PlayerB total attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerBState.TotalAttackCount,
		State.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerB used attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount,
		State.PlayerBState.UsedAttackCount);
	TestEqual(
		TEXT("Current attacker is unchanged"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		State.CurrentAttackingPlayer);
	TestEqual(
		TEXT("Opening PlayerA attack count is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount,
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount);
	TestEqual(
		TEXT("Opening PlayerB attack count is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount,
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount);
	TestEqual(
		TEXT("Opening snapshot success is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.bSuccess,
		State.OpeningResultSnapshot.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverNegativeScoreTest,
	"FMCodex.CoreRules.GoalResolver.NegativeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverNegativeScoreTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(-1, 0);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerA);

	TestFalse(TEXT("Negative score is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Negative score reports invalid runtime"),
		Result.ErrorCode,
		EGoalResolveErrorCode::InvalidRuntimeState);
	TestEqual(TEXT("Failed result keeps PlayerA score"), Result.PlayerAScoreAfter, -1);
	TestEqual(TEXT("Failed result keeps PlayerB score"), Result.PlayerBScoreAfter, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverInvalidRuntimeStateTest,
	"FMCodex.CoreRules.GoalResolver.InvalidRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverInvalidRuntimeStateTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState UninitializedState;
	const FGoalResolveResult UninitializedResult =
		FGoalResolver::RecordGoal(
			UninitializedState,
			EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Uninitialized runtime is rejected"), UninitializedResult.bSuccess);
	TestEqual(
		TEXT("Uninitialized runtime reports invalid state"),
		UninitializedResult.ErrorCode,
		EGoalResolveErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState InvalidCountsState =
		GoalResolverTests::MakeRuntimeState(0, 0, 5, 6, 6, 2);
	const FGoalResolveResult InvalidCountsResult =
		FGoalResolver::RecordGoal(
			InvalidCountsState,
			EInitialTurnOrderPlayer::PlayerB);
	TestFalse(TEXT("Invalid attack counts are rejected"), InvalidCountsResult.bSuccess);
	TestEqual(
		TEXT("Invalid attack counts report invalid state"),
		InvalidCountsResult.ErrorCode,
		EGoalResolveErrorCode::InvalidRuntimeState);
	TestTrue(
		TEXT("Runtime validation error remains readable"),
		InvalidCountsResult.ErrorMessage.Contains(
			TEXT("Match end validation failed")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverInvalidScoringSideTest,
	"FMCodex.CoreRules.GoalResolver.InvalidScoringSide",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverInvalidScoringSideTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState();
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::None);

	TestFalse(TEXT("None scoring side is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Invalid side reports its error"),
		Result.ErrorCode,
		EGoalResolveErrorCode::InvalidScoringSide);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.PlayerAScoreAfter, 0);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.PlayerBScoreAfter, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverMatchAlreadyEndedTest,
	"FMCodex.CoreRules.GoalResolver.MatchAlreadyEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverMatchAlreadyEndedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState(2, 1, 5, 5, 6, 6);
	const FGoalResolveResult Result =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerA);

	TestFalse(TEXT("Goal after match end is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Ended match reports its error"),
		Result.ErrorCode,
		EGoalResolveErrorCode::MatchAlreadyEnded);
	TestEqual(TEXT("PlayerA score is not increased"), Result.PlayerAScoreAfter, 2);
	TestEqual(TEXT("PlayerB score is not increased"), Result.PlayerBScoreAfter, 1);
	TestEqual(TEXT("Updated state keeps PlayerA score"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverConsecutiveGoalsTest,
	"FMCodex.CoreRules.GoalResolver.ConsecutiveGoals",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverConsecutiveGoalsTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		GoalResolverTests::MakeRuntimeState();
	const FGoalResolveResult FirstResult =
		FGoalResolver::RecordGoal(State, EInitialTurnOrderPlayer::PlayerA);
	const FGoalResolveResult SecondResult =
		FGoalResolver::RecordGoal(
			FirstResult.UpdatedRuntimeState,
			EInitialTurnOrderPlayer::PlayerA);
	const FGoalResolveResult ThirdResult =
		FGoalResolver::RecordGoal(
			SecondResult.UpdatedRuntimeState,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("First goal succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second goal succeeds"), SecondResult.bSuccess);
	TestTrue(TEXT("Third goal succeeds"), ThirdResult.bSuccess);
	TestEqual(TEXT("PlayerA can score consecutively"), SecondResult.PlayerAScoreAfter, 2);
	TestEqual(TEXT("PlayerA final score is retained"), ThirdResult.PlayerAScoreAfter, 2);
	TestEqual(TEXT("PlayerB final score increases"), ThirdResult.PlayerBScoreAfter, 1);
	TestEqual(TEXT("Original PlayerA score remains zero"), State.PlayerAState.Score, 0);
	TestEqual(TEXT("Original PlayerB score remains zero"), State.PlayerBState.Score, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FGoalResolverIgnoresRandomInputsTest,
	"FMCodex.CoreRules.GoalResolver.IgnoresRandomInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGoalResolverIgnoresRandomInputsTest::RunTest(const FString& Parameters)
{
	FMatchRuntimeState FirstState =
		GoalResolverTests::MakeRuntimeState(1, 1);
	FirstState.OpeningResultSnapshot.bSuccess = true;
	FirstState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 99;
	FirstState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerA;

	FMatchRuntimeState SecondState = FirstState;
	SecondState.OpeningResultSnapshot.bSuccess = false;
	SecondState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 1;
	SecondState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FGoalResolveResult FirstResult =
		FGoalResolver::RecordGoal(
			FirstState,
			EInitialTurnOrderPlayer::PlayerB);
	const FGoalResolveResult SecondResult =
		FGoalResolver::RecordGoal(
			SecondState,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("First unrelated-state variant succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second unrelated-state variant succeeds"), SecondResult.bSuccess);
	TestEqual(
		TEXT("Opening and turn-order state do not affect PlayerA score"),
		FirstResult.PlayerAScoreAfter,
		SecondResult.PlayerAScoreAfter);
	TestEqual(
		TEXT("Opening and turn-order state do not affect PlayerB score"),
		FirstResult.PlayerBScoreAfter,
		SecondResult.PlayerBScoreAfter);
	return true;
}

#endif
