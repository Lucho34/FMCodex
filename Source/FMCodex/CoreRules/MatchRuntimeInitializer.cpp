#include "MatchRuntimeInitializer.h"

namespace MatchRuntimeInitializer
{
	void AddError(
		FMatchRuntimeInitializeResult& Result,
		const EMatchRuntimeInitializeErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCodes.Add(ErrorCode);
		Result.ErrorMessages.Add(ErrorMessage);
	}

	EInitialTurnOrderPlayer GetOtherPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	void AppendOpeningErrors(
		FMatchRuntimeInitializeResult& Result,
		const FMatchOpeningResolveResult& OpeningResult)
	{
		if (OpeningResult.ErrorMessages.IsEmpty())
		{
			AddError(
				Result,
				EMatchRuntimeInitializeErrorCode::OpeningResultFailed,
				TEXT("Cannot initialize runtime state from a failed opening result."));
			return;
		}

		Result.ErrorCodes.Add(EMatchRuntimeInitializeErrorCode::OpeningResultFailed);
		for (const FString& OpeningError : OpeningResult.ErrorMessages)
		{
			Result.ErrorMessages.Add(FString::Printf(
				TEXT("Opening result: %s"),
				*OpeningError));
		}
	}
}

FMatchRuntimeInitializeResult FMatchRuntimeInitializer::InitializeRuntimeState(
	const FMatchOpeningResolveResult& OpeningResult)
{
	FMatchRuntimeInitializeResult Result;
	Result.RuntimeState.OpeningResultSnapshot = OpeningResult;

	if (!OpeningResult.bSuccess)
	{
		MatchRuntimeInitializer::AppendOpeningErrors(Result, OpeningResult);
		return Result;
	}

	const EInitialTurnOrderPlayer FirstPlayer =
		OpeningResult.InitialTurnOrderResult.FirstPlayer;
	if (FirstPlayer != EInitialTurnOrderPlayer::PlayerA
		&& FirstPlayer != EInitialTurnOrderPlayer::PlayerB)
	{
		MatchRuntimeInitializer::AddError(
			Result,
			EMatchRuntimeInitializeErrorCode::InvalidFirstPlayer,
			TEXT("A successful opening result must select PlayerA or PlayerB as first player."));
		return Result;
	}

	Result.RuntimeState.PlayerAState.TotalAttackCount =
		OpeningResult.AttackCountResult.PlayerAAttackCount;
	Result.RuntimeState.PlayerBState.TotalAttackCount =
		OpeningResult.AttackCountResult.PlayerBAttackCount;

	Result.RuntimeState.PlayerAState.UsedAttackCount = 0;
	Result.RuntimeState.PlayerBState.UsedAttackCount = 0;
	Result.RuntimeState.PlayerAState.Score = 0;
	Result.RuntimeState.PlayerBState.Score = 0;

	Result.RuntimeState.PlayerAState.GoalkeeperCardId =
		OpeningResult.MatchInitializationResult.PlayerADeckValidation.GoalkeeperCardId;
	Result.RuntimeState.PlayerBState.GoalkeeperCardId =
		OpeningResult.MatchInitializationResult.PlayerBDeckValidation.GoalkeeperCardId;
	Result.RuntimeState.PlayerAState.InitialDeckRarityScore =
		OpeningResult.MatchInitializationResult.PlayerADeckValidation.InitialDeckRarityScore;
	Result.RuntimeState.PlayerBState.InitialDeckRarityScore =
		OpeningResult.MatchInitializationResult.PlayerBDeckValidation.InitialDeckRarityScore;

	Result.RuntimeState.FirstPlayer = FirstPlayer;
	Result.RuntimeState.SecondPlayer =
		MatchRuntimeInitializer::GetOtherPlayer(FirstPlayer);
	Result.RuntimeState.CurrentAttackingPlayer = FirstPlayer;

	Result.RuntimeState.bIsInitialized = true;
	Result.bSuccess = true;
	return Result;
}
