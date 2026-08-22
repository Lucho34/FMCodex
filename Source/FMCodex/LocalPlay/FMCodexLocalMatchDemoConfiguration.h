#pragma once

#include "CoreMinimal.h"

#include "../CoreRules/MatchPlayOpeningInitializer.h"
#include "../CoreRules/SkillRuleSnapshot.h"

struct FMCODEX_API FFMCodexLocalMatchDemoConfiguration
{
	FMatchPlayOpeningInitializeInput OpeningInput;
	FSkillRuleSnapshotSet SkillRuleSet;
};

class FMCODEX_API FFMCodexLocalMatchDemoConfigurationFactory final
{
public:
	static FFMCodexLocalMatchDemoConfiguration Create();
};
