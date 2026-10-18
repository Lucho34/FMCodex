#pragma once

#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexPlayerUIPresentationText.h"

// Read-only formatting of the CURRENT displayed row. No rule evaluation,
// arithmetic, Trait activation, string parsing or result disclosure lives here.
namespace FMCodexFormulaExplanation
{
struct FLine
{
 FString Label, Value;
 bool bEmphasis=false;
};

inline FString Context(const FFMCodexUMGInlineFormulaSurfaceViewModel& P)
{
 if(P.ContestId==TEXT("LongShot.DirectShot"))return NSLOCTEXT("FormulaExplanation","Long","远射直接射门").ToString();
 if(P.ContestId==TEXT("ThroughBall.Feet"))return NSLOCTEXT("FormulaExplanation","Feet","直塞 · 脚下球").ToString();
 if(P.ContestId==TEXT("ThroughBall.BehindDefense.P1"))return NSLOCTEXT("FormulaExplanation","Behind","直塞 · 身后球对抗").ToString();
 if(P.ContestId==TEXT("ThroughBall.OneOnOne.DirectShot"))return NSLOCTEXT("FormulaExplanation","OneOnOne","单刀 · 直接射门").ToString();
 const auto Label=FFMCodexPlayerUIPresentationText::ResolutionContest(P.ContestId);
 return Label.IsEmpty()?P.ResolutionContextLabel:Label.ToString();
}

inline TArray<FLine> Build(const FFMCodexUMGInlineFormulaRowViewModel& Row,const FString& ContextLabel)
{
 using K=EFMCodexUMGInlineFormulaTermKind;
 TArray<FLine> Lines;
 TArray<FString> Names;
 for(const auto& P:Row.Participants)Names.AddUnique(P.PlayerName);
 Lines.Add({FText::Format(NSLOCTEXT("FMCodexTheater","CutBaseHeader","{0} · 基础值计算"),FText::FromString(Row.SideLabel)).ToString(),{},true});
 Lines.Add({ContextLabel+(Names.IsEmpty()?FString():TEXT(" · ")+FString::Join(Names,TEXT("、")))});
 for(const auto& Term:Row.Terms)
 {
  if(!Term.bResolved || Term.Kind==K::RawRoll)continue;
  // Legacy set-piece terms have accurate display expressions but do not all
  // carry per-term numeric contributions. Never treat their default zero as a fact.
  if(Term.Kind==K::FixedModifier)
  {
   if(Term.ModifierSourceLabel.IsEmpty())Lines.Add({Term.DisplayLabel,{},true});
   else Lines.Add({Term.ModifierSourceLabel,FString(Term.Contribution>=0?TEXT("+"):TEXT(""))+FText::AsNumber(Term.Contribution).ToString(),true});
   continue;
  }
  const bool Structured=!Term.AttributeLabel.IsEmpty() && Term.AttributeOperands.Num()<=1;
  if(Structured)Lines.Add({Term.ContributorDisplayName.IsEmpty()?Term.AttributeLabel:Term.ContributorDisplayName+TEXT(" · ")+Term.AttributeLabel,{},true});
  FString Input=FText::AsNumber(Term.SourceValue).ToString();
  for(const auto& Op:Term.AttributeOperands)
  {
   if(!Op.bValid)continue;
   const FString Attribute=FFMCodexPlayerUIPresentationText::ResolutionAttribute(Op.Attribute).ToString();
   if(!Structured)
   {
    const auto* Person=Row.Participants.FindByPredicate([&](const auto& P){return P.CardId==Op.CardId && !Op.CardId.IsNone();});
    const FString Identity=Person?Person->PlayerName:FFMCodexPlayerUIPresentationText::ResolutionParticipantRole(Op.Role).ToString();
    Lines.Add({Identity.IsEmpty()?Attribute:Identity+TEXT(" · ")+Attribute,{},true});
   }
   Input=FText::AsNumber(Op.BaseValue).ToString();
   Lines.Add({FText::Format(NSLOCTEXT("FMCodexTheater","CutBaseOriginal","原始{0}：{1}"),FText::FromString(Attribute),FText::AsNumber(Op.BaseValue)).ToString()});
   if(Op.Bonus!=0)
   {
    const FString Rank=Op.Rank==EPlayerTraitRank::S?TEXT("S"):Op.Rank==EPlayerTraitRank::A?TEXT("A"):Op.Rank==EPlayerTraitRank::B?TEXT("B"):TEXT("");
    const FString Signed=FString(Op.Bonus>=0?TEXT("+"):TEXT(""))+FText::AsNumber(Op.Bonus).ToString();
    Lines.Add({FText::Format(NSLOCTEXT("FMCodexTheater","CutBaseTrait","{0} {1}：{2} {3} → {4}"),FPlayerTraitFormula::DisplayName(Op.TraitId),
     FText::FromString(Rank),FText::FromString(Attribute),FText::FromString(Signed),FText::AsNumber(Op.EffectiveValue)).ToString()});
    Input=TEXT("(")+Input+TEXT(" ")+Signed+TEXT(")");
   }
  }
  if(Structured)
  {
   // SourceValue without a Trait operand is already an authoritative input;
   // its original pre-Trait value is not inferred.
   if(Term.AttributeOperands.IsEmpty())Lines.Add({FText::Format(NSLOCTEXT("FormulaExplanation","Input","属性值：{0}"),FText::AsNumber(Term.SourceValue)).ToString()});
   Lines.Add({FText::Format(NSLOCTEXT("FMCodexTheater","CutBaseWeighted","{0}贡献：{1} × {2}"),FText::FromString(Term.AttributeLabel),
    FText::FromString(Input),FText::AsNumber(Term.Multiplier)).ToString(),TEXT("= ")+FText::AsNumber(Term.Contribution).ToString()});
  }
  else if(!Term.DisplayLabel.IsEmpty())Lines.Add({Term.DisplayLabel});
 }
 Lines.Add({NSLOCTEXT("FMCodexTheater","CutBaseFinal","最终基础值").ToString(),Row.bKnownNonRollSubtotalResolved?FText::AsNumber(Row.KnownNonRollSubtotal).ToString():TEXT("?"),true});
 return Lines;
}
}
