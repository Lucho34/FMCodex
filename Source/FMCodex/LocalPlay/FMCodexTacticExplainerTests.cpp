#if WITH_DEV_AUTOMATION_TESTS
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "../CoreRules/PlayerTraitFormula.h"
#include "Misc/AutomationTest.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplainerContract,"FMCodex.LocalPlay.TacticExplainer.ContentContract",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplainerContract::RunTest(const FString&)
{
 const auto& C=FFMCodexTacticExplainerCatalog::Get();
 TestEqual(TEXT("Eight families"),C.Num(),8);
 const int32 Counts[]={2,2,2,5,2,2,2,2};
 TSet<FName> Ids;
 for(int32 I=0;I<C.Num();++I)
 {
  TestEqual(TEXT("Route/method coverage"),C[I].Routes.Num(),Counts[I]);
  TestEqual(TEXT("Two navigation groups"),C[I].bSetPiece,I>=4);
  TestTrue(TEXT("No PassControl"),C[I].Skill!=ESkillRuleType::PassControl);
  for(const auto& R:C[I].Routes)
  {
   TestFalse(TEXT("Unique stable route"),Ids.Contains(R.Id)); Ids.Add(R.Id);
   TestTrue(TEXT("Calculation present"),R.bProcedural?!R.Procedures.IsEmpty():!R.AttackCalculation.IsEmpty() && !R.DefenseCalculation.IsEmpty());
   if(!R.bProcedural)
   {
    TestEqual(TEXT("Concise common result"),R.Result.ToString(),FString(TEXT("比较双方最终值，数值较高的一方获胜。")));
    auto Check=[&](const TArray<FTacticalRuleDescriptionTerm>& Source,const TArray<FFMCodexExplainerCalculationTerm>& Display)
    {
     int32 Index=0;
     for(const auto& Term:Source)
     {
      if(Term.Kind==EMatchPlayResolutionFormulaTermKind::TacticalPlayerAdvantage) continue;
      if(!TestTrue(TEXT("Every canonical core operand retained"),Display.IsValidIndex(Index))) continue;
      const auto& Actual=Display[Index++].Source;
      TestEqual(TEXT("Operand kind"),Actual.Kind,Term.Kind);
      TestEqual(TEXT("Actual role"),Actual.ParticipantRole,Term.ParticipantRole);
      TestEqual(TEXT("Attribute"),Actual.Attribute,Term.Attribute);
      TestEqual(TEXT("Coefficient"),Actual.Multiplier,Term.Multiplier);
      TestEqual(TEXT("Fixed modifier"),Actual.FixedModifier,Term.FixedModifier);
      TestEqual(TEXT("Conditional contribution"),Actual.bOptional,Term.bOptional);
     }
     if(!Source.IsEmpty()) TestEqual(TEXT("No extra core operands"),Display.Num(),Index);
    };
    Check(R.Rule.AttackTerms,R.AttackCalculation); Check(R.Rule.DefenseTerms,R.DefenseCalculation);
   }
   for(FName Id:R.Traits) TestTrue(TEXT("Canonical localized Trait"),(IsKnownRankedPlayerTrait(Id)||IsKnownBinaryPlayerTrait(Id)) && !FPlayerTraitFormula::DisplayName(Id).IsEmpty());
   if(R.bProcedural) TestTrue(TEXT("Procedural never fabricates formula operands"),R.Defense.IsEmpty() && R.AttackCalculation.IsEmpty() && R.DefenseCalculation.IsEmpty());
  }
 }
 using A=EMatchPlayResolutionFormulaAttribute;
 const auto& LS=C[0].Routes[0];
 TestEqual(TEXT("LongShot Shooting"),LS.Rule.AttackTerms[0].Attribute,A::Shooting);
 TestTrue(TEXT("LongShot fixed +3"),LS.Rule.DefenseTerms.ContainsByPredicate([](const auto& T){return T.FixedModifier==3;}));
 const auto& Low=C[2].Routes[1];
 TestEqual(TEXT("Cross Low runner speed"),Low.Rule.AttackTerms[1].Attribute,A::Speed);
 TestEqual(TEXT("Cross Low helper speed"),Low.Rule.DefenseTerms[1].Attribute,A::Speed);
 TestEqual(TEXT("Cross Low four traits"),Low.Traits.Num(),4);
 const auto& Corner=C[4].Routes[1];
 TestEqual(TEXT("Corner Low control"),Corner.Rule.AttackTerms[0].Attribute,A::Control);
 TestEqual(TEXT("Corner Low defense"),Corner.Rule.DefenseTerms[0].Attribute,A::Defense);
 TestEqual(TEXT("Corner GK reflex"),Corner.Rule.DefenseTerms[1].Attribute,A::GoalkeeperReflex);
 TestTrue(TEXT("Near and Penalty preserve max"),C[5].Routes[0].bMaxAttributes && C[7].Routes[0].bMaxAttributes);
 TestEqual(TEXT("Long FK Shooting"),C[6].Routes[0].AttackCalculation[0].Source.Attribute,A::Shooting);
 TestEqual(TEXT("Long FK fixed +2"),C[6].Routes[0].DefenseCalculation.Last().Source.FixedModifier,2);
 TestEqual(TEXT("Near FK fixed +1"),C[5].Routes[0].DefenseCalculation.Last().Source.FixedModifier,1);
 TestEqual(TEXT("Penalty fixed minus3"),C[7].Routes[0].DefenseCalculation.Last().Source.FixedModifier,-3);
 for(int32 I:{5,7})
 {
  TestTrue(TEXT("Both max inputs visible before attack die"),C[I].Routes[0].AttackCalculation[0].bMaximum && C[I].Routes[0].AttackCalculation[0].Attribute.ToString()==TEXT("射门 与 传球 取较高值"));
  TestEqual(TEXT("Taker coefficient 1"),C[I].Routes[0].AttackCalculation[0].Source.Multiplier,1.f);
  TestEqual(TEXT("GK coefficient 1"),C[I].Routes[0].DefenseCalculation[0].Source.Multiplier,1.f);
  TestTrue(TEXT("Max methods have no early miss"),C[I].Routes[0].Precondition.IsEmpty());
 }
 TestFalse(TEXT("Long FK retains early miss"),C[6].Routes[0].Precondition.IsEmpty());
 TestTrue(TEXT("Corner-specific adjustment retained"),Corner.CalculationNote.ToString().Contains(TEXT("+2")) && Corner.CalculationNote.ToString().Contains(TEXT("+3")));
 TestTrue(TEXT("AntiOffside procedural actual runner"),C[3].Routes[2].bProcedural && C[3].Routes[2].Traits==TArray<FName>{TEXT("Trait.ThroughBallAntiRunner")});
 const TCHAR* GKNames[]={TEXT("站位"),TEXT("手控球"),TEXT("制空"),TEXT("单刀"),TEXT("制空"),TEXT("手控球"),TEXT("站位"),TEXT("预判")};
 for(int32 I=0;I<8;++I)
 {
  const auto& Rows=C[I].Routes[0].Defense;
  TestTrue(TEXT("Each family's actual GK attribute"),Rows.ContainsByPredicate([&](const auto& Row){return Row.Label.ToString().Contains(TEXT("门将")) && Row.Attributes.ToString()==GKNames[I];}));
 }
 TestFalse(TEXT("BehindDefense has no GK"),C[3].Routes[1].Defense.ContainsByPredicate([](const auto& Row){return Row.Label.ToString().Contains(TEXT("门将"));}));
 TestEqual(TEXT("CutInside carrier two terms same participant"),C[1].Routes[0].Rule.AttackTerms[0].ParticipantRole,C[1].Routes[0].Rule.AttackTerms[1].ParticipantRole);
 TestEqual(TEXT("CutInside Shooting coefficient"),C[1].Routes[0].Rule.AttackTerms[0].Multiplier,.5f);
 TestEqual(TEXT("CutInside Control coefficient"),C[1].Routes[0].Rule.AttackTerms[1].Multiplier,.5f);

 return true;
}
namespace
{
TSharedPtr<SWidget> FindExplainerNode(const TSharedRef<SWidget>& Root,const FString& Type,const FString& Text=FString())
{
 if(Root->GetTypeAsString()==Type && (Text.IsEmpty() || (Type==TEXT("STextBlock") && StaticCastSharedRef<STextBlock>(Root)->GetText().ToString()==Text))) return Root;
 auto* Children=Root->GetChildren();
 for(int32 I=0;I<Children->Num();++I) if(auto Found=FindExplainerNode(Children->GetChildAt(I),Type,Text)) return Found;
 return nullptr;
}
FString MountedExplainerText(const TSharedRef<SWidget>& Root)
{
 FString Text;
 if(Root->GetTypeAsString()==TEXT("STextBlock")) Text=StaticCastSharedRef<STextBlock>(Root)->GetText().ToString()+TEXT("\n");
 auto* Children=Root->GetChildren();
 for(int32 I=0;I<Children->Num();++I) Text+=MountedExplainerText(Children->GetChildAt(I));
 return Text;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplainerNavigation,"FMCodex.LocalPlay.TacticExplainer.NavigationAndDisclosure",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplainerNavigation::RunTest(const FString&)
{
 auto* W=NewObject<UFMCodexTacticExplainerWidget>(); const auto Root=W->TakeWidget();
 const auto Navigation=FindExplainerNode(Root,TEXT("STextBlock"),TEXT("基础进攻"));
 const auto Scroll=FindExplainerNode(Root,TEXT("SScrollBox"));
 W->SelectTactic(TEXT("Cross")); W->SelectRoute(TEXT("Cross.Low"));
 const FString Overview=W->CollectPlayerFacingText();
 TestTrue(TEXT("Tactic and route updates preserve shell, navigation and scroll widgets"),W->TakeWidget()==Root
  && FindExplainerNode(Root,TEXT("STextBlock"),TEXT("基础进攻"))==Navigation && FindExplainerNode(Root,TEXT("SScrollBox"))==Scroll);
 const FString Mounted=MountedExplainerText(Root);
 TestTrue(TEXT("Complete low-cross page is mounted together with no stale high-cross traits"),Mounted.Contains(TEXT("低球传中协防")) && !Mounted.Contains(TEXT("高球传中协防")));
 StaticCastSharedPtr<SScrollBox>(Scroll)->SetScrollOffset(100.f);
 W->SelectRoute(TEXT("Cross.High"));
 TestEqual(TEXT("Meaningful route switch resets scroll immediately"),W->GetScrollOffsetForTest(),0.f);
 TestTrue(TEXT("High route swaps a coherent page while retaining navigation"),MountedExplainerText(Root).Contains(TEXT("高球传中协防")) && !MountedExplainerText(Root).Contains(TEXT("低球传中协防"))
  && FindExplainerNode(Root,TEXT("STextBlock"),TEXT("基础进攻"))==Navigation);
 W->SelectRoute(TEXT("Cross.Low"));
 TestTrue(TEXT("Overview changes to Low attributes and canonical Traits"),Overview.Contains(TEXT("速度")) && Overview.Contains(TEXT("反应")) && Overview.Contains(TEXT("低球传中协防")) && !Overview.Contains(TEXT("高球传中协防")));
 TestTrue(TEXT("Conditional GK visible"),Overview.Contains(TEXT("门将（参与时）")));
 TestFalse(TEXT("Removed helper sentence absent"),Overview.Contains(TEXT("各角色的参与条件")));
 TestTrue(TEXT("Skill note retained"),Overview.Contains(TEXT("技能范围")));
 TestFalse(TEXT("Overview has no bonus/rank"),Overview.Contains(TEXT("+1")) || Overview.Contains(TEXT("S/A/B")));
 W->ShowDetails(true); TestTrue(TEXT("Same pane exact coefficients"),W->IsShowingDetails() && W->CollectPlayerFacingText().Contains(TEXT("0.5")));
 W->ShowDetails(false); TestEqual(TEXT("Return restores overview"),W->CollectPlayerFacingText(),Overview);
 TestFalse(TEXT("Cannot select foreign route"),W->SelectRoute(TEXT("Penalty.Direct")));
 W->SelectTactic(TEXT("ThroughBall")); W->SelectRoute(TEXT("ThroughBall.AntiOffside")); W->ShowDetails(true);
 TestTrue(TEXT("Binary 2D6 explanation"),W->CollectPlayerFacingText().Contains(TEXT("任意一次掷出 6")));
 W->SelectTactic(TEXT("NearFreeKick")); TestFalse(TEXT("Family switch resets detail"),W->IsShowingDetails());
 TestTrue(TEXT("Special max readable"),W->CollectPlayerFacingText().Contains(TEXT("取较高值")));
 // Scan every actual calculation selection, including conditional and set-piece routes.
 for(const auto& Family:FFMCodexTacticExplainerCatalog::Get())
 {
  W->SelectTactic(Family.Id);
  for(const auto& Route:Family.Routes)
  {
   W->SelectRoute(Route.Id); W->ShowDetails(true);
   const FString Copy=W->CollectPlayerFacingText();
   TestTrue(TEXT("Calculation heading and return"),Copy.Contains(TEXT("计算方式")) && Copy.Contains(TEXT("返回概览")));
   const TCHAR* Removed[]={TEXT("D6"),TEXT("S/A/B"),TEXT("体力总和"),TEXT("战术球员优势"),TEXT("快速压制"),TEXT("每次对抗"),TEXT("不复用"),TEXT("特性只由"),TEXT("查看详细规则")};
   for(const TCHAR* Word:Removed) TestFalse(TEXT("No generic/internal calculation prose"),Copy.Contains(Word));
   TestEqual(TEXT("Precondition only when present"),Copy.Contains(TEXT("前置判定")),!Route.Precondition.IsEmpty());
   if(!Route.bProcedural) TestTrue(TEXT("Arithmetic has attack/defense dice and result"),Copy.Contains(TEXT("进攻骰点")) && Copy.Contains(TEXT("防守骰点")) && Copy.Contains(TEXT("比较双方最终值")));
  }
 }
 W->SelectTactic(TEXT("CutInside")); W->ShowDetails(true);
 const FString Cut=W->CollectPlayerFacingText();
 TestTrue(TEXT("CutInside labeled fixed +2 and conditional GK"),Cut.Contains(TEXT("固定防守修正 +2")) && Cut.Contains(TEXT("门将 · 手控球 × 0.5（参与时）")));
 W->SelectTactic(TEXT("LongShot")); W->ShowDetails(true);
 TestTrue(TEXT("LongShot labeled fixed +3"),W->CollectPlayerFacingText().Contains(TEXT("固定防守修正 +3")));
 W->ShowDetails(false); TestTrue(TEXT("Overview CTA renamed"),W->CollectPlayerFacingText().Contains(TEXT("查看计算方式")));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplainerProcedures,"FMCodex.LocalPlay.TacticExplainer.CalculationView.ProceduralMethods",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplainerProcedures::RunTest(const FString&)
{
 auto* W=NewObject<UFMCodexTacticExplainerWidget>(); W->TakeWidget();
 W->SelectTactic(TEXT("ThroughBall")); W->SelectRoute(TEXT("ThroughBall.AntiOffside")); W->ShowDetails(true);
 const FString Anti=W->CollectPlayerFacingText();
 TestEqual(TEXT("Binary branches are two cards"),W->GetRoute().Procedures.Num(),2);
 TestTrue(TEXT("Normal one die / Binary two dice, any six and both failure paths"),Anti.Contains(TEXT("投掷 1 次骰子")) && Anti.Contains(TEXT("投掷 2 次骰子")) && Anti.Contains(TEXT("任意一次掷出 6")) && Anti.Contains(TEXT("未掷出 6，越位")) && Anti.Contains(TEXT("两次都未掷出 6，越位")));
 TestFalse(TEXT("AntiOffside no fake comparison or defender"),Anti.Contains(TEXT("防守骰点")) || Anti.Contains(TEXT("盯人球员")) || Anti.Contains(TEXT("协防球员")) || Anti.Contains(TEXT("最终值")));
 struct Case { const TCHAR* Family; const TCHAR* Route; const TCHAR* Action; const TCHAR* Win; const TCHAR* Lose; };
 const Case Cases[]={
  {TEXT("LongShot"),TEXT("LongShot.DeadCorner"),TEXT("投掷 2 次骰子"),TEXT("合计 11–12，进球"),TEXT("合计 2–10，不进球")},
  {TEXT("CutInside"),TEXT("CutInside.DeadCorner"),TEXT("投掷 2 次骰子"),TEXT("合计 11–12，进球"),TEXT("合计 2–10，不进球")},
  {TEXT("ThroughBall"),TEXT("ThroughBall.OneOnOneChip"),TEXT("投掷 1 次骰子"),TEXT("骰点 4–6，进球"),TEXT("骰点 1–3，不进球")},
  {TEXT("NearFreeKick"),TEXT("NearFreeKick.Angled"),TEXT("投掷 2 次骰子"),TEXT("合计 9–12，进球"),TEXT("合计 2–8，不进球")},
  {TEXT("LongFreeKick"),TEXT("LongFreeKick.Power"),TEXT("投掷 2 次骰子"),TEXT("合计 11–12，进球"),TEXT("合计 2–10，不进球")},
  {TEXT("Penalty"),TEXT("Penalty.Panenka"),TEXT("投掷 1 次骰子"),TEXT("骰点 2–6，进球"),TEXT("骰点 1，不进球")}};
 for(const auto& C:Cases)
 {
  W->SelectTactic(C.Family); W->SelectRoute(C.Route); W->ShowDetails(true);
  const FString Copy=W->CollectPlayerFacingText();
  TestTrue(TEXT("Actual dice count and inclusive success/failure ranges"),Copy.Contains(C.Action) && Copy.Contains(C.Win) && Copy.Contains(C.Lose));
  TestFalse(TEXT("Pure shot has no defense equation"),Copy.Contains(TEXT("防守骰点")) || Copy.Contains(TEXT("比较双方最终值")));
 }
 W->SelectTactic(TEXT("NearFreeKick")); W->SelectRoute(TEXT("NearFreeKick.Angled")); W->ShowDetails(true);
 TestTrue(TEXT("Tactical cooperation gate retained"),W->CollectPlayerFacingText().Contains(TEXT("射门 + 传球 ≥ 8 时可用")));
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplainerIsolation,"FMCodex.LocalPlay.TacticExplainer.ModalIntentIsolation",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplainerIsolation::RunTest(const FString&)
{
 struct FBackend : IFMCodexMatchScreenBackend
 {
  int32 Count=0;
  EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest&) override {++Count;return EFMCodexMatchScreenSubmission::Completed;}
  bool IsScreenIntentPending() const override {return false;}
 } Backend;
 auto* S=NewObject<UFMCodexLocalMatchScreenWidget>(); S->TakeWidget(); S->SetMatchBackend(&Backend);
 FFMCodexUMGMatchScreenViewModel P; P.Interaction.Category=EFMCodexUMGInteractionCategory::Deploy; P.Interaction.bCanFinishDeployment=true;
 S->RefreshFromPresentation(P); S->OpenDeploymentTacticalReference();
 auto* W=S->GetTacticExplainer();
 for(const auto& T:FFMCodexTacticExplainerCatalog::Get()) { W->SelectTactic(T.Id); for(const auto& R:T.Routes){W->SelectRoute(R.Id);W->ShowDetails(true);W->ShowDetails(false);} }
 S->RequestDeployOrdinary(TEXT("Card"),TEXT("Slot")); S->RequestFinishDeployment(); S->RequestRollTacticalPoints(); S->RequestStartNewMatch();
 TestEqual(TEXT("Modal and stale underlying callbacks submit zero gameplay intents"),Backend.Count,0);
 TestTrue(TEXT("Underlying finish cannot dismiss modal"),S->IsDeploymentTacticalReferenceOpen());
 W->OnClose.Broadcast(); S->RequestFinishDeployment();
 TestEqual(TEXT("Close restores action boundary"),Backend.Count,1);
 S->SetMatchBackend(nullptr); return true;
}
#endif
