#include "MatchResultResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchResultResolverTests
{
	FMatchRuntimeState MakeRuntimeState(
		const int32 HomeScore,
		const int32 AwayScore,
		const int32 PlayerATotal = 5,
		const int32 PlayerAUsed = 5,
		const int32 PlayerBTotal = 6,
		const int32 PlayerBUsed = 6)
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = PlayerATotal;
		State.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.PlayerAState.Score = HomeScore;
		State.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.PlayerBState.Score = AwayScore;
		State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::None;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultNotEndedTest,
	"FMCodex.CoreRules.MatchResultResolver.MatchNotEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultNotEndedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchResultResolverTests::MakeRuntimeState(3, 1, 5, 4, 6, 6);
	const FMatchResultResolveResult Result =
		FMatchResultResolver::ResolveMatchResult(State);

	TestFalse(TEXT("An unfinished match has no resolved result"), Result.bSuccess);
	TestFalse(TEXT("The match is reported as not ended"), Result.bIsMatchEnded);
	TestEqual(TEXT("No winner is returned"), Result.ResultType, EMatchResultType::NotEnded);
	TestEqual(
		TEXT("Unfinished match reports its error"),
		Result.ErrorCode,
		EMatchResultResolveErrorCode::MatchNotEnded);
	TestTrue(TEXT("Unfinished match has a readable error"), !Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultHomeWinTest,
	"FMCodex.CoreRules.MatchResultResolver.HomeWin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultHomeWinTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchResultResolverTests::MakeRuntimeState(3, 1);
	const FMatchResultResolveResult Result =
		FMatchResultResolver::ResolveMatchResult(State);

	TestTrue(TEXT("Ended match resolves successfully"), Result.bSuccess);
	TestTrue(TEXT("The match is reported as ended"), Result.bIsMatchEnded);
	TestEqual(TEXT("Higher home score wins"), Result.ResultType, EMatchResultType::HomeWin);
	TestEqual(TEXT("PlayerA score is reported as home"), Result.HomeScore, 3);
	TestEqual(TEXT("PlayerB score is reported as away"), Result.AwayScore, 1);
	TestEqual(
		TEXT("Successful result has no error"),
		Result.ErrorCode,
		EMatchResultResolveErrorCode::None);
	TestTrue(TEXT("Successful result has no error message"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultAwayWinTest,
	"FMCodex.CoreRules.MatchResultResolver.AwayWin",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultAwayWinTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchResultResolverTests::MakeRuntimeState(0, 2);
	const FMatchResultResolveResult Result =
		FMatchResultResolver::ResolveMatchResult(State);

	TestTrue(TEXT("Ended match resolves successfully"), Result.bSuccess);
	TestEqual(TEXT("Higher away score wins"), Result.ResultType, EMatchResultType::AwayWin);
	TestEqual(TEXT("Home score is reported"), Result.HomeScore, 0);
	TestEqual(TEXT("Away score is reported"), Result.AwayScore, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultDrawTest,
	"FMCodex.CoreRules.MatchResultResolver.Draw",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultDrawTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchResultResolverTests::MakeRuntimeState(2, 2);
	const FMatchResultResolveResult Result =
		FMatchResultResolver::ResolveMatchResult(State);

	TestTrue(TEXT("Ended tied match resolves successfully"), Result.bSuccess);
	TestEqual(TEXT("Equal scores produce a draw"), Result.ResultType, EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultNegativeScoreTest,
	"FMCodex.CoreRules.MatchResultResolver.NegativeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultNegativeScoreTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState NegativeHomeState =
		MatchResultResolverTests::MakeRuntimeState(-1, 0);
	const FMatchResultResolveResult NegativeHomeResult =
		FMatchResultResolver::ResolveMatchResult(NegativeHomeState);
	TestFalse(TEXT("Negative home score is rejected"), NegativeHomeResult.bSuccess);
	TestEqual(
		TEXT("Negative home score reports invalid runtime"),
		NegativeHomeResult.ErrorCode,
		EMatchResultResolveErrorCode::InvalidRuntimeState);
	TestEqual(
		TEXT("Negative score produces no result"),
		NegativeHomeResult.ResultType,
		EMatchResultType::NotEnded);

	const FMatchRuntimeState NegativeAwayState =
		MatchResultResolverTests::MakeRuntimeState(0, -1);
	const FMatchResultResolveResult NegativeAwayResult =
		FMatchResultResolver::ResolveMatchResult(NegativeAwayState);
	TestFalse(TEXT("Negative away score is rejected"), NegativeAwayResult.bSuccess);
	TestEqual(
		TEXT("Negative away score reports invalid runtime"),
		NegativeAwayResult.ErrorCode,
		EMatchResultResolveErrorCode::InvalidRuntimeState);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultInvalidRuntimeStateTest,
	"FMCodex.CoreRules.MatchResultResolver.InvalidRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultInvalidRuntimeStateTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState UninitializedState;
	const FMatchResultResolveResult UninitializedResult =
		FMatchResultResolver::ResolveMatchResult(UninitializedState);
	TestFalse(TEXT("Uninitialized runtime is rejected"), UninitializedResult.bSuccess);
	TestEqual(
		TEXT("Uninitialized runtime reports invalid state"),
		UninitializedResult.ErrorCode,
		EMatchResultResolveErrorCode::InvalidRuntimeState);
	TestTrue(
		TEXT("Match end validation error remains readable"),
		UninitializedResult.ErrorMessage.Contains(TEXT("Match end validation failed")));

	const FMatchRuntimeState InvalidCountsState =
		MatchResultResolverTests::MakeRuntimeState(1, 0, 5, 6, 6, 6);
	const FMatchResultResolveResult InvalidCountsResult =
		FMatchResultResolver::ResolveMatchResult(InvalidCountsState);
	TestFalse(TEXT("Used attacks above total are rejected"), InvalidCountsResult.bSuccess);
	TestEqual(
		TEXT("Invalid attack counts report invalid state"),
		InvalidCountsResult.ErrorCode,
		EMatchResultResolveErrorCode::InvalidRuntimeState);
	TestEqual(
		TEXT("Invalid runtime produces no result"),
		InvalidCountsResult.ResultType,
		EMatchResultType::NotEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultReadOnlyRepeatTest,
	"FMCodex.CoreRules.MatchResultResolver.ReadOnlyAndRepeatable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultReadOnlyRepeatTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchResultResolverTests::MakeRuntimeState(4, 2);
	const FMatchResultResolveResult FirstResult =
		FMatchResultResolver::ResolveMatchResult(State);
	const FMatchResultResolveResult SecondResult =
		FMatchResultResolver::ResolveMatchResult(State);

	TestEqual(TEXT("Input home score is unchanged"), State.PlayerAState.Score, 4);
	TestEqual(TEXT("Input away score is unchanged"), State.PlayerBState.Score, 2);
	TestEqual(TEXT("Input PlayerA used count is unchanged"), State.PlayerAState.UsedAttackCount, 5);
	TestEqual(TEXT("Input PlayerB used count is unchanged"), State.PlayerBState.UsedAttackCount, 6);
	TestEqual(TEXT("Repeated calls have the same success"), FirstResult.bSuccess, SecondResult.bSuccess);
	TestEqual(TEXT("Repeated calls have the same end state"), FirstResult.bIsMatchEnded, SecondResult.bIsMatchEnded);
	TestEqual(TEXT("Repeated calls have the same result"), FirstResult.ResultType, SecondResult.ResultType);
	TestEqual(TEXT("Repeated calls have the same home score"), FirstResult.HomeScore, SecondResult.HomeScore);
	TestEqual(TEXT("Repeated calls have the same away score"), FirstResult.AwayScore, SecondResult.AwayScore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchResultIgnoresRandomInputsTest,
	"FMCodex.CoreRules.MatchResultResolver.IgnoresRandomInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchResultIgnoresRandomInputsTest::RunTest(const FString& Parameters)
{
	FMatchRuntimeState FirstState =
		MatchResultResolverTests::MakeRuntimeState(1, 3);
	FirstState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
	FirstState.OpeningResultSnapshot.bSuccess = true;
	FirstState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 99;
	FirstState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerA;

	FMatchRuntimeState SecondState = FirstState;
	SecondState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::None;
	SecondState.OpeningResultSnapshot.bSuccess = false;
	SecondState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 1;
	SecondState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchResultResolveResult FirstResult =
		FMatchResultResolver::ResolveMatchResult(FirstState);
	const FMatchResultResolveResult SecondResult =
		FMatchResultResolver::ResolveMatchResult(SecondState);

	TestTrue(TEXT("First unrelated-state variant succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second unrelated-state variant succeeds"), SecondResult.bSuccess);
	TestEqual(
		TEXT("Opening and turn-order state do not affect result"),
		FirstResult.ResultType,
		SecondResult.ResultType);
	TestEqual(
		TEXT("Unrelated fields do not affect end state"),
		FirstResult.bIsMatchEnded,
		SecondResult.bIsMatchEnded);
	return true;
}

#endif
