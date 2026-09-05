#pragma once
#include "MatchPlayHostPort.h"
class FMatchPlayAuthoritativeSession;
class FMatchPlayServerCoordinator;

/** Bounded shared entry/deployment slice, used by both LocalPlay and NetworkPlay. */
class FMCODEX_API FMatchPlayEntryDeploymentPlayerIntentPort final : public IMatchPlayPlayerIntentPort
{
public:
	FMatchPlayEntryDeploymentPlayerIntentPort(FMatchPlayAuthoritativeSession& InSession,
		FMatchPlayServerCoordinator& InCoordinator) : Session(InSession), Coordinator(InCoordinator) {}
	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(const FMatchPlayPlayerIntent& Intent) override;
private:
	FMatchPlayAuthoritativeSession& Session;
	FMatchPlayServerCoordinator& Coordinator;
};
