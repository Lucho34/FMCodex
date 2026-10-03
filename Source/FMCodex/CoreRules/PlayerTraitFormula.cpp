#include "PlayerTraitFormula.h"

namespace
{
using R = EMatchPlayResolutionParticipantRole;
using A = EMatchPlayResolutionFormulaAttribute;
struct FEntry { FName Id; FText Name; FName Context; FName OtherContext; R Role; A Attribute; A OtherAttribute; };
const TArray<FEntry>& Registry()
{
 static const TArray<FEntry> Entries = {
  {TEXT("Trait.LongShotCarrier"), NSLOCTEXT("FMCodexTraits", "LongShotCarrier", "远射专家"), TEXT("LongShot.DirectShot"), NAME_None, R::Carrier, A::Shooting, A::None},
  {TEXT("Trait.CutInsideCarrier"), NSLOCTEXT("FMCodexTraits", "CutInsideCarrier", "内切专家"), TEXT("CutInsideShot.DirectShot"), NAME_None, R::Carrier, A::Control, A::None},
  {TEXT("Trait.CrossCarrier"), NSLOCTEXT("FMCodexTraits", "CrossCarrier", "传中专家"), TEXT("Cross.High"), TEXT("Cross.Low"), R::Carrier, A::Passing, A::None},
  {TEXT("Trait.CrossHighRunner"), NSLOCTEXT("FMCodexTraits", "CrossHighRunner", "高球传中接应"), TEXT("Cross.High"), NAME_None, R::Runner, A::Strength, A::None},
  {TEXT("Trait.CrossLowRunner"), NSLOCTEXT("FMCodexTraits", "CrossLowRunner", "低球传中接应"), TEXT("Cross.Low"), NAME_None, R::Runner, A::Speed, A::None},
  {TEXT("Trait.ThroughBallCarrier"), NSLOCTEXT("FMCodexTraits", "ThroughBallCarrier", "直塞大师"), TEXT("ThroughBall.Feet"), TEXT("ThroughBall.BehindDefense.P1"), R::Carrier, A::Passing, A::None},
  {TEXT("Trait.ThroughBallBehindRunner"), NSLOCTEXT("FMCodexTraits", "ThroughBallBehindRunner", "身后球接应"), TEXT("ThroughBall.BehindDefense.P1"), NAME_None, R::Runner, A::Speed, A::None},
  {TEXT("Trait.ThroughBallFeetRunner"), NSLOCTEXT("FMCodexTraits", "ThroughBallFeetRunner", "脚下球接应"), TEXT("ThroughBall.Feet"), NAME_None, R::Runner, A::Control, A::None},
  {TEXT("Trait.CornerHighThreat"), NSLOCTEXT("FMCodexTraits", "CornerHighThreat", "高球角球威胁"), TEXT("Corner.High"), NAME_None, R::Runner, A::Strength, A::None},
  {TEXT("Trait.CornerLowThreat"), NSLOCTEXT("FMCodexTraits", "CornerLowThreat", "低球角球威胁"), TEXT("Corner.Low"), NAME_None, R::Runner, A::Control, A::None},
  {TEXT("Trait.NearFreeKickTaker"), NSLOCTEXT("FMCodexTraits", "NearFreeKickTaker", "近距离任意球大师"), TEXT("NearFreeKick.Direct"), NAME_None, R::Taker, A::Shooting, A::Passing},
  {TEXT("Trait.LongFreeKickTaker"), NSLOCTEXT("FMCodexTraits", "LongFreeKickTaker", "远距离任意球大师"), TEXT("LongFreeKick.Direct"), NAME_None, R::Taker, A::Shooting, A::None},
  {TEXT("Trait.PenaltyTaker"), NSLOCTEXT("FMCodexTraits", "PenaltyTaker", "点球专家"), TEXT("Penalty.Direct"), NAME_None, R::Taker, A::Shooting, A::Passing},
  {TEXT("Trait.LongShotBlocker"), NSLOCTEXT("FMCodexTraits", "LongShotBlocker", "远射封堵"), TEXT("LongShot.DirectShot"), NAME_None, R::Marker, A::Defense, A::None},
  {TEXT("Trait.CutInsideStopper"), NSLOCTEXT("FMCodexTraits", "CutInsideStopper", "内切封锁"), TEXT("CutInsideShot.DirectShot"), NAME_None, R::Marker, A::Defense, A::None},
  {TEXT("Trait.CrossMarkerBlocker"), NSLOCTEXT("FMCodexTraits", "CrossMarkerBlocker", "传中封堵"), TEXT("Cross.High"), TEXT("Cross.Low"), R::Marker, A::Defense, A::None},
  {TEXT("Trait.CrossHighHelperDefense"), NSLOCTEXT("FMCodexTraits", "CrossHighHelperDefense", "高球传中协防"), TEXT("Cross.High"), NAME_None, R::Helper, A::Strength, A::None},
  {TEXT("Trait.CrossLowHelperDefense"), NSLOCTEXT("FMCodexTraits", "CrossLowHelperDefense", "低球传中协防"), TEXT("Cross.Low"), NAME_None, R::Helper, A::Speed, A::None},
  {TEXT("Trait.ThroughBallMarkerDefense"), NSLOCTEXT("FMCodexTraits", "ThroughBallMarkerDefense", "直塞盯防"), TEXT("ThroughBall.Feet"), TEXT("ThroughBall.BehindDefense.P1"), R::Marker, A::Defense, A::None},
  {TEXT("Trait.ThroughBallFeetHelperDefense"), NSLOCTEXT("FMCodexTraits", "ThroughBallFeetHelperDefense", "脚下球协防"), TEXT("ThroughBall.Feet"), NAME_None, R::Helper, A::Defense, A::None},
  {TEXT("Trait.ThroughBallBehindHelperDefense"), NSLOCTEXT("FMCodexTraits", "ThroughBallBehindHelperDefense", "身后球协防"), TEXT("ThroughBall.BehindDefense.P1"), NAME_None, R::Helper, A::Speed, A::None},
  {TEXT("Trait.CornerHighDefense"), NSLOCTEXT("FMCodexTraits", "CornerHighDefense", "高球角球防守"), TEXT("Corner.High"), NAME_None, R::Helper, A::Strength, A::None},
  {TEXT("Trait.CornerLowDefense"), NSLOCTEXT("FMCodexTraits", "CornerLowDefense", "低球角球防守"), TEXT("Corner.Low"), NAME_None, R::Helper, A::Defense, A::None},
 };
 return Entries;
}
}

int32 FPlayerTraitFormula::RankBonus(const EPlayerTraitRank Rank)
{
 switch (Rank) { case EPlayerTraitRank::S: return 3; case EPlayerTraitRank::A: return 2; case EPlayerTraitRank::B: return 1; default: return 0; }
}

FPlayerTraitFormulaOperand FPlayerTraitFormula::Resolve(const FPlayerCardRuleSnapshot& P,
 const FName Context, const R Role, const A Attribute)
{
 FPlayerTraitFormulaOperand Out;
 Out.CardId = P.CardId; Out.Role = Role; Out.Attribute = Attribute;
 if (P.CardId.IsNone() || P.bIsGoalkeeper || !ValidatePassivePlayerTraits(P.RankedTraits, P.BinaryTraits, false)) return Out;
 switch (Attribute)
 {
 case A::Shooting: Out.BaseValue = P.Attributes.Shooting; break;
 case A::Passing: Out.BaseValue = P.Attributes.Passing; break;
 case A::Control: Out.BaseValue = P.Attributes.Control; break;
 case A::Speed: Out.BaseValue = P.Attributes.Speed; break;
 case A::Strength: Out.BaseValue = P.Attributes.Strength; break;
 case A::Defense: Out.BaseValue = P.Attributes.Defense; break;
 default: return Out;
 }
 if (Out.BaseValue < 1 || Out.BaseValue > 6) return Out;
 const FEntry* Match = nullptr;
 const FPlayerRankedTrait* Assignment = nullptr;
 for (const auto& Entry : Registry())
 {
  if (Context.IsNone() || (Entry.Context != Context && Entry.OtherContext != Context) || Entry.Role != Role) continue;
  for (const auto& Trait : P.RankedTraits)
  {
   if (Trait.TraitId != Entry.Id) continue;
   // Count matches per participant, before attribute filtering. A dual-attribute
   // entry is one match; a second matching entry is invalid, never arbitration.
   if (Match) return Out;
   Match = &Entry; Assignment = &Trait;
  }
 }
 if (Match && (Match->Attribute == Attribute || Match->OtherAttribute == Attribute))
 {
  Out.TraitId = Match->Id; Out.Rank = Assignment->Rank; Out.Bonus = RankBonus(Out.Rank);
 }
 Out.EffectiveValue = Out.BaseValue + Out.Bonus;
 Out.bValid = true;
 return Out;
}

FPlayerTraitTakerFormula FPlayerTraitFormula::ResolveTaker(const FPlayerCardRuleSnapshot& P, const FName Context)
{
 FPlayerTraitTakerFormula Out;
 if (Context != TEXT("NearFreeKick.Direct") && Context != TEXT("Penalty.Direct") && Context != TEXT("LongFreeKick.Direct")) return Out;
 Out.Operands.Add(Resolve(P, Context, R::Taker, A::Shooting));
 if (Context != TEXT("LongFreeKick.Direct")) Out.Operands.Add(Resolve(P, Context, R::Taker, A::Passing));
 for (const auto& Operand : Out.Operands)
 {
  if (!Operand.bValid) return Out;
  Out.SelectedEffectiveValue = FMath::Max(Out.SelectedEffectiveValue, Operand.EffectiveValue);
 }
 Out.bValid = true;
 return Out;
}

FText FPlayerTraitFormula::DisplayName(const FName Id)
{
 if (IsKnownBinaryPlayerTrait(Id)) return NSLOCTEXT("FMCodexTraits", "ThroughBallAntiRunner", "反越位专家");
 for (const auto& Entry : Registry()) if (Entry.Id == Id) return Entry.Name;
 return FText::GetEmpty();
}

EPlayerTraitCategory FPlayerTraitFormula::Category(const FName Id)
{
 if (IsKnownBinaryPlayerTrait(Id)) return EPlayerTraitCategory::Offensive;
 for (const auto& Entry : Registry()) if (Entry.Id == Id)
  return Entry.Role == R::Carrier || Entry.Role == R::Runner || Entry.Role == R::Taker
   ? EPlayerTraitCategory::Offensive : EPlayerTraitCategory::Defensive;
 return EPlayerTraitCategory::None;
}
