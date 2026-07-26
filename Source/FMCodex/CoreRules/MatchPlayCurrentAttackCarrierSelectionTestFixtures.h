#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackCarrierSelectionLegality.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackCarrierSelection
{
	inline constexpr int64 ValidAttackSequence = 11;
	inline const FName CarrierOneId(TEXT("PlayerA.CarrierOne"));
	inline const FName CarrierTwoId(TEXT("PlayerA.CarrierTwo"));
	inline const FName GoalkeeperId(TEXT("PlayerA.Goalkeeper"));
	inline const FName DefenderSameId(TEXT("PlayerA.CarrierOne"));

	inline void SetBaseAttributes(FPlayerAttributes& Attributes)
	{
		Attributes.Shooting = 3;
		Attributes.Dribbling = 3;
		Attributes.Passing = 3;
		Attributes.OffBall = 3;
		Attributes.Marking = 3;
		Attributes.Tackling = 3;
		Attributes.Speed = 3;
		Attributes.Strength = 3;
		Attributes.Stamina = 3;
		Attributes.LongShot = 3;
	}

	inline void SetGoalkeeperAttributes(FGoalkeeperAttributes& Attributes)
	{
		Attributes.Handling = 3;
		Attributes.Positioning = 3;
		Attributes.Reflex = 3;
		Attributes.Aerial = 3;
		Attributes.Anticipation = 3;
		Attributes.OneOnOne = 3;
	}

	inline FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const bool bGoalkeeper = false)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {
			bGoalkeeper
				? EPlayerPositionType::Goalkeeper
				: EPlayerPositionType::Attack
		};
		SetBaseAttributes(Snapshot.Attributes);
		Snapshot.bIsGoalkeeper = bGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bGoalkeeper;
		if (bGoalkeeper)
		{
			SetGoalkeeperAttributes(Snapshot.GoalkeeperAttributes);
		}
		return Snapshot;
	}

	inline FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const TCHAR* SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = CardId;
		Placement.SlotId = FName(SlotId);
		return Placement;
	}

	inline FMatchPlayState MakeState()
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount = 4;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerBState.TotalAttackCount = 4;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.Score = 1;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			CarrierOneId,
			CarrierTwoId,
			GoalkeeperId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			DefenderSameId
		};
		State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(CarrierOneId),
			MakeCard(CarrierTwoId),
			MakeCard(GoalkeeperId, true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(DefenderSameId)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierOneId,
				TEXT("Slot.A1")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				DefenderSameId,
				TEXT("Slot.B1")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierTwoId,
				TEXT("Slot.A2"))
		};
		return State;
	}

	inline FMatchPlayCurrentAttackCarrierSelectionRequest MakeRequest(
		const FName CarrierCardId = CarrierOneId)
	{
		FMatchPlayCurrentAttackCarrierSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerA;
		Request.CarrierCardId = CarrierCardId;
		return Request;
	}

	inline bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}
}

#endif
