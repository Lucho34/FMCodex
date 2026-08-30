#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRouteStateValidator.h"
#include "MatchPlaySetPieceParticipantEligibility.h"

struct FMCODEX_API FMatchPlaySetPieceCarrierSelectionRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	int64 AttackSequence = 0;
	FName CardId = NAME_None;
};

enum class EMatchPlaySetPieceCarrierSelectionErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackSequence,
	InvalidRequestAttackSequence,
	AttackSequenceMismatch,
	WrongRoute,
	InvalidRouteState,
	UnsupportedSetPieceType,
	WrongStage,
	InvalidCurrentAttackingPlayer,
	InvalidRequestingSide,
	RequestingSideNotCurrentAttacker,
	ParticipantNotEligible,
	InvalidCandidateRouteState
};

struct FMCODEX_API FMatchPlaySetPieceCarrierSelectionResult
{
	bool bSuccess = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlaySetPieceCarrierSelectionRequest Request;
	ESetPieceSelectedType SetPieceType = ESetPieceSelectedType::None;
	FMatchPlaySetPieceParticipantEligibilityResult EligibilityResult;
	FMatchPlayCurrentAttackRouteStateValidationResult
		BeforeRouteValidationResult;
	FMatchPlayCurrentAttackRouteStateValidationResult
		AfterRouteValidationResult;
	EMatchPlaySetPieceCarrierSelectionErrorCode ErrorCode =
		EMatchPlaySetPieceCarrierSelectionErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySetPieceCarrierSelection final
{
public:
	static FMatchPlaySetPieceCarrierSelectionResult Submit(
		const FMatchPlayState& BeforeState,
		const FMatchPlaySetPieceCarrierSelectionRequest& Request);
};
