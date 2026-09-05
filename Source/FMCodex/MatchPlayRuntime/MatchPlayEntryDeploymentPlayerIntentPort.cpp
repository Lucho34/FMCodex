#include "MatchPlayEntryDeploymentPlayerIntentPort.h"
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
	default:
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent;
		return Result;
	}
	// Exactly one pass after any successful deployment, participant, Skill, branch, initial-route or Cross contest command; never on rejection.
	Result.CoordinatorResult = Coordinator.AdvanceToStableState();
	Result.bSuccess = Result.CoordinatorResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::ServerCoordinatorFailed;
		Result.ErrorMessage = Result.CoordinatorResult.ErrorMessage;
	}
	return Result;
}
