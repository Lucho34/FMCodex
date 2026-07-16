#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "ThroughBallBehindDefenseP1PlanQuery.h"

enum class EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode : uint8
{
	None,
	PlanQueryFailed,
	InvalidPlanQueryResult,
	UnsupportedPlanQueryDecision,
	MissingFormulaPlan,
	UnsupportedFormulaType,
	InvalidRequiredParticipantIdentity,
	InvalidOptionalParticipantState,
	InvalidAttackFormulaData,
	InvalidDefenseFormulaData,
	InvalidAttackParticipatingStamina,
	InvalidDefenseParticipatingStamina,
	InvalidLogContext,
	InvalidOwnerIdentity,
	InvalidInvolvedCardIds,
	InvalidWinnerMetadata
};

struct FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput
{
	FThroughBallBehindDefenseP1PlanQueryResult PlanQueryResult;
};

struct FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult
{
	bool bSuccess = false;

	EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode ErrorCode =
		EThroughBallBehindDefenseP1FormulaResolverInputAssemblyErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput Input;

	bool bHasResolverInput = false;
	FFormulaResolverInput ResolverInput;
};

class FMCODEX_API
FThroughBallBehindDefenseP1FormulaResolverInputAssembler final
{
public:
	static FThroughBallBehindDefenseP1FormulaResolverInputAssemblyResult Assemble(
		const FThroughBallBehindDefenseP1FormulaResolverInputAssemblyInput& Input);
};
