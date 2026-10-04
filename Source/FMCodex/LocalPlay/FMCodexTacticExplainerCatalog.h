#pragma once
#include "CoreMinimal.h"
#include "../CoreRules/TacticalRuleDescription.h"

// Static education only: no session, viewer, player snapshot, eligibility or outcome.
struct FFMCodexExplainerRole
{
 FText Label;
 FText Attributes;
};
// Presentation operands retain canonical coefficients without evaluating a match.
struct FFMCodexExplainerCalculationTerm
{
 FTacticalRuleDescriptionTerm Source;
 FText Role, Attribute;
 bool bMaximum = false;
};
struct FFMCodexExplainerProcedure
{
 FText Title, Action, Success, Failure;
};
struct FFMCodexExplainerRoute
{
 FName Id;
 FText Name, Memory;
 TArray<FFMCodexExplainerRole> Attack, Defense;
 TArray<FName> Traits;
 FText Precondition, Result, CalculationNote;
 TArray<FFMCodexExplainerCalculationTerm> AttackCalculation, DefenseCalculation;
 TArray<FFMCodexExplainerProcedure> Procedures;
 FTacticalRuleDescriptionBranch Rule;
 bool bProcedural = false;
 bool bMaxAttributes = false;
};
struct FFMCodexExplainerTactic
{
 FName Id;
 FText Name, Purpose;
 ESkillRuleType Skill = ESkillRuleType::None;
 bool bSetPiece = false;
 TArray<FFMCodexExplainerRoute> Routes;
};
class FMCODEX_API FFMCodexTacticExplainerCatalog
{
public:
 static const TArray<FFMCodexExplainerTactic>& Get();
 static const FFMCodexExplainerTactic* Find(FName Id);
 static FText SkillNote();
 static FText CalculationTermText(const FFMCodexExplainerCalculationTerm& Term, bool bAttack);
};
