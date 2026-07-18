#pragma once

#include "CoreMinimal.h"

#include "ThroughBallOneOnOneHandoffCreator.h"

enum class EThroughBallOneOnOneChipShotOutcomeDecision : uint8
{
	None,
	Goal,
	Miss
};

enum class EThroughBallOneOnOneChipShotOutcomeQueryErrorCode : uint8
{
	None,
	HandoffCreationFailed,
	InvalidHandoffCreationResult,
	MissingChipShotAttackD6,
	InvalidChipShotAttackD6,
	InvalidLogContext
};

struct FMCODEX_API FThroughBallOneOnOneChipShotOutcomeQueryInput
{
	FThroughBallOneOnOneHandoffCreationResult HandoffCreationResult;

	bool bHasChipShotAttackD6 = false;
	int32 ChipShotAttackD6 = 0;

	FGuid LogId;
	int32 TurnIndex = 0;
};

struct FMCODEX_API FThroughBallOneOnOneChipShotOutcomeQueryResult
{
	bool bSuccess = false;

	EThroughBallOneOnOneChipShotOutcomeQueryErrorCode ErrorCode =
		EThroughBallOneOnOneChipShotOutcomeQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallOneOnOneChipShotOutcomeQueryInput Input;

	EThroughBallOneOnOneChipShotOutcomeDecision Decision =
		EThroughBallOneOnOneChipShotOutcomeDecision::None;

	bool bAttackEnded = false;
	bool bContinueResolution = false;
	bool bIsGoal = false;
};

class FMCODEX_API FThroughBallOneOnOneChipShotOutcomeQuery final
{
public:
	static FThroughBallOneOnOneChipShotOutcomeQueryResult Evaluate(
		const FThroughBallOneOnOneChipShotOutcomeQueryInput& Input);
};
