#pragma once

#include "CoreMinimal.h"

#include "FMCodexLocalMatchInteractionView.h"

struct FFMCodexLocalMatchResolveIntentDeterminedRouteResult;
struct FFMCodexLocalMatchResolveInitialRouteResult;
struct FFMCodexLocalMatchResolveCrossPostRoutePlanResult;
struct FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult;
struct FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult;
struct FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult;
struct FFMCodexLocalMatchResolvePassControlPostRoutePlanResult;
struct FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult;
struct FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult;
struct FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult;
struct FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult;
struct FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult;
struct FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult;
struct FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult;
struct FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult;
struct FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult;
struct FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult;
struct FFMCodexLocalMatchApplyCrossTerminalResolutionResult;
struct FFMCodexLocalMatchApplyPassControlTerminalResolutionResult;
struct FFMCodexLocalMatchApplyShotTerminalResolutionResult;

struct FMCODEX_API FFMCodexLocalMatchResolutionFeedback
{
	bool bVisible = false;
	bool bRejected = false;
	bool bTerminal = false;
	FString CommandName;
	FString StepTitle;
	FString StepSummary;
	FString RouteSummary;
	TArray<FFMCodexLocalMatchRollView> DiceEntries;
	FMatchPlayCurrentAttackResolutionFactProjection ResolutionFacts;
	TArray<FString> ComparisonEntries;
	FString DecisionSummary;
	FString ContinuationSummary;
	FString TerminalSummary;
	FString ErrorMessage;
};

class FMCODEX_API FFMCodexLocalMatchResolutionFeedbackBuilder final
{
public:
	static FFMCodexLocalMatchResolutionFeedback BuildRejected(
		const FString& CommandName,
		const FString& ErrorMessage,
		bool bAuthoritativeAccepted);

	static FFMCodexLocalMatchResolutionFeedback BuildFromTerminalSnapshot(
		const FFMCodexLocalMatchInteractionView& TerminalView);

	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveIntentDeterminedRouteResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveInitialRouteResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveCrossPostRoutePlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolvePassControlPostRoutePlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchApplyCrossTerminalResolutionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchApplyPassControlTerminalResolutionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const FFMCodexLocalMatchApplyShotTerminalResolutionResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);

	template <typename TResult>
	static FFMCodexLocalMatchResolutionFeedback Build(
		const FString& CommandName,
		const TResult& Result,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView)
	{
		return BuildGenericAccepted(CommandName, BeforeView, AfterView);
	}

private:
	static FFMCodexLocalMatchResolutionFeedback BuildGenericAccepted(
		const FString& CommandName,
		const FFMCodexLocalMatchInteractionView& BeforeView,
		const FFMCodexLocalMatchInteractionView& AfterView);
};
