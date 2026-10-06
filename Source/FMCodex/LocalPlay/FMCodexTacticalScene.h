#pragma once
#include "CoreMinimal.h"
#include "FMCodexLocalMatchUMGPresentation.h"

class UWidgetTree;
class SWidget;

/** Local presentation only. No session, RNG, command or deployment coordinates. */
namespace FMCodexTacticalScene
{
enum class EPhase : uint8 { Hidden, Preview, Setup, Intent, FormulaHold, Outcome, ResultHold };
enum class EOutcome : uint8 { None, Goal, ImmediateMiss, DefensiveSuccess };
enum class EMethod : uint8 { Direct, DeadCorner, CrossHigh, CrossLow };
inline constexpr float SetupSeconds=.18f, IntentSeconds=.24f, OutcomeSeconds=.80f;
inline constexpr float CrossSetupSeconds=.5f, CrossIntentSeconds=.8f;
inline constexpr float CrossOutcomeActionSeconds=1.20f, CrossOutcomeHoldSeconds=.30f;
inline constexpr float CrossOutcomeSeconds=CrossOutcomeActionSeconds+CrossOutcomeHoldSeconds;
inline constexpr float CrossDeliveryFraction=.68f;
inline constexpr float CrossArrivalFraction=.90f;
bool IsCross(EMethod Method);
// A close view of the attacking third; the attacking goal is always on the right.
inline const FVector2D CarrierAnchor(345,218), MarkerAnchor(650,272), KeeperAnchor(1040,210);
inline const FVector2D GoalAnchor(1195,145), CornerAnchor(1250,178);
FVector2D OutcomeTarget(EOutcome Outcome, EMethod Method);
bool IsInsideGoal(FVector2D Point);
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
 EMethod Method=EMethod::Direct;
 EMatchPlayResolutionParticipantRole Highlight=EMatchPlayResolutionParticipantRole::None;
 TArray<FParticipant> Participants;
 EOutcome Outcome=EOutcome::None; // Only populated AFTER existing roll/formula disclosure.
};
FFacts Project(const FFMCodexUMGLongShotResolutionViewModel& Shot,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed);
FFacts ProjectCross(const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed);
struct FState
{
 FFacts Facts;
 EPhase Phase=EPhase::Hidden;
 float Elapsed=0.f;
 EMethod PreviewMethod=EMethod::Direct;
 float CornerBlend=0.f;
 float CrossLowBlend=0.f;
 float CrossEntryMotion=0.f;
 float PulseTime=0.f;
 bool bEnteredFromPreview=false;
 void Preview(EMethod Method);
 void Sync(const FFacts& Next);
 bool Tick(float DeltaSeconds);
 bool Skip();
 bool IsAnimating() const;
 float Progress() const;
 float PhaseSeconds() const;
 float CrossOutcomeActionProgress() const;
 bool IsCrossOutcomeVisualHold() const;
 void Gate(FFMCodexUMGInlineFormulaSurfaceViewModel& View) const;
 void Gate(FFMCodexUMGLongShotResolutionViewModel& View) const;
};
/** Semantic movement only, shared by rendering and focused presentation checks. */
// Far-touchline origin; Low advances toward the byline, then cuts back into
// the box. These are illustrative anchors, never deployment/world coordinates.
inline const FVector2D CrossCarrierWideAnchor(620,58), CrossCarrierBylineAnchor(930,45);
FVector2D CrossParticipantAnchor(const FState& State, EMatchPlayResolutionParticipantRole Role);
FVector2D CrossReceiveZone(float LowBlend);
FVector2D CrossDeliveryControl(float LowBlend);
FVector2D CrossBallStart(const FState& State);
/** Goal or aggregate defense only: never attributes a touch to a participant. */
FVector2D CrossOutcomeBall(const FState& State, float Flight);
void UpdatePreview(UWidgetTree& Tree, FState& State);
/** Cached Slate surface is refreshed only with safe facts; painting never loads assets. */
void RefreshSurface(UWidgetTree& Tree, FState& State, TFunction<void()> Skip);
}
