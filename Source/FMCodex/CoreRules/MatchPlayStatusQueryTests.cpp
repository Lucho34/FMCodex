#include "MatchPlayStatusQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayAttackFlow.h"
#include "Misc/AutomationTest.h"

namespace MatchPlayStatusQueryTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardAUsed(TEXT("CardAUsed"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardBUsed1(TEXT("CardBUsed1"));
	const FName CardBUsed2(TEXT("CardBUsed2"));

	FMatchPlayState MakeMatchPlayState()
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = 5;
		RuntimeState.PlayerAState.UsedAttackCount = 2;
		RuntimeState.PlayerAState.Score = 3;
		RuntimeState.PlayerBState.TotalAttackCount = 4;
		RuntimeState.PlayerBState.UsedAttackCount = 1;
		RuntimeState.PlayerBState.Score = 2;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = 5;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = 4;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerACardUsageState.UsedCardIds =
			{ CardAUsed };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		CardUsageState.PlayerBCardUsageState.UsedCardIds =
			{ CardBUsed1, CardBUsed2 };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlayState MakeAttackReadyState()
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = 3;
		RuntimeState.PlayerAState.Score = 0;
		RuntimeState.PlayerBState.TotalAttackCount = 3;
		RuntimeState.PlayerBState.Score = 0;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = 3;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = 3;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FFormulaResolverInput MakeGoalFormulaInput()
	{
		FFormulaResolverInput Input;
		Input.FormulaType = EFormulaType::Finishing;
		Input.Attacker.BaseValue = 6.0f;
		Input.Defender.BaseValue = 4.0f;
		Input.AttackerPlayerId = TEXT("Attacker");
		Input.DefenderPlayerId = TEXT("Defender");
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryScoresTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsScores",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryScoresTest::RunTest(const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestEqual(TEXT("PlayerA Home score is returned"), Status.PlayerAScore, 3);
	TestEqual(TEXT("PlayerB Away score is returned"), Status.PlayerBScore, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryAttackerTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsCurrentAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestEqual(
		TEXT("Current attacker is returned"),
		Status.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryRemainingAttacksTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsRemainingAttackCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryRemainingAttacksTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestEqual(
		TEXT("PlayerA remaining attacks are returned"),
		Status.PlayerARemainingAttackCount,
		3);
	TestEqual(
		TEXT("PlayerB remaining attacks are returned"),
		Status.PlayerBRemainingAttackCount,
		3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryMatchEndAvailabilityTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsMatchEndedFlag",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryMatchEndAvailabilityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestFalse(
		TEXT("MatchPlayState has no persisted match-end flag"),
		Status.bHasStoredMatchEndState);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryMatchResultAvailabilityTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsMatchResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryMatchResultAvailabilityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestFalse(
		TEXT("MatchPlayState has no persisted match result type"),
		Status.bHasStoredMatchResultType);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryPlayerACardsTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsPlayerACardCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryPlayerACardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestEqual(
		TEXT("PlayerA available card count is returned"),
		Status.PlayerAAvailableCardCount,
		2);
	TestEqual(
		TEXT("PlayerA used card count is returned"),
		Status.PlayerAUsedCardCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryPlayerBCardsTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryReturnsPlayerBCardCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryPlayerBCardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayStatus Status = FMatchPlayStatusQuery::QueryStatus(
		MatchPlayStatusQueryTests::MakeMatchPlayState());

	TestEqual(
		TEXT("PlayerB available card count is returned"),
		Status.PlayerBAvailableCardCount,
		1);
	TestEqual(
		TEXT("PlayerB used card count is returned"),
		Status.PlayerBUsedCardCount,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryDoesNotModifyInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayStatusQueryTests::MakeMatchPlayState();
	const FMatchPlayStatus Status =
		FMatchPlayStatusQuery::QueryStatus(State);

	TestTrue(TEXT("Status query returns initialized flag"), Status.bIsRuntimeInitialized);
	TestEqual(
		TEXT("Input PlayerA score is unchanged"),
		State.RuntimeState.PlayerAState.Score,
		3);
	TestEqual(
		TEXT("Input PlayerB used attacks are unchanged"),
		State.RuntimeState.PlayerBState.UsedAttackCount,
		1);
	TestTrue(
		TEXT("Input PlayerA available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== TArray<FName>({
				MatchPlayStatusQueryTests::CardA1,
				MatchPlayStatusQueryTests::CardA2
			}));
	TestTrue(
		TEXT("Input PlayerB used cards are unchanged"),
		State.CardUsageState.PlayerBCardUsageState.UsedCardIds
			== TArray<FName>({
				MatchPlayStatusQueryTests::CardBUsed1,
				MatchPlayStatusQueryTests::CardBUsed2
			}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryAfterAttackTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.QueryAfterAttackReflectsUpdatedState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryAfterAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState InitialState =
		MatchPlayStatusQueryTests::MakeAttackReadyState();
	const FMatchPlayAttackFlowResult AttackResult =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			InitialState,
			MatchPlayStatusQueryTests::CardA1,
			MatchPlayStatusQueryTests::MakeGoalFormulaInput());
	const FMatchPlayStatus Status =
		FMatchPlayStatusQuery::QueryStatus(
			AttackResult.UpdatedMatchPlayState);

	TestTrue(TEXT("Attack setup succeeds"), AttackResult.bSuccess);
	TestEqual(TEXT("Updated PlayerA score is queried"), Status.PlayerAScore, 1);
	TestEqual(
		TEXT("Updated PlayerA remaining attacks are queried"),
		Status.PlayerARemainingAttackCount,
		2);
	TestEqual(
		TEXT("Updated PlayerA available card count is queried"),
		Status.PlayerAAvailableCardCount,
		1);
	TestEqual(
		TEXT("Updated PlayerA used card count is queried"),
		Status.PlayerAUsedCardCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStatusQueryRepeatTest,
	"FMCodex.CoreRules.MatchPlayStatusQuery.RepeatedQueryIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStatusQueryRepeatTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayStatusQueryTests::MakeMatchPlayState();
	const FMatchPlayStatus First =
		FMatchPlayStatusQuery::QueryStatus(State);
	const FMatchPlayStatus Second =
		FMatchPlayStatusQuery::QueryStatus(State);

	TestEqual(TEXT("Repeated score query is stable"), First.PlayerAScore, Second.PlayerAScore);
	TestEqual(
		TEXT("Repeated remaining attack query is stable"),
		First.PlayerBRemainingAttackCount,
		Second.PlayerBRemainingAttackCount);
	TestEqual(
		TEXT("Repeated card count query is stable"),
		First.PlayerAAvailableCardCount,
		Second.PlayerAAvailableCardCount);
	return true;
}

#endif
