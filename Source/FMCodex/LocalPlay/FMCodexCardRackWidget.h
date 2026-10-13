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
DECLARE_MULTICAST_DELEGATE_OneParam(FFMCodexRackCardSelectionRequested, FName);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexCardRackWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexCardRackWidget(const FObjectInitializer& ObjectInitializer);

	void RefreshFromPresentation(
		const FFMCodexUMGCardRackViewModel& InPresentation,
		int64 TacticalPointSequence = INDEX_NONE, int32 TacticalPoint = 0,
		bool bAllowSkillReveal = true, const FString& MatchPhase = FString());

#if WITH_DEV_AUTOMATION_TESTS
	void AdvanceHandSkillRevealForTesting(float DeltaSeconds) { NativeTick(FGeometry(), DeltaSeconds); }
#endif

	const FFMCodexUMGCardRackViewModel& GetPresentation() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetRenderedCardWidgets() const;
	int32 GetRenderedCellCount() const;
	EFMCodexUMGCardInteractionState GetCellInteractionState(
		int32 StableIndex) const;

	// Presentation-only emphasis for the Theater candidate grid; default hand styling is unchanged.
	void SetProminentDraftSelection(bool bProminent) { bProminentDraftSelection = bProminent; }

	FFMCodexRackCardDragStarted OnCardDragStarted;
	FFMCodexRackCardDragFinished OnCardDragFinished;
	FFMCodexRackCardSelectionRequested OnCardSelectionRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void HandleCardDragStarted(FName CardId, bool bGoalkeeper);
	void HandleCardDragFinished();
	void HandleCardSelectionRequested(FName CardId);

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

	bool bProminentDraftSelection = false;
	int32 RenderedCellCount = 0;
	// The rack survives child reconstruction. Retain identity here so passive
	// view/ACK refreshes neither restart nor prematurely finish a label reveal.
	int64 HandSkillSequence = INDEX_NONE;
	int32 HandSkillTacticalPoint = 0;
	TMap<FName, FString> SeenHandSkillLabels;
	TMap<FName, float> HandSkillRevealElapsed;
	bool bHandSkillRevealAllowed = true;
	FString HandSkillMatchPhase;
};
