#include "MatchPlayDeploymentSlotCatalog.h"

namespace MatchPlayDeploymentSlotCatalog
{
	void SetValidationError(
		FMatchPlayDeploymentSlotCatalogValidationResult& Result,
		const EMatchPlayDeploymentSlotCatalogValidationErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	void SetQueryError(
		FMatchPlayDeploymentSlotCatalogQueryResult& Result,
		const EMatchPlayDeploymentSlotCatalogQueryErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	void SetResolveError(
		FMatchPlayRelativeDeploymentZoneResolveResult& Result,
		const EMatchPlayRelativeDeploymentZoneResolveErrorCode ErrorCode,
		const TCHAR* ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
	}

	bool IsValidNeutralSide(const EMatchPlayNeutralSlotSide NeutralSide)
	{
		return NeutralSide == EMatchPlayNeutralSlotSide::NearPlayerA
			|| NeutralSide == EMatchPlayNeutralSlotSide::NearPlayerB;
	}

	bool IsPlayer(const EInitialTurnOrderPlayer Player)
	{
		return Player == EInitialTurnOrderPlayer::PlayerA
			|| Player == EInitialTurnOrderPlayer::PlayerB;
	}
}

FMatchPlayDeploymentSlotCatalogValidationResult
FMatchPlayDeploymentSlotCatalogValidator::Validate(
	const FMatchPlayDeploymentSlotCatalog& SlotCatalog)
{
	FMatchPlayDeploymentSlotCatalogValidationResult Result;

	if (SlotCatalog.Slots.IsEmpty())
	{
		MatchPlayDeploymentSlotCatalog::SetValidationError(
			Result,
			EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptyCatalog,
			TEXT("Deployment slot catalog must contain at least one slot."));
		return Result;
	}

	for (const FMatchPlayDeploymentSlotDefinition& Slot : SlotCatalog.Slots)
	{
		if (Slot.SlotId.IsNone())
		{
			MatchPlayDeploymentSlotCatalog::SetValidationError(
				Result,
				EMatchPlayDeploymentSlotCatalogValidationErrorCode::EmptySlotId,
				TEXT("Every deployment slot must have a non-empty SlotId."));
			return Result;
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < SlotCatalog.Slots.Num(); ++SlotIndex)
	{
		for (int32 OtherSlotIndex = SlotIndex + 1;
			OtherSlotIndex < SlotCatalog.Slots.Num();
			++OtherSlotIndex)
		{
			if (SlotCatalog.Slots[SlotIndex].SlotId
				== SlotCatalog.Slots[OtherSlotIndex].SlotId)
			{
				MatchPlayDeploymentSlotCatalog::SetValidationError(
					Result,
					EMatchPlayDeploymentSlotCatalogValidationErrorCode
						::DuplicateSlotId,
					TEXT("Deployment slot catalog SlotIds must be globally unique."));
				return Result;
			}
		}
	}

	for (const FMatchPlayDeploymentSlotDefinition& Slot : SlotCatalog.Slots)
	{
		if (!MatchPlayDeploymentSlotCatalog::IsValidNeutralSide(
			Slot.NeutralSide))
		{
			MatchPlayDeploymentSlotCatalog::SetValidationError(
				Result,
				EMatchPlayDeploymentSlotCatalogValidationErrorCode
					::InvalidNeutralSide,
				TEXT("Every deployment slot NeutralSide must be NearPlayerA or NearPlayerB."));
			return Result;
		}
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayDeploymentSlotCatalogValidationErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}

FMatchPlayDeploymentSlotCatalogQueryResult
FMatchPlayDeploymentSlotCatalogQuery::FindSlot(
	const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
	const FName SlotId)
{
	FMatchPlayDeploymentSlotCatalogQueryResult Result;
	Result.SlotId = SlotId;

	if (SlotId.IsNone())
	{
		MatchPlayDeploymentSlotCatalog::SetQueryError(
			Result,
			EMatchPlayDeploymentSlotCatalogQueryErrorCode::InvalidSlotId,
			TEXT("Requested deployment SlotId must be non-empty."));
		return Result;
	}

	const FMatchPlayDeploymentSlotCatalogValidationResult ValidationResult =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(SlotCatalog);
	if (!ValidationResult.bSuccess)
	{
		MatchPlayDeploymentSlotCatalog::SetQueryError(
			Result,
			EMatchPlayDeploymentSlotCatalogQueryErrorCode::InvalidCatalog,
			TEXT("Deployment slot catalog must be valid before querying a slot."));
		return Result;
	}

	for (const FMatchPlayDeploymentSlotDefinition& Slot : SlotCatalog.Slots)
	{
		if (Slot.SlotId == SlotId)
		{
			Result.bSuccess = true;
			Result.SlotDefinition = Slot;
			Result.ErrorCode =
				EMatchPlayDeploymentSlotCatalogQueryErrorCode::None;
			Result.ErrorMessage.Empty();
			return Result;
		}
	}

	MatchPlayDeploymentSlotCatalog::SetQueryError(
		Result,
		EMatchPlayDeploymentSlotCatalogQueryErrorCode::SlotNotFound,
		TEXT("Requested deployment SlotId was not found in the catalog."));
	return Result;
}

FMatchPlayRelativeDeploymentZoneResolveResult
FMatchPlayRelativeDeploymentZoneResolver::Resolve(
	const FMatchPlayDeploymentSlotCatalog& SlotCatalog,
	const FName SlotId,
	const EInitialTurnOrderPlayer CurrentAttackingPlayer,
	const EInitialTurnOrderPlayer EvaluatedPlayerSide)
{
	FMatchPlayRelativeDeploymentZoneResolveResult Result;
	Result.SlotId = SlotId;
	Result.CurrentAttackingPlayer = CurrentAttackingPlayer;
	Result.EvaluatedPlayerSide = EvaluatedPlayerSide;

	if (SlotId.IsNone())
	{
		MatchPlayDeploymentSlotCatalog::SetResolveError(
			Result,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::InvalidSlotId,
			TEXT("Requested deployment SlotId must be non-empty."));
		return Result;
	}

	if (!MatchPlayDeploymentSlotCatalog::IsPlayer(CurrentAttackingPlayer))
	{
		MatchPlayDeploymentSlotCatalog::SetResolveError(
			Result,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode
				::InvalidCurrentAttackingPlayer,
			TEXT("CurrentAttackingPlayer must be PlayerA or PlayerB."));
		return Result;
	}

	if (!MatchPlayDeploymentSlotCatalog::IsPlayer(EvaluatedPlayerSide))
	{
		MatchPlayDeploymentSlotCatalog::SetResolveError(
			Result,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode
				::InvalidEvaluatedPlayerSide,
			TEXT("EvaluatedPlayerSide must be PlayerA or PlayerB."));
		return Result;
	}

	const FMatchPlayDeploymentSlotCatalogValidationResult ValidationResult =
		FMatchPlayDeploymentSlotCatalogValidator::Validate(SlotCatalog);
	if (!ValidationResult.bSuccess)
	{
		MatchPlayDeploymentSlotCatalog::SetResolveError(
			Result,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::InvalidCatalog,
			TEXT("Deployment slot catalog must be valid before resolving a relative zone."));
		return Result;
	}

	const FMatchPlayDeploymentSlotDefinition* FoundSlot = nullptr;
	for (const FMatchPlayDeploymentSlotDefinition& Slot : SlotCatalog.Slots)
	{
		if (Slot.SlotId == SlotId)
		{
			FoundSlot = &Slot;
			break;
		}
	}

	if (FoundSlot == nullptr)
	{
		MatchPlayDeploymentSlotCatalog::SetResolveError(
			Result,
			EMatchPlayRelativeDeploymentZoneResolveErrorCode::SlotNotFound,
			TEXT("Requested deployment SlotId was not found in the catalog."));
		return Result;
	}

	Result.NeutralSide = FoundSlot->NeutralSide;
	const bool bSlotIsNearCurrentAttacker =
		(CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerA
			&& FoundSlot->NeutralSide
				== EMatchPlayNeutralSlotSide::NearPlayerA)
		|| (CurrentAttackingPlayer == EInitialTurnOrderPlayer::PlayerB
			&& FoundSlot->NeutralSide
				== EMatchPlayNeutralSlotSide::NearPlayerB);
	if (bSlotIsNearCurrentAttacker)
	{
		Result.RelativeZone =
			EMatchPlayRelativeDeploymentZone::Midfield;
	}
	else if (EvaluatedPlayerSide == CurrentAttackingPlayer)
	{
		Result.RelativeZone =
			EMatchPlayRelativeDeploymentZone::Forward;
	}
	else
	{
		Result.RelativeZone =
			EMatchPlayRelativeDeploymentZone::Backfield;
	}

	Result.bSuccess = true;
	Result.ErrorCode =
		EMatchPlayRelativeDeploymentZoneResolveErrorCode::None;
	Result.ErrorMessage.Empty();
	return Result;
}
