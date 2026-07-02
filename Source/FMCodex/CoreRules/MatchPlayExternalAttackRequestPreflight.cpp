#include "MatchPlayExternalAttackRequestPreflight.h"

FMatchPlayExternalAttackRequestPreflightView
FMatchPlayExternalAttackRequestPreflight::Evaluate(
	const FMatchPlayState& State,
	const FMatchPlayAttackRequest& Request)
{
	FMatchPlayExternalAttackRequestPreflightView View;
	View.StateView = FMatchPlayExternalStateViewQuery::BuildView(State);
	View.SubmissionGateResult =
		FMatchPlaySubmissionGate::CanSubmit(State, Request);
	View.ValidationReport =
		View.SubmissionGateResult.RequestValidationReport;

	View.bStateReadyForAttackRequest =
		View.StateView.bCanSubmitAttackRequest;
	View.bRequestAcceptedByGate =
		View.SubmissionGateResult.bCanSubmit;
	View.bCanSubmit = View.SubmissionGateResult.bCanSubmit;
	View.CurrentAttackingPlayer =
		View.StateView.CurrentAttackingPlayer;
	View.RequestPlayer = Request.PlayerSide;
	View.CardId = Request.CardId;
	View.GateCode = View.SubmissionGateResult.Code;
	View.Reason = View.SubmissionGateResult.Reason;
	View.StateSummary = View.StateView.StatusSummary;

	if (!View.bCanSubmit)
	{
		View.CannotSubmitReason =
			View.SubmissionGateResult.ErrorMessage;
	}

	return View;
}
