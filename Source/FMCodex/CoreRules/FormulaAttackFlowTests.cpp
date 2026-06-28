#include "FormulaAttackFlow.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace FormulaAttackFlowTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardB2(TEXT("CardB2"));

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
			{ CardA1, CardA2 };
		State.PlayerBCardUsageState.AvailableCardIds =
			{ CardB1, CardB2 };
		return State;
	}

	FFormulaResolverInput MakeFinishingInput(
		const bool bAttackerWins)
	{
		FFormulaResolverInput Input;
		Input.FormulaType = EFormulaType::Finishing;
		Input.Attacker.BaseValue = bAttackerWins ? 6.0f : 4.0f;
		Input.Defender.BaseValue = bAttackerWins ? 4.0f : 6.0f;
		Input.AttackerPlayerId = TEXT("Attacker");
		Input.DefenderPlayerId = TEXT("Defender");
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackPlayerAGoalTest,
	"FMCodex.CoreRules.FormulaAttackFlow.PlayerAGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackPlayerAGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestTrue(TEXT("PlayerA formula attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Formula goal is mapped"), Result.bGoalScored);
	TestEqual(TEXT("Acting player is PlayerA"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerA);
	TestEqual(TEXT("PlayerA score increases"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerA card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardA1));
	TestTrue(TEXT("Formula result is retained"), Result.FormulaResult.bIsGoal);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackPlayerANoGoalTest,
	"FMCodex.CoreRules.FormulaAttackFlow.PlayerANoGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackPlayerANoGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(false);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA2,
			FormulaInput);

	TestTrue(TEXT("PlayerA non-goal formula attack succeeds"), Result.bSuccess);
	TestFalse(TEXT("Defender formula win is not a goal"), Result.bGoalScored);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 2);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("PlayerA attack is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerA card is still used"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardA2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackPlayerBGoalTest,
	"FMCodex.CoreRules.FormulaAttackFlow.PlayerBGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackPlayerBGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB,
			0,
			0);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardB1,
			FormulaInput);

	TestTrue(TEXT("PlayerB formula attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Formula goal is mapped"), Result.bGoalScored);
	TestEqual(TEXT("Acting player is PlayerB"), Result.ActingPlayer, EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("PlayerB score increases"), Result.UpdatedRuntimeState.PlayerBState.Score, 1);
	TestEqual(TEXT("PlayerB attack is consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerB card enters used cards"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardB1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackPlayerBNoGoalTest,
	"FMCodex.CoreRules.FormulaAttackFlow.PlayerBNoGoal",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackPlayerBNoGoalTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB,
			1,
			2);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(false);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardB2,
			FormulaInput);

	TestTrue(TEXT("PlayerB non-goal formula attack succeeds"), Result.bSuccess);
	TestFalse(TEXT("Defender formula win is not a goal"), Result.bGoalScored);
	TestEqual(TEXT("PlayerB score is unchanged"), Result.UpdatedRuntimeState.PlayerBState.Score, 2);
	TestEqual(TEXT("PlayerB attack is consumed"), Result.UpdatedRuntimeState.PlayerBState.UsedAttackCount, 1);
	TestTrue(
		TEXT("PlayerB card is still used"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardB2));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackInvalidFormulaTest,
	"FMCodex.CoreRules.FormulaAttackFlow.InvalidFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackInvalidFormulaTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState();
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);
	FormulaInput.FormulaType = EFormulaType::Transition;

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestFalse(TEXT("Non-finishing formula is rejected"), Result.bSuccess);
	TestEqual(
		TEXT("Non-finishing formula reports formula failure"),
		Result.ErrorCode,
		EFormulaAttackFlowErrorCode::FormulaResolveFailed);
	TestEqual(
		TEXT("Rejected formula is not resolved"),
		Result.FormulaResult.FormulaType,
		EFormulaType::None);
	TestEqual(TEXT("Score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 0);
	TestEqual(TEXT("Attack is not consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Card remains available"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(FormulaAttackFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackCardFailureTest,
	"FMCodex.CoreRules.FormulaAttackFlow.CardPlayFailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackCardFailureTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardB1,
			FormulaInput);

	TestFalse(TEXT("Wrong-player card fails the flow"), Result.bSuccess);
	TestEqual(
		TEXT("Formula is not resolved for an unavailable card"),
		Result.FormulaResult.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("Card validation failure is reported"),
		Result.ErrorCode,
		EFormulaAttackFlowErrorCode::PlayCardValidationFailed);
	TestEqual(
		TEXT("Unavailable-card detail is retained"),
		Result.PlayCardValidationErrorCode,
		EPlayCardResolveErrorCode::CardNotAvailable);
	TestEqual(TEXT("PlayerA score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Attack is not consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Card state remains unchanged"),
		Result.UpdatedMatchCardUsageState.PlayerBCardUsageState
			.AvailableCardIds.Contains(FormulaAttackFlowTests::CardB1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackDownstreamFailureTest,
	"FMCodex.CoreRules.FormulaAttackFlow.AttackCardPlayFlowFailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackDownstreamFailureTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0,
			1,
			1,
			2,
			0);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestFalse(TEXT("Downstream attack flow fails"), Result.bSuccess);
	TestTrue(
		TEXT("Formula resolves before downstream failure"),
		Result.FormulaResult.bIsGoal);
	TestEqual(
		TEXT("Downstream failure is reported"),
		Result.ErrorCode,
		EFormulaAttackFlowErrorCode::AttackCardPlayFlowFailed);
	TestEqual(
		TEXT("AttackCardPlayFlow reports attack resolution failure"),
		Result.AttackCardPlayResult.ErrorCode,
		EAttackCardPlayFlowErrorCode::AttackResolutionFlowFailed);
	TestEqual(
		TEXT("Score remains unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.Score,
		RuntimeState.PlayerAState.Score);
	TestEqual(
		TEXT("Used attack count remains unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount,
		RuntimeState.PlayerAState.UsedAttackCount);
	TestTrue(
		TEXT("Card remains available"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(FormulaAttackFlowTests::CardA1));
	TestFalse(
		TEXT("Card does not enter used cards"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardA1));
	TestTrue(
		TEXT("Input runtime remains unchanged"),
		RuntimeState.PlayerAState.Score == 1
			&& RuntimeState.PlayerAState.UsedAttackCount == 1);
	TestTrue(
		TEXT("Input card state remains unchanged"),
		CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(FormulaAttackFlowTests::CardA1)
			&& CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackUsedCardValidationTest,
	"FMCodex.CoreRules.FormulaAttackFlow.UsedCardRejectedBeforeFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackUsedCardValidationTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);
	FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		FormulaAttackFlowTests::CardA1);
	CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		FormulaAttackFlowTests::CardA1);
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestFalse(TEXT("Used card fails validation"), Result.bSuccess);
	TestEqual(
		TEXT("Formula is not resolved for a used card"),
		Result.FormulaResult.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("Used card reports validation failure"),
		Result.ErrorCode,
		EFormulaAttackFlowErrorCode::PlayCardValidationFailed);
	TestEqual(
		TEXT("Used-card detail is retained"),
		Result.PlayCardValidationErrorCode,
		EPlayCardResolveErrorCode::CardAlreadyUsed);
	TestEqual(
		TEXT("Score is unchanged"),
		Result.UpdatedRuntimeState.PlayerAState.Score,
		1);
	TestEqual(
		TEXT("Attack is not consumed"),
		Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestTrue(
		TEXT("Used card remains used"),
		Result.UpdatedMatchCardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(FormulaAttackFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackInputsUnchangedTest,
	"FMCodex.CoreRules.FormulaAttackFlow.InputsUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackInputsUnchangedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestTrue(TEXT("Formula attack succeeds"), Result.bSuccess);
	TestEqual(TEXT("Input score is unchanged"), RuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Input used attack count is unchanged"), RuntimeState.PlayerAState.UsedAttackCount, 0);
	TestTrue(
		TEXT("Input card remains available"),
		CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			FormulaAttackFlowTests::CardA1));
	TestEqual(TEXT("Formula input remains unchanged"), FormulaInput.Attacker.BaseValue, 6.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackAlreadyEndedTest,
	"FMCodex.CoreRules.FormulaAttackFlow.MatchAlreadyEnded",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackAlreadyEndedTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None,
			1,
			0,
			1,
			1,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestFalse(TEXT("Ended match rejects formula attack"), Result.bSuccess);
	TestTrue(TEXT("Ended state is reported"), Result.bMatchEnded);
	TestEqual(
		TEXT("Ended match reports its error"),
		Result.ErrorCode,
		EFormulaAttackFlowErrorCode::MatchAlreadyEnded);
	TestEqual(TEXT("Score is unchanged"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackLastAttackTest,
	"FMCodex.CoreRules.FormulaAttackFlow.LastAttackEndsMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackLastAttackTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			0,
			1,
			1);
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);

	TestTrue(TEXT("Last formula attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Last attack ends match"), Result.bMatchEnded);
	TestEqual(TEXT("Goal is recorded"), Result.UpdatedRuntimeState.PlayerAState.Score, 1);
	TestEqual(TEXT("Last opportunity is consumed"), Result.UpdatedRuntimeState.PlayerAState.UsedAttackCount, 1);
	TestEqual(TEXT("Final result is HomeWin"), Result.MatchResultType, EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackFinalResultsTest,
	"FMCodex.CoreRules.FormulaAttackFlow.FinalMatchResults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackFinalResultsTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput NoGoalFormula =
		FormulaAttackFlowTests::MakeFinishingInput(false);

	const FMatchRuntimeState HomeWinRuntime =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1,
			1,
			0,
			1,
			1);
	const FFormulaAttackFlowResult HomeWinResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			HomeWinRuntime,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			NoGoalFormula);
	TestTrue(TEXT("Home-win formula flow succeeds"), HomeWinResult.bSuccess);
	TestEqual(TEXT("HomeWin is returned"), HomeWinResult.MatchResultType, EMatchResultType::HomeWin);

	const FMatchRuntimeState AwayWinRuntime =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			2,
			1,
			0,
			1,
			1);
	const FFormulaAttackFlowResult AwayWinResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			AwayWinRuntime,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			NoGoalFormula);
	TestTrue(TEXT("Away-win formula flow succeeds"), AwayWinResult.bSuccess);
	TestEqual(TEXT("AwayWin is returned"), AwayWinResult.MatchResultType, EMatchResultType::AwayWin);

	const FMatchRuntimeState DrawRuntime =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			0,
			1,
			1);
	const FFormulaAttackFlowResult DrawResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			DrawRuntime,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			NoGoalFormula);
	TestTrue(TEXT("Draw formula flow succeeds"), DrawResult.bSuccess);
	TestEqual(TEXT("Draw is returned"), DrawResult.MatchResultType, EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackPreservesStructuralStateTest,
	"FMCodex.CoreRules.FormulaAttackFlow.PreservesStructuralState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackPreservesStructuralStateTest::RunTest(
	const FString& Parameters)
{
	FMatchRuntimeState RuntimeState =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::PlayerB);
	RuntimeState.OpeningResultSnapshot.InitialTurnOrderResult.FirstPlayer =
		EInitialTurnOrderPlayer::PlayerA;
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FFormulaAttackFlowResult Result =
		FFormulaAttackFlow::ResolveFormulaAttack(
			RuntimeState,
			CardUsageState,
			FormulaAttackFlowTests::CardB1,
			FormulaInput);

	TestTrue(TEXT("Formula attack succeeds"), Result.bSuccess);
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
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FFormulaAttackInvalidInputsTest,
	"FMCodex.CoreRules.FormulaAttackFlow.InvalidInputs",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFormulaAttackInvalidInputsTest::RunTest(const FString& Parameters)
{
	const FMatchCardUsageState CardUsageState =
		FormulaAttackFlowTests::MakeCardUsageState();
	const FFormulaResolverInput FormulaInput =
		FormulaAttackFlowTests::MakeFinishingInput(true);

	const FMatchRuntimeState UninitializedRuntime;
	const FFormulaAttackFlowResult UninitializedResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			UninitializedRuntime,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);
	TestEqual(
		TEXT("Uninitialized runtime is invalid"),
		UninitializedResult.ErrorCode,
		EFormulaAttackFlowErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState InvalidPlayerRuntime =
		FormulaAttackFlowTests::MakeRuntimeState(
			EInitialTurnOrderPlayer::None);
	const FFormulaAttackFlowResult InvalidPlayerResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			InvalidPlayerRuntime,
			CardUsageState,
			FormulaAttackFlowTests::CardA1,
			FormulaInput);
	TestEqual(
		TEXT("Invalid current player is invalid runtime"),
		InvalidPlayerResult.ErrorCode,
		EFormulaAttackFlowErrorCode::InvalidRuntimeState);

	const FMatchRuntimeState ValidRuntime =
		FormulaAttackFlowTests::MakeRuntimeState();
	const FFormulaAttackFlowResult InvalidCardResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			ValidRuntime,
			CardUsageState,
			NAME_None,
			FormulaInput);
	TestEqual(
		TEXT("None CardId is invalid"),
		InvalidCardResult.ErrorCode,
		EFormulaAttackFlowErrorCode::InvalidCardId);

	FMatchCardUsageState InvalidUsageState = CardUsageState;
	InvalidUsageState.PlayerACardUsageState.AvailableCardIds.Add(
		FormulaAttackFlowTests::CardA1);
	const FFormulaAttackFlowResult InvalidUsageResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			ValidRuntime,
			InvalidUsageState,
			FormulaAttackFlowTests::CardA2,
			FormulaInput);
	TestEqual(
		TEXT("Invalid selected card state is reported"),
		InvalidUsageResult.ErrorCode,
		EFormulaAttackFlowErrorCode::InvalidMatchCardUsageState);
	TestEqual(
		TEXT("Invalid selected state detail is retained"),
		InvalidUsageResult.PlayCardValidationErrorCode,
		EPlayCardResolveErrorCode::InvalidMatchCardUsageState);
	TestEqual(
		TEXT("Invalid card state is rejected before formula resolution"),
		InvalidUsageResult.FormulaResult.FormulaType,
		EFormulaType::None);
	return true;
}

#endif
