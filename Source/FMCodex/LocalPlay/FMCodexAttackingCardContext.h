#pragma once

#include "FMCodexLocalMatchUMGPresentation.h"

/** Informational Carrier preview, never legality or an active Formula bonus. */
struct FFMCodexAttackingCardContext
{
	TSet<FName> Skills;
	TSet<FName> Attributes;
	TSet<FName> Traits;
	static FFMCodexAttackingCardContext Build(const FFMCodexUMGCardViewModel& Card);
	/** Full Card only. Consumes existing viewer-safe presentation, never gameplay state. */
	static FFMCodexAttackingCardContext BuildRunner(
		const FFMCodexUMGMatchScreenViewModel& PublicView, FName CardId);
	static FText SkillNames(const TArray<FFMCodexUMGSkillViewModel>& Skills);
	static ESkillRuleType SkillType(FName RuleId);
};
