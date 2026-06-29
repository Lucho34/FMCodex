#include "MatchPlayAttackExecutor.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayAttackExecutorTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName SharedCard(TEXT("SharedCard"));
	const FName MissingCard(TEXT("MissingCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchRuntimeState RuntimeState;
		RuntimeState.bIsInitialized = true;
		RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		RuntimeState.PlayerAState.Score = PlayerAScore;
		RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		RuntimeState.PlayerBState.Score = PlayerBScore;
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
		return FMatchPlayState::Create(RuntimeState, CardUsageState);
	}

	FMatchPlayAttackRequest MakeRequest(
		const EInitialTurnOrderPlayer PlayerSide =
			EInitialTurnOrderPlayer::PlayerA,
		const FName CardId = CardA1,
		const bool bGoal = true,
		const bool bHasFormulaInput = true)
	{
		FMatchPlayAttackRequest Request;
		Request.PlayerSide = PlayerSide;
		Request.CardId = CardId;
		Request.bHasExternalFormulaInput = bHasFormulaInput;
		Request.FormulaInput.FormulaType = EFormulaType::Finishing;
		Request.FormulaInput.Attacker.BaseValue = bGoal ? 6.0f : 4.0f;
		Request.FormulaInput.Defender.BaseValue = bGoal ? 4.0f : 6.0f;
		Request.FormulaInput.AttackerPlayerId = TEXT("Attacker");
		Request.FormulaInput.DefenderPlayerId = TEXT("Defender");
		return Request;
	}

	bool IsUpdatedStateEmpty(const FMatchPlayState& State)
	{
		return !State.RuntimeState.bIsInitialized
			&& State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState.UsedCardIds.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorSuccessTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessExecutesValidAttackRequest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Valid attack request executes"), Result.bSuccess);
	TestTrue(TEXT("Attack is resolved"), Result.bAttackResolved);
	TestEqual(TEXT("Execution has no error"), Result.ErrorCode, EMatchPlayAttackExecutionErrorCode::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorUpdatedStateTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessUpdatesMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorUpdatedStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Updated runtime is initialized"),
		Result.UpdatedMatchPlayState.RuntimeState.bIsInitialized);
	TestEqual(
		TEXT("Updated score is returned"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		1);
	TestEqual(
		TEXT("Updated attack count is returned"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorConsumesCardTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessConsumesCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorConsumesCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestFalse(
		TEXT("Played card leaves available cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(MatchPlayAttackExecutorTests::CardA1));
	TestTrue(
		TEXT("Played card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(MatchPlayAttackExecutorTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorConsumesOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessConsumesAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorConsumesOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestEqual(
		TEXT("Current attack opportunity is consumed"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorGoalTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessGoalUpdatesScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorGoalTest::RunTest(const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				2,
				1),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Goal request succeeds"), Result.bSuccess);
	TestTrue(TEXT("Goal is exposed"), Result.bGoalScored);
	TestEqual(
		TEXT("PlayerA Home score increases"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		3);
	TestEqual(
		TEXT("PlayerB Away score is unchanged"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerBState.Score,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorNoGoalTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessNoGoalDoesNotUpdateScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorNoGoalTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				2,
				1),
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::CardA1,
				false));

	TestTrue(TEXT("Non-goal request succeeds"), Result.bSuccess);
	TestFalse(TEXT("No goal is exposed"), Result.bGoalScored);
	TestEqual(
		TEXT("PlayerA score is unchanged"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("PlayerB score is unchanged"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerBState.Score,
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorFlowResultTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessReturnsAttackFlowResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorFlowResultTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestTrue(TEXT("AttackFlow result succeeds"), Result.AttackFlowResult.bSuccess);
	TestEqual(
		TEXT("AttackFlow played CardId is retained"),
		Result.AttackFlowResult.PlayedCardId,
		MatchPlayAttackExecutorTests::CardA1);
	TestEqual(
		TEXT("AttackFlow acting player is retained"),
		Result.AttackFlowResult.ActingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorValidationResultTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SuccessReturnsValidationResult",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorValidationResultTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Validation result is valid"), Result.ValidationResult.bValid);
	TestTrue(TEXT("Validation permits card play"), Result.ValidationResult.bCanPlay);
	TestEqual(
		TEXT("Validated CardId is retained"),
		Result.ValidationResult.AttackRequest.CardId,
		MatchPlayAttackExecutorTests::CardA1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorValidationShortCircuitTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.ValidationFailureDoesNotCallAttackFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorValidationShortCircuitTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid request fails"), Result.bSuccess);
	TestEqual(
		TEXT("Validation failure is reported"),
		Result.ErrorCode,
		EMatchPlayAttackExecutionErrorCode::RequestValidationFailed);
	TestFalse(TEXT("AttackFlow result remains unsuccessful"), Result.AttackFlowResult.bSuccess);
	TestTrue(
		TEXT("AttackFlow is not populated"),
		Result.AttackFlowResult.PlayedCardId.IsNone());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorInvalidPlayerAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.InvalidRequestPlayerFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorInvalidPlayerAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid player fails"), Result.bSuccess);
	TestEqual(
		TEXT("Underlying invalid player is retained"),
		Result.UnderlyingRequestValidationErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::InvalidRequestPlayer);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	TestEqual(TEXT("Input score remains unchanged"), State.RuntimeState.PlayerAState.Score, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorMissingFormulaAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.MissingFormulaInputFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorMissingFormulaAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::CardA1,
				true,
				false));

	TestFalse(TEXT("Missing formula input fails"), Result.bSuccess);
	TestEqual(
		TEXT("Missing formula error is retained"),
		Result.UnderlyingRequestValidationErrorCode,
		EMatchPlayAttackRequestValidationErrorCode::MissingFormulaInput);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorNotCurrentAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.NotCurrentAttackerFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorNotCurrentAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackExecutorTests::CardB1));

	TestFalse(TEXT("Non-current attacker fails"), Result.bSuccess);
	TestEqual(
		TEXT("Not-current availability reason is retained"),
		Result.ValidationResult.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::NotCurrentAttacker);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorUnavailableAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.CardUnavailableFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorUnavailableAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::MissingCard));

	TestFalse(TEXT("Unavailable card fails"), Result.bSuccess);
	TestEqual(
		TEXT("Card unavailable reason is retained"),
		Result.ValidationResult.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardNotAvailable);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorUsedAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.CardAlreadyUsedFailsAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorUsedAtomicTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayAttackExecutorTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayAttackExecutorTests::CardA1);

	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest());

	TestFalse(TEXT("Already-used card fails"), Result.bSuccess);
	TestEqual(
		TEXT("Already-used reason is retained"),
		Result.ValidationResult.AvailabilityResult.ErrorCode,
		EMatchPlayAvailabilityErrorCode::CardAlreadyUsed);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorFlowFailureAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.AttackFlowFailureIsAtomicAtExecutorLevel",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorFlowFailureAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);
	FMatchPlayAttackRequest Request =
		MatchPlayAttackExecutorTests::MakeRequest();
	Request.FormulaInput.FormulaType = EFormulaType::Transition;

	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(State, Request);

	TestFalse(TEXT("Invalid execution formula fails AttackFlow"), Result.bSuccess);
	TestTrue(TEXT("Request validation succeeds first"), Result.ValidationResult.bValid);
	TestEqual(
		TEXT("AttackFlow failure is reported"),
		Result.ErrorCode,
		EMatchPlayAttackExecutionErrorCode::AttackFlowFailed);
	TestEqual(
		TEXT("Underlying MatchPlayAttackFlow failure is retained"),
		Result.UnderlyingMatchPlayAttackFlowErrorCode,
		EMatchPlayAttackFlowErrorCode::FormulaAttackFlowFailed);
	TestTrue(
		TEXT("Failure returns empty updated state"),
		MatchPlayAttackExecutorTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	TestEqual(TEXT("Input score remains unchanged"), State.RuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Input attack count remains unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Input card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackExecutorTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorInputStateTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.InputMatchPlayStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorInputStateTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const TArray<FName> AvailableCards =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;

	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input score is unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("Input used attacks are unchanged"), State.RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Input available cards are unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== AvailableCards);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorInputRequestTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.InputAttackRequestUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorInputRequestTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackExecutorTests::MakeRequest();

	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			Request);

	TestTrue(TEXT("Execution succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input request CardId is unchanged"), Request.CardId, MatchPlayAttackExecutorTests::CardA1);
	TestEqual(TEXT("Input request player is unchanged"), Request.PlayerSide, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("Input formula value is unchanged"), Request.FormulaInput.Attacker.BaseValue, 6.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorFailureCardTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.FailureDoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorFailureCardTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::MissingCard));

	TestFalse(TEXT("Invalid request fails"), Result.bSuccess);
	TestTrue(
		TEXT("Existing card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackExecutorTests::CardA1));
	TestTrue(
		TEXT("Used cards remain empty"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorFailureOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.FailureDoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorFailureOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid request fails"), Result.bSuccess);
	TestEqual(
		TEXT("Used attack count remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorFailureScoreTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.FailureDoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorFailureScoreTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::None));

	TestFalse(TEXT("Invalid request fails"), Result.bSuccess);
	TestEqual(TEXT("PlayerA score remains unchanged"), State.RuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score remains unchanged"), State.RuntimeState.PlayerBState.Score, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorGoalEndTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.GoalAndMatchEndPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorGoalEndTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				0,
				0,
				1,
				0,
				1,
				1),
			MatchPlayAttackExecutorTests::MakeRequest());

	TestTrue(TEXT("Final goal request succeeds"), Result.bSuccess);
	TestTrue(TEXT("Goal is exposed"), Result.bGoalScored);
	TestTrue(TEXT("Match end is exposed"), Result.bMatchEnded);
	TestEqual(TEXT("Home win is exposed"), Result.MatchResultType, EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorNoGoalEndTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.NoGoalAndMatchEndPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorNoGoalEndTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				1,
				1,
				1,
				0,
				1,
				1),
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::CardA1,
				false));

	TestTrue(TEXT("Final non-goal request succeeds"), Result.bSuccess);
	TestFalse(TEXT("No goal is exposed"), Result.bGoalScored);
	TestTrue(TEXT("Match end is exposed"), Result.bMatchEnded);
	TestEqual(TEXT("Draw is exposed"), Result.MatchResultType, EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorNotEndedTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.MatchNotEndedPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorNotEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			MatchPlayAttackExecutorTests::MakeState(),
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::CardA1,
				false));

	TestTrue(TEXT("Request succeeds"), Result.bSuccess);
	TestFalse(TEXT("Match remains active"), Result.bMatchEnded);
	TestEqual(TEXT("Result remains NotEnded"), Result.MatchResultType, EMatchResultType::NotEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorPlayerATest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.PlayerARequestUpdatesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorPlayerATest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::CardA2,
				false));

	TestTrue(TEXT("PlayerA request succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerA card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(MatchPlayAttackExecutorTests::CardA2));
	TestTrue(
		TEXT("PlayerB card state is unchanged"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds
			== State.CardUsageState.PlayerBCardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorPlayerBTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.PlayerBRequestUpdatesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorPlayerBTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState(
			EInitialTurnOrderPlayer::PlayerB);
	const FMatchPlayAttackExecutionResult Result =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			State,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackExecutorTests::CardB1));

	TestTrue(TEXT("PlayerB request succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("PlayerB card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(MatchPlayAttackExecutorTests::CardB1));
	TestTrue(
		TEXT("PlayerA card state is unchanged"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== State.CardUsageState.PlayerACardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorSharedCardTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.SameCardIdAcrossPlayersHandledCorrectly",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorSharedCardTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState PlayerAState =
		MatchPlayAttackExecutorTests::MakeState();
	PlayerAState.CardUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		MatchPlayAttackExecutorTests::SharedCard);
	PlayerAState.CardUsageState.PlayerBCardUsageState.AvailableCardIds.Add(
		MatchPlayAttackExecutorTests::SharedCard);
	FMatchPlayState PlayerBState = PlayerAState;
	PlayerBState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	const FMatchPlayAttackExecutionResult PlayerAResult =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			PlayerAState,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerA,
				MatchPlayAttackExecutorTests::SharedCard,
				false));
	const FMatchPlayAttackExecutionResult PlayerBResult =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(
			PlayerBState,
			MatchPlayAttackExecutorTests::MakeRequest(
				EInitialTurnOrderPlayer::PlayerB,
				MatchPlayAttackExecutorTests::SharedCard,
				false));

	TestTrue(TEXT("PlayerA shared-card request succeeds"), PlayerAResult.bSuccess);
	TestTrue(TEXT("PlayerB shared-card request succeeds"), PlayerBResult.bSuccess);
	TestTrue(
		TEXT("PlayerA consumes own shared card"),
		PlayerAResult.UpdatedMatchPlayState.CardUsageState
			.PlayerACardUsageState.UsedCardIds.Contains(
				MatchPlayAttackExecutorTests::SharedCard));
	TestTrue(
		TEXT("PlayerB consumes own shared card"),
		PlayerBResult.UpdatedMatchPlayState.CardUsageState
			.PlayerBCardUsageState.UsedCardIds.Contains(
				MatchPlayAttackExecutorTests::SharedCard));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackExecutorRepeatedInvalidTest,
	"FMCodex.CoreRules.MatchPlayAttackExecutor.RepeatedInvalidRequestIsConsistent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackExecutorRepeatedInvalidTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackExecutorTests::MakeState();
	const FMatchPlayAttackRequest Request =
		MatchPlayAttackExecutorTests::MakeRequest(
			EInitialTurnOrderPlayer::None);

	const FMatchPlayAttackExecutionResult First =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(State, Request);
	const FMatchPlayAttackExecutionResult Second =
		FMatchPlayAttackExecutor::ExecuteAttackRequest(State, Request);

	TestEqual(TEXT("Repeated failures are stable"), First.bSuccess, Second.bSuccess);
	TestEqual(TEXT("Repeated error codes are stable"), First.ErrorCode, Second.ErrorCode);
	TestEqual(
		TEXT("Repeated underlying errors are stable"),
		First.UnderlyingRequestValidationErrorCode,
		Second.UnderlyingRequestValidationErrorCode);
	TestTrue(
		TEXT("Repeated failure leaves input card available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackExecutorTests::CardA1));
	return true;
}

#endif
