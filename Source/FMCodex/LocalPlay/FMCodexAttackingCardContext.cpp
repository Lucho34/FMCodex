#include "FMCodexAttackingCardContext.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexPlayerUIPresentationText.h"

FFMCodexAttackingCardContext FFMCodexAttackingCardContext::Build(const FFMCodexUMGCardViewModel& Card)
{
	FFMCodexAttackingCardContext Result;
	if (Card.bGoalkeeper) return Result;
	// Exact stable rule identity -> semantic type only. TP matching is already
	// projected by authority; static catalog values never replace those facts.
	static const TMap<FName, ESkillRuleType> Types = []()
	{
		TMap<FName, ESkillRuleType> Map;
		for (const auto& Player : FFMCodexPrototypeTeamContent::GetDefinitions())
			for (const auto& Skill : Player.SkillAssignments) Map.Add(Skill.RuleId, Skill.SkillType);
		return Map;
	}();
	for (const auto* Matches : { &Card.HandMicroVisibleTacticalSkills, &Card.PitchMiniVisibleTacticalSkills })
		for (const auto& Skill : *Matches)
		{
			Result.Skills.Add(Skill.SkillId);
			const auto* Type = Types.Find(Skill.SkillId);
			if (!Type) continue;
			FName Trait;
			switch (*Type)
			{
			case ESkillRuleType::LongShot:
				Result.Attributes.Add(TEXT("SHO")); Trait = TEXT("Trait.LongShotCarrier"); break;
			case ESkillRuleType::CutInsideShot:
				Result.Attributes.Add(TEXT("SHO")); Result.Attributes.Add(TEXT("CON"));
				Trait = TEXT("Trait.CutInsideCarrier"); break;
			case ESkillRuleType::Cross:
				Result.Attributes.Add(TEXT("PAS")); Trait = TEXT("Trait.CrossCarrier"); break;
			case ESkillRuleType::ThroughBall:
				Result.Attributes.Add(TEXT("PAS")); Trait = TEXT("Trait.ThroughBallCarrier"); break;
			default: break;
			}
			if (!Trait.IsNone() && Card.RankedTraits.ContainsByPredicate([&](const auto& Owned)
				{ return Owned.TraitId == Trait && Owned.Rank != EPlayerTraitRank::None; }))
				Result.Traits.Add(Trait);
		}
	return Result;
}

FText FFMCodexAttackingCardContext::SkillNames(const TArray<FFMCodexUMGSkillViewModel>& Skills)
{
	TArray<FText> Labels;
	for (const auto& Skill : Skills)
		Labels.Add(FFMCodexPlayerUIPresentationText::Skill(Skill.CanonicalLabel));
	return FText::Join(FText::FromString(TEXT("·")), Labels);
}
