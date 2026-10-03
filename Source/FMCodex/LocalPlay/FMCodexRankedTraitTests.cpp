#include "FMCodexTraitFormulaPresentation.h"
#include "FMCodexPlayerOverall.h"
#include "../CoreRules/LongShotDirectShotPlanQuery.h"
#include "../CoreRules/CutInsideShotDirectShotPlanQuery.h"
#include "../CoreRules/CrossPlanQuery.h"
#include "../CoreRules/ThroughBallFeetPlanQuery.h"
#include "../CoreRules/ThroughBallBehindDefenseP1PlanQuery.h"
#include "../CoreRules/SingleCardFormulaResolverInputAssembler.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
namespace RankedTraitTests
{
using A = EMatchPlayResolutionFormulaAttribute;
using R = EMatchPlayResolutionParticipantRole;
FPlayerCardRuleSnapshot Card(const TCHAR* Id)
{
 FPlayerCardRuleSnapshot P; P.CardId = Id; P.PositionTypes = {EPlayerPositionType::Midfield};
 P.Attributes.Shooting = P.Attributes.Passing = P.Attributes.Control = P.Attributes.Speed = P.Attributes.Strength = P.Attributes.Defense = 4;
 P.Attributes.StaminaTier = EPlayerStaminaTier::A; P.SkillIds = {TEXT("Skill.Test")}; return P;
}
void Trait(FPlayerCardRuleSnapshot& P, const TCHAR* Id, EPlayerTraitRank Rank = EPlayerTraitRank::A)
{ FPlayerRankedTrait T; T.TraitId = Id; T.Rank = Rank; P.RankedTraits.Add(T); }
FSkillRuleSnapshotSet Rules(ESkillRuleType Type)
{
 FSkillRuleSnapshot S; S.SkillId = TEXT("Skill.Test"); S.SkillType = Type; S.MinTriggerActionPoint = 3; S.MaxTriggerActionPoint = 6;
 FSkillRuleSnapshotSet Out; Out.SkillRules.Add(S); return Out;
}
FSingleCardFormulaResolverInputAssemblyResult Assemble(const FPlayerCardRuleSnapshotSet& Cards,
 const FSingleCardFormulaInputAssemblyQueryInput& Attack, const FSingleCardFormulaInputAssemblyQueryInput& Defense)
{
 FSingleCardFormulaResolverInputAssemblyInput I;
 I.AttackerQueryResult = FSingleCardFormulaInputAssemblyQuery::Assemble(Cards, Attack);
 I.DefenderQueryResult = FSingleCardFormulaInputAssemblyQuery::Assemble(Cards, Defense);
 I.AttackerPlayerId = TEXT("A"); I.DefenderPlayerId = TEXT("D");
 return FSingleCardFormulaResolverInputAssembler::Assemble(I);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRankedTraitMatrix, "FMCodex.RankedTraits.MatrixAndBoundaries", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRankedTraitMatrix::RunTest(const FString&)
{
 using namespace RankedTraitTests;
 struct FCase { const TCHAR* Id; const TCHAR* Context; R Role; A Attribute; };
 const FCase Cases[] = {
 {TEXT("LongShotCarrier"), TEXT("LongShot.DirectShot"), R::Carrier, A::Shooting},
 {TEXT("CutInsideCarrier"), TEXT("CutInsideShot.DirectShot"), R::Carrier, A::Control},
 {TEXT("CrossCarrier"), TEXT("Cross.High"), R::Carrier, A::Passing},
 {TEXT("CrossHighRunner"), TEXT("Cross.High"), R::Runner, A::Strength},
 {TEXT("CrossLowRunner"), TEXT("Cross.Low"), R::Runner, A::Speed},
 {TEXT("ThroughBallCarrier"), TEXT("ThroughBall.Feet"), R::Carrier, A::Passing},
 {TEXT("ThroughBallBehindRunner"), TEXT("ThroughBall.BehindDefense.P1"), R::Runner, A::Speed},
 {TEXT("ThroughBallFeetRunner"), TEXT("ThroughBall.Feet"), R::Runner, A::Control},
 {TEXT("CornerHighThreat"), TEXT("Corner.High"), R::Runner, A::Strength},
 {TEXT("CornerLowThreat"), TEXT("Corner.Low"), R::Runner, A::Control},
 {TEXT("NearFreeKickTaker"), TEXT("NearFreeKick.Direct"), R::Taker, A::Shooting},
 {TEXT("LongFreeKickTaker"), TEXT("LongFreeKick.Direct"), R::Taker, A::Shooting},
 {TEXT("PenaltyTaker"), TEXT("Penalty.Direct"), R::Taker, A::Shooting},
 {TEXT("LongShotBlocker"), TEXT("LongShot.DirectShot"), R::Marker, A::Defense},
 {TEXT("CutInsideStopper"), TEXT("CutInsideShot.DirectShot"), R::Marker, A::Defense},
 {TEXT("CrossMarkerBlocker"), TEXT("Cross.Low"), R::Marker, A::Defense},
 {TEXT("CrossHighHelperDefense"), TEXT("Cross.High"), R::Helper, A::Strength},
 {TEXT("CrossLowHelperDefense"), TEXT("Cross.Low"), R::Helper, A::Speed},
 {TEXT("ThroughBallMarkerDefense"), TEXT("ThroughBall.BehindDefense.P1"), R::Marker, A::Defense},
 {TEXT("ThroughBallFeetHelperDefense"), TEXT("ThroughBall.Feet"), R::Helper, A::Defense},
 {TEXT("ThroughBallBehindHelperDefense"), TEXT("ThroughBall.BehindDefense.P1"), R::Helper, A::Speed},
 {TEXT("CornerHighDefense"), TEXT("Corner.High"), R::Helper, A::Strength},
 {TEXT("CornerLowDefense"), TEXT("Corner.Low"), R::Helper, A::Defense}
 };
 TestEqual(TEXT("All ranked mappings"), static_cast<int32>(UE_ARRAY_COUNT(Cases)), 23);
 for (const auto& C : Cases)
 {
  auto P = Card(TEXT("Participant")); const FString Id = FString(TEXT("Trait.")) + C.Id; Trait(P, *Id);
  const auto Fact = FPlayerTraitFormula::Resolve(P, C.Context, C.Role, C.Attribute);
  TestTrue(Id + TEXT(" valid"), Fact.bValid); TestEqual(Id + TEXT(" bonus"), Fact.Bonus, 2);
  TestEqual(TEXT("Base preserved"), Fact.BaseValue, 4); TestEqual(TEXT("Effective before coefficient"), Fact.EffectiveValue, 6);
  TestEqual(TEXT("Coefficient order"), .5f * Fact.EffectiveValue, 3.f);
  TestEqual(TEXT("Wrong role cannot activate"), FPlayerTraitFormula::Resolve(P, C.Context, R::Goalkeeper, C.Attribute).Bonus, 0);
  for (const TCHAR* Later : {TEXT("ThroughBall.AntiOffside"), TEXT("ThroughBall.OneOnOne.DirectShot"), TEXT("ThroughBall.OneOnOne.ChipShot"), TEXT("LongShot.DeadCorner"), TEXT("LongFreeKick.Power"), TEXT("Penalty.Panenka")})
   TestEqual(TEXT("Procedural and later stages cannot inherit"), FPlayerTraitFormula::Resolve(P, Later, C.Role, C.Attribute).Bonus, 0);
  TestFalse(TEXT("Chinese attribution exists"), FPlayerTraitFormula::DisplayName(Fact.TraitId).IsEmpty());
 }
 auto P = Card(TEXT("P")); P.Attributes.Shooting = 6;
 for (auto Rank : {EPlayerTraitRank::B,EPlayerTraitRank::A,EPlayerTraitRank::S})
 {
  P.RankedTraits.Reset(); Trait(P,TEXT("Trait.LongShotCarrier"),Rank);
  auto F=FPlayerTraitFormula::Resolve(P,TEXT("LongShot.DirectShot"),R::Carrier,A::Shooting);
  const int32 Expected=Rank==EPlayerTraitRank::B?1:Rank==EPlayerTraitRank::A?2:3;
  TestEqual(TEXT("Global rank scale"),F.Bonus,Expected); TestEqual(TEXT("Effective above six is not clamped"),F.EffectiveValue,6+Expected);
 }
 TestEqual(TEXT("Base remains six"),P.Attributes.Shooting,6);
 TestEqual(TEXT("OVR uses six bases only"),FFMCodexPlayerOverall::CalculateOutfield(P.Attributes,P.Rarity).Value,85);
 TestEqual(TEXT("Stamina tier unchanged"),P.Attributes.StaminaTier,EPlayerStaminaTier::A);
 TestEqual(TEXT("Tie and Recovery mapping unchanged"),PlayerStaminaGameplayValue(P.Attributes.StaminaTier),3);
 const auto Duplicate = P.RankedTraits[0];
 P.RankedTraits.Add(Duplicate);
 TestFalse(TEXT("Duplicate configuration rejected, not stacked"),FPlayerTraitFormula::Resolve(P,TEXT("LongShot.DirectShot"),R::Carrier,A::Shooting).bValid);
 P.RankedTraits.Reset(); Trait(P,TEXT("Trait.Unknown"));
 TestFalse(TEXT("Unknown ID rejected"),ValidatePassivePlayerTraits(P.RankedTraits,{},false));
 P.RankedTraits.Reset(); Trait(P,TEXT("Trait.LongShotCarrier"),EPlayerTraitRank::None);
 TestFalse(TEXT("Rank required"),ValidatePassivePlayerTraits(P.RankedTraits,{},false));
 P.RankedTraits.Reset(); Trait(P,TEXT("Trait.ThroughBallAntiRunner"));
 TestFalse(TEXT("Binary cannot carry rank"),ValidatePassivePlayerTraits(P.RankedTraits,{},false));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRankedTraitTakerUI, "FMCodex.RankedTraits.DualOperandsAndPresentation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRankedTraitTakerUI::RunTest(const FString&)
{
 using namespace RankedTraitTests;
 auto P=Card(TEXT("Taker")); P.Attributes.Passing=5;
 for (const auto& Pair : {TPair<FName,FName>(TEXT("NearFreeKick.Direct"),TEXT("Trait.NearFreeKickTaker")),TPair<FName,FName>(TEXT("Penalty.Direct"),TEXT("Trait.PenaltyTaker"))})
 {
  P.RankedTraits.Reset(); Trait(P,*Pair.Value.ToString());
  auto F=FPlayerTraitFormula::ResolveTaker(P,Pair.Key);
  if(!TestTrue(TEXT("Dual operand fact"),F.bValid&&F.Operands.Num()==2))return false;
  TestEqual(TEXT("Both bonuses before max"),F.Operands[0].Bonus+F.Operands[1].Bonus,4);
  TestEqual(TEXT("Shooting effective"),F.Operands[0].EffectiveValue,6); TestEqual(TEXT("Passing effective"),F.Operands[1].EffectiveValue,7);
  TestEqual(TEXT("Authoritative max"),F.SelectedEffectiveValue,7);
  TestTrue(TEXT("Base and bonus separately readable"),FMCodexTraitFormulaPresentation::Operand(F.Operands[0]).Contains(TEXT("射门 4 +2")));
  TestTrue(TEXT("Both candidates readable"),FMCodexTraitFormulaPresentation::Operand(F.Operands[1]).Contains(TEXT("传球 5 +2")));
 }
 P.RankedTraits.Reset(); auto Base=FPlayerTraitFormula::Resolve(P,TEXT("LongShot.DirectShot"),R::Carrier,A::Shooting);
 TestEqual(TEXT("No Trait stays plain"),FMCodexTraitFormulaPresentation::Operand(Base),FString(TEXT("射门 4")));
 P.Attributes.Shooting=6; Trait(P,TEXT("Trait.LongShotCarrier"),EPlayerTraitRank::S);
 auto F=FPlayerTraitFormula::Resolve(P,TEXT("LongShot.DirectShot"),R::Carrier,A::Shooting);
 TestEqual(TEXT("Effective nine preserved"),F.EffectiveValue,9);
 TestEqual(TEXT("Visible attribution"),FMCodexTraitFormulaPresentation::Operand(F),FString(TEXT("射门 6 +3 — 远射专家 S")));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRankedTraitPlanIntegration, "FMCodex.RankedTraits.PlanIntegration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRankedTraitPlanIntegration::RunTest(const FString&)
{
 using namespace RankedTraitTests;
 FPlayerCardRuleSnapshotSet Cards; Cards.Cards={Card(TEXT("C")),Card(TEXT("R")),Card(TEXT("M")),Card(TEXT("H"))};
 auto& C=Cards.Cards[0];auto& Rn=Cards.Cards[1];auto& M=Cards.Cards[2];auto& H=Cards.Cards[3];
 Trait(C,TEXT("Trait.LongShotCarrier"));Trait(M,TEXT("Trait.LongShotBlocker"));
 FLongShotDirectShotPlanQueryInput L;L.SkillId=TEXT("Skill.Test");L.AttackerCardId=C.CardId;L.DefenderCardId=M.CardId;L.CurrentActionPoint=4;
 L.bHasExternalAttackD6=L.bHasExternalDefenseD6=true;L.ExternalAttackD6=4;L.ExternalDefenseD6=3;L.LogId=FGuid(1,2,3,4);L.AttackerPlayerId=TEXT("A");L.DefenderPlayerId=TEXT("D");
 auto Long=FLongShotDirectShotPlanQuery::BuildPlan(Cards,Rules(ESkillRuleType::LongShot),L);
 if(!TestTrue(TEXT("LongShot plan"),Long.bSuccess))return false;
 auto LongInput=Assemble(Cards,Long.FormulaPlan.AttackerQueryInput,Long.FormulaPlan.DefenderQueryInput);
 TestTrue(TEXT("LongShot assembly"),LongInput.bSuccess);TestEqual(TEXT("Shooting enhanced"),LongInput.ResolverInput.Attacker.BaseValue,6.f);
 TestEqual(TEXT("Defense enhanced"),LongInput.ResolverInput.Defender.BaseValue,6.f);TestEqual(TEXT("LongShot fixed +3"),LongInput.ResolverInput.Defender.Modifier,3.f);
 C.RankedTraits.Reset();M.RankedTraits.Reset();Trait(C,TEXT("Trait.CutInsideCarrier"));Trait(M,TEXT("Trait.CutInsideStopper"));
 FCutInsideShotDirectShotPlanQueryInput I;I.SkillId=L.SkillId;I.AttackerCardId=L.AttackerCardId;I.DefenderCardId=L.DefenderCardId;I.CurrentActionPoint=4;
 I.bHasExternalAttackD6=I.bHasExternalDefenseD6=true;I.ExternalAttackD6=4;I.ExternalDefenseD6=3;I.LogId=L.LogId;I.AttackerPlayerId=L.AttackerPlayerId;I.DefenderPlayerId=L.DefenderPlayerId;
 auto Cut=FCutInsideShotDirectShotPlanQuery::BuildPlan(Cards,Rules(ESkillRuleType::CutInsideShot),I);
 if(!TestTrue(TEXT("CutInside plan"),Cut.bSuccess))return false;
 auto CutInput=Assemble(Cards,Cut.FormulaPlan.AttackerQueryInput,Cut.FormulaPlan.DefenderQueryInput);
 TestTrue(TEXT("CutInside assembly"),CutInput.bSuccess);
 TestEqual(TEXT("Control only: average 4 and 6"),CutInput.ResolverInput.Attacker.BaseValue+CutInput.ResolverInput.Attacker.Modifier,5.f);
 TestEqual(TEXT("Defense only: average 6 and 4 plus 2"),CutInput.ResolverInput.Defender.BaseValue+CutInput.ResolverInput.Defender.Modifier,7.f);
 for(bool High:{true,false})
 {
  for(auto& P:Cards.Cards)P.RankedTraits.Reset();
  Trait(C,TEXT("Trait.CrossCarrier"));Trait(Rn,High?TEXT("Trait.CrossHighRunner"):TEXT("Trait.CrossLowRunner"),EPlayerTraitRank::B);
  Trait(M,TEXT("Trait.CrossMarkerBlocker"),EPlayerTraitRank::B);Trait(H,High?TEXT("Trait.CrossHighHelperDefense"):TEXT("Trait.CrossLowHelperDefense"));
  FCrossPlanQueryInput X;X.SkillId=L.SkillId;X.ActualCrossType=High?ECrossPlanActualType::High:ECrossPlanActualType::Low;
  X.CarrierCardId=C.CardId;X.RunnerCardId=Rn.CardId;X.MarkerCardId=M.CardId;X.HelperCardId=H.CardId;
  X.CarrierPlayerId=TEXT("CP");X.RunnerPlayerId=TEXT("RP");X.MarkerPlayerId=TEXT("MP");X.HelperPlayerId=TEXT("HP");
  X.bRunnerInAttackingForwardArea=X.bHasHelper=true;X.CurrentActionPoint=4;X.AttackD6=4;X.DefenseD6=3;X.LogId=L.LogId;
  auto Cross=FCrossPlanQuery::BuildPlan(Cards,Rules(ESkillRuleType::Cross),X);
  if(!TestTrue(TEXT("Cross plan"),Cross.bSuccess))return false;
  auto CI=Assemble(Cards,Cross.FormulaPlan.AttackerQueryInput,Cross.FormulaPlan.DefenderQueryInput);
  TestTrue(TEXT("Cross assembly"),CI.bSuccess);
  TestEqual(TEXT("Carrier and Runner simultaneous"),CI.ResolverInput.Attacker.BaseValue+CI.ResolverInput.Attacker.Modifier,5.5f);
  TestEqual(TEXT("Marker and Helper simultaneous"),CI.ResolverInput.Defender.BaseValue+CI.ResolverInput.Defender.Modifier,7.5f);
 }
 for(bool Feet:{true,false})
 {
  for(auto& P:Cards.Cards)P.RankedTraits.Reset();
  Trait(C,TEXT("Trait.ThroughBallCarrier"));Trait(Rn,Feet?TEXT("Trait.ThroughBallFeetRunner"):TEXT("Trait.ThroughBallBehindRunner"),EPlayerTraitRank::B);
  Trait(M,TEXT("Trait.ThroughBallMarkerDefense"),EPlayerTraitRank::B);Trait(H,Feet?TEXT("Trait.ThroughBallFeetHelperDefense"):TEXT("Trait.ThroughBallBehindHelperDefense"));
  FThroughBallParticipantEligibilityQueryInput E;E.SelectedSkillId=L.SkillId;E.CurrentActionPoint=4;E.AttackingOwnerId=TEXT("A");E.DefendingOwnerId=TEXT("D");
  E.CarrierSnapshot=C;E.RunnerSnapshot=Rn;E.MarkerSnapshot=M;E.HelperSnapshot=H;E.bHasHelper=E.bIsRunnerInAttackingForwardArea=true;
  auto Eligible=FThroughBallParticipantEligibilityQuery::Evaluate(Rules(ESkillRuleType::ThroughBall),E);
  if(!TestTrue(TEXT("ThroughBall eligibility"),Eligible.bSuccess))return false;
  if(Feet)
  {
   FThroughBallFeetPlanQueryInput F;F.ParticipantEligibilityResult=Eligible;F.AttackD6=4;F.DefenseD6=3;F.LogId=L.LogId;
   auto Q=FThroughBallFeetPlanQuery::Evaluate(F);TestTrue(TEXT("Feet plan"),Q.bSuccess);
   TestEqual(TEXT("Feet attack effective average"),Q.FormulaPlan.AttackBaseValue,5.5f);TestEqual(TEXT("Feet defense effective average"),Q.FormulaPlan.DefenseBaseValue,5.5f);
  }
  else
  {
   FThroughBallBehindDefenseP1PlanQueryInput F;F.ParticipantEligibilityResult=Eligible;F.SelectedBranch=EThroughBallSelectedBranch::BehindDefense;
   F.bHasAttackD6=F.bHasDefenseD6=true;F.AttackD6=4;F.DefenseD6=3;F.LogId=L.LogId;
   auto Q=FThroughBallBehindDefenseP1PlanQuery::Evaluate(F);TestTrue(TEXT("Behind plan"),Q.bSuccess);
   TestEqual(TEXT("Behind attack effective average"),Q.FormulaPlan.AttackBaseValue,5.5f);TestEqual(TEXT("Behind defense effective average"),Q.FormulaPlan.DefenseBaseValue,5.5f);
   F.AttackD6=1;Q=FThroughBallBehindDefenseP1PlanQuery::Evaluate(F);TestTrue(TEXT("Traits do not rescue out of play"),Q.bSuccess&&!Q.bHasFormulaPlan&&Q.bAttackEnded);
  }
 }
 return true;
}
#endif
