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

/** Read-only values from the same input assembler used by final Corner resolution. */
struct FMCODEX_API FMatchPlayCornerFormulaPreview
{
	bool bAvailable = false;
	float AttackKnownSubtotal = 0.0f;
	float DefenseKnownSubtotal = 0.0f;
	float AttackCurrentTotal = 0.0f;
	float DefenseCurrentTotal = 0.0f;
};

class FMCODEX_API FMatchPlayCornerResolution final
{
public:
	/** Independent mapping for each ordered side; invalid count/roll returns INDEX_NONE. */
	static int32 MapParticipantIndex(int32 CandidateCount, int32 RawD6);
	static FFormulaResolverInput BuildFormulaInput(
		const FMatchPlayCornerRouteState& Corner,
		const FMatchPlayDefendingGoalkeeperQueryResult& Goalkeeper,
		EInitialTurnOrderPlayer Attacker, EInitialTurnOrderPlayer Defender,
		int64 AttackSequence);
	static FMatchPlayCornerFormulaPreview QueryFormulaPreview(const FMatchPlayState& State);

	static FMatchPlayCornerResolutionResult SubmitAttackerNominations(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerNominationRequest& Request);

	static FMatchPlayCornerResolutionResult SubmitDefenderNominations(
		const FMatchPlayState& BeforeState,
		const FMatchPlayCornerNominationRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider = nullptr);

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
