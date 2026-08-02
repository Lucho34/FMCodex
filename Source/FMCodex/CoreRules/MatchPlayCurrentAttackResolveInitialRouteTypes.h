#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackInitialRouteMappingQuery.h"
#include "MatchPlayCurrentAttackResolutionSessionStateValidator.h"
#include "MatchPlayInitialRouteRollProvider.h"

#include "MatchPlayCurrentAttackResolveInitialRouteTypes.generated.h"

USTRUCT()
struct FMCODEX_API FMatchPlayCurrentAttackResolveInitialRouteRequest
{
	GENERATED_BODY()

	UPROPERTY()
	int64 AttackSequence = 0;
};

enum class EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
	: uint8
{
	None,
	MatchPlayNotInitialized,
	NoCurrentAttack,
	InvalidCurrentAttackAttackSequence,
	InvalidRequestAttackSequence,
	AttackSequenceMismatch,
	CurrentAttackNotInResolution,
	InvalidResolutionSessionState,
	NoResolutionSession,
	SessionAttackSequenceMismatch,
	ResolutionSessionNotAwaitingRoute,
	UnsupportedAction,
	InvalidIntentForAction
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult
{
	bool bSuccess = false;
	bool bIsCanonicalDuplicate = false;
	FMatchPlayCurrentAttackResolveInitialRouteRequest Request;
	ESkillRuleType ActionType = ESkillRuleType::None;
	EMatchPlayElectiveBranchIntent Intent =
		EMatchPlayElectiveBranchIntent::None;
	bool bRequiresInitialRouteD6 = false;
	EMatchPlayCurrentAttackResolutionRollPurpose ExpectedRollPurpose =
		EMatchPlayCurrentAttackResolutionRollPurpose::None;
	FMatchPlayCurrentAttackResolutionSessionBundle ImmutableBundle;
	FMatchPlayCurrentAttackActualBranch ExistingActualBranch;
	TArray<FMatchPlayCurrentAttackResolutionRollRecord>
		ExistingInitialRouteRollRecords;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		SessionValidationResult;
	EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
		ErrorCode =
			EMatchPlayCurrentAttackResolveInitialRouteGlobalContextErrorCode
				::None;
	FString ErrorMessage;
};

enum class EMatchPlayCurrentAttackResolveInitialRouteLegalityErrorCode
	: uint8
{
	None,
	GlobalContextFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteLegalityResult
{
	bool bIsLegal = false;
	FMatchPlayCurrentAttackResolveInitialRouteRequest Request;
	FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult
		GlobalContextResult;
	EMatchPlayCurrentAttackResolveInitialRouteLegalityErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveInitialRouteLegalityErrorCode::None;
	FString ErrorMessage;
};

enum class EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode
	: uint8
{
	None,
	GlobalContextFailed,
	LegalityFailed,
	RngProviderUnavailable,
	RngProviderFailed,
	InvalidRngResult,
	InitialRouteMappingInvariantViolation,
	CandidateInvariantViolation
};

enum class EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
	: uint8
{
	None,
	RetryableExecutionFailure,
	NonRetryableExecutionFailure,
	NonRetryableInvariantFailure
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteWriterResult
{
	bool bSuccess = false;
	bool bResolvedNewRoute = false;
	FMatchPlayCurrentAttackResolveInitialRouteRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackActualBranch ActualBranch;
	TArray<FMatchPlayCurrentAttackResolutionRollRecord>
		InitialRouteRollRecords;
	EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode ErrorCode =
		EMatchPlayCurrentAttackResolveInitialRouteWriterErrorCode::None;
	FString ErrorMessage;
	EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
		FailureDisposition =
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None;
	FMatchPlayCurrentAttackResolveInitialRouteGlobalContextResult
		GlobalContextResult;
	FMatchPlayCurrentAttackResolveInitialRouteLegalityResult
		LegalityResult;
	bool bProviderCalled = false;
	FMatchPlayInitialRouteRollProviderResult ProviderResult;
	FMatchPlayCurrentAttackInitialRouteMappingResult MappingResult;
	FMatchPlayCurrentAttackResolutionSessionStateValidationResult
		CandidateValidationResult;
};
