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
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullCardTraitProjection,
 "FMCodex.LocalPlay.UI.FullCardTraits.AuthoritativePublicProjection",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFullCardTraitProjection::RunTest(const FString&)
{
 auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 auto Opening=FMatchPlayOpeningInitializer::InitializeMatchPlayOpening(Demo.OpeningInput);
 if (!TestTrue(TEXT("Canonical opening"),Opening.bSuccess)) return false;
 auto& State=Opening.MatchPlayState;
 auto& Saka=*State.CardSnapshotAuthority.PlayerACardSnapshots.Cards.FindByPredicate(
  [](const auto& C){return C.CardId==TEXT("Prototype.Arsenal.BukayoSaka");});
 Saka.RankedTraits[0].Rank=EPlayerTraitRank::S; // Different from catalog B: proves authoritative provenance.
 const auto Raw=FFMCodexLocalMatchInteractionViewBuilder::Build(State,Demo.SkillRuleSet);
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* Widget=CreateWidget<UFMCodexPlayerCardWidget>(World);auto Slate=Widget->TakeWidget();
 for (auto Viewer:{EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB,
  EInitialTurnOrderPlayer::None,static_cast<EInitialTurnOrderPlayer>(255)})
 {
  const bool Valid=Viewer==EInitialTurnOrderPlayer::PlayerA || Viewer==EInitialTurnOrderPlayer::PlayerB;
  auto CheckAssignments=[&](const auto& C)
  {
   const auto& Cards=Raw.PlayerACardRoster.ContainsByPredicate([&](const auto& P){return P.CardId==C.CardId;})
    ?State.CardSnapshotAuthority.PlayerACardSnapshots.Cards:State.CardSnapshotAuthority.PlayerBCardSnapshots.Cards;
   const auto* Snapshot=Cards.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
   if (!TestNotNull(TEXT("Public card has authoritative snapshot"),Snapshot)) return;
   TestEqual(TEXT("Both participants receive every ranked assignment; invalid viewers fail closed"),C.RankedTraits.Num(),Valid?Snapshot->RankedTraits.Num():0);
   TestEqual(TEXT("Both participants receive every binary assignment"),C.BinaryTraits.Num(),Valid?Snapshot->BinaryTraits.Num():0);
   for (int32 I=0;Valid && I<FMath::Min(C.RankedTraits.Num(),Snapshot->RankedTraits.Num());++I)
   {
    TestEqual(TEXT("Exact authoritative Trait ID"),C.RankedTraits[I].TraitId,Snapshot->RankedTraits[I].TraitId);
    TestEqual(TEXT("Exact authoritative S/A/B grade, never catalog fallback"),C.RankedTraits[I].Rank,Snapshot->RankedTraits[I].Rank);
   }
   if (Valid) TestTrue(TEXT("Exact authoritative Binary IDs/presence"),C.BinaryTraits==Snapshot->BinaryTraits);
  };
  auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Demo.SkillRuleSet,Viewer);
  for (const auto* Roster:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for (const auto& C:*Roster)
   CheckAssignments(C);
  // Exercise every nested card-bearing surface, including the invalid-viewer guard.
  auto Surfaces=Raw;
  for (const auto* Roster:{&Raw.PlayerACardRoster,&Raw.PlayerBCardRoster}) for (const auto& C:*Roster)
  {
   Surfaces.DeploymentGroups.AddDefaulted_GetRef().Card=C;
   Surfaces.SelectionOptions.AddDefaulted_GetRef().Card=C;
   Surfaces.PitchRegions.AddDefaulted_GetRef().Slots.AddDefaulted_GetRef().Card=C;
  }
  FFMCodexLocalMatchInteractionViewBuilder::ClearFullCardTraitsForInvalidViewer(Surfaces,Viewer);
  for (const auto& G:Surfaces.DeploymentGroups) CheckAssignments(G.Card);
  for (const auto& O:Surfaces.SelectionOptions) CheckAssignments(O.Card);
  for (const auto& R:Surfaces.PitchRegions) for (const auto& S:R.Slots) if (!S.Card.CardId.IsNone()) CheckAssignments(S.Card);
  auto CheckScreen=[&](const FFMCodexUMGMatchScreenViewModel& M)
  {
   TestEqual(TEXT("Both complete hands projected"),M.LocalRack.Cells.Num()+M.OpponentRack.Cells.Num(),40);
   for (const auto* Rack:{&M.LocalRack,&M.OpponentRack}) for (const auto& Cell:Rack->Cells)
   {
    const auto& C=Cell.Card;CheckAssignments(C);
    const auto* Source=Raw.PlayerACardRoster.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
    if (!Source) Source=Raw.PlayerBCardRoster.FindByPredicate([&](const auto& P){return P.CardId==C.CardId;});
    if (!TestNotNull(TEXT("Stable public identity"),Source)) continue;
    const auto Expected=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*Source);
    TestEqual(TEXT("Public name"),C.IdentityLabel,Expected.IdentityLabel);
    TestEqual(TEXT("Public position"),C.RoleLabel,Expected.RoleLabel);
    TestEqual(TEXT("Public OVR"),C.OverallRating,Expected.OverallRating);
    TestEqual(TEXT("Public outfield/GK attributes and stamina"),C.AttributeValues.Num(),Expected.AttributeValues.Num());
    for (int32 I=0;I<FMath::Min(C.AttributeValues.Num(),Expected.AttributeValues.Num());++I)
    {
     TestEqual(TEXT("Attribute identity"),C.AttributeValues[I].CanonicalLabel,Expected.AttributeValues[I].CanonicalLabel);
     TestEqual(TEXT("Attribute value"),C.AttributeValues[I].Value,Expected.AttributeValues[I].Value);
     TestEqual(TEXT("Stamina tier"),C.AttributeValues[I].ValueLabel,Expected.AttributeValues[I].ValueLabel);
    }
    TestEqual(TEXT("Every public Skill"),C.Skills.Num(),Expected.Skills.Num());
    for (int32 I=0;I<FMath::Min(C.Skills.Num(),Expected.Skills.Num());++I)
    {
     TestEqual(TEXT("Skill identity"),C.Skills[I].SkillId,Expected.Skills[I].SkillId);
     TestEqual(TEXT("Skill TP minimum"),C.Skills[I].MinTriggerActionPoint,Expected.Skills[I].MinTriggerActionPoint);
     TestEqual(TEXT("Skill TP maximum"),C.Skills[I].MaxTriggerActionPoint,Expected.Skills[I].MaxTriggerActionPoint);
    }
    Widget->RefreshFromPresentation(C,EFMCodexPlayerCardPresentationMode::InteractionChoice);
    int32 Off=0,Def=0;
    auto CheckRow=[&](FName Id,const FString& Grade)
    {
     const bool Offensive=FPlayerTraitFormula::Category(Id)==EPlayerTraitCategory::Offensive;
     const FString Base=FString(Offensive?TEXT("FullCardOffensiveTraits"):TEXT("FullCardDefensiveTraits"))+FString::FromInt(Offensive?Off++:Def++);
     const auto* Name=Cast<UTextBlock>(Widget->GetWidgetFromName(*(Base+TEXT("Name"))));
     if (TestNotNull(TEXT("Every public assignment reaches existing renderer"),Name))
      TestEqual(TEXT("Complete Chinese Trait name"),Name->GetText().ToString(),FPlayerTraitFormula::DisplayName(Id).ToString());
     const auto* Rank=Cast<UTextBlock>(Widget->GetWidgetFromName(*(Base+TEXT("Value"))));
     if (Grade.IsEmpty()) TestNull(TEXT("Binary has no invented rank"),Rank);
     else if (TestNotNull(TEXT("Rank badge exists"),Rank)) TestEqual(TEXT("Rendered authoritative grade"),Rank->GetText().ToString(),Grade);
    };
    for (const auto& T:C.RankedTraits) CheckRow(T.TraitId,T.Rank==EPlayerTraitRank::S?TEXT("S"):T.Rank==EPlayerTraitRank::A?TEXT("A"):TEXT("B"));
    for (const auto Id:C.BinaryTraits) CheckRow(Id,FString());
   }
  };
  CheckScreen(FFMCodexLocalMatchUMGPresentationBuilder::Build(Raw,{},TEXT(""),Viewer));
  if (Valid)
  {
   auto Net=FFMCodexNetworkMatchPresentationAdapter::Project(V,Viewer);
   TestTrue(TEXT("Safe network presentation produced for either participant"),Net.bAvailable);
   TArray<uint8> Bytes;FMemoryWriter Writer(Bytes);FObjectAndNameAsStringProxyArchive Save(Writer,false);
   FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Save,&Net,nullptr);
   FFMCodexNetworkMatchPresentation Restored;FMemoryReader Reader(Bytes);FObjectAndNameAsStringProxyArchive Load(Reader,false);
   FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Load,&Restored,nullptr);
   TestFalse(TEXT("Reflected DTO round trip succeeds"),Save.IsError()||Load.IsError());
   for (bool Pending:{false,true}) CheckScreen(FFMCodexNetworkMatchPresentationAdapter::Read(Restored,Pending));
  }
 }
 Widget->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
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
 TMap<int32,float> HeightsByRowCapacity;
 for (const auto* Roster:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for (const auto& C:*Roster)
 {
  const auto M=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(C);
  W->RefreshFromPresentation(M,EFMCodexPlayerCardPresentationMode::InteractionChoice);
  auto* Target=Renderer->DrawWidget(Slate,FVector2D(360,540));
  Renderer->DrawWidget(Target,Slate,FVector2D(360,540),0.f);
  auto Node=[&](const TCHAR* N){return W->GetWidgetFromName(N);};
  const FString Context=C.CardId.ToString()+TEXT(" ");
  auto Top=[](const UWidget* N){return N->GetCachedGeometry().GetAbsolutePosition().Y;};
  auto Bottom=[&](const UWidget* N){return Top(N)+N->GetCachedGeometry().GetAbsoluteSize().Y;};
  auto Contains=[&](const UWidget* Outer,const UWidget* Inner)
  {
   const auto A=Outer->GetCachedGeometry().GetAbsolutePosition(),B=Inner->GetCachedGeometry().GetAbsolutePosition();
   const auto AS=Outer->GetCachedGeometry().GetAbsoluteSize(),BS=Inner->GetCachedGeometry().GetAbsoluteSize();
   return B.X>=A.X-.1f && B.Y>=A.Y-.1f && B.X+BS.X<=A.X+AS.X+.1f && B.Y+BS.Y<=A.Y+AS.Y+.1f;
  };
  auto TextFits=[&](const UTextBlock* Text)
  {
   const auto Size=Text->GetCachedGeometry().GetLocalSize();
   // Font line boxes include leading beyond the painted glyphs; accepted compact
   // rows deliberately use less than DesiredSize.Y. Check the actual text lane.
   TestTrue(Context+Text->GetName()+TEXT(" visible and fits horizontal lane"),Text->IsVisible() &&
    Text->GetDesiredSize().X>0 && Text->GetDesiredSize().X<=Size.X+.1f && Size.Y>0);
  };
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
  }
  TestTrue(Context+TEXT("profile has four units clearance above identity"),
   Top(Node(TEXT("CardIdentityRegion")))-Bottom(Node(TEXT("InMatchFullCardBiographyBounds")))>=3.9f);
  for (const TCHAR* Fact:{TEXT("BiographyBirthDate"),TEXT("BiographyHeight"),TEXT("BiographyWeight"),TEXT("BiographyPosition"),TEXT("BiographyStamina")})
  {
   if (C.bGoalkeeper && FString(Fact)==TEXT("BiographyStamina")) continue;
   const auto* Row=Node(*(FString(Fact)+TEXT("Bounds")));
   const auto* Label=CastChecked<UTextBlock>(Node(*(FString(Fact)+TEXT("Label"))));
   const auto* Value=CastChecked<UTextBlock>(Node(*(FString(Fact)+TEXT("Value"))));
   TextFits(Label); TextFits(Value);
   TestTrue(Context+Fact+TEXT(" complete row within profile"),Contains(Node(TEXT("InMatchFullCardBiographyBounds")),Row));
   TestTrue(Context+Fact+TEXT(" label above value"),Bottom(Label)<=Top(Value)+.1f);
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
   const auto* Row=Node(*(Base+TEXT("Row")));
   const auto* Column=Node(Offensive?TEXT("FullCardOffensiveTraits"):TEXT("FullCardDefensiveTraits"));
   TextFits(Name); TextFits(Header);
   TestTrue(Context+Base+TEXT(" grouped and contained"),Contains(Column,Row) && Contains(Row,Name));
   TestTrue(Context+Base+TEXT(" clear of category heading"),Top(Row)>=Bottom(Header)+1.9f);
   const int32 Index=(Offensive?O:D)-1;
   if (Index>0)
   {
    const FString Previous=FString(Offensive?TEXT("FullCardOffensiveTraits"):TEXT("FullCardDefensiveTraits"))+FString::FromInt(Index-1)+TEXT("Row");
    TestTrue(Context+Base+TEXT(" separated from previous row"),Top(Row)>=Bottom(Node(*Previous))+1.9f);
   }
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
    TextFits(Rank);
    TestTrue(Context+Base+TEXT(" complete rank badge"),Contains(Row,Badge) && Contains(Badge,Rank));
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
  // Both columns reserve at least two rows, and expand together for actual content.
  // Different row capacities need not have the same section height.
  const int32 Capacity=FMath::Max(2,FMath::Max(Off,Def));
  if (const auto* Height=HeightsByRowCapacity.Find(Capacity))
   TestTrue(Context+TEXT("same row capacity has stable height"),FMath::IsNearlyEqual(*Height,TG.GetLocalSize().Y));
  else HeightsByRowCapacity.Add(Capacity,TG.GetLocalSize().Y);
  const auto* OffColumn=Node(TEXT("FullCardOffensiveTraits"));
  const auto* DefColumn=Node(TEXT("FullCardDefensiveTraits"));
  TestTrue(Context+TEXT("both categories fit trait region"),Contains(Node(TEXT("TraitPresentationRegion")),OffColumn) && Contains(Node(TEXT("TraitPresentationRegion")),DefColumn));
  TestTrue(Context+TEXT("category columns align and stay separate"),
   FMath::IsNearlyEqual(Top(OffColumn),Top(DefColumn),.1f) && FMath::IsNearlyEqual(Bottom(OffColumn),Bottom(DefColumn),.1f) &&
   OffColumn->GetCachedGeometry().GetAbsolutePosition().X+OffColumn->GetCachedGeometry().GetAbsoluteSize().X+11.9f<=DefColumn->GetCachedGeometry().GetAbsolutePosition().X);
  TestTrue(Context+TEXT("category headings clear section title"),Top(OffColumn)>=Bottom(Node(TEXT("TraitHeadingBounds")))+.9f);
  const auto& AG=Node(TEXT("AttributePresentationRegion"))->GetCachedGeometry();
  const auto& SG=Node(TEXT("SkillPresentationRegion"))->GetCachedGeometry();
  const auto& Bio=Node(TEXT("InMatchFullCardBiographyBounds"))->GetCachedGeometry();
  const auto& Identity=Node(TEXT("CardIdentityRegion"))->GetCachedGeometry();
  const auto& Footer=Node(TEXT("FullCardCollectionSurface"))->GetCachedGeometry();
  TestTrue(Context+TEXT("identity and profile stay in hero"),Contains(Node(TEXT("PortraitAssetBounds")),Node(TEXT("CardIdentityRegion"))) && Contains(Node(TEXT("PortraitAssetBounds")),Node(TEXT("InMatchFullCardBiographyBounds"))));
  TestTrue(Context+TEXT("identity stays above attributes"),Bottom(Node(TEXT("CardIdentityRegion")))<=Top(Node(TEXT("AttributePresentationRegion")))+.1f);
  TestTrue(Context+TEXT("footer remains within card"),Bottom(Node(TEXT("FullCardCollectionSurface")))<=540.f);
  TestTrue(Context+TEXT("footer clears final content"),Top(Node(TEXT("FullCardCollectionSurface")))>=
   Bottom(Node(C.Skills.IsEmpty()?TEXT("TraitPresentationRegion"):TEXT("SkillPresentationRegion")))+3.9f);
  TextFits(CastChecked<UTextBlock>(Node(TEXT("FullCardCollectionLine"))));
  for (int32 I=0;I<C.Skills.Num();++I)
   TestTrue(Context+TEXT("complete skill row within skill section"),Contains(Node(TEXT("SkillPresentationRegion")),Node(*FString::Printf(TEXT("FullCardSkillRowBounds%d"),I))));
  AddInfo(FString::Printf(TEXT("FULL_CARD_GEOMETRY %s off=%d def=%d skills=%d traitsHeight=%.1f profileBottom=%.1f identityTop=%.1f clearance=%.1f skillsBottom=%.1f footerBottom=%.1f"),
   *C.CardId.ToString(),Off,Def,C.Skills.Num(),TG.GetLocalSize().Y,
   Bio.GetAbsolutePosition().Y+Bio.GetAbsoluteSize().Y,Identity.GetAbsolutePosition().Y,
   Identity.GetAbsolutePosition().Y-Bio.GetAbsolutePosition().Y-Bio.GetAbsoluteSize().Y,
   SG.GetAbsolutePosition().Y+SG.GetAbsoluteSize().Y,Footer.GetAbsolutePosition().Y+Footer.GetAbsoluteSize().Y));
  // Optional native widget evidence, bounded to empty/medium/dense production cards.
  if (FParse::Param(FCommandLine::Get(),TEXT("FullCardGeometryEvidence")) &&
   (C.CardId==TEXT("Prototype.Arsenal.DavidRaya") || C.CardId==TEXT("Prototype.Arsenal.EberechiEze") ||
    C.CardId==TEXT("Prototype.Arsenal.MikelMerino") || C.CardId==TEXT("Prototype.ManchesterCity.Rodri")))
  {
   TArray<FColor> Pixels; FReadSurfaceDataFlags Flags(RCM_UNorm); Flags.SetLinearToGamma(false);
   if (TestTrue(TEXT("Production card pixels available"),Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels,Flags)))
   {
    const FString Directory=FPaths::ProjectSavedDir()/TEXT("Stage8_27A/FullCardGeometry");
    IFileManager::Get().MakeDirectory(*Directory,true);
    TArray64<uint8> PNG; FImageUtils::PNGCompressImageArray(360,540,Pixels,PNG);
    TestTrue(TEXT("Production card evidence saved"),FFileHelper::SaveArrayToFile(PNG,*(Directory/(C.CardId.ToString()+TEXT(".png")))));
   }
  }
  TestTrue(TEXT("Traits follow attributes"),TG.GetAbsolutePosition().Y>=AG.GetAbsolutePosition().Y+AG.GetAbsoluteSize().Y-.1f);
  if (!C.Skills.IsEmpty()) TestTrue(TEXT("Skills follow traits and remain in bounds"),SG.GetAbsolutePosition().Y>=TG.GetAbsolutePosition().Y+TG.GetAbsoluteSize().Y-.1f && SG.GetAbsolutePosition().Y+SG.GetAbsoluteSize().Y<=540.f);
  TestEqual(TEXT("Skills retained"),W->GetRenderedSkillCount(),C.Skills.Num());

 }
 if (HeightsByRowCapacity.Contains(2) && HeightsByRowCapacity.Contains(3))
  TestTrue(TEXT("Third row gets additional capacity"),HeightsByRowCapacity[3]>HeightsByRowCapacity[2]);
 TestEqual(TEXT("S rank is value 6 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::S),6);
 TestEqual(TEXT("A rank is value 5 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::A),5);
 TestEqual(TEXT("B rank is value 4 style"),UFMCodexPlayerCardWidget::GetTraitRankVisualTier(EPlayerTraitRank::B),4);
 TestTrue(TEXT("Production covers empty, offensive, defensive, both and Binary"),Empty&&OffOnly&&DefOnly&&Both&&Binary);
 AddInfo(FString::Printf(TEXT("CONTENT_CAPACITY offensive=%d defensive=%d total=%d longestOff=%d longestDef=%d skills=%d"),MaxOff,MaxDef,MaxTotal,LongestOff,LongestDef,MaxSkills));
 W->RemoveFromParent();BeginCleanup(Renderer);GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return true;
}
#endif
