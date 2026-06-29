#include "MatchPlayAttackFlow.h"

FMatchPlayAttackFlowResult FMatchPlayAttackFlow::ResolveMatchPlayAttack(
	const FMatchPlayState& CurrentMatchPlayState,
	const FName CardId,
	const FFormulaResolverInput& FormulaInput)
{
	FMatchPlayAttackFlowResult Result;
	Result.PlayedCardId = CardId;

	const FFormulaAttackFlowResult FormulaAttackResult =
		FFormulaAttackFlow::ResolveFormulaAttack(
			CurrentMatchPlayState.RuntimeState,
			CurrentMatchPlayState.CardUsageState,
			CardId,
			FormulaInput);
	if (!FormulaAttackResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayAttackFlowErrorCode::FormulaAttackFlowFailed;
		Result.UnderlyingFormulaAttackFlowErrorCode =
			FormulaAttackResult.ErrorCode;
		Result.ErrorMessage = FormulaAttackResult.ErrorMessage;
		return Result;
	}

	Result.UpdatedMatchPlayState = FMatchPlayState::Create(
		FormulaAttackResult.UpdatedRuntimeState,
		FormulaAttackResult.UpdatedMatchCardUsageState);
	Result.ActingPlayer = FormulaAttackResult.ActingPlayer;
	Result.bAttackResolved = true;
	Result.bGoalScored = FormulaAttackResult.bGoalScored;
	Result.bMatchEnded = FormulaAttackResult.bMatchEnded;
	Result.MatchResultType = FormulaAttackResult.MatchResultType;
	Result.FormulaResult = FormulaAttackResult.FormulaResult;
	Result.bSuccess = true;
	return Result;
}
