#include "MatchPlayOrdinaryDeploymentLegality.h"

namespace MatchPlayOrdinaryDeploymentLegalityImplementation
{
	void SetError(
		FMatchPlayOrdinaryDeploymentLegalityResult& Result,
		const EMatchPlayOrdinaryDeploymentErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}

	bool IsPositionEligibleForZone(
		const EPlayerPositionType PositionType,
		const EMatchPlayRelativeDeploymentZone RelativeZone)
	{
		switch (RelativeZone)
		{
		case EMatchPlayRelativeDeploymentZone::Midfield:
			return PositionType == EPlayerPositionType::Attack
				|| PositionType == EPlayerPositionType::Midfield
				|| PositionType == EPlayerPositionType::Defense;
		case EMatchPlayRelativeDeploymentZone::Forward:
			return PositionType == EPlayerPositionType::Attack
				|| PositionType == EPlayerPositionType::Midfield;
		case EMatchPlayRelativeDeploymentZone::Backfield:
			return PositionType == EPlayerPositionType::Midfield
				|| PositionType == EPlayerPositionType::Defense;
		case EMatchPlayRelativeDeploymentZone::None:
		default:
			return false;
		}
	}

	bool HasEligiblePosition(
		const TArray<EPlayerPositionType>& PositionTypes,
		const EMatchPlayRelativeDeploymentZone RelativeZone)
	{
		for (const EPlayerPositionType PositionType : PositionTypes)
		{
			if (IsPositionEligibleForZone(PositionType, RelativeZone))
			{
				return true;
			}
		}

		return false;
	}

	EMatchPlayOrdinaryDeploymentErrorCode MapCardUsageError(
		const EPlayCardResolveErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EPlayCardResolveErrorCode::CardNotAvailable:
			return EMatchPlayOrdinaryDeploymentErrorCode::CardNotAvailable;
		case EPlayCardResolveErrorCode::CardAlreadyUsed:
			return EMatchPlayOrdinaryDeploymentErrorCode::CardAlreadyUsed;
		case EPlayCardResolveErrorCode::InvalidMatchCardUsageState:
		case EPlayCardResolveErrorCode::CardUsageResolveFailed:
		case EPlayCardResolveErrorCode::InvalidPlayerSide:
		case EPlayCardResolveErrorCode::InvalidCardId:
		case EPlayCardResolveErrorCode::None:
		default:
			return EMatchPlayOrdinaryDeploymentErrorCode
				::CardUsageValidationFailed;
		}
	}
}

FMatchPlayOrdinaryDeploymentLegalityResult
FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
	const FMatchPlayState& BeforeState,
	const FMatchPlayOrdinaryDeploymentRequest& Request)
{
	using namespace MatchPlayOrdinaryDeploymentLegalityImplementation;

	FMatchPlayOrdinaryDeploymentLegalityResult Result;
	Result.Request = Request;

	if (!BeforeState.RuntimeState.bIsInitialized)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::MatchPlayStateNotInitialized,
			TEXT("Match play state must be initialized before ordinary deployment."));
		return Result;
	}

	if (!BeforeState.bHasCurrentAttack)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::NoCurrentAttack,
			TEXT("Ordinary deployment requires an active current attack."));
		return Result;
	}

	const FMatchPlayCurrentAttackState& CurrentAttack =
		BeforeState.CurrentAttack;
	if (CurrentAttack.AttackSequence <= 0)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::InvalidCurrentAttackSequence,
			TEXT("Current attack sequence must be greater than zero."));
		return Result;
	}

	if (Request.AttackSequence != CurrentAttack.AttackSequence)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::AttackSequenceMismatch,
			TEXT("Ordinary deployment attack sequence does not match the current attack."));
		return Result;
	}

	if (CurrentAttack.Phase != EMatchPlayCurrentAttackPhase::Deployment)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::CurrentAttackNotInDeployment,
			TEXT("Current attack must be in Deployment phase for ordinary deployment."));
		return Result;
	}

	const EInitialTurnOrderPlayer CurrentAttackingPlayer =
		BeforeState.RuntimeState.CurrentAttackingPlayer;
	if (!IsPlayer(CurrentAttackingPlayer))
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!IsPlayer(Request.RequestingSide))
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::InvalidRequestingSide,
			TEXT("RequestingSide must be PlayerA or PlayerB."));
		return Result;
	}

	if (!IsPlayer(CurrentAttack.CurrentLegalDeploymentSide))
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::InvalidCurrentLegalDeploymentSide,
			TEXT("CurrentLegalDeploymentSide must be PlayerA or PlayerB during Deployment."));
		return Result;
	}

	if (Request.RequestingSide
		!= CurrentAttack.CurrentLegalDeploymentSide)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::RequestingSideNotCurrentLegalDeploymentSide,
			TEXT("Only the current legal deployment side may deploy an ordinary player."));
		return Result;
	}

	if (CurrentAttack.bAttackerDeploymentFinished
		&& CurrentAttack.bDefenderDeploymentFinished)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
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
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::RequestingSideAlreadyFinished,
			TEXT("Requesting side has already finished deployment for the current attack."));
		return Result;
	}

	if (Request.CardId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::InvalidCardId,
			TEXT("Ordinary deployment CardId must be non-empty."));
		return Result;
	}

	if (Request.SlotId.IsNone())
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::InvalidSlotId,
			TEXT("Ordinary deployment SlotId must be non-empty."));
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
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::CardSnapshotLookupFailed,
			SnapshotQueryResult.ErrorMessage);
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
		SetError(
			Result,
			MapCardUsageError(CardUsageValidationResult.ErrorCode),
			CardUsageValidationResult.ErrorMessage);
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.PlayerSide == Request.RequestingSide
			&& Placement.CardId == Request.CardId)
		{
			SetError(
				Result,
				EMatchPlayOrdinaryDeploymentErrorCode
					::CardAlreadyPlacedBySide,
				TEXT("Requesting side has already placed this CardId in the current attack."));
			return Result;
		}
	}

	if (SnapshotQueryResult.Snapshot.bIsGoalkeeper)
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed,
			TEXT("Goalkeeper cards cannot use ordinary player deployment."));
		return Result;
	}

	const FMatchPlayDeploymentSlotCatalogQueryResult SlotQueryResult =
		FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
			BeforeState.DeploymentSlotCatalog,
			Request.SlotId);
	Result.UnderlyingSlotCatalogQueryErrorCode = SlotQueryResult.ErrorCode;
	if (!SlotQueryResult.bSuccess)
	{
		const EMatchPlayOrdinaryDeploymentErrorCode ErrorCode =
			SlotQueryResult.ErrorCode
				== EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound
					? EMatchPlayOrdinaryDeploymentErrorCode::SlotNotFound
					: EMatchPlayOrdinaryDeploymentErrorCode
						::SlotCatalogLookupFailed;
		SetError(Result, ErrorCode, SlotQueryResult.ErrorMessage);
		return Result;
	}

	for (const FMatchPlayDeploymentPlacement& Placement :
		CurrentAttack.DeploymentPlacements)
	{
		if (Placement.SlotId == Request.SlotId)
		{
			SetError(
				Result,
				EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied,
				TEXT("Requested deployment SlotId is already occupied in the current attack."));
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
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::RelativeZoneResolutionFailed,
			ZoneResult.ErrorMessage);
		return Result;
	}

	Result.ResolvedRelativeZone = ZoneResult.RelativeZone;
	if (!HasEligiblePosition(
		SnapshotQueryResult.Snapshot.PositionTypes,
		ZoneResult.RelativeZone))
	{
		SetError(
			Result,
			EMatchPlayOrdinaryDeploymentErrorCode
				::PositionNotEligibleForZone,
			TEXT("Card position types are not eligible for the resolved relative deployment zone."));
		return Result;
	}

	Result.bIsLegal = true;
	Result.ErrorCode = EMatchPlayOrdinaryDeploymentErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
