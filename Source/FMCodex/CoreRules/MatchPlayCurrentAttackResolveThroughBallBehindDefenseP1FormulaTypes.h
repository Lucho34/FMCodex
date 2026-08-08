#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanTypes.h"
#include "ThroughBallBehindDefenseP1FormulaResolutionExecutor.h"

enum class
	EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotThroughBallBehindDefenseBranch,
	InvalidPostRouteProgress,
	IncompletePostRouteRollProgress,
	SkillRuleSetUnavailable,
	PlanRegenerationFailed,
	PlanRegenerationConsumedRng,
	PlanRegenerationMutatedState,
	FormulaPlanUnavailable,
	ResolverInputAssemblyFailed,
	FormulaExecutionFailed,
	InvalidFormulaExecutionResult
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaErrorCode
				::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;

	int32 PlanRegenerationProviderCallCount = 0;
	int32 FormulaExecutionCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult
		PlanRegenerationResult;
	FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult
		ResolverInputAssemblyResult;
	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
		FormulaExecutionResult;

	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;
};
