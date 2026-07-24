#include "MatchPlayState.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayStateTests
{
	const FName PlayerACard1(TEXT("PlayerACard1"));
	const FName PlayerACard2(TEXT("PlayerACard2"));
	const FName PlayerAUsedCard(TEXT("PlayerAUsedCard"));
	const FName PlayerBCard1(TEXT("PlayerBCard1"));
	const FName PlayerBUsedCard(TEXT("PlayerBUsedCard"));

	FMatchRuntimeState MakeRuntimeState()
	{
		FMatchRuntimeState State;
		State.bIsInitialized = true;
		State.PlayerAState.TotalAttackCount = 4;
		State.PlayerAState.UsedAttackCount = 1;
		State.PlayerAState.Score = 2;
		State.PlayerAState.GoalkeeperCardId = TEXT("PlayerAGoalkeeper");
		State.PlayerAState.InitialDeckRarityScore = 11;
		State.PlayerBState.TotalAttackCount = 3;
		State.PlayerBState.UsedAttackCount = 2;
		State.PlayerBState.Score = 1;
		State.PlayerBState.GoalkeeperCardId = TEXT("PlayerBGoalkeeper");
		State.PlayerBState.InitialDeckRarityScore = 9;
		State.FirstPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.SecondPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.CurrentAttackingPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.OpeningResultSnapshot.bSuccess = true;
		State.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount = 4;
		State.OpeningResultSnapshot.AttackCountResult.PlayerBAttackCount = 3;
		return State;
	}

	FMatchCardUsageState MakeCardUsageState()
	{
		FMatchCardUsageState State;
		State.PlayerACardUsageState.AvailableCardIds =
			{ PlayerACard1, PlayerACard2 };
		State.PlayerACardUsageState.UsedCardIds =
			{ PlayerAUsedCard };
		State.PlayerBCardUsageState.AvailableCardIds =
			{ PlayerBCard1 };
		State.PlayerBCardUsageState.UsedCardIds =
			{ PlayerBUsedCard };
		return State;
	}

	FMatchPlayDeploymentSlotCatalog MakeDeploymentSlotCatalog()
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = TEXT("SharedSlotA");
		Slot.NeutralSide = EMatchPlayNeutralSlotSide::NearPlayerA;

		FMatchPlayDeploymentSlotCatalog Catalog;
		Catalog.Slots.Add(Slot);
		return Catalog;
	}

	FMatchPlayPerSideCardSnapshotAuthority MakeCardSnapshotAuthority()
	{
		FPlayerCardRuleSnapshot PlayerACard;
		PlayerACard.CardId = PlayerACard1;
		PlayerACard.PositionTypes = { EPlayerPositionType::Attack };

		FPlayerCardRuleSnapshot PlayerBCard;
		PlayerBCard.CardId = PlayerBCard1;
		PlayerBCard.PositionTypes = { EPlayerPositionType::Defense };

		FMatchPlayPerSideCardSnapshotAuthority Authority;
		Authority.PlayerACardSnapshots.Cards.Add(PlayerACard);
		Authority.PlayerBCardSnapshots.Cards.Add(PlayerBCard);
		return Authority;
	}

	FMatchPlayState MakeState(
		const FMatchRuntimeState& RuntimeState,
		const FMatchCardUsageState& CardUsageState)
	{
		FMatchPlayState State;
		State.RuntimeState = RuntimeState;
		State.CardUsageState = CardUsageState;
		State.DeploymentSlotCatalog = MakeDeploymentSlotCatalog();
		State.CardSnapshotAuthority = MakeCardSnapshotAuthority();
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateCompositionTest,
	"FMCodex.CoreRules.MatchPlayState.Composition",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateCompositionTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateTests::MakeRuntimeState();
	const FMatchCardUsageState CardUsageState =
		MatchPlayStateTests::MakeCardUsageState();

	const FMatchPlayState MatchPlayState =
		MatchPlayStateTests::MakeState(RuntimeState, CardUsageState);

	TestTrue(
		TEXT("Runtime initialization flag is preserved"),
		MatchPlayState.RuntimeState.bIsInitialized);
	TestEqual(
		TEXT("PlayerA score is preserved"),
		MatchPlayState.RuntimeState.PlayerAState.Score,
		RuntimeState.PlayerAState.Score);
	TestEqual(
		TEXT("PlayerB score is preserved"),
		MatchPlayState.RuntimeState.PlayerBState.Score,
		RuntimeState.PlayerBState.Score);
	TestEqual(
		TEXT("PlayerA total attacks are preserved"),
		MatchPlayState.RuntimeState.PlayerAState.TotalAttackCount,
		RuntimeState.PlayerAState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerA used attacks are preserved"),
		MatchPlayState.RuntimeState.PlayerAState.UsedAttackCount,
		RuntimeState.PlayerAState.UsedAttackCount);
	TestEqual(
		TEXT("PlayerB total attacks are preserved"),
		MatchPlayState.RuntimeState.PlayerBState.TotalAttackCount,
		RuntimeState.PlayerBState.TotalAttackCount);
	TestEqual(
		TEXT("PlayerB used attacks are preserved"),
		MatchPlayState.RuntimeState.PlayerBState.UsedAttackCount,
		RuntimeState.PlayerBState.UsedAttackCount);
	TestEqual(
		TEXT("PlayerA goalkeeper is preserved"),
		MatchPlayState.RuntimeState.PlayerAState.GoalkeeperCardId,
		RuntimeState.PlayerAState.GoalkeeperCardId);
	TestEqual(
		TEXT("PlayerB rarity score is preserved"),
		MatchPlayState.RuntimeState.PlayerBState.InitialDeckRarityScore,
		RuntimeState.PlayerBState.InitialDeckRarityScore);
	TestEqual(
		TEXT("First player is preserved"),
		MatchPlayState.RuntimeState.FirstPlayer,
		RuntimeState.FirstPlayer);
	TestEqual(
		TEXT("Second player is preserved"),
		MatchPlayState.RuntimeState.SecondPlayer,
		RuntimeState.SecondPlayer);
	TestEqual(
		TEXT("Current attacking player is preserved"),
		MatchPlayState.RuntimeState.CurrentAttackingPlayer,
		RuntimeState.CurrentAttackingPlayer);
	TestEqual(
		TEXT("Opening PlayerA attacks are preserved"),
		MatchPlayState.RuntimeState.OpeningResultSnapshot
			.AttackCountResult.PlayerAAttackCount,
		RuntimeState.OpeningResultSnapshot
			.AttackCountResult.PlayerAAttackCount);
	TestEqual(
		TEXT("Opening PlayerB attacks are preserved"),
		MatchPlayState.RuntimeState.OpeningResultSnapshot
			.AttackCountResult.PlayerBAttackCount,
		RuntimeState.OpeningResultSnapshot
			.AttackCountResult.PlayerBAttackCount);
	TestTrue(
		TEXT("PlayerA available cards are preserved"),
		MatchPlayState.CardUsageState.PlayerACardUsageState.AvailableCardIds
			== CardUsageState.PlayerACardUsageState.AvailableCardIds);
	TestTrue(
		TEXT("PlayerA used cards are preserved"),
		MatchPlayState.CardUsageState.PlayerACardUsageState.UsedCardIds
			== CardUsageState.PlayerACardUsageState.UsedCardIds);
	TestTrue(
		TEXT("PlayerB available cards are preserved"),
		MatchPlayState.CardUsageState.PlayerBCardUsageState.AvailableCardIds
			== CardUsageState.PlayerBCardUsageState.AvailableCardIds);
	TestTrue(
		TEXT("PlayerB used cards are preserved"),
		MatchPlayState.CardUsageState.PlayerBCardUsageState.UsedCardIds
			== CardUsageState.PlayerBCardUsageState.UsedCardIds);
	TestEqual(
		TEXT("Deployment slot catalog is present"),
		MatchPlayState.DeploymentSlotCatalog.Slots.Num(),
		1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateRuntimeInputTest,
	"FMCodex.CoreRules.MatchPlayState.RuntimeInputUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateRuntimeInputTest::RunTest(const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateTests::MakeRuntimeState();
	const FMatchCardUsageState CardUsageState =
		MatchPlayStateTests::MakeCardUsageState();
	FMatchPlayState MatchPlayState =
		MatchPlayStateTests::MakeState(RuntimeState, CardUsageState);

	MatchPlayState.RuntimeState.PlayerAState.Score = 99;
	MatchPlayState.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;

	TestEqual(
		TEXT("Input PlayerA score is unchanged"),
		RuntimeState.PlayerAState.Score,
		2);
	TestEqual(
		TEXT("Input current player is unchanged"),
		RuntimeState.CurrentAttackingPlayer,
		EInitialTurnOrderPlayer::PlayerA);
	TestEqual(
		TEXT("Input opening snapshot is unchanged"),
		RuntimeState.OpeningResultSnapshot.AttackCountResult.PlayerAAttackCount,
		4);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateCardUsageInputTest,
	"FMCodex.CoreRules.MatchPlayState.CardUsageInputUnchanged",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateCardUsageInputTest::RunTest(
	const FString& Parameters)
{
	const FMatchRuntimeState RuntimeState =
		MatchPlayStateTests::MakeRuntimeState();
	const FMatchCardUsageState CardUsageState =
		MatchPlayStateTests::MakeCardUsageState();
	const TArray<FName> PlayerAAvailableSnapshot =
		CardUsageState.PlayerACardUsageState.AvailableCardIds;
	const TArray<FName> PlayerBUsedSnapshot =
		CardUsageState.PlayerBCardUsageState.UsedCardIds;
	FMatchPlayState MatchPlayState =
		MatchPlayStateTests::MakeState(RuntimeState, CardUsageState);

	MatchPlayState.CardUsageState.PlayerACardUsageState
		.AvailableCardIds.Reset();
	MatchPlayState.CardUsageState.PlayerBCardUsageState
		.UsedCardIds.Add(TEXT("AnotherUsedCard"));

	TestTrue(
		TEXT("Input PlayerA available cards are unchanged"),
		CardUsageState.PlayerACardUsageState.AvailableCardIds
			== PlayerAAvailableSnapshot);
	TestTrue(
		TEXT("Input PlayerB used cards are unchanged"),
		CardUsageState.PlayerBCardUsageState.UsedCardIds
			== PlayerBUsedSnapshot);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateDefaultCurrentAttackTest,
	"FMCodex.CoreRules.MatchPlayState.DefaultHasCanonicalInactiveCurrentAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateDefaultCurrentAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State;
	const FMatchPlayCurrentAttackState DefaultPayload;

	TestFalse(TEXT("Default state has no current attack"), State.bHasCurrentAttack);
	TestTrue(TEXT("Default inactive payload is canonical"),
		FMatchPlayCurrentAttackState::StaticStruct()->CompareScriptStruct(
			&State.CurrentAttack,
			&DefaultPayload,
			0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateCreateCurrentAttackTest,
	"FMCodex.CoreRules.MatchPlayState.CreateHasCanonicalInactiveCurrentAttack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateCreateCurrentAttackTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State = MatchPlayStateTests::MakeState(
		MatchPlayStateTests::MakeRuntimeState(),
		MatchPlayStateTests::MakeCardUsageState());
	const FMatchPlayCurrentAttackState DefaultPayload;

	TestFalse(TEXT("Created state has no current attack"), State.bHasCurrentAttack);
	TestTrue(TEXT("Created inactive payload is canonical"),
		FMatchPlayCurrentAttackState::StaticStruct()->CompareScriptStruct(
			&State.CurrentAttack,
			&DefaultPayload,
			0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateDefaultCatalogTest,
	"FMCodex.CoreRules.MatchPlayState.DefaultCatalogIsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateDefaultCatalogTest::RunTest(const FString& Parameters)
{
	const FMatchPlayState State;
	TestTrue(
		TEXT("Default match play state has an empty deployment slot catalog"),
		State.DeploymentSlotCatalog.Slots.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateReflectedCatalogComparisonTest,
	"FMCodex.CoreRules.MatchPlayState.ReflectedComparisonIncludesCatalog",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateReflectedCatalogComparisonTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState First = MatchPlayStateTests::MakeState(
		MatchPlayStateTests::MakeRuntimeState(),
		MatchPlayStateTests::MakeCardUsageState());
	FMatchPlayState Copy = First;

	TestTrue(
		TEXT("Default state copy includes the deployment slot catalog"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&First,
			&Copy,
			0));

	Copy.DeploymentSlotCatalog.Slots[0].SlotId = TEXT("DifferentSlot");
	TestFalse(
		TEXT("Reflected whole-state comparison detects a different catalog"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&First,
			&Copy,
			0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateDefaultSnapshotAuthorityTest,
	"FMCodex.CoreRules.MatchPlayState.DefaultSnapshotAuthorityIsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateDefaultSnapshotAuthorityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState State;
	TestTrue(
		TEXT("Default PlayerA snapshot authority is empty"),
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.IsEmpty());
	TestTrue(
		TEXT("Default PlayerB snapshot authority is empty"),
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateReflectedSnapshotAuthorityComparisonTest,
	"FMCodex.CoreRules.MatchPlayState.ReflectedComparisonIncludesSnapshotAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateReflectedSnapshotAuthorityComparisonTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState First = MatchPlayStateTests::MakeState(
		MatchPlayStateTests::MakeRuntimeState(),
		MatchPlayStateTests::MakeCardUsageState());
	FMatchPlayState Copy = First;

	TestTrue(
		TEXT("Reflected copy initially matches"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&First,
			&Copy,
			0));
	Copy.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.Attributes.Shooting = 6;
	TestFalse(
		TEXT("Reflected comparison detects snapshot authority change"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&First,
			&Copy,
			0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayStateGoalkeeperUsageAuthorityTest,
	"FMCodex.CoreRules.MatchPlayState.ReflectedComparisonIncludesGoalkeeperUsageState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayStateGoalkeeperUsageAuthorityTest::RunTest(
	const FString& Parameters)
{
	const FMatchPlayState DefaultState;
	TestFalse(
		TEXT("Default PlayerA goalkeeper usage is false"),
		DefaultState.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed);
	TestFalse(
		TEXT("Default PlayerB goalkeeper usage is false"),
		DefaultState.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed);

	FMatchPlayState Copy = DefaultState;
	Copy.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed = true;
	TestFalse(
		TEXT("Whole-state reflected comparison detects goalkeeper usage"),
		FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&DefaultState,
			&Copy,
			0));
	return true;
}

#endif
