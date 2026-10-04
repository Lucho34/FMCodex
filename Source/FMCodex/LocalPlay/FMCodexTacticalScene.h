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
enum class EMethod : uint8 { Direct, DeadCorner };
inline constexpr float SetupSeconds=.18f, IntentSeconds=.24f, OutcomeSeconds=.80f;
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
 EMethod Method=EMethod::Direct;
 EMatchPlayResolutionParticipantRole Highlight=EMatchPlayResolutionParticipantRole::None;
 TArray<FParticipant> Participants;
 EOutcome Outcome=EOutcome::None; // Only populated AFTER existing roll/formula disclosure.
};
FFacts Project(const FFMCodexUMGLongShotResolutionViewModel& Shot,
 const FFMCodexUMGMatchHeaderViewModel& Header, bool bAllowed);
struct FState
{
 FFacts Facts;
 EPhase Phase=EPhase::Hidden;
 float Elapsed=0.f;
 EMethod PreviewMethod=EMethod::Direct;
 float CornerBlend=0.f;
 float PulseTime=0.f;
 bool bEnteredFromPreview=false;
 void Preview(EMethod Method);
 void Sync(const FFacts& Next);
 bool Tick(float DeltaSeconds);
 bool Skip();
 bool IsAnimating() const;
 float Progress() const;
 void Gate(FFMCodexUMGInlineFormulaSurfaceViewModel& View) const;
 void Gate(FFMCodexUMGLongShotResolutionViewModel& View) const;
};
void UpdatePreview(UWidgetTree& Tree, FState& State);
/** Cached Slate surface is refreshed only with safe facts; painting never loads assets. */
void RefreshSurface(UWidgetTree& Tree, FState& State, TFunction<void()> Skip);
}
