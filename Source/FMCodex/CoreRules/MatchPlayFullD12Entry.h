#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackEntryRollProvider.h"
#include "MatchPlayBeginOrdinaryAttack.h"
#include "MatchPlayCurrentAttackRouteStateValidator.h"

struct FMCODEX_API FMatchPlayFullD12EntryRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 ExpectedAttackSequence = 0;
};

enum class EMatchPlayFullD12EntryErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	CurrentAttackAlreadyActive,
	InvalidPlayerAAttackCountState,
	InvalidPlayerBAttackCountState,
	MatchAlreadyEnded,
	InvalidCurrentAttackingPlayer,
	InvalidRequestingSide,
	RequestingSideNotCurrentAttacker,
	CurrentAttackerHasNoRemainingAttackOpportunity,
	InvalidDerivedAttackSequence,
	InvalidExpectedAttackSequence,
	AttackSequenceMismatch,
	ProviderUnavailable,
	ProviderFailure,
	MalformedProviderResult,
	OrdinaryBeginFailed,
	InvalidRouteState
};

struct FMCODEX_API FMatchPlayFullD12EntryResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayFullD12EntryRequest Request;
	int64 AuthoritativeAttackSequence = 0;
	FMatchPlayAttackEntryRollProviderResult ProviderResult;
	FMatchPlayAttackEntryRollProviderResultValidationResult
		ProviderValidationResult;
	FMatchPlayBeginOrdinaryAttackResult OrdinaryBeginResult;
	FMatchPlayCurrentAttackRouteStateValidationResult RouteValidationResult;
	EMatchPlayFullD12EntryErrorCode ErrorCode =
		EMatchPlayFullD12EntryErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayFullD12Entry final
{
public:
	static FMatchPlayFullD12EntryResult Enter(
		const FMatchPlayState& BeforeState,
		const FMatchPlayFullD12EntryRequest& Request,
		IMatchPlayAttackEntryRollProvider* RollProvider);
};
