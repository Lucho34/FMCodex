#pragma once

#include "CoreMinimal.h"

// Production High Cross presentation; only the developer fallback override is non-Shipping.
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
struct FMotion
{
	bool bActive = false;
	bool bLastEnabled = false;
	float Elapsed = 1.f;
	float FieldProgress = 0.f;
	float FieldTransitionFrom = 0.f;
	bool bHasFieldGeometry = false;
	bool bAwaitingFirstFrame = false;
	FVector2D FieldScale = FVector2D(1.f);
	FVector2D FieldTranslation = FVector2D::ZeroVector;
};
bool IsEnabled();
bool WantsTheater(const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed);
UOverlay* Build(UWidgetTree& Tree, UButton*& Primary, UButton*& High, UButton*& Low, UTexture2D* Athletes);
void Refresh(UWidgetTree& Tree, const FFMCodexUMGMatchScreenViewModel& Screen,
	const FFMCodexUMGInlineFormulaSurfaceViewModel& Displayed,
	const FFMCodexUMGMatchHeaderViewModel& DisplayedHeader, bool bRequestPending);
void RefreshReel(UWidgetTree& Tree, const FFMCodexUMGRollReelViewModel& Reel);
void SetActive(UWidgetTree& Tree, FMotion& Motion, bool bActive);
void RefreshHover(UWidgetTree& Tree);
void Tick(UWidgetTree& Tree, FMotion& Motion, float DeltaSeconds);
}
