#pragma once

#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexLocalMatchDemoConfiguration.h"

namespace FMCodexDormantPassControlTests
{
	// Explicit capability fixture only. Never called by a production factory or a
	// running Session; callers supply this configuration before initialization.
	inline void AddCapability(FFMCodexLocalMatchDemoConfiguration& Configuration)
	{
		const FName SkillId(TEXT("Canonical.Skill.PassControl.6.8"));
		FSkillRuleSnapshot Rule;
		Rule.SkillId = SkillId;
		Rule.SkillType = ESkillRuleType::PassControl;
		Rule.MinTriggerActionPoint = 6;
		Rule.MaxTriggerActionPoint = 8;
		Configuration.SkillRuleSet.SkillRules.Add(Rule);
		for (TArray<FPlayerCardData>* Deck : {
			&Configuration.OpeningInput.OpeningInput.PlayerADeck,
			&Configuration.OpeningInput.OpeningInput.PlayerBDeck })
		{
			for (FPlayerCardData& Card : *Deck)
			{
				if (Card.CardId == TEXT("Prototype.Arsenal.MartinOdegaard")
					|| Card.CardId == TEXT("Prototype.ManchesterCity.Rodri")
					|| Card.CardId == TEXT("Prototype.ManchesterCity.BernardoSilva"))
				{
					Card.AttackSkillIds.Insert(SkillId, 0);
				}
			}
		}
	}
}
#endif
