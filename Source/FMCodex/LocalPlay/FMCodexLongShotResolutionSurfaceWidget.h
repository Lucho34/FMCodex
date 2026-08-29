#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexLongShotResolutionSurfaceWidget.generated.h"

class UBorder;
class UButton;
class UHorizontalBox;
class UTextBlock;
class UFMCodexInlineResolutionFormulaSurfaceWidget;
class UFMCodexInteractionOptionWidget;
class UFMCodexRollReelWidget;
class SWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFMCodexLongShotContinueRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexLongShotBranchRequested,
	EFMCodexUMGBranchIntent, Intent);

/** Shared LongShot/CutInside/PassControl resolution and Cross choice renderer. */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexLongShotResolutionSurfaceWidget final
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexLongShotResolutionSurfaceWidget(
		const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Long Shot")
	void RefreshFromPresentation(
		const FFMCodexUMGLongShotResolutionViewModel& InPresentation);

	const FFMCodexUMGLongShotResolutionViewModel& GetPresentation() const;
	UFMCodexRollReelWidget* GetRollReelWidget() const;
	UFMCodexInlineResolutionFormulaSurfaceWidget* GetFormulaSurface() const;
	const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>&
		GetBranchChoiceWidgets() const;

#if WITH_DEV_AUTOMATION_TESTS
	void ResetBranchDispatchForTesting();
	int32 GetBranchDispatchCountForTesting() const;
	EFMCodexUMGBranchIntent GetLastBranchDispatchForTesting() const;
#endif

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Long Shot")
	FFMCodexLongShotContinueRequested OnContinueRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Long Shot")
	FFMCodexLongShotBranchRequested OnBranchRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals(bool bPreserveChoices = false);
	void RebuildBranchChoices();

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleBranchClicked(EFMCodexUMGBranchIntent Intent);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Long Shot", meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGLongShotResolutionViewModel Presentation;

	UPROPERTY(Transient) TObjectPtr<UTextBlock> TitleText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> BranchText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StageText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> StatusText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> HintText;
	UPROPERTY(Transient) TObjectPtr<UHorizontalBox> BranchChoiceRow;
	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexInteractionOptionWidget>> BranchChoiceWidgets;
	UPROPERTY(Transient) TObjectPtr<UBorder> DiceRevealRegion;
	UPROPERTY(Transient) TObjectPtr<UFMCodexRollReelWidget> RollReel;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> PairedRollText;
	UPROPERTY(Transient)
	TObjectPtr<UFMCodexInlineResolutionFormulaSurfaceWidget> FormulaSurface;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> ResultTitleText;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> NarrativeText;
	UPROPERTY(Transient) TObjectPtr<UButton> ContinueButton;

#if WITH_DEV_AUTOMATION_TESTS
	int32 BranchDispatchCountForTesting = 0;
	EFMCodexUMGBranchIntent LastBranchDispatchForTesting =
		EFMCodexUMGBranchIntent::None;
#endif
};
