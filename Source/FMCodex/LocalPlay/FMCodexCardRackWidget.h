#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexCardRackWidget.generated.h"

class UTextBlock;
class UUniformGridPanel;
class UFMCodexPlayerCardWidget;
class SWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(
	FFMCodexRackCardDragStarted, FName, bool);
DECLARE_MULTICAST_DELEGATE(FFMCodexRackCardDragFinished);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexCardRackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexCardRackWidget(const FObjectInitializer& ObjectInitializer);

	void RefreshFromPresentation(
		const FFMCodexUMGCardRackViewModel& InPresentation);

	const FFMCodexUMGCardRackViewModel& GetPresentation() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetRenderedCardWidgets() const;
	int32 GetRenderedCellCount() const;
	EFMCodexUMGCardInteractionState GetCellInteractionState(
		int32 StableIndex) const;

	FFMCodexRackCardDragStarted OnCardDragStarted;
	FFMCodexRackCardDragFinished OnCardDragFinished;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void HandleCardDragStarted(FName CardId, bool bGoalkeeper);
	void HandleCardDragFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Rack Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGCardRackViewModel Presentation;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Rack Presentation")
	TSubclassOf<UFMCodexPlayerCardWidget> PlayerCardWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RackHeading;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TacticalPlayerCountText;

	UPROPERTY(Transient)
	TObjectPtr<UUniformGridPanel> RackGrid;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexPlayerCardWidget>> RenderedCardWidgets;

	int32 RenderedCellCount = 0;
};
