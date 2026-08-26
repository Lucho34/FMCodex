#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexThroughBallResolutionSurfaceWidget.generated.h"

class UBorder;
class UButton;
class UTextBlock;
class UFMCodexInlineResolutionFormulaSurfaceWidget;
class UFMCodexRollReelWidget;
class SWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexThroughBallContinueRequested);

/**
 * Production renderer for the read-only ThroughBall semantic shell.
 * It does not read Match State, map D6 ranges, calculate legality, or mutate play.
 */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexThroughBallResolutionSurfaceWidget final
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexThroughBallResolutionSurfaceWidget(
		const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Through Ball")
	void RefreshFromPresentation(
		const FFMCodexUMGThroughBallResolutionViewModel& InPresentation);

	const FFMCodexUMGThroughBallResolutionViewModel& GetPresentation() const;
	UFMCodexRollReelWidget* GetRollReelWidget() const;
	UFMCodexInlineResolutionFormulaSurfaceWidget* GetFormulaSurface() const;

	UFUNCTION(BlueprintCallable, Category = "Local Match|Through Ball")
	void RequestContinue();

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Through Ball")
	FFMCodexThroughBallContinueRequested OnContinueRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UFUNCTION()
	void HandleContinueClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Through Ball",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGThroughBallResolutionViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RouteText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StageText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> DiceRevealRegion;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexRollReelWidget> RollReel;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RouteResultText;

	UPROPERTY(Transient)
	TObjectPtr<UFMCodexInlineResolutionFormulaSurfaceWidget> FormulaSurface;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActionPromptText;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;
};
