#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanTypes.h"
#include "MatchPlayTacticalPlayerAdvantageQuery.h"
#include "ThroughBallFeetFormulaResolutionExecutor.h"

enum class
	EMatchPlayCurrentAttackResolveThroughBallFeetFormulaErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotThroughBallFeetBranch,
	InvalidPostRouteProgress,
	IncompletePostRouteRollProgress,
	SkillRuleSetUnavailable,
	PlanRegenerationFailed,
	PlanRegenerationConsumedRng,
	PlanRegenerationMutatedState,
	FormulaPlanUnavailable,
	ResolverInputAssemblyFailed,
	FormulaExecutionFailed,
	InvalidFormulaResolutionResult,
	TacticalPlayerAdvantageQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallFeetFormulaErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveThroughBallFeetFormulaErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	FMatchPlayTacticalPlayerAdvantageResult TacticalPlayerAdvantageResult;

	int32 PlanRegenerationProviderCallCount = 0;
	int32 FormulaExecutionCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
		PlanRegenerationResult;
	FThroughBallFeetFormulaResolverInputAssemblyResult
		ResolverInputAssemblyResult;
	FThroughBallFeetFormulaResolutionExecutionResult
		FormulaExecutionResult;

	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;
};
