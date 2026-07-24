#include "MatchPlayGoalkeeperDeploymentLegality.h"

namespace MatchPlayGoalkeeperDeploymentLegalityImplementation
{
	void SetGoalkeeperDeploymentError(
		FMatchPlayGoalkeeperDeploymentLegalityResult& Result,
		const EMatchPlayGoalkeeperDeploymentErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsValidGoalkeeperDeploymentPlayerSide(
		const EInitialTurnOrderPlayer PlayerSide)
	{
		return PlayerSide == EInitialTurnOrderPlayer::PlayerA
			|| PlayerSide == EInitialTurnOrderPlayer::PlayerB;
	}

	EInitialTurnOrderPlayer ResolveDefendingPlayerSide(
		const EInitialTurnOrderPlayer CurrentAttackingPlayer)
	{
		return CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			? EInitialTurnOrderPlayer::PlayerB
			: EInitialTurnOrderPlayer::PlayerA;
	}

	EMatchPlayGoalkeeperDeploymentErrorCode MapGoalkeeperCardUsageError(
		const EPlayCardResolveErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EPlayCardResolveErrorCode::CardNotAvailable:
			return EMatchPlayGoalkeeperDeploymentErrorCode::CardNotAvailable;
		case EPlayCardResolveErrorCode::CardAlreadyUsed:
			return EMatchPlayGoalkeeperDeploymentErrorCode::CardAlreadyUsed;
		case EPlayCardResolveErrorCode::InvalidMatchCardUsageState:
		case EPlayCardResolveErrorCode::CardUsageResolveFailed:
		case EPlayCardResolveErrorCode::InvalidPlayerSide:
		case EPlayCardResolveErrorCode::InvalidCardId:
		case EPlayCardResolveErrorCode::None:
		default:
			return EMatchPlayGoalkeeperDeploymentErrorCode
				::CardUsageValidationFailed;
		}
	}
}

FMatchPlayGoalkeeperDeploymentLegalityResult
FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayGoalkeeperDeploymentRequest& Request)
{
	FMatchPlayGoalkeeperDeploymentLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::MatchPlayStateNotInitialized,
				TEXT("Match play state must be initialized before goalkeeper deployment."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode::NoCurrentAttack,
				TEXT("Goalkeeper deployment requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidCurrentAttackSequence,
				TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::AttackSequenceMismatch,
				TEXT("Goalkeeper deployment attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Deployment)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::CurrentAttackNotInDeployment,
				TEXT("Current attack must be in Deployment phase for goalkeeper deployment."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!MatchPlayGoalkeeperDeploymentLegalityImplementation
		::IsValidGoalkeeperDeploymentPlayerSide(CurrentAttackingPlayer))
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidCurrentAttackingPlayer,
				TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!MatchPlayGoalkeeperDeploymentLegalityImplementation
		::IsValidGoalkeeperDeploymentPlayerSide(Request.RequestingSide))
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidRequestingSide,
				TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (!MatchPlayGoalkeeperDeploymentLegalityImplementation
		::IsValidGoalkeeperDeploymentPlayerSide(
			CurrentAttack.CurrentLegalDeploymentSide))
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidCurrentLegalDeploymentSide,
				TEXT("CurrentLegalDeploymentSide must be PlayerA or PlayerB during Deployment."));
		return Result;
	}

	if (Request.RequestingSide
		!= CurrentAttack.CurrentLegalDeploymentSide)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::RequestingSideNotCurrentLegalDeploymentSide,
				TEXT("Only the current legal deployment side may deploy a goalkeeper."));
		return Result;
	}

	if (CurrentAttack.bAttackerDeploymentFinished
		&& CurrentAttack.bDefenderDeploymentFinished)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidDeploymentFinishedState,
				TEXT("Deployment phase cannot have both deployment-finished flags set."));
		return Result;
	}

	const bool bRequestingSideIsAttacker =
		Request.RequestingSide == CurrentAttackingPlayer;
	const bool bRequestingSideAlreadyFinished =
		bRequestingSideIsAttacker
			? CurrentAttack.bAttackerDeploymentFinished
			: CurrentAttack.bDefenderDeploymentFinished;
	if (bRequestingSideAlreadyFinished)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::RequestingSideAlreadyFinished,
				TEXT("Requesting side has already finished deployment for the current attack."));
		return Result;
	}

	const EInitialTurnOrderPlayer DefendingPlayerSide =
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::ResolveDefendingPlayerSide(CurrentAttackingPlayer);
	if (Request.RequestingSide != DefendingPlayerSide)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::RequestingSideIsNotDefender,
				TEXT("Only the current defending side may deploy a goalkeeper."));
		return Result;
	}

	if (Request.CardId.IsNone())
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode::InvalidCardId,
				TEXT("Goalkeeper deployment CardId must be non-empty."));
		return Result;
	}

	if (Request.SlotId.IsNone())
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode::InvalidSlotId,
				TEXT("Goalkeeper deployment SlotId must be non-empty."));
		return Result;
	}

	const FMatchPlayCardSnapshotAuthorityQueryResult SnapshotQueryResult =
		FMatchPlayCardSnapshotAuthorityQuery::FindByPlayerSideAndCardId(
			BeforeState.CardSnapshotAuthority,
			Request.RequestingSide,
			Request.CardId);
	Result.UnderlyingSnapshotAuthorityQueryErrorCode =
		SnapshotQueryResult.ErrorCode;
	if (!SnapshotQueryResult.bSuccess)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::CardSnapshotLookupFailed,
				SnapshotQueryResult.ErrorMessage);
		return Result;
	}

	if (!SnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::CardIsNotGoalkeeper,
				TEXT("Goalkeeper deployment requires a goalkeeper card snapshot."));
		return Result;
	}

	const FPlayCardValidationResult CardUsageValidationResult =
		FPlayCardResolver::ValidateCanPlayCard(
			BeforeState.CardUsageState,
			Request.RequestingSide,
			Request.CardId);
	Result.UnderlyingPlayCardErrorCode =
		CardUsageValidationResult.ErrorCode;
	Result.UnderlyingCardUsageErrorCode =
		CardUsageValidationResult.CardUsageErrorCode;
	if (!CardUsageValidationResult.bSuccess)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				MatchPlayGoalkeeperDeploymentLegalityImplementation
					::MapGoalkeeperCardUsageError(
						CardUsageValidationResult.ErrorCode),
				CardUsageValidationResult.ErrorMessage);
		return Result;
	}

	const FMatchPlayGoalkeeperUsageQueryResult UsageQueryResult =
		FMatchPlayGoalkeeperUsageStateResolver::Query(
			BeforeState.GoalkeeperUsageState,
			Request.RequestingSide);
	Result.UnderlyingGoalkeeperUsageErrorCode =
		UsageQueryResult.ErrorCode;
	if (!UsageQueryResult.bSucceeded)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::GoalkeeperUsageStateQueryFailed,
				UsageQueryResult.ErrorMessage);
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == Request.RequestingSide
			&& Placement.CardId == Request.CardId)
		{
			++Result.MatchingCurrentGoalkeeperPlacementCount;
		}
	}

	const bool bGoalkeeperActivated =
		CurrentAttack.bCurrentDefenseGoalkeeperActivated;
	const bool bGoalkeeperUsed =
		UsageQueryResult.bGoalkeeperCardUsed;
	const bool bUsageStateIsInvalid =
		(!bGoalkeeperUsed && bGoalkeeperActivated)
		|| (bGoalkeeperActivated
			&& Result.MatchingCurrentGoalkeeperPlacementCount != 1)
		|| (!bGoalkeeperActivated
			&& Result.MatchingCurrentGoalkeeperPlacementCount > 0);
	if (bUsageStateIsInvalid)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::InvalidGoalkeeperUsageState,
				TEXT("Persistent goalkeeper usage, current activation, and current placement are inconsistent."));
		return Result;
	}

	if (bGoalkeeperActivated)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::GoalkeeperAlreadyActivatedThisAttack,
				TEXT("The current defense has already activated its goalkeeper."));
		return Result;
	}

	if (bGoalkeeperUsed)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::GoalkeeperAlreadyUsedThisMatch,
				TEXT("The requesting side has already used its goalkeeper this match."));
		return Result;
	}

	const FMatchPlayDeploymentSlotCatalogQueryResult SlotQueryResult =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			BeforeState.DeploymentSlotCatalog,
			Request.SlotId);
	Result.UnderlyingSlotCatalogQueryErrorCode = SlotQueryResult.ErrorCode;
	if (!SlotQueryResult.bSuccess)
	{
		const EMatchPlayGoalkeeperDeploymentErrorCode ErrorCode =
			SlotQueryResult.ErrorCode
				== EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound
					? EMatchPlayGoalkeeperDeploymentErrorCode::SlotNotFound
					: EMatchPlayGoalkeeperDeploymentErrorCode
						::SlotCatalogLookupFailed;
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				ErrorCode,
				SlotQueryResult.ErrorMessage);
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.SlotId == Request.SlotId)
		{
			MatchPlayGoalkeeperDeploymentLegalityImplementation
				::SetGoalkeeperDeploymentError(
					Result,
					EMatchPlayGoalkeeperDeploymentErrorCode
						::SlotAlreadyOccupied,
					TEXT("Requested goalkeeper deployment SlotId is already occupied."));
			return Result;
		}
	}

	const FMatchPlayRelativeDeploymentZoneResolveResult ZoneResult =
		FMatchPlayRelativeDeploymentZoneResolver::Resolve(
			BeforeState.DeploymentSlotCatalog,
			Request.SlotId,
			CurrentAttackingPlayer,
			Request.RequestingSide);
	Result.UnderlyingRelativeZoneResolutionErrorCode =
		ZoneResult.ErrorCode;
	if (!ZoneResult.bSuccess)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::RelativeZoneResolutionFailed,
				ZoneResult.ErrorMessage);
		return Result;
	}

	Result.ResolvedRelativeZone = ZoneResult.RelativeZone;
	if (ZoneResult.RelativeZone
		!= EMatchPlayRelativeDeploymentZone::Backfield)
	{
		MatchPlayGoalkeeperDeploymentLegalityImplementation
			::SetGoalkeeperDeploymentError(
				Result,
				EMatchPlayGoalkeeperDeploymentErrorCode
					::GoalkeeperSlotNotInDefenderBackfield,
				TEXT("Goalkeeper deployment requires a defender Backfield slot."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode = EMatchPlayGoalkeeperDeploymentErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
