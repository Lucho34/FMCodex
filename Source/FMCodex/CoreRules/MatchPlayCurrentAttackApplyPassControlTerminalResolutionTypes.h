#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaTypes.h"

enum class EMatchPlayCurrentAttackApplyPassControlTerminalResolutionErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotPassControlResolution,
	InvalidPassControlBranch,
	FormulaRegenerationFailed,
	InvalidTerminalFormulaSemantic,
	TerminalRegenerationConsumedRng,
	CompletionFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackApplyPassControlTerminalResolutionResult
{
	bool bSuccess = false;
	bool bIsGoal = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackApplyPassControlTerminalResolutionErrorCode ErrorCode =
		EMatchPlayCurrentAttackApplyPassControlTerminalResolutionErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	EMatchPlayPassControlTerminalSource TerminalSource =
		EMatchPlayPassControlTerminalSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	int32 RegenerationProviderCallCount = 0;
	int32 FormulaRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
		FormulaRegenerationResult;

	int32 CompletionExecutionCount = 0;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
};
