#include "MatchPlaySubmitAttackResultQuery.h"

FMatchPlaySubmitAttackResultView
FMatchPlaySubmitAttackResultQuery::BuildView(
	const FMatchPlaySubmitAttackFacadeResult& Result)
{
	FMatchPlaySubmitAttackResultView View;
	View.bSubmittedSuccessfully =
		Result.bSuccess
		&& Result.Code == EMatchPlaySubmitAttackFacadeCode::Submitted;
	View.bWasRejectedBeforeExecution =
		Result.Code
			== EMatchPlaySubmitAttackFacadeCode::SubmissionRejected;
	View.bAttackStepFailed =
		Result.Code == EMatchPlaySubmitAttackFacadeCode::AttackStepFailed;
	View.SubmitCode = Result.Code;
	View.Reason = Result.Reason;
	View.ErrorMessage = Result.ErrorMessage;
	View.ExecutionSummary = Result.ExecutionSummary;
	View.bMatchEnded = Result.ExecutionSummary.bMatchEnded;

	View.BeforeScore.PlayerAScore =
		Result.BeforeState.RuntimeState.PlayerAState.Score;
	View.BeforeScore.PlayerBScore =
		Result.BeforeState.RuntimeState.PlayerBState.Score;
	View.AfterScore.PlayerAScore =
		Result.AfterState.RuntimeState.PlayerAState.Score;
	View.AfterScore.PlayerBScore =
		Result.AfterState.RuntimeState.PlayerBState.Score;
	View.bScoreChanged =
		View.BeforeScore.PlayerAScore != View.AfterScore.PlayerAScore
		|| View.BeforeScore.PlayerBScore != View.AfterScore.PlayerBScore;

	View.BeforeCurrentAttacker =
		Result.BeforeState.RuntimeState.CurrentAttackingPlayer;
	View.AfterCurrentAttacker =
		Result.AfterState.RuntimeState.CurrentAttackingPlayer;
	return View;
}
