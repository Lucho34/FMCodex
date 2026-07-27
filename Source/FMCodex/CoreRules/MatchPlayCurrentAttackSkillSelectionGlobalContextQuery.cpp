#include "MatchPlayCurrentAttackSkillSelectionGlobalContextQuery.h"

namespace MatchPlayCurrentAttackSkillSelectionGlobalContextImplementation
{
	void SetError(
		FMatchPlayCurrentAttackSkillSelectionGlobalContextResult& Result,
		const EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayerSide(const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer GetDefendingPlayer(
		const EInitialTurnOrderPlayer AttackingPlayer)
	{
		if (AttackingPlayer == EInitialTurnOrderPlayer::PlayerA)
		{
			return EInitialTurnOrderPlayer::PlayerB;
		}
		if (AttackingPlayer == EInitialTurnOrderPlayer::PlayerB)
		{
			return EInitialTurnOrderPlayer::PlayerA;
		}
		return EInitialTurnOrderPlayer::None;
	}
}

FMatchPlayCurrentAttackSkillSelectionGlobalContextResult
FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	using namespace
		MatchPlayCurrentAttackSkillSelectionGlobalContextImplementation;

	FMatchPlayCurrentAttackSkillSelectionGlobalContextResult Result;
	Result.RequestedAttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	if (!State.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before skill selection."));
		return Result;
	}

	if (!State.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Skill selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		State.CurrentAttack;
	Result.AuthoritativeAttackSequence =
		CurrentAttack.AttackSequence;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Skill selection attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for skill selection."));
		return Result;
	}

	Result.CurrentAttackingPlayer =
		State.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(Result.CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	Result.CurrentDefendingPlayer =
		GetDefendingPlayer(Result.CurrentAttackingPlayer);
	if (!IsPlayerSide(Result.CurrentDefendingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentDefendingPlayer,
			TEXT("Current defending player could not be derived."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			CurrentAttack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}

	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingSkill)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Skill selection requires AwaitingSkill stage."));
		return Result;
	}

	if (!IsPlayerSide(RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (RequestingSide != Result.CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the skill."));
		return Result;
	}

	Result.FrozenCarrierCardId =
		CurrentAttack.ActionPreparation.CarrierCardId;
	if (Result.FrozenCarrierCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidFrozenCarrierCardId,
			TEXT("The frozen preparation carrier must be non-empty."));
		return Result;
	}

	Result.FrozenMarkerCardId =
		CurrentAttack.ActionPreparation.MarkerCardId;
	if (Result.FrozenMarkerCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidFrozenMarkerCardId,
			TEXT("The frozen preparation marker must be non-empty."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == Result.CurrentAttackingPlayer
			&& Placement.CardId == Result.FrozenCarrierCardId)
		{
			++Result.MatchingFrozenCarrierPlacementCount;
			Result.FrozenCarrierPlacement = Placement;
		}
	}

	if (Result.MatchingFrozenCarrierPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenCarrierNotDeployed,
			TEXT("The frozen carrier must be deployed by the current attacker."));
		return Result;
	}

	if (Result.MatchingFrozenCarrierPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenCarrierDeploymentAmbiguous,
			TEXT("The frozen carrier has multiple current-attacker placements."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == Result.CurrentDefendingPlayer
			&& Placement.CardId == Result.FrozenMarkerCardId)
		{
			++Result.MatchingFrozenMarkerPlacementCount;
			Result.FrozenMarkerPlacement = Placement;
		}
	}

	if (Result.MatchingFrozenMarkerPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerNotDeployed,
			TEXT("The frozen marker must be deployed by the current defender."));
		return Result;
	}

	if (Result.MatchingFrozenMarkerPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::FrozenMarkerDeploymentAmbiguous,
			TEXT("The frozen marker has multiple current-defender placements."));
		return Result;
	}

	Result.CarrierSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			Result.CurrentAttackingPlayer,
			Result.FrozenCarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		const FPlayerCardRuleSnapshotValidationResult&
			SnapshotValidation =
				Result.CarrierSnapshotQueryResult
					.UnderlyingQueryResult.ValidationResult;
		const bool bDuplicateSkillId =
			Result.CarrierSnapshotQueryResult.ErrorCode
				== EMatchPlayCardSnapshotAuthorityQueryErrorCode
					::SnapshotValidationFailed
			&& SnapshotValidation.ErrorCode
				== EPlayerCardRuleSnapshotValidationErrorCode
					::DuplicateSkillId;
		SetError(
			Result,
			bDuplicateSkillId
				? EMatchPlayCurrentAttackSkillSelectionErrorCode
					::DuplicateCarrierSkillId
				: EMatchPlayCurrentAttackSkillSelectionErrorCode
					::CarrierSnapshotQueryFailed,
			Result.CarrierSnapshotQueryResult.ErrorMessage);
		return Result;
	}
	Result.ResolvedCarrierSnapshot =
		Result.CarrierSnapshotQueryResult.Snapshot;

	Result.SkillRuleSetValidationResult =
		FSkillRuleSnapshotValidator::Validate(SkillRuleSet);
	if (!Result.SkillRuleSetValidationResult.bSuccess)
	{
		SetError(
			Result,
			Result.SkillRuleSetValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::DuplicateSkillId
				? EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleAmbiguous
				: EMatchPlayCurrentAttackSkillSelectionErrorCode
					::InvalidSkillRuleSet,
			Result.SkillRuleSetValidationResult.ErrorMessage);
		return Result;
	}

	if (CurrentAttack.ActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| CurrentAttack.ActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackActionPoint,
			TEXT("Current attack ActionPoint must be within 2 through 8."));
		return Result;
	}

	Result.ValidatedActionPoint = CurrentAttack.ActionPoint;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
