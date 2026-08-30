#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRouteStateValidator.h"
#include "MatchPlaySetPieceParticipantEligibility.h"

struct FMCODEX_API FMatchPlaySetPieceCarrierAvailabilityRequest
{
	int64 AttackSequence = 0;
};

enum class EMatchPlaySetPieceCarrierAvailabilityErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	InvalidAttackSequence,
	AttackSequenceMismatch,
	InvalidCurrentAttackingPlayer,
	WrongRoute,
	UnsupportedSetPieceType,
	WrongStage,
	InvalidRouteState,
	ParticipantEligibilityFailed
};

struct FMCODEX_API FMatchPlaySetPieceCarrierAvailabilityResult
{
	bool bSuccess = false;
	bool bHasLegalCarrier = false;
	FMatchPlaySetPieceCarrierAvailabilityRequest Request;
	EInitialTurnOrderPlayer AttackingSide = EInitialTurnOrderPlayer::None;
	ESetPieceSelectedType SetPieceType = ESetPieceSelectedType::None;
	TArray<FName> LegalCarrierCardIds;
	FMatchPlayCurrentAttackRouteStateValidationResult RouteValidationResult;
	TArray<FMatchPlaySetPieceParticipantEligibilityResult>
		ParticipantEligibilityResults;
	EMatchPlaySetPieceCarrierAvailabilityErrorCode ErrorCode =
		EMatchPlaySetPieceCarrierAvailabilityErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlaySetPieceCarrierAvailability final
{
public:
	static FMatchPlaySetPieceCarrierAvailabilityResult Query(
		const FMatchPlayState& State,
		const FMatchPlaySetPieceCarrierAvailabilityRequest& Request);
};
