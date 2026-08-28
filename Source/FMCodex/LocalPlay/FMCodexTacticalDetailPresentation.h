#pragma once

#include "CoreMinimal.h"

#include "FMCodexLocalMatchUMGPresentation.h"

/** Maps static tactical semantics into compact Chinese player-facing DTOs. */
class FMCODEX_API FFMCodexTacticalDetailPresentationBuilder final
{
public:
	static FFMCodexUMGTacticalDetailViewModel Build(
		ESkillRuleType SkillType);

	/** One canonical OneOnOne branch, projected for pre-choice hover. */
	static FFMCodexUMGTacticalDetailViewModel BuildOneOnOneChoice(
		EFMCodexUMGOneOnOneChoice Choice);

	/** Single-roll outcome metadata; arithmetic/aggregate branches stay hidden. */
	static FFMCodexUMGOutcomeRollHintViewModel BuildOutcomeRollHint(
		ESkillRuleType SkillType, FName BranchId);
};
