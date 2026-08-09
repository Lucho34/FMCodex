#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackCompletion.h"
#include "MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallFeetFormulaTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionTypes.h"
#include "MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaTypes.h"

enum class
	EMatchPlayCurrentAttackApplyThroughBallTerminalResolutionErrorCode
	: uint8
{
	None,
	MatchPlayStateNotInitialized,
	NoCurrentAttack,
	MissingResolutionSession,
	InvalidResolutionSessionState,
	RouteNotResolved,
	NotThroughBallResolution,
	IncompleteTerminalProvenance,
	SourceRegenerationFailed,
	SourceSemanticIsNonTerminal,
	InvalidTerminalSemantic,
	TerminalRegenerationConsumedRng,
	CompletionFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionResult
{
	bool bSuccess = false;
	bool bIsGoal = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayCurrentAttackApplyThroughBallTerminalResolutionErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackApplyThroughBallTerminalResolutionErrorCode
				::None;
	FString ErrorMessage;
	FName InvalidField = NAME_None;

	EMatchPlayThroughBallTerminalSource TerminalSource =
		EMatchPlayThroughBallTerminalSource::None;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionStateValidationResult;
	int32 RegenerationProviderCallCount = 0;

	int32 FeetFormulaRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult
		FeetFormulaRegenerationResult;
	int32 AntiOffsideRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
		AntiOffsideRegenerationResult;
	int32 BehindDefenseP1PlanRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult
		BehindDefenseP1PlanRegenerationResult;
	int32 BehindDefenseP1FormulaRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
		BehindDefenseP1FormulaRegenerationResult;
	int32 BehindDefenseP2RegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult
		BehindDefenseP2RegenerationResult;
	int32 OneOnOneRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult
		OneOnOneRegenerationResult;
	int32 DirectShotRegenerationCount = 0;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaResult
		DirectShotRegenerationResult;

	int32 CompletionExecutionCount = 0;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
};
