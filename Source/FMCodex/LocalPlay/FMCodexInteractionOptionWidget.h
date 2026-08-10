#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexInteractionOptionWidget.generated.h"

class UButton;
class UTextBlock;
class SWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(
	FFMCodexInteractionSimpleOptionRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionCardOptionRequested, FName, CardId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FFMCodexInteractionDeploymentOptionRequested,
	FName, CardId,
	FName, SlotId,
	bool, bGoalkeeper);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionBranchOptionRequested,
	EFMCodexUMGBranchIntent, Intent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionOneOnOneOptionRequested,
	EFMCodexUMGOneOnOneChoice, Choice);

UCLASS(Blueprintable)
class FMCODEX_API UFMCodexInteractionOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureSimple(const FString& InLabel);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureCard(const FString& InLabel, FName InCardId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureDeployment(
		const FString& InLabel,
		FName InCardId,
		FName InSlotId,
		bool bInGoalkeeper);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureBranch(
		const FString& InLabel,
		EFMCodexUMGBranchIntent InIntent);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureOneOnOne(
		const FString& InLabel,
		EFMCodexUMGOneOnOneChoice InChoice);

	const FString& GetLabel() const;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionSimpleOptionRequested OnSimpleRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionCardOptionRequested OnCardRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionDeploymentOptionRequested OnDeploymentRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionBranchOptionRequested OnBranchRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Intent")
	FFMCodexInteractionOneOnOneOptionRequested OnOneOnOneRequested;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	enum class EBindingMode : uint8
	{
		None,
		Simple,
		Card,
		Deployment,
		Branch,
		OneOnOne
	};

	void BuildWidgetTree();
	void RefreshVisuals();
	void BindConfiguredHandler();

	UFUNCTION()
	void HandleSimpleClicked();

	UFUNCTION()
	void HandleCardClicked();

	UFUNCTION()
	void HandleDeploymentClicked();

	UFUNCTION()
	void HandleBranchClicked();

	UFUNCTION()
	void HandleOneOnOneClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	FString Label;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	FName CardId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	FName SlotId = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	bool bGoalkeeper = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	EFMCodexUMGBranchIntent BranchIntent =
		EFMCodexUMGBranchIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Local Match|Interaction Presentation",
		meta = (AllowPrivateAccess = "true"))
	EFMCodexUMGOneOnOneChoice OneOnOneChoice =
		EFMCodexUMGOneOnOneChoice::None;

	UPROPERTY(Transient)
	TObjectPtr<UButton> OptionButton;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OptionLabelText;

	EBindingMode BindingMode = EBindingMode::None;
};
