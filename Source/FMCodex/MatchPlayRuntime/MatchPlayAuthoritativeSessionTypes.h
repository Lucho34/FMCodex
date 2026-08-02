#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayBeginOrdinaryAttack.h"
#include "../CoreRules/MatchPlayFinishDeployment.h"
#include "../CoreRules/MatchPlayOpeningInitializer.h"

enum class EMatchPlayAuthoritativeStateDisposition : uint8
{
	DoNotAdopt,
	Adopt
};

enum class EMatchPlayAuthoritativeCommandKind : uint8
{
	None,
	InitializeMatch,
	BeginOrdinaryAttack,
	FinishDeployment
};

enum class EMatchPlayAuthoritativeRuntimeFailureCode : uint8
{
	None,
	NotInitialized,
	AlreadyInitialized,
	ReentrantCommand
};

enum class EMatchPlayAuthoritativeFailureDisposition : uint8
{
	None,
	RetryableExecutionFailure,
	NonRetryableExecutionFailure,
	NonRetryableInvariantFailure
};

struct FMCODEX_API FMatchPlayAuthoritativeRuntimeEnvelope
{
	bool bAccepted = false;
	bool bDomainSuccess = false;
	bool bStateAdvanced = false;
	EMatchPlayAuthoritativeStateDisposition StateDisposition =
		EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
	bool bRuntimeFault = false;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	EMatchPlayAuthoritativeCommandKind CommandKind =
		EMatchPlayAuthoritativeCommandKind::None;
	int64 AttackSequence = 0;
	EMatchPlayAuthoritativeFailureDisposition FailureDisposition =
		EMatchPlayAuthoritativeFailureDisposition::None;
	EMatchPlayAuthoritativeRuntimeFailureCode RuntimeFailureCode =
		EMatchPlayAuthoritativeRuntimeFailureCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FMatchPlayAuthoritativeInitializeMatchResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayOpeningInitializeResult OpeningResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeBeginOrdinaryAttackResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayBeginOrdinaryAttackResult BeginResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeFinishDeploymentResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayFinishDeploymentResult FinishResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeStateAdoptionResult
{
	FMatchPlayState AdoptedAfterState;
	bool bStateAdvanced = false;
};

class FMCODEX_API FMatchPlayAuthoritativeStateAdoptionPolicy final
{
public:
	static FMatchPlayAuthoritativeStateAdoptionResult Apply(
		const FMatchPlayState& CurrentAuthoritativeState,
		const FMatchPlayState& CandidateAfterState,
		EMatchPlayAuthoritativeStateDisposition StateDisposition);
};
