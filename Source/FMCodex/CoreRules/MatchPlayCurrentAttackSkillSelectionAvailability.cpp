#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"

namespace MatchPlayCurrentAttackSkillSelectionAvailabilityImplementation
{
	const FName ProbeSkillId(
		TEXT("FMCodex.Internal.SkillAvailabilityProbe"));

	FMatchPlayCurrentAttackSkillSelectionRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName SkillId)
	{
		FMatchPlayCurrentAttackSkillSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.SkillId = SkillId;
		return Request;
	}

	bool IsGlobalBlocker(
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult& Result)
	{
		switch (Result.ErrorCode)
		{
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::NoCurrentAttack:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::AttackSequenceMismatch:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::CurrentAttackNotInResolution:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentDefendingPlayer:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidSelectionState:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::WrongSelectionStage:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidRequestingSide:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidFrozenCarrierCardId:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidFrozenMarkerCardId:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::FrozenCarrierNotDeployed:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::FrozenCarrierDeploymentAmbiguous:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::FrozenMarkerNotDeployed:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::FrozenMarkerDeploymentAmbiguous:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::CarrierSnapshotQueryFailed:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::DuplicateCarrierSkillId:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidSkillRuleSet:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::SkillRuleAmbiguous:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::ParticipantRequirementResolutionFailed:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentAttackActionPoint:
			return true;

		case EMatchPlayCurrentAttackSkillSelectionErrorCode::None:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidSkillId:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::CarrierDoesNotOwnSkill:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::SkillRuleNotFound:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::UnsupportedSkillRuleType:
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::ActionPointOutsideSkillRange:
		default:
			return false;
		}
	}

	void SetGlobalBlocker(
		FMatchPlayCurrentAttackSkillSelectionAvailabilityResult& Result,
		const FMatchPlayCurrentAttackSkillSelectionLegalityResult&
			LegalityResult)
	{
		Result.Candidates.Reset();
		Result.bCanSelectAnySkill = false;
		Result.bQuerySucceeded = false;
		Result.bHasGlobalBlockingLegalityResult = true;
		Result.GlobalBlockingLegalityResult = LegalityResult;
	}
}

FMatchPlayCurrentAttackSkillSelectionAvailabilityResult
FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide,
	const FSkillRuleSnapshotSet& SkillRuleSet)
{
	using namespace
		MatchPlayCurrentAttackSkillSelectionAvailabilityImplementation;

	FMatchPlayCurrentAttackSkillSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	const FMatchPlayCurrentAttackSkillSelectionLegalityResult
		ProbeResult =
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(
					BeforeState,
					SkillRuleSet,
					MakeRequest(
						AttackSequence,
						RequestingSide,
						ProbeSkillId));
	if (!ProbeResult.bIsLegal && IsGlobalBlocker(ProbeResult))
	{
		SetGlobalBlocker(Result, ProbeResult);
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	Result.CarrierSnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BeforeState.CardSnapshotAuthority,
			CurrentAttackingPlayer,
			BeforeState.CurrentAttack.ActionPreparation.CarrierCardId);
	if (!Result.CarrierSnapshotQueryResult.bSuccess)
	{
		SetGlobalBlocker(Result, ProbeResult);
		return Result;
	}

	Result.SkillRuleSetValidationResult =
		FSkillRuleSnapshotValidator::Validate(SkillRuleSet);
	if (!Result.SkillRuleSetValidationResult.bSuccess)
	{
		FMatchPlayCurrentAttackSkillSelectionLegalityResult
			RuleSetBlocker = ProbeResult;
		RuleSetBlocker.ErrorCode =
			Result.SkillRuleSetValidationResult.ErrorCode
					== ESkillRuleSnapshotValidationErrorCode
						::DuplicateSkillId
				? EMatchPlayCurrentAttackSkillSelectionErrorCode
					::SkillRuleAmbiguous
				: EMatchPlayCurrentAttackSkillSelectionErrorCode
					::InvalidSkillRuleSet;
		RuleSetBlocker.ErrorMessage =
			Result.SkillRuleSetValidationResult.ErrorMessage;
		SetGlobalBlocker(Result, RuleSetBlocker);
		return Result;
	}

	for (const FName SkillId :
		Result.CarrierSnapshotQueryResult.Snapshot.SkillIds)
	{
		FMatchPlayCurrentAttackSkillSelectionCandidateAvailability
			Candidate;
		Candidate.SkillId = SkillId;
		Candidate.LegalityResult =
			FMatchPlayCurrentAttackSkillSelectionLegalityEvaluator
				::Evaluate(
					BeforeState,
					SkillRuleSet,
					MakeRequest(
						AttackSequence,
						RequestingSide,
						SkillId));
		if (IsGlobalBlocker(Candidate.LegalityResult))
		{
			SetGlobalBlocker(Result, Candidate.LegalityResult);
			return Result;
		}
		if (Candidate.LegalityResult.bIsLegal)
		{
			Result.bCanSelectAnySkill = true;
		}
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	return Result;
}
