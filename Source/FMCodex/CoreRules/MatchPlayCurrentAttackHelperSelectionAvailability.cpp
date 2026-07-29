#include "MatchPlayCurrentAttackHelperSelectionAvailability.h"

FMatchPlayCurrentAttackHelperSelectionAvailabilityResult
FMatchPlayCurrentAttackHelperSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	FMatchPlayCurrentAttackHelperSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackHelperSelectionGlobalContextQuery::Query(
			BeforeState,
			AttackSequence,
			RequestingSide);
	if (!Result.GlobalContextResult.bSuccess)
	{
		Result.ErrorCode = Result.GlobalContextResult.ErrorCode;
		Result.ErrorMessage = Result.GlobalContextResult.ErrorMessage;
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		Result.GlobalContextResult.DefendingPlayerPlacements)
	{
		FMatchPlayCurrentAttackHelperSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.HelperCardId = Placement.CardId;

		FMatchPlayCurrentAttackHelperSelectionCandidateAvailability
			Candidate;
		Candidate.HelperCardId = Placement.CardId;
		Candidate.LegalityResult =
			FMatchPlayCurrentAttackHelperSelectionLegalityEvaluator
				::EvaluateWithGlobalContext(
					BeforeState,
					Request,
					Result.GlobalContextResult);
		Result.bCanSelectAnyHelper =
			Result.bCanSelectAnyHelper
				|| Candidate.LegalityResult.bSuccess;
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	return Result;
}
