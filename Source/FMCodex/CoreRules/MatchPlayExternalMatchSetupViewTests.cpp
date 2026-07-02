#include "MatchPlayExternalMatchSetupView.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace MatchPlayExternalMatchSetupViewTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));

	FMatchPlayState MakeInitializedState(
		const EInitialTurnOrderPlayer InitialAttacker =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 HomeTotal = 3,
		const int32 HomeUsed = 0,
		const int32 AwayTotal = 2,
		const int32 AwayUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = HomeTotal;
		RuntimeState.PlayerAState.UsedAttackCount = HomeUsed;
		RuntimeState.PlayerAState.Score = 0;
		RuntimeState.PlayerBState.TotalAttackCount = AwayTotal;
		RuntimeState.PlayerBState.UsedAttackCount = AwayUsed;
		RuntimeState.PlayerBState.Score = 0;
		RuntimeState.FirstPlayer = InitialAttacker;
		RuntimeState.SecondPlayer =
			InitialAttacker == EInitialTurnOrderPlayer::PlayerA
				? EInitialTurnOrderPlayer::PlayerB
				: EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.CurrentAttackingPlayer = InitialAttacker;
		RuntimeState.OpeningResultSnapshot.bSuccess = true;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = HomeTotal;
		RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = AwayTotal;

		FMatchCardUsageState CardUsageState;
		CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ CardA1, CardA2 };
		CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1 };
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FString LoadProductionSource()
	{
		FString Source;
		FFileHelper::LoadFileToString(
			Source,
			*(FPaths::ProjectDir()
				/ TEXT(
					"Source/FMCodex/CoreRules/MatchPlayExternalMatchSetupView.cpp")));
		return Source;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewSuccessTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.InitializedStateBuildsView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState());

	TestTrue(
		TEXT("Initialized state is read successfully"),
		View.bSuccessfullyReadInitializedState);
	TestTrue(
		TEXT("Underlying state view is initialized"),
		View.StateView.bIsRuntimeInitialized);
	TestTrue(TEXT("Setup summary is populated"), !View.SetupSummary.IsEmpty());
	TestTrue(TEXT("Successful read has no error"), View.ErrorMessage.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewCardCountsTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ReflectsCardCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewCardCountsTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState());

	TestEqual(TEXT("Home has two available cards"), View.HomeAvailableCardCount, 2);
	TestEqual(TEXT("Home has no used cards"), View.HomeUsedCardCount, 0);
	TestEqual(TEXT("Away has one available card"), View.AwayAvailableCardCount, 1);
	TestEqual(TEXT("Away has no used cards"), View.AwayUsedCardCount, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewScoresTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ReflectsInitialScores",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewScoresTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState());

	TestEqual(TEXT("Initial Home score is zero"), View.InitialHomeScore, 0);
	TestEqual(TEXT("Initial Away score is zero"), View.InitialAwayScore, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewAttackerTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ReflectsInitialAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState(
				EInitialTurnOrderPlayer::PlayerB));

	TestEqual(
		TEXT("Initial attacker is retained"),
		View.InitialAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewOpportunitiesTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ReflectsInitialOpportunities",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewOpportunitiesTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState());

	TestEqual(
		TEXT("Home has three remaining attacks"),
		View.InitialHomeRemainingAttackCount,
		3);
	TestEqual(
		TEXT("Away has two remaining attacks"),
		View.InitialAwayRemainingAttackCount,
		2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewWaitingTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ReflectsExternalRequestWaitingState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewWaitingTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(
			MatchPlayExternalMatchSetupViewTests::MakeInitializedState());

	TestTrue(
		TEXT("Initialized setup waits for external request"),
		View.bIsWaitingForExternalAttackRequest);
	TestFalse(TEXT("Initialized setup is not finished"), View.bIsMatchFinished);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewStableSummaryTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.EndedAndInvalidSummariesAreStable",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewStableSummaryTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState FinishedState =
		MatchPlayExternalMatchSetupViewTests::MakeInitializedState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			1);
	const FMatchPlayExternalMatchSetupView FinishedFirst =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(FinishedState);
	const FMatchPlayExternalMatchSetupView FinishedSecond =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(FinishedState);

	const FMatchPlayState InvalidState;
	const FMatchPlayExternalMatchSetupView InvalidFirst =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(InvalidState);
	const FMatchPlayExternalMatchSetupView InvalidSecond =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(InvalidState);

	TestTrue(TEXT("Finished setup is readable"), FinishedFirst.bSuccessfullyReadInitializedState);
	TestTrue(TEXT("Finished setup reports match end"), FinishedFirst.bIsMatchFinished);
	TestEqual(TEXT("Finished summary is stable"), FinishedFirst.SetupSummary, FinishedSecond.SetupSummary);
	TestFalse(TEXT("Invalid setup read fails"), InvalidFirst.bSuccessfullyReadInitializedState);
	TestFalse(TEXT("Invalid setup error is populated"), InvalidFirst.ErrorMessage.IsEmpty());
	TestEqual(TEXT("Invalid error is stable"), InvalidFirst.ErrorMessage, InvalidSecond.ErrorMessage);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.DoesNotModifyInputState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewInputUnchangedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalMatchSetupViewTests::MakeInitializedState();
	const int32 HomeScore = State.RuntimeState.PlayerAState.Score;
	const int32 HomeUsedAttackCount =
		State.RuntimeState.PlayerAState.UsedAttackCount;
	const TArray<FName> HomeAvailable =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;
	const TArray<FName> AwayAvailable =
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;

	const FMatchPlayExternalMatchSetupView View =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(State);

	TestTrue(TEXT("Setup view was built"), View.bSuccessfullyReadInitializedState);
	TestEqual(TEXT("Input Home score is unchanged"), State.RuntimeState.PlayerAState.Score, HomeScore);
	TestEqual(TEXT("Input opportunities are unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, HomeUsedAttackCount);
	TestTrue(TEXT("Input Home cards are unchanged"), State.CardUsageState.PlayerACardUsageState.AvailableCardIds == HomeAvailable);
	TestTrue(TEXT("Input Away cards are unchanged"), State.CardUsageState.PlayerBCardUsageState.AvailableCardIds == AwayAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalMatchSetupViewNoExecutionTest,
	"FMCodex.CoreRules.MatchPlayExternalMatchSetupView.ProductionOnlyBuildsReadOnlyStateView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalMatchSetupViewNoExecutionTest::RunTest(
	const FString& Parameters)
{
	const FString Source =
		MatchPlayExternalMatchSetupViewTests::LoadProductionSource();

	TestTrue(
		TEXT("Uses external state view"),
		Source.Contains(
			TEXT("FMatchPlayExternalStateViewQuery::BuildView")));
	TestFalse(
		TEXT("Does not call initializer"),
		Source.Contains(TEXT("Initializer::")));
	TestFalse(
		TEXT("Does not call external turn controller"),
		Source.Contains(TEXT("FMatchPlayExternalTurnController::")));
	TestFalse(
		TEXT("Does not call submit facade"),
		Source.Contains(TEXT("FMatchPlaySubmitAttackFacade::")));
	TestFalse(
		TEXT("Does not call AttackStep"),
		Source.Contains(TEXT("FMatchPlayAttackStep::")));
	TestFalse(TEXT("Does not call Flow"), Source.Contains(TEXT("Flow::")));
	TestFalse(
		TEXT("Does not call Resolver"),
		Source.Contains(TEXT("Resolver::")));
	TestFalse(
		TEXT("Does not call Executor"),
		Source.Contains(TEXT("Executor::")));
	return true;
}

#endif
