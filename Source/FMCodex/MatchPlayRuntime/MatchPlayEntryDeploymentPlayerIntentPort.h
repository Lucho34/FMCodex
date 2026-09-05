#pragma once
#include "MatchPlayHostPort.h"
class FMatchPlayAuthoritativeSession;
class FMatchPlayServerCoordinator;

/** Bounded shared entry/deployment/Carrier/Marker/Runner/Helper/Skill/branch/initial-route/Cross-contest slice for LocalPlay and NetworkPlay; no other contest or decline dispatch. */
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
