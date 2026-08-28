#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionTypes.h"
#include "ThroughBallOneOnOneDirectShotFormula.h"

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
{
	int64 AttackSequence = 0;

	enum class EMode : uint8
	{
		/** Legacy/reference compatibility; production uses explicit rolls. */
		CompletePlan,
		RegenerateCompletedPlan,
		ResolveAttackRoll,
		ResolveDefenseRoll
	};

	EMode Mode = EMode::CompletePlan;
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

enum class EMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackSequence,
	InvalidRequestedAttackSequence,
	AttackSequenceMismatch,
	InvalidRequestingSide,
	WrongRequestingSide,
	WrongDirectShotRollStep,
	CompletedPlanRequired,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	UnsupportedThroughBallBranch,
	OneOnOneShotChoiceNotSelected,
	OneOnOneShotChoiceDoesNotPermitDirectShot,
	InvalidPostRouteProgress,
	IncompleteSourceProvenance,
	UnsupportedSourcePhase,
	InvalidSourceProvenanceState,
	SourceDecisionRegenerationFailed,
	SourceDecisionDoesNotRequireOneOnOne,
	HandoffCreationFailed,
	ShooterIdentityMismatch,
	GoalkeeperSnapshotUnavailable,
	InvalidGoalkeeperSnapshot,
	ActiveGoalkeeperPlacementMismatch,
	UnexpectedDirectShotRollPurpose,
	PostRouteRollProviderUnavailable,
	PostRouteRollProviderFailed,
	MalformedPostRouteRollProviderResult,
	InvalidCandidateState,
	UnrepresentableTurnIndex,
	FormulaPlanCreationFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
{
	bool bSuccess = false;
	bool bResolvedNewRolls = false;
	bool bReplayedCompleteRolls = false;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanRequest
		Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	EMatchPlayThroughBallOneOnOneSource Source = EMatchPlayThroughBallOneOnOneSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult SessionStateValidationResult;
	FMatchPlayCurrentAttackPostRouteRollProgressResult BeforeProgressResult;
	FMatchPlayState SourceProvenanceState;
	FMatchPlayCurrentAttackPostRouteRollProgressResult SourceProvenanceProgressResult;
	int32 SourceDecisionRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult AntiOffsideRegenerationResult;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult BehindDefenseP1RegenerationResult;
	int32 HandoffCreationCount = 0;
	FThroughBallOneOnOneHandoffCreationResult HandoffCreationResult;
	int32 ProviderCallCount = 0;
	TArray<FMatchPlayPostRouteRollProviderResult> ProviderResults;
	TArray<FMatchPlayPostRouteRollProviderResultValidationResult> ProviderValidationResults;
	FMatchPlayCurrentAttackPostRouteRollProgressResult AfterProgressResult;
	FPlayerCardRuleSnapshot GoalkeeperSnapshot;
	bool bGoalkeeperActivated = false;
	bool bHasFormulaPlan = false;
	FThroughBallOneOnOneDirectShotFormulaPlan FormulaPlan;
};
