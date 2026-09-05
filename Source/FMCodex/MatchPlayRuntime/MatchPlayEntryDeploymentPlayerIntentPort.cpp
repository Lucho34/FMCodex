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
	if (Intent.CommandKind != EMatchPlayAuthoritativeCommandKind::DeployOrdinary)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent;
		return Result;
	}
	if (!Intent.Payload.IsType<FMatchPlayAuthoritativeDeployOrdinaryRequest>())
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch;
		Result.ErrorMessage = TEXT("PlayerIntent payload does not match its command kind.");
		return Result;
	}
	const auto AuthorityResult = Session.DeployOrdinary(Intent.Payload.Get<FMatchPlayAuthoritativeDeployOrdinaryRequest>());
	Result.AuthoritativeResult.RuntimeEnvelope = AuthorityResult.RuntimeEnvelope;
	Result.bPlayerIntentAccepted = AuthorityResult.RuntimeEnvelope.bAccepted;
	if (!AuthorityResult.RuntimeEnvelope.bAccepted || !AuthorityResult.RuntimeEnvelope.bDomainSuccess
		|| !AuthorityResult.DeploymentResult.bSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::AuthoritativeCommandRejected;
		Result.ErrorMessage = !AuthorityResult.RuntimeEnvelope.ErrorMessage.IsEmpty()
			? AuthorityResult.RuntimeEnvelope.ErrorMessage : AuthorityResult.DeploymentResult.ErrorMessage;
		return Result;
	}
	Result.CoordinatorResult = Coordinator.AdvanceToStableState();
	Result.bSuccess = Result.CoordinatorResult.bSuccess;
	if (!Result.bSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::ServerCoordinatorFailed;
		Result.ErrorMessage = Result.CoordinatorResult.ErrorMessage;
	}
	return Result;
}
