#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "FMCodexFormulaExplanation.h"
#include "FMCodexFormulaTooltipTestUtils.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "Misc/AutomationTest.h"

namespace
{
using Attribute=EMatchPlayResolutionFormulaAttribute;
using Role=EMatchPlayResolutionParticipantRole;
using Kind=EFMCodexUMGInlineFormulaTermKind;
using Row=FFMCodexUMGInlineFormulaRowViewModel;
using Term=FFMCodexUMGInlineFormulaTermViewModel;

Term Input(const TCHAR* Name,Role Participant,Attribute Attr,int32 Base,int32 Effective,float Weight,float Contribution)
{
 Term T;T.Kind=Kind::Attribute;T.bResolved=true;T.ContributorDisplayName=Name;
 T.AttributeLabel=FFMCodexPlayerUIPresentationText::ResolutionAttribute(Attr).ToString();
 T.SourceValue=Effective;T.Multiplier=Weight;T.Contribution=Contribution;
 FPlayerTraitFormulaOperand Op;Op.bValid=true;Op.Role=Participant;Op.Attribute=Attr;Op.BaseValue=Base;Op.EffectiveValue=Effective;
 T.AttributeOperands.Add(Op);return T;
}
FString Explain(const Row& R,const FString& Context)
{
 FString Result;
 for(const auto& L:FMCodexFormulaExplanation::Build(R,Context))Result+=L.Label+TEXT("\t")+L.Value+TEXT("\n");
 return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaExplanationStructuredTest,
 "FMCodex.LocalPlay.FormulaExplanation.StructuredRowsAndNativeHover",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFormulaExplanationStructuredTest::RunTest(const FString&)
{
 // Feed displayed facts, including deliberate non-derived totals: the view must
 // consume each supplied fact, not implement formula evaluation a second time.
 Row R;R.SideLabel=TEXT("玩家 A · 进攻");R.bKnownNonRollSubtotalResolved=true;R.KnownNonRollSubtotal=17.25f;
 R.Terms.Add(Input(TEXT("萨卡"),Role::Carrier,Attribute::Passing,5,8,.5f,4.f));
 auto& Op=R.Terms[0].AttributeOperands[0];Op.TraitId=TEXT("Trait.CrossCarrier");Op.Rank=EPlayerTraitRank::S;Op.Bonus=3;
 R.Terms.Add(Input(TEXT("哈弗茨"),Role::Runner,Attribute::Strength,4,4,.5f,2.f));
 Term Fixed;Fixed.Kind=Kind::FixedModifier;Fixed.bResolved=true;Fixed.ModifierSourceLabel=TEXT("战术球员加成");Fixed.Contribution=1;R.Terms.Add(Fixed);
 Term Hidden;Hidden.DisplayLabel=TEXT("未公开属性哨兵");Hidden.SourceValue=99;R.Terms.Add(Hidden);
 Term Dice;Dice.Kind=Kind::RawRoll;Dice.bResolved=true;Dice.DisplayLabel=TEXT("未来骰哨兵");Dice.RawD6=6;R.Terms.Add(Dice);
 R.bFinalValueResolved=true;R.FinalValue=999;R.FinalValueLabel=TEXT("未来结果哨兵");
 auto High=Explain(R,TEXT("高球传中"));
 TestTrue(TEXT("Ranked bonus precedes weight and effective attribute is not clamped"),High.Contains(TEXT("传球 +3 → 8")) && High.Contains(TEXT("传球贡献：(5 +3) × 0.5\t= 4")));
 TestTrue(TEXT("Separate participant without Trait retains original and supplied contribution"),High.Contains(TEXT("哈弗茨 · 力量")) && High.Contains(TEXT("原始力量：4")) && High.Contains(TEXT("力量贡献：4 × 0.5\t= 2")));
 TestTrue(TEXT("Named modifier and authoritative subtotal remain independent"),High.Contains(TEXT("战术球员加成\t+1")) && High.Contains(TEXT("最终基础值\t17.25")));
 TestFalse(TEXT("Base hover cannot expose unresolved attributes, dice or final result"),High.Contains(TEXT("哨兵")) || High.Contains(TEXT("999")));

 // Both routes use the real shared native hover; a changed route may not retain
 // the other route's attribute. No roll/theater scenario is needed for this contract.
 for(const bool bHigh:{true,false})
 {
  FFMCodexUMGMatchScreenViewModel P;
  P.InlineFormula.bVisible=true;P.InlineFormula.bShowFormulaRows=true;
  P.InlineFormula.ContestId=bHigh?TEXT("Cross.High"):TEXT("Cross.Low");
  P.InlineFormula.AttackRow=R;
  if(!bHigh)P.InlineFormula.AttackRow.Terms[1]=Input(TEXT("哈弗茨"),Role::Runner,Attribute::Speed,4,4,.5f,2.f);
  auto* Screen=NewObject<UFMCodexLocalMatchScreenWidget>(GetTransientPackage());
  Screen->TakeWidget();Screen->RefreshFromPresentation(P);
  auto* Hover=Screen->GetWidgetFromName(TEXT("TheaterAttackBaseHover"));
  if(!TestNotNull(TEXT("Shared theater base anchor exists"),Hover))return false;
  const FString Text=FMCodexFormulaTooltipTestUtils::Text(Hover->GetToolTip());
  TestTrue(TEXT("Native tooltip uses current route context"),Text.Contains(bHigh?TEXT("高球传中"):TEXT("低球传中")));
  TestTrue(TEXT("Native tooltip uses route-specific participant attribute"),Text.Contains(bHigh?TEXT("哈弗茨 · 力量"):TEXT("哈弗茨 · 速度")));
  TestFalse(TEXT("Other route is absent"),Text.Contains(bHigh?TEXT("原始速度"):TEXT("原始力量")));
  TestTrue(TEXT("Native tooltip renders supplied base"),Text.Contains(TEXT("最终基础值 17.25")));
 }
 FFMCodexUMGInlineFormulaSurfaceViewModel P;P.ContestId=TEXT("ThroughBall.Feet");
 TestEqual(TEXT("Feet context is not Behind or OneOnOne"),FMCodexFormulaExplanation::Context(P),FString(TEXT("直塞 · 脚下球")));
 P.ContestId=TEXT("ThroughBall.OneOnOne.DirectShot");
 TestEqual(TEXT("OneOnOne retains own formula stage"),FMCodexFormulaExplanation::Context(P),FString(TEXT("单刀 · 直接射门")));
 R.bKnownNonRollSubtotalResolved=false;
 TestTrue(TEXT("Unresolved subtotal remains unknown even with populated terms"),Explain(R,TEXT("远射直接射门")).Contains(TEXT("最终基础值\t?")));
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFormulaExplanationFallbackTest,
 "FMCodex.LocalPlay.FormulaExplanation.FlattenedSetPieceFacts",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFormulaExplanationFallbackTest::RunTest(const FString&)
{
 Row R;R.SideLabel=TEXT("进攻");R.bKnownNonRollSubtotalResolved=true;R.KnownNonRollSubtotal=8;
 auto Shooting=Input(TEXT("主罚球员"),Role::Taker,Attribute::Shooting,5,7,1,7);
 auto Passing=Input(TEXT("主罚球员"),Role::Taker,Attribute::Passing,6,8,1,8);
 auto& Op=Shooting.AttributeOperands[0];Op.TraitId=TEXT("Trait.NearFreeKickTaker");Op.Rank=EPlayerTraitRank::A;Op.Bonus=2;
 auto& PassOp=Passing.AttributeOperands[0];PassOp.TraitId=Op.TraitId;PassOp.Rank=Op.Rank;PassOp.Bonus=2;
 Term Max;Max.bResolved=true;Max.AttributeOperands={Op,Passing.AttributeOperands[0]};
 Max.DisplayLabel=TEXT("射门 5 +2 / 传球 6 +2（取较高值 → 8）");Max.Contribution=8;R.Terms.Add(Max);
 const auto Text=Explain(R,TEXT("近距离任意球 · 直接射门"));
 TestTrue(TEXT("Candidate originals are readable without being added together"),Text.Contains(TEXT("原始射门：5")) && Text.Contains(TEXT("原始传球：6")) && Text.Contains(Max.DisplayLabel));
 TestFalse(TEXT("No fake weighted contribution is invented for max expression"),Text.Contains(TEXT("贡献：")));
 TestTrue(TEXT("Provided max-based subtotal survives unchanged"),Text.Contains(TEXT("最终基础值\t8")));
 R.Terms.Reset();R.KnownNonRollSubtotal=2;
 Term Keeper;Keeper.bResolved=true;Keeper.DisplayLabel=TEXT("预判 5");R.Terms.Add(Keeper);
 Term Adjustment;Adjustment.bResolved=true;Adjustment.Kind=Kind::FixedModifier;Adjustment.DisplayLabel=TEXT("点球防守调整 -3");R.Terms.Add(Adjustment);
 const auto Legacy=Explain(R,TEXT("点球 · 常规点球"));
 TestTrue(TEXT("Legacy keeper and negative modifier remain accurate"),Legacy.Contains(TEXT("预判 5")) && Legacy.Contains(TEXT("点球防守调整 -3")));
 TestFalse(TEXT("Absent numeric metadata never creates a zero contribution"),Legacy.Contains(TEXT("= 0")) || Legacy.Contains(TEXT("+0")));
 TestTrue(TEXT("Final base still comes from authority"),Legacy.Contains(TEXT("最终基础值\t2")));
 return true;
}
#endif
