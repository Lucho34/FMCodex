#include "AttackResolutionFlow.h"

namespace AttackResolutionFlow
{
	void SetError(
		FAttackResolutionFlowResult& Result,
		const EAttackResolutionFlowErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	int32 GetCurrentPlayerRemainingAttackCount(
		const FMatchRuntimeState& RuntimeState)
	{
		const FPlayerRuntimeState& CurrentPlayerState =
			RuntimeState.CurrentAttackingPlayer
				== EInitialTurnOrderPlayer::PlayerA
				? RuntimeState.PlayerAState
				: RuntimeState.PlayerBState;
		return CurrentPlayerState.TotalAttackCount
			- CurrentPlayerState.UsedAttackCount;
	}
}

FAttackResolutionFlowResult FAttackResolutionFlow::ResolveAttack(
	const FMatchRuntimeState& RuntimeState,
	const bool bGoalScored)
{
	FAttackResolutionFlowResult Result;
	Result.UpdatedRuntimeState = RuntimeState;
	Result.bGoalScored = bGoalScored;

	const FMatchEndResolveResult InitialMatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(RuntimeState);
	if (!InitialMatchEndResult.bSuccess)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Initial match end validation failed: %s"),
				*InitialMatchEndResult.ErrorMessage));
		return Result;
	}

	if (RuntimeState.PlayerAState.Score < 0
		|| RuntimeState.PlayerBState.Score < 0)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match scores cannot be negative: PlayerA=%d, PlayerB=%d."),
				RuntimeState.PlayerAState.Score,
				RuntimeState.PlayerBState.Score));
		return Result;
	}

	Result.bMatchEnded = InitialMatchEndResult.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::MatchAlreadyEnded,
			TEXT("Cannot resolve another attack after the match has ended."));
		return Result;
	}

	if (!AttackResolutionFlow::IsPlayer(
		RuntimeState.CurrentAttackingPlayer))
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::InvalidRuntimeState,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (AttackResolutionFlow::GetCurrentPlayerRemainingAttackCount(
		RuntimeState) <= 0)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::NoAttackOpportunity,
			TEXT("The current attacking player has no attack opportunity to consume."));
		return Result;
	}

	FMatchRuntimeState WorkingState = RuntimeState;
	if (bGoalScored)
	{
		const FGoalResolveResult GoalResult =
			FGoalResolver::RecordGoal(
				WorkingState,
				WorkingState.CurrentAttackingPlayer);
		if (!GoalResult.bSuccess)
		{
			AttackResolutionFlow::SetError(
				Result,
				EAttackResolutionFlowErrorCode::GoalResolveFailed,
				FString::Printf(
					TEXT("Goal resolution failed: %s"),
					*GoalResult.ErrorMessage));
			return Result;
		}

		Result.ScoringSide = GoalResult.ScoringSide;
		WorkingState = GoalResult.UpdatedRuntimeState;
	}

	const FAttackOpportunityResolveResult AttackOpportunityResult =
		FAttackOpportunityResolver::ConsumeCurrentAttackOpportunity(
			WorkingState);
	if (!AttackOpportunityResult.bSuccess)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::AttackOpportunityResolveFailed,
			FString::Printf(
				TEXT("Attack opportunity resolution failed: %s"),
				*FString::Join(
					AttackOpportunityResult.ErrorMessages,
					TEXT("; "))));
		return Result;
	}
	WorkingState = AttackOpportunityResult.UpdatedRuntimeState;

	const FMatchEndResolveResult FinalMatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(WorkingState);
	if (!FinalMatchEndResult.bSuccess)
	{
		AttackResolutionFlow::SetError(
			Result,
			EAttackResolutionFlowErrorCode::MatchEndResolveFailed,
			FString::Printf(
				TEXT("Final match end resolution failed: %s"),
				*FinalMatchEndResult.ErrorMessage));
		return Result;
	}

	Result.bMatchEnded = FinalMatchEndResult.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		const FMatchResultResolveResult MatchResult =
			FMatchResultResolver::ResolveMatchResult(WorkingState);
		if (!MatchResult.bSuccess)
		{
			AttackResolutionFlow::SetError(
				Result,
				EAttackResolutionFlowErrorCode::MatchResultResolveFailed,
				FString::Printf(
					TEXT("Match result resolution failed: %s"),
					*MatchResult.ErrorMessage));
			return Result;
		}
		Result.MatchResultType = MatchResult.ResultType;
	}

	Result.UpdatedRuntimeState = WorkingState;
	Result.bSuccess = true;
	return Result;
}
