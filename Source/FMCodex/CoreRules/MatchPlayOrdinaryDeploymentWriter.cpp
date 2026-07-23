#include "MatchPlayOrdinaryDeploymentWriter.h"

FMatchPlayOrdinaryDeploymentWriterResult
FMatchPlayOrdinaryDeploymentWriter::Deploy(
	const FMatchPlayState& BeforeState,
	const FMatchPlayOrdinaryDeploymentRequest& Request)
{
	FMatchPlayOrdinaryDeploymentWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			BeforeState,
			Request);

	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayOrdinaryDeploymentWriterErrorCode::LegalityFailed;
		Result.ErrorMessage = Result.LegalityResult.ErrorMessage;
		return Result;
	}

	const FMatchPlayDeploymentTurnRotationResult RotationResult =
		FMatchPlayDeploymentTurnRotation::Resolve(
			BeforeState.RuntimeState.CurrentAttackingPlayer,
			Request.RequestingSide,
			BeforeState.CurrentAttack.bAttackerDeploymentFinished,
			BeforeState.CurrentAttack.bDefenderDeploymentFinished);
	return FinalizeAfterRotation(
		BeforeState,
		Request,
		MoveTemp(Result),
		RotationResult);
}

FMatchPlayOrdinaryDeploymentWriterResult
FMatchPlayOrdinaryDeploymentWriter::FinalizeAfterRotation(
	const FMatchPlayState& BeforeState,
	const FMatchPlayOrdinaryDeploymentRequest& Request,
	FMatchPlayOrdinaryDeploymentWriterResult Result,
	const FMatchPlayDeploymentTurnRotationResult& RotationResult)
{
	Result.UnderlyingTurnRotationErrorCode = RotationResult.ErrorCode;
	if (!RotationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayOrdinaryDeploymentWriterErrorCode
				::TurnRotationFailed;
		Result.ErrorMessage = RotationResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = Request.RequestingSide;
	Placement.CardId = Request.CardId;
	Placement.SlotId = Request.SlotId;
	WorkingState.CurrentAttack.DeploymentPlacements.Add(Placement);
	WorkingState.CurrentAttack.Phase = RotationResult.NextPhase;
	WorkingState.CurrentAttack.CurrentLegalDeploymentSide =
		RotationResult.NextLegalDeploymentSide;

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayOrdinaryDeploymentWriterErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
