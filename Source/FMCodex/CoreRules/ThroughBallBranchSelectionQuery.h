#pragma once

#include "CoreMinimal.h"

enum class EThroughBallSelectedBranch : uint8
{
	None,
	Feet,
	BehindDefense,
	AntiOffside
};

enum class EThroughBallBranchSelectionQueryErrorCode : uint8
{
	None,
	MissingSelectionD6,
	InvalidSelectionD6
};

struct FMCODEX_API FThroughBallBranchSelectionQueryInput
{
	bool bHasExternalSelectionD6 = false;
	int32 ExternalSelectionD6 = 0;
};

struct FMCODEX_API FThroughBallBranchSelectionQueryResult
{
	bool bSuccess = false;
	bool bHasSelectedThroughBallBranch = false;
	EThroughBallSelectedBranch SelectedThroughBallBranch =
		EThroughBallSelectedBranch::None;
	EThroughBallBranchSelectionQueryErrorCode ErrorCode =
		EThroughBallBranchSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FThroughBallBranchSelectionQueryInput Input;
};

class FMCODEX_API FThroughBallBranchSelectionQuery final
{
public:
	static FThroughBallBranchSelectionQueryResult Select(
		const FThroughBallBranchSelectionQueryInput& Input);
};
