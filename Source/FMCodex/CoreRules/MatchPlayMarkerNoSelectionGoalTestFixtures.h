#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayMarkerNoSelectionGoal.h"

namespace FMCodex::Tests::MatchPlayMarkerNoSelectionGoal
{
	inline constexpr int64 ValidAttackSequence = 3;
	inline const FName PlayerAOne(TEXT("PlayerA.One"));
	inline const FName PlayerATwo(TEXT("PlayerA.Two"));
	inline const FName PlayerAGoalkeeper(TEXT("PlayerA.Goalkeeper"));
	inline const FName PlayerBOne(TEXT("PlayerB.One"));
	inline const FName PlayerBTwo(TEXT("PlayerB.Two"));
	inline const FName PlayerBGoalkeeper(TEXT("PlayerB.Goalkeeper"));
	inline const FName NearPlayerASlotOne(TEXT("NearA.One"));
	inline const FName NearPlayerASlotTwo(TEXT("NearA.Two"));
	inline const FName NearPlayerASlotThree(TEXT("NearA.Three"));
	inline const FName NearPlayerASlotGoalkeeper(TEXT("NearA.Goalkeeper"));
	inline const FName NearPlayerBSlotOne(TEXT("NearB.One"));
	inline const FName NearPlayerBSlotTwo(TEXT("NearB.Two"));
	inline const FName NearPlayerBSlotThree(TEXT("NearB.Three"));
	inline const FName NearPlayerBSlotGoalkeeper(TEXT("NearB.Goalkeeper"));

	inline bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	inline EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	inline void SetAttributes(FPlayerAttributes& Attributes)
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

	inline void SetGoalkeeperAttributes(
		FGoalkeeperAttributes& Attributes)
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
				: EPlayerPositionType::Defense
		};
		SetAttributes(Snapshot.Attributes);
		Snapshot.bIsGoalkeeper = bGoalkeeper;
		Snapshot.bHasGoalkeeperAttributes = bGoalkeeper;
		if (bGoalkeeper)
		{
			SetGoalkeeperAttributes(Snapshot.GoalkeeperAttributes);
		}
		return Snapshot;
	}

	inline FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide Side)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = Side;
		return Slot;
	}

	inline FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer PlayerSide,
		const FName CardId,
		const FName SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = PlayerSide;
		Placement.CardId = CardId;
		Placement.SlotId = SlotId;
		return Placement;
	}

	inline FName GetOneCard(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? PlayerAOne
			: PlayerBOne;
	}

	inline FName GetTwoCard(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? PlayerATwo
			: PlayerBTwo;
	}

	inline FName GetGoalkeeperCard(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? PlayerAGoalkeeper
			: PlayerBGoalkeeper;
	}

	inline FName GetMatchingSlotOne(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? NearPlayerBSlotOne
			: NearPlayerASlotOne;
	}

	inline FName GetMatchingSlotTwo(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? NearPlayerBSlotTwo
			: NearPlayerASlotTwo;
	}

	inline FName GetMatchingSlotThree(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? NearPlayerBSlotThree
			: NearPlayerASlotThree;
	}

	inline FName GetOtherAreaSlot(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? NearPlayerASlotOne
			: NearPlayerBSlotOne;
	}

	inline FName GetGoalkeeperSlot(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? NearPlayerBSlotGoalkeeper
			: NearPlayerASlotGoalkeeper;
	}

	inline FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		const EInitialTurnOrderPlayer Defender = GetDefender(Attacker);
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.FirstPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer =
			EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer = Attacker;
		State.RuntimeState.PlayerAState.TotalAttackCount = 3;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.Score = 1;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			PlayerAOne,
			PlayerATwo,
			PlayerAGoalkeeper
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			PlayerBOne,
			PlayerBTwo,
			PlayerBGoalkeeper
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(PlayerAOne),
			MakeCard(PlayerATwo),
			MakeCard(PlayerAGoalkeeper, true)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(PlayerBOne),
			MakeCard(PlayerBTwo),
			MakeCard(PlayerBGoalkeeper, true)
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				NearPlayerASlotOne,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerASlotTwo,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerASlotThree,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerASlotGoalkeeper,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				NearPlayerBSlotOne,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				NearPlayerBSlotTwo,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				NearPlayerBSlotThree,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				NearPlayerBSlotGoalkeeper,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
		State.CurrentAttack.ActionPreparation.CarrierCardId =
			GetOneCard(Attacker);
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				Attacker,
				GetOneCard(Attacker),
				GetMatchingSlotOne(Attacker)),
			MakePlacement(
				Attacker,
				GetTwoCard(Attacker),
				GetMatchingSlotTwo(Attacker)),
			MakePlacement(
				Defender,
				GetOneCard(Defender),
				GetMatchingSlotThree(Attacker))
		};
		return State;
	}

	inline void RemoveDefenderPlacements(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Defender =
			GetDefender(State.RuntimeState.CurrentAttackingPlayer);
		State.CurrentAttack.DeploymentPlacements.RemoveAll(
			[Defender](const FMatchPlayDeploymentPlacement& Placement)
			{
				return Placement.PlayerSide == Defender;
			});
	}

	inline void MakeOnlyOtherAreaDefender(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender =
			GetDefender(Attacker);
		RemoveDefenderPlacements(State);
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				Defender,
				GetOneCard(Defender),
				GetOtherAreaSlot(Attacker)));
	}

	inline void MakeOnlyGoalkeeperDefender(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender =
			GetDefender(Attacker);
		RemoveDefenderPlacements(State);
		State.CurrentAttack.DeploymentPlacements.Add(
			MakePlacement(
				Defender,
				GetGoalkeeperCard(Defender),
				GetGoalkeeperSlot(Attacker)));
		State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
		if (Defender == EInitialTurnOrderPlayer::PlayerA)
		{
			State.GoalkeeperUsageState.bPlayerAGoalkeeperCardUsed = true;
		}
		else
		{
			State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
		}
	}

	inline FMatchPlayResolveNoLegalMarkerRequest MakeNoLegalRequest()
	{
		FMatchPlayResolveNoLegalMarkerRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		return Request;
	}

	inline FMatchPlayMarkerDeclineRequest MakeDeclineRequest(
		const EInitialTurnOrderPlayer Side =
			EInitialTurnOrderPlayer::PlayerB)
	{
		FMatchPlayMarkerDeclineRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = Side;
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
