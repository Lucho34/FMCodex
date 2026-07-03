#include "PlayerCardRuleSnapshotQuery.h"

namespace PlayerCardRuleSnapshotQuery
{
	void SetFailure(
		FPlayerCardRuleSnapshotQueryResult& Result,
		const EPlayerCardRuleSnapshotQueryErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}
}

FPlayerCardRuleSnapshotQueryResult
FPlayerCardRuleSnapshotQuery::FindByCardId(
	const FPlayerCardRuleSnapshotSet& SnapshotSet,
	const FName CardId)
{
	FPlayerCardRuleSnapshotQueryResult Result;
	Result.CardId = CardId;

	if (CardId.IsNone())
	{
		PlayerCardRuleSnapshotQuery::SetFailure(
			Result,
			EPlayerCardRuleSnapshotQueryErrorCode::InvalidCardId,
			TEXT("CardId must not be None."));
		return Result;
	}

	Result.ValidationResult =
		FPlayerCardRuleSnapshotValidator::Validate(SnapshotSet);
	if (!Result.ValidationResult.bSuccess)
	{
		PlayerCardRuleSnapshotQuery::SetFailure(
			Result,
			EPlayerCardRuleSnapshotQueryErrorCode::InvalidSnapshotSet,
			Result.ValidationResult.ErrorMessage);
		return Result;
	}

	for (const FPlayerCardRuleSnapshot& Snapshot : SnapshotSet.Cards)
	{
		if (Snapshot.CardId == CardId)
		{
			Result.bSuccess = true;
			Result.bFound = true;
			Result.Snapshot = Snapshot;
			return Result;
		}
	}

	PlayerCardRuleSnapshotQuery::SetFailure(
		Result,
		EPlayerCardRuleSnapshotQueryErrorCode::CardNotFound,
		FString::Printf(
			TEXT("CardId '%s' was not found in the player card rule snapshot set."),
			*CardId.ToString()));
	return Result;
}
