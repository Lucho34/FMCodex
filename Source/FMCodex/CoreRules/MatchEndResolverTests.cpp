#include "MatchEndResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchEndResolverTests
{
	FMatchRuntimeState MakeRuntimeState(
		const int32 PlayerATotal = 5,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 6,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = PlayerATotal;
		State.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndNewMatchTest,
	"FMCodex.CoreRules.MatchEndResolver.NewMatchHasRemainingAttacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndNewMatchTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchEndResolverTests::MakeRuntimeState();
	const FMatchEndResolveResult Result =
		FMatchEndResolver::ResolveMatchEnd(State);

	TestTrue(TEXT("Initialized runtime state resolves successfully"), Result.bSuccess);
	TestFalse(TEXT("A new match has not ended"), Result.bIsMatchEnded);
	TestEqual(TEXT("PlayerA total is reported"), Result.PlayerATotalAttackCount, 5);
	TestEqual(TEXT("PlayerA used is reported"), Result.PlayerAUsedAttackCount, 0);
	TestEqual(TEXT("PlayerA remaining is reported"), Result.PlayerARemainingAttackCount, 5);
	TestEqual(TEXT("PlayerB total is reported"), Result.PlayerBTotalAttackCount, 6);
	TestEqual(TEXT("PlayerB used is reported"), Result.PlayerBUsedAttackCount, 0);
	TestEqual(TEXT("PlayerB remaining is reported"), Result.PlayerBRemainingAttackCount, 6);
	TestEqual(TEXT("Successful result has no error"), Result.ErrorCode, EMatchEndResolveErrorCode::None);
	TestTrue(TEXT("Successful result has no error message"), Result.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndOnePlayerExhaustedTest,
	"FMCodex.CoreRules.MatchEndResolver.OnePlayerExhausted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndOnePlayerExhaustedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchEndResolverTests::MakeRuntimeState(5, 5, 6, 4);
	const FMatchEndResolveResult Result =
		FMatchEndResolver::ResolveMatchEnd(State);

	TestTrue(TEXT("Valid mixed availability resolves successfully"), Result.bSuccess);
	TestFalse(TEXT("The match continues while PlayerB has attacks"), Result.bIsMatchEnded);
	TestEqual(TEXT("PlayerA has no remaining attacks"), Result.PlayerARemainingAttackCount, 0);
	TestEqual(TEXT("PlayerB still has attacks"), Result.PlayerBRemainingAttackCount, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndBothPlayersExhaustedTest,
	"FMCodex.CoreRules.MatchEndResolver.BothPlayersExhausted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndBothPlayersExhaustedTest::RunTest(const FString& Parameters)
{
	FMatchRuntimeState State =
		MatchEndResolverTests::MakeRuntimeState(5, 5, 6, 6);
	State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::None;

	const FMatchEndResolveResult Result =
		FMatchEndResolver::ResolveMatchEnd(State);

	TestTrue(TEXT("Exhausted valid state resolves successfully"), Result.bSuccess);
	TestTrue(TEXT("The match ends when both players are exhausted"), Result.bIsMatchEnded);
	TestEqual(TEXT("PlayerA has no remaining attacks"), Result.PlayerARemainingAttackCount, 0);
	TestEqual(TEXT("PlayerB has no remaining attacks"), Result.PlayerBRemainingAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndReadOnlyRepeatTest,
	"FMCodex.CoreRules.MatchEndResolver.ReadOnlyAndRepeatable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndReadOnlyRepeatTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		MatchEndResolverTests::MakeRuntimeState(5, 2, 6, 3);
	const FMatchEndResolveResult FirstResult =
		FMatchEndResolver::ResolveMatchEnd(State);
	const FMatchEndResolveResult SecondResult =
		FMatchEndResolver::ResolveMatchEnd(State);

	TestEqual(TEXT("Input PlayerA total remains unchanged"), State.PlayerAState.TotalAttackCount, 5);
	TestEqual(TEXT("Input PlayerA used remains unchanged"), State.PlayerAState.UsedAttackCount, 2);
	TestEqual(TEXT("Input PlayerB total remains unchanged"), State.PlayerBState.TotalAttackCount, 6);
	TestEqual(TEXT("Input PlayerB used remains unchanged"), State.PlayerBState.UsedAttackCount, 3);
	TestEqual(TEXT("Repeated calls have the same success"), FirstResult.bSuccess, SecondResult.bSuccess);
	TestEqual(TEXT("Repeated calls have the same end state"), FirstResult.bIsMatchEnded, SecondResult.bIsMatchEnded);
	TestEqual(
		TEXT("Repeated calls have the same PlayerA remaining count"),
		FirstResult.PlayerARemainingAttackCount,
		SecondResult.PlayerARemainingAttackCount);
	TestEqual(
		TEXT("Repeated calls have the same PlayerB remaining count"),
		FirstResult.PlayerBRemainingAttackCount,
		SecondResult.PlayerBRemainingAttackCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndInvalidRuntimeStateTest,
	"FMCodex.CoreRules.MatchEndResolver.InvalidRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndInvalidRuntimeStateTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState UninitializedState;
	const FMatchEndResolveResult UninitializedResult =
		FMatchEndResolver::ResolveMatchEnd(UninitializedState);
	TestFalse(TEXT("Uninitialized state is rejected"), UninitializedResult.bSuccess);
	TestEqual(
		TEXT("Uninitialized state reports its error"),
		UninitializedResult.ErrorCode,
		EMatchEndResolveErrorCode::RuntimeStateNotInitialized);
	TestTrue(TEXT("Uninitialized state has a readable error"), !UninitializedResult.ErrorMessage.IsEmpty());

	const FMatchRuntimeState InvalidPlayerAState =
		MatchEndResolverTests::MakeRuntimeState(2, 3, 6, 0);
	const FMatchEndResolveResult InvalidPlayerAResult =
		FMatchEndResolver::ResolveMatchEnd(InvalidPlayerAState);
	TestFalse(TEXT("PlayerA used above total is rejected"), InvalidPlayerAResult.bSuccess);
	TestEqual(
		TEXT("Invalid PlayerA state reports its error"),
		InvalidPlayerAResult.ErrorCode,
		EMatchEndResolveErrorCode::InvalidPlayerAAttackCountState);

	const FMatchRuntimeState InvalidPlayerBState =
		MatchEndResolverTests::MakeRuntimeState(5, 0, -1, 0);
	const FMatchEndResolveResult InvalidPlayerBResult =
		FMatchEndResolver::ResolveMatchEnd(InvalidPlayerBState);
	TestFalse(TEXT("Negative PlayerB total is rejected"), InvalidPlayerBResult.bSuccess);
	TestEqual(
		TEXT("Invalid PlayerB state reports its error"),
		InvalidPlayerBResult.ErrorCode,
		EMatchEndResolveErrorCode::InvalidPlayerBAttackCountState);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchEndIgnoresUnrelatedStateTest,
	"FMCodex.CoreRules.MatchEndResolver.IgnoresUnrelatedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchEndIgnoresUnrelatedStateTest::RunTest(const FString& Parameters)
{
	FMatchRuntimeState FirstState =
		MatchEndResolverTests::MakeRuntimeState(5, 5, 6, 4);
	FirstState.PlayerAState.Score = 99;
	FirstState.PlayerBState.Score = 0;
	FirstState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
	FirstState.OpeningResultSnapshot.bSuccess = true;

	FMatchRuntimeState SecondState = FirstState;
	SecondState.PlayerAState.Score = 0;
	SecondState.PlayerBState.Score = 99;
	SecondState.CurrentAttackingPlayer = EInitialTurnOrderPlayer::None;
	SecondState.OpeningResultSnapshot.bSuccess = false;

	const FMatchEndResolveResult FirstResult =
		FMatchEndResolver::ResolveMatchEnd(FirstState);
	const FMatchEndResolveResult SecondResult =
		FMatchEndResolver::ResolveMatchEnd(SecondState);

	TestTrue(TEXT("First unrelated-state variant succeeds"), FirstResult.bSuccess);
	TestTrue(TEXT("Second unrelated-state variant succeeds"), SecondResult.bSuccess);
	TestEqual(
		TEXT("Scores, current attacker, and opening snapshot do not affect match end"),
		FirstResult.bIsMatchEnded,
		SecondResult.bIsMatchEnded);
	TestEqual(
		TEXT("Unrelated fields do not affect PlayerA remaining count"),
		FirstResult.PlayerARemainingAttackCount,
		SecondResult.PlayerARemainingAttackCount);
	TestEqual(
		TEXT("Unrelated fields do not affect PlayerB remaining count"),
		FirstResult.PlayerBRemainingAttackCount,
		SecondResult.PlayerBRemainingAttackCount);
	return true;
}

#endif
