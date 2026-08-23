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

#endif
