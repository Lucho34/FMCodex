#include "MatchPlaySubmitAttackFacade.h"

namespace
{
	void SetFailure(
		FMatchPlaySubmitAttackFacadeResult& Result,
		const EMatchPlaySubmitAttackFacadeCode Code,
		const FString& ErrorMessage)
	{
		Result.bSuccess = false;
		Result.Code = Code;
		Result.Reason = ErrorMessage;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlaySubmitAttackFacadeResult FMatchPlaySubmitAttackFacade::Submit(
	const FMatchPlayState& BeforeState,
	const FMatchPlayAttackRequest& Request)
{
	FMatchPlaySubmitAttackFacadeResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.SubmissionGateResult =
		FMatchPlaySubmissionGate::CanSubmit(BeforeState, Request);

	if (!Result.SubmissionGateResult.bCanSubmit)
	{
		SetFailure(
			Result,
			EMatchPlaySubmitAttackFacadeCode::SubmissionRejected,
			Result.SubmissionGateResult.ErrorMessage);
		return Result;
	}

	Result.AttackStepResult =
		FMatchPlayAttackStep::ExecuteAttackStep(BeforeState, Request);
	Result.ExecutionSummary = Result.AttackStepResult.ExecutionSummary;

	if (!Result.AttackStepResult.bSuccess)
	{
		SetFailure(
			Result,
			EMatchPlaySubmitAttackFacadeCode::AttackStepFailed,
			Result.AttackStepResult.ErrorMessage);
		return Result;
	}

	Result.bSuccess = true;
	Result.Code = EMatchPlaySubmitAttackFacadeCode::Submitted;
	Result.Reason = TEXT("Attack request was submitted and executed.");
	Result.ErrorMessage.Reset();
	Result.AfterState =
		Result.AttackStepResult.ExecutionResult.UpdatedMatchPlayState;
	return Result;
}
