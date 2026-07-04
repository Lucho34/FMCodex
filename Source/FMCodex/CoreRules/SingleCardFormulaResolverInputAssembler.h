#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "SingleCardFormulaInputAssemblyQuery.h"

struct FMCODEX_API FSingleCardFormulaResolverInputAssemblyInput
{
	FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult;
	FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult;
	FName AttackerPlayerId = NAME_None;
	FName DefenderPlayerId = NAME_None;
};

enum class ESingleCardFormulaResolverInputAssemblyErrorCode : uint8
{
	None,
	AttackerQueryFailed,
	DefenderQueryFailed,
	InvalidAttackerQueryResult,
	InvalidDefenderQueryResult,
	AttackerCardIdMismatch,
	DefenderCardIdMismatch,
	InvalidAttackerRole,
	InvalidDefenderRole,
	UnsupportedFormulaType,
	FormulaTypeMismatch,
	LogIdMismatch,
	TurnIndexMismatch,
	InvalidAttackerAttribute,
	InvalidDefenderAttribute
};

struct FMCODEX_API FSingleCardFormulaResolverInputAssemblyResult
{
	bool bSuccess = false;
	ESingleCardFormulaResolverInputAssemblyErrorCode ErrorCode =
		ESingleCardFormulaResolverInputAssemblyErrorCode::None;
	FString ErrorMessage;
	FName InvalidSide = NAME_None;
	FName InvalidField = NAME_None;
	FSingleCardFormulaInputAssemblyQueryResult AttackerQueryResult;
	FSingleCardFormulaInputAssemblyQueryResult DefenderQueryResult;
	FFormulaResolverInput ResolverInput;
};

class FMCODEX_API FSingleCardFormulaResolverInputAssembler final
{
public:
	static FSingleCardFormulaResolverInputAssemblyResult Assemble(
		const FSingleCardFormulaResolverInputAssemblyInput& Input);
};
