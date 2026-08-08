#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaTypes.h"

enum class EMatchPlayCurrentAttackApplyCrossTerminalResolutionErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotCrossResolution,
	InvalidCrossBranch,
	FormulaRegenerationFailed,
	InvalidTerminalFormulaSemantic,
	TerminalRegenerationConsumedRng,
	CompletionFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackApplyCrossTerminalResolutionResult
{
	bool bSuccess = false;
	bool bIsGoal = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackApplyCrossTerminalResolutionErrorCode ErrorCode =
		EMatchPlayCurrentAttackApplyCrossTerminalResolutionErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	EMatchPlayCrossTerminalSource TerminalSource =
		EMatchPlayCrossTerminalSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	int32 RegenerationProviderCallCount = 0;
	int32 FormulaRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
		FormulaRegenerationResult;

	int32 CompletionExecutionCount = 0;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
};
