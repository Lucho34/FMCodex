#include "MatchPlayCurrentAttackActionSelectionAvailability.h"

namespace MatchPlayCurrentAttackActionSelectionAvailabilityImplementation
{
	FMatchPlayCurrentAttackActionSelectionRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName CarrierCardId,
		const FName SkillId)
	{
		FMatchPlayCurrentAttackActionSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CarrierCardId = CarrierCardId;
		Request.SkillId = SkillId;
		return Request;
	}

	bool IsGlobalBlocker(
		const EMatchPlayCurrentAttackActionSelectionErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::NoCurrentAttack:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::AttackSequenceMismatch:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::CurrentAttackNotInResolution:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidRequestingSide:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::DeploymentNotFullyFinished:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidCurrentLegalDeploymentSide:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidSelectedActionState:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::ActionAlreadySelected:
			return true;
		case EMatchPlayCurrentAttackActionSelectionErrorCode::None:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidCarrierCardId:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidSkillId:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierNotDeployed:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierDeploymentAmbiguous:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierSnapshotLookupFailed:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::CarrierIsGoalkeeper:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::SkillNotOwnedByCarrier:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::SkillRuleSetValidationFailed:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::SkillRuleLookupFailed:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::UnsupportedActionType:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::InvalidCurrentActionPoint:
		case EMatchPlayCurrentAttackActionSelectionErrorCode
			::ActionPointOutsideSkillRange:
		default:
			return false;
		}
	}
}

FMatchPlayCurrentAttackActionSelectionAvailabilityResult
FMatchPlayCurrentAttackActionSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide,
	const FSkillRuleSnapshotSet& SkillRules)
{
	using namespace
		MatchPlayCurrentAttackActionSelectionAvailabilityImplementation;

	FMatchPlayCurrentAttackActionSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	const FMatchPlayCurrentAttackActionSelectionLegalityResult ProbeResult =
		FMatchPlayCurrentAttackActionSelectionLegalityEvaluator::Evaluate(
			BeforeState,
			MakeRequest(
				AttackSequence,
				RequestingSide,
				NAME_None,
				NAME_None),
			SkillRules);
	if (!ProbeResult.bIsLegal && IsGlobalBlocker(ProbeResult.ErrorCode))
	{
		Result.bQuerySucceeded = true;
		Result.bHasFirstBlockingLegalityResult = true;
		Result.FirstBlockingLegalityResult = ProbeResult;
		return Result;
	}

	const FSkillRuleSnapshotValidationResult SkillRuleValidationResult =
		FSkillRuleSnapshotValidator::Validate(SkillRules);
	Result.UnderlyingSkillRuleSetValidationErrorCode =
		SkillRuleValidationResult.ErrorCode;
	if (!SkillRuleValidationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode
				::SkillRuleSetValidationFailed;
		Result.ErrorMessage = SkillRuleValidationResult.ErrorMessage;
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

		const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotResult =
			FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
				BeforeState.CardSnapshotAuthority,
				CurrentAttackingPlayer,
				Placement.CardId);
		Result.UnderlyingSnapshotAuthorityQueryErrorCode =
			SnapshotResult.ErrorCode;
		if (!SnapshotResult.bSuccess)
		{
			Result.ErrorCode =
				EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode
					::CarrierSnapshotEnumerationFailed;
			Result.ErrorMessage = SnapshotResult.ErrorMessage;
			return Result;
		}

		if (SnapshotResult.Snapshot.bIsGoalkeeper)
		{
			continue;
		}

		for (const FName SkillId : SnapshotResult.Snapshot.SkillIds)
		{
			FMatchPlayCurrentAttackActionSelectionCandidateAvailability
				Candidate;
			Candidate.CarrierCardId = Placement.CardId;
			Candidate.SkillId = SkillId;
			Candidate.LegalityResult =
				FMatchPlayCurrentAttackActionSelectionLegalityEvaluator
					::Evaluate(
						BeforeState,
						MakeRequest(
							AttackSequence,
							RequestingSide,
							Placement.CardId,
							SkillId),
						SkillRules);

			if (Candidate.LegalityResult.bIsLegal)
			{
				Result.bCanSelectAnyAction = true;
			}
			else if (!Result.bHasFirstBlockingLegalityResult)
			{
				Result.bHasFirstBlockingLegalityResult = true;
				Result.FirstBlockingLegalityResult =
					Candidate.LegalityResult;
			}

			Result.Candidates.Add(MoveTemp(Candidate));
		}
	}

	Result.bQuerySucceeded = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackActionSelectionAvailabilityErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
