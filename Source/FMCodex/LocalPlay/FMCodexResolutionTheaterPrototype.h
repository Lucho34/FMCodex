#pragma once

#include "CoreMinimal.h"

// Production Cross and Free Kick presentation; developer fallback overrides are non-Shipping.
class UWidgetTree;
class UOverlay;
class UButton;
class UTexture2D;
struct FFMCodexUMGMatchScreenViewModel;
struct FFMCodexUMGMatchHeaderViewModel;
struct FFMCodexUMGInlineFormulaSurfaceViewModel;
struct FFMCodexUMGRollReelViewModel;

namespace FMCodexResolutionTheaterPrototype
{
// Transient inspection only; facts are copied from the actor-safe projection.
struct FTakerInspection
{
	bool bActive = false;
	bool bLongFreeKick = false;
	bool bPenalty = false;
	FName HoveredId = NAME_None;
	TMap<FName, bool> CombinationEligibility;
	uint32 Generation = 0;
};
void ClearTakerInspection(UWidgetTree& Tree, FTakerInspection& Inspection);
struct FMotion
{
	bool bActive = false;
	bool bLastEnabled = false;
	bool bLastLowEnabled = false;
	bool bLastNearEnabled = false;
	bool bLastLongEnabled = false;
	bool bLastPenaltyEnabled = false;
	float Elapsed = 1.f;
	float FieldProgress = 0.f;
	float FieldTransitionFrom = 0.f;
	bool bHasFieldGeometry = false;
	bool bAwaitingFirstFrame = false;
	FVector2D FieldScale = FVector2D(1.f);
	FVector2D FieldTranslation = FVector2D::ZeroVector;
};
bool IsEnabled();
// Production default in all targets; only Development exposes the fallback.
bool IsLowCrossEnabled();
// Production defaults in all targets; only Development exposes Near/Long fallbacks.
bool IsNearFreeKickEnabled();
bool IsLongFreeKickEnabled();
// Production default in all targets; only Development exposes the Penalty fallback.
bool IsPenaltyEnabled();
bool IsFormulaContest(FName ContestId);
bool WantsTheater(const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed);
UOverlay* Build(UWidgetTree& Tree, UButton*& Primary, UButton*& High, UButton*& Low, UTexture2D* Athletes);
void Refresh(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed,
	const FFMCodexUMGMatchHeaderViewModel& DisplayedHeader, bool bRequestPending, FTakerInspection& Inspection);
void RefreshReel(UWidgetTree& Tree, const FFMCodexUMGRollReelViewModel& Reel);
void SetActive(UWidgetTree& Tree, FMotion& Motion, bool bActive);
void RefreshHover(UWidgetTree& Tree);
void Tick(UWidgetTree& Tree, FMotion& Motion, float DeltaSeconds);
}
