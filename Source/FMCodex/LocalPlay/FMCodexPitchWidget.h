#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPitchWidget.generated.h"

class UFMCodexPitchSlotWidget;
class UVerticalBox;
class SWidget;

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

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Pitch Presentation",
		meta = (AllowPrivateAccess = "true"))
	TArray<FFMCodexUMGPitchRegionViewModel> Presentation;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Pitch Presentation")
	TSubclassOf<UFMCodexPitchSlotWidget> PitchSlotWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> FieldBody;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexPitchSlotWidget>> RenderedSlotWidgets;
};
