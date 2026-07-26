#include "MatchPlayCurrentAttackSkillSelectionLegality.h"

namespace MatchPlayCurrentAttackSkillSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackSkillSelectionLegalityResult& Result,
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

FMatchPlayCurrentAttackSkillSelectionLegalityResult
FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayCurrentAttackSkillSelectionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackSkillSelectionLegalityImplementation;

	FMatchPlayCurrentAttackSkillSelectionLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before skill selection."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Skill selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
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

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentDefendingPlayer =
		GetDefendingPlayer(CurrentAttackingPlayer);
	if (!IsPlayerSide(CurrentDefendingPlayer))
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

	if (!IsPlayerSide(Request.RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (Request.RequestingSide != CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the skill."));
		return Result;
	}

	if (Request.SkillId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillId,
			TEXT("SkillId must be non-empty."));
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
		if (Placement.PlayerSide == CurrentAttackingPlayer
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
		if (Placement.PlayerSide == CurrentDefendingPlayer
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
			BeforeState.CardSnapshotAuthority,
			CurrentAttackingPlayer,
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

	if (!Result.CarrierSnapshotQueryResult.Snapshot.SkillIds.Contains(
		Request.SkillId))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::CarrierDoesNotOwnSkill,
			TEXT("The frozen carrier does not own the requested skill."));
		return Result;
	}

	FSkillRuleSnapshotQueryInput SkillRuleQueryInput;
	SkillRuleQueryInput.SkillId = Request.SkillId;
	Result.SkillRuleQueryResult =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRuleSet,
			SkillRuleQueryInput);
	if (!Result.SkillRuleQueryResult.bSuccess)
	{
		EMatchPlayCurrentAttackSkillSelectionErrorCode ErrorCode =
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::InvalidSkillRuleSet;
		if (Result.SkillRuleQueryResult.ErrorCode
			== ESkillRuleSnapshotQueryErrorCode::SkillRuleNotFound)
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleNotFound;
		}
		else if (
			Result.SkillRuleQueryResult.ErrorCode
				== ESkillRuleSnapshotQueryErrorCode
					::SnapshotSetValidationFailed
			&& Result.SkillRuleQueryResult.ValidationResult.ErrorCode
				== ESkillRuleSnapshotValidationErrorCode
					::DuplicateSkillId)
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleAmbiguous;
		}
		else if (
			Result.SkillRuleQueryResult.ErrorCode
				== ESkillRuleSnapshotQueryErrorCode
					::SnapshotSetValidationFailed
			&& Result.SkillRuleQueryResult.ValidationResult.InvalidSkillId
				== Request.SkillId
			&& (Result.SkillRuleQueryResult.ValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::InvalidSkillType
				|| Result.SkillRuleQueryResult.ValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::UnsupportedSkillType))
		{
			ErrorCode =
				EMatchPlayCurrentAttackSkillSelectionErrorCode
					::UnsupportedSkillRuleType;
		}

		SetError(
			Result,
			ErrorCode,
			Result.SkillRuleQueryResult.ErrorMessage);
		return Result;
	}

	Result.ResolvedSkillRule =
		Result.SkillRuleQueryResult.Snapshot;
	Result.ResolvedActionType =
		Result.ResolvedSkillRule.SkillType;
	Result.ParticipantRequirementResult =
		FMatchPlaySkillParticipantRequirementQuery::Query(
			Result.ResolvedActionType);
	if (!Result.ParticipantRequirementResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ParticipantRequirementResolutionFailed,
			Result.ParticipantRequirementResult.ErrorMessage);
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

	if (CurrentAttack.ActionPoint
			< Result.ResolvedSkillRule.MinTriggerActionPoint
		|| CurrentAttack.ActionPoint
			> Result.ResolvedSkillRule.MaxTriggerActionPoint)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackSkillSelectionErrorCode
				::ActionPointOutsideSkillRange,
			TEXT("Current attack ActionPoint is outside the selected skill trigger range."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackSkillSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
