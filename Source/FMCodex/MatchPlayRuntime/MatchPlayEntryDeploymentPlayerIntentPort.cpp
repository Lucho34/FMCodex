#include "MatchPlayEntryDeploymentPlayerIntentPort.h"
#include "../Diagnostics/FMCodexHandoffLatencyAudit.h"
#include "MatchPlayFullD12PlayerIntentPort.h"
#include "MatchPlayAuthoritativeSession.h"
#include "MatchPlayServerCoordinator.h"

FMatchPlayPlayerIntentSubmissionResult FMatchPlayEntryDeploymentPlayerIntentPort::SubmitPlayerIntent(
	const FMatchPlayPlayerIntent& Intent)
{
	if (Intent.CommandKind == EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll)
	{
		return FMatchPlayFullD12PlayerIntentPort(Session, Coordinator).SubmitPlayerIntent(Intent);
	}
	FMatchPlayPlayerIntentSubmissionResult Result;
	auto Mismatch = [&]()
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch;
		Result.ErrorMessage = TEXT("PlayerIntent payload does not match its command kind.");
		return Result;
	};
	auto Record = [&](const FMatchPlayAuthoritativeRuntimeEnvelope& Envelope,
		bool bDomainSuccess, const FString& DomainError)
	{
		Result.AuthoritativeResult.RuntimeEnvelope = Envelope;
		Result.bPlayerIntentAccepted = Envelope.bAccepted;
		if (!Envelope.bAccepted || !Envelope.bDomainSuccess || !bDomainSuccess)
		{
			Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::AuthoritativeCommandRejected;
			// Preserve the existing LocalPlay domain rejection text.
			Result.ErrorMessage = !Envelope.ErrorMessage.IsEmpty() ? Envelope.ErrorMessage : DomainError;
			return false;
		}
		return true;
	};
	switch (Intent.CommandKind)
	{
	case EMatchPlayAuthoritativeCommandKind::RequestSetPieceTypeRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlaySetPieceTypeRollRequest>()) return Mismatch();
		const auto Authority = Session.RequestSetPieceTypeRoll(Intent.Payload.Get<FMatchPlaySetPieceTypeRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.TypeRollResult.bSuccess, Authority.TypeRollResult.ErrorMessage)) return Result;
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitSetPieceCarrier:
	{
		if (!Intent.Payload.IsType<FMatchPlaySetPieceCarrierSelectionRequest>()) return Mismatch();
		const auto Authority = Session.SubmitSetPieceCarrier(Intent.Payload.Get<FMatchPlaySetPieceCarrierSelectionRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.CarrierResult.bSuccess, Authority.CarrierResult.ErrorMessage)) return Result;
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitShortFreeKickMethod:
	{
		if (!Intent.Payload.IsType<FMatchPlayShortFreeKickMethodRequest>()) return Mismatch();
		const auto Authority = Session.SubmitShortFreeKickMethod(Intent.Payload.Get<FMatchPlayShortFreeKickMethodRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.ResolutionResult.bSuccess, Authority.ResolutionResult.ErrorMessage)) return Result;
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitLongFreeKickMethod:
	{
		if (!Intent.Payload.IsType<FMatchPlayLongFreeKickMethodRequest>()) return Mismatch();
		const auto Authority = Session.SubmitLongFreeKickMethod(Intent.Payload.Get<FMatchPlayLongFreeKickMethodRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.ResolutionResult.bSuccess, Authority.ResolutionResult.ErrorMessage)) return Result;
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitPenaltyMethod:
	{
		if (!Intent.Payload.IsType<FMatchPlayPenaltyMethodRequest>()) return Mismatch();
		const auto Authority = Session.SubmitPenaltyMethod(Intent.Payload.Get<FMatchPlayPenaltyMethodRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.ResolutionResult.bSuccess, Authority.ResolutionResult.ErrorMessage)) return Result;
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeployOrdinary:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeployOrdinaryRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeployOrdinary(Intent.Payload.Get<FMatchPlayAuthoritativeDeployOrdinaryRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeploymentResult.bSuccess, Authority.DeploymentResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeployGoalkeeper:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeployGoalkeeperRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeployGoalkeeper(Intent.Payload.Get<FMatchPlayAuthoritativeDeployGoalkeeperRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeploymentResult.bSucceeded, Authority.DeploymentResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::FinishDeployment:
	{
		if (!Intent.Payload.IsType<FMatchPlayFinishDeploymentIntent>()) { return Mismatch(); }
		const auto& Request = Intent.Payload.Get<FMatchPlayFinishDeploymentIntent>();
		const auto Authority = Session.FinishDeployment(Request.AttackSequence, Request.RequestingSide);
		if (!Record(Authority.RuntimeEnvelope, Authority.FinishResult.bSuccess, Authority.FinishResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitCarrier:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitCarrierRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitCarrier(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitCarrierRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.CarrierResult.bSuccess, Authority.CarrierResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitMarker:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitMarkerRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitMarker(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitMarkerRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.MarkerResult.bSuccess, Authority.MarkerResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeclineMarker:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeclineMarkerRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeclineMarker(Intent.Payload.Get<FMatchPlayAuthoritativeDeclineMarkerRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeclineResult.bSuccess, Authority.DeclineResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeclineRunner:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeclineRunnerRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeclineRunner(Intent.Payload.Get<FMatchPlayAuthoritativeDeclineRunnerRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeclineResult.bSuccess, Authority.DeclineResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeclineHelper:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeclineHelperRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeclineHelper(Intent.Payload.Get<FMatchPlayAuthoritativeDeclineHelperRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeclineResult.bSuccess, Authority.DeclineResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::DeclineSkill:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeclineSkillRequest>()) { return Mismatch(); }
		const auto Authority = Session.DeclineSkill(Intent.Payload.Get<FMatchPlayAuthoritativeDeclineSkillRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.DeclineResult.bSuccess, Authority.DeclineResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitRunner:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitRunnerRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitRunner(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitRunnerRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.RunnerResult.bSuccess, Authority.RunnerResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitHelper:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitHelperRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitHelper(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitHelperRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.HelperResult.bSuccess, Authority.HelperResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitSkill:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitSkillRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitSkill(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitSkillRequest>());
#if WITH_DEV_AUTOMATION_TESTS
		const auto& Legality = Authority.SkillResult.LegalityResult;
		UE_LOG(LogTemp, Log, TEXT("DEV pinned Skill lookup: Skill=%s LookupSuccess=%d ActionType=%s MinTP=%d MaxTP=%d"),
			*Intent.Payload.Get<FMatchPlayAuthoritativeSubmitSkillRequest>().SkillId.ToString(),
			Legality.SkillRuleQueryResult.bSuccess,
			*StaticEnum<ESkillRuleType>()->GetNameStringByValue(static_cast<int64>(Legality.ResolvedActionType)),
			Legality.ResolvedSkillRule.MinTriggerActionPoint, Legality.ResolvedSkillRule.MaxTriggerActionPoint);
#endif

		if (!Record(Authority.RuntimeEnvelope, Authority.SkillResult.bSuccess, Authority.SkillResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitBranchIntent:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitBranchIntentRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitBranchIntent(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitBranchIntentRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.IntentResult.bSuccess, Authority.IntentResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCrossInitialRouteRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCrossInitialRouteRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCrossInitialRouteRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolvePassControlInitialRouteRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolvePassControlInitialRouteRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolvePassControlInitialRouteRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallInitialRouteRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallInitialRouteRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallInitialRouteRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCrossHighAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCrossHighAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCrossHighAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCrossHighDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCrossHighDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCrossHighDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCrossLowAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCrossLowAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCrossLowAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCrossLowDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCrossLowDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCrossLowDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolvePassControlAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolvePassControlAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolvePassControlAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolvePassControlAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolvePassControlDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolvePassControlDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolvePassControlDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallFeetAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallFeetAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallFeetDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallFeetDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallFeetDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1AttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallBehindDefenseP1AttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1AttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallBehindDefenseP1DefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallBehindDefenseP1DefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallBehindDefenseP1DefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallAntiOffsideAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallAntiOffsideAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallAntiOffsideAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallOneOnOneDirectShotAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneDirectShotDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallOneOnOneDirectShotDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallOneOnOneDirectShotDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveThroughBallOneOnOneChipShotAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveThroughBallOneOnOneChipShotAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveThroughBallOneOnOneChipShotAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::SubmitThroughBallOneOnOneShotChoice:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest>()) { return Mismatch(); }
		const auto Authority = Session.SubmitThroughBallOneOnOneShotChoice(Intent.Payload.Get<FMatchPlayAuthoritativeSubmitThroughBallOneOnOneShotChoiceRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.ChoiceResult.bSuccess, Authority.ChoiceResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveLongShotDirectAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveLongShotDirectAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveLongShotDirectDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveLongShotDirectDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveLongShotDirectDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveLongShotDeadCornerRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveLongShotDeadCornerRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveLongShotDeadCornerRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectAttackRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCutInsideShotDirectAttackRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCutInsideShotDirectAttackRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDirectDefenseRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCutInsideShotDirectDefenseRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCutInsideShotDirectDefenseRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::ResolveCutInsideShotDeadCornerRoll:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest>()) { return Mismatch(); }
		const auto Authority = Session.ResolveCutInsideShotDeadCornerRoll(Intent.Payload.Get<FMatchPlayAuthoritativeResolveCutInsideShotDeadCornerRollRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.OrchestrationResult.bSuccess, Authority.OrchestrationResult.ErrorMessage)) { return Result; }
		break;
	}
	case EMatchPlayAuthoritativeCommandKind::AdvanceAfterTerminal:
	{
		if (!Intent.Payload.IsType<FMatchPlayAuthoritativeAdvanceAfterTerminalRequest>()) { return Mismatch(); }
		const auto Authority = Session.AdvanceAfterTerminal(Intent.Payload.Get<FMatchPlayAuthoritativeAdvanceAfterTerminalRequest>());
		if (!Record(Authority.RuntimeEnvelope, Authority.CompletionResult.bSuccess, Authority.CompletionResult.ErrorMessage)) { return Result; }
		break;
	}
	default:
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent;
		return Result;
	}
	// Exactly one pass after any successful deployment, participant, Skill, branch, initial-route, ordinary contest or terminal advance command; never on rejection.
#if !UE_BUILD_SHIPPING
	FMCodexHandoffAudit::AuthorityAccepted();
#endif
	Result.CoordinatorResult = Coordinator.AdvanceToStableState();
	Result.bSuccess = Result.CoordinatorResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::ServerCoordinatorFailed;
		Result.ErrorMessage = Result.CoordinatorResult.ErrorMessage;
	}
	return Result;
}
