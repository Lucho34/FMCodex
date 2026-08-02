#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionTypes.h"
#include "MatchPlayCurrentAttackResolveInitialRouteTypes.h"

#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrationTypes.generated.h"

USTRUCT()
struct FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest
{
	GENERATED_BODY()

	UPROPERTY()
	int64 AttackSequence = 0;
};

enum class EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
	: uint8
{
	None,
	BeginResolutionSession,
	ResolveInitialRoute
};

enum class EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode
	: uint8
{
	None,
	BeginResolutionSessionFailed,
	InitialRouteResolutionFailed
};

struct FMCODEX_API
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
{
	bool bSuccess = false;
	bool bBeganNewSession = false;
	bool bResolvedNewRoute = false;
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest Request;
	FMatchPlayState BeforeState;
	FMatchPlayState AfterState;
	FMatchPlayCurrentAttackBeginResolutionSessionWriterResult BeginResult;
	FMatchPlayCurrentAttackResolveInitialRouteWriterResult RouteResult;
	EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
		FailureStage =
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage::None;
	EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition
		FailureDisposition =
			EMatchPlayCurrentAttackResolveInitialRouteFailureDisposition::None;
	bool bProviderCalled = false;
	EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode ErrorCode =
		EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode::None;
	FString ErrorMessage;
};
