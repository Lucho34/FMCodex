#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexInteractionPanelWidget.generated.h"

class UButton;
class UFMCodexInteractionOptionWidget;
class UFMCodexPlayerCardWidget;
class UHorizontalBox;
class UTextBlock;
class UVerticalBox;
class UWrapBox;
class SWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionStartMatchRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionBeginAttackRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FFMCodexInteractionDeployOrdinaryRequested,
	FName, CardId,
	FName, SlotId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionDeployGoalkeeperRequested,
	FName, SlotId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionFinishDeploymentRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionCarrierRequested, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionMarkerRequested, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionSkillRequested, FName, SkillId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionRunnerRequested, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionHelperRequested, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionDeclineRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionNoLegalRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionBranchRequested,
	EFMCodexUMGBranchIntent, Intent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionOneOnOneRequested,
	EFMCodexUMGOneOnOneChoice, Choice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionContinueRequested);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexInteractionPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFMCodexInteractionPanelWidget(
		const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void RefreshFromPresentation(
		const FFMCodexUMGInteractionViewModel& InPresentation);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void SetInteractionBlocked(bool bBlocked);

	const FFMCodexUMGInteractionViewModel& GetPresentation() const;
	bool IsInteractionBlocked() const;
	const TArray<TObjectPtr<UFMCodexPlayerCardWidget>>&
		GetRenderedCandidateCardWidgets() const;
	const TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>&
		GetRenderedOptionWidgets() const;

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestStartMatch();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestBeginAttack();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestDeployment(FName CardId, FName SlotId, bool bGoalkeeper);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestFinishDeployment();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestCarrier(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestMarker(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestSkill(FName SkillId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestRunner(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestHelper(FName CardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestDecline();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestNoLegal();

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestBranch(EFMCodexUMGBranchIntent Intent);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestOneOnOne(EFMCodexUMGOneOnOneChoice Choice);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Intent")
	void RequestContinue();

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionStartMatchRequested OnStartMatchRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionBeginAttackRequested OnBeginAttackRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionDeployOrdinaryRequested OnDeployOrdinaryRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionDeployGoalkeeperRequested OnDeployGoalkeeperRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionFinishDeploymentRequested OnFinishDeploymentRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionCarrierRequested OnCarrierRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionMarkerRequested OnMarkerRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionSkillRequested OnSkillRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionRunnerRequested OnRunnerRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionHelperRequested OnHelperRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionDeclineRequested OnDeclineRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionNoLegalRequested OnNoLegalRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionBranchRequested OnBranchRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionOneOnOneRequested OnOneOnOneRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionContinueRequested OnContinueRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	void BuildWidgetTree();
	void RefreshVisuals();
	void RefreshCandidateChoices();
	UFMCodexInteractionOptionWidget* MakeOptionWidget(const FName Name);

	UFUNCTION()
	void HandleStartClicked();

	UFUNCTION()
	void HandleBeginClicked();

	UFUNCTION()
	void HandleFinishClicked();

	UFUNCTION()
	void HandleDeclineClicked();

	UFUNCTION()
	void HandleNoLegalClicked();

	UFUNCTION()
	void HandleContinueClicked();

	UFUNCTION()
	void HandleDeploymentOption(
		FName CardId, FName SlotId, bool bGoalkeeper);

	UFUNCTION()
	void HandleCarrierOption(FName CardId);

	UFUNCTION()
	void HandleMarkerOption(FName CardId);

	UFUNCTION()
	void HandleSkillOption(FName SkillId);

	UFUNCTION()
	void HandleRunnerOption(FName CardId);

	UFUNCTION()
	void HandleHelperOption(FName CardId);

	UFUNCTION()
	void HandleBranchOption(EFMCodexUMGBranchIntent Intent);

	UFUNCTION()
	void HandleOneOnOneOption(EFMCodexUMGOneOnOneChoice Choice);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	FFMCodexUMGInteractionViewModel Presentation;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Interaction Presentation")
	TSubclassOf<UFMCodexPlayerCardWidget> PlayerCardWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Local Match|Interaction Presentation")
	TSubclassOf<UFMCodexInteractionOptionWidget> OptionWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> KickerText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ActorText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ContextText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ChoiceSectionText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> EmptyStateText;

	UPROPERTY(Transient)
	TObjectPtr<UHorizontalBox> CandidateCardsBody;

	UPROPERTY(Transient)
	TObjectPtr<UWrapBox> ChoiceOptionsBody;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> BeginButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> FinishButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> DeclineButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> NoLegalButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexPlayerCardWidget>>
		RenderedCandidateCardWidgets;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UFMCodexInteractionOptionWidget>>
		RenderedOptionWidgets;

	bool bInteractionBlocked = false;
};
