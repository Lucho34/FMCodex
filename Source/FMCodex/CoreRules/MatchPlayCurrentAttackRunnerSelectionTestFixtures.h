#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayCurrentAttackRunnerSelectionWriter.h"

namespace FMCodex::Tests::MatchPlayCurrentAttackRunnerSelection
{
	inline constexpr int64 ValidAttackSequence = 31;
	inline const FName CarrierId(TEXT("PlayerA.RunnerCarrier"));
	inline const FName MarkerId(TEXT("PlayerB.RunnerMarker"));
	inline const FName MidfieldRunnerId(TEXT("PlayerA.MidfieldRunner"));
	inline const FName AttackRunnerId(TEXT("PlayerA.AttackRunner"));
	inline const FName ForwardRunnerId(TEXT("PlayerA.ForwardRunner"));
	inline const FName DefenseRunnerId(TEXT("PlayerA.DefenseRunner"));
	inline const FName GoalkeeperId(TEXT("PlayerA.GoalkeeperCandidate"));
	inline const FName MissingRunnerId(TEXT("PlayerA.MissingRunner"));
	inline const FName PassControlSkillId(TEXT("Skill.Runner.PassControl"));
	inline const FName CrossSkillId(TEXT("Skill.Runner.Cross"));
	inline const FName ThroughBallSkillId(TEXT("Skill.Runner.ThroughBall"));

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
		const TArray<EPlayerPositionType>& PositionTypes)
	{
		FPlayerCardRuleSnapshot Snapshot;
		Snapshot.CardId = CardId;
		Snapshot.PositionTypes = PositionTypes;
		SetBaseAttributes(Snapshot.Attributes);
		return Snapshot;
	}

	inline FPlayerCardRuleSnapshot MakeGoalkeeper(
		const FName CardId)
	{
		FPlayerCardRuleSnapshot Snapshot =
			MakeCard(
				CardId,
				{EPlayerPositionType::Goalkeeper});
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

	inline FMatchPlayDeploymentSlotDefinition MakeSlot(
		const TCHAR* SlotId,
		const EMatchPlayNeutralSlotSide NeutralSide)
	{
		FMatchPlayDeploymentSlotDefinition Slot;
		Slot.SlotId = FName(SlotId);
		Slot.NeutralSide = NeutralSide;
		return Slot;
	}

	inline FName GetSkillId(const ESkillRuleType ActionType)
	{
		if (ActionType == ESkillRuleType::PassControl)
		{
			return PassControlSkillId;
		}
		if (ActionType == ESkillRuleType::Cross)
		{
			return CrossSkillId;
		}
		return ThroughBallSkillId;
	}

	inline FMatchPlayState MakeState(
		const ESkillRuleType ActionType =
			ESkillRuleType::PassControl)
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
		State.RuntimeState.PlayerBState.TotalAttackCount = 3;
		State.RuntimeState.PlayerBState.UsedAttackCount = 1;
		State.bHasCurrentAttack = true;

		State.CurrentAttack.Phase =
			EMatchPlayCurrentAttackPhase::Resolution;
		State.CurrentAttack.AttackSequence = ValidAttackSequence;
		State.CurrentAttack.ActionPoint = 5;
		State.CurrentAttack.SelectionStage =
			EMatchPlayCurrentAttackSelectionStage::AwaitingRunner;
		State.CurrentAttack.ActionPreparation.CarrierCardId =
			CarrierId;
		State.CurrentAttack.ActionPreparation.MarkerCardId =
			MarkerId;
		State.CurrentAttack.ActionPreparation.SkillId =
			GetSkillId(ActionType);
		State.CurrentAttack.ActionPreparation.ActionType =
			ActionType;
		State.CurrentAttack.CurrentLegalDeploymentSide =
			EInitialTurnOrderPlayer::None;
		State.CurrentAttack.bAttackerDeploymentFinished = true;
		State.CurrentAttack.bDefenderDeploymentFinished = true;

		State.CurrentAttack.DeploymentPlacements = {
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				CarrierId,
				TEXT("Slot.Carrier")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerB,
				MarkerId,
				TEXT("Slot.Marker")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				MidfieldRunnerId,
				TEXT("Slot.Midfield")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				AttackRunnerId,
				TEXT("Slot.Attack")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				ForwardRunnerId,
				TEXT("Slot.Forward")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				DefenseRunnerId,
				TEXT("Slot.Defense")),
			MakePlacement(
				EInitialTurnOrderPlayer::PlayerA,
				GoalkeeperId,
				TEXT("Slot.Goalkeeper"))
		};

		State.DeploymentSlotCatalog.Slots = {
			MakeSlot(
				TEXT("Slot.Carrier"),
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				TEXT("Slot.Marker"),
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				TEXT("Slot.Midfield"),
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				TEXT("Slot.Attack"),
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				TEXT("Slot.Forward"),
				EMatchPlayNeutralSlotSide::NearPlayerB),
			MakeSlot(
				TEXT("Slot.Defense"),
				EMatchPlayNeutralSlotSide::NearPlayerA),
			MakeSlot(
				TEXT("Slot.Goalkeeper"),
				EMatchPlayNeutralSlotSide::NearPlayerA)
		};

		State.CardSnapshotAuthority.PlayerACardSnapshots.Cards = {
			MakeCard(
				CarrierId,
				{EPlayerPositionType::Midfield}),
			MakeCard(
				MidfieldRunnerId,
				{
					EPlayerPositionType::Midfield,
					EPlayerPositionType::Defense
				}),
			MakeCard(
				AttackRunnerId,
				{
					EPlayerPositionType::Attack,
					EPlayerPositionType::Midfield
				}),
			MakeCard(
				ForwardRunnerId,
				{EPlayerPositionType::Midfield}),
			MakeCard(
				DefenseRunnerId,
				{EPlayerPositionType::Defense}),
			MakeGoalkeeper(GoalkeeperId)
		};
		State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards = {
			MakeCard(
				MarkerId,
				{EPlayerPositionType::Defense})
		};
		return State;
	}

	inline FMatchPlayCurrentAttackRunnerSelectionRequest MakeRequest(
		const FName RunnerCardId)
	{
		FMatchPlayCurrentAttackRunnerSelectionRequest Request;
		Request.AttackSequence = ValidAttackSequence;
		Request.RequestingSide =
			EInitialTurnOrderPlayer::PlayerA;
		Request.RunnerCardId = RunnerCardId;
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

	inline bool AreGlobalResultsEqual(
		const FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult&
			Left,
		const FMatchPlayCurrentAttackRunnerSelectionGlobalContextResult&
			Right)
	{
		bool bPlacementsEqual =
			Left.AttackingPlayerPlacements.Num()
				== Right.AttackingPlayerPlacements.Num();
		for (int32 Index = 0;
			bPlacementsEqual
				&& Index < Left.AttackingPlayerPlacements.Num();
			++Index)
		{
			const FMatchPlayDeploymentPlacement& LeftPlacement =
				Left.AttackingPlayerPlacements[Index];
			const FMatchPlayDeploymentPlacement& RightPlacement =
				Right.AttackingPlayerPlacements[Index];
			bPlacementsEqual =
				LeftPlacement.PlayerSide == RightPlacement.PlayerSide
				&& LeftPlacement.CardId == RightPlacement.CardId
				&& LeftPlacement.SlotId == RightPlacement.SlotId;
		}

		return Left.bSuccess == Right.bSuccess
			&& Left.RequestedAttackSequence
				== Right.RequestedAttackSequence
			&& Left.AuthoritativeAttackSequence
				== Right.AuthoritativeAttackSequence
			&& Left.RequestingSide == Right.RequestingSide
			&& Left.CurrentAttackingPlayer
				== Right.CurrentAttackingPlayer
			&& Left.CurrentDefendingPlayer
				== Right.CurrentDefendingPlayer
			&& Left.SelectionStateValidationResult.ErrorCode
				== Right.SelectionStateValidationResult.ErrorCode
			&& Left.FrozenCarrierCardId
				== Right.FrozenCarrierCardId
			&& Left.FrozenMarkerCardId
				== Right.FrozenMarkerCardId
			&& Left.FrozenSkillId == Right.FrozenSkillId
			&& Left.FrozenActionType == Right.FrozenActionType
			&& Left.ParticipantRequirementResult.bSuccess
				== Right.ParticipantRequirementResult.bSuccess
			&& Left.MatchingFrozenCarrierPlacementCount
				== Right.MatchingFrozenCarrierPlacementCount
			&& Left.MatchingFrozenMarkerPlacementCount
				== Right.MatchingFrozenMarkerPlacementCount
			&& bPlacementsEqual
			&& Left.AttackingSnapshotSetValidationResult.ErrorCode
				== Right.AttackingSnapshotSetValidationResult.ErrorCode
			&& Left.SlotCatalogValidationResult.ErrorCode
				== Right.SlotCatalogValidationResult.ErrorCode
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}
}

#endif
