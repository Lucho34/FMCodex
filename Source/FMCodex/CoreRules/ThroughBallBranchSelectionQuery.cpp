#include "ThroughBallBranchSelectionQuery.h"

namespace ThroughBallBranchSelectionQuery
{
	constexpr int32 MinD6 = 1;
	constexpr int32 MaxD6 = 6;

	void SetFailure(
		FThroughBallBranchSelectionQueryResult& Result,
		const EThroughBallBranchSelectionQueryErrorCode ErrorCode,
		const FString& ErrorMessage)
	{
		Result.ErrorCode = ErrorCode;
		Result.ErrorMessage = ErrorMessage;
		Result.InvalidField = TEXT("ExternalSelectionD6");
	}

	bool IsD6InRange(const int32 D6)
	{
		return D6 >= MinD6 && D6 <= MaxD6;
	}

	EThroughBallSelectedBranch MapSelectedBranch(const int32 D6)
	{
		switch (D6)
		{
		case 1:
		case 2:
			return EThroughBallSelectedBranch::Feet;
		case 3:
		case 4:
			return EThroughBallSelectedBranch::BehindDefense;
		case 5:
		case 6:
			return EThroughBallSelectedBranch::AntiOffside;
		default:
			return EThroughBallSelectedBranch::None;
		}
	}
}

FThroughBallBranchSelectionQueryResult
FThroughBallBranchSelectionQuery::Select(
	const FThroughBallBranchSelectionQueryInput& Input)
{
	FThroughBallBranchSelectionQueryResult Result;
	Result.Input = Input;

	if (!Input.bHasExternalSelectionD6)
	{
		ThroughBallBranchSelectionQuery::SetFailure(
			Result,
			EThroughBallBranchSelectionQueryErrorCode::MissingSelectionD6,
			TEXT("External selection D6 must be explicitly provided."));
		return Result;
	}

	if (!ThroughBallBranchSelectionQuery::IsD6InRange(
			Input.ExternalSelectionD6))
	{
		ThroughBallBranchSelectionQuery::SetFailure(
			Result,
			EThroughBallBranchSelectionQueryErrorCode::InvalidSelectionD6,
			TEXT("External selection D6 must be in the range [1, 6]."));
		return Result;
	}

	const EThroughBallSelectedBranch SelectedBranch =
		ThroughBallBranchSelectionQuery::MapSelectedBranch(
			Input.ExternalSelectionD6);
	if (SelectedBranch == EThroughBallSelectedBranch::None)
	{
		ThroughBallBranchSelectionQuery::SetFailure(
			Result,
			EThroughBallBranchSelectionQueryErrorCode::InvalidSelectionD6,
			TEXT("External selection D6 did not map to a Through Ball branch."));
		return Result;
	}

	Result.bSuccess = true;
	Result.bHasSelectedThroughBallBranch = true;
	Result.SelectedThroughBallBranch = SelectedBranch;
	return Result;
}
