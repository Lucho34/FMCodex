#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexSelectionFeedbackToastWidget.generated.h"

class UBorder;
class UTextBlock;
class SWidget;

/** Reusable, non-modal lower-screen feedback surface. */
UCLASS(Blueprintable)
class FMCODEX_API UFMCodexSelectionFeedbackToastWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowFeedback(
		EFMCodexUMGSelectionFeedbackReason InReason,
		const FString& InLabel);
	void DismissFeedback();

	EFMCodexUMGSelectionFeedbackReason GetCurrentReason() const;
	FText GetDisplayedText() const;
	float GetDisplayDurationSeconds() const;
	int32 GetTriggerSerial() const;
	bool IsFeedbackVisible() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

private:
	void BuildWidgetTree();
	void ClearDismissTimer();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ToastFrame;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ToastText;

	EFMCodexUMGSelectionFeedbackReason CurrentReason =
		EFMCodexUMGSelectionFeedbackReason::None;
	FTimerHandle DismissTimerHandle;
	int32 TriggerSerial = 0;
	static constexpr float DisplayDurationSeconds = 2.0f;
};
