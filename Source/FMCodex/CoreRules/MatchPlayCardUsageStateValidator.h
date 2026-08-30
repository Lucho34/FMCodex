#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardSnapshotAuthority.h"
#include "PlayCardResolver.h"

enum class EMatchPlayCardUsageStateValidationErrorCode : uint8
{
	None,
	InvalidPlayerACardUsage,
	InvalidPlayerBCardUsage,
	CardSnapshotQueryFailed,
	GoalkeeperInEjectedZone
};

struct FMCODEX_API FMatchPlayCardUsageStateValidationResult
{
	bool bIsCanonical = false;
	EMatchPlayCardUsageStateValidationErrorCode ErrorCode =
		EMatchPlayCardUsageStateValidationErrorCode::None;
	EInitialTurnOrderPlayer FailingPlayerSide =
		EInitialTurnOrderPlayer::None;
	FName InvalidCardId = NAME_None;
	ECardUsageResolveErrorCode UnderlyingCardUsageErrorCode =
		ECardUsageResolveErrorCode::None;
	EMatchPlayCardSnapshotAuthorityQueryErrorCode UnderlyingSnapshotErrorCode =
		EMatchPlayCardSnapshotAuthorityQueryErrorCode::None;
	FString ErrorMessage;
};

class FMCODEX_API FMatchPlayCardUsageStateValidator final
{
public:
	static FMatchPlayCardUsageStateValidationResult Validate(
		const FMatchCardUsageState& CardUsageState,
		const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority);
};
