#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "FMCodexLocalMatchUMGPresentation.h"

#include "FMCodexInteractionOptionWidget.generated.h"

class UButton;
class USizeBox;
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FFMCodexInteractionTacticalDetailRequested, FName, SkillId);

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
	void ConfigureTacticalCard(
		const FString& InLabel,
		const FString& InSecondaryLabel,
		FName InSkillId);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureDeployment(
		const FString& InLabel,
		FName InCardId,
		FName InSlotId,
		bool bInGoalkeeper);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureBranch(
		const FString& InLabel,
		const FString& InSecondaryLabel,
		EFMCodexUMGBranchIntent InIntent);

	UFUNCTION(BlueprintCallable, Category = "Local Match|Interaction Presentation")
	void ConfigureOneOnOne(
		const FString& InLabel,
		const FString& InSecondaryLabel,
		EFMCodexUMGOneOnOneChoice InChoice);

	const FString& GetLabel() const;
	const FString& GetSecondaryLabel() const;
	bool IsTacticalCard() const;

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

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Presentation")
	FFMCodexInteractionTacticalDetailRequested OnTacticalDetailRequested;

	UPROPERTY(BlueprintAssignable, Category = "Local Match|Interaction Presentation")
	FFMCodexInteractionTacticalDetailRequested OnTacticalDetailDismissed;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

private:
	enum class EBindingMode : uint8
	{
		None,
		Simple,
		Card,
		TacticalCard,
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
	void HandleTacticalHovered();

	UFUNCTION()
	void HandleTacticalUnhovered();

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
	FString SecondaryLabel;

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
	TObjectPtr<USizeBox> OptionBounds;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OptionLabelText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OptionSecondaryText;

	EBindingMode BindingMode = EBindingMode::None;
};
