#include "MatchPlayAvailabilityQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayAvailabilityQueryTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName SharedCard(TEXT("SharedCard"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = 2;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = 1;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
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
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("TestDeploymentSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityCanPlayTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CanPlayWhenCurrentAttackerHasAvailableCardAndOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityCanPlayTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestTrue(TEXT("Current attacker can play available card"), Result.bCanPlay);
	TestEqual(
		TEXT("Successful query has no error"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::None);
	TestEqual(
		TEXT("Remaining opportunities are exposed"),
		Result.PlayerRemainingAttackCount,
		3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityNotCurrentTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayWhenNotCurrentAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityNotCurrentTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayAvailabilityQueryTests::CardB1);

	TestFalse(TEXT("Non-current player cannot play"), Result.bCanPlay);
	TestEqual(
		TEXT("Non-current player error is returned"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NotCurrentAttacker);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityNoPlayerOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayWhenNoRemainingAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityNoPlayerOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				2,
				0),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestFalse(TEXT("Player without opportunity cannot play"), Result.bCanPlay);
	TestEqual(
		TEXT("No-opportunity error is returned"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NoRemainingAttackOpportunity);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityNoMatchOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayWhenBothPlayersHaveNoRemainingAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityNoMatchOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				2,
				2),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestFalse(TEXT("Match without opportunities rejects play"), Result.bCanPlay);
	TestEqual(
		TEXT("No-match-opportunity error is returned"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode
			::MatchAlreadyEndedOrNoAttackRemaining);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityInvalidPlayerTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayInvalidPlayer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityInvalidPlayerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::None,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestFalse(TEXT("Invalid player cannot play"), Result.bCanPlay);
	TestEqual(
		TEXT("Invalid player error is returned"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::InvalidPlayer);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityInvalidCardTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayInvalidCardId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityInvalidCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			NAME_None);

	TestFalse(TEXT("None CardId cannot be played"), Result.bCanPlay);
	TestEqual(
		TEXT("Invalid CardId is mapped"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::InvalidCardId);
	TestEqual(
		TEXT("Underlying invalid CardId is preserved"),
		Result.UnderlyingPlayCardErrorCode,
		EPlayCardResolveErrorCode::InvalidCardId);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityUnavailableCardTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayUnavailableCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityUnavailableCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::MissingCard);

	TestFalse(TEXT("Unavailable card cannot be played"), Result.bCanPlay);
	TestEqual(
		TEXT("Unavailable card error is mapped"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityUsedCardTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.CannotPlayUsedCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityUsedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayAvailabilityQueryTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayAvailabilityQueryTests::CardA1);

	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestFalse(TEXT("Used card cannot be played"), Result.bCanPlay);
	TestEqual(
		TEXT("Used card error is mapped"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardAlreadyUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityPlayerAIsolationTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.PlayerACardAvailabilityUsesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityPlayerAIsolationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(),
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardB1);

	TestFalse(TEXT("PlayerA cannot use PlayerB-only card"), Result.bCanPlay);
	TestEqual(
		TEXT("PlayerA state reports card unavailable"),
		Result.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityPlayerBIsolationTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.PlayerBCardAvailabilityUsesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityPlayerBIsolationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			MatchPlayAvailabilityQueryTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB),
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayAvailabilityQueryTests::CardB1);

	TestTrue(TEXT("PlayerB can use PlayerB available card"), Result.bCanPlay);
	TestEqual(
		TEXT("PlayerB is retained in result"),
		Result.PlayerSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilitySharedCardTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.SameCardIdAcrossPlayersHandledCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilitySharedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState PlayerAState =
		MatchPlayAvailabilityQueryTests::MakeState();
	PlayerAState.CardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		MatchPlayAvailabilityQueryTests::SharedCard);
	PlayerAState.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		MatchPlayAvailabilityQueryTests::SharedCard);
	FMatchPlayState PlayerBState = PlayerAState;
	PlayerBState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchPlayAvailabilityResult PlayerAResult =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			PlayerAState,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::SharedCard);
	const FMatchPlayAvailabilityResult PlayerBResult =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			PlayerBState,
			EInitialTurnOrderPlayer::PlayerB,
			MatchPlayAvailabilityQueryTests::SharedCard);

	TestTrue(TEXT("PlayerA can use own shared CardId"), PlayerAResult.bCanPlay);
	TestTrue(TEXT("PlayerB can use own shared CardId"), PlayerBResult.bCanPlay);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.QueryDoesNotModifyInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();
	const TArray<FName> PlayerAAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestTrue(TEXT("Query succeeds"), Result.bCanPlay);
	TestEqual(
		TEXT("Input used attacks are unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestEqual(
		TEXT("Input score is unchanged"),
		State.RuntimeState.PlayerAState.Score,
		2);
	TestTrue(
		TEXT("Input available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerAAvailable);
	TestTrue(
		TEXT("Input used cards remain empty"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityDoesNotConsumeCardTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.QueryDoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityDoesNotConsumeCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();

	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestTrue(TEXT("Availability query succeeds"), Result.bCanPlay);
	TestTrue(
		TEXT("Queried card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAvailabilityQueryTests::CardA1));
	TestFalse(
		TEXT("Queried card does not enter used cards"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.Contains(
			MatchPlayAvailabilityQueryTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityDoesNotConsumeOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.QueryDoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityDoesNotConsumeOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();

	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestTrue(TEXT("Availability query succeeds"), Result.bCanPlay);
	TestEqual(
		TEXT("Used attack count remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityDoesNotChangeScoreTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.QueryDoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityDoesNotChangeScoreTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();

	const FMatchPlayAvailabilityResult Result =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestTrue(TEXT("Availability query succeeds"), Result.bCanPlay);
	TestEqual(TEXT("PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score remains unchanged"), State.RuntimeState.PlayerBState.Score, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAvailabilityRepeatedTest,
	"FMCodex.CoreRules.MatchPlayAvailabilityQuery.QueryCanRunRepeatedlyConsistently",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAvailabilityRepeatedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAvailabilityQueryTests::MakeState();

	const FMatchPlayAvailabilityResult First =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);
	const FMatchPlayAvailabilityResult Second =
		FMatchPlayAvailabilityQuery::QueryCanPlayCard(
			State,
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayAvailabilityQueryTests::CardA1);

	TestEqual(TEXT("Repeated bCanPlay is stable"), First.bCanPlay, Second.bCanPlay);
	TestEqual(TEXT("Repeated error code is stable"), First.ErrorCode, Second.ErrorCode);
	TestEqual(
		TEXT("Repeated remaining opportunity is stable"),
		First.PlayerRemainingAttackCount,
		Second.PlayerRemainingAttackCount);
	return true;
}

#endif
