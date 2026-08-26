#include "MatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator.h"

#include "MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator.h"
#include "ThroughBallFeetFormulaResolverInputAssembler.h"

namespace MatchPlayCurrentAttackResolveThroughBallFeetFormula
{
	using FResult =
		FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult;
	using EError =
		EMatchPlayCurrentAttackResolveThroughBallFeetFormulaErrorCode;

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

FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult
FMatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FSkillRuleSnapshotSet* SkillRuleSet)
{
	using namespace MatchPlayCurrentAttackResolveThroughBallFeetFormula;

	FResult Result;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;
	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetFailure(
			Result,
			EError::MatchPlayStateNotInitialized,
			TEXT("ThroughBall Feet Formula resolution requires initialized MatchPlay State."));
		return Result;
	}
	if (!BeforeState.bHasCurrentAttack)
	{
		SetFailure(
			Result,
			EError::NoCurrentAttack,
			TEXT("ThroughBall Feet Formula resolution requires an active CurrentAttack."));
		return Result;
	}
	if (!BeforeState.CurrentAttack.bHasResolutionSession)
	{
		SetFailure(
			Result,
			EError::MissingResolutionSession,
			TEXT("ThroughBall Feet Formula resolution requires a Resolution Session."));
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
			TEXT("ThroughBall Feet Formula resolution requires RouteResolved."));
		return Result;
	}
	if (!Session.bHasActualBranch
		|| Session.ActualBranch.ActionType != ESkillRuleType::ThroughBall
		|| Session.ActualBranch.ThroughBall
			!= EMatchPlayThroughBallActualBranch::Feet)
	{
		SetFailure(
			Result,
			EError::NotThroughBallFeetBranch,
			TEXT("This operation supports only the resolved ThroughBall Feet branch."));
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
			TEXT("Both canonical Feet post-route rolls must exist before Formula execution."));
		return Result;
	}
	if (SkillRuleSet == nullptr)
	{
		SetFailure(
			Result,
			EError::SkillRuleSetUnavailable,
			TEXT("ThroughBall Feet Formula resolution requires the authoritative SkillRuleSet."));
		return Result;
	}

	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest Request;
	Request.AttackSequence = BeforeState.CurrentAttack.AttackSequence;
	Request.Mode =
		FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
			::EMode::RegenerateCompletedPlan;
	Result.PlanRegenerationResult =
		FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator
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
			TEXT("Feet Formula plan regeneration must consume zero post-route RNG."));
		return Result;
	}
	if (!AreStatesEqual(
		Result.BeforeState,
		Result.PlanRegenerationResult.AfterState))
	{
		SetFailure(
			Result,
			EError::PlanRegenerationMutatedState,
			TEXT("Feet Formula plan regeneration must not mutate authoritative State."));
		return Result;
	}

	const FThroughBallFeetPlanQueryResult& PlanResult =
		Result.PlanRegenerationResult.PlanResult;
	if (!PlanResult.bSuccess
		|| PlanResult.Decision
			!= EThroughBallFeetPlanQueryDecision::FormulaResolutionRequired
		|| !PlanResult.bHasFormulaPlan)
	{
		SetFailure(
			Result,
			EError::FormulaPlanUnavailable,
			TEXT("Canonical Feet regeneration did not produce a Formula plan."));
		return Result;
	}

	FThroughBallFeetFormulaResolverInputAssemblyInput AssemblyInput;
	AssemblyInput.FormulaPlan = PlanResult.FormulaPlan;
	Result.ResolverInputAssemblyResult =
		FThroughBallFeetFormulaResolverInputAssembler::Assemble(AssemblyInput);
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
	Result.TacticalPlayerAdvantageResult =
		FMatchPlayTacticalPlayerAdvantageQuery::Evaluate(BeforeState);
	if (!Result.TacticalPlayerAdvantageResult.bSuccess)
	{
		SetFailure(
			Result,
			EError::TacticalPlayerAdvantageQueryFailed,
			Result.TacticalPlayerAdvantageResult.ErrorMessage,
			TEXT("TacticalPlayerAdvantage"));
		return Result;
	}
	Result.ResolverInputAssemblyResult.ResolverInput.Attacker
		.TacticalPlayerModifier = Result.TacticalPlayerAdvantageResult
		.AttackerFinishingModifier;
	Result.ResolverInputAssemblyResult.ResolverInput.Defender
		.TacticalPlayerModifier = Result.TacticalPlayerAdvantageResult
		.DefenderFinishingModifier;

	FThroughBallFeetFormulaResolutionExecutionInput ExecutionInput;
	ExecutionInput.ResolverInputAssemblyResult =
		Result.ResolverInputAssemblyResult;
	++Result.FormulaExecutionCount;
	Result.FormulaExecutionResult =
		FThroughBallFeetFormulaResolutionExecutor::Execute(ExecutionInput);
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

	const FFormulaResolutionResult& FormulaResult =
		Result.FormulaExecutionResult.FormulaResolutionResult;
	const bool bAttackerWon =
		FormulaResult.Winner == EFormulaWinner::Attacker;
	if (FormulaResult.FormulaType != EFormulaType::Finishing
		|| FormulaResult.Winner == EFormulaWinner::None
		|| FormulaResult.bIsGoal != bAttackerWon
		|| !FormulaResult.bAttackEnded
		|| FormulaResult.bContinueResolution)
	{
		SetFailure(
			Result,
			EError::InvalidFormulaResolutionResult,
			TEXT("ThroughBall Feet execution returned a non-terminal Formula result."),
			TEXT("FormulaResolutionResult"));
		return Result;
	}

	Result.FormulaResolutionResult = FormulaResult;
	Result.bHasFormulaResolution = true;
	Result.bSuccess = true;
	Result.ErrorCode = EError::None;
	Result.ErrorMessage.Empty();
	Result.InvalidField = NAME_None;
	return Result;
}
