#include "AttackCardPlayFlow.h"

#include "MatchEndResolver.h"

namespace AttackCardPlayFlow
{
	void SetError(
		FAttackCardPlayFlowResult& Result,
		const EAttackCardPlayFlowErrorCode ErrorCode,
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
}

FAttackCardPlayFlowResult FAttackCardPlayFlow::ResolveAttackCardPlay(
	const FMatchRuntimeState& RuntimeState,
	const FMatchCardUsageState& MatchCardUsageState,
	const FName CardId,
	const bool bGoalScored)
{
	FAttackCardPlayFlowResult Result;
	Result.UpdatedRuntimeState = RuntimeState;
	Result.UpdatedMatchCardUsageState = MatchCardUsageState;
	Result.PlayedCardId = CardId;
	Result.bGoalScored = bGoalScored;

	const FMatchEndResolveResult InitialMatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(RuntimeState);
	if (!InitialMatchEndResult.bSuccess)
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Initial match end validation failed: %s"),
				*InitialMatchEndResult.ErrorMessage));
		return Result;
	}

	if (RuntimeState.PlayerAState.Score < 0
		|| RuntimeState.PlayerBState.Score < 0)
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match scores cannot be negative: PlayerA=%d, PlayerB=%d."),
				RuntimeState.PlayerAState.Score,
				RuntimeState.PlayerBState.Score));
		return Result;
	}

	Result.bMatchEnded = InitialMatchEndResult.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::MatchAlreadyEnded,
			TEXT("Cannot play a card for an attack after the match has ended."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		RuntimeState.CurrentAttackingPlayer;
	if (!AttackCardPlayFlow::IsPlayer(CurrentAttackingPlayer))
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.ActingPlayer = CurrentAttackingPlayer;

	if (CardId.IsNone())
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::InvalidCardId,
			TEXT("CardId cannot be None."));
		return Result;
	}

	Result.PlayCardResult =
		FPlayCardResolver::PlayCard(
			MatchCardUsageState,
			CurrentAttackingPlayer,
			CardId);
	if (!Result.PlayCardResult.bSuccess)
	{
		const EAttackCardPlayFlowErrorCode ErrorCode =
			Result.PlayCardResult.ErrorCode
				== EPlayCardResolveErrorCode::InvalidMatchCardUsageState
				? EAttackCardPlayFlowErrorCode::InvalidMatchCardUsageState
				: EAttackCardPlayFlowErrorCode::PlayCardResolveFailed;
		AttackCardPlayFlow::SetError(
			Result,
			ErrorCode,
			FString::Printf(
				TEXT("Play card resolution failed: %s"),
				*Result.PlayCardResult.ErrorMessage));
		return Result;
	}

	Result.AttackResolutionResult =
		FAttackResolutionFlow::ResolveAttack(
			RuntimeState,
			bGoalScored);
	if (!Result.AttackResolutionResult.bSuccess)
	{
		AttackCardPlayFlow::SetError(
			Result,
			EAttackCardPlayFlowErrorCode::AttackResolutionFlowFailed,
			FString::Printf(
				TEXT("Attack resolution flow failed: %s"),
				*Result.AttackResolutionResult.ErrorMessage));
		return Result;
	}

	Result.UpdatedMatchCardUsageState =
		Result.PlayCardResult.UpdatedMatchCardUsageState;
	Result.UpdatedRuntimeState =
		Result.AttackResolutionResult.UpdatedRuntimeState;
	Result.bMatchEnded =
		Result.AttackResolutionResult.bMatchEnded;
	Result.MatchResultType =
		Result.AttackResolutionResult.MatchResultType;
	Result.bSuccess = true;
	return Result;
}
