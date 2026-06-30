#include "MatchPlayLoopReadiness.h"

namespace
{
	EMatchPlayLoopReadinessCode MapTurnGuardErrorCode(
		const EMatchPlayTurnGuardErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayTurnGuardErrorCode::None:
			return EMatchPlayLoopReadinessCode
				::ReadyForExternalAttackRequest;
		case EMatchPlayTurnGuardErrorCode::MatchStateNotInitialized:
			return EMatchPlayLoopReadinessCode::MatchStateNotInitialized;
		case EMatchPlayTurnGuardErrorCode::NoRemainingAttackOpportunities:
			return EMatchPlayLoopReadinessCode
				::NoRemainingAttackOpportunities;
		case EMatchPlayTurnGuardErrorCode
			::CurrentAttackerHasNoRemainingAttackOpportunity:
			return EMatchPlayLoopReadinessCode
				::CurrentAttackerHasNoRemainingAttackOpportunity;
		case EMatchPlayTurnGuardErrorCode
			::CurrentAttackerHasNoAvailableCards:
			return EMatchPlayLoopReadinessCode
				::CurrentAttackerHasNoAvailableCards;
		default:
			return EMatchPlayLoopReadinessCode::None;
		}
	}
}

FMatchPlayLoopReadinessResult FMatchPlayLoopReadiness::Evaluate(
	const FMatchPlayState& State)
{
	FMatchPlayLoopReadinessResult Result;
	Result.TurnGuardResult =
		FMatchPlayTurnGuard::QueryCanSubmitAttackRequest(State);
	Result.bShouldEnterAutomaticLoop = false;
	Result.Code = MapTurnGuardErrorCode(Result.TurnGuardResult.ErrorCode);
	Result.Reason = Result.TurnGuardResult.ErrorMessage;
	Result.ErrorMessage = Result.TurnGuardResult.ErrorMessage;

	if (Result.TurnGuardResult.bCanSubmitAttackRequest)
	{
		Result.bCanAcceptExternalAttackRequest = true;
		Result.bShouldWaitForExternalAttackRequest = true;
		Result.Code =
			EMatchPlayLoopReadinessCode
				::ReadyForExternalAttackRequest;
		Result.Reason = TEXT("Ready for external attack request.");
		Result.ErrorMessage.Reset();
	}

	return Result;
}

