#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackMarkerSelectionLegality.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackMarkerSelection
{
	inline constexpr int64 ValidAttackSequence = 17;
	inline const FName CarrierId(TEXT("PlayerA.Carrier"));
	inline const FName MarkerOneId(TEXT("PlayerB.MarkerOne"));
	inline const FName OtherAreaMarkerId(TEXT("PlayerB.OtherArea"));
	inline const FName GoalkeeperMarkerId(TEXT("PlayerB.Goalkeeper"));
	inline const FName MarkerTwoId(TEXT("PlayerB.MarkerTwo"));
	inline const FName MissingSnapshotMarkerId(
		TEXT("PlayerB.MissingSnapshot"));
	inline const FName AttackerOnlyMarkerId(
		TEXT("PlayerB.AttackerOnly"));
	inline const FName CarrierSlotId(TEXT("Physical.NearB.Carrier"));
	inline const FName MarkerOneSlotId(TEXT("Physical.NearB.MarkerOne"));
	inline const FName OtherAreaSlotId(TEXT("Physical.NearA.Other"));
	inline const FName GoalkeeperSlotId(TEXT("Physical.NearB.Goalkeeper"));
	inline const FName MarkerTwoSlotId(TEXT("Physical.NearB.MarkerTwo"));
	inline const FName MissingSnapshotSlotId(
		TEXT("Physical.NearB.MissingSnapshot"));

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
				: EPlayerPositionType::Defense
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

	inline FMatchPlayDeploymentSlotDefinition MakeSlot(
		const FName SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = SlotId;
		Slot.NeutralSide = NeutralSide;
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
			CarrierId,
			AttackerOnlyMarkerId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			MarkerOneId,
			OtherAreaMarkerId,
			GoalkeeperMarkerId,
			MarkerTwoId,
			MissingSnapshotMarkerId
		};
		State.GoalkeeperUsageState.bPlayerBGoalkeeperCardUsed = true;
		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				CarrierSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				MarkerOneSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				OtherAreaSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				GoalkeeperSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				MarkerTwoSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				MissingSnapshotSlotId,
				EMatchPlayNeutralSlotSide::NearPlayerB)
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(CarrierId),
			MakeCard(AttackerOnlyMarkerId)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(MarkerOneId),
			MakeCard(OtherAreaMarkerId),
			MakeCard(GoalkeeperMarkerId, true),
			MakeCard(MarkerTwoId)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingMarker;
		State.CurrentAttack.ActionPreparation.CarrierCardId =
			CarrierId;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierId,
				CarrierSlotId),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerOneId,
				MarkerOneSlotId),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				AttackerOnlyMarkerId,
				MarkerTwoSlotId),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				OtherAreaMarkerId,
				OtherAreaSlotId),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				GoalkeeperMarkerId,
				GoalkeeperSlotId),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerTwoId,
				MarkerTwoSlotId)
		};
		return State;
	}

	inline FMatchPlayCurrentAttackMarkerSelectionRequest MakeRequest(
		const FName MarkerCardId = MarkerOneId)
	{
		FMatchPlayCurrentAttackMarkerSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = EInitialTurnOrderPlayer::PlayerB;
		Request.MarkerCardId = MarkerCardId;
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
