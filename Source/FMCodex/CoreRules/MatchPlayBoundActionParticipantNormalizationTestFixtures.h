#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "MatchPlayBoundActionParticipantNormalizationQuery.h"
#include "MatchPlayCurrentAttackHelperSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionTestFixtures.h"
#include "MatchPlayCurrentAttackSkillSelectionWriter.h"

namespace FMCodex::Tests::MatchPlayBoundActionParticipantNormalization
{
	namespace HelperFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackHelperSelection;
	namespace SkillFixtures =
		FMCodex::Tests::MatchPlayCurrentAttackSkillSelection;

	inline FName GetNoRunnerSkillId(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::CutInsideShot
			? SkillFixtures::CutInsideSkillId
			: SkillFixtures::LongShotSkillId;
	}

	inline FMatchPlayState MakeReadyState(
		const ESkillRuleType ActionType,
		const bool bHasHelper = false,
		const EInitialTurnOrderPlayer AttackingPlayer =
			EInitialTurnOrderPlayer::PlayerA)
	{
		if (ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot)
		{
			const FName SkillId = GetNoRunnerSkillId(ActionType);
			const FMatchPlayCurrentAttackSkillSelectionWriterResult
				WriterResult =
				FMatchPlayCurrentAttackSkillSelectionWriter::Select(
					SkillFixtures::MakeState({SkillId}),
					SkillFixtures::MakeRuleSet(),
					SkillFixtures::MakeRequest(SkillId));
			return WriterResult.AfterState;
		}
		return HelperFixtures::MakeReadyState(
			ActionType,
			bHasHelper,
			AttackingPlayer);
	}

	inline FMatchPlayBoundActionParticipantNormalizationRequest MakeRequest(
		const FMatchPlayState& State)
	{
		FMatchPlayBoundActionParticipantNormalizationRequest Request;
		Request.AttackSequence = State.CurrentAttack.AttackSequence;
		return Request;
	}

	inline FPlayerCardRuleSnapshot* FindSnapshot(
		FMatchPlayState& State,
		const EInitialTurnOrderPlayer Side,
		const FName CardId)
	{
		FPlayerCardRuleSnapshotSet& Set =
			Side == EInitialTurnOrderPlayer::PlayerA
				? State.CardSnapshotAuthority.PlayerACardSnapshots
				: State.CardSnapshotAuthority.PlayerBCardSnapshots;
		return Set.Cards.FindByPredicate(
			[CardId](const FPlayerCardRuleSnapshot& Snapshot)
			{
				return Snapshot.CardId == CardId;
			});
	}

	inline void SetDistinctValues(
		FPlayerAttributes& Attributes,
		const int32 Offset = 0)
	{
		Attributes.Shooting = 1 + (Offset % 6);
		Attributes.Dribbling = 1 + ((Offset + 1) % 6);
		Attributes.Passing = 1 + ((Offset + 2) % 6);
		Attributes.OffBall = 1 + ((Offset + 3) % 6);
		Attributes.Marking = 1 + ((Offset + 4) % 6);
		Attributes.Tackling = 1 + ((Offset + 5) % 6);
		Attributes.Speed = 1 + ((Offset + 1) % 6);
		Attributes.Strength = 1 + ((Offset + 2) % 6);
		Attributes.Stamina = 1 + ((Offset + 3) % 6);
		Attributes.LongShot = 1 + ((Offset + 4) % 6);
	}

	inline bool ValuesMatch(
		const FMatchPlayBoundActionNormalizedParticipantValues& Values,
		const FPlayerAttributes& Attributes)
	{
		return Values.Shooting == Attributes.Shooting
			&& Values.Dribbling == Attributes.Dribbling
			&& Values.Passing == Attributes.Passing
			&& Values.OffBall == Attributes.OffBall
			&& Values.Marking == Attributes.Marking
			&& Values.Tackling == Attributes.Tackling
			&& Values.Speed == Attributes.Speed
			&& Values.Strength == Attributes.Strength
			&& Values.Stamina == Attributes.Stamina
			&& Values.LongShot == Attributes.LongShot;
	}

	inline bool ValuesAreZero(
		const FMatchPlayBoundActionNormalizedParticipantValues& Values)
	{
		return Values.Shooting == 0
			&& Values.Dribbling == 0
			&& Values.Passing == 0
			&& Values.OffBall == 0
			&& Values.Marking == 0
			&& Values.Tackling == 0
			&& Values.Speed == 0
			&& Values.Strength == 0
			&& Values.Stamina == 0
			&& Values.LongShot == 0;
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

	inline bool AreSnapshotsEqual(
		const FPlayerCardRuleSnapshot& Left,
		const FPlayerCardRuleSnapshot& Right)
	{
		return FPlayerCardRuleSnapshot::StaticStruct()
			->CompareScriptStruct(&Left, &Right, 0);
	}

	inline bool AreSnapshotValidationResultsEqual(
		const FPlayerCardRuleSnapshotValidationResult& Left,
		const FPlayerCardRuleSnapshotValidationResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bIsValid == Right.bIsValid
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.InvalidCardId == Right.InvalidCardId
			&& Left.DuplicateCardIds == Right.DuplicateCardIds;
	}

	inline bool AreSnapshotQueryResultsEqual(
		const FPlayerCardRuleSnapshotQueryResult& Left,
		const FPlayerCardRuleSnapshotQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.bFound == Right.bFound
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& AreSnapshotValidationResultsEqual(
				Left.ValidationResult,
				Right.ValidationResult);
	}

	inline bool AreAuthorityQueryResultsEqual(
		const FMatchPlayCardSnapshotAuthorityQueryResult& Left,
		const FMatchPlayCardSnapshotAuthorityQueryResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.PlayerSide == Right.PlayerSide
			&& Left.CardId == Right.CardId
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& AreSnapshotQueryResultsEqual(
				Left.UnderlyingQueryResult,
				Right.UnderlyingQueryResult)
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreHelperAuthorityResultsEqual(
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult& Left,
		const FMatchPlayCurrentAttackHelperParticipantAuthorityResult& Right)
	{
		return Left.bSuccess == Right.bSuccess
			&& Left.MatchingPlacementCount
				== Right.MatchingPlacementCount
			&& FMatchPlayDeploymentPlacement::StaticStruct()
				->CompareScriptStruct(
					&Left.Placement,
					&Right.Placement,
					0)
			&& AreAuthorityQueryResultsEqual(
				Left.SnapshotQueryResult,
				Right.SnapshotQueryResult)
			&& AreSnapshotsEqual(Left.Snapshot, Right.Snapshot)
			&& Left.ErrorCode == Right.ErrorCode
			&& Left.ErrorMessage == Right.ErrorMessage;
	}

	inline bool AreReadyResultsEqual(
		const FMatchPlayCurrentAttackReadyValidationResult& Left,
		const FMatchPlayCurrentAttackReadyValidationResult& Right)
	{
		return FMatchPlayCurrentAttackReadyValidationResult::StaticStruct()
				->CompareScriptStruct(&Left, &Right, 0)
			&& AreAuthorityQueryResultsEqual(
				Left.CarrierSnapshotQueryResult,
				Right.CarrierSnapshotQueryResult)
			&& AreAuthorityQueryResultsEqual(
				Left.MarkerSnapshotQueryResult,
				Right.MarkerSnapshotQueryResult)
			&& AreAuthorityQueryResultsEqual(
				Left.RunnerSnapshotQueryResult,
				Right.RunnerSnapshotQueryResult)
			&& AreHelperAuthorityResultsEqual(
				Left.HelperAuthorityResult,
				Right.HelperAuthorityResult)
			&& FMatchPlayRelativeDeploymentZoneResolveResult::StaticStruct()
				->CompareScriptStruct(
					&Left.RunnerRelativeZoneResolveResult,
					&Right.RunnerRelativeZoneResolveResult,
					0);
	}

	inline bool AreResultsEqual(
		const FMatchPlayBoundActionParticipantNormalizationResult& Left,
		const FMatchPlayBoundActionParticipantNormalizationResult& Right)
	{
		return FMatchPlayBoundActionParticipantNormalizationResult
			::StaticStruct()->CompareScriptStruct(&Left, &Right, 0)
			&& AreReadyResultsEqual(
				Left.BindingResult.ReadyValidationResult,
				Right.BindingResult.ReadyValidationResult);
	}
}

#endif
