#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "PassControlDribbleAdvancePlanQuery.h"
#include "PassControlPassAdvancePlanQuery.h"
#include "PassControlRunAdvancePlanQuery.h"

struct FMCODEX_API FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest
{
	int64 AttackSequence = 0;
};

enum class EMatchPlayCurrentAttackResolvePassControlPostRoutePlanErrorCode
	: uint8
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
	NotPassControlBranch,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	SkillRuleSetUnavailable,
	ParticipantSnapshotUnavailable,
	InputAdaptationFailed,
	PassAdvancePlanQueryFailed,
	DribbleAdvancePlanQueryFailed,
	RunAdvancePlanQueryFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolvePassControlPostRoutePlanRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolvePassControlPostRoutePlanErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolvePassControlPostRoutePlanErrorCode::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	EMatchPlayPassControlActualBranch ActualBranch =
		EMatchPlayPassControlActualBranch::None;
	FPlayerCardRuleSnapshotSet PlayerCardSnapshots;
	FPassControlPassAdvancePlanQueryInput PassAdvanceInput;
	FPassControlPassAdvancePlanQueryResult PassAdvanceResult;
	FPassControlDribbleAdvancePlanQueryInput DribbleAdvanceInput;
	FPassControlDribbleAdvancePlanQueryResult DribbleAdvanceResult;
	FPassControlRunAdvancePlanQueryInput RunAdvanceInput;
	FPassControlRunAdvancePlanQueryResult RunAdvanceResult;
	FString ErrorMessage;
};
