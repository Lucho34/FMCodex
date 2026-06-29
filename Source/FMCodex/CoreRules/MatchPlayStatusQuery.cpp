#include "MatchPlayStatusQuery.h"

FMatchPlayStatus FMatchPlayStatusQuery::QueryStatus(
	const FMatchPlayState& MatchPlayState)
{
	FMatchPlayStatus Status;
	const FMatchRuntimeState& RuntimeState = MatchPlayState.RuntimeState;
	const FMatchCardUsageState& CardUsageState =
		MatchPlayState.CardUsageState;

	Status.bIsRuntimeInitialized = RuntimeState.bIsInitialized;
	Status.CurrentAttackingPlayer = RuntimeState.CurrentAttackingPlayer;
	Status.PlayerAScore = RuntimeState.PlayerAState.Score;
	Status.PlayerBScore = RuntimeState.PlayerBState.Score;
	Status.PlayerARemainingAttackCount =
		RuntimeState.PlayerAState.TotalAttackCount
		- RuntimeState.PlayerAState.UsedAttackCount;
	Status.PlayerBRemainingAttackCount =
		RuntimeState.PlayerBState.TotalAttackCount
		- RuntimeState.PlayerBState.UsedAttackCount;
	Status.PlayerAAvailableCardCount =
		CardUsageState.PlayerACardUsageState.AvailableCardIds.Num();
	Status.PlayerAUsedCardCount =
		CardUsageState.PlayerACardUsageState.UsedCardIds.Num();
	Status.PlayerBAvailableCardCount =
		CardUsageState.PlayerBCardUsageState.AvailableCardIds.Num();
	Status.PlayerBUsedCardCount =
		CardUsageState.PlayerBCardUsageState.UsedCardIds.Num();
	return Status;
}
