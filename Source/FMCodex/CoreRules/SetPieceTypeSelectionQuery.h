#pragma once

#include "CoreMinimal.h"

#include "SetPieceTypeSelectionQuery.generated.h"

UENUM(BlueprintType)
enum class ESetPieceSelectedType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Corner = 1 UMETA(DisplayName = "Corner"),
	LongFreeKick = 2 UMETA(DisplayName = "Long Free Kick"),
	ShortFreeKick = 3 UMETA(DisplayName = "Short Free Kick"),
	Penalty = 4 UMETA(DisplayName = "Penalty")
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
