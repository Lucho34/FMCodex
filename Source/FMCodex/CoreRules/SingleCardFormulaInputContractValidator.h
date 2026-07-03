#pragma once

#include "CoreMinimal.h"

#include "SingleCardFormulaInputContract.h"

enum class ESingleCardFormulaInputContractValidationErrorCode : uint8
{
	None,
	InvalidCardId,
	UnsupportedFormulaType,
	InvalidParticipantRole,
	InvalidAttribute,
	GoalkeeperRoleRequiresGoalkeeperAttribute,
	NonGoalkeeperRoleCannotUseGoalkeeperAttribute,
	MissingExternalD6ComparePoint,
	D6ComparePointOutOfRange,
	MissingExternalModifier,
	InvalidModifier,
	InvalidLogContext
};

struct FMCODEX_API FSingleCardFormulaInputContractValidationResult
{
	bool bSuccess = false;
	bool bIsValid = false;
	ESingleCardFormulaInputContractValidationErrorCode ErrorCode =
		ESingleCardFormulaInputContractValidationErrorCode::None;
	FString ErrorMessage;
	FName InvalidCardId = NAME_None;
	FName InvalidField = NAME_None;
};

class FMCODEX_API FSingleCardFormulaInputContractValidator final
{
public:
	static constexpr int32 MinD6ComparePoint = 1;
	static constexpr int32 MaxD6ComparePoint = 6;

	static FSingleCardFormulaInputContractValidationResult Validate(
		const FSingleCardFormulaInputContract& Contract);
};
