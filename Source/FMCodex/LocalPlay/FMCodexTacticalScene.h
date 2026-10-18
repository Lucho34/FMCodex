#pragma once
#include "CoreMinimal.h"
#include "FMCodexGoalCelebration.h"
#include "FMCodexLocalMatchUMGPresentation.h"

class UWidgetTree;
class SWidget;
struct FStreamableHandle;

/** Local presentation only. No session, RNG, command or deployment coordinates. */
namespace FMCodexTacticalScene
{
enum class EPhase : uint8 { Hidden, Preview, Setup, Intent, FormulaHold, Outcome, ResultHold };
enum class EOutcome : uint8 { None, Goal, ImmediateMiss, DefensiveSuccess };
enum class EMethod : uint8 { Direct, DeadCorner, CrossHigh, CrossLow };
inline constexpr float SetupSeconds=.18f, IntentSeconds=.24f;
inline constexpr float OutcomeActionSeconds=1.15f, OutcomeHoldSeconds=.30f;
inline constexpr float OutcomeSeconds=OutcomeActionSeconds+OutcomeHoldSeconds;
inline constexpr float LongShotPreviewCycleSeconds=2.2f, PreviewTransitionSeconds=.15f;
inline constexpr float LongShotPressureArrival=.44f;
inline constexpr float CrossSetupSeconds=.5f, CrossIntentSeconds=.8f;
inline constexpr float CrossOutcomeActionSeconds=1.44f, CrossOutcomeHoldSeconds=.30f;
inline constexpr float CrossOutcomeSeconds=CrossOutcomeActionSeconds+CrossOutcomeHoldSeconds;
inline constexpr float CrossDeliveryFraction=.68f;
inline constexpr float CrossPendingDelivery=.56f;
inline constexpr float CrossArrivalFraction=.90f;
inline constexpr float CrossIdleCycleSeconds=3.f;
bool IsCross(EMethod Method);
// A close view of the attacking third; the attacking goal is always on the right.
inline const FVector2D CarrierAnchor(455,206), MarkerAnchor(720,250), KeeperAnchor(1140,142);
inline const FVector2D GoalAnchor(1195,145), CornerAnchor(1115,100);
FVector2D OutcomeTarget(EOutcome Outcome, EMethod Method);
inline const FVector2D CrossGoalAnchor(1195,245);
bool IsInsideGoal(FVector2D Point, EMethod Method=EMethod::Direct);
struct FParticipant
{
 FName CardId;
 EMatchPlayResolutionParticipantRole Role=EMatchPlayResolutionParticipantRole::None;
 EInitialTurnOrderPlayer Side=EInitialTurnOrderPlayer::None;
 FText Name;
 FLinearColor Accent=FLinearColor::White;
 bool bFormulaActive=false;
};
struct FFacts
{
 bool bActive=false;
 int64 AttackSequence=0;
 bool bMethodChoice=false;
 bool bRoutePending=false;
 bool bManualWaiting=false;
 EMethod Method=EMethod::Direct;
 EMatchPlayResolutionParticipantRole Highlight=EMatchPlayResolutionParticipantRole::None;
 TArray<FParticipant> Participants;
 EOutcome Outcome=EOutcome::None; // Only populated AFTER existing roll/formula disclosure.
};
FFacts Project(const FFMCodexUMGLongShotResolutionViewModel& Shot,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed);
FFacts ProjectCross(const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed);
// Presentation time starts only after two resource-ready, stable painted frames.
// Large/stale frame deltas never stand in for time spent seeing the scene.
inline constexpr float CrossIntroSeconds=.35f, CrossIntroHoldSeconds=1.1f;
struct FCrossEntrance
{
 uint64 LastPaintFrame=MAX_uint64;
 FVector2D PaintedSize=FVector2D::ZeroVector;
 int32 StablePaints=0;
 bool bStarted=false;
 float Elapsed=0.f;
 void Painted(uint64 Frame,FVector2D Size,bool bResourcesReady);
 void Tick(float DeltaSeconds,uint64 Frame);
 bool RecentlyPainted(uint64 Frame) const;
 float Progress() const {return FMath::Clamp(Elapsed/CrossIntroSeconds,0.f,1.f);}
 bool CanAutoRoll(uint64 Frame) const {return RecentlyPainted(Frame) && Elapsed>=CrossIntroSeconds+CrossIntroHoldSeconds;}
};
struct FState
{
 FCrossEntrance CrossEntrance;
 FFacts Facts;
 EPhase Phase=EPhase::Hidden;
 float Elapsed=0.f;
 EMethod PreviewMethod=EMethod::Direct;
 float CornerBlend=0.f;
 float CrossLowBlend=0.f;
 float CrossEntryMotion=0.f;
 float LongShotEntryPressure=0.f;
 float PulseTime=0.f;
 float CrossIdleTime=0.f;
 float CrossIdleSettle=1.f;
 bool IsCrossIdle() const;
 float CrossIdleBreath() const;
 float CrossIdleDashOffset() const;
 float CrossWaitingMotion() const;
 float CrossWaitingDelivery() const;
 float CrossWaitingBallOpacity() const;
 bool bEnteredFromPreview=false;
 FMCodexGoalCelebration::FState Celebration;
 void Preview(EMethod Method);
 void Sync(const FFacts& Next);
 bool Tick(float DeltaSeconds);
 bool Skip();
 bool IsAnimating() const;
 float Progress() const;
 float PhaseSeconds() const;
 float CrossOutcomeActionProgress() const;
 bool IsCrossOutcomeVisualHold() const;
 float OutcomeActionProgress() const;
 bool IsOutcomeVisualHold() const;
 void Gate(FFMCodexUMGInlineFormulaSurfaceViewModel& View) const;
 void Gate(FFMCodexUMGLongShotResolutionViewModel& View) const;
};
/** Semantic movement only, shared by rendering and focused presentation checks. */
// Camera follows the attacking goal for either side. High originates on the
// far flank; Low cuts back from the near-side byline, outside the goalmouth.
// These are role anchors, never player-specific or gameplay coordinates.
inline const FVector2D CrossCarrierWideAnchor(248,166), CrossCarrierBylineAnchor(1080,454);
FVector2D CrossParticipantAnchor(const FState& State, EMatchPlayResolutionParticipantRole Role);
FVector2D CrossReceiveZone(float LowBlend);
FVector2D CrossDeliveryControl(float LowBlend);
FVector2D CrossBallStart(const FState& State);
/** An attempted delivery stops short of the receiving zone; no controlled touch/result. */
FVector2D CrossIntentBall(const FState& State);
/** Goal or aggregate defense only: never attributes a touch to a participant. */
FVector2D CrossOutcomeBall(const FState& State, float Flight);
/** Pressure is an illustration of an actual Formula role, never a block fact. */
bool HasLongShotDefensivePressure(const FState& State);
FVector2D LongShotParticipantAnchor(const FState& State, EMatchPlayResolutionParticipantRole Role);
FVector2D LongShotAimControl(const FState& State);
FVector2D LongShotOutcomeBall(const FState& State, float Flight);
void UpdatePreview(UWidgetTree& Tree, FState& State);
TSharedPtr<FStreamableHandle> PreloadCrossAssets();
void WarmCrossSurface(UWidgetTree& Tree);
void AnimateCrossEntrance(UWidgetTree& Tree,const FState& State);
/** Cross-only aspect adaptation. Updates layout only when viewport dimensions change. */
void FitCrossViewport(UWidgetTree& Tree,bool bCross,FVector2D Viewport);
/** Cached Slate surface is refreshed only with safe facts; painting never loads assets. */
void RefreshSurface(UWidgetTree& Tree, FState& State, TFunction<void()> Skip);
}
