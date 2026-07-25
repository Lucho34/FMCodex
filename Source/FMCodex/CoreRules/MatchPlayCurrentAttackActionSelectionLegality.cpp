#include "MatchPlayCurrentAttackActionSelectionLegality.h"

namespace MatchPlayCurrentAttackActionSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackActionSelectionLegalityResult& Result,
		const EMatchPlayCurrentAttackActionSelectionErrorCode ErrorCode,
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

	bool IsSupportedActionType(const ESkillRuleType ActionType)
	{
		switch (ActionType)
		{
		case ESkillRuleType::LongShot:
		case ESkillRuleType::CutInsideShot:
		case ESkillRuleType::PassControl:
		case ESkillRuleType::Cross:
		case ESkillRuleType::ThroughBall:
			return true;
		case ESkillRuleType::None:
		default:
			return false;
		}
	}

	bool IsSelectedActionStateCanonical(
		const FMatchPlayCurrentAttackState& CurrentAttack)
	{
		const FMatchPlayCurrentAttackSelectedAction& SelectedAction =
			CurrentAttack.SelectedAction;
		if (!CurrentAttack.bHasSelectedAction)
		{
			return SelectedAction.CarrierCardId.IsNone()
				&& SelectedAction.SkillId.IsNone()
				&& SelectedAction.ActionType == ESkillRuleType::None;
		}

		return !SelectedAction.CarrierCardId.IsNone()
			&& !SelectedAction.SkillId.IsNone()
			&& IsSupportedActionType(SelectedAction.ActionType);
	}
}

FMatchPlayCurrentAttackActionSelectionLegalityResult
FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackActionSelectionRequest& Request,
	const FSkillRuleSnapshotSet& SkillRules)
{
	using namespace
		MatchPlayCurrentAttackActionSelectionLegalityImplementation;

	FMatchPlayCurrentAttackActionSelectionLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before action selection."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Action selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Action selection attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for action selection."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!IsPlayerSide(Request.RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (Request.RequestingSide != CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the current attack action."));
		return Result;
	}

	if (!CurrentAttack.bAttackerDeploymentFinished
		|| !CurrentAttack.bDefenderDeploymentFinished)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::DeploymentNotFullyFinished,
			TEXT("Both sides must finish deployment before action selection."));
		return Result;
	}

	if (CurrentAttack.CurrentLegalDeploymentSide
		!= EInitialTurnOrderPlayer::None)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidCurrentLegalDeploymentSide,
			TEXT("CurrentLegalDeploymentSide must be None in Resolution."));
		return Result;
	}

	if (!IsSelectedActionStateCanonical(CurrentAttack))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidSelectedActionState,
			TEXT("Current attack selected-action state is not canonical."));
		return Result;
	}

	if (CurrentAttack.bHasSelectedAction)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::ActionAlreadySelected,
			TEXT("The current attack already has a selected action."));
		return Result;
	}

	if (Request.CarrierCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidCarrierCardId,
			TEXT("CarrierCardId must be non-empty."));
		return Result;
	}

	if (Request.SkillId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidSkillId,
			TEXT("SkillId must be non-empty."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == CurrentAttackingPlayer
			&& Placement.CardId == Request.CarrierCardId)
		{
			++Result.MatchingCarrierPlacementCount;
		}
	}

	if (Result.MatchingCarrierPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::CarrierNotDeployed,
			TEXT("Carrier must be deployed exactly once by the current attacker."));
		return Result;
	}

	if (Result.MatchingCarrierPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::CarrierDeploymentAmbiguous,
			TEXT("Carrier has multiple current-attacker deployment placements."));
		return Result;
	}

	const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BeforeState.CardSnapshotAuthority,
			CurrentAttackingPlayer,
			Request.CarrierCardId);
	Result.UnderlyingSnapshotAuthorityQueryErrorCode =
		SnapshotResult.ErrorCode;
	if (!SnapshotResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::CarrierSnapshotLookupFailed,
			SnapshotResult.ErrorMessage);
		return Result;
	}

	if (SnapshotResult.Snapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::CarrierIsGoalkeeper,
			TEXT("A goalkeeper cannot be selected as the attack carrier."));
		return Result;
	}

	if (!SnapshotResult.Snapshot.SkillIds.Contains(Request.SkillId))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::SkillNotOwnedByCarrier,
			TEXT("Selected SkillId is not owned by the authoritative carrier snapshot."));
		return Result;
	}

	const FSkillRuleSnapshotValidationResult SkillRuleValidationResult =
		FSkillRuleSnapshotValidator::Validate(SkillRules);
	Result.UnderlyingSkillRuleSetValidationErrorCode =
		SkillRuleValidationResult.ErrorCode;
	if (!SkillRuleValidationResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::SkillRuleSetValidationFailed,
			SkillRuleValidationResult.ErrorMessage);
		return Result;
	}

	FSkillRuleSnapshotQueryInput SkillRuleQueryInput;
	SkillRuleQueryInput.SkillId = Request.SkillId;
	const FSkillRuleSnapshotQueryResult SkillRuleQueryResult =
		FSkillRuleSnapshotQuery::FindBySkillId(
			SkillRules,
			SkillRuleQueryInput);
	Result.UnderlyingSkillRuleQueryErrorCode =
		SkillRuleQueryResult.ErrorCode;
	if (!SkillRuleQueryResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::SkillRuleLookupFailed,
			SkillRuleQueryResult.ErrorMessage);
		return Result;
	}

	Result.ResolvedActionType = SkillRuleQueryResult.Snapshot.SkillType;
	Result.ResolvedMinTriggerActionPoint =
		SkillRuleQueryResult.Snapshot.MinTriggerActionPoint;
	Result.ResolvedMaxTriggerActionPoint =
		SkillRuleQueryResult.Snapshot.MaxTriggerActionPoint;
	if (!IsSupportedActionType(Result.ResolvedActionType))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::UnsupportedActionType,
			TEXT("Selected skill does not resolve to a supported action type."));
		return Result;
	}

	if (CurrentAttack.ActionPoint
			< FSkillRuleSnapshotValidator::MinTriggerActionPoint
		|| CurrentAttack.ActionPoint
			> FSkillRuleSnapshotValidator::MaxTriggerActionPoint)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::InvalidCurrentActionPoint,
			TEXT("Current attack ActionPoint must be between 2 and 8."));
		return Result;
	}

	if (CurrentAttack.ActionPoint
			< Result.ResolvedMinTriggerActionPoint
		|| CurrentAttack.ActionPoint
			> Result.ResolvedMaxTriggerActionPoint)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackActionSelectionErrorCode
				::ActionPointOutsideSkillRange,
			TEXT("Current attack ActionPoint is outside the selected skill trigger range."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackActionSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
