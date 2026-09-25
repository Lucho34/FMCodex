#pragma once

#include "CoreMinimal.h"
#include "Components/Border.h"
#include "FMCodexRollPresentationSurface.generated.h"

/** Visual variants share the same reel projection/clock. Future styles are not implemented. */
enum class EFMCodexRollVisualVariant : uint8
{
	Legacy,
	TheaterInline
};

/** Lightweight roll-only frame. Paints decoration; owns no roll or timing state. */
UCLASS()
class FMCODEX_API UFMCodexRollPresentationSurface final : public UBorder
{
	GENERATED_BODY()
public:
	bool bNumberChamber = false;
	EFMCodexRollVisualVariant VisualVariant = EFMCodexRollVisualVariant::Legacy;
	/** Decoration follows the existing projected neighbor fade, never a private timer. */
	void SetLockEmphasis(float InEmphasis);
	float GetLockEmphasis() const { return LockEmphasis; }
	/** Normalized activation supplied by the Screen's existing Cycling clock. */
	void SetActivationProgress(float InProgress);
	float GetActivationProgress() const { return ActivationProgress; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
private:
	float LockEmphasis = 0.0f;
	float ActivationProgress = 1.0f;
};
