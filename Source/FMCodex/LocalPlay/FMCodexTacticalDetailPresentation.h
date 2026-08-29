#pragma once

#include "CoreMinimal.h"

#include "FMCodexLocalMatchUMGPresentation.h"

/** Maps static tactical semantics into compact Chinese player-facing DTOs. */
class FMCODEX_API FFMCodexTacticalDetailPresentationBuilder final
{
public:
	static FFMCodexUMGTacticalDetailViewModel Build(
		ESkillRuleType SkillType);

	/** Single-roll outcome metadata; arithmetic/aggregate branches stay hidden. */
	static FFMCodexUMGOutcomeRollHintViewModel BuildOutcomeRollHint(
		ESkillRuleType SkillType, FName BranchId);

	/** Compact branch copy derived from the canonical tactical description. */
	static FText BuildBranchChoiceHint(
		ESkillRuleType SkillType, FName BranchId);
};
