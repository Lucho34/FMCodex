#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPitchSlotWidget.generated.h"

class UBorder;
class UFMCodexPlayerCardWidget;
class UTextBlock;
class UVerticalBox;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexPitchSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Local Match|Pitch Presentation")
	void RefreshFromPitchSlotPresentation(
		const FFMCodexUMGPitchSlotViewModel& InPresentation);

	const FFMCodexUMGPitchSlotViewModel& GetPresentation() const;
	UFMCodexPlayerCardWidget* GetCardWidget() const;
	bool IsShowingOccupiedCard() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Pitch Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGPitchSlotViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SlotLabelText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContextText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ContentBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EmptyStateText;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPlayerCardWidget> CardWidget;
};
