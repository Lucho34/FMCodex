#include "MatchPlayExternalStateView.h"

FMatchPlayExternalStateView FMatchPlayExternalStateViewQuery::BuildView(
	const FMatchPlayState& State)
{
	FMatchPlayExternalStateView View;
	View.Status = FMatchPlayStatusQuery::QueryStatus(State);
	View.LoopReadinessResult = FMatchPlayLoopReadiness::Evaluate(State);

	View.bIsRuntimeInitialized = View.Status.bIsRuntimeInitialized;
	View.HomeScore = View.Status.PlayerAScore;
	View.AwayScore = View.Status.PlayerBScore;
	View.CurrentAttackingPlayer = View.Status.CurrentAttackingPlayer;
	View.bIsMatchFinished =
		View.LoopReadinessResult.TurnGuardResult
			.bNoRemainingAttackOpportunities;
	View.bIsWaitingForExternalAttackRequest =
		View.LoopReadinessResult.bShouldWaitForExternalAttackRequest;
	View.bCanSubmitAttackRequest =
		View.LoopReadinessResult.bCanAcceptExternalAttackRequest;
	View.ReadinessCode = View.LoopReadinessResult.Code;
	View.StatusSummary = View.LoopReadinessResult.Reason;
	if (!View.bCanSubmitAttackRequest)
	{
		View.CannotSubmitReason =
			View.LoopReadinessResult.ErrorMessage;
	}

	View.HomeRemainingAttackCount =
		View.Status.PlayerARemainingAttackCount;
	View.AwayRemainingAttackCount =
		View.Status.PlayerBRemainingAttackCount;
	View.HomeAvailableCardCount =
		View.Status.PlayerAAvailableCardCount;
	View.HomeUsedCardCount = View.Status.PlayerAUsedCardCount;
	View.AwayAvailableCardCount =
		View.Status.PlayerBAvailableCardCount;
	View.AwayUsedCardCount = View.Status.PlayerBUsedCardCount;

	View.HomeAvailableCardIds =
		State.CardUsageState.PlayerACardUsageState.AvailableCardIds;
	View.HomeUsedCardIds =
		State.CardUsageState.PlayerACardUsageState.UsedCardIds;
	View.AwayAvailableCardIds =
		State.CardUsageState.PlayerBCardUsageState.AvailableCardIds;
	View.AwayUsedCardIds =
		State.CardUsageState.PlayerBCardUsageState.UsedCardIds;
	return View;
}
