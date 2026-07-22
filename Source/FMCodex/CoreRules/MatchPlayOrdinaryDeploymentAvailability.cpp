#include "MatchPlayOrdinaryDeploymentAvailability.h"

namespace MatchPlayOrdinaryDeploymentAvailabilityImplementation
{
	bool IsGlobalBlocker(
		const EMatchPlayOrdinaryDeploymentErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayOrdinaryDeploymentErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayOrdinaryDeploymentErrorCode::NoCurrentAttack:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayOrdinaryDeploymentErrorCode::AttackSequenceMismatch:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::CurrentAttackNotInDeployment:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayOrdinaryDeploymentErrorCode::InvalidRequestingSide:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::InvalidCurrentLegalDeploymentSide:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::InvalidDeploymentFinishedState:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::RequestingSideAlreadyFinished:
		case EMatchPlayOrdinaryDeploymentErrorCode::InvalidCardId:
		case EMatchPlayOrdinaryDeploymentErrorCode::CardSnapshotLookupFailed:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::CardUsageValidationFailed:
		case EMatchPlayOrdinaryDeploymentErrorCode::CardNotAvailable:
		case EMatchPlayOrdinaryDeploymentErrorCode::CardAlreadyUsed:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::CardAlreadyPlacedBySide:
		case EMatchPlayOrdinaryDeploymentErrorCode::GoalkeeperNotAllowed:
			return true;
		case EMatchPlayOrdinaryDeploymentErrorCode::None:
		case EMatchPlayOrdinaryDeploymentErrorCode::InvalidSlotId:
		case EMatchPlayOrdinaryDeploymentErrorCode::SlotCatalogLookupFailed:
		case EMatchPlayOrdinaryDeploymentErrorCode::SlotNotFound:
		case EMatchPlayOrdinaryDeploymentErrorCode::SlotAlreadyOccupied:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::RelativeZoneResolutionFailed:
		case EMatchPlayOrdinaryDeploymentErrorCode
			::PositionNotEligibleForZone:
		default:
			return false;
		}
	}

	FMatchPlayOrdinaryDeploymentRequest MakeRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName CardId,
		const FName SlotId)
	{
		FMatchPlayOrdinaryDeploymentRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CardId = CardId;
		Request.SlotId = SlotId;
		return Request;
	}
}

FMatchPlayOrdinaryDeploymentAvailabilityResult
FMatchPlayOrdinaryDeploymentAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide,
	const FName CardId)
{
	using namespace MatchPlayOrdinaryDeploymentAvailabilityImplementation;

	FMatchPlayOrdinaryDeploymentAvailabilityResult Result;
	Result.AttackSequence = AttackSequence;
	Result.RequestingSide = RequestingSide;
	Result.CardId = CardId;

	const FMatchPlayDeploymentSlotCatalogValidationResult CatalogResult =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(
			BeforeState.DeploymentSlotCatalog);
	Result.UnderlyingCatalogValidationErrorCode = CatalogResult.ErrorCode;
	if (!CatalogResult.bSuccess)
	{
		Result.ErrorCode =
			EMatchPlayOrdinaryDeploymentAvailabilityErrorCode
				::CatalogEnumerationFailed;
		Result.ErrorMessage = CatalogResult.ErrorMessage;
		return Result;
	}

	const FName FirstSlotId =
		BeforeState.DeploymentSlotCatalog.Slots[0].SlotId;
	const FMatchPlayOrdinaryDeploymentLegalityResult FirstResult =
		FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
			BeforeState,
			MakeRequest(
				AttackSequence,
				RequestingSide,
				CardId,
				FirstSlotId));
	if (!FirstResult.bIsLegal && IsGlobalBlocker(FirstResult.ErrorCode))
	{
		Result.bQuerySucceeded = true;
		Result.bHasFirstBlockingLegalityResult = true;
		Result.FirstBlockingLegalityResult = FirstResult;
		return Result;
	}

	for (const FMatchPlayDeploymentSlotDefinition& Slot :
		BeforeState.DeploymentSlotCatalog.Slots)
	{
		FMatchPlayOrdinaryDeploymentSlotAvailability SlotResult;
		SlotResult.SlotId = Slot.SlotId;
		SlotResult.LegalityResult =
			FMatchPlayOrdinaryDeploymentLegalityEvaluator::Evaluate(
				BeforeState,
				MakeRequest(
					AttackSequence,
					RequestingSide,
					CardId,
					Slot.SlotId));

		if (SlotResult.LegalityResult.bIsLegal)
		{
			Result.LegalSlotIds.Add(Slot.SlotId);
		}
		else if (!Result.bHasFirstBlockingLegalityResult)
		{
			Result.bHasFirstBlockingLegalityResult = true;
			Result.FirstBlockingLegalityResult =
				SlotResult.LegalityResult;
		}

		Result.SlotResults.Add(MoveTemp(SlotResult));
	}

	Result.bQuerySucceeded = true;
	Result.bCanDeployToAnySlot = !Result.LegalSlotIds.IsEmpty();
	Result.ErrorCode =
		EMatchPlayOrdinaryDeploymentAvailabilityErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
