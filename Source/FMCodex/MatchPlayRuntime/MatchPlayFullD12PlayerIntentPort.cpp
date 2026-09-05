#include "MatchPlayFullD12PlayerIntentPort.h"
#include "MatchPlayAuthoritativeSession.h"
#include "MatchPlayServerCoordinator.h"

FMatchPlayFullD12PlayerIntentPort::FMatchPlayFullD12PlayerIntentPort(
	FMatchPlayAuthoritativeSession& InSession, FMatchPlayServerCoordinator& InCoordinator)
	: Session(InSession), Coordinator(InCoordinator)
{
}

FMatchPlayPlayerIntentSubmissionResult FMatchPlayFullD12PlayerIntentPort::SubmitPlayerIntent(
	const FMatchPlayPlayerIntent& Intent)
{
	FMatchPlayPlayerIntentSubmissionResult Result;
	if (Intent.CommandKind != EMatchPlayAuthoritativeCommandKind::RequestInitialActionPointRoll)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::NotPlayerIntent;
		return Result;
	}
	if (!Intent.Payload.IsType<FMatchPlayFullD12EntryRequest>())
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::PayloadTypeMismatch;
		return Result;
	}
	const auto AuthorityResult = Session.RequestInitialActionPointRoll(
		Intent.Payload.Get<FMatchPlayFullD12EntryRequest>());
	Result.AuthoritativeResult.RuntimeEnvelope = AuthorityResult.RuntimeEnvelope;
	Result.bPlayerIntentAccepted = AuthorityResult.RuntimeEnvelope.bAccepted;
	if (!AuthorityResult.RuntimeEnvelope.bAccepted || !AuthorityResult.RuntimeEnvelope.bDomainSuccess)
	{
		Result.ErrorCode = EMatchPlayPlayerIntentPortErrorCode::AuthoritativeCommandRejected;
		Result.ErrorMessage = AuthorityResult.RuntimeEnvelope.ErrorMessage;
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
