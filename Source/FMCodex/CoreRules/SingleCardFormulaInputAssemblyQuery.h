#pragma once

#include "CoreMinimal.h"

#include "PlayerCardRuleSnapshotQuery.h"
#include "SingleCardFormulaInputContractValidator.h"

struct FMCODEX_API FSingleCardFormulaInputAssemblyQueryInput
{
	FName CardId = NAME_None;
	EFormulaType FormulaType = EFormulaType::None;
	ESingleCardFormulaParticipantRole ParticipantRole =
		ESingleCardFormulaParticipantRole::None;
	ESingleCardFormulaAttribute Attribute =
		ESingleCardFormulaAttribute::None;
	bool bHasExternalD6ComparePoint = false;
	int32 ExternalD6ComparePoint = 0;
	bool bHasExternalModifier = false;
	float ExternalModifier = 0.0f;
	FGuid LogId;
	int32 TurnIndex = 0;
	FName ContextTag = NAME_None;
};

enum class ESingleCardFormulaInputAssemblyQueryErrorCode : uint8
{
	None,
	ContractValidationFailed,
	SnapshotQueryFailed,
	GoalkeeperRoleRequiresGoalkeeperSnapshot,
	NonGoalkeeperRoleCannotUseGoalkeeperSnapshot
};

struct FMCODEX_API FSingleCardFormulaInputAssemblyQueryResult
{
	bool bSuccess = false;
	ESingleCardFormulaInputAssemblyQueryErrorCode ErrorCode =
		ESingleCardFormulaInputAssemblyQueryErrorCode::None;
	FString ErrorMessage;
	FName CardId = NAME_None;
	FName InvalidField = NAME_None;
	FPlayerCardRuleSnapshotQueryResult SnapshotQueryResult;
	FSingleCardFormulaInputContractValidationResult
		ContractValidationResult;
	FSingleCardFormulaInputContract Contract;
};

class FMCODEX_API FSingleCardFormulaInputAssemblyQuery final
{
public:
	static FSingleCardFormulaInputAssemblyQueryResult Assemble(
		const FPlayerCardRuleSnapshotSet& SnapshotSet,
		const FSingleCardFormulaInputAssemblyQueryInput& Input);
};
