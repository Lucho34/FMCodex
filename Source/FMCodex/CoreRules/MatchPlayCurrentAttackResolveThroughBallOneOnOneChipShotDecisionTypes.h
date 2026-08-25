#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackPostRouteRollProgressQuery.h"
#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaTypes.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "ThroughBallOneOnOneChipShotOutcomeQuery.h"

enum class EMatchPlayThroughBallOneOnOneSource : uint8
{
	None,
	AntiOffside,
	BehindDefense,
	/** Legacy source name retained as a non-canonical alias. */
	BehindDefenseP2 = BehindDefense
};

enum class
	EMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	UnsupportedThroughBallBranch,
	OneOnOneShotChoiceNotSelected,
	OneOnOneShotChoiceDoesNotPermitChipShot,
	InvalidPostRouteProgress,
	IncompleteSourceProvenance,
	UnsupportedSourcePhase,
	InvalidSourceProvenanceState,
	SourceDecisionRegenerationFailed,
	SourceDecisionDoesNotRequireOneOnOne,
	HandoffCreationFailed,
	UnexpectedChipShotRollPurpose,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	InvalidCandidateState,
	UnrepresentableTurnIndex,
	ChipShotOutcomeQueryFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult
{
	bool bSuccess = false;
	bool bResolvedNewRoll = false;
	bool bReplayedAcceptedRoll = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionErrorCode
				::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	EMatchPlayThroughBallOneOnOneSource Source =
		EMatchPlayThroughBallOneOnOneSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult BeforeProgressResult;
	FMatchPlayState SourceProvenanceState;
	FMatchPlayCurrentAttackPostRouteRollProgressResult
		SourceProvenanceProgressResult;
	int32 SourceDecisionRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
		AntiOffsideRegenerationResult;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
		BehindDefenseP1RegenerationResult;

	int32 HandoffCreationCount = 0;
	FThroughBallOneOnOneHandoffCreationResult HandoffCreationResult;

	int32 ProviderCallCount = 0;
	FMatchPlayPostRouteRollProviderResult ProviderResult;
	FMatchPlayPostRouteRollProviderResultValidationResult
		ProviderValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult AfterProgressResult;

	int32 ChipShotQueryExecutionCount = 0;
	FThroughBallOneOnOneChipShotOutcomeQueryInput QueryInput;
	FThroughBallOneOnOneChipShotOutcomeQueryResult QueryResult;
};
