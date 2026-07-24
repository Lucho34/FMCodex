#include "MatchPlayGoalkeeperUsageState.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayGoalkeeperUsageStateTests
{
	bool AreUsageStatesEqual(
		const FMatchPlayGoalkeeperUsageState& Left,
		const FMatchPlayGoalkeeperUsageState& Right)
	{
		return FMatchPlayGoalkeeperUsageState::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}
}

#define MATCH_PLAY_GOALKEEPER_USAGE_TEST(ClassName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
		ClassName, \
		"FMCodex.CoreRules.MatchPlayGoalkeeperUsageState." TestName, \
		EAutomationTestFlags::EditorContext \
			| EAutomationTestFlags::EngineFilter)

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageDefaultTest,
	"DefaultStateIsUnusedForBothSides")

bool FMatchPlayGoalkeeperUsageDefaultTest::RunTest(const FString& Parameters)
{
	const FMatchPlayGoalkeeperUsageState State;
	TestFalse(
		TEXT("PlayerA goalkeeper is unused by default"),
		State.bPlayerAGoalkeeperCardUsed);
	TestFalse(
		TEXT("PlayerB goalkeeper is unused by default"),
		State.bPlayerBGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageQueryPlayerATest,
	"QueryReturnsPlayerAValue")

bool FMatchPlayGoalkeeperUsageQueryPlayerATest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState State;
	State.bPlayerAGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageQueryResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::Query(
			State,
			EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("Query succeeds"), Result.bSucceeded);
	TestTrue(TEXT("PlayerA value is returned"), Result.bGoalkeeperCardUsed);
	TestEqual(
		TEXT("Query records PlayerA"),
		Result.PlayerSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Query has no error"),
		Result.ErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::None);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageQueryPlayerBTest,
	"QueryReturnsPlayerBValue")

bool FMatchPlayGoalkeeperUsageQueryPlayerBTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState State;
	State.bPlayerBGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageQueryResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::Query(
			State,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("Query succeeds"), Result.bSucceeded);
	TestTrue(TEXT("PlayerB value is returned"), Result.bGoalkeeperCardUsed);
	TestEqual(
		TEXT("Query records PlayerB"),
		Result.PlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageQueryIsolationTest,
	"QueryDoesNotCrossPlayerSides")

bool FMatchPlayGoalkeeperUsageQueryIsolationTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState State;
	State.bPlayerAGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageQueryResult PlayerBResult =
		FMatchPlayGoalkeeperUsageStateResolver::Query(
			State,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("PlayerB query succeeds"), PlayerBResult.bSucceeded);
	TestFalse(
		TEXT("PlayerA value does not leak into PlayerB"),
		PlayerBResult.bGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageInvalidQueryTest,
	"QueryRejectsInvalidPlayerSide")

bool FMatchPlayGoalkeeperUsageInvalidQueryTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayGoalkeeperUsageState State;
	const FMatchPlayGoalkeeperUsageQueryResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::Query(
			State,
			EInitialTurnOrderPlayer::None);

	TestFalse(TEXT("Query fails"), Result.bSucceeded);
	TestFalse(
		TEXT("Failed query returns the default used value"),
		Result.bGoalkeeperCardUsed);
	TestEqual(
		TEXT("Invalid side is precise"),
		Result.ErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::InvalidPlayerSide);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageMarkPlayerATest,
	"MarkUsedUpdatesOnlyPlayerA")

bool FMatchPlayGoalkeeperUsageMarkPlayerATest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayGoalkeeperUsageState BeforeState;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("MarkUsed succeeds"), Result.bSucceeded);
	TestTrue(
		TEXT("PlayerA is marked used"),
		Result.UpdatedUsageState.bPlayerAGoalkeeperCardUsed);
	TestFalse(
		TEXT("PlayerB remains unused"),
		Result.UpdatedUsageState.bPlayerBGoalkeeperCardUsed);
	TestFalse(
		TEXT("Input PlayerA remains unchanged"),
		BeforeState.bPlayerAGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageMarkPlayerBTest,
	"MarkUsedUpdatesOnlyPlayerB")

bool FMatchPlayGoalkeeperUsageMarkPlayerBTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayGoalkeeperUsageState BeforeState;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("MarkUsed succeeds"), Result.bSucceeded);
	TestFalse(
		TEXT("PlayerA remains unused"),
		Result.UpdatedUsageState.bPlayerAGoalkeeperCardUsed);
	TestTrue(
		TEXT("PlayerB is marked used"),
		Result.UpdatedUsageState.bPlayerBGoalkeeperCardUsed);
	TestFalse(
		TEXT("Input PlayerB remains unchanged"),
		BeforeState.bPlayerBGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageMarkPlayerAPreservesPlayerBTest,
	"MarkPlayerAPreservesExistingPlayerBValue")

bool FMatchPlayGoalkeeperUsageMarkPlayerAPreservesPlayerBTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState BeforeState;
	BeforeState.bPlayerBGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::PlayerA);

	TestTrue(TEXT("MarkUsed succeeds"), Result.bSucceeded);
	TestTrue(
		TEXT("Existing PlayerB value is preserved"),
		Result.UpdatedUsageState.bPlayerBGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageMarkPlayerBPreservesPlayerATest,
	"MarkPlayerBPreservesExistingPlayerAValue")

bool FMatchPlayGoalkeeperUsageMarkPlayerBPreservesPlayerATest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState BeforeState;
	BeforeState.bPlayerAGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("MarkUsed succeeds"), Result.bSucceeded);
	TestTrue(
		TEXT("Existing PlayerA value is preserved"),
		Result.UpdatedUsageState.bPlayerAGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageAlreadyUsedTest,
	"MarkUsedRejectsAlreadyUsedSideAtomically")

bool FMatchPlayGoalkeeperUsageAlreadyUsedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState BeforeState;
	BeforeState.bPlayerAGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageState OriginalInput = BeforeState;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::PlayerA);

	TestFalse(TEXT("MarkUsed fails"), Result.bSucceeded);
	TestEqual(
		TEXT("Already used is precise"),
		Result.ErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::GoalkeeperAlreadyUsed);
	TestTrue(
		TEXT("Failure result preserves BeforeState"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(
			Result.UpdatedUsageState,
			OriginalInput));
	TestTrue(
		TEXT("Input remains unchanged"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(
			BeforeState,
			OriginalInput));
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageInvalidMarkTest,
	"MarkUsedRejectsInvalidSideAtomically")

bool FMatchPlayGoalkeeperUsageInvalidMarkTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState BeforeState;
	BeforeState.bPlayerBGoalkeeperCardUsed = true;
	const FMatchPlayGoalkeeperUsageState OriginalInput = BeforeState;
	const FMatchPlayGoalkeeperUsageUpdateResult Result =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState,
			EInitialTurnOrderPlayer::None);

	TestFalse(TEXT("MarkUsed fails"), Result.bSucceeded);
	TestEqual(
		TEXT("Invalid side is precise"),
		Result.ErrorCode,
		EMatchPlayGoalkeeperUsageErrorCode::InvalidPlayerSide);
	TestTrue(
		TEXT("Failure result preserves BeforeState"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(
			Result.UpdatedUsageState,
			OriginalInput));
	TestTrue(
		TEXT("Input remains unchanged"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(
			BeforeState,
			OriginalInput));
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageBothSidesTest,
	"BothPlayerSidesCanBeMarkedIndependently")

bool FMatchPlayGoalkeeperUsageBothSidesTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayGoalkeeperUsageState InitialState;
	const FMatchPlayGoalkeeperUsageUpdateResult PlayerAResult =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			InitialState,
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchPlayGoalkeeperUsageUpdateResult PlayerBResult =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			PlayerAResult.UpdatedUsageState,
			EInitialTurnOrderPlayer::PlayerB);

	TestTrue(TEXT("PlayerA update succeeds"), PlayerAResult.bSucceeded);
	TestTrue(TEXT("PlayerB update succeeds"), PlayerBResult.bSucceeded);
	TestTrue(
		TEXT("PlayerA remains marked"),
		PlayerBResult.UpdatedUsageState.bPlayerAGoalkeeperCardUsed);
	TestTrue(
		TEXT("PlayerB is marked"),
		PlayerBResult.UpdatedUsageState.bPlayerBGoalkeeperCardUsed);
	return true;
}

MATCH_PLAY_GOALKEEPER_USAGE_TEST(
	FMatchPlayGoalkeeperUsageReflectedCopyTest,
	"ReflectedCopyPreservesAndDetectsValues")

bool FMatchPlayGoalkeeperUsageReflectedCopyTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayGoalkeeperUsageState State;
	State.bPlayerAGoalkeeperCardUsed = true;
	FMatchPlayGoalkeeperUsageState Copy = State;

	TestTrue(
		TEXT("Reflected copy matches"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(State, Copy));
	Copy.bPlayerBGoalkeeperCardUsed = true;
	TestFalse(
		TEXT("Reflected comparison detects PlayerB change"),
		MatchPlayGoalkeeperUsageStateTests::AreUsageStatesEqual(State, Copy));
	return true;
}

#undef MATCH_PLAY_GOALKEEPER_USAGE_TEST

#endif
