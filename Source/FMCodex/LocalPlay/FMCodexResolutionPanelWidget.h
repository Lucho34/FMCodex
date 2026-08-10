#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexResolutionPanelWidget.generated.h"

class UFMCodexDiceResultWidget;
class UBorder;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UWrapBox;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexResolutionPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexResolutionPanelWidget(
		const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Resolution Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGResolutionViewModel& InPresentation);

	const FFMCodexUMGResolutionViewModel& GetPresentation() const;
	const TArray<TObjectPtr<UFMCodexDiceResultWidget>>&
		GetRenderedDiceWidgets() const;
	int32 GetRenderedComparisonCount() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshDiceResults();
	void RefreshComparisonEvidence();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Resolution Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGResolutionViewModel Presentation;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Resolution Presentation")
	TSubclassOf<UFMCodexDiceResultWidget> DiceResultWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> AcceptedResultBody;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EmptyStateText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RejectionBody;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> RejectionRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RejectionReasonText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RejectionMessageText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StepTitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> StepSummaryText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> RouteText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> DiceSection;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> DiceBody;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ComparisonSection;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> ComparisonBody;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> DecisionSection;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> DecisionText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> ContinuationSection;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContinuationText;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> TerminalSection;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> TerminalRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TerminalText;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexDiceResultWidget>> RenderedDiceWidgets;

	int32 RenderedComparisonCount = 0;
};
