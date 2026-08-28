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

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionRequest
{
	int64 AttackSequence = 0;

	enum class EMode : uint8
	{
		/** Legacy/reference compatibility; production uses ResolveAttackRoll. */
		CompleteDecision,
		RegenerateCompletedDecision,
		ResolveAttackRoll
	};

	EMode Mode = EMode::CompleteDecision;
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

enum class
	EMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackSequence,
	InvalidRequestedAttackSequence,
	AttackSequenceMismatch,
	InvalidRequestingSide,
	WrongRequestingSide,
	WrongChipShotRollStep,
	CompletedDecisionRequired,
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
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionRequest
		Request;
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
