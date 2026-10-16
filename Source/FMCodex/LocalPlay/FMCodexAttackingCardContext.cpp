#include "FMCodexAttackingCardContext.h"
#include "FMCodexPrototypeTeamContent.h"
#include "FMCodexPlayerUIPresentationText.h"

ESkillRuleType FFMCodexAttackingCardContext::SkillType(FName RuleId)
{
	// Exact stable rule identity -> semantic type only. TP matching is already
	// projected by authority; static catalog values never replace those facts.
	static const TMap<FName, ESkillRuleType> Types = []()
	{
		TMap<FName, ESkillRuleType> Map;
		for (const auto& Player : FFMCodexPrototypeTeamContent::GetDefinitions())
			for (const auto& Skill : Player.SkillAssignments) Map.Add(Skill.RuleId, Skill.SkillType);
		return Map;
	}();
	const auto* Type = Types.Find(RuleId);
	return Type ? *Type : ESkillRuleType::None;
}

FFMCodexAttackingCardContext FFMCodexAttackingCardContext::Build(const FFMCodexUMGCardViewModel& Card)
{
	FFMCodexAttackingCardContext Result;
	if (Card.bGoalkeeper) return Result;
	for (const auto* Matches : { &Card.HandMicroVisibleTacticalSkills, &Card.PitchMiniVisibleTacticalSkills })
		for (const auto& Skill : *Matches)
		{
			Result.Skills.Add(Skill.SkillId);
			FName Trait;
			switch (SkillType(Skill.SkillId))
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

FFMCodexAttackingCardContext FFMCodexAttackingCardContext::BuildRunner(
	const FFMCodexUMGMatchScreenViewModel& View, FName CardId)
{
	FFMCodexAttackingCardContext Result;
	using C = EFMCodexUMGInteractionCategory;
	using R = EFMCodexUMGSelectedRole;
	using T = EFMCodexUMGThroughBallRoute;
	const auto Category = View.Interaction.Category;
	if (!View.Header.bMatchActive || !View.Header.bAttackActive || !View.Header.bHasCurrentAttacker
		|| View.Header.CurrentAttackerTacticalPoints <= 0
		|| View.Header.RouteKind != EMatchPlayCurrentAttackRouteKind::Ordinary
		|| View.FullTime.bVisible || View.Interaction.bMatchEnded || View.SetPiece.bVisible)
		return Result;
	// Explicit ordinary receiving-context allowlist also clears terminal/reset/other tactics.
	switch (Category)
	{
	case C::Deploy: case C::SelectCarrier: case C::SelectMarker: case C::SelectSkill:
	case C::SelectRunner: case C::SelectHelper: case C::SelectBranchIntent:
	case C::RollCrossRoute: case C::RollCrossAttack: case C::RollCrossDefense:
	case C::RollThroughBallInitialRoute: case C::RollThroughBallFeetAttack:
	case C::RollThroughBallFeetDefense: case C::RollThroughBallBehindDefenseAttack:
	case C::RollThroughBallBehindDefenseDefense: case C::RollThroughBallAntiOffsideAttack:
		break;
	default: return Result;
	}
	const auto& AttackRack = View.Header.bCurrentAttackerOnLeft ? View.LocalRack : View.OpponentRack;
	const auto* Cell = AttackRack.Cells.FindByPredicate([&](const auto& Entry) { return Entry.Card.CardId == CardId; });
	if (!Cell) return Result;
	const auto& Card = Cell->Card;
	const auto Potential = [Category](const FFMCodexUMGCardViewModel& P)
	{
		return !P.bGoalkeeper && !P.bUsed && (P.bAvailable || P.bDeployed)
			&& (Category == C::Deploy || P.bDeployed);
	};
	if (!Potential(Card)) return Result;
	FName Carrier, Runner;
	for (const auto& Region : View.PitchRegions) for (const auto& Slot : Region.Slots)
	{
		if (!Slot.bOccupied) continue;
		if (Slot.Card.SelectedRole == R::Carrier) Carrier = Slot.Card.CardId;
		if (Slot.Card.SelectedRole == R::Runner) Runner = Slot.Card.CardId;
	}
	if (CardId == Carrier || (!Runner.IsNone() && CardId != Runner)) return Result;
	TSet<ESkillRuleType> Families;
	for (const auto& Entry : AttackRack.Cells)
	{
		const auto& Initiator = Entry.Card;
		if (!Potential(Initiator) || Initiator.CardId == CardId
			|| (!Carrier.IsNone() && Initiator.CardId != Carrier)) continue;
		for (const auto& Skill : Initiator.EligibleTacticalSkills) Families.Add(SkillType(Skill.SkillId));
	}
	// Only stable semantic identities from existing public surfaces narrow the family.
	// SelectRunner/Helper may have no family field yet: retain the safe Carrier preview.
	const auto& Through = View.ThroughBallResolution;
	const FName Contest = View.InlineFormula.ContestId;
	if (Through.bVisible) Families = { ESkillRuleType::ThroughBall };
	else if (View.LongShotResolution.bVisible) Families = { View.LongShotResolution.SkillType };
	else if (Contest == TEXT("Cross.Setup") || Contest == TEXT("Cross.Route")
		|| Contest == TEXT("Cross.High") || Contest == TEXT("Cross.Low")) Families = { ESkillRuleType::Cross };
	const auto AddRanked = [&](const TCHAR* Id, const TCHAR* Attribute)
	{
		if (Card.RankedTraits.ContainsByPredicate([&](const auto& Owned)
			{ return Owned.TraitId == FName(Id) && Owned.Rank != EPlayerTraitRank::None; }))
		{ Result.Traits.Add(Id); Result.Attributes.Add(Attribute); }
	};
	if (Families.Contains(ESkillRuleType::Cross))
	{
		// Elective High/Low intent is not the random route. Keep both until actual contest disclosure.
		if (Contest != TEXT("Cross.Low")) AddRanked(TEXT("Trait.CrossHighRunner"), TEXT("STR"));
		if (Contest != TEXT("Cross.High")) AddRanked(TEXT("Trait.CrossLowRunner"), TEXT("SPD"));
	}
	if (Families.Contains(ESkillRuleType::ThroughBall))
	{
		const auto Route = Through.bVisible && Through.bRouteRevealComplete ? Through.Route : T::None;
		if (Through.Stage == EFMCodexUMGThroughBallStage::OneOnOneChoice
			|| Through.Stage == EFMCodexUMGThroughBallStage::OneOnOneResolution) return Result;
		if (Route == T::None || Route == T::Feet) AddRanked(TEXT("Trait.ThroughBallFeetRunner"), TEXT("CON"));
		if (Route == T::None || Route == T::BehindDefense) AddRanked(TEXT("Trait.ThroughBallBehindRunner"), TEXT("SPD"));
		if ((Route == T::None || Route == T::AntiOffside) && Card.BinaryTraits.Contains(TEXT("Trait.ThroughBallAntiRunner")))
			Result.Traits.Add(TEXT("Trait.ThroughBallAntiRunner"));
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
