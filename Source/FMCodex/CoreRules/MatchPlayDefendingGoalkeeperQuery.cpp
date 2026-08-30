#include "MatchPlayDefendingGoalkeeperQuery.h"

namespace MatchPlayDefendingGoalkeeperQuery
{
	bool IsPlayer(const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			|| Side == EInitialTurnOrderPlayer::PlayerB;
	}

	void Fail(
		FMatchPlayDefendingGoalkeeperQueryResult& Result,
		const EMatchPlayDefendingGoalkeeperQueryErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FMatchPlayDefendingGoalkeeperQueryResult
FMatchPlayDefendingGoalkeeperQuery::Query(
	const FMatchPlayState& State,
	const EInitialTurnOrderPlayer DefendingSide)
{
	using namespace MatchPlayDefendingGoalkeeperQuery;

	FMatchPlayDefendingGoalkeeperQueryResult Result;
	Result.DefendingSide = DefendingSide;
	if (!State.RuntimeState.bIsInitialized)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Defending goalkeeper lookup requires initialized match play state."));
		return Result;
	}
	if (!IsPlayer(DefendingSide))
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode::InvalidDefendingSide,
			TEXT("DefendingSide must be PlayerA or PlayerB."));
		return Result;
	}

	Result.CardUsageValidationResult =
		FMatchPlayCardUsageStateValidator::Validate(
			State.CardUsageState,
			State.CardSnapshotAuthority);
	if (!Result.CardUsageValidationResult.bIsCanonical)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::InvalidUnderlyingState,
			Result.CardUsageValidationResult.ErrorMessage);
		return Result;
	}

	const FPlayerRuntimeState& RuntimePlayer =
		DefendingSide == EInitialTurnOrderPlayer::PlayerA
			? State.RuntimeState.PlayerAState
			: State.RuntimeState.PlayerBState;
	Result.CardId = FName(*RuntimePlayer.GoalkeeperCardId);
	if (Result.CardId.IsNone())
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::MissingRuntimeGoalkeeperIdentity,
			TEXT("Defending side requires an authoritative goalkeeper CardId."));
		return Result;
	}

	const FPlayerCardRuleSnapshotSet& Snapshots =
		DefendingSide == EInitialTurnOrderPlayer::PlayerA
			? State.CardSnapshotAuthority.PlayerACardSnapshots
			: State.CardSnapshotAuthority.PlayerBCardSnapshots;
	int32 GoalkeeperCount = 0;
	FName UniqueGoalkeeperCardId = NAME_None;
	for (const FPlayerCardRuleSnapshot& Snapshot : Snapshots.Cards)
	{
		if (Snapshot.bIsGoalkeeper)
		{
			++GoalkeeperCount;
			UniqueGoalkeeperCardId = Snapshot.CardId;
		}
	}
	if (GoalkeeperCount != 1)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::GoalkeeperCountMismatch,
			TEXT("Defending side must have exactly one authoritative goalkeeper snapshot."));
		return Result;
	}
	if (UniqueGoalkeeperCardId != Result.CardId)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::GoalkeeperIdentityMismatch,
			TEXT("Runtime goalkeeper identity must match the unique side-owned goalkeeper snapshot."));
		return Result;
	}

	Result.SnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			DefendingSide,
			Result.CardId);
	if (!Result.SnapshotQueryResult.bSuccess)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode::SnapshotQueryFailed,
			Result.SnapshotQueryResult.ErrorMessage);
		return Result;
	}
	if (!Result.SnapshotQueryResult.Snapshot.bIsGoalkeeper
		|| !Result.SnapshotQueryResult.Snapshot.bHasGoalkeeperAttributes)
	{
		Fail(Result,
			EMatchPlayDefendingGoalkeeperQueryErrorCode
				::InvalidGoalkeeperSnapshot,
			TEXT("Defending goalkeeper snapshot must carry canonical GK identity and attributes."));
		return Result;
	}

	Result.Snapshot = Result.SnapshotQueryResult.Snapshot;
	Result.bSuccess = true;
	return Result;
}
