#include "MatchPlayBeginOrdinaryAttack.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayBeginOrdinaryAttackTests
{
	const FName PlayerACard(TEXT("PlayerACard"));
	const FName PlayerAUsedCard(TEXT("PlayerAUsedCard"));
	const FName PlayerBCard(TEXT("PlayerBCard"));
	const FName PlayerBUsedCard(TEXT("PlayerBUsedCard"));

	FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer CurrentAttacker =
			EInitialTurnOrderPlayer::PlayerA,
		const int32 PlayerATotal = 4,
		const int32 PlayerAUsed = 0,
		const int32 PlayerBTotal = 3,
		const int32 PlayerBUsed = 0)
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.PlayerAState.TotalAttackCount = PlayerATotal;
		State.RuntimeState.PlayerAState.UsedAttackCount = PlayerAUsed;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerAState.GoalkeeperCardId = TEXT("PlayerAGoalkeeper");
		State.RuntimeState.PlayerAState.InitialDeckRarityScore = 11;
		State.RuntimeState.PlayerBState.TotalAttackCount = PlayerBTotal;
		State.RuntimeState.PlayerBState.UsedAttackCount = PlayerBUsed;
		State.RuntimeState.PlayerBState.Score = 1;
		State.RuntimeState.PlayerBState.GoalkeeperCardId = TEXT("PlayerBGoalkeeper");
		State.RuntimeState.PlayerBState.InitialDeckRarityScore = 9;
		State.RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer = CurrentAttacker;
		State.RuntimeState.OpeningResultSnapshot.bSuccess = true;
		State.RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerAAttackCount = PlayerATotal;
		State.RuntimeState.OpeningResultSnapshot.AttackCountResult
			.PlayerBAttackCount = PlayerBTotal;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds =
			{ PlayerACard };
		State.CardUsageState.PlayerACardUsageState.UsedCardIds =
			{ PlayerAUsedCard };
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds =
			{ PlayerBCard };
		State.CardUsageState.PlayerBCardUsageState.UsedCardIds =
			{ PlayerBUsedCard };
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("SharedBeginSlot");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots.Add(Slot);
		return State;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}

	bool TestAtomicFailure(
		FAutomationTestBase& Test,
		const TCHAR* Context,
		const FMatchPlayState& BeforeState,
		const int32 ActionPoint,
		const EMatchPlayBeginOrdinaryAttackErrorCode ExpectedErrorCode)
	{
		const FMatchPlayBeginOrdinaryAttackResult Result =
			FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, ActionPoint);
		Test.TestFalse(
			*FString::Printf(TEXT("%s fails"), Context),
			Result.bSuccess);
		Test.TestEqual(
			*FString::Printf(TEXT("%s returns the expected error"), Context),
			Result.ErrorCode,
			ExpectedErrorCode);
		Test.TestTrue(
			*FString::Printf(TEXT("%s preserves BeforeState in the result"), Context),
			AreStatesEqual(Result.BeforeState, BeforeState));
		Test.TestTrue(
			*FString::Printf(TEXT("%s leaves AfterState field-for-field unchanged"), Context),
			AreStatesEqual(Result.AfterState, BeforeState));
		Test.TestEqual(
			*FString::Printf(TEXT("%s preserves ActionPoint"), Context),
			Result.ActionPoint,
			ActionPoint);
		return true;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackPlayerASuccessTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.PlayerABeginsCompleteDeploymentState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackPlayerASuccessTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState BeforeState =
		MatchPlayBeginOrdinaryAttackTests::MakeState();
	BeforeState.CurrentAttack.Phase =
		EMatchPlayCurrentAttackPhase::Resolution;
	BeforeState.CurrentAttack.AttackSequence = 99;
	BeforeState.CurrentAttack.ActionPoint = 8;
	BeforeState.CurrentAttack.CurrentLegalDeploymentSide =
		EInitialTurnOrderPlayer::PlayerB;
	BeforeState.CurrentAttack.bAttackerDeploymentFinished = true;
	BeforeState.CurrentAttack.bDefenderDeploymentFinished = true;
	BeforeState.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	FMatchPlayDeploymentPlacement StalePlacement;
	StalePlacement.PlayerSide = EInitialTurnOrderPlayer::PlayerB;
	StalePlacement.CardId = TEXT("StaleCard");
	StalePlacement.SlotId = TEXT("StaleSlot");
	BeforeState.CurrentAttack.DeploymentPlacements.Add(StalePlacement);
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, 5);

	TestTrue(TEXT("Begin succeeds"), Result.bSuccess);
	TestEqual(TEXT("Error is None"), Result.ErrorCode,
		EMatchPlayBeginOrdinaryAttackErrorCode::None);
	TestTrue(TEXT("Current attack is active"), Result.AfterState.bHasCurrentAttack);
	TestEqual(TEXT("Phase is Deployment"), Result.AfterState.CurrentAttack.Phase,
		EMatchPlayCurrentAttackPhase::Deployment);
	TestEqual(TEXT("Sequence starts at one"), Result.AfterState.CurrentAttack.AttackSequence,
		int64{ 1 });
	TestEqual(TEXT("Action point is retained"), Result.AfterState.CurrentAttack.ActionPoint, 5);
	TestEqual(TEXT("Attacker deploys first"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerA);
	TestFalse(TEXT("Attacker is not finished"),
		Result.AfterState.CurrentAttack.bAttackerDeploymentFinished);
	TestFalse(TEXT("Defender is not finished"),
		Result.AfterState.CurrentAttack.bDefenderDeploymentFinished);
	TestTrue(TEXT("Placements start empty"),
		Result.AfterState.CurrentAttack.DeploymentPlacements.IsEmpty());
	TestFalse(TEXT("Current-defense goalkeeper starts inactive"),
		Result.AfterState.CurrentAttack.bCurrentDefenseGoalkeeperActivated);
	TestTrue(TEXT("BeforeState is retained"),
		MatchPlayBeginOrdinaryAttackTests::AreStatesEqual(Result.BeforeState, BeforeState));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackPlayerBSuccessTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.PlayerBDeploysFirstWhenAttacking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackPlayerBSuccessTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(
			MatchPlayBeginOrdinaryAttackTests::MakeState(
				EInitialTurnOrderPlayer::PlayerB),
			6);

	TestTrue(TEXT("PlayerB Begin succeeds"), Result.bSuccess);
	TestEqual(TEXT("PlayerB is first legal deployment side"),
		Result.AfterState.CurrentAttack.CurrentLegalDeploymentSide,
		EInitialTurnOrderPlayer::PlayerB);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackSequenceTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.SequenceUsesBothUsedCounts",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackSequenceTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(
			MatchPlayBeginOrdinaryAttackTests::MakeState(
				EInitialTurnOrderPlayer::PlayerA,
				4,
				2,
				3,
				1),
			4);

	TestTrue(TEXT("Begin succeeds"), Result.bSuccess);
	TestEqual(TEXT("Sequence is A used plus B used plus one"),
		Result.AfterState.CurrentAttack.AttackSequence,
		int64{ 4 });
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackMinimumActionPointTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ActionPointTwoAccepted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackMinimumActionPointTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(
			MatchPlayBeginOrdinaryAttackTests::MakeState(),
			2);
	TestTrue(TEXT("ActionPoint 2 succeeds"), Result.bSuccess);
	TestEqual(TEXT("ActionPoint 2 is stored"), Result.AfterState.CurrentAttack.ActionPoint, 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackMaximumActionPointTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ActionPointEightAccepted",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackMaximumActionPointTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(
			MatchPlayBeginOrdinaryAttackTests::MakeState(),
			8);
	TestTrue(TEXT("ActionPoint 8 succeeds"), Result.bSuccess);
	TestEqual(TEXT("ActionPoint 8 is stored"), Result.AfterState.CurrentAttack.ActionPoint, 8);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackBelowRangeTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ActionPointOneRejectedAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackBelowRangeTest::RunTest(
	const FString& Parameters)
{
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("ActionPoint 1"),
		MatchPlayBeginOrdinaryAttackTests::MakeState(),
		1,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidActionPoint);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackAboveRangeTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ActionPointNineRejectedAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackAboveRangeTest::RunTest(
	const FString& Parameters)
{
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("ActionPoint 9"),
		MatchPlayBeginOrdinaryAttackTests::MakeState(),
		9,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidActionPoint);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackAlreadyActiveTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ActiveAttackRejectedAtomically",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackAlreadyActiveTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayBeginOrdinaryAttackTests::MakeState();
	State.bHasCurrentAttack = true;
	State.CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Resolution;
	State.CurrentAttack.AttackSequence = 77;
	State.CurrentAttack.ActionPoint = 8;
	State.CurrentAttack.CurrentLegalDeploymentSide = EInitialTurnOrderPlayer::PlayerB;
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = EInitialTurnOrderPlayer::PlayerA;
	Placement.CardId = TEXT("ExistingCard");
	Placement.SlotId = TEXT("ExistingSlot");
	State.CurrentAttack.DeploymentPlacements.Add(Placement);
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Active attack"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode::CurrentAttackAlreadyActive);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackUninitializedTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.UninitializedStateRejectedFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackUninitializedTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayBeginOrdinaryAttackTests::MakeState();
	State.RuntimeState.bIsInitialized = false;
	State.bHasCurrentAttack = true;
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Uninitialized state"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode::MatchPlayStateNotInitialized);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackInvalidPlayerACountTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.InvalidPlayerACountRejectedFirst",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackInvalidPlayerACountTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayBeginOrdinaryAttackTests::MakeState();
	State.RuntimeState.PlayerAState.UsedAttackCount = -1;
	State.RuntimeState.PlayerBState.UsedAttackCount = -1;
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Invalid PlayerA counts"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidPlayerAAttackCountState);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackInvalidPlayerBCountTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.InvalidPlayerBCountRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackInvalidPlayerBCountTest::RunTest(
	const FString& Parameters)
{
	FMatchPlayState State = MatchPlayBeginOrdinaryAttackTests::MakeState();
	State.RuntimeState.PlayerBState.UsedAttackCount =
		State.RuntimeState.PlayerBState.TotalAttackCount + 1;
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Invalid PlayerB counts"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidPlayerBAttackCountState);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackEndedTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.ExhaustedMatchRejectedBeforeAttacker",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackEndedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayBeginOrdinaryAttackTests::MakeState(
			EInitialTurnOrderPlayer::None,
			2,
			2,
			3,
			3);
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Exhausted match"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode::MatchAlreadyEnded);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackCurrentAttackerExhaustedTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.CurrentAttackerWithoutOpportunityRejected",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackCurrentAttackerExhaustedTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayBeginOrdinaryAttackTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			2,
			2,
			3,
			1);
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Exhausted current attacker"),
		State,
		5,
		EMatchPlayBeginOrdinaryAttackErrorCode
			::CurrentAttackerHasNoRemainingAttackOpportunity);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackInvalidAttackerTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.InvalidAttackerRejectedBeforeActionPoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackInvalidAttackerTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State =
		MatchPlayBeginOrdinaryAttackTests::MakeState(
			EInitialTurnOrderPlayer::None);
	return MatchPlayBeginOrdinaryAttackTests::TestAtomicFailure(
		*this,
		TEXT("Invalid current attacker"),
		State,
		1,
		EMatchPlayBeginOrdinaryAttackErrorCode::InvalidCurrentAttackingPlayer);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackPreservesAuthoritiesTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.SuccessPreservesRuntimeAndCardUsage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackPreservesAuthoritiesTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayBeginOrdinaryAttackTests::MakeState(
			EInitialTurnOrderPlayer::PlayerA,
			4,
			1,
			3,
			2);
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, 7);

	TestTrue(TEXT("Begin succeeds"), Result.bSuccess);
	TestTrue(TEXT("Runtime is field-for-field preserved"),
		FMatchRuntimeState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.RuntimeState,
			&BeforeState.RuntimeState,
			0));
	TestTrue(TEXT("Card usage is field-for-field preserved"),
		FMatchCardUsageState::StaticStruct()->CompareScriptStruct(
			&Result.AfterState.CardUsageState,
			&BeforeState.CardUsageState,
			0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackDeterministicTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.RepeatedEvaluationIsDeterministic",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackDeterministicTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayBeginOrdinaryAttackTests::MakeState(
			EInitialTurnOrderPlayer::PlayerB,
			4,
			1,
			4,
			2);
	const FMatchPlayBeginOrdinaryAttackResult First =
		FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, 6);
	const FMatchPlayBeginOrdinaryAttackResult Second =
		FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, 6);

	TestTrue(TEXT("Repeated Result is field-for-field identical"),
		FMatchPlayBeginOrdinaryAttackResult::StaticStruct()
			->CompareScriptStruct(&First, &Second, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayBeginOrdinaryAttackPreservesCatalogTest,
	"FMCodex.CoreRules.MatchPlayBeginOrdinaryAttack.SuccessPreservesDeploymentSlotCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayBeginOrdinaryAttackPreservesCatalogTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState BeforeState =
		MatchPlayBeginOrdinaryAttackTests::MakeState();
	const FMatchPlayBeginOrdinaryAttackResult Result =
		FMatchPlayBeginOrdinaryAttack::Begin(BeforeState, 5);

	TestTrue(TEXT("Begin succeeds"), Result.bSuccess);
	TestTrue(
		TEXT("Begin preserves the deployment slot catalog"),
		FMatchPlayDeploymentSlotCatalog::StaticStruct()
			->CompareScriptStruct(
				&Result.AfterState.DeploymentSlotCatalog,
				&BeforeState.DeploymentSlotCatalog,
				0));
	return true;
}

#endif
