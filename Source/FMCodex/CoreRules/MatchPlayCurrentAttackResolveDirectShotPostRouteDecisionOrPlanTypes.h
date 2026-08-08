#pragma once

#include "CoreMinimal.h"

#include "CutInsideShotDirectShotPlanQuery.h"
#include "LongShotDirectShotPlanQuery.h"
#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
{
	int64 AttackSequence = 0;
};

enum class
	EMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanErrorCode
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
	NotDirectShotBranch,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	SkillRuleSetUnavailable,
	ParticipantSnapshotUnavailable,
	InputAdaptationFailed,
	LongShotPlanQueryFailed,
	CutInsideShotPlanQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanRequest
		Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanErrorCode
				::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	ESkillRuleType ActionType = ESkillRuleType::None;
	FPlayerCardRuleSnapshotSet PlayerCardSnapshots;
	FLongShotDirectShotPlanQueryInput LongShotInput;
	FLongShotDirectShotPlanQueryResult LongShotResult;
	FCutInsideShotDirectShotPlanQueryInput CutInsideShotInput;
	FCutInsideShotDirectShotPlanQueryResult CutInsideShotResult;
	FString ErrorMessage;
};
