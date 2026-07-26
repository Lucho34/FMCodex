#include "MatchPlayCurrentAttackCarrierSelectionLegality.h"

namespace MatchPlayCurrentAttackCarrierSelectionLegalityImplementation
{
	void SetError(
		FMatchPlayCurrentAttackCarrierSelectionLegalityResult& Result,
		const EMatchPlayCurrentAttackCarrierSelectionErrorCode ErrorCode,
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
}

FMatchPlayCurrentAttackCarrierSelectionLegalityResult
FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackCarrierSelectionRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackCarrierSelectionLegalityImplementation;

	FMatchPlayCurrentAttackCarrierSelectionLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before carrier selection."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::NoCurrentAttack,
			TEXT("Carrier selection requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::AttackSequenceMismatch,
			TEXT("Carrier selection attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::CurrentAttackNotInResolution,
			TEXT("Current attack must be in Resolution phase for carrier selection."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayerSide(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	Result.SelectionStateValidationResult =
		FMatchPlayCurrentAttackSelectionStateValidator::Validate(
			CurrentAttack);
	if (!Result.SelectionStateValidationResult.bIsCanonical)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::InvalidSelectionState,
			Result.SelectionStateValidationResult.ErrorMessage);
		return Result;
	}

	if (CurrentAttack.SelectionStage
		!= EMatchPlayCurrentAttackSelectionStage::AwaitingCarrier)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::WrongSelectionStage,
			TEXT("Carrier selection requires AwaitingCarrier stage."));
		return Result;
	}

	if (!IsPlayerSide(Request.RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (Request.RequestingSide != CurrentAttackingPlayer)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::RequestingSideIsNotCurrentAttacker,
			TEXT("Only the current attacker may select the carrier."));
		return Result;
	}

	if (Request.CarrierCardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::InvalidCarrierCardId,
			TEXT("CarrierCardId must be non-empty."));
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
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::CarrierNotDeployed,
			TEXT("Carrier must be deployed by the current attacker."));
		return Result;
	}

	if (Result.MatchingCarrierPlacementCount > 1)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
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
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::CarrierSnapshotLookupFailed,
			SnapshotResult.ErrorMessage);
		return Result;
	}

	if (SnapshotResult.Snapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayCurrentAttackCarrierSelectionErrorCode
				::CarrierIsGoalkeeper,
			TEXT("A goalkeeper cannot be selected as the attack carrier."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackCarrierSelectionErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
