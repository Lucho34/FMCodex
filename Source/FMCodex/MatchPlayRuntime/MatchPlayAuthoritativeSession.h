#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAuthoritativeSessionTypes.h"

class FMCODEX_API FMatchPlayAuthoritativeSession final
{
public:
	FMatchPlayAuthoritativeSession();
	explicit FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider);
	FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	~FMatchPlayAuthoritativeSession();

	FMatchPlayAuthoritativeInitializeMatchResult InitializeMatch(
		const FMatchPlayOpeningInitializeInput& Input);

	FMatchPlayAuthoritativeBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);

	FMatchPlayAuthoritativeFinishDeploymentResult FinishDeployment(
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);

	FMatchPlayAuthoritativeDeployOrdinaryResult DeployOrdinary(
		const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request);

	FMatchPlayAuthoritativeSubmitCarrierResult SubmitCarrier(
		const FMatchPlayAuthoritativeSubmitCarrierRequest& Request);

	FMatchPlayAuthoritativeSubmitMarkerResult SubmitMarker(
		const FMatchPlayAuthoritativeSubmitMarkerRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalMarkerResult
	ResolveNoLegalMarker();

	FMatchPlayAuthoritativeDeclineMarkerResult DeclineMarker(
		const FMatchPlayAuthoritativeDeclineMarkerRequest& Request);

	FMatchPlayAuthoritativeSubmitSkillResult SubmitSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalSkillResult ResolveNoLegalSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet);

	FMatchPlayAuthoritativeDeclineSkillResult DeclineSkill(
		const FSkillRuleSnapshotSet& SkillRuleSet,
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);

	FMatchPlayAuthoritativeSubmitRunnerResult SubmitRunner(
		const FMatchPlayAuthoritativeSubmitRunnerRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalRunnerResult
	ResolveNoLegalRunner();

	FMatchPlayAuthoritativeDeclineRunnerResult DeclineRunner(
		const FMatchPlayAuthoritativeDeclineRunnerRequest& Request);

	FMatchPlayAuthoritativeSubmitHelperResult SubmitHelper(
		const FMatchPlayAuthoritativeSubmitHelperRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalHelperResult
	ResolveNoLegalHelper();

	FMatchPlayAuthoritativeDeclineHelperResult DeclineHelper(
		const FMatchPlayAuthoritativeDeclineHelperRequest& Request);

	FMatchPlayAuthoritativeBeginResolutionSessionResult
	BeginResolutionSession();

	FMatchPlayAuthoritativeSubmitBranchIntentResult SubmitBranchIntent(
		const FMatchPlayAuthoritativeSubmitBranchIntentRequest& Request);

	FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
	ResolveIntentDeterminedRoute();

	FMatchPlayAuthoritativeResolveInitialRouteResult ResolveInitialRoute();

	FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult
	ResolveCrossPostRoutePlan();

	FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult
	ResolveThroughBallFeetPostRoutePlan();

	FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult
	ResolvePassControlPostRoutePlan();

	FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult
	ResolveDeadCornerPostRouteDecision();

	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult
	ResolveThroughBallAntiOffsideDecision();

	FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult
	ResolveDirectShotPostRouteDecisionOrPlan();

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult
	ResolveThroughBallBehindDefenseP1DecisionOrPlan();

	FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult
	ResolveSingleCardFinishingFormula();

	FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult
	ResolveThroughBallFeetFormula();

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult
	ResolveThroughBallBehindDefenseP1Formula();

	FMatchPlayState GetStateSnapshot() const;

private:
	FMatchPlayAuthoritativeSession(
		const FMatchPlayAuthoritativeSession&) = delete;
	FMatchPlayAuthoritativeSession& operator=(
		const FMatchPlayAuthoritativeSession&) = delete;
	FMatchPlayAuthoritativeSession(
		FMatchPlayAuthoritativeSession&&) = delete;
	FMatchPlayAuthoritativeSession& operator=(
		FMatchPlayAuthoritativeSession&&) = delete;

	struct FDomainExecution
	{
		bool bSuccess = false;
		FMatchPlayState CandidateAfterState;
		EMatchPlayAuthoritativeStateDisposition StateDisposition =
			EMatchPlayAuthoritativeStateDisposition::DoNotAdopt;
		int64 AttackSequence = 0;
	};

	template <typename TTypedResult, typename TExecuteDomain>
	TTypedResult ExecuteSerialized(
		EMatchPlayAuthoritativeCommandKind CommandKind,
		bool bRequiresInitializedState,
		int64 CommandAttackSequence,
		TExecuteDomain&& ExecuteDomain);

	FMatchPlayState AuthoritativeState;
	IMatchPlayInitialRouteRollProvider* InitialRouteRollProvider = nullptr;
	IMatchPlayPostRouteRollProvider* PostRouteRollProvider = nullptr;
	FSkillRuleSnapshotSet AuthoritativeSkillRuleSet;
	bool bHasSkillRuleSet = false;
	bool bExecutingCommand = false;
};
