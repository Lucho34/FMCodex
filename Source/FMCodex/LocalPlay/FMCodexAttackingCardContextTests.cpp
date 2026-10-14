#include "FMCodexAttackingCardContext.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../NetworkPlay/FMCodexNetworkMatchPresentation.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Slate/WidgetRenderer.h"

namespace
{
FFMCodexUMGCardViewModel CardAt(int32 TP, FName Id, bool Deployed = false,
 EInitialTurnOrderPlayer Attacker = EInitialTurnOrderPlayer::PlayerA)
{
 auto Demo = FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession Session;
 Session.InitializeMatch(Demo.OpeningInput);
 Session.BeginOrdinaryAttack(TP);
 auto State = Session.GetStateSnapshot();
 State.RuntimeState.CurrentAttackingPlayer = Attacker;
 if (Deployed)
 {
  FMatchPlayDeploymentPlacement P; P.PlayerSide = EInitialTurnOrderPlayer::PlayerA;
  P.CardId = Id; P.SlotId = State.DeploymentSlotCatalog.Slots[0].SlotId;
  State.CurrentAttack.DeploymentPlacements.Add(P);
 }
 auto View = FFMCodexLocalMatchInteractionViewBuilder::Build(State, Demo.SkillRuleSet);
 const auto* Card = View.PlayerACardRoster.FindByPredicate([&](const auto& C){ return C.CardId == Id; });
 return Card ? FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(*Card) : FFMCodexUMGCardViewModel{};
}
bool Highlighted(UWidget* W)
{
 const auto* S = Cast<UFMCodexFullCardSurface>(W);
 return S && S->IsContextRelevant();
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackingContextSemantics,
 "FMCodex.LocalPlay.UI.AttackingCardContext.SemanticsAndPublicParity",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAttackingContextSemantics::RunTest(const FString&)
{
 const FName Eze = TEXT("Prototype.Arsenal.EberechiEze");
 auto Card = CardAt(4, Eze);
 auto C = FFMCodexAttackingCardContext::Build(Card);
 TestEqual(TEXT("Eze current TP4 two options"), C.Skills.Num(), 2);
 TestTrue(TEXT("Shooting/Control union only"), C.Attributes.Num()==2 && C.Attributes.Contains(TEXT("SHO")) && C.Attributes.Contains(TEXT("CON")));
 TestTrue(TEXT("Owned Carrier experts only"), C.Traits.Num()==2 && C.Traits.Contains(TEXT("Trait.LongShotCarrier")) && C.Traits.Contains(TEXT("Trait.CutInsideCarrier")));
 TestFalse(TEXT("Unrelated free kick excluded"), C.Traits.Contains(TEXT("Trait.LongFreeKickTaker")));
 TestEqual(TEXT("Canonical ordered Chinese labels"), FFMCodexAttackingCardContext::SkillNames(Card.HandMicroVisibleTacticalSkills).ToString(), FString(TEXT("远射·内切")));
 auto Field = FFMCodexAttackingCardContext::Build(CardAt(4, Eze, true));
 TestTrue(TEXT("Hand and field share identical context"), C.Skills.Includes(Field.Skills) && Field.Skills.Includes(C.Skills) && C.Attributes.Includes(Field.Attributes) && C.Traits.Includes(Field.Traits));
 for (int32 TP : {2,3,5,6})
 {
  const auto R = FFMCodexAttackingCardContext::Build(CardAt(TP,Eze));
  TestEqual(TEXT("Zero/single options follow current projected TP"),R.Skills.Num(),TP==3||TP==5?1:0);
  TestEqual(TEXT("LongShot SHO; CutInside SHO/CON; no match clears all"),R.Attributes.Num(),TP==3?1:TP==5?2:0);
 }
 Card.RankedTraits.Reset(); C = FFMCodexAttackingCardContext::Build(Card);
 TestTrue(TEXT("Missing Traits never invented; Skills and operands retained"),C.Traits.IsEmpty() && C.Attributes.Num()==2 && C.Skills.Num()==2);
 Card.bGoalkeeper=true; TestTrue(TEXT("GK never inherits outfield highlights"),FFMCodexAttackingCardContext::Build(Card).Skills.IsEmpty());
 TestTrue(TEXT("Defender excluded"), FFMCodexAttackingCardContext::Build(CardAt(4,Eze,false,EInitialTurnOrderPlayer::PlayerB)).Skills.IsEmpty());
 const auto Cross=FFMCodexAttackingCardContext::Build(CardAt(5,TEXT("Prototype.Arsenal.BenWhite")));
 TestTrue(TEXT("Cross Carrier previews Passing and owned Cross expert only"),Cross.Attributes.Num()==1 && Cross.Attributes.Contains(TEXT("PAS")) && Cross.Traits.Num()==1 && Cross.Traits.Contains(TEXT("Trait.CrossCarrier")));
 const auto Through=FFMCodexAttackingCardContext::Build(CardAt(7,TEXT("Prototype.Arsenal.MartinZubimendi")));
 TestTrue(TEXT("ThroughBall previews Passing and owned Carrier master only"),Through.Attributes.Num()==1 && Through.Attributes.Contains(TEXT("PAS")) && Through.Traits.Num()==1 && Through.Traits.Contains(TEXT("Trait.ThroughBallCarrier")));
 const auto RunnerOnly=FFMCodexAttackingCardContext::Build(CardAt(5,TEXT("Prototype.Arsenal.MikelMerino")));
 TestTrue(TEXT("Owning ThroughBall does not activate receiving Traits"),RunnerOnly.Skills.Num()==1 && RunnerOnly.Attributes.Contains(TEXT("PAS")) && RunnerOnly.Traits.IsEmpty());

 // Production safe projection for each viewer/attacker, all cards. No client-side
 // reconstruction of state and no changes to the existing network DTO.
 auto Demo = FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession Session; Session.InitializeMatch(Demo.OpeningInput); Session.BeginOrdinaryAttack(4);
 auto State=Session.GetStateSnapshot();
 for (auto Attacker : {EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB})
 for (auto Viewer : {EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB})
 {
  State.RuntimeState.CurrentAttackingPlayer=Attacker;
  auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State,Demo.SkillRuleSet,Viewer);
  auto Local=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,{},TEXT(""),Viewer);
  auto Network=FFMCodexNetworkMatchPresentationAdapter::Read(FFMCodexNetworkMatchPresentationAdapter::Project(V,Viewer),false);
  for (int32 R=0;R<2;++R)
  {
   const auto& L=R==0?Local.LocalRack:Local.OpponentRack;
   const auto& N=R==0?Network.LocalRack:Network.OpponentRack;
   TestEqual(TEXT("Both viewers keep complete roster"),L.Cells.Num(),N.Cells.Num());
   for(int32 I=0;I<FMath::Min(L.Cells.Num(),N.Cells.Num());++I)
   {
    const auto LC=FFMCodexAttackingCardContext::Build(L.Cells[I].Card), NC=FFMCodexAttackingCardContext::Build(N.Cells[I].Card);
    TestTrue(TEXT("Local/Network same context"),LC.Skills.Num()==NC.Skills.Num() && LC.Skills.Includes(NC.Skills) && LC.Traits.Num()==NC.Traits.Num() && LC.Traits.Includes(NC.Traits));
    for(const FName T:LC.Traits) TestTrue(TEXT("No Runner, free kick or defensive Trait"),T==TEXT("Trait.CrossCarrier")||T==TEXT("Trait.ThroughBallCarrier")||T==TEXT("Trait.LongShotCarrier")||T==TEXT("Trait.CutInsideCarrier"));
   }
  }
 }
 // Ordinary -> unrelated set piece -> no current attack cannot retain context.
 for (bool Active : {true,false})
 {
  State.bHasCurrentAttack=Active;
  State.CurrentAttack.RouteKind=EMatchPlayCurrentAttackRouteKind::SetPiece;
  auto V=FFMCodexLocalMatchInteractionViewBuilder::Build(State,Demo.SkillRuleSet);
  for(const auto* R:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for(const auto& P:*R)
   TestTrue(TEXT("Unrelated phase/ended attack clears preview"),FFMCodexAttackingCardContext::Build(FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(P)).Skills.IsEmpty());
 }
 return true;
}

// Keep the existing Full checks independently runnable from the Mini matrix.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackingContextFullRendering,
 "FMCodex.LocalPlay.UI.AttackingCardContext.FullCardRendering",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAttackingContextFullRendering::RunTest(const FString&)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W=CreateWidget<UFMCodexPlayerCardWidget>(World);auto Slate=W->TakeWidget();
 FWidgetRenderer Renderer(false);
 const FName Eze=TEXT("Prototype.Arsenal.EberechiEze");
 auto Full=CardAt(4,Eze);
 W->RefreshFromPresentation(Full);
 TestTrue(TEXT("SHO row emphasized"),Highlighted(W->FindAttributePresentationWidget(TEXT("SHO"))));
 TestTrue(TEXT("CON row emphasized"),Highlighted(W->FindAttributePresentationWidget(TEXT("CON"))));
 TestFalse(TEXT("PAS not emphasized"),Highlighted(W->FindAttributePresentationWidget(TEXT("PAS"))));
 for(const auto& S:Full.Skills)TestTrue(TEXT("Both actual Skill rows emphasized"),Highlighted(W->FindSkillPresentationWidget(S.SkillId)));
 TestTrue(TEXT("LS owned row emphasized"),Highlighted(W->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier"))));
 TestTrue(TEXT("CI owned row emphasized"),Highlighted(W->FindTraitPresentationWidget(TEXT("Trait.CutInsideCarrier"))));
 TestFalse(TEXT("Free kick row ordinary"),Highlighted(W->FindTraitPresentationWidget(TEXT("Trait.LongFreeKickTaker"))));
 const auto Colors=W->GetRenderedAttributeTierColors();
 const auto RankColor=CastChecked<UFMCodexFullCardSurface>(W->GetWidgetFromName(TEXT("FullCardOffensiveTraits0Badge")))->GetAccent();
 const TArray<FName> ColoredLabels={TEXT("SkillIdentity0"),TEXT("SkillIdentity1"),
  TEXT("FullCardOffensiveTraits0Name"),TEXT("FullCardOffensiveTraits1Name"),TEXT("AttributeLabel0"),TEXT("AttributeLabel2")};
 auto LabelColor=[&](FName Name){return CastChecked<UTextBlock>(W->GetWidgetFromName(Name))->GetColorAndOpacity().GetSpecifiedColor();};
 for(int32 I=0;I<ColoredLabels.Num();++I)
  TestTrue(TEXT("Approved text-only palette"),LabelColor(ColoredLabels[I]).ToFColor(true)==
   (I<2?FColor(0x8F,0xE6,0xC2):I<4?FColor(0x9B,0xD8,0xCF):FColor(0xA4,0xD4,0xCF)));
 const auto OrdinaryText=LabelColor(TEXT("AttributeLabel1"));
 TestTrue(TEXT("Unrelated free kick keeps ordinary text"),LabelColor(TEXT("FullCardOffensiveTraits2Name"))==OrdinaryText);
 auto* Target=Renderer.DrawWidget(Slate,FVector2D(360,540));
 TArray<FColor> Pixels;
 auto ReadBackgrounds=[&]()
 {
  Renderer.DrawWidget(Target,Slate,FVector2D(360,540),0.f);
  Pixels.Reset();
  FReadSurfaceDataFlags Flags;Flags.SetLinearToGamma(false);
  TestTrue(TEXT("Full Card paint readable"),Target->GameThread_GetRenderTargetResource()->ReadPixels(Pixels,Flags));
  TArray<FColor> Samples;
  // Blank interiors between the text and badges, away from the accent edges.
  const TArray<UWidget*> Rows={W->FindSkillPresentationWidget(Full.Skills[0].SkillId),
   W->FindTraitPresentationWidget(TEXT("Trait.LongShotCarrier")),W->FindAttributePresentationWidget(TEXT("SHO")),
   W->FindTraitPresentationWidget(TEXT("Trait.LongFreeKickTaker")),W->FindAttributePresentationWidget(TEXT("PAS"))};
  for(int32 I=0;I<Rows.Num();++I)
  {
   const auto P=Rows[I]->GetCachedGeometry().LocalToAbsolute(FVector2D(I==0?150.f:70.f,2.f));
   const int32 Index=FMath::RoundToInt(P.Y)*360+FMath::RoundToInt(P.X);
   TestTrue(TEXT("Background sample available"),Pixels.IsValidIndex(Index));
   Samples.Add(Pixels.IsValidIndex(Index)?Pixels[Index]:FColor::Black);
   const auto Edge=Rows[I]->GetCachedGeometry().LocalToAbsolute(FVector2D(2.65f,Rows[I]->GetCachedGeometry().GetLocalSize().Y*.5f));
   const int32 EdgeIndex=FMath::RoundToInt(Edge.Y)*360+FMath::RoundToInt(Edge.X);
   TestTrue(TEXT("Edge sample available"),Pixels.IsValidIndex(EdgeIndex));
   Samples.Add(Pixels.IsValidIndex(EdgeIndex)?Pixels[EdgeIndex]:FColor::Black);
  }
  return Samples;
 };
 const auto Emphasized=ReadBackgrounds();
 const auto EmphasizedPixels=Pixels;
 Full.HandMicroVisibleTacticalSkills.Reset();Full.HandMicroTacticalMatchCount=0;Full.bHasHandMicroTacticalMatch=false;
 W->RefreshFromPresentation(Full);
 TestTrue(TEXT("Context never alters attribute tier colors"),Colors==W->GetRenderedAttributeTierColors());
 TestTrue(TEXT("Context preserves Trait rank palette"),RankColor==CastChecked<UFMCodexFullCardSurface>(W->GetWidgetFromName(TEXT("FullCardOffensiveTraits0Badge")))->GetAccent());
 TestFalse(TEXT("Refresh clears emphasis"),Highlighted(W->FindAttributePresentationWidget(TEXT("SHO"))));
 const auto Ordinary=ReadBackgrounds();
 TestTrue(TEXT("Context never changes row background or accent edges"),Emphasized==Ordinary);
 for(FName Name:ColoredLabels)
 {
  TestTrue(TEXT("Refresh restores ordinary label color"),LabelColor(Name)==OrdinaryText);
  const auto& G=W->GetWidgetFromName(Name)->GetCachedGeometry();
  const auto P=G.GetAbsolutePosition(), Size=G.GetAbsoluteSize();
  bool bTextPaintChanged=false;
  for(int32 Y=FMath::Max(0,FMath::CeilToInt(P.Y));Y<FMath::Min(540,FMath::FloorToInt(P.Y+Size.Y));++Y)
   for(int32 X=FMath::Max(0,FMath::CeilToInt(P.X));X<FMath::Min(360,FMath::FloorToInt(P.X+Size.X));++X)
   {
    const int32 Index=Y*360+X;
    if(Pixels.IsValidIndex(Index) && EmphasizedPixels.IsValidIndex(Index) && Pixels[Index]!=EmphasizedPixels[Index]) bTextPaintChanged=true;
   }
  TestTrue(*FString::Printf(TEXT("Text color reaches rendered glyphs: %s"),*Name.ToString()),bTextPaintChanged);
 }
 W->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackingContextGeometry,
 "FMCodex.LocalPlay.UI.AttackingCardContext.RenderingAndMiniGeometry",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAttackingContextGeometry::RunTest(const FString&)
{
 UWorld* World=UWorld::CreateWorld(EWorldType::Game,false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W=CreateWidget<UFMCodexPlayerCardWidget>(World);auto Slate=W->TakeWidget();
 FWidgetRenderer Renderer(false);
 const auto Full=CardAt(4,TEXT("Prototype.Arsenal.EberechiEze"));
 auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession Session;Session.InitializeMatch(Demo.OpeningInput);Session.BeginOrdinaryAttack(4);
 const auto V=FFMCodexLocalMatchInteractionViewBuilder::Build(Session.GetStateSnapshot(),Demo.SkillRuleSet);
 for(const auto* Roster:{&V.PlayerACardRoster,&V.PlayerBCardRoster}) for(const auto& Source:*Roster)
 for(int32 Count:{0,1,2})
 {
  // Layout matrix uses actual public names/positions/art and the longest real
  // two-skill phrase, independent from the semantic matching assertions above.
  auto Card=FFMCodexLocalMatchUMGPresentationBuilder::BuildCard(Source);
  Card.PitchMiniVisibleTacticalSkills=Full.Skills;Card.PitchMiniVisibleTacticalSkills.SetNum(Count);
  Card.PitchMiniTacticalMatchCount=Count;Card.bHasPitchMiniTacticalMatch=Count>0;
  W->RefreshFromPresentation(Card,EFMCodexPlayerCardPresentationMode::PitchMini);
  auto* Target=Renderer.DrawWidget(Slate,FVector2D(136,140));Renderer.DrawWidget(Target,Slate,FVector2D(136,140),0.f);
  const auto* Names=CastChecked<UTextBlock>(W->GetWidgetFromName(TEXT("PitchMiniMatchingSkills")));
  TestEqual(TEXT("Exact ordered projected names"),Names->GetText().ToString(),FFMCodexAttackingCardContext::SkillNames(Card.PitchMiniVisibleTacticalSkills).ToString());
  TestEqual(TEXT("Fixed outer size"),W->GetConfiguredDimensions(),FVector2D(136,140));
  const auto* Name=W->GetWidgetFromName(TEXT("PitchMiniPlayerName"));
  const auto* Portrait=W->GetWidgetFromName(TEXT("PitchMiniPortraitBounds"));
  auto Top=[](const UWidget* N){return N->GetCachedGeometry().GetAbsolutePosition().Y;};
  auto Bottom=[&](const UWidget* N){return Top(N)+N->GetCachedGeometry().GetAbsoluteSize().Y;};
  TestTrue(TEXT("Portrait stays above identity"),Bottom(Portrait)<=Top(Name)+.1f);
  if(Count>0)
  {
   TestTrue(TEXT("Skills below name; no clipping"),Top(Names)>=Bottom(Name)-.1f && Bottom(Names)<=140.1f && Names->GetDesiredSize().X<=Names->GetCachedGeometry().GetLocalSize().X+.1f);
   const auto* Image=CastChecked<UImage>(W->GetWidgetFromName(TEXT("PitchMiniPortraitImage")));
   const FBox2f UV=Image->GetBrush().GetUVRegion();const auto Size=W->GetResolvedPortraitTexture()->GetImportedSize();
   TestTrue(TEXT("Reduced portrait remains undistorted"),FMath::IsNearlyEqual((UV.Max.X-UV.Min.X)*Size.X/((UV.Max.Y-UV.Min.Y)*Size.Y),130.f/96.f,.01f));
  }
  else TestFalse(TEXT("Zero has no empty label"),Names->IsVisible());
 }
 W->RemoveFromParent();GEngine->DestroyWorldContext(World);World->DestroyWorld(false);
 return true;
}
#endif
