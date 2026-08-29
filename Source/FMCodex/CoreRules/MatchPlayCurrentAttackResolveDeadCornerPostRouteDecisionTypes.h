#pragma once

#include "CoreMinimal.h"

#include "CutInsideShotDeadCornerDecisionQuery.h"
#include "LongShotDeadCornerDecisionQuery.h"
#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest
{
	int64 AttackSequence = 0;

	enum class EMode : uint8
	{
		/** Legacy/reference compatibility; production must use explicit rolls. */
		CompleteDecision,
		RegenerateCompletedDecision,
		ResolveLongShotPairedRolls
	};

	EMode Mode = EMode::CompleteDecision;
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

enum class EMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionErrorCode
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
	NotDeadCornerBranch,
	NotLongShotDeadCornerBranch,
	InvalidRequestingSide,
	WrongRequestingSide,
	WrongLongShotDeadCornerRollStep,
	CompletedDecisionRequired,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	SkillRuleSetUnavailable,
	AttackerSnapshotUnavailable,
	InputAdaptationFailed,
	LongShotDecisionQueryFailed,
	CutInsideShotDecisionQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionErrorCode::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	ESkillRuleType ActionType = ESkillRuleType::None;
	FPlayerCardRuleSnapshotSet PlayerCardSnapshots;
	FLongShotDeadCornerDecisionQueryInput LongShotInput;
	FLongShotDeadCornerDecisionQueryResult LongShotResult;
	FCutInsideShotDeadCornerDecisionQueryInput CutInsideShotInput;
	FCutInsideShotDeadCornerDecisionQueryResult CutInsideShotResult;
	FString ErrorMessage;
};
