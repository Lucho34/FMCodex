#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexTacticalDetailPanelWidget.generated.h"

class UTextBlock;
class UWrapBox;
class SWidget;

DECLARE_MULTICAST_DELEGATE(FFMCodexTacticalDetailPointerEvent);

/** Shared, non-modal read-only surface for one hovered tactical card. */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexTacticalDetailPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Local Match|Tactical Detail")
	void RefreshFromPresentation(
		const FFMCodexUMGTacticalDetailViewModel& InPresentation);

	void ClearPresentation();
	const FFMCodexUMGTacticalDetailViewModel& GetPresentation() const;
	FString CollectPlayerFacingText() const;

	FFMCodexTacticalDetailPointerEvent OnDetailPointerEntered;
	FFMCodexTacticalDetailPointerEvent OnDetailPointerLeft;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnMouseEnter(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Tactical Detail",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGTacticalDetailViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> BranchBody;
};
