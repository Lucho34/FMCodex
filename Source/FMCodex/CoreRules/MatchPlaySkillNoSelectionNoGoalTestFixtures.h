#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "MatchPlaySkillNoSelectionNoGoal.h"

namespace FMCodex::Tests::MatchPlaySkillNoSelectionNoGoal
{
	namespace SkillFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;
	namespace MarkerFixtures =
		FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	inline const FName ExtraAttackerCardId(
		TEXT("SkillNoGoal.ExtraAttacker"));
	inline const FName ExtraDefenderCardId(
		TEXT("SkillNoGoal.ExtraDefender"));
	inline const FName GoalkeeperCardId(
		TEXT("SkillNoGoal.Goalkeeper"));

	inline EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	inline FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA,
		const TArray<FName>& CarrierSkillIds = {
			SkillFixtures::LongShotSkillId
		})
	{
		FMatchPlayState State =
			SkillFixtures::MakeState(CarrierSkillIds);
		State.RuntimeState.CurrentAttackingPlayer = Attacker;
		if (Attacker == EInitialTurnOrderPlayer::PlayerB)
		{
			State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
				SkillFixtures::MakeCard(SkillFixtures::MarkerId)
			};
			State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
				SkillFixtures::MakeCard(
					SkillFixtures::CarrierId,
					CarrierSkillIds)
			};
			State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
				SkillFixtures::MarkerId
			};
			State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
				SkillFixtures::CarrierId
			};
			State.CurrentAttack.DeploymentPlacements = {
				SkillFixtures::MakePlacement(
					EInitialTurnOrderPlayer::PlayerB,
					SkillFixtures::CarrierId,
					TEXT("Slot.Attacker")),
				SkillFixtures::MakePlacement(
					EInitialTurnOrderPlayer::PlayerA,
					SkillFixtures::MarkerId,
					TEXT("Slot.Defender"))
			};
		}
		return State;
	}

	inline void AddOrdinaryDeployments(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const EInitialTurnOrderPlayer Defender =
			GetDefender(Attacker);
		FPlayerCardRuleSnapshot AttackerCard =
			SkillFixtures::MakeCard(ExtraAttackerCardId);
		FPlayerCardRuleSnapshot DefenderCard =
			SkillFixtures::MakeCard(ExtraDefenderCardId);
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
				AttackerCard);
			State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
				DefenderCard);
			State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.Add(ExtraAttackerCardId);
			State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.Add(ExtraDefenderCardId);
		}
		else
		{
			State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
				AttackerCard);
			State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
				DefenderCard);
			State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.Add(ExtraAttackerCardId);
			State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.Add(ExtraDefenderCardId);
		}
		State.CurrentAttack.DeploymentPlacements.Add(
			SkillFixtures::MakePlacement(
				Attacker,
				ExtraAttackerCardId,
				TEXT("Slot.ExtraAttacker")));
		State.CurrentAttack.DeploymentPlacements.Add(
			SkillFixtures::MakePlacement(
				Defender,
				ExtraDefenderCardId,
				TEXT("Slot.ExtraDefender")));
	}

	inline void AddActiveGoalkeeper(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Defender =
			GetDefender(
				State.RuntimeState.CurrentAttackingPlayer);
		const FPlayerCardRuleSnapshot Goalkeeper =
			MarkerFixtures::MakeCard(GoalkeeperCardId, true);
		if (Defender == EInitialTurnOrderPlayer::PlayerA)
		{
			State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(
				Goalkeeper);
			State.CardUsageState.PlayerACardUsageState
				.AvailableCardIds.Add(GoalkeeperCardId);
			State.GoalkeeperUsageState
				.bPlayerAGoalkeeperCardUsed = true;
		}
		else
		{
			State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.Add(
				Goalkeeper);
			State.CardUsageState.PlayerBCardUsageState
				.AvailableCardIds.Add(GoalkeeperCardId);
			State.GoalkeeperUsageState
				.bPlayerBGoalkeeperCardUsed = true;
		}
		State.CurrentAttack.DeploymentPlacements.Add(
			SkillFixtures::MakePlacement(
				Defender,
				GoalkeeperCardId,
				TEXT("Slot.Goalkeeper")));
		State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	}

	inline FMatchPlayResolveNoLegalSkillRequest MakeResolveRequest()
	{
		FMatchPlayResolveNoLegalSkillRequest Request;
		Request.AttackSequence =
			SkillFixtures::ValidAttackSequence;
		return Request;
	}

	inline FMatchPlaySkillDeclineRequest MakeDeclineRequest(
		const EInitialTurnOrderPlayer Side =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlaySkillDeclineRequest Request;
		Request.AttackSequence =
			SkillFixtures::ValidAttackSequence;
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
