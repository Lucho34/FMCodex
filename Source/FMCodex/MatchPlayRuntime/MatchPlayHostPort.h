#pragma once

#include "CoreMinimal.h"
#include "Misc/TVariant.h"

#include "MatchPlayAuthoritativeSessionTypes.h"

/** Player-owned deployment completion did not previously have a request DTO. */
struct FMCODEX_API FMatchPlayFinishDeploymentIntent
{
	int64 AttackSequence = 0;
	EInitialTurnOrderPlayer RequestingSide = EInitialTurnOrderPlayer::None;
};

using FMatchPlayPlayerIntentPayload = TVariant<
	FEmptyVariantState,
	FMatchPlayFullD12EntryRequest,
	FMatchPlaySetPieceTypeRollRequest,
	FMatchPlaySetPieceCarrierSelectionRequest,
	FMatchPlayShortFreeKickMethodRequest,
	FMatchPlayShortFreeKickRollRequest,
	FMatchPlayLongFreeKickMethodRequest,
	FMatchPlayLongFreeKickRollRequest,
	FMatchPlayPenaltyMethodRequest,
	FMatchPlayPenaltyRollRequest,
	FMatchPlayCornerNominationRequest,
	FMatchPlayCornerRollRequest,
	FMatchPlayCornerIntentRequest,
	FMatchPlayFinishDeploymentIntent,
	FMatchPlayAuthoritativeDeployOrdinaryRequest,
	FMatchPlayAuthoritativeDeployGoalkeeperRequest,
	FMatchPlayAuthoritativeSubmitCarrierRequest,
	FMatchPlayAuthoritativeSubmitMarkerRequest,
	FMatchPlayAuthoritativeDeclineMarkerRequest,
	FMatchPlayAuthoritativeSubmitSkillRequest,
	FMatchPlayAuthoritativeDeclineSkillRequest,
	FMatchPlayAuthoritativeSubmitRunnerRequest,
	FMatchPlayAuthoritativeDeclineRunnerRequest,
	FMatchPlayAuthoritativeSubmitHelperRequest,
	FMatchPlayAuthoritativeDeclineHelperRequest,
	FMatchPlayAuthoritativeSubmitBranchIntentRequest,
	FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest,
	FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest,
	FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest,
	FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest,
	FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest,
	FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest,
	FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest,
	FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest,
	FMatchPlayAuthoritativeResolvePassControlAttackRollRequest,
	FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest,
	FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest,
	FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest,
	FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest,
	FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest,
	FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest,
	FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest,
	FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest,
	FMatchPlayAuthoritativeAdvanceAfterTerminalRequest>;

/**
 * Transport-neutral player command. It contains only stable value identity and
 * an existing correlated request DTO; it never accepts providers, rolls,
 * formula results, rule sets, or UObject identity.
 */
struct FMCODEX_API FMatchPlayPlayerIntent
{
	EMatchPlayAuthoritativeCommandKind CommandKind =
		EMatchPlayAuthoritativeCommandKind::None;
	FMatchPlayPlayerIntentPayload Payload;

	template <typename TRequest>
	static FMatchPlayPlayerIntent Create(
		const EMatchPlayAuthoritativeCommandKind InCommandKind,
		const TRequest& Request)
	{
		FMatchPlayPlayerIntent Result;
		Result.CommandKind = InCommandKind;
		Result.Payload.Set<TRequest>(Request);
		return Result;
	}
};

enum class EMatchPlayPlayerIntentPortErrorCode : uint8
{
	None,
	NoActiveMatch,
	NotPlayerIntent,
	PayloadTypeMismatch,
	AuthoritativeCommandRejected,
	ServerCoordinatorFailed
};

enum class EMatchPlayServerCoordinatorStopReason : uint8
{
	None,
	WaitingForPlayerIntent,
	TerminalPendingAdvance,
	MatchEnded,
	NotInitialized,
	InternalActionFailed,
	InvalidAuthoritativeState,
	SafetyLimitReached
};

struct FMCODEX_API FMatchPlayServerCoordinatorStep
{
	EMatchPlayAuthoritativeCommandKind CommandKind =
		EMatchPlayAuthoritativeCommandKind::None;
	bool bStateAdvanced = false;
	EMatchPlayAuthoritativeFailureDisposition FailureDisposition =
		EMatchPlayAuthoritativeFailureDisposition::None;
	FString ErrorMessage;
};

struct FMCODEX_API FMatchPlayServerCoordinatorResult
{
	bool bSuccess = false;
	bool bStateAdvanced = false;
	EMatchPlayServerCoordinatorStopReason StopReason =
		EMatchPlayServerCoordinatorStopReason::None;
	TArray<FMatchPlayServerCoordinatorStep> Steps;
	FString ErrorMessage;
};

struct FMCODEX_API FMatchPlayPlayerIntentAuthorityResult
{
	FMatchPlayAuthoritativeRuntimeEnvelope RuntimeEnvelope;
};

struct FMCODEX_API FMatchPlayPlayerIntentSubmissionResult
{
	bool bSuccess = false;
	bool bPlayerIntentAccepted = false;
	FMatchPlayPlayerIntentAuthorityResult AuthoritativeResult;
	FMatchPlayServerCoordinatorResult CoordinatorResult;
	EMatchPlayPlayerIntentPortErrorCode ErrorCode =
		EMatchPlayPlayerIntentPortErrorCode::None;
	FString ErrorMessage;
};

/** Shared client-facing command seam. ServerInternalAction is never accepted. */
class FMCODEX_API IMatchPlayPlayerIntentPort
{
public:
	virtual ~IMatchPlayPlayerIntentPort() = default;

	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FMatchPlayPlayerIntent& Intent) = 0;
};
