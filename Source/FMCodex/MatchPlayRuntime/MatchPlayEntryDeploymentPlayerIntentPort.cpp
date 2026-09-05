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
	default:
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent;
		return Result;
	}
	// Exactly one pass after any successful deployment or Carrier command; never on rejection.
	Result.CoordinatorResult = Coordinator.AdvanceToStableState();
	Result.bSuccess = Result.CoordinatorResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::ServerCoordinatorFailed;
		Result.ErrorMessage = Result.CoordinatorResult.ErrorMessage;
	}
	return Result;
}
