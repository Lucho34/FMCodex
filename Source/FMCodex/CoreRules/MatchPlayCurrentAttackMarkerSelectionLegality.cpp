#include "MatchPlayCurrentAttackMarkerSelectionLegality.h"

namespace MatchPlayCurrentAttackMarkerSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackMarkerSelectionLegalityResult& Result,
		const EMatchPlayCurrentAttackMarkerSelectionErrorCode ErrorCode,
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

FMatchPlayCurrentAttackMarkerSelectionLegalityResult
FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackMarkerSelectionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackMarkerSelectionLegalityImplementation;

	FMatchPlayCurrentAttackMarkerSelectionLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before marker selection."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Marker selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Marker selection attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for marker selection."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
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
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
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
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}

	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingMarker)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Marker selection requires AwaitingMarker stage."));
		return Result;
	}

	if (!IsPlayerSide(Request.RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (Request.RequestingSide != CurrentDefendingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::RequestingSideIsNotCurrentDefender,
			TEXT("Only the current defender may select the marker."));
		return Result;
	}

	if (Request.MarkerCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::InvalidMarkerCardId,
			TEXT("MarkerCardId must be non-empty."));
		return Result;
	}

	Result.FrozenCarrierCardId =
		CurrentAttack.ActionPreparation.CarrierCardId;
	if (Result.FrozenCarrierCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::InvalidFrozenCarrierCardId,
			TEXT("The frozen preparation carrier must be non-empty."));
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
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::FrozenCarrierNotDeployed,
			TEXT("The frozen carrier must be deployed by the current attacker."));
		return Result;
	}

	if (Result.MatchingFrozenCarrierPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::FrozenCarrierDeploymentAmbiguous,
			TEXT("The frozen carrier has multiple current-attacker placements."));
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == CurrentDefendingPlayer
			&& Placement.CardId == Request.MarkerCardId)
		{
			++Result.MatchingMarkerPlacementCount;
			Result.MarkerPlacement = Placement;
		}
	}

	if (Result.MatchingMarkerPlacementCount == 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerNotDeployed,
			TEXT("Marker must be deployed by the current defender."));
		return Result;
	}

	if (Result.MatchingMarkerPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerDeploymentAmbiguous,
			TEXT("Marker has multiple current-defender placements."));
		return Result;
	}

	Result.MarkerSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BeforeState.CardSnapshotAuthority,
			CurrentDefendingPlayer,
			Request.MarkerCardId);
	if (!Result.MarkerSnapshotQueryResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerSnapshotQueryFailed,
			Result.MarkerSnapshotQueryResult.ErrorMessage);
		return Result;
	}

	Result.PhysicalAreaMatchResult =
		FMatchPlayDeploymentPhysicalAreaMatchQuery::Query(
			BeforeState.DeploymentSlotCatalog,
			CurrentAttackingPlayer,
			Result.FrozenCarrierPlacement,
			Result.MarkerPlacement);
	if (!Result.PhysicalAreaMatchResult.bSuccess)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::PhysicalAreaQueryFailed,
			Result.PhysicalAreaMatchResult.ErrorMessage);
		return Result;
	}

	if (!Result.PhysicalAreaMatchResult.bSamePhysicalArea)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerNotInCarrierPhysicalArea,
			TEXT("Marker must occupy the carrier's physical area."));
		return Result;
	}

	if (Result.MarkerSnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackMarkerSelectionErrorCode
				::MarkerIsGoalkeeper,
			TEXT("A goalkeeper cannot be selected as a normal marker."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackMarkerSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
