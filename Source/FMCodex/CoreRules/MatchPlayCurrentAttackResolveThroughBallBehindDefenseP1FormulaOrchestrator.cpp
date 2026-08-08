#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator.h"
#include "ThroughBallBehindDefenseP1FormulaResolverInputAssembler.h"

namespace MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1Formula
{
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult;
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaErrorCode;

	void SetFailure(
		FResult& Result,
		const EError ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField = NAME_None)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool AreStatesEqual(
		const FMatchPlayState& Left,
		const FMatchPlayState& Right)
	{
		return FMatchPlayState::StaticStruct()->CompareScriptStruct(
			&Left,
			&Right,
			0);
	}
}

FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator
	::Resolve(
		const FMatchPlayState& BeforeState,
		const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace
		MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1Formula;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("BehindDefense P1 Formula resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("BehindDefense P1 Formula resolution requires an active CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("BehindDefense P1 Formula resolution requires a Resolution Session."));
		return Result;
	}

	Result.SessionStateValidationResult =
		FMatchPlayCurrentAttackResolutionSessionStateValidator::Validate(
			BeforeState);
	if (!Result.SessionStateValidationResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidResolutionSessionState,
			Result.SessionStateValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayCurrentAttackResolutionSession& Session =
		BeforeState.CurrentAttack.ResolutionSession;
	if (Session.Stage != EMatchPlayCurrentAttackResolutionStage::RouteResolved)
	{
		SetFailure(
			Result,
			EError::RouteNotResolved,
			TEXT("BehindDefense P1 Formula resolution requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| Session.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::BehindDefense)
	{
		SetFailure(
			Result,
			EError::NotThroughBallBehindDefenseBranch,
			TEXT("This operation supports only the resolved ThroughBall BehindDefense branch."));
		return Result;
	}

	Result.ProgressResult =
		FMatchPlayCurrentAttackPostRouteRollProgressQuery::Evaluate(Session);
	if (!Result.ProgressResult.bIsCanonical)
	{
		SetFailure(
			Result,
			EError::InvalidPostRouteProgress,
			Result.ProgressResult.ErrorMessage);
		return Result;
	}
	if (!Result.ProgressResult.bContractComplete)
	{
		SetFailure(
			Result,
			EError::IncompletePostRouteRollProgress,
			TEXT("The canonical BehindDefense P1 roll contract must be complete before Formula execution."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("BehindDefense P1 Formula resolution requires the authoritative SkillRuleSet."));
		return Result;
	}

	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
		Request;
	Request.AttackSequence = BeforeState.CurrentAttack.AttackSequence;
	Result.PlanRegenerationResult =
		FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator
			::Resolve(BeforeState, Request, SkillRuleSet, nullptr);
	Result.PlanRegenerationProviderCallCount =
		Result.PlanRegenerationResult.ProviderCallCount;
	if (!Result.PlanRegenerationResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::PlanRegenerationFailed,
			Result.PlanRegenerationResult.ErrorMessage);
		return Result;
	}
	if (Result.PlanRegenerationProviderCallCount != 0)
	{
		SetFailure(
			Result,
			EError::PlanRegenerationConsumedRng,
			TEXT("BehindDefense P1 Formula plan regeneration must consume zero post-route RNG."));
		return Result;
	}
	if (!AreStatesEqual(
		Result.BeforeState,
		Result.PlanRegenerationResult.AfterState))
	{
		SetFailure(
			Result,
			EError::PlanRegenerationMutatedState,
			TEXT("BehindDefense P1 Formula plan regeneration must not mutate authoritative State."));
		return Result;
	}

	const FThroughBallBehindDefenseP1PlanQueryResult& PlanResult =
		Result.PlanRegenerationResult.P1PlanResult;
	if (!PlanResult.bSuccess
		|| PlanResult.Decision
			!= EThroughBallBehindDefenseP1PlanQueryDecision
				::FormulaResolutionRequired
		|| !PlanResult.bHasFormulaPlan)
	{
		SetFailure(
			Result,
			EError::FormulaPlanUnavailable,
			TEXT("Canonical BehindDefense P1 regeneration is not on its Formula path."));
		return Result;
	}

	FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput AssemblyInput;
	AssemblyInput.PlanQueryResult = PlanResult;
	Result.ResolverInputAssemblyResult =
		FThroughBallBehindDefenseP1FormulaResolverInputAssembler::Assemble(
			AssemblyInput);
	if (!Result.ResolverInputAssemblyResult.bSuccess
		|| !Result.ResolverInputAssemblyResult.bHasResolverInput)
	{
		SetFailure(
			Result,
			EError::ResolverInputAssemblyFailed,
			Result.ResolverInputAssemblyResult.ErrorMessage,
			Result.ResolverInputAssemblyResult.InvalidField);
		return Result;
	}

	FThroughBallBehindDefenseP1FormulaResolutionExecutionInput ExecutionInput;
	ExecutionInput.ResolverInputAssemblyResult =
		Result.ResolverInputAssemblyResult;
	++Result.FormulaExecutionCount;
	Result.FormulaExecutionResult =
		FThroughBallBehindDefenseP1FormulaResolutionExecutor::Execute(
			ExecutionInput);
	if (!Result.FormulaExecutionResult.bSuccess
		|| !Result.FormulaExecutionResult.bHasFormulaResolution)
	{
		SetFailure(
			Result,
			EError::FormulaExecutionFailed,
			Result.FormulaExecutionResult.ErrorMessage,
			Result.FormulaExecutionResult.InvalidField);
		return Result;
	}
	if (Result.FormulaExecutionResult.Decision
		== EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None)
	{
		SetFailure(
			Result,
			EError::InvalidFormulaExecutionResult,
			TEXT("BehindDefense P1 Formula execution must return a canonical transition decision."),
			TEXT("FormulaExecutionResult.Decision"));
		return Result;
	}

	Result.FormulaResolutionResult =
		Result.FormulaExecutionResult.FormulaResolutionResult;
	Result.bHasFormulaResolution = true;
	Result.bSuccess = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	Result.InvalidField = NAME_None;
	return Result;
}
