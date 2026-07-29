#include "MatchPlayCurrentAttackHelperParticipantAuthority.h"

FMatchPlayCurrentAttackHelperParticipantAuthorityResult
FMatchPlayCurrentAttackHelperParticipantAuthority::Evaluate(
	const FMatchPlayState& State,
	const EInitialTurnOrderPlayer DefendingSide,
	const FName MarkerCardId,
	const FName HelperCardId)
{
	FMatchPlayCurrentAttackHelperParticipantAuthorityResult Result;
	auto Fail = [&Result](
		const EMatchPlayCurrentAttackHelperSelectionErrorCode Error,
		const FString& Message)
	{
		Result.ErrorCode = Error;
		Result.ErrorMessage = Message;
	};

	if (HelperCardId.IsNone())
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::InvalidHelperCardId,
			TEXT("HelperCardId must be non-empty."));
		return Result;
	}
	for (const FMatchPlayDeploymentPlacement& Placement :
		State.CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == DefendingSide
			&& Placement.CardId == HelperCardId)
		{
			++Result.MatchingPlacementCount;
			Result.Placement = Placement;
		}
	}
	if (Result.MatchingPlacementCount == 0)
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperNotDeployed,
			TEXT("Helper must be deployed by the current defender."));
		return Result;
	}
	if (Result.MatchingPlacementCount > 1)
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperDeploymentAmbiguous,
			TEXT("Helper has multiple current-defender placements."));
		return Result;
	}

	Result.SnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			State.CardSnapshotAuthority,
			DefendingSide,
			HelperCardId);
	if (!Result.SnapshotQueryResult.bSuccess)
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperSnapshotQueryFailed,
			Result.SnapshotQueryResult.ErrorMessage);
		return Result;
	}
	Result.Snapshot = Result.SnapshotQueryResult.Snapshot;
	if (Result.Snapshot.bIsGoalkeeper)
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperIsGoalkeeper,
			TEXT("A goalkeeper cannot be selected as Helper."));
		return Result;
	}
	if (HelperCardId == MarkerCardId)
	{
		Fail(
			EMatchPlayCurrentAttackHelperSelectionErrorCode
				::HelperMatchesMarker,
			TEXT("Helper and Marker must be different defender participants."));
		return Result;
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayCurrentAttackHelperSelectionErrorCode::None;
	return Result;
}
