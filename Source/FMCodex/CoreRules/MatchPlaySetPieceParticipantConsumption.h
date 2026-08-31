#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackRouteStateValidator.h"

struct FMCODEX_API FMatchPlaySetPieceParticipantToConsume
{
	EInitialTurnOrderPlayer OwnerSide = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
};

enum class EMatchPlaySetPieceParticipantConsumptionErrorCode : uint8
{
	None,
	NoCurrentAttack,
	WrongRoute,
	InvalidRouteState,
	UnsupportedSetPieceType,
	InvalidShortFreeKickTerminal,
	InvalidLongFreeKickTerminal,
	InvalidPenaltyTerminal,
	InvalidCornerTerminal
};

struct FMCODEX_API FMatchPlaySetPieceParticipantConsumptionResult
{
	bool bSuccess = false;
	TArray<FMatchPlaySetPieceParticipantToConsume> Participants;
	FMatchPlayCurrentAttackRouteStateValidationResult RouteValidationResult;
	EMatchPlaySetPieceParticipantConsumptionErrorCode ErrorCode =
		EMatchPlaySetPieceParticipantConsumptionErrorCode::None;
	FString ErrorMessage;
};

/** Route-neutral extraction seam used by shared Advance card consumption. */
class FMCODEX_API FMatchPlaySetPieceParticipantConsumption final
{
public:
	static FMatchPlaySetPieceParticipantConsumptionResult Extract(
		const FMatchPlayState& State);
};
