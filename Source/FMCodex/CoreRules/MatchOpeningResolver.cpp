#include "MatchOpeningResolver.h"

namespace MatchOpeningResolver
{
	void AddStageFailure(
		FMatchOpeningResolveResult& Result,
		const EMatchOpeningResolveErrorCode ErrorCode,
		const TCHAR* StageLabel,
		const TArray<FString>& StageErrors)
	{
		Result.ErrorCodes.Add(ErrorCode);

		if (StageErrors.IsEmpty())
		{
			Result.ErrorMessages.Add(FString::Printf(
				TEXT("%s failed without a detailed error."),
				StageLabel));
			return;
		}

		for (const FString& StageError : StageErrors)
		{
			Result.ErrorMessages.Add(FString::Printf(
				TEXT("%s: %s"),
				StageLabel,
				*StageError));
		}
	}
}

FMatchOpeningResolveResult FMatchOpeningResolver::ResolveMatchOpening(
	const FMatchOpeningResolveInput& Input)
{
	FMatchOpeningResolveResult Result;

	Result.MatchInitializationResult = FMatchInitializer::InitializeMatch(
		Input.PlayerADeck,
		Input.PlayerBDeck);
	if (!Result.MatchInitializationResult.bSuccess)
	{
		MatchOpeningResolver::AddStageFailure(
			Result,
			EMatchOpeningResolveErrorCode::MatchInitializationFailed,
			TEXT("Match initialization"),
			Result.MatchInitializationResult.ErrorMessages);
		return Result;
	}

	const int32 PlayerAInitialDeckRarityScore =
		Result.MatchInitializationResult.PlayerADeckValidation.InitialDeckRarityScore;
	const int32 PlayerBInitialDeckRarityScore =
		Result.MatchInitializationResult.PlayerBDeckValidation.InitialDeckRarityScore;

	Result.AttackCountResult = FAttackCountResolver::ResolveAttackCounts(
		PlayerAInitialDeckRarityScore,
		PlayerBInitialDeckRarityScore,
		Input.PlayerAAttackCountD6Roll,
		Input.PlayerBAttackCountD6Roll);
	if (!Result.AttackCountResult.bSuccess)
	{
		MatchOpeningResolver::AddStageFailure(
			Result,
			EMatchOpeningResolveErrorCode::AttackCountResolutionFailed,
			TEXT("Attack count resolution"),
			Result.AttackCountResult.ErrorMessages);
		return Result;
	}

	Result.InitialTurnOrderResult = FInitialTurnOrderResolver::ResolveInitialTurnOrder(
		Result.AttackCountResult.PlayerAAttackCount,
		Result.AttackCountResult.PlayerBAttackCount,
		PlayerAInitialDeckRarityScore,
		PlayerBInitialDeckRarityScore,
		Input.PlayerATieBreakerRoll,
		Input.PlayerBTieBreakerRoll);
	if (!Result.InitialTurnOrderResult.bSuccess)
	{
		MatchOpeningResolver::AddStageFailure(
			Result,
			EMatchOpeningResolveErrorCode::InitialTurnOrderResolutionFailed,
			TEXT("Initial turn order resolution"),
			Result.InitialTurnOrderResult.ErrorMessages);
		return Result;
	}

	Result.bSuccess = true;
	return Result;
}
