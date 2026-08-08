#pragma once

#include "CoreMinimal.h"

#include "CrossPlanQuery.h"
#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest
{
	int64 AttackSequence = 0;
};

enum class EMatchPlayCurrentAttackResolveCrossPostRoutePlanErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackSequence,
	InvalidRequestedAttackSequence,
	AttackSequenceMismatch,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotCrossBranch,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	ParticipantSnapshotUnavailable,
	SkillRuleSetUnavailable,
	CrossInputAdaptationFailed,
	CrossPlanQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveCrossPostRoutePlanErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveCrossPostRoutePlanErrorCode::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	FPlayerCardRuleSnapshotSet ScopedPlayerCardSnapshots;
	FCrossPlanQueryInput QueryInput;
	FCrossPlanQueryResult PlanResult;
	FString ErrorMessage;
};
