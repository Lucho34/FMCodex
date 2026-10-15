#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexDefensiveCardContext.h"
#include "FMCodexAttackingCardContext.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../NetworkPlay/FMCodexNetworkMatchPresentation.h"
#include "../NetworkPlay/FMCodexNetworkInitialRouteTestFixture.h"
#include "Misc/AutomationTest.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Slate/WidgetRenderer.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"

namespace
{
using Side = EInitialTurnOrderPlayer;
using Skill = ESkillRuleType;
const TArray<FName> AllCounters={TEXT("Trait.LongShotBlocker"),TEXT("Trait.CutInsideStopper"),TEXT("Trait.CrossMarkerBlocker"),
 TEXT("Trait.CrossHighHelperDefense"),TEXT("Trait.CrossLowHelperDefense"),TEXT("Trait.ThroughBallMarkerDefense"),
 TEXT("Trait.ThroughBallFeetHelperDefense"),TEXT("Trait.ThroughBallBehindHelperDefense")};
FFMCodexLocalMatchInteractionView OpeningView()
{
 const auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession S;S.InitializeMatch(Demo.OpeningInput);S.BeginOrdinaryAttack(4);
 FFMCodexLocalMatchViewerDisclosure PublicTP;PublicTP.bRevealInitialActionPointRoll=true;
 return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(S.GetStateSnapshot(),Demo.SkillRuleSet,Side::PlayerA,PublicTP);
}
FFMCodexUMGMatchScreenViewModel Model(const FFMCodexLocalMatchInteractionView& V, Side Viewer=Side::PlayerA)
{ return FFMCodexLocalMatchUMGPresentationBuilder::Build(V,{},TEXT(""),Viewer); }
FFMCodexUMGCardViewModel Card(const FFMCodexUMGMatchScreenViewModel& M,FName Id)
{
 for(const auto* Rack:{&M.LocalRack,&M.OpponentRack})for(const auto& C:Rack->Cells)if(C.Card.CardId==Id)return C.Card;
 return {};
}
struct FPublicFixture
{
 FFMCodexLocalMatchInteractionView V=OpeningView();
 FName Carrier,Other,Defender;
 TMap<Skill,FFMCodexLocalMatchCardView::FSkill> Skills;
 FPublicFixture()
 {
  for(const auto& C:V.PlayerACardRoster)for(const auto& S:C.Skills)Skills.Add(FFMCodexAttackingCardContext::SkillType(S.SkillId),S);
  for(auto& C:V.PlayerACardRoster)
  {
   C.EligibleTacticalSkills.Reset();
   if(!C.bGoalkeeper && Carrier.IsNone())Carrier=C.CardId;
   else if(!C.bGoalkeeper && Other.IsNone())Other=C.CardId;
  }
  for(auto& C:V.PlayerACardRoster)
  {
   if(C.CardId==Carrier)C.EligibleTacticalSkills={Skills[Skill::LongShot],Skills[Skill::CutInsideShot]};
   if(C.CardId==Other)C.EligibleTacticalSkills={Skills[Skill::Cross],Skills[Skill::ThroughBall]};
  }
  auto* D=V.PlayerBCardRoster.FindByPredicate([](const auto& C){return !C.bGoalkeeper;});
  Defender=D->CardId;D->RankedTraits.Reset();
  for(FName T:AllCounters){FPlayerRankedTrait R;R.TraitId=T;R.Rank=EPlayerTraitRank::B;D->RankedTraits.Add(R);}
 }
 TArray<FName> Matches()const {return Card(Model(V),Defender).DefensiveCounterTraits;}
 void Final(Skill Family){V.SelectedCarrierCardId=Carrier;V.SelectedSkillId=Skills[Family].SkillId;V.PresentedActionType=Family;}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefensiveContextSemantics,"FMCodex.LocalPlay.UI.DefensiveCardContext.SemanticsAndProjection",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDefensiveContextSemantics::RunTest(const FString&)
{
 FPublicFixture F;
 TestEqual(TEXT("Deployment broad preview uses all available public families"),F.Matches().Num(),8);
 auto& Other=*F.V.PlayerACardRoster.FindByPredicate([&](const auto& C){return C.CardId==F.Other;});
 Other.bAvailable=false;Other.bUsed=true;
 TestEqual(TEXT("Used attacker never expands preview"),F.Matches().Num(),2);
 Other.bUsed=false;Other.bEjected=true;TestEqual(TEXT("Ejected attacker excluded"),F.Matches().Num(),2);
 Other.bEjected=false;Other.bAvailable=true;
 F.V.bPlayerADeploymentFinished=true;TestTrue(TEXT("Locked deployment does not use remaining hand"),F.Matches().IsEmpty());
 Other.bDeployed=true;TestEqual(TEXT("Locked deployment considers deployed prospect"),F.Matches().Num(),6);
 F.V.bPlayerADeploymentFinished=false;Other.bDeployed=false;
 F.V.SelectedCarrierCardId=F.Carrier;
 TestTrue(TEXT("Published Carrier narrows to its two families"),F.Matches()==TArray<FName>({AllCounters[0],AllCounters[1]}));
 F.Final(Skill::LongShot);TestTrue(TEXT("Final LongShot"),F.Matches()==TArray<FName>({AllCounters[0]}));
 F.Final(Skill::CutInsideShot);TestTrue(TEXT("Final CutInside"),F.Matches()==TArray<FName>({AllCounters[1]}));
 F.Final(Skill::Cross);TestEqual(TEXT("Undetermined Cross retains both Helper potentials"),F.Matches().Num(),3);
 auto& Facts=F.V.ResolutionFacts;Facts.bSuccess=Facts.bHasFacts=Facts.bHasActualBranch=true;
 Facts.ActualBranch.ActionType=Skill::Cross;Facts.ActualBranch.Cross=EMatchPlayCrossActualBranch::High;
 FMatchPlayResolutionParticipantFact P;P.CardId=F.Defender;P.Side=Side::PlayerB;P.Role=EMatchPlayResolutionParticipantRole::Helper;Facts.Participants={P};
 F.V.SelectedHelperCardId=F.Defender;
 TestTrue(TEXT("Published High Helper excludes Marker and Low"),F.Matches()==TArray<FName>({AllCounters[3]}));
 Facts.ActualBranch.Cross=EMatchPlayCrossActualBranch::Low;
 TestTrue(TEXT("Published Low Helper narrows"),F.Matches()==TArray<FName>({AllCounters[4]}));
 TestEqual(TEXT("Ordinary Low uses Speed, never Corner Defense"),FFMCodexDefensiveCardContext::Attribute(AllCounters[4]),FName(TEXT("SPD")));
 F.V.SelectedHelperCardId=NAME_None;F.V.SelectedMarkerCardId=F.Defender;Facts.Participants[0].Role=EMatchPlayResolutionParticipantRole::Marker;
 TestTrue(TEXT("Known Marker excludes Helper traits"),F.Matches()==TArray<FName>({AllCounters[2]}));
 F.V.SelectedMarkerCardId=NAME_None;Facts={};F.Final(Skill::ThroughBall);
 TestEqual(TEXT("ThroughBall unresolved potential"),F.Matches().Num(),3);
 Facts.bSuccess=Facts.bHasFacts=Facts.bHasActualBranch=true;Facts.ActualBranch.ActionType=Skill::ThroughBall;Facts.Participants={P};
 Facts.ActualBranch.ThroughBall=EMatchPlayThroughBallActualBranch::Feet;
 TestTrue(TEXT("Feet Helper"),F.Matches()==TArray<FName>({AllCounters[6]}));
 Facts.ActualBranch.ThroughBall=EMatchPlayThroughBallActualBranch::BehindDefense;
 TestTrue(TEXT("Behind Helper"),F.Matches()==TArray<FName>({AllCounters[7]}));
 Facts.FormulaContests.AddDefaulted_GetRef().ContestId=TEXT("ThroughBall.BehindDefense.P1");Facts.FormulaContests[0].bHasResolvedFormula=true;
 TestTrue(TEXT("Completed P1 does not carry counters into OneOnOne"),F.Matches().IsEmpty());
 Facts.FormulaContests[0].bHasResolvedFormula=false;Facts.FormulaContests[0].Application=EMatchPlayResolutionFormulaApplication::SkippedByAuthoritativeGate;
 TestTrue(TEXT("Public skipped P1 also clears its counters"),F.Matches().IsEmpty());Facts.FormulaContests.Reset();
 Facts.ActualBranch.ThroughBall=EMatchPlayThroughBallActualBranch::AntiOffside;
 TestTrue(TEXT("No invented AntiOffside defense"),F.Matches().IsEmpty());
 Facts={};F.Final(Skill::LongShot);
 auto& D=*F.V.PlayerBCardRoster.FindByPredicate([&](const auto& C){return C.CardId==F.Defender;});
 D.RankedTraits[0].Rank=EPlayerTraitRank::None;TestTrue(TEXT("No missing grade invented"),F.Matches().IsEmpty());D.RankedTraits[0].Rank=EPlayerTraitRank::B;
 D.bUsed=true;TestTrue(TEXT("Used defender clears"),F.Matches().IsEmpty());D.bUsed=false;
 F.V.RouteKind=EMatchPlayCurrentAttackRouteKind::SetPiece;TestTrue(TEXT("No ordinary hints in Corner/set piece"),F.Matches().IsEmpty());F.V.RouteKind=EMatchPlayCurrentAttackRouteKind::Ordinary;
 F.V.bTerminalPendingAdvance=true;TestTrue(TEXT("Terminal clears"),F.Matches().IsEmpty());F.V.bTerminalPendingAdvance=false;
 F.V.bMatchEnded=true;TestTrue(TEXT("Match end clears"),F.Matches().IsEmpty());F.V.bMatchEnded=false;
 F.V.ActionPoint=0;TestTrue(TEXT("Pre-TP clears"),F.Matches().IsEmpty());F.V.ActionPoint=4;
 F.V.CurrentAttackingPlayer=Side::PlayerB;TestTrue(TEXT("Possession switch removes old defending context"),F.Matches().IsEmpty());

 // Both current attacker sides and both viewers use the same safe-derived card DTO.
 const auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();FMatchPlayAuthoritativeSession S;S.InitializeMatch(Demo.OpeningInput);S.BeginOrdinaryAttack(4);
 auto State=S.GetStateSnapshot();
 for(Side Attacker:{Side::PlayerA,Side::PlayerB})
 {
  State.RuntimeState.CurrentAttackingPlayer=Attacker;TMap<FName,TArray<FName>> First;
  for(Side Viewer:{Side::PlayerA,Side::PlayerB})
  {
   FFMCodexLocalMatchViewerDisclosure PublicTP;PublicTP.bRevealInitialActionPointRoll=true;
   auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Demo.SkillRuleSet,Viewer,PublicTP);
   const auto Local=Model(V,Viewer);auto Net=FFMCodexNetworkMatchPresentationAdapter::Project(V,Viewer);
   bool HasCounter=false;for(const auto* Rack:{&Local.LocalRack,&Local.OpponentRack})for(const auto& Cell:Rack->Cells)HasCounter|=!Cell.Card.DefensiveCounterTraits.IsEmpty();
   TestTrue(TEXT("Public TP parity fixture contains real production counters"),HasCounter);
   TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
   FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Save,&Net,nullptr);
   FFMCodexNetworkMatchPresentation Restored;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,false);
   FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Load,&Restored,nullptr);
   TestFalse(TEXT("DTO reflection round trip"),Save.IsError()||Load.IsError());
   const auto Remote=FFMCodexNetworkMatchPresentationAdapter::Read(Restored,true);
   for(const auto* Rack:{&Local.LocalRack,&Local.OpponentRack})for(const auto& Cell:Rack->Cells)
   {
    const auto& C=Cell.Card;
    TestTrue(TEXT("Local/Network pending preserves identical public hints"),C.DefensiveCounterTraits==Card(Remote,C.CardId).DefensiveCounterTraits);
    if(Viewer==Side::PlayerA)First.Add(C.CardId,C.DefensiveCounterTraits);
    else TestTrue(TEXT("Both viewers receive same counters"),First.FindRef(C.CardId)==C.DefensiveCounterTraits);
   }
   FFMCodexLocalMatchViewerDisclosure Hidden;
   const auto HiddenModel=Model(FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Demo.SkillRuleSet,Viewer,Hidden),Viewer);
   for(const auto* Rack:{&HiddenModel.LocalRack,&HiddenModel.OpponentRack})for(const auto& C:Rack->Cells)
    TestTrue(TEXT("Withheld TP cannot be inferred through hints"),C.Card.DefensiveCounterTraits.IsEmpty());
  }
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefensiveContextRendering,"FMCodex.LocalPlay.UI.DefensiveCardContext.CardRendering",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDefensiveContextRendering::RunTest(const FString&)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W=CreateWidget<UFMCodexPlayerCardWidget>(World);auto Slate=W->TakeWidget();FWidgetRenderer Renderer(false);
 auto V=OpeningView();auto Source=V.PlayerBCardRoster.FindByPredicate([](const auto& C){return C.CardId==FName(TEXT("Prototype.ManchesterCity.Rodri"));});
 if(!Source)Source=V.PlayerBCardRoster.FindByPredicate([](const auto& C){return !C.bGoalkeeper;});
 auto C=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*Source);
 C.RoleLabel=TEXT("M/D");C.AssignedPlayerNumber=TEXT("16");C.RankedTraits.Reset();
 for(int32 I:{0,3,4}){FPlayerRankedTrait T;T.TraitId=AllCounters[I];T.Rank=EPlayerTraitRank::B;C.RankedTraits.Add(T);}
 auto Text=[&](const TCHAR* Name){return CastChecked<UTextBlock>(W->GetWidgetFromName(Name));};
 for(auto Mode:{EFMCodexPlayerCardPresentationMode::HandMicro,EFMCodexPlayerCardPresentationMode::PitchMini})
 for(int32 Count:{0,1,3})
 {
  C.DefensiveCounterTraits.Reset();for(int32 I=0;I<Count;++I)C.DefensiveCounterTraits.Add(C.RankedTraits[I].TraitId);
  W->RefreshFromPresentation(C,Mode);W->SetHandSkillRevealProgress(0.f);
  const bool Hand=Mode==EFMCodexPlayerCardPresentationMode::HandMicro;
  const FVector2D Size=Hand?FVector2D(220,68):FVector2D(136,140);
  auto* Target=Renderer.DrawWidget(Slate,Size);Renderer.DrawWidget(Target,Slate,Size,0.f);
  TestEqual(TEXT("Existing card footprint"),W->GetConfiguredDimensions(),Size);
  const auto* Label=Text(Hand?TEXT("HandMicroMatchingSkills"):TEXT("PitchMiniMatchingSkills"));
  TestEqual(TEXT("Presence determines label visibility"),Label->IsVisible(),Count>0);
  if(Count)
  {
   TestTrue(TEXT("Defensive label has no offensive reveal delay"),Label->GetRenderOpacity()>.99f);
   const FString Content=Label->GetText().ToString();
   TestTrue(TEXT("Readable counter or exact narrow-lane count summary"),Content.Contains(TEXT("防远射"))||(Count==3 && Content==TEXT("防守3项")));
   if(Count==3)TestTrue(TEXT("Overflow explicitly counts omitted traits"),Content.Contains(TEXT("+2"))||Content.Contains(TEXT("+1"))||Content==TEXT("防守3项"));
   AddInfo(FString::Printf(TEXT("COUNTER_LABEL mode=%s count=%d text=%s width=%.1f/%.1f"),Hand?TEXT("Hand"):TEXT("Mini"),Count,*Content,Label->GetDesiredSize().X,Label->GetCachedGeometry().GetLocalSize().X));
   TestTrue(TEXT("Measured text fits its allocated lane"),Label->GetDesiredSize().X<=Label->GetCachedGeometry().GetLocalSize().X+.1f);
  }
  // Both purposes share the existing top-left pip pair. Defense uses only one.
  TestEqual(TEXT("Exactly one defensive presence dot"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipTop"))->IsVisible(),Count>0);
  TestFalse(TEXT("Multiple Traits never produce a second dot"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipBottom"))->IsVisible());
 }
 C.DefensiveCounterTraits={AllCounters[0],AllCounters[3]};W->RefreshFromPresentation(C);
 auto Highlighted=[&](UWidget* N){return CastChecked<UFMCodexFullCardSurface>(N)->IsContextRelevant();};
 TestTrue(TEXT("DEF/STR from owned relevant traits"),Highlighted(W->FindAttributePresentationWidget(TEXT("DEF")))&&Highlighted(W->FindAttributePresentationWidget(TEXT("STR"))));
 TestFalse(TEXT("Unrelated Speed stays ordinary"),Highlighted(W->FindAttributePresentationWidget(TEXT("SPD"))));
 TestTrue(TEXT("Relevant defensive row"),Highlighted(W->FindTraitPresentationWidget(AllCounters[0])));
 TestFalse(TEXT("Unrelated owned Low Helper stays ordinary"),Highlighted(W->FindTraitPresentationWidget(AllCounters[4])));
 const auto Colors=W->GetRenderedAttributeTierColors();
 TestTrue(TEXT("Stage 8.30A attribute text style reused"),Text(TEXT("AttributeLabel5"))->GetColorAndOpacity().GetSpecifiedColor().ToFColor(true)==FColor(0xA4,0xD4,0xCF));
 C.DefensiveCounterTraits.Reset();W->RefreshFromPresentation(C);
 TestFalse(TEXT("Full context clears on refresh"),Highlighted(W->FindAttributePresentationWidget(TEXT("DEF"))));
 TestTrue(TEXT("Attribute tier colors unchanged"),Colors==W->GetRenderedAttributeTierColors());
 auto A=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*V.PlayerACardRoster.FindByPredicate([](const auto& Card){return Card.CardId==FName(TEXT("Prototype.Arsenal.EberechiEze"));}));
 W->RefreshFromPresentation(A,EFMCodexPlayerCardPresentationMode::HandMicro);W->SetHandSkillRevealProgress(0.f);
 TestEqual(TEXT("Attacker still waits for its reveal"),Text(TEXT("HandMicroMatchingSkills"))->GetRenderOpacity(),0.f);
 W->SetHandSkillRevealProgress(1.f);TestEqual(TEXT("Attacker Skill labels preserved"),Text(TEXT("HandMicroMatchingSkills"))->GetText().ToString(),FString(TEXT("远射·内切")));
 A.PitchMiniVisibleTacticalSkills=A.HandMicroVisibleTacticalSkills;A.PitchMiniTacticalMatchCount=2;A.bHasPitchMiniTacticalMatch=true;
 W->RefreshFromPresentation(A,EFMCodexPlayerCardPresentationMode::PitchMini);
 TestEqual(TEXT("Attacker Mini labels preserved"),Text(TEXT("PitchMiniMatchingSkills"))->GetText().ToString(),FString(TEXT("远射·内切")));
 TestTrue(TEXT("Attacker Mini still has two skill dots"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipBottom"))->IsVisible());
 TestEqual(TEXT("Stable order and accurate prefix overflow"),FFMCodexDefensiveCardContext::CompactLabels({AllCounters[4],AllCounters[3],AllCounters[0]},[](const FText& T){return T.ToString().Len()<=6;}).ToString(),FString(TEXT("防远射·+2")));
 W->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefensiveContextDisclosure,"FMCodex.LocalPlay.UI.DefensiveCardContext.PublicRouteDisclosure",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDefensiveContextDisclosure::RunTest(const FString&)
{
 using namespace FMCodexNetworkInitialRouteTests;
 FFixture F(false,6,false,false);
 if(!TestTrue(TEXT("Canonical Cross selection"),F.ReachRoute(ESkillRuleType::Cross,1)))return false;
 if(!TestTrue(TEXT("Real provider resolves route"),F.Send(F.Attacker(),Kind::CrossInitialRouteRoll)))return false;
 const auto Before=Access::Session(*F.Mode).GetStateSnapshot();
 const int32 RNGCalls=F.Entropy->Calls;
 for(auto Viewer:{EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB})
 for(bool Reveal:{false,true})
 {
  auto V=Access::Safe(*F.Mode,Viewer,true,Reveal);
  TestEqual(TEXT("Only existing disclosure publishes actual route"),V.ResolutionFacts.bHasActualBranch,Reveal);
  const FName Helper=V.SelectedHelperCardId;
  auto* H=V.PlayerBCardRoster.FindByPredicate([&](const auto& C){return C.CardId==Helper;});
  if(!TestNotNull(TEXT("Public selected Helper"),H))return false;
  // Test-only ownership stress: no production roster mutation. The route and
  // participant facts still come exclusively from the real safe projection.
  H->RankedTraits.Reset();
  for(int32 I:{3,4}){FPlayerRankedTrait T;T.TraitId=AllCounters[I];T.Rank=EPlayerTraitRank::B;H->RankedTraits.Add(T);}
  const auto Local=Model(V,Viewer);
  const auto Counters=Card(Local,Helper).DefensiveCounterTraits;
  TestEqual(TEXT("Unrevealed result retains both possibilities; public result narrows"),Counters.Num(),Reveal?1:2);
  if(Reveal)TestTrue(TEXT("Public High route selects Strength helper only"),Counters==TArray<FName>({AllCounters[3]}));
  const auto Remote=FFMCodexNetworkMatchPresentationAdapter::Read(FFMCodexNetworkMatchPresentationAdapter::Project(V,Viewer),false);
  TestTrue(TEXT("Client receives only safe-derived counters"),Counters==Card(Remote,Helper).DefensiveCounterTraits);
 }
 TestTrue(TEXT("Presentation never mutates authority"),SameState(Before,Access::Session(*F.Mode).GetStateSnapshot()));
 TestEqual(TEXT("Presentation never consumes RNG"),F.Entropy->Calls,RNGCalls);
 return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDefensiveHandOverflow,"FMCodex.LocalPlay.UI.DefensiveCardContext.HandMicroOverflowRendering",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDefensiveHandOverflow::RunTest(const FString&)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W=CreateWidget<UFMCodexPlayerCardWidget>(World);auto Slate=W->TakeWidget();FWidgetRenderer Renderer(false);
 const auto V=OpeningView();
 auto C=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*V.PlayerBCardRoster.FindByPredicate([](const auto& P){return P.CardId==TEXT("Prototype.ManchesterCity.Rodri");}));
 auto* Label=CastChecked<UTextBlock>(W->GetWidgetFromName(TEXT("HandMicroMatchingSkills")));
 auto* Number=CastChecked<UTextBlock>(W->GetWidgetFromName(TEXT("AssignedPlayerNumber")));
 auto Draw=[&](){W->RefreshFromPresentation(C,EFMCodexPlayerCardPresentationMode::HandMicro);auto* RT=Renderer.DrawWidget(Slate,FVector2D(220,68));Renderer.DrawWidget(RT,Slate,FVector2D(220,68),0.f);};
 // Ownership includes unrelated traits; formatting consumes only the current projected list.
 for(int32 Case=0;Case<4;++Case)
 {
  C.RoleLabel=Case==1?TEXT("D"):TEXT("M/D");C.AssignedPlayerNumber=Case==1?TEXT(""):TEXT("16");
  C.DefensiveCounterTraits={AllCounters[0]};
  if(Case>0)C.DefensiveCounterTraits.Add(AllCounters[1]);
  if(Case==3)C.DefensiveCounterTraits.Add(AllCounters[3]);
  Draw();
  const FString Expected=Case==0?TEXT("防远射"):Case==1?TEXT("防远射·防内切"):Case==2?TEXT("防守2项"):TEXT("防守3项");
  TestEqual(TEXT("Complete names or exact contextual total"),Label->GetText().ToString(),Expected);
  TestFalse(TEXT("No defensive Hand partial-prefix suffix"),Label->GetText().ToString().Contains(TEXT("+")));
  TestTrue(TEXT("Text fits allocated lane"),Label->GetDesiredSize().X<=Label->GetCachedGeometry().GetLocalSize().X+.1f);
  TestEqual(TEXT("Hand dimensions preserved"),W->GetConfiguredDimensions(),FVector2D(220,68));
  if(!C.AssignedPlayerNumber.IsEmpty())
  {
   const auto& NG=Number->GetCachedGeometry();const auto& LG=Label->GetCachedGeometry();
   const float NumberLeft=NG.LocalToAbsolute(FVector2D(NG.GetLocalSize().X-Number->GetDesiredSize().X,0)).X;
   const float LabelRight=LG.LocalToAbsolute(FVector2D(Label->GetDesiredSize().X,0)).X;
   TestTrue(TEXT("Visible label and right-aligned shirt number remain separate"),LabelRight<NumberLeft);
   TestEqual(TEXT("Shirt number not shrunk"),Number->GetFont().Size,16.f);
  }
  TestTrue(TEXT("Defensive cue remains one dot"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipTop"))->IsVisible());
  TestFalse(TEXT("Defensive cue never counts traits"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipBottom"))->IsVisible());
 }
 auto Font=Label->GetFont();Font.Size=10;
 const auto Measure=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
 auto Fits=[&](const FText& Text){return Measure->Measure(Text,Font).X<=50.f;};
 TestFalse(TEXT("Single long label exceeds measured test lane"),Fits(FText::FromString(TEXT("高球协防"))));
 const FText Single=FFMCodexDefensiveCardContext::CompactLabels({AllCounters[3]},Fits,false);
 TestEqual(TEXT("Single overflow uses count one"),Single.ToString(),FString(TEXT("防守1项")));
 TestTrue(TEXT("Single summary fits measured test lane"),Fits(Single));
 C=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*V.PlayerACardRoster.FindByPredicate([](const auto& P){return P.CardId==TEXT("Prototype.Arsenal.EberechiEze");}));Draw();
 TestEqual(TEXT("Attacking names unchanged"),Label->GetText().ToString(),FString(TEXT("远射·内切")));
 TestTrue(TEXT("Attacking two-Skill dots unchanged"),W->GetWidgetFromName(TEXT("PitchMiniTacticalMatchPipBottom"))->IsVisible());
 W->SetHandSkillRevealProgress(0.f);TestEqual(TEXT("Attacking reveal unchanged"),Label->GetRenderOpacity(),0.f);
 W->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);return true;
}
#endif
