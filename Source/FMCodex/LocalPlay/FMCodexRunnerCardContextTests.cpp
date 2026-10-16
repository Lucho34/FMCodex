#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexAttackingCardContext.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPitchSlotWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexFullCardSurface.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../NetworkPlay/FMCodexNetworkMatchPresentation.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/TextBlock.h"
#include "Slate/WidgetRenderer.h"

namespace
{
using Context = FFMCodexAttackingCardContext;
using Category = EFMCodexUMGInteractionCategory;
using Side = EInitialTurnOrderPlayer;
const FName Feet = TEXT("Trait.ThroughBallFeetRunner"), Behind = TEXT("Trait.ThroughBallBehindRunner"),
 Anti = TEXT("Trait.ThroughBallAntiRunner"), High = TEXT("Trait.CrossHighRunner"), Low = TEXT("Trait.CrossLowRunner");
FFMCodexLocalMatchInteractionView PublicView(Side Attacker = Side::PlayerA, Side Viewer = Side::PlayerA, bool Disclosed = true)
{
 const auto Demo = FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession Session; Session.InitializeMatch(Demo.OpeningInput); Session.BeginOrdinaryAttack(4);
 auto State = Session.GetStateSnapshot(); State.RuntimeState.CurrentAttackingPlayer = Attacker;
 FFMCodexLocalMatchViewerDisclosure Disclosure; Disclosure.bRevealInitialActionPointRoll = Disclosed;
 return FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(State, Demo.SkillRuleSet, Viewer, Disclosure);
}
FFMCodexUMGMatchScreenViewModel Model(const FFMCodexLocalMatchInteractionView& V, Side Viewer = Side::PlayerA)
{ return FFMCodexLocalMatchUMGPresentationBuilder::Build(V, {}, TEXT(""), Viewer); }
bool Highlighted(UWidget* W)
{ const auto* Surface = Cast<UFMCodexFullCardSurface>(W); return Surface && Surface->IsContextRelevant(); }
struct FFixture
{
 FFMCodexUMGMatchScreenViewModel M = Model(PublicView());
 FName Carrier, Runner;
 TMap<ESkillRuleType, FFMCodexUMGSkillViewModel> Skills;
 FFixture()
 {
  for (auto& Cell : M.LocalRack.Cells)
  {
   for (const auto& S : Cell.Card.Skills) Skills.Add(Context::SkillType(S.SkillId), S);
   Cell.Card.EligibleTacticalSkills.Reset(); Cell.Card.HandMicroVisibleTacticalSkills.Reset(); Cell.Card.PitchMiniVisibleTacticalSkills.Reset();
   Cell.Card.HandMicroTacticalMatchCount = Cell.Card.PitchMiniTacticalMatchCount = 0;
   Cell.Card.bHasHandMicroTacticalMatch = Cell.Card.bHasPitchMiniTacticalMatch = false;
   if (!Cell.Card.bGoalkeeper && Carrier.IsNone()) Carrier = Cell.Card.CardId;
   else if (!Cell.Card.bGoalkeeper && Runner.IsNone()) Runner = Cell.Card.CardId;
  }
  Card(Carrier).EligibleTacticalSkills = { Skills[ESkillRuleType::ThroughBall], Skills[ESkillRuleType::Cross] };
  auto& R = Card(Runner); R.Skills.Reset(); R.RankedTraits.Reset(); R.BinaryTraits = { Anti };
  for (FName Id : {Feet, Behind, High, Low}) R.RankedTraits.Add({Id, EPlayerTraitRank::A});
  M.Interaction.Category = Category::Deploy;
  M.Header.RawInitialD12 = 0; // No active reel in this public presentation fixture.
 }
 FFMCodexUMGCardViewModel& Card(FName Id)
 { return M.LocalRack.Cells.FindByPredicate([&](const auto& C) { return C.Card.CardId == Id; })->Card; }
 void Role(FName Id, EFMCodexUMGSelectedRole Role)
 {
  Card(Id).bDeployed = true;
  auto& Region = M.PitchRegions.AddDefaulted_GetRef(); auto& Slot = Region.Slots.AddDefaulted_GetRef();
  Slot.bOccupied = true; Slot.Card = Card(Id); Slot.Card.SelectedRole = Role;
 }
 Context Get() const { return Context::BuildRunner(M, Runner); }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunnerContextSemantics, "FMCodex.LocalPlay.UI.RunnerCardContext.SemanticsAndProjection",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRunnerContextSemantics::RunTest(const FString&)
{
 FFixture F;
 auto C = F.Get();
 TestTrue(TEXT("Receiver without initiating Skills previews all five owned traits and three attributes"),
  F.Card(F.Runner).Skills.IsEmpty() && C.Traits.Num() == 5 && C.Attributes.Num() == 3 && C.Skills.IsEmpty());
 F.Card(F.Runner).RankedTraits.Reset(); C = F.Get();
 TestTrue(TEXT("Binary only, no invented attribute"), C.Traits.Num() == 1 && C.Traits.Contains(Anti) && C.Attributes.IsEmpty());
 F.Card(F.Runner).BinaryTraits.Reset(); TestTrue(TEXT("Missing traits not invented"), F.Get().Traits.IsEmpty());
 F = FFixture(); F.Card(F.Carrier).bUsed = true; TestTrue(TEXT("Used initiator cannot enable preview"), F.Get().Traits.IsEmpty());
 F.Card(F.Carrier).bUsed = false; F.Card(F.Carrier).bAvailable = false;
 TestTrue(TEXT("Unavailable initiator cannot enable preview"), F.Get().Traits.IsEmpty());
 F = FFixture(); F.Role(F.Carrier, EFMCodexUMGSelectedRole::Carrier);
 TestTrue(TEXT("Carrier never counts as receiving Runner"), Context::BuildRunner(F.M, F.Carrier).Traits.IsEmpty());
 F.Card(F.Carrier).EligibleTacticalSkills = {F.Skills[ESkillRuleType::LongShot]};
 TestTrue(TEXT("Public Carrier narrows to its own available family"), F.Get().Traits.IsEmpty());
 F.Card(F.Carrier).EligibleTacticalSkills = {F.Skills[ESkillRuleType::ThroughBall], F.Skills[ESkillRuleType::Cross]};
 F.M.Interaction.Category = Category::SelectRunner;
 TestTrue(TEXT("After deployment hand-only player is no longer potential receiver"), F.Get().Traits.IsEmpty());
 F.Card(F.Runner).bDeployed = true;
 TestEqual(TEXT("Missing selected-Skill DTO safely retains Carrier possibilities"), F.Get().Traits.Num(), 5);
 F.M.ThroughBallResolution.bVisible = true;
 F.M.ThroughBallResolution.Route = EFMCodexUMGThroughBallRoute::Feet;
 TestEqual(TEXT("Undisclosed route never narrows"), F.Get().Traits.Num(), 3);
 F.M.ThroughBallResolution.bRouteRevealComplete = true;
 C = F.Get(); TestTrue(TEXT("Feet uses only receiving Control"), C.Traits.Num() == 1 && C.Traits.Contains(Feet) && C.Attributes.Contains(TEXT("CON")));
 F.M.ThroughBallResolution.Route = EFMCodexUMGThroughBallRoute::BehindDefense;
 C = F.Get(); TestTrue(TEXT("Behind P1 uses only receiving Speed"), C.Traits.Num() == 1 && C.Traits.Contains(Behind) && C.Attributes.Contains(TEXT("SPD")));
 F.M.ThroughBallResolution.Route = EFMCodexUMGThroughBallRoute::AntiOffside;
 C = F.Get(); TestTrue(TEXT("AntiOffside is binary and attribute-free"), C.Traits.Num() == 1 && C.Traits.Contains(Anti) && C.Attributes.IsEmpty());
 F.M.ThroughBallResolution.Stage = EFMCodexUMGThroughBallStage::OneOnOneChoice;
 TestTrue(TEXT("One-on-one no longer previews receiving traits"), F.Get().Traits.IsEmpty());
 F.M.ThroughBallResolution = {}; F.M.InlineFormula.ContestId = TEXT("Cross.Route");
 F.M.InlineFormula.SpatialCrossIntent = EMatchPlayElectiveBranchIntent::CrossHigh;
 TestEqual(TEXT("Cross intent is not actual route disclosure"), F.Get().Traits.Num(), 2);
 F.M.InlineFormula.ContestId = TEXT("Cross.High"); C = F.Get();
 TestTrue(TEXT("Cross High receiving Strength only"), C.Traits.Num() == 1 && C.Traits.Contains(High) && C.Attributes.Contains(TEXT("STR")));
 F.M.InlineFormula.ContestId = TEXT("Cross.Low"); C = F.Get();
 TestTrue(TEXT("Cross Low receiving Speed, never Corner Control"), C.Traits.Num() == 1 && C.Traits.Contains(Low) && C.Attributes.Contains(TEXT("SPD")) && !C.Attributes.Contains(TEXT("CON")));
 F.Role(F.Runner, EFMCodexUMGSelectedRole::Runner);
 TestEqual(TEXT("Bound actual receiver remains relevant"), F.Get().Traits.Num(), 1);
 F.M.PitchRegions.Last().Slots.Last().Card.CardId = TEXT("AnotherPublicRunner");
 TestTrue(TEXT("Non-selected receiver clears"), F.Get().Traits.IsEmpty());
 F = FFixture(); F.M.LongShotResolution.bVisible = true; F.M.LongShotResolution.SkillType = ESkillRuleType::LongShot;
 TestTrue(TEXT("Different public final Skill clears"), F.Get().Traits.IsEmpty());
 for (auto End : {Category::AdvanceAfterTerminal, Category::AttackComplete, Category::MatchEnded, Category::TacticalPointRoll})
 { F = FFixture(); F.M.Interaction.Category = End; TestTrue(TEXT("Lifecycle clears"), F.Get().Traits.IsEmpty()); }
 F = FFixture(); F.M.Header.bCurrentAttackerOnLeft = false; TestTrue(TEXT("Possession switch clears old attackers"), F.Get().Traits.IsEmpty());
 F = FFixture(); F.M.Header.RouteKind = EMatchPlayCurrentAttackRouteKind::SetPiece; TestTrue(TEXT("No Corner preview"), F.Get().Traits.IsEmpty());
 F = FFixture(); F.M.Header.bAttackActive = false; TestTrue(TEXT("Attack reset clears"), F.Get().Traits.IsEmpty());

 // Real shared safe projection through the unchanged reflected network payload.
 for (Side Attacker : {Side::PlayerA, Side::PlayerB}) for (Side Viewer : {Side::PlayerA, Side::PlayerB})
 {
  const auto V = PublicView(Attacker, Viewer); const auto Local = Model(V, Viewer);
  auto Net = FFMCodexNetworkMatchPresentationAdapter::Project(V, Viewer);
  TArray<uint8> Bytes; FMemoryWriter Writer(Bytes); FObjectAndNameAsStringProxyArchive Save(Writer, false);
  FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Save, &Net, nullptr);
  FFMCodexNetworkMatchPresentation Restored; FMemoryReader Reader(Bytes); FObjectAndNameAsStringProxyArchive Load(Reader, false);
  FFMCodexNetworkMatchPresentation::StaticStruct()->SerializeItem(Load, &Restored, nullptr);
  TestFalse(TEXT("Existing DTO wire roundtrip"), Save.IsError() || Load.IsError());
  const auto Remote = FFMCodexNetworkMatchPresentationAdapter::Read(Restored, true);
  const auto Hidden = Model(PublicView(Attacker, Viewer, false), Viewer);
  bool HasPreview = false;
  for (const auto* Rack : {&Local.LocalRack, &Local.OpponentRack}) for (const auto& Cell : Rack->Cells)
  {
   const auto A = Context::BuildRunner(Local, Cell.Card.CardId), B = Context::BuildRunner(Remote, Cell.Card.CardId);
   HasPreview |= !A.Traits.IsEmpty();
   TestTrue(TEXT("Both viewers / Local / network pending agree"), A.Traits.Num() == B.Traits.Num() && A.Traits.Includes(B.Traits) && A.Attributes.Num() == B.Attributes.Num());
   TestTrue(TEXT("No withheld TP hint"), Context::BuildRunner(Hidden, Cell.Card.CardId).Traits.IsEmpty());
  }
  TestTrue(TEXT("Real content parity exercises receiving traits"), HasPreview);
 }
 return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRunnerContextRendering, "FMCodex.LocalPlay.UI.RunnerCardContext.FullCardRendering",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FRunnerContextRendering::RunTest(const FString&)
{
 UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
 GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
 auto* W = CreateWidget<UFMCodexPlayerCardWidget>(World); auto Slate = W->TakeWidget(); FWidgetRenderer Renderer(false);
 FFixture F;
 for (const auto& Pair : TArray<TPair<FName, FName>>{{Feet, TEXT("CON")}, {Behind, TEXT("SPD")}, {High, TEXT("STR")}, {Low, TEXT("SPD")}, {Anti, NAME_None}})
 {
  auto Card = F.Card(F.Runner); Card.RankedTraits.Reset(); Card.BinaryTraits.Reset();
  if (Pair.Key == Anti) Card.BinaryTraits.Add(Anti); else Card.RankedTraits.Add({Pair.Key, EPlayerTraitRank::A});
  Card.RankedTraits.Add({TEXT("Trait.LongFreeKickTaker"), EPlayerTraitRank::B});
  F.Card(F.Runner) = Card;
  W->RefreshFromPresentation(Card); const auto Colors = W->GetRenderedAttributeTierColors();
  W->SetFullCardRunnerContext(F.Get()); Renderer.DrawWidget(Slate, FVector2D(360,540));
  TestTrue(TEXT("Owned receiving trait reaches Full Card row"), Highlighted(W->FindTraitPresentationWidget(Pair.Key)));
  const FName TraitLabel = Pair.Key == Anti ? TEXT("FullCardOffensiveTraits1Name") : TEXT("FullCardOffensiveTraits0Name");
  TestTrue(TEXT("Receiving trait uses accepted mint text"), CastChecked<UTextBlock>(W->GetWidgetFromName(TraitLabel))->GetColorAndOpacity().GetSpecifiedColor().ToFColor(true) == FColor(0x9B,0xD8,0xCF));
  if (!Pair.Value.IsNone()) TestTrue(TEXT("Corresponding attribute row highlighted"), Highlighted(W->FindAttributePresentationWidget(Pair.Value)));
  else for (FName Key : {FName(TEXT("SHO")), FName(TEXT("PAS")), FName(TEXT("CON")), FName(TEXT("STR")), FName(TEXT("SPD")), FName(TEXT("DEF"))})
   TestFalse(TEXT("Binary does not invent an attribute highlight"), Highlighted(W->FindAttributePresentationWidget(Key)));
  TestFalse(TEXT("Unrelated trait unchanged"), Highlighted(W->FindTraitPresentationWidget(TEXT("Trait.LongFreeKickTaker"))));
  TestTrue(TEXT("Base rating colors preserved"), Colors == W->GetRenderedAttributeTierColors());
  TestEqual(TEXT("Full geometry unchanged"), W->GetConfiguredDimensions(), FVector2D(360,540));
  W->RefreshFromPresentation(Card); TestFalse(TEXT("Rebind clears transient context"), Highlighted(W->FindTraitPresentationWidget(Pair.Key)));
  for (auto Mode : {EFMCodexPlayerCardPresentationMode::HandMicro, EFMCodexPlayerCardPresentationMode::PitchMini})
  {
   W->RefreshFromPresentation(Card, Mode); const int32 Count = W->GetRenderedSkillCount();
   W->SetFullCardRunnerContext(F.Get());
   TestEqual(TEXT("Compact indicators unchanged"), W->GetRenderedSkillCount(), Count);
   TestTrue(TEXT("Compact source has no new hint fields"), W->GetPresentation().DefensiveCounterTraits.IsEmpty() && W->GetPresentation().HandMicroVisibleTacticalSkills.IsEmpty() && W->GetPresentation().PitchMiniVisibleTacticalSkills.IsEmpty());
  }
 }
 // Receiving emphasis composes with independently valid Carrier and defensive rows.
 F = FFixture(); auto Combined = F.Card(F.Runner);
 Combined.RankedTraits = {{Feet, EPlayerTraitRank::A}, {TEXT("Trait.ThroughBallCarrier"), EPlayerTraitRank::B}, {TEXT("Trait.ThroughBallMarkerDefense"), EPlayerTraitRank::A}};
 Combined.HandMicroVisibleTacticalSkills = {F.Skills[ESkillRuleType::ThroughBall]};
 Combined.HandMicroTacticalMatchCount = 1; Combined.bHasHandMicroTacticalMatch = true;
 Combined.DefensiveCounterTraits = {TEXT("Trait.ThroughBallMarkerDefense")};
 F.Card(F.Runner) = Combined; W->RefreshFromPresentation(Combined); W->SetFullCardRunnerContext(F.Get());
 for (FName Id : {Feet, FName(TEXT("Trait.ThroughBallCarrier")), FName(TEXT("Trait.ThroughBallMarkerDefense"))})
  TestTrue(TEXT("Existing contexts remain independently emphasized"), Highlighted(W->FindTraitPresentationWidget(Id)));
 // Exercise the actual shared hover binding, first from Hand, then deployed Pitch.
 F = FFixture(); F.Card(F.Runner).RankedTraits = {{Feet, EPlayerTraitRank::A}}; F.Card(F.Runner).BinaryTraits.Reset();
 auto* Screen = CreateWidget<UFMCodexLocalMatchScreenWidget>(World); auto ScreenSlate = Screen->TakeWidget();
 Screen->RefreshFromPresentation(F.M);
 Renderer.DrawWidget(ScreenSlate, FVector2D(1920,1080));
 UFMCodexPlayerCardWidget* Hand = nullptr;
 for (UFMCodexPlayerCardWidget* Card : Screen->GetLocalRackWidget()->GetRenderedCardWidgets()) if (Card->GetPresentation().CardId == F.Runner) Hand = Card;
 if (TestNotNull(TEXT("Hand source"), Hand))
 {
  Hand->RequestFullCardDetailHover(); TestTrue(TEXT("Hand opens receiving Full Card"), Screen->IsDetailOverlayVisible() && Highlighted(Screen->GetDetailOverlayCard()->FindTraitPresentationWidget(Feet)));
 }
 F.Card(F.Runner).bDeployed = true;
 auto& Slot = F.M.PitchRegions[0].Slots[0]; Slot.bOccupied = true; Slot.Card = F.Card(F.Runner);
 Screen->RefreshFromPresentation(F.M);
 Renderer.DrawWidget(ScreenSlate, FVector2D(1920,1080));
 TestFalse(TEXT("Public refresh dismisses stale inspector"), Screen->IsDetailOverlayVisible());
 UFMCodexPlayerCardWidget* Pitch = nullptr;
 for (UFMCodexPitchSlotWidget* RenderedSlot : Screen->GetPitchWidget()->GetRenderedSlotWidgets())
  if (auto* Card = RenderedSlot->GetCardWidget(); Card && Card->GetPresentation().CardId == F.Runner) Pitch = Card;
 if (TestNotNull(TEXT("Pitch source"), Pitch))
 {
  Pitch->RequestFullCardDetailHover(); TestTrue(TEXT("Pitch opens identical receiving Full Card"), Screen->IsDetailOverlayVisible() && Highlighted(Screen->GetDetailOverlayCard()->FindTraitPresentationWidget(Feet)));
 }
 Screen->RemoveFromParent(); W->RemoveFromParent(); GEngine->DestroyWorldContext(World); World->DestroyWorld(false);
 return true;
}
#endif
