#include "MatchPlayFinishDeployment.h"

#include "MatchPlayDeploymentTurnRotation.h"

namespace MatchPlayFinishDeployment
{
	void SetError(
		FMatchPlayFinishDeploymentResult& Result,
		const EMatchPlayFinishDeploymentErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}
}

FMatchPlayFinishDeploymentResult FMatchPlayFinishDeployment::Finish(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide)
{
	FMatchPlayFinishDeploymentResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before finishing deployment."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode::NoCurrentAttack,
			TEXT("Cannot finish deployment without an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (AttackSequence != CurrentAttack.AttackSequence)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode::AttackSequenceMismatch,
			TEXT("Finish deployment attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase
		!= EMatchPlayCurrentAttackPhase::Deployment)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::CurrentAttackNotInDeployment,
			TEXT("Current attack must be in Deployment phase to finish deployment."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!MatchPlayFinishDeployment::IsPlayer(CurrentAttackingPlayer))
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!MatchPlayFinishDeployment::IsPlayer(RequestingSide))
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (!MatchPlayFinishDeployment::IsPlayer(
		CurrentAttack.CurrentLegalDeploymentSide))
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::InvalidCurrentLegalDeploymentSide,
			TEXT("CurrentLegalDeploymentSide must be PlayerA or PlayerB during Deployment."));
		return Result;
	}

	if (RequestingSide != CurrentAttack.CurrentLegalDeploymentSide)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::RequestingSideNotCurrentLegalDeploymentSide,
			TEXT("Only the current legal deployment side can finish deployment."));
		return Result;
	}

	if (CurrentAttack.bAttackerDeploymentFinished
		&& CurrentAttack.bDefenderDeploymentFinished)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::InvalidDeploymentFinishedState,
			TEXT("Deployment phase cannot have both deployment-finished flags set."));
		return Result;
	}

	const bool bRequestingSideIsAttacker =
		RequestingSide == CurrentAttackingPlayer;
	const bool bRequestingSideAlreadyFinished =
		bRequestingSideIsAttacker
			? CurrentAttack.bAttackerDeploymentFinished
			: CurrentAttack.bDefenderDeploymentFinished;
	if (bRequestingSideAlreadyFinished)
	{
		MatchPlayFinishDeployment::SetError(
			Result,
			EMatchPlayFinishDeploymentErrorCode
				::RequestingSideAlreadyFinished,
			TEXT("Requesting side has already finished deployment for the current attack."));
		return Result;
	}

	FMatchPlayState WorkingState = BeforeState;
	FMatchPlayCurrentAttackState& WorkingAttack =
		WorkingState.CurrentAttack;
	if (bRequestingSideIsAttacker)
	{
		WorkingAttack.bAttackerDeploymentFinished = true;
	}
	else
	{
		WorkingAttack.bDefenderDeploymentFinished = true;
	}

	const FMatchPlayDeploymentTurnRotationResult RotationResult =
		FMatchPlayDeploymentTurnRotation::Resolve(
			CurrentAttackingPlayer,
			RequestingSide,
			WorkingAttack.bAttackerDeploymentFinished,
			WorkingAttack.bDefenderDeploymentFinished);
	if (!RotationResult.bSuccess)
	{
		const EMatchPlayFinishDeploymentErrorCode ErrorCode =
			RotationResult.ErrorCode
				== EMatchPlayDeploymentTurnRotationErrorCode
					::InvalidCurrentAttackingPlayer
					? EMatchPlayFinishDeploymentErrorCode
						::InvalidCurrentAttackingPlayer
					: EMatchPlayFinishDeploymentErrorCode
						::InvalidRequestingSide;
		MatchPlayFinishDeployment::SetError(
			Result,
			ErrorCode,
			*RotationResult.ErrorMessage);
		return Result;
	}

	WorkingAttack.Phase = RotationResult.NextPhase;
	WorkingAttack.CurrentLegalDeploymentSide =
		RotationResult.NextLegalDeploymentSide;

	Result.AfterState = MoveTemp(WorkingState);
	Result.bSuccess = true;
	Result.ErrorCode = EMatchPlayFinishDeploymentErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
