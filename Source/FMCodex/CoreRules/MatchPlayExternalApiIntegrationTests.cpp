#include "MatchPlayExternalStateView.h"
#include "MatchPlayExternalTurnController.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayExternalApiIntegrationTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const int32 HomeTotal = 2,
		const int32 HomeUsed = 0,
		const int32 AwayTotal = 2,
		const int32 AwayUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = HomeTotal;
		RuntimeState.PlayerAState.UsedAttackCount = HomeUsed;
		RuntimeState.PlayerAState.Score = 1;
		RuntimeState.PlayerBState.TotalAttackCount = AwayTotal;
		RuntimeState.PlayerBState.UsedAttackCount = AwayUsed;
		RuntimeState.PlayerBState.Score = 0;
		RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
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

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer Player =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = Player;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = true;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = 6.0f;
		Request.FormulaInput.Defender.BaseValue = 4.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return Left.RuntimeState.bIsInitialized
				== Right.RuntimeState.bIsInitialized
			&& Left.RuntimeState.PlayerAState.TotalAttackCount
				== Right.RuntimeState.PlayerAState.TotalAttackCount
			&& Left.RuntimeState.PlayerAState.UsedAttackCount
				== Right.RuntimeState.PlayerAState.UsedAttackCount
			&& Left.RuntimeState.PlayerAState.Score
				== Right.RuntimeState.PlayerAState.Score
			&& Left.RuntimeState.PlayerBState.TotalAttackCount
				== Right.RuntimeState.PlayerBState.TotalAttackCount
			&& Left.RuntimeState.PlayerBState.UsedAttackCount
				== Right.RuntimeState.PlayerBState.UsedAttackCount
			&& Left.RuntimeState.PlayerBState.Score
				== Right.RuntimeState.PlayerBState.Score
			&& Left.RuntimeState.CurrentAttackingPlayer
				== Right.RuntimeState.CurrentAttackingPlayer
			&& Left.CardUsageState.PlayerACardUsageState.AvailableCardIds
				== Right.CardUsageState.PlayerACardUsageState.AvailableCardIds
			&& Left.CardUsageState.PlayerACardUsageState.UsedCardIds
				== Right.CardUsageState.PlayerACardUsageState.UsedCardIds
			&& Left.CardUsageState.PlayerBCardUsageState.AvailableCardIds
				== Right.CardUsageState.PlayerBCardUsageState.AvailableCardIds
			&& Left.CardUsageState.PlayerBCardUsageState.UsedCardIds
				== Right.CardUsageState.PlayerBCardUsageState.UsedCardIds;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiInitialViewTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.InitialViewCanAttemptSubmission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiInitialViewTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(
			MatchPlayExternalApiIntegrationTests::MakeState());

	TestTrue(TEXT("Initial state can accept a request"), View.bCanSubmitAttackRequest);
	TestTrue(
		TEXT("Initial state waits for external request"),
		View.bIsWaitingForExternalAttackRequest);
	TestFalse(TEXT("Initial state is not finished"), View.bIsMatchFinished);
	TestEqual(
		TEXT("Initial attacker is Home"),
		View.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiControllerSuccessTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.ValidRequestReturnsReadableSuccess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiControllerSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState();
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest());

	TestTrue(TEXT("Request is handled"), Result.bHandled);
	TestTrue(TEXT("Submission succeeds"), Result.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Controller reports success"),
		Result.Code,
		EMatchPlayExternalTurnControllerCode::SubmittedSuccessfully);
	TestTrue(TEXT("Submit result succeeds"), Result.SubmitResult.bSuccess);
	TestTrue(
		TEXT("Readable result view reports success"),
		Result.ResultView.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Readable result view reports goal"),
		Result.ResultView.ExecutionSummary.bGoalScored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiCardSummaryDeltaTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.PostSubmitViewShowsCardUsageDelta",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiCardSummaryDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState();
	const FMatchPlayExternalStateView BeforeView =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest());
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestEqual(TEXT("Home begins with two available cards"), BeforeView.HomeAvailableCardCount, 2);
	TestEqual(TEXT("Home begins with no used cards"), BeforeView.HomeUsedCardCount, 0);
	TestEqual(TEXT("Home has one available card after submit"), AfterView.HomeAvailableCardCount, 1);
	TestEqual(TEXT("Home has one used card after submit"), AfterView.HomeUsedCardCount, 1);
	TestFalse(
		TEXT("Played card is no longer available"),
		AfterView.HomeAvailableCardIds.Contains(
			MatchPlayExternalApiIntegrationTests::CardA1));
	TestTrue(
		TEXT("Played card is listed as used"),
		AfterView.HomeUsedCardIds.Contains(
			MatchPlayExternalApiIntegrationTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiTurnDeltaTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.PostSubmitViewShowsTurnDelta",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiTurnDeltaTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState();
	const FMatchPlayExternalStateView BeforeView =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest());
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestEqual(TEXT("Home score increases by one"), AfterView.HomeScore, BeforeView.HomeScore + 1);
	TestEqual(TEXT("Away score is unchanged"), AfterView.AwayScore, BeforeView.AwayScore);
	TestEqual(TEXT("Home remaining attacks decrease by one"), AfterView.HomeRemainingAttackCount, BeforeView.HomeRemainingAttackCount - 1);
	TestEqual(TEXT("Away remaining attacks are unchanged"), AfterView.AwayRemainingAttackCount, BeforeView.AwayRemainingAttackCount);
	TestEqual(
		TEXT("Current attacker advances to Away"),
		AfterView.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiFinishedConsistencyTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.FinishedStateViewAndControllerAgree",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiFinishedConsistencyTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState(1, 1, 1, 1);
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest());

	TestTrue(TEXT("View reports finished match"), View.bIsMatchFinished);
	TestFalse(TEXT("View rejects submission attempt"), View.bCanSubmitAttackRequest);
	TestFalse(TEXT("Controller does not submit"), Result.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Controller reports submission rejection"),
		Result.Code,
		EMatchPlayExternalTurnControllerCode::SubmissionRejected);
	TestTrue(
		TEXT("Rejected result remains atomic"),
		MatchPlayExternalApiIntegrationTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiInvalidAtomicTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.InvalidRequestKeepsViewsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiInvalidAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState();
	const FMatchPlayExternalStateView BeforeView =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayExternalApiIntegrationTests::CardB1));
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestFalse(TEXT("Invalid request does not submit"), Result.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Invalid request keeps state unchanged"),
		MatchPlayExternalApiIntegrationTests::AreStatesEqual(
			Result.AfterState,
			State));
	TestEqual(TEXT("Score view is unchanged"), AfterView.HomeScore, BeforeView.HomeScore);
	TestEqual(TEXT("Attacker view is unchanged"), AfterView.CurrentAttackingPlayer, BeforeView.CurrentAttackingPlayer);
	TestTrue(TEXT("Available-card view is unchanged"), AfterView.HomeAvailableCardIds == BeforeView.HomeAvailableCardIds);
	TestTrue(TEXT("Used-card view is unchanged"), AfterView.HomeUsedCardIds == BeforeView.HomeUsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiReadinessNotValidityTest,
	"FMCodex.CoreRules.MatchPlayExternalApiIntegration.StateReadinessDoesNotGuaranteeRequestValidity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiReadinessNotValidityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiIntegrationTests::MakeState();
	const FMatchPlayExternalStateView View =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			MatchPlayExternalApiIntegrationTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayExternalApiIntegrationTests::MissingCard));

	TestTrue(
		TEXT("State is ready to accept a request"),
		View.bCanSubmitAttackRequest);
	TestFalse(
		TEXT("Specific unavailable-card request is rejected"),
		Result.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Result view identifies pre-execution rejection"),
		Result.ResultView.bWasRejectedBeforeExecution);
	TestEqual(
		TEXT("Gate preserves specific card reason"),
		Result.SubmitResult.SubmissionGateResult.Code,
		EMatchPlaySubmissionGateCode::CardNotAvailable);
	TestTrue(
		TEXT("Rejected request keeps state unchanged"),
		MatchPlayExternalApiIntegrationTests::AreStatesEqual(
			Result.AfterState,
			State));
	return true;
}

#endif
