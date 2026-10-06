#include "FMCodexTacticalScene.h"
#include "FMCodexMatchShellStyle.h"
#include "FMCodexPlayerUIAssetReferences.h"
#include "FMCodexPitchWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/NativeWidgetHost.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/OverlaySlot.h"
#include "Components/RichTextBlock.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/DrawElements.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/SlateBrush.h"
#include "Styling/CoreStyle.h"
#include "Engine/DataTable.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SLeafWidget.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

namespace FMCodexTacticalScene
{
bool IsCross(EMethod Method) { return Method==EMethod::CrossHigh || Method==EMethod::CrossLow; }
FFacts ProjectCross(const FFMCodexUMGInlineFormulaSurfaceViewModel& View,
 const FFMCodexUMGMatchHeaderViewModel& Header,bool bAllowed)
{
 FFacts F;
 F.bMethodChoice=View.ContestId==TEXT("Cross.Setup");
 F.bRoutePending=View.ContestId==TEXT("Cross.Route");
 F.bActive=bAllowed && (F.bMethodChoice || (View.bVisible && (F.bRoutePending
  || View.ContestId==TEXT("Cross.High") || View.ContestId==TEXT("Cross.Low"))));
 if(!F.bActive)return F;
 F.AttackSequence=Header.AttackSequence;
 // During route reveal retain the player's intent; the displayed Formula
 // switches to the actual route only after the existing route reveal finishes.
 F.Method=(F.bMethodChoice || F.bRoutePending
  ?View.SpatialCrossIntent==EMatchPlayElectiveBranchIntent::CrossLow:View.ContestId==TEXT("Cross.Low"))
  ?EMethod::CrossLow:EMethod::CrossHigh;
 using R=EMatchPlayResolutionParticipantRole;
 for(const auto* Row:{&View.AttackRow,&View.DefenseRow}) for(const auto& P:Row->Participants)
 {
  if(P.CardId.IsNone() || P.Role==R::None)continue;
  if(F.Participants.ContainsByPredicate([&](const auto& Existing){return Existing.Role==P.Role;}))continue;
  const auto& Tracker=Row->Side==Header.LeftPlayerSide?Header.LeftAttackTurnTracker:Header.RightAttackTurnTracker;
  F.Participants.Add({P.CardId,P.Role,Row->Side,FText::FromString(P.PlayerName),
   FMCodexMatchShellStyle::HeaderAccent(Tracker.PrimarySideColor),!F.bMethodChoice && !F.bRoutePending});
 }
 const auto& Keeper=View.SpatialGoalkeeper;
 if(!Keeper.CardId.IsNone() && !F.Participants.ContainsByPredicate([](const auto& P){return P.Role==R::Goalkeeper;}))
 {
  const auto Side=View.DefenseRow.Side;
  const auto& Tracker=Side==Header.LeftPlayerSide?Header.LeftAttackTurnTracker:Header.RightAttackTurnTracker;
  F.Participants.Add({Keeper.CardId,R::Goalkeeper,Side,FText::FromString(Keeper.PlayerName),
   FMCodexMatchShellStyle::HeaderAccent(Tracker.PrimarySideColor),false});
 }
 if(!F.bMethodChoice && !F.bRoutePending)
 {
  F.Highlight=View.bAttackRowActive?R::Carrier:View.bDefenseRowActive?R::Marker:R::None;
  if(View.bNarrativeAvailable && !View.bDiceRevealVisible)
   F.Outcome=View.SpatialOutcome==EMatchPlayResolutionDecisionOutcome::Goal?EOutcome::Goal
    :View.SpatialOutcome==EMatchPlayResolutionDecisionOutcome::Miss?EOutcome::DefensiveSuccess:EOutcome::None;
 }
 return F;
}
FFacts Project(const FFMCodexUMGLongShotResolutionViewModel& Shot,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed)
{
 FFacts F;
 F.bActive=bAllowed && Shot.bVisible && Shot.SkillType==ESkillRuleType::LongShot
  && (Shot.Stage==EFMCodexUMGLongShotStage::BranchChoice
   || Shot.Stage==EFMCodexUMGLongShotStage::DeadCorner
   || (Shot.Stage==EFMCodexUMGLongShotStage::DirectShot && Shot.Formula.bVisible));
 if (!F.bActive) return F;
 using R=EMatchPlayResolutionParticipantRole;
 F.AttackSequence=Header.AttackSequence;
 F.bMethodChoice=Shot.Stage==EFMCodexUMGLongShotStage::BranchChoice;
 F.Method=Shot.Stage==EFMCodexUMGLongShotStage::DeadCorner?EMethod::DeadCorner:EMethod::Direct;
 auto Add=[&](const FFMCodexUMGInlineFormulaParticipantViewModel& P,EInitialTurnOrderPlayer Side,bool FormulaActive)
 {
  if(P.CardId.IsNone() || (P.Role!=R::Carrier && P.Role!=R::Marker && P.Role!=R::Goalkeeper)) return;
  auto* Existing=F.Participants.FindByPredicate([&](const auto& X){return X.Role==P.Role;});
  if(Existing) {Existing->bFormulaActive |= FormulaActive && Existing->CardId==P.CardId; return;}
  const auto& Tracker=Side==Header.LeftPlayerSide?Header.LeftAttackTurnTracker:Header.RightAttackTurnTracker;
  F.Participants.Add({P.CardId,P.Role,Side,FText::FromString(P.PlayerName),FMCodexMatchShellStyle::HeaderAccent(Tracker.PrimarySideColor),FormulaActive});
 };
 const auto Defense=Shot.SpatialAttackingSide==EInitialTurnOrderPlayer::PlayerA?EInitialTurnOrderPlayer::PlayerB:EInitialTurnOrderPlayer::PlayerA;
 Add(Shot.SpatialCarrier,Shot.SpatialAttackingSide,false);
 Add(Shot.SpatialMarker,Defense,false);
 Add(Shot.SpatialGoalkeeper,Defense,false);
 if(!F.bMethodChoice && F.Method==EMethod::Direct)
  for(const auto* Row:{&Shot.Formula.AttackRow,&Shot.Formula.DefenseRow})
   for(const auto& P:Row->Participants) Add(P,Row->Side,true);
 if(Shot.Formula.bDiceRevealVisible || Shot.Formula.bShowFormulaRows)
  F.Highlight=Shot.Formula.bAttackRowActive?R::Carrier:Shot.Formula.bDefenseRowActive?R::Marker:R::None;
 const bool Ready=F.Method==EMethod::DeadCorner
  ?Shot.bNarrativeAvailable && !Shot.bDiceRevealVisible
  :Shot.Formula.bNarrativeAvailable && !Shot.Formula.bDiceRevealVisible;
 // Only typed, already-disclosed decisions. Never infer from dice, totals or text.
 if(Ready)
  switch(Shot.Formula.SpatialOutcome)
  {
  case EMatchPlayResolutionDecisionOutcome::Goal:F.Outcome=EOutcome::Goal;break;
  case EMatchPlayResolutionDecisionOutcome::ImmediateMiss:F.Outcome=EOutcome::ImmediateMiss;break;
  case EMatchPlayResolutionDecisionOutcome::Miss:F.Outcome=EOutcome::DefensiveSuccess;break;
  default:break;
  }
 return F;
}
void FState::Sync(const FFacts& Next)
{
 if (!Next.bActive) { Facts=Next; Phase=EPhase::Hidden; Elapsed=0; bEnteredFromPreview=false; Celebration.Reset(); return; }
 const bool bWasPreview=Phase==EPhase::Preview;
 const bool bNew=Phase==EPhase::Hidden || Facts.AttackSequence!=Next.AttackSequence || IsCross(Facts.Method)!=IsCross(Next.Method);
 if(bNew)Celebration.Reset();
 Facts=Next;
 if(Facts.bMethodChoice || Facts.bRoutePending)
 {
  if(bNew) {PreviewMethod=Facts.Method;CornerBlend=0;CrossLowBlend=Facts.Method==EMethod::CrossLow?1.f:0.f;PulseTime=0;}
  if(Facts.bRoutePending)PreviewMethod=Facts.Method;
  Phase=EPhase::Preview;Elapsed=0;return;
 }
 PreviewMethod=Facts.Method;
 if (bNew || bWasPreview)
 {
  bEnteredFromPreview=bWasPreview;
  CrossEntryMotion=bWasPreview?.5f-.5f*FMath::Cos(PulseTime*2.f):0.f;
  LongShotEntryPressure=bWasPreview?(.5f-.5f*FMath::Cos(PulseTime*2.f*PI/LongShotPreviewCycleSeconds))*(1.f-CornerBlend):0.f;
  if(bNew && IsCross(Facts.Method))CrossLowBlend=Facts.Method==EMethod::CrossLow?1.f:0.f;
  if(bNew && !IsCross(Facts.Method))CornerBlend=Facts.Method==EMethod::DeadCorner?1.f:0.f;
  // A reopened resolved view snaps to the disclosed result, never asks for another roll.
  Phase=Facts.Outcome==EOutcome::None?EPhase::Setup:EPhase::ResultHold; Elapsed=0;
 }
 else if (Phase==EPhase::FormulaHold && Facts.Outcome!=EOutcome::None)
 { Phase=EPhase::Outcome; Elapsed=0; }
}
bool FState::IsAnimating() const
{ return Phase==EPhase::Setup || Phase==EPhase::Intent || Phase==EPhase::Outcome; }
float FState::Progress() const
{
 const float Duration=PhaseSeconds();
 return IsAnimating()?FMath::Clamp(Elapsed/Duration,0.f,1.f):1.f;
}
float FState::PhaseSeconds() const
{
 if(IsCross(Facts.Method))return Phase==EPhase::Setup?CrossSetupSeconds:Phase==EPhase::Intent?CrossIntentSeconds:CrossOutcomeSeconds;
 return Phase==EPhase::Setup?SetupSeconds:Phase==EPhase::Intent?IntentSeconds:OutcomeSeconds;
}
float FState::CrossOutcomeActionProgress() const
{
 return Phase==EPhase::ResultHold?1.f:Phase==EPhase::Outcome
  ?FMath::Clamp(Elapsed/CrossOutcomeActionSeconds,0.f,1.f):0.f;
}
bool FState::IsCrossOutcomeVisualHold() const
{return IsCross(Facts.Method) && Phase==EPhase::Outcome && Elapsed>=CrossOutcomeActionSeconds;}
float FState::OutcomeActionProgress() const
{
 if(IsCross(Facts.Method))return CrossOutcomeActionProgress();
 return Phase==EPhase::ResultHold?1.f:Phase==EPhase::Outcome?FMath::Clamp(Elapsed/OutcomeActionSeconds,0.f,1.f):0.f;
}
bool FState::IsOutcomeVisualHold() const
{return Phase==EPhase::Outcome && Elapsed>=(IsCross(Facts.Method)?CrossOutcomeActionSeconds:OutcomeActionSeconds);}
bool FState::Skip()
{
 if(Celebration.Skip())return true;
 if (!IsAnimating()) return false;
 Phase=Phase==EPhase::Setup?EPhase::Intent:Phase==EPhase::Intent
  ?(Facts.Outcome==EOutcome::None?EPhase::FormulaHold:EPhase::Outcome):EPhase::ResultHold;
 Elapsed=0; return true;
}
bool FState::Tick(float DeltaSeconds)
{
 const float Delta=FMath::Max(0.f,DeltaSeconds);
 Celebration.Tick(Delta);
 PulseTime+=Delta;
 CornerBlend=FMath::FInterpConstantTo(CornerBlend,PreviewMethod==EMethod::DeadCorner?1.f:0.f,Delta,1.f/PreviewTransitionSeconds);
 CrossLowBlend=FMath::FInterpConstantTo(CrossLowBlend,PreviewMethod==EMethod::CrossLow?1.f:0.f,Delta,1.f/.15f);
 if (!IsAnimating()) return false;
 Elapsed+=Delta;
 bool Changed=false;
 // Carry actual elapsed time across adjacent spatial beats; never carry into
 // the mandatory Formula / Roll wait or send a gameplay continuation.
 while(IsAnimating() && Progress()>=1.f)
 {
  const float Duration=PhaseSeconds();
  const float Remainder=FMath::Max(0.f,Elapsed-Duration);
  const bool GoalArrived=Phase==EPhase::Outcome && Facts.Outcome==EOutcome::Goal && !IsCross(Facts.Method);
  Skip(); Changed=true;
  // ResultHold is the existing score/narrative disclosure boundary. A reopened
  // result never enters here; duplicate views cannot replay the celebration.
  if(GoalArrived) {Celebration.Start(true);Celebration.Tick(Remainder);}
  if(IsAnimating()) Elapsed=Remainder;
 }
 return Changed;
}
void FState::Gate(FFMCodexUMGInlineFormulaSurfaceViewModel& View) const
{
 if (Phase==EPhase::Hidden || Phase==EPhase::ResultHold) return;
 // Keep existing reel/operands/totals intact. Only the spatial handoff delays terminal text/CTA.
 View.bNarrativeAvailable=false; View.ResultTitle.Reset(); View.NarrativeHeadline.Reset();
 View.OutcomeText={}; View.ResultSubtitle.Reset(); View.ResolutionReasonLabel.Reset();
 if (IsAnimating()) { View.PrimaryAction={}; View.bCanContinue=false; View.ContinueActionLabel.Reset(); }
}

void FState::Preview(EMethod Method)
{
 if(Phase==EPhase::Preview && !Facts.bRoutePending && IsCross(Method)==IsCross(Facts.Method)) PreviewMethod=Method;
}
void FState::Gate(FFMCodexUMGLongShotResolutionViewModel& View) const
{
 Gate(View.Formula);
 if(Phase==EPhase::Hidden || Phase==EPhase::Preview || Phase==EPhase::ResultHold) return;
 View.bNarrativeAvailable=false;View.NarrativeHeadline.Reset();View.ResultTitle.Reset();View.OutcomeText={};
 if(IsAnimating()) {View.PrimaryAction={};View.bCanContinue=false;View.ContinueActionLabel.Reset();}
}
FVector2D OutcomeTarget(EOutcome Outcome,EMethod Method)
{
 if(IsCross(Method) && Outcome==EOutcome::DefensiveSuccess)
  return CrossReceiveZone(Method==EMethod::CrossLow?1.f:0.f)+FVector2D(-140,65);
 if(Outcome==EOutcome::ImmediateMiss) return FVector2D(1290,75);
 if(Outcome==EOutcome::DefensiveSuccess) return Method==EMethod::DeadCorner?FVector2D(1350,180):FVector2D(1010,350);
 return Method==EMethod::DeadCorner?CornerAnchor:GoalAnchor;
}
FVector2D CrossReceiveZone(float LowBlend) {return FMath::Lerp(FVector2D(940,240),FVector2D(855,282),LowBlend);}
FVector2D CrossDeliveryControl(float LowBlend) {return FMath::Lerp(FVector2D(875,-45),FVector2D(920,174),LowBlend);}
FVector2D CrossBallStart(const FState& State)
{return CrossParticipantAnchor(State,EMatchPlayResolutionParticipantRole::Carrier)+FVector2D(FMath::Lerp(48.f,-48.f,State.CrossLowBlend),21);}
FVector2D CrossOutcomeBall(const FState& State,float Flight)
{
 const auto Start=CrossBallStart(State);
 if(State.Facts.Outcome==EOutcome::None)return Start;
 const auto Zone=CrossReceiveZone(State.CrossLowBlend);
 const auto End=OutcomeTarget(State.Facts.Outcome,State.Facts.Method);
 const float T=FMath::Clamp(Flight,0.f,1.f);
 auto Bezier=[](FVector2D A,FVector2D C,FVector2D B,float U)
 {return FMath::Lerp(FMath::Lerp(A,C,U),FMath::Lerp(C,B,U),U);};
 if(T<CrossDeliveryFraction)return Bezier(Start,CrossDeliveryControl(State.CrossLowBlend),Zone,T/CrossDeliveryFraction);
 const float Finish=(T-CrossDeliveryFraction)/(1.f-CrossDeliveryFraction);
 // Both outcomes first reach the contest/receiving area. An aggregate Miss
 // loses the clean attacking continuation, without selecting an interceptor.
 return State.Facts.Outcome==EOutcome::Goal?FMath::Lerp(Zone,End,Finish)
  :Bezier(Zone,Zone+FVector2D(-28,48),End,Finish);
}
FVector2D CrossParticipantAnchor(const FState& State,EMatchPlayResolutionParticipantRole Role)
{
 using R=EMatchPlayResolutionParticipantRole;
 // Preview rehearses approach and return, with no delivery/result. Persistent
 // tokens interpolate between High positioning and Low acceleration in 150ms.
 const float Motion=State.Phase==EPhase::Preview?.5f-.5f*FMath::Cos(State.PulseTime*2.f)
  :State.Phase==EPhase::Setup?State.CrossEntryMotion
  :State.Phase==EPhase::Intent?FMath::Lerp(State.CrossEntryMotion,1.f,State.Progress()):1.f;
 const float Smooth=Motion*Motion*(3-2*Motion);
 const float Low=State.CrossLowBlend;
 if(Role==R::Carrier)return FMath::Lerp(CrossCarrierWideAnchor,CrossCarrierBylineAnchor,Low);
 if(Role==R::Marker)return FMath::Lerp(FVector2D(750-Smooth*15,130-Smooth*8),
  FVector2D(800+Smooth*20,116-Smooth*17),Low);
 if(Role==R::Goalkeeper)return FVector2D(1100,108);
 if(Role==R::Runner)
  return FMath::Lerp(FMath::Lerp(FVector2D(795,200),FVector2D(850,175),Smooth),
   FMath::Lerp(FVector2D(640,240),FVector2D(780,242),Motion*Motion),Low);
 if(Role==R::Helper)
  return FMath::Lerp(FMath::Lerp(FVector2D(1060,292),FVector2D(1020,240),Smooth),
   FMath::Lerp(FVector2D(1025,299),FVector2D(940,264),FMath::Pow(Motion,1.5f)),Low);
 return FVector2D::ZeroVector;
}
bool HasLongShotDefensivePressure(const FState& State)
{
 return State.Facts.Method==EMethod::Direct && State.Facts.Outcome==EOutcome::DefensiveSuccess
  && State.Facts.Participants.ContainsByPredicate([](const auto& P)
  {return P.Role==EMatchPlayResolutionParticipantRole::Marker && P.bFormulaActive;});
}
FVector2D LongShotParticipantAnchor(const FState& State,EMatchPlayResolutionParticipantRole Role)
{
 using R=EMatchPlayResolutionParticipantRole;
 if(Role==R::Goalkeeper)return KeeperAnchor;
 if(Role==R::Carrier)return CarrierAnchor;
 if(Role!=R::Marker)return FVector2D::ZeroVector;
 // Preview pressure never touches the ball and never predicts a result.
 if(State.Phase==EPhase::Preview)
 {
  const float Cycle=.5f-.5f*FMath::Cos(State.PulseTime*2.f*PI/LongShotPreviewCycleSeconds);
  return MarkerAnchor+FVector2D(-12,-32)*(1.f-State.CornerBlend)*Cycle;
 }
 if(State.Phase==EPhase::Setup || State.Phase==EPhase::Intent)
  return MarkerAnchor+FVector2D(-12,-32)*State.LongShotEntryPressure*(State.Phase==EPhase::Setup?1.f:1.f-State.Progress());
 if(HasLongShotDefensivePressure(State) && (State.Phase==EPhase::Outcome || State.Phase==EPhase::ResultHold))
 {
  const float Move=FMath::Clamp(State.OutcomeActionProgress()/.48f,0.f,1.f);
  return FMath::Lerp(MarkerAnchor,MarkerAnchor+FVector2D(-2,-54),Move*Move*(3-2*Move));
 }
 return MarkerAnchor;
}
FVector2D LongShotAimControl(const FState& State)
{return FMath::Lerp(FVector2D(760,170),FVector2D(760,40),State.CornerBlend);}
FVector2D LongShotOutcomeBall(const FState& State,float Flight)
{
 const FVector2D Start=CarrierAnchor+FVector2D(48,21);
 if(State.Facts.Outcome==EOutcome::None)return Start;
 const float T=FMath::Clamp(Flight,0.f,1.f);
 const auto End=OutcomeTarget(State.Facts.Outcome,State.Facts.Method);
 auto Curve=[](FVector2D A,FVector2D C,FVector2D B,float U)
 {return FMath::Lerp(FMath::Lerp(A,C,U),FMath::Lerp(C,B,U),U);};
 if(HasLongShotDefensivePressure(State))
 {
  // The ball passes clear of the portrait/ground token: pressure, not contact.
  const FVector2D Lane=MarkerAnchor+FVector2D(50,-122);
  return T<LongShotPressureArrival?Curve(Start,FVector2D(630,155),Lane,T/LongShotPressureArrival)
   :Curve(Lane,FVector2D(880,204),End,(T-LongShotPressureArrival)/(1.f-LongShotPressureArrival));
 }
 // Preserve accepted DeadCorner and shooter-error geometry exactly.
 const auto Control=State.Facts.Method==EMethod::DeadCorner?FVector2D(760,40)
  :State.Facts.Outcome==EOutcome::ImmediateMiss?FVector2D(760,58):FVector2D(760,170);
 return Curve(Start,Control,End,T);
}
bool IsInsideGoal(FVector2D Point)
{
 // Visible mouth, with a ball-radius margin from either post and crossbar.
 const float U=(Point.X-1092.f)/178.f;
 const float Top=67.f+96.f*U;
 return U>.08f && U<.92f && Point.Y>Top+10.f && Point.Y<Top+82.f;
}
void UpdatePreview(UWidgetTree& Tree,FState& State)
{
 if(State.Phase!=EPhase::Preview) return;
 if(State.Facts.bRoutePending)return;
 // Pointer intent takes precedence over focus retained from keyboard navigation.
 for(bool Hover:{true,false}) for(bool Direct:{true,false})
  if(auto* B=Cast<UButton>(Tree.FindWidget(IsCross(State.Facts.Method)
   ?(Direct?TEXT("TheaterHigh"):TEXT("TheaterLow")):(Direct?TEXT("TheaterNearDirect"):TEXT("TheaterNearCombination")))))
   if(B->GetIsEnabled() && (Hover?B->IsHovered():(B->HasKeyboardFocus() || B->HasAnyUserFocus())))
   { State.Preview(IsCross(State.Facts.Method)?(Direct?EMethod::CrossHigh:EMethod::CrossLow):(Direct?EMethod::Direct:EMethod::DeadCorner)); return; }
}

namespace
{
using namespace FMCodexMatchShellStyle;
using R=EMatchPlayResolutionParticipantRole;
// Source extent / padded extent. Import padding enables mips without changing
// the original source art; sample only the authored region of both scene assets.
constexpr float SceneArtUV=1254.f/2048.f;
FText RoleText(R Role)
{
 if (Role==R::Carrier) return NSLOCTEXT("TacticalScene","Carrier","持球");
 if (Role==R::Marker) return NSLOCTEXT("TacticalScene","Marker","盯人");
 if (Role==R::Runner) return NSLOCTEXT("TacticalScene","Runner","跑位");
 if (Role==R::Helper) return NSLOCTEXT("TacticalScene","Helper","协防");
 return NSLOCTEXT("TacticalScene","Keeper","门将");
}
class SScene final : public SLeafWidget
{
public:
 SLATE_BEGIN_ARGS(SScene) {} SLATE_END_ARGS()
 void Construct(const FArguments&)
 {
  SetCanTick(false); ForceVolatile(true);
  StrokeSegment.SetNum(2);
  // Independent scene assets; never recolor or replace the deployment board's turf.
  // Loaded once per persistent Slate surface, never in Paint or Tick.
  SceneTurf.Reset(LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/TacticalScene/T_TacticalScene_Turf.T_TacticalScene_Turf")));
  BallTexture.Reset(LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/TacticalScene/T_TacticalScene_Ball.T_TacticalScene_Ball")));
  StadiumTexture.Reset(LoadObject<UTexture2D>(nullptr,TEXT("/Game/UI/MatchShell/T_MatchShell_Stadium.T_MatchShell_Stadium")));
  BallBrush.SetResourceObject(BallTexture.Get());
  BallBrush.ImageSize=FVector2D(256,256);
  BallBrush.SetUVRegion(FBox2f(FVector2f::ZeroVector,FVector2f(SceneArtUV,SceneArtUV)));
  CrowdBrush.SetResourceObject(StadiumTexture.Get());
 }
 void Refresh(FState& InState,const FSlateBrush& InTurf,TFunction<void()> InSkip)
 {
  State=&InState; Skip=MoveTemp(InSkip); Turf=InTurf;
  if(SceneTurf.IsValid()) {Turf.SetResourceObject(SceneTurf.Get());Turf.TintColor=FLinearColor::White;}
  // At most five cached portraits. No per-frame asset loading or participant widgets.
  for (int32 I=0;I<5;++I)
  {
   const FName Id=State->Facts.Participants.IsValidIndex(I)?State->Facts.Participants[I].CardId:NAME_None;
   if (Ids[I]==Id) continue;
   Ids[I]=Id; Portraits[I].Reset(); Brushes[I]=FSlateBrush();
   if (Id.IsNone()) continue;
   const auto Art=FFMCodexPlayerUIAssetReferences::Get().ResolveCardArt(Id);
   UTexture2D* Texture=Art.FullCardPortrait.LoadSynchronous();
   if (!Texture) Texture=Art.Portrait.LoadSynchronous();
   Portraits[I].Reset(Texture); Brushes[I].SetResourceObject(Texture);
   Brushes[I].ImageSize=FVector2D(128,128);
  }
  Invalidate(EInvalidateWidgetReason::Paint);
 }
 FVector2D ComputeDesiredSize(float) const override { return FVector2D(1324,State && !IsCross(State->Facts.Method)?380:400); }
 bool SupportsKeyboardFocus() const override { return true; }
 FReply OnMouseButtonDown(const FGeometry&,const FPointerEvent& E) override
 {
  if (E.GetEffectingButton()!=EKeys::LeftMouseButton) return FReply::Unhandled();
  if (Skip) Skip();
  return FReply::Handled().SetUserFocus(SharedThis(this),EFocusCause::Mouse);
 }
 FReply OnKeyDown(const FGeometry&,const FKeyEvent& E) override
 {
  if (E.GetKey()!=EKeys::SpaceBar) return FReply::Unhandled();
  if (!E.IsRepeat() && Skip) Skip();
  return FReply::Handled();
 }
 int32 OnPaint(const FPaintArgs&,const FGeometry& Geometry,const FSlateRect&,
  FSlateWindowElementList& Out,int32 Layer,const FWidgetStyle&,bool) const override
 {
  if (!State || !State->Facts.bActive) return Layer;
  const float Scale=Geometry.GetLocalSize().X/1324.f;
  const bool bLongShot=!IsCross(State->Facts.Method);
  const float Height=bLongShot?380.f:400.f;
  const FGeometry G=Geometry.MakeChild(FVector2D(1324,Height),FSlateLayoutTransform(Scale));
  auto Line=[&](const TArray<FVector2D>& P,FLinearColor C,float W=1.f,int Offset=2)
  { FSlateDrawElement::MakeLines(Out,Layer+Offset,G.ToPaintGeometry(),P,ESlateDrawEffect::None,C,true,W); };
  auto Box=[&](FVector2D P,FVector2D Size,const FSlateBrush& Brush,FLinearColor C)
  { FSlateDrawElement::MakeBox(Out,Layer+4,G.ToPaintGeometry(Size,FSlateLayoutTransform(P)),&Brush,ESlateDrawEffect::None,Brush.GetTint(FWidgetStyle())*C); };
  auto Label=[&](const FText& T,FVector2D Center,float Size,FLinearColor C,bool Bold=false)
  {
   auto F=Font(Size,Bold);
   auto M=FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
   FVector2D Extent=M->Measure(T,F);
   if (Extent.X>210) {F.Size=FMath::Max(11,FMath::FloorToInt(Size*210/Extent.X)); Extent=M->Measure(T,F);}
   FSlateDrawElement::MakeText(Out,Layer+7,G.ToPaintGeometry(Extent,FSlateLayoutTransform(Center-Extent*.5)),T,F,ESlateDrawEffect::None,C);
  };
  auto Circle=[&](FVector2D C,float Radius,FLinearColor Ink,float Width)
  { TArray<FVector2D> P; for(int I=0;I<=48;++I) {float A=2*PI*I/48;P.Add(C+FVector2D(FMath::Cos(A),FMath::Sin(A))*Radius);} Line(P,Ink,Width); };
  // Continuous stadium + final-third surface. The ground extends beyond the
  // viewport, avoiding a detached tabletop/trapezoid silhouette.
  const FSlateBrush& Solid=*FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
  auto Poly=[&](const TArray<FVector2D>& Points,FLinearColor Color,int Offset=0)
  {
   TArray<FSlateVertex> V;TArray<SlateIndex> IX;
   for(auto P:Points) V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(P),FVector2f::ZeroVector,Color.ToFColor(true)));
   for(int I=1;I+1<V.Num();++I){IX.Add(0);IX.Add(I);IX.Add(I+1);}
   FSlateDrawElement::MakeCustomVerts(Out,Layer+Offset,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Solid),V,IX,nullptr,0,0);
  };
  // A level far touchline opens the penalty area into a compact tactical
  // overview. LongShot keeps this same frame through selection and resolution.
  const FVector2D FarLeft(-200,bLongShot?45:145),FarRight(900,bLongShot?35:50),
   NearRight(1470,bLongShot?383:374),NearLeft(-80,bLongShot?560:540);
  const FVector2D GoalRailEnd(2320,688),GoalBoardRise(0,-30);
  auto Ground=[&](float X,float Y){return FMath::Lerp(FMath::Lerp(FarLeft,FarRight,X),FMath::Lerp(NearLeft,NearRight,X),Y);};
  auto CrowdStrip=[&](const TArray<FVector2D>& Points,FVector2f UVMin,FVector2f UVMax,float U0,float U1,bool GoalStand=false)
  {
   if(!CrowdBrush.GetResourceObject()) return;
   TArray<FSlateVertex> V,StandVeil;TArray<SlateIndex> IX;
   constexpr int N=8;
   for(int Y=0;Y<=N;++Y) for(int X=0;X<=N;++X)
   {
    const float U=X/float(N),W=Y/float(N),Across=FMath::Lerp(U0,U1,U);
    const auto P=FMath::Lerp(FMath::Lerp(Points[0],Points[1],U),FMath::Lerp(Points[3],Points[2],U),W);
    const float Edge=GoalStand?1.f:FMath::Clamp(float(P.X/90),0.f,1.f)*FMath::Clamp((1-Across)/.14f,0.f,1.f);
    const float Alpha=FMath::SmoothStep(0.f,.9f,W)*Edge*(GoalStand?.82f:.48f);
    const FColor Tint=(GoalStand?FLinearColor(.48f,.59f,.67f,Alpha):FLinearColor(.29f,.40f,.46f,Alpha)).ToFColor(true);
    V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(P),FVector2f(FMath::Lerp(UVMin.X,UVMax.X,U),FMath::Lerp(UVMin.Y,UVMax.Y,W)),Tint));
    if(GoalStand)StandVeil.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(P),FVector2f::ZeroVector,
     Color(5,19,29).CopyWithNewOpacity(FMath::SmoothStep(0.f,.65f,W)*.97f).ToFColor(true)));
    if(X<N && Y<N){const int I=Y*(N+1)+X;IX.Append({SlateIndex(I),SlateIndex(I+1),SlateIndex(I+N+2),SlateIndex(I),SlateIndex(I+N+2),SlateIndex(I+N+1)});}
   }
   if(GoalStand)FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Solid),StandVeil,IX,nullptr,0,0);
   FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(CrowdBrush),V,IX,nullptr,0,0);
  };
  // Crop the existing crowd artwork (not its pitch) into the same perspective as
  // the touchline. This supplies a stand/aisle/LED transition, not a floating board.
  const FVector2D StandRise(0,-104),BoardRise(0,-21);
  for(int I=0;I<8;++I)
  {
   const float U0=I/8.f,U1=(I+1)/8.f;
   const auto L=Ground(U0,0),Rt=Ground(U1,0);
   CrowdStrip({L+StandRise,Rt+StandRise,Rt+BoardRise,L+BoardRise},
    FVector2f(.12f+U0*.75f,.19f),FVector2f(.12f+U1*.75f,.306f),U0,U1);
  }
  // LongShot uses one ground plane through the controls, instead of fading out at
  // the spatial lane and exposing the differently lit grass in the backdrop.
  // Separate UV tiles keep the padded source art inside its valid texture region.
  if(Turf.GetResourceObject())
  {
   TArray<FSlateVertex> V,Veil;TArray<SlateIndex> IX;
   const int NX=bLongShot?24:56,NY=bLongShot?16:26;
   const int FirstTileX=bLongShot?-1:0,LastTileX=bLongShot?1:0,LastTileY=bLongShot?2:0;
   for(int TileY=0;TileY<=LastTileY;++TileY)for(int TileX=FirstTileX;TileX<=LastTileX;++TileX)
   {
    const int Start=V.Num();
    for(int Y=0;Y<=NY;++Y)for(int X=0;X<=NX;++X)
    {
     const float TileU=X/float(NX),TileW=Y/float(NY),U=TileX+TileU,W=TileY+TileW;
     const float Pool=FMath::Exp(-FMath::Square((U-.55f)*1.6f)-FMath::Square((W-.44f)*(bLongShot?.95f:1.4f)));
     const float Mow=bLongShot?.94f+.045f*FMath::Sin(U*8*PI):.94f+.008f*FMath::Sin(U*14*PI);
     const float Light=(bLongShot?.34f+.37f*Pool:.40f+.24f*Pool)*Mow;
     const auto Point=Ground(U,W);
     const float GoalBoundary=FMath::Lerp(float(FarRight.Y),float(GoalRailEnd.Y),float((Point.X-FarRight.X)/(GoalRailEnd.X-FarRight.X)));
     const float Edge=bLongShot?(Point.X>FarRight.X?FMath::SmoothStep(-3.f,3.f,float(Point.Y)-GoalBoundary):1.f)
      :FMath::Clamp(float(FMath::Min(Point.X,1324-Point.X)/100),0.f,1.f)
      *FMath::Clamp(float((Height-Point.Y)/65),0.f,1.f);
     const FColor Tint=(bLongShot?FLinearColor(Light*.54f,Light*1.08f,Light*.96f,Edge)
      :FLinearColor(Light*.65f,Light*.93f,Light*.86f,Edge)).ToFColor(true);
     const float UVScale=SceneTurf.IsValid()?SceneArtUV:1.f;
     FVector2f UV=FVector2f(TileU,TileW)*UVScale;
     if(bLongShot)
     {
      // Mirror adjacent patches and inset half a texel so filtering cannot sample
      // the transparent padding or leave a dark seam along the tile edges.
      const float Inset=SceneTurf.IsValid()?.5f/2048.f:0.f;
      UV=FVector2f(FMath::Lerp(Inset,UVScale-Inset,TileX%2==0?TileU:1-TileU),
       FMath::Lerp(Inset,UVScale-Inset,TileY%2==0?TileW:1-TileW));
     }
     V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(Point),UV,Tint));
     // A continuous, low-contrast green veil quiets the fine texture. It does not
     // fade to a second pitch texture at the controls or at any tile boundary.
     FLinearColor Atmosphere=Color(20,49,35,uint8(255*Edge*(.10f+.20f*(1-FMath::Clamp(W,0.f,1.f)))));
     if(bLongShot)
     {
      const float NearShade=FMath::SmoothStep(Height*.72f,Height+390.f,float(Point.Y));
      const float SideShade=FMath::SmoothStep(360.f,800.f,float(FMath::Abs(Point.X-662.f)));
      Atmosphere=FMath::Lerp(Color(13,65,43),Color(5,24,25),NearShade*.9f)
       .CopyWithNewOpacity((.30f+.38f*NearShade+.12f*SideShade)*Edge);
     }
     Veil.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(Point),FVector2f::ZeroVector,Atmosphere.ToFColor(true)));
     if(X<NX && Y<NY){int I=Start+Y*(NX+1)+X;IX.Append({SlateIndex(I),SlateIndex(I+1),SlateIndex(I+NX+2),SlateIndex(I),SlateIndex(I+NX+2),SlateIndex(I+NX+1)});}
    }
   }
   FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Turf),V,IX,nullptr,0,0);
   FSlateDrawElement::MakeCustomVerts(Out,Layer,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Solid),Veil,IX,nullptr,0,0);
  }
  // Continue the far-side enclosure behind the net. Reuse the existing crowd
  // artwork above a dark concourse, with boards in front and the goal above both.
  if(bLongShot)
  {
   // Carry the stand and rail beyond the viewport; no exposed cut-out edge on turf.
   const FVector2D UpperLeft(FarRight.X,-22),UpperRight(GoalRailEnd.X,-22);
   CrowdStrip({UpperLeft,UpperRight,GoalRailEnd+GoalBoardRise,FarRight+GoalBoardRise},
    FVector2f(.32f,.19f),FVector2f(.87f,.306f),0,1,true);
   Line({FarRight+FVector2D(0,-38),GoalRailEnd+FVector2D(0,-38)},Color(92,126,142,100),1.f,1);
  }
  // Small, foreshortened LED panels with a top rail, inset screen and cast contact
  // shadow. Use only project branding; ads are cosmetic and carry no gameplay data.
  auto Boards=[&](FVector2D Start,FVector2D End,int Count,FVector2D Rise)
  {
   for(int I=0;I<Count;++I)
   {
   const auto L=FMath::Lerp(Start,End,I/float(Count)),Rt=FMath::Lerp(Start,End,(I+1)/float(Count));
   Poly({L,Rt,Rt+FVector2D(0,8),L+FVector2D(0,8)},FLinearColor(0,.006f,.009f,.32f),1);
   Poly({L,Rt,Rt+Rise,L+Rise},Color(7,24,40),1);
   Poly({L+FVector2D(2,-4),Rt+FVector2D(-2,-4),Rt+Rise+FVector2D(-2,4),L+Rise+FVector2D(2,4)},I%2?Color(13,39,63):Color(10,32,52),1);
   Line({L+Rise,Rt+Rise},Color(115,143,154,140),1.f);
   Line({L,Rt},Color(7,17,24),2.4f);
   Line({L,L+Rise},Color(90,121,139,85),1.f);
   const FText Brand=FText::FromString(I%2?TEXT("FMCODEX"):TEXT("FOOTBALL"));
   const auto BrandFont=Font(9,true);
   const auto Extent=FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(Brand,BrandFont);
   const FVector2D Center=(L+Rt)*.5f+Rise*.5f;
   if(Center.X<36 || Center.X>(bLongShot?1800:1288)) continue;
   const float Angle=FMath::Atan2(Rt.Y-L.Y,Rt.X-L.X);
   const auto BrandGeometry=G.MakeChild(Extent,FSlateLayoutTransform(Center-Extent*.5f),FSlateRenderTransform(FQuat2D(Angle)),FVector2D(.5,.5));
   FSlateDrawElement::MakeText(Out,Layer+2,BrandGeometry.ToPaintGeometry(),Brand,BrandFont,ESlateDrawEffect::None,Color(160,186,202,210));
   }
  };
  Boards(FarLeft,FarRight,8,BoardRise);
  if(bLongShot)Boards(FarRight,GoalRailEnd,12,GoalBoardRise);
  // Chalk lies in the grass plane: soft contact edge plus a thinner warm-white core.
  // Perspective widths grow slightly towards the near touchline.
  const FLinearColor White=bLongShot?Color(227,239,228,224):Color(207,219,203,166);
  auto Chalk=[&](const TArray<FVector2D>& Points,float Width)
  {
   const float PaintWidth=Width*(bLongShot?1.3f:1.f);
   if(bLongShot)
   {
    for(int I=1;I<Points.Num();++I)
    {
     // Turf continues below the controls, but its markings belong to the hero
     // area. Fade short pieces before that boundary instead of drawing through UI.
     const int Pieces=FMath::Max(1,FMath::CeilToInt((Points[I]-Points[I-1]).Size()/16.f));
     for(int K=0;K<Pieces;++K)
     {
      const auto A=FMath::Lerp(Points[I-1],Points[I],K/float(Pieces));
      const auto B=FMath::Lerp(Points[I-1],Points[I],(K+1)/float(Pieces));
      const float Opacity=1.f-FMath::SmoothStep(294.f,330.f,float(FMath::Max(A.Y,B.Y)));
      if(Opacity<=0)continue;
      Line({A,B},Color(188,204,183,34).CopyWithNewOpacity(.133f*Opacity),PaintWidth+2.f,1);
      Line({A,B},White.CopyWithNewOpacity(White.A*Opacity),PaintWidth,1);
     }
    }
   }
   else
   {
    Line(Points,Color(188,204,183,34),PaintWidth+2.f,1);
    Line(Points,White,PaintWidth,1);
   }
  };
  if(bLongShot)Chalk({Ground(0,.024f),Ground(1,.024f),Ground(1,.84f)},1.35f);
  else Chalk({Ground(0,.024f),Ground(1,.024f),Ground(1,.98f),Ground(0,.98f)},1.35f);
  Chalk({Ground(1,.18f),Ground(.59f,.18f),Ground(.59f,.84f),Ground(1,.84f)},1.55f);
  Chalk({Ground(1,.36f),Ground(.85f,.36f),Ground(.85f,.66f),Ground(1,.66f)},1.45f);
  TArray<FVector2D> Arc;
  for(int I=0;I<=32;++I){float A=PI*.5f+PI*I/32;Arc.Add(Ground(.59f+FMath::Cos(A)*.09f,.51f+FMath::Sin(A)*.20f));}
  Chalk(Arc,1.45f);
  Circle(Ground(.76f,.51f),1.35f,White,1.8f);
  // Three net planes and cylindrical-looking posts share the existing outcome
  // anchors. Lighting changes here never add a save, block or post-hit event.
  const FVector2D TL(1092,67),TR(1270,163),BL(1092,159),BR(1270,255),Depth(44,-24);
  Poly({BL+FVector2D(-8,8),BR+FVector2D(-8,8),BR+Depth+FVector2D(14,8),BL+Depth+FVector2D(14,8)},Color(0,8,12,62),1);
  Poly({BL,BR,BR+Depth,BL+Depth},Color(7,18,22,72),1);
  Poly({TL+Depth,TR+Depth,BR+Depth,BL+Depth},Color(123,156,157,14),1);
  Poly({TL,TR,TR+Depth,TL+Depth},Color(179,194,191,12),1);
  const float Progress=State->Progress();
  const bool bCross=IsCross(State->Facts.Method);
  const bool bOutcome=State->Phase==EPhase::Outcome || State->Phase==EPhase::ResultHold;
  // Each family owns its action budget; both settle before the final visual hold.
  const float ActionProgress=State->OutcomeActionProgress();
  const float Travel=State->Phase==EPhase::ResultHold?1.f:
   State->Phase==EPhase::Outcome?(bCross?FMath::Clamp(ActionProgress/CrossArrivalFraction,0.f,1.f):FMath::Clamp((ActionProgress-.075f)/.725f,0.f,1.f)):0.f;
  const float Flight=bCross?Travel:1.f-FMath::Pow(1.f-Travel,1.2f);
  const float Contact=State->Phase==EPhase::Outcome?(bCross
   ?FMath::Clamp((ActionProgress-CrossArrivalFraction)/(1.f-CrossArrivalFraction),0.f,1.f)
   :FMath::Clamp((ActionProgress-.80f)/.20f,0.f,1.f)):0.f;
  const float NetPulse=State->Phase==EPhase::Outcome && State->Facts.Outcome==EOutcome::Goal
   ?FMath::Sin(Contact*PI):0.f;
  const auto NetInk=Color(173,191,192).CopyWithNewOpacity(.24f+NetPulse*.16f);
  for(int I=0;I<=18;++I)
  {
   const float T=I/18.f;
   const auto A=FMath::Lerp(TL,TR,T)+Depth,B=FMath::Lerp(BL,BR,T)+Depth;
   Line({A,(A+B)*.5f+FVector2D((1.2f+NetPulse*3.f)*FMath::Sin(T*PI),1),B},NetInk,.6f);
   Line({FMath::Lerp(TL,TR,T),FMath::Lerp(TL,TR,T)+Depth},NetInk.CopyWithNewOpacity(.20f),.6f);
  }
  for(int I=0;I<=11;++I)
  {
   const float T=I/11.f;
   const auto A=FMath::Lerp(TL,BL,T)+Depth,B=FMath::Lerp(TR,BR,T)+Depth;
   Line({A,(A+B)*.5f+FVector2D(NetPulse*2,1.4f*FMath::Sin(T*PI)),B},NetInk,.6f);
   Line({FMath::Lerp(TR,BR,T),FMath::Lerp(TR,BR,T)+Depth},NetInk,.6f);
   Line({FMath::Lerp(TL,BL,T),FMath::Lerp(TL,BL,T)+Depth},NetInk.CopyWithNewOpacity(.17f),.6f);
  }
  for(int I=1;I<6;++I)
  {
   const float T=I/6.f;
   Line({FMath::Lerp(TR,TR+Depth,T),FMath::Lerp(BR,BR+Depth,T)},NetInk,.65f);
   Line({FMath::Lerp(TL,TL+Depth,T),FMath::Lerp(BL,BL+Depth,T)},NetInk.CopyWithNewOpacity(.17f),.6f);
   Line({FMath::Lerp(TL,TL+Depth,T),FMath::Lerp(TR,TR+Depth,T)},NetInk.CopyWithNewOpacity(.20f),.6f);
  }
  Line({BL+Depth,TL+Depth,TR+Depth,BR+Depth},Color(143,165,170,185),1.3f);
  Line({BL,BL+Depth,BR+Depth,BR},Color(166,183,180,160),1.4f);
  Line({TL,TL+Depth,TR+Depth,TR},Color(182,201,204,200),1.5f);
  Line({BL+FVector2D(1.6,2),TL+FVector2D(1.6,2),TR+FVector2D(1.6,2),BR+FVector2D(1.6,2)},Color(3,12,18,180),7.f);
  Line({BL,TL,TR,BR},Color(163,184,188),5.f);
  Line({BL-FVector2D(.7,.7),TL-FVector2D(.7,.7),TR-FVector2D(.7,.7),BR-FVector2D(.7,.7)},Color(211,227,225),2.f);
  const float Intent=State->Phase==EPhase::Preview || State->bEnteredFromPreview?1.f:
   State->Phase==EPhase::Setup?0.f:State->Phase==EPhase::Intent?Progress:1.f;
  auto RoleAnchor=[&](R Role){return bCross?CrossParticipantAnchor(*State,Role):LongShotParticipantAnchor(*State,Role);};
  // At the byline keep the ball on the pitch-facing side of the portrait;
  // the LongShot/right-side boot offset would put it beyond the goal line.
  const FVector2D BallStart=bCross?CrossBallStart(*State):RoleAnchor(R::Carrier)+FVector2D(48,21);
  const FVector2D PreviewTarget=bCross?CrossReceiveZone(State->CrossLowBlend):FMath::Lerp(GoalAnchor,CornerAnchor,State->CornerBlend);
  const FVector2D Target=bOutcome?OutcomeTarget(State->Facts.Outcome,State->Facts.Method):PreviewTarget;
  const FVector2D Control=bCross?CrossDeliveryControl(State->CrossLowBlend)
   :LongShotAimControl(*State);
  auto Path=[&](float T,FVector2D End)
  {return FMath::Lerp(FMath::Lerp(BallStart,Control,T),FMath::Lerp(Control,End,T),T);};
  auto Curve=[&](float T)
  {
   if(bCross && bOutcome)return CrossOutcomeBall(*State,T);
   if(!bCross && bOutcome)return LongShotOutcomeBall(*State,T);
   return Path(T,Target);
  };
  if(bCross && !bOutcome)
  {
   const float Low=State->CrossLowBlend;
   // High: both roles converge on one landing area. Low: the receiver arrives
   // into a cutback while the helper closes from the goal side, not a depth race.
   TArray<FVector2D> Landing;
   for(int I=0;I<=48;++I){float A=2*PI*I/48;Landing.Add(PreviewTarget+FVector2D(FMath::Cos(A)*47,FMath::Sin(A)*18));}
   Line(Landing,Mint().CopyWithNewOpacity((1-Low)*.46f),1.5f);
   for(const auto& P:State->Facts.Participants)
   {
    if(P.Role!=R::Runner && P.Role!=R::Helper)continue;
    const auto C=RoleAnchor(P.Role)+FVector2D(0,32);
    Line({C,FMath::Lerp(C,PreviewTarget,.7f)},P.Accent.CopyWithNewOpacity((1-Low)*.25f),2.f);
    const auto Direction=(PreviewTarget-C).GetSafeNormal();
    const FVector2D Normal(-Direction.Y,Direction.X);
    for(int I=0;I<3;++I)
     Line({C-Direction*(88+I*12)+Normal*(7+I*5),C-Direction*(48+I*6)+Normal*(7+I*5)},P.Accent.CopyWithNewOpacity(Low*(.46f-I*.1f)),1.8f);
    Line({C+Direction*44,FMath::Lerp(C,PreviewTarget,.8f)},P.Accent.CopyWithNewOpacity(Low*.35f),1.4f);
   }
  }
  // Keep the intended route stable through the roll. Never replace it with a
  // complete failure trajectory before the ball has actually travelled there.
  const float IntentOpacity=bOutcome?FMath::Max(0.f,1.f-Progress*4.f):State->Phase==EPhase::FormulaHold?.38f:1.f;
  if (Intent>0 && IntentOpacity>0)
  {
   // Arc-length spacing, not equal Bezier parameters: short dashes keep a
   // consistent screen-space rhythm near both shooter and goal.
   float Distance=0;
   constexpr float DashLength=8.f,DashPeriod=14.f;
   for(int I=0;I<160 && I/160.f<Intent;++I)
   {
    const auto A=Path(I/160.f,PreviewTarget),B=Path(FMath::Min((I+1)/160.f,Intent),PreviewTarget);
    const float Length=(B-A).Size();
    float Along=0;
    while(Along<Length-KINDA_SMALL_NUMBER)
    {
     const float InPeriod=FMath::Fmod(Distance+Along,DashPeriod);
     const bool Visible=InPeriod<DashLength;
     const float Take=FMath::Min(Length-Along,FMath::Max(.001f,(Visible?DashLength:DashPeriod)-InPeriod));
     if(Visible)
     {
      StrokeSegment[0]=FMath::Lerp(A,B,Along/Length);StrokeSegment[1]=FMath::Lerp(A,B,(Along+Take)/Length);
      Line(StrokeSegment,Mint().CopyWithNewOpacity(.06f*IntentOpacity),5.f);
      Line(StrokeSegment,Mint().CopyWithNewOpacity(.78f*IntentOpacity),1.8f);
      Line(StrokeSegment,Color(201,255,244).CopyWithNewOpacity(.45f*IntentOpacity),.65f);
     }
     Along+=Take;
    }
    Distance+=Length;
   }
   if(State->Phase==EPhase::Preview)
   {
    const float Cycle=.5f-.5f*FMath::Cos(State->PulseTime*2.f*PI/LongShotPreviewCycleSeconds);
    Circle(Target,bCross?10:10+6*Cycle,Mint().CopyWithNewOpacity(bCross?.4f:.25f+.15f*Cycle),1.f);
    if(!bCross)
    {
     // A light sweep suggests aim, never a preview ball travelling into goal.
     const float CycleTime=FMath::Fmod(State->PulseTime/LongShotPreviewCycleSeconds,1.f);
     const float Sweep=.12f+.70f*CycleTime;
     Line({Path(Sweep-.035f,PreviewTarget),Path(Sweep,PreviewTarget),Path(Sweep+.035f,PreviewTarget)},
      Mint().CopyWithNewOpacity(.43f*FMath::Square(FMath::Sin(CycleTime*PI))),3.f);
     const auto Aim=(Path(.08f,PreviewTarget)-BallStart).GetSafeNormal();
     Line({BallStart+Aim*24,BallStart+Aim*65},Mint().CopyWithNewOpacity(.35f),2.f);
     if(State->CornerBlend>0)
      Line({Target+FVector2D(-18,-10),Target+FVector2D(10,-10),Target+FVector2D(10,16)},
       Mint().CopyWithNewOpacity(State->CornerBlend*(.35f+.2f*Cycle)),2.f);
    }
   }
   const auto Tip=Path(Intent,PreviewTarget); auto Dir=(Tip-Path(FMath::Max(0.f,Intent-.025f),PreviewTarget)).GetSafeNormal();
   Poly({Tip,Tip-Dir*11+FVector2D(-Dir.Y,Dir.X)*4,Tip-Dir*9,Tip-Dir*11-FVector2D(-Dir.Y,Dir.X)*4},Color(119,255,231).CopyWithNewOpacity(IntentOpacity),3);
  }
  if(bOutcome && Flight>0)
  {
   for(int I=1;I<=80;++I)
   {
    const float T=I/80.f*Flight;
    const float Tail=FMath::Clamp(1.f-(Flight-T)/.11f,0.f,1.f);
    const float Moving=State->Phase==EPhase::Outcome?(1.f-Contact):0.f;
    StrokeSegment[0]=Curve((I-1)/80.f*Flight);StrokeSegment[1]=Curve(T);
    Line(StrokeSegment,Mint().CopyWithNewOpacity(.12f+.30f*Tail*Moving),1.15f,3);
    if(Tail>0 && Moving>0) Line(StrokeSegment,Color(219,255,245).CopyWithNewOpacity(Tail*Moving*.65f),2.f,3);
   }
  }
  const auto* Marker=State->Facts.Participants.FindByPredicate([](const auto& P){return P.Role==R::Marker;});
  if (Marker)
  {
   const auto Carrier=RoleAnchor(R::Carrier),Mark=RoleAnchor(R::Marker);
   const float Length=(Carrier-Mark).Size();
   for(float D=0;D<Length;D+=12.f)
    Line({FMath::Lerp(Mark,Carrier,D/Length),FMath::Lerp(Mark,Carrier,FMath::Min(D+5.f,Length)/Length)},
     Marker->Accent.CopyWithNewOpacity((bOutcome?.18f:.36f)*(bCross?1.f:1.f-State->CornerBlend*.92f)),1.f);
   if(!bCross && State->CornerBlend<1.f && State->Facts.Outcome!=EOutcome::ImmediateMiss)
    Line({Mark+FVector2D(0,-38),Mark+FVector2D(15,-57),Mark+FVector2D(30,-61)},
     Marker->Accent.CopyWithNewOpacity((1.f-State->CornerBlend)*.4f),2.f);
  }
  const float Fade=State->Phase==EPhase::Setup && !State->bEnteredFromPreview?FMath::Clamp(Progress*2,0.f,1.f):1.f;
  for(int I=0;I<State->Facts.Participants.Num() && I<5;++I)
  {
   const auto& P=State->Facts.Participants[I]; auto C=RoleAnchor(P.Role);
   // Persistent portraits: only the real Marker illustrates pressure. GK does
   // not become a save actor merely because it contributes to the Formula.
   C.Y+=(1-Fade)*8;
   const float Radius=34*(.9f+.1f*Fade);
   const bool Linked=State->Phase==EPhase::FormulaHold && P.bFormulaActive
    && (P.Role==State->Facts.Highlight || (P.Role==R::Goalkeeper && State->Facts.Highlight==R::Marker)
     || (bCross && ((P.Role==R::Runner && State->Facts.Highlight==R::Carrier)
      || (P.Role==R::Helper && State->Facts.Highlight==R::Marker))));
   const float Pulse=Linked?.18f+.10f*FMath::Sin(State->PulseTime*4.f):0.f;
   const float Context=!bCross && P.Role==R::Marker?1.f-State->CornerBlend*.65f:1.f;
   const float Emphasis=(P.Role==R::Goalkeeper && !P.bFormulaActive?.45f:1.f)*Context;
   TArray<FVector2D> GroundRing;
   for(int K=0;K<=48;++K){float A=2*PI*K/48;GroundRing.Add(C+FVector2D(FMath::Cos(A)*49,31+FMath::Sin(A)*12));}
   Line(GroundRing,Navy().CopyWithNewOpacity(.4f),13);
   Line(GroundRing,P.Accent.CopyWithNewOpacity(Fade*(.15f+Pulse)*Emphasis),4);
   Line(GroundRing,P.Accent.CopyWithNewOpacity(Fade*(.46f+Pulse)*Emphasis),1.f);
   Circle(C,Radius+4,P.Accent.CopyWithNewOpacity(Fade*(.10f+Pulse)*Emphasis),5);
   Circle(C,Radius+1,P.Accent.CopyWithNewOpacity(Fade*(.9f+Pulse)*Emphasis),2.5f);
   // Triangle fan clips the portrait to a true circle without a new material/asset pipeline.
   if(Portraits[I].IsValid())
   {
    TArray<FSlateVertex> V; TArray<SlateIndex> IX;
    auto Vertex=[&](FVector2D Pos,FVector2f UV){V.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),FVector2f(Pos),UV,FColor(255,255,255,uint8(255*Fade))));};
    Vertex(C,FVector2f(.5f,.33f));
    for(int K=0;K<=48;++K){float A=2*PI*K/48;float X=FMath::Cos(A),Y=FMath::Sin(A);Vertex(C+FVector2D(X,Y)*Radius,FVector2f(.5f+X*.46f,.33f+Y*.285f));if(K>0){IX.Add(0);IX.Add(K);IX.Add(K+1);}}
    FSlateDrawElement::MakeCustomVerts(Out,Layer+5,FSlateApplication::Get().GetRenderer()->GetResourceHandle(Brushes[I]),V,IX,nullptr,0,0);
   }
   const float NameWidth=FMath::Clamp(float(FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(P.Name,Font(18,true)).X)+24.f,94.f,220.f);
   // Identity follows the existing configurable side accent. Neutral name text
   // remains readable; inactive GK keeps the identity but not Formula emphasis.
   const float Identity=P.Role==R::Goalkeeper && !P.bFormulaActive?.55f:1.f;
   const FSlateRoundedBoxBrush Plaque(Navy(),6.f,P.Accent.CopyWithNewOpacity(.78f*Identity),1.4f);
   const FSlateRoundedBoxBrush RolePlaque(FMath::Lerp(Navy(),P.Accent,.14f*Identity),5.f,
    P.Accent.CopyWithNewOpacity(.65f*Identity),1.f);
   Box(C+FVector2D(-NameWidth*.5f,39),FVector2D(NameWidth,27),Plaque,FLinearColor::White.CopyWithNewOpacity(Fade));
   Line({C+FVector2D(-NameWidth*.5f+8,40),C+FVector2D(NameWidth*.5f-8,40)},P.Accent.CopyWithNewOpacity(.8f*Identity*Fade),2.f,6);
   Label(P.Name,C+FVector2D(0,51),18,Text().CopyWithNewOpacity(Fade),true);
   Box(C+FVector2D(-32,69),FVector2D(64,23),RolePlaque,FLinearColor::White.CopyWithNewOpacity(Fade));
   Label(RoleText(P.Role),C+FVector2D(0,80),14,Text().CopyWithNewOpacity(Fade));
  }
  const FVector2D Ball=Flight>0?Curve(Flight):BallStart;
  const float Lift=Flight>0?FMath::Sin(Flight*PI)*(bCross?FMath::Lerp(2.5f,.12f,State->CrossLowBlend):1.f):0.f;
  const FVector2D Shadow=Ball+FVector2D(1,8+Lift*17);
  for(int Ring=3;Ring>0;--Ring)
  {
   TArray<FVector2D> Ellipse;
   for(int I=0;I<32;++I){const float A=2*PI*I/32;Ellipse.Add(Shadow+FVector2D(FMath::Cos(A)*(7+Ring*2),FMath::Sin(A)*(2+Ring)));}
   Poly(Ellipse,Color(1,8,12,uint8(22*(1-Lift*.4f))),3);
  }
  if(BallTexture.IsValid())
  {
   const float Diameter=FMath::Lerp(27.f,20.f,Flight);
   const FVector2D Size(Diameter,Diameter);
   const auto BallGeometry=G.MakeChild(Size,FSlateLayoutTransform(Ball-Size*.5f),
    FSlateRenderTransform(FQuat2D(Flight*PI*2.6f)),FVector2D(.5,.5));
   FSlateDrawElement::MakeBox(Out,Layer+6,BallGeometry.ToPaintGeometry(),&BallBrush,ESlateDrawEffect::None,FLinearColor::White);
  }
  else
   Circle(Ball,6,Text(),8);
  if((State->Phase==EPhase::ResultHold || State->IsOutcomeVisualHold()) && State->Facts.Outcome==EOutcome::Goal)
   Circle(Target,16,Mint().CopyWithNewOpacity(.35f),1.5f);
  // Acceleration remains on click/Space; commercial playback has no persistent hint.
  return FMCodexGoalCelebration::Paint(State->Celebration,G,Out,Layer+8);
 }
private:
 FState* State=nullptr;
 mutable TArray<FVector2D> StrokeSegment; // Reuse line storage across preview/flight paints.
 TFunction<void()> Skip;
 FSlateBrush Turf,BallBrush,CrowdBrush,Brushes[5]; FName Ids[5];
 TStrongObjectPtr<UTexture2D> Portraits[5],SceneTurf,BallTexture,StadiumTexture;
};
}
void RefreshSurface(UWidgetTree& Tree,FState& State,TFunction<void()> Skip)
{
 auto* Host=Cast<UNativeWidgetHost>(Tree.FindWidget(TEXT("TacticalScene")));
 if (!Host) return;
 const bool Active=State.Facts.bActive;
 Tree.FindWidget(TEXT("TheaterSpatialLane"))->SetVisibility(Active?ESlateVisibility::SelfHitTestInvisible:ESlateVisibility::Collapsed);
 auto* Bounds=CastChecked<USizeBox>(Tree.FindWidget(TEXT("TacticalSceneBounds")));
 const bool LongShot=Active && !IsCross(State.Facts.Method);
 // Ground may continue beneath the later-painted controls. Keep the
 // scene's layout/hit area unchanged; the Theater root still clips to its bounds.
 Host->SetClipping(LongShot?EWidgetClipping::Inherit:EWidgetClipping::ClipToBounds);
 const bool MethodChoice=LongShot && State.Facts.bMethodChoice;
 for(const auto Name:{TEXT("TheaterNearDirect"),TEXT("TheaterNearCombination")})
 {
  auto* ChoiceBounds=CastChecked<USizeBox>(Tree.FindWidget(Name)->GetParent());
  ChoiceBounds->SetWidthOverride(MethodChoice?448.f:410.f);
  ChoiceBounds->SetHeightOverride(MethodChoice?66.f:62.f);
 }
 for(const auto Name:{TEXT("TheaterDirectExplanationBounds"),TEXT("TheaterAlternativeExplanationBounds")})
  CastChecked<USizeBox>(Tree.FindWidget(Name))->SetWidthOverride(MethodChoice?448.f:410.f);
 Bounds->SetHeightOverride(LongShot?380.f:400.f);
 CastChecked<USizeBox>(Tree.FindWidget(TEXT("TheaterSpatialLane")))->SetHeightOverride(LongShot?340.f:360.f);
 auto* SpatialReserve=CastChecked<USizeBox>(Tree.FindWidget(TEXT("TheaterSpatialReserve")));
 // Lift the LongShot roll stack, preserving its fit scale and pitch origin.
 // Outcome playback retains the accepted space for the low failed-shot endpoint.
 const bool RollPhase=State.Phase==EPhase::Setup || State.Phase==EPhase::Intent || State.Phase==EPhase::FormulaHold;
 const float StackLift=LongShot && RollPhase?24.f:0.f;
 SpatialReserve->SetHeightOverride((LongShot?340.f:360.f)-StackLift);
 SpatialReserve->SetVisibility(Active?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
 Bounds->SetVisibility(Active?ESlateVisibility::Visible:ESlateVisibility::Collapsed);
 if (Active)
 {
  FSlateBrush Turf;
  if (auto* Pitch=Cast<UFMCodexPitchWidget>(Tree.FindWidget(TEXT("DedicatedFootballPitchWidget"))))
   if (auto* Background=Cast<UBorder>(Pitch->GetWidgetFromName(TEXT("PitchBackgroundAssetHook")))) Turf=Background->Background;
  if (!Host->GetContent().IsValid()) Host->SetContent(SNew(SScene));
  StaticCastSharedPtr<SScene>(Host->GetContent())->Refresh(State,Turf,MoveTemp(Skip));
 }
 auto* Top=CastChecked<USizeBox>(Tree.FindWidget(TEXT("TheaterTopBounds")));
 Top->SetHeightOverride(LongShot?124.f:Active?144.f:178.f);
 auto* Center=CastChecked<USizeBox>(Tree.FindWidget(TEXT("TheaterCenterBounds")));
 CastChecked<UOverlaySlot>(Tree.FindWidget(TEXT("TheaterCompositionFit"))->Slot)->SetPadding(FMargin(0,0,0,StackLift));
 Center->ClearHeightOverride();
 if (Active)
 {
  // Fit only the lower controls; the pitch above retains identical geometry
  // across method selection, dice, outcome and result.
  auto* Fit=CastChecked<UScaleBox>(Tree.FindWidget(TEXT("TheaterCompositionFit")));
  Fit->SetStretch(EStretch::ScaleToFit);
  CastChecked<UOverlaySlot>(Fit->Slot)->SetVerticalAlignment(VAlign_Fill);
  auto* Title=CastChecked<UTextBlock>(Tree.FindWidget(TEXT("TheaterTitle")));
  const bool Final=State.Phase==EPhase::ResultHold;
  const bool Cross=IsCross(State.Facts.Method);
  const FText Method=Cross?(State.PreviewMethod==EMethod::CrossLow?NSLOCTEXT("TacticalScene","Low","低球传中"):NSLOCTEXT("TacticalScene","High","高球传中"))
   :State.Facts.Method==EMethod::DeadCorner?NSLOCTEXT("TacticalScene","Corner","射向死角"):NSLOCTEXT("TacticalScene","Method","直接射门");
  auto TitleFont=Title->GetFont(); TitleFont.Size=Final?20:38; Title->SetFont(TitleFont);
  Title->SetText(Cross?(Final || (!State.Facts.bMethodChoice && !State.Facts.bRoutePending)?Method:Title->GetText())
   :Final?FText::Format(NSLOCTEXT("TacticalScene","Identity","远射 · {0}"),Method):NSLOCTEXT("TacticalScene","Title","远射"));
  if(!Cross || !State.Facts.bRoutePending)
   CastChecked<UTextBlock>(Tree.FindWidget(TEXT("TheaterSubtitle")))->SetText(State.Facts.bMethodChoice
    ?(Cross?NSLOCTEXT("TacticalScene","CrossPreview","选择传中方式"):NSLOCTEXT("TacticalScene","Preview","选择远射方式")):Method);
  // Existing TheaterOutcome already contains the disclosure-gated authoritative
  // narrative and semantic rich text. It remains the only result headline.
  Tree.FindWidget(TEXT("TheaterSubtitle"))->SetVisibility(Final?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);
  CastChecked<UBorder>(Tree.FindWidget(TEXT("TheaterBodyPadding")))->SetPadding(FMargin(0,10,0,20));
  if(State.IsAnimating())
  {
   // Formula has completed before Outcome, so its old "waiting for defense"
   // fallback is no longer the action underway. This describes animation only.
   CastChecked<UTextBlock>(Tree.FindWidget(TEXT("TheaterDetail")))->SetText(Cross
    ?(State.Phase==EPhase::Outcome?NSLOCTEXT("TacticalScene","CrossFlight","正在传中"):NSLOCTEXT("TacticalScene","CrossPrepare","准备传中"))
    :State.Phase==EPhase::Outcome?NSLOCTEXT("TacticalScene","ShotInFlight","正在射门"):NSLOCTEXT("TacticalScene","PreparingShot","准备远射"));
  }
  if(State.Facts.bMethodChoice && !Cross)
  {
   Tree.FindWidget(TEXT("TheaterDuel"))->SetVisibility(ESlateVisibility::Collapsed);
   for(const auto Name:{TEXT("TheaterDirectExplanationBounds"),TEXT("TheaterAlternativeExplanationBounds")})
    CastChecked<USizeBox>(Tree.FindWidget(Name))->SetHeightOverride(88.f);
  }
 }
 else Tree.FindWidget(TEXT("TheaterSubtitle"))->SetVisibility(ESlateVisibility::HitTestInvisible);
 auto* Outcome=CastChecked<URichTextBlock>(Tree.FindWidget(TEXT("TheaterOutcome")));
 const int32 ResultFontSize=Active?34:52;
 if(Outcome->GetCurrentDefaultTextStyle().Font.Size!=ResultFontSize)
 {
  auto ResultStyle=Outcome->GetCurrentDefaultTextStyle();
  auto ResultFont=CastChecked<UTextBlock>(Tree.FindWidget(TEXT("TheaterTitle")))->GetFont();ResultFont.Size=ResultFontSize;
  ResultStyle.SetFont(ResultFont);
  auto* Styles=Outcome->GetTextStyleSet();
  for(const auto RowName:{FName(TEXT("Goal")),FName(TEXT("NoGoal"))})
   if(auto* Row=Styles->FindRow<FRichTextStyleRow>(RowName,TEXT("TacticalScene"))) Row->TextStyle.SetFont(ResultFont);
  Outcome->SetTextStyleSet(nullptr);Outcome->SetTextStyleSet(Styles);
  // UE 5.3 SetTextStyleSet resets the live Slate default style even when an
  // override exists. Restore it after updating decorator styles, preserving CJK.
  Outcome->SetDefaultTextStyle(ResultStyle);
 }
 // Existing numeric widgets remain intact. Compact only the participant typography/decoration.
 for (const FString Prefix:{FString(TEXT("TheaterAttack")),FString(TEXT("TheaterDefense"))})
 {
  auto* Body=CastChecked<UVerticalBox>(Tree.FindWidget(FName(*(Prefix+TEXT("SafeContent")))));
  const bool Attack=Prefix==TEXT("TheaterAttack");
  CastChecked<UOverlaySlot>(Body->Slot)->SetPadding(Active
   ?(Attack?FMargin(102,10,18,10):FMargin(18,10,102,10))
   :(Attack?FMargin(160,16,22,16):FMargin(22,16,160,16)));
  auto* Figure=CastChecked<USizeBox>(Tree.FindWidget(FName(*(Prefix+TEXT("SilhouetteBounds")))));
  Figure->SetWidthOverride(Active?84:138); Figure->SetHeightOverride(Active?108:178);
  auto* Side=CastChecked<UTextBlock>(Tree.FindWidget(FName(*(Prefix+TEXT("Side")))));
  auto Font=Side->GetFont(); Font.Size=Active?24:32; Side->SetFont(Font);
  auto* Value=Tree.FindWidget(FName(*(Prefix+TEXT("Value"))));
  if (auto* Slot=Cast<UVerticalBoxSlot>(Value->Slot)) Slot->SetPadding(FMargin(0,Active?6:14,0,0));
  for(int I=0;I<3;++I)
   if (auto* Name=Cast<UTextBlock>(Tree.FindWidget(FName(*(Prefix+TEXT("Name")+FString::FromInt(I))))))
   {auto NameFont=Name->GetFont();NameFont.Size=Active?19:23;Name->SetFont(NameFont);}
 }
}
}
