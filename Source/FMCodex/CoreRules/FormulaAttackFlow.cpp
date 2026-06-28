#include "FormulaAttackFlow.h"

#include "MatchEndResolver.h"

namespace FormulaAttackFlow
{
	void SetError(
		FFormulaAttackFlowResult& Result,
		const EFormulaAttackFlowErrorCode ErrorCode,
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

	bool IsCompleteFinishingResult(
		const FFormulaResolutionResult& FormulaResult)
	{
		return FormulaResult.FormulaType == EFormulaType::Finishing
			&& FormulaResult.Winner != EFormulaWinner::None
			&& FormulaResult.bAttackEnded
			&& !FormulaResult.bContinueResolution;
	}
}

FFormulaAttackFlowResult FFormulaAttackFlow::ResolveFormulaAttack(
	const FMatchRuntimeState& RuntimeState,
	const FMatchCardUsageState& MatchCardUsageState,
	const FName CardId,
	const FFormulaResolverInput& FormulaInput)
{
	FFormulaAttackFlowResult Result;
	Result.UpdatedRuntimeState = RuntimeState;
	Result.UpdatedMatchCardUsageState = MatchCardUsageState;
	Result.PlayedCardId = CardId;

	const FMatchEndResolveResult InitialMatchEndResult =
		FMatchEndResolver::ResolveMatchEnd(RuntimeState);
	if (!InitialMatchEndResult.bSuccess)
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Initial match end validation failed: %s"),
				*InitialMatchEndResult.ErrorMessage));
		return Result;
	}

	if (RuntimeState.PlayerAState.Score < 0
		|| RuntimeState.PlayerBState.Score < 0)
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::InvalidRuntimeState,
			FString::Printf(
				TEXT("Match scores cannot be negative: PlayerA=%d, PlayerB=%d."),
				RuntimeState.PlayerAState.Score,
				RuntimeState.PlayerBState.Score));
		return Result;
	}

	Result.bMatchEnded = InitialMatchEndResult.bIsMatchEnded;
	if (Result.bMatchEnded)
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::MatchAlreadyEnded,
			TEXT("Cannot resolve a formula attack after the match has ended."));
		return Result;
	}

	if (CardId.IsNone())
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::InvalidCardId,
			TEXT("CardId cannot be None."));
		return Result;
	}

	if (!FormulaAttackFlow::IsPlayer(
		RuntimeState.CurrentAttackingPlayer))
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::InvalidRuntimeState,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}
	Result.ActingPlayer = RuntimeState.CurrentAttackingPlayer;

	const FPlayCardValidationResult PlayCardValidationResult =
		FPlayCardResolver::ValidateCanPlayCard(
			MatchCardUsageState,
			Result.ActingPlayer,
			CardId);
	Result.PlayCardValidationErrorCode =
		PlayCardValidationResult.ErrorCode;
	if (!PlayCardValidationResult.bSuccess)
	{
		const EFormulaAttackFlowErrorCode ErrorCode =
			PlayCardValidationResult.ErrorCode
				== EPlayCardResolveErrorCode::InvalidMatchCardUsageState
				? EFormulaAttackFlowErrorCode::InvalidMatchCardUsageState
				: EFormulaAttackFlowErrorCode::PlayCardValidationFailed;
		FormulaAttackFlow::SetError(
			Result,
			ErrorCode,
			FString::Printf(
				TEXT("Play card validation failed: %s"),
				*PlayCardValidationResult.ErrorMessage));
		return Result;
	}

	if (FormulaInput.FormulaType != EFormulaType::Finishing)
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::FormulaResolveFailed,
			TEXT("FormulaAttackFlow requires a Finishing formula that completes the current attack."));
		return Result;
	}

	Result.FormulaResult =
		UFormulaResolver::ResolveFormula(FormulaInput);
	if (!FormulaAttackFlow::IsCompleteFinishingResult(
		Result.FormulaResult))
	{
		FormulaAttackFlow::SetError(
			Result,
			EFormulaAttackFlowErrorCode::FormulaResolveFailed,
			TEXT("FormulaResolver did not return a complete Finishing result."));
		return Result;
	}

	Result.bGoalScored = Result.FormulaResult.bIsGoal;
	Result.AttackCardPlayResult =
		FAttackCardPlayFlow::ResolveAttackCardPlay(
			RuntimeState,
			MatchCardUsageState,
			CardId,
			Result.bGoalScored);
	if (!Result.AttackCardPlayResult.bSuccess)
	{
		const EFormulaAttackFlowErrorCode ErrorCode =
			Result.AttackCardPlayResult.ErrorCode
				== EAttackCardPlayFlowErrorCode::InvalidMatchCardUsageState
				? EFormulaAttackFlowErrorCode::InvalidMatchCardUsageState
				: EFormulaAttackFlowErrorCode::AttackCardPlayFlowFailed;
		FormulaAttackFlow::SetError(
			Result,
			ErrorCode,
			FString::Printf(
				TEXT("Attack card play flow failed: %s"),
				*Result.AttackCardPlayResult.ErrorMessage));
		return Result;
	}

	Result.UpdatedRuntimeState =
		Result.AttackCardPlayResult.UpdatedRuntimeState;
	Result.UpdatedMatchCardUsageState =
		Result.AttackCardPlayResult.UpdatedMatchCardUsageState;
	Result.ActingPlayer =
		Result.AttackCardPlayResult.ActingPlayer;
	Result.bMatchEnded =
		Result.AttackCardPlayResult.bMatchEnded;
	Result.MatchResultType =
		Result.AttackCardPlayResult.MatchResultType;
	Result.bSuccess = true;
	return Result;
}
