#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "ThroughBallFeetPlanQuery.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
{
	int64 AttackSequence = 0;
};

enum class
	EMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanErrorCode
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
	NotThroughBallFeetBranch,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	ParticipantSnapshotUnavailable,
	SkillRuleSetUnavailable,
	ParticipantEligibilityFailed,
	FeetInputAdaptationFailed,
	FeetPlanQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanRequest
		Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanErrorCode
				::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	FThroughBallParticipantEligibilityQueryResult
		ParticipantEligibilityResult;
	FThroughBallFeetPlanQueryInput QueryInput;
	FThroughBallFeetPlanQueryResult PlanResult;
	FString ErrorMessage;
};
