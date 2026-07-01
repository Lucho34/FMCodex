#include "MatchPlayExternalTurnController.h"

namespace
{
	EMatchPlayExternalTurnControllerCode MapSubmitCode(
		const EMatchPlaySubmitAttackFacadeCode Code)
	{
		switch (Code)
		{
		case EMatchPlaySubmitAttackFacadeCode::Submitted:
			return EMatchPlayExternalTurnControllerCode
				::SubmittedSuccessfully;
		case EMatchPlaySubmitAttackFacadeCode::AttackStepFailed:
			return EMatchPlayExternalTurnControllerCode::AttackStepFailed;
		case EMatchPlaySubmitAttackFacadeCode::SubmissionRejected:
		default:
			return EMatchPlayExternalTurnControllerCode
				::SubmissionRejected;
		}
	}
}

FMatchPlayExternalTurnControllerResult
FMatchPlayExternalTurnController::HandleAttackRequest(
	const FMatchPlayState& BeforeState,
	const FMatchPlayAttackRequest& Request)
{
	FMatchPlayExternalTurnControllerResult Result;
	Result.SubmitResult =
		FMatchPlaySubmitAttackFacade::Submit(BeforeState, Request);
	Result.ResultView =
		FMatchPlaySubmitAttackResultQuery::BuildView(Result.SubmitResult);

	Result.bHandled = true;
	Result.bSubmittedSuccessfully =
		Result.ResultView.bSubmittedSuccessfully;
	Result.Code = MapSubmitCode(Result.SubmitResult.Code);
	Result.Reason = Result.ResultView.Reason;
	Result.ErrorMessage = Result.ResultView.ErrorMessage;
	Result.BeforeState = Result.SubmitResult.BeforeState;
	Result.AfterState = Result.SubmitResult.AfterState;
	return Result;
}
