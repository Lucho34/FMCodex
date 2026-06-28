#include "GoalResolver.h"

namespace GoalResolver
{
	void SetError(
		FGoalResolveResult& Result,
		const EGoalResolveErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FGoalResolveResult FGoalResolver::RecordGoal(
	const FMatchRuntimeState& RuntimeState,
	const EInitialTurnOrderPlayer ScoringSide)
{
	FGoalResolveResult Result;
	Result.UpdatedRuntimeState = RuntimeState;
	Result.ScoringSide = ScoringSide;
	Result.PlayerAScoreBefore = RuntimeState.PlayerAState.Score;
	Result.PlayerBScoreBefore = RuntimeState.PlayerBState.Score;
	Result.PlayerAScoreAfter = Result.PlayerAScoreBefore;
	Result.PlayerBScoreAfter = Result.PlayerBScoreBefore;

	const FMatchEndResolveResult MatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(RuntimeState);
	if (!MatchEndResult.bSuccess)
	{
		GoalResolver::SetError(
			Result,
			EGoalResolveErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match end validation failed: %s"),
				*MatchEndResult.ErrorMessage));
		return Result;
	}

	if (Result.PlayerAScoreBefore < 0 || Result.PlayerBScoreBefore < 0)
	{
		GoalResolver::SetError(
			Result,
			EGoalResolveErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match scores cannot be negative: PlayerA=%d, PlayerB=%d."),
				Result.PlayerAScoreBefore,
				Result.PlayerBScoreBefore));
		return Result;
	}

	if (ScoringSide != EInitialTurnOrderPlayer::PlayerA
		&& ScoringSide != EInitialTurnOrderPlayer::PlayerB)
	{
		GoalResolver::SetError(
			Result,
			EGoalResolveErrorCode::InvalidScoringSide,
			TEXT("ScoringSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (MatchEndResult.bIsMatchEnded)
	{
		GoalResolver::SetError(
			Result,
			EGoalResolveErrorCode::MatchAlreadyEnded,
			TEXT("Cannot record a goal after all attack opportunities have been consumed."));
		return Result;
	}

	FPlayerRuntimeState& ScoringPlayerState =
		ScoringSide == EInitialTurnOrderPlayer::PlayerA
			? Result.UpdatedRuntimeState.PlayerAState
			: Result.UpdatedRuntimeState.PlayerBState;
	if (ScoringPlayerState.Score == MAX_int32)
	{
		GoalResolver::SetError(
			Result,
			EGoalResolveErrorCode::InvalidRuntimeState,
			TEXT("The scoring side score cannot be incremented beyond the int32 range."));
		return Result;
	}

	++ScoringPlayerState.Score;
	Result.PlayerAScoreAfter =
		Result.UpdatedRuntimeState.PlayerAState.Score;
	Result.PlayerBScoreAfter =
		Result.UpdatedRuntimeState.PlayerBState.Score;
	Result.bSuccess = true;
	return Result;
}
