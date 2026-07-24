#include "MatchPlayGoalkeeperDeploymentWriter.h"

FMatchPlayGoalkeeperDeploymentWriterResult
FMatchPlayGoalkeeperDeploymentWriter::Deploy(
	const FMatchPlayState& BeforeState,
	const FMatchPlayGoalkeeperDeploymentRequest& Request)
{
	FMatchPlayGoalkeeperDeploymentWriterResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.LegalityResult =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			BeforeState,
			Request);

	if (!Result.LegalityResult.bIsLegal)
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperDeploymentWriterErrorCode::LegalityFailed;
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

FMatchPlayGoalkeeperDeploymentWriterResult
FMatchPlayGoalkeeperDeploymentWriter::FinalizeAfterRotation(
	const FMatchPlayState& BeforeState,
	const FMatchPlayGoalkeeperDeploymentRequest& Request,
	FMatchPlayGoalkeeperDeploymentWriterResult Result,
	const FMatchPlayDeploymentTurnRotationResult& RotationResult)
{
	Result.UnderlyingTurnRotationErrorCode = RotationResult.ErrorCode;
	if (!RotationResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperDeploymentWriterErrorCode
				::TurnRotationFailed;
		Result.ErrorMessage = RotationResult.ErrorMessage;
		return Result;
	}

	const FMatchPlayGoalkeeperUsageUpdateResult UsageUpdateResult =
		FMatchPlayGoalkeeperUsageStateResolver::MarkUsed(
			BeforeState.GoalkeeperUsageState,
			Request.RequestingSide);
	return FinalizeAfterUsageUpdate(
		BeforeState,
		Request,
		MoveTemp(Result),
		RotationResult,
		UsageUpdateResult);
}

FMatchPlayGoalkeeperDeploymentWriterResult
FMatchPlayGoalkeeperDeploymentWriter::FinalizeAfterUsageUpdate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayGoalkeeperDeploymentRequest& Request,
	FMatchPlayGoalkeeperDeploymentWriterResult Result,
	const FMatchPlayDeploymentTurnRotationResult& RotationResult,
	const FMatchPlayGoalkeeperUsageUpdateResult& UsageUpdateResult)
{
	Result.UnderlyingGoalkeeperUsageErrorCode =
		UsageUpdateResult.ErrorCode;
	if (!UsageUpdateResult.bSucceeded)
	{
		Result.ErrorCode =
			EMatchPlayGoalkeeperDeploymentWriterErrorCode
				::GoalkeeperUsageUpdateFailed;
		Result.ErrorMessage = UsageUpdateResult.ErrorMessage;
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayDeploymentPlacement Placement;
	Placement.PlayerSide = Request.RequestingSide;
	Placement.CardId = Request.CardId;
	Placement.SlotId = Request.SlotId;
	WorkingState.CurrentAttack.DeploymentPlacements.Add(Placement);
	WorkingState.GoalkeeperUsageState =
		UsageUpdateResult.UpdatedUsageState;
	WorkingState.CurrentAttack.bCurrentDefenseGoalkeeperActivated = true;
	WorkingState.CurrentAttack.CurrentLegalDeploymentSide =
		RotationResult.NextLegalDeploymentSide;

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSucceeded = true;
	Result.ErrorCode =
		EMatchPlayGoalkeeperDeploymentWriterErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
