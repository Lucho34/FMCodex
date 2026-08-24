#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexRollReelWidget.generated.h"

class UBorder;
class UOverlay;
class UTextBlock;
class SWidget;

/**
 * Reusable clipped vertical number reel. It renders already-projected cosmetic
 * geometry and never reads Match State, rolls, converts, or calculates values.
 */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexRollReelWidget final : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexRollReelWidget(const FObjectInitializer& ObjectInitializer);

	void RefreshFromPresentation(
		const FFMCodexUMGRollReelViewModel& InPresentation);

	const FFMCodexUMGRollReelViewModel& GetPresentation() const;
	int32 GetStripDigitCount() const;
	int32 GetRenderedChildCount() const;
	float GetCenterVerticalOffset() const;
	float GetCenterRenderScale() const;
	float GetCenterRenderOpacity() const;
	float GetMaximumNeighborRenderOpacity() const;
	FLinearColor GetFrameBrushColor() const;
	const UTextBlock* GetCenterDigitWidget() const;
	bool HasClippedWindow() const;
	int32 GetVisibleNeighborDigitCount() const;
	bool IsStaticResultTileVisible() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Roll Reel",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGRollReelViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ReelFrame;

	UPROPERTY(Transient)
	TObjectPtr<UOverlay> NumberStrip;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PreviousText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CenterText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NextText;

	float LastCenterOffset = 0.0f;
	float LastCenterScale = 1.0f;
	int32 LastPreviousValue = MIN_int32;
	int32 LastCenterValue = MIN_int32;
	int32 LastNextValue = MIN_int32;
	bool bHasRenderedVisualState = false;
	bool bLastShowNeighborDigits = false;
};
