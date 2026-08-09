#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionTypes.h"
#include "MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanTypes.h"
#include "MatchPlayCurrentAttackResolveSingleCardFinishingFormulaTypes.h"

enum class EMatchPlayCurrentAttackApplyShotTerminalResolutionErrorCode : uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotShotResolution,
	InvalidShotBranch,
	SourceRegenerationFailed,
	SourceRegenerationMutatedState,
	InvalidDirectShotSemantic,
	FormulaRegenerationFailed,
	InvalidTerminalFormulaSemantic,
	InvalidDeadCornerSemantic,
	TerminalRegenerationConsumedRng,
	CompletionFailed
};

struct FMCODEX_API FMatchPlayCurrentAttackApplyShotTerminalResolutionResult
{
	bool bSuccess = false;
	bool bIsGoal = false;
	bool bUsedFormula = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackApplyShotTerminalResolutionErrorCode ErrorCode =
		EMatchPlayCurrentAttackApplyShotTerminalResolutionErrorCode::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	ESkillRuleType ActionType = ESkillRuleType::None;
	EMatchPlayShotTerminalSource TerminalSource =
		EMatchPlayShotTerminalSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	int32 RegenerationProviderCallCount = 0;
	int32 SourceRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
		DirectShotRegenerationResult;
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult
		DeadCornerRegenerationResult;

	int32 FormulaRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
		FormulaRegenerationResult;

	int32 CompletionExecutionCount = 0;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
};
