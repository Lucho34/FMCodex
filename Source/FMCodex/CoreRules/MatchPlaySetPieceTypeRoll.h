#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAttackEntryRollProvider.h"
#include "MatchPlayCurrentAttackRouteStateValidator.h"
#include "SetPieceTypeSelectionQuery.h"

struct FMCODEX_API FMatchPlaySetPieceTypeRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
};

enum class EMatchPlaySetPieceTypeRollErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidAttackSequence,
	AttackSequenceMismatch,
	WrongRoute,
	WrongStage,
	InvalidCurrentAttackingPlayer,
	InvalidRequestingSide,
	RequestingSideNotCurrentAttacker,
	ProviderUnavailable,
	ProviderFailure,
	MalformedProviderResult,
	TypeSelectionFailed,
	InvalidRouteState
};

struct FMCODEX_API FMatchPlaySetPieceTypeRollResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlaySetPieceTypeRollRequest Request;
	FMatchPlayAttackEntryRollProviderResult ProviderResult;
	FMatchPlayAttackEntryRollProviderResultValidationResult
		ProviderValidationResult;
	FSetPieceTypeSelectionQueryResult SelectionResult;
	FMatchPlayCurrentAttackRouteStateValidationResult RouteValidationResult;
	EMatchPlaySetPieceTypeRollErrorCode ErrorCode =
		EMatchPlaySetPieceTypeRollErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySetPieceTypeRoll final
{
public:
	static FMatchPlaySetPieceTypeRollResult Resolve(
		const FMatchPlayState& BeforeState,
		const FMatchPlaySetPieceTypeRollRequest& Request,
		IMatchPlayAttackEntryRollProvider* RollProvider);
};
