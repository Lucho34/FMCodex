#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCardUsageStateValidator.h"
#include "MatchPlayState.h"

enum class EMatchPlayDefendingGoalkeeperQueryErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	InvalidDefendingSide,
	InvalidUnderlyingState,
	MissingRuntimeGoalkeeperIdentity,
	GoalkeeperCountMismatch,
	GoalkeeperIdentityMismatch,
	SnapshotQueryFailed,
	InvalidGoalkeeperSnapshot
};

struct FMCODEX_API FMatchPlayDefendingGoalkeeperQueryResult
{
	bool bSuccess = false;
	EInitialTurnOrderPlayer DefendingSide = EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FPlayerCardRuleSnapshot Snapshot;
	FMatchPlayCardUsageStateValidationResult CardUsageValidationResult;
	FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQueryResult;
	EMatchPlayDefendingGoalkeeperQueryErrorCode ErrorCode =
		EMatchPlayDefendingGoalkeeperQueryErrorCode::None;
	FString ErrorMessage;
};

/** Pure unique defending-GK authority lookup; it never activates or consumes GK. */
class FMCODEX_API FMatchPlayDefendingGoalkeeperQuery final
{
public:
	static FMatchPlayDefendingGoalkeeperQueryResult Query(
		const FMatchPlayState& State,
		EInitialTurnOrderPlayer DefendingSide);
};
