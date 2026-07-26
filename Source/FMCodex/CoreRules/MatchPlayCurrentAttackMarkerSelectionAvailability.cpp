#include "MatchPlayCurrentAttackMarkerSelectionAvailability.h"

namespace MatchPlayCurrentAttackMarkerSelectionAvailabilityImplementation
{
	FMatchPlayCurrentAttackMarkerSelectionRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName MarkerCardId)
	{
		FMatchPlayCurrentAttackMarkerSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.MarkerCardId = MarkerCardId;
		return Request;
	}

	bool IsGlobalBlocker(
		const FMatchPlayCurrentAttackMarkerSelectionLegalityResult& Result)
	{
		switch (Result.ErrorCode)
		{
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::NoCurrentAttack:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::AttackSequenceMismatch:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::CurrentAttackNotInResolution:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidCurrentDefendingPlayer:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidSelectionState:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::WrongSelectionStage:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidRequestingSide:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::RequestingSideIsNotCurrentDefender:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidFrozenCarrierCardId:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::FrozenCarrierNotDeployed:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::FrozenCarrierDeploymentAmbiguous:
			return true;

		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::PhysicalAreaQueryFailed:
			switch (Result.PhysicalAreaMatchResult.ErrorCode)
			{
			case EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::InvalidSlotCatalog:
			case EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::InvalidFirstPlayerSide:
			case EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::InvalidFirstSlotId:
			case EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::FirstSlotNotFound:
			case EMatchPlayDeploymentPhysicalAreaMatchErrorCode
				::FirstSlotLookupFailed:
				return true;

			default:
				return false;
			}

		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerSnapshotQueryFailed:
			return Result.MarkerSnapshotQueryResult.ErrorCode
				== EMatchPlayCardSnapshotAuthorityQueryErrorCode
					::SnapshotValidationFailed;

		case EMatchPlayCurrentAttackMarkerSelectionErrorCode::None:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::InvalidMarkerCardId:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerNotDeployed:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerDeploymentAmbiguous:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerNotInCarrierPhysicalArea:
		case EMatchPlayCurrentAttackMarkerSelectionErrorCode
			::MarkerIsGoalkeeper:
		default:
			return false;
		}
	}
}

FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult
FMatchPlayCurrentAttackMarkerSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackMarkerSelectionAvailabilityImplementation;

	FMatchPlayCurrentAttackMarkerSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	const FMatchPlayCurrentAttackMarkerSelectionLegalityResult ProbeResult =
		FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			MakeRequest(
				AttackSequence,
				RequestingSide,
				NAME_None));
	if (!ProbeResult.bIsLegal && IsGlobalBlocker(ProbeResult))
	{
		Result.bQuerySucceeded = true;
		Result.bHasGlobalBlockingLegalityResult = true;
		Result.GlobalBlockingLegalityResult = ProbeResult;
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	const EInitialTurnOrderPlayer CurrentDefendingPlayer =
		CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	for (const FMatchPlayDeploymentPlacement& Placement :
		BeforeState.CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide != CurrentDefendingPlayer)
		{
			continue;
		}

		FMatchPlayCurrentAttackMarkerSelectionCandidateAvailability
			Candidate;
		Candidate.MarkerCardId = Placement.CardId;
		Candidate.LegalityResult =
			FMatchPlayCurrentAttackMarkerSelectionLegalityEvaluator
				::Evaluate(
					BeforeState,
					MakeRequest(
						AttackSequence,
						RequestingSide,
						Placement.CardId));
		if (IsGlobalBlocker(Candidate.LegalityResult))
		{
			Result.Candidates.Reset();
			Result.bCanSelectAnyMarker = false;
			Result.bQuerySucceeded = true;
			Result.bHasGlobalBlockingLegalityResult = true;
			Result.GlobalBlockingLegalityResult =
				Candidate.LegalityResult;
			return Result;
		}
		if (Candidate.LegalityResult.bIsLegal)
		{
			Result.bCanSelectAnyMarker = true;
		}
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	return Result;
}
