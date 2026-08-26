#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayBeginOrdinaryAttack.h"
#include "../CoreRules/MatchPlayCarrierNoSelectionNoGoal.h"
#include "../CoreRules/MatchPlayCurrentAttackApplyCrossTerminalResolutionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackApplyPassControlTerminalResolutionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackApplyShotTerminalResolutionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackApplyThroughBallTerminalResolutionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackCompletion.h"
#include "../CoreRules/MatchPlayCurrentAttackBeginResolutionSessionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackBranchIntentSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackCarrierSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackHelperSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackMarkerSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveInitialRouteOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveCrossPostRoutePlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolvePassControlPostRoutePlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveSingleCardFinishingFormulaOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallFeetFormulaOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanOrchestrator.h"
#include "../CoreRules/MatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackResolveInitialRouteWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackRunnerSelectionWriter.h"
#include "../CoreRules/MatchPlayCurrentAttackSkillSelectionWriter.h"
#include "../CoreRules/MatchPlayFinishDeployment.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentAvailability.h"
#include "../CoreRules/MatchPlayGoalkeeperDeploymentWriter.h"
#include "../CoreRules/MatchPlayHelperAbsence.h"
#include "../CoreRules/MatchPlayMarkerNoSelectionGoal.h"
#include "../CoreRules/MatchPlayOpeningInitializer.h"
#include "../CoreRules/MatchPlayOrdinaryDeploymentWriter.h"
#include "../CoreRules/MatchPlayRunnerNoSelectionNoGoal.h"
#include "../CoreRules/MatchPlaySkillNoSelectionNoGoal.h"

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
	FinishDeployment,
	DeployOrdinary,
	SubmitCarrier,
	SubmitMarker,
	ResolveNoLegalMarker,
	DeclineMarker,
	SubmitSkill,
	ResolveNoLegalSkill,
	DeclineSkill,
	SubmitRunner,
	ResolveNoLegalRunner,
	DeclineRunner,
	SubmitHelper,
	ResolveNoLegalHelper,
	DeclineHelper,
	BeginResolutionSession,
	SubmitBranchIntent,
	ResolveIntentDeterminedRoute,
	ResolveInitialRoute,
	ResolveCrossPostRoutePlan,
	ResolveThroughBallFeetPostRoutePlan,
	ResolvePassControlPostRoutePlan,
	ResolveDeadCornerPostRouteDecision,
	ResolveThroughBallAntiOffsideDecision,
	ResolveDirectShotPostRouteDecisionOrPlan,
	ResolveThroughBallBehindDefenseP1DecisionOrPlan,
	ResolveSingleCardFinishingFormula,
	ResolveThroughBallFeetFormula,
	ResolveThroughBallBehindDefenseP1Formula,
	ResolveThroughBallBehindDefenseP2Decision,
	SubmitThroughBallOneOnOneShotChoice,
	ResolveThroughBallOneOnOneChipShotDecision,
	ResolveThroughBallOneOnOneDirectShotPostRoutePlan,
	ResolveThroughBallOneOnOneDirectShotFormula,
	ApplyThroughBallTerminalResolution,
	ApplyCrossTerminalResolution,
	ApplyPassControlTerminalResolution,
	ApplyShotTerminalResolution,
	DeployGoalkeeper,
	ResolveNoLegalCarrier,
	ResolveCrossHighAttackRoll,
	ResolveCrossHighDefenseRoll,
	ResolveCrossLowAttackRoll,
	ResolveCrossLowDefenseRoll,
	ResolveThroughBallFeetAttackRoll,
	ResolveThroughBallFeetDefenseRoll,
	AdvanceAfterTerminal
};

enum class EMatchPlayAuthoritativeRuntimeFailureCode : uint8
{
	None,
	NotInitialized,
	AlreadyInitialized,
	ReentrantCommand,
	TerminalAdvanceRequired
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

struct FMCODEX_API FMatchPlayAuthoritativeDeployOrdinaryRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName CardId = NAME_None;
	FName SlotId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeployGoalkeeperRequest
{
	FName SlotId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitCarrierRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName CarrierCardId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitMarkerRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName MarkerCardId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineMarkerRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitSkillRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName SkillId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineSkillRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitRunnerRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName RunnerCardId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineRunnerRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitHelperRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	FName HelperCardId = NAME_None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineHelperRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitBranchIntentRequest
{
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
	EMatchPlayElectiveBranchIntent Intent =
		EMatchPlayElectiveBranchIntent::None;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
	EMatchPlayThroughBallOneOnOneShotChoice Choice =
		EMatchPlayThroughBallOneOnOneShotChoice::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeployOrdinaryResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayOrdinaryDeploymentWriterResult DeploymentResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeployGoalkeeperResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayGoalkeeperDeploymentAvailabilityResult AvailabilityResult;
	FMatchPlayGoalkeeperDeploymentWriterResult DeploymentResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitCarrierResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackCarrierSelectionWriterResult CarrierResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveNoLegalCarrierResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayResolveNoLegalCarrierResult ResolutionResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitMarkerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackMarkerSelectionWriterResult MarkerResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveNoLegalMarkerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayResolveNoLegalMarkerResult ResolutionResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineMarkerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayMarkerDeclineResult DeclineResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitSkillResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackSkillSelectionWriterResult SkillResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveNoLegalSkillResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayResolveNoLegalSkillResult ResolutionResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineSkillResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlaySkillDeclineResult DeclineResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitRunnerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackRunnerSelectionWriterResult RunnerResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveNoLegalRunnerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayResolveNoLegalRunnerResult ResolutionResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineRunnerResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayRunnerDeclineResult DeclineResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitHelperResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackHelperSelectionWriterResult HelperResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveNoLegalHelperResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayResolveNoLegalHelperResult ResolutionResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeDeclineHelperResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayHelperDeclineResult DeclineResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeBeginResolutionSessionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackBeginResolutionSessionWriterResult BeginResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeSubmitBranchIntentResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackBranchIntentSelectionWriterResult IntentResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveInitialRouteWriterResult RouteResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveInitialRouteResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveInitialRouteOrchestrationResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossHighAttackRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossLowAttackRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveCrossPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest
{
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallFeetPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolvePassControlPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveDeadCornerPostRouteDecisionResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallAntiOffsideDecisionResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveDirectShotPostRouteDecisionOrPlanResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1DecisionOrPlanResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveSingleCardFinishingFormulaResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallFeetFormulaResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP1FormulaResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallBehindDefenseP2DecisionResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackThroughBallOneOnOneShotChoiceSelectionWriterResult
		ChoiceResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneChipShotDecisionResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackResolveThroughBallOneOnOneDirectShotFormulaResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackApplyThroughBallTerminalResolutionResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackApplyCrossTerminalResolutionResult
		OrchestrationResult;
};

struct FMCODEX_API
	FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackApplyPassControlTerminalResolutionResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeApplyShotTerminalResolutionResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackApplyShotTerminalResolutionResult
		OrchestrationResult;
};

struct FMCODEX_API FMatchPlayAuthoritativeAdvanceAfterTerminalRequest
{
	int64 AttackSequence = 0;
	EInitialTurnOrderPlayer RequestingSide =
		EInitialTurnOrderPlayer::None;
};

struct FMCODEX_API FMatchPlayAuthoritativeAdvanceAfterTerminalResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
	FMatchPlayCurrentAttackCompletionResult CompletionResult;
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
