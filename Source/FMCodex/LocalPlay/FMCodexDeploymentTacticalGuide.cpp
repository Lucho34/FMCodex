#include "FMCodexDeploymentTacticalGuide.h"
#include "FMCodexAttackingCardContext.h"
#include "FMCodexTacticExplainerWidget.h"
#include "FMCodexMatchShellStyle.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "InputCoreTypes.h"

namespace
{
namespace Theme=FMCodexMatchShellStyle;
const FButtonStyle* ChipStyle(bool bSelected)
{
 static const FButtonStyle Normal=[]()
 {
  FButtonStyle S;
  S.SetNormal(FSlateRoundedBoxBrush(Theme::Navy(),5.f,Theme::Border(),1.f));
  S.SetHovered(FSlateRoundedBoxBrush(Theme::Color(18,48,62),5.f,Theme::Color(101,185,190),1.f));
  S.SetPressed(FSlateRoundedBoxBrush(Theme::Color(22,64,74),5.f,Theme::Mint(),1.f));
  S.SetNormalPadding(FMargin(16,0)); S.SetPressedPadding(FMargin(16,0)); return S;
 }();
 static const FButtonStyle Selected=[]()
 {
  auto S=Normal; S.SetNormal(FSlateRoundedBoxBrush(Theme::Color(18,57,68),5.f,Theme::Mint(),2.f));
  S.SetHovered(S.Normal); S.SetPressed(S.Normal); return S;
 }();
 return bSelected?&Selected:&Normal;
}
class FGuideInputObserver final : public IInputProcessor
{
 TWeakObjectPtr<UFMCodexDeploymentTacticalGuide> Owner;
public:
 explicit FGuideInputObserver(UFMCodexDeploymentTacticalGuide* InOwner):Owner(InOwner){}
 virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}
 virtual bool HandleMouseButtonDownEvent(FSlateApplication&,const FPointerEvent& Event) override
 { if(Owner.IsValid()) Owner->HandlePointerDown(Event.GetScreenSpacePosition()); return false; }
 virtual bool HandleKeyDownEvent(FSlateApplication&,const FKeyEvent& Event) override
 { return Event.GetKey()==EKeys::Escape && Owner.IsValid() && Owner->HandleEscape(); }
};
}

TArray<FName> UFMCodexDeploymentTacticalGuide::Project(const FFMCodexUMGMatchScreenViewModel& View)
{
 TArray<FName> Result;
 if(View.Interaction.Category!=EFMCodexUMGInteractionCategory::Deploy
  || !View.Header.bMatchActive || !View.Header.bAttackActive || !View.Header.bHasCurrentAttacker
  || View.Header.bMatchEnded || View.FullTime.bVisible
  || View.Header.CurrentAttackerTacticalPoints<=0 || View.Header.RouteKind!=EMatchPlayCurrentAttackRouteKind::Ordinary) return Result;
 TSet<ESkillRuleType> Types;
 // Consume the same illuminated lists as the cards, including a deployed card
 // whose hand representation no longer carries Hand Micro matches.
 auto Read=[&](const FFMCodexUMGCardViewModel& Card)
 {
  if(Card.bGoalkeeper || Card.bUsed || (!Card.bAvailable && !Card.bDeployed)) return;
  for(const auto* Matches:{&Card.HandMicroVisibleTacticalSkills,&Card.PitchMiniVisibleTacticalSkills})
   for(const auto& Skill:*Matches) Types.Add(FFMCodexAttackingCardContext::SkillType(Skill.SkillId));
 };
 const auto& Rack=View.Header.bCurrentAttackerOnLeft?View.LocalRack:View.OpponentRack;
 for(const auto& Cell:Rack.Cells) Read(Cell.Card);
 for(const auto& Region:View.PitchRegions) for(const auto& Slot:Region.Slots)
  if(Slot.bOccupied && Rack.Cells.ContainsByPredicate([&](const auto& C){return C.Card.CardId==Slot.Card.CardId;})) Read(Slot.Card);
 // Catalog order is the existing reference order; identity, never localized text, deduplicates.
 for(const auto& T:FFMCodexTacticExplainerCatalog::Get())
  if(!T.bSetPiece && Types.Contains(T.Skill)) Result.Add(T.Id);
 return Result;
}

void UFMCodexDeploymentTacticalGuide::Refresh(const FFMCodexUMGMatchScreenViewModel& View,bool bResultDismissed)
{
 // Local hot-seat swaps left/right after deployment; orientation is not attack identity.
 const auto Left=View.Header.LeftPlayerSide;
 const auto CurrentAttacker=View.Header.bCurrentAttackerOnLeft?Left:
  Left==EInitialTurnOrderPlayer::PlayerA?EInitialTurnOrderPlayer::PlayerB:EInitialTurnOrderPlayer::PlayerA;
 const bool bNewContext=Sequence!=View.Header.AttackSequence || TP!=View.Header.CurrentAttackerTacticalPoints
  || Attacker!=CurrentAttacker;
 if(bNewContext)
 {
  Close(); Sequence=View.Header.AttackSequence; TP=View.Header.CurrentAttackerTacticalPoints;
  Attacker=CurrentAttacker; bSeenContext=false;
 }
 auto Next=bResultDismissed?Project(View):TArray<FName>();
 if(!Selection.IsNone() && !Next.Contains(Selection)) Close();
 if(Next!=Tactics) { Tactics=MoveTemp(Next); RefreshChips(); }
 SetVisibility(Tactics.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::Visible);
 if(bResultDismissed && View.Interaction.Category==EFMCodexUMGInteractionCategory::Deploy && !bSeenContext)
 {
  bSeenContext=true; EntranceElapsed=Tactics.IsEmpty()?.4f:0.f; bFirstEntranceFrame=true;
 }
 if(Chips) Chips->SetRenderOpacity(GetEntranceProgress());
}
float UFMCodexDeploymentTacticalGuide::GetEntranceProgress() const { return FMath::Clamp(EntranceElapsed/.4f,0.f,1.f); }
void UFMCodexDeploymentTacticalGuide::AdvanceEntrance(float DeltaSeconds,FVector2D ViewportSize)
{
 if(Tactics.IsEmpty() || EntranceElapsed>=.4f) return;
 if(bFirstEntranceFrame) bFirstEntranceFrame=false;
 else EntranceElapsed+=FMath::Max(0.f,DeltaSeconds);
 if(Chips)
 {
  Chips->SetRenderOpacity(GetEntranceProgress());
  Chips->SetRenderTransform(FSlateRenderTransform(FVector2D(0,4.f*(1.f-GetEntranceProgress()))));
 }
}
void UFMCodexDeploymentTacticalGuide::Toggle(FName Id)
{
 if(!Tactics.Contains(Id)) return;
 if(Selection==Id) { Close(); return; }
 Selection=Id;
 if(Explainer) Explainer->SelectTactic(Id);
 RefreshChips();
 if(Explainer) Explainer->SetVisibility(ESlateVisibility::Visible);
 if(!InputObserver && FSlateApplication::IsInitialized())
 { InputObserver=MakeShared<FGuideInputObserver>(this); FSlateApplication::Get().RegisterInputPreProcessor(InputObserver,0); }
}
void UFMCodexDeploymentTacticalGuide::StopObservingInput()
{
 if(InputObserver && FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(InputObserver);
 InputObserver.Reset();
}
void UFMCodexDeploymentTacticalGuide::Close()
{
 Selection=NAME_None; if(Explainer) Explainer->SetVisibility(ESlateVisibility::Collapsed); StopObservingInput(); RefreshChips();
}
void UFMCodexDeploymentTacticalGuide::ResetContext()
{ Close(); Tactics.Reset(); Sequence=INDEX_NONE; TP=0; bSeenContext=false; EntranceElapsed=.4f; RefreshChips(); SetVisibility(ESlateVisibility::Collapsed); }
bool UFMCodexDeploymentTacticalGuide::HandlePointerDown(FVector2D Position)
{
 if(!Selection.IsNone() && !GetCachedGeometry().IsUnderLocation(Position)
  && !(Explainer && Explainer->GetCachedGeometry().IsUnderLocation(Position))) Close();
 return false;
}
bool UFMCodexDeploymentTacticalGuide::HandleEscape()
{ if(Selection.IsNone()) return false; Close(); return true; }
TSharedPtr<SButton> UFMCodexDeploymentTacticalGuide::GetChip(FName Id) const { return Buttons.FindRef(Id); }
FGeometry UFMCodexDeploymentTacticalGuide::GetDetailGeometry() const { return Explainer?Explainer->GetCachedGeometry():FGeometry(); }
void UFMCodexDeploymentTacticalGuide::ReleaseSlateResources(bool bReleaseChildren)
{
 Close(); Super::ReleaseSlateResources(bReleaseChildren); Chips.Reset(); Buttons.Reset();
}
void UFMCodexDeploymentTacticalGuide::AttachDetailLayer(UCanvasPanel* Layer)
{
 if(!Explainer)
 {
  Explainer=NewObject<UFMCodexTacticExplainerWidget>(this); Explainer->SetEmbedded(true);
  Explainer->OnClose.AddUObject(this,&UFMCodexDeploymentTacticalGuide::Close);
  Explainer->SetVisibility(ESlateVisibility::Collapsed);
 }
 if(Layer && Explainer->GetParent()!=Layer) Layer->AddChildToCanvas(Explainer);
}
void UFMCodexDeploymentTacticalGuide::UpdateDetailBounds(const FGeometry& Screen,const FGeometry& Pitch)
{
 if(!Explainer || Pitch.GetLocalSize().IsNearlyZero()) return;
 if(auto* DetailSlot=Cast<UCanvasPanelSlot>(Explainer->Slot))
 {
  const FVector2D Top=Screen.AbsoluteToLocal(Pitch.GetAbsolutePosition());
  const FVector2D Bottom=Screen.AbsoluteToLocal(Pitch.LocalToAbsolute(Pitch.GetLocalSize()));
  const FVector2D Available=Bottom-Top-FVector2D(16,16);
  const FVector2D Size(FMath::Min(880.f,Available.X),FMath::Min(800.f,Available.Y));
  DetailSlot->SetSize(Size); DetailSlot->SetPosition(FVector2D((Top.X+Bottom.X-Size.X)*.5f,Bottom.Y-8-Size.Y));
 }
}
TSharedRef<SWidget> UFMCodexDeploymentTacticalGuide::RebuildWidget()
{
 AttachDetailLayer(nullptr);
 auto Result=SAssignNew(Chips,SHorizontalBox);
 RefreshChips(); return Result;
}
void UFMCodexDeploymentTacticalGuide::RefreshChips()
{
 if(!Chips) return;
 // Keep mounted buttons when selection changes, retaining the click target/capture.
 bool bSame=Buttons.Num()==Tactics.Num(); for(FName Id:Tactics) bSame &= Buttons.Contains(Id);
 if(!bSame)
 {
  Chips->ClearChildren(); Buttons.Reset();
  for(FName Id:Tactics)
  {
   const auto* T=FFMCodexTacticExplainerCatalog::Find(Id);
   auto Button=SNew(SButton).ButtonStyle(ChipStyle(Id==Selection)).IsFocusable(true).HAlign(HAlign_Center).VAlign(VAlign_Center)
    .OnClicked_Lambda([this,Id](){Toggle(Id);return FReply::Handled();})
    [SNew(STextBlock).Text(T->Name).Justification(ETextJustify::Center).Font(Theme::Font(18)).ColorAndOpacity(Theme::Mint())];
   Buttons.Add(Id,Button); Chips->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(8,0,0,0)
    [SNew(SBox).HeightOverride(44)[Button]];
  }
 }
 for(const auto& Button:Buttons) Button.Value->SetButtonStyle(ChipStyle(Button.Key==Selection));
 Chips->SetRenderOpacity(GetEntranceProgress());
}
