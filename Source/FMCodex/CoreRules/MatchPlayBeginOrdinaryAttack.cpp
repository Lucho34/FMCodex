#include "MatchPlayBeginOrdinaryAttack.h"

namespace
{
	void SetError(
		FMatchPlayBeginOrdinaryAttackResult& Result,
		const EMatchPlayBeginOrdinaryAttackErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsAttackCountStateValid(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.UsedAttackCount >= 0
			&& PlayerState.UsedAttackCount <= PlayerState.TotalAttackCount;
	}

	int32 GetRemainingAttackCount(const FPlayerRuntimeState& PlayerState)
	{
		return PlayerState.TotalAttackCount - PlayerState.UsedAttackCount;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}
}

FMatchPlayBeginOrdinaryAttackResult
FMatchPlayBeginOrdinaryAttack::Begin(
	const FMatchPlayState& BeforeState,
	const int32 ActionPoint)
{
	FMatchPlayBeginOrdinaryAttackResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	Result.ActionPoint = ActionPoint;

	const FMatchRuntimeState& RuntimeState = BeforeState.RuntimeState;
	if (!RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before beginning an attack."));
		return Result;
	}

	if (BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::CurrentAttackAlreadyActive,
			TEXT("Cannot begin an ordinary attack while another current attack is active."));
		return Result;
	}

	if (!IsAttackCountStateValid(RuntimeState.PlayerAState))
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::InvalidPlayerAAttackCountState,
			TEXT("PlayerA attack counts must satisfy 0 <= used <= total."));
		return Result;
	}

	if (!IsAttackCountStateValid(RuntimeState.PlayerBState))
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::InvalidPlayerBAttackCountState,
			TEXT("PlayerB attack counts must satisfy 0 <= used <= total."));
		return Result;
	}

	const int32 PlayerARemainingAttackCount =
		GetRemainingAttackCount(RuntimeState.PlayerAState);
	const int32 PlayerBRemainingAttackCount =
		GetRemainingAttackCount(RuntimeState.PlayerBState);
	if (PlayerARemainingAttackCount == 0
		&& PlayerBRemainingAttackCount == 0)
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode::MatchAlreadyEnded,
			TEXT("Cannot begin an ordinary attack after all attack opportunities are exhausted."));
		return Result;
	}

	if (!IsPlayer(RuntimeState.CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	const int32 CurrentAttackerRemainingAttackCount =
		RuntimeState.CurrentAttackingPlayer
			== EInitialTurnOrderPlayer::PlayerA
			? PlayerARemainingAttackCount
			: PlayerBRemainingAttackCount;
	if (CurrentAttackerRemainingAttackCount == 0)
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode
				::CurrentAttackerHasNoRemainingAttackOpportunity,
			TEXT("Current attacker has no remaining attack opportunity."));
		return Result;
	}

	if (ActionPoint < 2 || ActionPoint > 8)
	{
		SetError(
			Result,
			EMatchPlayBeginOrdinaryAttackErrorCode::InvalidActionPoint,
			TEXT("Ordinary attack ActionPoint must be between 2 and 8."));
		return Result;
	}

	const int64 AttackSequence =
		static_cast<int64>(RuntimeState.PlayerAState.UsedAttackCount)
		+ static_cast<int64>(RuntimeState.PlayerBState.UsedAttackCount)
		+ 1;

	FMatchPlayCurrentAttackState CurrentAttack;
	CurrentAttack.Phase = EMatchPlayCurrentAttackPhase::Deployment;
	CurrentAttack.AttackSequence = AttackSequence;
	CurrentAttack.ActionPoint = ActionPoint;
	CurrentAttack.CurrentLegalDeploymentSide =
		RuntimeState.CurrentAttackingPlayer;
	CurrentAttack.bAttackerDeploymentFinished = false;
	CurrentAttack.bDefenderDeploymentFinished = false;
	CurrentAttack.DeploymentPlacements.Reset();
	CurrentAttack.bCurrentDefenseGoalkeeperActivated = false;

	Result.AfterState.CurrentAttack = MoveTemp(CurrentAttack);
	Result.AfterState.bHasCurrentAttack = true;
	Result.bSuccess = true;
	Result.ErrorCode = EMatchPlayBeginOrdinaryAttackErrorCode::None;
	return Result;
}
