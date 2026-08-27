#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "ThroughBallBehindDefenseP1PlanQuery.h"

struct FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest
{
	int64 AttackSequence = 0;

	enum class EMode : uint8
	{
		/** Legacy/reference compatibility; production must use explicit rolls. */
		CompleteP1Plan,
		RegenerateCompletedPlan,
		ResolveAttackRoll,
		ResolveDefenseRoll
	};

	EMode Mode = EMode::CompleteP1Plan;
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

enum class EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanErrorCode : uint8
{
	None, MatchPlayStateNotInitialized, NoCurrentAttack, InvalidCurrentAttackSequence,
	InvalidRequestedAttackSequence, AttackSequenceMismatch, MissingResolutionSession,
	InvalidResolutionSessionState, RouteNotResolved, NotThroughBallBehindDefenseBranch,
	InvalidRequestingSide, WrongRequestingSide, WrongBehindDefenseRollStep,
	CompletedPlanRequired,
	InvalidPostRouteProgress, PostRouteRollProviderUnavailable, PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult, SkillRuleSetUnavailable, ParticipantSnapshotUnavailable,
	ParticipantEligibilityFailed, InputAdaptationFailed, P1PlanQueryFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanErrorCode::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult> ProviderValidationResults;
	FThroughBallParticipantEligibilityQueryResult ParticipantEligibilityResult;
	FThroughBallBehindDefenseP1PlanQueryInput QueryInput;
	FThroughBallBehindDefenseP1PlanQueryResult P1PlanResult;
	FString ErrorMessage;
};
