#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexDiceResultWidget.generated.h"

class UTextBlock;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexDiceResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexDiceResultWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Resolution Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGDiceResultViewModel& InPresentation);

	const FFMCodexUMGDiceResultViewModel& GetPresentation() const;
	int32 GetDisplayedRawD6() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Resolution Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGDiceResultViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContextText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PurposeText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RawValueText;
};
