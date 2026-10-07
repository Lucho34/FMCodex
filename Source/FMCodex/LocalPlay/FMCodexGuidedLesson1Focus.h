#pragma once
#include "CoreMinimal.h"
#if !UE_BUILD_SHIPPING
#include "Fonts/SlateFontInfo.h"
#include "Layout/Geometry.h"
class AFMCodexLocalMatchPlayerController;
class FFMCodexGuidedLesson1;
class UFMCodexLocalMatchScreenWidget;
class UWidget;
class SWidget;

/** Geometry is only for drawing; existing semantic Screen/Host gates own input. */
namespace FMCodexLesson1Focus
{
struct FGlyphDots
{
	TArray<float> Centers;
	FVector2D TextSize=FVector2D::ZeroVector;
	float Diameter=0.f, Gap=0.f;
	float ExtraHeight() const { return Gap+Diameter; }
};
/** Lesson keywords are CJK/Latin. Measures actual advances, never width/count. */
FGlyphDots MeasureGlyphDots(const FString& Text, const FSlateFontInfo& Font, float Scale=1.f);
enum class ETargetShape : uint8 { Card, Button, FullCardRow, DeploymentSlot, FormulaValue, Content };
TOptional<FSlateRect> ConvertTargetGeometry(const FGeometry& TargetDesktop, const FGeometry& OverlayDesktop, ETargetShape Shape);
TOptional<FSlateRect> TargetRect(UWidget* Target, const FGeometry& OverlayDesktop, ETargetShape Shape);
TArray<UWidget*> Targets(UFMCodexLocalMatchScreenWidget* Screen, const FFMCodexGuidedLesson1& Lesson);
TSharedRef<SWidget> Build(AFMCodexLocalMatchPlayerController* Controller);
}
#endif
