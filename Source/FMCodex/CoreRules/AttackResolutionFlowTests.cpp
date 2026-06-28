#include "AttackResolutionFlow.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace AttackResolutionFlowTests
{
	FMatchRuntimeState MakeRuntimeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = PlayerATotal;
		State.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.PlayerAState.Score = PlayerAScore;
		State.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.PlayerBState.Score = PlayerBScore;
		State.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.CurrentAttackingPlayer = CurrentPlayer;
		State.OpeningResultSnapshot.bSuccess = true;
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount =
			PlayerATotal;
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount =
			PlayerBTotal;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowPlayerAGoalTest,
	"FMCodex.CoreRules.AttackResolutionFlow.PlayerAGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowPlayerAGoalTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestTrue(TEXT("PlayerA attack resolves"), Result.bSuccess);
	TestTrue(TEXT("Goal request is retained"), Result.bGoalScored);
	TestEqual(TEXT("Scoring side is PlayerA"), Result.ScoringSide, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA score increases"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 0);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestEqual(
		TEXT("Attack switches to PlayerB"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowPlayerBGoalTest,
	"FMCodex.CoreRules.AttackResolutionFlow.PlayerBGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowPlayerBGoalTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestTrue(TEXT("PlayerB attack resolves"), Result.bSuccess);
	TestEqual(TEXT("Scoring side is PlayerB"), Result.ScoringSide, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("PlayerB score increases"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("PlayerB attack is consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestEqual(
		TEXT("Attack switches to PlayerA"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowNoGoalTest,
	"FMCodex.CoreRules.AttackResolutionFlow.NoGoalStillConsumesAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowNoGoalTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, false);

	TestTrue(TEXT("Non-goal attack resolves"), Result.bSuccess);
	TestFalse(TEXT("No goal is reported"), Result.bGoalScored);
	TestEqual(TEXT("There is no scoring side"), Result.ScoringSide, EInitialTurnOrderPlayer::None);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowInputUnchangedTest,
	"FMCodex.CoreRules.AttackResolutionFlow.InputStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestTrue(TEXT("Attack resolves"), Result.bSuccess);
	TestEqual(TEXT("Input PlayerA score is unchanged"), State.PlayerAState.Score, 1);
	TestEqual(TEXT("Input PlayerB score is unchanged"), State.PlayerBState.Score, 1);
	TestEqual(TEXT("Input PlayerA used count is unchanged"), State.PlayerAState.UsedAttackCount, 0);
	TestEqual(
		TEXT("Input current attacker is unchanged"),
		State.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Returned PlayerA score changes"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Returned PlayerA used count changes"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowMatchContinuesTest,
	"FMCodex.CoreRules.AttackResolutionFlow.MatchContinues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowMatchContinuesTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState();
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, false);

	TestTrue(TEXT("Attack resolves"), Result.bSuccess);
	TestFalse(TEXT("Match continues while attacks remain"), Result.bMatchEnded);
	TestEqual(
		TEXT("No final result exists"),
		Result.MatchResultType,
		EMatchResultType::NotEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowLastAttackTest,
	"FMCodex.CoreRules.AttackResolutionFlow.LastAttackEndsMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowLastAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			0,
			1,
			1);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestTrue(TEXT("Last attack resolves"), Result.bSuccess);
	TestTrue(TEXT("Last attack ends the match"), Result.bMatchEnded);
	TestEqual(TEXT("Last goal is recorded"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Last attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestEqual(
		TEXT("No current attacker remains"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::None);
	TestEqual(
		TEXT("Final result is resolved"),
		Result.MatchResultType,
		EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowAlreadyEndedTest,
	"FMCodex.CoreRules.AttackResolutionFlow.MatchAlreadyEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowAlreadyEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None,
			2,
			1,
			1,
			1,
			1,
			1);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestFalse(TEXT("Attack after match end is rejected"), Result.bSuccess);
	TestTrue(TEXT("Ended state is reported"), Result.bMatchEnded);
	TestEqual(
		TEXT("Ended match reports its error"),
		Result.ErrorCode,
		EAttackResolutionFlowErrorCode::MatchAlreadyEnded);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerA used count is unchanged"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowInvalidRuntimeTest,
	"FMCodex.CoreRules.AttackResolutionFlow.InvalidRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowInvalidRuntimeTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState UninitializedState;
	const FAttackResolutionFlowResult UninitializedResult =
		FAttackResolutionFlow::ResolveAttack(UninitializedState, false);
	TestFalse(TEXT("Uninitialized runtime is rejected"), UninitializedResult.bSuccess);
	TestEqual(
		TEXT("Uninitialized runtime reports invalid state"),
		UninitializedResult.ErrorCode,
		EAttackResolutionFlowErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState NegativeScoreState =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			-1,
			0);
	const FAttackResolutionFlowResult NegativeScoreResult =
		FAttackResolutionFlow::ResolveAttack(NegativeScoreState, false);
	TestFalse(TEXT("Negative score is rejected"), NegativeScoreResult.bSuccess);
	TestEqual(
		TEXT("Negative score reports invalid state"),
		NegativeScoreResult.ErrorCode,
		EAttackResolutionFlowErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState InvalidCurrentPlayerState =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None);
	const FAttackResolutionFlowResult InvalidCurrentPlayerResult =
		FAttackResolutionFlow::ResolveAttack(
			InvalidCurrentPlayerState,
			false);
	TestFalse(TEXT("Invalid current player is rejected"), InvalidCurrentPlayerResult.bSuccess);
	TestEqual(
		TEXT("Invalid current player reports invalid state"),
		InvalidCurrentPlayerResult.ErrorCode,
		EAttackResolutionFlowErrorCode::InvalidRuntimeState);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowNoOpportunityTest,
	"FMCodex.CoreRules.AttackResolutionFlow.NoAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowNoOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			1,
			2,
			0);
	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestFalse(TEXT("Exhausted current player is rejected"), Result.bSuccess);
	TestFalse(TEXT("Match is not ended because PlayerB can attack"), Result.bMatchEnded);
	TestEqual(
		TEXT("Current-player exhaustion reports no opportunity"),
		Result.ErrorCode,
		EAttackResolutionFlowErrorCode::NoAttackOpportunity);
	TestEqual(TEXT("No goal is applied"), Result.UpdatedRuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("PlayerA used count is unchanged"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowPreservesStructuralStateTest,
	"FMCodex.CoreRules.AttackResolutionFlow.PreservesStructuralState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowPreservesStructuralStateTest::RunTest(
	const FString& Parameters)
{
	FMatchRuntimeState State =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	State.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerA;

	const FAttackResolutionFlowResult Result =
		FAttackResolutionFlow::ResolveAttack(State, true);

	TestTrue(TEXT("Attack resolves"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA total attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.TotalAttackCount,
		State.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerB total attack count is unchanged"),
		Result.UpdatedRuntimeState.PlayerBState.TotalAttackCount,
		State.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("Opening PlayerA attack count is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount,
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount);
	TestEqual(
		TEXT("Opening PlayerB attack count is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount,
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount);
	TestEqual(
		TEXT("Opening first player is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer,
		State.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer);
	TestTrue(TEXT("Opening snapshot success is unchanged"), Result.UpdatedRuntimeState.OpeningResultSnapshot.bSuccess);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackResolutionFlowFinalResultsTest,
	"FMCodex.CoreRules.AttackResolutionFlow.FinalMatchResults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackResolutionFlowFinalResultsTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState HomeWinState =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1,
			1,
			0,
			1,
			1);
	const FAttackResolutionFlowResult HomeWinResult =
		FAttackResolutionFlow::ResolveAttack(HomeWinState, false);
	TestTrue(TEXT("Home-win attack resolves"), HomeWinResult.bSuccess);
	TestTrue(TEXT("Home-win attack ends match"), HomeWinResult.bMatchEnded);
	TestEqual(
		TEXT("Home win is returned"),
		HomeWinResult.MatchResultType,
		EMatchResultType::HomeWin);

	const FMatchRuntimeState AwayWinState =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			2,
			1,
			0,
			1,
			1);
	const FAttackResolutionFlowResult AwayWinResult =
		FAttackResolutionFlow::ResolveAttack(AwayWinState, false);
	TestTrue(TEXT("Away-win attack resolves"), AwayWinResult.bSuccess);
	TestEqual(
		TEXT("Away win is returned"),
		AwayWinResult.MatchResultType,
		EMatchResultType::AwayWin);

	const FMatchRuntimeState DrawState =
		AttackResolutionFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			0,
			1,
			1);
	const FAttackResolutionFlowResult DrawResult =
		FAttackResolutionFlow::ResolveAttack(DrawState, false);
	TestTrue(TEXT("Draw attack resolves"), DrawResult.bSuccess);
	TestEqual(
		TEXT("Draw is returned"),
		DrawResult.MatchResultType,
		EMatchResultType::Draw);
	return true;
}

#endif
