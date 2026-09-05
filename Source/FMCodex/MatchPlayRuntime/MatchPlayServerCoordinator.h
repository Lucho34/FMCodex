#pragma once

#include "CoreMinimal.h"

#include "MatchPlayAuthoritativeSession.h"
#include "MatchPlayHostPort.h"

/**
 * Server-only orchestration. It owns no gameplay state and mutates only through
 * FMatchPlayAuthoritativeSession commands.
 */
class FMCODEX_API FMatchPlayServerCoordinator final
{
public:
	explicit FMatchPlayServerCoordinator(
		FMatchPlayAuthoritativeSession& InAuthoritativeSession,
		const FSkillRuleSnapshotSet& InSkillRuleSet);

	FMatchPlayServerCoordinatorResult AdvanceToStableState();
#if WITH_DEV_AUTOMATION_TESTS
	int32 GetInvocationCountForTests() const { return InvocationCountForTests; }
#endif
private:
#if WITH_DEV_AUTOMATION_TESTS
	int32 InvocationCountForTests = 0;
#endif
	FMatchPlayAuthoritativeSession& AuthoritativeSession;
	const FSkillRuleSnapshotSet& SkillRuleSet;
};
