#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardUsageStateValidator.h"
#include "MatchPlayState.h"

enum class EMatchPlaySetPieceParticipantRole : uint8
{
	None,
	Carrier,
	CornerRunner,
	CornerHelper
};

struct FMCODEX_API FMatchPlaySetPieceParticipantEligibilityRequest
{
	EInitialTurnOrderPlayer ExpectedOwnerSide =
		EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	EMatchPlaySetPieceParticipantRole Role =
		EMatchPlaySetPieceParticipantRole::None;
};

enum class EMatchPlaySetPieceParticipantEligibilityErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	InvalidUnderlyingState,
	InvalidExpectedOwnerSide,
	InvalidRole,
	InvalidCardId,
	CardUsed,
	CardEjected,
	CardNotAvailable,
	SnapshotQueryFailed,
	GoalkeeperNotEligible
};

struct FMCODEX_API FMatchPlaySetPieceParticipantEligibilityResult
{
	bool bIsEligible = false;
	FMatchPlaySetPieceParticipantEligibilityRequest Request;
	FMatchPlaySetPieceParticipantBinding Binding;
	FMatchPlayCardUsageStateValidationResult CardUsageValidationResult;
	FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQueryResult;
	EMatchPlaySetPieceParticipantEligibilityErrorCode ErrorCode =
		EMatchPlaySetPieceParticipantEligibilityErrorCode::None;
	FString ErrorMessage;
};

/**
 * Pure match-authority eligibility for hand-based Set Piece participants.
 * Method-specific attributes intentionally remain outside this common boundary.
 */
class FMCODEX_API FMatchPlaySetPieceParticipantEligibility final
{
public:
	static FMatchPlaySetPieceParticipantEligibilityResult Evaluate(
		const FMatchPlayState& State,
		const FMatchPlaySetPieceParticipantEligibilityRequest& Request);
};
