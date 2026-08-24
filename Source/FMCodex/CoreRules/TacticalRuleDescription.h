#pragma once

#include "CoreMinimal.h"

#include "MatchPlayCurrentAttackResolutionFactProjection.h"
#include "SkillRuleSnapshot.h"

/** A static formula term used only to explain a canonical tactical rule. */
struct FMCODEX_API FTacticalRuleDescriptionTerm
{
	EMatchPlayResolutionFormulaTermKind Kind =
		EMatchPlayResolutionFormulaTermKind::None;
	EMatchPlayResolutionParticipantRole ParticipantRole =
		EMatchPlayResolutionParticipantRole::None;
	EMatchPlayResolutionFormulaAttribute Attribute =
		EMatchPlayResolutionFormulaAttribute::None;
	float Multiplier = 1.0f;
	int32 FixedModifier = 0;
	bool bOptional = false;
};

/** One authoritative roll range and its semantic, player-visible outcome key. */
struct FMCODEX_API FTacticalRuleDescriptionOutcome
{
	int32 Minimum = 0;
	int32 Maximum = 0;
	FName OutcomeId = NAME_None;
};

/**
 * A static branch description. It intentionally has no current player,
 * attribute value, Raw Roll, FinalValue or winner.
 */
struct FMCODEX_API FTacticalRuleDescriptionBranch
{
	FName BranchId = NAME_None;
	EMatchPlayResolutionRollSemantics RollSemantics =
		EMatchPlayResolutionRollSemantics::None;
	TArray<FTacticalRuleDescriptionTerm> AttackTerms;
	TArray<FTacticalRuleDescriptionTerm> DefenseTerms;
	TArray<FTacticalRuleDescriptionOutcome> Outcomes;
	int32 OutcomeRollCount = 1;
	bool bOutcomeUsesRollTotal = false;
	bool bConditional = false;
	bool bUsesTacticalPlayerAdvantage = false;
	FName SpecialRuleId = NAME_None;
};

/**
 * State-independent educational metadata for one canonical tactical family.
 * It is not a legality source and is never consumed by a resolver.
 */
struct FMCODEX_API FTacticalRuleDescription
{
	ESkillRuleType SkillType = ESkillRuleType::None;
	FName SummaryId = NAME_None;
	TArray<EMatchPlayResolutionParticipantRole> RequiredRoles;
	TArray<EMatchPlayResolutionParticipantRole> OptionalRoles;
	TArray<EMatchPlayResolutionParticipantRole> AutomaticRoles;
	TArray<FTacticalRuleDescriptionOutcome> InitialRouteOutcomes;
	FName InitialRouteRuleId = NAME_None;
	TArray<FTacticalRuleDescriptionBranch> Branches;
};

/** Read-only catalog available without Match State or a Resolution Session. */
class FMCODEX_API FTacticalRuleDescriptionCatalog final
{
public:
	static const FTacticalRuleDescription* FindBySkillType(
		ESkillRuleType SkillType);
	static const TArray<FTacticalRuleDescription>& GetAll();
};

