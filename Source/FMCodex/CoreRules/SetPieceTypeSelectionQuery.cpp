#include "SetPieceTypeSelectionQuery.h"

namespace SetPieceTypeSelectionQuery
{
	constexpr int32 MinSetPieceActionPoint = 9;
	constexpr int32 MaxSetPieceActionPoint = 12;
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;

	void SetFailure(
		FSetPieceTypeSelectionQueryResult& Result,
		const ESetPieceTypeSelectionQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsEligibleActionPoint(const int32 CurrentActionPoint)
	{
		return CurrentActionPoint >= MinSetPieceActionPoint
			&& CurrentActionPoint <= MaxSetPieceActionPoint;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}

	ESetPieceSelectedType MapSelectedType(const int32 D6)
	{
		switch (D6)
		{
		case 1:
		case 2:
			return ESetPieceSelectedType::Corner;
		case 3:
		case 4:
			return ESetPieceSelectedType::LongFreeKick;
		case 5:
			return ESetPieceSelectedType::ShortFreeKick;
		case 6:
			return ESetPieceSelectedType::Penalty;
		default:
			return ESetPieceSelectedType::None;
		}
	}
}

FSetPieceTypeSelectionQueryResult FSetPieceTypeSelectionQuery::Select(
	const FSetPieceTypeSelectionQueryInput& Input)
{
	FSetPieceTypeSelectionQueryResult Result;
	Result.Input = Input;

	if (!SetPieceTypeSelectionQuery::IsEligibleActionPoint(
			Input.CurrentActionPoint))
	{
		SetPieceTypeSelectionQuery::SetFailure(
			Result,
			ESetPieceTypeSelectionQueryErrorCode::
				ActionPointNotEligibleForSetPiece,
			TEXT("CurrentActionPoint must be in range [9, 12] for Set Piece selection."),
			TEXT("CurrentActionPoint"));
		return Result;
	}

	if (!Input.bHasExternalSelectionD6)
	{
		SetPieceTypeSelectionQuery::SetFailure(
			Result,
			ESetPieceTypeSelectionQueryErrorCode::MissingSelectionD6,
			TEXT("An externally supplied SelectionD6 is required."),
			TEXT("ExternalSelectionD6"));
		return Result;
	}

	if (!SetPieceTypeSelectionQuery::IsD6InRange(
			Input.ExternalSelectionD6))
	{
		SetPieceTypeSelectionQuery::SetFailure(
			Result,
			ESetPieceTypeSelectionQueryErrorCode::InvalidSelectionD6,
			TEXT("ExternalSelectionD6 must be in range [1, 6]."),
			TEXT("ExternalSelectionD6"));
		return Result;
	}

	const ESetPieceSelectedType SelectedType =
		SetPieceTypeSelectionQuery::MapSelectedType(
			Input.ExternalSelectionD6);
	if (SelectedType == ESetPieceSelectedType::None)
	{
		SetPieceTypeSelectionQuery::SetFailure(
			Result,
			ESetPieceTypeSelectionQueryErrorCode::InvalidSelectionD6,
			TEXT("ExternalSelectionD6 did not map to a Set Piece type."),
			TEXT("ExternalSelectionD6"));
		return Result;
	}

	Result.bSuccess = true;
	Result.bHasSelectedSetPieceType = true;
	Result.SelectedSetPieceType = SelectedType;
	return Result;
}
