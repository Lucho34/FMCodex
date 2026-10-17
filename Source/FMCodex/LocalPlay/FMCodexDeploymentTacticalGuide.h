#pragma once
#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "FMCodexLocalMatchUMGPresentation.h"
#include "FMCodexDeploymentTacticalGuide.generated.h"

class UCanvasPanel;
class SHorizontalBox;
class SBox;
class SButton;
class IInputProcessor;
class UFMCodexTacticExplainerWidget;

/** Public TP information only. No backend, command delegate or gameplay state. */
UCLASS()
class FMCODEX_API UFMCodexDeploymentTacticalGuide : public UWidget
{
 GENERATED_BODY()
public:
 static TArray<FName> Project(const FFMCodexUMGMatchScreenViewModel& View);
 void Refresh(const FFMCodexUMGMatchScreenViewModel& View, bool bResultDismissed);
 void AdvanceEntrance(float DeltaSeconds, FVector2D ViewportSize);
 void AttachDetailLayer(UCanvasPanel* Layer);
 void UpdateDetailBounds(const FGeometry& Screen, const FGeometry& Pitch);
 void Toggle(FName Tactic);
 void Close();
 void ResetContext();
 /** Observer path: outside clicks close but remain unhandled for the real target. */
 bool HandlePointerDown(FVector2D Position);
 bool HandleEscape();
 const TArray<FName>& GetTactics() const { return Tactics; }
 FName GetSelection() const { return Selection; }
 float GetEntranceProgress() const;
 UFMCodexTacticExplainerWidget* GetExplainer() const { return Explainer; }
 TSharedPtr<SButton> GetChip(FName Id) const;
 FGeometry GetDetailGeometry() const;
 virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:
 virtual TSharedRef<SWidget> RebuildWidget() override;
private:
 void RefreshChips();
 void StopObservingInput();
 UPROPERTY(Transient) TObjectPtr<UFMCodexTacticExplainerWidget> Explainer;
 TArray<FName> Tactics;
 FName Selection;
 int64 Sequence=INDEX_NONE;
 int32 TP=0;
 EInitialTurnOrderPlayer Attacker=EInitialTurnOrderPlayer::None;
 bool bSeenContext=false, bFirstEntranceFrame=false;
 float EntranceElapsed=.4f;
 TSharedPtr<SHorizontalBox> Chips;
 TMap<FName,TSharedPtr<SButton>> Buttons;
 TSharedPtr<IInputProcessor> InputObserver;
};
