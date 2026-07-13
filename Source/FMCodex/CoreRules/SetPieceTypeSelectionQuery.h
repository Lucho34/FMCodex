#pragma once

#include "CoreMinimal.h"

enum class ESetPieceSelectedType : uint8
{
	None,
	Corner,
	LongFreeKick,
	ShortFreeKick,
	Penalty
};

enum class ESetPieceTypeSelectionQueryErrorCode : uint8
{
	None,
	ActionPointNotEligibleForSetPiece,
	MissingSelectionD6,
	InvalidSelectionD6
};

struct FMCODEX_API FSetPieceTypeSelectionQueryInput
{
	int32 CurrentActionPoint = 0;
	bool bHasExternalSelectionD6 = false;
	int32 ExternalSelectionD6 = 0;
};

struct FMCODEX_API FSetPieceTypeSelectionQueryResult
{
	bool bSuccess = false;
	bool bHasSelectedSetPieceType = false;
	ESetPieceSelectedType SelectedSetPieceType =
		ESetPieceSelectedType::None;
	ESetPieceTypeSelectionQueryErrorCode ErrorCode =
		ESetPieceTypeSelectionQueryErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;
	FSetPieceTypeSelectionQueryInput Input;
};

class FMCODEX_API FSetPieceTypeSelectionQuery final
{
public:
	static FSetPieceTypeSelectionQueryResult Select(
		const FSetPieceTypeSelectionQueryInput& Input);
};
