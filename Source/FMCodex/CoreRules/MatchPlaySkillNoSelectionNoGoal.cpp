#include "MatchPlaySkillNoSelectionNoGoal.h"

namespace MatchPlaySkillNoSelectionNoGoalImplementation
{
	bool IsSkillNoGoalPlayer(
		const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	EMatchPlaySkillNoSelectionNoGoalErrorCode
	MapAvailabilityError(
		const EMatchPlayCurrentAttackSkillSelectionErrorCode
			ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::MatchPlayStateNotInitialized:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::MatchPlayStateNotInitialized;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::NoCurrentAttack:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoCurrentAttack;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentAttackSequence:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackSequence;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::AttackSequenceMismatch:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::AttackSequenceMismatch;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::CurrentAttackNotInResolution:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::CurrentAttackNotInResolution;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentAttackingPlayer:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackingPlayer;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidCurrentDefendingPlayer:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidCurrentDefendingPlayer;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidSelectionState:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidSelectionState;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::WrongSelectionStage:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::WrongSelectionStage;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::InvalidRequestingSide:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidRequestingSide;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode
			::RequestingSideIsNotCurrentAttacker:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::RequestingSideIsNotCurrentAttacker;
		case EMatchPlayCurrentAttackSkillSelectionErrorCode::None:
		default:
			return EMatchPlaySkillNoSelectionNoGoalErrorCode
				::AvailabilityQueryFailed;
		}
	}

	void SetAvailabilityFailure(
		EMatchPlaySkillNoSelectionNoGoalErrorCode& OutErrorCode,
		FString& OutErrorMessage,
		const FMatchPlayCurrentAttackSkillSelectionAvailabilityResult&
			Availability)
	{
		OutErrorCode = MapAvailabilityError(
			Availability.GlobalContextResult.ErrorCode);
		OutErrorMessage =
			Availability.GlobalContextResult.ErrorMessage.IsEmpty()
				? TEXT("Skill availability query failed.")
				: Availability.GlobalContextResult.ErrorMessage;
	}
}

FMatchPlayResolveNoLegalSkillResult
FMatchPlayResolveNoLegalSkill::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlayResolveNoLegalSkillRequest& Request)
{
	using namespace MatchPlaySkillNoSelectionNoGoalImplementation;

	FMatchPlayResolveNoLegalSkillResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::MatchPlayStateNotInitialized;
		Result.ErrorMessage =
			TEXT("Match play state must be initialized before resolving no legal skill.");
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoCurrentAttack;
		Result.ErrorMessage =
			TEXT("Resolving no legal skill requires an active current attack.");
		return Result;
	}

	const EInitialTurnOrderPlayer Attacker =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsSkillNoGoalPlayer(Attacker))
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::InvalidCurrentAttackingPlayer;
		Result.ErrorMessage =
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB.");
		return Result;
	}

	Result.SkillAvailabilityResult =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Attacker,
			SkillRuleSet);
	if (!Result.SkillAvailabilityResult.bQuerySucceeded
		|| !Result.SkillAvailabilityResult
			.GlobalContextResult.bSuccess)
	{
		SetAvailabilityFailure(
			Result.ErrorCode,
			Result.ErrorMessage,
			Result.SkillAvailabilityResult);
		return Result;
	}
	if (Result.SkillAvailabilityResult.bCanSelectAnySkill)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::LegalSkillExists;
		Result.ErrorMessage =
			TEXT("Cannot resolve no legal skill while a legal skill exists.");
		return Result;
	}

	Result.Reason =
		EMatchPlaySkillNoSelectionNoGoalReason::NoLegalSkill;
	Result.Source =
		EMatchPlaySkillNoSelectionNoGoalSource
			::ResolveNoLegalSkill;
	const FMatchPlaySkillNoSelectionNoGoalCapability Capability(
		FMatchPlaySkillNoSelectionNoGoalCapability
			::FResolveNoLegalSkillIssuerTag(),
		Request.AttackSequence,
		Result.SkillAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteSkillNoGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlaySkillNoSelectionNoGoalErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}

FMatchPlaySkillDeclineResult FMatchPlaySkillDecline::Decline(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet& SkillRuleSet,
	const FMatchPlaySkillDeclineRequest& Request)
{
	using namespace MatchPlaySkillNoSelectionNoGoalImplementation;

	FMatchPlaySkillDeclineResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	Result.SkillAvailabilityResult =
		FMatchPlayCurrentAttackSkillSelectionAvailability::Query(
			BeforeState,
			Request.AttackSequence,
			Request.RequestingSide,
			SkillRuleSet);
	if (!Result.SkillAvailabilityResult.bQuerySucceeded
		|| !Result.SkillAvailabilityResult
			.GlobalContextResult.bSuccess)
	{
		SetAvailabilityFailure(
			Result.ErrorCode,
			Result.ErrorMessage,
			Result.SkillAvailabilityResult);
		return Result;
	}
	if (!Result.SkillAvailabilityResult.bCanSelectAnySkill)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::NoLegalSkillToDecline;
		Result.ErrorMessage =
			TEXT("Decline requires at least one legal skill; use the no-legal-skill entry instead.");
		return Result;
	}

	Result.Reason =
		EMatchPlaySkillNoSelectionNoGoalReason::SkillDeclined;
	Result.Source =
		EMatchPlaySkillNoSelectionNoGoalSource::DeclineSkill;
	const FMatchPlaySkillNoSelectionNoGoalCapability Capability(
		FMatchPlaySkillNoSelectionNoGoalCapability
			::FDeclineSkillIssuerTag(),
		Request.AttackSequence,
		Result.SkillAvailabilityResult);

	Result.CompletionResult =
		FMatchPlayCurrentAttackCompletion::CompleteSkillNoGoal(
			BeforeState,
			Capability);
	if (!Result.CompletionResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlaySkillNoSelectionNoGoalErrorCode
				::CompletionFailed;
		Result.ErrorMessage =
			Result.CompletionResult.ErrorMessage;
		return Result;
	}

	Result.AfterState = Result.CompletionResult.AfterState;
	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlaySkillNoSelectionNoGoalErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
