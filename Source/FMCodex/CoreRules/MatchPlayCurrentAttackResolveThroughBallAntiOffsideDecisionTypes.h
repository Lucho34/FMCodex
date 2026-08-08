#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "ThroughBallAntiOffsideOutcomeQuery.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest
{
	int64 AttackSequence = 0;
};

enum class EMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionErrorCode
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
	NotThroughBallAntiOffsideBranch,
	InvalidPostRouteProgress,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	SkillRuleSetUnavailable,
	ParticipantSnapshotUnavailable,
	BranchSelectionFailed,
	ParticipantEligibilityFailed,
	InputAdaptationFailed,
	AntiOffsideOutcomeQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	int32 ProviderCallCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionErrorCode
				::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult ProgressResult;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult>
		ProviderValidationResults;
	FThroughBallBranchSelectionQueryResult BranchSelectionResult;
	FThroughBallParticipantEligibilityQueryResult ParticipantEligibilityResult;
	FThroughBallAntiOffsideOutcomeQueryInput QueryInput;
	FThroughBallAntiOffsideOutcomeQueryResult OutcomeResult;
	FString ErrorMessage;
};
