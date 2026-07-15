#pragma once

#include "CoreMinimal.h"

#include "ThroughBallFeetFormulaResolverInputAssembler.h"

struct FMCODEX_API FThroughBallFeetFormulaResolutionExecutionInput
{
	FThroughBallFeetFormulaResolverInputAssemblyResult
		ResolverInputAssemblyResult;
};

enum class EThroughBallFeetFormulaResolutionExecutionErrorCode : uint8
{
	None,
	ResolverInputAssemblyFailed,
	InvalidResolverInputAssemblyResult
};

struct FMCODEX_API FThroughBallFeetFormulaResolutionExecutionResult
{
	bool bSuccess = false;

	EThroughBallFeetFormulaResolutionExecutionErrorCode ErrorCode =
		EThroughBallFeetFormulaResolutionExecutionErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallFeetFormulaResolutionExecutionInput Input;

	bool bHasFormulaResolution = false;
	FFormulaResolutionResult FormulaResolutionResult;
};

class FMCODEX_API FThroughBallFeetFormulaResolutionExecutor final
{
public:
	static FThroughBallFeetFormulaResolutionExecutionResult Execute(
		const FThroughBallFeetFormulaResolutionExecutionInput& Input);
};
