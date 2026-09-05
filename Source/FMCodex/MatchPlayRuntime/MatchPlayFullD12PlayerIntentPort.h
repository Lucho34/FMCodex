#pragma once

#include "MatchPlayHostPort.h"

class FMatchPlayAuthoritativeSession;
class FMatchPlayServerCoordinator;

/** Shared Full D12 dispatch for the LocalPlay and NetworkPlay host adapters. */
class FMCODEX_API FMatchPlayFullD12PlayerIntentPort final : public IMatchPlayPlayerIntentPort
{
public:
	FMatchPlayFullD12PlayerIntentPort(FMatchPlayAuthoritativeSession& InSession,
		FMatchPlayServerCoordinator& InCoordinator);
	virtual FMatchPlayPlayerIntentSubmissionResult SubmitPlayerIntent(
		const FMatchPlayPlayerIntent& Intent) override;
private:
	FMatchPlayAuthoritativeSession& Session;
	FMatchPlayServerCoordinator& Coordinator;
};
