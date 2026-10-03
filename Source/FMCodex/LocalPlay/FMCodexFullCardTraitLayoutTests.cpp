#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "../CoreRules/PlayerTraitFormula.h"
#include "../NetworkPlay/FMCodexNetworkMatchPresentation.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridSlot.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullCardTraitProjection,
 "FMCodex.LocalPlay.UI.FullCardTraits.AuthoritativeOwnerProjection",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFullCardTraitProjection::RunTest(const FString&)
{
 auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 auto Opening=FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Demo.OpeningInput);
 if (!TestTrue(TEXT("Canonical opening"),Opening.bSuccess)) return false;
 auto& State=Opening.MatchPlayState;
 auto& Saka=*State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.FindByPredicate(
  [](const auto& C){return C.CardId==TEXT("Prototype.Arsenal.BukayoSaka");});
 Saka.RankedTraits[0].Rank=EPlayerTraitRank::S; // Different from catalog B: proves snapshot ownership.
 for (auto Viewer:{EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB,EInitialTurnOrderPlayer::None})
 {
  auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Demo.SkillRuleSet,Viewer);
  for (const auto* Roster:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for (const auto& C:*Roster)
  {
   const bool Own=Viewer!=EInitialTurnOrderPlayer::None && C.Side==Viewer;
   const auto& Cards=C.Side==EInitialTurnOrderPlayer::PlayerA?State.CardSnapshotAuthority.PlayerACardSnapshots.Cards:State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;
   const auto& Snapshot=*Cards.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
   TestEqual(TEXT("Owner assignment count; opponent and invalid viewer receive none"),C.RankedTraits.Num(),Own?Snapshot.RankedTraits.Num():0);
   TestEqual(TEXT("Binary assignments are owner-safe too"),C.BinaryTraits.Num(),Own?Snapshot.BinaryTraits.Num():0);
   if (Own && C.CardId==Saka.CardId) TestEqual(TEXT("Snapshot rank overrides catalog"),C.RankedTraits[0].Rank,EPlayerTraitRank::S);
  }
  if (Viewer!=EInitialTurnOrderPlayer::None)
  {
   const auto Net=FFMCodexNetworkMatchPresentationAdapter::Project(V,Viewer);
   TestTrue(TEXT("Safe network presentation produced"),Net.bAvailable);
   const auto Read=FFMCodexNetworkMatchPresentationAdapter::Read(Net,false);
   for (const auto& Cell:Read.OpponentRack.Cells)
    TestTrue(TEXT("Network read does not restore opponent traits"),Cell.Card.RankedTraits.IsEmpty() && Cell.Card.BinaryTraits.IsEmpty());
   int32 OwnCount=0;for (const auto& Cell:Read.LocalRack.Cells) OwnCount+=Cell.Card.RankedTraits.Num()+Cell.Card.BinaryTraits.Num();
   TestTrue(TEXT("Network catalog strip/hydration retains owner assignment facts"),OwnCount>0);
  }
  // Local hot-seat consumes the same complete authority projection but gates at its read adapter.
  const auto Raw=FFMCodexLocalMatchInteractionViewBuilder::Build(State,Demo.SkillRuleSet);
  const auto UMG=FFMCodexLocalMatchUMGPresentationBuilder::Build(Raw,{},TEXT(""),Viewer);
  for (const auto& Cell:UMG.OpponentRack.Cells)
   TestTrue(TEXT("Opponent UMG carries no static Trait payload"),Cell.Card.RankedTraits.IsEmpty() && Cell.Card.BinaryTraits.IsEmpty());
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullCardTraitLayout,
 "FMCodex.LocalPlay.UI.FullCardTraits.ProductionCapacityAndGeometry",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFullCardTraitLayout::RunTest(const FString&)
{
 const auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 const auto Opening=FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Demo.OpeningInput);
 if (!TestTrue(TEXT("Production opening"),Opening.bSuccess)) return false;
 const auto V=FFMCodexLocalMatchInteractionViewBuilder::Build(Opening.MatchPlayState,Demo.SkillRuleSet);
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W=CreateWidget<UFMCodexPlayerCardWidget>(World); auto Slate=W->TakeWidget();
 auto* Renderer=new FWidgetRenderer(false);
 int32 MaxOff=0,MaxDef=0,MaxTotal=0,MaxSkills=0,LongestOff=0,LongestDef=0;
 bool Empty=false,OffOnly=false,DefOnly=false,Both=false,Binary=false;
 float TraitHeight=-1;
 for (const auto* Roster:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for (const auto& C:*Roster)
 {
  const auto M=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(C);
  W->RefreshFromPresentation(M,EFMCodexPlayerCardPresentationMode::InteractionChoice);
  auto* Target=Renderer->DrawWidget(Slate,FVector2D(360,540));
  Renderer->DrawWidget(Target,Slate,FVector2D(360,540),0.f);
  auto Node=[&](const TCHAR* N){return W->GetWidgetFromName(N);};
  TestEqual(TEXT("Full footprint frozen"),W->GetConfiguredDimensions(),FVector2D(360,540));
  TestEqual(TEXT("Six base attributes; GK retained"),W->GetRenderedAttributeCount(),6);
  for (int32 I=0;I<6;++I)
  {
   const auto* Cell=Node(*FString::Printf(TEXT("AttributeCellBounds%d"),I));
   const auto* Slot=CastChecked<UUniformGridSlot>(Cell->Slot);
   TestEqual(TEXT("Three rows in canonical order"),Slot->GetRow(),I%3);
   TestEqual(TEXT("Two columns in canonical order"),Slot->GetColumn(),I/3);
   if (!C.bGoalkeeper)
   {
    const TCHAR* Labels[]={TEXT("射门"),TEXT("传球"),TEXT("控球"),TEXT("速度"),TEXT("力量"),TEXT("防守")};
    TestEqual(TEXT("Outfield labels exclude stamina"),CastChecked<UTextBlock>(Node(*FString::Printf(TEXT("AttributeLabel%d"),I)))->GetText().ToString(),FString(Labels[I]));
   }
  }
  TestEqual(TEXT("Outfield stamina is fifth biography row; GK keeps four"),W->GetRenderedBiographyRowCount(),C.bGoalkeeper?4:5);
  if (!C.bGoalkeeper)
  {
   const auto* Tier=CastChecked<UTextBlock>(Node(TEXT("BiographyStaminaValue")));
   const auto* Source=C.AttributeValues.FindByPredicate([](const auto& A){return A.CanonicalLabel==TEXT("StaminaTier");});
   TestTrue(TEXT("Stamina shows authoritative semantic tier"),Source && Tier->GetText().ToString()==Source->ValueLabel);
   TestTrue(C.CardId.ToString()+TEXT(" profile ends above identity"),Node(TEXT("InMatchFullCardBiographyBounds"))->GetCachedGeometry().GetAbsolutePosition().Y+Node(TEXT("InMatchFullCardBiographyBounds"))->GetCachedGeometry().GetAbsoluteSize().Y <= Node(TEXT("CardIdentityRegion"))->GetCachedGeometry().GetAbsolutePosition().Y);
  }
  int32 Off=C.BinaryTraits.Num(),Def=0;
  for (const auto& T:C.RankedTraits) (FPlayerTraitFormula::Category(T.TraitId)==EPlayerTraitCategory::Offensive?Off:Def)++;
  MaxOff=FMath::Max(MaxOff,Off);MaxDef=FMath::Max(MaxDef,Def);MaxTotal=FMath::Max(MaxTotal,Off+Def);MaxSkills=FMath::Max(MaxSkills,C.Skills.Num());
  Empty|=Off+Def==0;OffOnly|=Off>0&&Def==0;DefOnly|=Off==0&&Def>0;Both|=Off>0&&Def>0;
  int32 O=0,D=0;
  auto CheckTrait=[&](FName Id,const FString& Value,int32 Tier,bool IsBinary)
  {
   const bool Offensive=FPlayerTraitFormula::Category(Id)==EPlayerTraitCategory::Offensive;
   const FString Base=FString(Offensive?TEXT("FullCardOffensiveTraits"):TEXT("FullCardDefensiveTraits"))+FString::FromInt(Offensive?O++:D++);
   const auto* Name=CastChecked<UTextBlock>(Node(*(Base+TEXT("Name"))));
   const auto* Header=CastChecked<UTextBlock>(Node(Offensive?TEXT("FullCardOffensiveTraitsHeading"):TEXT("FullCardDefensiveTraitsHeading")));
   TestEqual(TEXT("Full localized assignment name retained"),Name->GetText().ToString(),FPlayerTraitFormula::DisplayName(Id).ToString());
   TestTrue(TEXT("Subsection heading aligns with name content edge"),FMath::IsNearlyEqual(Header->GetCachedGeometry().GetAbsolutePosition().X,Name->GetCachedGeometry().GetAbsolutePosition().X,.1f));
   TestTrue(TEXT("No clipped name"),Name->GetDesiredSize().X<=Name->GetCachedGeometry().GetLocalSize().X+.1f);
   if (IsBinary)
   {
    TestNull(TEXT("Binary emits no presence text or placeholder value"),Node(*(Base+TEXT("Value"))));
    TestNull(TEXT("Binary emits no rank/presence badge"),Node(*(Base+TEXT("Badge"))));
   }
   else
   {
    const auto* Rank=CastChecked<UTextBlock>(Node(*(Base+TEXT("Value"))));
    const auto* Badge=CastChecked<UFMCodexFullCardSurface>(Node(*(Base+TEXT("Badge"))));
    TestEqual(TEXT("Ranked badge retains S/A/B"),Rank->GetText().ToString(),Value);
    TestTrue(TEXT("Name stays clear of badge"),Name->GetCachedGeometry().GetAbsolutePosition().X+Name->GetDesiredSize().X<=Badge->GetCachedGeometry().GetAbsolutePosition().X);
    TestTrue(TEXT("Badge reuses attribute tier color"),Badge->GetAccent().Equals(UFMCodexPlayerCardWidget::GetAttributeTierColor(Tier)));
   }
   (Offensive?LongestOff:LongestDef)=FMath::Max(Offensive?LongestOff:LongestDef,Name->GetText().ToString().Len());
  };
  for (const auto& T:C.RankedTraits) CheckTrait(T.TraitId,T.Rank==EPlayerTraitRank::S?TEXT("S"):T.Rank==EPlayerTraitRank::A?TEXT("A"):TEXT("B"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(T.Rank),false);
  for (const auto Id:C.BinaryTraits) {CheckTrait(Id,FString(),0,true);Binary=true;}
  for (auto Category:{TEXT("FullCardOffensiveTraits"),TEXT("FullCardDefensiveTraits")})
   if ((FString(Category).Contains(TEXT("Offensive"))?Off:Def)==0)
    TestEqual(TEXT("Quiet empty category"),CastChecked<UTextBlock>(Node(*(FString(Category)+TEXT("Empty"))))->GetText().ToString(),FString(TEXT("—")));
  const auto& TG=Node(TEXT("TraitPresentationRegion"))->GetCachedGeometry();
  if (TraitHeight<0) TraitHeight=TG.GetLocalSize().Y;
  TestTrue(TEXT("Stable category height regardless of assignments"),FMath::IsNearlyEqual(TraitHeight,TG.GetLocalSize().Y));
  const auto& AG=Node(TEXT("AttributePresentationRegion"))->GetCachedGeometry();
  const auto& SG=Node(TEXT("SkillPresentationRegion"))->GetCachedGeometry();
  TestTrue(TEXT("Traits follow attributes"),TG.GetAbsolutePosition().Y>=AG.GetAbsolutePosition().Y+AG.GetAbsoluteSize().Y-.1f);
  if (!C.Skills.IsEmpty()) TestTrue(TEXT("Skills follow traits and remain in bounds"),SG.GetAbsolutePosition().Y>=TG.GetAbsolutePosition().Y+TG.GetAbsoluteSize().Y-.1f && SG.GetAbsolutePosition().Y+SG.GetAbsoluteSize().Y<=540.f);
  TestEqual(TEXT("Skills retained"),W->GetRenderedSkillCount(),C.Skills.Num());

 }
 TestEqual(TEXT("S rank is value 6 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::S),6);
 TestEqual(TEXT("A rank is value 5 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::A),5);
 TestEqual(TEXT("B rank is value 4 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::B),4);
 TestTrue(TEXT("Production covers empty, offensive, defensive, both and Binary"),Empty&&OffOnly&&DefOnly&&Both&&Binary);
 AddInfo(FString::Printf(TEXT("CONTENT_CAPACITY offensive=%d defensive=%d total=%d longestOff=%d longestDef=%d skills=%d"),MaxOff,MaxDef,MaxTotal,LongestOff,LongestDef,MaxSkills));
 W->RemoveFromParent();BeginCleanup(Renderer);GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return true;
}
#endif
