#pragma once

#include "CoreMinimal.h"

#include "ThroughBallBehindDefenseP1FormulaResolverInputAssembler.h"

enum class
EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision : uint8
{
	None,
	DefenderStoppedAttack,
	P2Required
};

enum class
EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode : uint8
{
	None,
	ResolverInputAssemblyFailed,
	InvalidResolverInputAssemblyResult,
	InvalidFormulaResolutionResult
};

struct FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolutionExecutionInput
{
	FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult
		ResolverInputAssemblyResult;
};

struct FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolutionExecutionResult
{
	bool bSuccess = false;

	EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode ErrorCode =
		EThroughBallBehindDefenseP1FormulaResolutionExecutionErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallBehindDefenseP1FormulaResolutionExecutionInput Input;

	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;

	EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision Decision =
		EThroughBallBehindDefenseP1FormulaResolutionExecutionDecision::None;

	bool bAttackEnded = false;
	bool bContinueResolution = false;
	bool bRequiresP2 = false;

	FName RunnerId = NAME_None;
};

class FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolutionExecutor final
{
public:
	static FThroughBallBehindDefenseP1FormulaResolutionExecutionResult Execute(
		const FThroughBallBehindDefenseP1FormulaResolutionExecutionInput& Input);
};
