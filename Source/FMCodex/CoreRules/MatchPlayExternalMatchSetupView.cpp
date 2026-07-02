#include "MatchPlayExternalMatchSetupView.h"

FMatchPlayExternalMatchSetupView
FMatchPlayExternalMatchSetupViewQuery::BuildView(
	const FMatchPlayState& InitializedState)
{
	FMatchPlayExternalMatchSetupView View;
	View.StateView =
		FMatchPlayExternalStateViewQuery::BuildView(InitializedState);

	View.bSuccessfullyReadInitializedState =
		View.StateView.bIsRuntimeInitialized
		&& View.StateView.ReadinessCode
			!= EMatchPlayLoopReadinessCode::MatchStateNotInitialized;
	View.HomeAvailableCardCount =
		View.StateView.HomeAvailableCardCount;
	View.HomeUsedCardCount = View.StateView.HomeUsedCardCount;
	View.AwayAvailableCardCount =
		View.StateView.AwayAvailableCardCount;
	View.AwayUsedCardCount = View.StateView.AwayUsedCardCount;
	View.InitialAttackingPlayer =
		View.StateView.CurrentAttackingPlayer;
	View.InitialHomeScore = View.StateView.HomeScore;
	View.InitialAwayScore = View.StateView.AwayScore;
	View.InitialHomeRemainingAttackCount =
		View.StateView.HomeRemainingAttackCount;
	View.InitialAwayRemainingAttackCount =
		View.StateView.AwayRemainingAttackCount;
	View.bIsMatchFinished = View.StateView.bIsMatchFinished;
	View.bIsWaitingForExternalAttackRequest =
		View.StateView.bIsWaitingForExternalAttackRequest;
	View.SetupSummary = View.StateView.StatusSummary;

	if (!View.bSuccessfullyReadInitializedState)
	{
		View.ErrorMessage = View.StateView.CannotSubmitReason;
		if (View.ErrorMessage.IsEmpty())
		{
			View.ErrorMessage = View.SetupSummary;
		}
	}

	return View;
}
