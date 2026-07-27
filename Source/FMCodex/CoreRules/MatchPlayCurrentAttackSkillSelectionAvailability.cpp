#include "MatchPlayCurrentAttackSkillSelectionAvailability.h"

namespace MatchPlayCurrentAttackSkillSelectionAvailabilityImplementation
{
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

	void SetGlobalBlocker(
		FMatchPlayCurrentAttackSkillSelectionAvailabilityResult& Result)
	{
		Result.Candidates.Reset();
		Result.bCanSelectAnySkill = false;
		Result.bQuerySucceeded = false;
		Result.bHasGlobalBlockingLegalityResult = true;

		FMatchPlayCurrentAttackSkillSelectionLegalityResult&
			LegacyProjection =
				Result.GlobalBlockingLegalityResult;
		LegacyProjection.Request.AttackSequence =
			Result.AttackSequence;
		LegacyProjection.Request.RequestingSide =
			Result.RequestingSide;
		LegacyProjection.GlobalContextResult =
			Result.GlobalContextResult;
		LegacyProjection.ErrorCode =
			Result.GlobalContextResult.ErrorCode;
		LegacyProjection.ErrorMessage =
			Result.GlobalContextResult.ErrorMessage;
		LegacyProjection.SelectionStateValidationResult =
			Result.GlobalContextResult.SelectionStateValidationResult;
		LegacyProjection.FrozenCarrierCardId =
			Result.GlobalContextResult.FrozenCarrierCardId;
		LegacyProjection.FrozenMarkerCardId =
			Result.GlobalContextResult.FrozenMarkerCardId;
		LegacyProjection.MatchingFrozenCarrierPlacementCount =
			Result.GlobalContextResult
				.MatchingFrozenCarrierPlacementCount;
		LegacyProjection.MatchingFrozenMarkerPlacementCount =
			Result.GlobalContextResult
				.MatchingFrozenMarkerPlacementCount;
		LegacyProjection.FrozenCarrierPlacement =
			Result.GlobalContextResult.FrozenCarrierPlacement;
		LegacyProjection.FrozenMarkerPlacement =
			Result.GlobalContextResult.FrozenMarkerPlacement;
		LegacyProjection.CarrierSnapshotQueryResult =
			Result.GlobalContextResult.CarrierSnapshotQueryResult;
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
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackSkillSelectionGlobalContextQuery::Query(
			BeforeState,
			AttackSequence,
			RequestingSide,
			SkillRuleSet);
	Result.CarrierSnapshotQueryResult =
		Result.GlobalContextResult.CarrierSnapshotQueryResult;
	Result.SkillRuleSetValidationResult =
		Result.GlobalContextResult.SkillRuleSetValidationResult;
	if (!Result.GlobalContextResult.bSuccess)
	{
		SetGlobalBlocker(Result);
		return Result;
	}

	for (const FName SkillId :
		Result.GlobalContextResult.ResolvedCarrierSnapshot.SkillIds)
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
		if (Candidate.LegalityResult.bIsLegal)
		{
			Result.bCanSelectAnySkill = true;
		}
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	return Result;
}
