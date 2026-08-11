#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexPitchSlotWidget.generated.h"

class UBorder;
class UDragDropOperation;
class UFMCodexDeploymentDragDropOperation;
class UFMCodexPlayerCardWidget;
class UTextBlock;
class UVerticalBox;
class SWidget;

DECLARE_MULTICAST_DELEGATE_ThreeParams(
	FFMCodexPitchDeploymentDropped, FName, FName, bool);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexPitchSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexPitchSlotWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Pitch Presentation")
	void RefreshFromPitchSlotPresentation(
		const FFMCodexUMGPitchSlotViewModel& InPresentation);

	const FFMCodexUMGPitchSlotViewModel& GetPresentation() const;
	UFMCodexPlayerCardWidget* GetCardWidget() const;
	bool IsShowingOccupiedCard() const;
	bool CanAcceptDeploymentOperation(
		const UFMCodexDeploymentDragDropOperation* Operation) const;
	bool TryHandleDeploymentDrop(
		UFMCodexDeploymentDragDropOperation* Operation);

	FFMCodexPitchDeploymentDropped OnDeploymentDropped;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnDragEnter(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Pitch Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGPitchSlotViewModel Presentation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
		Category = "Local Match|Pitch Presentation",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UFMCodexPlayerCardWidget> PlayerCardWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SlotLabelText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContextText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TargetStateText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ContentBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EmptyStateText;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexPlayerCardWidget> CardWidget;

	bool bDragHovered = false;
};
