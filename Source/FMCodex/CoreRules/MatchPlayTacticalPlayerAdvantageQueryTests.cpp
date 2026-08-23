#include "MatchPlayTacticalPlayerAdvantageQuery.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace MatchPlayTacticalPlayerAdvantageQueryTests
{
	FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const EPlayerPositionType Position)
	{
		FPlayerCardRuleSnapshot Card;
		Card.CardId = CardId;
		Card.PositionTypes = { Position };
		return Card;
	}

	void AddCard(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const EPlayerPositionType Position,
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FPlayerCardRuleSnapshotSet& Cards =
			Side == EInitialTurnOrderPlayer::PlayerA
				? State.CardSnapshotAuthority.PlayerACardSnapshots
				: State.CardSnapshotAuthority.PlayerBCardSnapshots;
		Cards.Cards.Add(MakeCard(CardId, Position));
		State.DeploymentSlotCatalog.Slots.Add({ SlotId, NeutralSide });
		State.CurrentAttack.DeploymentPlacements.Add({ Side, CardId, SlotId });
	}

	FMatchPlayState MakeState()
	{
		FMatchPlayState State;
		State.bHasCurrentAttack = true;
		State.CurrentAttack.bHasResolutionSession = true;
		State.CurrentAttack.ResolutionSession.Bundle.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.CurrentAttack.ResolutionSession.Bundle.CurrentDefendingPlayer =
			EInitialTurnOrderPlayer::PlayerB;
		return State;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTacticalPlayerAdvantageQueryTest,
	"FMCodex.CoreRules.MatchPlay.TacticalPlayer.AdvantageQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTacticalPlayerAdvantageQueryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayTacticalPlayerAdvantageQueryTests;

	TestEqual(TEXT("Zero or one advantage has no modifier"),
		FMatchPlayTacticalPlayerAdvantageQuery::ModifierForCountAdvantage(1),
		0.0f);
	TestEqual(TEXT("Two-player advantage is +1"),
		FMatchPlayTacticalPlayerAdvantageQuery::ModifierForCountAdvantage(2),
		1.0f);
	TestEqual(TEXT("Three-player advantage is +2"),
		FMatchPlayTacticalPlayerAdvantageQuery::ModifierForCountAdvantage(3),
		2.0f);
	TestEqual(TEXT("Modifier remains capped at +2"),
		FMatchPlayTacticalPlayerAdvantageQuery::ModifierForCountAdvantage(7),
		2.0f);

	FMatchPlayState State = MakeState();
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.Forward"),
		EPlayerPositionType::Attack, TEXT("Slot.A.Forward"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.Midfield"),
		EPlayerPositionType::Midfield, TEXT("Slot.A.Midfield"),
		EMatchPlayNeutralSlotSide::NearPlayerA);
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.ForwardTwo"),
		EPlayerPositionType::Attack, TEXT("Slot.A.ForwardTwo"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	AddCard(State, EInitialTurnOrderPlayer::PlayerB, TEXT("B.Mismatch"),
		EPlayerPositionType::Attack, TEXT("Slot.B.Backfield"),
		EMatchPlayNeutralSlotSide::NearPlayerB);

	const FMatchPlayTacticalPlayerAdvantageResult Result =
		FMatchPlayTacticalPlayerAdvantageQuery::Evaluate(State);
	TestTrue(TEXT("Canonical deployment classification succeeds"),
		Result.bSuccess);
	TestEqual(TEXT("All three matching attackers are counted"),
		Result.AttackerTacticalPlayerCount, 3);
	TestEqual(TEXT("Mismatched defender is not counted"),
		Result.DefenderTacticalPlayerCount, 0);
	TestEqual(TEXT("Attacker receives canonical +2"),
		Result.AttackerFinishingModifier, 2.0f);
	TestEqual(TEXT("Defender receives no modifier"),
		Result.DefenderFinishingModifier, 0.0f);
	TestEqual(TEXT("Identities remain available for presentation"),
		Result.TacticalPlayers.Num(), 3);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMatchPlayTacticalPlayerBoardStatusQueryTest,
	"FMCodex.CoreRules.MatchPlay.TacticalPlayer.BoardStatusProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMatchPlayTacticalPlayerBoardStatusQueryTest::RunTest(
	const FString& Parameters)
{
	using namespace MatchPlayTacticalPlayerAdvantageQueryTests;

	FMatchPlayState BetweenAttacks;
	BetweenAttacks.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerA;
	const FMatchPlayTacticalPlayerAdvantageResult Zero =
		FMatchPlayTacticalPlayerAdvantageQuery::EvaluateBoardStatus(
			BetweenAttacks);
	TestTrue(TEXT("Board status remains queryable outside a Resolution Session"),
		Zero.bSuccess);
	TestTrue(TEXT("Between attacks projects explicit zero counts"),
		Zero.AttackerTacticalPlayerCount == 0
			&& Zero.DefenderTacticalPlayerCount == 0);

	FMatchPlayState State;
	State.bHasCurrentAttack = true;
	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerA;
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.ForwardOne"),
		EPlayerPositionType::Attack, TEXT("Slot.A.ForwardOne"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.Midfield"),
		EPlayerPositionType::Midfield, TEXT("Slot.A.Midfield"),
		EMatchPlayNeutralSlotSide::NearPlayerA);
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.ForwardTwo"),
		EPlayerPositionType::Attack, TEXT("Slot.A.ForwardTwo"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	AddCard(State, EInitialTurnOrderPlayer::PlayerA, TEXT("A.MultiPosition"),
		EPlayerPositionType::Defense, TEXT("Slot.A.MultiPosition"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Last()
		.PositionTypes.Add(EPlayerPositionType::Attack);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[0]
		.PositionTypes.Add(EPlayerPositionType::Midfield);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[1]
		.PositionTypes.Add(EPlayerPositionType::Defense);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[2]
		.PositionTypes.Add(EPlayerPositionType::Midfield);
	State.CardSnapshotAuthority.PlayerACardSnapshots.Cards[3]
		.PositionTypes.Add(EPlayerPositionType::Midfield);
	AddCard(State, EInitialTurnOrderPlayer::PlayerB, TEXT("B.Midfield"),
		EPlayerPositionType::Midfield, TEXT("Slot.B.Midfield"),
		EMatchPlayNeutralSlotSide::NearPlayerA);
	AddCard(State, EInitialTurnOrderPlayer::PlayerB, TEXT("B.Backfield"),
		EPlayerPositionType::Defense, TEXT("Slot.B.Backfield"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[0]
		.PositionTypes.Add(EPlayerPositionType::Attack);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards[1]
		.PositionTypes.Add(EPlayerPositionType::Midfield);
	AddCard(State, EInitialTurnOrderPlayer::PlayerB, TEXT("B.Goalkeeper"),
		EPlayerPositionType::Goalkeeper, TEXT("Slot.B.Goalkeeper"),
		EMatchPlayNeutralSlotSide::NearPlayerB);
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Last()
		.bIsGoalkeeper = true;
	State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Last()
		.bHasGoalkeeperAttributes = true;
	AddCard(State, EInitialTurnOrderPlayer::PlayerB, TEXT("B.Mismatch"),
		EPlayerPositionType::Attack, TEXT("Slot.B.Mismatch"),
		EMatchPlayNeutralSlotSide::NearPlayerB);

	const FMatchPlayTacticalPlayerAdvantageResult Result =
		FMatchPlayTacticalPlayerAdvantageQuery::EvaluateBoardStatus(State);
	TestTrue(TEXT("Deployment board status succeeds without frozen session"),
		Result.bSuccess);
	TestTrue(TEXT("Current attack identity remains explicit"),
		Result.AttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			&& Result.DefendingPlayer == EInitialTurnOrderPlayer::PlayerB);
	TestEqual(TEXT("Multi-position matching card is included"),
		Result.AttackerTacticalPlayerCount, 4);
	TestEqual(TEXT("Goalkeeper and mismatched cards are excluded"),
		Result.DefenderTacticalPlayerCount, 2);
	TestEqual(TEXT("Raw 4 versus 2 count projects canonical +1 modifier"),
		Result.AttackerFinishingModifier, 1.0f);
	TestEqual(TEXT("Status keeps all six qualifying player identities"),
		Result.TacticalPlayers.Num(), 6);

	State.RuntimeState.CurrentAttackingPlayer =
		EInitialTurnOrderPlayer::PlayerB;
	const FMatchPlayTacticalPlayerAdvantageResult Switched =
		FMatchPlayTacticalPlayerAdvantageQuery::EvaluateBoardStatus(State);
	TestTrue(TEXT("Attack-side switch preserves side identity in relative fields"),
		Switched.bSuccess
			&& Switched.AttackingPlayer == EInitialTurnOrderPlayer::PlayerB
			&& Switched.DefendingPlayer == EInitialTurnOrderPlayer::PlayerA
			&& Switched.AttackerTacticalPlayerCount == 2
			&& Switched.DefenderTacticalPlayerCount == 4);

	const FMatchPlayTacticalPlayerAdvantageResult SessionOnly =
		FMatchPlayTacticalPlayerAdvantageQuery::Evaluate(State);
	TestFalse(TEXT("Formula query keeps its Resolution Session requirement"),
		SessionOnly.bSuccess);
	TestEqual(TEXT("Formula query error contract is unchanged"),
		SessionOnly.ErrorCode,
		EMatchPlayTacticalPlayerAdvantageErrorCode::MissingResolutionSession);
	return true;
}

#endif
