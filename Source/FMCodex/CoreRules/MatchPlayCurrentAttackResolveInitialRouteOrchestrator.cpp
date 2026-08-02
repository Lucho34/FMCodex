#include "MatchPlayCurrentAttackResolveInitialRouteOrchestrator.h"

#include "MatchPlayCurrentAttackBeginResolutionSessionWriter.h"
#include "MatchPlayCurrentAttackResolveInitialRouteWriter.h"

FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
FMatchPlayCurrentAttackResolveInitialRouteOrchestrator::Resolve(
	const FMatchPlayState& BeforeState,
	const FMatchPlayCurrentAttackResolveInitialRouteOrchestrationRequest&
		Request,
	IMatchPlayInitialRouteRollProvider* RollProvider)
{
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult Result;
	Result.Request = Request;
	Result.BeforeState = BeforeState;
	Result.AfterState = BeforeState;

	FMatchPlayCurrentAttackBeginResolutionSessionRequest BeginRequest;
	BeginRequest.AttackSequence = Request.AttackSequence;
	Result.BeginResult =
		FMatchPlayCurrentAttackBeginResolutionSessionWriter::Begin(
			BeforeState,
			BeginRequest);
	Result.bBeganNewSession = Result.BeginResult.bCreatedNewSession;
	if (!Result.BeginResult.bSuccess)
	{
		Result.FailureStage =
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
				::BeginResolutionSession;
		Result.ErrorCode =
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode
				::BeginResolutionSessionFailed;
		Result.ErrorMessage = FString::Printf(
			TEXT("Initial Route orchestration failed while beginning the Resolution Session: %s"),
			*Result.BeginResult.ErrorMessage);
		return Result;
	}

	FMatchPlayCurrentAttackResolveInitialRouteRequest RouteRequest;
	RouteRequest.AttackSequence = Request.AttackSequence;
	Result.RouteResult =
		FMatchPlayCurrentAttackResolveInitialRouteWriter::Resolve(
			Result.BeginResult.AfterState,
			RouteRequest,
			RollProvider);
	Result.bResolvedNewRoute =
		Result.RouteResult.bResolvedNewRoute;
	Result.FailureDisposition =
		Result.RouteResult.FailureDisposition;
	Result.bProviderCalled = Result.RouteResult.bProviderCalled;
	if (!Result.RouteResult.bSuccess)
	{
		Result.AfterState = Result.BeginResult.AfterState;
		Result.FailureStage =
			EMatchPlayCurrentAttackInitialRouteOrchestrationFailureStage
				::ResolveInitialRoute;
		Result.ErrorCode =
			EMatchPlayCurrentAttackInitialRouteOrchestrationErrorCode
				::InitialRouteResolutionFailed;
		Result.ErrorMessage = FString::Printf(
			TEXT("Initial Route orchestration failed while resolving the Initial Route: %s"),
			*Result.RouteResult.ErrorMessage);
		return Result;
	}

	Result.AfterState = Result.RouteResult.AfterState;
	Result.bSuccess = true;
	return Result;
}
