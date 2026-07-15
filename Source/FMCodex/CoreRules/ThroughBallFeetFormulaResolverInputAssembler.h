#pragma once

#include "CoreMinimal.h"

#include "FormulaResolver.h"
#include "ThroughBallFeetPlanQuery.h"

struct FMCODEX_API FThroughBallFeetFormulaResolverInputAssemblyInput
{
	FThroughBallFeetFormulaPlan FormulaPlan;
};

enum class EThroughBallFeetFormulaResolverInputAssemblyErrorCode : uint8
{
	None,
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
	InvalidGoalScorerIdentity,
	InvalidTerminalMetadata
};

struct FMCODEX_API FThroughBallFeetFormulaResolverInputAssemblyResult
{
	bool bSuccess = false;

	EThroughBallFeetFormulaResolverInputAssemblyErrorCode ErrorCode =
		EThroughBallFeetFormulaResolverInputAssemblyErrorCode::None;

	FString ErrorMessage;
	FName InvalidField = NAME_None;

	FThroughBallFeetFormulaResolverInputAssemblyInput Input;

	bool bHasResolverInput = false;
	FFormulaResolverInput ResolverInput;
};

class FMCODEX_API FThroughBallFeetFormulaResolverInputAssembler final
{
public:
	static FThroughBallFeetFormulaResolverInputAssemblyResult Assemble(
		const FThroughBallFeetFormulaResolverInputAssemblyInput& Input);
};
