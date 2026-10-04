#pragma once
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
class AFMCodexLocalMatchPlayerController;
class FFMCodexGuidedLesson1;
class UFMCodexLocalMatchScreenWidget;
class UWidget;
class SWidget;

/** Geometry is only for drawing; existing semantic Screen/Host gates own input. */
namespace FMCodexLesson1Focus
{
TArray<UWidget*> Targets(UFMCodexLocalMatchScreenWidget* Screen, const FFMCodexGuidedLesson1& Lesson);
TSharedRef<SWidget> Build(AFMCodexLocalMatchPlayerController* Controller);
}
#endif
