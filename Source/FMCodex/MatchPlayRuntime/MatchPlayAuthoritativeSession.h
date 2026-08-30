#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAuthoritativeSessionTypes.h"

class FMCODEX_API FMatchPlayAuthoritativeSession final
{
public:
	FMatchPlayAuthoritativeSession();
	explicit FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider);
	explicit FMatchPlayAuthoritativeSession(
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider);
	FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	FMatchPlayAuthoritativeSession(
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	~FMatchPlayAuthoritativeSession();

#if WITH_DEV_AUTOMATION_TESTS
	FMatchPlayAuthoritativeSession(
		FMatchPlayState InReconstructedState,
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider);
#endif

	FMatchPlayAuthoritativeInitializeMatchResult InitializeMatch(
		const FMatchPlayOpeningInitializeInput& Input);

	FMatchPlayAuthoritativeBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);

	FMatchPlayAuthoritativeRequestInitialActionPointRollResult
	RequestInitialActionPointRoll(
		const FMatchPlayFullD12EntryRequest& Request);

	FMatchPlayAuthoritativeRequestSetPieceTypeRollResult
	RequestSetPieceTypeRoll(
		const FMatchPlaySetPieceTypeRollRequest& Request);

	FMatchPlayAuthoritativeResolveSendingOffResult ResolveSendingOff(
		const FMatchPlaySendingOffResolutionRequest& Request);

	FMatchPlayAuthoritativeFinishDeploymentResult FinishDeployment(
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);

	FMatchPlayAuthoritativeDeployOrdinaryResult DeployOrdinary(
		const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request);

	FMatchPlayAuthoritativeDeployGoalkeeperResult DeployGoalkeeper(
		const FMatchPlayAuthoritativeDeployGoalkeeperRequest& Request);

	FMatchPlayAuthoritativeSubmitCarrierResult SubmitCarrier(
		const FMatchPlayAuthoritativeSubmitCarrierRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalCarrierResult
	ResolveNoLegalCarrier();

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

	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollResult
	ResolveThroughBallInitialRouteRoll(
		const FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest&
			Request);

	FMatchPlayAuthoritativeResolvePassControlInitialRouteRollResult
	ResolvePassControlInitialRouteRoll(
		const FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveCrossInitialRouteRollResult
	ResolveCrossInitialRouteRoll(
		const FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest& Request);

	FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult
	ResolveCrossPostRoutePlan();

	FMatchPlayAuthoritativeResolveCrossHighAttackRollResult
	ResolveCrossHighAttackRoll(
		const FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest& Request);

	FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult
	ResolveCrossHighDefenseRoll(
		const FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest& Request);

	FMatchPlayAuthoritativeResolveCrossLowAttackRollResult
	ResolveCrossLowAttackRoll(
		const FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest& Request);

	FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult
	ResolveCrossLowDefenseRoll(
		const FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest& Request);

	FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult
	ResolveThroughBallFeetPostRoutePlan();

	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult
	ResolveThroughBallFeetAttackRoll(
		const FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult
	ResolveThroughBallFeetDefenseRoll(
		const FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest&
			Request);

	FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult
	ResolvePassControlPostRoutePlan();

	FMatchPlayAuthoritativeResolvePassControlAttackRollResult
	ResolvePassControlAttackRoll(
		const FMatchPlayAuthoritativeResolvePassControlAttackRollRequest& Request);

	FMatchPlayAuthoritativeResolvePassControlDefenseRollResult
	ResolvePassControlDefenseRoll(
		const FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest& Request);

	FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult
	ResolveDeadCornerPostRouteDecision();

	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollResult
	ResolveLongShotDeadCornerRoll(
		const FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest& Request);

	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult
	ResolveThroughBallAntiOffsideDecision();

	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollResult
	ResolveThroughBallAntiOffsideAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult
	ResolveDirectShotPostRouteDecisionOrPlan();

	FMatchPlayAuthoritativeResolveLongShotDirectAttackRollResult
	ResolveLongShotDirectAttackRoll(
		const FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollResult
	ResolveLongShotDirectDefenseRoll(
		const FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollResult
	ResolveCutInsideShotDirectAttackRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollResult
	ResolveCutInsideShotDirectDefenseRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollResult
	ResolveCutInsideShotDeadCornerRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest&
			Request);

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult
	ResolveThroughBallBehindDefenseP1DecisionOrPlan();

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollResult
	ResolveThroughBallBehindDefenseP1AttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollResult
	ResolveThroughBallBehindDefenseP1DefenseRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult
	ResolveSingleCardFinishingFormula();

	FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult
	ResolveThroughBallFeetFormula();

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult
	ResolveThroughBallBehindDefenseP1Formula();

	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult
	ResolveThroughBallBehindDefenseP2Decision();

	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult
	SubmitThroughBallOneOnOneShotChoice(
		const
			FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&
				Request);

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult
	ResolveThroughBallOneOnOneChipShotDecision();

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollResult
	ResolveThroughBallOneOnOneChipShotAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
	ResolveThroughBallOneOnOneDirectShotPostRoutePlan();

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollResult
	ResolveThroughBallOneOnOneDirectShotAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollResult
	ResolveThroughBallOneOnOneDirectShotDefenseRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest&
				Request);

	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult
	ResolveThroughBallOneOnOneDirectShotFormula();

	FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult
	ApplyThroughBallTerminalResolution();

	FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult
	ApplyCrossTerminalResolution();

	FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult
	ApplyPassControlTerminalResolution();

	FMatchPlayAuthoritativeApplyShotTerminalResolutionResult
	ApplyShotTerminalResolution();

	FMatchPlayAuthoritativeAdvanceAfterTerminalResult AdvanceAfterTerminal(
		const FMatchPlayAuthoritativeAdvanceAfterTerminalRequest& Request);

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
	IMatchPlayAttackEntryRollProvider* AttackEntryRollProvider = nullptr;
	IMatchPlayInitialRouteRollProvider* InitialRouteRollProvider = nullptr;
	IMatchPlayPostRouteRollProvider* PostRouteRollProvider = nullptr;
	FSkillRuleSnapshotSet AuthoritativeSkillRuleSet;
	bool bHasSkillRuleSet = false;
	bool bExecutingCommand = false;
};
