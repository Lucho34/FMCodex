#include "MatchPlayAttackFlow.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayAttackFlowTests
{
	const FName CardA1(TEXT("CardA1"));
	const FName CardA2(TEXT("CardA2"));
	const FName CardB1(TEXT("CardB1"));
	const FName CardB2(TEXT("CardB2"));
	const FName MissingCard(TEXT("MissingCard"));

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

	FMatchPlayDeploymentSlotCatalog MakeDeploymentSlotCatalog()
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("SharedAttackFlowSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;

		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots.Add(Slot);
		return Catalog;
	}

	FMatchPlayPerSideCardSnapshotAuthority MakeCardSnapshotAuthority()
	{
		FPlayerCardRuleSnapshot PlayerACard;
		PlayerACard.CardId = CardA1;
		PlayerACard.PositionTypes = { EPlayerPositionType::Attack };
		FPlayerCardRuleSnapshot PlayerBCard;
		PlayerBCard.CardId = CardB1;
		PlayerBCard.PositionTypes = { EPlayerPositionType::Defense };
		FMatchPlayPerSideCardSnapshotAuthority Authority;
		Authority.PlayerACardSnapshots.Cards.Add(PlayerACard);
		Authority.PlayerBCardSnapshots.Cards.Add(PlayerBCard);
		return Authority;
	}

	FMatchPlayState MakeMatchPlayState(
		const EInitialTurnOrderPlayer CurrentPlayer =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerAScore = 0,
		const int32 PlayerBScore = 0,
		const int32 PlayerATotal = 3,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchPlayState State;
		State.RuntimeState = MakeRuntimeState(
				CurrentPlayer,
				PlayerAScore,
				PlayerBScore,
				PlayerATotal,
				PlayerAUsed,
				PlayerBTotal,
				PlayerBUsed);
		State.CardUsageState = MakeCardUsageState();
		State.DeploymentSlotCatalog = MakeDeploymentSlotCatalog();
		State.CardSnapshotAuthority = MakeCardSnapshotAuthority();
		return State;
	}

	FFormulaResolverInput MakeFinishingInput(const bool bAttackerWins)
	{
		FFormulaResolverInput Input;
		Input.FormulaType = EFormulaType::Finishing;
		Input.Attacker.BaseValue = bAttackerWins ? 6.0f : 4.0f;
		Input.Defender.BaseValue = bAttackerWins ? 4.0f : 6.0f;
		Input.AttackerPlayerId = TEXT("Attacker");
		Input.DefenderPlayerId = TEXT("Defender");
		return Input;
	}

	bool IsUpdatedStateEmpty(const FMatchPlayState& State)
	{
		return !State.RuntimeState.bIsInitialized
			&& State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.IsEmpty()
			&& State.CardUsageState.PlayerBCardUsageState.UsedCardIds.IsEmpty()
			&& State.DeploymentSlotCatalog.Slots.IsEmpty();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackNoGoalTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.SuccessNoGoalUpdatesAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackNoGoalTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestTrue(TEXT("Non-goal attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Attack is reported as resolved"), Result.bAttackResolved);
	TestFalse(TEXT("Attack does not score"), Result.bGoalScored);
	TestEqual(
		TEXT("PlayerA attack opportunity is consumed"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		1);
	TestEqual(
		TEXT("Score remains unchanged"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackGoalTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.SuccessGoalUpdatesScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackGoalTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestTrue(TEXT("Goal attack succeeds"), Result.bSuccess);
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
	FMatchPlayAttackCardUsageTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.SuccessUpdatesCardUsage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackCardUsageTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestFalse(
		TEXT("Played card leaves available cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds.Contains(MatchPlayAttackFlowTests::CardA1));
	TestTrue(
		TEXT("Played card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(MatchPlayAttackFlowTests::CardA1));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackUpdatedStateTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.SuccessReturnsUpdatedMatchPlayState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackUpdatedStateTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();
	const FFormulaResolverInput FormulaInput =
		MatchPlayAttackFlowTests::MakeFinishingInput(true);
	const FFormulaAttackFlowResult Expected =
		FFormulaAttackFlow::ResolveFormulaAttack(
			State.RuntimeState,
			State.CardUsageState,
			MatchPlayAttackFlowTests::CardA1,
			FormulaInput);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			FormulaInput);

	TestTrue(TEXT("Wrapper attack succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Runtime used attack count comes from FormulaAttackFlow"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		Expected.UpdatedRuntimeState.PlayerAState.UsedAttackCount);
	TestEqual(
		TEXT("Runtime score comes from FormulaAttackFlow"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		Expected.UpdatedRuntimeState.PlayerAState.Score);
	TestTrue(
		TEXT("PlayerA available cards come from FormulaAttackFlow"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== Expected.UpdatedMatchCardUsageState.PlayerACardUsageState
				.AvailableCardIds);
	TestTrue(
		TEXT("PlayerA used cards come from FormulaAttackFlow"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds
			== Expected.UpdatedMatchCardUsageState.PlayerACardUsageState
				.UsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackInputUnchangedTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.InputMatchPlayStateUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackInputUnchangedTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0);

	FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));
	Result.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score = 99;
	Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
		.AvailableCardIds.Reset();

	TestEqual(
		TEXT("Input score remains unchanged"),
		State.RuntimeState.PlayerAState.Score,
		1);
	TestEqual(
		TEXT("Input used attack count remains unchanged"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestTrue(
		TEXT("Input card remains available"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackFlowTests::CardA1));
	TestTrue(
		TEXT("Input used cards remain empty"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackInvalidCardTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.InvalidCardFailsBeforeFormula",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackInvalidCardTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();
	const FMatchPlayAttackFlowResult NoneResult =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			NAME_None,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));
	const FMatchPlayAttackFlowResult MissingResult =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::MissingCard,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("None CardId fails"), NoneResult.bSuccess);
	TestEqual(
		TEXT("None CardId keeps formula result empty"),
		NoneResult.FormulaResult.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("None CardId detail is preserved"),
		NoneResult.UnderlyingFormulaAttackFlowErrorCode,
		EFormulaAttackFlowErrorCode::InvalidCardId);
	TestFalse(TEXT("Unavailable card fails"), MissingResult.bSuccess);
	TestEqual(
		TEXT("Unavailable card keeps formula result empty"),
		MissingResult.FormulaResult.FormulaType,
		EFormulaType::None);
	TestEqual(
		TEXT("Unavailable card validation failure is preserved"),
		MissingResult.UnderlyingFormulaAttackFlowErrorCode,
		EFormulaAttackFlowErrorCode::PlayCardValidationFailed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackUsedCardTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.UsedCardFails",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackUsedCardTest::RunTest(const FString& Parameters)
{
	FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();
	State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Remove(
		MatchPlayAttackFlowTests::CardA1);
	State.CardUsageState.PlayerACardUsageState.UsedCardIds.Add(
		MatchPlayAttackFlowTests::CardA1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("Used card cannot be played again"), Result.bSuccess);
	TestEqual(
		TEXT("Used card failure comes from FormulaAttackFlow"),
		Result.UnderlyingFormulaAttackFlowErrorCode,
		EFormulaAttackFlowErrorCode::PlayCardValidationFailed);
	TestEqual(
		TEXT("Formula remains unresolved"),
		Result.FormulaResult.FormulaType,
		EFormulaType::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackWrongStateAtomicTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.WrongStateFailureIsAtomic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackWrongStateAtomicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			0,
			1,
			1,
			2,
			0);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("State with no current-player opportunity fails"), Result.bSuccess);
	TestEqual(
		TEXT("Downstream failure is preserved"),
		Result.UnderlyingFormulaAttackFlowErrorCode,
		EFormulaAttackFlowErrorCode::AttackCardPlayFlowFailed);
	TestFalse(TEXT("Failed attack is not reported as resolved"), Result.bAttackResolved);
	TestTrue(
		TEXT("Failure returns no partially updated match play state"),
		MatchPlayAttackFlowTests::IsUpdatedStateEmpty(
			Result.UpdatedMatchPlayState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackGoalEndTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.GoalAndMatchEndPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackGoalEndTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			0,
			0,
			1,
			0,
			1,
			1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestTrue(TEXT("Last scoring attack succeeds"), Result.bSuccess);
	TestTrue(TEXT("Goal is exposed"), Result.bGoalScored);
	TestTrue(TEXT("Match end is exposed"), Result.bMatchEnded);
	TestEqual(
		TEXT("PlayerA Home win is exposed"),
		Result.MatchResultType,
		EMatchResultType::HomeWin);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackNoGoalEndTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.NoGoalAndMatchEndPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackNoGoalEndTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			1,
			1,
			1,
			0,
			1,
			1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestTrue(TEXT("Last non-scoring attack succeeds"), Result.bSuccess);
	TestFalse(TEXT("No goal is exposed"), Result.bGoalScored);
	TestTrue(TEXT("Match end is exposed"), Result.bMatchEnded);
	TestEqual(
		TEXT("Draw result is exposed"),
		Result.MatchResultType,
		EMatchResultType::Draw);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackNotEndedTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.MatchNotEndedPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackNotEndedTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestTrue(TEXT("Attack succeeds"), Result.bSuccess);
	TestFalse(TEXT("Match remains active"), Result.bMatchEnded);
	TestEqual(
		TEXT("Result remains NotEnded"),
		Result.MatchResultType,
		EMatchResultType::NotEnded);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackPlayerATest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.PlayerAAttackUpdatesPlayerAState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackPlayerATest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA2,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestEqual(
		TEXT("Acting player is PlayerA"),
		Result.ActingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestTrue(
		TEXT("PlayerA card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.Contains(MatchPlayAttackFlowTests::CardA2));
	TestTrue(
		TEXT("PlayerB card state is unchanged"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerBCardUsageState
			.AvailableCardIds
			== State.CardUsageState.PlayerBCardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackPlayerBTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.PlayerBAttackUpdatesPlayerBState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackPlayerBTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerB,
			0,
			1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardB1,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestTrue(TEXT("PlayerB attack succeeds"), Result.bSuccess);
	TestEqual(
		TEXT("Acting player is PlayerB"),
		Result.ActingPlayer,
		EInitialTurnOrderPlayer::PlayerB);
	TestEqual(
		TEXT("PlayerB Away score increases"),
		Result.UpdatedMatchPlayState.RuntimeState.PlayerBState.Score,
		2);
	TestTrue(
		TEXT("PlayerB card enters used cards"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerBCardUsageState
			.UsedCardIds.Contains(MatchPlayAttackFlowTests::CardB1));
	TestTrue(
		TEXT("PlayerA card state is unchanged"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.AvailableCardIds
			== State.CardUsageState.PlayerACardUsageState.AvailableCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackFailureCardTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.FailureDoesNotConsumeCard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackFailureCardTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::MissingCard,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("Unavailable card fails"), Result.bSuccess);
	TestTrue(
		TEXT("Input available cards remain unchanged"),
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds.Contains(
			MatchPlayAttackFlowTests::CardA1));
	TestTrue(
		TEXT("Input used cards remain empty"),
		State.CardUsageState.PlayerACardUsageState.UsedCardIds.IsEmpty());
	TestTrue(
		TEXT("Failure does not expose updated card state"),
		Result.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackFailureOpportunityTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.FailureDoesNotConsumeAttackOpportunity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackFailureOpportunityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			NAME_None,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("Invalid card fails"), Result.bSuccess);
	TestEqual(
		TEXT("Input attack opportunity remains unconsumed"),
		State.RuntimeState.PlayerAState.UsedAttackCount,
		0);
	TestFalse(
		TEXT("Failure does not expose initialized updated runtime"),
		Result.UpdatedMatchPlayState.RuntimeState.bIsInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackFailureScoreTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.FailureDoesNotChangeScore",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackFailureScoreTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			1);

	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			NAME_None,
			MatchPlayAttackFlowTests::MakeFinishingInput(true));

	TestFalse(TEXT("Invalid card fails"), Result.bSuccess);
	TestEqual(
		TEXT("Input PlayerA score remains unchanged"),
		State.RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("Input PlayerB score remains unchanged"),
		State.RuntimeState.PlayerBState.Score,
		1);
	TestFalse(
		TEXT("Failure does not expose initialized updated runtime"),
		Result.UpdatedMatchPlayState.RuntimeState.bIsInitialized);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackDeterministicTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.DeterministicFormulaInput",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackDeterministicTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();
	const FFormulaResolverInput FormulaInput =
		MatchPlayAttackFlowTests::MakeFinishingInput(true);

	const FMatchPlayAttackFlowResult First =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			FormulaInput);
	const FMatchPlayAttackFlowResult Second =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			FormulaInput);

	TestTrue(TEXT("Repeated deterministic attacks succeed"), First.bSuccess && Second.bSuccess);
	TestEqual(
		TEXT("Repeated input keeps score result"),
		First.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score,
		Second.UpdatedMatchPlayState.RuntimeState.PlayerAState.Score);
	TestEqual(
		TEXT("Repeated input keeps used attack count"),
		First.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		Second.UpdatedMatchPlayState.RuntimeState.PlayerAState.UsedAttackCount);
	TestTrue(
		TEXT("Repeated input keeps used card state"),
		First.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
			.UsedCardIds
			== Second.UpdatedMatchPlayState.CardUsageState.PlayerACardUsageState
				.UsedCardIds);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayAttackPreservesDeploymentSlotCatalogTest,
	"FMCodex.CoreRules.MatchPlayAttackFlow.SuccessPreservesDeploymentSlotCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayAttackPreservesDeploymentSlotCatalogTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayAttackFlowTests::MakeMatchPlayState();
	const FMatchPlayAttackFlowResult Result =
		FMatchPlayAttackFlow::ResolveMatchPlayAttack(
			State,
			MatchPlayAttackFlowTests::CardA1,
			MatchPlayAttackFlowTests::MakeFinishingInput(false));

	TestTrue(TEXT("Attack flow succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Successful attack flow preserves the deployment slot catalog"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.UpdatedMatchPlayState.DeploymentSlotCatalog,
				&State.DeploymentSlotCatalog,
				0));
	TestTrue(
		TEXT("Successful attack flow preserves card snapshot authority"),
		FMatchPlayPerSideCardSnapshotAuthority::StaticStruct()
			->CompareScriptStruct(
				&Result.UpdatedMatchPlayState.CardSnapshotAuthority,
				&State.CardSnapshotAuthority,
				0));
	TestFalse(
		TEXT("Existing attack flow still returns no current attack"),
		Result.UpdatedMatchPlayState.bHasCurrentAttack);
	return true;
}

#endif
