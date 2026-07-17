#pragma once

#include "CoreMinimal.h"

#include "ThroughBallBehindDefenseP1FormulaResolutionExecutor.h"

enum class EThroughBallBehindDefenseP2OutcomeDecision : uint8
{
	None,
	Offside,
	OneOnOneRequired
};

enum class EThroughBallBehindDefenseP2OutcomeQueryErrorCode : uint8
{
	None,
	P1ExecutionFailed,
	InvalidP1ExecutionResult,
	MissingP2DefenseD6,
	InvalidP2DefenseD6
};

struct FMCODEX_API FThroughBallBehindDefenseP2OutcomeQueryInput
{
	FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
		P1ExecutionResult;

	bool bHasP2DefenseD6 = false;
	int32 P2DefenseD6 = 0;
};

struct FMCODEX_API FThroughBallBehindDefenseP2OutcomeQueryResult
{
	bool bSuccess = false;

	EThroughBallBehindDefenseP2OutcomeQueryErrorCode ErrorCode =
		EThroughBallBehindDefenseP2OutcomeQueryErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallBehindDefenseP2OutcomeQueryInput Input;

	EThroughBallBehindDefenseP2OutcomeDecision Decision =
		EThroughBallBehindDefenseP2OutcomeDecision::None;

	bool bAttackEnded = false;
	bool bContinueResolution = false;
	bool bRequiresOneOnOne = false;

	FName RunnerId = NAME_None;
};

class FMCODEX_API FThroughBallBehindDefenseP2OutcomeQuery final
{
public:
	static FThroughBallBehindDefenseP2OutcomeQueryResult Evaluate(
		const FThroughBallBehindDefenseP2OutcomeQueryInput& Input);
};
