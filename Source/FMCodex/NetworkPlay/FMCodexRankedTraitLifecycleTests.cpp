#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexNetworkThroughBallConditionalTestFixture.h"
#include "FMCodexNetworkMatchPresentation.h"
using namespace FMCodexThroughBallConditionalTests;
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRankedTraitLifecycle, "FMCodex.RankedTraits.ThroughBall.LifecycleAndSafeDTO", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRankedTraitLifecycle::RunTest(const FString&)
{
 for(bool Anti:{false,true})for(bool Chip:{false,true})
 {
  FConditionalFixture F;
  if(!TestTrue(TEXT("Canonical production ThroughBall route"),F.Route(Anti?5:3)))return false;
  if(!Anti)
  {
   const auto Safe=F.Safe(0);bool Active=false;
   for(const auto& Contest:Safe.ResolutionFacts.FormulaContests)
    for(const auto& Term:Contest.AttackRow.Terms)
     Active|=Term.AttributeOperand.TraitId==TEXT("Trait.ThroughBallCarrier")&&Term.AttributeOperand.Bonus>0;
   if(!TestTrue(TEXT("Production Carrier Trait active in P1"),Active))return false;
   const auto Model=FFMCodexNetworkMatchPresentationAdapter::Project(Safe,Side::PlayerA);
   TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
   auto Copy=Model;FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Save,&Copy,nullptr);
   FFMCodexNetworkMatchPresentation Restored;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,false);
   FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Load,&Restored,nullptr);
   bool RestoredBonus=false;
   for(const auto& Term:Restored.ThroughBallSurface.Formula.AttackRow.Terms)
    for(const auto& Operand:Term.AttributeOperands)RestoredBonus|=Operand.Bonus>0&&Operand.EffectiveValue>Operand.BaseValue;
   TestTrue(TEXT("Safe replicated struct retains base/bonus/effective"),RestoredBonus);
   TestEqual(TEXT("P1 attack"),F.Roll(Kind::ThroughBallBehindDefenseP1AttackRoll,6).Code,Code::Accepted);
   TestEqual(TEXT("P1 defense"),F.Roll(Kind::ThroughBallBehindDefenseP1DefenseRoll,1).Code,Code::Accepted);
  }
  else
  {
   const int32 Before=F.Entropy->Calls;
   TestEqual(TEXT("Anti action"),F.Roll(Kind::ThroughBallAntiOffsideAttackRoll,6).Code,Code::Accepted);
   TestEqual(TEXT("Anti remains exactly one die"),F.Entropy->Calls,Before+1);
   TestTrue(TEXT("Anti has no ranked Formula"),F.Safe(1).ResolutionFacts.FormulaContests.IsEmpty());
  }
  TestEqual(TEXT("OneOnOne choice"),F.Roll(Kind::SubmitThroughBallOneOnOneShotChoice,6,Chip?Shot::ChipShot:Shot::DirectShot).Code,Code::Accepted);
  if(Chip)
   TestEqual(TEXT("Chip remains procedural"),F.Roll(Kind::ThroughBallOneOnOneChipShotAttackRoll,4).Code,Code::Accepted);
  else
  {
   const auto State=Access::Session(*F.Mode).GetStateSnapshot();
   const auto Safe=F.Safe(Anti?1:2);
   const auto* Contest=Safe.ResolutionFacts.FormulaContests.FindByPredicate([](const auto& C){return C.ContestId==TEXT("ThroughBall.OneOnOne.DirectShot");});
   if(!TestNotNull(TEXT("New DirectShot Formula exists"),Contest))return false;
   for(const auto* Row:{&Contest->AttackRow,&Contest->DefenseRow})for(const auto& Term:Row->Terms)
    TestTrue(TEXT("Earlier roles cannot leak Trait facts into OneOnOne"),Term.AttributeOperand.Bonus==0&&Term.AttributeOperand.TraitId.IsNone());
   const auto* Shooter=Contest->AttackRow.Terms.FindByPredicate([](const auto& T){return T.Attribute==EMatchPlayResolutionFormulaAttribute::Shooting;});
   if(!TestNotNull(TEXT("Actual shooter base operand"),Shooter))return false;
   TestEqual(TEXT("OneOnOne reads unmodified shooter base"),Shooter->SourceValue,float(State.CurrentAttack.ResolutionSession.Bundle.Runner.Values.Shooting));
   TestEqual(TEXT("Direct attack"),F.Roll(Kind::ThroughBallOneOnOneDirectShotAttackRoll,3).Code,Code::Accepted);
   TestEqual(TEXT("Direct defense"),F.Roll(Kind::ThroughBallOneOnOneDirectShotDefenseRoll,3).Code,Code::Accepted);
  }
 }
 return true;
}
#endif
