#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackBranchIntentSelectionWriter.h"
#include "MatchPlayCurrentAttackHelperSelectionWriter.h"
#include "MatchPlayHelperAbsence.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackHelperSelection
{
	inline constexpr int64 ValidAttackSequence = 41;
	inline const FName CarrierId(TEXT("Helper.Carrier"));
	inline const FName RunnerId(TEXT("Helper.Runner"));
	inline const FName MarkerId(TEXT("Helper.Marker"));
	inline const FName HelperId(TEXT("Helper.Valid"));
	inline const FName HelperTwoId(TEXT("Helper.ValidTwo"));
	inline const FName GoalkeeperId(TEXT("Helper.Goalkeeper"));
	inline const FName MissingSnapshotId(TEXT("Helper.MissingSnapshot"));
	inline const FName MissingId(TEXT("Helper.NotDeployed"));
	inline const FName PassControlSkillId(TEXT("Skill.Helper.PassControl"));
	inline const FName CrossSkillId(TEXT("Skill.Helper.Cross"));
	inline const FName ThroughBallSkillId(TEXT("Skill.Helper.ThroughBall"));

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

	inline FPlayerCardRuleSnapshot MakeCard(
		const FName CardId,
		const TArray<EPlayerPositionType>& Positions)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = Positions;
		SetBaseAttributes(Snapshot.Attributes);
		return Snapshot;
	}

	inline FPlayerCardRuleSnapshot MakeGoalkeeper(const FName CardId)
	{
		FPlayerCardRuleSnapshot Snapshot =
			MakeCard(CardId, {EPlayerPositionType::Goalkeeper});
		Snapshot.bIsGoalkeeper = true;
		Snapshot.bHasGoalkeeperAttributes = true;
		Snapshot.GoalkeeperAttributes.Handling = 3;
		Snapshot.GoalkeeperAttributes.Positioning = 3;
		Snapshot.GoalkeeperAttributes.Reflex = 3;
		Snapshot.GoalkeeperAttributes.Aerial = 3;
		Snapshot.GoalkeeperAttributes.Anticipation = 3;
		Snapshot.GoalkeeperAttributes.OneOnOne = 3;
		return Snapshot;
	}

	inline FMatchPlayDeploymentPlacement MakePlacement(
		const EInitialTurnOrderPlayer Side,
		const FName CardId,
		const TCHAR* SlotId)
	{
		FMatchPlayDeploymentPlacement Placement;
		Placement.PlayerSide = Side;
		Placement.CardId = CardId;
		Placement.SlotId = FName(SlotId);
		return Placement;
	}

	inline FMatchPlayDeploymentSlotDefinition MakeSlot(
		const TCHAR* SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = FName(SlotId);
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	inline FName GetSkillId(const ESkillRuleType Type)
	{
		if (Type == ESkillRuleType::PassControl)
		{
			return PassControlSkillId;
		}
		if (Type == ESkillRuleType::Cross)
		{
			return CrossSkillId;
		}
		return ThroughBallSkillId;
	}

	inline EInitialTurnOrderPlayer GetDefender(
		const EInitialTurnOrderPlayer Attacker)
	{
		return Attacker == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	inline FMatchPlayState MakeState(
		const ESkillRuleType Type = ESkillRuleType::PassControl,
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		const EInitialTurnOrderPlayer Defender = GetDefender(Attacker);
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.FirstPlayer = EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer = EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer = Attacker;
		State.RuntimeState.PlayerAState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingHelper;
		State.CurrentAttack.ActionPreparation.CarrierCardId = CarrierId;
		State.CurrentAttack.ActionPreparation.MarkerCardId = MarkerId;
		State.CurrentAttack.ActionPreparation.SkillId = GetSkillId(Type);
		State.CurrentAttack.ActionPreparation.ActionType = Type;
		State.CurrentAttack.ActionPreparation.RunnerCardId = RunnerId;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(Attacker, CarrierId, TEXT("Slot.HelperCarrier")),
			MakePlacement(Defender, MarkerId, TEXT("Slot.HelperMarker")),
			MakePlacement(Attacker, RunnerId, TEXT("Slot.HelperRunner")),
			MakePlacement(Defender, HelperId, TEXT("Slot.HelperValid")),
			MakePlacement(Defender, GoalkeeperId, TEXT("Slot.HelperGK")),
			MakePlacement(
				Defender,
				MissingSnapshotId,
				TEXT("Slot.HelperMissing"))
		};

		const EMatchPlayNeutralSlotSide AttackerForward =
			Attacker == EInitialTurnOrderPlayer::PlayerA
				? EMatchPlayNeutralSlotSide::NearPlayerB
				: EMatchPlayNeutralSlotSide::NearPlayerA;
		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				TEXT("Slot.HelperCarrier"),
				Attacker == EInitialTurnOrderPlayer::PlayerA
					? EMatchPlayNeutralSlotSide::NearPlayerA
					: EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				TEXT("Slot.HelperMarker"),
				AttackerForward),
			MakeSlot(TEXT("Slot.HelperRunner"), AttackerForward),
			MakeSlot(TEXT("Slot.HelperValid"), AttackerForward),
			MakeSlot(TEXT("Slot.HelperGK"), AttackerForward),
			MakeSlot(TEXT("Slot.HelperMissing"), AttackerForward)
		};

		FPlayerCardRuleSnapshotSet AttackerSnapshots;
		AttackerSnapshots.Cards = {
			MakeCard(CarrierId, {EPlayerPositionType::Midfield}),
			MakeCard(
				RunnerId,
				{
					EPlayerPositionType::Midfield,
					EPlayerPositionType::Attack
				})
		};
		FPlayerCardRuleSnapshotSet DefenderSnapshots;
		DefenderSnapshots.Cards = {
			MakeCard(MarkerId, {EPlayerPositionType::Defense}),
			MakeCard(HelperId, {EPlayerPositionType::Attack}),
			MakeGoalkeeper(GoalkeeperId)
		};
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			State.CardSnapshotAuthority.PlayerACardSnapshots =
				AttackerSnapshots;
			State.CardSnapshotAuthority.PlayerBCardSnapshots =
				DefenderSnapshots;
		}
		else
		{
			State.CardSnapshotAuthority.PlayerBCardSnapshots =
				AttackerSnapshots;
			State.CardSnapshotAuthority.PlayerACardSnapshots =
				DefenderSnapshots;
		}
		return State;
	}

	inline FMatchPlayCurrentAttackHelperSelectionRequest MakeRequest(
		const FName CardId = HelperId,
		const EInitialTurnOrderPlayer Defender =
			EInitialTurnOrderPlayer::PlayerB)
	{
		FMatchPlayCurrentAttackHelperSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide = Defender;
		Request.HelperCardId = CardId;
		return Request;
	}

	inline FMatchPlayState MakeZeroLegalStateForType(
		const ESkillRuleType Type,
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayState State = MakeState(
			Type,
			Attacker);
		State.CurrentAttack.DeploymentPlacements.RemoveAll(
			[](const FMatchPlayDeploymentPlacement& Placement)
			{
				return Placement.CardId == HelperId;
			});
		if (Attacker == EInitialTurnOrderPlayer::PlayerA)
		{
			State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards.RemoveAll(
				[](const FPlayerCardRuleSnapshot& Snapshot)
				{
					return Snapshot.CardId == HelperId;
				});
		}
		else
		{
			State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.RemoveAll(
				[](const FPlayerCardRuleSnapshot& Snapshot)
				{
					return Snapshot.CardId == HelperId;
				});
		}
		return State;
	}

	inline FMatchPlayState MakeZeroLegalState(
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		return MakeZeroLegalStateForType(
			ESkillRuleType::PassControl,
			Attacker);
	}

	inline FMatchPlayState MakeReadyState(
		const ESkillRuleType Type = ESkillRuleType::PassControl,
		const bool bHasHelper = true,
		const EInitialTurnOrderPlayer Attacker =
			EInitialTurnOrderPlayer::PlayerA)
	{
		FMatchPlayState State = MakeState(Type, Attacker);
		if (bHasHelper)
		{
			State =
				FMatchPlayCurrentAttackHelperSelectionWriter::Select(
					State,
					MakeRequest(HelperId, GetDefender(Attacker)))
					.AfterState;
		}
		else
		{
			FMatchPlayHelperDeclineRequest Request;
			Request.AttackSequence = ValidAttackSequence;
			Request.RequestingSide = GetDefender(Attacker);
			State =
				FMatchPlayHelperDecline::Decline(State, Request)
					.AfterState;
		}
		if (Type == ESkillRuleType::Cross)
		{
			FMatchPlayCurrentAttackBranchIntentSelectionRequest Request;
			Request.AttackSequence = ValidAttackSequence;
			Request.RequestingSide = Attacker;
			Request.Intent =
				EMatchPlayElectiveBranchIntent::CrossHigh;
			State =
				FMatchPlayCurrentAttackBranchIntentSelectionWriter
					::Select(State, Request)
					.AfterState;
		}
		return State;
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
