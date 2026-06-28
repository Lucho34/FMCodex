#include "AttackCardPlayFlow.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace AttackCardPlayFlowTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardA3(TEXT("CardA3"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardB2(TEXT("CardB2"));
	const FName CardB3(TEXT("CardB3"));

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

	FMatchCardUsageState MakeCardUsageState()
	{
		FMatchCardUsageState State;
		State.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2, CardA3 };
		State.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1, CardB2, CardB3 };
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerANoGoalTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerANoGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerANoGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);

	TestTrue(TEXT("PlayerA attack card play succeeds"), Result.bSuccess);
	TestEqual(TEXT("Acting player is PlayerA"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("No goal is reported"), Result.bGoalScored);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerA card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(AttackCardPlayFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerAGoalTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerAGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerAGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA2,
			true);

	TestTrue(TEXT("PlayerA scoring attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Goal is reported"), Result.bGoalScored);
	TestEqual(TEXT("PlayerA score increases"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestTrue(
		TEXT("Played card is used"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(AttackCardPlayFlowTests::CardA2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerBNoGoalTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerBNoGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerBNoGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardB1,
			false);

	TestTrue(TEXT("PlayerB attack card play succeeds"), Result.bSuccess);
	TestEqual(TEXT("Acting player is PlayerB"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 0);
	TestEqual(TEXT("PlayerB attack is consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerB card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(AttackCardPlayFlowTests::CardB1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerBGoalTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerBGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerBGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardB2,
			true);

	TestTrue(TEXT("PlayerB scoring attack succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("PlayerB score increases"), Result.UpdatedRuntimeState.PlayerBState.Score, 2);
	TestEqual(TEXT("PlayerB attack is consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestTrue(
		TEXT("Played card is used"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(AttackCardPlayFlowTests::CardB2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerACannotUsePlayerBCardTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerACannotUsePlayerBCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerACannotUsePlayerBCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardB1,
			false);

	TestFalse(TEXT("PlayerA cannot use PlayerB-only card"), Result.bSuccess);
	TestEqual(
		TEXT("Cross-player card reports play failure"),
		Result.ErrorCode,
		EAttackCardPlayFlowErrorCode::PlayCardResolveFailed);
	TestEqual(TEXT("PlayerA attack is not consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPlayerBCannotUsePlayerACardTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayerBCannotUsePlayerACard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPlayerBCannotUsePlayerACardTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);

	TestFalse(TEXT("PlayerB cannot use PlayerA-only card"), Result.bSuccess);
	TestEqual(
		TEXT("Cross-player card reports play failure"),
		Result.ErrorCode,
		EAttackCardPlayFlowErrorCode::PlayCardResolveFailed);
	TestEqual(TEXT("PlayerB attack is not consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayInputsUnchangedTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.InputsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayInputsUnchangedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			true);

	TestTrue(TEXT("Attack card play succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input PlayerA score is unchanged"), RuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Input PlayerA used attack count is unchanged"), RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Input card remains available"),
		CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			AttackCardPlayFlowTests::CardA1));
	TestTrue(
		TEXT("Input used cards remain empty"),
		CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayFailureAtomicTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PlayCardFailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayFailureAtomicTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			FName(TEXT("UnknownCard")),
			true);

	TestFalse(TEXT("Unavailable card fails the flow"), Result.bSuccess);
	TestEqual(
		TEXT("Play failure is reported"),
		Result.ErrorCode,
		EAttackCardPlayFlowErrorCode::PlayCardResolveFailed);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("Attack opportunity is not consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Card usage state is unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== CardUsageState.PlayerACardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayAttackFailureAtomicTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.AttackResolutionFailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayAttackFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			1,
			2,
			0);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);

	TestFalse(TEXT("Attack without current opportunity fails"), Result.bSuccess);
	TestTrue(TEXT("Card play substep succeeded"), Result.PlayCardResult.bSuccess);
	TestFalse(TEXT("Attack substep failed"), Result.AttackResolutionResult.bSuccess);
	TestEqual(
		TEXT("Attack failure is reported"),
		Result.ErrorCode,
		EAttackCardPlayFlowErrorCode::AttackResolutionFlowFailed);
	TestEqual(TEXT("Attack count remains unchanged"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestTrue(
		TEXT("Top-level card state rolls back"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(AttackCardPlayFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayAlreadyEndedTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.MatchAlreadyEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayAlreadyEndedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None,
			2,
			1,
			1,
			1,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);

	TestFalse(TEXT("Ended match rejects card play"), Result.bSuccess);
	TestTrue(TEXT("Ended state is reported"), Result.bMatchEnded);
	TestEqual(
		TEXT("Ended match reports its error"),
		Result.ErrorCode,
		EAttackCardPlayFlowErrorCode::MatchAlreadyEnded);
	TestTrue(
		TEXT("Card remains available"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(AttackCardPlayFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayLastAttackTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.LastAttackEndsMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayLastAttackTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			0,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			true);

	TestTrue(TEXT("Last attack card play succeeds"), Result.bSuccess);
	TestTrue(TEXT("Last attack ends match"), Result.bMatchEnded);
	TestEqual(TEXT("Last goal is recorded"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Last opportunity is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestEqual(
		TEXT("Final result is HomeWin"),
		Result.MatchResultType,
		EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayFinalResultsTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.FinalMatchResults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayFinalResultsTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();

	const FMatchRuntimeState HomeWinRuntime =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1,
			1,
			0,
			1,
			1);
	const FAttackCardPlayFlowResult HomeWinResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			HomeWinRuntime,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);
	TestTrue(TEXT("Home-win flow succeeds"), HomeWinResult.bSuccess);
	TestEqual(TEXT("HomeWin is returned"), HomeWinResult.MatchResultType, EMatchResultType::HomeWin);

	const FMatchRuntimeState AwayWinRuntime =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			2,
			1,
			0,
			1,
			1);
	const FAttackCardPlayFlowResult AwayWinResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			AwayWinRuntime,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);
	TestTrue(TEXT("Away-win flow succeeds"), AwayWinResult.bSuccess);
	TestEqual(TEXT("AwayWin is returned"), AwayWinResult.MatchResultType, EMatchResultType::AwayWin);

	const FMatchRuntimeState DrawRuntime =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			0,
			1,
			1);
	const FAttackCardPlayFlowResult DrawResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			DrawRuntime,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);
	TestTrue(TEXT("Draw flow succeeds"), DrawResult.bSuccess);
	TestEqual(TEXT("Draw is returned"), DrawResult.MatchResultType, EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayPreservesStructuralStateTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.PreservesStructuralState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayPreservesStructuralStateTest::RunTest(
	const FString& Parameters)
{
	FMatchRuntimeState RuntimeState =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	RuntimeState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerA;
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();

	const FAttackCardPlayFlowResult Result =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			CardUsageState,
			AttackCardPlayFlowTests::CardB1,
			true);

	TestTrue(TEXT("Attack card play succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA total attacks are unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.TotalAttackCount,
		RuntimeState.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerB total attacks are unchanged"),
		Result.UpdatedRuntimeState.PlayerBState.TotalAttackCount,
		RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("Opening PlayerA attacks are unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount,
		RuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount);
	TestEqual(
		TEXT("Opening PlayerB attacks are unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount,
		RuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount);
	TestEqual(
		TEXT("Opening first player is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer,
		RuntimeState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer);
	TestTrue(
		TEXT("PlayerA card usage state is unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState.AvailableCardIds
			== CardUsageState.PlayerACardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackCardPlayInvalidInputsTest,
	"FMCodex.CoreRules.AttackCardPlayFlow.InvalidInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackCardPlayInvalidInputsTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState CardUsageState =
		AttackCardPlayFlowTests::MakeCardUsageState();
	const FMatchRuntimeState UninitializedRuntime;
	const FAttackCardPlayFlowResult UninitializedResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			UninitializedRuntime,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);
	TestEqual(
		TEXT("Uninitialized runtime is invalid"),
		UninitializedResult.ErrorCode,
		EAttackCardPlayFlowErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState InvalidPlayerRuntime =
		AttackCardPlayFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None);
	const FAttackCardPlayFlowResult InvalidPlayerResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			InvalidPlayerRuntime,
			CardUsageState,
			AttackCardPlayFlowTests::CardA1,
			false);
	TestEqual(
		TEXT("Invalid current player is reported"),
		InvalidPlayerResult.ErrorCode,
		EAttackCardPlayFlowErrorCode::InvalidCurrentAttackingPlayer);

	const FMatchRuntimeState ValidRuntime =
		AttackCardPlayFlowTests::MakeRuntimeState();
	const FAttackCardPlayFlowResult InvalidCardResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			ValidRuntime,
			CardUsageState,
			NAME_None,
			false);
	TestEqual(
		TEXT("None CardId is invalid"),
		InvalidCardResult.ErrorCode,
		EAttackCardPlayFlowErrorCode::InvalidCardId);

	FMatchCardUsageState InvalidCardUsageState = CardUsageState;
	InvalidCardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		AttackCardPlayFlowTests::CardA1);
	const FAttackCardPlayFlowResult InvalidUsageResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			ValidRuntime,
			InvalidCardUsageState,
			AttackCardPlayFlowTests::CardA2,
			false);
	TestEqual(
		TEXT("Invalid selected card state is reported"),
		InvalidUsageResult.ErrorCode,
		EAttackCardPlayFlowErrorCode::InvalidMatchCardUsageState);
	return true;
}

#endif
