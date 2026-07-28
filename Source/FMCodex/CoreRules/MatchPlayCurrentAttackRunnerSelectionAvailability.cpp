#include "MatchPlayCurrentAttackRunnerSelectionAvailability.h"

namespace MatchPlayCurrentAttackRunnerSelectionAvailabilityImplementation
{
	FMatchPlayCurrentAttackRunnerSelectionRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName RunnerCardId)
	{
		FMatchPlayCurrentAttackRunnerSelectionRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.RunnerCardId = RunnerCardId;
		return Request;
	}
}

FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult
FMatchPlayCurrentAttackRunnerSelectionAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	using namespace
		MatchPlayCurrentAttackRunnerSelectionAvailabilityImplementation;

	FMatchPlayCurrentAttackRunnerSelectionAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;
	Result.GlobalContextResult =
		FMatchPlayCurrentAttackRunnerSelectionGlobalContextQuery::Query(
			BeforeState,
			AttackSequence,
			RequestingSide);
	if (!Result.GlobalContextResult.bSuccess)
	{
		Result.ErrorCode =
			Result.GlobalContextResult.ErrorCode;
		Result.ErrorMessage =
			Result.GlobalContextResult.ErrorMessage;
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		BeforeState.CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide
			!= Result.GlobalContextResult.CurrentAttackingPlayer)
		{
			continue;
		}

		FMatchPlayCurrentAttackRunnerSelectionCandidateAvailability
			Candidate;
		Candidate.RunnerCardId = Placement.CardId;
		Candidate.LegalityResult =
			FMatchPlayCurrentAttackRunnerSelectionLegalityEvaluator
				::Evaluate(
					BeforeState,
					MakeRequest(
						AttackSequence,
						RequestingSide,
						Placement.CardId));
		if (Candidate.LegalityResult.bIsLegal)
		{
			Result.bCanSelectAnyRunner = true;
		}
		Result.Candidates.Add(MoveTemp(Candidate));
	}

	Result.bQuerySucceeded = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackRunnerSelectionErrorCode::None;
	return Result;
}
