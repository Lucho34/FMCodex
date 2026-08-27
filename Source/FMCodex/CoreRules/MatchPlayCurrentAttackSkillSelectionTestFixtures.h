#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackSkillSelectionLegality.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackSkillSelection
{
	inline constexpr int64 ValidAttackSequence = 23;
	inline const FName CarrierId(TEXT("PlayerA.SkillCarrier"));
	inline const FName MarkerId(TEXT("PlayerB.SkillMarker"));
	inline const FName SharedCardId(TEXT("Shared.Card"));
	inline const FName LongShotSkillId(TEXT("Skill.LongShot"));
	inline const FName CutInsideSkillId(TEXT("Skill.CutInside"));
	inline const FName PassControlSkillId(TEXT("Skill.PassControl"));
	inline const FName CrossSkillId(TEXT("Skill.Cross"));
	inline const FName ThroughBallSkillId(TEXT("Skill.ThroughBall"));
	inline const FName MissingSkillId(TEXT("Skill.Missing"));

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
		const TArray<FName>& SkillIds = {})
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = {EPlayerPositionType::Attack};
		SetBaseAttributes(Snapshot.Attributes);
		Snapshot.SkillIds = SkillIds;
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

	inline FSkillRuleSnapshot MakeRule(
		const FName SkillId,
		const ESkillRuleType SkillType,
		const int32 Minimum = 2,
		const int32 Maximum = 8)
	{
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = SkillType;
		Rule.MinTriggerActionPoint = Minimum;
		Rule.MaxTriggerActionPoint = Maximum;
		return Rule;
	}

	inline FSkillRuleSnapshotSet MakeRuleSet()
	{
		FSkillRuleSnapshotSet Rules;
		Rules.SkillRules = {
			MakeRule(LongShotSkillId, ESkillRuleType::LongShot),
			MakeRule(
				CutInsideSkillId,
				ESkillRuleType::CutInsideShot),
			MakeRule(
				PassControlSkillId,
				ESkillRuleType::PassControl),
			MakeRule(CrossSkillId, ESkillRuleType::Cross),
			MakeRule(
				ThroughBallSkillId,
				ESkillRuleType::ThroughBall)
		};
		return Rules;
	}

	inline FMatchPlayState MakeState(
		const TArray<FName>& CarrierSkillIds = {
			LongShotSkillId,
			CrossSkillId,
			PassControlSkillId
		})
	{
		FMatchPlayState State;
		State.RuntimeState.bIsInitialized = true;
		State.RuntimeState.FirstPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.SecondPlayer =
			EInitialTurnOrderPlayer::PlayerB;
		State.RuntimeState.CurrentAttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA;
		State.RuntimeState.PlayerAState.TotalAttackCount = 3;
		State.RuntimeState.PlayerAState.UsedAttackCount = 1;
		State.RuntimeState.PlayerAState.Score = 2;
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.RuntimeState.PlayerBState.Score = 1;
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds = {
			CarrierId
		};
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds = {
			MarkerId
		};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(CarrierId, CarrierSkillIds)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(MarkerId)
		};
		State.bHasCurrentAttack = true;
		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
		State.CurrentAttack.ActionPreparation.CarrierCardId =
			CarrierId;
		State.CurrentAttack.ActionPreparation.MarkerCardId =
			MarkerId;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;
		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierId,
				TEXT("Slot.Attacker")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerId,
				TEXT("Slot.Defender"))
		};
		return State;
	}

	inline FMatchPlayCurrentAttackSkillSelectionRequest MakeRequest(
		const FName SkillId = LongShotSkillId)
	{
		FMatchPlayCurrentAttackSkillSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide =
			EInitialTurnOrderPlayer::PlayerA;
		Request.SkillId = SkillId;
		return Request;
	}

	inline FMatchPlayState MakeParticipantFirstRunnerState(
		const EMatchPlayNeutralSlotSide RunnerNeutralSide)
	{
		const FName RunnerId(TEXT("PlayerA.PreparedMidfieldRunner"));
		FMatchPlayState State = MakeState(
			{ThroughBallSkillId, PassControlSkillId});
		FPlayerCardRuleSnapshot Runner = MakeCard(RunnerId);
		Runner.PositionTypes = {EPlayerPositionType::Midfield};
		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.Add(Runner);
		State.CurrentAttack.DeploymentPlacements.Add(MakePlacement(
			EInitialTurnOrderPlayer::PlayerA,
			RunnerId,
			TEXT("Slot.PreparedRunner")));
		auto MakeSlot = [](const TCHAR* SlotId,
			const EMatchPlayNeutralSlotSide NeutralSide)
		{
			FMatchPlayDeploymentSlotDefinition Slot;
			Slot.SlotId = FName(SlotId);
			Slot.NeutralSide = NeutralSide;
			return Slot;
		};
		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(TEXT("Slot.Attacker"),
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(TEXT("Slot.Defender"),
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(TEXT("Slot.PreparedRunner"), RunnerNeutralSide)
		};
		State.CurrentAttack.ActionPreparation.RunnerCardId = RunnerId;
		State.CurrentAttack.ActionPreparation.bSkillSelectionDeferred = true;
		State.CurrentAttack.ActionPreparation.SkillId = NAME_None;
		State.CurrentAttack.ActionPreparation.ActionType = ESkillRuleType::None;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingSkill;
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
