#include "MatchPlayDeploymentTurnRotation.h"

namespace MatchPlayDeploymentTurnRotationImplementation
{
	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer OtherSide(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}
}

FMatchPlayDeploymentTurnRotationResult
FMatchPlayDeploymentTurnRotation::Resolve(
	const EInitialTurnOrderPlayer CurrentAttackingPlayer,
	const EInitialTurnOrderPlayer ActingSide,
	const bool bAttackerDeploymentFinished,
	const bool bDefenderDeploymentFinished)
{
	FMatchPlayDeploymentTurnRotationResult Result;
	if (!MatchPlayDeploymentTurnRotationImplementation::IsPlayer(
			CurrentAttackingPlayer))
	{
		Result.ErrorCode =
			EMatchPlayDeploymentTurnRotationErrorCode
				::InvalidCurrentAttackingPlayer;
		Result.ErrorMessage =
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB for deployment turn rotation.");
		return Result;
	}

	if (!MatchPlayDeploymentTurnRotationImplementation::IsPlayer(
			ActingSide))
	{
		Result.ErrorCode =
			EMatchPlayDeploymentTurnRotationErrorCode::InvalidActingSide;
		Result.ErrorMessage =
			TEXT("ActingSide must be PlayerA or PlayerB for deployment turn rotation.");
		return Result;
	}

	if (bAttackerDeploymentFinished && bDefenderDeploymentFinished)
	{
		Result.bSuccess = true;
		Result.NextPhase = EMatchPlayCurrentAttackPhase::Resolution;
		Result.NextLegalDeploymentSide = EInitialTurnOrderPlayer::None;
		return Result;
	}

	const bool bActingSideIsAttacker =
		ActingSide == CurrentAttackingPlayer;
	const bool bOpponentAlreadyFinished = bActingSideIsAttacker
		? bDefenderDeploymentFinished
		: bAttackerDeploymentFinished;

	Result.bSuccess = true;
	Result.NextPhase = EMatchPlayCurrentAttackPhase::Deployment;
	Result.NextLegalDeploymentSide = bOpponentAlreadyFinished
		? ActingSide
		: MatchPlayDeploymentTurnRotationImplementation::OtherSide(
			ActingSide);
	return Result;
}
