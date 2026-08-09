#include "MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter.h"

FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterResult
FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter::Select(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionRequest&
		Request,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterResult
		Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionLegalityEvaluator
			::Evaluate(BeforeState, Request, SkillRuleSet);
	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
				::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	WorkingState.CurrentAttack.ResolutionSession
		.ThroughBallOneOnOneShotChoice = Request.Choice;
	Result.CandidateStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			WorkingState);
	if (!Result.CandidateStateValidationResult.bIsCanonical)
	{
		Result.ErrorCode =
			EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
				::InvalidCandidateState;
		Result.ErrorMessage =
			Result.CandidateStateValidationResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterErrorCode
			::None;
	return Result;
}
