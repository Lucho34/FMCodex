#pragma once

#include "CoreMinimal.h"

#include <type_traits>

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
	template<
		typename TPostRouteProvider,
		std::enable_if_t<
			std::is_base_of_v<
				IMatchPlayPostRouteRollProvider,
				TPostRouteProvider>
			&& std::is_base_of_v<
				IMatchPlayRecoveryProvider,
				TPostRouteProvider>,
			int> = 0>
	FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		TPostRouteProvider& InSharedPostRouteAndRecoveryProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet)
		: FMatchPlayAuthoritativeSession(
			InInitialRouteRollProvider,
			static_cast<IMatchPlayPostRouteRollProvider&>(
				InSharedPostRouteAndRecoveryProvider),
			static_cast<IMatchPlayRecoveryProvider&>(
				InSharedPostRouteAndRecoveryProvider),
			InSkillRuleSet)
	{
	}
	FMatchPlayAuthoritativeSession(
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		IMatchPlayRecoveryProvider& InRecoveryProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	FMatchPlayAuthoritativeSession(
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	FMatchPlayAuthoritativeSession(
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayInitialRouteRollProvider& InInitialRouteRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		IMatchPlayRecoveryProvider& InRecoveryProvider,
		const FSkillRuleSnapshotSet& InSkillRuleSet);
	~FMatchPlayAuthoritativeSession();

#if WITH_DEV_AUTOMATION_TESTS
	FMatchPlayAuthoritativeSession(
		FMatchPlayState InReconstructedState,
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider);
	FMatchPlayAuthoritativeSession(
		FMatchPlayState InReconstructedState,
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayRecoveryProvider& InRecoveryProvider);
	FMatchPlayAuthoritativeSession(
		FMatchPlayState InReconstructedState,
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider);
	FMatchPlayAuthoritativeSession(
		FMatchPlayState InReconstructedState,
		IMatchPlayAttackEntryRollProvider& InAttackEntryRollProvider,
		IMatchPlayPostRouteRollProvider& InPostRouteRollProvider,
		IMatchPlayRecoveryProvider& InRecoveryProvider);
#endif

	FMatchPlayAuthoritativeInitializeMatchResult InitializeMatch(
		const FMatchPlayOpeningInitializeInput& Input);

#if WITH_DEV_AUTOMATION_TESTS
	/** Legacy fixture entry only. Production attack entry is Full D12. */
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);
#endif

	FMatchPlayAuthoritativeRequestInitialActionPointRollResult
	RequestInitialActionPointRoll(
		const FMatchPlayFullD12EntryRequest& Request);

	FMatchPlayAuthoritativeRequestSetPieceTypeRollResult
	RequestSetPieceTypeRoll(
		const FMatchPlaySetPieceTypeRollRequest& Request);

	FMatchPlayAuthoritativeResolveSendingOffResult ResolveSendingOff(
		const FMatchPlaySendingOffResolutionRequest& Request);

	FMatchPlayAuthoritativeSubmitSetPieceCarrierResult SubmitSetPieceCarrier(
		const FMatchPlaySetPieceCarrierSelectionRequest& Request);

	FMatchPlayAuthoritativeSubmitShortFreeKickMethodResult
	SubmitShortFreeKickMethod(
		const FMatchPlayShortFreeKickMethodRequest& Request);

	FMatchPlayAuthoritativeResolveShortFreeKickDirectAttackRollResult
	ResolveShortFreeKickDirectAttackRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveShortFreeKickDirectDefenseRollResult
	ResolveShortFreeKickDirectDefenseRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveShortFreeKickAngledRollResult
	ResolveShortFreeKickAngledRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalSetPieceCarrierResult
	ResolveNoLegalSetPieceCarrier(
		const FMatchPlayShortFreeKickNoLegalCarrierRequest& Request);

	FMatchPlayAuthoritativeSubmitLongFreeKickMethodResult
	SubmitLongFreeKickMethod(
		const FMatchPlayLongFreeKickMethodRequest& Request);

	FMatchPlayAuthoritativeResolveLongFreeKickDirectAttackRollResult
	ResolveLongFreeKickDirectAttackRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveLongFreeKickDirectDefenseRollResult
	ResolveLongFreeKickDirectDefenseRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveLongFreeKickPowerRollResult
	ResolveLongFreeKickPowerRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalLongFreeKickCarrierResult
	ResolveNoLegalSetPieceCarrier(
		const FMatchPlayLongFreeKickNoLegalCarrierRequest& Request);

	FMatchPlayAuthoritativeSubmitPenaltyMethodResult SubmitPenaltyMethod(
		const FMatchPlayPenaltyMethodRequest& Request);

	FMatchPlayAuthoritativeResolvePenaltyDirectAttackRollResult
	ResolvePenaltyDirectAttackRoll(
		const FMatchPlayPenaltyRollRequest& Request);

	FMatchPlayAuthoritativeResolvePenaltyDirectDefenseRollResult
	ResolvePenaltyDirectDefenseRoll(
		const FMatchPlayPenaltyRollRequest& Request);

	FMatchPlayAuthoritativeResolvePenaltyPanenkaRollResult
	ResolvePenaltyPanenkaRoll(
		const FMatchPlayPenaltyRollRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalPenaltyCarrierResult
	ResolveNoLegalSetPieceCarrier(
		const FMatchPlayPenaltyNoLegalCarrierRequest& Request);

	FMatchPlayAuthoritativeSubmitCornerAttackerNominationsResult
	SubmitCornerAttackerNominations(
		const FMatchPlayCornerNominationRequest& Request);

	FMatchPlayAuthoritativeSubmitCornerDefenderNominationsResult
	SubmitCornerDefenderNominations(
		const FMatchPlayCornerNominationRequest& Request);

	FMatchPlayAuthoritativeRequestCornerParticipantSelectionRollResult
	RequestCornerParticipantSelectionRoll(
		const FMatchPlayCornerRollRequest& Request);

	FMatchPlayAuthoritativeSubmitCornerIntentResult SubmitCornerIntent(
		const FMatchPlayCornerIntentRequest& Request);

	FMatchPlayAuthoritativeRequestCornerRouteRollResult
	RequestCornerRouteRoll(const FMatchPlayCornerRollRequest& Request);

	FMatchPlayAuthoritativeRequestCornerAttackRollResult
	RequestCornerAttackRoll(const FMatchPlayCornerRollRequest& Request);

	FMatchPlayAuthoritativeRequestCornerDefenseRollResult
	RequestCornerDefenseRoll(const FMatchPlayCornerRollRequest& Request);

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
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);

	FMatchPlayAuthoritativeResolveNoLegalSkillResult ResolveNoLegalSkill();

	FMatchPlayAuthoritativeDeclineSkillResult DeclineSkill(
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);

#if WITH_DEV_AUTOMATION_TESTS
	/** Test-fixture compatibility only; never a production transport surface. */
	FMatchPlayAuthoritativeSubmitSkillResult SubmitSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet,
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);
	FMatchPlayAuthoritativeResolveNoLegalSkillResult ResolveNoLegalSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet);
	FMatchPlayAuthoritativeDeclineSkillResult DeclineSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet,
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);
#endif

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
	IMatchPlayRecoveryProvider* RecoveryProvider = nullptr;
	FSkillRuleSnapshotSet AuthoritativeSkillRuleSet;
	bool bHasSkillRuleSet = false;
	bool bExecutingCommand = false;
};
