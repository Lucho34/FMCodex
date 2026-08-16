#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPitchWidget.generated.h"

class UFMCodexPitchSlotWidget;
class UCanvasPanel;
class SWidget;

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FFMCodexPitchWidgetDeploymentDropped, FName, FName, bool);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexPitchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexPitchWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Pitch Presentation")
	void RefreshFromPitchPresentation(
		const TArray<FFMCodexUMGPitchRegionViewModel>& InPresentation);

	const TArray<FFMCodexUMGPitchRegionViewModel>& GetPresentation() const;
	const TArray<TObjectPtr<UFMCodexPitchSlotWidget>>&
		GetRenderedSlotWidgets() const;
	void BeginDeploymentDrag(
		FName CardId,
		const TArray<FFMCodexUMGDeploymentChoiceViewModel>& Choices);
	void EndDeploymentDrag();
	FName GetActiveDeploymentCardId() const;

	FFMCodexPitchWidgetDeploymentDropped OnDeploymentDropped;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void HandleSlotDeploymentDropped(
		FName CardId, FName SlotId, bool bGoalkeeper);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Pitch Presentation",
		meta = (AllowPrivateAccess = "true"))
	TArray<FFMCodexUMGPitchRegionViewModel> Presentation;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Pitch Presentation")
	TSubclassOf<UFMCodexPitchSlotWidget> PitchSlotWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UCanvasPanel> FieldCanvas;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexPitchSlotWidget>> RenderedSlotWidgets;

	FName ActiveDeploymentCardId = NAME_None;
};
