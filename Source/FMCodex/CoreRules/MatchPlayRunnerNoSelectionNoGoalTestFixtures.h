#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionTestFixtures.h"
#include "MatchPlayMarkerNoSelectionGoalTestFixtures.h"
#include "MatchPlayRunnerNoSelectionNoGoal.h"

namespace FMCodex::Tests::MatchPlayRunnerNoSelectionNoGoal
{
	namespace RunnerFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection;
	namespace MarkerFixtures =
		FMCodex::Tests::MatchPlayMarkerNoSelectionGoal;

	inline const FName LegalRunnerOneId(
		TEXT("RunnerNoGoal.LegalOne"));
	inline const FName LegalRunnerTwoId(
		TEXT("RunnerNoGoal.LegalTwo"));
	inline const FName IllegalRunnerOneId(
		TEXT("RunnerNoGoal.IllegalOne"));
	inline const FName IllegalRunnerTwoId(
		TEXT("RunnerNoGoal.IllegalTwo"));
	inline const FName ExtraAttackerCardId(
		TEXT("RunnerNoGoal.ExtraAttacker"));
	inline const FName ExtraDefenderCardId(
		TEXT("RunnerNoGoal.ExtraDefender"));
	inline const FName GoalkeeperCardId(
		TEXT("RunnerNoGoal.Goalkeeper"));
	inline const FName SharedCardId(
		TEXT("RunnerNoGoal.Shared"));

	inline EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	inline FPlayerCardRuleSnapshotSet& GetSnapshots(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	}

	inline FCardUsageState& GetCardUsage(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.CardUsageState.PlayerACardUsageState
			: State.CardUsageState.PlayerBCardUsageState;
	}

	inline EMatchPlayNeutralSlotSide GetNearSide(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EMatchPlayNeutralSlotSide::NearPlayerA
			: EMatchPlayNeutralSlotSide::NearPlayerB;
	}

	inline EMatchPlayNeutralSlotSide GetFarSide(
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? EMatchPlayNeutralSlotSide::NearPlayerB
			: EMatchPlayNeutralSlotSide::NearPlayerA;
	}

	inline FName MakeSlotId(const FName CardId)
	{
		return FName(
			*FString::Printf(
				TEXT("Slot.%s"),
				*CardId.ToString()));
	}

	inline FMatchPlayState MakeState(
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA,
		const ESkillRuleType ActionType =
			ESkillRuleType::PassControl)
	{
		const EInitialTurnOrderPlayer Defender =
			GetDefender(Attacker);
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
		State.bHasCurrentAttack = true;

		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence =
			RunnerFixtures::ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.CurrentAttack.ActionPreparation.CarrierCardId =
			RunnerFixtures::CarrierId;
		State.CurrentAttack.ActionPreparation.MarkerCardId =
			RunnerFixtures::MarkerId;
		State.CurrentAttack.ActionPreparation.SkillId =
			RunnerFixtures::GetSkillId(ActionType);
		State.CurrentAttack.ActionPreparation.ActionType =
			ActionType;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			RunnerFixtures::MakePlacement(
				Attacker,
				RunnerFixtures::CarrierId,
				TEXT("Slot.RunnerCarrier")),
			RunnerFixtures::MakePlacement(
				Defender,
				RunnerFixtures::MarkerId,
				TEXT("Slot.RunnerMarker"))
		};

		GetSnapshots(State, Attacker).Cards = {
			RunnerFixtures::MakeCard(
				RunnerFixtures::CarrierId,
				{EPlayerPositionType::Midfield})
		};
		GetSnapshots(State, Defender).Cards = {
			RunnerFixtures::MakeCard(
				RunnerFixtures::MarkerId,
				{EPlayerPositionType::Defense})
		};
		GetCardUsage(State, Attacker).AvailableCardIds = {
			RunnerFixtures::CarrierId
		};
		GetCardUsage(State, Defender).AvailableCardIds = {
			RunnerFixtures::MarkerId
		};
		State.DeploymentSlotCatalog.Slots = {
			RunnerFixtures::MakeSlot(
				TEXT("Slot.RunnerCarrier"),
				GetNearSide(Attacker)),
			RunnerFixtures::MakeSlot(
				TEXT("Slot.RunnerMarker"),
				GetNearSide(Defender))
		};
		return State;
	}

	inline void AddOrdinaryDeployment(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const TArray<EPlayerPositionType>& PositionTypes,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		const FName SlotId = MakeSlotId(CardId);
		GetSnapshots(State, Side).Cards.Add(
			RunnerFixtures::MakeCard(CardId, PositionTypes));
		GetCardUsage(State, Side).AvailableCardIds.Add(CardId);
		State.CurrentAttack.DeploymentPlacements.Add(
			RunnerFixtures::MakePlacement(
				Side,
				CardId,
				*SlotId.ToString()));
		State.DeploymentSlotCatalog.Slots.Add(
			RunnerFixtures::MakeSlot(
				*SlotId.ToString(),
				NeutralSide));
	}

	inline void AddLegalRunner(
		FMatchPlayState& State,
		const FName CardId = LegalRunnerOneId)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const ESkillRuleType ActionType =
			State.CurrentAttack.ActionPreparation.ActionType;
		TArray<EPlayerPositionType> PositionTypes = {
			EPlayerPositionType::Defense
		};
		EMatchPlayNeutralSlotSide NeutralSide =
			GetNearSide(Attacker);
		if (ActionType == ESkillRuleType::PassControl)
		{
			PositionTypes = {EPlayerPositionType::Midfield};
		}
		else if (ActionType == ESkillRuleType::Cross)
		{
			PositionTypes = {EPlayerPositionType::Attack};
		}
		else if (ActionType == ESkillRuleType::ThroughBall)
		{
			NeutralSide = GetFarSide(Attacker);
		}
		AddOrdinaryDeployment(
			State,
			Attacker,
			CardId,
			PositionTypes,
			NeutralSide);
	}

	inline void AddIllegalRunner(
		FMatchPlayState& State,
		const FName CardId = IllegalRunnerOneId)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const ESkillRuleType ActionType =
			State.CurrentAttack.ActionPreparation.ActionType;
		const TArray<EPlayerPositionType> PositionTypes =
			ActionType == ESkillRuleType::Cross
				? TArray<EPlayerPositionType>{
					EPlayerPositionType::Midfield}
				: TArray<EPlayerPositionType>{
					EPlayerPositionType::Defense};
		AddOrdinaryDeployment(
			State,
			Attacker,
			CardId,
			PositionTypes,
			GetNearSide(Attacker));
	}

	inline void AddActiveGoalkeeper(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Defender =
			GetDefender(
				State.RuntimeState.CurrentAttackingPlayer);
		const FName SlotId = MakeSlotId(GoalkeeperCardId);
		GetSnapshots(State, Defender).Cards.Add(
			MarkerFixtures::MakeCard(GoalkeeperCardId, true));
		GetCardUsage(State, Defender).AvailableCardIds.Add(
			GoalkeeperCardId);
		State.CurrentAttack.DeploymentPlacements.Add(
			RunnerFixtures::MakePlacement(
				Defender,
				GoalkeeperCardId,
				*SlotId.ToString()));
		State.DeploymentSlotCatalog.Slots.Add(
			RunnerFixtures::MakeSlot(
				*SlotId.ToString(),
				GetNearSide(Defender)));
		State.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
		if (Defender == EInitialTurnOrderPlayer::PlayerA)
		{
			State.GoalkeeperUsageState
				.bPlayerAGoalkeeperCardUsed = true;
		}
		else
		{
			State.GoalkeeperUsageState
				.bPlayerBGoalkeeperCardUsed = true;
		}
	}

	inline void AddAttackerGoalkeeper(FMatchPlayState& State)
	{
		const EInitialTurnOrderPlayer Attacker =
			State.RuntimeState.CurrentAttackingPlayer;
		const FName SlotId = MakeSlotId(GoalkeeperCardId);
		GetSnapshots(State, Attacker).Cards.Add(
			MarkerFixtures::MakeCard(GoalkeeperCardId, true));
		GetCardUsage(State, Attacker).AvailableCardIds.Add(
			GoalkeeperCardId);
		State.CurrentAttack.DeploymentPlacements.Add(
			RunnerFixtures::MakePlacement(
				Attacker,
				GoalkeeperCardId,
				*SlotId.ToString()));
		State.DeploymentSlotCatalog.Slots.Add(
			RunnerFixtures::MakeSlot(
				*SlotId.ToString(),
				GetNearSide(Attacker)));
	}

	inline FMatchPlayResolveNoLegalRunnerRequest MakeResolveRequest()
	{
		FMatchPlayResolveNoLegalRunnerRequest Request;
		Request.AttackSequence =
			RunnerFixtures::ValidAttackSequence;
		return Request;
	}

	inline FMatchPlayRunnerDeclineRequest MakeDeclineRequest(
		const EInitialTurnOrderPlayer Side =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayRunnerDeclineRequest Request;
		Request.AttackSequence =
			RunnerFixtures::ValidAttackSequence;
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
