#include "CrossSelectionQuery.h"

namespace CrossSelectionQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;

	void SetFailure(
		FCrossSelectionQueryResult& Result,
		const ECrossSelectionQueryErrorCode ErrorCode,
		const FString& ErrorMessage,
		const FName InvalidField)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = InvalidField;
	}

	bool IsValidIntentType(const ECrossIntentType IntentType)
	{
		return IntentType == ECrossIntentType::High
			|| IntentType == ECrossIntentType::Low;
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}

	ECrossActualType MapToActualType(
		const ECrossIntentType IntentType,
		const int32 D6)
	{
		const bool bMatchesIntent = D6 <= 4;
		if (IntentType == ECrossIntentType::High)
		{
			return bMatchesIntent
				? ECrossActualType::High
				: ECrossActualType::Low;
		}

		return bMatchesIntent
			? ECrossActualType::Low
			: ECrossActualType::High;
	}
}

FCrossSelectionQueryResult FCrossSelectionQuery::Select(
	const FCrossSelectionQueryInput& Input)
{
	FCrossSelectionQueryResult Result;
	Result.Input = Input;

	if (!CrossSelectionQuery::IsValidIntentType(Input.IntendedCrossType))
	{
		CrossSelectionQuery::SetFailure(
			Result,
			ECrossSelectionQueryErrorCode::InvalidIntendedCrossType,
			TEXT("IntendedCrossType must be High or Low."),
			TEXT("IntendedCrossType"));
		return Result;
	}

	if (!Input.bHasExternalSelectionD6)
	{
		CrossSelectionQuery::SetFailure(
			Result,
			ECrossSelectionQueryErrorCode::MissingSelectionD6,
			TEXT("An externally supplied SelectionD6 is required."),
			TEXT("ExternalSelectionD6"));
		return Result;
	}

	if (!CrossSelectionQuery::IsD6InRange(
			Input.ExternalSelectionD6))
	{
		CrossSelectionQuery::SetFailure(
			Result,
			ECrossSelectionQueryErrorCode::InvalidSelectionD6,
			TEXT("ExternalSelectionD6 must be in range [1, 6]."),
			TEXT("ExternalSelectionD6"));
		return Result;
	}

	Result.bSuccess = true;
	Result.bHasActualCrossType = true;
	Result.ActualCrossType = CrossSelectionQuery::MapToActualType(
		Input.IntendedCrossType,
		Input.ExternalSelectionD6);
	return Result;
}
