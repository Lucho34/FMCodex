#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "MatchPlaySetPieceParticipantEligibility.h"

struct FMCODEX_API FMatchPlayCornerNominationRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	TArray<FName> OrderedCardIds;
};

struct FMCODEX_API FMatchPlayCornerIntentRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	EMatchPlayCornerRouteIntent IntendedRoute =
		EMatchPlayCornerRouteIntent::None;
};

struct FMCODEX_API FMatchPlayCornerRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

enum class EMatchPlayCornerResolutionErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidAttackSequence,
	AttackSequenceMismatch,
	WrongRoute,
	WrongSetPieceType,
	WrongStage,
	InvalidRequestingSide,
	UnauthorizedRequester,
	TooManyNominees,
	DuplicateNominee,
	NomineeNotEligible,
	InvalidIntent,
	InvalidRouteState,
	DefendingGoalkeeperUnavailable,
	ProviderUnavailable,
	ProviderFailure,
	MalformedProviderResult,
	FormulaResolutionFailed,
	GoalResolutionFailed,
	TerminalPersistenceFailed,
	InvalidCandidateRouteState
};

struct FMCODEX_API FMatchPlayCornerResolutionResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackRouteStateValidationResult BeforeRouteValidation;
	FMatchPlayCurrentAttackRouteStateValidationResult AfterRouteValidation;
	TArray<FMatchPlaySetPieceParticipantEligibilityResult>
		NomineeEligibilityResults;
	FMatchPlayDefendingGoalkeeperQueryResult GoalkeeperQueryResult;
	FMatchPlayPostRouteRollProviderResult ProviderResult;
	FMatchPlayPostRouteRollProviderResultValidationResult ProviderValidation;
	FFormulaResolverInput FormulaInput;
	FFormulaResolutionResult FormulaResolution;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
	EMatchPlayCornerResolutionErrorCode ErrorCode =
		EMatchPlayCornerResolutionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCornerResolution final
{
public:
	static FMatchPlayCornerResolutionResult SubmitAttackerNominations(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerNominationRequest& Request);

	static FMatchPlayCornerResolutionResult SubmitDefenderNominations(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerNominationRequest& Request);

	static FMatchPlayCornerResolutionResult RequestParticipantSelectionRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayCornerResolutionResult SubmitIntent(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerIntentRequest& Request);

	static FMatchPlayCornerResolutionResult RequestRouteRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayCornerResolutionResult RequestAttackRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayCornerResolutionResult RequestDefenseRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

private:
	static bool PersistTerminal(
		FMatchPlayCornerResolutionResult& Result,
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		EInitialTurnOrderPlayer Attacker,
		EMatchPlayCornerGameplayOutcome Outcome);
};
