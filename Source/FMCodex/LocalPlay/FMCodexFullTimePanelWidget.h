#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "FMCodexFullTimePresentation.h"
#include "FMCodexFullTimePanelWidget.generated.h"

class UTextBlock;
class UButton;
class UBorder;
class UScrollBox;

/** Native vector-like broadcast decoration. No screenshot/background texture. */
UCLASS()
class FMCODEX_API UFMCodexFullTimeArtworkWidget : public UWidget
{
	GENERATED_BODY()
public:
	bool bFootballOnly = false;
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
};

/** Acknowledgement is local presentation only and never advances the match. */
UCLASS()
class FMCODEX_API UFMCodexFullTimePanelWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void RefreshFromPresentation(const FFMCodexFullTimePresentation& InPresentation);
	const FFMCodexFullTimePresentation& GetPresentation() const { return Presentation; }
	bool IsAcknowledged() const { return bAcknowledged; }
	UFUNCTION() void Acknowledge();
	UButton* GetConfirmButton() const { return ConfirmButton; }
protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual FReply NativeOnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
	void BuildWidgetTree();
	void RefreshVisuals();
	UPROPERTY() FFMCodexFullTimePresentation Presentation;
	UPROPERTY() TObjectPtr<UTextBlock> PlayerAName;
	UPROPERTY() TObjectPtr<UTextBlock> PlayerBName;
	UPROPERTY() TObjectPtr<UTextBlock> TeamAName;
	UPROPERTY() TObjectPtr<UTextBlock> TeamBName;
	UPROPERTY() TObjectPtr<UTextBlock> TeamAScore;
	UPROPERTY() TObjectPtr<UTextBlock> TeamBScore;
	UPROPERTY() TObjectPtr<UTextBlock> TeamABadge;
	UPROPERTY() TObjectPtr<UTextBlock> TeamBBadge;
	UPROPERTY() TObjectPtr<UBorder> TeamABadgeBorder;
	UPROPERTY() TObjectPtr<UBorder> TeamBBadgeBorder;
	UPROPERTY() TObjectPtr<UScrollBox> TeamAGoals;
	UPROPERTY() TObjectPtr<UScrollBox> TeamBGoals;
	UPROPERTY() TObjectPtr<UButton> ConfirmButton;
	UPROPERTY() TObjectPtr<UTextBlock> ConfirmText;
	bool bAcknowledged = false;
};
