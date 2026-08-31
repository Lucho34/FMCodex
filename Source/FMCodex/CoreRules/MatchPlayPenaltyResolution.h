#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "MatchPlaySetPieceCarrierAvailability.h"
#include "SingleCardFormulaResolutionExecutor.h"

struct FMCODEX_API FMatchPlayPenaltyMethodRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	EMatchPlayPenaltyMethod Method = EMatchPlayPenaltyMethod::None;
};

struct FMCODEX_API FMatchPlayPenaltyRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

struct FMCODEX_API FMatchPlayPenaltyNoLegalCarrierRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

enum class EMatchPlayPenaltyResolutionErrorCode : uint8
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
	InvalidMethod,
	InvalidRouteState,
	DefendingGoalkeeperUnavailable,
	ProviderUnavailable,
	ProviderFailure,
	MalformedProviderResult,
	FormulaResolutionFailed,
	GoalResolutionFailed,
	LegalCarrierExists,
	CarrierAvailabilityFailed,
	TerminalPersistenceFailed,
	InvalidCandidateRouteState
};

struct FMCODEX_API FMatchPlayPenaltyResolutionResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackRouteStateValidationResult BeforeRouteValidation;
	FMatchPlayCurrentAttackRouteStateValidationResult AfterRouteValidation;
	FMatchPlayDefendingGoalkeeperQueryResult GoalkeeperQueryResult;
	FMatchPlayPostRouteRollProviderResult ProviderResult;
	FMatchPlayPostRouteRollProviderResultValidationResult ProviderValidation;
	FSingleCardFormulaResolutionExecutionResult FormulaExecutionResult;
	FMatchPlaySetPieceCarrierAvailabilityResult CarrierAvailabilityResult;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
	EMatchPlayPenaltyResolutionErrorCode ErrorCode =
		EMatchPlayPenaltyResolutionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayPenaltyResolution final
{
public:
	static FMatchPlayPenaltyResolutionResult SubmitMethod(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPenaltyMethodRequest& Request);

	static FMatchPlayPenaltyResolutionResult ResolveDirectAttackRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPenaltyRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayPenaltyResolutionResult ResolveDirectDefenseRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPenaltyRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayPenaltyResolutionResult ResolvePanenkaRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPenaltyRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayPenaltyResolutionResult ResolveNoLegalCarrier(
		const FMatchPlayState& BeforeState,
		const FMatchPlayPenaltyNoLegalCarrierRequest& Request);

private:
	static bool PersistTerminal(
		FMatchPlayPenaltyResolutionResult& Result,
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		EInitialTurnOrderPlayer Attacker,
		bool bGoal);
};
