#include "MatchPlayGoalkeeperDeploymentAvailability.h"

namespace MatchPlayGoalkeeperDeploymentAvailabilityImplementation
{
	bool IsGoalkeeperDeploymentGlobalBlocker(
		const EMatchPlayGoalkeeperDeploymentErrorCode ErrorCode)
	{
		switch (ErrorCode)
		{
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::MatchPlayStateNotInitialized:
		case EMatchPlayGoalkeeperDeploymentErrorCode::NoCurrentAttack:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidCurrentAttackSequence:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::AttackSequenceMismatch:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::CurrentAttackNotInDeployment:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidCurrentAttackingPlayer:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidRequestingSide:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidCurrentLegalDeploymentSide:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideNotCurrentLegalDeploymentSide:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidDeploymentFinishedState:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideAlreadyFinished:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::RequestingSideIsNotDefender:
		case EMatchPlayGoalkeeperDeploymentErrorCode::InvalidCardId:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::CardSnapshotLookupFailed:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::CardIsNotGoalkeeper:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::CardUsageValidationFailed:
		case EMatchPlayGoalkeeperDeploymentErrorCode::CardNotAvailable:
		case EMatchPlayGoalkeeperDeploymentErrorCode::CardAlreadyUsed:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperUsageStateQueryFailed:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::InvalidGoalkeeperUsageState:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyActivatedThisAttack:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperAlreadyUsedThisMatch:
			return true;
		case EMatchPlayGoalkeeperDeploymentErrorCode::None:
		case EMatchPlayGoalkeeperDeploymentErrorCode::InvalidSlotId:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::SlotCatalogLookupFailed:
		case EMatchPlayGoalkeeperDeploymentErrorCode::SlotNotFound:
		case EMatchPlayGoalkeeperDeploymentErrorCode::SlotAlreadyOccupied:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::RelativeZoneResolutionFailed:
		case EMatchPlayGoalkeeperDeploymentErrorCode
			::GoalkeeperSlotNotInDefenderBackfield:
		default:
			return false;
		}
	}

	FMatchPlayGoalkeeperDeploymentRequest MakeGoalkeeperDeploymentRequest(
		const int64 AttackSequence,
		const EInitialTurnOrderPlayer RequestingSide,
		const FName CardId,
		const FName SlotId)
	{
		FMatchPlayGoalkeeperDeploymentRequest Request;
		Request.AttackSequence = AttackSequence;
		Request.RequestingSide = RequestingSide;
		Request.CardId = CardId;
		Request.SlotId = SlotId;
		return Request;
	}
}

FMatchPlayGoalkeeperDeploymentAvailabilityResult
FMatchPlayGoalkeeperDeploymentAvailability::Query(
	const FMatchPlayState& BeforeState,
	const int64 AttackSequence,
	const EInitialTurnOrderPlayer RequestingSide,
	const FName CardId)
{
	FMatchPlayGoalkeeperDeploymentAvailabilityResult Result;
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
			EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode
				::CatalogEnumerationFailed;
		Result.ErrorMessage = CatalogResult.ErrorMessage;
		return Result;
	}

	const FName FirstSlotId =
		BeforeState.DeploymentSlotCatalog.Slots[0].SlotId;
	const FMatchPlayGoalkeeperDeploymentLegalityResult FirstResult =
		FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
			BeforeState,
			MatchPlayGoalkeeperDeploymentAvailabilityImplementation
				::MakeGoalkeeperDeploymentRequest(
					AttackSequence,
					RequestingSide,
					CardId,
					FirstSlotId));
	if (!FirstResult.bIsLegal
		&& MatchPlayGoalkeeperDeploymentAvailabilityImplementation
			::IsGoalkeeperDeploymentGlobalBlocker(FirstResult.ErrorCode))
	{
		Result.bQuerySucceeded = true;
		Result.bHasFirstBlockingLegalityResult = true;
		Result.FirstBlockingLegalityResult = FirstResult;
		return Result;
	}

	for (const FMatchPlayDeploymentSlotDefinition& Slot :
		BeforeState.DeploymentSlotCatalog.Slots)
	{
		FMatchPlayGoalkeeperDeploymentSlotAvailability SlotResult;
		SlotResult.SlotId = Slot.SlotId;
		SlotResult.LegalityResult =
			FMatchPlayGoalkeeperDeploymentLegalityEvaluator::Evaluate(
				BeforeState,
				MatchPlayGoalkeeperDeploymentAvailabilityImplementation
					::MakeGoalkeeperDeploymentRequest(
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
		EMatchPlayGoalkeeperDeploymentAvailabilityErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
