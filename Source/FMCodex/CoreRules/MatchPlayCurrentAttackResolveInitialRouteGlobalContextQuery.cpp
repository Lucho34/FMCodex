#include "MatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery.h"

#include "MatchPlayElectiveBranchIntentRules.h"

namespace MatchPlayCurrentAttackResolveInitialRouteGlobalContextImplementation
{
	void SetFailure(
		FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult& Result,
		const
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsSupportedAction(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::LongShot
			|| ActionType == ESkillRuleType::CutInsideShot
			|| ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
	}

	bool RequiresInitialRouteD6(const ESkillRuleType ActionType)
	{
		return ActionType == ESkillRuleType::Cross
			|| ActionType == ESkillRuleType::PassControl
			|| ActionType == ESkillRuleType::ThroughBall;
	}
}

FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult
FMatchPlayCurrentAttackResolveInitialRouteGlobalContextQuery::Query(
	const FMatchPlayState& State,
	const FMatchPlayCurrentAttackResolveInitialRouteRequest& Request)
{
	using namespace
		MatchPlayCurrentAttackResolveInitialRouteGlobalContextImplementation;

	FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult Result;
	Result.Request = Request;
	if (!State.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::MatchPlayNotInitialized,
			TEXT("Initial Route resolution requires initialized MatchPlay state."));
		return Result;
	}
	if (!State.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::NoCurrentAttack,
			TEXT("Initial Route resolution requires CurrentAttack authority."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack = State.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::InvalidCurrentAttackAttackSequence,
			TEXT("CurrentAttack AttackSequence must be greater than zero."));
		return Result;
	}
	if (Request.AttackSequence <= 0)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::InvalidRequestAttackSequence,
			TEXT("Requested AttackSequence must be greater than zero."));
		return Result;
	}
	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::AttackSequenceMismatch,
			TEXT("Requested AttackSequence does not match CurrentAttack."));
		return Result;
	}
	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Resolution)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::CurrentAttackNotInResolution,
			TEXT("CurrentAttack must be in Resolution phase."));
		return Result;
	}

	Result.SessionValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			State);
	if (!Result.SessionValidationResult.bIsCanonical)
	{
		const bool bSessionSequenceMismatch =
			Result.SessionValidationResult.ErrorCode
			== EMatchPlayCurrentAttackResolutionSessionStateValidationErrorCode
				::SessionAttackSequenceMismatch;
		SetFailure(
			Result,
			bSessionSequenceMismatch
				? EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
					::SessionAttackSequenceMismatch
				: EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
					::InvalidResolutionSessionState,
			Result.SessionValidationResult.ErrorMessage);
		return Result;
	}
	if (!CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::NoResolutionSession,
			TEXT("Initial Route resolution requires a Resolution Session."));
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		CurrentAttack.ResolutionSession;
	if (Session.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::SessionAttackSequenceMismatch,
			TEXT("Resolution Session AttackSequence does not match CurrentAttack."));
		return Result;
	}

	Result.ImmutableBundle = Session.Bundle;
	Result.ActionType = Session.Bundle.Binding.ActionType;
	Result.Intent = Session.Bundle.Binding.ElectiveBranchIntent;
	if (Session.Stage
		== EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		Result.bSuccess = true;
		Result.bIsCanonicalDuplicate = true;
		Result.ExistingActualBranch = Session.ActualBranch;
		Result.ExistingInitialRouteRollRecords =
			Session.InitialRouteRollRecords;
		return Result;
	}
	if (Session.Stage
		!= EMatchPlayCurrentAttackResolutionStage::AwaitingRoute)
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::ResolutionSessionNotAwaitingRoute,
			TEXT("First Initial Route resolution requires AwaitingRoute stage."));
		return Result;
	}
	if (!IsSupportedAction(Result.ActionType))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::UnsupportedAction,
			TEXT("Initial Route resolution requires a supported ActionType."));
		return Result;
	}
	if (!MatchPlayElectiveBranchIntentRules::IsLegalIntent(
			Result.ActionType,
			Result.Intent))
	{
		SetFailure(
			Result,
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::InvalidIntentForAction,
			TEXT("Session elective branch Intent is invalid for ActionType."));
		return Result;
	}

	Result.bRequiresInitialRouteD6 =
		RequiresInitialRouteD6(Result.ActionType);
	if (Result.bRequiresInitialRouteD6)
	{
		Result.ExpectedRollPurpose =
			EMatchPlayCurrentAttackResolutionRollPurpose::InitialRoute;
	}
	Result.bSuccess = true;
	return Result;
}
