#include "MatchPlayCurrentAttackCarrierSelectionAvailability.h"

namespace MatchPlayCurrentAttackCarrierSelectionAvailabilityImplementation
{
	FMatchPlayCurrentAttackCarrierSelectionRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName CarrierCardId)
	{
		FMatchPlayCurrentAttackCarrierSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CarrierCardId = CarrierCardId;
		return Request;
	}

	bool IsGlobalBlocker(
		const EMatchPlayCurrentAttackCarrierSelectionErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::NoCurrentAttack:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::AttackSequenceMismatch:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CurrentAttackNotInResolution:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::InvalidSelectionState:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::WrongSelectionStage:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::InvalidRequestingSide:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker:
			return true;

		case EMatchPlayCurrentAttackCarrierSelectionErrorCode::None:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::InvalidCarrierCardId:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierNotDeployed:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierDeploymentAmbiguous:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierSnapshotLookupFailed:
		case EMatchPlayCurrentAttackCarrierSelectionErrorCode
			::CarrierIsGoalkeeper:
		default:
			return false;
		}
	}
}

FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult
FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackCarrierSelectionAvailabilityImplementation;

	FMatchPlayCurrentAttackCarrierSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	const FMatchPlayCurrentAttackCarrierSelectionLegalityResult ProbeResult =
		FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			MakeRequest(
				AttackSequence,
				RequestingSide,
				NAME_None));
	if (!ProbeResult.bIsLegal && IsGlobalBlocker(ProbeResult.ErrorCode))
	{
		Result.bQuerySucceeded = true;
		Result.bHasGlobalBlockingLegalityResult = true;
		Result.GlobalBlockingLegalityResult = ProbeResult;
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	for (const FMatchPlayDeploymentPlacement& Placement :
		BeforeState.CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide != CurrentAttackingPlayer)
		{
			continue;
		}

		FMatchPlayCurrentAttackCarrierSelectionCandidateAvailability
			Candidate;
		Candidate.CarrierCardId = Placement.CardId;
		Candidate.LegalityResult =
			FMatchPlayCurrentAttackCarrierSelectionLegalityEvaluator
				::Evaluate(
					BeforeState,
					MakeRequest(
						AttackSequence,
						RequestingSide,
						Placement.CardId));
		if (Candidate.LegalityResult.bIsLegal)
		{
			Result.bCanSelectAnyCarrier = true;
		}
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	return Result;
}
