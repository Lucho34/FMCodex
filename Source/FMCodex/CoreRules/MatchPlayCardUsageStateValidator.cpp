#include "MatchPlayCardUsageStateValidator.h"

namespace MatchPlayCardUsageStateValidator
{
	const FCardUsageState& GetUsage(
		const FMatchCardUsageState& State,
		const EInitialTurnOrderPlayer Side)
	{
		return Side == EInitialTurnOrderPlayer::PlayerA
			? State.PlayerACardUsageState
			: State.PlayerBCardUsageState;
	}

	bool ValidateZoneProvenance(
		FMatchPlayCardUsageStateValidationResult& Result,
		const FMatchPlayPerSideCardSnapshotAuthority& Authority,
		const EInitialTurnOrderPlayer Side,
		const TArray<FName>& CardIds,
		const bool bIsEjectedZone)
	{
		for (const FName CardId : CardIds)
		{
			const FMatchPlayCardSnapshotAuthorityQueryResult Query =
				FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
					Authority,
					Side,
					CardId);
			if (!Query.bSuccess)
			{
				Result.ErrorCode = EMatchPlayCardUsageStateValidationErrorCode
					::CardSnapshotQueryFailed;
				Result.FailingPlayerSide = Side;
				Result.InvalidCardId = CardId;
				Result.UnderlyingSnapshotErrorCode = Query.ErrorCode;
				Result.ErrorMessage = Query.ErrorMessage;
				return false;
			}

			if (bIsEjectedZone && Query.Snapshot.bIsGoalkeeper)
			{
				Result.ErrorCode = EMatchPlayCardUsageStateValidationErrorCode
					::GoalkeeperInEjectedZone;
				Result.FailingPlayerSide = Side;
				Result.InvalidCardId = CardId;
				Result.ErrorMessage = FString::Printf(
					TEXT("Goalkeeper CardId '%s' cannot exist in EjectedCardIds."),
					*CardId.ToString());
				return false;
			}
		}
		return true;
	}
}

FMatchPlayCardUsageStateValidationResult
FMatchPlayCardUsageStateValidator::Validate(
	const FMatchCardUsageState& CardUsageState,
	const FMatchPlayPerSideCardSnapshotAuthority& CardSnapshotAuthority)
{
	using namespace MatchPlayCardUsageStateValidator;

	FMatchPlayCardUsageStateValidationResult Result;
	for (const EInitialTurnOrderPlayer Side : {
		EInitialTurnOrderPlayer::PlayerA,
		EInitialTurnOrderPlayer::PlayerB })
	{
		const FCardUsageState& Usage = GetUsage(CardUsageState, Side);
		const FCardUsageResolveResult UsageValidation =
			FCardUsageResolver::ValidateState(Usage);
		if (!UsageValidation.bSuccess)
		{
			Result.ErrorCode = Side == EInitialTurnOrderPlayer::PlayerA
				? EMatchPlayCardUsageStateValidationErrorCode
					::InvalidPlayerACardUsage
				: EMatchPlayCardUsageStateValidationErrorCode
					::InvalidPlayerBCardUsage;
			Result.FailingPlayerSide = Side;
			Result.UnderlyingCardUsageErrorCode =
				UsageValidation.ErrorCode;
			Result.ErrorMessage = UsageValidation.ErrorMessage;
			return Result;
		}

		if (!ValidateZoneProvenance(
				Result,
				CardSnapshotAuthority,
				Side,
				Usage.AvailableCardIds,
				false)
			|| !ValidateZoneProvenance(
				Result,
				CardSnapshotAuthority,
				Side,
				Usage.UsedCardIds,
				false)
			|| !ValidateZoneProvenance(
				Result,
				CardSnapshotAuthority,
				Side,
				Usage.EjectedCardIds,
				true))
		{
			return Result;
		}
	}

	Result.bIsCanonical = true;
	return Result;
}
