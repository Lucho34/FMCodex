#pragma once

#include "CoreMinimal.h"

enum class ECrossIntentType : uint8
{
	None,
	High,
	Low
};

enum class ECrossActualType : uint8
{
	None,
	High,
	Low
};

enum class ECrossSelectionQueryErrorCode : uint8
{
	None,
	InvalidIntendedCrossType,
	MissingSelectionD6,
	InvalidSelectionD6
};

struct FMCODEX_API FCrossSelectionQueryInput
{
	ECrossIntentType IntendedCrossType = ECrossIntentType::None;
	bool bHasExternalSelectionD6 = false;
	int32 ExternalSelectionD6 = 0;
};

struct FMCODEX_API FCrossSelectionQueryResult
{
	bool bSuccess = false;
	bool bHasActualCrossType = false;
	ECrossActualType ActualCrossType = ECrossActualType::None;
	ECrossSelectionQueryErrorCode ErrorCode =
		ECrossSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FCrossSelectionQueryInput Input;
};

class FMCODEX_API FCrossSelectionQuery final
{
public:
	static FCrossSelectionQueryResult Select(
		const FCrossSelectionQueryInput& Input);
};
