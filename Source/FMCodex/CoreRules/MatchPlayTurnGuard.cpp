#include "MatchPlayTurnGuard.h"

namespace
{
	void SetGuardFailure(
		FMatchPlayTurnGuardResult& Result,
		const EMatchPlayTurnGuardErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayTurnGuardResult
FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(
	const FMatchPlayState& MatchPlayState)
{
	const FMatchPlayStatus Status =
		FMatchPlayStatusQuery::QueryStatus(MatchPlayState);

	FMatchPlayTurnGuardResult Result;
	Result.CurrentAttacker = Status.CurrentAttackingPlayer;
	Result.PlayerARemainingAttackCount =
		Status.PlayerARemainingAttackCount;
	Result.PlayerBRemainingAttackCount =
		Status.PlayerBRemainingAttackCount;

	if (Result.CurrentAttacker == EInitialTurnOrderPlayer::PlayerA)
	{
		Result.CurrentAttackerRemainingAttackCount =
			Status.PlayerARemainingAttackCount;
		Result.CurrentAttackerAvailableCardCount =
			Status.PlayerAAvailableCardCount;
	}
	else if (Result.CurrentAttacker == EInitialTurnOrderPlayer::PlayerB)
	{
		Result.CurrentAttackerRemainingAttackCount =
			Status.PlayerBRemainingAttackCount;
		Result.CurrentAttackerAvailableCardCount =
			Status.PlayerBAvailableCardCount;
	}

	if (!Status.bIsRuntimeInitialized)
	{
		SetGuardFailure(
			Result,
			EMatchPlayTurnGuardErrorCode::MatchStateNotInitialized,
			TEXT("Match play state is not initialized."));
		return Result;
	}

	if (Result.PlayerARemainingAttackCount <= 0
		&& Result.PlayerBRemainingAttackCount <= 0)
	{
		Result.bNoRemainingAttackOpportunities = true;
		SetGuardFailure(
			Result,
			EMatchPlayTurnGuardErrorCode
				::NoRemainingAttackOpportunities,
			TEXT("No remaining attack opportunities."));
		return Result;
	}

	if (Result.CurrentAttacker == EInitialTurnOrderPlayer::None)
	{
		SetGuardFailure(
			Result,
			EMatchPlayTurnGuardErrorCode::MatchStateNotInitialized,
			TEXT("Match play state is not initialized."));
		return Result;
	}

	if (Result.CurrentAttackerRemainingAttackCount <= 0)
	{
		SetGuardFailure(
			Result,
			EMatchPlayTurnGuardErrorCode
				::CurrentAttackerHasNoRemainingAttackOpportunity,
			TEXT("Current attacker has no remaining attack opportunity."));
		return Result;
	}

	if (Result.CurrentAttackerAvailableCardCount <= 0)
	{
		SetGuardFailure(
			Result,
			EMatchPlayTurnGuardErrorCode
				::CurrentAttackerHasNoAvailableCards,
			TEXT("Current attacker has no available cards."));
		return Result;
	}

	Result.bCanSubmitAttackRequest = true;
	Result.bShouldWaitForExternalAttackRequest = true;
	return Result;
}
