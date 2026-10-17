#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "FMCodexDeploymentTacticalGuide.h"
#include "FMCodexAttackingCardContext.h"
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexInteractionPanelWidget.h"
#include "FMCodexLocalMatchScreenWidget.h"
#include "FMCodexLocalMatchDemoConfiguration.h"
#include "FMCodexLocalMatchInteractionView.h"
#include "FMCodexLocalMatchResolutionFeedback.h"
#include "../MatchPlayRuntime/MatchPlayAuthoritativeSession.h"
#include "../NetworkPlay/FMCodexNetworkMatchPresentation.h"
#include "Misc/AutomationTest.h"
#include "Components/Button.h"
#include "Slate/WidgetRenderer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Components/Overlay.h"
#include "FMCodexCardRackWidget.h"
#include "FMCodexPlayerCardWidget.h"
#include "FMCodexPitchWidget.h"
#include "FMCodexPitchSlotWidget.h"

namespace
{
TSharedPtr<SWidget> FindGuideNode(const TSharedRef<SWidget>& Root,const FString& Type)
{
 if(Root->GetTypeAsString()==Type) return Root;
 auto* Children=Root->GetChildren();
 for(int32 I=0;I<Children->Num();++I) if(auto Found=FindGuideNode(Children->GetChildAt(I),Type)) return Found;
 return nullptr;
}
FFMCodexUMGMatchScreenViewModel GuideModel()
{
 const auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create();
 FMatchPlayAuthoritativeSession S; S.InitializeMatch(Demo.OpeningInput); S.BeginOrdinaryAttack(4);
 FFMCodexLocalMatchViewerDisclosure D; D.bRevealInitialActionPointRoll=true;
 const auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(S.GetStateSnapshot(),Demo.SkillRuleSet,EInitialTurnOrderPlayer::PlayerA,D);
 auto M=FFMCodexLocalMatchUMGPresentationBuilder::Build(V,{},TEXT("")); M.Header.RawInitialD12=0; return M;
}
void SetGuideCount(FFMCodexUMGMatchScreenViewModel& M,int32 Count)
{
 TMap<ESkillRuleType,FFMCodexUMGSkillViewModel> Skills;
 for(const auto& Cell:M.LocalRack.Cells) for(const auto& S:Cell.Card.Skills) Skills.Add(FFMCodexAttackingCardContext::SkillType(S.SkillId),S);
 int32 Index=0;
 for(auto& Cell:M.LocalRack.Cells)
 {
  auto& C=Cell.Card; C.HandMicroVisibleTacticalSkills.Reset(); C.PitchMiniVisibleTacticalSkills.Reset();
  C.HandMicroTacticalMatchCount=0; C.bHasHandMicroTacticalMatch=false;
  if(C.bGoalkeeper || Index>=Count) continue;
  const auto Type=FFMCodexTacticExplainerCatalog::Get()[Index++].Skill;
  C.HandMicroVisibleTacticalSkills={Skills[Type]}; C.HandMicroTacticalMatchCount=1; C.bHasHandMicroTacticalMatch=true;
 }
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDeploymentGuideProjection,"FMCodex.LocalPlay.UI.DeploymentTacticalGuide.ProjectionAndLifecycle",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDeploymentGuideProjection::RunTest(const FString&)
{
 using Guide=UFMCodexDeploymentTacticalGuide;
 auto M=GuideModel(); const auto Expected=TArray<FName>{TEXT("LongShot"),TEXT("CutInside"),TEXT("Cross")};
 TestTrue(TEXT("TP4 illuminated union, duplicates removed in reference order"),Guide::Project(M)==Expected);
 const auto Original=M;
 for(auto& Cell:M.LocalRack.Cells)
 {
  Cell.Card.bDeployed=true; Cell.Card.PitchMiniVisibleTacticalSkills=Cell.Card.HandMicroVisibleTacticalSkills;
  Cell.Card.HandMicroVisibleTacticalSkills.Reset();
 }
 TestTrue(TEXT("Hand to field preserves every family"),Guide::Project(M)==Expected);
 for(auto& Cell:M.LocalRack.Cells) Cell.Card.bUsed=true;
 TestTrue(TEXT("Used/unavailable cards never enlarge pool"),Guide::Project(M).IsEmpty());
 M=Original; M.Header.CurrentAttackerTacticalPoints=0; TestTrue(TEXT("No public TP"),Guide::Project(M).IsEmpty());
 M=Original; M.Header.RouteKind=EMatchPlayCurrentAttackRouteKind::SetPiece; TestTrue(TEXT("No set-piece tactic invention"),Guide::Project(M).IsEmpty());
 M.Header.RouteKind=EMatchPlayCurrentAttackRouteKind::SendingOff; TestTrue(TEXT("No ejection options"),Guide::Project(M).IsEmpty());
 M=Original; SetGuideCount(M,0); TestTrue(TEXT("Zero matches has no placeholder"),Guide::Project(M).IsEmpty());
 SetGuideCount(M,4); TestEqual(TEXT("Four distinct families"),Guide::Project(M).Num(),4);
 auto* W=NewObject<Guide>(); W->TakeWidget(); W->Refresh(M,false);
 TestTrue(TEXT("TP result still visible hides chips"),W->GetTactics().IsEmpty());
 W->Refresh(M,true); TestEqual(TEXT("Fresh dismissed context starts at zero"),W->GetEntranceProgress(),0.f);
 W->AdvanceEntrance(5.f,FVector2D(1920,1080)); TestEqual(TEXT("First frame spends no pre-dismissal time"),W->GetEntranceProgress(),0.f);
 W->AdvanceEntrance(.2f,FVector2D(1920,1080)); TestEqual(TEXT("Half of 0.4s cue"),W->GetEntranceProgress(),.5f);
 W->Refresh(M,true); TestEqual(TEXT("Repaint does not restart cue"),W->GetEntranceProgress(),.5f);
 W->AdvanceEntrance(.2f,FVector2D(1920,1080));
 W->ReleaseSlateResources(true); W->TakeWidget(); W->Refresh(M,true);
 TestEqual(TEXT("Reconstruction of same public context does not replay"),W->GetEntranceProgress(),1.f);
 Swap(M.LocalRack,M.OpponentRack); M.Header.LeftPlayerSide=EInitialTurnOrderPlayer::PlayerB; M.Header.bCurrentAttackerOnLeft=false;
 W->Refresh(M,true); TestEqual(TEXT("Hot-seat orientation swap is the same attacking context"),W->GetEntranceProgress(),1.f);
 M.Interaction.Category=EFMCodexUMGInteractionCategory::SelectCarrier; W->Refresh(M,true);
 TestTrue(TEXT("Leaving deployment clears"),W->GetTactics().IsEmpty() && W->GetSelection().IsNone());
 W->ResetContext(); W->Refresh(Original,true); TestEqual(TEXT("New session can reveal same TP again"),W->GetEntranceProgress(),0.f);
 // Unchanged public adapter, both viewers; no action/legality recomputation.
 const auto Demo=FFMCodexLocalMatchDemoConfigurationFactory::Create(); FMatchPlayAuthoritativeSession S;
 S.InitializeMatch(Demo.OpeningInput); S.BeginOrdinaryAttack(4);
 for(auto Side:{EInitialTurnOrderPlayer::PlayerA,EInitialTurnOrderPlayer::PlayerB})
 {
  FFMCodexLocalMatchViewerDisclosure D; D.bRevealInitialActionPointRoll=true;
  auto V=FFMCodexLocalMatchInteractionViewBuilder::BuildForViewer(S.GetStateSnapshot(),Demo.SkillRuleSet,Side,D);
  const auto Net=FFMCodexNetworkMatchPresentationAdapter::Read(FFMCodexNetworkMatchPresentationAdapter::Project(V,Side),false);
  TestTrue(TEXT("Waiting and acting viewers share the same public union"),Guide::Project(Net)==Expected);
 }
 W->ReleaseSlateResources(true); return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDeploymentGuideInteraction,"FMCodex.LocalPlay.UI.DeploymentTacticalGuide.InteractionAndGeometry",
 EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDeploymentGuideInteraction::RunTest(const FString&)
{
 struct FBackend:IFMCodexMatchScreenBackend
 {
  int32 Count=0;
  EFMCodexMatchScreenSubmission SubmitScreenIntent(const FFMCodexMatchScreenRequest&) override {++Count;return EFMCodexMatchScreenSubmission::Completed;}
  bool IsScreenIntentPending() const override {return false;}
 } Backend;
 auto* S=NewObject<UFMCodexLocalMatchScreenWidget>(); auto Root=S->TakeWidget(); S->SetMatchBackend(&Backend);
 auto M=GuideModel(); FWidgetRenderer Renderer(false);
 for(int32 Count:{0,1,4})
 {
  SetGuideCount(M,Count); S->RefreshFromPresentation(M); Renderer.DrawWidget(Root,FVector2D(1920,1080));
  auto* G=S->GetInteractionPanel()->GetTacticalGuide();
  TestEqual(TEXT("Rendered chip count"),G->GetTactics().Num(),Count);
  auto* Finish=CastChecked<UButton>(S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionFinishDeploymentButton")));
  TestTrue(TEXT("Finish remains enabled primary action"),Finish->IsVisible() && Finish->GetIsEnabled());
  TestEqual(TEXT("Normal deployment reference entry removed"),S->GetInteractionPanel()->GetWidgetFromName(TEXT("DeploymentTacticalReferenceEntryButton"))->GetVisibility(),ESlateVisibility::Collapsed);
  float Right=Finish->GetCachedGeometry().GetAbsolutePosition().X+Finish->GetCachedGeometry().GetAbsoluteSize().X;
  for(FName Id:G->GetTactics())
  {
   auto Chip=G->GetChip(Id); const auto Geo=Chip->GetCachedGeometry();
   TestTrue(TEXT("Chips ordered after Finish without overlap and inside viewport"),Geo.GetAbsolutePosition().X>=Right-1.f && Geo.GetAbsolutePosition().X+Geo.GetAbsoluteSize().X<=1920.f);
   TestTrue(TEXT("Compact readable chip height"),Geo.GetLocalSize().Y>=40 && Geo.GetLocalSize().Y<=50);
   const auto Label=FindGuideNode(Chip.ToSharedRef(),TEXT("STextBlock"));
   const auto TextGeo=Label->GetCachedGeometry();
   TestTrue(TEXT("Label centered on both axes without clipping"),
    Geo.LocalToAbsolute(Geo.GetLocalSize()*.5f).Equals(TextGeo.LocalToAbsolute(TextGeo.GetLocalSize()*.5f),.5f)
    && TextGeo.GetLocalSize().Y>=Label->GetDesiredSize().Y-.5f);
   Right=Geo.GetAbsolutePosition().X+Geo.GetAbsoluteSize().X;
  }
 }
 auto* G=S->GetInteractionPanel()->GetTacticalGuide();
 auto DrawGuide=[&]()
 {
  auto* Pitch=S->GetWidgetFromName(TEXT("CentralPitchRegion"));
  check(Pitch);
  G->UpdateDetailBounds(S->GetCachedGeometry(),Pitch->GetCachedGeometry());
  Renderer.DrawWidget(Root,FVector2D(1920,1080)); Renderer.DrawWidget(Root,FVector2D(1920,1080));
 };
 for(const auto& Tactic:FFMCodexTacticExplainerCatalog::Get())
 {
  if(Tactic.bSetPiece) continue;
  G->Close(); G->Toggle(Tactic.Id);
  for(const auto& Route:Tactic.Routes) for(bool Details:{false,true})
  {
   G->GetExplainer()->SelectRoute(Route.Id); G->GetExplainer()->ShowDetails(Details); DrawGuide();
   const auto Scroll=StaticCastSharedPtr<SScrollBox>(FindGuideNode(G->GetExplainer()->TakeWidget(),TEXT("SScrollBox")));
   TestTrue(FString::Printf(TEXT("Desktop %s %s fits without scrolling (overflow %.1f)"),*Route.Id.ToString(),Details?TEXT("calculation"):TEXT("overview"),Scroll->GetScrollOffsetOfEnd()),Scroll->GetScrollOffsetOfEnd()<1.f);
  }
 }
 const auto Panel=G->GetDetailGeometry();
 const auto Left=S->GetLocalRackWidget()->GetCachedGeometry(),RightRack=S->GetOpponentRackWidget()->GetCachedGeometry();
 auto* Finish=S->GetInteractionPanel()->GetWidgetFromName(TEXT("InteractionFinishDeploymentButton"));
 TestTrue(TEXT("Panel stays between hands and above primary controls"),
  Panel.GetAbsolutePosition().X>=Left.LocalToAbsolute(Left.GetLocalSize()).X
  && Panel.LocalToAbsolute(Panel.GetLocalSize()).X<=RightRack.GetAbsolutePosition().X
  && Panel.LocalToAbsolute(Panel.GetLocalSize()).Y<=Finish->GetCachedGeometry().GetAbsolutePosition().Y
  && Panel.GetAbsolutePosition().Y>=128.f);
 auto* GuideLayer=S->GetWidgetFromName(TEXT("DeploymentGuideLayer"));
 auto* CardLayer=S->GetWidgetFromName(TEXT("FullCardDetailOverlayCanvas"));
 auto* Overlay=CastChecked<UOverlay>(GuideLayer->GetParent());
 TestTrue(TEXT("Same production overlay paints Full Card above guide; no popup layer"),CardLayer->GetParent()==Overlay
  && Overlay->GetChildIndex(CardLayer)>Overlay->GetChildIndex(GuideLayer)
  && G->GetExplainer()->GetParent()==GuideLayer
  && GuideLayer->GetVisibility()==ESlateVisibility::SelfHitTestInvisible);
 const auto Selection=G->GetSelection();
 for(auto* Rack:{S->GetLocalRackWidget(),S->GetOpponentRackWidget()})
 {
  auto* Card=Rack->GetRenderedCardWidgets()[0].Get(); Card->RequestFullCardDetailHover(); DrawGuide();
  TestTrue(TEXT("Hand hover opens original 360x540 inspector above persistent guide"),S->IsDetailOverlayVisible()
   && S->GetDetailOverlayCard()->GetConfiguredDimensions()==FVector2D(360,540) && G->GetSelection()==Selection);
  Card->OnDetailHoverDismissed.Broadcast(Card);
  TestTrue(TEXT("Inspection dismissal retains guide"),!S->IsDetailOverlayVisible() && G->GetSelection()==Selection);
 }
 auto& Deployed=M.PitchRegions[0].Slots[0]; Deployed.bOccupied=true; Deployed.Card=M.LocalRack.Cells[1].Card; Deployed.Card.bDeployed=true;
 S->RefreshFromPresentation(M); DrawGuide();
 bool bCheckedPitch=false;
 for(UFMCodexPitchSlotWidget* Slot:S->GetPitchWidget()->GetRenderedSlotWidgets())
  if(auto* Card=Slot->GetCardWidget();Card && Card->GetPresentation().CardId==Deployed.Card.CardId)
  {
   Card->RequestFullCardDetailHover(); DrawGuide(); bCheckedPitch=true;
   TestTrue(TEXT("Pitch Mini uses same upper Full Card while guide persists"),S->IsDetailOverlayVisible() && G->GetSelection()==Selection);
   Card->OnDetailHoverDismissed.Broadcast(Card); break;
  }
 TestTrue(TEXT("Deployed inspection exercised"),bCheckedPitch);
 G->Close();
 auto Long=G->GetChip(TEXT("LongShot"));
 const auto BeforeText=FindGuideNode(Long.ToSharedRef(),TEXT("STextBlock"))->GetCachedGeometry().GetAbsolutePosition();
 FPointerEvent Pointer(0,FVector2D(1,1),FVector2D(1,1),TSet<FKey>(),EKeys::Invalid,0,FModifierKeysState());
 Long->OnMouseEnter(Long->GetCachedGeometry(),Pointer); TestTrue(TEXT("Hover never opens details"),G->GetSelection().IsNone()); Long->OnMouseLeave(Pointer);
 G->Toggle(TEXT("LongShot")); TestEqual(TEXT("Click selects own explanation"),G->GetExplainer()->GetTacticId(),FName(TEXT("LongShot")));
 DrawGuide();
 TestTrue(TEXT("Selected label does not shift"),FindGuideNode(Long.ToSharedRef(),TEXT("STextBlock"))->GetCachedGeometry().GetAbsolutePosition().Equals(BeforeText,.5f));
 G->GetExplainer()->ShowDetails(true); TestTrue(TEXT("Complete established calculation content reused"),G->GetExplainer()->CollectPlayerFacingText().Contains(TEXT("固定防守修正 +3")));
 S->RefreshFromPresentation(M); TestEqual(TEXT("Refresh retains persistent selection"),G->GetSelection(),FName(TEXT("LongShot")));
 G->Toggle(TEXT("Cross")); TestEqual(TEXT("Different chip replaces one panel"),G->GetExplainer()->GetTacticId(),FName(TEXT("Cross")));
 G->Toggle(TEXT("Cross")); TestTrue(TEXT("Same chip toggles closed"),G->GetSelection().IsNone());
 G->Toggle(TEXT("ThroughBall")); TestTrue(TEXT("Esc closes open explanation"),G->HandleEscape()); TestFalse(TEXT("Esc falls through when closed"),G->HandleEscape());
 G->Toggle(TEXT("Cross")); TestFalse(TEXT("Outside click never consumes gameplay input"),G->HandlePointerDown(FVector2D(-100,-100))); TestTrue(TEXT("Outside closes"),G->GetSelection().IsNone());
 G->Toggle(TEXT("Cross")); SetGuideCount(M,1); S->RefreshFromPresentation(M); TestTrue(TEXT("Removed public option clears selection"),G->GetSelection().IsNone());
 G->Toggle(TEXT("LongShot")); TestEqual(TEXT("Information interactions issued no gameplay commands"),Backend.Count,0);
 S->RequestFinishDeployment(); TestEqual(TEXT("Finish remains usable with explanation open"),Backend.Count,1);
 G->Close(); S->SetMatchBackend(nullptr); S->ReleaseSlateResources(true); return true;
}
#endif
