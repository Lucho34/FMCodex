#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "FMCodexBroadcastPanel.generated.h"

/** Decorative geometry only; color is supplied by the existing presentation. */
enum class EFMCodexBroadcastSurface : uint8
{
	Panel, HeaderLeft, HeaderRight, Score, Progress, Dock, Prompt, Instruction,
	RosterHeading, PitchSurround, PitchHUD, TurfLighting, Brand, MidfieldLeft, MidfieldRight
};

UCLASS()
class FMCODEX_API UFMCodexBroadcastPanel : public UBorder
{
	GENERATED_BODY()
public:
	EFMCodexBroadcastSurface Surface = EFMCodexBroadcastSurface::Panel;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** Same UButton input behavior, with a matte lighting pass over its role brush. */
UCLASS()
class FMCODEX_API UFMCodexBroadcastButton : public UButton
{
	GENERATED_BODY()
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};
