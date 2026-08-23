#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayCurrentAttackResolveCrossPostRoutePlanTypes.h"
#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanTypes.h"
#include "MatchPlayCurrentAttackResolvePassControlPostRoutePlanTypes.h"
#include "MatchPlayTacticalPlayerAdvantageQuery.h"
#include "SingleCardFormulaResolverInputAssembler.h"
#include "SingleCardFormulaResolutionExecutor.h"

enum class EMatchPlaySingleCardFinishingFormulaFamily : uint8
{
	None,
	Cross,
	PassAdvance,
	DribbleAdvance,
	RunAdvance,
	LongShotDirectShot,
	CutInsideShotDirectShot
};

enum class
	EMatchPlayCurrentAttackResolveSingleCardFinishingFormulaErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	MissingActualBranch,
	UnsupportedFormulaFamily,
	InvalidPostRouteProgress,
	IncompletePostRouteRollProgress,
	SkillRuleSetUnavailable,
	PlanRegenerationFailed,
	PlanRegenerationConsumedRng,
	PlanRegenerationMutatedState,
	FormulaPlanUnavailable,
	AttackerInputAssemblyFailed,
	DefenderInputAssemblyFailed,
	ResolverInputAssemblyFailed,
	FormulaExecutionFailed,
	InvalidFormulaResolutionResult,
	ActiveGoalkeeperSnapshotUnavailable,
	InvalidActiveGoalkeeperSnapshot,
	TacticalPlayerAdvantageQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveSingleCardFinishingFormulaErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveSingleCardFinishingFormulaErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	FMatchPlayTacticalPlayerAdvantageResult TacticalPlayerAdvantageResult;
	EMatchPlaySingleCardFinishingFormulaFamily Family =
		EMatchPlaySingleCardFinishingFormulaFamily::None;

	int32 PlanRegenerationProviderCallCount = 0;
	int32 FormulaExecutionCount = 0;

	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		CrossRegenerationResult;
	FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult
		PassControlRegenerationResult;
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
		DirectShotRegenerationResult;

	FPlayerCardRuleSnapshotSet PlayerCardSnapshots;
	FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult;
	FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult;
	FSingleCardFormulaResolverInputAssemblyResult ResolverInputAssemblyResult;
	FSingleCardFormulaResolutionExecutionResult FormulaExecutionResult;

	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;
};
