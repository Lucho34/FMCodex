#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayDefendingGoalkeeperQuery.h"
#include "MatchPlayPostRouteRollProvider.h"
#include "MatchPlaySetPieceCarrierAvailability.h"
#include "SingleCardFormulaResolutionExecutor.h"

struct FMCODEX_API FMatchPlayLongFreeKickMethodRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	EMatchPlayLongFreeKickMethod Method =
		EMatchPlayLongFreeKickMethod::None;
};

struct FMCODEX_API FMatchPlayLongFreeKickRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

struct FMCODEX_API FMatchPlayLongFreeKickNoLegalCarrierRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

enum class EMatchPlayLongFreeKickResolutionErrorCode : uint8
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

struct FMCODEX_API FMatchPlayLongFreeKickResolutionResult
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
	EMatchPlayLongFreeKickResolutionErrorCode ErrorCode =
		EMatchPlayLongFreeKickResolutionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayLongFreeKickResolution final
{
public:
	static FMatchPlayLongFreeKickResolutionResult SubmitMethod(
		const FMatchPlayState& BeforeState,
		const FMatchPlayLongFreeKickMethodRequest& Request);

	static FMatchPlayLongFreeKickResolutionResult ResolveDirectAttackRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayLongFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayLongFreeKickResolutionResult ResolveDirectDefenseRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayLongFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayLongFreeKickResolutionResult ResolvePowerRoll(
		const FMatchPlayState& BeforeState,
		const FMatchPlayLongFreeKickRollRequest& Request,
		IMatchPlayPostRouteRollProvider* RollProvider);

	static FMatchPlayLongFreeKickResolutionResult ResolveNoLegalCarrier(
		const FMatchPlayState& BeforeState,
		const FMatchPlayLongFreeKickNoLegalCarrierRequest& Request);

private:
	static bool PersistTerminal(
		FMatchPlayLongFreeKickResolutionResult& Result,
		const FMatchPlayState& BeforeState,
		FMatchPlayState WorkingState,
		EInitialTurnOrderPlayer Attacker,
		bool bGoal);
};
