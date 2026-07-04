#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"

enum class ESingleCardFormulaResolutionExecutionErrorCode : uint8
{
	None,
	InvalidInput,
	UnsupportedFormulaType,
	FormulaResolverFailed
};

struct FMCODEX_API FSingleCardFormulaResolutionExecutionResult
{
	bool bSuccess = false;
	bool bExecuted = false;
	ESingleCardFormulaResolutionExecutionErrorCode ErrorCode =
		ESingleCardFormulaResolutionExecutionErrorCode::None;
	FString ErrorMessage;
	FName InvalidSide = NAME_None;
	FName InvalidField = NAME_None;
	FFormulaResolverInput ResolverInput;
	FFormulaResolutionResult FormulaResolutionResult;
};

class FMCODEX_API FSingleCardFormulaResolutionExecutor final
{
public:
	static FSingleCardFormulaResolutionExecutionResult Execute(
		const FFormulaResolverInput& Input);
};
