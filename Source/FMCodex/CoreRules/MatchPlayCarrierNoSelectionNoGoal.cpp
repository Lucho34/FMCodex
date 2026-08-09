#include "MatchPlayCarrierNoSelectionNoGoal.h"

#include "MatchPlayNoLegalCarrierCompletionCapability.h"

namespace MatchPlayCarrierNoSelectionNoGoal
{
	bool IsValidCarrierPlayerSide(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	void Fail(
		FMatchPlayResolveNoLegalCarrierResult& Result,
		const EMatchPlayCarrierNoSelectionNoGoalErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayResolveNoLegalCarrierResult
FMatchPlayResolveNoLegalCarrier::Resolve(
	const FMatchPlayState& BeforeState)
{
	using namespace MatchPlayCarrierNoSelectionNoGoal;

	FMatchPlayResolveNoLegalCarrierResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play State must be initialized before resolving no legal Carrier."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode::NoCurrentAttack,
			TEXT("No-legal Carrier resolution requires an active CurrentAttack."));
		return Result;
	}

	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsValidCarrierPlayerSide(Attacker))
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	const int64 AttackSequence =
		BeforeState.CurrentAttack.AttackSequence;
	Result.CarrierAvailabilityResult =
		FMatchPlayCurrentAttackCarrierSelectionAvailability::Query(
			BeforeState,
			AttackSequence,
			Attacker);
	if (!Result.CarrierAvailabilityResult.bQuerySucceeded
		|| Result.CarrierAvailabilityResult
			.bHasGlobalBlockingLegalityResult)
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode
				::AvailabilityQueryFailed,
			Result.CarrierAvailabilityResult
				.bHasGlobalBlockingLegalityResult
					? Result.CarrierAvailabilityResult
						.GlobalBlockingLegalityResult.ErrorMessage
					: TEXT("Carrier availability query failed."));
		return Result;
	}
	if (Result.CarrierAvailabilityResult.bCanSelectAnyCarrier)
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode::LegalCarrierExists,
			TEXT("No-legal Carrier resolution cannot run while a legal Carrier exists."));
		return Result;
	}

	const FMatchPlayNoLegalCarrierCompletionCapability Capability(
		FMatchPlayNoLegalCarrierCompletionCapability
			::FResolveNoLegalCarrierIssuerTag(),
		AttackSequence,
		Result.CarrierAvailabilityResult);
	++Result.CompletionExecutionCount;
	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteCarrierNoGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Fail(Result,
			EMatchPlayCarrierNoSelectionNoGoalErrorCode::CompletionFailed,
			Result.CompletionResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode = EMatchPlayCarrierNoSelectionNoGoalErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
