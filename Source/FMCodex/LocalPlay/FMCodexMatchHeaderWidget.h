#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexMatchHeaderWidget.generated.h"

class UBorder;
class UTextBlock;
class SWidget;

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexMatchHeaderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexMatchHeaderWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Header Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGMatchHeaderViewModel& InPresentation);

	const FFMCodexUMGMatchHeaderViewModel& GetPresentation() const;
	FString GetDisplayedScoreLabel() const;
	FString GetDisplayedAttackerLabel() const;
	FString GetDisplayedActorLabel() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Header Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGMatchHeaderViewModel Presentation;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> MatchStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerAIdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerBIdentityText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerAScoreText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PlayerBScoreText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> AttackerStatusRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> AttackerStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> ActorStatusRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActorStatusText;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> FinalResultRegion;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> FinalResultText;
};
