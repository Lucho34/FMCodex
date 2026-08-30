#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "MatchPlaySetPieceCarrierAvailability.h"
#include "SingleCardFormulaResolutionExecutor.h"

struct FMCODEX_API FMatchPlayShortFreeKickMethodRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	EMatchPlayShortFreeKickMethod Method =
		EMatchPlayShortFreeKickMethod::None;
};

struct FMCODEX_API FMatchPlayShortFreeKickRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

struct FMCODEX_API FMatchPlayShortFreeKickNoLegalCarrierRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

enum class EMatchPlayShortFreeKickResolutionErrorCode : uint8
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
	AngledMethodNotEligible,
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

struct FMCODEX_API FMatchPlayShortFreeKickResolutionResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackRouteStateValidationResult BeforeRouteValidation;
	FMatchPlayCurrentAttackRouteStateValidationResult AfterRouteValidation;
	FMatchPlayDefendingGoalkeeperQueryResult GoalkeeperQueryResult;
	FMatchPlayPostRouteRollProviderResult FirstProviderResult;
	FMatchPlayPostRouteRollProviderResult SecondProviderResult;
	FMatchPlayPostRouteRollProviderResultValidationResult
		FirstProviderValidation;
	FMatchPlayPostRouteRollProviderResultValidationResult
		SecondProviderValidation;
	FSingleCardFormulaResolutionExecutionResult FormulaExecutionResult;
	FMatchPlaySetPieceCarrierAvailabilityResult CarrierAvailabilityResult;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
	EMatchPlayShortFreeKickResolutionErrorCode ErrorCode =
		EMatchPlayShortFreeKickResolutionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayShortFreeKickResolution final
{
public:
	static FMatchPlayShortFreeKickResolutionResult SubmitMethod(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShortFreeKickMethodRequest& Request);

	static FMatchPlayShortFreeKickResolutionResult ResolveDirectAttackRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShortFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayShortFreeKickResolutionResult ResolveDirectDefenseRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShortFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayShortFreeKickResolutionResult ResolveAngledRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShortFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayShortFreeKickResolutionResult ResolveNoLegalCarrier(
		const FMatchPlayState& BeforeState,
		const FMatchPlayShortFreeKickNoLegalCarrierRequest& Request);

private:
	static bool PersistTerminal(
		FMatchPlayShortFreeKickResolutionResult& Result,
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		EInitialTurnOrderPlayer Attacker,
		bool bGoal);
};
