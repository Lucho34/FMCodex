#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "FMCodexLocalMatchD6Provider.h"
#include "FMCodexMatchClientViewPort.h"
#if !UE_BUILD_SHIPPING
#include "FMCodexLocalDevRollOverride.h"
#endif
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../MatchPlayRuntime/MatchPlayHostPort.h"
#include "../MatchPlayRuntime/MatchPlayServerCoordinator.h"

#include "FMCodexLocalMatchHostGameMode.generated.h"

enum class EFMCodexLocalMatchHostErrorCode : uint8
{
	None,
	NoActiveMatch,
	AuthoritativeInitializationFailed,
	AuthoritativeCommandFailed,
	RuleConfigurationMismatch,
	InvalidRequestingSide,
	RequestingSideNotCurrentAttacker,
	TacticalPointRollNotReady
};

struct FMCODEX_API FFMCodexStartNewLocalMatchResult
{
	bool bSuccess = false;
	bool bReplacedExistingMatch = false;
	FMatchPlayAuthoritativeInitializeMatchResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSnapshotResult
{
	bool bSuccess = false;
	FMatchPlayState Snapshot;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSkillRuleSnapshotResult
{
	bool bSuccess = false;
	FSkillRuleSnapshotSet Snapshot;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchBeginOrdinaryAttackResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchRollTacticalPointsResult
{
	bool bSuccess = false;
	int32 TacticalPoints = 0;
	FMatchPlayAuthoritativeBeginOrdinaryAttackResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeployOrdinaryResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeployOrdinaryResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeployGoalkeeperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeployGoalkeeperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchFinishDeploymentResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeFinishDeploymentResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitCarrierResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitCarrierResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalCarrierResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalCarrierResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineMarkerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineMarkerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineSkillResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineSkillResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineRunnerResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineRunnerResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveNoLegalHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveNoLegalHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchDeclineHelperResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeDeclineHelperResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitBranchIntentResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitBranchIntentResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchBeginResolutionSessionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeBeginResolutionSessionResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveIntentDeterminedRouteResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveIntentDeterminedRouteResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveInitialRouteResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveInitialRouteResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveThroughBallInitialRouteRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolvePassControlInitialRouteRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolvePassControlInitialRouteRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossInitialRouteRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossInitialRouteRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossPostRoutePlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossPostRoutePlanResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossHighAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossHighAttackRollResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossHighDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossHighDefenseRollResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossLowAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossLowAttackRollResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveCrossLowDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCrossLowDefenseRollResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallFeetPostRoutePlanResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolvePassControlPostRoutePlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolvePassControlPostRoutePlanResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolvePassControlAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolvePassControlAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolvePassControlDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolvePassControlDefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveDeadCornerPostRouteDecisionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveLongShotDeadCornerRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideDecisionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallAntiOffsideAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveDirectShotPostRouteDecisionOrPlanResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveLongShotDirectAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveLongShotDirectAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveLongShotDirectDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveCutInsideShotDirectAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveCutInsideShotDirectDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveCutInsideShotDeadCornerRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DecisionOrPlanResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1AttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveSingleCardFinishingFormulaResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveSingleCardFinishingFormulaResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchResolveThroughBallFeetFormulaResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallFeetFormulaResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1FormulaResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP2DecisionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotDecisionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotAttackRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotDefenseRollResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API
	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotFormulaResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeApplyThroughBallTerminalResolutionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchApplyCrossTerminalResolutionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeApplyCrossTerminalResolutionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchApplyPassControlTerminalResolutionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeApplyPassControlTerminalResolutionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchApplyShotTerminalResolutionResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeApplyShotTerminalResolutionResult
		AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

struct FMCODEX_API FFMCodexLocalMatchAdvanceAfterTerminalResult
{
	bool bSuccess = false;
	FMatchPlayAuthoritativeAdvanceAfterTerminalResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

/** Bounded LocalPlay transport wrapper for the already-typed Session APIs. */
template <typename TAuthoritativeResult>
struct TFMCodexLocalMatchTypedCommandResult
{
	bool bSuccess = false;
	TAuthoritativeResult AuthoritativeResult;
	EFMCodexLocalMatchHostErrorCode ErrorCode =
		EFMCodexLocalMatchHostErrorCode::None;
	FString ErrorMessage;
};

using FFMCodexLocalMatchRequestInitialActionPointRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestInitialActionPointRollResult>;
using FFMCodexLocalMatchRequestSetPieceTypeRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestSetPieceTypeRollResult>;
using FFMCodexLocalMatchResolveSendingOffResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveSendingOffResult>;
using FFMCodexLocalMatchSubmitSetPieceCarrierResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitSetPieceCarrierResult>;
using FFMCodexLocalMatchSubmitShortFreeKickMethodResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitShortFreeKickMethodResult>;
using FFMCodexLocalMatchResolveShortFreeKickDirectAttackRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveShortFreeKickDirectAttackRollResult>;
using FFMCodexLocalMatchResolveShortFreeKickDirectDefenseRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveShortFreeKickDirectDefenseRollResult>;
using FFMCodexLocalMatchResolveShortFreeKickAngledRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveShortFreeKickAngledRollResult>;
using FFMCodexLocalMatchSubmitLongFreeKickMethodResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitLongFreeKickMethodResult>;
using FFMCodexLocalMatchResolveLongFreeKickDirectAttackRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveLongFreeKickDirectAttackRollResult>;
using FFMCodexLocalMatchResolveLongFreeKickDirectDefenseRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveLongFreeKickDirectDefenseRollResult>;
using FFMCodexLocalMatchResolveLongFreeKickPowerRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveLongFreeKickPowerRollResult>;
using FFMCodexLocalMatchSubmitPenaltyMethodResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitPenaltyMethodResult>;
using FFMCodexLocalMatchResolvePenaltyDirectAttackRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolvePenaltyDirectAttackRollResult>;
using FFMCodexLocalMatchResolvePenaltyDirectDefenseRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolvePenaltyDirectDefenseRollResult>;
using FFMCodexLocalMatchResolvePenaltyPanenkaRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolvePenaltyPanenkaRollResult>;
using FFMCodexLocalMatchResolveNoLegalShortFreeKickCarrierResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveNoLegalSetPieceCarrierResult>;
using FFMCodexLocalMatchResolveNoLegalLongFreeKickCarrierResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveNoLegalLongFreeKickCarrierResult>;
using FFMCodexLocalMatchResolveNoLegalPenaltyCarrierResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeResolveNoLegalPenaltyCarrierResult>;
using FFMCodexLocalMatchSubmitCornerAttackerNominationsResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitCornerAttackerNominationsResult>;
using FFMCodexLocalMatchSubmitCornerDefenderNominationsResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitCornerDefenderNominationsResult>;
using FFMCodexLocalMatchRequestCornerParticipantSelectionRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestCornerParticipantSelectionRollResult>;
using FFMCodexLocalMatchSubmitCornerIntentResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeSubmitCornerIntentResult>;
using FFMCodexLocalMatchRequestCornerRouteRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestCornerRouteRollResult>;
using FFMCodexLocalMatchRequestCornerAttackRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestCornerAttackRollResult>;
using FFMCodexLocalMatchRequestCornerDefenseRollResult =
	TFMCodexLocalMatchTypedCommandResult<
		FMatchPlayAuthoritativeRequestCornerDefenseRollResult>;

UCLASS()
class FMCODEX_API AFMCodexLocalMatchHostGameMode final
	: public AGameModeBase
	, public IMatchPlayPlayerIntentPort
	, public IFMCodexMatchClientViewPort
{
	GENERATED_BODY()

public:
	AFMCodexLocalMatchHostGameMode();

	bool HasActiveLocalMatch() const;

	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FMatchPlayPlayerIntent& Intent) override;

	virtual FFMCodexMatchClientViewResult GetViewForViewer(
		const FFMCodexMatchClientViewRequest& Request) const override;

	/** Server/local-owner retry seam; never part of the player command port. */
	FMatchPlayServerCoordinatorResult AdvanceServerCoordinator();

	FFMCodexStartNewLocalMatchResult StartNewLocalMatch(
		const FMatchPlayOpeningInitializeInput& Input);

	FFMCodexStartNewLocalMatchResult StartNewLocalMatch(
		const FMatchPlayOpeningInitializeInput& Input,
		const FSkillRuleSnapshotSet& SkillRuleSet);

#if WITH_DEV_AUTOMATION_TESTS
	FFMCodexStartNewLocalMatchResult StartNewLocalMatch(
		const FMatchPlayOpeningInitializeInput& Input,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		int32 DeterministicSeedForTesting);
#endif

	FFMCodexLocalMatchSnapshotResult GetMatchSnapshot() const;

	FFMCodexLocalMatchSkillRuleSnapshotResult
		GetSkillRuleSnapshot() const;

#if WITH_DEV_AUTOMATION_TESTS
	/** Legacy fixture entry only. Production uses RequestInitialActionPointRoll. */
	FFMCodexLocalMatchRollTacticalPointsResult RollTacticalPoints(
		EInitialTurnOrderPlayer RequestingSide);
#endif

	FFMCodexLocalMatchRequestInitialActionPointRollResult
	RequestInitialActionPointRoll(const FMatchPlayFullD12EntryRequest& Request);
	FFMCodexLocalMatchRequestSetPieceTypeRollResult RequestSetPieceTypeRoll(
		const FMatchPlaySetPieceTypeRollRequest& Request);
	FFMCodexLocalMatchResolveSendingOffResult ResolveSendingOff(
		const FMatchPlaySendingOffResolutionRequest& Request);
	FFMCodexLocalMatchSubmitSetPieceCarrierResult SubmitSetPieceCarrier(
		const FMatchPlaySetPieceCarrierSelectionRequest& Request);
	FFMCodexLocalMatchSubmitShortFreeKickMethodResult SubmitShortFreeKickMethod(
		const FMatchPlayShortFreeKickMethodRequest& Request);
	FFMCodexLocalMatchResolveShortFreeKickDirectAttackRollResult
	ResolveShortFreeKickDirectAttackRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);
	FFMCodexLocalMatchResolveShortFreeKickDirectDefenseRollResult
	ResolveShortFreeKickDirectDefenseRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);
	FFMCodexLocalMatchResolveShortFreeKickAngledRollResult
	ResolveShortFreeKickAngledRoll(
		const FMatchPlayShortFreeKickRollRequest& Request);
	FFMCodexLocalMatchSubmitLongFreeKickMethodResult SubmitLongFreeKickMethod(
		const FMatchPlayLongFreeKickMethodRequest& Request);
	FFMCodexLocalMatchResolveLongFreeKickDirectAttackRollResult
	ResolveLongFreeKickDirectAttackRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);
	FFMCodexLocalMatchResolveLongFreeKickDirectDefenseRollResult
	ResolveLongFreeKickDirectDefenseRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);
	FFMCodexLocalMatchResolveLongFreeKickPowerRollResult ResolveLongFreeKickPowerRoll(
		const FMatchPlayLongFreeKickRollRequest& Request);
	FFMCodexLocalMatchSubmitPenaltyMethodResult SubmitPenaltyMethod(
		const FMatchPlayPenaltyMethodRequest& Request);
	FFMCodexLocalMatchResolvePenaltyDirectAttackRollResult
	ResolvePenaltyDirectAttackRoll(const FMatchPlayPenaltyRollRequest& Request);
	FFMCodexLocalMatchResolvePenaltyDirectDefenseRollResult
	ResolvePenaltyDirectDefenseRoll(const FMatchPlayPenaltyRollRequest& Request);
	FFMCodexLocalMatchResolvePenaltyPanenkaRollResult ResolvePenaltyPanenkaRoll(
		const FMatchPlayPenaltyRollRequest& Request);
	FFMCodexLocalMatchResolveNoLegalShortFreeKickCarrierResult
	ResolveNoLegalShortFreeKickCarrier(
		const FMatchPlayShortFreeKickNoLegalCarrierRequest& Request);
	FFMCodexLocalMatchResolveNoLegalLongFreeKickCarrierResult
	ResolveNoLegalLongFreeKickCarrier(
		const FMatchPlayLongFreeKickNoLegalCarrierRequest& Request);
	FFMCodexLocalMatchResolveNoLegalPenaltyCarrierResult
	ResolveNoLegalPenaltyCarrier(
		const FMatchPlayPenaltyNoLegalCarrierRequest& Request);
	FFMCodexLocalMatchSubmitCornerAttackerNominationsResult
	SubmitCornerAttackerNominations(const FMatchPlayCornerNominationRequest& Request);
	FFMCodexLocalMatchSubmitCornerDefenderNominationsResult
	SubmitCornerDefenderNominations(const FMatchPlayCornerNominationRequest& Request);
	FFMCodexLocalMatchRequestCornerParticipantSelectionRollResult
	RequestCornerParticipantSelectionRoll(const FMatchPlayCornerRollRequest& Request);
	FFMCodexLocalMatchSubmitCornerIntentResult SubmitCornerIntent(
		const FMatchPlayCornerIntentRequest& Request);
	FFMCodexLocalMatchRequestCornerRouteRollResult RequestCornerRouteRoll(
		const FMatchPlayCornerRollRequest& Request);
	FFMCodexLocalMatchRequestCornerAttackRollResult RequestCornerAttackRoll(
		const FMatchPlayCornerRollRequest& Request);
	FFMCodexLocalMatchRequestCornerDefenseRollResult RequestCornerDefenseRoll(
		const FMatchPlayCornerRollRequest& Request);

#if WITH_DEV_AUTOMATION_TESTS
	FFMCodexLocalMatchBeginOrdinaryAttackResult BeginOrdinaryAttack(
		int32 ActionPoint);
#endif

	FFMCodexLocalMatchDeployOrdinaryResult DeployOrdinary(
		const FMatchPlayAuthoritativeDeployOrdinaryRequest& Request);

	FFMCodexLocalMatchDeployGoalkeeperResult DeployGoalkeeper(
		const FMatchPlayAuthoritativeDeployGoalkeeperRequest& Request);

	FFMCodexLocalMatchFinishDeploymentResult FinishDeployment(
		int64 AttackSequence,
		EInitialTurnOrderPlayer RequestingSide);

	FFMCodexLocalMatchSubmitCarrierResult SubmitCarrier(
		const FMatchPlayAuthoritativeSubmitCarrierRequest& Request);

	FFMCodexLocalMatchResolveNoLegalCarrierResult ResolveNoLegalCarrier();

	FFMCodexLocalMatchSubmitMarkerResult SubmitMarker(
		const FMatchPlayAuthoritativeSubmitMarkerRequest& Request);

	FFMCodexLocalMatchResolveNoLegalMarkerResult ResolveNoLegalMarker();

	FFMCodexLocalMatchDeclineMarkerResult DeclineMarker(
		const FMatchPlayAuthoritativeDeclineMarkerRequest& Request);

	FFMCodexLocalMatchSubmitSkillResult SubmitSkill(
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);

	FFMCodexLocalMatchResolveNoLegalSkillResult ResolveNoLegalSkill();

	FFMCodexLocalMatchDeclineSkillResult DeclineSkill(
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);

#if WITH_DEV_AUTOMATION_TESTS
	/** Test-fixture compatibility only; rules here are not player intent. */
	FFMCodexLocalMatchSubmitSkillResult SubmitSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet,
		const FMatchPlayAuthoritativeSubmitSkillRequest& Request);
	FFMCodexLocalMatchResolveNoLegalSkillResult ResolveNoLegalSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet);
	FFMCodexLocalMatchDeclineSkillResult DeclineSkill(
		const FSkillRuleSnapshotSet& FixtureSkillRuleSet,
		const FMatchPlayAuthoritativeDeclineSkillRequest& Request);
#endif

	FFMCodexLocalMatchSubmitRunnerResult SubmitRunner(
		const FMatchPlayAuthoritativeSubmitRunnerRequest& Request);

	FFMCodexLocalMatchResolveNoLegalRunnerResult ResolveNoLegalRunner();

	FFMCodexLocalMatchDeclineRunnerResult DeclineRunner(
		const FMatchPlayAuthoritativeDeclineRunnerRequest& Request);

	FFMCodexLocalMatchSubmitHelperResult SubmitHelper(
		const FMatchPlayAuthoritativeSubmitHelperRequest& Request);

	FFMCodexLocalMatchResolveNoLegalHelperResult ResolveNoLegalHelper();

	FFMCodexLocalMatchDeclineHelperResult DeclineHelper(
		const FMatchPlayAuthoritativeDeclineHelperRequest& Request);

	FFMCodexLocalMatchSubmitBranchIntentResult SubmitBranchIntent(
		const FMatchPlayAuthoritativeSubmitBranchIntentRequest& Request);

	FFMCodexLocalMatchSubmitThroughBallOneOnOneShotChoiceResult
	SubmitThroughBallOneOnOneShotChoice(
		const FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest&
			Request);

	FFMCodexLocalMatchBeginResolutionSessionResult BeginResolutionSession();

	FFMCodexLocalMatchResolveIntentDeterminedRouteResult
	ResolveIntentDeterminedRoute();

	FFMCodexLocalMatchResolveInitialRouteResult ResolveInitialRoute();

	FFMCodexLocalMatchResolveThroughBallInitialRouteRollResult
	ResolveThroughBallInitialRouteRoll(
		const FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest&
			Request);

	FFMCodexLocalMatchResolvePassControlInitialRouteRollResult
	ResolvePassControlInitialRouteRoll(
		const FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest&
			Request);

	FFMCodexLocalMatchResolveCrossInitialRouteRollResult
	ResolveCrossInitialRouteRoll(
		const FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest& Request);

	FFMCodexLocalMatchResolveCrossPostRoutePlanResult
	ResolveCrossPostRoutePlan();

	FFMCodexLocalMatchResolveCrossHighAttackRollResult
	ResolveCrossHighAttackRoll(
		const FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest& Request);

	FFMCodexLocalMatchResolveCrossHighDefenseRollResult
	ResolveCrossHighDefenseRoll(
		const FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest& Request);

	FFMCodexLocalMatchResolveCrossLowAttackRollResult
	ResolveCrossLowAttackRoll(
		const FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest& Request);

	FFMCodexLocalMatchResolveCrossLowDefenseRollResult
	ResolveCrossLowDefenseRoll(
		const FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest& Request);

	FFMCodexLocalMatchResolveThroughBallFeetPostRoutePlanResult
	ResolveThroughBallFeetPostRoutePlan();

	FFMCodexLocalMatchResolveThroughBallFeetAttackRollResult
	ResolveThroughBallFeetAttackRoll(
		const FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest&
			Request);

	FFMCodexLocalMatchResolveThroughBallFeetDefenseRollResult
	ResolveThroughBallFeetDefenseRoll(
		const FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest&
			Request);

	FFMCodexLocalMatchResolvePassControlPostRoutePlanResult
	ResolvePassControlPostRoutePlan();

	FFMCodexLocalMatchResolvePassControlAttackRollResult
	ResolvePassControlAttackRoll(
		const FMatchPlayAuthoritativeResolvePassControlAttackRollRequest& Request);

	FFMCodexLocalMatchResolvePassControlDefenseRollResult
	ResolvePassControlDefenseRoll(
		const FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest& Request);

	FFMCodexLocalMatchResolveDeadCornerPostRouteDecisionResult
	ResolveDeadCornerPostRouteDecision();

	FFMCodexLocalMatchResolveLongShotDeadCornerRollResult
	ResolveLongShotDeadCornerRoll(
		const FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest& Request);

	FFMCodexLocalMatchResolveThroughBallAntiOffsideDecisionResult
	ResolveThroughBallAntiOffsideDecision();

	FFMCodexLocalMatchResolveThroughBallAntiOffsideAttackRollResult
	ResolveThroughBallAntiOffsideAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest&
				Request);

	FFMCodexLocalMatchResolveDirectShotPostRouteDecisionOrPlanResult
	ResolveDirectShotPostRouteDecisionOrPlan();

	FFMCodexLocalMatchResolveLongShotDirectAttackRollResult
	ResolveLongShotDirectAttackRoll(
		const FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest&
			Request);

	FFMCodexLocalMatchResolveLongShotDirectDefenseRollResult
	ResolveLongShotDirectDefenseRoll(
		const FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest&
			Request);

	FFMCodexLocalMatchResolveCutInsideShotDirectAttackRollResult
	ResolveCutInsideShotDirectAttackRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest&
			Request);

	FFMCodexLocalMatchResolveCutInsideShotDirectDefenseRollResult
	ResolveCutInsideShotDirectDefenseRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest&
			Request);

	FFMCodexLocalMatchResolveCutInsideShotDeadCornerRollResult
	ResolveCutInsideShotDeadCornerRoll(
		const FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest&
			Request);

	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DecisionOrPlanResult
	ResolveThroughBallBehindDefenseP1DecisionOrPlan();

	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1AttackRollResult
	ResolveThroughBallBehindDefenseP1AttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest&
				Request);

	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1DefenseRollResult
	ResolveThroughBallBehindDefenseP1DefenseRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest&
				Request);

	FFMCodexLocalMatchResolveSingleCardFinishingFormulaResult
	ResolveSingleCardFinishingFormula();

	FFMCodexLocalMatchResolveThroughBallFeetFormulaResult
	ResolveThroughBallFeetFormula();

	FFMCodexLocalMatchResolveThroughBallBehindDefenseP1FormulaResult
	ResolveThroughBallBehindDefenseP1Formula();

	FFMCodexLocalMatchResolveThroughBallBehindDefenseP2DecisionResult
	ResolveThroughBallBehindDefenseP2Decision();

	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotDecisionResult
	ResolveThroughBallOneOnOneChipShotDecision();

	FFMCodexLocalMatchResolveThroughBallOneOnOneChipShotAttackRollResult
	ResolveThroughBallOneOnOneChipShotAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest&
				Request);

	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotPostRoutePlanResult
	ResolveThroughBallOneOnOneDirectShotPostRoutePlan();

	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotAttackRollResult
	ResolveThroughBallOneOnOneDirectShotAttackRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest&
				Request);

	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotDefenseRollResult
	ResolveThroughBallOneOnOneDirectShotDefenseRoll(
		const
			FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest&
				Request);

	FFMCodexLocalMatchResolveThroughBallOneOnOneDirectShotFormulaResult
	ResolveThroughBallOneOnOneDirectShotFormula();

	FFMCodexLocalMatchApplyThroughBallTerminalResolutionResult
	ApplyThroughBallTerminalResolution();

	FFMCodexLocalMatchApplyCrossTerminalResolutionResult
	ApplyCrossTerminalResolution();

	FFMCodexLocalMatchApplyPassControlTerminalResolutionResult
	ApplyPassControlTerminalResolution();

	FFMCodexLocalMatchApplyShotTerminalResolutionResult
	ApplyShotTerminalResolution();

	FFMCodexLocalMatchAdvanceAfterTerminalResult AdvanceAfterTerminal(
		const FMatchPlayAuthoritativeAdvanceAfterTerminalRequest& Request);

#if !UE_BUILD_SHIPPING
	FFMCodexLocalDevRollOverrideCommandResult SetLocalDevRollOverride(
		const FFMCodexLocalDevRollOverrideRequest& Request);
	FFMCodexLocalDevRollOverrideCommandResult SetLocalDevRecoveryOverride(
		const TArray<int32>& OrderedCandidateIndices);
	bool ClearLocalDevRecoveryOverride();
	bool ClearLocalDevRollOverride(EFMCodexLocalDevRollTarget Target);
	void ClearAllLocalDevRollOverrides();
	TArray<FFMCodexLocalDevPendingRollOverride>
		GetLocalDevPendingRollOverrides() const;
#endif

private:
	FFMCodexStartNewLocalMatchResult StartNewLocalMatchWithSeed(
		const FMatchPlayOpeningInitializeInput& Input,
		const FSkillRuleSnapshotSet& SkillRuleSet,
		int32 Seed);

	struct FLocalMatchRuntime final
	{
		FLocalMatchRuntime(
			int32 Seed,
			const FSkillRuleSnapshotSet& InSkillRuleSet);

		template <typename TCallable>
		decltype(auto) ExecuteProviderCall(
#if !UE_BUILD_SHIPPING
			const EFMCodexLocalDevRollInvocation DevInvocation,
#endif
			TCallable&& Callable)
		{
#if !UE_BUILD_SHIPPING
			return DevRollOverride.InvokeAs(
				DevInvocation,
				Forward<TCallable>(Callable));
#else
			return Forward<TCallable>(Callable)();
#endif
		}

		FFMCodexLocalMatchD6Provider D6Provider;
#if !UE_BUILD_SHIPPING
		FFMCodexLocalDevRollOverride DevRollOverride;
#endif
		const FSkillRuleSnapshotSet SkillRuleSet;
		FMatchPlayAuthoritativeSession AuthoritativeSession;
		FMatchPlayServerCoordinator ServerCoordinator;
	};

	template <typename TLocalResult, typename TCallable>
	TLocalResult ExecuteTypedCommand(TCallable&& Callable)
	{
		TLocalResult Result;
		if (!ActiveMatchRuntime.IsValid())
		{
			Result.ErrorCode = EFMCodexLocalMatchHostErrorCode::NoActiveMatch;
			Result.ErrorMessage = TEXT("No local match is active.");
			return Result;
		}
		Result.AuthoritativeResult = Callable(*ActiveMatchRuntime);
		const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope =
			Result.AuthoritativeResult.RuntimeEnvelope;
		Result.bSuccess = Envelope.bAccepted && Envelope.bDomainSuccess;
		if (!Result.bSuccess)
		{
			Result.ErrorCode =
				EFMCodexLocalMatchHostErrorCode::AuthoritativeCommandFailed;
			Result.ErrorMessage = Envelope.ErrorMessage;
		}
		return Result;
	}

	TUniquePtr<FLocalMatchRuntime> ActiveMatchRuntime;
};
