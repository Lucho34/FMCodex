#pragma once
#include "../CoreRules/PlayerTraitFormula.h"
#include "FMCodexPlayerUIPresentationText.h"

// Formatting only: all values, activation and selection come from safe facts.
namespace FMCodexTraitFormulaPresentation
{
inline FString Operand(const FPlayerTraitFormulaOperand& Fact, float Coefficient = 1.0f)
{
 if (!Fact.bValid) return NSLOCTEXT("FMCodexTraits", "Unavailable", "公式数据不可用").ToString();
 const FString Attribute = FFMCodexPlayerUIPresentationText::ResolutionAttribute(Fact.Attribute).ToString();
 FString Text = FString::Printf(TEXT("%s %d"), *Attribute, Fact.BaseValue);
 if (Fact.Bonus > 0) Text += FString::Printf(TEXT(" +%d"), Fact.Bonus);
 if (!FMath::IsNearlyEqual(Coefficient, 1.f))
  Text = FString::Printf(TEXT("(%s) ×%s"), *Text, *FText::AsNumber(Coefficient).ToString());
 if (Fact.Bonus > 0)
 {
  const TCHAR* Rank = Fact.Rank == EPlayerTraitRank::S ? TEXT("S") : Fact.Rank == EPlayerTraitRank::A ? TEXT("A") : TEXT("B");
  Text += FString::Printf(TEXT(" — %s %s"), *FPlayerTraitFormula::DisplayName(Fact.TraitId).ToString(), Rank);
 }
 return Text;
}
}
