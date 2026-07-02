#include "MatchPlayExternalAttackRequestPreflight.h"
#include "MatchPlayExternalMatchSetupView.h"
#include "MatchPlayExternalStateView.h"
#include "MatchPlayExternalTurnController.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayExternalApiV1LifecycleTests
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
			&& Left.RuntimeState.FirstPlayer
				== Right.RuntimeState.FirstPlayer
			&& Left.RuntimeState.SecondPlayer
				== Right.RuntimeState.SecondPlayer
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
	FMatchPlayExternalApiV1SuccessfulLifecycleTest,
	"FMCodex.CoreRules.MatchPlayExternalApiV1Lifecycle.SuccessfulRequestTraversesRecommendedLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiV1SuccessfulLifecycleTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiV1LifecycleTests::MakeState();
	const FMatchPlayExternalMatchSetupView SetupView =
		FMatchPlayExternalMatchSetupViewQuery::BuildView(State);
	const FMatchPlayExternalStateView BeforeView =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayAttackRequest Request =
		MatchPlayExternalApiV1LifecycleTests::MakeRequest();
	const FMatchPlayExternalAttackRequestPreflightView Preflight =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(State, Request);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			Request);
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestTrue(
		TEXT("Setup view reads initialized state"),
		SetupView.bSuccessfullyReadInitializedState);
	TestEqual(
		TEXT("Setup view preserves initial attacker"),
		SetupView.InitialAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestTrue(
		TEXT("State view reports state-level readiness"),
		BeforeView.bCanSubmitAttackRequest);
	TestTrue(
		TEXT("Specific request passes preflight"),
		Preflight.bCanSubmit);
	TestTrue(
		TEXT("Preflight retains state-level readiness"),
		Preflight.bStateReadyForAttackRequest);
	TestTrue(
		TEXT("Controller submits request"),
		Result.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Result view reports readable success"),
		Result.ResultView.bSubmittedSuccessfully);
	TestEqual(
		TEXT("Home score increases"),
		AfterView.HomeScore,
		BeforeView.HomeScore + 1);
	TestEqual(
		TEXT("Home remaining opportunities decrease"),
		AfterView.HomeRemainingAttackCount,
		BeforeView.HomeRemainingAttackCount - 1);
	TestEqual(
		TEXT("Turn advances to Away"),
		AfterView.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("Home available-card count decreases"),
		AfterView.HomeAvailableCardCount,
		BeforeView.HomeAvailableCardCount - 1);
	TestEqual(
		TEXT("Home used-card count increases"),
		AfterView.HomeUsedCardCount,
		BeforeView.HomeUsedCardCount + 1);
	TestTrue(
		TEXT("Played card appears in used-card summary"),
		AfterView.HomeUsedCardIds.Contains(
			MatchPlayExternalApiV1LifecycleTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiV1InvalidAtomicLifecycleTest,
	"FMCodex.CoreRules.MatchPlayExternalApiV1Lifecycle.InvalidRequestRemainsAtomicAcrossViews",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiV1InvalidAtomicLifecycleTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiV1LifecycleTests::MakeState();
	const FMatchPlayExternalStateView BeforeView =
		FMatchPlayExternalStateViewQuery::BuildView(State);
	const FMatchPlayAttackRequest Request =
		MatchPlayExternalApiV1LifecycleTests::MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayExternalApiV1LifecycleTests::MissingCard);
	const FMatchPlayExternalAttackRequestPreflightView Preflight =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(State, Request);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			Request);
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestTrue(
		TEXT("State remains ready for a well-formed request"),
		BeforeView.bCanSubmitAttackRequest);
	TestFalse(
		TEXT("Unavailable card fails specific preflight"),
		Preflight.bCanSubmit);
	TestEqual(
		TEXT("Preflight exposes card rejection"),
		Preflight.GateCode,
		EMatchPlaySubmissionGateCode::CardNotAvailable);
	TestFalse(
		TEXT("Controller rejects invalid request"),
		Result.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Result identifies pre-execution rejection"),
		Result.ResultView.bWasRejectedBeforeExecution);
	TestTrue(
		TEXT("Rejected request preserves complete state"),
		MatchPlayExternalApiV1LifecycleTests::AreStatesEqual(
			Result.AfterState,
			State));
	TestEqual(
		TEXT("Score remains unchanged"),
		AfterView.HomeScore,
		BeforeView.HomeScore);
	TestEqual(
		TEXT("Remaining opportunities remain unchanged"),
		AfterView.HomeRemainingAttackCount,
		BeforeView.HomeRemainingAttackCount);
	TestTrue(
		TEXT("Available-card summary remains unchanged"),
		AfterView.HomeAvailableCardIds
			== BeforeView.HomeAvailableCardIds);
	TestTrue(
		TEXT("Used-card summary remains unchanged"),
		AfterView.HomeUsedCardIds == BeforeView.HomeUsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiV1CompletionLifecycleTest,
	"FMCodex.CoreRules.MatchPlayExternalApiV1Lifecycle.FinalSubmissionMatchesFinishedStateView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiV1CompletionLifecycleTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayExternalApiV1LifecycleTests::MakeState(1, 0, 1, 1);
	const FMatchPlayAttackRequest Request =
		MatchPlayExternalApiV1LifecycleTests::MakeRequest();
	const FMatchPlayExternalAttackRequestPreflightView Preflight =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(State, Request);
	const FMatchPlayExternalTurnControllerResult Result =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			State,
			Request);
	const FMatchPlayExternalStateView AfterView =
		FMatchPlayExternalStateViewQuery::BuildView(Result.AfterState);

	TestTrue(
		TEXT("Final request passes preflight"),
		Preflight.bCanSubmit);
	TestTrue(
		TEXT("Final request submits"),
		Result.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Result view reports match ended"),
		Result.ResultView.bMatchEnded);
	TestEqual(
		TEXT("Execution summary reports Home win"),
		Result.ResultView.ExecutionSummary.MatchResultType,
		EMatchResultType::HomeWin);
	TestTrue(
		TEXT("Post-submit state view reports finished match"),
		AfterView.bIsMatchFinished);
	TestFalse(
		TEXT("Finished state cannot accept another request"),
		AfterView.bCanSubmitAttackRequest);
	TestEqual(
		TEXT("Result and state view agree on final Home score"),
		Result.ResultView.AfterScore.PlayerAScore,
		AfterView.HomeScore);
	TestEqual(
		TEXT("Result and state view agree on final Away score"),
		Result.ResultView.AfterScore.PlayerBScore,
		AfterView.AwayScore);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayExternalApiV1PreflightRevalidationTest,
	"FMCodex.CoreRules.MatchPlayExternalApiV1Lifecycle.PreflightDoesNotBypassSubmitRevalidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayExternalApiV1PreflightRevalidationTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState InitialState =
		MatchPlayExternalApiV1LifecycleTests::MakeState();
	const FMatchPlayAttackRequest StaleRequest =
		MatchPlayExternalApiV1LifecycleTests::MakeRequest(
			EInitialTurnOrderPlayer::PlayerA,
			MatchPlayExternalApiV1LifecycleTests::CardA1);
	const FMatchPlayExternalAttackRequestPreflightView InitialPreflight =
		FMatchPlayExternalAttackRequestPreflight::Evaluate(
			InitialState,
			StaleRequest);
	const FMatchPlayExternalTurnControllerResult InterveningResult =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			InitialState,
			MatchPlayExternalApiV1LifecycleTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayExternalApiV1LifecycleTests::CardA2));
	const FMatchPlayExternalTurnControllerResult StaleResult =
		FMatchPlayExternalTurnController::HandleAttackRequest(
			InterveningResult.AfterState,
			StaleRequest);

	TestTrue(
		TEXT("Request is valid at preflight time"),
		InitialPreflight.bCanSubmit);
	TestTrue(
		TEXT("Intervening external request changes state"),
		InterveningResult.bSubmittedSuccessfully);
	TestFalse(
		TEXT("Stale request is rejected by final submit validation"),
		StaleResult.bSubmittedSuccessfully);
	TestTrue(
		TEXT("Stale rejection occurs before execution"),
		StaleResult.ResultView.bWasRejectedBeforeExecution);
	TestTrue(
		TEXT("Stale rejection preserves latest state"),
		MatchPlayExternalApiV1LifecycleTests::AreStatesEqual(
			StaleResult.AfterState,
			InterveningResult.AfterState));
	TestTrue(
		TEXT("Unplayed stale card remains available"),
		StaleResult.AfterState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(
				MatchPlayExternalApiV1LifecycleTests::CardA1));
	return true;
}

#endif
