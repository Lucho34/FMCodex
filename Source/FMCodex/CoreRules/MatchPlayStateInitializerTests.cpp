#include "MatchPlayStateInitializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayStateInitializerTests
{
	const FName PlayerACard1(TEXT("PlayerACard1"));
	const FName PlayerACard2(TEXT("PlayerACard2"));
	const FName PlayerBCard1(TEXT("PlayerBCard1"));
	const FName PlayerBCard2(TEXT("PlayerBCard2"));

	FMatchRuntimeState MakeRuntimeState()
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = 4;
		State.PlayerAState.UsedAttackCount = 1;
		State.PlayerAState.Score = 2;
		State.PlayerAState.GoalkeeperCardId = TEXT("PlayerAGoalkeeper");
		State.PlayerAState.InitialDeckRarityScore = 11;
		State.PlayerBState.TotalAttackCount = 3;
		State.PlayerBState.UsedAttackCount = 2;
		State.PlayerBState.Score = 1;
		State.PlayerBState.GoalkeeperCardId = TEXT("PlayerBGoalkeeper");
		State.PlayerBState.InitialDeckRarityScore = 9;
		State.FirstPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.SecondPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.OpeningResultSnapshot.bSuccess = true;
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 4;
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount = 3;
		return State;
	}

	TArray<FName> MakePlayerACards()
	{
		return { PlayerACard1, PlayerACard2 };
	}

	TArray<FName> MakePlayerBCards()
	{
		return { PlayerBCard1, PlayerBCard2 };
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerSuccessTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.Success",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerSuccessTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateInitializerTests::MakeRuntimeState();
	const TArray<FName> PlayerACardIds =
		MatchPlayStateInitializerTests::MakePlayerACards();
	const TArray<FName> PlayerBCardIds =
		MatchPlayStateInitializerTests::MakePlayerBCards();

	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			RuntimeState,
			PlayerACardIds,
			PlayerBCardIds);

	TestTrue(TEXT("Match play state initialization succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Top-level error code is None"),
		Result.ErrorCode,
		EMatchPlayStateInitializeErrorCode::None);
	TestEqual(
		TEXT("Underlying error code is None"),
		Result.UnderlyingCardUsageErrorCode,
		EMatchCardUsageInitializeErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerKeepsRuntimeTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.SuccessKeepsRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerKeepsRuntimeTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateInitializerTests::MakeRuntimeState();
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			RuntimeState,
			MatchPlayStateInitializerTests::MakePlayerACards(),
			MatchPlayStateInitializerTests::MakePlayerBCards());

	TestTrue(TEXT("Initialization succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA score is preserved"),
		Result.MatchPlayState.RuntimeState.PlayerAState.Score,
		RuntimeState.PlayerAState.Score);
	TestEqual(
		TEXT("PlayerB total attacks are preserved"),
		Result.MatchPlayState.RuntimeState.PlayerBState.TotalAttackCount,
		RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("Current attacking player is preserved"),
		Result.MatchPlayState.RuntimeState.CurrentAttackingPlayer,
		RuntimeState.CurrentAttackingPlayer);
	TestEqual(
		TEXT("Opening snapshot is preserved"),
		Result.MatchPlayState.RuntimeState.OpeningResultSnapshot
			.AttackCountResult.PlayerAAttackCount,
		RuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerACardsTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.SuccessInitializesPlayerACards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerACardsTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		MatchPlayStateInitializerTests::MakePlayerACards();
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			PlayerACardIds,
			MatchPlayStateInitializerTests::MakePlayerBCards());

	TestTrue(
		TEXT("PlayerA available cards preserve input"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds == PlayerACardIds);
	TestTrue(
		TEXT("PlayerA used cards start empty"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerBCardsTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.SuccessInitializesPlayerBCards",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerBCardsTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerBCardIds =
		MatchPlayStateInitializerTests::MakePlayerBCards();
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			MatchPlayStateInitializerTests::MakePlayerACards(),
			PlayerBCardIds);

	TestTrue(
		TEXT("PlayerB available cards preserve input"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds == PlayerBCardIds);
	TestTrue(
		TEXT("PlayerB used cards start empty"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerRuntimeInputTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.RuntimeInputUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerRuntimeInputTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateInitializerTests::MakeRuntimeState();
	FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			RuntimeState,
			MatchPlayStateInitializerTests::MakePlayerACards(),
			MatchPlayStateInitializerTests::MakePlayerBCards());

	Result.MatchPlayState.RuntimeState.PlayerAState.Score = 99;
	Result.MatchPlayState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	TestEqual(
		TEXT("Input PlayerA score is unchanged"),
		RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("Input current attacker is unchanged"),
		RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerCardInputsTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.CardInputsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerCardInputsTest::RunTest(
	const FString& Parameters)
{
	const TArray<FName> PlayerACardIds =
		MatchPlayStateInitializerTests::MakePlayerACards();
	const TArray<FName> PlayerBCardIds =
		MatchPlayStateInitializerTests::MakePlayerBCards();
	const TArray<FName> PlayerASnapshot = PlayerACardIds;
	const TArray<FName> PlayerBSnapshot = PlayerBCardIds;

	FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			PlayerACardIds,
			PlayerBCardIds);
	Result.MatchPlayState.CardUsageState.PlayerACardUsageState
		.AvailableCardIds.Reset();
	Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
		.AvailableCardIds.Add(TEXT("AnotherCard"));

	TestTrue(
		TEXT("PlayerA card input is unchanged"),
		PlayerACardIds == PlayerASnapshot);
	TestTrue(
		TEXT("PlayerB card input is unchanged"),
		PlayerBCardIds == PlayerBSnapshot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerAInvalidTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.PlayerAInvalidCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerAInvalidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			{ MatchPlayStateInitializerTests::PlayerACard1, NAME_None },
			MatchPlayStateInitializerTests::MakePlayerBCards());

	TestFalse(TEXT("Invalid PlayerA card fails"), Result.bSuccess);
	TestEqual(
		TEXT("Failure is mapped to card usage initialization"),
		Result.ErrorCode,
		EMatchPlayStateInitializeErrorCode::CardUsageInitializationFailed);
	TestEqual(
		TEXT("PlayerA invalid error is preserved"),
		Result.UnderlyingCardUsageErrorCode,
		EMatchCardUsageInitializeErrorCode::InvalidPlayerACardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerBInvalidTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.PlayerBInvalidCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerBInvalidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			MatchPlayStateInitializerTests::MakePlayerACards(),
			{ NAME_None });

	TestFalse(TEXT("Invalid PlayerB card fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB invalid error is preserved"),
		Result.UnderlyingCardUsageErrorCode,
		EMatchCardUsageInitializeErrorCode::InvalidPlayerBCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerADuplicateTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.PlayerADuplicateCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerADuplicateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			{
				MatchPlayStateInitializerTests::PlayerACard1,
				MatchPlayStateInitializerTests::PlayerACard1
			},
			MatchPlayStateInitializerTests::MakePlayerBCards());

	TestFalse(TEXT("Duplicate PlayerA card fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerA duplicate error is preserved"),
		Result.UnderlyingCardUsageErrorCode,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerACardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerPlayerBDuplicateTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.PlayerBDuplicateCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerPlayerBDuplicateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			MatchPlayStateInitializerTests::MakePlayerACards(),
			{
				MatchPlayStateInitializerTests::PlayerBCard1,
				MatchPlayStateInitializerTests::PlayerBCard1
			});

	TestFalse(TEXT("Duplicate PlayerB card fails"), Result.bSuccess);
	TestEqual(
		TEXT("PlayerB duplicate error is preserved"),
		Result.UnderlyingCardUsageErrorCode,
		EMatchCardUsageInitializeErrorCode::DuplicatePlayerBCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateInitializerFailureAtomicTest,
	"FMCodex.CoreRules.MatchPlayStateInitializer.FailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateInitializerFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStateInitializeResult Result =
		FMatchPlayStateInitializer::InitializeMatchPlayState(
			MatchPlayStateInitializerTests::MakeRuntimeState(),
			MatchPlayStateInitializerTests::MakePlayerACards(),
			{ MatchPlayStateInitializerTests::PlayerBCard1, NAME_None });

	TestFalse(TEXT("Failed initialization is not successful"), Result.bSuccess);
	TestFalse(
		TEXT("Failed result does not expose initialized runtime state"),
		Result.MatchPlayState.RuntimeState.bIsInitialized);
	TestTrue(
		TEXT("Failed result has no PlayerA available cards"),
		Result.MatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.IsEmpty());
	TestTrue(
		TEXT("Failed result has no PlayerB available cards"),
		Result.MatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds.IsEmpty());
	TestTrue(
		TEXT("Failed result has a diagnostic message"),
		!Result.ErrorMessage.IsEmpty());
	return true;
}

#endif
