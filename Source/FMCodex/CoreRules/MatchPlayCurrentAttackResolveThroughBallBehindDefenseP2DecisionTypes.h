#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaTypes.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "ThroughBallBehindDefenseP2OutcomeQuery.h"

enum class
	EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionErrorCode
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
	IncompleteP1Progress,
	UnsupportedPostRoutePhase,
	InvalidP1ProvenanceState,
	P1FormulaRegenerationFailed,
	P1FormulaUnavailable,
	P1ResultDoesNotRequireP2,
	UnexpectedP2RollPurpose,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	InvalidCandidateState,
	P2OutcomeQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult
{
	bool bSuccess = false;
	bool bResolvedNewRoll = false;
	bool bReplayedAcceptedRoll = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionErrorCode
				::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult BeforeProgressResult;
	FMatchPlayState P1ProvenanceState;
	FMatchPlayCurrentAttackPostRouteRollProgressResult
		P1ProvenanceProgressResult;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
		P1FormulaRegenerationResult;

	int32 ProviderCallCount = 0;
	FMatchPlayPostRouteRollProviderResult ProviderResult;
	FMatchPlayPostRouteRollProviderResultValidationResult
		ProviderValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult AfterProgressResult;

	int32 P2QueryExecutionCount = 0;
	FThroughBallBehindDefenseP2OutcomeQueryInput QueryInput;
	FThroughBallBehindDefenseP2OutcomeQueryResult QueryResult;
};
