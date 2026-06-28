#include "AttackOpportunityResolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace AttackOpportunityResolverTests
{
	FMatchRuntimeState MakeRuntimeState(
		const EInitialTurnOrderPlayer CurrentPlayer,
		const int32 PlayerATotal = 2,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 2,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = PlayerATotal;
		State.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.PlayerAState.Score = 3;
		State.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.PlayerBState.Score = 4;
		State.CurrentAttackingPlayer = CurrentPlayer;
		State.OpeningResultSnapshot.bSuccess = true;
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = PlayerATotal;
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount = PlayerBTotal;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityUninitializedStateTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.UninitializedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityUninitializedStateTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State;
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestFalse(TEXT("Uninitialized state is rejected"), Result.bSuccess);
	TestTrue(
		TEXT("Uninitialized state reports structured error"),
		Result.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::RuntimeStateNotInitialized));
	TestEqual(
		TEXT("No acting player is reported"),
		Result.ActingPlayer,
		EInitialTurnOrderPlayer::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityPlayerAConsumeTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.PlayerAConsumeAndSwitch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityPlayerAConsumeTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA);
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestTrue(TEXT("PlayerA consumes an attack"), Result.bSuccess);
	TestEqual(TEXT("PlayerA used count increases"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestEqual(TEXT("PlayerB used count is unchanged"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 0);
	TestEqual(TEXT("Acting player is PlayerA"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Next player is PlayerB"), Result.NextAttackingPlayer, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Updated current attacker is PlayerB"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerA total is unchanged"), Result.UpdatedRuntimeState.PlayerAState.TotalAttackCount, 2);
	TestEqual(TEXT("PlayerB total is unchanged"), Result.UpdatedRuntimeState.PlayerBState.TotalAttackCount, 2);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 3);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 4);
	TestTrue(
		TEXT("Opening snapshot is unchanged"),
		Result.UpdatedRuntimeState.OpeningResultSnapshot.bSuccess);
	TestEqual(TEXT("Input state remains unchanged"), State.PlayerAState.UsedAttackCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityPlayerBConsumeTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.PlayerBConsumeAndSwitch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityPlayerBConsumeTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestTrue(TEXT("PlayerB consumes an attack"), Result.bSuccess);
	TestEqual(TEXT("PlayerA used count is unchanged"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 0);
	TestEqual(TEXT("PlayerB used count increases"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestEqual(TEXT("Acting player is PlayerB"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Next player is PlayerA"), Result.NextAttackingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Updated current attacker is PlayerA"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityOpponentExhaustedTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.OpponentExhaustedKeepsCurrent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityOpponentExhaustedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			3,
			1,
			1,
			1);
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestTrue(TEXT("Available current-player attack succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerA used count increases"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 2);
	TestEqual(TEXT("Acting player is PlayerA"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA remains next"), Result.NextAttackingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Updated current attacker remains PlayerA"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestTrue(TEXT("The match still has an attack"), Result.bMatchHasRemainingAttack);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityLastAttackTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.LastAttackEndsAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityLastAttackTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0,
			1,
			1);
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestTrue(TEXT("Last attack is consumed successfully"), Result.bSuccess);
	TestFalse(TEXT("No attacks remain"), Result.bMatchHasRemainingAttack);
	TestEqual(TEXT("Acting player is PlayerA"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("There is no next attacker"), Result.NextAttackingPlayer, EInitialTurnOrderPlayer::None);
	TestEqual(
		TEXT("Runtime current attacker becomes None"),
		Result.UpdatedRuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityInvalidCurrentPlayerTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.InvalidCurrentPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityInvalidCurrentPlayerTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState State =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None);
	const FAttackOpportunityResolveResult Result =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(State);

	TestFalse(TEXT("None current player is rejected"), Result.bSuccess);
	TestTrue(
		TEXT("Invalid current player reports structured error"),
		Result.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::InvalidCurrentAttackingPlayer));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityInvalidCountsTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.InvalidAttackCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityInvalidCountsTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState UsedAboveTotalState =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			2);
	const FAttackOpportunityResolveResult UsedAboveTotalResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			UsedAboveTotalState);
	TestFalse(TEXT("Used count above total is rejected"), UsedAboveTotalResult.bSuccess);
	TestTrue(
		TEXT("Used count above total identifies PlayerA"),
		UsedAboveTotalResult.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::InvalidPlayerAAttackCountState));

	const FMatchRuntimeState NegativeUsedState =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB,
			2,
			0,
			2,
			-1);
	const FAttackOpportunityResolveResult NegativeUsedResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			NegativeUsedState);
	TestFalse(TEXT("Negative used count is rejected"), NegativeUsedResult.bSuccess);
	TestTrue(
		TEXT("Negative used count identifies PlayerB"),
		NegativeUsedResult.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::InvalidPlayerBAttackCountState));

	const FMatchRuntimeState NegativeTotalState =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			-1,
			0);
	const FAttackOpportunityResolveResult NegativeTotalResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			NegativeTotalState);
	TestFalse(TEXT("Negative total count is rejected"), NegativeTotalResult.bSuccess);
	TestTrue(
		TEXT("Negative total count identifies PlayerA"),
		NegativeTotalResult.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::InvalidPlayerAAttackCountState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAttackOpportunityNoCurrentOrMatchAttackTest,
	"FMCodex.CoreRules.AttackOpportunityResolver.NoCurrentOrMatchAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAttackOpportunityNoCurrentOrMatchAttackTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState CurrentExhaustedState =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			2,
			0);
	const FAttackOpportunityResolveResult CurrentExhaustedResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			CurrentExhaustedState);
	TestFalse(TEXT("Exhausted current player is rejected"), CurrentExhaustedResult.bSuccess);
	TestTrue(TEXT("Opponent still has an attack"), CurrentExhaustedResult.bMatchHasRemainingAttack);
	TestTrue(
		TEXT("Exhausted current player reports structured error"),
		CurrentExhaustedResult.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::CurrentPlayerHasNoRemainingAttack));
	TestEqual(
		TEXT("Failed result does not change used count"),
		CurrentExhaustedResult.UpdatedRuntimeState.PlayerAState.UsedAttackCount,
		1);

	const FMatchRuntimeState BothExhaustedState =
		AttackOpportunityResolverTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			1);
	const FAttackOpportunityResolveResult BothExhaustedResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			BothExhaustedState);
	TestFalse(TEXT("Both exhausted players are rejected"), BothExhaustedResult.bSuccess);
	TestFalse(TEXT("No match attack remains"), BothExhaustedResult.bMatchHasRemainingAttack);
	TestTrue(
		TEXT("Both exhausted players report structured error"),
		BothExhaustedResult.ErrorCodes.Contains(
			EAttackOpportunityResolveErrorCode::NoRemainingAttackForBothPlayers));
	return true;
}

#endif
