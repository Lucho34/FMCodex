#pragma once

#include "FMCodexLocalMatchUMGPresentation.h"

/** Informational Carrier preview, never legality or an active Formula bonus. */
struct FFMCodexAttackingCardContext
{
	TSet<FName> Skills;
	TSet<FName> Attributes;
	TSet<FName> Traits;
	static FFMCodexAttackingCardContext Build(const FFMCodexUMGCardViewModel& Card);
	static FText SkillNames(const TArray<FFMCodexUMGSkillViewModel>& Skills);
};
