#include "MatchPlayExternalStateView.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayExternalStateViewTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardAUsed(TEXT("CardAUsed"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardBUsed(TEXT("CardBUsed"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 HomeScore = 2,
		const int32 AwayScore = 1,
		const int32 HomeTotal = 3,
		const int32 HomeUsed = 1,
		const int32 AwayTotal = 4,
		const int32 AwayUsed = 2,
		const bool bHomeHasAvailableCards = true)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = HomeTotal;
		RuntimeState.PlayerAState.UsedAttackCount = HomeUsed;
		RuntimeState.PlayerAState.Score = HomeScore;
		RuntimeState.PlayerBState.TotalAttackCount = AwayTotal;
		RuntimeState.PlayerBState.UsedAttackCount = AwayUsed;
		RuntimeState.PlayerBState.Score = AwayScore;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer = CurrentPlayer;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = HomeTotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = AwayTotal;

		FMatchCardUsageState CardUsageState;
		if (bHomeHasAvailableCards)
		{
			CardUsageState.PlayerACardUsageState.AvailableCardIds =
				{ CardA1, CardA2 };
		}
		CardUsageState.PlayerACardUsageState.UsedCardIds =
			{ CardAUsed };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		CardUsageState.PlayerBCardUsageState.UsedCardIds =
			{ CardBUsed };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlayExternalStateView.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewNormalTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.NormalStateBuildsView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewNormalTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(
			MatchPlayExternalStateViewTests::MakeState());

	TestTrue(TEXT("Runtime is initialized"), View.bIsRuntimeInitialized);
	TestFalse(TEXT("Match is not finished"), View.bIsMatchFinished);
	TestTrue(
		TEXT("View waits for external request"),
		View.bIsWaitingForExternalAttackRequest);
	TestTrue(
		TEXT("View can accept an attack request"),
		View.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Ready code is retained"),
		View.ReadinessCode,
		EMatchPlayLoopReadinessCode::ReadyForExternalAttackRequest);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewScoreAndOpportunityTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ReflectsScoresAttackerAndOpportunities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewScoreAndOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(
			MatchPlayExternalStateViewTests::MakeState());

	TestEqual(TEXT("Home score is PlayerA score"), View.HomeScore, 2);
	TestEqual(TEXT("Away score is PlayerB score"), View.AwayScore, 1);
	TestEqual(
		TEXT("Current attacker is retained"),
		View.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Home remaining attacks are retained"),
		View.HomeRemainingAttackCount,
		2);
	TestEqual(
		TEXT("Away remaining attacks are retained"),
		View.AwayRemainingAttackCount,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewCardsTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ReflectsCardCountsAndIds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewCardsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(
			MatchPlayExternalStateViewTests::MakeState());

	TestEqual(TEXT("Home available count is retained"), View.HomeAvailableCardCount, 2);
	TestEqual(TEXT("Home used count is retained"), View.HomeUsedCardCount, 1);
	TestEqual(TEXT("Away available count is retained"), View.AwayAvailableCardCount, 1);
	TestEqual(TEXT("Away used count is retained"), View.AwayUsedCardCount, 1);
	TestTrue(
		TEXT("Home available IDs contain CardA1"),
		View.HomeAvailableCardIds.Contains(
			MatchPlayExternalStateViewTests::CardA1));
	TestTrue(
		TEXT("Home used IDs contain CardAUsed"),
		View.HomeUsedCardIds.Contains(
			MatchPlayExternalStateViewTests::CardAUsed));
	TestTrue(
		TEXT("Away available IDs contain CardB1"),
		View.AwayAvailableCardIds.Contains(
			MatchPlayExternalStateViewTests::CardB1));
	TestTrue(
		TEXT("Away used IDs contain CardBUsed"),
		View.AwayUsedCardIds.Contains(
			MatchPlayExternalStateViewTests::CardBUsed));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewFinishedTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.FinishedMatchCannotSubmit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewFinishedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(
			MatchPlayExternalStateViewTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				3,
				2,
				1,
				1,
				1,
				1));

	TestTrue(TEXT("Match is finished"), View.bIsMatchFinished);
	TestFalse(TEXT("Finished match cannot submit"), View.bCanSubmitAttackRequest);
	TestFalse(
		TEXT("Finished match is not waiting"),
		View.bIsWaitingForExternalAttackRequest);
	TestEqual(
		TEXT("No-opportunities code is retained"),
		View.ReadinessCode,
		EMatchPlayLoopReadinessCode::NoRemainingAttackOpportunities);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewStableReasonTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.CannotSubmitReasonIsStable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewStableReasonTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalStateViewTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1,
			3,
			1,
			4,
			2,
			false);
	const FMatchPlayExternalStateView First =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalStateView Second =
		FMatchPlayExternalStateViewQuery::BuildView(State);

	TestFalse(TEXT("No-card state cannot submit"), First.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("No-card readiness code is retained"),
		First.ReadinessCode,
		EMatchPlayLoopReadinessCode::CurrentAttackerHasNoAvailableCards);
	TestFalse(
		TEXT("Cannot-submit reason is populated"),
		First.CannotSubmitReason.IsEmpty());
	TestEqual(
		TEXT("Reason is stable"),
		First.CannotSubmitReason,
		Second.CannotSubmitReason);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.DoesNotModifyInputState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalStateViewTests::MakeState();
	const int32 HomeScore = State.RuntimeState.PlayerAState.Score;
	const int32 HomeUsedAttackCount =
		State.RuntimeState.PlayerAState.UsedAttackCount;
	const TArray<FName> HomeAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;
	const TArray<FName> HomeUsed =
		State.CardUsageState.PlayerACardUsageState.UsedCardIds;

	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(State);

	TestTrue(TEXT("View was built"), View.bIsRuntimeInitialized);
	TestEqual(
		TEXT("Input score is unchanged"),
		State.RuntimeState.PlayerAState.Score,
		HomeScore);
	TestEqual(
		TEXT("Input opportunity use is unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		HomeUsedAttackCount);
	TestTrue(
		TEXT("Input available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== HomeAvailable);
	TestTrue(
		TEXT("Input used cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds
			== HomeUsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewNoAdvancingCallsTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ProductionHasNoStateAdvancingCalls",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewNoAdvancingCallsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalStateViewTests::LoadProductionSource();

	TestFalse(
		TEXT("Does not call Submit facade"),
		Source.Contains(TEXT("FMatchPlaySubmitAttackFacade::")));
	TestFalse(
		TEXT("Does not call AttackStep"),
		Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(
		TEXT("Does not call MatchPlayAttackFlow"),
		Source.Contains(TEXT("FMatchPlayAttackFlow::")));
	TestFalse(
		TEXT("Does not call AttackResolutionFlow"),
		Source.Contains(TEXT("FAttackResolutionFlow::")));
	TestFalse(
		TEXT("Does not call FormulaAttackFlow"),
		Source.Contains(TEXT("FFormulaAttackFlow::")));
	TestFalse(
		TEXT("Does not call Resolver"),
		Source.Contains(TEXT("Resolver::")));
	TestFalse(
		TEXT("Does not call Executor"),
		Source.Contains(TEXT("Executor::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewReadOnlyAggregationTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ProductionUsesReadOnlyQueries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewReadOnlyAggregationTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalStateViewTests::LoadProductionSource();

	TestTrue(
		TEXT("Uses status query"),
		Source.Contains(TEXT("FMatchPlayStatusQuery::QueryStatus")));
	TestTrue(
		TEXT("Uses loop readiness"),
		Source.Contains(TEXT("FMatchPlayLoopReadiness::Evaluate")));
	TestFalse(
		TEXT("Does not call SubmissionGate"),
		Source.Contains(TEXT("FMatchPlaySubmissionGate::")));
	TestFalse(
		TEXT("Does not call AvailabilityQuery"),
		Source.Contains(TEXT("FMatchPlayAvailabilityQuery::")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewNoLoopOrRandomnessTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ProductionHasNoLoopOrRandomness",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewNoLoopOrRandomnessTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalStateViewTests::LoadProductionSource();

	TestFalse(TEXT("No while loop"), Source.Contains(TEXT("while (")));
	TestFalse(TEXT("No for loop"), Source.Contains(TEXT("for (")));
	TestFalse(TEXT("No Rand call"), Source.Contains(TEXT("Rand(")));
	TestFalse(TEXT("No RandRange call"), Source.Contains(TEXT("RandRange")));
	TestFalse(
		TEXT("No RandomStream usage"),
		Source.Contains(TEXT("RandomStream")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalStateViewNoExternalSystemsTest,
	"FMCodex.CoreRules.MatchPlayExternalStateView.ProductionHasNoExternalSystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalStateViewNoExternalSystemsTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalStateViewTests::LoadProductionSource();

	TestFalse(
		TEXT("No UI dependency"),
		Source.Contains(TEXT("UI"), ESearchCase::CaseSensitive));
	TestFalse(
		TEXT("No Blueprint dependency"),
		Source.Contains(TEXT("Blueprint")));
	TestFalse(TEXT("No networking dependency"), Source.Contains(TEXT("Network")));
	TestFalse(TEXT("No Steam dependency"), Source.Contains(TEXT("Steam")));
	TestFalse(TEXT("No skill dependency"), Source.Contains(TEXT("Skill")));
	TestFalse(
		TEXT("No card database dependency"),
		Source.Contains(TEXT("Database")));
	TestFalse(TEXT("No AI dependency"), Source.Contains(TEXT("AIModule")));
	TestFalse(
		TEXT("No automatic card selection"),
		Source.Contains(TEXT("SelectCard")));
	return true;
}

#endif
